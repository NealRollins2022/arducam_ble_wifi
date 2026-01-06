# Off-Tree ArduCAM Mega Zephyr Module (nRF5340)

This repository contains an **off-tree port** of the ArduCAM Mega SPI driver and a minimal sample program for the **nRF5340** using the **nRF Connect SDK / Zephyr**.

It allows you to capture images from an **ArduCAM Mega SPI camera** on Nordic boards, even if you are new to Zephyr or `west`.

---

## Features

- Capture JPEG images using the **ArduCAM Mega SPI camera**.
- Manual CS GPIO for precise SPI timing.
- Minimal capture demo:
  - Triggers a capture
  - Polls until complete
  - Reads FIFO length
  - Logs first 16 bytes of image (JPEG header expected)

---

## Quick Start (Baby-Proof)

### Step 1 — Create or go to a Zephyr workspace

A workspace is just a folder where your projects live.  

```bash
mkdir ~/zephyr-workspace
cd ~/zephyr-workspace
```

> On Windows PowerShell, replace with `mkdir C:\zephyr-workspace` and `cd C:\zephyr-workspace`.

---

### Step 2 — Add this off-tree module

Open (or create) `west.yml` in your workspace and add:

```yaml
projects:
  - name: arducam_offtree
    remote: <your-repo-url>
    revision: main
    path: modules/arducam
```

> Replace `<your-repo-url>` with the URL to this repository.

---

### Step 3 — Update the workspace

```bash
west update
```

> Downloads the ArduCAM module and any dependencies.

---

### Step 4 — Build the sample application

```bash
west build -b nrf5340dk_nrf5340_cpuapp_ns modules/arducam/samples/drivers/video/arducam_mega
```

- `-b nrf5340dk_nrf5340_cpuapp_ns` → selects CPU app for nRF5340.  
- Last argument → path to the sample program.

---

### Step 5 — Flash the board

1. Connect the nRF5340 DK via USB.  
2. Run:

```bash
west flash
```

> The board will reboot after flashing.

---

### Step 6 — Verify the capture

Open a **serial terminal** (e.g., PuTTY, minicom) to the board’s COM port.  
Expected output:

```
<inf> app: Starting capture...
<inf> app: Capture complete, FIFO length = 12345 bytes
<inf> app: FIFO[0:16]: 0xFF 0xD8 0xFF 0xE0 ...
```

- `0xFF 0xD8 0xFF 0xE0` confirms a valid JPEG header.

---

## Extra Documentation

- [Installation Guide (Off-Tree)](docs/OFF_TREE_INSTALL.md) — step-by-step, beginner-friendly instructions.  
- [Porting Notes & Pitfalls](docs/PORTING_NOTES.md) — major logic decisions and gotchas during this off-tree port.

---

## License

MIT License. See [LICENSE](LICENSE) for details.
