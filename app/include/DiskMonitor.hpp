#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace SysGuard {

struct DiskPartition {
    std::string device;
    std::string mount_point;
    std::string fs_type;
    uint64_t    total_bytes{0};
    uint64_t    used_bytes{0};
    uint64_t    available_bytes{0};
    double      usage_pct{0.0};
};

class DiskMonitor {
public:
    DiskMonitor() = default;
    std::vector<DiskPartition> fetch();
    void printReport(const std::vector<DiskPartition>& partitions) const;

private:
    bool isPhysicalFilesystem(const std::string& fs_type, const std::string& mount_point) const;
};

} // namespace SysGuard
