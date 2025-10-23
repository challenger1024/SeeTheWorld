#include <iostream>
#include <string>
#include <curl/curl.h>

// 用于接收返回的响应
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    std::string api_key = std::getenv("ARK_API_KEY"); // 从环境变量读取
    std::string api_url = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";

    // 构造请求体（JSON）
    std::string json_data = R"(
    {
        "model": "doubao-seed-1-6-lite-251015",
        "messages": [{"role": "user", "content": "hello"}]
    }
    )";

    CURL* curl = curl_easy_init();
    if(curl) {
        std::string response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response:\n" << response << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return 0;
}
