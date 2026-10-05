#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
MODULE_PATH="$PROJECT_ROOT/driver/sysguard.ko"

echo "=== Loading SysGuard Linux Kernel Module ==="

# Check if already loaded
if lsmod | grep -q "^sysguard "; then
    echo "Module 'sysguard' is already loaded."
    exit 0
fi

# Check if driver binary exists
if [ ! -f "$MODULE_PATH" ]; then
    echo "Module binary not found at $MODULE_PATH. Building now..."
    make -C "$PROJECT_ROOT" driver
fi

if [ ! -f "$MODULE_PATH" ]; then
    echo "Error: Failed to find or build $MODULE_PATH"
    exit 1
fi

echo "Inserting kernel module with sudo insmod..."
sudo insmod "$MODULE_PATH"

# Wait briefly for udev to create device node
sleep 0.5

if [ -e "/dev/sysguard" ]; then
    echo "Device node /dev/sysguard created successfully."
    echo "Setting read/write permissions for user space testing..."
    sudo chmod 666 /dev/sysguard
    ls -l /dev/sysguard
else
    echo "Warning: /dev/sysguard was not automatically created by udev."
    echo "Checking dmesg for major number..."
    MAJOR=$(dmesg | grep "sysguard: device registered" | tail -n 1 | awk '{print $6}' | cut -d'=' -f2)
    if [ -n "$MAJOR" ]; then
        echo "Creating node manually: sudo mknod /dev/sysguard c $MAJOR 0"
        sudo mknod /dev/sysguard c "$MAJOR" 0
        sudo chmod 666 /dev/sysguard
    fi
fi

echo "Kernel log output:"
dmesg | grep "sysguard" | tail -n 5

echo "=== Driver loaded successfully ==="
