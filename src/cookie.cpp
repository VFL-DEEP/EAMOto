#include "cookie.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <filesystem>

namespace eamoto {
namespace cookie {

struct ProfileEntry {
    std::string name;
    std::string path;
    bool isRelative = true;
    bool isDefault  = false;
};

static std::string findProfileFromIni(const std::string& baseDir)
{
    std::string iniPath = baseDir + "/profiles.ini";
    if (!std::filesystem::exists(iniPath)) return "";

    std::ifstream file(iniPath);
    if (!file.is_open()) return "";

    std::string line;
    std::string installDefault;
    std::vector<ProfileEntry> profiles;

    enum class SectionType { None, Install, Profile, Other };
    SectionType currentSection = SectionType::None;
    ProfileEntry currentProfile;

    auto flushProfile = [&]() {
        if (currentSection == SectionType::Profile && !currentProfile.path.empty())
            profiles.push_back(currentProfile);
        currentProfile = ProfileEntry{};
    };

    while (std::getline(file, line)) {
        while (!line.empty() && (line.back()=='\r'||line.back()==' '||line.back()=='\t'))
            line.pop_back();
        if (line.empty()) continue;

        if (line.front() == '[') {
            flushProfile();
            if      (line.find("[Install") == 0) currentSection = SectionType::Install;
            else if (line.find("[Profile") == 0) currentSection = SectionType::Profile;
            else                                 currentSection = SectionType::Other;
            continue;
        }

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        std::string key   = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        if (currentSection == SectionType::Install) {
            if (key == "Default") installDefault = value;
        } else if (currentSection == SectionType::Profile) {
            if      (key == "Name")       currentProfile.name       = value;
            else if (key == "Path")       currentProfile.path       = value;
            else if (key == "IsRelative") currentProfile.isRelative = (value == "1");
            else if (key == "Default")    currentProfile.isDefault  = (value == "1");
        }
    }
    flushProfile();

    auto resolvePath = [&](const std::string& p, bool rel) -> std::string {
        std::string full = rel ? (baseDir + "/" + p) : p;
        return std::filesystem::exists(full) ? full : "";
    };

    if (!installDefault.empty()) {
        std::string full = baseDir + "/" + installDefault;
        if (std::filesystem::exists(full)) return full;
    }
    for (const auto& p : profiles) {
        if (p.isDefault) {
            std::string full = resolvePath(p.path, p.isRelative);
            if (!full.empty()) return full;
        }
    }
    for (const auto& p : profiles) {
        if (p.path.find("default-release")  != std::string::npos ||
            p.path.find("Default (release)") != std::string::npos ||
            p.name.find("default-release")  != std::string::npos ||
            p.name.find("Default (release)") != std::string::npos) {
            std::string full = resolvePath(p.path, p.isRelative);
            if (!full.empty()) return full;
        }
    }
    for (const auto& p : profiles) {
        std::string full = resolvePath(p.path, p.isRelative);
        if (!full.empty()) return full;
    }
    return "";
}

static bool profileHasEpicCookies(const std::string& profilePath)
{
    std::string dbPath = profilePath + "/cookies.sqlite";
    if (!std::filesystem::exists(dbPath)) return false;

    std::string tmpPath = std::filesystem::temp_directory_path().string()
                        + "/eamoto_probe_" + std::to_string(getpid()) + ".sqlite";
    try {
        std::filesystem::copy_file(dbPath, tmpPath,
            std::filesystem::copy_options::overwrite_existing);
        if (std::filesystem::exists(dbPath + "-wal"))
            std::filesystem::copy_file(dbPath + "-wal", tmpPath + "-wal",
                std::filesystem::copy_options::overwrite_existing);
        if (std::filesystem::exists(dbPath + "-shm"))
            std::filesystem::copy_file(dbPath + "-shm", tmpPath + "-shm",
                std::filesystem::copy_options::overwrite_existing);
    } catch (...) { return false; }

    sqlite3* db = nullptr;
    bool found = false;
    if (sqlite3_open(tmpPath.c_str(), &db) == SQLITE_OK) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT COUNT(*) FROM moz_cookies "
                          "WHERE host LIKE '%epicgames%' AND name = 'EPIC_SSO'";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                found = sqlite3_column_int(stmt, 0) > 0;
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
    std::filesystem::remove(tmpPath);
    std::filesystem::remove(tmpPath + "-wal");
    std::filesystem::remove(tmpPath + "-shm");
    return found;
}

std::string getDefaultFirefoxProfilePath()
{
    std::string home = eamoto::utils::getHomeDir();
    if (home.empty()) {
        std::cerr << "Cannot determine home directory." << std::endl;
        return "";
    }

    // Zen first — active Epic session lives there, not Firefox
    std::vector<std::string> baseDirs = {
        home + "/.zen",
        home + "/.mozilla/firefox",
        home + "/.var/app/io.zen_browser.zen/.zen"
    };

    // First pass: profile that actually has Epic cookies
    for (const auto& baseDir : baseDirs) {
        std::string result = findProfileFromIni(baseDir);
        if (!result.empty() && profileHasEpicCookies(result)) {
            std::cout << "Found Epic session in: " << result << std::endl;
            return result;
        }
    }

    // Second pass: any found profile
    for (const auto& baseDir : baseDirs) {
        std::string result = findProfileFromIni(baseDir);
        if (!result.empty()) return result;
    }

    return "";
}

std::string getEpicGamesCookies(const std::string& profilePath)
{
    std::string cookieDbPath = profilePath + "/cookies.sqlite";
    std::string walPath      = cookieDbPath + "-wal";
    std::string shmPath      = cookieDbPath + "-shm";

    if (!std::filesystem::exists(cookieDbPath)) {
        std::cerr << "Cookie DB not found: " << cookieDbPath << std::endl;
        return "";
    }

    std::string tempDbPath = std::filesystem::temp_directory_path().string()
                           + "/eamoto_cookies_" + std::to_string(getpid()) + ".sqlite";

    auto cleanupTempFiles = [&]() {
        std::filesystem::remove(tempDbPath);
        std::filesystem::remove(tempDbPath + "-wal");
        std::filesystem::remove(tempDbPath + "-shm");
    };

    try {
        std::filesystem::copy_file(cookieDbPath, tempDbPath,
            std::filesystem::copy_options::overwrite_existing);
        if (std::filesystem::exists(walPath))
            std::filesystem::copy_file(walPath, tempDbPath + "-wal",
                std::filesystem::copy_options::overwrite_existing);
        if (std::filesystem::exists(shmPath))
            std::filesystem::copy_file(shmPath, tempDbPath + "-shm",
                std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception& e) {
        std::cerr << "Failed to copy cookies db: " << e.what() << std::endl;
        return "";
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(tempDbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open cookie database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        cleanupTempFiles();
        return "";
    }

    // Deduplicate by name using MAX(lastAccessed) — picks the most recently used
    // cookie when the same name appears multiple times (e.g. two sessions).
    // EPIC_SESSION_AP is required for auth code to be non-null.
    const char* sql =
        "SELECT name, value FROM moz_cookies "
        "WHERE host LIKE '%epicgames%' "
        "AND rowid IN ("
        "  SELECT rowid FROM moz_cookies c2 "
        "  WHERE c2.host LIKE '%epicgames%' "
        "  AND c2.lastAccessed = ("
        "    SELECT MAX(c3.lastAccessed) FROM moz_cookies c3 "
        "    WHERE c3.host LIKE '%epicgames%' AND c3.name = c2.name"
        "  )"
        ")";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        cleanupTempFiles();
        return "";
    }

    std::string cookieStr;
    int rowCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* namePtr  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* valuePtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (namePtr && valuePtr) {
            cookieStr += std::string(namePtr) + "=" + std::string(valuePtr) + "; ";
            rowCount++;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    cleanupTempFiles();

    if (rowCount == 0) {
        std::cerr << "No epicgames.com cookies found." << std::endl;
        std::cerr << "Please log in to Epic Games in Zen Browser or Firefox." << std::endl;
        return "";
    }

    return cookieStr;
}

} // namespace cookie
} // namespace eamoto
