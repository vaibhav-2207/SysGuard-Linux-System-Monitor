#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== SysGuard Build Script ==="
echo "Project Directory: $PROJECT_ROOT"

cd "$PROJECT_ROOT"

# Check prerequisites
MISSING_TOOLS=()
for tool in g++ make; do
    if ! command -v "$tool" &>/dev/null; then
        MISSING_TOOLS+=("$tool")
    fi
done

KERNEL_BUILD="/lib/modules/$(uname -r)/build"
if [ ! -d "$KERNEL_BUILD" ]; then
    echo "Warning: Kernel build directory $KERNEL_BUILD not found."
    echo "To compile the kernel driver, install kernel headers: sudo apt install linux-headers-\$(uname -r)"
fi

if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo "Error: Missing required build tools: ${MISSING_TOOLS[*]}"
    echo "Please install them via: sudo apt update && sudo apt install -y build-essential"
    exit 1
fi

echo "Building C++ User Application & Tests..."
make app tests

echo ""
echo "Attempting to build Linux Kernel Module..."
if [ -d "$KERNEL_BUILD" ]; then
    make driver || echo "Notice: Kernel driver build failed or headers missing. App can still run without driver."
else
    echo "Skipping kernel driver compilation (kernel headers not linked)."
fi

echo ""
echo "=== Build Finished Successfully! ==="
echo "Application binary: $PROJECT_ROOT/bin/sysguard"
echo "Run interactive app: ./bin/sysguard"
