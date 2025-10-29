// C++ version of the Python demo for Xunfei TTS WebSocket
// Requires: websocketpp, boost::asio, openssl, nlohmann_json
// Build with CMake (see provided CMakeLists.txt)

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include <nlohmann/json.hpp>
#include"tts_ws_client.h"

using json = nlohmann::json;
using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

static std::mutex file_mutex;

//
// Helper: RFC1123 formatted date (GMT)
//
std::string rfc1123_date_now() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    // gmtime_r/gmtime_s for thread safe: use gmtime_s on Windows
    std::tm gm;
#ifdef _WIN32
    gmtime_s(&gm, &t);
#else
    gmtime_r(&t, &gm);
#endif
    char buf[64];
    // Example: "Wed, 29 Oct 2025 12:34:56 GMT"
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gm);
    return std::string(buf);
}

//
// Helper: URL encode (simple)
//
std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (char c : value) {
        // Unreserved characters according to RFC3986
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << "%20";
        } else {
            escaped << '%' << std::uppercase << std::setw(2) << int((unsigned char)c) << std::nouppercase;
        }
    }
    return escaped.str();
}

//
// Helper: base64 encode using OpenSSL
//
std::string base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    // No newlines
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, buffer, (int)length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string b64text(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    return b64text;
}

std::string base64_encode(const std::string &in) {
    return base64_encode((const unsigned char*)in.data(), in.size());
}

//
// Helper: base64 decode (returns binary string)
//
std::string base64_decode(const std::string &in) {
    BIO *bio, *b64;
    int decodeLen = (int)in.length();
    std::string out;
    out.resize(decodeLen);
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(in.data(), (int)in.length());
    bio = BIO_push(b64, bio);
    int outLen = BIO_read(bio, &out[0], (int)in.length());
    if (outLen < 0) outLen = 0;
    out.resize(outLen);
    BIO_free_all(bio);
    return out;
}

//
// HMAC-SHA256 and base64 result
//
std::string hmac_sha256_base64(const std::string &key, const std::string &data) {
    unsigned char* digest;
    unsigned int digest_len = EVP_MAX_MD_SIZE;
    digest = (unsigned char*)OPENSSL_malloc(digest_len);
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key.data(), (int)key.size(), EVP_sha256(), NULL);
    HMAC_Update(ctx, (unsigned char*)data.data(), data.size());
    HMAC_Final(ctx, digest, &digest_len);
    HMAC_CTX_free(ctx);
    std::string b64 = base64_encode(digest, digest_len);
    OPENSSL_free(digest);
    return b64;
}

//
// Build the signed URL following the Python demo logic
//
std::string create_url(const std::string &APPID,
                       const std::string &APIKey,
                       const std::string &APISecret) {
    std::string base = "wss://tts-api.xfyun.cn/v2/tts";
    std::string date = rfc1123_date_now();

    // signature_origin = "host: " + "ws-api.xfyun.cn" + "\n" + "date: " + date + "\n" + "GET " + "/v2/tts " + "HTTP/1.1"
    std::string signature_origin = "host: ws-api.xfyun.cn\n";
    signature_origin += "date: " + date + "\n";
    signature_origin += "GET /v2/tts HTTP/1.1";

    // HMAC-SHA256 then base64
    std::string signature_sha = hmac_sha256_base64(APISecret, signature_origin);

    // authorization_origin = api_key="...", algorithm="hmac-sha256", headers="host date request-line", signature="..."
    std::ostringstream auth_ori;
    auth_ori << "api_key=\"" << APIKey << "\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"" << signature_sha << "\"";
    std::string authorization = base64_encode(auth_ori.str());

    // v = { "authorization": authorization, "date": date, "host": "ws-api.xfyun.cn" }
    std::ostringstream qs;
    qs << "authorization=" << url_encode(authorization)
       << "&date=" << url_encode(date)
       << "&host=" << url_encode(std::string("ws-api.xfyun.cn"));

    return base + "?" + qs.str();
}

//
// Global config & handlers
//
struct WsParam {
    std::string APPID;
    std::string APIKey;
    std::string APISecret;
    std::string Text;

    // business args default mirroring python demo
    std::string business_json() const {
        json b;
        b["aue"] = "raw";
        b["auf"] = "audio/L16;rate=16000";
        b["vcn"] = "x4_yezi";
        b["tte"] = "utf8";
        return b.dump();
    }

    std::string data_json() const {
        // Python demo does base64.b64encode(Text.encode('utf-8'))
        std::string text_b64 = base64_encode(Text);
        json d;
        d["status"] = 2;
        d["text"] = text_b64;
        return d.dump();
    }

    std::string common_json() const {
        json c;
        c["app_id"] = APPID;
        return c.dump();
    }
};

int tts_speak(const std::string &text) {
    // ----- 请在这里填入你的凭证 -----
    WsParam wsParam;

    // 从环境变量读取配置
    const char* appid = std::getenv("APPID");
    const char* apiKey = std::getenv("API_KEY");
    const char* apiSecret = std::getenv("API_SECRET");

    // 如果环境变量没设置，给出错误提示
    if (!appid || !apiKey || !apiSecret) {
        std::cerr << "❌ 请先设置环境变量 APPID, API_KEY, API_SECRET" << std::endl;
        return 1;
    }
    wsParam.APPID = appid;
    wsParam.APIKey = apiKey;
    wsParam.APISecret = apiSecret;
    wsParam.Text = text; // 若合成小语种，按Python注释使用UTF-16LE后 base64 编码

    // create url
    std::string uri = create_url(wsParam.APPID, wsParam.APIKey, wsParam.APISecret);

    std::cout << "Connecting to: " << uri << std::endl;

    client c;
    std::string out_filename = "demo.pcm";

    try {
        c.clear_access_channels(websocketpp::log::alevel::all);
        c.clear_error_channels(websocketpp::log::elevel::all);

        // init ASIO
        c.init_asio();

        // TLS init handler
        c.set_tls_init_handler([&](websocketpp::connection_hdl) -> websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> {
            namespace asio = websocketpp::lib::asio;
            websocketpp::lib::shared_ptr<asio::ssl::context> ctx(new asio::ssl::context(asio::ssl::context::sslv23));
            try {
                ctx->set_options(asio::ssl::context::default_workarounds |
                                 asio::ssl::context::no_sslv2 |
                                 asio::ssl::context::no_sslv3 |
                                 asio::ssl::context::single_dh_use);
            } catch (std::exception &e) {
                std::cerr << "Error in TLS init: " << e.what() << std::endl;
            }
            return ctx;
        });

        // set message handler
        c.set_message_handler([&](connection_hdl hdl, client::message_ptr msg) {
            try {
                std::string payload = msg->get_payload();
                // parse json
                auto j = json::parse(payload);
                if (j.contains("code")) {
                    int code = j["code"].get<int>();
                    std::string sid = j.value("sid", std::string(""));
                    if (j.contains("data") && j["data"].contains("audio")) {
                        std::string audio_b64 = j["data"]["audio"].get<std::string>();
                        std::string audio_bin = base64_decode(audio_b64);

                        {
                            std::lock_guard<std::mutex> lock(file_mutex);
                            std::ofstream ofs(out_filename, std::ios::binary | std::ios::app);
                            if (!ofs) {
                                std::cerr << "Failed to open output file\n";
                            } else {
                                ofs.write(audio_bin.data(), (std::streamsize)audio_bin.size());
                            }
                        }
                    }

                    int status = 0;
                    if (j.contains("data") && j["data"].contains("status")) status = j["data"]["status"].get<int>();

                    if (code != 0) {
                        std::string errMsg = j.value("message", std::string("unknown"));
                        std::cerr << "sid:" << sid << " call error:" << errMsg << " code is:" << code << std::endl;
                    } else {
                        std::cout << "Received chunk, status=" << status << std::endl;
                    }

                    if (status == 2) {
                        std::cout << "Server indicated finished (status==2). Closing..." << std::endl;
                        c.close(hdl, websocketpp::close::status::normal, "finish");
                    }
                } else {
                    std::cout << "Received message: " << payload << std::endl;
                }
            } catch (const std::exception &e) {
                std::cerr << "Exception in message handler: " << e.what() << std::endl;
            }
        });

        c.set_open_handler([&](connection_hdl hdl) {
            std::cout << "Connection opened, sending synth request..." << std::endl;
            // build payload json with common/business/data
            json d;
            d["common"] = json::parse(wsParam.common_json());
            d["business"] = json::parse(wsParam.business_json());
            d["data"] = json::parse(wsParam.data_json());

            std::string payload = d.dump();
            {
                std::lock_guard<std::mutex> lock(file_mutex);
                // remove existing file if present
                std::remove(out_filename.c_str());
            }
            websocketpp::lib::error_code ec;
            c.send(hdl, payload, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cerr << "Send failed: " << ec.message() << std::endl;
            } else {
                std::cout << "Sent synthesis request." << std::endl;
            }
        });

        c.set_fail_handler([&](connection_hdl) {
            std::cout << "Connection failed" << std::endl;
        });

        c.set_close_handler([&](connection_hdl) {
            std::cout << "Connection closed by server" << std::endl;
        });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cerr << "Get connection error: " << ec.message() << std::endl;
            return -1;
        }

        c.connect(con);
        // run (this will block until connection closes)
        c.run();

    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    std::cout << "Finished. Output saved to demo.pcm" << std::endl;
    return 0;
}