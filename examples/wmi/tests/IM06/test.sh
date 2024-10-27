#!/bin/bash
make
insmod wafer_jl_im06_wmi.ko
dmesg | tail
rmmod wafer_jl_im06_wmi
