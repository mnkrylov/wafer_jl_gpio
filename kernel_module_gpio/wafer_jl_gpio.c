#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/wmi.h>  // For working with WMI

#define ACPI_METHOD_NAME "\\ISM.SDIO"  // ACPI method name user \\_SB.ISM.SDIO for BIOS 1.3
#define DEVICE_NAME "gpio_wafer"  // Device name
#define CLASS_NAME "gpio_class"   // Class name for the device
#define WMI_DEVICE_GUID "ABBC0F6A-8EA1-11D1-00A0-C90629100000"  // WMI device GUID

static dev_t dev_number;           // Major and minor device number
static struct class *gpio_class;   // Pointer to device class
static struct cdev gpio_cdev;      // Character device structure

// Function to invoke the SDIO ACPI method
static acpi_status call_acpi_sdio_method(unsigned int arg1)
{
    acpi_status status;
    struct acpi_object_list arg_list;
    union acpi_object args[1];  // Arguments for the SDIO method
    struct acpi_buffer result = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result

    arg_list.count = 1;
    arg_list.pointer = args;
    args[0].type = ACPI_TYPE_INTEGER;
    args[0].integer.value = arg1;

    // Call the ACPI method
    status = acpi_evaluate_object(NULL, ACPI_METHOD_NAME, &arg_list, &result);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call SDIO ACPI method %s with arg 0x%x\n", ACPI_METHOD_NAME, arg1);
    }

    // Free the result buffer if the method returned data
    if (result.pointer) {
        kfree(result.pointer);
        result.pointer = NULL;  // Set the pointer to NULL after freeing memory
    }

    return status;
}

// Write function for the device
static ssize_t gpio_wafer_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    unsigned int arg1;
    char buf[16];
    
    if (len > sizeof(buf) - 1)
        return -EINVAL;

    // Copy data from user space
    if (copy_from_user(buf, buffer, len))
        return -EFAULT;
    
    buf[len] = '\0';  // Null-terminate the string

    // Convert the string to an integer
    if (kstrtouint(buf, 0, &arg1))
        return -EINVAL;

    // Invoke the ACPI method with the provided argument
    call_acpi_sdio_method(arg1);

    return len;
}

// File operations for the character device
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = gpio_wafer_write,
};

// Init function to create the character device
static int __init gpio_wafer_init(void)
{
    int ret;

    // Check if the WMI device with the given GUID is present
    if (!wmi_has_guid(WMI_DEVICE_GUID)) {
        pr_err("WMI device with GUID %s not found\n", WMI_DEVICE_GUID);
        return -ENODEV;  // Device not found, return an error
    }

    // Step 1: Register the character device
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }

    // Step 2: Initialize the character device
    cdev_init(&gpio_cdev, &fops);
    gpio_cdev.owner = THIS_MODULE;
    ret = cdev_add(&gpio_cdev, dev_number, 1);
    if (ret < 0) {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    // Step 3: Create the device class
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        pr_err("Failed to create class\n");
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(gpio_class);
    }

    // Step 4: Create the device file in /dev
    device_create(gpio_class, NULL, dev_number, NULL, DEVICE_NAME);
    pr_info("/dev/%s device created\n", DEVICE_NAME);

    return 0;
}

// Exit function to clean up
static void __exit gpio_wafer_exit(void)
{
    // Remove the device file
    device_destroy(gpio_class, dev_number);

    // Destroy the device class
    class_destroy(gpio_class);

    // Remove the character device
    cdev_del(&gpio_cdev);

    // Free the device number
    unregister_chrdev_region(dev_number, 1);

    pr_info("/dev/%s device removed\n", DEVICE_NAME);
}

module_init(gpio_wafer_init);
module_exit(gpio_wafer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krylov MN");
MODULE_DESCRIPTION("ACPI method caller for \\_SB.ISM.SDIO through /dev/gpio_wafer with WMI device check");
