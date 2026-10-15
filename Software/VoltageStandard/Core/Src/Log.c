#include "Log.h"
#include <stdarg.h>
#include <stdio.h>
#include "usb_device.h"
#include <stdint.h>
#include "usbd_cdc_if.h"
#define BUF_LEN 128
#define USB_TIMEOUT 10
extern USBD_HandleTypeDef hUsbDeviceFS;
extern volatile uint8_t usb_Connected;

uint8_t USB_printf(const char *fmt, ...) {

    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED || !usb_Connected) {
        return 0;
    }

    


    // Prepare buffer
    char TxBuffer[BUF_LEN];

    va_list args;
    va_start(args, fmt);

    int TxBufferLen = vsnprintf((char*)TxBuffer, BUF_LEN, fmt, args);
    va_end(args);

    if (TxBufferLen <= 0) {
        return 0;
    }


    // Transmit

    uint32_t TickStart = HAL_GetTick();

    while(CDC_Transmit_FS((uint8_t*)TxBuffer, TxBufferLen) == USBD_BUSY) {
        if ((HAL_GetTick() - TickStart) > USB_TIMEOUT) {
            return 0;
        }
    }
    return TxBufferLen;
}