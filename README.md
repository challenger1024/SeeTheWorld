# SeeTheWorld

## 简介
一个运行在Lichee Pi 4 A上的应用程序，实现以下功能：
- **采集图像信息：** 使用摄像头设备采集图像信息
- **AI识别：** 调用大模型API对图像进行识别
- **图像描述：** 使用大语言模型对图像进行描述
- **语音输出：** 使用扬声器或耳机设备将图像描述文字转成音频输出

## 安装依赖
执行命令
```bash
sudo apt update
sudo apt install -y \
  build-essential cmake pkg-config \
  libopencv-dev \
  libcurl4-openssl-dev libssl-dev \
  libboost-system-dev libboost-thread-dev
```
## 编译运行
执行命令：
```bash
git clone https://github.com/challenger1024/SeeTheWorld.git
cd SeeTheWorld
make
````
编译成功后会在当前目录下生成二进制可执行文件`see_the_world`  
运行前请保证已经配置好所需的环境变量  
使用`./see_the_world`运行它  
使用`make clean`清理编译生成的中间文件和最终可执行文件 ，让工程恢复到“干净状态。
## 环境变量配置
修改setENV.sh中的内容，替换为你的配置  
然后执行`./setENV.sh`来应用他们。
