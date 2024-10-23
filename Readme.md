# Wafer-JL GPIO on Linux

This project is focused on implementing GPIO control for the Wafer-JL board. It includes data for analysis and code examples to support the development of GPIO management through various methods, such as low-level interactions via ACPI methods.

## Current Status

The implementation of GPIO control is still in progress. The project contains collected ACPI methods and WMI-related data that will be used in the development.

## Files Overview

- **acpi_methods.txt**: Contains the ACPI methods relevant to the Wafer-JL board, including GPIO methods such as `STEC` (Set DIO State) and `SDIO` (Set DIO).
- **guesses.txt**: Contains guessed WMI device GUIDs and associated methods for GPIO control.
- **iasl_data.txt**: Data dump from `iasl`, useful for debugging and understanding ACPI tables.

## Available now

- **STEC**: Manually set the DIO state (called from SDIO) with additional parameters.
- **SDIO**: Interface for setting DIO values.

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
