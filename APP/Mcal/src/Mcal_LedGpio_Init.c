/**
 * @file    Mcal_LedGpio_Init.c
 * @brief   LED GPIO driver implementation: two LEDs on PA2 / PA3.
 * @author  D0cC
 * @date    2026-09-10
 * @version V1.0.3
 *
 * @note    Targets the N32H785 using the N32H7xx standard peripheral library.
 */

#include "Mcal_LedGpio_Init.h"

typedef struct
{
    GPIO_Module *port;
    uint16_t     pin;
} Led_Hw_t;

static const Led_Hw_t Led_Hw[LED_ID_MAX] =
{
    { LED1_PORT, LED1_PIN },   /* PA2 */
    { LED2_PORT, LED2_PIN },   /* PA3 */
};

static void Led_WritePin(Led_Id_t id, Bit_OperateType action);

/**
 * @name    Mcal_LedGpio_Init
 * @brief   Configure both LED pins as push-pull outputs and turn them off.
 * @param   None
 * @retval  None
 */
void Mcal_LedGpio_Init(void)
{
    GPIO_InitType gpio_init;
    uint8_t       i;

    LED_GPIO_CLK_ENABLE();

    for (i = 0U; i < (uint8_t)LED_ID_MAX; i++)
    {
        GPIO_InitStruct(&gpio_init);
        gpio_init.Pin       = Led_Hw[i].pin;
        gpio_init.GPIO_Mode = LED_GPIO_MODE;
        gpio_init.GPIO_Pull = LED_GPIO_PULL;

        /* Drive the off level before enabling the output to avoid a power-on flash */
        GPIO_WriteBit(Led_Hw[i].port, Led_Hw[i].pin, LED_OFF_LEVEL);
        GPIO_InitPeripheral(Led_Hw[i].port, &gpio_init);
    }

    Led_AllOff();
}

/**
 * @name    Led_On
 * @brief   Turn the specified LED on.
 * @param   id: identifier of the LED.
 * @retval  None
 */
void Led_On(Led_Id_t id)
{
    if (id >= LED_ID_MAX)
    {
        return;
    }
    Led_WritePin(id, LED_ON_LEVEL);
}

/**
 * @name    Led_Off
 * @brief   Turn the specified LED off.
 * @param   id: identifier of the LED.
 * @retval  None
 */
void Led_Off(Led_Id_t id)
{
    if (id >= LED_ID_MAX)
    {
        return;
    }
    Led_WritePin(id, LED_OFF_LEVEL);
}

/**
 * @name    Led_Toggle
 * @brief   Invert the current output level of the specified LED.
 * @param   id: identifier of the LED.
 * @retval  None
 */
void Led_Toggle(Led_Id_t id)
{
    if (id >= LED_ID_MAX)
    {
        return;
    }

    if (GPIO_ReadOutputDataBit(Led_Hw[id].port, Led_Hw[id].pin) != 0U)
    {
        GPIO_ResetBits(Led_Hw[id].port, Led_Hw[id].pin);
    }
    else
    {
        GPIO_SetBits(Led_Hw[id].port, Led_Hw[id].pin);
    }
}

/**
 * @name    Led_SetState
 * @brief   Drive the specified LED to an explicit on or off state.
 * @param   id: identifier of the LED.
 * @param   state: LED_STATE_ON or LED_STATE_OFF.
 * @retval  None
 */
void Led_SetState(Led_Id_t id, Led_State_t state)
{
    if (state == LED_STATE_ON)
    {
        Led_On(id);
    }
    else
    {
        Led_Off(id);
    }
}

/**
 * @name    Led_SetLevel
 * @brief   Drive the specified LED to a raw electrical level.
 * @param   id: identifier of the LED.
 * @param   level: LED_LEVEL_HIGH or LED_LEVEL_LOW.
 * @retval  None
 */
void Led_SetLevel(Led_Id_t id, Led_Level_t level)
{
    if (id >= LED_ID_MAX)
    {
        return;
    }
    Led_WritePin(id, (level == LED_LEVEL_HIGH) ? Bit_SET : Bit_RESET);
}

/**
 * @name    Led_AllOn
 * @brief   Turn every LED on.
 * @param   None
 * @retval  None
 */
void Led_AllOn(void)
{
    uint8_t i;
    for (i = 0U; i < (uint8_t)LED_ID_MAX; i++)
    {
        Led_WritePin((Led_Id_t)i, LED_ON_LEVEL);
    }
}

/**
 * @name    Led_AllOff
 * @brief   Turn every LED off.
 * @param   None
 * @retval  None
 */
void Led_AllOff(void)
{
    uint8_t i;
    for (i = 0U; i < (uint8_t)LED_ID_MAX; i++)
    {
        Led_WritePin((Led_Id_t)i, LED_OFF_LEVEL);
    }
}

/**
 * @name    Led_WritePin
 * @brief   Write a raw level to the pin backing the specified LED.
 * @param   id: identifier of the LED.
 * @param   action: Bit_SET or Bit_RESET.
 * @retval  None
 */
static void Led_WritePin(Led_Id_t id, Bit_OperateType action)
{
    GPIO_WriteBit(Led_Hw[id].port, Led_Hw[id].pin, action);
}
