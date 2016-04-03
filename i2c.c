/* Basic I2C bus handling */

#include "general.h"
#include "i2c.h"

void I2C_write_byte(uint8_t DEV_ADDR, uint8_t REG_TO_WRITE, uint8_t BYTE_TO_WRITE)
{
	TWI_start();
	TWI_write(DEV_ADDR);
	TWI_write(REG_TO_WRITE);
	TWI_write(BYTE_TO_WRITE);
	TWI_stop();
}

uint8_t I2C_read_byte(uint8_t DEV_ADDR, uint8_t REG_TO_READ)
{
	uint8_t temp;
	TWI_start();
	TWI_write(DEV_ADDR);
	TWI_write(REG_TO_READ);
	TWI_repeated_start();
	TWI_write(DEV_ADDR | 0x01);
	temp = TWI_read(NACK);
	TWI_stop();
	return temp;
}

void I2C_read_bytes(uint8_t DEV_ADDR, uint8_t REG_TO_READ,
		uint8_t NUM_OF_BYTES, uint8_t * destination)
{
	TWI_start();
	TWI_write(DEV_ADDR);
	TWI_write(REG_TO_READ);
	TWI_repeated_start();
	TWI_write(DEV_ADDR | 0x01);
	for(uint8_t i = NUM_OF_BYTES; i--;)
		destination[NUM_OF_BYTES - i - 1] = TWI_read(i ? ACK : NACK);
	TWI_stop();
}
