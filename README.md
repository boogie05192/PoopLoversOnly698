# Rocket logger for Raspberry Pi Pico 2 W

This workspace contains a starter firmware project for a Pico 2 W system that will eventually log data from a BerryIMU v3 and an SD card module.

## What this scaffold does
- Initializes USB serial output.
- Configures I2C for the IMU.
- Configures SPI for the SD card interface.
- Reads a sample from the IMU when the device responds.
- Prints the sample over USB and forwards it to a placeholder SD logging function.

## Hardware wiring (starting point)
- IMU I2C:
  - SDA -> GP0
  - SCL -> GP1
- SD card SPI:
  - SCK -> GP10
  - MOSI -> GP11
  - MISO -> GP12
  - CS -> GP13
- LED -> GP25

## Build steps in VS Code
1. Install the Raspberry Pi Pico SDK and the ARM GCC toolchain.
   - If you cloned `pico-sdk` next to this project, CMake will use it automatically.
   - Otherwise set `PICO_SDK_PATH` to the SDK root or use `-DPICO_SDK_PATH=` with CMake.
2. Install CMake and Ninja if they are not already available.
3. Open this folder in VS Code.
4. If picotool does not build, use the no-picotool flow:
   - `cmake -S . -B build_nopicotool -G Ninja -DPICO_NO_PICOTOOL=1`
   - `cmake --build build_nopicotool`
5. Convert the binary into UF2:
   - `py pico-sdk/tools/uf2/utils/uf2conv.py -b 0x1000 -o build_nopicotool/rocket_logger.uf2 build_nopicotool/rocket_logger.bin`
   - If that file is missing, your `pico-sdk` clone is incomplete. Run:
     - `git -C pico-sdk fetch --all`
     - `git -C pico-sdk reset --hard origin/master`
     - `git -C pico-sdk submodule update --init --recursive`
6. Put the Pico into bootloader mode and copy the UF2 file to it.

> This repository includes the generated `build_nopicotool/rocket_logger.uf2` and `build_nopicotool/rocket_logger.bin` artifacts so another user can download the repo and flash the Pico directly.

> If you later install a host compiler and `picotool`, you can switch back to the normal `build` directory and remove `-DPICO_NO_PICOTOOL=1`.

## Toolchain note
This workspace is ready for Pico firmware development, but the environment here did not have the required compiler tools installed, so I could not complete a build test from this session.

## Quick VS Code workflow
- Open the Command Palette and run "Tasks: Run Build Task".
- Use the "Build Pico firmware (no picotool)" task to build the project.
- Use the "Create UF2 file" task to convert the binary output into UF2.
- Use the "Flash Pico UF2" task and enter the Pico bootloader drive letter when prompted.
- If you prefer manual flashing, follow [FLASHING_WINDOWS.md](FLASHING_WINDOWS.md) to copy the UF2 file to the Pico.

## Important next steps
- Keep the IMU address at 0x6A for the LSM6DSL gyro/accelerometer unless your board shows a different address.
- The SD logger now writes raw 512-byte blocks to the card so you can recover the data later without needing a filesystem yet.
- Test the firmware in stages:
  1. Confirm the LED blinks and USB serial prints.
  2. Confirm the IMU responds on I2C.
  3. Confirm the SD logger writes blocks and can read them back.
  4. If you want a true file system later, replace the raw-block logger with FATFS or LittleFS.
- A built-in test mode is enabled by default so the firmware emits synthetic sample values even before the hardware is connected.
- The firmware is designed to run continuously without any Pico button input.
