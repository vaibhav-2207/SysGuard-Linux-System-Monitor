# Stage 4: Implementation & Prototype Construction

## 1. Prototype Implementation Summary
The SysGuard prototype was implemented following modular software engineering principles in C++17 (for the application layer) and C99/Linux kernel standards (for the driver layer).

### 1.1 Kernel Module (`driver/sysguard.c`)
- **Initialization (`sysguard_init`):**
  - Allocates character device region with `alloc_chrdev_region`.
  - Initializes `cdev` structure with `cdev_init` binding to `sysguard_fops`.
  - Creates class `sysguard_class` (accounting for Linux kernel 6.4+ API differences) and creates device `/dev/sysguard`.
  - Records boot timestamp via `ktime_get_boottime_seconds()`.
- **Character Device Operations:**
  - `sysguard_open`: Tracks file descriptor opens.
  - `sysguard_release`: Handles device descriptor release.
  - `sysguard_read`: Generates diagnostic string from kernel state and copies to user buffer with `copy_to_user()`.
  - `sysguard_ioctl`: Evaluates command magic and bounds; safely executes `GET_STATUS`, `GET_KERNEL_INFO`, `RESET_STATS`, and `ECHO_MSG`.
- **Cleanup (`sysguard_exit`):**
  - Unregisters device node, destroys sysfs class, removes cdev, and frees major/minor regions.

### 1.2 User-Space Modules
- **`DriverClient`:** Checks if `/dev/sysguard` is registered, handles `EACCES` permissions, sends IOCTL commands, and validates error returns.
- **`SystemInfoMonitor`:** Combines `uname`, `gethostname`, and `/etc/os-release` parsing.
- **`CpuMonitor`:** Samples `/proc/stat` to calculate exact non-blocking CPU usage.
- **`MemoryMonitor`:** Processes `MemTotal`, `MemAvailable`, and `Swap` values from `/proc/meminfo`.
- **`ProcessMonitor`:** Scans `/proc`, extracts PID metadata from `/proc/[pid]/stat`, and gracefully tolerates short-lived terminating processes.
- **`DiskMonitor`:** Iterates through `/proc/mounts`, filters virtual filesystems, and invokes `statvfs()`.
- **`NetworkMonitor`:** Extracts bytes and packets from `/proc/net/dev` and operational states from `/sys/class/net/`.
- **`Dashboard`:** Renders interactive terminal menus and unified live monitoring screens with ANSI visual color bars.

### 1.3 Build & Automation Scripts
- Root `Makefile`: Unified build orchestrator (`make all`, `make app`, `make driver`, `make tests`).
- `driver/Makefile`: Kbuild kernel module Makefile.
- `scripts/build.sh`: Builds with prerequisite checks.
- `scripts/load_driver.sh`: Automated insertion with permission assignment.
- `scripts/unload_driver.sh`: Clean module unloading.
- `scripts/run_tests.sh`: Automated test runner.
