#include "general.h"
#include "uart.h"

void Timer2_init() {
	TCCR2A |= (1<<WGM21);													// CTC mode
	TCCR2B |= (1<<CS21);													// Prescaler F_CPU/8
	OCR2A = (float)F_CPU / UART_PRESCALER / UART_BAUD_RATE + 0.5;			// 8000000/8/9600 = 104,1(6)
}