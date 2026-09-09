/**
 *******************************************************************************
 * @file    App_VectorTable.c
 * @brief   ITCM vector table runtime management module (see header for design)
 *******************************************************************************
 */

#include "App_VectorTable.h"
#include "n32h7xx.h"
#include "misc.h"

/* ITCM vector table base: whole APP image is linked at 0x00000000 (scatter),
 * boot copies image into ITCM and sets SCB->VTOR = 0 before jumping. */
#define APP_VECTORTABLE_BASE        (0x00000000u)

static uint32_t App_VectorSlotAddress(uint16_t slot)
{
    return APP_VECTORTABLE_BASE + ((uint32_t)slot * 4u);
}

/**
 * @brief Redirect vector table slots to user handlers.
 * @param config  array of {slot, handler} entries
 * @param count   number of entries
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
 * @brief Init: align VTOR with ITCM image and apply slot redirections.
 * @note  Call once at the beginning of main(), before any peripheral init.
 */
void App_VectorTableInit(void)
{
    /* Defensive: make sure VTOR points to our ITCM vector table.
     * boot already sets this, keep it idempotent for standalone runs. */
    SCB->VTOR = APP_VECTORTABLE_BASE;
    __DSB();
    __ISB();

    /* SDK slot convention remap: real SysTick_Handler lives at slot 250,
     * exception slot 15 points to an empty N32SysTick_Handler. */
    extern void SysTick_Handler(void);
    static const AppVectorTable_t sysTickRemap =
    {
        .slot    = APP_VECTOR_SLOT_SYSTICK,
        .handler = SysTick_Handler,
    };

    App_VectorTableRedirect(&sysTickRemap, 1u);
}