/* main.c */

#include "general.h"

char filename[8]; //data + <max. 3 cyfry> + <znak koñca linii> = 8 [cyfr]
FIL fil_obj;

int main(void)
{
	//	Change of clock tick frequency from 1 MHz to 8 MHz by software
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		CLKPR = (1<<CLKPCE);
		CLKPR = (0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
	}

	//	Signalization diode
	DDRD |= (1<<PD6);

	//	FAT32 service object
	static FATFS FATFS_Obj;

	//	New filename (to prevent from overwriting)
	x_strcpy(filename, "data");
	itoa(EEPROM_get_file_num(), filename + 4, 10);

	//	Microcontroller modules initialization
	Timer0_init();
	Timer1_init();
	Timer2_init();
	SPI_init();
	TWI_init();

	//	Initialization of slave devices connected to I2C bus
	mpl_init();
	hts_init();
	bmp_init();

	//	LED ON
	PORTD |= (1<<PD6);

	//	SD card initialization and creating new file
	disk_status(STA_NOINIT);
	f_mount(0, &FATFS_Obj);
	f_open(&fil_obj, filename, FA_CREATE_ALWAYS | FA_WRITE );

	//	Deputing measurement
	t_mpl_dep_alt_meas();
	t_hts_dep_meas();
	t_bmp_dep_meas();

	//	Initialize USART module (TX, RX) and UART handling.
	USART_init();
	UART_init();

	//	Change Talker ID of GNSS module from GP to GN (multiple system servicing)
	set_GN_TID();

	//	Run the watch timer to depute measurement if the GNSS data aren't earned
	TIMSK1 |= (1<<OCIE1A);

	//	Wait until sensors end a conversion
	_delay_ms(258);

	// TCB initialization
	init();

	//	Asynchrounous task handling initialization
	asynch_app_timer_init();

	//	Setting sleep mode to minimize power consumption
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	//	Global permission on interrupts
	sei();
	
	add_task(t_test_wr_uart_init_data);
	
	// Main loop
	while(true) {
		if(norm_task_list.first) {
			norm_task_list.first->exec();
			delete_task(norm_task_list.first);
		} else {					
			//	Turn on sleep mode. It can be possible if there will be Usart RX software handling. TODO!
		}
	}

	return 0;
}


