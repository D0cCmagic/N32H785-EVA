#ifndef _MCAL_TIMERTASK_H_
#define _MCAL_TIMERTASK_H_

#include <stdint.h>

#define Task_Timer_MaxNum        5

typedef void (*Task_Time_HandleFun)(void);

typedef struct
{
    uint8_t             run_flag;
    uint32_t            reload;
    uint32_t            last_tick;
    Task_Time_HandleFun Task_Time_Fun;
} Task_Timer_t;

typedef enum
{
    Task_10ms_ID,
    Task_20ms_ID,
    Task_50ms_ID,
    Task_200ms_ID,
    Task_300ms_ID,
} Task_Handle_ID;

void Mcal_TimerTask_Init(void);
void Task_Create(Task_Time_HandleFun fun, Task_Handle_ID Task_ID, uint32_t Task_Time);
void Task_Stop(Task_Handle_ID Task_ID);
void Task_Run(Task_Handle_ID Task_ID);
void Task_Handle_Cycle(void);

#endif /* _MCAL_TIMERTASK_H_ */
