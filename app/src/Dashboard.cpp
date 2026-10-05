#include "Dashboard.hpp"
#include "Utils.hpp"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <csignal>
#include <unistd.h>
#include <termios.h>

namespace SysGuard {

static volatile bool g_running = true;

static void sigHandler(int) {
    g_running = false;
}

Dashboard::Dashboard() = default;

void Dashboard::printBanner() const {
    std::cout << Color::CYAN << Color::BOLD
              << "╔════════════════════════════════════════════════════════════════════════════╗\n"
              << "║       SysGuard - Linux System Resource, Process & Device Monitor          ║\n"
              << "║         User-Space & Kernel Driver Monitoring Framework (v1.0.0)           ║\n"
              << "╚════════════════════════════════════════════════════════════════════════════╝\n"
              << Color::RESET;
}

void Dashboard::printMenu() const {
    std::cout << Color::BOLD << "Available Monitors & Actions:" << Color::RESET << "\n";
    std::cout << "  [1] System Information      (Hostname, OS, Kernel, CPU, Uptime)\n";
    std::cout << "  [2] CPU Utilization         (Overall, Per-Core, Load Averages)\n";
    std::cout << "  [3] Memory Utilization      (Physical RAM, Swap, Cache/Buffers)\n";
    std::cout << "  [4] Process Monitor         (Task list, PID, States, Memory, Threads)\n";
    std::cout << "  [5] Disk Storage            (Partitions, Filesystems, Usage)\n";
    std::cout << "  [6] Network Interfaces      (Interfaces, State, Throughput, MAC)\n";
    std::cout << "  [7] Kernel Driver Status    (/dev/sysguard IOCTL & Diagnostics)\n";
    std::cout << "  [8] Full Live Dashboard     (Auto-refreshing unified monitoring view)\n";
    std::cout << "  [9] Test Driver Echo IOCTL  (Bidirectional Kernel Transfer Verification)\n";
    std::cout << "  [0] Exit SysGuard\n";
    std::cout << "\n" << Color::BOLD << Color::GREEN << "Enter selection [0-9]: " << Color::RESET;
}

void Dashboard::showSystemInfo() {
    auto info = sys_monitor.fetch();
    sys_monitor.printReport(info);
}

void Dashboard::showCpu() {
    auto metrics = cpu_monitor.fetch();
    cpu_monitor.printReport(metrics);
}

void Dashboard::showMemory() {
    auto metrics = mem_monitor.fetch();
    mem_monitor.printReport(metrics);
}

void Dashboard::showProcesses() {
    auto metrics = proc_monitor.fetch(15, ProcessSort::MEMORY);
    proc_monitor.printReport(metrics);
}

void Dashboard::showDisk() {
    auto parts = disk_monitor.fetch();
    disk_monitor.printReport(parts);
}

void Dashboard::showNetwork() {
    auto ifaces = net_monitor.fetch();
    net_monitor.printReport(ifaces);
}

void Dashboard::showDriverStatus() {
    driver_client.printReport();
}

void Dashboard::testDriverEcho() {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│            DRIVER IOCTL BIDIRECTIONAL TEST             │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";

    if (driver_client.checkStatus() != DriverAccessStatus::AVAILABLE) {
        std::cout << "  " << Color::RED << "Error: " << driver_client.getLastError() << Color::RESET << "\n\n";
        return;
    }

    std::string test_msg = "Hello Linux Kernel from SysGuard App!";
    std::string reply;

    std::cout << "  Sending message to /dev/sysguard via SYSGUARD_IOCTL_ECHO_MSG...\n";
    std::cout << "  Input:  \"" << test_msg << "\"\n";

    if (driver_client.sendEcho(test_msg, reply)) {
        std::cout << "  " << Color::GREEN << "Success!" << Color::RESET << " Received response from kernel space:\n";
        std::cout << "  Output: \"" << Color::BOLD << reply << Color::RESET << "\"\n";
    } else {
        std::cout << "  " << Color::RED << "Failed: " << driver_client.getLastError() << Color::RESET << "\n";
    }
    std::cout << "\n";
}

void Dashboard::showUnifiedDashboard(bool clear_screen) {
    if (clear_screen) {
        std::cout << "\033[2J\033[1;1H"; // clear screen and move cursor to top-left
    }

    printBanner();

    // 1. Fetch system info & metrics
    auto sys = sys_monitor.fetch();
    auto cpu = cpu_monitor.fetch();
    auto mem = mem_monitor.fetch();
    auto procs = proc_monitor.fetch(8, ProcessSort::MEMORY);
    auto disks = disk_monitor.fetch();
    auto nets = net_monitor.fetch();

    // System summary banner
    std::cout << Color::BOLD << "Host: " << Color::RESET << sys.hostname
              << " | " << Color::BOLD << "OS: " << Color::RESET << sys.os_name
              << " | " << Color::BOLD << "Kernel: " << Color::RESET << sys.kernel_version
              << " | " << Color::BOLD << "Uptime: " << Color::RESET << Utils::formatDuration(sys.uptime_seconds)
              << "\n";
    std::cout << std::string(80, '=') << "\n\n";

    // CPU & Memory section side by side / compact
    std::cout << Color::BOLD << Color::YELLOW << "RESOURCE UTILIZATION" << Color::RESET << "\n";
    std::cout << "  CPU Usage:    " << Utils::renderProgressBar(cpu.total_utilization_pct, 25)
              << "   Load Avg: " << cpu.load_1m << ", " << cpu.load_5m << ", " << cpu.load_15m << "\n";
    std::cout << "  RAM Usage:    " << Utils::renderProgressBar(mem.ram_utilization_pct, 25)
              << "   Used: " << Utils::formatBytes(mem.used_ram) << " / " << Utils::formatBytes(mem.total_ram) << "\n";
    if (mem.total_swap > 0) {
        std::cout << "  Swap Usage:   " << Utils::renderProgressBar(mem.swap_utilization_pct, 25)
                  << "   Used: " << Utils::formatBytes(mem.used_swap) << " / " << Utils::formatBytes(mem.total_swap) << "\n";
    }
    std::cout << "\n";

    // Driver Status summary
    std::cout << Color::BOLD << Color::YELLOW << "KERNEL DRIVER (/dev/sysguard)" << Color::RESET << "\n";
    DriverAccessStatus drv_st = driver_client.checkStatus();
    if (drv_st == DriverAccessStatus::AVAILABLE) {
        struct sysguard_status dstatus;
        if (driver_client.getDriverStatus(dstatus)) {
            std::cout << "  Status: " << Color::GREEN << "LOADED & ACTIVE" << Color::RESET
                      << " | Driver: " << dstatus.driver_name << " v" << dstatus.version_major << "." << dstatus.version_minor << "." << dstatus.version_patch
                      << " | Read Ops: " << dstatus.total_read_ops
                      << " | IOCTL Ops: " << dstatus.total_ioctl_ops << "\n";
        } else {
            std::cout << "  Status: " << Color::YELLOW << "DEVICE ACCESSIBLE (IOCTL error)" << Color::RESET << "\n";
        }
    } else {
        std::cout << "  Status: " << Color::RED << "DRIVER NOT LOADED" << Color::RESET
                  << " (" << SYSGUARD_DEVICE_PATH << " unavailable - load via ./scripts/load_driver.sh)\n";
    }
    std::cout << "\n";

    // Top processes
    std::cout << Color::BOLD << Color::YELLOW << "TOP PROCESSES (By Memory)" << Color::RESET
              << " [Total Tasks: " << procs.total_count << ", Running: " << procs.running_count << "]\n";
    std::cout << Color::DIM << "  "
              << std::left
              << std::setw(8) << "PID"
              << std::setw(22) << "NAME"
              << std::setw(8) << "STATE"
              << std::setw(14) << "RSS MEMORY"
              << std::setw(14) << "VIRT MEMORY"
              << Color::RESET << "\n";

    for (const auto& p : procs.top_processes) {
        std::string dname = p.name.length() > 20 ? p.name.substr(0, 17) + "..." : p.name;
        std::cout << "  "
                  << std::left
                  << std::setw(8) << p.pid
                  << std::setw(22) << dname
                  << std::setw(8) << p.state
                  << std::setw(14) << Utils::formatBytes(p.rss_bytes)
                  << std::setw(14) << Utils::formatBytes(p.vmsize_bytes)
                  << "\n";
    }
    std::cout << "\n";

    // Storage summary
    std::cout << Color::BOLD << Color::YELLOW << "STORAGE VOLUMES" << Color::RESET << "\n";
    for (const auto& d : disks) {
        std::cout << "  " << std::left << std::setw(16) << d.mount_point
                  << Utils::renderProgressBar(d.usage_pct, 15) << "  "
                  << Utils::formatBytes(d.used_bytes) << " / " << Utils::formatBytes(d.total_bytes) << "\n";
    }
    std::cout << "\n";

    // Network summary
    std::cout << Color::BOLD << Color::YELLOW << "NETWORK TRAFFIC" << Color::RESET << "\n";
    for (const auto& n : nets) {
        const char* st_c = (n.operstate == "up") ? Color::GREEN : Color::RED;
        std::cout << "  " << std::left << std::setw(10) << n.name
                  << "[" << st_c << n.operstate << Color::RESET << "] "
                  << "RX: " << Utils::formatBytes(n.rx_bytes) << " | "
                  << "TX: " << Utils::formatBytes(n.tx_bytes) << "\n";
    }
    std::cout << "\n";
}

void Dashboard::runLiveMonitoring(int refresh_rate_sec) {
    g_running = true;
    signal(SIGINT, sigHandler);

    std::cout << "\033[?25l"; // Hide cursor

    while (g_running) {
        showUnifiedDashboard(true);
        std::cout << Color::DIM << "Live update every " << refresh_rate_sec
                  << "s. Press Ctrl+C to return to main menu...\n" << Color::RESET;

        for (int i = 0; i < refresh_rate_sec * 10; ++i) {
            if (!g_running) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "\033[?25h"; // Restore cursor
    signal(SIGINT, SIG_DFL);
    std::cout << "\nExited live view.\n";
}

void Dashboard::runInteractive() {
    printBanner();

    std::string line;
    while (true) {
        printMenu();
        if (!std::getline(std::cin, line)) break;

        line = Utils::trim(line);
        if (line.empty()) continue;

        char choice = line[0];
        std::cout << "\n";

        switch (choice) {
            case '1': showSystemInfo(); break;
            case '2': showCpu(); break;
            case '3': showMemory(); break;
            case '4': showProcesses(); break;
            case '5': showDisk(); break;
            case '6': showNetwork(); break;
            case '7': showDriverStatus(); break;
            case '8': runLiveMonitoring(2); break;
            case '9': testDriverEcho(); break;
            case '0':
            case 'q':
            case 'Q':
                std::cout << Color::GREEN << "Exiting SysGuard. Goodbye!\n" << Color::RESET;
                return;
            default:
                std::cout << Color::RED << "Invalid option. Please choose between 0 and 9.\n\n" << Color::RESET;
                break;
        }

        std::cout << Color::DIM << "Press Enter to continue..." << Color::RESET;
        std::string dummy;
        std::getline(std::cin, dummy);
        std::cout << "\n";
    }
}

} // namespace SysGuard
