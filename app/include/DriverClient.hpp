#pragma once

#include <string>
#include <cstdint>
#include "../../include/sysguard_ioctl.h"

namespace SysGuard {

enum class DriverAccessStatus {
    AVAILABLE,
    DEVICE_NOT_FOUND,
    PERMISSION_DENIED,
    OPEN_FAILED,
    COMMUNICATION_ERROR
};

class DriverClient {
public:
    DriverClient(const std::string& dev_path = SYSGUARD_DEVICE_PATH);
    ~DriverClient();

    DriverAccessStatus checkStatus();
    bool openDevice();
    void closeDevice();

    bool readDiagnostics(std::string& out_diagnostics);
    bool getDriverStatus(struct sysguard_status& out_status);
    bool getKernelInfo(struct sysguard_kernel_info& out_info);
    bool resetStats();
    bool sendEcho(const std::string& message, std::string& reply);
    bool testInvalidIoctl();

    void printReport();

    std::string getLastError() const { return last_error; }

private:
    std::string device_path;
    int fd{-1};
    std::string last_error;
};

} // namespace SysGuard
