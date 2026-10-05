#ifndef SYSGUARD_IOCTL_H
#define SYSGUARD_IOCTL_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#else
#include <stdint.h>
#include <sys/ioctl.h>
#endif

#define SYSGUARD_DEVICE_NAME     "sysguard"
#define SYSGUARD_DEVICE_PATH     "/dev/sysguard"
#define SYSGUARD_CLASS_NAME      "sysguard_class"

#define SYSGUARD_VERSION_MAJOR   1
#define SYSGUARD_VERSION_MINOR   0
#define SYSGUARD_VERSION_PATCH   0

#define SYSGUARD_IOC_MAGIC       'g'

/* Data structure returned by SYSGUARD_IOCTL_GET_STATUS */
struct sysguard_status {
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t version_patch;
    uint64_t module_load_time_sec;
    uint64_t total_read_ops;
    uint64_t total_ioctl_ops;
    char     driver_name[32];
    char     status_str[64];
};

/* Data structure returned by SYSGUARD_IOCTL_GET_KERNEL_INFO */
struct sysguard_kernel_info {
    uint64_t uptime_sec;
    uint64_t total_ram_bytes;
    uint64_t free_ram_bytes;
    uint32_t total_processes;
    uint32_t cpu_count;
    char     kernel_release[64];
};

/* Data structure for testing bidirectional IOCTL transfer */
struct sysguard_custom_msg {
    char     message[128];
    uint32_t length;
};

/* IOCTL Command definitions */
#define SYSGUARD_IOCTL_GET_STATUS      _IOR(SYSGUARD_IOC_MAGIC, 1, struct sysguard_status)
#define SYSGUARD_IOCTL_GET_KERNEL_INFO  _IOR(SYSGUARD_IOC_MAGIC, 2, struct sysguard_kernel_info)
#define SYSGUARD_IOCTL_RESET_STATS      _IO(SYSGUARD_IOC_MAGIC, 3)
#define SYSGUARD_IOCTL_ECHO_MSG         _IOWR(SYSGUARD_IOC_MAGIC, 4, struct sysguard_custom_msg)

#define SYSGUARD_IOC_MAXNR 4

#endif /* SYSGUARD_IOCTL_H */
