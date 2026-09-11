/**
 * @file    r900pnr_spi_init.c
 * @brief   R900PNR SPI interface setup: GPIO / SPI / EXTI / NVIC.
 * @note    Targets the N32H785 dev board, M7 core, standard peripheral library.
 *          Logging comes from the LWIP log.c, so this file does not bring up a UART.
 *          DMA is not used: hgic_sdspi_v2.c shares one buffer between TX and RX,
 *          so DMA with the D-Cache corrupts stack variables. CPU polling is used instead.
 */
#include "n32h7xx.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_spi.h"
#include "n32h7xx_exti.h"
#include "n32h7xx_rcc.h"
#include "r900pnr_spi_cfg.h"
#include "r900pnr_spi_api.h"

/**
 * @name    R900PNR_SPI_GPIO_Init
 * @brief   Configure the SPI1 pins: SCK/MISO/MOSI as alternate function,
 *          CS as a GPIO output held high, INT as a floating input.
 * @param   None
 * @retval  None
 */
static void R900PNR_SPI_GPIO_Init(void)
{
    GPIO_InitType GPIO_InitStructure;

    /* GPIOA clock covers SPI1 and the interrupt pin alike */
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

    /* Software chip select, idle high */
    GPIO_InitStructure.Pin          = R900PNR_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0;
    GPIO_InitPeripheral(R900PNR_SPI_CS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(R900PNR_SPI_CS_PORT, R900PNR_SPI_CS_PIN);

    /* R900PNR interrupt pin, driven low when data is available */
    GPIO_InitStructure.Pin          = R900PNR_SPI_INT_PIN;
    GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Pull    = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF0;
    GPIO_InitPeripheral(R900PNR_SPI_INT_PORT, &GPIO_InitStructure);
}

/**
 * @name    R900PNR_SPI_EXTI_Init
 * @brief   Route PA8 to EXTI line 8 and enable the falling-edge interrupt.
 * @param   None
 * @retval  None
 */
static void R900PNR_SPI_EXTI_Init(void)
{
    EXTI_InitType EXTI_InitStructure;

    /* Map EXTI_LINE8 onto PA8 */
    GPIO_ConfigEXTILine(R900PNR_SPI_INT_EXTI_LINE, R900PNR_SPI_INT_EXTI_GPIO);

    /* Interrupt mode, falling edge */
    EXTI_InitStruct(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line    = R900PNR_SPI_INT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /* Clear any stale pending bit, then enable the NVIC line */
    EXTI_ClrITPendBit(R900PNR_SPI_INT_EXTI_LINE);
    NVIC_SetPriority(R900PNR_SPI_INT_IRQn, 5);
    NVIC_EnableIRQ(R900PNR_SPI_INT_IRQn);
}

/**
 * @name    R900PNR_SPI_Periph_Init
 * @brief   Bring up SPI1 in master, full-duplex, Mode 0.
 * @param   None
 * @retval  None
 */
static void R900PNR_SPI_Periph_Init(void)
{
    SPI_InitType SPI_InitStructure;

    R900PNR_SPIx_APBx_CLK_CMD(R900PNR_SPIx_CLK, ENABLE);

    SPI_I2S_DeInit(R900PNR_SPIx);

    SPI_InitStruct(&SPI_InitStructure);

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

    SPI_Enable(R900PNR_SPIx, ENABLE);
}

/**
 * @name    R900PNR_SPI_Init
 * @brief   Single entry point that brings up GPIO, EXTI and the SPI1 peripheral.
 * @param   None
 * @retval  0 on success.
 */
int R900PNR_SPI_Init(void)
{
    R900PNR_SPI_GPIO_Init();
    R900PNR_SPI_EXTI_Init();
    R900PNR_SPI_Periph_Init();

    return 0;
}
