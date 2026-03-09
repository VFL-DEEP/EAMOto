#pragma once
#include <string>

namespace eamoto
{
    namespace utils
    {

        // Returns the current user's home directory.
        // Uses $HOME environment variable first, falls back to getpwuid().
        // Returns empty string if home directory cannot be determined.
        std::string getHomeDir();

    } // namespace utils
} // namespace eamoto
