#!/bin/bash
make
insmod wafer_jl_IM06.ko
dmesg | tail
rmmod wafer_jl_IM06
