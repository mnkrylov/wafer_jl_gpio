/*
 *
 * LED-Blink (ko)
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>

#define ACPI_METHOD_NAME "\\_SB.ISM.SDIO"  // ACPI method name

// Function to set up and invoke the ACPI method
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

    // Calling the ACPI method
    status = acpi_evaluate_object(NULL, ACPI_METHOD_NAME, &arg_list, &result);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call SDIO ACPI method %s with arg 0x%x \n", ACPI_METHOD_NAME, arg1);
    } else {
        pr_info("Successfully called SDIO ACPI method %s with arg 0x%x \n", ACPI_METHOD_NAME, arg1);
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
           // status = call_acpi_sdio_method(0);
            status = call_acpi_sdio_method(arg_values[j]);
            if (ACPI_FAILURE(status)) {
                pr_err("Failed first SDIO ACPI call on cycle %d\n", i);
            }

            // Pause for 1 second
            msleep(1000);
        }
    }

    return 0;
}

static void __exit acpi_method_caller_exit(void)
{
    pr_info("SDIO ACPI method caller module unloaded.\n");
}

module_init(acpi_method_caller_init);
module_exit(acpi_method_caller_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krylov MN");
MODULE_DESCRIPTION("ACPI method caller for \\_SB.ISM.SDIO");
