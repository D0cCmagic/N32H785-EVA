#ifndef _IAP_H__
#define _IAP_H__

#include "n32h7xx.h"
#include "flash.h"


extern uint32_t FLASH_APP_START_ADDRESS ;
extern uint32_t FLASH_APP_END_ADDRESS   ;

typedef enum {
  FLASH_BANKA  = 0x00u, 
  FLASH_BANKB  = 0x01u 
} flash_bank;



#define FLASH_TIMEOUT_VALUE (0x000B0000)
#define ITCM_BASE_ADDR      (0x00000000)
#define COPY_SIZE           (0x00010000)


void SystemNVICReset(void);
flash_bank check_bank(uint8_t bank_select);
void check_update(void);
void Jump_To_App(uint32_t address);


void updata_key_gpio_init(void);
void flash_jump_to_app(uint32_t address);

ErrorStatus check_app_crc(uint32_t app_start_addr, uint32_t app_end_addr);






#endif


