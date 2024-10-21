#!/bin/bash
make
insmod wafer_jl_SDIO.ko
dmesg | tail
rmmod wafer_jl_SDIO
