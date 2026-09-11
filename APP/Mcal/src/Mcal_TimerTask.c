/**
 * @file    Mcal_TimerTask.c
 * @brief   Timebase-driven cooperative software task scheduler.
 * @author  D0cC
 * @date    2026-09-10
 * @version V1.0.1
 *
 * @note    The 1ms interrupt only increments TimeBase.
 *          Task_Handle_Cycle() is called from the main while(1) loop.
 *          All task periods are registered in Mcal_TimerTask_Init().
 */

#include "Mcal_TimerTask.h"
#include "App_TaskServer_Handle.h"
#include "main.h"                   /* provides extern volatile uint32_t TimeBase */

static Task_Timer_t task_list[Task_Timer_MaxNum];

/**
 * @name    Mcal_TimerTask_Init
 * @brief   Reset the timebase and register every periodic task slot.
 * @param   None
 * @retval  None
 */
void Mcal_TimerTask_Init(void)
{
    uint8_t i;

    TimeBase = 0U;

    for (i = 0U; i < Task_Timer_MaxNum; i++)
    {
        task_list[i].run_flag      = 0U;
        task_list[i].reload        = 0U;
        task_list[i].last_tick     = 0U;
        task_list[i].Task_Time_Fun = 0;
    }

    /* Every task slot is registered here, once the application starts */
    Task_Create(Task_10ms_Handle,  Task_10ms_ID,  10U);
    Task_Create(Task_20ms_Handle,  Task_20ms_ID,  20U);
    Task_Create(Task_50ms_Handle,  Task_50ms_ID,  50U);
    Task_Create(Task_200ms_Handle, Task_200ms_ID, 200U);
    Task_Create(Task_300ms_Handle, Task_300ms_ID, 300U);
}

/**
 * @name    Task_Create
 * @brief   Bind a handler to a slot and start it with the given period.
 * @param   fun: handler to call when the period expires.
 * @param   Task_ID: slot index, below Task_Timer_MaxNum.
 * @param   Task_Time: period in milliseconds, must be non-zero.
 * @retval  None
 */
void Task_Create(Task_Time_HandleFun fun, Task_Handle_ID Task_ID, uint32_t Task_Time)
{
    if ((Task_ID >= Task_Timer_MaxNum) || (fun == 0) || (Task_Time == 0U))
    {
        return;
    }

    task_list[Task_ID].Task_Time_Fun = fun;
    task_list[Task_ID].reload        = Task_Time;
    task_list[Task_ID].last_tick     = TimeBase;
    task_list[Task_ID].run_flag      = 1U;
}

/**
 * @name    Task_Stop
 * @brief   Suspend a slot without losing its registration.
 * @param   Task_ID: slot index, below Task_Timer_MaxNum.
 * @retval  None
 */
void Task_Stop(Task_Handle_ID Task_ID)
{
    if (Task_ID >= Task_Timer_MaxNum)
    {
        return;
    }
    task_list[Task_ID].run_flag = 0U;
}

/**
 * @name    Task_Run
 * @brief   Resume a suspended slot and restart its period from now.
 * @param   Task_ID: slot index, below Task_Timer_MaxNum.
 * @retval  None
 */
void Task_Run(Task_Handle_ID Task_ID)
{
    if (Task_ID >= Task_Timer_MaxNum)
    {
        return;
    }
    task_list[Task_ID].run_flag  = 1U;
    task_list[Task_ID].last_tick = TimeBase;
}

/**
 * @name    Task_Handle_Cycle
 * @brief   Run every slot whose period has expired; call from the main loop.
 * @param   None
 * @retval  None
 */
void Task_Handle_Cycle(void)
{
    uint8_t  i;
    uint32_t now;

    now = TimeBase;

    for (i = 0U; i < Task_Timer_MaxNum; i++)
    {
        if (task_list[i].run_flag == 0U)
        {
            continue;
        }
        if (task_list[i].Task_Time_Fun == 0)
        {
            continue;
        }

        if ((now - task_list[i].last_tick) >= task_list[i].reload)
        {
            task_list[i].last_tick += task_list[i].reload;

            /* Resynchronize if the slot fell more than one period behind */
            if ((now - task_list[i].last_tick) >= task_list[i].reload)
            {
                task_list[i].last_tick = now;
            }

            task_list[i].Task_Time_Fun();
        }
    }
}
