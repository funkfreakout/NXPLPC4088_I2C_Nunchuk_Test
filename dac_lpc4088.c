/***************************************************************************//**
 * \file    dac_lpc4088.c
 *
 * \brief   Funciones de manejo del DAC del LPC4088.
 */

#include "dac_lpc4088.h"
#include <LPC407x_8x_177x_8x.h>

/***************************************************************************//**
 * \brief
 */
void dac_inicializar(void)
{
    LPC_IOCON->P0_26 = (1<<16) | 2;
}

/***************************************************************************//**
 * \brief
 */
int dac_convertir(int valor)
{
    if (valor < 0 || valor > 1023) return 0;
    
    LPC_DAC->CR = valor << 6;
    
    return 1;
}
