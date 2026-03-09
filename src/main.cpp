#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "cookie.hpp"
#include "auth.hpp"
#include "config.hpp"

using namespace eamoto;

void runEAM()
{
    // Kill any existing EAM instance so it re-reads settings fresh
    std::system("flatpak kill io.github.achetagames.epic_asset_manager 2>/dev/null");
    sleep(1);

    std::cout << "Starting Epic Asset Manager..." << std::endl;
    std::system("flatpak run io.github.achetagames.epic_asset_manager");
}

int main()
{
    std::cout << "EAMOto: Epic Asset Manager Auto-Auth" << std::endl;
    std::cout << "Fetching fresh token from Firefox/Zen cookies..." << std::endl;

    // Step 1: Extract cookies
    std::string profileFolder = cookie::getDefaultFirefoxProfilePath();
    if (profileFolder.empty())
    {
        std::cerr << "Could not locate a default browser profile." << std::endl;
        std::cerr << "Please ensure you have logged into Epic Games via Firefox or Zen Browser." << std::endl;
        return 1;
    }

    std::cout << "Found browser profile: " << profileFolder << std::endl;
    std::string cookieStr = cookie::getEpicGamesCookies(profileFolder);

    if (cookieStr.empty())
    {
        std::cerr << "No epicgames.com cookies found. Please log in first in your browser." << std::endl;
        return 1;
    }

    // Step 3: Get Authorization Code
    std::cout << "Requesting authorization code..." << std::endl;
    auto authCode = auth::getAuthorizationCode(cookieStr);
    if (!authCode)
    {
        std::cerr << "Failed to fetch authorization code. Are the cookies valid and unexpired?" << std::endl;
        return 1;
    }

    std::cout << "Authorization Code received." << std::endl;

    // Step 4: Token Exchange
    std::cout << "Exchanging code for token..." << std::endl;
    auto tokens = auth::exchangeToken(authCode.value());
    if (!tokens)
    {
        std::cerr << "Token exchange failed." << std::endl;
        return 1;
    }

    // Step 5: Write to Config
    std::cout << "Writing new token details to keyfile..." << std::endl;
    if (!config::writeTokenDetails(tokens.value()))
    {
        std::cerr << "Error writing to Keyfile." << std::endl;
        return 1;
    }

    // Step 6: Launch Epic Asset Manager
    std::cout << "Authentication success!" << std::endl;
    runEAM();

    return 0;
}
