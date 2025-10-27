#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <curl/curl.h>
#include<cstring>
#include"SeeTheWorld.h"

// 在response中筛选出AI的回答
std::string extractAIResponse(const std::string& response) {
    // 查找 "content":" 的起始位置
    size_t start = response.find("\"content\":\"");
    if (start == std::string::npos) return "未找到 AI 回复内容。";

    // 跳过关键字部分
    start += strlen("\"content\":\"");

    // 查找对应的结尾引号
    size_t end = response.find("\",", start);
    if (end == std::string::npos) end = response.size();

    // 截取字符串
    std::string content = response.substr(start, end - start);

    // 处理转义符（如 \" -> "，\\n -> 换行）
    std::string clean;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\\' && i + 1 < content.size()) {
            char next = content[i + 1];
            if (next == 'n') { clean += '\n'; ++i; continue; }
            if (next == 't') { clean += '\t'; ++i; continue; }
            if (next == '"') { clean += '"'; ++i; continue; }
            if (next == '\\') { clean += '\\'; ++i; continue; }
        }
        clean += content[i];
    }
    return clean;
}

// 回调函数，用于接收 HTTP 响应
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 将图片读取并转换为 Base64
std::string readFileAsBase64(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open image file: " << path << std::endl;
        return "";
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string bytes = oss.str();

    static const char* base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    int val=0, valb=-6;
    for (unsigned char c : bytes) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            ret.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) ret.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (ret.size() % 4) ret.push_back('=');
    return ret;
}

void send_image() {
    // 从环境变量获取 API Key
    std::string api_key = std::getenv("ARK_API_KEY");
    if(api_key.empty()){
        std::cerr << "Please set ARK_API_KEY environment variable!" << std::endl;
        return 1;
    }

    // 模型接口 URL
    std::string api_url = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";

    // 读取图片并转 Base64
    std::string base64_image = readFileAsBase64("image.jpg");
    if(base64_image.empty()) return 1;

    // 构造 JSON 请求体
    std::string json_data = "{ \"model\": \"doubao-seed-1-6-lite-251015\", "
        "\"messages\": [ { \"role\": \"user\", \"content\": [ "
        "{ \"type\": \"image_url\", \"image_url\": { \"url\": \"data:image/jpeg;base64," + base64_image + "\" } }, "
        "{ \"type\": \"text\", \"text\": \"请描述图片中的内容\" } "
        "] } ] }";

    CURL* curl = curl_easy_init();
    if (curl) {
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
        if(res != CURLE_OK){
            std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;
        } else {
//            std::cout << "Response:\n" << response << std::endl;
            std::string answer = extractAIResponse(response);
            std::cout << "\nAI的描述:\n" << answer << std::endl;
            this->speak(answer);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

}
