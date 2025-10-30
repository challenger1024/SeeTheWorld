#include <opencv2/opencv.hpp>
#include <iostream>
#include"SeeTheWorld.h"

int SeeTheWorld::capture() {
//    cv::VideoCapture cap(0, cv::CAP_ANY);  // 打开默认摄像头
cv::VideoCapture cap(0, cv::CAP_V4L2);

//    setenv("OPENCV_LOG_LEVEL", "ERROR", 1);
/*
    std::cout << "=== 当前摄像头参数 ===" << std::endl;
    std::cout << "亮度 (Brightness)        : " << cap.get(cv::CAP_PROP_BRIGHTNESS) << std::endl;
    std::cout << "对比度 (Contrast)       : " << cap.get(cv::CAP_PROP_CONTRAST) << std::endl;
    std::cout << "饱和度 (Saturation)     : " << cap.get(cv::CAP_PROP_SATURATION) << std::endl;
    std::cout << "色调 (Hue)             : " << cap.get(cv::CAP_PROP_HUE) << std::endl;
    std::cout << "增益 (Gain)             : " << cap.get(cv::CAP_PROP_GAIN) << std::endl;
    std::cout << "曝光 (Exposure)         : " << cap.get(cv::CAP_PROP_EXPOSURE) << std::endl;
    std::cout << "自动曝光 (Auto Exposure): " << cap.get(cv::CAP_PROP_AUTO_EXPOSURE) << std::endl;
    std::cout << "焦距 (Focus)            : " << cap.get(cv::CAP_PROP_FOCUS) << std::endl;
*/
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头！" << std::endl;
        return false;
    }

    // -------------------------
    // 摄像头参数调整区
    // -------------------------

    // 关闭自动曝光（注意：不同平台参数意义略有不同）
    // 在大多数 UVC 摄像头上：
    // 0.25 = 手动模式，0.75 = 自动模式
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 3);

    // 设置曝光值（负值代表短曝光，通常在 -5 到 -10 之间可用）
    cap.set(cv::CAP_PROP_EXPOSURE, EXPOSURE);

    // 适当降低亮度和增益（这些值需要根据摄像头情况调试）
    cap.set(cv::CAP_PROP_BRIGHTNESS, 50);
//    cap.set(cv::CAP_PROP_GAIN, 0.5);
//    cap.set(cv::CAP_PROP_CONTRAST, 40);
//    cap.set(cv::CAP_PROP_SATURATION, 50);//饱和度 50
//    cap.set(cv::CAP_PROP_HUE, 50);//色调 50

    // 等待相机稳定（有的摄像头需要几帧时间调整）
    cv::Mat frame;
    for (int i = 0; i < 5; i++) {
        cap >> frame;
    }

    if (frame.empty()) {
        std::cerr << "拍照失败！" << std::endl;
        return false;
    }
    std::cout<<"修改后"<<std::endl;
    std::cout << "=== 当前摄像头参数 ===" << std::endl;
    std::cout << "亮度 (Brightness)        : " << cap.get(cv::CAP_PROP_BRIGHTNESS) << std::endl;
    std::cout << "对比度 (Contrast)       : " << cap.get(cv::CAP_PROP_CONTRAST) << std::endl;
    std::cout << "饱和度 (Saturation)     : " << cap.get(cv::CAP_PROP_SATURATION) << std::endl;
    std::cout << "色调 (Hue)             : " << cap.get(cv::CAP_PROP_HUE) << std::endl;
    std::cout << "增益 (Gain)             : " << cap.get(cv::CAP_PROP_GAIN) << std::endl;
    std::cout << "曝光 (Exposure)         : " << cap.get(cv::CAP_PROP_EXPOSURE) << std::endl;
    std::cout << "自动曝光 (Auto Exposure): " << cap.get(cv::CAP_PROP_AUTO_EXPOSURE) << std::endl;
    std::cout << "焦距 (Focus)            : " << cap.get(cv::CAP_PROP_FOCUS) << std::endl;

    cv::imwrite("image.jpg", frame);
    std::cout << "已保存图像 image.jpg" << std::endl;
    return 0;
}
