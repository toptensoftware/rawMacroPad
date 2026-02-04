/**
  ******************************************************************************
  * @file    RawHID.cpp
  * @brief   Arduino-style Raw HID implementation for STM32
  ******************************************************************************
  */

#include "RawHID.h"

#ifdef USBD_USE_CUSTOM_HID

extern "C" {
#include "usbd_customhid_if.h"
}

RawHID_::RawHID_() : _initialized(false) {
}

void RawHID_::begin(void) {
    if (!_initialized) {
        CustomHID_Init();
        _initialized = true;
    }
}

void RawHID_::end(void) {
    if (_initialized) {
        CustomHID_DeInit();
        _initialized = false;
    }
}

int RawHID_::send(const void *data, int len) {
    if (!_initialized || data == nullptr || len <= 0) {
        return 0;
    }

    if (len > RAWHID_TX_SIZE) {
        len = RAWHID_TX_SIZE;
    }

    uint8_t result = CustomHID_SendReport((uint8_t *)data, len);
    return (result == 0) ? len : 0;  // USBD_OK is 0
}

int RawHID_::sendPadded(const void *data, int len) {
    if (!_initialized || data == nullptr || len < 0) {
        return 0;
    }

    uint8_t buffer[RAWHID_TX_SIZE] = {0};

    if (len > RAWHID_TX_SIZE) {
        len = RAWHID_TX_SIZE;
    }

    if (len > 0) {
        memcpy(buffer, data, len);
    }

    uint8_t result = CustomHID_SendReport(buffer, RAWHID_TX_SIZE);
    return (result == 0) ? RAWHID_TX_SIZE : 0;
}

int RawHID_::available(void) {
    if (!_initialized) {
        return 0;
    }
    return CustomHID_Available() ? RAWHID_RX_SIZE : 0;
}

int RawHID_::recv(void *data, int len) {
    if (!_initialized || data == nullptr || len <= 0) {
        return 0;
    }

    return CustomHID_Receive((uint8_t *)data, len);
}

int RawHID_::recv(void *data, int len, int timeout_ms) {
    if (!_initialized || data == nullptr || len <= 0) {
        return 0;
    }

    uint32_t start = millis();
    while ((millis() - start) < (uint32_t)timeout_ms) {
        if (CustomHID_Available()) {
            return CustomHID_Receive((uint8_t *)data, len);
        }
        delay(1);  // Small delay to avoid busy-waiting
    }

    return 0;  // Timeout
}

void RawHID_::setRxHandler(void (*callback)(uint8_t *data, uint16_t len)) {
    CustomHID_SetRxCallback(callback);
}

/* Global instance */
RawHID_ RawHID;

#endif /* USBD_USE_CUSTOM_HID */
