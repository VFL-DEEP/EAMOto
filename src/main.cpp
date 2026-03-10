#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "cookie.hpp"
#include "auth.hpp"

using namespace eamoto;

int main()
{
    std::cout << "EAMOto: Epic Asset Manager Auto-Auth" << std::endl;
    std::cout << "─────────────────────────────────────" << std::endl;

    // Step 1: Find browser profile
    std::string profileFolder = cookie::getDefaultFirefoxProfilePath();
    if (profileFolder.empty()) {
        std::cerr << "❌ Could not locate a browser profile with Epic session." << std::endl;
        return 1;
    }

    // Step 2: Extract cookies
    std::string cookieStr = cookie::getEpicGamesCookies(profileFolder);
    if (cookieStr.empty()) {
        std::cerr << "❌ No Epic cookies found. Please log in to Epic Games in your browser." << std::endl;
        return 1;
    }

    // Step 3: Launch EAM
    std::cout << "Launching Epic Asset Manager..." << std::endl;
    std::system("flatpak run io.github.achetagames.epic_asset_manager &");
    std::cout << "Waiting for login screen..." << std::endl;
    sleep(5);

    // Step 4: Get auth code AFTER EAM is open
    std::cout << "Requesting authorization code..." << std::endl;
    auto authCode = auth::getAuthorizationCode(cookieStr);
    if (!authCode) {
        std::cerr << "❌ Failed to get authorization code." << std::endl;
        return 1;
    }

    std::cout << "✅ Auth Code received." << std::endl;

    // Step 5: Copy code to clipboard
    std::string copyCmd = "echo -n '" + authCode.value() + "' | xclip -selection clipboard";
    std::system(copyCmd.c_str());
    sleep(1);

    // Step 6: Focus EAM window and paste
    std::system("xdotool search --onlyvisible --class 'epic_asset_manager' windowfocus key ctrl+v");
    sleep(1);
    std::system("xdotool search --onlyvisible --class 'epic_asset_manager' windowfocus key Return");

    std::cout << "✓ Done! Code entered into EAM." << std::endl;

    return 0;
}
