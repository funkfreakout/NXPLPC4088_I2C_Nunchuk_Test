#include <LPC407x_8x_177x_8x.h>
#include "sd_lpc4088_mci.h"
#include "diskio.h"
#include "rtc_lpc4088.h"

static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */
unsigned int rel_addr = 0;
static int sdhc_flag = 0;
static int direcciones_de_bloque = 0;

void delay(void)
{
    unsigned int t;
    for (t = 0; t < 1000; t++);
}

/***************************************************************************//**
 *
 */
unsigned int sd_init(void)
{
    unsigned int resp;
    unsigned int resp2[4];
    unsigned int retry = 10000;
    volatile int t;
    
    /* ---- Preparar el interfaz SD/MMC del LPC2378 ----------------------------
    */

    /* Activar alimentación del interfaz SC/MMC.
     */
    
    LPC_SC->PCONP |= (1<<28);
    
#if SD_LPC4088_MCI_DMA
    LPC_SC->PCONP |= (1<<29);
#endif    
    
    /* Configurar pines de E/S para funcionar como interfaz SD/MMC:
     *
     * MCICLK          P1[2]/ENET_TXD2/SD_CLK/PWM0[1]    
     * MCICMD          P1[3]/ENET_TXD3/SD_CMD/PWM0[2]    
     * MCIPWR          P1[5]/ENET_TX_ER/SD_PWR/PWM0[3]/-/CMP1_IN[2]/-/-
     * MCIDAT0         P1[6]/ENET_TX_CLK/SD_DAT[0]/PWM0[4]/-/CMP0_IN[4]/-/-
     * MCIDAT1         P1[7]/ENET_COL/SD_DAT[1]/PWM0[5]/-/CMP1_IN[1]/-/-
     * MCIDAT2         P1[11]/ENET_RXD2/SD_DAT[2]/PWM0[6]
     * MCIDAT3         P1[12]/ENET_RXD3/SD_DAT[3]/PWM0_CAP0/-/CMP1_OUT/-/-
     */
    
    LPC_IOCON->P1_2 =  (1<<9) | (2<<4) | 2;						/* Fast slew rate, pull-up, func = SD_CLK */
    LPC_IOCON->P1_3 =  (1<<9) | (1<<5) | (2 << 4) | 2;			/* Fast slew rate, hysteresis, pull-up, func = SD_CMD */
    LPC_IOCON->P1_5 =  (1<<7) | (2<<4) | 2;						/* Digital-mode, pull-up, func = SD_PWR */
    LPC_IOCON->P1_6 =  (1<<9) | (1<<7) | (1<<5) | (2 << 4) | 2;	/* Fast slew rate, digital mode, hysteresis, pull-up, func = SD_DAT[0] */
    LPC_IOCON->P1_7 =  (1<<9) | (1<<7) | (1<<5) | (2 << 4) | 2;	/* Fast slew rate, digital mode, hysteresis, pull-up, func = SD_DAT[1] */
    LPC_IOCON->P1_11 = (1<<9) | (1<<5) | (2<<4) | 2;            /* Fast slew rate, hysteresis, pull-up, func = SD_DAT[2] */
    LPC_IOCON->P1_12 = (1<<9) | (1<<5) | (2<<4) | 2;            /* Fast slew rate, hysteresis, pull-up, func = SD_DAT[3] */
            
    LPC_MCI->POWER = 2;   /* Power-up. */

    delay();
    
    LPC_MCI->POWER = 3;   /* Power-on. */

    delay();
    
    LPC_MCI->DATATMR = 0x1FFFFFFF;
    LPC_MCI->COMMAND = 0;
    delay();
    LPC_MCI->DATACTRL = 0;
    delay();
    
    /* Programar el registro MCIClock
    */

    #define MCI_CLOCK_CLKDIV    59      /* => MCICLK = 1 MHz (por ahora
                                         * probamos con un valor bajo). */    
    
    LPC_MCI->CLOCK = MCI_CLOCK_WIDEBUS | MCI_CLOCK_ENABLE | MCI_CLOCK_CLKDIV;    
    
#if SD_LPC4088_MCI_DMA    
    LPC_GPDMA->Config = 0x01;
    while (!(LPC_GPDMA->Config & 0x01));
#endif
    
    /* ---- Inicializar la tarjeta SD ------------------------------------------
    */
    
    /* Falta comprobar si hay tarjeta insertada (STA_NODISK)
    *
    * Si no hay tarjeta => Sta |= STA_NODISK y return Stat;
    *
    */
    
    /* Utilizar baja velocidad durante la inicialización.
    */

    mci_set_speed(200);
    
    sd_command(GO_IDLE_STATE, NO_RESPONSE, 0, 0);

    /* Necesario para tarjetas SDHC.
     */
    sd_command(SEND_IF_COND, SHORT_RESPONSE,  0x000001AA, &resp);
    
    if (resp == 5)
    {
        /* Si la respuesta es 5 (IDLE | ILLEGAL_COMMAD) la tarjeta no es SDHC.
         * Las direcciones enviadas en los comandos a la tarjeta son
         * direcciones de byte y no de bloque.
         */
        sdhc_flag = 0;
        direcciones_de_bloque = 0;
    }
    else if (resp == 0x1AA)
    {
        /* Si la respuesta es 0x1AA la tarjeta es SDHC y puede operar en el
         * rango de tensión necesario.
         */
         sdhc_flag = 1;
    }
    else
    {
        /* En caso contrario se rechaza la tarjeta.
         */
        Stat = STA_NOINIT;
        return Stat;
    }
    
    while (retry)
    {
        sd_command(APP_CMD, SHORT_RESPONSE, 0, &resp);
        if (sdhc_flag)
        {
            sd_command(SD_SEND_OP_COND, SHORT_RESPONSE, OCR_VOLTAGE_WINDOW | OCR_HCS, &resp);
        }
        else
        {
            sd_command(SD_SEND_OP_COND, SHORT_RESPONSE, OCR_VOLTAGE_WINDOW, &resp);
        }
        if (resp & 0x80000000) break;
        --retry;
    }
    if (retry == 0)
    {
        Stat = STA_NOINIT;
        return Stat;
    }

    if (sdhc_flag && (resp & (1<<30)))
    {
        /* Si el bit 30 de la respuesta está a 1 las direcciones enviadas en
         * los comandos a la tarjeta son direcciones de bloque, no de byte.
         */
        direcciones_de_bloque = 1;
    }
    else
    {
        direcciones_de_bloque = 0;
    }
    
    sd_command(ALL_SEND_CID, LONG_RESPONSE, 0, resp2);
    
    sd_command(SEND_RELATIVE_ADDR, SHORT_RESPONSE, 0, &resp);
    
    rel_addr = resp & 0xFFFF0000;
    
    sd_command(SELECT_DESELECT_CARD, SHORT_RESPONSE, rel_addr, &resp); 
    
    /* Seleccionar bus de datos de 4 bits.
    */

    sd_command(APP_CMD, SHORT_RESPONSE, rel_addr, &resp);
    sd_command(SET_BUS_WIDTH, SHORT_RESPONSE, 2, &resp);
    
    sd_command(SELECT_DESELECT_CARD, SHORT_RESPONSE, rel_addr, &resp);	
    
    sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
    
    if ((resp & 0x00000F00) != 0x00000900)  /* Tarjeta SD "ready"
                                             * y en el estado "trans".
                                             */
    {
        Stat = STA_NOINIT;
        return Stat;
    }
    
    mci_set_speed(MCI_PRESCALE_MIN);
    
    
    /* Falta comprobar si está protegida contra escritura (STA_PROTECTED)
    *
    * Si está protegida contra escritura Stat |= STA_PROTECTED.
    */
    
    Stat &= ~STA_NOINIT;
    
    return 0;
}

/***************************************************************************//**
 *
 */
void mci_set_speed(int speed)
{
    speed &= 0xFF;
    if (speed < MCI_PRESCALE_MIN) speed = MCI_PRESCALE_MIN;
    LPC_MCI->CLOCK = (LPC_MCI->CLOCK & (~0xFF)) | speed;
    delay();	
}


/***************************************************************************//**
 *
 */
unsigned int sd_status(void)
{
    return Stat;
}

#if SD_LPC4088_MCI_DMA

/****** Funciones de lectura y escritura de sectores con DMA *******************
 */

/***************************************************************************//**
 *
 */
unsigned int sd_read(unsigned char *buff, int sector, int count)
{
    unsigned int place;
    unsigned int resp;

    /* MEJORAR PARA UTILIZAR EL COMANDO DE LECTURA MÚLTIPLE.
     */

    if (direcciones_de_bloque == 0)
    {
        place = 512*sector;
    }
    else
    {
        place = sector;
    }
	
    while (count)
    {
        LPC_MCI->CLEAR = 0x7FF;

        /* ---- Programar el canal 0 del controlador DMA -----------------------
         *
         * IMPORTANTE: El registro GPDMA_CONFIG tiene que estar ya
         * configurado.
         *
         * IMPORTANTE: El controlador DMA GPDMA solo trabaja con
         * la memoria de 8Kbytes que está en el bus AHB1 (ver página
         * 9 del manual).
         *
         * IMPORTANTE: la función f_mkfs utiliza un offset 446 dentro de fs->win.
         * 446 no es divisible entre 4 por tanto la función f_mkfs no puede usarse.
         * Para poder usarla habría que modificar la función sd_write para que
         * las transferencias DMA desde la fuente sean byte a byte.
         *
         * El canal 0 del GPDMA se programa para que cada vez que el MCI
         * genere una petición DMA se transfiera un burst de 8 palabras
         * (la mitad de la profundidad del FIFO del MCI) de 32 bits
         * desde el MCI a la memoria. La longitud total de la transferencia
         * será de 512 bytes (un sector de la tarjeta).
         */
    
        LPC_GPDMA->IntTCClear = 0x01;                   /* Borrar ints. Terminal Count pendientes. */
        LPC_GPDMA->IntErrClr = 0x01;                    /* Borrar errores pendientes. */
        LPC_GPDMACH0->CSrcAddr = (uint32_t)LPC_MCI->FIFO;  /* Dirección fuente: FIFO del SD controller. */
        LPC_GPDMACH0->CDestAddr = (unsigned int)buff;   /* Dirección destino: buff. */

        LPC_GPDMACH0->CControl = 0x80000000   | /* Habilitar int. TC. Realmente no se usa. */
                                 (1 << 27)    | /* Auntoincrementar dirección destino. */
                                 (0x02 << 21) | /* Ancho de las transferencias destino: 32 bits. */
                                 (0x02 << 18) | /* Ancho de las transferencias fuente: 32 bits. */
                                 (0x02 << 15) | /* Número de transferencias destino en cada burst: 8. */
                                 (0x02 << 12) | /* Número de transferencias fuente en cada burst: 8. */
                                 512;           /* Número total de bytes a transferir (un sector). */

        LPC_GPDMACH0->CConfig = (1<<16)      |  /* Habilitar "locked transfers". */
                                (0x06 << 11) |  /* Control de flujo: el periferico (SD controller). */
                                (0x01 << 1)  |  /* Periférico fuente: SD controller. */
                                (1 << 0);       /* Habilitar canal. */

        do
        {
            sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
        }
        while ((resp & 0x00000F00) != 0x00000900);  /* Tarjeta SD "ready" y en el estado "trans". */
        
        sd_command(CMDREAD, SHORT_RESPONSE, place, &resp);
         
        LPC_MCI->DATATMR = 0x1FFFFFFF;
        LPC_MCI->DATALEN = 512;                      
        LPC_MCI->DATACTRL = (9 << 4) | (1 << 3) | (1 << 1) | 1;            
                                    
        while (!(LPC_MCI->STATUS & MCI_STATUS_DATABLOCKEND));

        /* Por precaución, esperar a que termine la transferencia DMA.
         * Quizás no sea realmente necesario.
         */
        
        while (!(LPC_GPDMA->RawIntTCStat & (1<<0)));
    
        place += 512;
        --count;
    }
		
    return 0;
}

/***************************************************************************//**
 *
 */
unsigned int sd_write(const unsigned char *buf, int sector, int count)
{
    unsigned int place;
    unsigned int resp;

    if (direcciones_de_bloque == 0)
    {
        place = 512*sector;
    }
    else
    {
        place = sector;
    }
	
     /* MEJORAR PARA UTILIZAR EL COMANDO DE ESCRITURA MÚLTIPLE.
      */
	
    while (count)
    {
        LPC_MCI->CLEAR = 0x7FF;

        /* ---- Programar el canal 0 del controlador DMA -----------
         *
         * IMPORTANTE: El registro GPDMA_CONFIG tiene que estar ya
         * configurado.
         *
         * IMPORTANTE: El controlador DMA GPDMA solo trabaja con
         * la memoria de 8Kbytes que está en el bus AHB1 (ver página
         * 9 del manual).
         *
         * IMPORTANTE: la función f_mkfs utiliza un offset 446 dentro de fs->win.
         * 446 no es divisible entre 4 por tanto la función f_mkfs no puede usarse.
         * Para poder usarla habría que modificar la función sd_write para que
         * las transferencias DMA desde la fuente sean byte a byte.
         *
         * El canal 0 del GPDMA se programa para que cada vez que el MCI
         * genere una petición DMA se transfiera un burst de 8 palabras
         * (la mitad de la profundidad del FIFO del MCI) de 32 bits
         * desde la memoria hacia el MCI. La longitud total de la transferencia
         * será de 512 bytes (un sector del disco).
         */
    
        LPC_GPDMA->IntTCClear = 0x01;               /* Borrar ints. Terminal Count pendientes. */
        LPC_GPDMA->IntErrClr = 0x01;                /* Borrar errores pendientes. */
        LPC_GPDMACH0->CSrcAddr = (unsigned int)buf; /* Dirección fuente: buff. */
        LPC_GPDMACH0->CDestAddr = (uint32_t)LPC_MCI->FIFO; /* Dirección destino: FIFO del SD controller. */

        LPC_GPDMACH0->CControl = 0x80000000   | /* Habilitar int. TC. Realmente no se usa .*/
                                 (1<<26)      | /* Auntoincrementar dirección fuente. */
                                 (0x02 << 21) | /* Ancho de las transferencias destino: 32 bits. */
                                 (0x02 << 18) | /* Ancho de las transferencias fuente: 32 bits. */
                                 (0x02 << 15) | /* Número de transferencias destino en cada burst: 8. */
                                 (0x02 << 12) | /* Número de transferencias fuente en cada burst: 8. */
                                 512;           /* Número total de bytes a transferir (un sector). */

        LPC_GPDMACH0->CConfig = (1 << 16)    |  /* Habilitar "locked transfers". */
                                (0x05 << 11) |  /* Control de flujo: el periferico (SD controller). */
                                (0x01 << 6)  |  /* Periférico destino: SD controller. */
                                (1 << 0);       /* Habilitar canal. */

        do
        {
            sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
        }
        while ((resp & 0x00000F00) != 0x00000900);
        
        sd_command(CMDWRITE, SHORT_RESPONSE, place, &resp);
            
        LPC_MCI->DATATMR = 0x1FFFFFFF;
        LPC_MCI->DATALEN = 512;		
        LPC_MCI->DATACTRL = (9 << 4) | (1 << 3) | 1;
    
        while (!(LPC_MCI->STATUS & MCI_STATUS_DATABLOCKEND));

        place += 512;
        --count;
    }

    return 0;
}

#else   /* SD_LPC4088_MCI_DMA */

/****** Funciones de lectura y escritura de sectores sin DMA *******************
 */

/***************************************************************************//**
 *
 */
unsigned int sd_read(unsigned char *buff, int sector, int count)
{
    unsigned int i;
    unsigned int place;
    unsigned int resp;
    unsigned int words;
    unsigned int d;
    volatile unsigned t;
    
    if (direcciones_de_bloque == 0)
    {
        place = 512*sector;
    }
    else
    {
        place = sector;
    }

     /* MEJORAR PARA UTILIZAR EL COMANDO DE LECTURA MÚLTIPLE.
      */
	
    while (count)
    {
        do
        {
            sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
        }
        while( (resp & 0x00000F00) != 0x00000900);  /* Tarjeta SD "ready" y en el estado "trans". */
                   
        //sd_command(CMDREAD, SHORT_RESPONSE, place, &resp);
        //sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
            
        sd_command(CMDREAD, SHORT_RESPONSE, place, &resp);        
        
        LPC_MCI->DATATMR = 0x1FFFFFFF;
        LPC_MCI->DATALEN = 512;
        LPC_MCI->DATACTRL = (9 << 4) | 2 | 1;             
        
        words = 512/4;
            
        while (words)              
        {		            
            while (!(LPC_MCI->STATUS & MCI_STATUS_RXFIFOHALFFULL));
            for (i = 0; i < 8; i++)
            {
                d = LPC_MCI->FIFO[0];
                *buff++ = d & 0xFF;
                *buff++ = (d >> 8 ) & 0xFF;
                *buff++ = (d >> 16) & 0xFF;
                *buff++ = (d >> 24) & 0xFF;						 
                words--;
            }
        }
        
        while (!(LPC_MCI->STATUS & MCI_STATUS_DATABLOCKEND));	
    
        place += 512;
        --count;
    }
		
    return 0;
}

/***************************************************************************//**
 *
 */
unsigned int sd_write(const unsigned char *buf, int sector, int count)
{
    unsigned int place;
    unsigned int i;
    unsigned int resp;
    unsigned int words;
    unsigned int d, s;
    volatile int t;

    if (direcciones_de_bloque == 0)
    {
        place = 512*sector;
    }
    else
    {
        place = sector;
    }
	
    LPC_MCI->CLEAR = 0x7FF;
    LPC_MCI->DATACTRL = 0;
    for (t = 0; t < 16; t++);

    /* MEJORAR PARA UTILIZAR EL COMANDO DE ESCRITURA MÚLTIPLE.
     */
	
    while (count)
    {
        do
        {
            sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
        }
        while ((resp & 0x00000F00) != 0x00000900);

        /* do
         * {
         *     sd_command(SEND_STATUS, SHORT_RESPONSE, rel_addr, &resp);
         * }
         * while( !(resp & 0x00000100));   // Bit de estado "READY_FOR_DATA"
         */
     
        LPC_MCI->CLEAR = 0x7FF;
        
        sd_command(CMDWRITE, SHORT_RESPONSE, place, &resp);        
        
        LPC_MCI->DATATMR = 0x1FFFFFFF;
        LPC_MCI->DATALEN = 512;		             
        LPC_MCI->DATACTRL = (9 << 4) | 1;
        
        words = 512/4;
           
        for (i = 0; i < 16; i++)
        {
            d = *buf + (*(buf+1) << 8) + (*(buf+2) << 16) + (*(buf+3) << 24);
            LPC_MCI->FIFO[0] = d;
            buf += 4;
            words--;
        }
        
        while (words)
        {
            while (!(LPC_MCI->STATUS & MCI_STATUS_TXFIFOHALFEMPTY));

            for (i = 0; i < 8; i++)
            {
                d = *buf + (*(buf+1) << 8) + (*(buf+2) << 16) + (*(buf+3) << 24);
                LPC_MCI->FIFO[0] = d;
                buf+=4;
                words--;
            }
        }
    
        while (!((s = LPC_MCI->STATUS) & MCI_STATUS_DATABLOCKEND));
    
        LPC_MCI->CLEAR = 0x7FF;

        place += 512;
        --count;
    }

    return 0;
}

#endif   /* SD_LPC4088_MCI_DMA */

/***************************************************************************//**
 *
 */
unsigned int sd_ioctl(int ctrl, void *buff)
{
    unsigned short c_size, c_size_mult, read_bl_len;
    unsigned int resp[4];

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch (ctrl)
    {
    case CTRL_SYNC:
        break;

    case GET_SECTOR_COUNT:
        sd_command(SELECT_DESELECT_CARD, SHORT_RESPONSE, 0, resp);	

        sd_command(CMDREADCSD, LONG_RESPONSE, rel_addr, resp);
	
        c_size = (resp[2] & 0x3FF) | ((resp[1] >> 30) & 0x03);
        read_bl_len = (resp[2] >> 16) & 0x0F;
        c_size_mult = (resp[1] >> 15) & 0x07;
	
        *((unsigned int *)buff) = ((c_size + 1)*(2 << (c_size_mult + 2))*(2 << read_bl_len)) >> 9;

        sd_command(SELECT_DESELECT_CARD, SHORT_RESPONSE, rel_addr, resp);	
        break;

    case GET_BLOCK_SIZE:
        *((unsigned int *)buff) = 512;
        break;
    }

    return RES_OK;
}

/***************************************************************************//**
 *
 */
unsigned int sd_command(unsigned int cmd, unsigned int resp_type,
                        unsigned int arg, unsigned int* resp)
{
    unsigned int temp;
    volatile unsigned int i;
    unsigned int CmdStatus;

    cmd &= 0x3F;

    while ((CmdStatus = LPC_MCI->STATUS) & MCI_STATUS_CMDACTIVE)
    {
        LPC_MCI->COMMAND = 0;
        LPC_MCI->CLEAR = CmdStatus | MCI_STATUS_CMDACTIVE;
    }

    delay();
    
    if (resp_type != NO_RESPONSE)
    {
        cmd |= MCI_COMMAND_RESPONSE;
        if (resp_type == LONG_RESPONSE)
        {
            cmd |= MCI_COMMAND_LONGRESP;
        }
    }
    
    LPC_MCI->ARGUMENT = arg;
    LPC_MCI->COMMAND = MCI_COMMAND_ENABLE | cmd;
    
    delay();
    
    if (resp_type == NO_RESPONSE)
    {
        while (!(LPC_MCI->STATUS & (MCI_STATUS_CMDCRCFAIL | MCI_STATUS_CMDSENT)));		
    }
    else
    {
        while (!(LPC_MCI->STATUS & (MCI_STATUS_CMDCRCFAIL | MCI_STATUS_CMDSENT
               | MCI_STATUS_CMDRESPEND | MCI_STATUS_CMDTIMEOUT)));

        if (resp_type == LONG_RESPONSE)
        {
            if (resp != 0)
            {
                *resp++ = LPC_MCI->RESP3;
                *resp++ = LPC_MCI->RESP2;
                *resp++ = LPC_MCI->RESP1;
                *resp = LPC_MCI->RESP0;
            }
        }
        else
        {
            if (resp != 0) *resp = LPC_MCI->RESP0;			
        }		
    }
    temp = LPC_MCI->STATUS;
    LPC_MCI->CLEAR = 0x7FF;	

    return temp;
}

/***************************************************************************//**
 *
 */
DWORD get_fattime( void )
{
    unsigned int dia, mes, ano, horas, minutos, segundos;

    /* Descomentar si se usa el RTC.
     */
     rtc_leer_hora(&horas, &minutos, &segundos);
     rtc_leer_fecha(&dia, &mes, &ano);
     

    /* Comentar si se usa el RTC.
     */
/*
    dia = 1;
    mes = 1;
    ano = 2015;
    horas = 0;
    minutos = 0;
    segundos = 0;
*/
    return ( ( (ano - 1980) << 25 ) |
             ( mes << 21 ) |
             ( dia << 16 ) |
             ( horas << 11 ) |
             ( minutos << 5 ) |
             ( segundos>>1 ) ) ;
}

