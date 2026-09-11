/**
 * @file    App_VectorTable.c
 * @brief   ITCM vector table runtime management module (see header for design).
 */

#include "App_VectorTable.h"
#include "n32h7xx.h"
#include "misc.h"

/* ITCM vector table base: the whole APP image is linked at 0x00000000 (scatter),
 * the boot copies it into ITCM and sets SCB->VTOR = 0 before jumping. */
#define APP_VECTORTABLE_BASE        (0x00000000u)

/**
 * @name    App_VectorSlotAddress
 * @brief   Compute the address of one vector table slot.
 * @param   slot: exception number (0~15) or IRQ number (16+).
 * @retval  Address of the slot.
 */
static uint32_t App_VectorSlotAddress(uint16_t slot)
{
    return APP_VECTORTABLE_BASE + ((uint32_t)slot * 4u);
}

/**
 * @name    App_VectorTableRedirect
 * @brief   Redirect the given vector table slots to user handlers.
 * @param   config: array of {slot, handler} entries.
 * @param   count: number of entries in the array.
 * @retval  None
 */
void App_VectorTableRedirect(const AppVectorTable_t *config, uint32_t count)
{
    if (config == (AppVectorTable_t *)0)
    {
        return;
    }

    __disable_irq();

    for (uint32_t i = 0u; i < count; i++)
    {
        *(volatile uint32_t *)App_VectorSlotAddress(config[i].slot) =
            (uint32_t)config[i].handler;
    }

    __DSB();
    __ISB();
    __enable_irq();
}

/**
 * @name    App_VectorTableInit
 * @brief   Align VTOR with the ITCM image and apply the slot redirections.
 * @param   None
 * @retval  None
 */
void App_VectorTableInit(void)
{
    /* Keep VTOR on our ITCM table; the boot already set it */
    SCB->VTOR = APP_VECTORTABLE_BASE;
    __DSB();
    __ISB();

    /* The real SysTick_Handler is linked at slot 250, not exception slot 15 */
    extern void SysTick_Handler(void);
    static const AppVectorTable_t sysTickRemap =
    {
        .slot    = APP_VECTOR_SLOT_SYSTICK,
        .handler = SysTick_Handler,
    };

    App_VectorTableRedirect(&sysTickRemap, 1u);
}
