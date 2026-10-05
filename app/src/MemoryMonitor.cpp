#include "MemoryMonitor.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace SysGuard {

void MemoryMonitor::parseMemInfo(MemoryMetrics& metrics) const {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key;
        uint64_t val_kb = 0;
        ss >> key >> val_kb;

        uint64_t val_bytes = val_kb * 1024;

        if (key == "MemTotal:") {
            metrics.total_ram = val_bytes;
        } else if (key == "MemFree:") {
            metrics.free_ram = val_bytes;
        } else if (key == "MemAvailable:") {
            metrics.available_ram = val_bytes;
        } else if (key == "Buffers:") {
            metrics.buffers = val_bytes;
        } else if (key == "Cached:") {
            metrics.cached = val_bytes;
        } else if (key == "SwapTotal:") {
            metrics.total_swap = val_bytes;
        } else if (key == "SwapFree:") {
            metrics.free_swap = val_bytes;
        }
    }

    if (metrics.total_ram > 0) {
        if (metrics.available_ram > 0) {
            metrics.used_ram = (metrics.total_ram > metrics.available_ram)
                ? (metrics.total_ram - metrics.available_ram) : 0;
        } else {
            metrics.used_ram = metrics.total_ram - metrics.free_ram - metrics.buffers - metrics.cached;
        }
        metrics.ram_utilization_pct = (static_cast<double>(metrics.used_ram) / metrics.total_ram) * 100.0;
    }

    if (metrics.total_swap > 0) {
        metrics.used_swap = (metrics.total_swap > metrics.free_swap)
            ? (metrics.total_swap - metrics.free_swap) : 0;
        metrics.swap_utilization_pct = (static_cast<double>(metrics.used_swap) / metrics.total_swap) * 100.0;
    }
}

MemoryMetrics MemoryMonitor::fetch() {
    MemoryMetrics metrics;
    parseMemInfo(metrics);
    return metrics;
}

void MemoryMonitor::printReport(const MemoryMetrics& metrics) const {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│                   MEMORY UTILIZATION                   │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
    std::cout << "  " << Color::BOLD << "Physical RAM:    " << Color::RESET
              << Utils::renderProgressBar(metrics.ram_utilization_pct, 26) << "\n";
    std::cout << "    Total:         " << Utils::formatBytes(metrics.total_ram) << "\n";
    std::cout << "    Used:          " << Utils::formatBytes(metrics.used_ram) << "\n";
    std::cout << "    Available:     " << Utils::formatBytes(metrics.available_ram) << "\n";
    std::cout << "    Free:          " << Utils::formatBytes(metrics.free_ram) << "\n";
    std::cout << "    Buffers/Cache: " << Utils::formatBytes(metrics.buffers + metrics.cached) << "\n";

    if (metrics.total_swap > 0) {
        std::cout << "\n  " << Color::BOLD << "Swap Space:      " << Color::RESET
                  << Utils::renderProgressBar(metrics.swap_utilization_pct, 26) << "\n";
        std::cout << "    Total Swap:    " << Utils::formatBytes(metrics.total_swap) << "\n";
        std::cout << "    Used Swap:     " << Utils::formatBytes(metrics.used_swap) << "\n";
        std::cout << "    Free Swap:     " << Utils::formatBytes(metrics.free_swap) << "\n";
    } else {
        std::cout << "\n  " << Color::BOLD << "Swap Space:      " << Color::RESET << "No Swap Configured\n";
    }
    std::cout << "\n";
}

} // namespace SysGuard
