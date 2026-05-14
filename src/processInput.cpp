#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "SeeTheWorld.h"

using namespace std;

// 检测是否有键盘输入（Linux版 kbhit）
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);          // 获取终端属性
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);        // 关闭缓冲 & 回显
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);  // 获取文件描述符状态
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // 恢复终端设置
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// 获取一个按键（不会等待回车）
char getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// 键盘输入处理函数
void SeeTheWorld::processInput() {
    if (kbhit()) {
        char key = getch();
        if (key == ' ') { // 空格键拍照
            cout << "📸 拍照触发" << endl;
            if(this->capture()){
                this->send_image();
                std::cout<<"按下空格键拍照"<<std::endl;
                return;
            }
            
        } else if (key == 'q' || key == 'Q') {
            cout << "👋 程序退出" << endl;
            this->running = false;
        } else if (key == '\n' || key == '\r' || key == '\t') {
            return;
        } else {
            cout << "❌ 无效输入，请按空格拍照或Q退出。" << endl;
        }
    }
}
