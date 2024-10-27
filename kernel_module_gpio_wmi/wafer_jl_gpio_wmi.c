#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/wmi.h>  
#include <linux/types.h>
#include <linux/slab.h>

#define ACPI_METHOD_NAME "\\ISM.SDIO"  // ACPI method name user \\_SB.ISM.SDIO for BIOS 1.3
#define DEVICE_NAME "gpio_wafer"  // Device name
#define CLASS_NAME "gpio_class"   // Class name for the device
#define WMI_DEVICE_GUID "ABBC0F6A-8EA1-11D1-00A0-C90629100000"  // WMI device GUID
#define WMI_ID_IM06 0x06  // Replace with the correct method ID

static dev_t dev_number;           // Major and minor device number
static struct class *gpio_class;   // Pointer to device class
static struct cdev gpio_cdev;      // Character device structure
static struct acpi_buffer output_buffer = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result

// Convert buffer to hex string
static int buffer_to_hex_str(const unsigned char *buffer, size_t length, char *hex_str, size_t hex_str_size) {
    int i;
    int offset = 0;

    // Convert each byte to hex format
    for (i = 0; i < length && (offset + 2) < hex_str_size; i++) {
        offset += scnprintf(hex_str + offset, hex_str_size - offset, "%02x", buffer[i]);
    }
    return offset;
}

// Function to call the IM06 WMI method
static acpi_status call_wmi_im06_method(u32 arg1)
{
    acpi_status status;
    struct acpi_buffer input_buffer;

    input_buffer.length = sizeof(arg1);
    input_buffer.pointer = &arg1;

    // Call the WMI method
    status = wmi_evaluate_method(WMI_DEVICE_GUID, 0, WMI_ID_IM06, &input_buffer, &output_buffer);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call WMI method with arg 0x%x\n", arg1);
    } else {
        pr_info("Successfully called WMI method with arg 0x%x\n", arg1);
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

    // Invoke the WMI method with the provided argument
    call_wmi_im06_method(arg1);

    return len;
}

// Read function for the device
static ssize_t gpio_wafer_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    acpi_status status;
    union acpi_object *obj;
    char *hex_str;
    size_t hex_str_len;

    // Call the WMI method with argument 0x0000000
    status = call_wmi_im06_method(0x0000000);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call WMI method for reading.\n");
        return -EIO;
    }

    if (!output_buffer.pointer) {
        pr_info("No data returned from WMI method.\n");
        return 0;  // No data to read
    }

    obj = output_buffer.pointer;

    // Check if the result is a buffer
    if (obj->type == ACPI_TYPE_BUFFER) {
        hex_str_len = obj->buffer.length * 2 + 1;  // Each byte is 2 hex chars + null terminator
        hex_str = kmalloc(hex_str_len, GFP_KERNEL);
        if (!hex_str) {
            pr_err("Failed to allocate memory for hex string.\n");
            return -ENOMEM;
        }

        // Convert buffer to hex string
        buffer_to_hex_str(obj->buffer.pointer, obj->buffer.length, hex_str, hex_str_len);

        // Copy hex string to user space
        if (copy_to_user(buffer, hex_str, min(len, hex_str_len))) {
            kfree(hex_str);
            return -EFAULT;
        }

        kfree(hex_str);
        return min(len, hex_str_len);
    } else {
        pr_info("Unexpected data type from WMI method.\n");
        return -EINVAL;
    }
}

// File operations for the character device
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = gpio_wafer_write,
    .read = gpio_wafer_read,
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
MODULE_DESCRIPTION("WMI method caller for IM06 through /dev/gpio_wafer");
