# Stage 2: Detailed Requirements Specification

## 1. Functional Requirements Matrix

### FR-01: System Information
- **Interface:** POSIX `uname()`, `gethostname()`, `/etc/os-release`, `/proc/uptime`, `/proc/cpuinfo`.
- **Output:** Hostname, distribution release, Linux kernel version, hardware architecture, CPU model name, physical/logical CPU core count, system uptime in days/hours/minutes.

### FR-02: CPU Monitoring
- **Interface:** `/proc/stat`, `/proc/loadavg`, `/proc/cpuinfo`.
- **Metrics:** User, nice, system, idle, iowait, irq, softirq, steal time.
- **Computation:** Utilization calculated over time window:
  $$\text{Utilization \%} = \left(1.0 - \frac{\Delta \text{idle} + \Delta \text{iowait}}{\Delta \text{total}}\right) \times 100$$
- **Metrics:** Per-core percentage breakdown and 1-minute, 5-minute, and 15-minute load averages.

### FR-03: Memory Monitoring
- **Interface:** `/proc/meminfo`.
- **Metrics:** `MemTotal`, `MemFree`, `MemAvailable`, `Buffers`, `Cached`, `SwapTotal`, `SwapFree`.
- **Formulas:**
  $$\text{Used RAM} = \text{MemTotal} - \text{MemAvailable}$$
  $$\text{RAM \%} = \frac{\text{Used RAM}}{\text{MemTotal}} \times 100$$

### FR-04: Process Monitoring
- **Interface:** Directory iteration over `/proc/[pid]/stat`, `/proc/[pid]/status`.
- **Fields:** PID, parent PID (`PPID`), process name (`comm`), execution state (`R`, `S`, `D`, `Z`, `T`), thread count, virtual memory (`VmSize`), resident memory (`RSS`).
- **Resilience:** Gracefully catch disappearing processes during concurrent termination.

### FR-05: Disk Monitoring
- **Interface:** `/proc/mounts`, POSIX `statvfs()`.
- **Metrics:** Mount point, filesystem type, total disk space, used disk space, available disk space, usage percentage. Filters out virtual pseudofs (such as sysfs, proc, devtmpfs).

### FR-06: Network Monitoring
- **Interface:** `/proc/net/dev`, `/sys/class/net/<iface>/operstate`, `/sys/class/net/<iface>/address`.
- **Metrics:** Interface names, link state (up/down), MAC addresses, cumulative received bytes (RX), transmitted bytes (TX), packets, and real-time transfer rates.

### FR-07 through FR-11: Custom Kernel Module & Character Device
- **Module Name:** `sysguard.ko` (built with Kbuild).
- **Device Node:** `/dev/sysguard`.
- **File Operations:**
  - `open()`: Tracks open instances with synchronization lock.
  - `release()`: Handles device release.
  - `read()`: Returns human-readable diagnostic buffer.
  - `unlocked_ioctl()`: Handles ioctl commands.
- **IOCTL Commands:**
  - `SYSGUARD_IOCTL_GET_STATUS` (`_IOR`): Driver version, load timestamp, total read and IOCTL counter stats.
  - `SYSGUARD_IOCTL_GET_KERNEL_INFO` (`_IOR`): Kernel uptime, memory stats via `si_meminfo`, CPU core count, active task count.
  - `SYSGUARD_IOCTL_RESET_STATS` (`_IO`): Clears performance telemetry counters.
  - `SYSGUARD_IOCTL_ECHO_MSG` (`_IOWR`): Safe roundtrip string buffer exchange with `copy_from_user` / `copy_to_user`.

### FR-12: Interactive Terminal Dashboard
- Unified real-time dashboard and menu-driven command interface with ANSI terminal formatting and progress bars.

---

## 2. Non-Functional Requirements
- **NFR-01 Performance:** Lightweight C++ design with near-zero latency overhead.
- **NFR-02 Reliability:** Defensive error handling for missing device nodes, permission denial, invalid IOCTL codes, and dynamic PID changes.
- **NFR-03 Security:** Strict kernel validation of user buffers, magic numbers, and command bounds.
- **NFR-04 Portability:** Runs cleanly on Ubuntu 22.04 and 24.04 LTS (x86_64).
