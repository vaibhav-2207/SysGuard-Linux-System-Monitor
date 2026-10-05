#include "ProcessMonitor.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <iomanip>

namespace SysGuard {

bool ProcessMonitor::readProcessDetails(pid_t pid, ProcessInfo& proc) {
    proc.pid = pid;

    // Read /proc/[pid]/stat
    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat_file(stat_path);
    if (!stat_file.is_open()) {
        // Process disappeared or access denied
        return false;
    }

    std::string line;
    if (!std::getline(stat_file, line)) {
        return false;
    }

    size_t open_paren = line.find('(');
    size_t close_paren = line.rfind(')');
    if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren) {
        return false;
    }

    proc.name = line.substr(open_paren + 1, close_paren - open_paren - 1);

    std::string rest = line.substr(close_paren + 2);
    std::istringstream ss(rest);

    int pgrp, session, tty_nr, tpgid;
    unsigned int flags;
    uint64_t minflt, cminflt, majflt, cmajflt, utime, stime;
    int64_t cutime, cstime, priority, nice;
    int32_t num_threads;
    int64_t itrealvalue;
    uint64_t starttime, vsize;
    int64_t rss_pages;

    if (!(ss >> proc.state >> proc.ppid >> pgrp >> session >> tty_nr >> tpgid
             >> flags >> minflt >> cminflt >> majflt >> cmajflt
             >> utime >> stime >> cutime >> cstime >> priority >> nice
             >> num_threads >> itrealvalue >> starttime >> vsize >> rss_pages)) {
        return false;
    }

    proc.threads = (num_threads > 0) ? num_threads : 1;
    proc.cpu_ticks = utime + stime;
    proc.vmsize_bytes = vsize;

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;
    proc.rss_bytes = (rss_pages > 0) ? static_cast<uint64_t>(rss_pages * page_size) : 0;

    return true;
}

std::string ProcessMonitor::getStateName(char state) const {
    switch (state) {
        case 'R': return "Running";
        case 'S': return "Sleeping";
        case 'D': return "Disk Sleep";
        case 'Z': return "Zombie";
        case 'T': return "Stopped";
        case 't': return "Tracing";
        case 'X': return "Dead";
        case 'I': return "Idle";
        default:  return std::string(1, state);
    }
}

ProcessMetrics ProcessMonitor::fetch(size_t limit, ProcessSort sort_by) {
    ProcessMetrics metrics;
    std::vector<ProcessInfo> all_procs;

    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        return metrics;
    }

    struct dirent* entry = nullptr;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN) {
            continue;
        }

        // Check if directory name is numeric (PID)
        char* endptr = nullptr;
        long pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0' || pid <= 0) {
            continue;
        }

        ProcessInfo pinfo;
        if (readProcessDetails(static_cast<pid_t>(pid), pinfo)) {
            metrics.total_count++;
            if (pinfo.state == 'R') metrics.running_count++;
            else if (pinfo.state == 'S' || pinfo.state == 'I') metrics.sleeping_count++;
            else if (pinfo.state == 'Z') metrics.zombie_count++;

            all_procs.push_back(pinfo);
        }
    }
    closedir(proc_dir);

    // Sorting
    if (sort_by == ProcessSort::MEMORY) {
        std::sort(all_procs.begin(), all_procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.rss_bytes > b.rss_bytes;
        });
    } else if (sort_by == ProcessSort::PID) {
        std::sort(all_procs.begin(), all_procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.pid < b.pid;
        });
    } else if (sort_by == ProcessSort::NAME) {
        std::sort(all_procs.begin(), all_procs.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.name < b.name;
        });
    }

    if (all_procs.size() > limit) {
        metrics.top_processes.assign(all_procs.begin(), all_procs.begin() + limit);
    } else {
        metrics.top_processes = all_procs;
    }

    return metrics;
}

void ProcessMonitor::printReport(const ProcessMetrics& metrics) const {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│                    PROCESS MONITOR                     │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";
    std::cout << "  " << Color::BOLD << "Total Tasks: " << Color::RESET << metrics.total_count
              << "  |  " << Color::GREEN << "Running: " << metrics.running_count << Color::RESET
              << "  |  " << Color::BLUE << "Sleeping: " << metrics.sleeping_count << Color::RESET
              << "  |  " << Color::RED << "Zombie: " << metrics.zombie_count << Color::RESET << "\n\n";

    std::cout << Color::BOLD
              << std::left
              << std::setw(8)  << "PID"
              << std::setw(8)  << "PPID"
              << std::setw(22) << "NAME"
              << std::setw(12) << "STATE"
              << std::setw(8)  << "THRDS"
              << std::setw(12) << "RSS MEM"
              << std::setw(12) << "VM SIZE"
              << Color::RESET << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& proc : metrics.top_processes) {
        std::string display_name = proc.name;
        if (display_name.length() > 20) {
            display_name = display_name.substr(0, 17) + "...";
        }

        std::string state_str = getStateName(proc.state);
        const char* state_col = Color::WHITE;
        if (proc.state == 'R') state_col = Color::GREEN;
        else if (proc.state == 'Z') state_col = Color::RED;
        else if (proc.state == 'D') state_col = Color::YELLOW;

        std::cout << std::left
                  << std::setw(8)  << proc.pid
                  << std::setw(8)  << proc.ppid
                  << std::setw(22) << display_name
                  << state_col << std::setw(12) << state_str << Color::RESET
                  << std::setw(8)  << proc.threads
                  << std::setw(12) << Utils::formatBytes(proc.rss_bytes)
                  << std::setw(12) << Utils::formatBytes(proc.vmsize_bytes)
                  << "\n";
    }
    std::cout << "\n";
}

} // namespace SysGuard
