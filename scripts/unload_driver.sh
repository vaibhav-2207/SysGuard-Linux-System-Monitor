#!/usr/bin/env bash
set -e

echo "=== Unloading SysGuard Linux Kernel Module ==="

if ! lsmod | grep -q "^sysguard "; then
    echo "Module 'sysguard' is not currently loaded."
    exit 0
fi

echo "Removing kernel module with sudo rmmod sysguard..."
sudo rmmod sysguard

if [ -e "/dev/sysguard" ]; then
    sudo rm -f /dev/sysguard
fi

echo "Kernel log output:"
dmesg | grep "sysguard" | tail -n 5

echo "=== Driver unloaded successfully ==="
