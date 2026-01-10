# ArduCAM NORD Zephyr Module for nRF5340 + 7002 EK Shield

This Zephyr/NCS module provides **support for the ArduCAM Mega SPI camera** on the nRF5340, including a minimal capture demo and BLE/Wi-Fi integration. It is designed to work with the **nRF5340 Development Kit** and **nRF7002 Wi-Fi/BLE shield**.

---

## Features

* Manual CS GPIO for **precise SPI timing**
* Minimal capture demo that:

  * Triggers an image capture
  * Polls until capture is complete
  * Reads FIFO length of captured image
  * Logs first 16 bytes (JPEG header expected)
* Wi-Fi + BLE image transfer capability
* Supports **camera controls**: resolution, brightness, contrast, saturation, exposure, white balance, special effects, sharpness, manual gain/exposure, low-power mode
* Compatible with **off-tree NCS integration**
* Flexible **SPI/I²C/pin configuration**

---

## Hardware Images

| Image                                         | Description                                   |
| --------------------------------------------- | --------------------------------------------- |
| ![nRF7002 beside box](images/nrf7002_box.jpg) | nRF7002 EK Shield next to its box             |
| ![nRF5340 beside box](images/nrf5340_box.jpg) | nRF5340 DK beside its box                     |
| ![Scale with ruler](images/scale_ruler.jpg)   | Physical scale reference with ruler           |
| ![Backside header](images/back_header.jpg)    | Backside showing custom header for SPI access |
| ![SPI wiring](images/spi_wiring.jpg)          | SPI wiring with shield on top                 |

*(Replace `images/...` with your actual image file paths.)*

---

## Sample Driver Summary

The included **Wi-Fi + BLE camera driver** demonstrates real-time capture and streaming of images from the ArduCAM Mega via SPI:

* **Device Initialization**

  * Detects and configures ArduCAM Mega through `video` API (`DEVICE_DT_GET(DT_NODELABEL(arducam0))`)
  * Allocates multiple video buffers and queues them for capture

* **Camera Control**

  * Commands for resolution, picture/video mode, brightness, contrast, saturation, exposure, gain, white balance, special effects, sharpness, low-power mode
  * Supports manual or automatic camera settings
  * Uses `video_set_ctrl()` and `video_get_ctrl()` for camera property control

* **Streaming & Capture**

  * Handles single captures and continuous streams
  * Streams to BLE clients via `app_bt_send_picture_data()`
  * Streams to Wi-Fi clients via `cam_send()` over sockets
  * Tracks active streaming state per client (`client_state_ble`, `client_state_socket`)

* **Command Handling**

  * Commands are queued via `k_msgq` (`msgq_app_commands`)
  * Commands processed in the main loop include:

    * Take picture
    * Change camera resolution
    * Start/stop streaming
    * Socket commands from Wi-Fi clients
  * Uses `recv_process()` to parse incoming commands

* **Performance Monitoring**

  * Counts bytes sent per second with a timer (`k_timer`) for bandwidth monitoring
  * Logs debug info via `LOG_INF` / `LOG_DBG`

* **Bluetooth Integration**

  * `app_bt_callbacks` handle BLE connection, readiness, disconnection, capture, stream enable, and resolution changes
  * Sends camera info and status to BLE client automatically

* **Socket Integration**

  * Receives commands and data from TCP/UDP clients
  * Sends picture data in chunks for efficient transmission

This driver serves as a **reference implementation** for streaming ArduCAM Mega data over BLE and Wi-Fi in Zephyr using nRF5340 + nRF7002.

---

## Setup Instructions

1. **Add the module to your workspace** via `west.yml`:

```yaml
- name: arducam_ble_wifi
  remote: arducam_ble_wifi
  revision: main
  path: modules/arducam_ble_wifi
```

2. **Update your workspace**:

```bash
west update
```

3. **Build the sample**:

```bash
west build -b nrf5340dk_nrf5340_cpuapp_ns modules/arducam_nord_wifi_ble_ncs
```

4. **Flash to the board**:

```bash
west flash
```

---

## Device Tree / Peripheral Configuration

```dts
&uart0 { status = "okay"; current-speed = <921600>; };
&i2c0 { status = "disabled"; };
&spi0 { status = "disabled"; };
&i2c1 { status = "disabled"; };

&spi1 {
    label = "spi1";
    status = "okay";
    pinctrl-0 = <&spi1_default>;
    pinctrl-1 = <&spi1_sleep>;
    pinctrl-names = "default", "sleep";
    cs-gpios = <&gpio0 25 GPIO_ACTIVE_LOW>;

    arducam0: arducam@0 {
        compatible = "arducam,nord";
        reg = <0>;
        spi-max-frequency = <8000000>;
        label = "arducam0";
        status = "okay";
    };
};

&pinctrl {
    spi1_default: spi1_default {
        group1 { psels = <NRF_PSEL(SPIM_SCK,0,6)>, <NRF_PSEL(SPIM_MOSI,0,7)>, <NRF_PSEL(SPIM_MISO,0,26)>; };
    };
    spi1_sleep: spi1_sleep {
        group1 { psels = <NRF_PSEL(SPIM_SCK,0,6)>, <NRF_PSEL(SPIM_MOSI,0,7)>, <NRF_PSEL(SPIM_MISO,0,26)>; low-power-enable; };
    };
};
```

---

## Related Repositories

* [devacademy-ncsinter](https://github.com/NordicDeveloperAcademy/ncs-inter) – Course exercises and solutions
* [Wi-Fi + BLE Image Transfer Demo](https://github.com/NordicPlayground/nrf70-wifi-ble-image-transfer-demo) – Nordic’s image transfer example for nRF70
* [nRF Connect SDK](https://github.com/nrfconnect/sdk-nrf) – Official Nordic SDK repository

---

## Extra Documentation

* [Installation Guide (Off-Tree)](docs/OFF_TREE_INSTALL.md) – Step-by-step, beginner-friendly instructions
* [Porting Notes & Pitfalls](docs/PORTING_NOTES.md) – Key logic decisions and gotchas during off-tree porting
