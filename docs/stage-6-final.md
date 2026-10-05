# Stage 6: Final Evaluation & Demonstration Manual

## 1. Project Evaluation & Acceptance Criteria

| Criteria | Status | Evidence |
|---|---|---|
| C++ application compiles cleanly on target Ubuntu system | ✅ Passed | Strict `-Wall -Wextra` flags with modern C++17 |
| Kernel module compiles against running kernel headers | ✅ Passed | Built using Linux Kbuild subsystem |
| `sysguard` module loads and unloads cleanly | ✅ Passed | Tested via `insmod` and `rmmod` via automated scripts |
| `/dev/sysguard` character device created dynamically | ✅ Passed | `device_create()` handles dynamic udev creation |
| C++ application communicates with character device | ✅ Passed | `open()`, `read()`, and `ioctl()` verified |
| CPU, memory, process, disk, and network stats displayed | ✅ Passed | Terminal dashboard monitors all subsystems |
| Invalid driver requests handled safely | ✅ Passed | Invalid IOCTL returns `-ENOTTY` safely |
| Comprehensive documentation, SRS, and diagrams provided | ✅ Passed | Complete documentation in `docs/` and `diagrams/` |

## 2. Trainer Evaluation & Demonstration Walkthrough

Follow these steps during evaluation:

### Step 1: Clone and Build
```bash
cd SysGuard-Linux-System-Monitor
make all
```

### Step 2: Load Kernel Module
```bash
./scripts/load_driver.sh
```
Verify the device node:
```bash
ls -l /dev/sysguard
```

### Step 3: Run Automated Test Suite
```bash
./scripts/run_tests.sh
```

### Step 4: Run SysGuard Monitoring Application
Interactive Menu Mode:
```bash
./bin/sysguard
```
- Select `1` for System Information.
- Select `2` for CPU Utilization.
- Select `3` for Memory Utilization.
- Select `4` for Process Monitor.
- Select `5` for Disk Storage.
- Select `6` for Network Interfaces.
- Select `7` for Kernel Driver Status.
- Select `8` for Live Continuous Dashboard.
- Select `9` to test Kernel Echo IOCTL.
- Select `0` to exit.

Command-Line Instant Flags:
```bash
./bin/sysguard --dashboard    # Render unified dashboard
./bin/sysguard --driver       # Query /dev/sysguard
./bin/sysguard --echo "Test"  # IOCTL kernel echo
```

### Step 5: Unload Kernel Module
```bash
./scripts/unload_driver.sh
```
Verify clean module removal via kernel logs:
```bash
dmesg | grep sysguard | tail -n 5
```
