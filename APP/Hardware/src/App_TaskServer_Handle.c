/**
 * @file    App_TaskServer_Handle.c
 * @brief   Cooperative scheduler task slots called from the main loop.
 * @author  D0cC
 * @date    2026-09-10
 * @version V1.0.0
 *
 * @note    None
 */

#include "App_TaskServer_Handle.h"
#include "Mcal_InitSum.h"



/**
 * @name    Task_10ms_Handle
 * @brief   Empty slot; the radio poll now runs directly in the main loop.
 * @param   None
 * @retval  None
 */
void Task_10ms_Handle(void)
{
    /* Kept registered so the 10ms period stays available */
}

/**
 * @name    Task_20ms_Handle
 * @brief   Poll for an OTA upgrade request.
 * @param   None
 * @retval  None
 */
void Task_20ms_Handle(void)
{
	OTA_UpdateCheck_Handle();
}

/**
 * @name    Task_50ms_Handle
 * @brief   Empty slot, reserved for future use.
 * @param   None
 * @retval  None
 */
void Task_50ms_Handle(void)
{
}

/**
 * @name    Task_200ms_Handle
 * @brief   Blink LED1.
 * @param   None
 * @retval  None
 */
void Task_200ms_Handle(void)
{
    Led_Toggle(LED_ID_1);   /* PA2 */
}

/**
 * @name    Task_300ms_Handle
 * @brief   Blink LED2.
 * @param   None
 * @retval  None
 */
void Task_300ms_Handle(void)
{
    Led_Toggle(LED_ID_2);   /* PA3 */
}
