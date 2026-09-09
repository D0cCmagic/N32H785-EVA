/******************************************************************************
 * @file    r900pnr_spi_api.h
 * @brief   R900PNR SPI 底层驱动函数声明 + 全局标志
 * @note    适配 N32H785 开发板, M7 内核
 *          本文件函数主要供泰芯 non-os driver 调用
 *          printf 重定向由应用层提供, 本文件不声明 log 函数
 ******************************************************************************/
#ifndef __R900PNR_SPI_API_H__
#define __R900PNR_SPI_API_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* R900PNR 中断引脚触发标志, 在 EXTI9_5_IRQHandler 中置位 */
extern volatile uint8_t g_r900pnr_spi_rx_flag;

/* DMA 传输完成标志, 在 DMA1_CH0/CH1 中断中置位 */
extern volatile uint8_t g_dma_tx_done;
extern volatile uint8_t g_dma_rx_done;

/* SPI / GPIO / EXTI / DMA 统一初始化 */
extern int R900PNR_SPI_Init(void);

/* SPI 片选控制: enable=1 拉低片选, enable=0 拉高片选 */
extern void spidrv_cs(void *priv, char enable);

/* SPI 同时收发 (泰芯 hgic_sdspi_v2.c 需要, DMA 模式) */
extern void spidrv_write_read(void *priv, unsigned char *wdata, unsigned char *rdata, unsigned int len);

/* SPI 只发 (dma_flag=1: DMA, dma_flag=0: 批量轮询) */
extern void spidrv_write(void *priv, unsigned char *data, unsigned int len, char dma_flag);

/* SPI 只收 (dma_flag=1: DMA, dma_flag=0: 批量轮询) */
extern void spidrv_read(void *priv, unsigned char *data, unsigned int len, char dma_flag);

/* SPI 硬件 CRC, R900PNR 不使用, 返回 0 */
extern int spidrv_hw_crc(void *priv, unsigned char *data, unsigned int len, char flag);

/* EXTI9_5 中断服务函数, 处理 R900PNR_INT/PA8 */
extern void R900PNR_SPI_INT_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __R900PNR_SPI_API_H__ */
