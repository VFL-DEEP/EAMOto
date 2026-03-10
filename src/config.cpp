#include "config.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <ctime>

namespace eamoto
{
    namespace config
    {

        std::string getKeyfilePath()
        {
            std::string home = eamoto::utils::getHomeDir();
            if (home.empty()) return "";
            return home + "/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile";
        }

        bool hasValidToken()
        {
            // Always return false — we refresh token on every launch
            return false;
        }

        bool writeTokenDetails(const eamoto::auth::TokenDetails &tokens)
        {
            std::string path = getKeyfilePath();
            std::vector<std::string> lines;

            if (std::filesystem::exists(path)) {
                std::ifstream file(path);
                std::string line;
                while (std::getline(file, line))
                    lines.push_back(line);
            }

            bool inSection = false, sectionExists = false;
            int sectionEndIdx = -1;
            bool foundToken = false, foundRefresh = false, foundExp = false, foundRefreshExp = false;

            for (size_t i = 0; i < lines.size(); ++i) {
                if (lines[i] == "[io/github/achetagames/epic_asset_manager]") {
                    inSection = true; sectionExists = true; sectionEndIdx = i + 1;
                } else if (lines[i].rfind("[", 0) == 0 && inSection) {
                    inSection = false; sectionEndIdx = i;
                }
                if (inSection) {
                    if (i == lines.size() - 1) sectionEndIdx = i + 1;
                    if      (lines[i].rfind("refresh-token=", 0) == 0)            { lines[i] = "refresh-token=" + tokens.refresh_token; foundRefresh = true; }
                    else if (lines[i].rfind("token=", 0) == 0)                    { lines[i] = "token=" + tokens.access_token; foundToken = true; }
                    else if (lines[i].rfind("token-expiration=", 0) == 0)         { lines[i] = "token-expiration=" + tokens.expires_at; foundExp = true; }
                    else if (lines[i].rfind("refresh-token-expiration=", 0) == 0) { lines[i] = "refresh-token-expiration=" + tokens.refresh_expires_at; foundRefreshExp = true; }
                }
            }

            if (!sectionExists) {
                lines.push_back("");
                lines.push_back("[io/github/achetagames/epic_asset_manager]");
                lines.push_back("token=" + tokens.access_token);
                lines.push_back("refresh-token=" + tokens.refresh_token);
                lines.push_back("token-expiration=" + tokens.expires_at);
                lines.push_back("refresh-token-expiration=" + tokens.refresh_expires_at);
            } else {
                auto it = lines.begin() + sectionEndIdx;
                if (!foundRefreshExp) it = lines.insert(it, "refresh-token-expiration=" + tokens.refresh_expires_at);
                if (!foundExp)        it = lines.insert(it, "token-expiration=" + tokens.expires_at);
                if (!foundRefresh)    it = lines.insert(it, "refresh-token=" + tokens.refresh_token);
                if (!foundToken)      it = lines.insert(it, "token=" + tokens.access_token);
            }

            std::filesystem::path p{path};
            if (!std::filesystem::exists(p.parent_path()))
                std::filesystem::create_directories(p.parent_path());

            std::ofstream out(path, std::ios::trunc);
            if (!out.is_open()) return false;
            for (const auto &l : lines) out << l << "\n";
            return true;
        }

    } // namespace config
} // namespace eamoto
