#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== Running SysGuard Verification Test Suite ==="

cd "$PROJECT_ROOT"

if [ ! -f "bin/sysguard_tests" ]; then
    echo "Test binary not found. Compiling tests..."
    make tests
fi

./bin/sysguard_tests
