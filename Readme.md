# Wafer-JL GPIO on Linux

This project is focused on implementing GPIO control for the Wafer-JL board. It includes data for analysis and code examples to support the development of GPIO management through various methods, such as low-level interactions via ACPI methods.


## Current Status

The implementation of GPIO control is still in progress. The project contains collected ACPI methods and WMI-related data that will be used in the development.

## Files Overview

- **acpi_methods.txt**: Contains the ACPI methods relevant to the Wafer-JL board, including GPIO methods such as `STEC` (Set DIO State) and `SDIO` (Set DIO).
- **guesses.txt**: Contains guessed WMI device GUIDs and associated methods for GPIO control.
- **iasl_data.txt**: Data dump from `iasl`, useful for debugging and understanding ACPI tables.
- **wafer_jl_STEC.c**: C source code that demonstrates GPIO management using ACPI methods.

## How to Build

1. Navigate to the `examples/acpi/tests/STEC` directory.
2. Run `make` to build the example code.
3. Execute `test.sh` to test GPIO functionality.

---

Feel free to contribute or report any issues!
