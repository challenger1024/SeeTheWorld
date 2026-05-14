#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <regex>
#include <array>
#include"SeeTheWorld.h"
#include"tts_ws_client.h"

namespace {
bool enterPressed() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int ready = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);
    if (ready <= 0 || !FD_ISSET(STDIN_FILENO, &readfds)) {
        return false;
    }

    char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) <= 0) {
        return false;
    }

    return ch == '\n' || ch == '\r';
}

std::string detectHdmiAudioDevice() {
    FILE* pipe = popen("aplay -l 2>/dev/null", "r");
    if (!pipe) {
        return "";
    }

    std::array<char, 512> buffer{};
    std::regex devicePattern(R"(card ([0-9]+):.*device ([0-9]+):)", std::regex::icase);
    std::smatch match;
    std::string fallbackDevice;

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        std::string line(buffer.data());
        if (!std::regex_search(line, match, devicePattern)) {
            continue;
        }

        std::string device = "plughw:" + match[1].str() + "," + match[2].str();
        if (fallbackDevice.empty()) {
            fallbackDevice = device;
        }

        if (line.find("HDMI") != std::string::npos || line.find("hdmi") != std::string::npos) {
            pclose(pipe);
            return device;
        }
    }

    pclose(pipe);
    return fallbackDevice;
}
}

void SeeTheWorld::speak(const std::string& text) {
    tts_speak(text);
    const char* skipAudio = std::getenv("STW_SKIP_AUDIO");
    if (skipAudio && skipAudio[0] != '\0' && std::string(skipAudio) != "0") {
        std::cout << "已生成 demo.pcm，跳过音频播放。" << std::endl;
        return;
    }

    std::string audioDevice;
    const char* configuredAudioDevice = std::getenv("STW_AUDIO_DEVICE");
    if (configuredAudioDevice && configuredAudioDevice[0] != '\0') {
        audioDevice = configuredAudioDevice;
    } else {
        audioDevice = detectHdmiAudioDevice();
        if (!audioDevice.empty()) {
            std::cout << "自动检测到音频设备: " << audioDevice << std::endl;
        } else {
            audioDevice = "default";
            std::cout << "未检测到 HDMI/ALSA 设备，使用 default。" << std::endl;
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程执行 aplay
        execlp("aplay", "aplay", "-D", audioDevice.c_str(), "-f", "S16_LE", "-r", "16000", "-c", "1", "demo.pcm", (char*)NULL);
        _exit(1);
    } else {
        std::cout << "播放中，设备 " << audioDevice << "，按回车停止..." << std::endl;
        bool stoppedByUser = false;
        int status = 0;

        while (true) {
            pid_t finished = waitpid(pid, &status, WNOHANG);
            if (finished == pid) {
                break;
            }

            if (finished == -1) {
                std::cerr << "等待播放进程失败。" << std::endl;
                break;
            }

            if (enterPressed()) {
                kill(pid, SIGTERM);
                waitpid(pid, &status, 0);
                stoppedByUser = true;
                break;
            }
        }

        if (stoppedByUser) {
            std::cout << "播放已中止。" << std::endl;
        } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            std::cerr << "播放失败，aplay 退出码: " << WEXITSTATUS(status) << std::endl;
        } else {
            std::cout << "播放完成。" << std::endl;
        }
    }
}
