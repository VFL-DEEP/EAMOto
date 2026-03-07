#pragma once
#include <string>

namespace eamoto {
namespace cookie {

struct CookieData {
    std::string name;
    std::string value;
};

// Finds the default Firefox profile directory
std::string getDefaultFirefoxProfilePath();

// Reads epicgames cookies from the specified profile path and formats them as a valid HTTP Cookie header string
std::string getEpicGamesCookies(const std::string& profilePath);

} // namespace cookie
} // namespace eamoto
