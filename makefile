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

ifeq ($(ARCH),riscv)
TARGET := $(TARGET_NAME)-riscv64
ifeq ($(origin CXX),default)
CXX := riscv64-linux-gnu-g++
endif
PKG_CONFIG ?= $(shell if command -v riscv64-linux-gnu-pkg-config >/dev/null 2>&1; then echo riscv64-linux-gnu-pkg-config; else echo pkg-config; fi)
OBJ_DIR := $(BUILD_DIR)/riscv64

ifneq ($(SYSROOT),)
CXXFLAGS_SYSROOT := --sysroot=$(SYSROOT)
LDFLAGS_SYSROOT := --sysroot=$(SYSROOT)
PKG_CONFIG_SYSROOT_DIR := $(SYSROOT)
PKG_CONFIG_LIBDIR := $(SYSROOT)/usr/lib/riscv64-linux-gnu/pkgconfig:$(SYSROOT)/usr/lib/pkgconfig:$(SYSROOT)/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR
export PKG_CONFIG_LIBDIR
endif
else
TARGET := $(TARGET_NAME)
ifeq ($(origin CXX),default)
CXX := g++
endif
PKG_CONFIG ?= pkg-config
OBJ_DIR := $(BUILD_DIR)/native
endif

PKG_CONFIG_DEPS := opencv4

CXXFLAGS ?= -std=c++17 -Wall -Wextra
PKG_CONFIG_CFLAGS := $(shell $(PKG_CONFIG) --silence-errors --cflags $(PKG_CONFIG_DEPS))
PKG_CONFIG_LIBS := $(shell $(PKG_CONFIG) --silence-errors --libs $(PKG_CONFIG_DEPS))
CPPFLAGS += $(CXXFLAGS_SYSROOT) $(PKG_CONFIG_CFLAGS)
LDLIBS += $(PKG_CONFIG_LIBS) \
          -lcurl -lssl -lcrypto -lboost_system -lboost_thread -lpthread
LDFLAGS += $(LDFLAGS_SYSROOT)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all native riscv check-deps clean clean-native clean-riscv print-config

all: check-deps $(TARGET)

native:
	$(MAKE) ARCH=native

riscv:
	$(MAKE) ARCH=riscv SYSROOT="$(SYSROOT)"

check-deps:
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { \
		echo "Missing pkg-config command: $(PKG_CONFIG)"; \
		exit 1; \
	}
	@$(PKG_CONFIG) --exists $(PKG_CONFIG_DEPS) || { \
		echo "Missing pkg-config package: $(PKG_CONFIG_DEPS)"; \
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
	@echo "PKG_CONFIG=$(PKG_CONFIG)"
	@echo "SYSROOT=$(SYSROOT)"
	@echo "TARGET=$(TARGET)"
	@echo "OBJ_DIR=$(OBJ_DIR)"
	@echo "PKG_CONFIG_CFLAGS=$(PKG_CONFIG_CFLAGS)"
	@echo "PKG_CONFIG_LIBS=$(PKG_CONFIG_LIBS)"

clean: clean-native clean-riscv

clean-native:
	rm -rf $(BUILD_DIR)/native $(TARGET_NAME)

clean-riscv:
	rm -rf $(BUILD_DIR)/riscv64 $(TARGET_NAME)-riscv64

-include $(DEPS)
