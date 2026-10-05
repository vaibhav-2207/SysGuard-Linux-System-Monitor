# Stage 1: Project Introduction & Problem Definition

## 1. Project Background
Linux system monitoring is traditionally performed using disjointed commands (`top`, `htop`, `vmstat`, `iostat`, `dmesg`). While these tools are useful for end-users, system developers and OS students often struggle to understand the underlying layers of interaction between user applications, kernel virtual filesystems (`/proc`, `/sys`), POSIX system calls, character device drivers, and the kernel itself.

**SysGuard** is designed as an individual capstone project to bridge this conceptual gap. It provides a full-stack Linux systems solution:
1. A **user-space monitoring application** written in modern C++ that probes `/proc`, `/sys`, and POSIX APIs.
2. A **custom Linux kernel module** (`sysguard.ko`) that registers a character device `/dev/sysguard` and implements a private IOCTL telemetry interface.

## 2. Project Scope & Goals
- Provide comprehensive observability into core Linux subsystems:
  - Processor performance, utilization, and load averages.
  - Physical memory, virtual swap, buffers, and cache allocation.
  - Process lifecycle, task table parsing, thread tracking, and memory footprint.
  - Storage partitions, filesystem mount points, and block usage via `statvfs`.
  - Network interfaces, operational status, packet counters, and data throughput.
- Demonstrate modern Linux Kernel Device Driver development:
  - Dynamic major/minor device allocation via `alloc_chrdev_region`.
  - Character device registration (`cdev_init`, `cdev_add`).
  - Sysfs class and device creation (`class_create`, `device_create`) generating `/dev/sysguard`.
  - Implementation of kernel `file_operations`: `open`, `release`, `read`, and `unlocked_ioctl`.
  - Safe user-kernel data exchange using `copy_to_user` and `copy_from_user`.

## 3. Technology Stack
- **User-Space Programming:** C++17, STL, POSIX APIs (`sysinfo`, `statvfs`, `dirent`, `unistd`)
- **Kernel-Space Programming:** C99 / GNU C, Linux Kernel Module subsystem, Kbuild
- **Build System:** GNU Make, GCC, G++
- **Operating Environment:** Ubuntu Linux (x86_64 architecture)
- **Version Control:** Git & GitHub
