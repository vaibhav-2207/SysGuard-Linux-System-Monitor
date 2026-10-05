#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <chrono>

namespace SysGuard {

struct NetworkInterface {
    std::string name;
    std::string operstate;
    std::string mac_address;
    uint64_t    rx_bytes{0};
    uint64_t    tx_bytes{0};
    uint64_t    rx_packets{0};
    uint64_t    tx_packets{0};
    uint64_t    rx_errors{0};
    uint64_t    tx_errors{0};

    double      rx_rate_kbps{0.0};
    double      tx_rate_kbps{0.0};
};

class NetworkMonitor {
public:
    NetworkMonitor() = default;
    std::vector<NetworkInterface> fetch();
    void printReport(const std::vector<NetworkInterface>& interfaces) const;

private:
    std::map<std::string, std::pair<uint64_t, uint64_t>> prev_bytes; // iface -> (rx, tx)
    std::chrono::steady_clock::time_point prev_time{std::chrono::steady_clock::now()};

    std::string readSysfsAttr(const std::string& iface, const std::string& attr) const;
};

} // namespace SysGuard
