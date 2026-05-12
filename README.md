# SeeTheWorld

## 简介
一个运行在Lichee Pi 4 A上的应用程序，实现以下功能：
- **采集图像信息：** 使用摄像头设备采集图像信息
- **AI识别：** 调用大模型API对图像进行识别
- **图像描述：** 使用大语言模型对图像进行描述
- **语音输出：** 使用扬声器或耳机设备将图像描述文字转成音频输出

## 架构说明
当前工程是一个 Linux 用户态 C++ 程序，不是裸机/板级程序，也没有直接使用 RISC-V 汇编、CSR、MMIO、设备树或特权级接口。

它主要依赖：
- OpenCV + V4L2 摄像头采集
- libcurl/OpenSSL 网络请求
- websocketpp、Boost、nlohmann-json 连接讯飞 TTS WebSocket
- ALSA `aplay` 播放音频
- POSIX 终端、进程和线程接口

因此可以在 x86 Linux 主机上开发和构建 native 版本，用于调试大部分用户态逻辑；部署到 Lichee Pi 4A 时再使用 RISC-V 交叉编译，或在开发板上原生编译。摄像头、声卡设备号、V4L2 参数和网络/TTS 凭证仍需要在真板上最终验证。

## 安装依赖
### x86 开发机依赖
执行命令：
```bash
sudo apt update
sudo apt install -y \
  build-essential pkg-config \
  libopencv-dev \
  libcurl4-openssl-dev libssl-dev \
  libboost-system-dev libboost-thread-dev \
  libwebsocketpp-dev nlohmann-json3-dev \
  alsa-utils
```

### RISC-V 交叉编译依赖
如果在 x86 主机上生成 RISC-V 可执行文件，需要安装交叉编译器和一套 RISC-V sysroot。具体包名与系统版本有关，常见起点如下：

```bash
sudo apt update
sudo apt install -y \
  g++-riscv64-linux-gnu \
  pkg-config
```

还需要确保 RISC-V sysroot 中存在 OpenCV、curl、OpenSSL、Boost、websocketpp、nlohmann-json 等开发库。最稳妥的方式是从开发板系统同步 sysroot，或在开发板上直接安装依赖后原生编译。

## 编译运行
### x86 native 构建
```bash
git clone https://github.com/challenger1024/SeeTheWorld.git
cd SeeTheWorld
make
```

编译成功后会在当前目录下生成二进制可执行文件`see_the_world`  
运行前请保证已经配置好所需的环境变量  
使用`./see_the_world`运行它  
使用`./see_the_world [integer]`，设置曝光度，目前是50，integer是一个整数
使用`make clean`清理编译生成的中间文件和最终可执行文件，让工程恢复到干净状态。

### RISC-V 构建
如果是在 RISC-V 开发板上原生编译，可以直接执行：

```bash
make
```

如果开发板系统自带的 `g++` 和 assembler/binutils 版本不匹配，推荐使用 PLCT 工具链：

```bash
make clean
make riscv
```

`make riscv` 默认使用：

```bash
~/tjc/venv-gnu-plct/bin/riscv64-plct-linux-gnu-g++
```

也可以手动覆盖：

```bash
make riscv CXX=~/tjc/venv-gnu-plct/bin/riscv64-plct-linux-gnu-g++
```

如果是在 x86 主机上交叉编译，并且系统里已经有可用的 `riscv64-linux-gnu-g++` 和 `riscv64-linux-gnu-pkg-config`：

```bash
make riscv
```

生成的目标文件是：

```bash
./see_the_world-riscv64
```

如果需要指定开发板 sysroot：

```bash
make riscv SYSROOT=/path/to/riscv/sysroot
```

部署示例：

```bash
scp see_the_world-riscv64 user@board:/home/user/see_the_world
ssh user@board /home/user/see_the_world
```

也可以查看当前构建配置：

```bash
make print-config
make print-config ARCH=riscv
```

### RISC-V 架构参数
Makefile 在 RISC-V 交叉构建，或在 RISC-V 开发板上原生构建时，默认使用较保守的架构参数：

```bash
-march=rv64gc -mabi=lp64d -misa-spec=2.2 -mno-riscv-attribute -mno-relax -Wa,-march=rv64imafdc
```

这是为了避免部分开发板系统上的 assembler/binutils 版本较旧，不能识别 GCC 14 生成的较新 RISC-V 扩展名、arch attribute 或 relaxation 相关汇编，例如 `zaamo`、`zalrsc`、`non-constant .uleb128`。其中 `-Wa,-march=rv64imafdc` 会给 assembler 单独指定旧格式架构字符串，`-mno-relax` 会禁用 RISC-V relaxation，避免旧 assembler 处理 `.uleb128` 失败。如果你的开发板工具链报类似错误：

```text
unknown prefixed ISA extension `zaamo'
non-constant .uleb128 is not supported
```

可以先清理后重新构建：

```bash
make clean
make
```

如果你的系统 ABI 或 CPU 架构不同，可以覆盖默认值：

```bash
make RISCV_MARCH=rv64imafdc RISCV_MABI=lp64d
```

如果仍然遇到 `zaamo`，可以显式指定 assembler 使用的架构字符串：

```bash
make clean
make RISCV_AS_MARCH=rv64imafdc
```

如果开发板上的 `g++` 太旧，不支持 `-misa-spec` 或 `-mno-riscv-attribute`，可以清空兼容参数再编译：

```bash
make clean
make RISCV_COMPAT_FLAGS=
```

## 环境变量配置
复制模板文件，填入你自己的 API Key：

```bash
cp .setENV.sh setENV.sh
vim setENV.sh
```

应用环境变量：

```bash
source ./setENV.sh
```

`setENV.sh` 是本地私密配置文件，已经加入 `.gitignore`，不要提交到 GitHub。提交前可以检查：

```bash
git status --short
git ls-files setENV.sh
```

如果 `git ls-files setENV.sh` 仍然输出 `setENV.sh`，说明它曾经被 Git 跟踪过，需要执行：

```bash
git rm --cached setENV.sh
```

这个命令只会让 Git 停止跟踪 `setENV.sh`，不会删除你的本地文件。

## 摄像头配置
程序默认会按顺序尝试打开 `/dev/video0` 到 `/dev/video5`，如果都失败，再尝试 OpenCV 默认摄像头后端。为了让 x86 开发机和 RISC-V 开发板都稳定使用指定摄像头，推荐显式设置环境变量：

```bash
export STW_CAMERA_DEVICE=/dev/video0
```

也可以只写编号：

```bash
export STW_CAMERA_DEVICE=0
```

查看系统上的摄像头设备：

```bash
ls -l /dev/video*
```

如果安装了 `v4l-utils`，可以查看摄像头名称、驱动和支持的格式：

```bash
sudo apt install -y v4l-utils
v4l2-ctl --list-devices
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

如果程序提示无法打开摄像头，请检查当前用户是否有 `video` 组权限：

```bash
groups
sudo usermod -aG video "$USER"
```

加入用户组后需要重新登录或重启终端会话。还要确认摄像头没有被浏览器、播放器、测试程序或其他进程占用。

## 开发模式：使用本地图片
在 WSL 或没有摄像头的开发环境中，可以跳过摄像头采集，直接使用本地图片作为 AI 描述输入。把图片放在工程目录下，例如 `test.jpg`，然后执行：

```bash
export STW_IMAGE_FILE=test.jpg
./see_the_world
```

设置 `STW_IMAGE_FILE` 后，程序按空格触发时不会打开摄像头，而是读取指定图片并保存为后续流程使用的 `image.jpg`，然后继续发送给 AI 进行描述。

恢复真实摄像头模式：

```bash
unset STW_IMAGE_FILE
```
