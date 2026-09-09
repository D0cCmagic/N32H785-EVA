/**
 * @file    xmodem.c
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module is the implementation of the Xmodem protocol.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#include "xmodem.h"
#include "crc.h"
#include "IAP.h"
#include "delay.h"

/* 等待首包：1Hz 节奏发送 C，60s 无首包视同 CAN 中止 */
#define X_FIRST_PACKET_TIMEOUT_MS  (60000u)
#define X_C_PERIOD_MS              (1000u)
#define X_POLL_SLICE_MS             (10u)


/* Global variables. */
static uint8_t xmodem_packet_number = 1u;         /**< Packet number counter. */
static uint32_t xmodem_actual_flash_address = 0u; /**< Address where we have to write. */
static uint8_t x_first_packet_received = false;   /**< First packet or not. */

/* Local functions. */
static uint16_t xmodem_calc_crc(uint8_t *data, uint16_t length);
static xmodem_status xmodem_handle_packet(uint8_t size);
static xmodem_status xmodem_error_handler(uint8_t *error_number, uint8_t max_error_number);

uint8_t received_packet_data[X_PACKET_1024_SIZE];
uint32_t received_packet_number_total = 0;  //add 20210728----packet number total
uint32_t program_page_packet_number = 0;

uint8_t   rcvFlag = 0;

/**
 * @brief   This function is the base of the Xmodem protocol.
 *          When we receive a header from UART, it decides what action it shall take.
 * @param   void
 * @return  void
 */
 

 
void xmodem_receive(void)
{
  volatile xmodem_status status = X_OK;
  uint8_t error_number = 0u;
  uart_status comm_status = UART_ERROR; //20250513

  x_first_packet_received = false;
  xmodem_packet_number = 1u;
  xmodem_actual_flash_address = FLASH_APP_START_ADDRESS;
  uint32_t flash_page_backup[FLASH_SECTOR_SIZE/4] = {0};
  
  /* Loop until there isn't any error (or until we jump to the user application). */
  while (X_OK == status)
  {
    uint8_t header = 0x00u;

    /* Get the header from UART. */
    //uart_status comm_status = uart_receive(&header, 1u);
    
    if(rcvFlag)
    {
        comm_status = UART_OK;
        header = received_packet_data[0];

        /* 脏数据过滤（仅等待首包阶段）：首字节非 XMODEM 协议字节的批次
         * 视为无效（如误发的命令），消费 rcvFlag 并复位 comm_status，
         * 使下方等待分支重新成立（C 恢复/超时计时）。
         * 首包阶段任何协议字节都直接进入 switch 正常处理 */
        if ((false == x_first_packet_received) &&
            (header != X_SOH) && (header != X_STX) &&
            (header != X_EOT) && (header != X_CAN))
        {
            rcvFlag = 0;
            comm_status = UART_ERROR;
            header = 0x00u;
        }
    }

    /* Spam the host (until we receive something) with ACSII "C", to notify it, we want to use CRC-16. */
    if ((UART_OK != comm_status) && (false == x_first_packet_received))
    {
      /* 以 1Hz 节奏发送 C 等待首包（串口接收为 DMA+空闲中断，阻塞延时期间数据不丢失） */
      uint32_t waited_ms = 0u;

      while ((0u == rcvFlag) && (waited_ms < X_FIRST_PACKET_TIMEOUT_MS))
      {
        (void)uart_transmit_ch(X_C);
        for (uint32_t t = 0u; (t < X_C_PERIOD_MS) && (0u == rcvFlag); t += X_POLL_SLICE_MS)
        {
          systick_delay_ms(X_POLL_SLICE_MS);
        }
        waited_ms += X_C_PERIOD_MS;
      }

      if (0u != rcvFlag)
      {
        comm_status = UART_OK;
        header = received_packet_data[0];
      }
      else
      {
        /* 超时：视同主机取消（CAN），复位后按正常流程运行旧版本 */
        (void)uart_transmit_ch(X_CAN);
        (void)uart_transmit_ch(X_CAN);
        (void)uart_transmit_str((uint8_t*)"\n\rTimeout, no data in 60s, reboot to run old firmware.\n\r");
        SystemNVICReset();
      }
    }
    /* Uart timeout or any other errors. */
    else if ((UART_OK != comm_status) && (true == x_first_packet_received))
    {
      status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
    }
    else
    {
      /* Do nothing. */
    }

    /* The header can be: SOH, STX, EOT and CAN. */
    xmodem_status packet_status = X_ERROR;
    
    switch(header)
    {
        /* 128 or 1024 bytes of data. */
        case X_SOH:
        case X_STX:
            /* If the handling was successful, then send an ACK. */
            packet_status = xmodem_handle_packet(header);
            if (X_OK == packet_status)
            {
              (void)uart_transmit_ch(X_ACK);
            }
            /* If the error was flash related, then immediately set the error counter to max (graceful abort). */
            else if (X_ERROR_FLASH == packet_status)
            {
              error_number = X_MAX_ERRORS;
              status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
            }
            /* Error while processing the packet, either send a NAK or do graceful abort. */
            else
            {
              status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
            }
        break;
      /* End of Transmission. */
      case X_EOT:
          
      
        /* ACK, feedback to user (as a text), then jump to user application. */
        (void)uart_transmit_ch(X_ACK);
        (void)uart_transmit_str((uint8_t*)"\n\rFirmware updated!\n\r");
        (void)uart_transmit_str((uint8_t*)"Jumping to user application...\n\r");
      

        for(int i=0; i<FLASH_SECTOR_SIZE/4; i++)
        {
            flash_page_backup[i] = *((uint32_t *)(BOOT_LSAST_SECTOR + i*4));
        }

#ifdef  BANK_AB_UPDATA
        
        uint16_t bankA_cnt = *((uint16_t *)BANKA_UPDATE_CNT_ADDR);
        uint16_t bankB_cnt = *((uint16_t *)BANKB_UPDATE_CNT_ADDR);
        
        if(check_bank(10) == FLASH_BANKA) //record bankA update number
        {
            bankA_cnt++;        
        }
        else //bankB update number
        {
            bankB_cnt++;
        }
        flash_page_backup[FLASH_SECTOR_SIZE/4-1] = (bankA_cnt<<16) | bankB_cnt;
#endif       
        
        
        (void)uart_transmit_str((uint8_t*)"\n\rWaiting, crc32 of appcation is calculating ...... \n\r");
        /* record app CRC */
        flash_page_backup[FLASH_SECTOR_SIZE/4-2] = CRC32((uint32_t *)FLASH_APP_START_ADDRESS, (FLASH_APP_END_ADDRESS - FLASH_APP_START_ADDRESS + 1)/4, 0xFFFFFFFF);
 
        if (FLASH_SUCCESS != SMU_EraseFlash(BOOT_LSAST_SECTOR))
        {
            (void)uart_transmit_str((uint8_t*)"\n\r boot last sector erase fail!\n\r");
            while(1);
        }

        if(FLASH_OK != flash_write(BOOT_LSAST_SECTOR, (uint32_t*)(&flash_page_backup[0]), FLASH_SECTOR_SIZE))
        {
            (void)uart_transmit_str((uint8_t*)"\n\r boot last page sector fail!\n\r");
             while(1);
        }
        
        
        flash_jump_to_app(FLASH_APP_START_ADDRESS);
        break;
       
      /* Abort from host. */
      case X_CAN:
        status = X_ERROR;
        break;
      default:
        /* Wrong header. */
        if (UART_OK == comm_status)
        {
          /* Wrong header: 原版行为，状态消费统一移至循环顶部 */
        }
        break;
    }
  }
}

/**
 * @brief   Calculates the CRC-16 for the input package.
 * @param   *data:  Array of the data which we want to calculate.
 * @param   length: Size of the data, either 128 or 1024 bytes.
 * @return  status: The calculated CRC.
 */
static uint16_t xmodem_calc_crc(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0u;
    while (length)
    {
        length--;
        crc = crc ^ ((uint16_t)*data++ << 8u);
        for (uint8_t i = 0u; i < 8u; i++)
        {
            if (crc & 0x8000u)
            {
                crc = (crc << 1u) ^ 0x1021u;
            }
            else
            {
                crc = crc << 1u;
            }
        }
    }
    return crc;
}

/**
 * @brief   This function handles the data packet we get from the xmodem protocol.
 * @param   header: SOH or STX.
 * @return  status: Report about the packet.
 */
static xmodem_status xmodem_handle_packet(uint8_t header)
{
  xmodem_status status = X_OK;
  uint16_t size = 0u;

  /* 2 bytes for packet number, 1024 for data, 2 for CRC*/
  uint8_t received_packet_number[X_PACKET_NUMBER_SIZE];

  //uint8_t received_packet_data[X_PACKET_1024_SIZE];
  uint8_t received_packet_crc[X_PACKET_CRC_SIZE];


  /* Get the size of the data. */
  if (X_SOH == header)
  {
    size = X_PACKET_128_SIZE;
  }
  else if (X_STX == header)
  {
    size = X_PACKET_1024_SIZE;
  }
  else
  {
    /* Wrong header type. This shoudn't be possible... */
    status |= X_ERROR;
  }

  uart_status comm_status = UART_OK;
  /* Get the packet number, data and CRC from UART. */
  /* 消费快照：记录本包处理前的 rcvFlag。处理过程中若 ISR 已收到下一包
   * （rcvFlag 被重新置 1），则不清除它，避免吞掉下一包的到达标志 */
  {   uint8_t rcvFlag_snapshot = rcvFlag;
        received_packet_number[0] = received_packet_data[1];
        received_packet_number[1] = received_packet_data[2];

        received_packet_crc[0]    = received_packet_data[131];
        received_packet_crc[1]    = received_packet_data[132];
        
        for(int i=0; i<size; i++)
        {
            received_packet_data[i] = received_packet_data[i+3];
        }
        if (rcvFlag_snapshot) { rcvFlag = 0; }   /* 仅当没有更新的包到达时才消费 */
    }
    
    
    
  /* Merge the two bytes of CRC. */ 
  uint16_t crc_received = ((uint16_t)received_packet_crc[X_PACKET_CRC_HIGH_INDEX] << 8u) | ((uint16_t)received_packet_crc[X_PACKET_CRC_LOW_INDEX]);
  /* We calculate it too. */
  uint16_t crc_calculated = xmodem_calc_crc(&received_packet_data[0u], size);
    
  /* Communication error. */
  if (UART_OK != comm_status)
  {
    status |= X_ERROR_UART;
  }

    
  /* If it is the first packet, then erase the memory. */
  if ((X_OK == status) && (false == x_first_packet_received))
  {
    if (FLASH_OK == flash_erase(FLASH_APP_START_ADDRESS))
    {
      x_first_packet_received = true;
    }
    else
    {
      status |= X_ERROR_FLASH;
    }
  }

  /* Error handling and flashing. */
  if (X_OK == status)
  {
    if (xmodem_packet_number != received_packet_number[0u])
    {
      /* Packet number counter mismatch. */
      status |= X_ERROR_NUMBER;
    }
    if (255u != (received_packet_number[X_PACKET_NUMBER_INDEX] + received_packet_number[X_PACKET_NUMBER_COMPLEMENT_INDEX]))
    {
      /* The sum of the packet number and packet number complement aren't 255. */
      /* The sum always has to be 255. */
      status |= X_ERROR_NUMBER;
    }
    if (crc_calculated != crc_received)
    {
      /* The calculated and received CRC are different. */
      status |= X_ERROR_CRC;
    }
  }


    /* Do the actual flashing (if there weren't any errors). */
    if ((X_OK == status) && (FLASH_OK != flash_write(xmodem_actual_flash_address, (uint32_t*)&received_packet_data[0u], (uint32_t)size)))    
    {
        /* Flashing error. */
        status |= X_ERROR_FLASH;
    }
 
    /* Raise the packet number and the address counters (if there weren't any errors). */
    if (X_OK == status)
    {
        xmodem_packet_number++;
        xmodem_actual_flash_address += size;
        
        received_packet_number_total += size;
    }
 
  return status;
}

/**
 * @brief   Handles the xmodem error.
 *          Raises the error counter, then if the number of the errors reached critical, do a graceful abort, otherwise send a NAK.
 * @param   *error_number:    Number of current errors (passed as a pointer).
 * @param   max_error_number: Maximal allowed number of errors.
 * @return  status: X_ERROR in case of too many errors, X_OK otherwise.
 */
static xmodem_status xmodem_error_handler(uint8_t *error_number, uint8_t max_error_number)
{
  xmodem_status status = X_OK;
  /* Raise the error counter. */
  (*error_number)++;
  /* If the counter reached the max value, then abort. */
  if ((*error_number) >= max_error_number)
  {
    /* Graceful abort. */
    (void)uart_transmit_ch(X_CAN);
    (void)uart_transmit_ch(X_CAN);
    status = X_ERROR;
  }
  /* Otherwise send a NAK for a repeat. */
  else
  {
    (void)uart_transmit_ch(X_NAK);
    status = X_OK;
  }
  return status;
}



void xmodem_download(void)
{
    {
        /* Ask for new data and start the Xmodem protocol. */
        uart_transmit_str((uint8_t*)"Please send a new binary file with Xmodem protocol to update the firmware.\n\r");
        xmodem_receive();
    }

    /* We only exit the xmodem protocol, if there are any errors.
    * In that case, notify the user and start over. */
    uart_transmit_str((uint8_t*)"\n\rFailed... Please try again.\n\r");
}

