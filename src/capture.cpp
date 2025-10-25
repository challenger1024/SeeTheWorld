
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap(1); // /dev/video0
    if(!cap.isOpened()) {
        std::cerr << "无法打开摄像头！" << std::endl;
        return -1;
    }
    cv::Mat frame;
    cap >> frame;
    if(frame.empty()) {
        std::cerr << "拍照失败！" << std::endl;
        return -1;
    }
    cv::imwrite("image.jpg", frame);
    std::cout << "已保存图像 image.jpg" << std::endl;
    return 0;
}
