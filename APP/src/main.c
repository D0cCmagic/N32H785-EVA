/**
*     Copyright (c) 2025, Nations Technologies Inc.
*
*     All rights reserved.
*
*     This software is the exclusive property of Nations Technologies Inc.
*/

/**
*\*\file main.c
*\*\brief  N32H785 + ETH + R900PNR (TXW8301 FMAC) combined example
*\*\        also adds R900PNR SPI raw data communication
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
#include "Mcal_InitSum.h"

__IO int ReceiveDataFlag = 0;
__IO uint32_t TimeBase = 0;
/**
 * @name    sys_now
 * @brief   Return the 1ms system timebase.
 * @param   None
 * @retval  Milliseconds elapsed since TimeBase was reset.
 */
uint32_t sys_now(void)
{
    return TimeBase;
}

/**
 * @name    MPU_Config
 * @brief   Configure the MPU with the privileged default memory map.
 * @param   None
 * @retval  None
 */
static void MPU_Config(void)
{
    MPU_Region_InitType MPU_InitStruct;

    MPU_Disable();
	
    MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}




/**
 * @name    main
 * @brief   Bring up clocks, caches and peripherals, then run the main loop.
 * @param   None
 * @retval  Never returns.
 */
int main(void)
{
	
    App_VectorTableInit();   /* Point VTOR at the ITCM image and remap the SysTick slot */
    RCC_ClocksTypeDef clk;
    SCB_InvalidateDCache();
    SCB_InvalidateICache();
    RCC_SetSysClkToMode0();
    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();
    RCC_GetClocksFreqValue(&clk);
    SysTick_CLKSourceConfig(SysTick_CLKSource_CORECLK);
    SysTick_Config(clk.M7ClkFreq / 1000);
    NVIC_SetPriority(SysTick_IRQn, 1);
		
	
		Mcal_InitSum();
	
	
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
			App_R900PnrPoll();     
			Task_Handle_Cycle();   
    }
}
