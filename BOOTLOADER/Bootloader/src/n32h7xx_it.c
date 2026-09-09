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
 *\*\file n32h7xx_it.c
 *\*\author Nations
 *\*\version v1.0.0
 *\*\copyright Copyright (c) 2025, Nations Technologies Inc. All rights reserved.
 **/

#include "n32h7xx_it.h"
#include "n32h7xx.h"
#include "main.h"
#include "uart.h"

/***  Cortex-M7 Processor Exceptions Handlers ***/

/**
 *\*\name   NMI_Handler.
 *\*\fun    This function handles NMI exception.
 *\*\param  none
 *\*\return none
 */
void NMI_Handler(void)
{
}

/**
*\*\name    HardFault_Handler.
*\*\fun     This function handles Hard Fault exception.
*\*\return  None
**/
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
 *\*\name   MemManage_Handler.
 *\*\fun    This function handles Memory Manage exception.
 *\*\param  none
 *\*\return none
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 *\*\name   BusFault_Handler.
 *\*\fun    This function handles Bus Fault exception.
 *\*\param  none
 *\*\return none
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 *\*\name   UsageFault_Handler.
 *\*\fun    This function handles Usage Fault exception.
 *\*\param  none
 *\*\return none
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 *\*\name   SVC_Handler.
 *\*\fun    This function handles SVCall exception.
 *\*\param  none
 *\*\return none
 */
void SVC_Handler(void)
{
}

/**
 *\*\name   DebugMon_Handler.
 *\*\fun    This function handles Debug Monitor exception.
 *\*\param  none
 *\*\return none
 */
void DebugMon_Handler(void)
{
}

/**
 *\*\name   SysTick_Handler.
 *\*\fun    This function handles SysTick Handler.
 *\*\param  none
 *\*\return none
 */
void SysTick_Handler(void)
{
}


/*** N32H7xx Peripherals Interrupt Handlers,available peripheral interrupt 
    handler's name please refer to the startup file (startup_n32h7xx.s)***/

extern uint8_t rcvFlag;

extern void uart_dma_init(void);
extern uint8_t received_packet_data[X_PACKET_1024_SIZE];

uint16_t int_cnt = 0;

void USART1_IRQHandler(void)
{
    uint16_t usTemp = 0;

    if(USART_GetFlagStatus(USART1, USART_INT_IDLEF) != RESET)
    {
        int_cnt++;
        
        USART_ReceiveData(USART1);  
        DMA_ChannelCmd(DMA1, DMA_CHANNEL_0, DISABLE);

        DMA_ClearChannelEventStatus(DMA1, DMA_CHANNEL_0, DMA_CH_EVENT_TRANSFER_COMPLETE);
        usTemp = USARTx_MAX_UART_BUF - DMA_GetTransferredNumber(DMA1, DMA_CHANNEL_0);		
    

        DMA_SetChannelDestinationAddress(DMA1, DMA_CHANNEL_0, (uint32_t *)received_packet_data);
        DMA_SetChannelBlockSize(DMA1, DMA_CHANNEL_0, USARTx_MAX_UART_BUF);


        DMA_ChannelCmd(DMA1, DMA_CHANNEL_0, ENABLE);
    
        rcvFlag = 1;
    }
}


void DMA1_Channel0_IRQHandler(void)
{
    if (DMA_GetCombinedStatus(DMA1))
    {
        if (DMA_GetChannelIntSrcTranStatus(DMA1, DMA_CHANNEL_0) == SET)
        {
            /* Clear interrupt event status */
            DMA_ClearChannelEventStatus(DMA1, DMA_CHANNEL_0, DMA_CH_EVENT_SRC_TRANSACTION_COMPLETE);
        }
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetCombinedStatus(DMA1))
    {
        if (DMA_GetChannelIntTfrStatus(DMA1, DMA_CHANNEL_1) == SET)
        {
            /* Sets the global flag variable */
            //DataTransferCompletionFlag = 1U;
            
            /* Clear interrupt event status */
            DMA_ClearChannelEventStatus(DMA1, DMA_CHANNEL_1, DMA_CH_EVENT_TRANSFER_COMPLETE);
        }
    }
}


