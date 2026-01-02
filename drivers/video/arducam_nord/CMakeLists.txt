# Existing content
zephyr_library()

zephyr_library_sources(arducam_nord.c)

# Prepend for local overrides
target_include_directories(zephyr_interface BEFORE INTERFACE ${ZEPHYR_CURRENT_MODULE_DIR}/include)
