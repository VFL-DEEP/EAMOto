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

            std::string readBuffer;
            std::string url = "https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code";

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

            if (!cookieString.empty())
                curl_easy_setopt(curl, CURLOPT_COOKIE, cookieString.c_str());

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
            headers = curl_slist_append(headers, "Accept: application/json, text/plain, */*");
            headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
            headers = curl_slist_append(headers, "DNT: 1");
            headers = curl_slist_append(headers, "Connection: keep-alive");
            headers = curl_slist_append(headers, "Sec-Fetch-Dest: document");
            headers = curl_slist_append(headers, "Sec-Fetch-Mode: navigate");
            headers = curl_slist_append(headers, "Sec-Fetch-Site: none");
            headers = curl_slist_append(headers, "Cache-Control: max-age=0");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate, br");
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

            CURLcode res = curl_easy_perform(curl);
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                std::cerr << "❌ Network error: " << curl_easy_strerror(res) << std::endl;
                return std::nullopt;
            }
            if (httpCode != 200) {
                std::cerr << "❌ HTTP Error: " << httpCode << std::endl;
                return std::nullopt;
            }

            try {
                auto json = nlohmann::json::parse(readBuffer);
                if (json.contains("authorizationCode") && !json["authorizationCode"].is_null())
                    return json["authorizationCode"].get<std::string>();
                else {
                    std::cerr << "\n❌ Epic returned null auth code. Your session may have expired." << std::endl;
                    std::cerr << "→ Please log in to Epic Games in Firefox/Zen and try again." << std::endl;
                }
            } catch (const std::exception &e) {
                std::cerr << "❌ JSON parse error: " << e.what() << std::endl;
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

            char *encodedCode = curl_easy_escape(curl, authCode.c_str(), authCode.length());
            if (!encodedCode) {
                curl_easy_cleanup(curl);
                return std::nullopt;
            }
            std::string postData = "grant_type=authorization_code&code=" + std::string(encodedCode);
            curl_free(encodedCode);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            headers = curl_slist_append(headers, "Authorization: Basic MzRhMDJjZjhmNDQxNGUyOWIxNTkyMTg3NmRhMzZmOWE6ZGFhZmJjY2M3Mzc3NDUwMzlkZmZlNTNkOTRmYzc2Y2Y=");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                std::cerr << "❌ Network error (token exchange): " << curl_easy_strerror(res) << std::endl;
                return std::nullopt;
            }

            try {
                auto json = nlohmann::json::parse(readBuffer);
                if (json.contains("access_token")) {
                    TokenDetails d;
                    d.access_token      = json["access_token"].get<std::string>();
                    d.refresh_token     = json["refresh_token"].get<std::string>();
                    d.expires_at        = json["expires_at"].get<std::string>();
                    d.refresh_expires_at = json["refresh_expires_at"].get<std::string>();
                    return d;
                }
            } catch (const std::exception &e) {
                std::cerr << "❌ JSON parse error (token exchange): " << e.what() << std::endl;
            }

            return std::nullopt;
        }

    } // namespace auth
} // namespace eamoto
