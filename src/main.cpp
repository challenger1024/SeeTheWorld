#include"SeeTheWorld.h"
#include <unistd.h>  // for usleep

void SeeTheWorld::run() {
    this->running = true;
    while (running) {
        this->processInput(); // 检测键盘
        usleep(100000); // 0.1 秒轮询
    }
}

int main(){
    SeeTheWorld stw;
    stw.run();
    return 0;
}