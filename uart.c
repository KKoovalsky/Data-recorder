#include "general.h"
#include "uart.h"

#define GENERATE_SOFT_INT (PORTB |= (1<<PB2))

#define UART_PIN_STATE (PIND & (1<<PD0))

#define UART_TCNT_MARGIN (uint8_t)((F_CPU / 8.0 / UART_BAUD_RATE - 0.5) * 0.8)

#define TCNT_CLEAR (TCNT2 = 0)

#define CHECK_TIM_ENA (TIMSK2 & (1<<OCIE2A))

#define CHECK_OCM_FLAG_SET (TIFR2 & (1<<OCF2A))

#define CLEAR_OCM_FLAG (TIFR2 |= (1<<OCF2A))

volatile uint8_t cycles_not_int = 0xFF;
volatile uint8_t byte_rec;
volatile uint8_t bits_rec_cnt;

inline void Timer2_start() {
	TIMSK2 |= (1<<OCIE2A);
	TCNT_CLEAR;
	CLEAR_OCM_FLAG;
}

inline void Timer2_stop() {
	TIMSK2 &= ~(1<<OCIE2A);
	TCNT_CLEAR;
	CLEAR_OCM_FLAG;
}

ISR(PCINT3_vect) {
	//	Firstly read TCNT content to avoid inconsistency.
	uint8_t TCNT_content = TCNT2;
	
	//	Clear TCNT register.
	TCNT_CLEAR;
	
	//	Start period count timer, when start bit's falling edge detected.
	if(!CHECK_TIM_ENA) {
		Timer2_start();
		return;
	}
	
	//	If TCNT content is almost equal OCR content it is understood, that next bit was received.
	cycles_not_int += (TCNT_content > UART_TCNT_MARGIN ? 1 : 0);
	
	//	If both edge and compare match took place:
	if(CHECK_OCM_FLAG_SET) {
		// To clear flag write logical one to it.
		CLEAR_OCM_FLAG;
		// When TCNT set flag and got cleared.
		if(TCNT_content < UART_TCNT_MARGIN) cycles_not_int++;
	}
	
	//	Interpreting data sent.
	if(!UART_PIN_STATE) byte_rec |= ((1 << cycles_not_int) - 1) << bits_rec_cnt;
	
	bits_rec_cnt += cycles_not_int;
	cycles_not_int = 0;
	
	if(bits_rec_cnt == UART_NUM_DATA_BITS) {
		Timer2_stop();
		bits_rec_cnt = 0;
		cycles_not_int = 0xFF; //	Initialized with '-1', to avoid interpreting start bit as data bit.
		
		uart_byte_rec = byte_rec;
		GENERATE_SOFT_INT;
		
		byte_rec = 0;
		return;
	}
}

ISR(TIMER2_COMPA_vect) {
	//	1 bit period ended
	cycles_not_int += 1;
	
	//	When last bit logic zero.
	if(bits_rec_cnt + cycles_not_int == UART_NUM_DATA_BITS && UART_PIN_STATE) {
		Timer2_stop();
		byte_rec |= ((1 << cycles_not_int) - 1) << bits_rec_cnt;
		cycles_not_int = 0xFF;
		bits_rec_cnt = 0;
		
		uart_byte_rec = byte_rec;
		GENERATE_SOFT_INT;
		
		byte_rec = 0;
	}
}

