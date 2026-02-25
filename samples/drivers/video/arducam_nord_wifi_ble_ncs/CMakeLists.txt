# Minimum required version must come first
cmake_minimum_required(VERSION 3.20.0)

# Tell Zephyr where your out-of-tree modules are BEFORE find_package
list(APPEND ZEPHYR_EXTRA_MODULES "C:/ncs/v3.2.0/modules/arducam_ble_wifi")

# Find Zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(arducam_nord_wifi_ble_ncs)

# Add your application source files
file(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# Add your include directories
target_include_directories(app PRIVATE include)

