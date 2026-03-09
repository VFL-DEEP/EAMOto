#include "utils.hpp"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace eamoto
{
    namespace utils
    {

        std::string getHomeDir()
        {
            const char *homeData = std::getenv("HOME");
            if (!homeData)
            {
                struct passwd *pw = getpwuid(getuid());
                if (!pw)
                {
                    std::cerr << "Cannot determine home directory: getpwuid() failed" << std::endl;
                    return "";
                }
                homeData = pw->pw_dir;
            }
            return std::string(homeData);
        }

    } // namespace utils
} // namespace eamoto
