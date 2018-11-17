#ifndef GLCD_H 
#define GLCD_H

#include <stdint.h>
#include "sdram.h"

#define GLCD_VRAM_BASE_ADDR 	((uint32_t)SDRAM_BASE + 0x0010000)
#define GLCD_CURSOR_BASE_ADDR 	((uint32_t)0x20088800)

#define GLCD_X_SIZE        480	         /* LCD Horizontal length */
#define GLCD_Y_SIZE        272		     /* LCD Vertical length */
#define PIXELS_PER_PILE    GLCD_X_SIZE   /* Pixels per line */

#define H_SYNC_PULSE_WIDTH 2             /* HSYNC pulse width */
#define H_SYNC_FRONT_PORCH  5            /* Horizontal front porch */
#define H_SYNC_BACK_PORCH                40            /* Horizontal back porch */

#define LINES_PER_PANEL    GLCD_Y_SIZE   /* Lines per panel */
#define V_SYNC_PULSE_WIDTH 2             /* VSYNC pulse width */
#define V_SYNC_FRONT_PORCH 8             /* Vertical front porch */
#define V_SYNC_BACK_PORCH  8             /* Vertical back porch */

#define AC_BIAS            1             /* AC bias frequency (not used) */
#define INVERT_V_SYNS	   1             /* ? Invert VSYNC */
#define INVERT_H_SYNS      1             /* ? Invert HSYNC */
#define INVERT_PANEL_CLOCK 0             /* ? Invert panel clock */
#define CLOCKS_PER_LINE    PIXELS_PER_LINE /* Clock per line (identical to pixels per line) */
#define BITS_PER_PIXEL     6             /* Bits per pixel b110 = 16 bpp 5:6:5 mode */
#define BGR                1             /* Swap red and blue */
#define LCDTFT             1             /* LCD TFT panel */
#define OPTIMAL_CLOCK     (9*1000000)      /* Optimal clock rate (Hz) between 1-8.22 MHz */

#define BYPASS_CLOCK_DIVIDER_BIT    26        /* Bypass pixel clock divider */

/* ---- Colores ----------------------------------------------------------------
 */

#define RGB565BITS(r, g, b)    ((((r) & 0x1f) << 11) | (((g) & 0x3f) << 5) | ((b) & 0x1f))

#define BLACK           RGB565BITS(0, 0, 0)
#define DARKBLUE        RGB565BITS(0, 0, 15)
#define DARKGREEN       RGB565BITS(0, 31, 0)
#define DARKCYAN        RGB565BITS(0, 31, 13)
#define MAROON          RGB565BITS(15, 0, 0)
#define PURPLE          RGB565BITS(15, 0, 15)
#define OLIVE           RGB565BITS(15, 31, 0)
#define ORANGE          RGB565BITS(31, 47, 0)
#define LIGHTGREY       RGB565BITS(23, 47, 23)
#define DARKGREY        RGB565BITS(15, 31, 15)
#define BLUE            RGB565BITS(0, 0, 31)
#define GREEN           RGB565BITS(0, 63, 0)
#define CYAN            RGB565BITS(0, 63, 31)
#define RED             RGB565BITS(31, 0, 0)
#define MAGENTA         RGB565BITS(31, 0, 31)
#define YELLOW          RGB565BITS(31, 63, 0)
#define WHITE           RGB565BITS(31, 63, 31)

#define GLCD_PCA9532_I2C_ADDR    0x64

/* ---- Regitros del PCA9532 ---------------------------------------------------
 */
#define GLCD_PCA9532_INPUT0   0x00
#define GLCD_PCA9532_INPUT1   0x01
#define GLCD_PCA9532_PSC0     0x02
#define GLCD_PCA9532_PWM0     0x03
#define GLCD_PCA9532_PSC1     0x04
#define GLCD_PCA9532_PWM1     0x05
#define GLCD_PCA9532_LS0      0x06
#define GLCD_PCA9532_LS1      0x07
#define GLCD_PCA9532_LS2      0x08
#define GLCD_PCA9532_LS3      0x09
#define GLCD_PCA9532_AUTO_INC 0x10

#define GLCD_LS_MODE_ON     0x01
#define GLCD_LS_MODE_BLINK0 0x02
#define GLCD_LS_MODE_BLINK1 0x03

#define GLCD_CTRL_3V3     0x0001
#define GLCD_CTRL_5V      0x0002
#define GLCD_CTRL_DISP_EN 0x0010
#define GLCD_CTRL_BL_EN   0x0080
#define GLCD_CTRL_BL_C    0x0100

/* ---- Fonts ------------------------------------------------------------------
 */

#define FONT8X16        0
#define FONT16X32       1

/* ---- Prototipos de funciones ------------------------------------------------
 */

void glcd_init (void);
void glcd_cursor_config(int config);
void glcd_cursor_enable(int cursor);
void glcd_cursor_disable(void);
void glcd_move_cursor(int x, int y);
void glcd_copy_cursor(const uint32_t *pCursor, int cursor, int size);

void glcd_clear(uint16_t color);
void glcd_setpixel(uint16_t Xpos, uint16_t Ypos, uint16_t color);
void glcd_line( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );
void glcd_char(uint16_t x, uint16_t y, char c, uint16_t color,
               uint16_t bkcolor, uint32_t font);
void glcd_text(uint16_t Xpos, uint16_t Ypos, char *str,uint16_t Color,
               uint16_t bkColor, uint32_t font);
void glcd_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                    uint16_t color);

#endif  /* GLCD_H */
