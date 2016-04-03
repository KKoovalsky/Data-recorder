/* Variables and functions declaration for GNSS module. */

#ifndef _GPS_H
#define _GPS_H

#define BAUD_RATE_9600_H 0
#define BAUD_RATE_9600_L 51

#define USART_ENTER 0x0D
#define USART_NEW_LINE 0x0A

#define COMMAS_TO_IND_LOC 2
#define COMMAS_TO_IND_ALT 5

#define TIME_LETTERS_NUM_RAW 9
#define LATITUDE_LETTERS_NUM_RAW 10
#define LONGTITUDE_LETTERS_NUM_RAW 11
#define ALTITUDE_LETTERS_NUM 10
#define ALTITUDE_UNITS_LETTERS_NUM 3

#define TIME_LETTERS_NUM 9
#define LATITUDE_LETTERS_NUM 14
#define LONGTITUDE_LETTERS_NUM 15

extern volatile uint8_t commas_to_ignore;
extern volatile uint8_t NMEA_ind;
extern volatile uint8_t data_ind;
extern volatile bool GGA_located;

//	Setting talker id from GP to GN to allow positioning based on multiple systems.
inline void set_GN_TID() {
	//	Allow to transmit data by USART
	UCSR0B |= (1<<UDRIE0);
}

void USART_init();


void t_prep_gnss_data();
void t_save_gnss_data();

#endif
