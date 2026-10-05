#include "NetworkMonitor.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace SysGuard {

std::string NetworkMonitor::readSysfsAttr(const std::string& iface, const std::string& attr) const {
    std::string path = "/sys/class/net/" + iface + "/" + attr;
    std::ifstream file(path);
    if (!file.is_open()) return "unknown";

    std::string val;
    file >> val;
    return val;
}

std::vector<NetworkInterface> NetworkMonitor::fetch() {
    std::vector<NetworkInterface> interfaces;
    std::ifstream netdev("/proc/net/dev");
    if (!netdev.is_open()) return interfaces;

    auto current_time = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(current_time - prev_time).count();
    if (elapsed_sec <= 0.0) elapsed_sec = 1.0;

    std::string line;
    // Skip two header lines
    std::getline(netdev, line);
    std::getline(netdev, line);

    while (std::getline(netdev, line)) {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string iface_name = Utils::trim(line.substr(0, colon));
        std::string stats_part = line.substr(colon + 1);
        std::istringstream ss(stats_part);

        NetworkInterface iface;
        iface.name = iface_name;

        uint64_t dummy;
        // Format: rx_bytes, rx_packets, rx_errs, rx_drop, fifo, frame, compressed, multicast,
        //         tx_bytes, tx_packets, tx_errs, tx_drop, fifo, colls, carrier, compressed
        if (!(ss >> iface.rx_bytes >> iface.rx_packets >> iface.rx_errors >> dummy
                 >> dummy >> dummy >> dummy >> dummy
                 >> iface.tx_bytes >> iface.tx_packets >> iface.tx_errors)) {
            continue;
        }

        // Read state and MAC from /sys/class/net
        iface.operstate = readSysfsAttr(iface_name, "operstate");
        iface.mac_address = readSysfsAttr(iface_name, "address");

        // Calculate rate if previous record exists
        if (prev_bytes.find(iface_name) != prev_bytes.end()) {
            auto [prev_rx, prev_tx] = prev_bytes[iface_name];
            uint64_t delta_rx = (iface.rx_bytes >= prev_rx) ? (iface.rx_bytes - prev_rx) : 0;
            uint64_t delta_tx = (iface.tx_bytes >= prev_tx) ? (iface.tx_bytes - prev_tx) : 0;

            iface.rx_rate_kbps = (delta_rx / 1024.0) / elapsed_sec;
            iface.tx_rate_kbps = (delta_tx / 1024.0) / elapsed_sec;
        }

        prev_bytes[iface_name] = {iface.rx_bytes, iface.tx_bytes};
        interfaces.push_back(iface);
    }

    prev_time = current_time;
    return interfaces;
}

void NetworkMonitor::printReport(const std::vector<NetworkInterface>& interfaces) const {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│                  NETWORK INTERFACES                    │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";

    if (interfaces.empty()) {
        std::cout << "  No network interfaces found.\n\n";
        return;
    }

    std::cout << Color::BOLD
              << std::left
              << std::setw(12) << "INTERFACE"
              << std::setw(10) << "STATE"
              << std::setw(18) << "MAC ADDRESS"
              << std::setw(14) << "RX DATA"
              << std::setw(14) << "TX DATA"
              << std::setw(12) << "RX PKTS"
              << std::setw(12) << "TX PKTS"
              << Color::RESET << "\n";
    std::cout << std::string(82, '-') << "\n";

    for (const auto& iface : interfaces) {
        const char* state_col = (iface.operstate == "up") ? Color::GREEN : Color::RED;

        std::cout << std::left
                  << std::setw(12) << iface.name
                  << state_col << std::setw(10) << iface.operstate << Color::RESET
                  << std::setw(18) << iface.mac_address
                  << std::setw(14) << Utils::formatBytes(iface.rx_bytes)
                  << std::setw(14) << Utils::formatBytes(iface.tx_bytes)
                  << std::setw(12) << iface.rx_packets
                  << std::setw(12) << iface.tx_packets
                  << "\n";
    }
    std::cout << "\n";
}

} // namespace SysGuard
