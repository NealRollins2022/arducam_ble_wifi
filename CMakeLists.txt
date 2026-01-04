# CMakeLists.txt for Arducam module (C:\ncs\v2.7.0\modules\arducam-ble-wifi\CMakeLists.txt)

# Override headers FIRST (for local video.h and video-controls.h)
target_include_directories(zephyr_interface BEFORE INTERFACE
    ${ZEPHYR_CURRENT_MODULE_DIR}/include
)

# Add the driver source — unconditionally (since it's an off-tree module)
zephyr_library_sources(
    drivers/video/arducam_nord/arducam_nord.c
)
# Add VIDEO core only if not already in the build
if(CONFIG_VIDEO)
  add_subdirectory(${ZEPHYR_BASE}/drivers/video drivers_video)
endif()

# REMOVE THIS BLOCK — it is causing duplicate addition of the source
# if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/CMakeLists.txt)
#     add_subdirectory(drivers/video)
# endif()
