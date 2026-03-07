#include "cookie.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdlib>
#include <filesystem>

namespace eamoto {
namespace cookie {

std::string getHomeDir() {
    const char* homeData = std::getenv("HOME");
    if (!homeData) {
        homeData = getpwuid(getuid())->pw_dir;
    }
    return std::string(homeData);
}

std::string getDefaultFirefoxProfilePath() {
    std::string home = getHomeDir();
    // Prioritize specific user file if present
    std::string directPath = home + "/.mozilla/firefox/s850yvut.default-release";
    if (std::filesystem::exists(directPath)) {
        return directPath;
    }

    std::vector<std::string> baseDirs = {
        home + "/.mozilla/firefox",
        home + "/.zen",
        home + "/.var/app/io.zen_browser.zen/.zen"
    };

    for (const auto& baseDir : baseDirs) {
        std::string iniPath = baseDir + "/profiles.ini";
        if (std::filesystem::exists(iniPath)) {
            std::ifstream file(iniPath);
            std::string line;
            std::string installDefault;
            std::vector<std::string> profiles;
            while (std::getline(file, line)) {
                if (line.rfind("Default=", 0) == 0 && line.length() > 8) {
                    std::string def = line.substr(8);
                    if (def != "1") installDefault = def;
                } else if (line.rfind("Path=", 0) == 0 && line.length() > 5) {
                    profiles.push_back(line.substr(5));
                }
            }
            if (!installDefault.empty() && std::filesystem::exists(baseDir + "/" + installDefault)) {
                return baseDir + "/" + installDefault;
            }
            for (const auto& p : profiles) {
                if (p.find("default-release") != std::string::npos && std::filesystem::exists(baseDir + "/" + p)) {
                    return baseDir + "/" + p;
                }
            }
        }
    }
    return "";
}

std::string getEpicGamesCookies(const std::string& profilePath) {
    std::string cookieDbPath = profilePath + "/cookies.sqlite";
    if (!std::filesystem::exists(cookieDbPath)) {
        std::cerr << "Cookie DB not found: " << cookieDbPath << std::endl;
        return "";
    }

    // SQLite locks the file. Let's make a copy to avoid SQLITE_BUSY if Firefox is open
    std::string tempDbPath = "/tmp/eamoto_cookies.sqlite";
    try {
        std::filesystem::copy_file(cookieDbPath, tempDbPath, std::filesystem::copy_options::overwrite_existing);
    } catch (std::exception& e) {
        std::cerr << "Failed to copy cookies db: " << e.what() << std::endl;
        return "";
    }

    sqlite3* db;
    if (sqlite3_open(tempDbPath.c_str(), &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return "";
    }

    const char* sql = "SELECT name, value FROM moz_cookies WHERE host LIKE '%epicgames.com'";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return "";
    }

    std::string cookieStr;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cookieStr += name + "=" + value + "; ";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::filesystem::remove(tempDbPath);

    return cookieStr;
}

} // namespace cookie
} // namespace eamoto
