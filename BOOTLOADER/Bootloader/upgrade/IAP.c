

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "uart.h"
#include "IAP.h"
#include "flash.h"
#include "xmodem.h"
#include "delay.h"
#include "crc.h"


typedef void (*pFunction)(void);
pFunction Jump_To_Application;

uint32_t FLASH_APP_START_ADDRESS = 0x15004000;  //user app start address
uint32_t FLASH_APP_END_ADDRESS   = 0x151DFFFF;  //user app end address

/* APP 升级请求标志（须与 APP 侧保持一致）：
 * 软复位后 SRAM 保持、断电后消失，bootloader 据此直接进入升级模式 */
#define OTA_FLAG_ADDR   (0x2401FF00u)          /* AXI SRAM 尾部约定地址（scatter 已预留） */
#define OTA_FLAG_MAGIC  (0x4F544100u)          /* "OTA\0" */

extern uint32_t received_packet_number_total; 

void SystemNVICReset(void)
{
    __set_FAULTMASK((uint32_t)1);
    __NVIC_SystemReset();
}



/**
 * @brief   This function erases the memory.
 * @param   address: First address to be erased (the last is the end of the flash).
 * @return  status: Report about the success of the erasing.
 */
flash_status flash_erase(uint32_t address)
{
    uint32_t erase_addr = 0;
 
    for(erase_addr = address; erase_addr < FLASH_APP_END_ADDRESS; erase_addr += FLASH_SECTOR_SIZE)
    {
        
        if (FLASH_SUCCESS != SMU_EraseFlash(erase_addr))
        {
            printf("\r\nERASE FAILED\r\n");
            return FLASH_ERROR;
        }
    }

    return FLASH_OK;
}


/**
 * @brief   This function flashes the memory.
 * @param   address: First address to be written to.
 * @param   *data:   Array of the data that we want to write.
 * @param   *length: Size of the array.
 * @return  status: Report about the success of the writing.
 */
flash_status flash_write(uint32_t address, uint32_t *data, uint32_t length)
{
    flash_status status = FLASH_OK;
    
    if (FLASH_SUCCESS != SMU_WriteFlash(address, (uint8_t*)data, length))
    {
        status = FLASH_ERROR_WRITE;
        return status;
    }
    
    for(uint32_t i = 0u; (i < length/4) && (FLASH_OK == status); i++)
    {
        /* Read back the content of the memory. If it is wrong, then report an error. */
        if(((data[i])) != (*(volatile uint32_t *)address))
        {
            status = FLASH_ERROR_READBACK;
            break;
        }
        
        address += 4u;
    }
    return status;
}



/**
 * @brief   This function check  bank A/B which will be use.
 * @param   bank_select: bank use be updata or run code.
 * @return  bank_use: BankA or BankB.
 */
flash_bank check_bank(uint8_t bank_select)
{
    flash_bank bank_use = FLASH_BANKA;

    if(bank_select >= 4) //when Bank count equal of bankA and bankB, next updata erea in bankA
    {
        if(*((uint16_t *)BANKA_UPDATE_CNT_ADDR) ==  *((uint16_t *)BANKB_UPDATE_CNT_ADDR))
        {
            bank_use = FLASH_BANKA;
        }
        else if((*((uint16_t *)BANKA_UPDATE_CNT_ADDR) != 0xFFFF) && (*((uint16_t *)BANKB_UPDATE_CNT_ADDR) == 0xFFFF))
        {
            bank_use = FLASH_BANKB;
        }
        else if(*((uint16_t *)BANKA_UPDATE_CNT_ADDR) > *((uint16_t *)BANKB_UPDATE_CNT_ADDR))
        {
            bank_use = FLASH_BANKB;
        }
    }
    else   //not updata, mcu reset the code is run with the last upgraded bank
    {
        if(*((uint16_t *)BANKA_UPDATE_CNT_ADDR) ==  *((uint16_t *)BANKB_UPDATE_CNT_ADDR))
        {
            bank_use = FLASH_BANKB;
        }
        else if((*((uint16_t *)BANKA_UPDATE_CNT_ADDR) != 0xFFFF) && (*((uint16_t *)BANKB_UPDATE_CNT_ADDR) == 0xFFFF))
        {
            bank_use = FLASH_BANKA;
        }
        else if(*((uint16_t *)BANKA_UPDATE_CNT_ADDR) > *((uint16_t *)BANKB_UPDATE_CNT_ADDR))
        {
            bank_use = FLASH_BANKA;
        }
    }

    return bank_use;
}


void updata_key_gpio_init(void)
{
    GPIO_InitType GPIO_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin       = GPIO_PIN_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Pull = GPIO_NO_PULL;
    GPIO_InitPeripheral( GPIOA, &GPIO_InitStructure);
}


/**
 * @brief   This function check firmware if need update.
 * @param   void
 * @return  void
 */
void check_update(void)
{
#ifdef USER_KEY_UPDATA
    uint8_t wait_time   = 10;
    uint8_t update_flag = 0;

    /* APP 通过串口命令写入 SRAM 标志请求升级：软复位后保持，断电后自动消失 */
    if (*(volatile uint32_t *)OTA_FLAG_ADDR == OTA_FLAG_MAGIC)
    {
        /* 立即消费标志，防止升级完成/取消后重启再次误入升级模式 */
        *(volatile uint32_t *)OTA_FLAG_ADDR = 0;
        printf("\r\n OTA update request from APP\r\n");
        /* 与长按 WKUP 相同的升级路径：跳过按键检测，check_bank 走升级选库逻辑 */
        update_flag = 4;
        wait_time   = 0;
    }

    while(wait_time > 0) // wait time 5s, updata key if has been pressed
    {
        systick_delay_ms(500);

        if((wait_time % 2) == 0)
        {
            printf(" %d ---", (wait_time / 2));
        }

        if(GPIO_ReadInputDataBit(UPDATA_KEY_PORT, UPDATA_KEY_PIN))  //key need press at least 5s
        {
            LED3_PORT->POD ^= LED3_PIN;
            update_flag++;
        }
        else
        {
            if((wait_time % 2) == 0)
            {
                LED3_PORT->POD ^= LED3_PIN;
            }
        }
        wait_time--;
    }
    LED3_PORT->POD = 0x00;
   
#ifdef BANK_AB_UPDATA  

    flash_bank bank_use = check_bank(update_flag);

    if(bank_use == FLASH_BANKA)
    {
        FLASH_APP_START_ADDRESS = FLASH_BANKA_START_ADDR;
        FLASH_APP_END_ADDRESS   = FLASH_BANKA_END_ADDR;
    }
    else
    {
        FLASH_APP_START_ADDRESS = FLASH_BANKB_START_ADDR;
        FLASH_APP_END_ADDRESS   = FLASH_BANKB_END_ADDR;
    }
#else
        FLASH_APP_START_ADDRESS = FLASH_BANKB_START_ADDR;
        FLASH_APP_END_ADDRESS   = FLASH_BANKB_END_ADDR;    
#endif
    printf("\r\n app start_addr: 0x%08x, app end_addr: 0x%08x\r\n", FLASH_APP_START_ADDRESS, FLASH_APP_END_ADDRESS);

    
#ifdef USER_KEY_UPDATA
    if(update_flag >= 4)  //when mcu reset, and the updata key press at least 2s
#endif
    {
        xmodem_download();
    }
#endif
}

ErrorStatus __attribute__((used, section(".ramfunc"))) check_app_crc(uint32_t app_start_addr, uint32_t app_end_addr)
{
    uint32_t app_crc = 0;
    
    app_crc = CRC32((uint32_t *)app_start_addr, (app_end_addr - app_start_addr + 1)/4, 0xffffffff);
    
    if(app_crc != *((uint32_t *)(BOOT_END_ADDR + 1 -8)))
    {
        return ERROR;
    }
    else 
    {
        return SUCCESS;
    }
}

void copy_app_to_itcm(uint32_t copy_size)
{
    uint8_t *srcAddr, *dstAddr;
    uint32_t offset = 0;
    
    dstAddr = (uint8_t *)ITCM_BASE_ADDR;
    srcAddr = (uint8_t *)FLASH_APP_START_ADDRESS;
    
    while(offset < copy_size)
    {
        if(copy_size - offset >= COPY_SIZE)
        {
            memcpy(dstAddr, srcAddr, COPY_SIZE);
            dstAddr += COPY_SIZE;
            srcAddr += COPY_SIZE;
            offset += COPY_SIZE;
        }
        else
        {
            memcpy(dstAddr, srcAddr, copy_size - offset);
            offset = copy_size;
        }
    }
}

/**
 * @brief   Actually jumps to the user application.
 * @param   address: 
 * @return  void
 */
void flash_jump_to_app(uint32_t address)
{
    uint32_t JumpAddress;

    /* Judge whether the top of stack address is legal or not */
    if(((*(__IO uint32_t *)address) & 0x24000000) == 0x24000000)
    {
        uint32_t *pResetHandler = ( uint32_t *)(address + 4);
        
        if(( *pResetHandler &  FLASH_BANKA_START_ADDR) == ITCM_BASE_ADDR)
        {
            /* Reset all peripherals */
            RCC_EnableAHB5PeriphReset1(RCC_AHB5_PERIPHRST_GPIOA | RCC_AHB5_PERIPHRST_GPIOB );//reset GPIOA~GPIOB
            RCC_EnableAPB1PeriphReset3(RCC_APB1_PERIPHRST_USART1);//reset usart1
            RCC_EnableAHB1PeriphReset1(RCC_AHB1_PERIPHRST_DMAMUX1);//reset DMAMUX1 
            RCC_EnableAHB1PeriphReset3(RCC_AHB1_PERIPHRST_DMA1);   //reset DMA1
            
            /* Copy application data to itcm */    
            copy_app_to_itcm(FLASH_APP_END_ADDRESS - FLASH_APP_START_ADDRESS + 1 );
            
            JumpAddress = *(__IO uint32_t*)(ITCM_BASE_ADDR + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Set Vetor Table*/
            SCB->VTOR = ITCM_BASE_ADDR; 
            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) address);
            /* Jump to user application */            
            Jump_To_Application();
        }
    }    
}



