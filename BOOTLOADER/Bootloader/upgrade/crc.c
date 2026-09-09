
#include "crc.h"



extern uint8_t pattern_buf[1227*42];
extern uint8_t crc_buf[PATTERN_NUM];
#define BYTE_NUM  1227*PATTERN_NUM
#define CRC32_DFE_POLY 0x04C11DB7

/**
 *	@brief:		polymal is 0x1021, x^6 + x^2 + x^5 + x^0
 *	@param:		data:		byte data that need to calculate
 *				crcin:		previous PEC data result
 *	@return:	PEC result
 */
uint16_t crc_16_r(uint8_t *data, uint32_t length)
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
 *	@brief:		polymal is 0x107, x^8 + x^2 + x^1 + x^0
 *	@param:		data:		byte data that need to calculate
 *				crcin:		previous PEC data result
 *	@return:	PEC result
 */
//unsigned char computeCrc8(unsigned char crcInitialization, 
//                          unsigned char polynomial,
//                          unsigned char *pData, 
//                          unsigned int  dataLen )
unsigned char crc8(unsigned char *pData, unsigned int  dataLen)
{
    unsigned char i;
    unsigned char crc;
    unsigned char polynomial = 0x07; // Polynom: x^8 + x^2 + x + 1 = 100000111 = 0x107
    unsigned char crcInitialization = 0xFF; 

    crc = crcInitialization;
    while (dataLen --)
    {
        crc ^= *pData ++;
        for( i = 0; i < 8; i++ )
        {
            if(crc & 0x80)
            {
                crc <<= 1; // shift left once
                crc ^= polynomial; // XOR with polynomial
            }
            else
            { 
                crc <<= 1; // shift left once
            }
        }
    }

    return crc;
}



/**
 *	@brief:		polymal is x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1
 *	@param:		data:		byte data that need to calculate
 *				crcin:		previous PEC data result
 *	@return:	PEC result
 */
__attribute__((section("ramfunc"))) 
uint32_t CRC32(uint32_t *pbuf, uint32_t len , uint32_t initcrc)
{
    uint32_t    crc32 = initcrc;
    uint32_t    xbit;
    uint32_t    bits;
    uint32_t    i;

    for(i = 0;i < len; i ++)
    {
        xbit = 0x80000000;
        for (bits = 0; bits < 32; bits++)
        {
            if (crc32 & 0x80000000)
            {
                crc32 <<= 1;
                crc32 ^= CRC32_DFE_POLY;
            }
            else
            {  
                crc32 <<= 1;
            }
            if (pbuf[i] & xbit)
            {
                crc32 ^= CRC32_DFE_POLY;
            }
            xbit >>= 1;
        }
    }
    return crc32;
}






