# Arducam NORD Driver for nRF5340 CPUAPP

This document explains how to integrate and build the Arducam NORD off-tree driver for the nRF5340 **CPUAPP** image in Zephyr. It ensures modularity, prevents duplicate compilation, and avoids undefined references in other images (CPUNET, Shield, TF-M).  

---

## 1. Build Integration

### Gate the Driver to CPUAPP

In your module `CMakeLists.txt`:

```cmake
if(CONFIG_VIDEO_ARDUCAM_NORD AND CONFIG_CPUAPP)
  target_include_directories(zephyr_interface BEFORE INTERFACE
    ${ZEPHYR_CURRENT_MODULE_DIR}/include
  )

  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/arducam_nord/arducam_nord.c
  )

  if(EXISTS ${ZEPHYR_CURRENT_MODULE_DIR}/drivers/video/CMakeLists.txt)
    add_subdirectory(drivers/video)
  endif()
endif()
```

**Key Points:**

- Ensures compilation occurs **only in CPUAPP**.
- Keeps the off-tree driver modular.
- Prevents undefined references in other images.
- `BEFORE INTERFACE` exposes headers to dependent targets.

---

### Include Submodules Before the Consumer Application

In your **root `CMakeLists.txt`**, add:

```cmake
add_subdirectory(modules/arducam)
```

This ensures your application sees the driver and its headers before building.

---

## 2. Device Initialization

Use **`DT_NODELABEL`** to obtain the video device:

```c
video = DEVICE_DT_GET(DT_NODELABEL(arducam0));
if (!device_is_ready(video)) {
    LOG_ERR("Video device %s not ready.", video->name);
    return -1;
}
```

**Purpose:**

- Ensures the video subsystem is initialized correctly.
- Avoids runtime null pointer errors.

---

## 3. Video Buffer Management

- Allocate video buffers manually.
- Enqueue buffers using `video_enqueue()` before starting the stream.
- Stream processing (e.g., BLE or socket transmission) occurs in your **main loop**.

---

## 4. Build Command

```bash
west build -b nrf5340dk_nrf5340_cpuapp --pristine
```

**Requirements:**

- `CONFIG_CPUAPP` must be enabled in `prj.conf`.
- The driver will **not** compile in other images (CPUNET, Shield, TF-M).

---

## 5. Key Notes / Gotchas

- Off-tree modules must expose headers via:

  ```cmake
target_include_directories(... BEFORE INTERFACE)
```

- Gate compilation **only** via CMake, **not Kconfig**.
- The video subsystem and Arducam driver must be included together.
- Avoid source-level duplication: each driver source should compile in **one image only**.
- This guide describes only **verified behavior** — no assumptions about Kconfig symbols or message queues.

---

## 6. Visual Overview

```text
+------------------+       +------------------+
|    CPUAPP        |       |   CPUNET / TF-M  |
|------------------|       |-----------------|
| Arducam NORD     |       | No Arducam build |
| Video Subsystem  |       |                 |
| Application      |       |                 |
+------------------+       +-----------------+
```

- The **Arducam driver** and **video subsystem** are compiled only in CPUAPP.
- Other images do not compile the driver, preventing conflicts.

