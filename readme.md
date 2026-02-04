# MMD-KM16 Custom RAW HID Firmware

This is a custom firmware and Node.js client library for the MMD KM16 macropad.

## Background

The MMD-KM16 is an excellent macropad keypad controller that runs QMK, appears to Windows
as a regular HID keyboard and is programmable via the well-known VIA tool.

For most use cases it's perfectly fine.  For custom integrations however, direct RAW HID access is preferrable:

* Bypasses the operating system keyboard stack.
* Allows exclusive control of device from a particular application (ie: works for a dedicated app
  regardless of operating system input focus)
* Allows direct control of LED lighting without introducing flash-wear.

The project provides a custom firmware with RAW HID USB interface and a Node.js client
library for interfacing with the device.


## Hardware Spec

There is no public official hardware specification for the KM16 however the device has been
reverse engineered and documented in the file [km16_hardware_spec.md](./notes/km16_hardware_spec.md).


## Firmware Implementation

The firmware is implemented as an Arduino sketch in the [firmware](./firmware/) folder.

The firmware is mostly straight-forward except for the custom USB RAW HID implementation which is 
[documented here](./notes/RawHID_Implementation.md).


## Installing the Firmware

### Saving and Restoring the Original Firmware

Before proceeding it's highly recommend to backup the device's original firmware:

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

4. Recommended: save your old firmware.  

    ```
    dfu-util -d 1eaf:0003 -a 2 -U original_firmware.bin
    ```

    The "Error during upload" is normal but before proceeding

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

5. Check the file `original_firmware.bin` is 120Kb and store it somewhere safe.



To restore the original firmware:

1. Put the device into bootloader mode by pressing the top-left key while connecting the USB cable.

2. Run the following command

    ```
    dfu-util -d 1eaf:0003 -a 2 -D original_firmware.bin
    ```


### Building and Flashing the Custom Firmware

To build and flash the firmware:

1. Install [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/)

2. Install the STM32 library:

    ```
    arduino-cli config add board_manager.additional_urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
    arduino-cli core update-index
    arduino-cli core install STMicroelectronics:stm32
    ```

3. Run the build script (requires bash) - this will build and flash the device (if connected)

    ```
    cd firmware
    ./b
    ```


## Using the Node.js Library

To install the Node.js package:

```
npm install --save @toptensoftware/km16
```

See the [example program](./node/test/test.js) for how to use it. See also the [km16.js](./node/km16.js) file itself.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.