#include "CpuMonitor.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

namespace SysGuard {

CpuMonitor::CpuMonitor() {
    prev_snapshots = readCpuSnapshots();
}

std::vector<CpuTimeSnapshot> CpuMonitor::readCpuSnapshots() const {
    std::vector<CpuTimeSnapshot> snapshots;
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return snapshots;

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("cpu", 0) == 0) {
            std::istringstream ss(line);
            std::string cpu_label;
            CpuTimeSnapshot snap;
            ss >> cpu_label >> snap.user >> snap.nice >> snap.system
               >> snap.idle >> snap.iowait >> snap.irq >> snap.softirq >> snap.steal;
            snapshots.push_back(snap);
        }
    }
    return snapshots;
}

double CpuMonitor::calculateUsage(const CpuTimeSnapshot& prev, const CpuTimeSnapshot& curr) const {
    uint64_t prev_total = prev.getTotal();
    uint64_t curr_total = curr.getTotal();
    uint64_t prev_idle = prev.getIdle();
    uint64_t curr_idle = curr.getIdle();

    uint64_t delta_total = (curr_total > prev_total) ? (curr_total - prev_total) : 0;
    uint64_t delta_idle = (curr_idle > prev_idle) ? (curr_idle - prev_idle) : 0;

    if (delta_total == 0) return 0.0;
    double usage = (1.0 - (static_cast<double>(delta_idle) / delta_total)) * 100.0;
    if (usage < 0.0) usage = 0.0;
    if (usage > 100.0) usage = 100.0;
    return usage;
}

void CpuMonitor::readLoadAvg(CpuMetrics& metrics) const {
    std::ifstream file("/proc/loadavg");
    if (!file.is_open()) return;

    std::string proc_info;
    file >> metrics.load_1m >> metrics.load_5m >> metrics.load_15m >> proc_info;

    size_t slash = proc_info.find('/');
    if (slash != std::string::npos) {
        try {
            metrics.running_processes = std::stoul(proc_info.substr(0, slash));
            metrics.total_threads = std::stoul(proc_info.substr(slash + 1));
        } catch (...) {}
    }
}

CpuMetrics CpuMonitor::fetch() {
    CpuMetrics metrics;

    auto current_snapshots = readCpuSnapshots();

    // If prev_snapshots is empty or sizes don't match, sample briefly
    if (prev_snapshots.empty() || prev_snapshots.size() != current_snapshots.size()) {
        prev_snapshots = current_snapshots;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        current_snapshots = readCpuSnapshots();
    }

    if (!current_snapshots.empty() && !prev_snapshots.empty()) {
        metrics.total_utilization_pct = calculateUsage(prev_snapshots[0], current_snapshots[0]);

        for (size_t i = 1; i < current_snapshots.size() && i < prev_snapshots.size(); ++i) {
            metrics.per_core_utilization.push_back(
                calculateUsage(prev_snapshots[i], current_snapshots[i])
            );
        }
    }

    readLoadAvg(metrics);
    prev_snapshots = current_snapshots;
    return metrics;
}

void CpuMonitor::printReport(const CpuMetrics& metrics) const {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│                    CPU UTILIZATION                     │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
    std::cout << "  " << Color::BOLD << "Total CPU Usage: " << Color::RESET
              << Utils::renderProgressBar(metrics.total_utilization_pct, 28) << "\n";
    std::cout << "  " << Color::BOLD << "Load Averages:   " << Color::RESET
              << "1m: " << metrics.load_1m << "  |  "
              << "5m: " << metrics.load_5m << "  |  "
              << "15m: " << metrics.load_15m << "\n";
    std::cout << "  " << Color::BOLD << "Running Tasks:   " << Color::RESET
              << metrics.running_processes << " running / " << metrics.total_threads << " schedulable entities\n";

    if (!metrics.per_core_utilization.empty()) {
        std::cout << "\n  " << Color::BOLD << "Per-Core Breakdown:" << Color::RESET << "\n";
        for (size_t i = 0; i < metrics.per_core_utilization.size(); ++i) {
            std::cout << "    Core " << std::setw(2) << i << ": "
                      << Utils::renderProgressBar(metrics.per_core_utilization[i], 20) << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace SysGuard
