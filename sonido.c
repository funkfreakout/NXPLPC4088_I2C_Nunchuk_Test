#include <LPC407x_8x_177x_8x.h>
#include "sonido.h"
#include "dac_lpc4088.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Variables globales usadas por la función de interrupción del timer 0
 * que lleva a cabo la reproducción de sonido.
 */


static const short *ptr_muestras = NULL;
static int contador_muestras = 0;
static int reproduciendo = 0;

/***************************************************************************//**
 * \brief   Inicializar el sistema de reproducción de sonido. Debe ser llamada
 *          antes del primer uso de la función sonido_reproducir.
 */
void sonido_inicializar(void)
{
    /* Inicializar las variavles globales del módulo.
     */
    
    ptr_muestras = NULL;
    contador_muestras = 0;
    reproduciendo = 0;
    
    /* Inicializar el DAC.
     */
    
    dac_inicializar();
    
    /* Habilitar las interrupciones del timer 0.
     */
    
    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);
    __enable_irq();
    
    /* Preparar el timer 0 para generar interrupciones cada 125 us pero sin ponerlo
     * en marcha todavía.
     */
    
    LPC_TIM0->TCR = 0;
    LPC_TIM0->PC = 0;
    LPC_TIM0->TC = 0;
    LPC_TIM0->PR = PeripheralClock/1000000 - 1;
    LPC_TIM0->MR0 = 23 - 1;
    LPC_TIM0->MCR = 3;
}

/***************************************************************************//**
 * \brief   Conocer si en este momento se está reproduciendo un sonido.
 */
int sonido_en_reproduccion(void)
{
     return reproduciendo;
}

/***************************************************************************//**
 * \brief       Iniciar la reproducción de un sonido.
 *
 * \param[in]   snd     Puntero a una estructura de tipo sonido_t que indica el
 *                      sonido a reproducir.
 *
 * \return      1 => éxito, 0 => puntero snd o snd->muestras son NULL.
 */
int sonido_reproducir(const struct sonido_t* snd)
{    
    /* Comprobar que snd y snd->muestras son válidos.
     */
    
    if (snd == NULL) return 0;
    if (snd->muestras == NULL) return 0;
    
    /* Inicializar el puntero ptr_muestras a partir de snd->muestras
     * y contador_muestras a partir de tamano.
     * La variable reproduciendo se pone a 1 para indicar que se está
     * reproduciendo un sonido.
     */

    ptr_muestras = snd->muestras;
    contador_muestras = snd->tamano;
    reproduciendo = 1;                 

    /* Poner en marcha el timer.
     */
   
    LPC_TIM0->TCR = 1;
    
    return 1;
}


/***************************************************************************//**
 * \brief       Manejador de interrupción del timer 0.
 */
void TIMER0_IRQHandler(void)
{ 
    /* Borrar el flag de petición de interrupción.
     */

    LPC_TIM0->IR = 1;

    if (ptr_muestras != NULL)
    {
        /* Enviar al DAC la siguiente muestra de sonido y avanzar el
         * puntero a la siguiente y decrementar el contador de muestras. 
         */
        short dato_dac = (*ptr_muestras / 64) + 512;
        LPC_DAC->CR = dato_dac << 6;
        ptr_muestras++;
        contador_muestras--;
        
        /* Si se llegó al final del sonido, invalidar el puntero ptr_muestras,
         * parar el timer y poner a cero la variable reproduciendo.
         */
        if (contador_muestras == 0)
        {
            ptr_muestras = NULL;
            LPC_TIM0->TCR = 0;
            reproduciendo = 0;
        }
    }
}
