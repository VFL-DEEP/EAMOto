#pragma once
#include <string>
#include <optional>

namespace eamoto {
namespace auth {

struct TokenDetails {
    std::string access_token;
    std::string refresh_token;
    std::string expires_at;
    std::string refresh_expires_at;
};

// Interacts with Epic Games API to get the authorization code
std::optional<std::string> getAuthorizationCode(const std::string& cookieString);

// Exchanges the authorization code for access and refresh tokens
std::optional<TokenDetails> exchangeToken(const std::string& authCode);

} // namespace auth
} // namespace eamoto
