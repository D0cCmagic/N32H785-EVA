#include "Mcal_InitSum.h"

/**
 * @name    Mcal_InitSum
 * @brief   Bring up every MCAL module in the required order.
 * @param   None
 * @retval  None
 */
void Mcal_InitSum(void)
{
	Mcal_LedGpio_Init();
	log_init();
	Mcal_TimerTask_Init();
}

