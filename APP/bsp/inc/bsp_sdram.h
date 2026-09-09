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
*\*\file bsp_sdram.h
*\*\author Nations
*\*\version v1.0.0
*\*\copyright Copyright (c) 2025, Nations Technologies Inc. All rights reserved.
**/

#ifndef __BSP_SDRAM_H__
#define __BSP_SDRAM_H__

#include "n32h7xx.h"
#include "n32h7xx_sdram.h"
#include "n32h7xx_eval.h"

/* Address  */
#define SDRAM1_ADDR ((uint32_t)0xC0000000) // 0xC0000000~CFFFFFFF ->total 256M
                                           // 0xC0000000~C3FFFFFF : bank1 64M
                                           // 0xC4000000~C7FFFFFF : bank2 64M
                                           // 0xC8000000~CBFFFFFF : bank3 64M
                                           // 0xCC000000~CFFFFFFF : bank4 64M
#define SDRAM2_ADDR ((uint32_t)0xD0000000) // 0xD0000000~DFFFFFFF ->total 256M


/** Defines the Load Mode Register parameters: M2:0**/
typedef enum
{
    LOADMODE_BURSTLEN_1             = 0,
    LOADMODE_BURSTLEN_2             = 1, 
    LOADMODE_BURSTLEN_4             = 2,
    LOADMODE_BURSTLEN_8             = 3,
    LOADMODE_BURSTLEN_FULLPAGE      = 7    
} SDRAM_LoadMode_BurstLen;

/** Defines the Load Mode Register parameters: M3**/
typedef enum
{
    LOADMODE_BURSTTYP_SEQUENTIAL             = 0,
    LOADMODE_BURSTTYP_INTERLEAVED            = 1                     
} SDRAM_LoadMode_BurstType;

/** Defines the Load Mode Register parameters: M6:4**/
typedef enum
{
    LOADMODE_CASLATENCY_1             = 1,
    LOADMODE_CASLATENCY_2             = 2,
    LOADMODE_CASLATENCY_3             = 3   
} SDRAM_LoadMode_CASLatency;

/** Defines the Load Mode Register parameters: M8:7**/
typedef enum
{
    LOADMODE_STANDARD             = 0,
    LOADMODE_OTHER                = 1                     
} SDRAM_LoadMode_OPModeType;

/** Defines the Load Mode Register parameters: M9**/
typedef enum
{
    LOADMODE_WRITE_BURST             = 0,
    LOADMODE_WRITE_SINGLE            = 1                     
} SDRAM_LoadMode_WBModeType;

/** Defines the SDRAM Load Mode register content          **/
typedef union
{
    uint32_t cmd;
    struct
    {
        uint32_t BurstLen   : 3;
        uint32_t BurstType  : 1;
        uint32_t CASLatency : 3;
        uint32_t OpMode     : 2;
        uint32_t WBMode     : 1;
        uint32_t Reserved                    : 22;
    }Bits;
} SDRAM_LoadModeRegisterType;

/**  SDRAM Operation structure definition   **/
typedef struct
{
    FunctionalState ClockEnable;             /* Enables the SDRAM device clock.   */
    
    uint32_t OperationCode;                  /* Defines the operation code        */
    
    uint32_t ChipSelect;                     /* Defines SDRAM1, or SDRAM2, or All will select  */
    
    uint32_t BankAddress;                    /* Defines the bank address issued to external memory bus.  */          
     
    SDRAM_LoadModeRegisterType Address;      /* Defines the address(bank-row-column) issued to external memory bus.  */   
    
} SDRAM_OperationInitType;


/**  SDRAM Configuration structure definition   **/
typedef struct
{
    FunctionalState SdramEnable;             /* Enables the SDRAM 1 or 2 device.   */
    
    FunctionalState RefreshEnable;             /* Enables the SDRAM Refresh commands .   */
    
    FunctionalState AutoPrechargeEnable;       /* Enables the SDRAM Auto precharge.   */
    
    uint32_t        BusWidth;                  /* Defines the Bus width        */
    
    uint32_t        BurstLength;               /* Defines the SDRAM Burst length.   */
    
    uint32_t        CAS_Latency;               /* Defines the SDRAM CAS latency.   */
    
    FunctionalState PrefetchReadEnable;        /* Enables the SDRAM Prefetch read.   */
    
    FunctionalState SOM_Enable;                /* Enables the SDRAM Store On Miss(SOM) .   */
    
    FunctionalState BankInterleavEnable;       /* Enables the SDRAM Bank interleaving.   */
    
    uint32_t        AddressConfig;             /* Defines the SDRAM Address configuration.   */   
    
} SDRAM_ConfigurationInitType; 

#define SDRAM_AFIO_DRIVER
//#define SDRAM1_REMAP

#define SDRAM_M12L128324A            0/* 16Mbyte-4096row-256col-32bit,SDRAM2*/
#define SDRAM_W9825G6KH              1/* 32Mbyte-8192row-512col-16bit,SDRAM1 */
#define SDRAM_IS42S32800J_HMI        2/* 32Mbyte-4096row-512col-32bit,SDRAM1 */
#define SDRAM_IS42S32800J_EVB        3/* 32Mbyte-4096row-512col-32bit,SDRAM1 */
#define SDRAM_W9812G6KH_EVB_PRO      4/* 16Mbyte-4096row-512col-16bit,SDRAM1 */

/* Matching different evaluation boards */
#if DEMO_BOARD == N32H787_EVB
#define SDRAM_DEVICE  SDRAM_IS42S32800J_EVB
#elif DEMO_BOARD == N32H787_HMI
#define SDRAM_DEVICE  SDRAM_IS42S32800J_HMI
#elif DEMO_BOARD == N32H787_EVB_PRO
#define SDRAM_DEVICE  SDRAM_W9812G6KH_EVB_PRO
#else
#define SDRAM_DEVICE  SDRAM_M12L128324A
#endif

#if (SDRAM_DEVICE == SDRAM_M12L128324A)
#define SDRAM_CS_SDRAMx SDRAM_CS_SDRAM2_ONLY
#else
#define SDRAM_CS_SDRAMx SDRAM_CS_SDRAM1_ONLY
#endif

/* ADD pin */
#define SDRAM_A0_PORT        	   GPIOF
#define SDRAM_A0_PIN               GPIO_PIN_0
#define SDRAM_A0_AF			       GPIO_AF0
#define AFIO_SDRAM_A0		       AFIO_HIGH_SPEED_IO_PF0

#define SDRAM_A1_PORT        	   GPIOF
#define SDRAM_A1_PIN               GPIO_PIN_1
#define SDRAM_A1_AF			       GPIO_AF0
#define AFIO_SDRAM_A1		       AFIO_HIGH_SPEED_IO_PF1

#define SDRAM_A2_PORT        	   GPIOF
#define SDRAM_A2_PIN               GPIO_PIN_2
#define SDRAM_A2_AF			       GPIO_AF0
#define AFIO_SDRAM_A2		       AFIO_HIGH_SPEED_IO_PF2

#define SDRAM_A3_PORT        	   GPIOF
#define SDRAM_A3_PIN               GPIO_PIN_3
#define SDRAM_A3_AF			       GPIO_AF0
#define AFIO_SDRAM_A3		       AFIO_HIGH_SPEED_IO_PF3

#define SDRAM_A4_PORT        	   GPIOF
#define SDRAM_A4_PIN               GPIO_PIN_4
#define SDRAM_A4_AF			       GPIO_AF0
#define AFIO_SDRAM_A4		       AFIO_HIGH_SPEED_IO_PF4

#define SDRAM_A5_PORT        	   GPIOF
#define SDRAM_A5_PIN               GPIO_PIN_5
#define SDRAM_A5_AF			       GPIO_AF0
#define AFIO_SDRAM_A5		       AFIO_HIGH_SPEED_IO_PF5

#define SDRAM_A6_PORT        	   GPIOF
#define SDRAM_A6_PIN               GPIO_PIN_12
#define SDRAM_A6_AF			       GPIO_AF0
#define AFIO_SDRAM_A6		       AFIO_HIGH_SPEED_IO_PF12

#define SDRAM_A7_PORT        	   GPIOF
#define SDRAM_A7_PIN               GPIO_PIN_13
#define SDRAM_A7_AF			       GPIO_AF0
#define AFIO_SDRAM_A7		       AFIO_HIGH_SPEED_IO_PF13

#define SDRAM_A8_PORT        	   GPIOF
#define SDRAM_A8_PIN               GPIO_PIN_14
#define SDRAM_A8_AF			       GPIO_AF0
#define AFIO_SDRAM_A8		       AFIO_HIGH_SPEED_IO_PF14

#define SDRAM_A9_PORT        	   GPIOF
#define SDRAM_A9_PIN               GPIO_PIN_15
#define SDRAM_A9_AF			       GPIO_AF0
#define AFIO_SDRAM_A9		       AFIO_HIGH_SPEED_IO_PF15

#define SDRAM_A10_PORT        	   GPIOG
#define SDRAM_A10_PIN              GPIO_PIN_0
#define SDRAM_A10_AF			   GPIO_AF0
#define AFIO_SDRAM_A10		       AFIO_HIGH_SPEED_IO_PG0

#define SDRAM_A11_PORT        	   GPIOG
#define SDRAM_A11_PIN              GPIO_PIN_1
#define SDRAM_A11_AF			   GPIO_AF0
#define AFIO_SDRAM_A11		       AFIO_HIGH_SPEED_IO_PG1

#if (SDRAM_DEVICE == SDRAM_W9825G6KH) || (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
#define SDRAM_A12_PORT        	   GPIOG
#define SDRAM_A12_PIN              GPIO_PIN_2
#define SDRAM_A12_AF			   GPIO_AF0
#define AFIO_SDRAM_A12		       AFIO_HIGH_SPEED_IO_PG2
#endif

/* DATA pin */
#define SDRAM_D0_PORT        	   GPIOD
#define SDRAM_D0_PIN               GPIO_PIN_14
#define SDRAM_D0_AF			       GPIO_AF0
#define AFIO_SDRAM_D0		       AFIO_HIGH_SPEED_IO_PD14

#define SDRAM_D1_PORT        	   GPIOD
#define SDRAM_D1_PIN               GPIO_PIN_15
#define SDRAM_D1_AF			       GPIO_AF0
#define AFIO_SDRAM_D1		       AFIO_HIGH_SPEED_IO_PD15

#define SDRAM_D2_PORT        	   GPIOD
#define SDRAM_D2_PIN               GPIO_PIN_0
#define SDRAM_D2_AF			       GPIO_AF0
#define AFIO_SDRAM_D2		       AFIO_HIGH_SPEED_IO_PD0

#define SDRAM_D3_PORT        	   GPIOD
#define SDRAM_D3_PIN               GPIO_PIN_1
#define SDRAM_D3_AF			       GPIO_AF0
#define AFIO_SDRAM_D3		       AFIO_HIGH_SPEED_IO_PD1

#define SDRAM_D4_PORT        	   GPIOE
#define SDRAM_D4_PIN               GPIO_PIN_7
#define SDRAM_D4_AF			       GPIO_AF0
#define AFIO_SDRAM_D4		       AFIO_HIGH_SPEED_IO_PE7

#define SDRAM_D5_PORT        	   GPIOE
#define SDRAM_D5_PIN               GPIO_PIN_8
#define SDRAM_D5_AF			       GPIO_AF0
#define AFIO_SDRAM_D5		       AFIO_HIGH_SPEED_IO_PE8

#define SDRAM_D6_PORT        	   GPIOE
#define SDRAM_D6_PIN               GPIO_PIN_9
#define SDRAM_D6_AF			       GPIO_AF0
#define AFIO_SDRAM_D6		       AFIO_HIGH_SPEED_IO_PE9

#define SDRAM_D7_PORT        	   GPIOE
#define SDRAM_D7_PIN               GPIO_PIN_10
#define SDRAM_D7_AF			       GPIO_AF0
#define AFIO_SDRAM_D7		       AFIO_HIGH_SPEED_IO_PE10

#define SDRAM_D8_PORT        	   GPIOE
#define SDRAM_D8_PIN               GPIO_PIN_11
#define SDRAM_D8_AF			       GPIO_AF0
#define AFIO_SDRAM_D8		       AFIO_HIGH_SPEED_IO_PE11

#if (SDRAM_DEVICE == SDRAM_W9825G6KH) || (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
#define SDRAM_D9_PORT        	   GPIOE
#define SDRAM_D9_PIN               GPIO_PIN_12
#define SDRAM_D9_AF			       GPIO_AF0
#define AFIO_SDRAM_D9		       AFIO_HIGH_SPEED_IO_PE12
#else
#define SDRAM_D9_PORT        	   GPIOA
#define SDRAM_D9_PIN               GPIO_PIN_5
#define SDRAM_D9_AF			       GPIO_AF0
#define AFIO_SDRAM_D9		       AFIO_HIGH_SPEED_IO_PA5
#endif

#define SDRAM_D10_PORT        	   GPIOE
#define SDRAM_D10_PIN              GPIO_PIN_13
#define SDRAM_D10_AF			   GPIO_AF0
#define AFIO_SDRAM_D10		       AFIO_HIGH_SPEED_IO_PE13

#define SDRAM_D11_PORT        	   GPIOE
#define SDRAM_D11_PIN              GPIO_PIN_14
#define SDRAM_D11_AF			   GPIO_AF0
#define AFIO_SDRAM_D11		       AFIO_HIGH_SPEED_IO_PE14

#if (SDRAM_DEVICE == SDRAM_W9825G6KH) || (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
#define SDRAM_D12_PORT        	   GPIOE
#define SDRAM_D12_PIN              GPIO_PIN_15
#define SDRAM_D12_AF			   GPIO_AF0
#define AFIO_SDRAM_D12		       AFIO_HIGH_SPEED_IO_PE15
#else
#define SDRAM_D12_PORT        	   GPIOC
#define SDRAM_D12_PIN              GPIO_PIN_0
#define SDRAM_D12_AF			   GPIO_AF1
#define AFIO_SDRAM_D12		       AFIO_HIGH_SPEED_IO_PC0
#endif

#define SDRAM_D13_PORT        	   GPIOD
#define SDRAM_D13_PIN              GPIO_PIN_8
#define SDRAM_D13_AF		       GPIO_AF0
#define AFIO_SDRAM_D13		       AFIO_HIGH_SPEED_IO_PD8

#define SDRAM_D14_PORT        	   GPIOD
#define SDRAM_D14_PIN              GPIO_PIN_9
#define SDRAM_D14_AF	           GPIO_AF0
#define AFIO_SDRAM_D14		       AFIO_HIGH_SPEED_IO_PD9

#define SDRAM_D15_PORT        	   GPIOD
#define SDRAM_D15_PIN              GPIO_PIN_10
#define SDRAM_D15_AF		       GPIO_AF0
#define AFIO_SDRAM_D15		       AFIO_HIGH_SPEED_IO_PD10

#if (SDRAM_DEVICE == SDRAM_M12L128324A) || (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_IS42S32800J_EVB)
#define SDRAM_D16_PORT        	   GPIOH
#define SDRAM_D16_PIN              GPIO_PIN_8
#define SDRAM_D16_AF		       GPIO_AF0
#define AFIO_SDRAM_D16		       AFIO_HIGH_SPEED_IO_PH8

#define SDRAM_D17_PORT        	   GPIOH
#define SDRAM_D17_PIN              GPIO_PIN_9
#define SDRAM_D17_AF		       GPIO_AF0
#define AFIO_SDRAM_D17		       AFIO_HIGH_SPEED_IO_PH9

#define SDRAM_D18_PORT        	   GPIOH
#define SDRAM_D18_PIN              GPIO_PIN_10
#define SDRAM_D18_AF		       GPIO_AF0
#define AFIO_SDRAM_D18		       AFIO_HIGH_SPEED_IO_PH10

#define SDRAM_D19_PORT        	   GPIOH
#define SDRAM_D19_PIN              GPIO_PIN_11
#define SDRAM_D19_AF		       GPIO_AF0
#define AFIO_SDRAM_D19		       AFIO_HIGH_SPEED_IO_PH11

#define SDRAM_D20_PORT        	   GPIOH
#define SDRAM_D20_PIN              GPIO_PIN_12
#define SDRAM_D20_AF		       GPIO_AF0
#define AFIO_SDRAM_D20		       AFIO_HIGH_SPEED_IO_PH12

#define SDRAM_D21_PORT        	   GPIOH
#define SDRAM_D21_PIN              GPIO_PIN_13
#define SDRAM_D21_AF		       GPIO_AF0
#define AFIO_SDRAM_D21		       AFIO_HIGH_SPEED_IO_PH13

#define SDRAM_D22_PORT        	   GPIOH
#define SDRAM_D22_PIN              GPIO_PIN_14
#define SDRAM_D22_AF		       GPIO_AF0
#define AFIO_SDRAM_D22		       AFIO_HIGH_SPEED_IO_PH14

#define SDRAM_D23_PORT        	   GPIOH
#define SDRAM_D23_PIN              GPIO_PIN_15
#define SDRAM_D23_AF		       GPIO_AF0
#define AFIO_SDRAM_D23		       AFIO_HIGH_SPEED_IO_PH15

#define SDRAM_D24_PORT        	   GPIOI
#define SDRAM_D24_PIN              GPIO_PIN_0
#define SDRAM_D24_AF		       GPIO_AF0
#define AFIO_SDRAM_D24		       AFIO_HIGH_SPEED_IO_PI0

#define SDRAM_D25_PORT        	   GPIOI
#define SDRAM_D25_PIN              GPIO_PIN_1
#define SDRAM_D25_AF		       GPIO_AF0
#define AFIO_SDRAM_D25		       AFIO_HIGH_SPEED_IO_PI1

#define SDRAM_D26_PORT        	   GPIOI
#define SDRAM_D26_PIN              GPIO_PIN_2
#define SDRAM_D26_AF		       GPIO_AF0
#define AFIO_SDRAM_D26		       AFIO_HIGH_SPEED_IO_PI2

#define SDRAM_D27_PORT        	   GPIOI
#define SDRAM_D27_PIN              GPIO_PIN_3
#define SDRAM_D27_AF		       GPIO_AF0
#define AFIO_SDRAM_D27		       AFIO_HIGH_SPEED_IO_PI3

#define SDRAM_D28_PORT        	   GPIOI
#define SDRAM_D28_PIN              GPIO_PIN_6
#define SDRAM_D28_AF		       GPIO_AF0
#define AFIO_SDRAM_D28		       AFIO_HIGH_SPEED_IO_PI6

#define SDRAM_D29_PORT        	   GPIOI
#define SDRAM_D29_PIN              GPIO_PIN_7
#define SDRAM_D29_AF		       GPIO_AF0
#define AFIO_SDRAM_D29		       AFIO_HIGH_SPEED_IO_PI7

#define SDRAM_D30_PORT        	   GPIOI
#define SDRAM_D30_PIN              GPIO_PIN_9
#define SDRAM_D30_AF		       GPIO_AF0
#define AFIO_SDRAM_D30		       AFIO_HIGH_SPEED_IO_PI9

#define SDRAM_D31_PORT        	   GPIOI
#define SDRAM_D31_PIN              GPIO_PIN_10
#define SDRAM_D31_AF		       GPIO_AF0
#define AFIO_SDRAM_D31		       AFIO_HIGH_SPEED_IO_PI10
#endif

/* BA signal pin */
#define SDRAM_BA0_PORT        	   GPIOG
#define SDRAM_BA0_PIN              GPIO_PIN_4
#define SDRAM_BA0_AF			   GPIO_AF0
#define AFIO_SDRAM_BA0		       AFIO_HIGH_SPEED_IO_PG4

#define SDRAM_BA1_PORT        	   GPIOG
#define SDRAM_BA1_PIN              GPIO_PIN_5
#define SDRAM_BA1_AF			   GPIO_AF0
#define AFIO_SDRAM_BA1		       AFIO_HIGH_SPEED_IO_PG5

/* NCE signal pin */
#if (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
#define SDRAM_NCE0_PORT        	   GPIOC //SDRAM1
#define SDRAM_NCE0_PIN             GPIO_PIN_2
#define SDRAM_NCE0_AF			   GPIO_AF0
#define AFIO_SDRAM_NCE0		       AFIO_HIGH_SPEED_IO_PC2
#elif SDRAM_DEVICE == SDRAM_IS42S32800J_EVB
#define SDRAM_NCE0_PORT        	   GPIOH //SDRAM1
#define SDRAM_NCE0_PIN             GPIO_PIN_3
#define SDRAM_NCE0_AF			   GPIO_AF0
#define AFIO_SDRAM_NCE0		       AFIO_HIGH_SPEED_IO_PH3
#else
#define SDRAM_NCE0_PORT        	   GPIOC //SDRAM1
#define SDRAM_NCE0_PIN             GPIO_PIN_4
#define SDRAM_NCE0_AF			   GPIO_AF0
#define AFIO_SDRAM_NCE0		       AFIO_HIGH_SPEED_IO_PC4
#endif

#define SDRAM_NCE1_PORT        	   GPIOB //SDRAM2
#define SDRAM_NCE1_PIN             GPIO_PIN_6
#define SDRAM_NCE1_AF			   GPIO_AF0
#define AFIO_SDRAM_NCE1		       AFIO_HIGH_SPEED_IO_PB6

/* NWE pin */
#if (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO)
#define SDRAM_NWE_PORT        	   GPIOC
#define SDRAM_NWE_PIN              GPIO_PIN_0
#define SDRAM_NWE_AF			   GPIO_AF0
#define AFIO_SDRAM_NWE		       AFIO_HIGH_SPEED_IO_PC0
#else
#define SDRAM_NWE_PORT        	   GPIOH
#define SDRAM_NWE_PIN              GPIO_PIN_5
#define SDRAM_NWE_AF			   GPIO_AF0
#define AFIO_SDRAM_NWE		       AFIO_HIGH_SPEED_IO_PH5
#endif

/* NRAS pin */
#define SDRAM_NRAS_PORT        	   GPIOF
#define SDRAM_NRAS_PIN             GPIO_PIN_11
#define SDRAM_NRAS_AF			   GPIO_AF0
#define AFIO_SDRAM_NRAS		       AFIO_HIGH_SPEED_IO_PF11

/* NCAS pin */
#define SDRAM_NCAS_PORT        	   GPIOG
#define SDRAM_NCAS_PIN             GPIO_PIN_15
#define SDRAM_NCAS_AF			   GPIO_AF0
#define AFIO_SDRAM_NCAS		       AFIO_HIGH_SPEED_IO_PG15

/* DQM signal pin */
#define SDRAM_DQM0_PORT        	   GPIOE
#define SDRAM_DQM0_PIN             GPIO_PIN_0
#define SDRAM_DQM0_AF			   GPIO_AF0
#define AFIO_SDRAM_DQM0		       AFIO_HIGH_SPEED_IO_PE0

#define SDRAM_DQM1_PORT        	   GPIOE
#define SDRAM_DQM1_PIN             GPIO_PIN_1
#define SDRAM_DQM1_AF			   GPIO_AF0
#define AFIO_SDRAM_DQM1		       AFIO_HIGH_SPEED_IO_PE1

#define SDRAM_DQM2_PORT        	   GPIOI
#define SDRAM_DQM2_PIN             GPIO_PIN_4
#define SDRAM_DQM2_AF			   GPIO_AF0
#define AFIO_SDRAM_DQM2		       AFIO_HIGH_SPEED_IO_PI4

#define SDRAM_DQM3_PORT        	   GPIOI
#define SDRAM_DQM3_PIN             GPIO_PIN_5
#define SDRAM_DQM3_AF			   GPIO_AF0
#define AFIO_SDRAM_DQM3		       AFIO_HIGH_SPEED_IO_PI5

/* CKE signal pin */
#if (SDRAM_DEVICE == SDRAM_IS42S32800J_HMI) || (SDRAM_DEVICE == SDRAM_IS42S32800J_EVB)
#define SDRAM_CKE0_PORT        	   GPIOH //SDRAM1
#define SDRAM_CKE0_PIN             GPIO_PIN_2
#define SDRAM_CKE0_AF			   GPIO_AF0
#define AFIO_SDRAM_CKE0		       AFIO_HIGH_SPEED_IO_PH2
#elif SDRAM_DEVICE == SDRAM_W9812G6KH_EVB_PRO
#define SDRAM_CKE0_PORT        	   GPIOC //SDRAM1
#define SDRAM_CKE0_PIN             GPIO_PIN_3
#define SDRAM_CKE0_AF			   GPIO_AF0
#define AFIO_SDRAM_CKE0		       AFIO_HIGH_SPEED_IO_PC3
#else
#define SDRAM_CKE0_PORT        	   GPIOC //SDRAM1
#define SDRAM_CKE0_PIN             GPIO_PIN_5
#define SDRAM_CKE0_AF			   GPIO_AF0
#define AFIO_SDRAM_CKE0		       AFIO_HIGH_SPEED_IO_PC5
#endif

#define SDRAM_CKE1_PORT        	   GPIOB //SDRAM2
#define SDRAM_CKE1_PIN             GPIO_PIN_5
#define SDRAM_CKE1_AF			   GPIO_AF0
#define AFIO_SDRAM_CKE1		       AFIO_HIGH_SPEED_IO_PB5

/* CLK pin */
#define SDRAM_CLK_PORT        	   GPIOG
#define SDRAM_CLK_PIN              GPIO_PIN_8
#define SDRAM_CLK_AF			   GPIO_AF0
#define AFIO_SDRAM_CLK		       AFIO_HIGH_SPEED_IO_PG8

void SDRAM_RCC_Configuration(void);
void SDRAM_GPIO_Init(void);
void SDRAM_DeviceInit(void);

void SDRAM_OperationInit(SDRAM_OperationInitType SDRAM_OS);
void SDRAM_ConfigurationInit(SDRAM_DeviceType DeviceNo, SDRAM_ConfigurationInitType SDRAM_CFG);


#endif //__BSP_SDRAM_H__
