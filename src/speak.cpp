#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <cstdlib>
#include <string>
#include"SeeTheWorld.h"
#include"tts_ws_client.h"


void SeeTheWorld::speak(const std::string& text) {
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程执行 aplay
        execlp("aplay", "aplay", "-D", "hw:1,0", "-f", "S16_LE", "-r", "16000", "-c", "1", "demo.pcm", (char*)NULL);
        _exit(1);
    } else {
        std::cout << "播放中，按回车停止..." << std::endl;
        std::cin.get();  // 等待用户按下回车
        kill(pid, SIGTERM); // 杀掉 aplay 进程
        waitpid(pid, NULL, 0); // 等待回收
        std::cout << "播放已中止。" << std::endl;
    }
}
