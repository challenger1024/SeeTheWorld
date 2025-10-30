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

/*
int main(){
    std::cout<<"按下空格拍照，Q键退出！"<<std::endl;
    SeeTheWorld stw;
//    stw.run();

    if(stw.capture()){
        stw.send_image();
    }
    return 0;
}
*/

int main(int argc, char* argv[]) {
    std::cout << "按下空格拍照，Q键退出！" << std::endl;

    // 检查是否传入参数 e
    if (argc > 1) {
        try {
            int e = std::stoi(argv[1]);  // 转换字符串为整数
            EXPOSURE = e;
            std::cout << "曝光度已覆盖为：" << EXPOSURE << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "参数错误：" << ex.what() << "，应传入整数曝光值。" << std::endl;
        }
    }

    SeeTheWorld stw;
    stw.run();
/*
    if (stw.capture()) {
        stw.send_image();
    }
*/
    return 0;
}