/**
  ******************************************************************************
  * @file    usbd_customhid_if.h
  * @brief   Header for Custom HID interface
  ******************************************************************************
  */

#ifndef __USBD_CUSTOMHID_IF_H
#define __USBD_CUSTOMHID_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_customhid.h"

/* Raw HID report size (excluding report ID if used) */
#define RAWHID_TX_SIZE     64
#define RAWHID_RX_SIZE     64

/* Initialize Custom HID */
void CustomHID_Init(void);

/* Deinitialize Custom HID */
void CustomHID_DeInit(void);

/* Send a report (blocking until previous send completes or timeout) */
uint8_t CustomHID_SendReport(uint8_t *report, uint16_t len);

/* Check if data is available to receive */
uint8_t CustomHID_Available(void);

/* Get received data (returns number of bytes, 0 if none available) */
uint8_t CustomHID_Receive(uint8_t *buf, uint16_t maxLen);

/* Set callback for received data (called from ISR context!) */
typedef void (*CustomHID_RxCallback_t)(uint8_t *data, uint16_t len);
void CustomHID_SetRxCallback(CustomHID_RxCallback_t callback);

/* Get the USB device handle (for advanced use) */
USBD_HandleTypeDef* CustomHID_GetDeviceHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CUSTOMHID_IF_H */
