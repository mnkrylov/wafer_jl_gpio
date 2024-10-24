#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/wmi.h>  // Include WMI interface
#include <linux/acpi.h>
#include <linux/types.h>

#define WMI_DEVICE_GUID "ABBC0F6A-8EA1-11D1-00A0-C90629100000"  // Replace with the correct GUID
#define WMI_ID_IM06 0x06  // Replace with the correct method ID

// Function to print the contents of the buffer in bytes
static void print_output_buffer(struct acpi_buffer *buffer)
{
    if (buffer && buffer->pointer) {
        unsigned char *data = buffer->pointer;
        size_t i;

        // Print all bytes in hex format
        pr_info("Output buffer (hex): ");
        for (i = 0; i < buffer->length; i++) {
            pr_cont("%02x", data[i]);  // Print each byte in hex without a newline
        }
        pr_cont("\n");  // End the line after printing all bytes
    } else {
        pr_info("Output buffer is empty or NULL.\n");
    }
}

// Function to call the WMI method
static acpi_status call_wmi_im06_method(u32 arg1)
{
    acpi_status status;
    struct acpi_buffer input_buffer;
    struct acpi_buffer output_buffer = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result

    input_buffer.length = sizeof(arg1);
    input_buffer.pointer = &arg1;

    // Call the WMI method
    status = wmi_evaluate_method(WMI_DEVICE_GUID, 0, WMI_ID_IM06, &input_buffer, &output_buffer);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call WMI method with arg 0x%x\n", arg1);
    } else {
        pr_info("Successfully called WMI method with arg 0x%x\n", arg1);

        // Print the contents of output_buffer
        print_output_buffer(&output_buffer);
    }
    
    // Free the result buffer if the method returned data
    if (output_buffer.pointer) {
        kfree(output_buffer.pointer);
        output_buffer.pointer = NULL;
    }

    return status;
}

static int __init wmi_method_caller_init(void)
{
    int i, j;
    acpi_status status;

    // Array of values to call the method
    unsigned int arg_values[] = {  0x1000000
                                ,  0x1000001
                                ,  0x1000002 
                                ,  0x1000004 
                                ,  0x1000008 
                                ,  0x100003F  
                                };

    // Loop to call the method 3 times
    for (i = 0; i < 1; i++) {
        for (j = 0; j < 6; j++) {
            status = call_wmi_im06_method(arg_values[j]);
            if (ACPI_FAILURE(status)) {
                pr_err("Failed WMI call on cycle %d\n", i);
            }

            // Pause for 1 second
            msleep(1000);
        }
    }

    return 0;
}

static void __exit wmi_method_caller_exit(void)
{
    pr_info("WMI method caller module unloaded.\n");
}

module_init(wmi_method_caller_init);
module_exit(wmi_method_caller_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krylov MN");
MODULE_DESCRIPTION("WMI method caller for IM06");
