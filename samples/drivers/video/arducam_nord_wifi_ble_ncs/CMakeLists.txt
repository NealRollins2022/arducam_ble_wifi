#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)
# Add before find_package(Zephyr ...)
list(APPEND ZEPHYR_EXTRA_MODULES C:/ncs/v2.7.0/modules/arducam-ble-wifi)  # Replace with your exact path; use forward slashes for CMake compatibility
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(WiFi_BLE_Camera)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE include)
