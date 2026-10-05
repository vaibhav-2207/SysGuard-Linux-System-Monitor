#pragma once

#include "SystemInfoMonitor.hpp"
#include "CpuMonitor.hpp"
#include "MemoryMonitor.hpp"
#include "ProcessMonitor.hpp"
#include "DiskMonitor.hpp"
#include "NetworkMonitor.hpp"
#include "DriverClient.hpp"

namespace SysGuard {

class Dashboard {
public:
    Dashboard();

    void runInteractive();
    void showUnifiedDashboard(bool clear_screen = true);
    void runLiveMonitoring(int refresh_rate_sec = 2);

    void showSystemInfo();
    void showCpu();
    void showMemory();
    void showProcesses();
    void showDisk();
    void showNetwork();
    void showDriverStatus();
    void testDriverEcho();

private:
    SystemInfoMonitor sys_monitor;
    CpuMonitor        cpu_monitor;
    MemoryMonitor     mem_monitor;
    ProcessMonitor    proc_monitor;
    DiskMonitor       disk_monitor;
    NetworkMonitor    net_monitor;
    DriverClient      driver_client;

    void printBanner() const;
    void printMenu() const;
};

} // namespace SysGuard
