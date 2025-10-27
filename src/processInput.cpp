#include <iostream>
#include <conio.h>  // Windows可用；若在Linux下使用，请换为 termios 实现 kbhit/getch
#include <mutex>

void SeeTheWorld::processInput() {
    if (kbhit()) {
        char key = getch();

        if (key == ' ') { // 空格拍照
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "[键盘触发] 拍照中...\n";

            if (capture()) {
                std::cout << "[拍照完成]...\n";
                send_message();
//                speak("已拍照并获取描述");
            } else {
                speak("拍照失败");
            }
        }

        else if (key == 'q' || key == 'Q') { // 按 Q 键退出
            std::cout << "[系统提示] 检测到退出命令。\n";
            speak("程序即将退出");
            running = false;
        }

        else { // 错误输入
            std::cout << "[警告] 无效输入，请按空格拍照或 Q 退出。\n";
            speak("无效输入，请按空格拍照或Q退出");
        }
    }
}
