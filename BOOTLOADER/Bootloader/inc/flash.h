/**
 * @file    flash.h
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module handles the memory related functions.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#ifndef FLASH_H_
#define FLASH_H_


#include "n32h7xx_smu.h"


/* Start and end addresses of the user application. */


#define FLASH_PAGE_SIZE   256 
#define FLASH_SECTOR_SIZE 4096 

/*--------------------------------------------------------------------------------
---------N32H7xx series
---------boot 16K Bytes
---------N32H7xx 2M flash ----  16K boot + 880K  BankA + 1024K  BankB = 1920k
---------N32H7xx 4M flash ----- 16K boot + 1904K  BankA + 2048K  BankB = 3968k
---------------------------------------------------------------------------------*/
 
#define N32H7xx_2M_SERIES
//#define N32H7xx_4M_SERIES



#define BANK_AB_UPDATA     //A,B bank updata
#define USER_KEY_UPDATA    //user key need, mcu power-up or reset updata key need press longer than 5s
#define BOOT_START_ADDR    (0x15000000)
#define BOOT_END_ADDR      (0x15003FFF)        //16K
#define BOOT_LSAST_PAGE    (0x15004000 - 256)  //0x15003800
#define BOOT_LSAST_SECTOR  (0x15004000 - 4096) //0x15003000

#define BANKA_UPDATE_CNT_ADDR  (0x15004000 - 2)   //store number of upgrades of BankA
#define BANKB_UPDATE_CNT_ADDR  (0x15004000 - 4)   //store number of upgrades of BankB

#if defined N32H7xx_2M_SERIES
#define FLASH_BANKA_START_ADDR ((uint32_t)0x15004000u) //bankA 880K
#define FLASH_BANKA_END_ADDR   ((uint32_t)0x150DFFFFu)
#define FLASH_BANKB_START_ADDR ((uint32_t)0x150E0000u) //bankB 1024K
#define FLASH_BANKB_END_ADDR   ((uint32_t)0x151DFFFFu)
#elif defined  N32H7xx_4M_SERIES
#define FLASH_BANKA_START_ADDR ((uint32_t)0x15004000u) //bankA 1904K
#define FLASH_BANKA_END_ADDR   ((uint32_t)0x151DFFFFu) //
#define FLASH_BANKB_START_ADDR ((uint32_t)0x151E0000u) //bankB 2048K
#define FLASH_BANKB_END_ADDR   ((uint32_t)0x152E0000u)

#endif





//#define FLASH_APP_END_ADDRESS   ((uint32_t)FLASH_BANK1_END-0x10u) /**< Leave a little extra space at the end. */

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x15000000) /* Base @ of Sector 0, 4 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x15001000) /* Base @ of Sector 1, 8 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x15002000) /* Base @ of Sector 2, 12 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x15003000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x15004000) /* Base @ of Sector 4, 20 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x15005000) /* Base @ of Sector 5, 24 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x15006000) /* Base @ of Sector 6, 28 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x15007000) /* Base @ of Sector 7, 32 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x15008000) /* Base @ of Sector 8, 36 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x15009000) /* Base @ of Sector 9, 40 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x1500a000) /* Base @ of Sector 10, 44 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x1500b000) /* Base @ of Sector 11, 48 Kbytes */


#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_4   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     FLASH_BANKA_END_ADDR 



/* Status report for the functions. */
typedef enum {
  FLASH_OK              = 0x00u, /**< The action was successful. */
  FLASH_ERROR_SIZE      = 0x01u, /**< The binary is too big. */
  FLASH_ERROR_WRITE     = 0x02u, /**< Writing failed. */
  FLASH_ERROR_READBACK  = 0x04u, /**< Writing was successful, but the content of the memory is wrong. */
  FLASH_ERROR           = 0xFFu  /**< Generic error. */
} flash_status;

flash_status flash_erase(uint32_t address);
flash_status flash_write(uint32_t address, uint32_t *data, uint32_t length);


//void flash_jump_to_app(void);

void wite_pattern_crc(uint16_t crc_16);

#endif /* FLASH_H_ */
