/***************************************************************************//**
 * \file    dac_lpc4088.c
 *
 * \brief   Funciones de manejo del DAC del LPC4088.
 */
 
#ifndef DAC_LPC4088_H
#define DAC_LPC4088_H

void dac_inicializar(void);
int dac_convertir(int valor);

#endif /* DAC_LPC4088_H */
