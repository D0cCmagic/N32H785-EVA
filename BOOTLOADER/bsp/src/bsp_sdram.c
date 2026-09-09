/**
*     Copyright (c) 2025, Nations Technologies Inc.
* 
*     All rights reserved.
*
*     This software is the exclusive property of Nations Technologies Inc. (Hereinafter 
* referred to as NATIONS). This software, and the product of NATIONS described herein 
* (Hereinafter referred to as the Product) are owned by NATIONS under the laws and treaties
* of the People's Republic of China and other applicable jurisdictions worldwide.
*
*     NATIONS does not grant any license under its patents, copyrights, trademarks, or other 
* intellectual property rights. Names and brands of third party may be mentioned or referred 
* thereto (if any) for identification purposes only.
*
*     NATIONS reserves the right to make changes, corrections, enhancements, modifications, and 
* improvements to this software at any time without notice. Please contact NATIONS and obtain 
* the latest version of this software before placing orders.

*     Although NATIONS has attempted to provide accurate and reliable information, NATIONS assumes 
* no responsibility for the accuracy and reliability of this software.
* 
*     It is the responsibility of the user of this software to properly design, program, and test 
* the functionality and safety of any application made of this information and any resulting product. 
* In no event shall NATIONS be liable for any direct, indirect, incidental, special,exemplary, or 
* consequential damages arising in any way out of the use of this software or the Product.
*
*     NATIONS Products are neither intended nor warranted for usage in systems or equipment, any
* malfunction or failure of which may cause loss of human life, bodily injury or severe property 
* damage. Such applications are deemed, "Insecure Usage".
*
*     All Insecure Usage shall be made at user's risk. User shall indemnify NATIONS and hold NATIONS 
* harmless from and against all claims, costs, damages, and other liabilities, arising from or related 
* to any customer's Insecure Usage.

*     Any express or implied warranty with regard to this software or the Product, including,but not 
* limited to, the warranties of merchantability, fitness for a particular purpose and non-infringement
* are disclaimed to the fullest extent permitted by law.

*     Unless otherwise explicitly permitted by NATIONS, anyone may not duplicate, modify, transcribe
* or otherwise distribute this software for any purposes, in whole or in part.
*
*     NATIONS products and technologies shall not be used for or incorporated into any products or systems
* whose manufacture, use, or sale is prohibited under any applicable domestic or foreign laws or regulations. 
* User shall comply with any applicable export control laws and regulations promulgated and administered by 
* the governments of any countries asserting jurisdiction over the parties or transactions.
**/

/**
*\*\file bsp_sdram.c
*\*\author Nations
*\*\version v1.0.0
*\*\copyright Copyright (c) 2025, Nations Technologies Inc. All rights reserved.
**/

#include "n32h7xx_sdram.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_rcc.h"
#include "log.h"
#include "delay.h"
#include "bsp_sdram.h"

/**
*\*\name    SDRAM_RCC_Configuration.
*\*\fun     Configures the peripheral clocks.
*\*\param   none
*\*\return  none
**/
void SDRAM_RCC_Configuration(void)
{
    /* Enable GPIO clock*/
    RCC_EnableAHB5PeriphClk1(RCC_AHB5_PERIPHEN_M7_GPIOA|RCC_AHB5_PERIPHEN_M7_GPIOB|RCC_AHB5_PERIPHEN_M7_GPIOC|RCC_AHB5_PERIPHEN_M7_GPIOD,ENABLE);
    RCC_EnableAHB5PeriphClk1(RCC_AHB5_PERIPHEN_M7_GPIOE|RCC_AHB5_PERIPHEN_M7_GPIOF|RCC_AHB5_PERIPHEN_M7_GPIOG|RCC_AHB5_PERIPHEN_M7_GPIOH,ENABLE);
    RCC_EnableAHB5PeriphClk2(RCC_AHB5_PERIPHEN_M7_GPIOI|RCC_AHB5_PERIPHEN_M7_AFIO,ENABLE);
    /* Enable HSE */
    RCC_ConfigHse(RCC_HSE_ENABLE);
    /* Wait HSE Stable */
    RCC_WaitHseStable();
    /* Config PLL3 is 665M */
    RCC_ConfigPll3(RCC_PLL_SRC_HSE,25000000,665000000,ENABLE);
    /* Config PLL3A is 133M */
    RCC_ConfigPLL3ADivider(RCC_PLLA_DIV5);
    /* select PLL3A is SDRAM memory clock = 133M */
    RCC_ConfigSDRAMMemClk(RCC_SDRAMMEMCLK_SRC_PLL3A, RCC_SDRAMMEMCLK_AXIDIV1);
    /* Enable SDRAM clock*/
    RCC_EnableAXIPeriphClk4(RCC_AXI_PERIPHEN_M7_SDRAM | RCC_AXI_PERIPHEN_M7_SDRAMLP, ENABLE);
    /* Enable SDRAM clock delay*/
    RCC_ConfigSDRAMDelay(RCC_SDRAM_DELAY_0_2NS);
    RCC_EnableSDRAMDelayChain(ENABLE);
}

/**
*\*\name    SDRAM_GPIO_Init.
*\*\fun     SDRAM gpio initialized.
*\*\param   none
*\*\return  none 
**/
void SDRAM_GPIO_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    
    /*-- GPIO Configuration -----------------------*/
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Slew_Rate = GPIO_SLEW_RATE_FAST;
    GPIO_InitStructure.GPIO_Current      = GPIO_DC_2mA;
    GPIO_InitStructure.GPIO_Pull      = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
    
    /* ADD pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_A0_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A0_AF;
    GPIO_InitPeripheral(SDRAM_A0_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A0_PORT,SDRAM_A0_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A0,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A0,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A0,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A1_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A1_AF;
    GPIO_InitPeripheral(SDRAM_A1_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A1_PORT,SDRAM_A1_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A1,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A1,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A1,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A2_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A2_AF;
    GPIO_InitPeripheral(SDRAM_A2_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A2_PORT,SDRAM_A2_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A2,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A2,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A2,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A3_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A3_AF;
    GPIO_InitPeripheral(SDRAM_A3_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A3_PORT,SDRAM_A3_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A3,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A3,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A3,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A4_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A4_AF;
    GPIO_InitPeripheral(SDRAM_A4_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A4_PORT,SDRAM_A4_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A4,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A4,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A4,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A5_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A5_AF;
    GPIO_InitPeripheral(SDRAM_A5_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A5_PORT,SDRAM_A5_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A5,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A5,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A5,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A6_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A6_AF;
    GPIO_InitPeripheral(SDRAM_A6_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A6_PORT,SDRAM_A6_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A6,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A6,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A6,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A7_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A7_AF;
    GPIO_InitPeripheral(SDRAM_A7_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A7_PORT,SDRAM_A7_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A7,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A7,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A7,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A8_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A8_AF;
    GPIO_InitPeripheral(SDRAM_A8_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A8_PORT,SDRAM_A8_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A8,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A8,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A8,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A9_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A9_AF;
    GPIO_InitPeripheral(SDRAM_A9_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A9_PORT,SDRAM_A9_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A9,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A9,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A9,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A10_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A10_AF;
    GPIO_InitPeripheral(SDRAM_A10_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A10_PORT,SDRAM_A10_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A10,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A10	,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A10	,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_A11_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A11_AF;
    GPIO_InitPeripheral(SDRAM_A11_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A11_PORT,SDRAM_A11_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A11,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A11	,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A11	,ENABLE);
    #endif
    
#if SDRAM_DEVICE == SDRAM_W9825G6KH
    GPIO_InitStructure.Pin            = SDRAM_A12_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_A12_AF;
    GPIO_InitPeripheral(SDRAM_A12_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_A12_PORT,SDRAM_A12_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_A12,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_A12	,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_A12	,ENABLE);
    #endif
#endif
 
    /*  DATA pin configuration  */
    GPIO_InitStructure.Pin            = SDRAM_D0_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D0_AF;
    GPIO_InitPeripheral(SDRAM_D0_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D0_PORT,SDRAM_D0_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D0,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D0,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D0,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D1_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D1_AF;
    GPIO_InitPeripheral(SDRAM_D1_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D1_PORT,SDRAM_D1_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D1,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D1,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D1,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D2_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D2_AF;
    GPIO_InitPeripheral(SDRAM_D2_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D2_PORT,SDRAM_D2_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D2,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D2,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D2,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D3_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D3_AF;
    GPIO_InitPeripheral(SDRAM_D3_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D3_PORT,SDRAM_D3_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D3,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D3,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D3,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D4_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D4_AF;
    GPIO_InitPeripheral(SDRAM_D4_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D4_PORT,SDRAM_D4_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D4,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D4,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D4,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D5_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D5_AF;
    GPIO_InitPeripheral(SDRAM_D5_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D5_PORT,SDRAM_D5_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D5,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D5,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D5,ENABLE);
    #endif
    
    
    GPIO_InitStructure.Pin            = SDRAM_D6_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D6_AF;
    GPIO_InitPeripheral(SDRAM_D6_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D6_PORT,SDRAM_D6_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D6,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D6,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D6,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D7_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D7_AF;
    GPIO_InitPeripheral(SDRAM_D7_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D7_PORT,SDRAM_D7_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D7,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D7,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D7,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D8_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D8_AF;
    GPIO_InitPeripheral(SDRAM_D8_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D8_PORT,SDRAM_D8_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D8,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D8,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D8,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D9_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D9_AF;
    GPIO_InitPeripheral(SDRAM_D9_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D9_PORT,SDRAM_D9_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D9,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D9,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D9,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D10_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D10_AF;
    GPIO_InitPeripheral(SDRAM_D10_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D10_PORT,SDRAM_D10_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D10,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D10,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D10,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D11_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D11_AF;
    GPIO_InitPeripheral(SDRAM_D11_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D11_PORT,SDRAM_D11_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D11,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D11,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D11,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D12_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D12_AF;
    GPIO_InitPeripheral(SDRAM_D12_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D12_PORT,SDRAM_D12_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D12,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D12,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D12,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D13_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D13_AF;
    GPIO_InitPeripheral(SDRAM_D13_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D13_PORT,SDRAM_D13_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D13,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D13,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D13,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D14_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D14_AF;
    GPIO_InitPeripheral(SDRAM_D14_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D14_PORT,SDRAM_D14_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D14,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D14,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D14,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_D15_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D15_AF;
    GPIO_InitPeripheral(SDRAM_D15_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D15_PORT,SDRAM_D15_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D15,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D15,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D15,ENABLE);
    #endif

#if (SDRAM_DEVICE == SDRAM_M12L128324A) || (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_IS42S32800J_EVB)
    GPIO_InitStructure.Pin            = SDRAM_D16_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D16_AF;
    GPIO_InitPeripheral(SDRAM_D16_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D16_PORT,SDRAM_D16_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D16,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D16,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D16,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D17_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D17_AF;
    GPIO_InitPeripheral(SDRAM_D17_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D17_PORT,SDRAM_D17_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D17,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D17,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D17,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D18_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D18_AF;
    GPIO_InitPeripheral(SDRAM_D18_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D18_PORT,SDRAM_D18_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D18,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D18,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D18,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D19_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D19_AF;
    GPIO_InitPeripheral(SDRAM_D19_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D19_PORT,SDRAM_D19_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D19,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D19,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D19,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D20_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D20_AF;
    GPIO_InitPeripheral(SDRAM_D20_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D20_PORT,SDRAM_D20_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D20,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D20,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D20,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D21_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D21_AF;
    GPIO_InitPeripheral(SDRAM_D21_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D21_PORT,SDRAM_D21_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D21,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D21,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D21,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D22_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D22_AF;
    GPIO_InitPeripheral(SDRAM_D22_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D22_PORT,SDRAM_D22_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D22,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D22,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D22,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D23_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D23_AF;
    GPIO_InitPeripheral(SDRAM_D23_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D23_PORT,SDRAM_D23_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D23,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D23,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D23,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D24_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D24_AF;
    GPIO_InitPeripheral(SDRAM_D24_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D24_PORT,SDRAM_D24_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D24,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D24,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D24,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D25_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D25_AF;
    GPIO_InitPeripheral(SDRAM_D25_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D25_PORT,SDRAM_D25_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D25,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D25,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D25,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D26_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D26_AF;
    GPIO_InitPeripheral(SDRAM_D26_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D26_PORT,SDRAM_D26_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D26,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D26,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D26,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D27_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D27_AF;
    GPIO_InitPeripheral(SDRAM_D27_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D27_PORT,SDRAM_D27_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D27,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D27,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D27,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D28_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D28_AF;
    GPIO_InitPeripheral(SDRAM_D28_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D28_PORT,SDRAM_D28_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D28,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D28,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D28,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D29_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D29_AF;
    GPIO_InitPeripheral(SDRAM_D29_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D29_PORT,SDRAM_D29_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D29,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D29,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D29,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D30_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D30_AF;
    GPIO_InitPeripheral(SDRAM_D30_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D30_PORT,SDRAM_D30_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D30,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D30,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D30,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_D31_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_D31_AF;
    GPIO_InitPeripheral(SDRAM_D31_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_D31_PORT,SDRAM_D31_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_D31,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_D31,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_D31,ENABLE);
    #endif
#endif

    /* BA signal pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_BA0_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_BA0_AF;
    GPIO_InitPeripheral(SDRAM_BA0_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_BA0_PORT,SDRAM_BA0_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_BA0,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_BA0,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_BA0,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_BA1_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_BA1_AF;
    GPIO_InitPeripheral(SDRAM_BA1_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_BA1_PORT,SDRAM_BA1_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_BA1,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_BA1,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_BA1,ENABLE);
    #endif

#if (SDRAM_DEVICE == SDRAM_M12L128324A) 
    /* NCE signal pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_NCE1_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_NCE1_AF;
    GPIO_InitPeripheral(SDRAM_NCE1_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_NCE1_PORT,SDRAM_NCE1_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_NCE1,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_NCE1,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_NCE1,ENABLE);
    #endif
#else
    /* NCE signal pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_NCE0_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_NCE0_AF;
    GPIO_InitPeripheral(SDRAM_NCE0_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_NCE0_PORT,SDRAM_NCE0_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_NCE0,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_NCE0,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_NCE0,ENABLE);
    #endif
#endif
    
    /* NWE pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_NWE_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_NWE_AF;
    GPIO_InitPeripheral(SDRAM_NWE_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_NWE_PORT,SDRAM_NWE_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_NWE,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_NWE,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_NWE,ENABLE);
    #endif
    
    /* NRAS pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_NRAS_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_NRAS_AF;
    GPIO_InitPeripheral(SDRAM_NRAS_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_NRAS_PORT,SDRAM_NRAS_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_NRAS,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_NRAS,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_NRAS,ENABLE);
    #endif
    
    /* NCAS pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_NCAS_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_NCAS_AF;
    GPIO_InitPeripheral(SDRAM_NCAS_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_NCAS_PORT,SDRAM_NCAS_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_NCAS,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_NCAS,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_NCAS,ENABLE);
    #endif

    /* DQM signal pin configuration */
    GPIO_InitStructure.Pin            = SDRAM_DQM0_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_DQM0_AF;
    GPIO_InitPeripheral(SDRAM_DQM0_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_DQM0_PORT,SDRAM_DQM0_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_DQM0,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_DQM0,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_DQM0,ENABLE);
    #endif
    
    GPIO_InitStructure.Pin            = SDRAM_DQM1_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_DQM1_AF;
    GPIO_InitPeripheral(SDRAM_DQM1_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_DQM1_PORT,SDRAM_DQM1_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_DQM1,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_DQM1,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_DQM1,ENABLE);
    #endif

#if (SDRAM_DEVICE == SDRAM_M12L128324A) || (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_IS42S32800J_EVB)
    GPIO_InitStructure.Pin            = SDRAM_DQM2_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_DQM2_AF;
    GPIO_InitPeripheral(SDRAM_DQM2_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_DQM2_PORT,SDRAM_DQM2_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_DQM2,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_DQM2,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_DQM2,ENABLE);
    #endif

    GPIO_InitStructure.Pin            = SDRAM_DQM3_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_DQM3_AF;
    GPIO_InitPeripheral(SDRAM_DQM3_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_DQM3_PORT,SDRAM_DQM3_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_DQM3,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_DQM3,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_DQM3,ENABLE);
    #endif
#endif

#if (SDRAM_DEVICE == SDRAM_M12L128324A)
    /* CKE signal pin configuration*/
    GPIO_InitStructure.Pin            = SDRAM_CKE1_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_CKE1_AF;
    GPIO_InitPeripheral(SDRAM_CKE1_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_CKE1_PORT,SDRAM_CKE1_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_CKE1,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_CKE1,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_CKE1,ENABLE);
    #endif
#else
    /* CKE signal pin configuration*/
    GPIO_InitStructure.Pin            = SDRAM_CKE0_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_CKE0_AF;
    GPIO_InitPeripheral(SDRAM_CKE0_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_CKE0_PORT,SDRAM_CKE0_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_CKE0,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_CKE0,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_CKE0,ENABLE);
    #endif
#endif

    /* CLK pin configuration*/
    GPIO_InitStructure.Pin            = SDRAM_CLK_PIN;
    GPIO_InitStructure.GPIO_Alternate = SDRAM_CLK_AF;
//    GPIO_InitStructure.GPIO_Current      = GPIO_DC_4mA;
    GPIO_InitPeripheral(SDRAM_CLK_PORT, &GPIO_InitStructure);
    #ifdef SDRAM_AFIO_DRIVER
    AFIO_ConfigHSMODE(SDRAM_CLK_PORT,SDRAM_CLK_PIN,ENABLE);
    AFIO_ConfigHSMODEVREFRemap(AFIO_SDRAM_CLK,ENABLE);
    AFIO_ConfigHSPortDSNRemap(AFIO_SDRAM_CLK,ENABLE);
    AFIO_ConfigHSPortDSPRemap(AFIO_SDRAM_CLK,ENABLE);
    #endif
}

/**
*\*\name    SDRAM_DeviceInit.
*\*\fun     initialize the one or twe SDRAM device.    
*\*\param   none
*\*\return  none 
**/
void SDRAM_DeviceInit()
{
    SDRAM_TimingType SDRAM_Timing;
    SDRAM_OperationInitType SDRAM_Operation;
    SDRAM_ConfigurationInitType SDRAM_Configuration;
   
#if SDRAM_DEVICE == SDRAM_W9825G6KH
     /* Set SDRAM address: BaseAddr = 0xC0000000, AddrMask = 0xFFFFFFFF-(SDRAMSIZI-1)*/
    SDRAM_SetDeviceAddress(SDRAM_DEVICE_1, SDRAM1_ADDR, 0xFE000000);//AddrMask=0xFFFFFFFF-(32Mbyte-1)
    /* Set SDRAM Refresh Interval=(Refresh Period / Number of Rows / Mem clock Period)*/
    SDRAM_RefreshIntervalInit(0x300);//64000000ns/8192rows/(1/133M=8)ns =0x3D0,whichever is smaller
#elif (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_IS42S32800J_EVB)
    /* Set SDRAM address: BaseAddr = 0xC0000000, AddrMask = 0xFFFFFFFF-(SDRAMSIZI-1)*/
    SDRAM_SetDeviceAddress(SDRAM_DEVICE_1, SDRAM1_ADDR, 0xFE000000);//AddrMask=0xFFFFFFFF-(32Mbyte-1)
    /* Set SDRAM Refresh Interval=(Refresh Period / Number of Rows / Mem clock Period)*/
    SDRAM_RefreshIntervalInit(0x750);//64000000ns/4096rows/(1/133M=8)ns =0x7A1,whichever is smaller
#elif (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)  
    /* Set SDRAM address: BaseAddr = 0xC0000000, AddrMask = 0xFFFFFFFF-(SDRAMSIZI-1)*/
    SDRAM_SetDeviceAddress(SDRAM_DEVICE_1, SDRAM1_ADDR, 0xFF000000);//AddrMask=0xFFFFFFFF-(16Mbyte-1)
    /* Set SDRAM Refresh Interval=(Refresh Period / Number of Rows / Mem clock Period)*/
    SDRAM_RefreshIntervalInit(0x750);//64000000ns/4096rows/(1/133M=8)ns =0x7A1,whichever is smaller
#else    
    /* Set SDRAM address: BaseAddr = 0xD0000000, AddrMask = 0xFFFFFFFF-(SDRAMSIZI-1)*/
    SDRAM_SetDeviceAddress(SDRAM_DEVICE_2, SDRAM2_ADDR, 0xFF000000);//AddrMask=0xFFFFFFFF-(16Mbyte-1)
    /* Set SDRAM Refresh Interval=(Refresh Period / Number of Rows / Mem clock Period)*/
    SDRAM_RefreshIntervalInit(0x750);//64000000ns/4096rows/(1/133M=8)ns =0x7A1,whichever is smaller
#endif
    
    /* Set SDRAM Timing registers*/
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Timing.RowActiveTime       = 0x7;
    SDRAM_Timing.RowCycleTime        = 0x9;
    SDRAM_Timing.RowActToRowActDelay = 0x3;
    SDRAM_Timing.PrechargeTime       = 0x4;
    SDRAM_Timing.WriteRecoveryTime   = 0x3;
    SDRAM_Timing.RefreshCycleTime    = 0x9;
    SDRAM_Timing.RAStoCASDelay       = 0x4;
    SDRAM_TimingInit(&SDRAM_Timing);
    
     /* Set SDRAM configuration registers*/
    SDRAM_Configuration.SdramEnable         = ENABLE;
    SDRAM_Configuration.RefreshEnable       = ENABLE;
    SDRAM_Configuration.AutoPrechargeEnable = DISABLE;
    SDRAM_Configuration.PrefetchReadEnable  = DISABLE;
    SDRAM_Configuration.SOM_Enable          = ENABLE;
    SDRAM_Configuration.BankInterleavEnable = DISABLE;
#if (SDRAM_DEVICE == SDRAM_W9825G6KH) 
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.BusWidth            = SDRAM_DEVICE_BUSWID_16BITS;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.BurstLen */
    SDRAM_Configuration.BurstLength         = SDRAM_DEVICE_BURSTLEN_4;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.CASLatency */
    SDRAM_Configuration.CAS_Latency         = SDRAM_DEVICE_CASLTCY_3;
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.AddressConfig       = SDRAM_BANK4_ROW8192_COL512;
    SDRAM_ConfigurationInit(SDRAM_DEVICE_1, SDRAM_Configuration);
#elif (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.BusWidth            = SDRAM_DEVICE_BUSWID_16BITS;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.BurstLen */
    SDRAM_Configuration.BurstLength         = SDRAM_DEVICE_BURSTLEN_4;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.CASLatency */
    SDRAM_Configuration.CAS_Latency         = SDRAM_DEVICE_CASLTCY_3;
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.AddressConfig       = SDRAM_BANK4_ROW4096_COL512;
    SDRAM_ConfigurationInit(SDRAM_DEVICE_1, SDRAM_Configuration);
#elif (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_IS42S32800J_EVB)
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.BusWidth            = SDRAM_DEVICE_BUSWID_32BITS;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.BurstLen */
    SDRAM_Configuration.BurstLength         = SDRAM_DEVICE_BURSTLEN_2;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.CASLatency */
    SDRAM_Configuration.CAS_Latency         = SDRAM_DEVICE_CASLTCY_3;
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.AddressConfig       = SDRAM_BANK4_ROW4096_COL512;
    SDRAM_ConfigurationInit(SDRAM_DEVICE_1, SDRAM_Configuration);
#else    
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.BusWidth            = SDRAM_DEVICE_BUSWID_32BITS;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.BurstLen */
    SDRAM_Configuration.BurstLength         = SDRAM_DEVICE_BURSTLEN_2;
    /* Needs to be consistent with SDRAM_Operation.Address.Bits.CASLatency */
    SDRAM_Configuration.CAS_Latency         = SDRAM_DEVICE_CASLTCY_3;
    /* Needs to be consistent with SDRAM device characteristics */
    SDRAM_Configuration.AddressConfig       = SDRAM_BANK4_ROW4096_COL256;
    SDRAM_ConfigurationInit(SDRAM_DEVICE_2, SDRAM_Configuration);
#endif
    
    /* Precharge all banks, auto-refresh, load mode register */
    /* Precharge all banks */
    SDRAM_Operation.ClockEnable   = ENABLE;
    SDRAM_Operation.OperationCode = SDRAM_OPCODE_PRECHRG;
    SDRAM_Operation.ChipSelect    = SDRAM_CS_SDRAMx;
    SDRAM_Operation.BankAddress   = SDRAM_BANKADD_1;
    SDRAM_Operation.Address.cmd   = 0x00;
    SDRAM_OperationInit(SDRAM_Operation);
    
    /* auto-refresh */
    for(uint8_t i = 0; i < 2; i++)
    {
        SDRAM_Operation.ClockEnable   = ENABLE;
        SDRAM_Operation.OperationCode = SDRAM_OPCODE_REFRESH;
        SDRAM_Operation.ChipSelect    = SDRAM_CS_SDRAMx;
        SDRAM_Operation.BankAddress   = SDRAM_BANKADD_1;
        SDRAM_Operation.Address.cmd   = 0x00;
        SDRAM_OperationInit(SDRAM_Operation);
    }
    
    /* load mode register */
    SDRAM_Operation.ClockEnable   = ENABLE;
    SDRAM_Operation.OperationCode = SDRAM_OPCODE_LOADMODE;
    SDRAM_Operation.ChipSelect    = SDRAM_CS_SDRAMx;     
    SDRAM_Operation.BankAddress   = SDRAM_BANKADD_1;
#if (SDRAM_DEVICE == SDRAM_W9825G6KH) || (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
    SDRAM_Operation.Address.Bits.BurstLen = LOADMODE_BURSTLEN_4;
#else
    SDRAM_Operation.Address.Bits.BurstLen = LOADMODE_BURSTLEN_2;
#endif
    SDRAM_Operation.Address.Bits.BurstType = LOADMODE_BURSTTYP_SEQUENTIAL;
    SDRAM_Operation.Address.Bits.CASLatency = LOADMODE_CASLATENCY_3;
    SDRAM_Operation.Address.Bits.OpMode = LOADMODE_STANDARD; 
    SDRAM_Operation.Address.Bits.WBMode = LOADMODE_WRITE_BURST;
    SDRAM_OperationInit(SDRAM_Operation);
    
    /* wait at least 300us*/
    systick_delay_us(500);
}

/**
*\*\name    SDRAM_OperationInit.
*\*\fun     Initialization SDRAM using Precharge all banks, auto-refresh and load mode register
*\*\param   The input parameters must be the following values: 
*\*\          - SDRAM_OS: 
*\*\               - ClockEnable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - OperationCode:
*\*\                   - NO_OPERATION
*\*\                   - PRECHARGE_ALL_DEVICE    
*\*\                   - REFRESH(when ClockEnable = ENABLE is auto refresh; when  ClockEnable = DISABLE is self refresh)                
*\*\                   - LOAD_MODE_REGISTER   
*\*\               - ChipSelect:
*\*\                   - SDRAM_CS_ALL
*\*\                   - SDRAM_CS_SDRAM2_ONLY
*\*\                   - SDRAM_CS_SDRAM1_ONLY
*\*\                   - SDRAM_CS_NONE 
*\*\               - BankAddress:
*\*\               - Address: load mode register parameters
*\*\                   - BurstLen : 3;
*\*\                   - BurstType : 1;
*\*\                   - CASLatency : 3;
*\*\                   - OpMode : 2;
*\*\                   - WBMode : 1;
*\*\return  none
**/
void SDRAM_OperationInit(SDRAM_OperationInitType SDRAM_OS)
{  
    /* Set the clock enable */
    SDRAM_EnableClock(SDRAM_OS.ClockEnable);
    
    /*Set the OP_CODE */
    SDRAM_SetOperationCode(SDRAM_OS.OperationCode);
    
    //SDRAM->OS |= 0x0C000000;//reserve 26\27
    
    /*Set the sdram cs */
    SDRAM_SetDeviceSelect(SDRAM_OS.ChipSelect);
    
    /*Set Bank address */
    SDRAM_SetBank(SDRAM_OS.BankAddress);
    
    /*Set address */
    SDRAM_SetAddress(SDRAM_OS.Address.cmd);

    /*A dummy access to activate previous SDRAM_OS configuration, read/write data is dont care*/
    (void)(SDRAM->OR);
}

/**
*\*\name    SDRAM_ConfigurationInit.
*\*\fun     Configure SDRAM1 or 2 bus width, burst length, latency, and other information
*\*\param   The input parameters must be the following values: 
*\*\          - DeviceNo: 
*\*\               - SDRAM_DEVICE_1
*\*\               - SDRAM_DEVICE_2
*\*\          - SDRAM_CFG: 
*\*\               - SdramEnable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - RefreshEnable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - AutoPrechargeEnable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - PrefetchReadEnable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - SOM_Enable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - BankInterleavEnable:
*\*\                   - ENABLE
*\*\                   - DISABLE 
*\*\               - BusWidth:
*\*\                   - SDRAM_DEVICE_BUSWID_8BITS
*\*\                   - SDRAM_DEVICE_BUSWID_16BITS
*\*\                   - SDRAM_DEVICE_BUSWID_32BITS
*\*\               - BurstLength:
*\*\                   - SDRAM_DEVICE_BURSTLEN_1
*\*\                   - SDRAM_DEVICE_BURSTLEN_2
*\*\                   - SDRAM_DEVICE_BURSTLEN_4
*\*\                   - SDRAM_DEVICE_BURSTLEN_8
*\*\               - CAS_Latency: 
*\*\                   - SDRAM_DEVICE_CASLTCY_0
*\*\                   - SDRAM_DEVICE_CASLTCY_1
*\*\                   - SDRAM_DEVICE_CASLTCY_2
*\*\                   - SDRAM_DEVICE_CASLTCY_3
*\*\               - AddressConfig:
*\*\                   - SDRAM_BANK4_ROW4096_COL256 
*\*\                   - SDRAM_BANK4_ROW4096_COL512 
*\*\                   - SDRAM_BANK4_ROW4096_COL1024
*\*\                   - SDRAM_BANK4_ROW4096_COL2048
*\*\                   - SDRAM_BANK4_ROW8192_COL256 
*\*\                   - SDRAM_BANK4_ROW8192_COL512 
*\*\                   - SDRAM_BANK4_ROW8192_COL1024
*\*\                   - SDRAM_BANK4_ROW8192_COL2048
*\*\                   - SDRAM_BANK4_ROW2048_COL256 
*\*\                   - SDRAM_BANK4_ROW2048_COL512 
*\*\                   - SDRAM_BANK4_ROW2048_COL1024
*\*\                   - SDRAM_BANK4_ROW2048_COL2048
*\*\return  none
**/
void SDRAM_ConfigurationInit(SDRAM_DeviceType DeviceNo, SDRAM_ConfigurationInitType SDRAM_CFG)
{
    /* Set the clock enable */
    SDRAM_EnableDevice(DeviceNo, SDRAM_CFG.SdramEnable);

    SDRAM_EnableRefreshCMD(DeviceNo, SDRAM_CFG.RefreshEnable);
    
    SDRAM_EnableAutoPrecharge(DeviceNo, SDRAM_CFG.AutoPrechargeEnable);
    
    SDRAM_EnablePrefetchRead(DeviceNo, SDRAM_CFG.PrefetchReadEnable);
    
    SDRAM_EnableSOM(DeviceNo, SDRAM_CFG.SOM_Enable);
    
    SDRAM_EnableBankInterleave(DeviceNo, SDRAM_CFG.BankInterleavEnable);
    
    SDRAM_ConfigBusWidth(DeviceNo, SDRAM_CFG.BusWidth);
    
    SDRAM_ConfigBurstLength(DeviceNo, SDRAM_CFG.BurstLength);
    
    SDRAM_ConfigCASLatency(DeviceNo, SDRAM_CFG.CAS_Latency);
    
    SDRAM_ConfigAddress(DeviceNo, SDRAM_CFG.AddressConfig);
    
}

/**
*\*\name    SDRAM_Read_8b.
*\*\fun     Reads 8-bit data buffer from the SDRAM memory.
*\*\param   The input parameters must be the following values: 
*\*\          - pAddress  :  pAddress Pointer to read start address
*\*\          - pDstBuffer:  pDstBuffer Pointer to destination buffer
*\*\          - BufferSize:  BufferSize Size of the buffer to read from memory
*\*\return  none
**/
void SDRAM_Read_8b(uint32_t *pAddress, uint8_t *pDstBuffer, uint32_t BufferSize)
{
    uint32_t size;
    __IO uint8_t *pSdramAddress = (uint8_t *)pAddress;
    uint8_t *pdestbuff = pDstBuffer;
    
    /* Read data from source */
    for (size = BufferSize; size != 0U; size--)
    {
        *pdestbuff = *(__IO uint8_t *)pSdramAddress;
        pdestbuff++;
        pSdramAddress++;
    }
}

/**
*\*\name    SDRAM_Write_8b.
*\*\fun     Writes 8-bit data buffer to SDRAM memory.
*\*\param   The input parameters must be the following values: 
*\*\          - pAddress  :  pAddress Pointer to read start address
*\*\          - pSrcBuffer:  pSrcBuffer Pointer to source buffer to write
*\*\          - BufferSize:  BufferSize Size of the buffer to read from memory
*\*\return  none
**/
void SDRAM_Write_8b(uint32_t *pAddress, uint8_t *pSrcBuffer, uint32_t BufferSize)
{
    uint32_t size;
    __IO uint8_t *pSdramAddress = (uint8_t *)pAddress;
    uint8_t *psrcbuff = pSrcBuffer;
    
    /* Write data to memory */
    for (size = BufferSize; size != 0U; size--)
    {
        *(__IO uint8_t *)pSdramAddress = *psrcbuff;
        psrcbuff++;
        pSdramAddress++;
    }
}

/**
*\*\name    SDRAM_Read_16b.
*\*\fun     Reads 16-bit data buffer from the SDRAM memory.
*\*\param   The input parameters must be the following values: 
*\*\          - pAddress  :  pAddress Pointer to read start address
*\*\          - pDstBuffer:  pDstBuffer Pointer to destination buffer
*\*\          - BufferSize:  BufferSize Size of the buffer to read from memory
*\*\return  none
**/
void SDRAM_Read_16b(uint32_t *pAddress, uint16_t *pDstBuffer, uint32_t BufferSize)
{
    uint32_t size;
    __IO uint32_t *pSdramAddress = pAddress;
    uint16_t *pdestbuff = pDstBuffer;
    
    /* Read data from memory */
    for (size = BufferSize; size >= 2U ; size -= 2U)
    {
        *pdestbuff = (uint16_t)((*pSdramAddress) & 0x0000FFFFU);
        pdestbuff++;
        *pdestbuff = (uint16_t)(((*pSdramAddress) & 0xFFFF0000U) >> 16U);
        pdestbuff++;
        pSdramAddress++;
    }
}

/**
*\*\name    SDRAM_Write_16b.
*\*\fun     Writes 16-bit data buffer to SDRAM memory.
*\*\param   The input parameters must be the following values: 
*\*\          - pAddress  :  pAddress Pointer to read start address
*\*\          - pSrcBuffer:  pSrcBuffer Pointer to source buffer to write
*\*\          - BufferSize:  BufferSize Size of the buffer to read from memory
*\*\return  none
**/
void SDRAM_Write_16b(uint32_t *pAddress, uint16_t *pSrcBuffer, uint32_t BufferSize)
{
    uint32_t size;
    __IO uint32_t *psdramaddress = pAddress;
    uint16_t *psrcbuff = pSrcBuffer; 
    
    /* Write data to memory */
    for (size = BufferSize; size >= 2U ; size -= 2U)
    {
        *psdramaddress = (uint32_t)(*psrcbuff);
        psrcbuff++;
        *psdramaddress |= ((uint32_t)(*psrcbuff) << 16U);
        psrcbuff++;
        psdramaddress++;
    }
}

/**
*\*\name    SDRAM_Read_32b.
*\*\fun     Reads 32-bit data buffer from the SDRAM memory.
*\*\param   The input parameters must be the following values: 
*\*\          - pAddress  :  pAddress Pointer to read start address
*\*\          - pDstBuffer:  pSrcBuffer Pointer to source buffer to write
*\*\          - BufferSize:  BufferSize Size of the buffer to read from memory
*\*\return  none
**/
void SDRAM_Read_32b(uint32_t *pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
    uint32_t size;
    __IO uint32_t *pSdramAddress = (uint32_t *)pAddress;
    uint32_t *pdestbuff = pDstBuffer;
    
    /* Read data from source */
    for (size = BufferSize; size != 0U; size--)
    {
        *pdestbuff = *(__IO uint32_t *)pSdramAddress;
        pdestbuff++;
        pSdramAddress++;
    }
}

/**
*\*\name    SDRAM_Write_32b.
*\*\fun     Writes 32-bit data buffer to SDRAM memory.
*\*\param   The input parameters must be the following values: 
*\*\          - pAddress  :  pAddress Pointer to read start address
*\*\          - pSrcBuffer:  pSrcBuffer Pointer to source buffer to write
*\*\          - BufferSize:  BufferSize Size of the buffer to read from memory
*\*\return  none
**/
void SDRAM_Write_32b(uint32_t *pAddress, uint32_t *pSrcBuffer, uint32_t BufferSize)
{
    uint32_t size;
    __IO uint32_t *pSdramAddress = pAddress;
    uint32_t *psrcbuff = pSrcBuffer;
    
    /* Write data to memory */
    for (size = BufferSize; size != 0U; size--)
    {
        *pSdramAddress = *psrcbuff;
        psrcbuff++;
        pSdramAddress++;
    }
}

