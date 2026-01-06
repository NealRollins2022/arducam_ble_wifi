
# Installing the Course Repository as a Third-Party (Off-Tree) Repository

This guide shows how to add the course repository as a third-party repository inside the nRF Connect SDK. You only need to do this **once** for the entire course.

## Step A: Open the nRF Connect Terminal

1. Launch **VS Code**.
2. Open the **nRF Connect Terminal** (do not use the regular VS Code terminal).
   - The nRF Connect Terminal ensures the `west` command and other SDK tools are available.
3. If prompted, select the **SDK version or toolchain** you plan to use.

## Step B: Open the SDK Manifest File

1. In the nRF Connect Terminal, run:

   ```bash
   west manifest --path
   ```

2. Hover over the printed file path, hold **Ctrl**, and **left-click** to open the `west.yml` file.

## Step C: Add the Course Repository

1. In the `west.yml` file, locate the section:

   ```yaml
   # Other third-party repositories
   ```

2. Add the following lines **exactly as shown with proper indentation**:

   ```yaml
   - name: devacademy-ncsinter
     path: nrf/samples/devacademy/ncs-inter
     revision: main
     url: https://github.com/NordicDeveloperAcademy/ncs-inter
   ```

3. Notes:
   - `revision: main` corresponds to **nRF Connect SDK v3.1.1**.
   - For other SDK versions, change the `revision` to the branch matching your SDK.
   - Incorrect indentation will cause `west` to fail.

## Step D: Fetch the Repository

1. In the nRF Connect Terminal, run:

   ```bash
   west update devacademy-ncsinter
   ```

2. The course exercises and solutions will be downloaded to:

   ```
   <SDK_PATH>/nrf/samples/devacademy/ncs-inter
   ```

Once this is done, the course repository is integrated into your SDK, ready for use in VS Code.

