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

namespace eamoto
{
    namespace cookie
    {

        // Represents a parsed profile entry from profiles.ini
        struct ProfileEntry
        {
            std::string name;
            std::string path;
            bool isRelative = true;
            bool isDefault = false;
        };

        // Finds the best profile directory from a given browser base directory (e.g. ~/.mozilla/firefox)
        static std::string findProfileFromIni(const std::string &baseDir)
        {
            std::string iniPath = baseDir + "/profiles.ini";
            if (!std::filesystem::exists(iniPath))
                return "";

            std::ifstream file(iniPath);
            std::string line;

            std::string installDefault; // Default= from [Install*] section
            std::vector<ProfileEntry> profiles;

            enum class SectionType
            {
                None,
                Install,
                Profile,
                Other
            };
            SectionType currentSection = SectionType::None;
            ProfileEntry currentProfile;

            auto flushProfile = [&]()
            {
                if (currentSection == SectionType::Profile && !currentProfile.path.empty())
                {
                    profiles.push_back(currentProfile);
                }
                currentProfile = ProfileEntry{};
            };

            while (std::getline(file, line))
            {
                // Trim trailing whitespace/carriage return
                while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
                {
                    line.pop_back();
                }

                // Detect section headers
                if (!line.empty() && line.front() == '[')
                {
                    flushProfile();
                    if (line.find("[Install") == 0)
                    {
                        currentSection = SectionType::Install;
                    }
                    else if (line.find("[Profile") == 0)
                    {
                        currentSection = SectionType::Profile;
                    }
                    else
                    {
                        currentSection = SectionType::Other;
                    }
                    continue;
                }

                // Parse key=value pairs
                auto eqPos = line.find('=');
                if (eqPos == std::string::npos)
                    continue;

                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);

                if (currentSection == SectionType::Install)
                {
                    if (key == "Default")
                    {
                        installDefault = value;
                    }
                }
                else if (currentSection == SectionType::Profile)
                {
                    if (key == "Name")
                    {
                        currentProfile.name = value;
                    }
                    else if (key == "Path")
                    {
                        currentProfile.path = value;
                    }
                    else if (key == "IsRelative")
                    {
                        currentProfile.isRelative = (value == "1");
                    }
                    else if (key == "Default")
                    {
                        currentProfile.isDefault = (value == "1");
                    }
                }
            }
            flushProfile(); // flush last profile

            // Helper to resolve a profile path
            auto resolvePath = [&](const std::string &profilePath, bool isRelative) -> std::string
            {
                if (isRelative)
                {
                    return baseDir + "/" + profilePath;
                }
                return profilePath;
            };

            // Priority 1: Install section default (most reliable)
            if (!installDefault.empty())
            {
                std::string fullPath = baseDir + "/" + installDefault;
                if (std::filesystem::exists(fullPath))
                    return fullPath;
            }

            // Priority 2: Profile with Default=1
            for (const auto &p : profiles)
            {
                if (p.isDefault)
                {
                    std::string fullPath = resolvePath(p.path, p.isRelative);
                    if (std::filesystem::exists(fullPath))
                        return fullPath;
                }
            }

            // Priority 3: Profile with "default-release" in path or name
            for (const auto &p : profiles)
            {
                if (p.path.find("default-release") != std::string::npos ||
                    p.name.find("default-release") != std::string::npos)
                {
                    std::string fullPath = resolvePath(p.path, p.isRelative);
                    if (std::filesystem::exists(fullPath))
                        return fullPath;
                }
            }

            // Priority 4: Any profile that exists on disk
            for (const auto &p : profiles)
            {
                std::string fullPath = resolvePath(p.path, p.isRelative);
                if (std::filesystem::exists(fullPath))
                    return fullPath;
            }

            return "";
        }

        std::string getDefaultFirefoxProfilePath()
        {
            std::string home = eamoto::utils::getHomeDir();

            if (home.empty())
            {
                std::cerr << "Cannot determine home directory. Please set HOME environment variable." << std::endl;
                return "";
            }

            // Search across Firefox and Zen Browser base directories
            std::vector<std::string> baseDirs = {
                home + "/.mozilla/firefox",
                home + "/.zen",
                home + "/.var/app/io.zen_browser.zen/.zen"};

            for (const auto &baseDir : baseDirs)
            {
                std::string result = findProfileFromIni(baseDir);
                if (!result.empty())
                    return result;
            }

            return "";
        }

        std::string getEpicGamesCookies(const std::string &profilePath)
        {
            std::string cookieDbPath = profilePath + "/cookies.sqlite";
            std::string walPath = cookieDbPath + "-wal";
            std::string shmPath = cookieDbPath + "-shm";

            if (!std::filesystem::exists(cookieDbPath))
            {
                std::cerr << "Cookie DB not found: " << cookieDbPath << std::endl;
                return "";
            }

            // SQLite locks the file. Copy the database (and WAL/SHM files if Firefox is using WAL mode)
            // Use PID in temp filename to avoid race conditions with concurrent instances
            std::string tempDbPath = std::filesystem::temp_directory_path().string() +
                                     "/eamoto_cookies_" + std::to_string(getpid()) + ".sqlite";
            try
            {
                std::filesystem::copy_file(cookieDbPath, tempDbPath, std::filesystem::copy_options::overwrite_existing);
                // Firefox uses WAL mode — the latest data lives in the WAL file when the browser is running
                if (std::filesystem::exists(walPath))
                {
                    std::filesystem::copy_file(walPath, tempDbPath + "-wal", std::filesystem::copy_options::overwrite_existing);
                }
                if (std::filesystem::exists(shmPath))
                {
                    std::filesystem::copy_file(shmPath, tempDbPath + "-shm", std::filesystem::copy_options::overwrite_existing);
                }
            }
            catch (std::exception &e)
            {
                std::cerr << "Failed to copy cookies db: " << e.what() << std::endl;
                return "";
            }

            // Helper lambda to clean up all temp files
            auto cleanupTempFiles = [&]()
            {
                std::filesystem::remove(tempDbPath);
                std::filesystem::remove(tempDbPath + "-wal");
                std::filesystem::remove(tempDbPath + "-shm");
            };

            sqlite3 *db;
            if (sqlite3_open(tempDbPath.c_str(), &db))
            {
                std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
                cleanupTempFiles();
                return "";
            }

            // Fetch cookies from all epicgames.com host variants.
            // EPIC_SSO, EPIC_BEARER_TOKEN, EPIC_SESSION_AP live under '.epicgames.com'.
            // Use GROUP BY name to deduplicate — when the same cookie name appears
            // multiple times (e.g. from two browser profiles or sessions), prefer
            // the one with the longest value (most likely the freshest session token).
            const char *sql =
                "SELECT name, value FROM moz_cookies "
                "WHERE host LIKE '%epicgames%' "
                "GROUP BY name HAVING MAX(length(value))";
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
            {
                std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << "\n";
                sqlite3_close(db);
                cleanupTempFiles();
                return "";
            }

            std::string cookieStr;
            int rowCount = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                const char *namePtr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                const char *valuePtr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                if (namePtr && valuePtr)
                {
                    cookieStr += std::string(namePtr) + "=" + std::string(valuePtr) + "; ";
                    rowCount++;
                }
            }

            if (rowCount == 0)
            {
                std::cerr << "No epicgames.com cookies found. Please log in to Epic Games in Firefox/Zen." << std::endl;
            }

            sqlite3_finalize(stmt);
            sqlite3_close(db);

            // Clean up all temp files
            cleanupTempFiles();

            return cookieStr;
        }

    } // namespace cookie
} // namespace eamoto
