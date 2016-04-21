#ifndef UART_H_
#define UART_H_

#define UART_BAUD_RATE 9600.0

/*	To change prescaling additional updates in Timer2_init function should be added,
	according to datasheet.																*/
#define UART_PRESCALER 8.0

#define UART_RX_PIN PD0	

#define UART_RX_PIN_STATE (PIND & (1<<UART_RX_PIN))

#define OCR_UART_CONTENT ((float)F_CPU / UART_PRESCALER / UART_BAUD_RATE + 0.5)

#define NEXT_RX_BIT_GATE (0.8 * OCR_UART_CONTENT)

#define UART_NUM_DATA_BITS 8

#define GENERATE_SOFT_INT (PORTB |= (1<<PB2))

inline void Timer2_start() {
	TCNT2 = 0;
	TIMSK2 |= (1<<OCIE2A);				//	Allow interrupt in CTC mode
}

inline void Timer2_stop() {
	TIMSK2 &= ~(1<<OCIE2A);				//	Forbid interrupt in CTC mode
	TCNT2 = 0;
}

// RXD0 corresponds to PCINT24 pin, so any hardware change is needed.
inline void UART_init() {
	PORTD |= (1<<UART_RX_PIN);			//	PD0 - RXD0 - PCINT24 pin as input - VCC pull up.
	PCICR |= (1<<PCIE3);				//	Enable pin change interrupt on PCINT31:24 pins.
	PCMSK3 |= (1<<PCINT24);				//	Enable pin change interrupt PCINT24 pin.
	
	/*	Set PB2 (INT2 pin) as output to generate software interrupt, to move received bytes handling 
		on other level of application.														*/
	DDRB |= (1<<PB2);
	EICRA |= (1<<ISC21) | (1<<ISC20);	//	Set rising edge triggering software interrupt.
	EIMSK |= (1<<INT2);					//	Enable interrupt generated by INT2 pin.		
}

//	Timer 2 is used to count bits in frame.
void Timer2_init();

#endif /* UART_H_ */