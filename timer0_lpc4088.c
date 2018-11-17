/***************************************************************************//**
 * \file    timer0_lpc4088.c
 *
 * \brief   Funciones de retardo con el timer 0.
 */

#include <LPC407x_8x_177x_8x.h>
#include "timer0_lpc4088.h"

void timer0_retardo_ms(uint32_t ms)
{
    if (ms == 0) return;

    LPC_TIM0->TCR = 0;
    LPC_TIM0->TC = 0;
    LPC_TIM0->PC = 0;
    LPC_TIM0->PR = PeripheralClock/1000000 - 1;
    LPC_TIM0->MR0 = 1000*ms - 1;
    LPC_TIM0->MCR |= 7;
    LPC_TIM0->IR = 1;
    LPC_TIM0->TCR = 1;    
    while ((LPC_TIM0->IR & 1) == 0);
}

void timer0_retardo_us(uint32_t us)
{
    if (us == 0) return;

    LPC_TIM0->TCR = 0;
    LPC_TIM0->TC = 0;
    LPC_TIM0->PC = 0;
    LPC_TIM0->PR = PeripheralClock/10000000 - 1;
    LPC_TIM0->MR0 = 10*us - 1;
    LPC_TIM0->MCR |= 7;
    LPC_TIM0->IR = 1;
    LPC_TIM0->TCR = 1;    
    while ((LPC_TIM0->IR & 1) == 0);
}
