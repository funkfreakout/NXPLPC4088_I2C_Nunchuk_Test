/***************************************************************************//**
 * \file    timer1_lpc4088.h
 *
 * \brief   Funciones de retardo con el timer 1.
 */

#ifndef TIMER1_LPC4088_H
#define TIMER1_LPC4088_H

#include <stdint.h>

void timer1_retardo_ms(uint32_t ms);
void timer1_retardo_us(uint32_t us);

#endif 
