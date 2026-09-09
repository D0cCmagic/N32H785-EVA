#ifndef __APP_OTA_HANDLE_H__
#define __APP_OTA_HANDLE_H__
#include "stdint.h"

/* APP 触发 OTA 升级：串口命令 "UPDATE"（须与 Bootloader 侧 IAP.c 保持一致） */
#define OTA_FLAG_ADDR    (0x2401FF00u)   /* AXI SRAM 尾部约定地址（scatter 已预留） */
#define OTA_FLAG_MAGIC   (0x4F544100u)   /* "OTA\0" */
#define OTA_CMD_STRING   "UPDATE"

extern volatile uint8_t ota_request_flag;
uint8_t ota_cmd_feed(uint8_t ch);
void ota_request_reset(void);
void OTA_UpdateCheck_Handle(void);
#endif
