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

ISR(PCINT3_vect) {
	//	Firstly read TCNT content to avoid inconsistency.
	uint8_t TCNT_content = TCNT2;
	
	//	If TCNT content is almost equal OCR content it is understood, that next bit was received. 
	cycles_not_int += (TCNT_content > NEXT_RX_BIT_GATE ? 1 : 0);
	
	//	Start period count timer, when start bit's falling edge detected.
	if(cycles_not_int == 0xFF) {
		Timer2_start();
		return;
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
		cycles_not_int = 0xFF; //	Initialized with '-1', to avoid interpreting start bit as data bit.
		uart_byte_rec = byte_rec;
		GENERATE_SOFT_INT;
		return;
	}
	
	//	Clear TCNT register.
	TCNT2 = 0;
}

ISR(TIMER2_COMPA_vect) {
	//	1 bit period ended
	cycles_not_int += 1;
	
	//	When last bit logic zero.
	if(bits_rec_cnt + cycles_not_int == UART_NUM_DATA_BITS && UART_RX_PIN_STATE) {
		Timer2_stop();
		cycles_not_int = 0xFF;
		bits_rec_cnt = 0;
		uart_byte_rec = byte_rec;
		GENERATE_SOFT_INT;
	}
}