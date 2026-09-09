/******************************************************************************
 * @file    r900pnr_spi_cfg.h
 * @brief   R900PNR (TXW8301 FMAC) SDSPI 接口引脚/时钟/参数宏定义
 * @note    适配 N32H785 开发板, M7 内核, 标准外设库
 *          基于泰芯 non-os driver v2.0.0 的 SPI 底层需求
 *          完全适配 hgic_sdspi_demo_stm32.c 的 DMA 模式
 ******************************************************************************/
#ifndef __R900PNR_SPI_CFG_H__
#define __R900PNR_SPI_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 * 1. SPI 外设选择 (推荐使用 SPI1, 默认引脚在 GPIOA, 便于布线)
 *============================================================================*/
#define R900PNR_SPIx                    SPI1
#define R900PNR_SPIx_CLK                RCC_APB2_PERIPHEN_M7_SPI1
#define R900PNR_SPIx_APBx_CLK_CMD       RCC_EnableAPB2PeriphClk2
#define R900PNR_SPIx_IRQn               SPI1_IRQn
#define R900PNR_SPIx_IRQHandler         SPI1_IRQHandler

/*============================================================================*
 * 2. SPI 引脚定义 (GPIOA)
 *    开发板默认 SPI1 引脚: PA4=NSS, PA5=SCK, PA6=MISO, PA7=MOSI
 *    NSS 使用软件控制, 复用为 GPIO 输出
 *============================================================================*/
#define R900PNR_SPI_SCK_PIN             GPIO_PIN_5
#define R900PNR_SPI_SCK_PORT            GPIOA
#define R900PNR_SPI_SCK_CLK             RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_SCK_AF              GPIO_AF4          /* SPI1_SCK 复用 */

#define R900PNR_SPI_MISO_PIN            GPIO_PIN_6
#define R900PNR_SPI_MISO_PORT           GPIOA
#define R900PNR_SPI_MISO_CLK            RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_MISO_AF             GPIO_AF4          /* SPI1_MISO 复用 */

#define R900PNR_SPI_MOSI_PIN            GPIO_PIN_7
#define R900PNR_SPI_MOSI_PORT           GPIOA
#define R900PNR_SPI_MOSI_CLK            RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_MOSI_AF             GPIO_AF5          /* SPI1_MOSI 复用 */

/* 软件片选 */
#define R900PNR_SPI_CS_PIN              GPIO_PIN_4
#define R900PNR_SPI_CS_PORT             GPIOA
#define R900PNR_SPI_CS_CLK              RCC_AHB5_PERIPHEN_M7_GPIOA

/* 中断引脚: R900PNR 有数据时拉低通知 MCU 读取 */
#define R900PNR_SPI_INT_PIN             GPIO_PIN_8
#define R900PNR_SPI_INT_PORT            GPIOA
#define R900PNR_SPI_INT_CLK             RCC_AHB5_PERIPHEN_M7_GPIOA
#define R900PNR_SPI_INT_EXTI_LINE       EXTI_LINE8
#define R900PNR_SPI_INT_EXTI_GPIO       EXTI_GPIOA_Pin_8  /* AFIO EXTI 配置 */
#define R900PNR_SPI_INT_IRQn            EXTI9_5_IRQn
#define R900PNR_SPI_INT_IRQHandler      EXTI9_5_IRQHandler

/*============================================================================*
 * 3. SPI 通信参数
 *    泰芯 R900PNR FMAC SDSPI 模式要求:
 *      CPOL = Low, CPHA = 1 Edge (Mode 0)
 *      8 bit, MSB first, 软件 NSS
 *============================================================================*/
#define R900PNR_SPI_MODE                SPI_MODE_MASTER
#define R900PNR_SPI_DIR                 SPI_DIR_DOUBLELINE_FULLDUPLEX
#define R900PNR_SPI_DATA_SIZE           SPI_DATA_SIZE_8BITS
#define R900PNR_SPI_CPOL                SPI_CLKPOL_LOW
#define R900PNR_SPI_CPHA                SPI_CLKPHA_FIRST_EDGE
#define R900PNR_SPI_NSS                 SPI_NSS_SOFT
#define R900PNR_SPI_FIRST_BIT           SPI_FB_MSB
#define R900PNR_SPI_CRC_POLY            7

/* 波特率分频: 根据 APB2 时钟调整
 * 当前使用 SPI_BR_PRESCALER_4  SPI 瓶颈提升后 WiFi 射频成为新瓶颈 */
#define R900PNR_SPI_BAUDRATE_PRES       SPI_BR_PRESCALER_4    // 37.5MHz

/*============================================================================*
 * 4. 缓冲区与超时参数
 *============================================================================*/
#define R900PNR_SPI_RX_BUF_SIZE         2048u             /* 单帧接收缓冲区 */
#define R900PNR_SPI_TX_BUF_SIZE         2048u             /* 单帧发送缓冲区 */
#define R900PNR_SPI_TIMEOUT_MS          1000u             /* 轮询收发超时 */
#define R900PNR_DMA_TEMP_BUF_SIZE       2048u             /* DMA 临时缓冲区 (仅发/仅收时用) */

/*============================================================================*
 * 5. DMA 配置 (适配 hgic_sdspi_v2.c 的 DMA_TRANS 路径)
 *    DMA1 CH0 = SPI1 TX (M2P), DMAMUX1 CH0 -> SPI1_TX
 *    DMA1 CH1 = SPI1 RX (P2M), DMAMUX1 CH1 -> SPI1_RX
 *============================================================================*/
#define R900PNR_DMAx                    DMA1
#define R900PNR_DMAx_CLK                RCC_AHB1_PERIPHEN_M7_DMA1

#define R900PNR_DMAMUX_ID               DMAMUX1_ID
#define R900PNR_DMAMUXx_CLK             RCC_AHB1_PERIPHEN_M7_DMAMUX1

/* TX 通道 */
#define R900PNR_DMA_TX_CHANNEL          DMA_CHANNEL_0
#define R900PNR_DMA_TX_IRQn             DMA1_Channel0_IRQn
#define R900PNR_DMA_TX_IRQHandler       DMA1_Channel0_IRQHandler
#define R900PNR_DMA_TX_DMAMUX_CH        DMAMUX_CHANNEL_0
#define R900PNR_DMA_TX_DMAMUX_REQ       DMAMUX1_REQUEST_SPI1_TX

/* RX 通道 */
#define R900PNR_DMA_RX_CHANNEL          DMA_CHANNEL_1
#define R900PNR_DMA_RX_IRQn             DMA1_Channel1_IRQn
#define R900PNR_DMA_RX_IRQHandler       DMA1_Channel1_IRQHandler
#define R900PNR_DMA_RX_DMAMUX_CH        DMAMUX_CHANNEL_1
#define R900PNR_DMA_RX_DMAMUX_REQ       DMAMUX1_REQUEST_SPI1_RX

/* SPI 数据寄存器地址 (DMA 源/目的) */
#define R900PNR_SPI_DR_ADDR             ((uint32_t)((uint32_t)R900PNR_SPIx + 0x0CU))

/*============================================================================*
 * 6. 调试串口 (当前未使用)
 *    本工程使用 LWIP 例程的 log.c 提供的 printf/fputc,
 *    因此 r900pnr_spi_init.c 不再初始化 USART1。
 *    以下宏仅作备用, 如需自行初始化串口可复用。
 *============================================================================*/
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
