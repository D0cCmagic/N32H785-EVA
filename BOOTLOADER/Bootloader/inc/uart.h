
#ifndef __UART_H__
#define __UART_H__

#include "n32h7xx.h"
#include "xmodem.h"




#define DMA_SRC  (&USART1->DAT)
#define USARTx_MAX_UART_BUF   1024



/* Status report for the functions. */
typedef enum {
  UART_OK     = 0x00u, /**< The action was successful. */
  UART_ERROR  = 0xFFu  /**< Generic error. */
} uart_status;







void uart_init(void);

uart_status uart_transmit_str(uint8_t *data);
uart_status uart_transmit_ch(uint8_t data);



#endif
