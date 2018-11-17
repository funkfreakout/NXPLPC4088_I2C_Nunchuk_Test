#include "i2c0_lpc4088.h"
#include "nunchuk.h"

/***************************************************************************//**
 * \brief       Inicializar el interfaz I2C 0 del LPC4088 como master.
 *				Envia la direccion esclava del nunchuk e inicializa la 
 *				transmision.
 */
void nunchuk_init(void){
	i2c0_inicializar(SCLL,SCLH);
	i2c0_start();
	i2c0_transmitir_byte(SLAVE_ID_NUNCHUK << 1);
	//recibir ACK
	i2c0_transmitir_byte(0x40);
	//recibir ACK
	i2c0_transmitir_byte(0x00);
	//recibir ACK
	i2c0_stop();
}

/***************************************************************************//**
 * \brief       Inicializar el interfaz I2C 0 del LPC4088 como master.
 *				Envia un '0' indicando que empieza la recepcion de datos.
 */
void nunchuk_send_zero(void){
	//i2c0_inicializar(SCLL,SCLH);
	i2c0_start();
	i2c0_transmitir_byte(SLAVE_ID_NUNCHUK << 1);
	//recibir ACK
	i2c0_transmitir_byte(0x00);
	//recibir ACK
	i2c0_stop();

}

/***************************************************************************//**
 * \brief       Decodifica los datos enviados por el nunchuk
 *
 * \param[in]   c: Caracter a decodificar.
 *
 */
unsigned char nunchuk_decode_byte(unsigned char c){
	c = (c ^ 0x17) + 0x17;

	return c;
}

/***************************************************************************//**
 * \brief       El nunchuk se encuentra en modo lectura y envia al master los datos 		
 *				codificados.	
 * \param[in]   ptr: Puntero donde se almacenan los datos enviados.
 *
 */
void receive_nunchuk(unsigned char* ptr){
	int cnt = 0;
	
	i2c0_start();
	i2c0_transmitir_byte(0xA5);
	
	while(cnt < 5) {i2c0_recibir_byte(ptr,ACK);ptr++;cnt++;}
	i2c0_recibir_byte(ptr,NACK);
	i2c0_stop();
}
