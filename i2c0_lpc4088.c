/***************************************************************************//**
 * \file    i2c0_lpc4088.c
 *
 * \brief   Funciones de manejo del interfaz I2C 0 del LPC4088. El LPC4088
 *          act�a como master.
 *
 */

#include <LPC407x_8x_177x_8x.h>
#include "i2c0_lpc4088.h"

/***************************************************************************//**
 * \brief       Inicializar el interfaz I2C 0 del LPC4088 como master.
 *
 * \param[in]   scll: Valor para el registro SCLL.
 * \param[in]   sclh: Valor para el registro SCLH.
 *
 */
void i2c0_inicializar(unsigned int scll, unsigned int sclh) 
{
    /* Aplicar alimentaci�n al m�dulo I2C0.
     */ 

    LPC_SC->PCONP |= (1 << 7);
    
    /* Configurar P0[27] como I2C0_SDA y P0[28] como I2C0_SCL.
     */

    LPC_IOCON->P0_27 = 1;
    LPC_IOCON->P0_28 = 1;
	
    /* Borrar flags.
     */

    LPC_I2C0->CONCLR = (1 << 6) | (1 << 5) | (1 << 3) | (1 << 2);    

    /* Configurar velocidad.
     */

    LPC_I2C0->SCLL   = scll;
    LPC_I2C0->SCLH   = sclh;

    /* Habilitar el interfaz.
     */

    LPC_I2C0->CONSET = 1 << 6; 
}

/***************************************************************************//**
 * \brief   Crear condici�n de start.
 *
 * \return  Valor del registro STAT o -1 si expir� el timeout.
 *
 */
int i2c0_start(void)
{   
    /* Borrar el flag de interrupci�n SI (bit 3) del registro CON.
     */
    
    LPC_I2C0->CONCLR = 1 << 3; 
    
    /* Crear condici�n START activando el bit STA (bit 5) del registro CON.
     */
    LPC_I2C0->CONSET = 1 << 5;
     
    /* Esperar hasta que se genere START (bit SI (bit 3) del registro CON a 1).
     */

    while(((LPC_I2C0->CONSET) & (1 << 3)) == 0);

    /* Borrar el flag STA (bit 5) del registro CON.
     */

    LPC_I2C0->CONCLR = 1 << 5;

    /* Retornar el estado del interfaz.
     */

    return LPC_I2C0->STAT;
}

/***************************************************************************//**
 * \brief   Crear condici�n de stop.
 *
 */
void i2c0_stop(void)
{
    /* Borrar el flag de interrupci�n SI (bit 3) del registro CON.
     */
    LPC_I2C0->CONCLR = 1 << 3; 
    
    /* Crear condici�n STOP activando el bit STO (bit 4) del registro CON.
     */
    LPC_I2C0->CONSET = 1 << 4;

    /* Despu�s de crear la condici�n STOP el bit SI del registro CON no
     * cambia por lo que no podemos basarnos en �l para detectar que se
     * ha generado la condici�n de STOP. Pero el bit STO del registro CON
     * s� cambia a 0. Usaremos esto para detectar que ya se ha generado
     * la condici�n de STOP.
     */
    while(((LPC_I2C0->CONSET) & (1 << 4)) == 0);
}

/***************************************************************************//**
 * \brief       Transmitir un byte a un esclavo a trav�s del I2C 0.
 *
 * \param[in]   dato    byte a transmitir.
 *
 * \return      Valor del registro STAT.
 *
 */
int i2c0_transmitir_byte(unsigned char dato) 
{
	
	    LPC_I2C0->DAT = dato;
    /* Borrar el flag de interrupci�n SI (bit 3) del registro CON.
     */
    
    LPC_I2C0->CONCLR = 1 << 3; 

    /* Escribir el dato a enviar en el registro de datos DAT.
     */
    

    
    /* Esperar hasta que el bit SI (bit 3) del registro CON a 1.
     * Esto indica que ha terminado la transmisi�n.
     */    

    while(((LPC_I2C0->CONSET) & (1 << 3)) == 0);
    
    /* Retornar el estado del interfaz.
     */

    return LPC_I2C0->STAT;
}

/***************************************************************************//**
 * \brief       Recibir un byte desde un esclavo a trav�s del I2C 0.
 *
 * \param[in]   ptr     direcci�n donde almacenar el dato.
 * \param[in]   ack     0 => generar un ACK, 1 => generar un NACK.
 *
 * \return      Valor del registro I2C0STAT.
 *
 */
int i2c0_recibir_byte(unsigned char *ptr, int ack)
{
    /* Borrar el flag de interrupci�n SI (bit 3) del registro CON.
     */
    
    

    /* Si queremos reconocer el dato, activar el bit AA (bit 2) del registro CON.
     * Si no queremos reconocer el dato, borrar el bit AA.
     */
    
    if(ack) LPC_I2C0->CONSET = 1 << 2;
    else LPC_I2C0->CONCLR = 1 << 2;
	
	LPC_I2C0->CONCLR = 1 << 3;

    /* Esperar hasta que el bit SI (bit 3) del registro CON a 1.
     * Esto indica que ha terminado la recepci�n.
     */

    while(((LPC_I2C0->CONSET) & (1 << 3)) == 0);
	
    
    /* Recoger el dato recibido del registro de datos DAT y almacenarlo en la
     * posici�n indicada por el puntero ptr.
     */

    *ptr = LPC_I2C0->DAT;
    
    /* Retornar el estado del interfaz.
     */

    return LPC_I2C0->STAT;
}

