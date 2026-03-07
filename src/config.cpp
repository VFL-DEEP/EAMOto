#include "config.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <ctime>

namespace eamoto {
namespace config {

std::string getHomeDir() {
    const char* homeData = std::getenv("HOME");
    if (!homeData) {
        homeData = getpwuid(getuid())->pw_dir;
    }
    return std::string(homeData);
}

std::string getKeyfilePath() {
    return getHomeDir() + "/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile";
}

std::time_t parseIso8601(const std::string& timeStr) {
    if (timeStr.empty()) return 0;
    std::tm t = {};
    if (sscanf(timeStr.c_str(), "%d-%d-%dT%d:%d:%d", 
        &t.tm_year, &t.tm_mon, &t.tm_mday, 
        &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        t.tm_isdst = -1;
        return timegm(&t); // treating as UTC
    }
    return 0;
}

bool hasValidToken() {
    std::string path = getKeyfilePath();
    if (!std::filesystem::exists(path)) return false;

    std::ifstream file(path);
    std::string line;
    std::string expTimeStr;

    bool inTargetSection = false;
    while (std::getline(file, line)) {
        if (line == "[io/github/achetagames/epic_asset_manager]") {
            inTargetSection = true;
        } else if (line.rfind("[", 0) == 0 && inTargetSection) {
            inTargetSection = false; // Left section
        }

        if (inTargetSection && line.rfind("token-expiration=", 0) == 0) {
            expTimeStr = line.substr(17);
            // remove quotes if arbitrary
            if (expTimeStr.front() == '\'') expTimeStr.erase(0, 1);
            if (expTimeStr.back() == '\'') expTimeStr.pop_back();
            break;
        }
    }

    if (expTimeStr.empty()) return false;

    std::time_t expTime = parseIso8601(expTimeStr);
    std::time_t nowTime = std::time(nullptr);

    // Provide a small buffer (e.g., 5 mins) before explicit expiration
    return (expTime > nowTime + 300);
}

bool writeTokenDetails(const eamoto::auth::TokenDetails& tokens) {
    std::string path = getKeyfilePath();
    std::vector<std::string> lines;
    
    // Read existing file
    if (std::filesystem::exists(path)) {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
    }

    // Modify or append in memory
    bool inTargetSection = false;
    bool sectionExists = false;

    // We will collect indices to either replace or add if they don't exist
    int sectionEndIdx = -1;
    bool foundToken = false, foundRefreshToken = false, foundTokenExp = false, foundRefreshTokenExp = false;

    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i] == "[io/github/achetagames/epic_asset_manager]") {
            inTargetSection = true;
            sectionExists = true;
            sectionEndIdx = i + 1;
        } else if (lines[i].rfind("[", 0) == 0 && inTargetSection) {
            inTargetSection = false;
            sectionEndIdx = i; // The line of next section
        }

        if (inTargetSection || (sectionExists && i == lines.size() - 1 && inTargetSection)) {
            if (inTargetSection && i == lines.size() - 1) sectionEndIdx = i + 1;
            
            if (lines[i].rfind("token=", 0) == 0) {
                lines[i] = "token='" + tokens.access_token + "'";
                foundToken = true;
            } else if (lines[i].rfind("refresh-token=", 0) == 0) {
                lines[i] = "refresh-token='" + tokens.refresh_token + "'";
                foundRefreshToken = true;
            } else if (lines[i].rfind("token-expiration=", 0) == 0) {
                lines[i] = "token-expiration='" + tokens.expires_at + "'";
                foundTokenExp = true;
            } else if (lines[i].rfind("refresh-token-expiration=", 0) == 0) {
                lines[i] = "refresh-token-expiration='" + tokens.refresh_expires_at + "'";
                foundRefreshTokenExp = true;
            }
        }
    }

    if (!sectionExists) {
        lines.push_back("");
        lines.push_back("[io/github/achetagames/epic_asset_manager]");
        lines.push_back("token='" + tokens.access_token + "'");
        lines.push_back("refresh-token='" + tokens.refresh_token + "'");
        lines.push_back("token-expiration='" + tokens.expires_at + "'");
        lines.push_back("refresh-token-expiration='" + tokens.refresh_expires_at + "'");
    } else {
        // If some lines were not found in the section, insert them at the end of the section
        auto insertAt = lines.begin() + sectionEndIdx;
        
        if (!foundRefreshTokenExp) { insertAt = lines.insert(insertAt, "refresh-token-expiration='" + tokens.refresh_expires_at + "'"); }
        if (!foundTokenExp)        { insertAt = lines.insert(insertAt, "token-expiration='" + tokens.expires_at + "'"); }
        if (!foundRefreshToken)    { insertAt = lines.insert(insertAt, "refresh-token='" + tokens.refresh_token + "'"); }
        if (!foundToken)           { insertAt = lines.insert(insertAt, "token='" + tokens.access_token + "'"); }
    }

    // Ensure directory exists
    std::filesystem::path p{path};
    if (!std::filesystem::exists(p.parent_path())) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) return false;
    for (const auto& l : lines) {
        out << l << "\n";
    }

    return true;
}

} // namespace config
} // namespace eamoto
