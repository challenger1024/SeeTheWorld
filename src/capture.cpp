#include <opencv2/opencv.hpp>
#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include"SeeTheWorld.h"

namespace {
const char* kCapturedImagePath = "image.jpg";

bool isInteger(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    size_t start = value[0] == '-' ? 1 : 0;
    if (start == value.size()) {
        return false;
    }

    for (size_t i = start; i < value.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }

    return true;
}

bool openCameraSource(cv::VideoCapture& cap, const std::string& source, int backend) {
    if (isInteger(source)) {
        return cap.open(std::stoi(source), backend);
    }

    return cap.open(source, backend);
}

bool openConfiguredCamera(cv::VideoCapture& cap) {
    const char* configured = std::getenv("STW_CAMERA_DEVICE");
    if (configured && configured[0] != '\0') {
        std::cout << "尝试打开摄像头: " << configured << std::endl;
        if (openCameraSource(cap, configured, cv::CAP_V4L2)) {
            return true;
        }

        std::cerr << "无法打开 STW_CAMERA_DEVICE 指定的摄像头: " << configured << std::endl;
        return false;
    }

    for (int index = 0; index <= 5; ++index) {
        if (cap.open(index, cv::CAP_V4L2)) {
            std::cout << "已打开摄像头 /dev/video" << index << std::endl;
            return true;
        }
        cap.release();
    }

    if (cap.open(0, cv::CAP_ANY)) {
        std::cout << "已使用 OpenCV 默认后端打开摄像头 0" << std::endl;
        return true;
    }

    return false;
}

void printCameraTroubleshooting() {
    std::cerr << "无法打开摄像头！" << std::endl;
    std::cerr << "请检查：" << std::endl;
    std::cerr << "1. 摄像头设备是否存在：ls -l /dev/video*" << std::endl;
    std::cerr << "2. 当前用户是否有权限：groups，通常需要加入 video 组" << std::endl;
    std::cerr << "3. 是否有其他程序占用摄像头" << std::endl;
    std::cerr << "4. 可手动指定设备，例如：export STW_CAMERA_DEVICE=/dev/video0" << std::endl;
    std::cerr << "5. 如果只知道编号，也可以：export STW_CAMERA_DEVICE=0" << std::endl;
}

bool useImageFileForDevelopment() {
    const char* imageFile = std::getenv("STW_IMAGE_FILE");
    if (!imageFile || imageFile[0] == '\0') {
        return false;
    }

    std::filesystem::path source(imageFile);
    if (!std::filesystem::exists(source)) {
        std::cerr << "开发模式图片不存在: " << source << std::endl;
        return false;
    }

    cv::Mat image = cv::imread(source.string(), cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "无法读取开发模式图片: " << source << std::endl;
        return false;
    }

    if (!cv::imwrite(kCapturedImagePath, image)) {
        std::cerr << "无法保存开发模式图片到 " << kCapturedImagePath << std::endl;
        return false;
    }

    std::cout << "开发模式：已使用 " << source << " 作为输入图片" << std::endl;
    std::cout << "已保存图像 " << kCapturedImagePath << std::endl;
    return true;
}
}

bool SeeTheWorld::capture() {
    const char* imageFile = std::getenv("STW_IMAGE_FILE");
    if (imageFile && imageFile[0] != '\0') {
        return useImageFileForDevelopment();
    }

    cv::VideoCapture cap;

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
    if (!openConfiguredCamera(cap)) {
        printCameraTroubleshooting();
        return false;
    }

    // -------------------------
    // 摄像头参数调整区
    // -------------------------

    // 关闭自动曝光（注意：不同平台参数意义略有不同）
    // 在大多数 UVC 摄像头上：
    // 0.25 = 手动模式，0.75 = 自动模式
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);

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

    cv::imwrite(kCapturedImagePath, frame);
    std::cout << "已保存图像 " << kCapturedImagePath << std::endl;
//    this->send_image();
    return true;
}
