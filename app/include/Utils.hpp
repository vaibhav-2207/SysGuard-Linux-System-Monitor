#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace SysGuard {

class Color {
public:
    static constexpr const char* RESET   = "\033[0m";
    static constexpr const char* BOLD    = "\033[1m";
    static constexpr const char* DIM     = "\033[2m";
    static constexpr const char* RED     = "\033[31m";
    static constexpr const char* GREEN   = "\033[32m";
    static constexpr const char* YELLOW  = "\033[33m";
    static constexpr const char* BLUE    = "\033[34m";
    static constexpr const char* MAGENTA = "\033[35m";
    static constexpr const char* CYAN    = "\033[36m";
    static constexpr const char* WHITE   = "\033[37m";

    static constexpr const char* BG_BLUE = "\033[44m";
    static constexpr const char* BG_DARK = "\033[40m";
};

class Utils {
public:
    static std::string formatBytes(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_idx = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unit_idx < 4) {
            size /= 1024.0;
            unit_idx++;
        }

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << size << " " << units[unit_idx];
        return ss.str();
    }

    static std::string formatDuration(uint64_t seconds) {
        uint64_t days = seconds / 86400;
        uint64_t hours = (seconds % 86400) / 3600;
        uint64_t mins = (seconds % 3600) / 60;
        uint64_t secs = seconds % 60;

        std::ostringstream ss;
        if (days > 0) ss << days << "d ";
        if (hours > 0 || days > 0) ss << hours << "h ";
        ss << mins << "m " << secs << "s";
        return ss.str();
    }

    static std::string renderProgressBar(double percentage, int width = 25) {
        if (percentage < 0.0) percentage = 0.0;
        if (percentage > 100.0) percentage = 100.0;

        int filled = static_cast<int>((percentage / 100.0) * width);
        std::ostringstream ss;

        const char* color = Color::GREEN;
        if (percentage >= 85.0) color = Color::RED;
        else if (percentage >= 65.0) color = Color::YELLOW;

        ss << color << "[";
        for (int i = 0; i < width; ++i) {
            if (i < filled) ss << "=";
            else if (i == filled) ss << ">";
            else ss << " ";
        }
        ss << "] " << std::fixed << std::setprecision(1) << percentage << "%" << Color::RESET;
        return ss.str();
    }

    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }
};

} // namespace SysGuard
