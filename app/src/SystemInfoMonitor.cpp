#include "SystemInfoMonitor.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

namespace SysGuard {

SystemInfo SystemInfoMonitor::fetch() {
    SystemInfo info;

    // 1. Hostname
    char host_buffer[256];
    if (gethostname(host_buffer, sizeof(host_buffer)) == 0) {
        info.hostname = host_buffer;
    } else {
        info.hostname = "Unknown";
    }

    // 2. Kernel version & Architecture
    struct utsname uts;
    if (uname(&uts) == 0) {
        info.kernel_version = std::string(uts.sysname) + " " + uts.release;
        info.architecture = uts.machine;
    } else {
        info.kernel_version = "Unknown";
        info.architecture = "Unknown";
    }

    // 3. OS Name from /etc/os-release
    info.os_name = parseOSName();

    // 4. CPU Model & Cores from /proc/cpuinfo
    auto [model, cores] = parseCpuInfo();
    info.cpu_model = model;
    info.cpu_cores = cores;

    // 5. Uptime
    info.uptime_seconds = parseUptime();

    return info;
}

std::string SystemInfoMonitor::parseOSName() const {
    std::ifstream file("/etc/os-release");
    if (!file.is_open()) return "Linux";

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("PRETTY_NAME=", 0) == 0) {
            std::string val = line.substr(12);
            if (!val.empty() && (val.front() == '"' || val.front() == '\'')) val.erase(0, 1);
            if (!val.empty() && (val.back() == '"' || val.back() == '\'')) val.pop_back();
            return val;
        }
    }
    return "Linux";
}

std::pair<std::string, uint32_t> SystemInfoMonitor::parseCpuInfo() const {
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return {"Generic Processor", 1};

    std::string line;
    std::string model = "Unknown Processor";
    uint32_t count = 0;

    while (std::getline(file, line)) {
        if (line.rfind("model name", 0) == 0) {
            size_t colon = line.find(':');
            if (colon != std::string::npos && model == "Unknown Processor") {
                model = Utils::trim(line.substr(colon + 1));
            }
        } else if (line.rfind("processor", 0) == 0) {
            count++;
        }
    }

    if (count == 0) count = 1;
    return {model, count};
}

uint64_t SystemInfoMonitor::parseUptime() const {
    std::ifstream file("/proc/uptime");
    if (file.is_open()) {
        double uptime_sec = 0.0;
        if (file >> uptime_sec) {
            return static_cast<uint64_t>(uptime_sec);
        }
    }

    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        return static_cast<uint64_t>(si.uptime);
    }
    return 0;
}

void SystemInfoMonitor::printReport(const SystemInfo& info) const {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│                   SYSTEM INFORMATION                   │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
    std::cout << "  " << Color::BOLD << "Hostname:       " << Color::RESET << info.hostname << "\n";
    std::cout << "  " << Color::BOLD << "Operating Sys:  " << Color::RESET << info.os_name << "\n";
    std::cout << "  " << Color::BOLD << "Kernel Version: " << Color::RESET << info.kernel_version << "\n";
    std::cout << "  " << Color::BOLD << "Architecture:   " << Color::RESET << info.architecture << "\n";
    std::cout << "  " << Color::BOLD << "Processor:      " << Color::RESET << info.cpu_model << " (" << info.cpu_cores << " Cores)\n";
    std::cout << "  " << Color::BOLD << "System Uptime:  " << Color::RESET << Utils::formatDuration(info.uptime_seconds) << "\n";
    std::cout << "\n";
}

} // namespace SysGuard
