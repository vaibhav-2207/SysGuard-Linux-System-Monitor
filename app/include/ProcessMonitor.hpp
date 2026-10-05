#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <sys/types.h>

namespace SysGuard {

enum class ProcessSort {
    MEMORY,
    PID,
    NAME
};

struct ProcessInfo {
    pid_t       pid{0};
    pid_t       ppid{0};
    std::string name;
    char        state{'?'};
    uint32_t    threads{1};
    uint64_t    vmsize_bytes{0};
    uint64_t    rss_bytes{0};
    uint64_t    cpu_ticks{0};
};

struct ProcessMetrics {
    std::vector<ProcessInfo> top_processes;
    uint32_t total_count{0};
    uint32_t running_count{0};
    uint32_t sleeping_count{0};
    uint32_t zombie_count{0};
};

class ProcessMonitor {
public:
    ProcessMonitor() = default;
    ProcessMetrics fetch(size_t limit = 15, ProcessSort sort_by = ProcessSort::MEMORY);
    void printReport(const ProcessMetrics& metrics) const;

private:
    bool readProcessDetails(pid_t pid, ProcessInfo& proc);
    std::string getStateName(char state) const;
};

} // namespace SysGuard
