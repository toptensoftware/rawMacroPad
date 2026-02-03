/**
  ******************************************************************************
  * @file    usbd_customhid_if.c
  * @brief   Custom HID interface implementation for Raw HID
  ******************************************************************************
  */

#ifdef USBCON
#ifdef USBD_USE_CUSTOM_HID

#include <stdbool.h>
#include <string.h>
#include "usbd_ep_conf.h"
#include "usbd_customhid_if.h"
#include "usbd_desc.h"

/* USB Device handle */
static USBD_HandleTypeDef hUSBD_Device_CustomHID;

/* Receive buffer and state */
static volatile bool rxDataAvailable = false;
static uint8_t rxBuffer[RAWHID_RX_SIZE];
static volatile uint16_t rxLen = 0;

/* User callback */
static CustomHID_RxCallback_t userRxCallback = NULL;

/* Forward declarations for interface callbacks */
static int8_t CustomHID_Itf_Init(void);
static int8_t CustomHID_Itf_DeInit(void);
static int8_t CustomHID_Itf_OutEvent(uint8_t event_idx, uint8_t state);

/* Raw HID Report Descriptor - Generic 64-byte IN/OUT */
__ALIGN_BEGIN static uint8_t CustomHID_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        // Usage (Vendor Usage 1)
    0xA1, 0x01,        // Collection (Application)

    // Input Report (Device to Host)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x40,        //   Report Count (64)
    0x09, 0x01,        //   Usage (Vendor Usage 1)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    // Output Report (Host to Device)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x40,        //   Report Count (64)
    0x09, 0x01,        //   Usage (Vendor Usage 1)
    0x91, 0x02,        //   Output (Data,Var,Abs)

    0xC0               // End Collection
};

/* Interface callbacks structure */
static USBD_CUSTOM_HID_ItfTypeDef CustomHID_fops = {
    CustomHID_ReportDesc,
    CustomHID_Itf_Init,
    CustomHID_Itf_DeInit,
    CustomHID_Itf_OutEvent,
};

/**
  * @brief  Initialize the Custom HID interface
  */
static int8_t CustomHID_Itf_Init(void)
{
    rxDataAvailable = false;
    rxLen = 0;
    return USBD_OK;
}

/**
  * @brief  DeInitialize the Custom HID interface
  */
static int8_t CustomHID_Itf_DeInit(void)
{
    return USBD_OK;
}

/**
  * @brief  Called when data is received from host
  * @note   This is called from USB interrupt context!
  *         The parameters event_idx and state are only the first 2 bytes.
  *         For full data, we need to access the report buffer directly.
  */
static int8_t CustomHID_Itf_OutEvent(uint8_t event_idx, uint8_t state)
{
    USBD_CUSTOM_HID_HandleTypeDef *hhid =
        (USBD_CUSTOM_HID_HandleTypeDef *)hUSBD_Device_CustomHID.pClassDataCmsit[hUSBD_Device_CustomHID.classId];

    if (hhid != NULL) {
        /* Copy the full report buffer */
        memcpy((void*)rxBuffer, hhid->Report_buf, RAWHID_RX_SIZE);
        rxLen = RAWHID_RX_SIZE;
        rxDataAvailable = true;

        /* Call user callback if registered */
        if (userRxCallback != NULL) {
            userRxCallback((uint8_t*)rxBuffer, rxLen);
        }
    }

    /* Prepare for next reception */
    USBD_CUSTOM_HID_ReceivePacket(&hUSBD_Device_CustomHID);

    return USBD_OK;
}

/**
  * @brief  Initialize Custom HID USB device
  */
void CustomHID_Init(void)
{
    /* Init Device Library */
    if (USBD_Init(&hUSBD_Device_CustomHID, &USBD_Desc, 0) == USBD_OK) {
        /* Register the Custom HID interface */
        if (USBD_CUSTOM_HID_RegisterInterface(&hUSBD_Device_CustomHID, &CustomHID_fops) == USBD_OK) {
            /* Add Supported Class */
            if (USBD_RegisterClass(&hUSBD_Device_CustomHID, USBD_CUSTOM_HID_CLASS) == USBD_OK) {
                /* Start Device Process */
                USBD_Start(&hUSBD_Device_CustomHID);
            }
        }
    }
}

/**
  * @brief  DeInitialize Custom HID USB device
  */
void CustomHID_DeInit(void)
{
    USBD_Stop(&hUSBD_Device_CustomHID);
    USBD_DeInit(&hUSBD_Device_CustomHID);
}

/**
  * @brief  Send a Custom HID report
  * @param  report: pointer to report data
  * @param  len: report length (max 64 bytes)
  * @retval USBD status
  */
uint8_t CustomHID_SendReport(uint8_t *report, uint16_t len)
{
    return USBD_CUSTOM_HID_SendReport(&hUSBD_Device_CustomHID, report, len);
}

/**
  * @brief  Check if received data is available
  * @retval 1 if data available, 0 otherwise
  */
uint8_t CustomHID_Available(void)
{
    return rxDataAvailable ? 1 : 0;
}

/**
  * @brief  Get received data
  * @param  buf: buffer to store data
  * @param  maxLen: maximum bytes to read
  * @retval number of bytes copied, 0 if no data
  */
uint8_t CustomHID_Receive(uint8_t *buf, uint16_t maxLen)
{
    if (!rxDataAvailable) {
        return 0;
    }

    uint16_t copyLen = (rxLen < maxLen) ? rxLen : maxLen;
    memcpy(buf, (void*)rxBuffer, copyLen);
    rxDataAvailable = false;

    return copyLen;
}

/**
  * @brief  Set callback for received data
  * @param  callback: function to call when data received (ISR context!)
  */
void CustomHID_SetRxCallback(CustomHID_RxCallback_t callback)
{
    userRxCallback = callback;
}

/**
  * @brief  Get USB device handle for advanced operations
  */
USBD_HandleTypeDef* CustomHID_GetDeviceHandle(void)
{
    return &hUSBD_Device_CustomHID;
}

#endif /* USBD_USE_CUSTOM_HID */
#endif /* USBCON */
