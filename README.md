# ArduCAM Mega Zephyr Module for nRF5340

This module adds support for the ArduCAM Mega SPI camera in the nRF Connect SDK.

## Features
- Manual CS GPIO for precise timing
- Minimal capture demo that:
  - Triggers a capture
  - Polls until done
  - Reads FIFO length
  - Logs first 16 bytes of image (JPEG header expected)

## Setup

1. Add this module to your NCS workspace via `west.yml`:

```yaml
- name: arducam_ble_wifi
  remote: arducam_ble_wifi
  revision: main
  path: modules/arducam_ble_wifi
```

2. Update your workspace:

```bash
west update
```

3. Build the sample:

```bash
west build -b nrf5340dk_nrf5340_cpuapp_ns modules/arducam/samples/drivers/video/arducam_mega
```

4. Flash:

```bash
west flash
```

## Related Repositories

- [devacademy-ncsinter](https://github.com/NordicDeveloperAcademy/ncs-inter) – Course exercises and solutions
- [Wi-Fi + BLE Image Transfer Demo](https://github.com/NordicPlayground/nrf70-wifi-ble-image-transfer-demo) – Nordic’s image transfer example for nRF70
- [nRF Connect SDK](https://github.com/nrfconnect/sdk-nrf) – Official Nordic SDK repository

