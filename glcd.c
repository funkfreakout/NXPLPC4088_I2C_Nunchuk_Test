/***************************************************************************//**
 * \brief   Funciones de manejo del LCD de 4.3" desde la tarjeta Embedded Artist
 *          Developer's Kit.
 */
 
#include <LPC407x_8x_177x_8x.h>
#include "glcd.h"
#include "sdram.h"
#include <string.h>

/***************************************************************************//**
 */
static void glcd_timer0_retardo_ms(int ms)
{
    if (ms <= 0) return;
    LPC_TIM0->TCR = 0;
    LPC_TIM0->PC = 0;
    LPC_TIM0->TC = 0;
    LPC_TIM0->PR = PeripheralClock/1000000 - 1;
    LPC_TIM0->MR0 = 1000*ms - 1;
    LPC_TIM0->MCR |= 7;
    LPC_TIM0->IR = 1;
    LPC_TIM0->TCR = 1;
    while ((LPC_TIM0->IR & 1) == 0);
}

/***************************************************************************//**
 * \brief       Inicializar el interfaz I2C 0 del LPC237 como master.
 *
 * \param[in]   scll: Valor para el registro I20SCLL.
 * \param[in]   sclh: Valor para el registro I20SCLH.
 *
 */
static void glcd_i2c0_inicializar(unsigned int scll, unsigned int sclh) 
{
    /* Aplicar alimentación al módulo I2C0.
     */ 

    LPC_SC->PCONP |= (1 << 7);
    
    /* Configurar P0.27 como SDA0 y P0.28 como SCL0.
     */

    LPC_IOCON->P0_27 |= 1;
		LPC_IOCON->P0_28 |= 1;
	
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
 * \brief   Crear condición de start.
 *
 * \return  Valor del registro I2C0STAT.
 *
 */
static int glcd_i2c0_start(void)
{
    int contador = 0;
    
    /* Crear condición START.
     */
    LPC_I2C0->CONSET = 1 << 5;
 
    /* Esperar hasta que se transmita START o expire el I2C0_TIMEOUT.
     */

    while ((LPC_I2C0->CONSET & (1 << 3)) == 0)
    {
        contador++;
        if (contador > 0x01000000) return -1;
    }

    LPC_I2C0->CONCLR = 1 << 5;

    return LPC_I2C0->STAT;
}

/***************************************************************************//**
 * \brief   Crear condición de stop.
 *
 */
static void glcd_i2c0_stop(void)
{
    LPC_I2C0->CONCLR = 1 << 3;     
    LPC_I2C0->CONSET = 1 << 4; 
                
    while (LPC_I2C0->CONSET & (1 << 4));    
}

/***************************************************************************//**
 * \brief       Transmitir un byte a un esclavo a través del I2C 0.
 *
 * \param[in]   dato    byte a transmitir.
 *
 * \return      Valor del registro I2C0STAT.
 *
 */
static int glcd_i2c0_transmitir_byte(unsigned char dato) 
{
    LPC_I2C0->CONCLR = 1 << 3;    
    
    LPC_I2C0->DAT = dato;

    while ((LPC_I2C0->CONSET & (1 << 3)) == 0);

    return LPC_I2C0->STAT;
}

static int glcd_i2c0_transmitir_buffer(uint32_t addr, uint8_t* buf, uint32_t len) 
{
    int i;
	
	glcd_i2c0_start();
    glcd_i2c0_transmitir_byte(addr << 1);
	for (i = 0; i < len; i++)
	{
        glcd_i2c0_transmitir_byte(*buf);
		buf++;
	}
    glcd_i2c0_stop();
		
	return 0;
}

/***************************************************************************//**
 * \brief Find closest clock divider to get the desired clock rate.
 */
//static uint32_t glcd_clockdivide(uint32_t lcd_clock) 
//{	
//	uint32_t divide, result;
//	
//	divide = 1;
//	
//	while ( ( (SystemCoreClock / divide) > lcd_clock ) && (divide <= 0x3F) ) 
//	{
//		divide++;
//	}
//	if (divide <= 1) 
//	{
//		result = (1 << BCD_BIT);   /* Skip divider logic if clock divider is 1 */
//	} 
//	else 
//	{
//		result = 0 | (((divide >> 0) & 0x1F) | (((divide >> 5) & 0x1F) << 27));
//	}
//	return result;
//}


/***************************************************************************//**
 * \brief 
 */
void glcd_init(void)
{
    int i;
    uint8_t buffer[5];
    
    glcd_i2c0_inicializar(300, 300);
    sdram_init();    
    
    LPC_IOCON->P0_4 |= 7; /* LCD_VD_0 @ P0[4] */
    LPC_IOCON->P0_5 |= 7; /* LCD_VD_1 @ P0[5] */
    LPC_IOCON->P0_6 |= 7; /* LCD_VD_8 @ P0[6] */
    LPC_IOCON->P0_7 |= 7; /* LCD_VD_9 @ P0[7] */
    LPC_IOCON->P0_8 |= 7; /* LCD_VD_16 @ P0[8] */
    LPC_IOCON->P0_9 |= 7; /* LCD_VD_17 @ P0[9] */
    LPC_IOCON->P0_10 |= 7; /* LCD_VD_5 @ P0[10] */

    LPC_IOCON->P1_20 |= 7; /* LCD_VD_10 @ P1[20] */
    LPC_IOCON->P1_21 |= 7; /* LCD_VD_11 @ P1[21] */
    LPC_IOCON->P1_22 |= 7; /* LCD_VD_12 @ P1[22] */
    LPC_IOCON->P1_23 |= 7; /* LCD_VD_13 @ P1[23] */
    LPC_IOCON->P1_24 |= 7; /* LCD_VD_14 @ P1[24] */
    LPC_IOCON->P1_25 |= 7; /* LCD_VD_15 @ P1[25] */
    LPC_IOCON->P1_26 |= 7; /* LCD_VD_20 @ P1[26] */
    LPC_IOCON->P1_27 |= 7; /* LCD_VD_21 @ P1[27] */
    LPC_IOCON->P1_28 |= 7; /* LCD_VD_22 @ P1[28] */
    LPC_IOCON->P1_29 |= 7; /* LCD_VD_23 @ P1[29] */

    LPC_IOCON->P2_0 |= 7; /* LCD_PWR @ P2[0] */
    LPC_IOCON->P2_1 |= 7; /* LCD_LE  @ P2[1] */
    LPC_IOCON->P2_2 |= 7; /* LCD_DCLK @ P2[2] */
    LPC_IOCON->P2_3 |= 7; /* LCD_FP @ P2[3] */
    LPC_IOCON->P2_4 |= 7; /* LCD_ENAB_M @ P2[4] */
    LPC_IOCON->P2_5 |= 7; /* LCD_LP @ P2[5] */
    LPC_IOCON->P2_6 |= 7; /* LCD_VD_4 @ P2[6] */  
    LPC_IOCON->P2_8 |= 7; /* LCD_VD_6 @ P2[8] */
    LPC_IOCON->P2_9 |= 7; /* LCD_VD_7 @ P2[9] */

    LPC_IOCON->P2_11 |= 7; /* LCD_CLKIN @ P2[11] */
    LPC_IOCON->P2_12 |= 5; /* LCD_VD_3 @ P2[12] Señal marcada como LCD_VD_18 en la
                            tarjeta base, pero llevará la señal LCD_VD_3 */
    LPC_IOCON->P2_13 |= 7; /* LCD_VD_19 @ P2.13 */    
    
    buffer[0] = GLCD_PCA9532_LS0 | GLCD_PCA9532_AUTO_INC;
    buffer[1] = 0;
    buffer[2] = 1;
    buffer[3] = 0;
    buffer[4] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 5);
    
    buffer[0] = GLCD_PCA9532_PWM0;
    buffer[1] = 255;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 2);
    
    buffer[0] = GLCD_PCA9532_PSC0;
    buffer[1] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 2);
    
    buffer[0] = GLCD_PCA9532_LS0 | GLCD_PCA9532_AUTO_INC;
    buffer[1] = 0;
    buffer[2] = 1;
    buffer[3] = 2;
    buffer[4] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 5);
    
    buffer[0] = GLCD_PCA9532_LS0 | GLCD_PCA9532_AUTO_INC;
    buffer[1] = 0;
    buffer[2] = 1;
    buffer[3] = 2;
    buffer[4] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 5);
    
    glcd_timer0_retardo_ms(100);
    
    buffer[0] = GLCD_PCA9532_LS0 | GLCD_PCA9532_AUTO_INC;
    buffer[1] = 1;
    buffer[2] = 1;
    buffer[3] = 2;
    buffer[4] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 5);
    
    glcd_timer0_retardo_ms(100);
    
    buffer[0] = GLCD_PCA9532_LS0 | GLCD_PCA9532_AUTO_INC;
    buffer[1] = 1;
    buffer[2] = 0;
    buffer[3] = 2;
    buffer[4] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 5);
    
    glcd_timer0_retardo_ms(10);
    
    LPC_SC->PCONP |= 0x00000001;
        
    LPC_LCD->CTRL &= ~1;
    
    LPC_LCD->TIMH = 0x27040174;
    LPC_LCD->TIMV = 0x0808050F;
    LPC_LCD->POL = 0x01DF180C;
    LPC_LCD->LE = 0x00000000;
    LPC_LCD->INTMSK = 0x00000000;
    LPC_LCD->CTRL = 0x12c;
    for (i = 0; i < 256; i++) LPC_LCD->PAL[i] = 0;
    LPC_SC->LCD_CFG = 0x0;
    
    LPC_LCD->CTRL |= 1;
    LPC_LCD->CTRL |= 1 << 11;
    LPC_LCD->UPBASE = GLCD_VRAM_BASE_ADDR & ~7UL;
	LPC_LCD->LPBASE = GLCD_VRAM_BASE_ADDR & ~7UL;
        
    memset((void*)GLCD_VRAM_BASE_ADDR, 0x4d, GLCD_X_SIZE*GLCD_Y_SIZE*2);    

    LPC_IOCON->P2_22 = 0;
	LPC_IOCON->P2_23 = 0;
	LPC_IOCON->P2_25 = 0;
	LPC_IOCON->P2_26 = 0;
	LPC_IOCON->P2_27 = 0;    
    
    buffer[0] = GLCD_PCA9532_PWM0;
    buffer[1] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 2);
    
    buffer[0] = GLCD_PCA9532_PSC0;
    buffer[1] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 2);
    
    buffer[0] = GLCD_PCA9532_LS0 | GLCD_PCA9532_AUTO_INC;
    buffer[1] = 1;
    buffer[2] = 0;
    buffer[3] = 2;
    buffer[4] = 0;
    glcd_i2c0_transmitir_buffer(GLCD_PCA9532_I2C_ADDR, buffer, 5);
}

/***************************************************************************//**
 * \brief 
 */
void glcd_cursor_config(int config)
{
	LPC_LCD->CRSR_CFG = config;
}

/***************************************************************************//**
 * \brief 
 */
void glcd_cursor_enable(int cursor)
{
	LPC_LCD->CRSR_CTRL |= (cursor<<4);
	LPC_LCD->CRSR_CTRL |= 1;
}

/***************************************************************************//**
 * \brief 
 */
void glcd_cursor_disable(void)
{
	LPC_LCD->CRSR_CTRL &= (1<<0);
}

/***************************************************************************//**
 * \brief 
 */
void glcd_move_cursor(int x, int y)
{
	LPC_LCD->CRSR_CLIP = 0;
	LPC_LCD->CRSR_XY = 0;

	if(x >= 0)
	{  
		LPC_LCD->CRSR_XY |= (x & 0x3FF);  /* no clipping */
	}
	else
	{
		LPC_LCD->CRSR_CLIP |= -x;		  /* clip x */
	}

	if(y >= 0)
	{
		LPC_LCD->CRSR_XY |= (y << 16);	  /* no clipping */
	}
	else
	{
		LPC_LCD->CRSR_CLIP |= (-y << 8);  /* clip y */
	}
}

/***************************************************************************//**
 * \brief 
 */
void glcd_copy_cursor (const uint32_t *pCursor, int cursor, int size)
{
   	uint32_t i ;
   	uint32_t * dest_address = (uint32_t *)GLCD_CURSOR_BASE_ADDR;
   
   	dest_address += cursor * 64;

   	for(i = 0; i < size ; i++) 
	{
		*dest_address = *pCursor;
		dest_address++;
		pCursor++;
	}
}

/***************************************************************************//**
 * \brief 
 */
void glcd_setpixel(uint16_t x, uint16_t y, uint16_t color)
{
    volatile uint16_t *ptr = (uint16_t*)GLCD_VRAM_BASE_ADDR;
  
    if (x >= GLCD_X_SIZE || y >= GLCD_Y_SIZE) return;

    ptr[y*GLCD_X_SIZE + x] = color;	
}

/***************************************************************************//**
 * \brief 
 */
void glcd_clear(uint16_t color)
{ 
    int i;
    volatile uint16_t *ptr = (uint16_t*)GLCD_VRAM_BASE_ADDR;

    for (i = 0; i < GLCD_X_SIZE*GLCD_Y_SIZE; i++)
    {
        *ptr++ = color;
    }
}

/***************************************************************************//**
 * \brief 
 */
void glcd_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bkcolor,
               uint32_t font)
{
	uint16_t i, j;
    const uint8_t* ptr;
    uint8_t b;

    extern const unsigned char font8x16[];
    extern const unsigned char font16x32[];

    if (font == FONT8X16)
    {
        ptr = font8x16 + (c - 32)*16;
        for (i = 0; i < 16; i++)
        {
            b = *ptr++;
            for (j = 0; j < 8; j++)
            {
                if (((b >> (7 - j)) & 0x01) == 0x01)
                {
                    glcd_setpixel(x + j, y + i, color );
                }
                else
                {
                    glcd_setpixel(x + j, y + i, bkcolor );
                }
            }
        }
    }
    else if (font == FONT16X32)
    {
        ptr = font16x32 + (c - 32)*32*2;
        for (i = 0; i < 32; i++)
        {
            b = *ptr++;
            for (j = 0; j < 8; j++)
            {
                if (((b >> (7 - j)) & 0x01) == 0x01)
                {
                    glcd_setpixel(x + j, y + i, color );
                }
                else
                {
                    glcd_setpixel(x + j, y + i, bkcolor );
                }
            }
            b = *ptr++;
            for (j = 0; j < 8; j++)
            {
                if (((b >> (7 - j)) & 0x01) == 0x01)
                {
                    glcd_setpixel(x + j + 8, y + i, color );
                }
                else
                {
                    glcd_setpixel(x + j + 8, y + i, bkcolor );
                }
            }
        }
    }
}

/***************************************************************************//**
 * \brief 
 */
void glcd_text(uint16_t x, uint16_t y, char *str, uint16_t color,
               uint16_t bkcolor, uint32_t font)
{
    while (*str != 0)
    {
        glcd_char(x, y, *str, color, bkcolor, font);    
        if (font == FONT8X16)
        {
            if (x < GLCD_X_SIZE - 8)
            {
                x += 8;
            } 
            else if (y < GLCD_Y_SIZE - 16)
            {
                x = 0;
                y += 16;
            }   
            else
            {
                x = 0;
                y = 0;
            }
        }
        else if (font == FONT16X32)
        {
            if (x < GLCD_X_SIZE - 16)
            {
                x += 16;
            } 
            else if (y < GLCD_Y_SIZE - 32)
            {
                x = 0;
                y += 32;
            }   
            else
            {
                x = 0;
                y = 0;
            }
        }
        str++;
    }
}

/***************************************************************************//**
 * \brief 
 */
void glcd_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
               uint16_t color)
{
    short dx, dy;
    short temp;

    if( x0 > x1 )
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if( y0 > y1 )
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;
	dy = y1-y0;  

    if( dx == 0 )
    {
        do
        { 
            glcd_setpixel(x0, y0, color);
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )
    {
        do
        {
            glcd_setpixel(x0, y0, color);
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }

    if( dx > dy ) 
    {
	    temp = 2 * dy - dx;   
        while( x0 != x1 )
        {
	        glcd_setpixel(x0, y0, color);
	        x0++;
	        if( temp > 0 )           
	        {
	            y0++;                
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;   
			}       
        }
        glcd_setpixel(x0, y0, color);
    }  
    else
    {
	    temp = 2 * dx - dy;              
        while( y0 != y1 )
        {
	 	    glcd_setpixel(x0, y0, color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp += 2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        glcd_setpixel(x0, y0, color);
	}
} 


/***************************************************************************//**
 * \brief 
 */
void glcd_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                    uint16_t color)
{
    uint16_t temp;
    int i, j;
    uint16_t* ptr = (uint16_t*)GLCD_VRAM_BASE_ADDR;
    uint16_t* ptr2;

    if (x0 > x1)
    {
        temp = x0;
        x0 = x1;
        x1 = temp;   
    }

    if (y0 > y1)
    {
        temp = y0;
        y0 = y1;
        y1 = temp;   
    }

    ptr +=  GLCD_X_SIZE*y0 + x0;
        
    for (j = y0; j <= y1; j++)
    {
        ptr2 = ptr;
        ptr += GLCD_X_SIZE;
        for (i = x0; i <= x1; i++)
        {
            *ptr2++ = color;
        }    
    }
}
