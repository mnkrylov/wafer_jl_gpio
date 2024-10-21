#!/bin/bash
make
insmod wafer_jl_STEC.ko
dmesg | tail
rmmod wafer_jl_STEC
