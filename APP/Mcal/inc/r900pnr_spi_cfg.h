/**
 * @file    r900pnr_spi_cfg.h
 * @brief   R900PNR (TXW8301 FMAC) SDSPI pin, clock and parameter definitions.
 * @note    Targets the N32H785 dev board, M7 core, standard peripheral library.
 *          Matches the SPI requirements of the Taixin non-os driver v2.0.0
 *          and the DMA mode used by hgic_sdspi_demo_stm32.c.
 */
#ifndef __R900PNR_SPI_CFG_H__
#define __R900PNR_SPI_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 1. SPI peripheral: SPI1 with its default GPIOA pins keeps routing simple */
#define R900PNR_SPIx                    SPI1
#define R900PNR_SPIx_CLK                RCC_APB2_PERIPHEN_M7_SPI1
#define R900PNR_SPIx_APBx_CLK_CMD       RCC_EnableAPB2PeriphClk2
#define R900PNR_SPIx_IRQn               SPI1_IRQn
#define R900PNR_SPIx_IRQHandler         SPI1_IRQHandler

/* 2. SPI pins on GPIOA: PA4=NSS, PA5=SCK, PA6=MISO, PA7=MOSI
 *    NSS is driven in software, so PA4 is used as a plain GPIO output */
#define R900PNR_SPI_SCK_PIN             GPIO_PIN_5
#define R900PNR_SPI_SCK_PORT            GPIOA
#define R900PNR_SPI_SCK_CLK             RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_SCK_AF              GPIO_AF4          /* SPI1_SCK alternate function */

#define R900PNR_SPI_MISO_PIN            GPIO_PIN_6
#define R900PNR_SPI_MISO_PORT           GPIOA
#define R900PNR_SPI_MISO_CLK            RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_MISO_AF             GPIO_AF4          /* SPI1_MISO alternate function */

#define R900PNR_SPI_MOSI_PIN            GPIO_PIN_7
#define R900PNR_SPI_MOSI_PORT           GPIOA
#define R900PNR_SPI_MOSI_CLK            RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_MOSI_AF             GPIO_AF5          /* SPI1_MOSI alternate function */

/* Software chip select */
#define R900PNR_SPI_CS_PIN              GPIO_PIN_4
#define R900PNR_SPI_CS_PORT             GPIOA
#define R900PNR_SPI_CS_CLK              RCC_AHB5_PERIPHEN_M7_GPIOA

/* Interrupt pin: the R900PNR pulls it low to tell the MCU data is waiting */
#define R900PNR_SPI_INT_PIN             GPIO_PIN_8
#define R900PNR_SPI_INT_PORT            GPIOA
#define R900PNR_SPI_INT_CLK             RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_INT_EXTI_LINE       EXTI_LINE8
#define R900PNR_SPI_INT_EXTI_GPIO       EXTI_GPIOA_Pin_8  /* AFIO EXTI routing */
#define R900PNR_SPI_INT_IRQn            EXTI9_5_IRQn
#define R900PNR_SPI_INT_IRQHandler      EXTI9_5_IRQHandler

/* 3. SPI parameters required by the Taixin SDSPI mode:
 *      CPOL = Low, CPHA = 1 Edge (Mode 0), 8 bit, MSB first, software NSS */
#define R900PNR_SPI_MODE                SPI_MODE_MASTER
#define R900PNR_SPI_DIR                 SPI_DIR_DOUBLELINE_FULLDUPLEX
#define R900PNR_SPI_DATA_SIZE           SPI_DATA_SIZE_8BITS
#define R900PNR_SPI_CPOL                SPI_CLKPOL_LOW
#define R900PNR_SPI_CPHA                SPI_CLKPHA_FIRST_EDGE
#define R900PNR_SPI_NSS                 SPI_NSS_SOFT
#define R900PNR_SPI_FIRST_BIT           SPI_FB_MSB
#define R900PNR_SPI_CRC_POLY            7

/* Baud rate prescaler, adjusted against the APB2 clock.
 * At SPI_BR_PRESCALER_4 the SPI link is no longer the bottleneck */
#define R900PNR_SPI_BAUDRATE_PRES       SPI_BR_PRESCALER_4    // 37.5MHz

/* 4. Buffers and timeouts */
#define R900PNR_SPI_RX_BUF_SIZE         2048u             /* Single frame receive buffer */
#define R900PNR_SPI_TX_BUF_SIZE         2048u             /* Single frame transmit buffer */
#define R900PNR_SPI_TIMEOUT_MS          1000u             /* Polled transfer timeout */
#define R900PNR_DMA_TEMP_BUF_SIZE       2048u             /* DMA scratch buffer (TX-only or RX-only) */

/* 5. DMA setup for the DMA_TRANS path in hgic_sdspi_v2.c
 *    DMA1 CH0 = SPI1 TX (M2P), DMAMUX1 CH0 -> SPI1_TX
 *    DMA1 CH1 = SPI1 RX (P2M), DMAMUX1 CH1 -> SPI1_RX */
#define R900PNR_DMAx                    DMA1
#define R900PNR_DMAx_CLK                RCC_AHB1_PERIPHEN_M7_DMA1

#define R900PNR_DMAMUX_ID               DMAMUX1_ID
#define R900PNR_DMAMUXx_CLK             RCC_AHB1_PERIPHEN_M7_DMAMUX1

/* TX channel */
#define R900PNR_DMA_TX_CHANNEL          DMA_CHANNEL_0
#define R900PNR_DMA_TX_IRQn             DMA1_Channel0_IRQn
#define R900PNR_DMA_TX_IRQHandler       DMA1_Channel0_IRQHandler
#define R900PNR_DMA_TX_DMAMUX_CH        DMAMUX_CHANNEL_0
#define R900PNR_DMA_TX_DMAMUX_REQ       DMAMUX1_REQUEST_SPI1_TX

/* RX channel */
#define R900PNR_DMA_RX_CHANNEL          DMA_CHANNEL_1
#define R900PNR_DMA_RX_IRQn             DMA1_Channel1_IRQn
#define R900PNR_DMA_RX_IRQHandler       DMA1_Channel1_IRQHandler
#define R900PNR_DMA_RX_DMAMUX_CH        DMAMUX_CHANNEL_1
#define R900PNR_DMA_RX_DMAMUX_REQ       DMAMUX1_REQUEST_SPI1_RX

/* SPI data register address, used as the DMA source or destination */
#define R900PNR_SPI_DR_ADDR             ((uint32_t)((uint32_t)R900PNR_SPIx + 0x0CU))

/* 6. Debug UART, currently unused.
 *    The project logs through printf/fputc from the LWIP log.c, so
 *    r900pnr_spi_init.c no longer brings up USART1. These macros are kept
 *    as a spare in case a dedicated debug UART is wanted later. */
#define R900PNR_LOG_USARTx              USART1
#define R900PNR_LOG_USARTx_CLK          RCC_APB1_PERIPHEN_M7_USART1
#define R900PNR_LOG_USARTx_APBx_CLK_CMD RCC_EnableAPB1PeriphClk3
#define R900PNR_LOG_USART_TX_PIN        GPIO_PIN_9
#define R900PNR_LOG_USART_TX_PORT       GPIOA
#define R900PNR_LOG_USART_TX_CLK        RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_LOG_USART_TX_AF         GPIO_AF7
#define R900PNR_LOG_USART_BAUDRATE      115200u

#ifdef __cplusplus
}
#endif

#endif /* __R900PNR_SPI_CFG_H__ */
