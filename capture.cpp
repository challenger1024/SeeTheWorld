
#include <opencv2/opencv.hpp>
#include <iostream>
#include"SeeTheWorld.h"

bool  SeeTheWorld::capture() {
    cv::VideoCapture cap(0); // /dev/video0
    setenv("OPENCV_LOG_LEVEL", "ERROR", 1);

    if(!cap.isOpened()) {
        std::cerr << "无法打开摄像头！" << std::endl;
        return false;
    }
    cv::Mat frame;
    cap >> frame;
    if(frame.empty()) {
        std::cerr << "拍照失败！" << std::endl;
        return false;
    }
    cv::imwrite("image.jpg", frame);
    std::cout << "已保存图像 image.jpg" << std::endl;
    return true;
}
