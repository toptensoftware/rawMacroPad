/**
  ******************************************************************************
  * @file    RawHID.h
  * @brief   Arduino-style Raw HID interface for STM32
  ******************************************************************************
  */

#ifndef _RAWHID_H_
#define _RAWHID_H_

#include <Arduino.h>

// Pull in USBDevice library for USB core functions
#include <usbd_if.h>

#ifdef USBD_USE_CUSTOM_HID

#define RAWHID_TX_SIZE 64
#define RAWHID_RX_SIZE 64

class RawHID_ {
public:
    RawHID_();

    /**
     * Initialize Raw HID USB device
     */
    void begin(void);

    /**
     * Shut down Raw HID USB device
     */
    void end(void);

    /**
     * Send a raw HID report to the host
     * @param data pointer to data buffer
     * @param len number of bytes to send (max 64)
     * @return number of bytes sent, or 0 on error
     */
    int send(const void *data, int len);

    /**
     * Send a raw HID report, padding to full 64 bytes
     * @param data pointer to data buffer
     * @param len number of bytes of actual data
     * @return number of bytes sent, or 0 on error
     */
    int sendPadded(const void *data, int len);

    /**
     * Check if data has been received from host
     * @return number of bytes available (0 if none)
     */
    int available(void);

    /**
     * Receive data from host (non-blocking)
     * @param data buffer to store received data
     * @param len maximum bytes to read
     * @return number of bytes received, or 0 if none available
     */
    int recv(void *data, int len);

    /**
     * Receive data from host with timeout
     * @param data buffer to store received data
     * @param len maximum bytes to read
     * @param timeout_ms timeout in milliseconds
     * @return number of bytes received, or 0 on timeout
     */
    int recv(void *data, int len, int timeout_ms);

    /**
     * Set a callback function for received data
     * Note: Callback is called from USB interrupt context!
     * Keep it short and don't call send() from within.
     * @param callback function pointer, or nullptr to disable
     */
    void setRxHandler(void (*callback)(uint8_t *data, uint16_t len));

private:
    bool _initialized;
};

extern RawHID_ RawHID;

#endif /* USBD_USE_CUSTOM_HID */
#endif /* _RAWHID_H_ */
