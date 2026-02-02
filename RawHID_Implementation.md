# Raw HID Implementation for STM32 Arduino

This document describes the custom Raw HID (Custom HID) implementation added to this project for bidirectional USB communication with a host computer.

## Overview

The official STM32duino library supports USB HID for Keyboard and Mouse, but does not expose Raw HID (Custom HID) functionality at the Arduino level. This implementation adds Raw HID support by:

1. Copying and modifying the minimal required USB device files into the sketch's `src/` folder
2. Adding a new `USBD_USE_CUSTOM_HID` mode alongside the existing CDC and HID_COMPOSITE modes
3. Providing an Arduino-friendly `RawHID` wrapper class

## Why This Approach?

### The Problem

The STM32 Arduino USB stack has three layers:

```
Arduino Wrappers (Keyboard.h, Mouse.h, Serial)
         ↓
USBDevice Library (usbd_hid_composite_if.c, usbd_cdc_if.c)
         ↓
ST Middleware (usbd_customhid.c, usbd_cdc.c, etc.)
         ↓
USB Device Core + HAL
```

The CustomHID class exists in the ST Middleware (`system/Middlewares/.../Class/CustomHID/`) but:
- No Arduino wrapper exists for it
- The endpoint configuration (`usbd_ep_conf.c`) only defines endpoints for CDC or HID_COMPOSITE modes
- The descriptor file (`usbd_desc.c`) only handles CDC or HID_COMPOSITE

### The Solution

Rather than modifying the installed STM32 Arduino core (which would be overwritten on updates), we copy the necessary files into the sketch's `src/` folder. Arduino compiles sketch files with higher priority than library files, so our modified versions take precedence.

## File Structure

```
km16/
├── km16.ino                 # Main sketch
├── build_opt.h              # Compiler flags for USB configuration
├── src/
│   ├── RawHID.h             # Arduino wrapper - header
│   ├── RawHID.cpp           # Arduino wrapper - implementation
│   ├── usbd_customhid_if.h  # Custom HID interface - header
│   ├── usbd_customhid_if.c  # Custom HID interface - implementation
│   ├── usbd_customhid.h     # ST Custom HID class - header (from middleware)
│   ├── usbd_customhid.c     # ST Custom HID class - implementation (from middleware)
│   ├── usbd_ep_conf.h       # Endpoint configuration - modified for Custom HID
│   ├── usbd_ep_conf.c       # Endpoint configuration - modified for Custom HID
│   ├── usbd_desc.c          # USB descriptors - modified for Custom HID
│   ├── usbd_conf.c          # USB device configuration (from USBDevice library)
│   ├── usbd_if.c            # USB interface helpers (from USBDevice library)
│   ├── usb_device_core.c    # USB core wrapper (from USBDevice library)
│   ├── usb_device_ctlreq.c  # USB control requests wrapper (from USBDevice library)
│   └── usb_device_ioreq.c   # USB I/O requests wrapper (from USBDevice library)
```

## Build Configuration

### build_opt.h

This file contains compiler flags that configure the USB stack:

```c
-DUSBCON                              // Enable USB
-DUSBD_USE_CUSTOM_HID                 // Use Custom HID mode (not CDC or HID_COMPOSITE)
-DHAL_PCD_MODULE_ENABLED              // Enable USB peripheral driver
-DUSBD_VID=0x0483                     // USB Vendor ID
-DUSBD_PID=0x5750                     // USB Product ID
-DUSBD_CUSTOMHID_OUTREPORT_BUF_SIZE=64  // Receive buffer size
match actual descriptor)
```

### Arduino CLI Build Command

The build script (`b`) uses `usb=none` to prevent the built-in USB modes from conflicting:

```bash
arduino-cli compile \
    --fqbn STMicroelectronics:stm32:GenF1:pnum=BLACKPILL_F103CB,upload_method=dfu2Method,xserial=generic,usb=none \
    --export-binaries .
```

**Important:** Using `usb=none` disables the built-in USB configuration. All USB settings come from `build_opt.h` instead.

## Customization

### USB Vendor ID (VID) and Product ID (PID)

Edit `build_opt.h`:

```c
-DUSBD_VID=0x1234    // Your vendor ID
-DUSBD_PID=0x5678    // Your product ID
```

**Note:** Using someone else's VID without authorization violates USB-IF rules. For personal projects, you can use:
- `0x1209` - pid.codes (free PIDs for open source projects: https://pid.codes)
- `0x0483` - STMicroelectronics (default, acceptable for development)

### USB Product and Manufacturer Strings

Edit `build_opt.h` to add:

```c
-DUSB_PRODUCT_STRING="My Device Name"
-DUSB_MANUFACTURER_STRING="My Company"
```

Or edit `src/usbd_desc.c` directly to modify the string definitions.

### Report Size

The default configuration uses 64-byte reports (the maximum for Full Speed USB HID).

To change the report size, edit multiple files:

1. **build_opt.h** - Buffer sizes:
```c
-DUSBD_CUSTOMHID_OUTREPORT_BUF_SIZE=32   // Receive buffer (must be >= your report size)
```

2. **src/usbd_ep_conf.h** - Endpoint sizes:
```c
#define CUSTOM_HID_EPIN_SIZE          0x20U   // 32 bytes (hex)
#define CUSTOM_HID_EPOUT_SIZE         0x20U   // 32 bytes (hex)
```

3. **src/usbd_customhid_if.h** - Interface buffer sizes:
```c
#define RAWHID_TX_SIZE     32
#define RAWHID_RX_SIZE     32
```

4. **src/usbd_customhid_if.c** - Report descriptor:
```c
static uint8_t CustomHID_ReportDesc[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined)
    0x09, 0x01,        // Usage (Vendor Usage 1)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x20,        //   Report Count (32) <-- CHANGE THIS
    0x09, 0x01,        //   Usage (Vendor Usage 1)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x20,        //   Report Count (32) <-- CHANGE THIS
    0x09, 0x01,        //   Usage (Vendor Usage 1)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0xC0               // End Collection
};
```

5. **src/RawHID.h** - Wrapper constants:
```c
#define RAWHID_TX_SIZE 32
#define RAWHID_RX_SIZE 32
```

### Polling Interval

The default polling interval is 5ms. To change it, edit `build_opt.h`:

```c
-DCUSTOM_HID_FS_BINTERVAL=0x01    // 1ms polling (fastest)
-DCUSTOM_HID_FS_BINTERVAL=0x0A    // 10ms polling
```

## API Reference

### RawHID Class

```cpp
#include "src/RawHID.h"

// Global instance
extern RawHID_ RawHID;
```

#### Methods

**`void begin()`**
Initialize the Raw HID USB device. Call once in `setup()`.

**`void end()`**
Shut down the Raw HID USB device.

**`int send(const void *data, int len)`**
Send a raw HID report to the host.
- `data`: Pointer to data buffer
- `len`: Number of bytes to send (max 64)
- Returns: Number of bytes sent, or 0 on error

**`int sendPadded(const void *data, int len)`**
Send a raw HID report, zero-padding to full 64 bytes.
- `data`: Pointer to data buffer
- `len`: Number of bytes of actual data
- Returns: 64 on success, 0 on error

**`int available()`**
Check if data has been received from host.
- Returns: Number of bytes available (64 if data ready, 0 if none)

**`int recv(void *data, int len)`**
Receive data from host (non-blocking).
- `data`: Buffer to store received data
- `len`: Maximum bytes to read
- Returns: Number of bytes received, or 0 if none available

**`int recv(void *data, int len, int timeout_ms)`**
Receive data from host with timeout.
- `data`: Buffer to store received data
- `len`: Maximum bytes to read
- `timeout_ms`: Timeout in milliseconds
- Returns: Number of bytes received, or 0 on timeout

**`void setRxHandler(void (*callback)(uint8_t *data, uint16_t len))`**
Set a callback function for received data.
- **Warning:** Callback is called from USB interrupt context. Keep it short and don't call `send()` from within.

### Example Usage

```cpp
#include "src/RawHID.h"

void setup() {
    RawHID.begin();
}

void loop() {
    // Send data to host
    uint8_t report[64] = {0};
    report[0] = 0x01;           // Message type
    report[1] = analogRead(A0); // Some data
    RawHID.send(report, 64);

    // Receive data from host
    if (RawHID.available()) {
        uint8_t buf[64];
        int len = RawHID.recv(buf, sizeof(buf));

        // Process received command
        switch (buf[0]) {
            case 0x10:
                // Handle command 0x10
                break;
            case 0x11:
                // Handle command 0x11
                break;
        }
    }

    delay(10);
}
```

## Host-Side Communication

On the host computer, you can communicate with this device using various libraries:

### Python (hidapi)
```python
import hid

# Find device
device = hid.device()
device.open(0x0483, 0x5750)  # VID, PID

# Send data (first byte is report ID, use 0 for single-report devices)
device.write([0x00] + [0x01, 0x02, 0x03] + [0x00] * 61)

# Receive data
data = device.read(64, timeout_ms=1000)
print(data)

device.close()
```

### Node.js (node-hid)
```javascript
const HID = require('node-hid');

const device = new HID.HID(0x0483, 0x5750);

// Send data
device.write([0x01, 0x02, 0x03, ...Array(61).fill(0)]);

// Receive data
device.on('data', (data) => {
    console.log('Received:', data);
});

// Or synchronous read
const data = device.readSync();
```

### C/C++ (hidapi)
```c
#include <hidapi/hidapi.h>

hid_device *handle = hid_open(0x0483, 0x5750, NULL);

// Send
unsigned char buf[65] = {0x00, 0x01, 0x02, 0x03};  // Report ID + data
hid_write(handle, buf, 65);

// Receive
unsigned char rbuf[64];
int res = hid_read_timeout(handle, rbuf, 64, 1000);

hid_close(handle);
```

## Troubleshooting

### Device not recognized
- Check that `USBCON` and `HAL_PCD_MODULE_ENABLED` are defined in `build_opt.h`
- Verify the build uses `usb=none` in the FQBN
- Check USB cable (some cables are charge-only)

### Send returns 0 / USBD_BUSY
- The previous transfer hasn't completed yet
- Add a small delay between sends, or check return value and retry

### Receive doesn't work
- Ensure the report descriptor includes an Output report
- On some OSes, you may need to send a report with Report ID 0 as the first byte


## References

- [USB HID Specification](https://www.usb.org/hid)
- [HID Report Descriptor Tutorial](https://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/)
- [STM32 USB Device Library Documentation](https://www.st.com/resource/en/user_manual/um1734-stm32cube-usb-device-library-stmicroelectronics.pdf)
- [STM32duino GitHub](https://github.com/stm32duino/Arduino_Core_STM32)
