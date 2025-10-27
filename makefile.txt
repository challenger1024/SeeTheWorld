# 编译器
CXX = g++
# 编译选项
CXXFLAGS = -std=c++11 -I/usr/include/opencv4 `pkg-config --cflags opencv4`
# 链接选项
LDFLAGS = `pkg-config --libs opencv4` -L/usr/lib/riscv64-linux-gnu -lcurl
# 源文件路径
SRC_DIR = src
# 目标文件路径
OBJ_DIR = obj
# 可执行文件名
TARGET = see_the_world

# 自动收集所有 .cpp 源文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# 将 src/xxx.cpp 转换为 obj/xxx.o
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# 默认目标
all: $(TARGET)

# 链接阶段
$(TARGET): $(OBJS)
	@echo "Linking..."
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# 编译阶段
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling $< ..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建 obj 目录
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 清理
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# 伪目标
.PHONY: all clean
