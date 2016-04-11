#include "general.h"
#include "uart.h"

//	Counter of received bits of the same value.
volatile uint8_t cycles_not_int;

volatile uint8_t bits_rec_cnt;
volatile uint8_t byte_rec;

void Timer2_init() {
	TCCR2A |= (1<<WGM21);				// CTC mode
	TCCR2B |= (1<<CS21);				// Prescaler F_CPU/8
	OCR2A = OCR_UART_CONTENT;			// 8000000/8/9600 = 104,1(6)
}

void PCINT_handler() {
	//	Firstly read TCNT content to avoid inconsistency.
	uint8_t TCNT_content = TCNT2;
	
	//	If TCNT content is almost equal OCR content it is understood, that next bit was received. 
	cycles_not_int += (TCNT_content > NEXT_RX_BIT_GATE ? 1 : 0);
	
	//	When received logic one after start bit skip start bit from counting.
	if(!bits_rec_cnt) {
		cycles_not_int -=1;
		byte_rec;
	}
	
	//	No negation needed, because logical one corresponds to GND voltage level (zero - VCC).
	if(UART_RX_PIN_STATE) {
		for(uint8_t i = 0; i < cycles_not_int; i++)
			byte_rec |= (1 << (UART_NUM_DATA_BITS - i - bits_rec_cnt - 1));
	}
	
	bits_rec_cnt += cycles_not_int;
	cycles_not_int = 0;
	
	if(bits_rec_cnt == UART_NUM_DATA_BITS) {
		Timer2_stop();
		bits_rec_cnt = 0;
		//return data_byte
		return;
	}
	
	//	Clear TCNT register and allow to TIMER2 overflow interrupt (no problem if was enabled).
	Timer2_start();
}

void TIMER2_OVF_handler() {
	//	1 bit period ended
	cycles_not_int += 1;
	
	//	If only ones were received:
	if(!bits_rec_cnt && cycles_not_int == UART_NUM_DATA_BITS + 1) {
		Timer2_stop();
		//return 0xFF
	}
	
	//	When last bit was one.
	if(bits_rec_cnt + cycles_not_int == UART_NUM_DATA_BITS) {
		for(uint8_t i = 0; i < cycles_not_int; i++)
			byte_rec |= (1 << (UART_NUM_DATA_BITS - i - bits_rec_cnt - 1));
		
		Timer2_stop();
		cycles_not_int = 0;
		bits_rec_cnt = 0;
		//return data_byte;
	}
}