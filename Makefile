CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iapp/include -Iinclude -pthread

APP_SRCS = app/src/main.cpp \
           app/src/SystemInfoMonitor.cpp \
           app/src/CpuMonitor.cpp \
           app/src/MemoryMonitor.cpp \
           app/src/ProcessMonitor.cpp \
           app/src/DiskMonitor.cpp \
           app/src/NetworkMonitor.cpp \
           app/src/DriverClient.cpp \
           app/src/Dashboard.cpp

TEST_SRCS = tests/test_suite.cpp \
            app/src/SystemInfoMonitor.cpp \
            app/src/CpuMonitor.cpp \
            app/src/MemoryMonitor.cpp \
            app/src/ProcessMonitor.cpp \
            app/src/DiskMonitor.cpp \
            app/src/NetworkMonitor.cpp \
            app/src/DriverClient.cpp

BIN_DIR = bin
APP_BIN = $(BIN_DIR)/sysguard
TEST_BIN = $(BIN_DIR)/sysguard_tests

.PHONY: all app driver tests clean install help

all: app driver tests
	@echo "=========================================================="
	@echo "SysGuard build completed successfully!"
	@echo "  User App:   $(APP_BIN)"
	@echo "  Test Suite: $(TEST_BIN)"
	@echo "  Kernel Mod: driver/sysguard.ko"
	@echo "=========================================================="

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

app: $(BIN_DIR) $(APP_SRCS)
	@echo "Compiling SysGuard C++ User Application..."
	$(CXX) $(CXXFLAGS) $(APP_SRCS) -o $(APP_BIN)
	@echo "Built: $(APP_BIN)"

driver:
	@echo "Compiling SysGuard Linux Kernel Module..."
	$(MAKE) -C driver

tests: $(BIN_DIR) $(TEST_SRCS)
	@echo "Compiling SysGuard Test Suite..."
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_BIN)
	@echo "Built: $(TEST_BIN)"

clean:
	@echo "Cleaning application binaries..."
	rm -rf $(BIN_DIR)
	@echo "Cleaning kernel module..."
	$(MAKE) -C driver clean

help:
	@echo "SysGuard Build System Options:"
	@echo "  make all      - Compile user application, driver, and tests"
	@echo "  make app      - Compile only the C++ user monitoring application"
	@echo "  make driver   - Compile only the Linux kernel module (sysguard.ko)"
	@echo "  make tests    - Compile the automated test suite"
	@echo "  make clean    - Remove all compiled artifacts"
	@echo "  make help     - Display this message"
