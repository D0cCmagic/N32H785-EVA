/**
 * @file    Mcal_LedGpio_Init.h
 * @brief   LED GPIO driver interface: two LEDs on PA2 / PA3.
 * @author  D0cC
 * @date    2026-09-10
 * @version V1.0.2
 *
 * @note    Targets the N32H785 using the N32H7xx standard peripheral library.
 */

#ifndef MCAL_LED_GPIO_INIT_H
#define MCAL_LED_GPIO_INIT_H

#include <stdint.h>
#include "n32h7xx.h"
#include "n32h7xx_gpio.h"
#include "n32h7xx_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LED1_PORT               GPIOA
#define LED1_PIN                GPIO_PIN_2

#define LED2_PORT               GPIOA
#define LED2_PIN                GPIO_PIN_3

/* LED1 and LED2 share GPIOA, so one clock enable covers both */
#define LED_GPIO_CLK_ENABLE()                                           \
    do {                                                                \
        RCC_EnableAHB5PeriphClk1(RCC_AHB5_PERIPHEN_M7_GPIOA, ENABLE);   \
    } while (0)

#define LED_GPIO_MODE           GPIO_MODE_OUTPUT_PP
#define LED_GPIO_PULL           GPIO_NO_PULL


#define LED_ACTIVE_HIGH         (1U)

#if (LED_ACTIVE_HIGH != 0U)
#define LED_ON_LEVEL            Bit_SET
#define LED_OFF_LEVEL           Bit_RESET
#else
#define LED_ON_LEVEL            Bit_RESET
#define LED_OFF_LEVEL           Bit_SET
#endif

typedef enum
{
    LED_ID_1 = 0,
    LED_ID_2,
    LED_ID_MAX
} Led_Id_t;

typedef enum
{
    LED_LEVEL_LOW = 0,
    LED_LEVEL_HIGH = 1
} Led_Level_t;

typedef enum
{
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1
} Led_State_t;

void Mcal_LedGpio_Init(void);

void Led_On(Led_Id_t id);
void Led_Off(Led_Id_t id);
void Led_Toggle(Led_Id_t id);
void Led_SetState(Led_Id_t id, Led_State_t state);
void Led_SetLevel(Led_Id_t id, Led_Level_t level);

void Led_AllOn(void);
void Led_AllOff(void);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_LED_GPIO_INIT_H */
