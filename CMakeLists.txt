# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ADC_for_Vape)

target_sources(app PRIVATE 
    src/main.c
    src/saadc.c
    src/error_handling.c
    src/timer_and_ppi.c
)
