/***************************************************************************//**
 * \file    nunchuk.h
 *
 * \brief   Funciones de manejo del nunchuk wii por medio de i2c
 *
 */

#ifndef NUNCHUK_H
#define NUNCHUK_H

#define SCLH 300
#define SCLL 300
#define SLAVE_ID_NUNCHUK 0x52
#define BYTES_NUNCHUK 6

void nunchuk_init(void);
void nunchuk_send_zero(void);
unsigned char nunchuk_decode_byte(unsigned char c);
void receive_nunchuk(unsigned char* ptr);

#endif
