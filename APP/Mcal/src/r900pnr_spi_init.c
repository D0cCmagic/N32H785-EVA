/******************************************************************************
 * @file    r900pnr_spi_init.c
 * @brief   R900PNR SPI 接口初始化: GPIO / SPI / EXTI / NVIC
 * @note    适配 N32H785 开发板, M7 内核, 标准外设库
 *          调试串口 printf 由 LWIP 例程的 log.c 提供, 本文件不再初始化 USART
 *          不使用 DMA: hgic_sdspi_v2.c 内部用同一 buffer 做 TX+RX,
 *          DMA + D-Cache 会导致栈变量损坏, 改用 CPU 批量轮询
 ******************************************************************************/
#include "n32h7xx.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_spi.h"
#include "n32h7xx_exti.h"
#include "n32h7xx_rcc.h"
#include "r900pnr_spi_cfg.h"
#include "r900pnr_spi_api.h"

/**
 * @brief  配置 SPI1 相关 GPIO
 *         SCK/PA5, MISO/PA6, MOSI/PA7 配置为复用推挽
 *         CS/PA4    配置为 GPIO 推挽输出, 默认高电平
 *         INT/PA8   配置为浮空输入, 外部中断下降沿
 */
static void R900PNR_SPI_GPIO_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    /* 使能 GPIOA 时钟 (SPI1 和 INT 都在 GPIOA) */
    RCC_EnableAHB5PeriphClk1(R900PNR_SPI_SCK_CLK | R900PNR_SPI_MISO_CLK |
                            R900PNR_SPI_MOSI_CLK | R900PNR_SPI_CS_CLK |
                            R900PNR_SPI_INT_CLK, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);

    /* SPI1 SCK */
    GPIO_InitStructure.Pin          = R900PNR_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = R900PNR_SPI_SCK_AF;
    GPIO_InitPeripheral(R900PNR_SPI_SCK_PORT, &GPIO_InitStructure);

    /* SPI1 MISO */
    GPIO_InitStructure.Pin          = R900PNR_SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = R900PNR_SPI_MISO_AF;
    GPIO_InitPeripheral(R900PNR_SPI_MISO_PORT, &GPIO_InitStructure);

    /* SPI1 MOSI */
    GPIO_InitStructure.Pin          = R900PNR_SPI_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = R900PNR_SPI_MOSI_AF;
    GPIO_InitPeripheral(R900PNR_SPI_MOSI_PORT, &GPIO_InitStructure);

    /* 软件片选 CS, 默认拉高 */
    GPIO_InitStructure.Pin          = R900PNR_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0;
    GPIO_InitPeripheral(R900PNR_SPI_CS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(R900PNR_SPI_CS_PORT, R900PNR_SPI_CS_PIN);

    /* R900PNR 中断引脚, 下降沿触发 */
    GPIO_InitStructure.Pin          = R900PNR_SPI_INT_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0;
    GPIO_InitPeripheral(R900PNR_SPI_INT_PORT, &GPIO_InitStructure);
}

/**
 * @brief  配置 EXTI: PA8 -> EXTI_LINE8, 下降沿中断
 */
static void R900PNR_SPI_EXTI_Init(void)
{
    EXTI_InitType EXTI_InitStructure;

    /* 将 EXTI_LINE8 映射到 GPIOA_Pin_8 */
    GPIO_ConfigEXTILine(R900PNR_SPI_INT_EXTI_LINE, R900PNR_SPI_INT_EXTI_GPIO);

    /* 配置 EXTI_LINE8: 中断模式, 下降沿触发 */
    EXTI_InitStruct(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line    = R900PNR_SPI_INT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /* 清除挂起位, 使能 NVIC */
    EXTI_ClrITPendBit(R900PNR_SPI_INT_EXTI_LINE);
    NVIC_SetPriority(R900PNR_SPI_INT_IRQn, 5);
    NVIC_EnableIRQ(R900PNR_SPI_INT_IRQn);
}

/**
 * @brief  配置 SPI1 主模式, 全双工, Mode 0
 */
static void R900PNR_SPI_Periph_Init(void)
{
    SPI_InitType SPI_InitStructure;

    /* 使能 SPI1 时钟 */
    R900PNR_SPIx_APBx_CLK_CMD(R900PNR_SPIx_CLK, ENABLE);

    /* SPI 去初始化 */
    SPI_I2S_DeInit(R900PNR_SPIx);

    /* 填充默认参数 */
    SPI_InitStruct(&SPI_InitStructure);

    /* 配置 SPI1 参数 */
    SPI_InitStructure.SpiMode       = R900PNR_SPI_MODE;
    SPI_InitStructure.DataDirection = R900PNR_SPI_DIR;
    SPI_InitStructure.DataLen       = R900PNR_SPI_DATA_SIZE;
    SPI_InitStructure.CLKPOL        = R900PNR_SPI_CPOL;
    SPI_InitStructure.CLKPHA        = R900PNR_SPI_CPHA;
    SPI_InitStructure.NSS           = R900PNR_SPI_NSS;
    SPI_InitStructure.BaudRatePres  = R900PNR_SPI_BAUDRATE_PRES;
    SPI_InitStructure.FirstBit      = R900PNR_SPI_FIRST_BIT;
    SPI_InitStructure.CRCPoly       = R900PNR_SPI_CRC_POLY;

    SPI_Init(R900PNR_SPIx, &SPI_InitStructure);

    /* 使能 SPI1 */
    SPI_Enable(R900PNR_SPIx, ENABLE);
}

/**
 * @brief  R900PNR SPI 接口统一初始化入口
 * @retval 0 成功, 其他失败
 */
int R900PNR_SPI_Init(void)
{
    R900PNR_SPI_GPIO_Init();
    R900PNR_SPI_EXTI_Init();
    R900PNR_SPI_Periph_Init();

    return 0;
}
