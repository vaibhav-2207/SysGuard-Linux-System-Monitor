# Software Requirements Specification (SRS)
## SysGuard – Linux System Resource, Process and Device Monitoring System

**Version:** 1.0  
**Date:** 05 October 2026  
**Project Type:** Individual Capstone Project  
**Technology:** C, C++, Linux, Linux Kernel Module, Bash, Make, Git  
**Target OS:** Ubuntu Linux 22.04 / 24.04 LTS (x86_64)  
**Architecture:** User Space + Kernel Space  

---

## 1. Introduction

### 1.1 Purpose
SysGuard is a Linux-based system monitoring and diagnostic application developed using C and C++. The system provides information about important Linux resources such as CPU, memory, processes, disk storage, network interfaces, system information, and kernel/device-driver status.

The project also includes a custom Linux kernel module that creates a character device, `/dev/sysguard`. The user-space C++ application communicates with this kernel module using standard Linux mechanisms such as `open()`, `read()`, `ioctl()`, and `close()`.

The main purpose is to demonstrate Linux system programming, C/C++ programming, operating system architecture, user-space and kernel-space communication, Linux kernel modules, character device drivers, CPU and memory management, process management, file-system and I/O concepts, hardware/software interaction, and Git-based development.

### 1.2 Project Scope
SysGuard provides a terminal-based interface through which a user can inspect the current state of a Linux system. It monitors system information, CPU utilization, memory utilization, running processes, disk storage, network interfaces, and kernel/driver status. The project is intended primarily as an educational Linux system-programming and architecture project rather than a replacement for enterprise monitoring platforms.

---

## 2. Problem Statement
Linux provides several separate interfaces for monitoring system resources, including `/proc`, `/sys`, system calls, and device interfaces. Understanding how these interfaces connect applications to the Linux kernel and underlying hardware can be difficult for learners. SysGuard addresses this by combining a practical monitoring application with a custom character-device driver.

```
User Application
       ↓
Linux System Calls
       ↓
Custom Character Device (/dev/sysguard)
       ↓
Linux Kernel Module (sysguard.ko)
       ↓
Linux Kernel
       ↓
Hardware / System Resources
```

---

## 3. Objectives
- Develop a Linux-based system monitoring application.
- Implement the user-space application using C++.
- Implement a custom Linux kernel module using C.
- Create a character device named `/dev/sysguard`.
- Demonstrate communication between user space and kernel space.
- Monitor CPU, memory, processes, disk and network resources.
- Use Linux interfaces such as `/proc` and `/sys`.
- Demonstrate hardware/software architecture concepts.
- Implement safe kernel-user data transfer.
- Maintain the project using Git and GitHub.
- Provide documentation, UML diagrams and testing evidence.

---

## 4. Intended Users
- **Primary users:** Linux users who want to inspect system resources through a terminal-based monitoring application.
- **Secondary users:** C/C++ students, Linux learners, Operating Systems students, system-programming learners, and developers learning Linux kernel modules.

---

## 5. Overall System Description

### 5.1 Product Perspective
- **User-Space Application:** Written in C++, responsible for the dashboard, resource monitoring, data processing, formatting, and communication with `/dev/sysguard`.
- **Kernel-Space Module:** Written in C, responsible for registering the character device, creating `/dev/sysguard`, handling controlled requests, implementing device operations, and safely transferring data.

### 5.2 System Architecture
```
+--------------------------------------------------+
| SysGuard C++ Application                         |
| System Info | CPU | RAM | Processes | Disk       |
| Network | Driver Status | Dashboard              |
+-------------------------+------------------------+
                          |
                   open/read/ioctl
                          |
                          ↓
+--------------------------------------------------+
| /dev/sysguard                                    |
| Character Device Interface                       |
+-------------------------+------------------------+
                          |
                          ↓
+--------------------------------------------------+
| SysGuard Linux Kernel Module                     |
| file_operations | open | read | ioctl | release  |
+-------------------------+------------------------+
                          |
                          ↓
+--------------------------------------------------+
| Linux Kernel                                     |
| Process Management | Memory | CPU | I/O | Net    |
+-------------------------+------------------------+
                          |
                          ↓
+--------------------------------------------------+
| Hardware                                         |
| CPU | RAM | Storage | Network Devices            |
+--------------------------------------------------+
```

---

## 6. Functional Requirements

| ID | Requirement | Description |
|---|---|---|
| **FR-01** | System Information | Display hostname, kernel version, operating system, architecture, CPU information and system uptime. |
| **FR-02** | CPU Monitoring | Read CPU statistics, calculate utilization, display load and CPU-related information. Linux interfaces include `/proc/stat`, `/proc/loadavg` and `/proc/cpuinfo`. |
| **FR-03** | Memory Monitoring | Display total, used, available and free memory and memory utilization percentage, primarily using `/proc/meminfo`. |
| **FR-04** | Process Monitoring | Display information about running processes, including PID, process name, state and available CPU/memory-related information. |
| **FR-05** | Disk Monitoring | Display total disk space, used space, available space and usage percentage using Linux filesystem APIs such as `statvfs()`. |
| **FR-06** | Network Monitoring | Display interface name, interface state, RX/TX statistics and available network interfaces using `/sys/class/net/` and/or `/proc/net/dev`. |
| **FR-07** | Kernel Module | Provide a Linux kernel module named `sysguard`, compiled as `sysguard.ko` and loadable with `insmod` and removable with `rmmod`. |
| **FR-08** | Character Device | Create the character device `/dev/sysguard` as the user-space/kernel-space interface. |
| **FR-09** | Device Operations | Implement appropriate `file_operations`, demonstrating `open()`, `read()`, `ioctl()` and `release()`. |
| **FR-10** | IOCTL Communication | Provide a limited set of device-specific commands through `ioctl()`, defined in a shared header and validated by the driver. |
| **FR-11** | Kernel/User Transfer | Use safe mechanisms such as `copy_to_user()` and `copy_from_user()` for transferring data. |
| **FR-12** | Dashboard | Provide a clear terminal-based dashboard presenting system, CPU, memory, process, disk, network and driver information. |

---

## 7. Non-Functional Requirements

- **NFR-01 Performance:** Minimal system overhead during metrics collection and rendering.
- **NFR-02 Reliability:** Gracefully handle missing devices, permission errors, invalid ioctl commands, and disappearing processes.
- **NFR-03 Security:** Safe memory transfer (`copy_to_user`, `copy_from_user`), validate inputs and IOCTL command bounds, reject unauthorized memory modifications, avoid arbitrary execution.
- **NFR-04 Portability:** Target Ubuntu Linux on x86_64 and build against running kernel headers.
- **NFR-05 Maintainability:** Modular C++ classes with separate source and header files.
- **NFR-06 Usability:** Readable terminal interface without requiring GUI/X11.
- **NFR-07 Documentation:** Complete documentation including README, SRS, architecture, UML diagrams, build instructions, and stage reports.

---

## 8. Hardware & Software Requirements

- **Hardware:** Dual-core x86_64 CPU or better, 4 GB RAM recommended for VM, 20 GB storage.
- **Software:** Ubuntu Linux 22.04 / 24.04 LTS, GCC, G++, GNU Make, Linux Kernel Headers, Git, Bash.
