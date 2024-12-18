# Wafer-JL GPIO on Linux
**Device GUID** ABBC0F6A-8EA1-11D1-00A0-C90629100000.

This project is focused on implementing GPIO control for the Wafer-JL board. It includes data for analysis and code examples to support the development of GPIO management through various methods, such as low-level interactions via ACPI methods.

## Current Status

The implementation of GPIO control is still in progress. The project contains collected ACPI methods and WMI-related data that will be used in the development.

## Files Overview

- **acpi_methods.txt**: Contains the ACPI methods relevant to the Wafer-JL board, including GPIO methods such as `STEC` (Set DIO State) and `SDIO` (Set DIO).
- **guesses.txt**: Contains guessed WMI device GUIDs and associated methods for GPIO control.
- **iasl_data.txt**: Data dump from `iasl`, useful for debugging and understanding ACPI tables.
- **examples/acpi/\<method name\>**: Examples of ACPI method calls.
- **examples/wmi/\<method name\>**: Examples of WMI method calls.

## Available now

### Examples of ACPI calls:

- **STEC**: Manually set the DIO state (called from SDIO) with additional parameters.
- **SDIO**: Interface for setting DIO values.
- **WMAA**: Root method for selecting an action based on argument 2. Implemented WMAA->IM06->SDIO call for toggling the LED.

### Examples of WMI calls:

- **WMAA**: IM06 - Method for DIO control (equivalent to SetDIOWMI). Toggles the LED and prints the returned status in `dmesg`.

> **Note:** All test outputs print additional debug information in `dmesg`.

## ACPI Methods

The full name of an ACPI method may depend on the version of the BIOS. For example, it was found that:

```c
#define ACPI_METHOD_NAME "\\_SB.ISM.SDIO" // for BIOS version 1.3
// or
#define ACPI_METHOD_NAME "\\ISM.SDIO"     // for BIOS version 1.4
```

### Important:

It is crucial to specify the correct `ACPI_METHOD_NAME` in the `.c` file of the kernel module to ensure proper GPIO control. Incorrect method names can result in failure to communicate with the GPIO hardware. Make sure to update the method name in your source code based on the BIOS version detected, like so:

To check the current ACPI method name (for example, for the `SDIO` method), you can use the `acpiexec` tool from the `acpica-tools` package. Run the following command to search for the `SDIO` method in the DSDT table:

```
acpiexec -b Method -l /sys/firmware/acpi/tables/DSDT | grep SDIO
```

## SDIO values

| DIO Output | Bit Position | Hex Value  | Full Command Value  |
|------------|--------------|------------|---------------------|
| DIO0       | 0            | 0x00000001 | 0x1000001           |
| DIO1       | 1            | 0x00000002 | 0x1000002           |
| DIO2       | 2            | 0x00000004 | 0x1000004           |
| DIO3       | 3            | 0x00000008 | 0x1000008           |
| DIO4       | 4            | 0x00000010 | 0x1000010           |
| DIO5       | 5            | 0x00000020 | 0x1000020           |
## Kernel Module: `wafer_jl_gpio`

The `wafer_jl_gpio` kernel module, located in the `kernel_module_gpio` directory, allows users to control GPIO through `/dev/gpio_wafer`. This provides a simple interface for managing GPIO pins. For example:

To turn off all DIOs:

```
echo 0x1000000 > /dev/gpio_wafer
```

To control specific DIO outputs, refer to the SDIO values in the table above and use corresponding hex values. For instance, to turn on DIO0:

```
echo 0x1000001 > /dev/gpio_wafer
```

Or to turn on DIO1:

```
echo 0x1000002 > /dev/gpio_wafer
```

## Kernel Module: `wafer_jl_gpio_wmi` RECOMMENDED

The `wafer_jl_gpio_wmi` kernel module offers similar functionality to `wafer_jl_gpio.ko` but with some key distinctions. Installation and operation are identical; simply load the module using `insmod`, and interact with it via `/dev/gpio_wafer`.

### Key Features

1. **Potential Independence from BIOS Version:** Unlike ACPI-based methods, which can vary across BIOS versions, `wafer_jl_gpio_wmi` relies on WMI method numbers, making it potentially less dependent on the specific BIOS version.
  
2. **Additional Functionality:**
   - Besides commands like:
  
     ```
     echo 0x1000002 > /dev/gpio_wafer
     ```
  
   - `wafer_jl_gpio_wmi` also reads the current DIO buffer state, including INPUT pins. For example, you can check the current state of all DIO pins by running:
  
     ```
     cat /dev/gpio_wafer
     ```

This allows users not only to set GPIO states but also to monitor the input state of the DIO pins, enhancing GPIO management capabilities on the Wafer-JL board.

## How to Build

For building the project examples, navigate to the respective directories and use `make`. For example:

1. Navigate to the `examples/acpi/tests/<project>` directory.
2. Run `make` to build the example code (this will build all examples in the project).
3. Execute `test.sh` to test GPIO functionality.

To build the kernel module:

1. Navigate to the `kernel_module_gpio` directory.
2. Run `make` to build the kernel module.
3. Load the module using `insmod` and interact with `/dev/gpio_wafer` as described above.

---

Feel free to contribute or report any issues!
