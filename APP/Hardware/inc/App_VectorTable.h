/**
 *******************************************************************************
 * @file    App_VectorTable.h
 * @brief   ITCM vector table runtime management module
 *
 * @note    SDK startup file slot convention: slot 15 (SysTick) points to an
 *          empty N32SysTick_Handler, while the real user handler is linked at
 *          slot 250 (normally remapped by CopyVectTable, which this ITCM
 *          APP does not use). ITCM is RAM, so the table is writable.
 *******************************************************************************
 */

#ifndef __APP_VECTORTABLE_H__
#define __APP_VECTORTABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef struct
{
    uint16_t slot;          /**< slot index: exception number (0~15) or IRQ number (16+) */
    void (*handler)(void); /**< target handler */
} AppVectorTable_t;

typedef enum
{
    APP_VECTOR_SLOT_SYSTICK = 15,   /**< SysTick exception, slot 15 */
} AppVectorSlot_e;

void App_VectorTableInit(void);
void App_VectorTableRedirect(const AppVectorTable_t *config, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* __APP_VECTORTABLE_H__ */