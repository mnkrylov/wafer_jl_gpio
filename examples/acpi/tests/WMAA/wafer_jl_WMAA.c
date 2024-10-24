
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>

#define ACPI_METHOD_NAME "\\ISM.WMAA"  // ACPI method name

// Function to set up and invoke the ACPI method
static acpi_status call_acpi_wmaa_method(unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    acpi_status status;
    struct acpi_object_list arg_list;
    union acpi_object args[3];  // Arguments for the STEC method
    struct acpi_buffer result = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result

    // Setting up arguments for the method call
    args[0].type = ACPI_TYPE_INTEGER;
    args[0].integer.value = arg1;

    args[1].type = ACPI_TYPE_INTEGER;
    args[1].integer.value = arg2;  // Value for args[1] (0x51 or 0x71)


    args[2].type = ACPI_TYPE_BUFFER;
    args[2].buffer.length = 4;
    args[2].buffer.pointer = (char*)&arg3;


    arg_list.count =3;
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
    int arg_size;

    // Array of values for the method call
    unsigned int arg_values[] = {  0x1000000
                                ,  0x1000001
                                ,  0x1000002 
                                ,  0x1000004 
                                ,  0x1000008 
                                ,  0x100003F  
                                };
    arg_size= sizeof(arg_values);
    // Loop to call the method 5 times
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 6; j++) {
            status = call_acpi_wmaa_method(0, 0x06, arg_values[j]);
            if (ACPI_FAILURE(status)) {
                pr_err("Failed first ACPI call on cycle %d\n", i);
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
MODULE_DESCRIPTION("ACPI method caller for WMAA->IO6->SDIO");
