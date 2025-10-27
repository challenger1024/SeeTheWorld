#pragma once
#ifndef STW_SEE_THE_WORLD_H
#define STW_SEE_THE_WORLD_H

#include <string>
#include <atomic>
#include <thread>
#include <mutex>

// 前置声明（避免循环依赖）

// SeeTheWorld 类：负责整合拍照、语音提示和触发逻辑
class SeeTheWorld {
public:
    SeeTheWorld();
    ~SeeTheWorld();

    bool init();

    // 主循环（检测输入事件并响应）
    void run();

    // 拍照功能
    bool capture();
    void speak(const std::string& text); // 统一语音播报接口
    void send_image();
    // 语音反馈

    void processInput();   // 键盘或GPIO触发

    // 停止系统（安全退出）
    void stop();

private:
    // 状态标记
    std::atomic<bool> running;
    
    // 辅助线程（用于监听或后台任务）
    std::thread voiceThread;
    std::mutex mtx;

    bool running;
};

#endif // STW_SEE_THE_WORLD_H
