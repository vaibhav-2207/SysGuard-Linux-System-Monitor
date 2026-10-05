#pragma once

#include <cstdint>
#include <string>

namespace SysGuard {

struct MemoryMetrics {
    uint64_t total_ram{0};
    uint64_t free_ram{0};
    uint64_t available_ram{0};
    uint64_t used_ram{0};
    uint64_t buffers{0};
    uint64_t cached{0};

    uint64_t total_swap{0};
    uint64_t free_swap{0};
    uint64_t used_swap{0};

    double ram_utilization_pct{0.0};
    double swap_utilization_pct{0.0};
};

class MemoryMonitor {
public:
    MemoryMonitor() = default;
    MemoryMetrics fetch();
    void printReport(const MemoryMetrics& metrics) const;

private:
    void parseMemInfo(MemoryMetrics& metrics) const;
};

} // namespace SysGuard
