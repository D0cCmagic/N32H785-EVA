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
*\*\file bsp_eth.c
*\*\author Nations
*\*\version v1.0.0
*\*\copyright Copyright (c) 2025, Nations Technologies Inc. All rights reserved.
**/ 

#include "bsp_eth.h"


/* Local MAC address */
uint8_t aMACAddr[6];

/* ETH DMA TX RX descriptor physical address */
ETH_DMADescType aDMARxDscrTab[ETH_RX_DESC_NUMBER];
ETH_DMADescType aDMATxDscrTab[ETH_TX_DESC_NUMBER];

/* ETH receive buffer physical address */
uint8_t aRxBuffer[ETH_RX_DESC_NUMBER][ETH_RX_BUFFER_SIZE];

/* Global ETH information structure variable */
ETH_InfoType sEthInfo;

/* Global ETH initialize structure variable */
ETH_InitType sETH_InitParam;

/* Global ETH TX packet structure variable */
ETH_TxPacketType sTxPacket;

/* ETH1 RMII */
ETH_PinType ETH1RMIIPins[] = 
{
    /* ETH1_RMII_CRS_DV */
    {GPIOA, GPIO_PIN_7,  GPIO_AF3, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_RMII_TXD0 */
    {GPIOG, GPIO_PIN_13, GPIO_AF0, GPIO_MODE_AF_PP, GPIO_DC_2mA},
#if ((DEMO_BOARD == N32H787_HMI) || (DEMO_BOARD == N32H785_YT))
    /* ETH1_RMII_TXD1 */
    {GPIOG, GPIO_PIN_14, GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
#else
    /* ETH1_RMII_TXD1 */
    {GPIOG, GPIO_PIN_12, GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
#endif
#if (DEMO_BOARD == N32H787_HMI)
    /* ETH1_RMII_TXEN */
    {GPIOB, GPIO_PIN_11, GPIO_AF0, GPIO_MODE_AF_PP, GPIO_DC_2mA},
#else
    /* ETH1_RMII_TXEN */
    {GPIOG, GPIO_PIN_11, GPIO_AF2, GPIO_MODE_AF_PP, GPIO_DC_2mA},
#endif
    /* ETH1_RMII_RXD0 */
    {GPIOC, GPIO_PIN_4,  GPIO_AF1, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_RMII_RXD1 */
    {GPIOC, GPIO_PIN_5,  GPIO_AF2, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_RMII_REFCLK */
    {GPIOA, GPIO_PIN_1,  GPIO_AF2, GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_MDC */
    {GPIOC, GPIO_PIN_1,  GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MDIO */
    {GPIOA, GPIO_PIN_2,  GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA}
};

/* ETH1 MII */
ETH_PinType ETH1MIIPins[] = 
{
    /* ETH1_MII_RX_CLK */
    {GPIOA, GPIO_PIN_1,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_MII_RX_DV */
    {GPIOA, GPIO_PIN_7,  GPIO_AF3,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_MII_RX_ERR */
    {GPIOB, GPIO_PIN_10, GPIO_AF1,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_MII_RXD0 */
    {GPIOC, GPIO_PIN_4,  GPIO_AF1,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_MII_RXD1 */
    {GPIOC, GPIO_PIN_5,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_MII_RXD2 */
    {GPIOB, GPIO_PIN_0,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_MII_RXD3 */
    {GPIOB, GPIO_PIN_1,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_MII_TX_CLK */
    {GPIOC, GPIO_PIN_3,  GPIO_AF4,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_MII_TX_EN */
    {GPIOG, GPIO_PIN_11, GPIO_AF2,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MII_TXD0 */
    {GPIOG, GPIO_PIN_13, GPIO_AF0,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MII_TXD1 */
    {GPIOG, GPIO_PIN_14, GPIO_AF1,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MII_TXD2 */
    {GPIOE, GPIO_PIN_3,  GPIO_AF0,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MII_TXD3 */
    {GPIOE, GPIO_PIN_2,  GPIO_AF1,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MII_COL */
    {GPIOA, GPIO_PIN_3,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_MII_CRS */
    {GPIOH, GPIO_PIN_2,  GPIO_AF4,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_MDC */
    {GPIOC, GPIO_PIN_1,  GPIO_AF1,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MDIO */
    {GPIOA, GPIO_PIN_2,  GPIO_AF1,  GPIO_MODE_AF_PP, GPIO_DC_2mA}
};

/* ETH1 GMII */
ETH_PinType ETH1GMIIPins[] = 
{
    /* ETH1_GMII_RXDV */
    {GPIOA, GPIO_PIN_7,  GPIO_AF3, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_TXD0 */
    {GPIOG, GPIO_PIN_13, GPIO_AF0, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_TXD1 */
    {GPIOG, GPIO_PIN_12, GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_TXD2 */
    {GPIOE, GPIO_PIN_3,  GPIO_AF0, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_TXD3 */
    {GPIOE, GPIO_PIN_2,  GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_TXD4 */
    {GPIOI, GPIO_PIN_4,  GPIO_AF2, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_TXD5 */
    {GPIOI, GPIO_PIN_5,  GPIO_AF3, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_TXD6 */
    {GPIOI, GPIO_PIN_6,  GPIO_AF2, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_TXD7 */
    {GPIOI, GPIO_PIN_7,  GPIO_AF3, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_TXEN */
    {GPIOG, GPIO_PIN_11, GPIO_AF2, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_RXD0 */
    {GPIOC, GPIO_PIN_4,  GPIO_AF1, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD1 */
    {GPIOC, GPIO_PIN_5,  GPIO_AF2, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD2 */
    {GPIOH, GPIO_PIN_6,  GPIO_AF2, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD3 */
    {GPIOH, GPIO_PIN_7,  GPIO_AF1, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD4 */
    {GPIOE, GPIO_PIN_12, GPIO_AF2, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD5 */
    {GPIOE, GPIO_PIN_13, GPIO_AF1, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD6 */
    {GPIOE, GPIO_PIN_14, GPIO_AF2, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXD7 */
    {GPIOE, GPIO_PIN_15, GPIO_AF1, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_RXCLK */
    {GPIOA, GPIO_PIN_1,  GPIO_AF2, GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_GMII_COL */
    {GPIOH, GPIO_PIN_3,  GPIO_AF3, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_CRS */
    {GPIOA, GPIO_PIN_0,  GPIO_AF0, GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH1_GMII_RXER */
    {GPIOB, GPIO_PIN_10, GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_TXER */
    {GPIOB, GPIO_PIN_2,  GPIO_AF3, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_GMII_TXCLK */
    {GPIOC, GPIO_PIN_3,  GPIO_AF4, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_GMII_GTXCLK */
    {GPIOF, GPIO_PIN_5,  GPIO_AF2, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH1_CLK125 */
    {GPIOD, GPIO_PIN_10, GPIO_AF1, GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH1_MDC */
    {GPIOC, GPIO_PIN_1,  GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH1_MDIO */
    {GPIOA, GPIO_PIN_2,  GPIO_AF1, GPIO_MODE_AF_PP, GPIO_DC_2mA}
};

/* ETH2 RMII */
ETH_PinType ETH2RMIIPins[] = 
{
    /* ETH2_RMII_CRS_DV */
    {GPIOJ, GPIO_PIN_3,  GPIO_AF0, GPIO_MODE_INPUT, GPIO_5VTOL_DC_2mA},
    /* ETH2_RMII_TXD0 */
    {GPIOF, GPIO_PIN_12, GPIO_AF2, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_RMII_TXD1 */
    {GPIOF, GPIO_PIN_13, GPIO_AF2, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_RMII_TXEN */
    {GPIOF, GPIO_PIN_11, GPIO_AF6, GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_RMII_RXD0 */
    {GPIOG, GPIO_PIN_9,  GPIO_AF1, GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH2_RMII_RXD1 */
    {GPIOG, GPIO_PIN_10, GPIO_AF2, GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH2_RMII_REFCLK */
    {GPIOJ, GPIO_PIN_4,  GPIO_AF0, GPIO_MODE_INPUT, GPIO_5VTOL_DC_2mA},
    /* ETH2_MDC */
    {GPIOF, GPIO_PIN_7,  GPIO_AF2, GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH2_MDIO */
    {GPIOF, GPIO_PIN_6,  GPIO_AF2, GPIO_MODE_AF_PP, GPIO_DC_2mA}
};

/* ETH2 MII */
ETH_PinType ETH2MIIPins[] = 
{
    /* ETH2_MII_RX_CLK */
    {GPIOJ, GPIO_PIN_4,  GPIO_AF0,  GPIO_MODE_INPUT, GPIO_5VTOL_DC_2mA},
    /* ETH2_MII_RX_DV */
    {GPIOJ, GPIO_PIN_3,  GPIO_AF0,  GPIO_MODE_INPUT, GPIO_5VTOL_DC_2mA},
    /* ETH2_MII_RX_ERR */
    {GPIOI, GPIO_PIN_3,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_RXD0 */
    {GPIOG, GPIO_PIN_9,  GPIO_AF1,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH2_MII_RXD1 */
    {GPIOG, GPIO_PIN_10, GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH2_MII_RXD2 */
    {GPIOG, GPIO_PIN_4,  GPIO_AF1,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_RXD3 */
    {GPIOG, GPIO_PIN_5,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_TX_CLK */
    {GPIOG, GPIO_PIN_0,  GPIO_AF4,  GPIO_MODE_INPUT, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_TX_EN */
    {GPIOF, GPIO_PIN_11, GPIO_AF6,  GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_TXD0 */
    {GPIOF, GPIO_PIN_12, GPIO_AF2,  GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_TXD1 */
    {GPIOF, GPIO_PIN_13, GPIO_AF2,  GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_TXD2 */
    {GPIOF, GPIO_PIN_14, GPIO_AF1,  GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_TXD3 */
    {GPIOF, GPIO_PIN_15, GPIO_AF2,  GPIO_MODE_AF_PP, GPIO_HS_IO_DC_2mA},
    /* ETH2_MII_COL */
    {GPIOF, GPIO_PIN_8,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH2_MII_CRS */
    {GPIOF, GPIO_PIN_9,  GPIO_AF2,  GPIO_MODE_INPUT, GPIO_DC_2mA},
    /* ETH2_MDC */
    {GPIOF, GPIO_PIN_7,  GPIO_AF2,  GPIO_MODE_AF_PP, GPIO_DC_2mA},
    /* ETH2_MDIO */
    {GPIOF, GPIO_PIN_6,  GPIO_AF2,  GPIO_MODE_AF_PP, GPIO_DC_2mA}
};

/** ETH_BSP Private Defines **/


/** ETH_BSP Driving Functions Declaration **/

/**
*\*\name    ETH_BSP_GPIOInit.
*\*\fun     Initialization the GPIOs used by the ETH module.
*\*\param   pInfo :
*\*\          - Pointer to an ETH_InfoType structure parameter containing various
*\*\            information about the operation of the ETH module.
*\*\return  none
**/
void ETH_BSP_GPIOInit(ETH_InfoType* pInfo)
{
    uint32_t      index = 0;
    GPIO_InitType GPIO_InitStructure;

    /* Initialize the pins */
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Slew_Rate     = GPIO_SLEW_RATE_FAST;
    if ((ETH == ETH1) && (pInfo->MediaInterface == ETH_RMII_MODE))
    {
        /* Using ETH1 with RMII hardware interface */
        for (index = 0; index < (sizeof(ETH1RMIIPins) / sizeof(ETH_PinType)); index++)
        {
            GPIO_InitStructure.Pin            = ETH1RMIIPins[index].Pin;
            GPIO_InitStructure.GPIO_Mode      = ETH1RMIIPins[index].Mode;
            GPIO_InitStructure.GPIO_Current   = ETH1RMIIPins[index].Current;
            GPIO_InitStructure.GPIO_Alternate = ETH1RMIIPins[index].Alternate;
            GPIO_InitPeripheral(ETH1RMIIPins[index].GPIOx, &GPIO_InitStructure);
        }
    }
    else if ((ETH == ETH1) && (pInfo->MediaInterface == ETH_MII_MODE))
    {
        /* Using ETH1 with MII hardware interface */
        for (index = 0; index < (sizeof(ETH1MIIPins) / sizeof(ETH_PinType)); index++)
        {
            GPIO_InitStructure.Pin            = ETH1MIIPins[index].Pin;
            GPIO_InitStructure.GPIO_Mode      = ETH1MIIPins[index].Mode;
            GPIO_InitStructure.GPIO_Current   = ETH1MIIPins[index].Current;
            GPIO_InitStructure.GPIO_Alternate = ETH1MIIPins[index].Alternate;
            GPIO_InitPeripheral(ETH1MIIPins[index].GPIOx, &GPIO_InitStructure);
        }
    }
    else if ((ETH == ETH1) && (pInfo->MediaInterface == ETH_GMII_MODE))
    {
        /* Using ETH1 with GMII hardware interface */
        for (index = 0; index < (sizeof(ETH1GMIIPins) / sizeof(ETH_PinType)); index++)
        {
            GPIO_InitStructure.Pin            = ETH1GMIIPins[index].Pin;
            GPIO_InitStructure.GPIO_Mode      = ETH1GMIIPins[index].Mode;
            GPIO_InitStructure.GPIO_Current   = ETH1GMIIPins[index].Current;
            GPIO_InitStructure.GPIO_Alternate = ETH1GMIIPins[index].Alternate;
            GPIO_InitPeripheral(ETH1GMIIPins[index].GPIOx, &GPIO_InitStructure);
        }
    }
    else if ((ETH == ETH2) && (pInfo->MediaInterface == ETH_RMII_MODE))
    {
        /* Using ETH2 with RMII hardware interface */
        for (index = 0; index < (sizeof(ETH2RMIIPins) / sizeof(ETH_PinType)); index++)
        {
            GPIO_InitStructure.Pin            = ETH2RMIIPins[index].Pin;
            GPIO_InitStructure.GPIO_Mode      = ETH2RMIIPins[index].Mode;
            GPIO_InitStructure.GPIO_Current   = ETH2RMIIPins[index].Current;
            GPIO_InitStructure.GPIO_Alternate = ETH2RMIIPins[index].Alternate;
            GPIO_InitPeripheral(ETH2RMIIPins[index].GPIOx, &GPIO_InitStructure);
        }
    }
    else if ((ETH == ETH2) && (pInfo->MediaInterface == ETH_MII_MODE))
    {
        /* Using ETH2 with MII hardware interface */
        for (index = 0; index < (sizeof(ETH2MIIPins) / sizeof(ETH_PinType)); index++)
        {
            GPIO_InitStructure.Pin            = ETH2MIIPins[index].Pin;
            GPIO_InitStructure.GPIO_Mode      = ETH2MIIPins[index].Mode;
            GPIO_InitStructure.GPIO_Current   = ETH2MIIPins[index].Current;
            GPIO_InitStructure.GPIO_Alternate = ETH2MIIPins[index].Alternate;
            GPIO_InitPeripheral(ETH2MIIPins[index].GPIOx, &GPIO_InitStructure);
        }
    }
    else
    {
        /* The interface mode doesn't match the hardware development board, and does nothing */
    }

    /* Check whether to enable PPS output */
    if (pInfo->PPSOutCmd != DISABLE)
    {
        if (ETH == ETH1)
        {
            /* ETH1_PPS_OUT: PB5(AF2)/PG8(AF2)/PD14(AF2), Alternate Function Push Pull Mode */
            GPIO_InitStructure.Pin            = GPIO_PIN_5;
            GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStructure.GPIO_Alternate = GPIO_AF2;
            GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
        }
        else
        {
            /* ETH2_PPS_OUT: PD10(AF2)/PF10(AF2)/PI15(AF0), Alternate Function Push Pull Mode */
            GPIO_InitStructure.Pin            = GPIO_PIN_10;
            GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStructure.GPIO_Alternate = GPIO_AF2;
            GPIO_InitPeripheral(GPIOG, &GPIO_InitStructure);
        }
    }
}

/**
*\*\name    ETH_BSP_NVICInit.
*\*\fun     Initialization Configuration NVIC.
*\*\param   pInfo :
*\*\          - Pointer to an ETH_InfoType structure parameter containing various
*\*\            information about the operation of the ETH module.
*\*\return  none
**/
void ETH_BSP_NVICInit(ETH_InfoType* pInfo)
{
    NVIC_InitType NVIC_InitStructure;
    EXTI_InitType EXTI_InitStructure;
    
    /* Configures the priority group */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /* Configures the ETH global interrupt NVIC */
    NVIC_InitStructure.NVIC_IRQChannel                   = (uint8_t)ETH_GLOBAL_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRE_PRIORITY_5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = NVIC_SUB_PRIORITY_0;
    NVIC_Init(&NVIC_InitStructure);

    /* Check whether to enable PMT */
    if ((pInfo->PMTCmd != DISABLE) || (pInfo->LPICmd != DISABLE))
    {
        /* Configures the PMT connected to EXTI Line n */
        EXTI_InitStruct(&EXTI_InitStructure);
        EXTI_InitStructure.EXTI_Line                     = ETH_EXTI_LINE;
        EXTI_InitStructure.EXTI_LineCmd                  = ENABLE;
        EXTI_InitStructure.EXTI_Mode                     = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger                  = EXTI_Trigger_Rising;
        EXTI_InitPeripheral(&EXTI_InitStructure);
        /* Configures the ETH PMT wakeup interrupt and LPI interrupt NVIC */
        NVIC_InitStructure.NVIC_IRQChannel               = (uint8_t)ETH_PMT_LPI_IRQ;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority    = NVIC_SUB_PRIORITY_1;
        NVIC_Init(&NVIC_InitStructure);
    }
}

/**
*\*\name    ETH_BSP_ClockCmd.
*\*\fun     Enables or disables the ETH, GPIO, AFIO, etc. clocks.
*\*\param   Cmd (The input parameters must be the following values):
*\*\          - ENABLE
*\*\          - DISABLE
*\*\return  none
**/
void ETH_BSP_ClockCmd(FunctionalStatus Cmd)
{
    /* Enable or disable GPIOx Clock */
    RCC_EnableAHB5PeriphClk1((RCC_AHB5_PERIPHEN_M7_GPIOA | RCC_AHB5_PERIPHEN_M7_GPIOB
                            | RCC_AHB5_PERIPHEN_M7_GPIOC | RCC_AHB5_PERIPHEN_M7_GPIOD
                            | RCC_AHB5_PERIPHEN_M7_GPIOE | RCC_AHB5_PERIPHEN_M7_GPIOF
                            | RCC_AHB5_PERIPHEN_M7_GPIOG | RCC_AHB5_PERIPHEN_M7_GPIOH),
                             Cmd);
    RCC_EnableAHB5PeriphClk2((RCC_AHB5_PERIPHEN_M7_GPIOI | RCC_AHB5_PERIPHEN_M7_GPIOJ
                            | RCC_AHB5_PERIPHEN_M7_GPIOK), Cmd);

    /* Enable or disable AFIO Clock */
    RCC_EnableAHB5PeriphClk2(RCC_AHB5_PERIPHEN_M7_AFIO, Cmd);
    
    /* Enable or disable PWR Clock */
    RCC_EnableAHB5PeriphClk2(RCC_AHB5_PERIPHEN_PWR, Cmd);
    /* Enable or disable the power domain of the ETH */
    PWR_MoudlePowerEnable(ETH_PWR_CTRL, Cmd);

    /* Enable or disable ETH Clock */
    ETH_RCC_ENABLE_AHB_PERIPHEN_CLK(ETH_RCC_AHB_PERIPHEN, Cmd);
}

/**
*\*\name    ETH_BSP_GetPhyLinkStatus.
*\*\fun     Get PHY link status.
*\*\param   phyAddr:
*\*\          - PHY port address.
*\*\param   phyReg:
*\*\          - PHY register address.
*\*\return  Acquired PHY link state value containing speed core duplex mode
**/
uint16_t ETH_BSP_GetPhyLinkStatus(uint16_t phyAddr, uint16_t phyReg)
{
    uint16_t retVlaue;
    uint32_t regVlaue;
    
    /* Read the PHY register */
    if (ETH_ReadPHYRegister(ETH, (uint32_t)phyAddr, (uint32_t)phyReg, &regVlaue) == 0U)
    {
        /* Read ERROR */
        retVlaue = 0U;
    }
    else
    {
        /* Get link status */
        retVlaue = ((regVlaue & PHY_SPEEDDUPLEX_MASK) == PHY_FULLDUPLEX10M_STS)? ETH_LINK_10FULL :
                   ((regVlaue & PHY_SPEEDDUPLEX_MASK) == PHY_HALFDUPLEX10M_STS)? ETH_LINK_10HALF :
                   ((regVlaue & PHY_SPEEDDUPLEX_MASK) == PHY_FULLDUPLEX100M_STS)? ETH_LINK_100FULL :
                   ((regVlaue & PHY_SPEEDDUPLEX_MASK) == PHY_HALFDUPLEX100M_STS)? ETH_LINK_100HALF :
#ifdef PHY_USE_RTL8211EG
                   ((regVlaue & PHY_SPEEDDUPLEX_MASK) == PHY_FULLDUPLEX1000M_STS)? ETH_LINK_1000FULL :
                   ((regVlaue & PHY_SPEEDDUPLEX_MASK) == PHY_HALFDUPLEX1000M_STS)? ETH_LINK_1000HALF :
#endif
                   0U;
    }
    
    return retVlaue;
}

/**
*\*\name    ETH_BSP_Init.
*\*\fun     ETH initialization function, called in the low_level_init() function.
*\*\param   none
*\*\return  SUCCESS or ERROR.
**/
ErrorStatus ETH_BSP_Init(void)
{
    uint32_t TempIndex;
    
    /* Set the local MAC address */
    aMACAddr[0] = MAC_ADDR0;
    aMACAddr[1] = MAC_ADDR1;
    aMACAddr[2] = MAC_ADDR2;
    aMACAddr[3] = MAC_ADDR3;
    aMACAddr[4] = MAC_ADDR4;
    aMACAddr[5] = MAC_ADDR5;

    /* Clear the sEthInfo structure variable */
    memset(&sEthInfo, 0, sizeof(ETH_InfoType));
    /* Set ETH operation-related information via sEthInfo */
    sEthInfo.AutoNegCmd     = (FunctionalStatus)ETH_AUTONEG_CMD;
    sEthInfo.MDCClockMode   = ETH_MDCCLK_NORMAL;
    sEthInfo.MediaInterface = ETH_SEL_MEDIAIF;
    sEthInfo.pMACAddr       = &aMACAddr[0];
    sEthInfo.PMTCmd         = (FunctionalStatus)ETH_PMT_CMD;
    sEthInfo.LPICmd         = (FunctionalStatus)ETH_LPI_CMD;
    sEthInfo.PPSOutCmd      = (FunctionalStatus)ETH_PPSOUT_CMD;
    sEthInfo.pRxDesc        = aDMARxDscrTab;
    sEthInfo.pTxDesc        = aDMATxDscrTab;
    sEthInfo.RxBuffLen      = ETH_RX_BUFFER_SIZE;
    sEthInfo.PHYInfo.phyAddr       = PHY_ADDR;
    sEthInfo.PHYInfo.bcRegAddr     = PHY_BCR;
    sEthInfo.PHYInfo.bsRegAddr     = PHY_BSR;
    sEthInfo.PHYInfo.sdRegAddr     = PHY_SDSR;
    sEthInfo.PHYInfo.phyReset      = PHY_RESET;
    sEthInfo.PHYInfo.phyAutoNeg    = PHY_AUTONEGOTIATION;
    sEthInfo.PHYInfo.phyAutoNegOK  = PHY_AUTONEGO_COMPLETE;
    sEthInfo.PHYInfo.phyLinkOK     = PHY_LINKED_STATUS;
    sEthInfo.PHYInfo.phyDuplexMask = PHY_DUPLEX_MASK;
    sEthInfo.PHYInfo.phySpeedMask  = PHY_SPEED_MASK;
    sEthInfo.PHYInfo.phyGetLinkStatus = ETH_BSP_GetPhyLinkStatus;

    /* Enable related clocks */
    ETH_BSP_ClockCmd(ENABLE);
    /* Configuring GPIOs */
    ETH_BSP_GPIOInit(&sEthInfo);
    /* Configuring NVIC */
    ETH_BSP_NVICInit(&sEthInfo);

    /* DeInitializes the ETH peripheral */
    ETH_DeInit(ETH);
    /* Set ETH initialization parameters by default */
    ETH_StructInit(ETH, &sETH_InitParam);
    /* Modify ETH initialization parameters */
#if (ETH_AUTONEG_CMD == 0U)
    sETH_InitParam.Duplex               = ETH_SEL_DUPLEX;
    sETH_InitParam.SpeedSelect          = ETH_SEL_SPEED;
#endif
    sETH_InitParam.AutoPadCRCStrip      = ((uint32_t)ENABLE << 20);
    sETH_InitParam.CRCStripTypePacket   = ((uint32_t)ENABLE << 21);
    sETH_InitParam.ChecksumOffload      = ((uint32_t)ENABLE << 27);
    sETH_InitParam.GiantPacketSizeLimit = (0x618U);

    sETH_InitParam.ProgramWatchdog    = ((uint32_t)ENABLE << 8);
    sETH_InitParam.TxQueueOperateMode = ETH_TXQUEUE_OPERATE_THRESHOLD_64;
    sETH_InitParam.RxQueueOperateMode = ETH_RXQUEUE_OPERATE_THRESHOLD_64;

    sETH_InitParam.BurstMode         = ETH_BURST_MODE_FIXED;
    sETH_InitParam.AddrAlignedBeats  = ((uint32_t)ENABLE << 12);
    sETH_InitParam.DescriptorSkipLen = ETH_DESC_SKIP_LEN_64BIT;
    sETH_InitParam.TxBurstLength     = ETH_TX_PROGRAM_BURST_LEN_32;
    sETH_InitParam.RxBurstLength     = ETH_RX_PROGRAM_BURST_LEN_32;
    
#if (ETH_AUTONEG_CMD == 0U)
    sEthInfo.PHYInfo.phyAutoNeg = (~PHY_AUTONEGOTIATION);
    if ((sETH_InitParam.Duplex == ETH_FULL_DUPLEX_MODE)
            && (sETH_InitParam.SpeedSelect == ETH_SPEED_100M))
    {
        /* Set the full-duplex mode at 100 Mb/s to PHY */
        sEthInfo.PHYInfo.phyMode = PHY_FULLDUPLEX_100M;
    }
    else if ((sETH_InitParam.Duplex == ETH_FULL_DUPLEX_MODE)
            && (sETH_InitParam.SpeedSelect == ETH_SPEED_10M))
    {
        /* Set the full-duplex mode at 10 Mb/s to PHY */
        sEthInfo.PHYInfo.phyMode = PHY_FULLDUPLEX_10M;
    }
    else if ((sETH_InitParam.Duplex == ETH_HALF_DUPLEX_MODE)
            && (sETH_InitParam.SpeedSelect == ETH_SPEED_100M))
    {
        /* Set the half-duplex mode at 100 Mb/s to PHY */
        sEthInfo.PHYInfo.phyMode = PHY_HALFDUPLEX_100M;
    }
    else if ((sETH_InitParam.Duplex == ETH_HALF_DUPLEX_MODE)
            && (sETH_InitParam.SpeedSelect == ETH_SPEED_10M))
    {
        /* Set the half-duplex mode at 10 Mb/s to PHY */
        sEthInfo.PHYInfo.phyMode = PHY_HALFDUPLEX_10M;
    }
    else
    {
        /* Auto-Negotiation is enabled when speed is set to 1000Base-T */
        return ERROR;
    }
#endif
    
    /* GTX_CLK (125 MHz) must be configured when GMII */
    if (ETH_SEL_MEDIAIF == ETH_GMII_MODE)
    {
        /* GTX_CLK clock from GPIO (provided by external PHY) */
        RCC_ConfigETH1GMIITXClk(RCC_ETH1GMIITXCLK_SRC_IOM);
    }

    /* Checks whether initializing the ETH was successful */
    if (ETH_Init(ETH, &sEthInfo, &sETH_InitParam) != ETH_SUCCESS)
    {
        /* ETH initialization failed */
        return ERROR;
    }

    /* Assign memory for each RX descriptor */
    for (TempIndex = 0; TempIndex < ETH_RX_DESC_NUMBER; TempIndex++)
    {
        /* Checks whether the assign of memory was successful */
        if (ETH_RxDescAssignMemory(&sEthInfo, TempIndex, aRxBuffer[TempIndex], NULL) != ETH_SUCCESS)
        {
            /* Assign memory failed for descriptor */
            return ERROR;
        }
    }

    /* Clear the sTxPacket structure variable */
    memset(&sTxPacket, 0, sizeof(ETH_TxPacketType));
    /* Set TxPacket-related config via sTxPacket */
    sTxPacket.Attributes   = ((uint32_t)ETH_TX_PACKETS_FEATURES_CSUM | (uint32_t)ETH_TX_PACKETS_FEATURES_CRCPAD);
    sTxPacket.CRCPadCtrl   = ETH_CRC_PAD_INSERT;
    sTxPacket.ChecksumCtrl = ETH_CHECKSUM_INSERT_IPHDR_PAYLOAD_PHDR_CALC;

    return SUCCESS;
}

/**
*\*\name    ETH_BSP_DeInit.
*\*\fun     ETH DeInitializes function.
*\*\param   none
*\*\return  none.
**/
void ETH_BSP_DeInit(void)
{
    /* Clear the sEthInfo structure variable */
    memset(&sEthInfo, 0, sizeof(ETH_InfoType));
    /* Clear the sETH_InitParam structure variable */
    memset(&sETH_InitParam, 0, sizeof(ETH_InitType));
    /* Clear the sTxPacket structure variable */
    memset(&sTxPacket, 0, sizeof(ETH_TxPacketType));
    /* DISABLE related clocks */
    ETH_BSP_ClockCmd(DISABLE);
}


