# MMD-KM16 Custom RAW HID Firmware

This is a custom firmware and Node.js client library for the MMD KM16 macropad.

![MMD KM16](km16.jpg)



## Background

The MMD-KM16 is an excellent macropad keypad controller that runs [QMK](https://qmk.fm/), appears to Windows
as a regular HID keyboard and is programmable via the well-known [VIA](https://www.usevia.app/) tool.

For most use cases it's perfectly fine.  For custom integrations however, direct RAW HID access is preferable
because it:

* Bypasses the operating system keyboard stack.
* Allows exclusive control of device from a particular application (ie: works for a dedicated app
  regardless of operating system input focus)
* Allows direct control of LED lighting without introducing flash-wear.

The project provides a custom firmware with RAW HID USB interface and a Node.js client
library for interfacing with the device.

The firmware also includes an optional "watch-dog" timer that displays a flashing red indicator on the device
if no client program is connected (eg: client program crashed, OS hung, etc...).


## Disclaimer

**USE AT YOUR OWN RISK.** This software is provided without any warranty or guarantee of any kind.
The authors take no responsibility for any damage, including but not limited to bricked devices,
that may result from using this firmware.

This project was developed through reverse engineering of a specific MMD-KM16 device. There is no
guarantee that other devices sold under the same or similar branding will have identical hardware,
bootloaders, or behavior. Variations in manufacturing, hardware revisions, or regional differences
may cause this firmware to be incompatible with or damaging to your device.

Before flashing, ensure you have backed up your original firmware and understand the risks involved.


## Hardware Spec

There is no public official hardware specification for the KM16 however the device has been
reverse engineered and documented in the file [km16_hardware_spec.md](./notes/km16_hardware_spec.md).


## Firmware Implementation

The firmware is implemented as an Arduino sketch in the [firmware](./firmware/) folder.

The firmware is mostly straight-forward except for the custom USB RAW HID implementation which is
[documented here](./notes/RawHID_Implementation.md).


## Installing the Firmware

### Saving the Original Firmware

Before proceeding it's highly recommended to backup the device's original firmware:

1. Install [dfu-util](https://dfu-util.sourceforge.net/releases/)

2. Put the device into bootloader mode by pressing the top-left key while connecting the USB cable.

3. Optional: check the bootloader is active:

    ```
    dfu-util -l
    ```

    you should see something like:

    ```
    dfu-util 0.11

    Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
    Copyright 2010-2021 Tormod Volden and Stefan Schmidt
    This program is Free Software and has ABSOLUTELY NO WARRANTY
    Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

    Found DFU: [1eaf:0003] ver=0201, devnum=1, cfg=1, intf=0, path="2-3", alt=2, name="STM32duino bootloader v1.0  Upload to Flash 0x8002000", serial="LLM 003"
    Found DFU: [1eaf:0003] ver=0201, devnum=1, cfg=1, intf=0, path="2-3", alt=1, name="STM32duino bootloader v1.0  Upload to Flash 0x8005000", serial="LLM 003"
    Found DFU: [1eaf:0003] ver=0201, devnum=1, cfg=1, intf=0, path="2-3", alt=0, name="STM32duino bootloader v1.0  ERROR. Upload to RAM not supported.", serial="LLM 003"
    ```

4. Save your old firmware:

    ```
    dfu-util -d 1eaf:0003 -a 2 -U original_firmware.bin
    ```

    You will see an "Error during upload" message at the end — this is normal and expected.
    Verify the output shows "Received a total of 122880 bytes":

    ```
    dfu-util 0.11

    Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
    Copyright 2010-2021 Tormod Volden and Stefan Schmidt
    This program is Free Software and has ABSOLUTELY NO WARRANTY
    Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

    Opening DFU capable USB device...
    Device ID 1eaf:0003
    Device DFU version 0110
    Claiming USB DFU Interface...
    Setting Alternate Interface #2 ...
    Determining device status...
    DFU state(2) = dfuIDLE, status(0) = No error condition is present
    DFU mode device DFU version 0110
    Device returned transfer size 2048
    Copying data from DFU device to PC
    Upload  [======================== ]  99%         2048 bytes
    Error during upload (LIBUSB_ERROR_PIPE)

    Received a total of 122880 bytes
    ```

5. Check the file `original_firmware.bin` is 120KB and store it somewhere safe.


### Restoring the Original Firmware

1. Put the device into bootloader mode by pressing the top-left key while connecting the USB cable.

2. Run the following command:

    ```
    dfu-util -d 1eaf:0003 -a 2 -D original_firmware.bin
    ```


### Building and Flashing the Custom Firmware

The build process should work on any platform where arduino-cli is supported.

1. Install [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/)

2. Install the STM32 board support:

    ```
    arduino-cli config add board_manager.additional_urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
    arduino-cli core update-index
    arduino-cli core install STMicroelectronics:stm32
    ```

3. Put the device into bootloader mode by pressing the top-left key while connecting the USB cable.

4. Build and flash the firmware.

    On Linux/macOS (or Windows with Git Bash), run the provided script:

    ```
    cd firmware
    ./build-and-flash
    ```

    On other platforms, run the equivalent commands directly:

    ```
    arduino-cli compile \
        --fqbn STMicroelectronics:stm32:GenF1:pnum=BLACKPILL_F103CB,upload_method=dfu2Method,xserial=generic,usb=none \
        --export-binaries .

    dfu-util -d 1eaf:0003 -a 2 -D build/STMicroelectronics.stm32.GenF1/firmware.ino.bin
    ```

5. After flashing, the device will automatically reboot into the new firmware. The layer indicator
   LED should be flashing red, indicating the watchdog is active and no client is connected.


## Using the Node.js Library

The Node.js library has been tested on Windows with Node.js v22. It should work on any OS and
Node.js version where [node-hid](https://github.com/node-hid/node-hid) is supported, but this
is untested.

To install the Node.js package:

```
npm install --save @toptensoftware/km16
```

See the [example program](./node/test/test.js) for how to use it. See also the [km16.js](./node/km16.js) file itself.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
