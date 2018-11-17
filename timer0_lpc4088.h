/***************************************************************************//**
 * \file    timer0_lpc4088.h
 *
 * \brief   Funciones de retardo con el timer 0.
 */

#ifndef TIMER0_LPC4088_H
#define TIMER0_LPC4088_H

#include <stdint.h>

void timer0_retardo_ms(uint32_t ms);
void timer0_retardo_us(uint32_t us);

#endif  /* SST25VF016B_SSP0_LPC4088_H */
