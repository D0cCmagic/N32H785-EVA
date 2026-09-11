#ifndef __APP_OTA_HANDLE_H__
#define __APP_OTA_HANDLE_H__
#include "stdint.h"

/* App side requests an OTA upgrade via the serial command "UPDATE".
 * Must stay in sync with the Bootloader side in IAP.c. */
#define OTA_FLAG_ADDR    (0x2401FF00u)   /* Agreed address at the end of AXI SRAM, reserved in scatter */
#define OTA_FLAG_MAGIC   (0x4F544100u)   /* "OTA\0" */
#define OTA_CMD_STRING   "UPDATE"

extern volatile uint8_t ota_request_flag;
uint8_t ota_cmd_feed(uint8_t ch);
void ota_request_reset(void);
void OTA_UpdateCheck_Handle(void);
#endif
