#include "DriverClient.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace SysGuard {

DriverClient::DriverClient(const std::string& dev_path)
    : device_path(dev_path), fd(-1) {}

DriverClient::~DriverClient() {
    closeDevice();
}

DriverAccessStatus DriverClient::checkStatus() {
    if (access(device_path.c_str(), F_OK) != 0) {
        last_error = "Device node '" + device_path + "' not found. Kernel module sysguard is likely not loaded.\n"
                     "  -> Suggestion: Load driver with 'sudo ./scripts/load_driver.sh' or 'sudo insmod driver/sysguard.ko'";
        return DriverAccessStatus::DEVICE_NOT_FOUND;
    }

    if (access(device_path.c_str(), R_OK | W_OK) != 0) {
        last_error = "Permission denied accessing '" + device_path + "'.\n"
                     "  -> Suggestion: Run with sudo or grant access via 'sudo chmod 666 " + device_path + "'";
        return DriverAccessStatus::PERMISSION_DENIED;
    }

    return DriverAccessStatus::AVAILABLE;
}

bool DriverClient::openDevice() {
    if (fd >= 0) return true;

    DriverAccessStatus st = checkStatus();
    if (st != DriverAccessStatus::AVAILABLE) {
        return false;
    }

    fd = open(device_path.c_str(), O_RDWR);
    if (fd < 0) {
        last_error = "Failed to open '" + device_path + "': " + std::strerror(errno);
        return false;
    }
    return true;
}

void DriverClient::closeDevice() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool DriverClient::readDiagnostics(std::string& out_diagnostics) {
    if (!openDevice()) return false;

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        last_error = "read() failed from '" + device_path + "': " + std::strerror(errno);
        return false;
    }

    buffer[bytes] = '\0';
    out_diagnostics = std::string(buffer);
    return true;
}

bool DriverClient::getDriverStatus(struct sysguard_status& out_status) {
    if (!openDevice()) return false;

    memset(&out_status, 0, sizeof(out_status));
    if (ioctl(fd, SYSGUARD_IOCTL_GET_STATUS, &out_status) < 0) {
        last_error = "ioctl(SYSGUARD_IOCTL_GET_STATUS) failed: " + std::string(std::strerror(errno));
        return false;
    }
    return true;
}

bool DriverClient::getKernelInfo(struct sysguard_kernel_info& out_info) {
    if (!openDevice()) return false;

    memset(&out_info, 0, sizeof(out_info));
    if (ioctl(fd, SYSGUARD_IOCTL_GET_KERNEL_INFO, &out_info) < 0) {
        last_error = "ioctl(SYSGUARD_IOCTL_GET_KERNEL_INFO) failed: " + std::string(std::strerror(errno));
        return false;
    }
    return true;
}

bool DriverClient::resetStats() {
    if (!openDevice()) return false;

    if (ioctl(fd, SYSGUARD_IOCTL_RESET_STATS) < 0) {
        last_error = "ioctl(SYSGUARD_IOCTL_RESET_STATS) failed: " + std::string(std::strerror(errno));
        return false;
    }
    return true;
}

bool DriverClient::sendEcho(const std::string& message, std::string& reply) {
    if (!openDevice()) return false;

    struct sysguard_custom_msg msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.message, message.c_str(), sizeof(msg.message) - 1);
    msg.length = static_cast<uint32_t>(message.length());

    if (ioctl(fd, SYSGUARD_IOCTL_ECHO_MSG, &msg) < 0) {
        last_error = "ioctl(SYSGUARD_IOCTL_ECHO_MSG) failed: " + std::string(std::strerror(errno));
        return false;
    }

    reply = msg.message;
    return true;
}

bool DriverClient::testInvalidIoctl() {
    if (!openDevice()) return false;

    // Send an invalid ioctl command (magic 'z', number 99)
    unsigned int invalid_cmd = _IO('z', 99);
    int ret = ioctl(fd, invalid_cmd, 0);

    if (ret < 0 && (errno == ENOTTY || errno == EINVAL)) {
        return true; // Successfully rejected invalid command
    }
    return false;
}

void DriverClient::printReport() {
    std::cout << Color::BOLD << Color::CYAN << "┌────────────────────────────────────────────────────────┐\n"
              << "│            KERNEL MODULE & DRIVER STATUS               │\n"
              << "└────────────────────────────────────────────────────────┘" << Color::RESET << "\n";

    DriverAccessStatus st = checkStatus();
    if (st == DriverAccessStatus::DEVICE_NOT_FOUND) {
        std::cout << "  " << Color::RED << "Status: DRIVER NOT LOADED" << Color::RESET << "\n";
        std::cout << "  " << Color::YELLOW << last_error << Color::RESET << "\n\n";
        return;
    }

    if (st == DriverAccessStatus::PERMISSION_DENIED) {
        std::cout << "  " << Color::RED << "Status: PERMISSION DENIED" << Color::RESET << "\n";
        std::cout << "  " << Color::YELLOW << last_error << Color::RESET << "\n\n";
        return;
    }

    if (!openDevice()) {
        std::cout << "  " << Color::RED << "Status: OPEN FAILED" << Color::RESET << "\n";
        std::cout << "  " << Color::YELLOW << last_error << Color::RESET << "\n\n";
        return;
    }

    struct sysguard_status drv_status;
    struct sysguard_kernel_info kinfo;

    bool status_ok = getDriverStatus(drv_status);
    bool kinfo_ok = getKernelInfo(kinfo);

    if (status_ok) {
        std::cout << "  " << Color::BOLD << "Driver Device:   " << Color::RESET << SYSGUARD_DEVICE_PATH << "\n";
        std::cout << "  " << Color::BOLD << "Driver Module:   " << Color::RESET << drv_status.driver_name << "\n";
        std::cout << "  " << Color::BOLD << "Driver Version:  " << Color::GREEN
                  << "v" << drv_status.version_major << "." << drv_status.version_minor << "." << drv_status.version_patch
                  << Color::RESET << "\n";
        std::cout << "  " << Color::BOLD << "Driver State:    " << Color::GREEN << drv_status.status_str << Color::RESET << "\n";
        std::cout << "  " << Color::BOLD << "Read Ops Count:  " << Color::RESET << drv_status.total_read_ops << "\n";
        std::cout << "  " << Color::BOLD << "IOCTL Ops Count: " << Color::RESET << drv_status.total_ioctl_ops << "\n";
    }

    if (kinfo_ok) {
        std::cout << "\n  " << Color::BOLD << "Kernel-Space Telemetry (via IOCTL):" << Color::RESET << "\n";
        std::cout << "    Kernel Release: " << kinfo.kernel_release << "\n";
        std::cout << "    Kernel Uptime:  " << Utils::formatDuration(kinfo.uptime_sec) << "\n";
        std::cout << "    Total RAM:      " << Utils::formatBytes(kinfo.total_ram_bytes) << "\n";
        std::cout << "    Free RAM:       " << Utils::formatBytes(kinfo.free_ram_bytes) << "\n";
        std::cout << "    Active Tasks:   " << kinfo.total_processes << "\n";
        std::cout << "    Online CPUs:    " << kinfo.cpu_count << "\n";
    }

    // Demonstrate read() interface
    std::string diag;
    if (readDiagnostics(diag)) {
        std::cout << "\n  " << Color::BOLD << "Character Device read() Output:" << Color::RESET << "\n";
        std::istringstream ss(diag);
        std::string line;
        while (std::getline(ss, line)) {
            std::cout << "    " << Color::DIM << line << Color::RESET << "\n";
        }
    }

    // Verify invalid ioctl validation (NFR-03 & Security)
    bool invalid_handled = testInvalidIoctl();
    std::cout << "\n  " << Color::BOLD << "Security Validation:" << Color::RESET << "\n";
    std::cout << "    Invalid IOCTL Rejection: "
              << (invalid_handled ? (std::string(Color::GREEN) + "PASSED (-ENOTTY returned)" + Color::RESET)
                                  : (std::string(Color::RED) + "FAILED" + Color::RESET))
              << "\n";

    std::cout << "\n";
}

} // namespace SysGuard
