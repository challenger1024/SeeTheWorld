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
#include <vector>
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

std::vector<std::string> detectAudioDevices() {
    FILE* pipe = popen("aplay -l 2>/dev/null", "r");
    if (!pipe) {
        return {};
    }

    std::array<char, 512> buffer{};
    std::regex devicePattern(R"(card ([0-9]+):.*device ([0-9]+):)", std::regex::icase);
    std::smatch match;
    std::vector<std::string> devices;

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        std::string line(buffer.data());
        if (!std::regex_search(line, match, devicePattern)) {
            continue;
        }

        std::string device = "plughw:" + match[1].str() + "," + match[2].str();
        devices.push_back(device);
    }

    pclose(pipe);
    return devices;
}

int playPcmWithAplay(const std::string& audioDevice) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("aplay", "aplay", "-D", audioDevice.c_str(), "-f", "S16_LE", "-r", "16000", "-c", "1", "demo.pcm", (char*)NULL);
        _exit(127);
    }

    if (pid < 0) {
        std::cerr << "启动 aplay 失败。" << std::endl;
        return 1;
    }

    std::cout << "播放中，设备 " << audioDevice << "，按回车停止..." << std::endl;
    int status = 0;

    while (true) {
        pid_t finished = waitpid(pid, &status, WNOHANG);
        if (finished == pid) {
            break;
        }

        if (finished == -1) {
            std::cerr << "等待播放进程失败。" << std::endl;
            return 1;
        }

        if (enterPressed()) {
            kill(pid, SIGTERM);
            waitpid(pid, &status, 0);
            std::cout << "播放已中止。" << std::endl;
            return -1;
        }
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        std::cerr << "播放进程被信号中止: " << WTERMSIG(status) << std::endl;
        return 1;
    }

    return 1;
}
}

void SeeTheWorld::speak(const std::string& text) {
    tts_speak(text);
    const char* skipAudio = std::getenv("STW_SKIP_AUDIO");
    if (skipAudio && skipAudio[0] != '\0' && std::string(skipAudio) != "0") {
        std::cout << "已生成 demo.pcm，跳过音频播放。" << std::endl;
        return;
    }

    std::vector<std::string> audioDevices;
    const char* configuredAudioDevice = std::getenv("STW_AUDIO_DEVICE");
    if (configuredAudioDevice && configuredAudioDevice[0] != '\0') {
        audioDevices.push_back(configuredAudioDevice);
    } else {
        audioDevices = detectAudioDevices();
        if (!audioDevices.empty()) {
            std::cout << "自动检测到音频设备: ";
            for (size_t i = 0; i < audioDevices.size(); ++i) {
                if (i > 0) {
                    std::cout << ", ";
                }
                std::cout << audioDevices[i];
            }
            std::cout << std::endl;
        } else {
            audioDevices.push_back("default");
            std::cout << "未检测到 ALSA 播放设备，使用 default。" << std::endl;
        }
    }

    for (size_t i = 0; i < audioDevices.size(); ++i) {
        int result = playPcmWithAplay(audioDevices[i]);
        if (result == 0) {
            std::cout << "播放完成。" << std::endl;
            return;
        }

        if (result < 0) {
            return;
        }

        std::cerr << "播放失败，设备 " << audioDevices[i] << "，aplay 退出码: " << result << std::endl;
        if (i + 1 < audioDevices.size()) {
            std::cout << "尝试下一个音频设备..." << std::endl;
        }
    }

    std::cerr << "所有检测到的音频设备都播放失败，请使用 STW_AUDIO_DEVICE 手动指定可用设备。" << std::endl;
}
