# Stage 5: Testing & Verification Documentation

## 1. Test Strategy & Plan
SysGuard incorporates automated unit and integration tests implemented in `tests/test_suite.cpp`, along with interactive verification protocols.

| Test ID | Test Objective | Target Component | Expected Outcome |
|---|---|---|---|
| **TC-01** | Value Formatting | `Utils::formatBytes`, `formatDuration` | Correct string representations ("1.00 KB", "1h 1m 5s") |
| **TC-02** | System Info Parsing | `SystemInfoMonitor` | Hostname, OS name, and kernel version correctly resolved |
| **TC-03** | CPU Metric Bounds | `CpuMonitor` | CPU utilization percentage between $0.0\%$ and $100.0\%$ |
| **TC-04** | Memory Parsing | `MemoryMonitor` | RAM utilization percentage between $0.0\%$ and $100.0\%$ |
| **TC-05** | Process Discovery & Resilience | `ProcessMonitor` | At least 1 process detected; handles disappearing PIDs without crash |
| **TC-06** | Disk Filesystem Stats | `DiskMonitor` | Valid non-empty mount points and valid byte capacities |
| **TC-07** | Network Interface Detection | `NetworkMonitor` | Network interfaces detected (e.g. `lo`, `eth0`) |
| **TC-08** | Driver Detection / Handling | `DriverClient` | If driver missing: reports graceful warning without aborting |
| **TC-09** | Driver IOCTL Operations | `DriverClient` & Kernel Module | Successful `GET_STATUS`, `GET_KERNEL_INFO`, `ECHO_MSG` transfers |
| **TC-10** | Security: Invalid IOCTL Rejection | `DriverClient` & Kernel Module | Reject unauthorized IOCTLs with `-ENOTTY` |

## 2. Test Execution Instructions
Run the automated test suite via:
```bash
./scripts/run_tests.sh
# or
make tests && ./bin/sysguard_tests
```

## 3. Negative & Boundary Testing
1. **Unloaded Driver Handling:** When `/dev/sysguard` is missing, the application clearly notifies the user and suggests loading the driver via `./scripts/load_driver.sh` rather than crashing.
2. **Access Rights Boundary:** When `/dev/sysguard` has `0600` permissions and accessed by non-root user, `DriverAccessStatus::PERMISSION_DENIED` is triggered with clear advice.
3. **Invalid IOCTL Rejection:** Calling an unregistered ioctl command code returns `-ENOTTY` and errno 25, verifying kernel module boundaries.
4. **Process Eviction Race:** When processes terminate while `/proc` is being traversed, `readProcessDetails()` silently skips the entry without disruption.
