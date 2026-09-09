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
*\*\file log.c
*\*\author Nations
*\*\version v1.0.0
*\*\copyright Copyright (c) 2025, Nations Technologies Inc. All rights reserved.
**/

#include "log.h"

#if LOG_ENABLE

#include "n32h7xx.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_usart.h"
#include "n32h7xx_rcc.h"
#include "n32h7xx_eval.h"

#ifdef CORE_CM7
    #define  LOG_USARTx                     USART1
    #define  DEBUG_USART_CLK                (RCC_APB1_PERIPHEN_M7_USART1)
    #define  DEBUG_USART_APBxClkCmd         RCC_EnableAPB1PeriphClk3
    #define  DEBUG_USART_BAUDRATE           (115200)


    #define  DEBUG_USART_GPIO_CLK           (RCC_AHB5_PERIPHEN_M7_GPIOA| RCC_AHB5_PERIPHEN_M7_GPIOB )
    #define  DEBUG_USART_GPIO_APBxClkCmd     RCC_EnableAHB5PeriphClk1

    #define  DEBUG_USART_TX_GPIO_PORT       GPIOA
    #define  DEBUG_USART_TX_GPIO_PIN        GPIO_PIN_9
    #define  DEBUG_USART_RX_GPIO_PORT       GPIOA
    #define  DEBUG_USART_RX_GPIO_PIN        GPIO_PIN_10

    #define  DEBUG_USART_TX_GPIO_AF           ((uint8_t)0x07)
    #define  DEBUG_USART_RX_GPIO_AF           ((uint8_t)0x05)
#else

    #if (DEMO_BOARD == N32H787_EVB)
        #define  LOG_USARTx                     UART9
        #define  DEBUG_USART_CLK                (RCC_APB1_PERIPHEN_M4_UART9)
        #define  DEBUG_USART_APBxClkCmd         RCC_EnableAPB1PeriphClk3
        #define  DEBUG_USART_BAUDRATE           (115200)


        #define  DEBUG_USART_GPIO_CLK           (RCC_AHB5_PERIPHEN_M4_GPIOA )
        #define  DEBUG_USART_GPIO_APBxClkCmd     RCC_EnableAHB5PeriphClk1

        #define  DEBUG_USART_TX_GPIO_PORT       GPIOA
        #define  DEBUG_USART_TX_GPIO_PIN        GPIO_PIN_12
        #define  DEBUG_USART_RX_GPIO_PORT       GPIOA
        #define  DEBUG_USART_RX_GPIO_PIN        GPIO_PIN_11

        #define  DEBUG_USART_TX_GPIO_AF           ((uint8_t)0x07)
        #define  DEBUG_USART_RX_GPIO_AF           ((uint8_t)0x07)
    
    #elif (DEMO_BOARD == N32H787_HMI)
        #define  LOG_USARTx                     UART14
        #define  DEBUG_USART_CLK                (RCC_APB2_PERIPHEN_M4_UART14)
        #define  DEBUG_USART_APBxClkCmd         RCC_EnableAPB2PeriphClk3
        #define  DEBUG_USART_BAUDRATE           (115200)


        #define  DEBUG_USART_GPIO_CLK           (RCC_AHB5_PERIPHEN_M4_GPIOH )
        #define  DEBUG_USART_GPIO_APBxClkCmd     RCC_EnableAHB5PeriphClk1

        #define  DEBUG_USART_TX_GPIO_PORT       GPIOH
        #define  DEBUG_USART_TX_GPIO_PIN        GPIO_PIN_7
        #define  DEBUG_USART_RX_GPIO_PORT       GPIOH
        #define  DEBUG_USART_RX_GPIO_PIN        GPIO_PIN_6

        #define  DEBUG_USART_TX_GPIO_AF           ((uint8_t)0x07)
        #define  DEBUG_USART_RX_GPIO_AF           ((uint8_t)0x09)
    #else
    
        #define  LOG_USARTx                     USART3
        #define  DEBUG_USART_CLK                (RCC_APB1_PERIPHEN_M4_USART3)
        #define  DEBUG_USART_APBxClkCmd         RCC_EnableAPB1PeriphClk3
        #define  DEBUG_USART_BAUDRATE           (115200)


        #define  DEBUG_USART_GPIO_CLK           (RCC_AHB5_PERIPHEN_M4_GPIOB )
        #define  DEBUG_USART_GPIO_APBxClkCmd     RCC_EnableAHB5PeriphClk1

        #define  DEBUG_USART_TX_GPIO_PORT       GPIOB
        #define  DEBUG_USART_TX_GPIO_PIN        GPIO_PIN_10
        #define  DEBUG_USART_RX_GPIO_PORT       GPIOB
        #define  DEBUG_USART_RX_GPIO_PIN        GPIO_PIN_11

        #define  DEBUG_USART_TX_GPIO_AF           ((uint8_t)0x09)
        #define  DEBUG_USART_RX_GPIO_AF           ((uint8_t)0x06)
        
    #endif
#endif

void log_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;

    DEBUG_USART_GPIO_APBxClkCmd( DEBUG_USART_GPIO_CLK, ENABLE);

    DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);	
    GPIO_InitStructure.Pin          = DEBUG_USART_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate   = DEBUG_USART_TX_GPIO_AF;
    GPIO_InitPeripheral( DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure );


    GPIO_InitStructure.Pin          = DEBUG_USART_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate   = DEBUG_USART_RX_GPIO_AF;
    GPIO_InitPeripheral( DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure );
    
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.BaudRate = DEBUG_USART_BAUDRATE;
    USART_InitStructure.WordLength = USART_WL_8B;
    USART_InitStructure.StopBits = USART_STPB_1;
    USART_InitStructure.Parity = USART_PE_NO ;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode = USART_MODE_RX | USART_MODE_TX;
    USART_Init( LOG_USARTx, &USART_InitStructure );

    USART_Enable( LOG_USARTx, ENABLE );
}
static int is_lr_sent = 0;

int fputc(int ch, FILE* f)
{
    if (ch == '\r')
    {
        is_lr_sent = 1;
    }
    else if (ch == '\n')
    {
        if (!is_lr_sent)
        {
            /* Loop until the end of transmission */
            while (USART_GetFlagStatus(LOG_USARTx, USART_FLAG_TXC) == RESET)
            {
            }
            USART_SendData(LOG_USARTx, (uint8_t)'\r');
        }
        is_lr_sent = 0;
    }
    else
    {
        is_lr_sent = 0;
    }
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(LOG_USARTx, USART_FLAG_TXC) == RESET)
    {
    }
    USART_SendData(LOG_USARTx, (uint8_t)ch);
    return ch;
}


#endif // LOG_ENABLE
