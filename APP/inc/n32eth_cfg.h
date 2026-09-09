/**
*     Copyright (c) 2025, Nsing Technologies Inc.
* 
*     All rights reserved.
*
*     This software is the exclusive property of Nsing Technologies Inc. (Hereinafter 
* referred to as Nsing). This software, and the product of Nsing described herein 
* (Hereinafter referred to as the Product) are owned by Nsing under the laws and treaties
* of the People's Republic of China and other applicable jurisdictions worldwide.
*
*     Nsing does not grant any license under its patents, copyrights, trademarks, or other 
* intellectual property rights. Names and brands of third party may be mentioned or referred 
* thereto (if any) for identification purposes only.
*
*     Nsing reserves the right to make changes, corrections, enhancements, modifications, and 
* improvements to this software at any time without notice. Please contact Nsing and obtain 
* the latest version of this software before placing orders.

*     Although Nsing has attempted to provide accurate and reliable information, Nsing assumes 
* no responsibility for the accuracy and reliability of this software.
* 
*     It is the responsibility of the user of this software to properly design, program, and test 
* the functionality and safety of any application made of this information and any resulting product. 
* In no event shall Nsing be liable for any direct, indirect, incidental, special,exemplary, or 
* consequential damages arising in any way out of the use of this software or the Product.
*
*     Nsing Products are neither intended nor warranted for usage in systems or equipment, any
* malfunction or failure of which may cause loss of human life, bodily injury or severe property 
* damage. Such applications are deemed, "Insecure Usage".
*
*     All Insecure Usage shall be made at user's risk. User shall indemnify Nsing and hold Nsing 
* harmless from and against all claims, costs, damages, and other liabilities, arising from or related 
* to any customer's Insecure Usage.

*     Any express or implied warranty with regard to this software or the Product, including,but not 
* limited to, the warranties of merchantability, fitness for a particular purpose and non-infringement
* are disclaimed to the fullest extent permitted by law.

*     Unless otherwise explicitly permitted by Nsing, anyone may not duplicate, modify, transcribe
* or otherwise distribute this software for any purposes, in whole or in part.
*
*     Nsing products and technologies shall not be used for or incorporated into any products or systems
* whose manufacture, use, or sale is prohibited under any applicable domestic or foreign laws or regulations. 
* User shall comply with any applicable export control laws and regulations promulgated and administered by 
* the governments of any countries asserting jurisdiction over the parties or transactions.
**/

/**
*\*\file n32eth_cfg.h
*\*\author Nsing
*\*\version v1.0.0
*\*\copyright Copyright (c) 2025, Nsing Technologies Inc. All rights reserved.
**/

#ifndef __N32ETH_CFG_H__
#define __N32ETH_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "n32h7xx.h"
#include "n32h7xx_rcc.h"
#include "n32h7xx_pwr.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_exti.h"
#include "n32h7xx_usart.h"


/* Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0                    ((uint8_t)192U)
#define IP_ADDR1                    ((uint8_t)168U)
#define IP_ADDR2                    ((uint8_t)0U)
#define IP_ADDR3                    ((uint8_t)32U)

/* NETMASK: NETMASK_ADDR0.NETMASK_ADDR1.NETMASK_ADDR2.NETMASK_ADDR3 */
#define NETMASK_ADDR0               ((uint8_t)255U)
#define NETMASK_ADDR1               ((uint8_t)255U)
#define NETMASK_ADDR2               ((uint8_t)255U)
#define NETMASK_ADDR3               ((uint8_t)0U)

/* Gateway Address: GW_ADDR0.GW_ADDR1.GW_ADDR2.GW_ADDR3 */
#define GW_ADDR0                    ((uint8_t)192U)
#define GW_ADDR1                    ((uint8_t)168U)
#define GW_ADDR2                    ((uint8_t)0U)
#define GW_ADDR3                    ((uint8_t)1U)

/* ETH MAC address macro definition */
#define MAC_ADDR0                   ((uint8_t)0x10U)
#define MAC_ADDR1                   ((uint8_t)0x32U)
#define MAC_ADDR2                   ((uint8_t)0x54U)
#define MAC_ADDR3                   ((uint8_t)0x76U)
#define MAC_ADDR4                   ((uint8_t)0x89U)
#define MAC_ADDR5                   ((uint8_t)0xBAU)

/* Tx Descriptor Length: Minimum is 4, maximum is 1024 */
#define ETH_TX_DESC_NUMBER          ((uint32_t)5U)
/* Rx Descriptor Length: Minimum is 4, maximum is 1024 */
#define ETH_RX_DESC_NUMBER          ((uint32_t)5U)


#ifdef __cplusplus
}
#endif

#endif /* __N32ETH_CFG_H__ */


