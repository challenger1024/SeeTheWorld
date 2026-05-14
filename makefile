# SeeTheWorld build file
#
# Default build:
#   make
#
# RISC-V cross build:
#   make riscv
#   make riscv SYSROOT=/path/to/riscv/sysroot

SRC_DIR := src
BUILD_DIR := build
TARGET_NAME := see_the_world

ARCH ?= native
HOST_MACHINE := $(shell uname -m)
RISCV_CXX ?= $(HOME)/tjc/venv-gnu-plct/bin/riscv64-plct-linux-gnu-g++
RISCV_MARCH ?= rv64gc
RISCV_MABI ?= lp64d
RISCV_ISA_SPEC ?= 2.2
RISCV_AS_MARCH ?= rv64imafdc
RISCV_SYSTEM_INCLUDE_DIRS ?= /usr/include /usr/include/riscv64-linux-gnu /usr/local/include /usr/include/mit-krb5 /usr/include/p11-kit-1
RISCV_SYSTEM_LIBRARY_DIRS ?= /usr/lib/riscv64-linux-gnu /lib/riscv64-linux-gnu
RISCV_LINK_COMPAT_FLAGS ?= -Wl,--allow-shlib-undefined
RISCV_COMPAT_FLAGS ?= -misa-spec=$(RISCV_ISA_SPEC) -mno-riscv-attribute -mno-relax -Wa,-march=$(RISCV_AS_MARCH)
RISCV_FLAGS := -march=$(RISCV_MARCH) -mabi=$(RISCV_MABI) $(RISCV_COMPAT_FLAGS)
RISCV_SYSTEM_CPPFLAGS := $(foreach dir,$(RISCV_SYSTEM_INCLUDE_DIRS),-isystem $(dir))
RISCV_SYSTEM_LDFLAGS := $(foreach dir,$(RISCV_SYSTEM_LIBRARY_DIRS),-L$(dir) -Wl,-rpath-link,$(dir))

ifeq ($(ARCH),riscv)
TARGET := $(TARGET_NAME)-riscv64
ifeq ($(origin CXX),default)
CXX := $(RISCV_CXX)
endif
ifeq ($(HOST_MACHINE),riscv64)
PKG_CONFIG ?= pkg-config
else
PKG_CONFIG ?= $(shell if command -v riscv64-linux-gnu-pkg-config >/dev/null 2>&1; then echo riscv64-linux-gnu-pkg-config; else echo pkg-config; fi)
endif
OBJ_DIR := $(BUILD_DIR)/riscv64
TARGET_ARCH_FLAGS := $(RISCV_FLAGS)

ifneq ($(SYSROOT),)
CXXFLAGS_SYSROOT := --sysroot=$(SYSROOT)
LDFLAGS_SYSROOT := --sysroot=$(SYSROOT)
PKG_CONFIG_SYSROOT_DIR := $(SYSROOT)
PKG_CONFIG_LIBDIR := $(SYSROOT)/usr/lib/riscv64-linux-gnu/pkgconfig:$(SYSROOT)/usr/lib/pkgconfig:$(SYSROOT)/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR
export PKG_CONFIG_LIBDIR
else ifeq ($(HOST_MACHINE),riscv64)
CXXFLAGS_SYSROOT := $(RISCV_SYSTEM_CPPFLAGS)
LDFLAGS_SYSROOT := $(RISCV_SYSTEM_LDFLAGS) $(RISCV_LINK_COMPAT_FLAGS)
endif
else
TARGET := $(TARGET_NAME)
ifeq ($(origin CXX),default)
CXX := g++
endif
PKG_CONFIG ?= pkg-config
OBJ_DIR := $(BUILD_DIR)/native
ifeq ($(HOST_MACHINE),riscv64)
TARGET_ARCH_FLAGS := $(RISCV_FLAGS)
endif
endif

PKG_CONFIG_DEPS := libcurl openssl
OPENCV_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags opencv4)
OPENCV_LIBS ?= -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core

CXXFLAGS ?= -std=c++17 -Wall -Wextra
PKG_CONFIG_CFLAGS := $(shell $(PKG_CONFIG) --silence-errors --cflags $(PKG_CONFIG_DEPS))
PKG_CONFIG_LIBS := $(shell $(PKG_CONFIG) --silence-errors --libs $(PKG_CONFIG_DEPS))
CPPFLAGS += $(CXXFLAGS_SYSROOT) $(OPENCV_CFLAGS) $(PKG_CONFIG_CFLAGS)
CXXFLAGS += $(TARGET_ARCH_FLAGS)
LDLIBS += $(OPENCV_LIBS) $(PKG_CONFIG_LIBS) -lboost_system -lboost_thread -lpthread
LDFLAGS += $(LDFLAGS_SYSROOT)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all native riscv check-deps clean clean-native clean-riscv print-config

all: check-deps $(TARGET)

native:
	$(MAKE) ARCH=native

riscv:
	$(MAKE) ARCH=riscv CXX="$(RISCV_CXX)" SYSROOT="$(SYSROOT)"

check-deps:
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { \
		echo "Missing pkg-config command: $(PKG_CONFIG)"; \
		exit 1; \
	}
	@$(PKG_CONFIG) --exists opencv4 $(PKG_CONFIG_DEPS) || { \
		echo "Missing pkg-config package: opencv4 $(PKG_CONFIG_DEPS)"; \
		echo "Install the development package, or set PKG_CONFIG_PATH/PKG_CONFIG_LIBDIR for the target sysroot."; \
		exit 1; \
	}

$(TARGET): $(OBJS)
	@echo "Linking $@ ..."
	$(CXX) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling $< ..."
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

print-config:
	@echo "ARCH=$(ARCH)"
	@echo "CXX=$(CXX)"
	@echo "RISCV_CXX=$(RISCV_CXX)"
	@echo "PKG_CONFIG=$(PKG_CONFIG)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "TARGET=$(TARGET)"
	@echo "OBJ_DIR=$(OBJ_DIR)"
	@echo "TARGET_ARCH_FLAGS=$(TARGET_ARCH_FLAGS)"
	@echo "RISCV_COMPAT_FLAGS=$(RISCV_COMPAT_FLAGS)"
	@echo "RISCV_AS_MARCH=$(RISCV_AS_MARCH)"
	@echo "RISCV_SYSTEM_INCLUDE_DIRS=$(RISCV_SYSTEM_INCLUDE_DIRS)"
	@echo "RISCV_SYSTEM_LIBRARY_DIRS=$(RISCV_SYSTEM_LIBRARY_DIRS)"
	@echo "RISCV_LINK_COMPAT_FLAGS=$(RISCV_LINK_COMPAT_FLAGS)"
	@echo "OPENCV_CFLAGS=$(OPENCV_CFLAGS)"
	@echo "OPENCV_LIBS=$(OPENCV_LIBS)"
	@echo "PKG_CONFIG_CFLAGS=$(PKG_CONFIG_CFLAGS)"
	@echo "PKG_CONFIG_LIBS=$(PKG_CONFIG_LIBS)"

clean: clean-native clean-riscv

clean-native:
	rm -rf $(BUILD_DIR)/native $(TARGET_NAME)

clean-riscv:
	rm -rf $(BUILD_DIR)/riscv64 $(TARGET_NAME)-riscv64

-include $(DEPS)
