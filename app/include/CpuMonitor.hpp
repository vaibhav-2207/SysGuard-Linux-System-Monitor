#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace SysGuard {

struct CpuTimeSnapshot {
    uint64_t user{0};
    uint64_t nice{0};
    uint64_t system{0};
    uint64_t idle{0};
    uint64_t iowait{0};
    uint64_t irq{0};
    uint64_t softirq{0};
    uint64_t steal{0};

    uint64_t getTotal() const {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    uint64_t getIdle() const {
        return idle + iowait;
    }
};

struct CpuMetrics {
    double total_utilization_pct{0.0};
    std::vector<double> per_core_utilization;
    double load_1m{0.0};
    double load_5m{0.0};
    double load_15m{0.0};
    uint32_t running_processes{0};
    uint32_t total_threads{0};
};

class CpuMonitor {
public:
    CpuMonitor();
    CpuMetrics fetch();
    void printReport(const CpuMetrics& metrics) const;

private:
    std::vector<CpuTimeSnapshot> prev_snapshots;

    std::vector<CpuTimeSnapshot> readCpuSnapshots() const;
    void readLoadAvg(CpuMetrics& metrics) const;
    double calculateUsage(const CpuTimeSnapshot& prev, const CpuTimeSnapshot& curr) const;
};

} // namespace SysGuard
