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

static acpi_status carusel1(void){
    //from DIO0 to DIO5;
    /*
     (1 << 24) | (1 << 0) DIO0 HIGH 
     (1 << 24) | (1 << 1) DIO1 HIGH
     ...
     (1 << 24) | (1 << 5) DIO5 HIGH
    */
    unsigned int  j;
    acpi_status status;
    for (j = 0; j < 6; j++) {
        status = call_acpi_sdio_method( (1 << 24) | (1 << j));
        if (ACPI_FAILURE(status)) {
            pr_err("Failed first SDIO ACPI call on cycle %d of carusel1\n", j);
        }
        msleep(500);
    }
    return status;
}

static acpi_status ladder(void){
    //set HIGH from DIO0 to DIO5 with holding of the previous state;
    /*
     (1 << 24) | (1 << 0) | (1 << 1) DIO0 HIGH && DIO1 HIGH
     ...
     (1 << 24) | (1 << 0) | (1 << 1) | ... | (1 << 4) | (1 << 5) DIO0 to DIO5 HIGH
    */
    unsigned int j, state;
    acpi_status status;
    state = 1<<24; 
    for (j = 0; j < 6; j++) {
        state |= (1<<j);
        status = call_acpi_sdio_method( state);
        if (ACPI_FAILURE(status)) {
            pr_err("Failed first SDIO ACPI call on cycle %d of ladder \n ", j);
        }
        msleep(500);
    }
    return status;
}

static acpi_status reversed_ladder(void){
    //set LOW from DIO5 to DIO0 with holding of the previous state;
    /*
     (1 << 24) | (1 << 0) | (1 << 1) DIO0 HIGH && DIO1 HIGH
     ...
     (1 << 24) | (1 << 0) | (1 << 1) | ... | (1 << 4) | (1 << 5) DIO0 to DIO5 HIGH
    */
    unsigned int j, state;
    acpi_status status;
    state = 0x100003F; // ALL HIGH 
    for (j = 0; j < 6; j++) {
        state &= ~(1<<j);
        status = call_acpi_sdio_method( state);
        if (ACPI_FAILURE(status)) {
            pr_err("Failed first SDIO ACPI call on cycle %d of R. Ladder\n ", j);
        }
        msleep(500);
    }
    return status;
}


static acpi_status blink_all(void){
    //set LOW from DIO5 to DIO0 with holding of the previous state;
    /*
     (1 << 24) | (1 << 0) | (1 << 1) DIO0 HIGH && DIO1 HIGH
     ...
     (1 << 24) | (1 << 0) | (1 << 1) | ... | (1 << 4) | (1 << 5) DIO0 to DIO5 HIGH
    */
    unsigned int  j, state;
    acpi_status status;
    for (j = 0; j < 6; j++) {
        status = call_acpi_sdio_method( 0x1000000); //All LOW
        if (ACPI_FAILURE(status)) {
            pr_err("Failed first SDIO ACPI call on cycle %d of blink_all step 1\n ", j);
        }
        msleep(1000);
        status = call_acpi_sdio_method( 0x100003F);  //All HIGH
        msleep(1000);
        if (ACPI_FAILURE(status)) {
              pr_err("Failed first SDIO ACPI call on cycle %d of blink_all step 2\n ", j);
        }
        msleep(500);
    }
    return status;
}


static int __init acpi_method_caller_init(void)
{
    unsigned int i;
    acpi_status status;
    // Loop to call the method 5 times
    for (i = 0; i < 2; i++) {
        carusel1();
        ladder();
        reversed_ladder();
        blink_all();

        status = call_acpi_sdio_method( 0x1000000); //All LOW
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
