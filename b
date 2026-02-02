#!/bin/bash

# Build
arduino-cli compile \
    --fqbn STMicroelectronics:stm32:GenF1:pnum=BLACKPILL_F103CB,upload_method=dfu2Method,xserial=generic,usb=none \
    --verbose \
    --export-binaries . > build.log 2>&1


# Upload (device must be in DFU mode)
#dfu-util -d 1eaf:0003 -a 2 -D build/STMicroelectronics.stm32.GenF1/km16_firmware.ino.bin

# Monitor
#sleep 0.7s
#python -m serial.tools.miniterm COM4 115200