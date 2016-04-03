#include "general.h"

volatile WORD Timer;		/* 100Hz increment timer */
uint8_t ee_file_val EEMEM;

DWORD get_fattime () {
	RTC rtc;

	rtc.year = 2015;
	rtc.month = 3;
	rtc.mday = 20;
	rtc.hour = 10;
	rtc.min = 05;
	rtc.sec = 12;


	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(rtc.year - 1980) << 25)
			| ((DWORD)rtc.month << 21)
			| ((DWORD)rtc.mday << 16)
			| ((DWORD)rtc.hour << 11)
			| ((DWORD)rtc.min << 5)
			| ((DWORD)rtc.sec >> 1);
}

char * conv_meas_data(double data_to_put, data_meas_t what_data) {

	const char* units[4] = {" m ", " *C ", " \%rH ", " hPa "};
	const uint8_t units_strlen[4] = {3, 4, 5, 5};

	char buffer[15];
	char *strPtr;
	uint8_t comma_place, sign, str_len;
	char *exp_place_ptr;
	sign = ((uint8_t)what_data > 1) ? 0 : DTOSTR_ALWAYS_SIGN;
	dtostre(data_to_put, buffer, 7, sign);
	exp_place_ptr = x_strchr(buffer, 'e');
	comma_place = (uint8_t)buffer[exp_place_ptr - buffer + 3] - 48;
	if(comma_place != 0) {
		if(buffer[exp_place_ptr - buffer + 1] == '-') {
			switch(comma_place)
			{
				case 0: break;
				case 1:
				case 2:
					x_memmove(buffer + 1 + sign, buffer + 2 + sign, 7);
					x_memmove(buffer + 1 + comma_place + sign, buffer + sign, 7);
					buffer[0 + sign] = '0';
					buffer[1 + sign] = '.';
					if(comma_place == 2) buffer[2 + sign] = '0';
					break;
				default: {
					x_strcpy(buffer, "0.00");
					break;
				}
			}
			comma_place = 0;
		}
		else {
			x_memmove(buffer + 1 + sign, buffer + 2 + sign, comma_place);
			buffer[comma_place + 1 + sign] = '.';
		}
	}
	str_len = sign + comma_place + 5 + units_strlen[(uint8_t)what_data];
	strPtr = (char*) malloc (str_len);
	x_memcpy(strPtr, buffer, sign + comma_place + 4);
	strPtr[sign + comma_place + 4] = '\0';
	x_strcat(strPtr, units[(uint8_t)what_data]);
	strPtr[str_len - 1] = '\0';
	return strPtr;
}

uint8_t EEPROM_get_file_num() {
	uint8_t temp;
	while(EECR & (1<<EEPE));
	EEAR = ee_file_val;
	EECR |= (1<<EERE);
	temp = EEDR;
	while(EECR & (1<<EEPE));
	EEAR = ee_file_val;
	EEDR = temp + 1;
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
	return temp;
}

/* Timer - 100 Hz */
void Timer0_init (void) {
	TCCR0A |= (1<<WGM01);				// CTC mode
	TCCR0B |= (1<<CS02) | (1<<CS00);	// Prescaler F_CPU/1024
	OCR0A = 78;							// 8000000/1024/78 = 100,16 Hz ~ 100 Hz
	TIMSK0 |= (1<<OCIE0A);				// Allow interrupt in CTC mode
}

/* Timer 1,1 Hz */
void Timer1_init (void) {
	/* 	Prescaler F_CPU/1024; CTC mode	*/
	TCCR1B |= (1<<WGM12) | (1<<CS12) | (1<<CS10);

	/*  1BBE(16) = 7102(10)
		8000000/1024/7102 = 1,1 Hz */
	OCR1AH = 0x1B;
	OCR1AL = 0xBE;
}

void SPI_init() {

	//	Set adequate pins (used in SPI communication) as output
	DDRB |= (1<<CS) | (1<<MOSI) | (1<<SCK);

	//	Select SD card as active Slave device
	PORTB |= (1<<CS);

	//	High level on MISO pin
	PORTB |= (1<<MISO);

	//	SPI internal module activation and setting AVR as Master
	SPCR |= (1<<SPE) | (1<<MSTR);
}


ISR(TIMER0_COMPA_vect) {
	Timer++;			/* Performance counter for this module */
	disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
}

ISR(TIMER1_COMPA_vect) {
	
	/*	Firstly, clear flags used in USART RX interrupt
		to provide data consistency					*/
	NMEA_ind = 0;
	data_ind = 0;
	GGA_located = false;		
	commas_to_ignore = 0;

	add_task(t_add_file_endline);

	add_task(t_bmp_take_meas);
	add_task(t_hts_take_meas);
	add_task(t_mpl_take_alt_meas);
}

void t_add_file_endline() {
	SD_put_data_prog(PSTR("\n\r"));
}
