# SysGuard – Linux System Resource, Process & Device Monitoring System

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20Ubuntu-orange.svg)]()
[![Language](https://img.shields.io/badge/Language-C%20%7C%20C%2B%2B17-brightgreen.svg)]()

SysGuard is an individual capstone project demonstrating end-to-end Linux systems programming, operating system architecture, and kernel-user space communication. It pairs a **modern C++ user-space monitoring dashboard** with a **custom Linux character device driver** (`/dev/sysguard`).

---

## Architecture Overview

```
User Application (C++ Dashboard & Monitors)
                 │
                 │  open() / read() / ioctl() / close()
                 ▼
Custom Character Device Node (/dev/sysguard)
                 │
                 ▼
Linux Kernel Module (driver/sysguard.ko)
                 │
                 ▼
Linux Kernel Core Subsystems & Hardware Resources
```

---

## Key Features

- **System Information (FR-01):** Hostname, kernel release, OS name, CPU architecture, processor model, core count, and system uptime.
- **CPU Utilization (FR-02):** Utilization percentage computed from `/proc/stat`, per-core breakdown, and 1m/5m/15m load averages from `/proc/loadavg`.
- **Memory Utilization (FR-03):** Total, used, available, and free physical RAM and Swap usage parsed from `/proc/meminfo`.
- **Process Monitoring (FR-04):** Active process table listing PID, PPID, process name, state, thread count, RSS, and virtual memory. Built with defensive handling for rapidly terminating processes.
- **Disk Storage (FR-05):** Filesystem capacity and usage metrics acquired through POSIX `statvfs()` queries over `/proc/mounts`.
- **Network Monitoring (FR-06):** Interface status (up/down), MAC addresses, packet counts, RX/TX bytes, and bandwidth rates from `/proc/net/dev` and `/sys/class/net/`.
- **Custom Character Driver (FR-07 to FR-11):**
  - Character device node `/dev/sysguard`.
  - Implements `file_operations`: `open`, `release`, `read`, and `unlocked_ioctl`.
  - Safe memory transfer via `copy_to_user()` and `copy_from_user()`.
  - Private IOCTL protocol (`SYSGUARD_IOCTL_GET_STATUS`, `SYSGUARD_IOCTL_GET_KERNEL_INFO`, `SYSGUARD_IOCTL_RESET_STATS`, `SYSGUARD_IOCTL_ECHO_MSG`).
  - Strict input validation rejecting invalid commands with `-ENOTTY` (NFR-03).
- **Interactive Terminal Dashboard (FR-12):** Colored ANSI dashboard with real-time continuous refresh mode and menu navigation.

---

## Project Structure

```
SysGuard-Linux-System-Monitor/
├── app/
│   ├── include/                # C++ class headers
│   │   ├── CpuMonitor.hpp
│   │   ├── Dashboard.hpp
│   │   ├── DiskMonitor.hpp
│   │   ├── DriverClient.hpp
│   │   ├── MemoryMonitor.hpp
│   │   ├── NetworkMonitor.hpp
│   │   ├── ProcessMonitor.hpp
│   │   ├── SystemInfoMonitor.hpp
│   │   └── Utils.hpp
│   └── src/                    # C++ implementations & main
│       ├── CpuMonitor.cpp
│       ├── Dashboard.cpp
│       ├── DiskMonitor.cpp
│       ├── DriverClient.cpp
│       ├── MemoryMonitor.cpp
│       ├── NetworkMonitor.cpp
│       ├── ProcessMonitor.cpp
│       ├── SystemInfoMonitor.cpp
│       └── main.cpp
├── driver/
│   ├── sysguard.c              # Linux kernel module source
│   └── Makefile                # Kbuild driver makefile
├── include/
│   └── sysguard_ioctl.h        # Shared user-kernel IOCTL protocol
├── tests/
│   └── test_suite.cpp          # Automated unit & integration tests
├── scripts/
│   ├── build.sh                # Automated build script
│   ├── load_driver.sh          # Driver insertion & permissions script
│   ├── unload_driver.sh        # Driver removal script
│   └── run_tests.sh            # Test suite runner
├── docs/
│   ├── SRS.md                  # Complete Software Requirements Specification
│   ├── stage-1-introduction.md # Stage 1 Report
│   ├── stage-2-requirements.md # Stage 2 Report
│   ├── stage-3-architecture.md # Stage 3 Report
│   ├── stage-4-prototype.md    # Stage 4 Report
│   ├── stage-5-testing.md      # Stage 5 Report
│   └── stage-6-final.md        # Stage 6 Report
├── diagrams/
│   ├── architecture.mermaid    # Mermaid architecture diagram
│   └── sequence.mermaid        # Mermaid sequence diagram
├── Makefile                    # Root build orchestrator
├── README.md
├── .gitignore
└── LICENSE
```

---

## Prerequisites & Installation

On Ubuntu Linux (22.04 / 24.04 / 26.04 LTS):
```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) git
```

---

## Build Instructions

Build all targets (user application, kernel module, and test suite):
```bash
make all
```

Or build individual components:
```bash
make app      # Compiles bin/sysguard
make driver   # Compiles driver/sysguard.ko
make tests    # Compiles bin/sysguard_tests
make clean    # Removes build artifacts
```

---

## Driver Management

Insert the kernel module and configure device node permissions:
```bash
./scripts/load_driver.sh
```

Unload the kernel module:
```bash
./scripts/unload_driver.sh
```

---

## Running SysGuard

### 1. Interactive Menu Mode
```bash
./bin/sysguard
```

### 2. Live Monitoring Mode
```bash
./bin/sysguard --live
```

### 3. Command-Line Snapshot
```bash
./bin/sysguard --dashboard    # Full dashboard snapshot
./bin/sysguard --driver       # Kernel driver status & telemetry
./bin/sysguard --cpu          # CPU statistics
./bin/sysguard --mem          # Memory utilization
./bin/sysguard --proc         # Process table
./bin/sysguard --disk         # Filesystem metrics
./bin/sysguard --net          # Network interface status
```

---

## Verification & Testing

Execute the automated test suite:
```bash
./scripts/run_tests.sh
```

---

## Capstone Documentation Roadmap

Detailed stage reports matching capstone milestones are located in `docs/`:
- **[Stage 1: Introduction](docs/stage-1-introduction.md)**
- **[Stage 2: Requirements](docs/stage-2-requirements.md)**
- **[Stage 3: Architecture](docs/stage-3-architecture.md)**
- **[Stage 4: Prototype Construction](docs/stage-4-prototype.md)**
- **[Stage 5: Testing & Verification](docs/stage-5-testing.md)**
- **[Stage 6: Final Evaluation & Manual](docs/stage-6-final.md)**
