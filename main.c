#include <LPC407x_8x_177x_8x.h>
#include "i2c0_lpc4088.h"
#include "nunchuk.h"
#include "glcd.h"
#include "stdio.h"
#include "stdlib.h"
#include "timer0_lpc4088.h"
#include "timer1_lpc4088.h"
#include "rtc_lpc4088.h"
#include "bmp.h"
#include "ff.h"
#include "wav_file.h"
#include "sonido.h"

__align(4) FATFS fs;

#define DIRECCION 0xA0200000
short drum_muestras[36000] __attribute__((at(DIRECCION)));
#define DIRECCION2 (DIRECCION + 36000*2)
short maraca_muestras[69704] __attribute__((at(DIRECCION2)));
#define DIRECCION3 (DIRECCION2 + 69704*2)
short taiko_muestras[114976] __attribute__((at(DIRECCION3)));

BITMAPFILEHEADER bmpfh;
BITMAPINFOHEADER bmpih;
WAV_HEADER wavh;

struct sonido_t bateria;
struct sonido_t maraca;
struct sonido_t taiko;

unsigned char bytes_from_nunchuck[BYTES_NUNCHUK];
unsigned char joystick_x;
unsigned char joystick_y;
int acelerometro_x = 0;
int acelerometro_y = 0;
int acelerometro_z = 0;
unsigned char c_button;
unsigned char z_button;

/***************************************************************************//**
 * \brief       Cambia de opcion segun el estado del boton C.
 *
 * \param[in]  	option: opcion de instrumento actual.
 * \param[in]   cb: estado del boton C.
 *
 */
unsigned int next_option(unsigned int option, unsigned char cb){
	if(cb == 0) option = (option+1)%3;

	return option;
}

/***************************************************************************//**
 * \brief       Dibuja imagen pixel a pixel segun la opcion escogida.
 *
 * \param[in]   file: puntero al fichero de lectura.
 * \param[in]   err: valor de retorno de las funciones de la libreria ff.h. Error flag.
 * \param[in]   leidos: cursor de lectura/escritura del fichero.
 * \param[in]   option: opcion de instrumento actual.
 *
 */
unsigned int draw_image(FIL* file, volatile int err,unsigned int leidos, unsigned int option){
	unsigned int x, y;
	unsigned short bmp_c;
	switch(option){
		case 0: err = f_open(file, "bateria.bmp", FA_READ);
						
						err = f_read(file, &bmpfh, sizeof(BITMAPFILEHEADER), &leidos);
						
						err = f_read(file, &bmpih, sizeof(BITMAPINFOHEADER), &leidos);
						
						f_lseek(file,bmpfh.bfOffBits);
						
						for(y=0;y<bmpih.biHeight;y++)
							for(x=0;x<bmpih.biWidth;x++){
								err = f_read(file, &bmp_c,(sizeof(unsigned short)), &leidos);
								glcd_setpixel(x,y,bmp_c);
							}

						f_close(file);
							
						break;
			case 1: err = f_open(file, "maraca.bmp", FA_READ);
					
					err = f_read(file, &bmpfh, sizeof(BITMAPFILEHEADER), &leidos);
					
					err = f_read(file, &bmpih, sizeof(BITMAPINFOHEADER), &leidos);
					
					f_lseek(file,bmpfh.bfOffBits);
					
					for(y=0;y<bmpih.biHeight;y++)
						for(x=0;x<bmpih.biWidth;x++){
							err = f_read(file, &bmp_c,(sizeof(unsigned short)), &leidos);
							glcd_setpixel(x,y,bmp_c);
						}

					f_close(file);
						
					break;
						
			case 2: err = f_open(file, "taiko.bmp", FA_READ);
					
					err = f_read(file, &bmpfh, sizeof(BITMAPFILEHEADER), &leidos);
					
					err = f_read(file, &bmpih, sizeof(BITMAPINFOHEADER), &leidos);
					
					f_lseek(file,bmpfh.bfOffBits);
					
					for(y=0;y<bmpih.biHeight;y++)
						for(x=0;x<bmpih.biWidth;x++){
							err = f_read(file, &bmp_c,(sizeof(unsigned short)), &leidos);
							glcd_setpixel(x,y,bmp_c);
						}

					f_close(file);
						
					break;
						
			default: break;
	}
}


/***************************************************************************//**
 * \brief       Reproduce sonido cargado en SDRAM dependiendo de la opcion actual y
 *				estado del boton Z.
 * \param[in]   option: opcion de instrumento actual.
 * \param[in]   zb: estado del boton Z.
 *
 */
void sound_on(unsigned int option,unsigned char zb){
	switch(option){
		case 0:		if(zb == 0){
							sonido_reproducir(&bateria);
								while (sonido_en_reproduccion()) {}
							}
							break;
		case 1:		if(zb == 0){
							sonido_reproducir(&maraca);
								while (sonido_en_reproduccion()) {}
							}
							break;
		case 2:		if(zb == 0){
							sonido_reproducir(&taiko);
								while (sonido_en_reproduccion()) {}
							}
							break;
		default: break;
	}
}

int main(int argc, char* argv[]){
	int i;
	FIL file;
	volatile int err;
	unsigned int leidos;
	unsigned char *ptr;
	unsigned int option = 0;

	glcd_init();
	glcd_clear(BLACK);
	rtc_inicializar();
	rtc_ajustar_fecha(13, 11, 2015);
  rtc_ajustar_hora(11, 15, 0);
	
	f_mount(0, &fs);
	
	//BATERIA
	err = f_open(&file, "drum.wav", FA_READ);
	
	err = f_read(&file, &wavh, sizeof(WAV_HEADER), &leidos);
	
	ptr = (unsigned char*)drum_muestras;
	for (i = 0; i < (wavh.Subchunk2size/512); i++){
		err = f_read(&file, ptr, 512,&leidos);
		ptr += 512;
	}
	
	bateria.tamano = wavh.Subchunk2size/2;
	bateria.muestras = drum_muestras;
	
	f_close(&file);
	
	//MARACA
	err = f_open(&file, "maraca.wav", FA_READ);
	
	err = f_read(&file, &wavh, sizeof(WAV_HEADER), &leidos);
	
	ptr = (unsigned char*)maraca_muestras;
	for (i = 0; i < (wavh.Subchunk2size/512); i++){
		err = f_read(&file, ptr, 512,&leidos);
		ptr += 512;
	}
	
	maraca.tamano = wavh.Subchunk2size/2;
	maraca.muestras = maraca_muestras;
	
	f_close(&file);
	
	
	//TAIKO
	err = f_open(&file, "taiko.wav", FA_READ);
	
	err = f_read(&file, &wavh, sizeof(WAV_HEADER), &leidos);
	
	ptr = (unsigned char*)taiko_muestras;
	for (i = 0; i < (wavh.Subchunk2size/512); i++){
		err = f_read(&file, ptr, 512,&leidos);
		ptr += 512;
	}
	
	taiko.tamano = wavh.Subchunk2size/2;
	taiko.muestras = taiko_muestras;
	
	f_close(&file);
	
	sonido_inicializar();
	nunchuk_init();
	
	while(1){

		nunchuk_send_zero();
		timer1_retardo_ms(1);
		receive_nunchuk(bytes_from_nunchuck);
		
		for(i=0; i<BYTES_NUNCHUK; i++)
			bytes_from_nunchuck[i] = nunchuk_decode_byte(bytes_from_nunchuck[i]);
		
		joystick_x = bytes_from_nunchuck[0];
		joystick_y = bytes_from_nunchuck[1];
		acelerometro_x = (bytes_from_nunchuck[2] << 2) | ((bytes_from_nunchuck[5] & 0x0C) >> 2);
		acelerometro_y = (bytes_from_nunchuck[3] << 2) | ((bytes_from_nunchuck[5] & 0x30) >> 4);	
		acelerometro_z = (bytes_from_nunchuck[4] << 2) | ((bytes_from_nunchuck[5] & 0xC0) >> 6);
		c_button = ((bytes_from_nunchuck[5] & 0x02) >> 1);
		z_button = bytes_from_nunchuck[5] & 0x01;

		option = next_option(option,c_button);
		draw_image(&file, err, leidos, option);
		sound_on(option,z_button);
		
	}
}
