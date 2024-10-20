/*
 *
 * LED-Blink (ko)
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>

#define ACPI_METHOD_NAME "\\_SB.ISM.STEC"  // ACPI method name

// Function to set up and invoke the ACPI method
static acpi_status call_acpi_stec_method(unsigned int arg1, unsigned int arg3)
{
    acpi_status status;
    struct acpi_object_list arg_list;
    union acpi_object args[4];  // Arguments for the STEC method
    struct acpi_buffer result = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result

    // Setting up arguments for the method call
    args[0].type = ACPI_TYPE_INTEGER;
    args[0].integer.value = 0x81;

    args[1].type = ACPI_TYPE_INTEGER;
    args[1].integer.value = arg1;  // Value for args[1] (0x51 or 0x71)

    args[2].type = ACPI_TYPE_INTEGER;
    args[2].integer.value = 0x88;

    args[3].type = ACPI_TYPE_INTEGER;
    args[3].integer.value = arg3;  // Value for args[3] (from the argument list)

    arg_list.count = 4;
    arg_list.pointer = args;

    // Calling the ACPI method
    status = acpi_evaluate_object(NULL, ACPI_METHOD_NAME, &arg_list, &result);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call ACPI method %s with args 0x%x and 0x%x\n", ACPI_METHOD_NAME, arg1, arg3);
    } else {
        pr_info("Successfully called ACPI method %s with args 0x%x and 0x%x\n", ACPI_METHOD_NAME, arg1, arg3);
    }

    // Free the result buffer if the method returned data
    if (result.pointer) {
        kfree(result.pointer);
        result.pointer = NULL;  // Set the pointer to NULL after freeing memory
    }

    return status;
}

static int __init acpi_method_caller_init(void)
{
    int i, j;
    acpi_status status;

    // Array of values for the method call
    unsigned int arg_values[] = {  0x1000000, 0x1FFFFFF };

    // Loop to call the method 5 times
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 2; j++) {
            // First method call (0x51 for args[1])
            status = call_acpi_stec_method(0x51, (((arg_values[j] & 0x0F) << 0x04) & 0xF0));
            if (ACPI_FAILURE(status)) {
                pr_err("Failed first ACPI call on cycle %d\n", i);
            }

            // Second method call (0x71 for args[1])
            status = call_acpi_stec_method(0x71, (((arg_values[j] & 0x30) >> 0x02) & 0x0C));
            if (ACPI_FAILURE(status)) {
                pr_err("Failed second ACPI call on cycle %d\n", i);
            }

            // Pause for 1 second
            msleep(1000);
        }
    }

    return 0;
}

static void __exit acpi_method_caller_exit(void)
{
    pr_info("ACPI method caller module unloaded.\n");
}

module_init(acpi_method_caller_init);
module_exit(acpi_method_caller_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krylov MN");
MODULE_DESCRIPTION("ACPI method caller for \\_SB.ISM.STEC with argument sequence and delay");
