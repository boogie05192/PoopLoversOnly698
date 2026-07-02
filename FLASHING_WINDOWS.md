# Flashing the Pico 2 W on Windows

## 1. Put the Pico into bootloader mode
- Connect the Pico to the PC with a USB cable while holding the BOOTSEL bdutton.
- Release BOOTSEL after the drive appears as a USB mass storage device.

## 2. Build the firmware
In VS Code, run the build task:
- "Build Pico firmware"

## 3. Copy the UF2 file
- Find the generated UF2 file in the `build/` folder.
- Drag and drop it onto the Pico drive.
- The Pico will reboot and run the new firmware.

## 4. Check the output
- Open a serial terminal at 115200 baud.
- You should see startup messages from the firmware.
