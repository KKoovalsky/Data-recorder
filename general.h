//	Common file
//	general.c implements handlers of Timer1 and Timer0 overflow interrupt

#ifndef _GENERAL_H
#define _GENERAL_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <avr/eeprom.h>

#include "tcb.h"

#include "x_str.h"

#include "SDCARD/integer.h"
#include "SDCARD/diskio.h"
#include "SDCARD/ff.h"
#include "SDCARD/ffconf.h"

#include "gps.h"
#include "i2c.h"

#include "SENSORS/hts.h"
#include "SENSORS/mpl.h"
#include "SENSORS/bmp.h"

#define ACK 1
#define NACK 0

#define SCK PB7
#define MISO PB6
#define MOSI PB5
#define CS PB4

#define LED_TOG (PORTD ^= (1<<PD6))

typedef enum data_meas {
	altitude, temperature, humidity, pressure
} data_meas_t;

typedef struct {
	WORD	year;	/* 2000..2099 */
	BYTE	month;	/* 1..12 */
	BYTE	mday;	/* 1.. 31 */
	BYTE	wday;	/* 1..7 */
	BYTE	hour;	/* 0..23 */
	BYTE	min;	/* 0..59 */
	BYTE	sec;	/* 0..59 */
} RTC;

uint32_t f_bytes_wr;

extern char filename[8];
extern FIL fil_obj;

inline void SD_put_data(char * str) {
	f_open(&fil_obj, filename, FA_WRITE);
	f_lseek(&fil_obj, f_bytes_wr);
	f_puts(str, &fil_obj);
	f_bytes_wr += x_strlen(str);
	f_close(&fil_obj);
}

inline void SD_put_data_prog(const char * str) {
	uint8_t len = x_strlen_prog(str);
	char * temp_str = (char *) malloc (sizeof(char) * (len + 1));
	x_sprinft_prog(temp_str, str);

	f_open(&fil_obj, filename, FA_WRITE);
	f_lseek(&fil_obj, f_bytes_wr);
	f_puts(temp_str, &fil_obj);
	f_bytes_wr += len;
	f_close(&fil_obj);

	free(temp_str);
}

DWORD get_fattime();

char * conv_meas_data(double data_to_put, data_meas_t what_data);

uint8_t EEPROM_get_file_num();

void Timer0_init (void);
void Timer1_init (void);
void SPI_init();

void t_add_file_endline();

#endif
