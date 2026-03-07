#include "auth.hpp"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace eamoto {
namespace auth {

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::optional<std::string> getAuthorizationCode(const std::string& cookieString) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string readBuffer;
    std::string url = "https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    
    // Set cookie
    if (!cookieString.empty()) {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookieString.c_str());
    }

    // Some HTTP clients require user agent to get correct response
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    try {
        auto jsonResponse = nlohmann::json::parse(readBuffer);
        if (jsonResponse.contains("authorizationCode") && !jsonResponse["authorizationCode"].is_null()) {
            return jsonResponse["authorizationCode"].get<std::string>();
        }
    } catch (std::exception& e) {
        std::cerr << "JSON Parse error (Auth Code): " << e.what() << "\nResponse was: " << readBuffer << std::endl;
    }

    return std::nullopt;
}

std::optional<TokenDetails> exchangeToken(const std::string& authCode) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string readBuffer;
    std::string url = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token";
    std::string postData = "grant_type=authorization_code&code=" + authCode;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    // "launcherAppClient2" base64
    headers = curl_slist_append(headers, "Authorization: Basic MzRhMDJjZjhmNDQxNGUyOWIxNTkyMTg3NmRhMzZmOWE6ZGFhZmJjY2M3Mzc3NDUwMzlkZmZlNTNkOTRmYzc2Y2Y=");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed for token: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    try {
        auto jsonResponse = nlohmann::json::parse(readBuffer);
        if (jsonResponse.contains("access_token")) {
            TokenDetails details;
            details.access_token = jsonResponse["access_token"].get<std::string>();
            details.refresh_token = jsonResponse["refresh_token"].get<std::string>();
            details.expires_at = jsonResponse["expires_at"].get<std::string>();
            details.refresh_expires_at = jsonResponse["refresh_expires_at"].get<std::string>();
            return details;
        } else {
            std::cerr << "Token exchange error: " << readBuffer << "\n";
        }
    } catch (std::exception& e) {
        std::cerr << "JSON Parse error (Token Exchange): " << e.what() << "\nResponse was: " << readBuffer << std::endl;
    }

    return std::nullopt;
}

} // namespace auth
} // namespace eamoto
