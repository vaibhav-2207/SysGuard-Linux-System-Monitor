#include "Dashboard.hpp"
#include "Utils.hpp"

#include <iostream>
#include <string>

void printHelp(const char* prog) {
    std::cout << "Usage: " << prog << " [OPTIONS]\n\n"
              << "SysGuard - Linux System Resource, Process and Device Monitoring System\n\n"
              << "Options:\n"
              << "  -h, --help        Show this help message and exit\n"
              << "  -d, --dashboard   Display unified system dashboard once and exit\n"
              << "  -l, --live        Start live real-time dashboard\n"
              << "  --sysinfo         Display system architecture and uptime information\n"
              << "  --cpu             Display CPU utilization and load averages\n"
              << "  --mem             Display physical and swap memory usage\n"
              << "  --proc            Display active process table\n"
              << "  --disk            Display mounted filesystem storage usage\n"
              << "  --net             Display network interfaces and statistics\n"
              << "  --driver          Display /dev/sysguard kernel driver metrics\n"
              << "  --echo <msg>      Send test message to driver via IOCTL echo\n\n"
              << "Without options, SysGuard launches the interactive menu interface.\n";
}

int main(int argc, char* argv[]) {
    SysGuard::Dashboard dashboard;

    if (argc > 1) {
        std::string arg = argv[1];

        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-d" || arg == "--dashboard") {
            dashboard.showUnifiedDashboard(false);
            return 0;
        } else if (arg == "-l" || arg == "--live") {
            dashboard.runLiveMonitoring(2);
            return 0;
        } else if (arg == "--sysinfo") {
            dashboard.showSystemInfo();
            return 0;
        } else if (arg == "--cpu") {
            dashboard.showCpu();
            return 0;
        } else if (arg == "--mem") {
            dashboard.showMemory();
            return 0;
        } else if (arg == "--proc") {
            dashboard.showProcesses();
            return 0;
        } else if (arg == "--disk") {
            dashboard.showDisk();
            return 0;
        } else if (arg == "--net") {
            dashboard.showNetwork();
            return 0;
        } else if (arg == "--driver") {
            dashboard.showDriverStatus();
            return 0;
        } else if (arg == "--echo") {
            if (argc > 2) {
                SysGuard::DriverClient client;
                std::string reply;
                if (client.sendEcho(argv[2], reply)) {
                    std::cout << "Kernel reply: " << reply << "\n";
                    return 0;
                } else {
                    std::cerr << "Driver error: " << client.getLastError() << "\n";
                    return 1;
                }
            } else {
                dashboard.testDriverEcho();
                return 0;
            }
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printHelp(argv[0]);
            return 1;
        }
    }

    // Launch interactive mode
    dashboard.runInteractive();
    return 0;
}
