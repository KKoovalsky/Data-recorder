/* Declarations for basic I2C bus handling */

#ifndef _I2C_H
#define _I2C_H

inline void TWI_init() {
	TWBR = 32;   		//SCL 100 kHz
}

inline void TWI_start() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
	while(!(TWCR&(1<<TWINT)));
}

inline void TWI_stop() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	while((TWCR&(1<<TWSTO)));
}

inline void TWI_repeated_start() {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
	while(!(TWCR&(1<<TWINT)));
}

inline void TWI_write(uint8_t data_byte) {
	TWDR = data_byte;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
}

inline uint8_t TWI_read(uint8_t ack) {
	TWCR = (1<<TWINT) | (ack<<TWEA) | (1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
	return TWDR;
}


void I2C_write_byte(uint8_t DEV_ADDR, uint8_t REG_TO_WRITE, uint8_t BYTE_TO_WRITE);
uint8_t I2C_read_byte(uint8_t DEV_ADDR, uint8_t REG_TO_READ);
void I2C_read_bytes(uint8_t DEV_ADDR, uint8_t REG_TO_READ,
		uint8_t NUM_OF_BYTES, uint8_t * destination);


#endif
