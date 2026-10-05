/*
 * sysguard.c - Linux Kernel Module for SysGuard System Monitoring
 *
 * Provides a character device interface (/dev/sysguard) demonstrating
 * safe user-space <-> kernel-space communication via open, read, ioctl, and release.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/mm.h>
#include <linux/utsname.h>
#include <linux/mutex.h>
#include <linux/sched/stat.h>
#include <linux/cpumask.h>

#include "../include/sysguard_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysGuard Capstone Team");
MODULE_DESCRIPTION("SysGuard Character Device Driver for Linux System Monitoring");
MODULE_VERSION("1.0.0");

static dev_t dev_num;
static struct cdev sysguard_cdev;
static struct class *sysguard_class = NULL;
static struct device *sysguard_device = NULL;

static DEFINE_MUTEX(sysguard_lock);

static uint64_t load_time_sec = 0;
static uint64_t total_reads = 0;
static uint64_t total_ioctls = 0;
static int open_count = 0;

/* Device file operations */
static int sysguard_open(struct inode *inodep, struct file *filep)
{
    mutex_lock(&sysguard_lock);
    open_count++;
    pr_info("sysguard: device opened (active references: %d)\n", open_count);
    mutex_unlock(&sysguard_lock);
    return 0;
}

static int sysguard_release(struct inode *inodep, struct file *filep)
{
    mutex_lock(&sysguard_lock);
    if (open_count > 0)
        open_count--;
    pr_info("sysguard: device closed (active references: %d)\n", open_count);
    mutex_unlock(&sysguard_lock);
    return 0;
}

static ssize_t sysguard_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
    char msg[512];
    size_t msg_len;
    ssize_t bytes_to_copy;
    struct sysinfo si;

    if (*offset > 0)
        return 0; /* EOF */

    mutex_lock(&sysguard_lock);
    total_reads++;
    si_meminfo(&si);

    msg_len = snprintf(msg, sizeof(msg),
        "=== SysGuard Kernel Driver Diagnostics ===\n"
        "Driver: %s v%d.%d.%d\n"
        "Status: Active\n"
        "Driver Uptime: %llu seconds\n"
        "Total Read Ops: %llu\n"
        "Total IOCTL Ops: %llu\n"
        "Total RAM: %lu MB | Free RAM: %lu MB\n"
        "Online CPUs: %u\n",
        SYSGUARD_DEVICE_NAME,
        SYSGUARD_VERSION_MAJOR, SYSGUARD_VERSION_MINOR, SYSGUARD_VERSION_PATCH,
        (uint64_t)(ktime_get_boottime_seconds() - load_time_sec),
        total_reads,
        total_ioctls,
        (si.totalram * si.mem_unit) / (1024 * 1024),
        (si.freeram * si.mem_unit) / (1024 * 1024),
        num_online_cpus()
    );
    mutex_unlock(&sysguard_lock);

    if (len < msg_len)
        bytes_to_copy = len;
    else
        bytes_to_copy = msg_len;

    if (copy_to_user(buffer, msg, bytes_to_copy)) {
        pr_err("sysguard: copy_to_user failed in read()\n");
        return -EFAULT;
    }

    *offset += bytes_to_copy;
    return bytes_to_copy;
}

static long sysguard_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    /* Verify IOCTL magic number and command bounds */
    if (_IOC_TYPE(cmd) != SYSGUARD_IOC_MAGIC) {
        pr_warn("sysguard: invalid ioctl magic number 0x%x (expected 0x%x)\n",
                _IOC_TYPE(cmd), SYSGUARD_IOC_MAGIC);
        return -ENOTTY;
    }

    if (_IOC_NR(cmd) > SYSGUARD_IOC_MAXNR) {
        pr_warn("sysguard: invalid ioctl command number %u (max %u)\n",
                _IOC_NR(cmd), SYSGUARD_IOC_MAXNR);
        return -ENOTTY;
    }

    mutex_lock(&sysguard_lock);
    total_ioctls++;
    mutex_unlock(&sysguard_lock);

    switch (cmd) {
    case SYSGUARD_IOCTL_GET_STATUS: {
        struct sysguard_status status;
        memset(&status, 0, sizeof(status));

        mutex_lock(&sysguard_lock);
        status.version_major = SYSGUARD_VERSION_MAJOR;
        status.version_minor = SYSGUARD_VERSION_MINOR;
        status.version_patch = SYSGUARD_VERSION_PATCH;
        status.module_load_time_sec = load_time_sec;
        status.total_read_ops = total_reads;
        status.total_ioctl_ops = total_ioctls;
        strncpy(status.driver_name, SYSGUARD_DEVICE_NAME, sizeof(status.driver_name) - 1);
        strncpy(status.status_str, "OPERATIONAL", sizeof(status.status_str) - 1);
        mutex_unlock(&sysguard_lock);

        if (copy_to_user((struct sysguard_status __user *)arg, &status, sizeof(status))) {
            pr_err("sysguard: copy_to_user failed for GET_STATUS\n");
            return -EFAULT;
        }
        break;
    }

    case SYSGUARD_IOCTL_GET_KERNEL_INFO: {
        struct sysguard_kernel_info info;
        struct sysinfo si;
        memset(&info, 0, sizeof(info));

        si_meminfo(&si);
        info.uptime_sec = ktime_get_boottime_seconds();
        info.total_ram_bytes = (uint64_t)si.totalram * si.mem_unit;
        info.free_ram_bytes = (uint64_t)si.freeram * si.mem_unit;
        info.cpu_count = num_online_cpus();
        info.total_processes = (uint32_t)si.procs;
        strncpy(info.kernel_release, init_utsname()->release, sizeof(info.kernel_release) - 1);

        if (copy_to_user((struct sysguard_kernel_info __user *)arg, &info, sizeof(info))) {
            pr_err("sysguard: copy_to_user failed for GET_KERNEL_INFO\n");
            return -EFAULT;
        }
        break;
    }

    case SYSGUARD_IOCTL_RESET_STATS: {
        mutex_lock(&sysguard_lock);
        total_reads = 0;
        total_ioctls = 0;
        mutex_unlock(&sysguard_lock);
        pr_info("sysguard: operation counters reset\n");
        break;
    }

    case SYSGUARD_IOCTL_ECHO_MSG: {
        struct sysguard_custom_msg user_msg;

        if (copy_from_user(&user_msg, (struct sysguard_custom_msg __user *)arg, sizeof(user_msg))) {
            pr_err("sysguard: copy_from_user failed for ECHO_MSG\n");
            return -EFAULT;
        }

        /* Null terminate safe guarantee */
        user_msg.message[sizeof(user_msg.message) - 1] = '\0';
        pr_info("sysguard: received echo request: '%s'\n", user_msg.message);

        /* Modify message safely for roundtrip echo */
        char reply[128];
        snprintf(reply, sizeof(reply), "Kernel ACK: %s", user_msg.message);
        strncpy(user_msg.message, reply, sizeof(user_msg.message) - 1);
        user_msg.length = strlen(user_msg.message);

        if (copy_to_user((struct sysguard_custom_msg __user *)arg, &user_msg, sizeof(user_msg))) {
            pr_err("sysguard: copy_to_user failed for ECHO_MSG\n");
            return -EFAULT;
        }
        break;
    }

    default:
        return -ENOTTY;
    }

    return ret;
}

static const struct file_operations sysguard_fops = {
    .owner          = THIS_MODULE,
    .open           = sysguard_open,
    .release        = sysguard_release,
    .read           = sysguard_read,
    .unlocked_ioctl = sysguard_ioctl,
};

static int __init sysguard_init(void)
{
    int ret = 0;

    pr_info("sysguard: initializing module v%d.%d.%d\n",
            SYSGUARD_VERSION_MAJOR, SYSGUARD_VERSION_MINOR, SYSGUARD_VERSION_PATCH);

    load_time_sec = ktime_get_boottime_seconds();

    /* 1. Allocate dynamic major and minor numbers */
    ret = alloc_chrdev_region(&dev_num, 0, 1, SYSGUARD_DEVICE_NAME);
    if (ret < 0) {
        pr_err("sysguard: failed to allocate chrdev region (err: %d)\n", ret);
        return ret;
    }

    /* 2. Initialize and register cdev */
    cdev_init(&sysguard_cdev, &sysguard_fops);
    sysguard_cdev.owner = THIS_MODULE;

    ret = cdev_add(&sysguard_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("sysguard: failed to add cdev (err: %d)\n", ret);
        goto unregister_chrdev;
    }

    /* 3. Create device class (compat with Linux <6.4 and >=6.4) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    sysguard_class = class_create(SYSGUARD_CLASS_NAME);
#else
    sysguard_class = class_create(THIS_MODULE, SYSGUARD_CLASS_NAME);
#endif

    if (IS_ERR(sysguard_class)) {
        ret = PTR_ERR(sysguard_class);
        pr_err("sysguard: failed to create device class (err: %d)\n", ret);
        goto del_cdev;
    }

    /* 4. Create device node /dev/sysguard */
    sysguard_device = device_create(sysguard_class, NULL, dev_num, NULL, SYSGUARD_DEVICE_NAME);
    if (IS_ERR(sysguard_device)) {
        ret = PTR_ERR(sysguard_device);
        pr_err("sysguard: failed to create device /dev/%s (err: %d)\n", SYSGUARD_DEVICE_NAME, ret);
        goto destroy_class;
    }

    pr_info("sysguard: device registered successfully major=%d minor=%d at /dev/%s\n",
            MAJOR(dev_num), MINOR(dev_num), SYSGUARD_DEVICE_NAME);

    return 0;

destroy_class:
    class_destroy(sysguard_class);
del_cdev:
    cdev_del(&sysguard_cdev);
unregister_chrdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit sysguard_exit(void)
{
    pr_info("sysguard: cleaning up module\n");

    if (sysguard_device)
        device_destroy(sysguard_class, dev_num);

    if (sysguard_class)
        class_destroy(sysguard_class);

    cdev_del(&sysguard_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("sysguard: module unloaded successfully\n");
}

module_init(sysguard_init);
module_exit(sysguard_exit);
