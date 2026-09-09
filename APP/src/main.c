/**
*     Copyright (c) 2025, Nations Technologies Inc.
*
*     All rights reserved.
*
*     This software is the exclusive property of Nations Technologies Inc.
*/

/**
*\*\file main.c
*\*\brief  N32H785 + ETH + R900PNR (TXW8301 FMAC) 融合示例w
*\*\       同时加入 R900PNR SPI raw data 通信功能
*/

#include "main.h"
#include <stdio.h>
#include <string.h>
#include "r900pnr_spi_cfg.h"
#include "r900pnr_spi_api.h"
#include "hgic_raw.h"
#include "hgic_sdspi.h"
#include "app_r900pnr_handle.h"
#include "delay.h"
#include "App_OTA_Handle.h"    
#include "App_VectorTable.h"


__IO int ReceiveDataFlag = 0;
__IO uint32_t TimeBase = 0;
uint32_t sys_now(void)
{
    return TimeBase;
}
static void MPU_Config(void)
{
    MPU_Region_InitType MPU_InitStruct;

    MPU_Disable();
	
    MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

int main(void)
{
	
    App_VectorTableInit();   /* VTOR 指向 ITCM 镜像 + SysTick 槽位重定向（替代原裸 VTOR 赋值） */
    RCC_ClocksTypeDef clk;
    SCB_InvalidateDCache();
    SCB_InvalidateICache();
    RCC_SetSysClkToMode0();
    log_init();
    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();
    RCC_GetClocksFreqValue(&clk);
    SysTick_CLKSourceConfig(SysTick_CLKSource_CORECLK);
    SysTick_Config(clk.M7ClkFreq / 1000);
    NVIC_SetPriority(SysTick_IRQn, 1);

#if (APP_WIFI_ROLE == APP_ROLE_AP)
    log_info("[ROLE] AP receiver (R900PNR rate test)\r\n");
#else
    log_info("[ROLE] STA sender (R900PNR rate test)\r\n");
#endif
		if(0 != r900pnr_wifi_module_init())
		log_error("Wi-Fi module init failed");
     App_WiFiConfigure();

    while (1)
    {
			/* R900PNR 处理 */
			App_R900PnrPoll();
			OTA_UpdateCheck_Handle();
    }
}
