#include "DiskMonitor.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <sys/statvfs.h>

namespace SysGuard {

bool DiskMonitor::isPhysicalFilesystem(const std::string& fs_type, const std::string& mount_point) const {
    static const std::set<std::string> ignored_types = {
        "proc", "sysfs", "devpts", "cgroup", "cgroup2", "pstore",
        "bpf", "debugfs", "tracefs", "securityfs", "fusectl", "mqueue",
        "autofs", "binfmt_misc", "configfs", "ramfs", "devtmpfs", "overlay"
    };

    if (ignored_types.count(fs_type) > 0) return false;
    if (mount_point.rfind("/proc", 0) == 0 || mount_point.rfind("/sys", 0) == 0) return false;
    if (mount_point.rfind("/dev", 0) == 0 && mount_point != "/dev") return false;

    return true;
}

std::vector<DiskPartition> DiskMonitor::fetch() {
    std::vector<DiskPartition> partitions;
    std::ifstream mounts_file("/proc/mounts");
    if (!mounts_file.is_open()) return partitions;

    std::set<std::string> seen_mounts;
    std::string line;

    while (std::getline(mounts_file, line)) {
        std::istringstream ss(line);
        std::string dev, mount, type;
        if (!(ss >> dev >> mount >> type)) continue;

        if (!isPhysicalFilesystem(type, mount)) continue;
        if (seen_mounts.count(mount) > 0) continue;
        seen_mounts.insert(mount);

        struct statvfs vfs;
        if (statvfs(mount.c_str(), &vfs) == 0) {
            uint64_t block_size = (vfs.f_frsize > 0) ? vfs.f_frsize : vfs.f_bsize;
            uint64_t total = vfs.f_blocks * block_size;
            uint64_t free = vfs.f_bfree * block_size;
            uint64_t avail = vfs.f_bavail * block_size;
            uint64_t used = (total >= free) ? (total - free) : 0;

            if (total == 0) continue; // skip 0-size mounts

            DiskPartition part;
            part.device = dev;
            part.mount_point = mount;
            part.fs_type = type;
            part.total_bytes = total;
            part.used_bytes = used;
            part.available_bytes = avail;
            part.usage_pct = (static_cast<double>(used) / total) * 100.0;

            partitions.push_back(part);
        }
    }

    return partitions;
}

void DiskMonitor::printReport(const std::vector<DiskPartition>& partitions) const {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│                  DISK STORAGE USAGE                    │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";

    if (partitions.empty()) {
        std::cout << "  No physical disk partitions found.\n\n";
        return;
    }

    std::cout << Color::BOLD
              << std::left
              << std::setw(18) << "FILESYSTEM"
              << std::setw(10) << "TYPE"
              << std::setw(12) << "TOTAL"
              << std::setw(12) << "USED"
              << std::setw(12) << "AVAIL"
              << std::setw(18) << "CAPACITY"
              << std::setw(16) << "MOUNTED ON"
              << Color::RESET << "\n";
    std::cout << std::string(88, '-') << "\n";

    for (const auto& part : partitions) {
        std::cout << std::left
                  << std::setw(18) << (part.device.length() > 16 ? part.device.substr(0, 14) + ".." : part.device)
                  << std::setw(10) << part.fs_type
                  << std::setw(12) << Utils::formatBytes(part.total_bytes)
                  << std::setw(12) << Utils::formatBytes(part.used_bytes)
                  << std::setw(12) << Utils::formatBytes(part.available_bytes)
                  << Utils::renderProgressBar(part.usage_pct, 10) << "  "
                  << std::setw(16) << part.mount_point
                  << "\n";
    }
    std::cout << "\n";
}

} // namespace SysGuard
