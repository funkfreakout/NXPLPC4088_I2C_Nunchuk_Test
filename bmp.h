/***************************************************************************//**
 * \file    bmp.h
 *
 * \brief   Estructuras de manejo de ficheros .bmp
 *
 */

#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct { 
    uint16_t   bfType; 
    uint32_t  bfSize; 
    uint16_t   bfReserved1; 
    uint16_t   bfReserved2; 
    uint32_t  bfOffBits; 
} __attribute__((__packed__))
BITMAPFILEHEADER;

typedef struct {
    uint32_t  biSize; 
    int32_t   biWidth; 
    int32_t   biHeight; 
    uint16_t   biPlanes; 
    uint16_t   biBitCount; 
    uint32_t  biCompression; 
    uint32_t  biSizeImage; 
    int32_t   biXPelsPerMeter; 
    int32_t   biYPelsPerMeter; 
    uint32_t  biClrUsed; 
    uint32_t  biClrImportant; 
} __attribute__((__packed__))
BITMAPINFOHEADER; 

#endif //BMP_H
