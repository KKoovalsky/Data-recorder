/* Functions and variables definitions for dealing with GNSS module		*/

#include "general.h"
#include "gps.h"

volatile char time_raw[TIME_LETTERS_NUM_RAW + 1];
volatile char lat_raw[LATITUDE_LETTERS_NUM_RAW + 1];
volatile char longt_raw[LONGTITUDE_LETTERS_NUM_RAW + 1];
volatile char alt[ALTITUDE_LETTERS_NUM + 1];
volatile char alt_units[ALTITUDE_UNITS_LETTERS_NUM + 1];

volatile char time[TIME_LETTERS_NUM + 1];
volatile char lat[LATITUDE_LETTERS_NUM + 1];
volatile char longt[LONGTITUDE_LETTERS_NUM + 1];

volatile char *raw_data[5] = {time_raw, lat_raw, longt_raw, alt, alt_units};
volatile char *rdy_data[5] = {time, lat, longt, alt, alt_units};

const char NMEA_GGA_option[] PROGMEM = "GNGGA";
const uint8_t NMEA_GGA_strlen PROGMEM = 5;

/*	Nr of commas which should be skipped after reading important data.
 	Each index corresponds to important data like ordered in GGA message. */
const uint8_t comm_dc_after_imp[] PROGMEM = {0, 1, 4, 0, 0};

volatile uint8_t commas_to_ignore;
volatile uint8_t NMEA_ind;
volatile uint8_t data_ind;
volatile bool GGA_located = false;

const char def_data_1[] PROGMEM = "000000.00";
const char def_data_2[] PROGMEM = "0000.00000";
const char def_data_3[] PROGMEM = "00000.00000";
const char def_data_4[] PROGMEM = "00.0";
const char def_data_5[] PROGMEM = "M";
const char * def_data[] = {def_data_1, def_data_2, def_data_3, def_data_4, def_data_5};

//	21 bytes of command to change talker ID from GP (default) to GN
const char GN_TID_comm[] PROGMEM = { 0xB5, 0x62, 0x06, 0x17, 0x0C, 0x00, 0x00, 0x23,
		0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x51, 0xFD, '\0'};


void USART_init() {
	UBRR0H = BAUD_RATE_9600_H;
	UBRR0L = BAUD_RATE_9600_L;

	//	Activation of USART's RX and TX module. Allowing on byte received interrupt.
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);

	//	1 stop bit, 8 bit in one baud, no parity control
	UCSR0C = (3<<UCSZ00);
}

ISR(USART0_UDRE_vect) {
	static const char * str = GN_TID_comm;
	char data;
	if((data = pgm_read_byte(str++))) UDR0 = data;
	else UCSR0B &= ~(1<<UDRIE0);
}

ISR(USART0_RX_vect) {

	char data;
	data = UDR0;

	PORTD ^= (1<<PD6);

	if(data_ind == 5) {
		TCNT1 = 0;

		data_ind = 0;
		GGA_located = false;

		PORTD &= ~(1<<PD6);

		add_task(t_add_file_endline);

		add_task(t_prep_gnss_data);

		add_task(t_bmp_take_meas);
		add_task(t_hts_take_meas);
		add_task(t_mpl_take_alt_meas);

		return;
	}

	if(commas_to_ignore) {
		if(data == ',') commas_to_ignore --;
		return;
	}

	if(GGA_located) {
		if((!NMEA_ind) && data == ',') {
			x_sprinft_prog((char *)raw_data[data_ind], def_data[data_ind]);
			commas_to_ignore = pgm_read_byte(comm_dc_after_imp[data_ind]);
			data_ind ++;
			NMEA_ind = 0;
			return;
		}
		if(data == ',') {
			raw_data[data_ind][NMEA_ind] = '\0';
			commas_to_ignore = pgm_read_byte(comm_dc_after_imp[data_ind]);
			data_ind ++;
			NMEA_ind = 0;
			return;
		}
		raw_data[data_ind][NMEA_ind] = data;
		NMEA_ind ++;
		return;
	}

	if(data == pgm_read_byte(NMEA_GGA_option[NMEA_ind])) {
		NMEA_ind++;
		if(NMEA_ind == pgm_read_byte(NMEA_GGA_strlen)) {
			commas_to_ignore = 1;
			GGA_located = true;
			NMEA_ind = 0;
		}
	}
	else NMEA_ind = 0;
}



void t_prep_gnss_data()
{
	uint32_t seconds_raw;
	uint8_t temp_strlen;
	double seconds;
	char buffer[11];
	char *data_raw = (char *)lat_raw;
	char *data = (char *)lat;

	x_memcpy((char *)time, (char *)time_raw, 2);
	time[2] = ':';
	x_memcpy((char *)time + 3, (char *)time_raw + 2, 2);
	time[5] = ':';
	x_memcpy((char *)time + 6, (char *)time_raw + 4, 2);
	time[8] = ' ';
	time[9] = '\0';

	for(uint8_t j = 0; j < 2; j ++)
	{
		seconds_raw = atoi(data_raw + 5 + j);
		seconds = (double)seconds_raw / 100000.0 * 60.0;
		if(seconds < 10.0) {
			dtostre(seconds, buffer + 1, 3, 0);
			buffer[0] = '0';
		} else {
			dtostre(seconds, buffer, 4, 0);
			buffer[1] = buffer[2];
			buffer[2] = '.';
		}

		x_memcpy(data, data_raw, 2 + j);
		data[2 + j] = (j ? 'E' : 'N');
		x_memcpy(data + 3 + j, data_raw + 2 + j, 2);
		data[5 + j] = 39;							//39 - ASCII - ' - geo minutes
		x_memcpy(data + 6 + j, buffer, 6);
		data[12 + j] = 34;				//34 - ASCII - " - geo seconds
		data[13 + j] = ' ';
		data[14 + j] = '\0';

		data_raw = (char *)longt_raw;
		data = (char *)longt;
	}

	temp_strlen = x_strlen((char *)alt);
	alt[temp_strlen] = ' ';
	alt[temp_strlen + 1] = '\0';

	temp_strlen = x_strlen((char *)alt_units);
	alt_units[temp_strlen] = ' ';
	alt_units[temp_strlen + 1] = '\0';

	add_task(t_save_gnss_data);
}

void t_save_gnss_data() {
	for(uint8_t i = 0; i < 5; i ++)
		SD_put_data((char*)rdy_data[i]);
}


