#!/bin/bash
make
insmod wafer_jl_WMAA.ko
dmesg | tail
rmmod wafer_jl_WMAA
