#include "App_OTA_Handle.h"
#include <stdint.h>
#include "stdio.h"
#include "misc.h"
volatile uint8_t ota_request_flag = 0;

uint8_t ota_cmd_feed(uint8_t ch)
{
    static const char cmd[] = OTA_CMD_STRING;
    static uint8_t match_idx = 0;

    if (match_idx >= (sizeof(cmd) - 1u))
    {
        if ((ch == '\r') || (ch == '\n'))
        {
            match_idx = 0;
            return 1;
        }
        match_idx = (ch == (uint8_t)cmd[0]) ? 1u : 0u;
        return 0;
    }

    if (ch == (uint8_t)cmd[match_idx])
    {
        match_idx++;
    }
    else
    {
        match_idx = (ch == (uint8_t)cmd[0]) ? 1u : 0u;
    }
    return 0;
}

void ota_request_reset(void)
{
    printf("\r\n OTA update request received, reboot to bootloader...\r\n");
    *(volatile uint32_t *)OTA_FLAG_ADDR = OTA_FLAG_MAGIC;
    SCB_CleanDCache();  
    __set_FAULTMASK(1);
    __NVIC_SystemReset();
}


void OTA_UpdateCheck_Handle(void)
{
	if (ota_request_flag) { ota_request_reset(); }
}
