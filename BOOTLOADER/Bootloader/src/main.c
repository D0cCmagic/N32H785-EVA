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
 *\*\file main.c
 *\*\author Nations
 *\*\version v1.1.0
 *\*\copyright Copyright (c) 2025, Nations Technologies Inc. All rights reserved.
 **/

#include "main.h"
#include "delay.h"
#include "uart.h"
#include "IAP.h"

void GPIO_Configuration(void);
void RCC_Configuration(void);
/**
 *\*\name   main.
 *\*\fun    Main program.
 *\*\param  none
 *\*\return none
 */
int main(void)
{
    uint8_t  DATD = 100;
    CopyVectTable(FLASH_BASE, SRAM_BASE, VECT_SIZE);    
    /* Initialize system clock */
    RCC_SetSysClkToMode0();
    /* RCC configuration -------------------------------------------------------*/
    RCC_Configuration();
    /* Uart configuration -------------------------------------------------------*/
    uart_init();
    /* GPIO configuration ------------------------------------------------------*/
    GPIO_Configuration();
    
    printf("\r\n************************ N32h7xx uart IAP ************************\r\n");
    
    /* Check for 5s, if needed updated, excute download process ----------------*/
    check_update();
    
    while (1)
    {
        printf("\r\nWait a minute, crc32 calculating ...... \r\n");
                
        if((check_app_crc(FLASH_APP_START_ADDRESS, FLASH_APP_END_ADDRESS)) == SUCCESS)
        {
            flash_jump_to_app(FLASH_APP_START_ADDRESS);	 //Jump to app
        }
        else
        {
            printf("Error, APP CRC check err, app start_addr: 0x%08x, app end_addr: 0x%08x\r\n", FLASH_APP_START_ADDRESS, FLASH_APP_END_ADDRESS);
        }    
    }
}

/**
*\*\name    GPIO_Configuration.
*\*\fun     Configures the different GPIO ports.
*\*\return  none
**/
void GPIO_Configuration(void)
{
    GPIO_InitType GPIO_InitStructure;
    /* LED3 configuration ------------------------------------------------------*/
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin     = LED3_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitPeripheral( LED3_PORT, &GPIO_InitStructure);
    /* Updata key(WKUP) configuration ------------------------------------------*/    
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin       = UPDATA_KEY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Pull = GPIO_NO_PULL;
    GPIO_InitPeripheral( UPDATA_KEY_PORT, &GPIO_InitStructure);    
    
}
/**
*\*\name    RCC_Configuration.
*\*\fun     Configures the different system clocks.
*\*\return  none
**/
void RCC_Configuration(void)
{
    /* Enable peripheral clocks ------------------------------------------------*/
    /* Enable LED2 and Updata key(WKUP) clocks */
    RCC_EnableAHB5PeriphClk1(LED3_CLOCK, ENABLE);
}


