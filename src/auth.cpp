#include "auth.hpp"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace eamoto
{
    namespace auth
    {

        size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            ((std::string *)userp)->append((char *)contents, size * nmemb);
            return size * nmemb;
        }

        std::optional<std::string> getAuthorizationCode(const std::string &cookieString)
        {
            CURL *curl = curl_easy_init();
            if (!curl)
                return std::nullopt;

            // Debug: Check if cookies are being passed
            if (cookieString.empty())
            {
                std::cerr << "Warning: No cookies provided to getAuthorizationCode()" << std::endl;
            }
            else
            {
                std::cerr << "DEBUG: Sending " << cookieString.length() << " bytes of cookies" << std::endl;
            }

            std::string readBuffer;
            std::string url = "https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code";

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow HTTP redirects
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);      // Limit redirects to prevent infinite loops

            // Set cookie
            if (!cookieString.empty())
            {
                curl_easy_setopt(curl, CURLOPT_COOKIE, cookieString.c_str());
            }

            // Add HTTP headers to bypass Cloudflare protection
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
            headers = curl_slist_append(headers, "Accept: application/json, text/plain, */*");
            headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
            headers = curl_slist_append(headers, "DNT: 1");
            headers = curl_slist_append(headers, "Connection: keep-alive");
            headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
            headers = curl_slist_append(headers, "Sec-Fetch-Dest: document");
            headers = curl_slist_append(headers, "Sec-Fetch-Mode: navigate");
            headers = curl_slist_append(headers, "Sec-Fetch-Site: none");
            headers = curl_slist_append(headers, "Cache-Control: max-age=0");

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br"); // Allow automatic decompression
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

            CURLcode res = curl_easy_perform(curl);

            // Get HTTP status code
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK)
            {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                return std::nullopt;
            }

            // Check HTTP status code
            if (httpCode != 200)
            {
                std::cerr << "HTTP Error: " << httpCode << std::endl;
                if (readBuffer.find("Just a moment") != std::string::npos)
                {
                    std::cerr << "Cloudflare bot detection triggered. Cookies may be expired or invalid." << std::endl;
                }
                std::cerr << "Response (first 500 chars): " << readBuffer.substr(0, 500) << std::endl;
                return std::nullopt;
            }

            std::cerr << "DEBUG: Full API Response (" << readBuffer.size() << " bytes): " << readBuffer << std::endl;

            try
            {
                auto jsonResponse = nlohmann::json::parse(readBuffer);
                if (jsonResponse.contains("authorizationCode") && !jsonResponse["authorizationCode"].is_null())
                {
                    return jsonResponse["authorizationCode"].get<std::string>();
                }
                else
                {
                    std::cerr << "\n❌ Authentication Failed!" << std::endl;
                    std::cerr << "The API returned null authorization codes." << std::endl;
                    std::cerr << "\nPossible causes:" << std::endl;
                    std::cerr << "  1. Your Epic Games session has expired" << std::endl;
                    std::cerr << "  2. Your cookies are invalid or outdated" << std::endl;
                    std::cerr << "  3. You're using Flatpak (isolated environment)" << std::endl;
                    std::cerr << "\n🔧 Solution:" << std::endl;
                    std::cerr << "  1. Open Firefox/Zen and go to: https://www.epicgames.com/account/personal" << std::endl;
                    std::cerr << "  2. Log in again with your Epic Games credentials" << std::endl;
                    std::cerr << "  3. Verify your account is logged in and cookies are saved" << std::endl;
                    std::cerr << "  4. Make sure you're not in Private/Incognito mode" << std::endl;
                    std::cerr << "  5. Try running 'eamoto' again" << std::endl
                              << std::endl;

                    return std::nullopt;
                }
            }
            catch (std::exception &e)
            {
                std::cerr << "JSON Parse error (Auth Code): " << e.what()
                          << "\nResponse length: " << readBuffer.size() << " chars"
                          << "\nResponse (first 500 chars): " << readBuffer.substr(0, 500) << std::endl;
            }

            return std::nullopt;
        }

        std::optional<TokenDetails> exchangeToken(const std::string &authCode)
        {
            CURL *curl = curl_easy_init();
            if (!curl)
                return std::nullopt;

            std::string readBuffer;
            std::string url = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token";

            // URL-encode the auth code to handle special characters (+, =, &, etc.)
            char *encodedCode = curl_easy_escape(curl, authCode.c_str(), authCode.length());
            if (!encodedCode)
            {
                std::cerr << "Failed to URL-encode auth code" << std::endl;
                curl_easy_cleanup(curl);
                return std::nullopt;
            }
            std::string postData = "grant_type=authorization_code&code=" + std::string(encodedCode);
            curl_free(encodedCode);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            // "launcherAppClient2" base64
            headers = curl_slist_append(headers, "Authorization: Basic MzRhMDJjZjhmNDQxNGUyOWIxNTkyMTg3NmRhMzZmOWE6ZGFhZmJjY2M3Mzc3NDUwMzlkZmZlNTNkOTRmYzc2Y2Y=");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow HTTP redirects
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);      // Limit redirects to prevent infinite loops

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK)
            {
                std::cerr << "curl_easy_perform() failed for token: " << curl_easy_strerror(res) << std::endl;
                return std::nullopt;
            }

            std::cerr << "[DEBUG] Token exchange response (" << readBuffer.size() << " bytes):\n" << readBuffer << std::endl;

            try
            {
                auto jsonResponse = nlohmann::json::parse(readBuffer);
                if (jsonResponse.contains("access_token"))
                {
                    TokenDetails details;
                    details.access_token = jsonResponse["access_token"].get<std::string>();
                    details.refresh_token = jsonResponse["refresh_token"].get<std::string>();
                    details.expires_at = jsonResponse["expires_at"].get<std::string>();
                    details.refresh_expires_at = jsonResponse["refresh_expires_at"].get<std::string>();
                    return details;
                }
                else
                {
                    std::cerr << "Token exchange error: response length=" << readBuffer.size() << " chars" << std::endl;
                }
            }
            catch (std::exception &e)
            {
                std::cerr << "JSON Parse error (Token Exchange): " << e.what()
                          << "\nResponse length: " << readBuffer.size() << " chars" << std::endl;
            }

            return std::nullopt;
        }

    } // namespace auth
} // namespace eamoto
