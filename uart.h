#ifndef UART_H_
#define UART_H_

#define UART_BAUD_RATE 9600.0

/*	To change prescaling additional updates in Timer2_init function should be added,
	according to datasheet.																*/
#define UART_PRESCALER 8.0

#define UART_RX_PIN PD0	

//	Timer 2 is used to count bits in frame.
void Timer2_init();

inline void Timer2_start() {
	TCNT2 = 0;
	TIMSK2 |= (1<<OCIE2A);				//	Allow interrupt in CTC mode
}

inline void Timer2_stop() {
	TIMSK2 &= ~(1<<OCIE2A);				//	Forbid interrupt in CTC mode
}

// RXD0 corresponds to PCINT24 pin, so any hardware change is needed.
inline void UART_init() {
	//DDRD = (1<<UART_RX_PIN);			//	PD0 - RXD0 - PCINT24 pin as input
	PCICR |= (1<<PCIE3);				//	Enable pin change interrupt on PCINT31:24 pins.
	PCMSK3 |= (1<<PCINT24);				//	Enable pin change interrupt PCINT24 pin.
}

#endif /* UART_H_ */