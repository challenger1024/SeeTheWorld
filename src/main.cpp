#include"SeeTheWorld.h"
#include <unistd.h>  // for usleep
#include<iostream>


void SeeTheWorld::run() {
    this->running = true;
    while (running) {
        this->processInput(); // 检测键盘
        usleep(100000); // 0.1 秒轮询
//        std::cout<<"按下空格键拍照，按下q退出程序！"<<std::endl;
    }
}

int main(){
    std::cout<<"按下空格拍照，Q键退出！"<<std::endl;
    SeeTheWorld stw;
//    stw.run();

    if(stw.capture()){
        stw.send_image();
    }
    return 0;
}