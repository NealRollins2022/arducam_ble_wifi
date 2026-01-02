# No Kconfig guard needed — driver is always built when module is included

# Add include directory for headers (e.g., <drivers/video/arducam_nord.h>)
zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)

# Include the driver source
zephyr_library_sources(
    drivers/video/arducam_nord/arducam_nord.c
)

# If you have a CMakeLists.txt in drivers/video/, include it
if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/CMakeLists.txt)
    add_subdirectory(drivers/video)
endif()
