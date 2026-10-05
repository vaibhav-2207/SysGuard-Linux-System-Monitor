#pragma once

#include <string>
#include <cstdint>

namespace SysGuard {

struct SystemInfo {
    std::string hostname;
    std::string os_name;
    std::string kernel_version;
    std::string architecture;
    std::string cpu_model;
    uint32_t    cpu_cores{0};
    uint64_t    uptime_seconds{0};
    std::string boot_time;
};

class SystemInfoMonitor {
public:
    SystemInfoMonitor() = default;
    SystemInfo fetch();
    void printReport(const SystemInfo& info) const;

private:
    std::string parseOSName() const;
    std::pair<std::string, uint32_t> parseCpuInfo() const;
    uint64_t parseUptime() const;
};

} // namespace SysGuard
