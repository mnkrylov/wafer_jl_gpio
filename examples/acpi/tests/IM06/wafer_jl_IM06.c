
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>

#define ACPI_METHOD_NAME "\\ISM.IM06"  // ACPI method name

static void print_args_bytes(union acpi_object *args, int count) {
    int i, j;
    unsigned char *data;

    for (i = 0; i < count; i++) {
        if (args[i].type == ACPI_TYPE_INTEGER) {
            pr_info("Arg[%d] (INTEGER): 0x%llx", i, args[i].integer.value);
            pr_cont(" (%zu)\n", sizeof(args[i].integer.value));
        } else if (args[i].type == ACPI_TYPE_BUFFER) {
            pr_info("Arg[%d] (BUFFER): ", i);
            data = (unsigned char *)args[i].buffer.pointer;
            for (j = 0; j < args[i].buffer.length; j++) {
                pr_cont("%02x ", data[j]);
               
            }
            pr_cont(" (%zu) [%zu]\n", sizeof(data[0]), sizeof(data));
        }
        
    }
}

static void print_result_buffer(struct acpi_buffer *result) {
    if (result && result->pointer) {
        union acpi_object *obj = result->pointer;
        switch (obj->type) {
            case ACPI_TYPE_INTEGER:
                pr_info("Result (INTEGER): 0x%llx\n", obj->integer.value);
                break;
            case ACPI_TYPE_STRING:
                pr_info("Result (STRING): %s\n", obj->string.pointer);
                break;
            case ACPI_TYPE_BUFFER:
                pr_info("Result (BUFFER): ");
                for (size_t i = 0; i < obj->buffer.length; i++) {
                    pr_cont("%02x ", obj->buffer.pointer[i]);
                }
                pr_cont("\n");
                break;
            default:
                pr_info("Result type is unsupported or unknown\n");
                break;
        }
    } else {
        pr_info("Result buffer is empty or NULL.\n");
    }
}


// Function to set up and invoke the ACPI method
static acpi_status call_acpi_IM06_method(unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    acpi_status status;
    struct acpi_object_list arg_list;
    union acpi_object args[1];  // Arguments for the STEC method
    struct acpi_buffer result = { ACPI_ALLOCATE_BUFFER, NULL };  // Buffer for the result
    
    // Setting up arguments for the method call
    args[0].type = ACPI_TYPE_BUFFER;
    args[0].buffer.length = 4;
    args[0].buffer.pointer = (char*)&arg3;
    //print_args_bytes(args, 3);

    arg_list.count =1;
    arg_list.pointer = args;
    // Calling the ACPI method
    status = acpi_evaluate_object(NULL, ACPI_METHOD_NAME, &arg_list, &result);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to call ACPI method %s with args 0x%x and 0x%x\n", ACPI_METHOD_NAME, arg1, arg3);
    } else {
        print_result_buffer(&result);
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
                                 , 0x0000000    
                                };
    arg_size= sizeof(arg_values);
    // Loop to call the method 5 times
    for (i = 0; i < 1; i++) {
        for (j = 0; j < 7; j++) {
            status = call_acpi_IM06_method(0, 0x06, arg_values[j]);
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
MODULE_DESCRIPTION("ACPI method caller for IM06->IO6->SDIO");
