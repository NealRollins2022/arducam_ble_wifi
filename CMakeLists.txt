# modules/arducam-ble-wifi/CMakeLists.txt

# Only compile the Arducam Nord driver when building the CPUAPP image
# This prevents compilation in net core, hci_ipc, TF-M, etc.
# We detect CPUAPP build via the standard NCS Kconfig symbol used in multi-image projects

if(CONFIG_VIDEO_ARDUCAM_NORD AND CONFIG_CPUAPP)
  # Expose public headers
  target_include_directories(zephyr_interface INTERFACE
    ${ZEPHYR_CURRENT_MODULE_DIR}/include
  )

  # Force compilation of the driver source — only once, in CPUAPP
  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/arducam_nord/arducam_nord.c
  )

  # Optional: if you have a CMakeLists.txt in drivers/video/
  if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/CMakeLists.txt)
    add_subdirectory(drivers/video)
  endif()

endif()
