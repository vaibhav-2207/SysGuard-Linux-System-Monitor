# Stage 3: System Architecture & Design

## 1. High-Level Architecture Overview
SysGuard adopts a dual-tier architecture spanning Linux user space and Linux kernel space:

```
+-----------------------------------------------------------------------------+
|                               USER SPACE                                    |
|                                                                             |
|  +-----------------------------------------------------------------------+  |
|  |                             Dashboard                                 |  |
|  |       (Interactive Terminal UI, Menus, Live Continuous Refresh)       |  |
|  +-----------------------------------------------------------------------+  |
|         |               |             |            |            |           |
|         v               v             v            v            v           |
|  +--------------+ +-----------+ +-----------+ +----------+ +-------------+  |
|  | SystemInfo   | | Cpu       | | Memory    | | Process  | | Disk / Net  |  |
|  | Monitor      | | Monitor   | | Monitor   | | Monitor  | | Monitors    |  |
|  +--------------+ +-----------+ +-----------+ +----------+ +-------------+  |
|         |               |             |            |            |           |
|         |               |             |            |            |           |
|         +---------------+-------------+------------+            |           |
|                         |                                       |           |
|                         v                                       |           |
|               Linux Virtual Filesystems                         |           |
|             /proc/stat, /proc/meminfo,                          |           |
|             /proc/[pid], /sys/class/net                         |           |
|                         |                                       |           |
|                         |                                       v           |
|                         |                                +---------------+  |
|                         |                                | DriverClient  |  |
|                         |                                +---------------+  |
+-------------------------|---------------------------------------|-----------+
                          |                                       |
                   Standard POSIX                           open() / read()
                     System Calls                           ioctl()
                          |                                       |
==========================v=======================================v============
                               KERNEL SPACE
                                                                  |
                                                                  v
                                                        +-------------------+
                                                        |   /dev/sysguard   |
                                                        | (Character Device)|
                                                        +-------------------+
                                                                  |
                                                                  v
                                                        +-------------------+
                                                        |  sysguard Driver  |
                                                        | (sysguard.c / ko) |
                                                        +-------------------+
                                                        | - file_operations |
                                                        | - copy_to_user()  |
                                                        | - copy_from_user()|
                                                        | - sysinfo / ktime |
                                                        +-------------------+
                                                                  |
                                                                  v
                                                        +-------------------+
                                                        |    Linux Kernel   |
                                                        | Core & Subsystems |
                                                        +-------------------+
```

## 2. Character Device Subsystem Details
The kernel module `sysguard.ko` implements a standard character device:

1. **Device Number Registration:** Dynamically obtains a major device number using `alloc_chrdev_region()`.
2. **Character Device Setup:** Associates the `file_operations` table (`open`, `release`, `read`, `unlocked_ioctl`) via `cdev_init()` and `cdev_add()`.
3. **Class & Device Node Creation:** Automatically creates the `/sys/class/sysguard_class` and `/dev/sysguard` device node using `class_create()` and `device_create()`.
4. **Synchronization:** Uses kernel `mutex` primitives (`DEFINE_MUTEX`) to guard internal operation counters and reference tracking against concurrent thread races.

## 3. Communication Protocol (IOCTL)
The IOCTL protocol between user space and kernel space is defined in `include/sysguard_ioctl.h`:
- `SYSGUARD_IOC_MAGIC`: Defined as `'g'`.
- Command 1: `SYSGUARD_IOCTL_GET_STATUS` (`_IOR`): Fills `struct sysguard_status`.
- Command 2: `SYSGUARD_IOCTL_GET_KERNEL_INFO` (`_IOR`): Fills `struct sysguard_kernel_info`.
- Command 3: `SYSGUARD_IOCTL_RESET_STATS` (`_IO`): Clears telemetry metrics.
- Command 4: `SYSGUARD_IOCTL_ECHO_MSG` (`_IOWR`): Exchanging `struct sysguard_custom_msg`.

## 4. User-Space Class Structure
- `SystemInfoMonitor`: Probes `/etc/os-release`, `uname()`, `/proc/cpuinfo`, and `/proc/uptime`.
- `CpuMonitor`: Keeps time snapshots of `/proc/stat` to calculate exact utilization deltas.
- `MemoryMonitor`: Parses `/proc/meminfo` and computes RAM/Swap utilization.
- `ProcessMonitor`: Scans `/proc`, safely extracts `/proc/[pid]/stat`, and gracefully tolerates short-lived processes.
- `DiskMonitor`: Discovers mounts from `/proc/mounts` and issues `statvfs()` queries.
- `NetworkMonitor`: Reads `/proc/net/dev` and sysfs attributes to compute interface data rates.
- `DriverClient`: Encapsulates POSIX file descriptor handling for `/dev/sysguard` and IOCTL wrappers.
- `Dashboard`: Coordinates rendering, user input handling, and terminal presentation.
