/***************************************************************************//**
 * \file    i2c0_lpc4088.h
 *
 * \brief   Funciones de manejo del interfaz I2C 0 del LPC4088. El LPC4088
 *          actúa como master.
 *
 */

#ifndef I2C0_LPC4088_H
#define I2C0_LPC4088_H

/* Timeout al crear las condiciones start y restart.
 */

#define I2C0_TIMEOUT        1000000

/* Símbolos para ACK y NACK.
 */

#define ACK 1
#define NACK 0

/* Prototipos de funciones.
 */

void i2c0_inicializar(unsigned int scll, unsigned int sclh);
int i2c0_start(void);
int i2c0_restart(void);
void i2c0_stop(void);
int i2c0_transmitir_byte(unsigned char dato);
int i2c0_recibir_byte(unsigned char *ptr, int ack);

#endif /*I2C0_LPC4088_H*/
