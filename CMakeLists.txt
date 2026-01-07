# CMakeLists.txt for Arducam module (C:\ncs\v2.7.0\modules\arducam\CMakeLists.txt)
  target_include_directories(zephyr_interface BEFORE INTERFACE
    ${ZEPHYR_CURRENT_MODULE_DIR}/include
  )
if(CONFIG_VIDEO_ARDUCAM_NORD AND CONFIG_CPUAPP)
  # Add include directory for drivers/video/arducam_nord.h


  # Include driver source file (updated path)
  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/arducam_nord/arducam_nord.c
  )

  # Add drivers/video subdirectory only if it contains a CMakeLists.txt
  if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/CMakeLists.txt)
    add_subdirectory(drivers/video)
  endif()
  if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/spi/CMakeLists.txt)
    add_subdirectory(drivers/spi)
  endif()
endif()
