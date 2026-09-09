
#ifndef __CRC_H
#define	__CRC_H

#include "stdint.h"


#define PATTERN_NUM          539
#define EVERY_PATTERN_SIZE   1227

typedef enum 
{
  OK     = 0x00U,
  ERR    = 0x01U
} CRC_Def;


uint16_t crc_16_r(uint8_t *data, uint32_t length);
uint8_t crc8(unsigned char *pData, unsigned int  dataLen );
uint16_t cal_pattern_crc16(void);
CRC_Def check_pattern_crc(void);
uint32_t CRC32(uint32_t *pbuf, uint32_t len , uint32_t initcrc);


#endif

