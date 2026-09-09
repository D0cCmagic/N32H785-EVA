/******************************************************************************
 * @file    r900pnr_spi_api.c
 * @brief   R900PNR SPI 底层驱动函数实现 (批量轮询模式)
 *
 * @note    重要: 放弃 DMA 模式, 全部使用 CPU 批量轮询!
 *
 *          原因: hgic_sdspi_v2.c 内部大量使用同一 buffer 做 TX+RX
 *          (sd_send_command: sd_mgr.status 既是 TX 又是 RX;
 *           sd_setup_data_message: temp[69] 既是 TX 又是 RX)
 *
 *          DMA + Cortex-M7 D-Cache 下, SCB_InvalidateDCache_by_Addr 按
 *          32 字节 cache line 操作, 会误伤 buffer 旁边的栈变量/全局变量,
 *          导致 cp 指针、resp 等被损坏, 命令解析失败, 模块通信崩溃。
 *
 *          CPU 批量轮询无此问题: CPU 始终通过 cache 访问内存, 透明一致。
 *          在 SPI 2.3MHz 下, 每字节 3.5µs, 循环开销 ~50ns, 完全够用。
 *
 *          适配 N32H785 开发板, M7 内核, 标准外设库
 ******************************************************************************/
#include "n32h7xx.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_spi.h"
#include "n32h7xx_exti.h"
#include "r900pnr_spi_cfg.h"
#include "r900pnr_spi_api.h"
#include <string.h>

/* 全局中断标志: R900PNR 有数据时由 EXTI 中断置 1 */
volatile uint8_t g_r900pnr_spi_rx_flag = 0;

/* DMA 完成标志 (保留声明, 当前不使用 DMA) */
volatile uint8_t g_dma_tx_done = 0;
volatile uint8_t g_dma_rx_done = 0;

/******************************************************************************
 * 内部辅助: 批量轮询全双工收发
 * 直接操作 SPI 数据寄存器, 无 DMA, 无 D-Cache 问题
 * 每字节: 等 TXE -> 写 DR -> 等 RXNE -> 读 DR
 ******************************************************************************/
static void R900PNR_SPI_BatchXfer(const uint8_t *tx, uint8_t *rx, uint32_t len)
{
    uint32_t i;
    uint16_t dummy;

    for (i = 0; i < len; i++)
    {
        /* 等待 TX 缓冲区空 (TXE flag) */
        while (SPI_I2S_GetStatus(R900PNR_SPIx, SPI_I2S_TE_FLAG) == RESET)
        {
        }
        /* 写入发送数据 (如果 tx 为 NULL, 发 0xFF 产生时钟) */
        SPI_I2S_TransmitData(R900PNR_SPIx, tx ? tx[i] : 0xFF);

        /* 等待 RX 缓冲区非空 (RXNE flag) */
        while (SPI_I2S_GetStatus(R900PNR_SPIx, SPI_I2S_RNE_FLAG) == RESET)
        {
        }
        /* 读取接收数据 (如果 rx 为 NULL, 丢弃) */
        dummy = SPI_I2S_ReceiveData(R900PNR_SPIx);
        if (rx)
        {
            rx[i] = (uint8_t)dummy;
        }
    }
}

/******************************************************************************
 * 泰芯 non-os driver 所需的 SPI 底层回调函数
 * 全部使用 CPU 批量轮询, 忽略 dma_flag 参数
 ******************************************************************************/

/**
 * @brief  SPI 片选控制
 * @param  enable: 1=拉低片选(选中), 0=拉高片选(释放)
 */
void spidrv_cs(void *priv, char enable)
{
    (void)priv;

    if (enable)
    {
        GPIO_ResetBits(R900PNR_SPI_CS_PORT, R900PNR_SPI_CS_PIN);
    }
    else
    {
        GPIO_SetBits(R900PNR_SPI_CS_PORT, R900PNR_SPI_CS_PIN);
    }
}

/**
 * @brief  SPI 全双工同时收发 (CPU 批量轮询)
 *         对应 STM32 DEMO: HAL_SPI_TransmitReceive_DMA
 *
 *         注意: hgic_sdspi_v2.c 传入的 wdata 和 rdata 可能是同一指针
 *         (sd_send_command: sd_mgr.status 既做 TX 又做 RX;
 *          sd_setup_data_message: temp 既做 TX 又做 RX)
 *         CPU 批量轮询下, 先读 tx[i] 再写 rx[i], 同一 buffer 安全。
 *
 * @param  wdata 发送缓冲区
 * @param  rdata 接收缓冲区 (可能与 wdata 相同)
 * @param  len   字节数
 */
void spidrv_write_read(void *priv, unsigned char *wdata, unsigned char *rdata,
                       unsigned int len)
{
    (void)priv;

    if (len == 0)
    {
        return;
    }

    R900PNR_SPI_BatchXfer(wdata, rdata, len);
}

/**
 * @brief  SPI 只发送 (接收数据丢弃)
 *         对应 STM32 DEMO: HAL_SPI_TransmitReceive_DMA(data, temp_buf)
 *         CPU 批量轮询: 发送 data, 丢弃接收
 * @param  data     发送缓冲区
 * @param  len      字节数
 * @param  dma_flag 忽略 (统一用 CPU 轮询)
 */
void spidrv_write(void *priv, unsigned char *data, unsigned int len, char dma_flag)
{
    (void)priv;
    (void)dma_flag;

    if (len == 0)
    {
        return;
    }

    R900PNR_SPI_BatchXfer(data, NULL, len);
}

/**
 * @brief  SPI 只接收 (发送 0xFF 产生时钟)
 *         对应 STM32 DEMO: memset(temp_buf,0xff); HAL_SPI_TransmitReceive_DMA(temp_buf, data)
 *         CPU 批量轮询: 发送 0xFF, 接收到 data
 * @param  data     接收缓冲区
 * @param  len      字节数
 * @param  dma_flag 忽略 (统一用 CPU 轮询)
 */
void spidrv_read(void *priv, unsigned char *data, unsigned int len, char dma_flag)
{
    (void)priv;
    (void)dma_flag;

    if (len == 0)
    {
        return;
    }

    R900PNR_SPI_BatchXfer(NULL, data, len);
}

/**
 * @brief  SPI 硬件 CRC, R900PNR 不使用
 * @retval 0
 */
int spidrv_hw_crc(void *priv, unsigned char *data, unsigned int len, char flag)
{
    (void)priv;
    (void)data;
    (void)len;
    (void)flag;

    return 0;
}

/******************************************************************************
 * 中断处理
 ******************************************************************************/

/**
 * @brief  EXTI9_5 中断服务函数
 *         R900PNR_INT/PA8 下降沿触发, 表示模块有数据待读取
 */
void R900PNR_SPI_INT_IRQHandler(void)
{
    if (EXTI_GetITStatus(R900PNR_SPI_INT_EXTI_LINE) != RESET)
    {
        g_r900pnr_spi_rx_flag = 1;
        /* 清除 EXTI 挂起位 */
        EXTI_ClrITPendBit(R900PNR_SPI_INT_EXTI_LINE);
    }
}
