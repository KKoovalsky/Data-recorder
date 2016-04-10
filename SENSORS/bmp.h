/* Functions and variables declarations used for measurement control and getting data from BMP280 */

#ifndef _BMP_H
#define _BMP_H

#define BAUD_RATE_9600_H 0
#define BAUD_RATE_9600_L 51

#define USART_ENTER 0x0D
#define USART_NEW_LINE 0x0A

#define ACK 1
#define NACK 0

#define SLA_BMP_WR 0xEC
#define SLA_BMP_RD 0xED
#define SLA_BMP_ID_ADDR 0xD0
#define SLA_BMP_ID 0x58

#define BMP_CTRL_MEAS 0xF4
#define BMP_OVRSMPL_1 0x01
#define BMP_OVRSMPL_2 0x02
#define BMP_OVRSMPL_4 0x03
#define BMP_OVRSMPL_8 0x04
#define BMP_OVRSMPL_16 0x05
#define BMP_SET_TEMP_OVRSMPL(x) ((x)<<5)
#define BMP_SET_PRES_OVRSMPL(x) ((x)<<2)

#define BMP_FORCED_MODE 0x02

#define BMP_COMP_PARAMS 0x88
#define BMP_COMP_REG_NUM 24

#define BMP_STATUS_MEAS 0x08
#define BMP_STATUS_ADDR 0xF3

#define BMP_MEAS_REGS 0xF7
#define BMP_MEAS_REGS_NUM 6

typedef struct {
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
} bmp_comp_regs_s;

typedef union {
	uint8_t comp_reg[24];
	bmp_comp_regs_s comp_regs;
} bmp_comp_regs_t;

typedef int32_t BMP_temp_raw_t;
typedef uint32_t BMP_pres_raw_t;

void bmp_init();
int32_t conv_bmp_meas_data(uint8_t XLSB, uint8_t LSB, uint8_t MSB);
void bmp_comp_temp(BMP_temp_raw_t temp_data);
void bmp_comp_pres(BMP_pres_raw_t pres_data);
bool bmp_take_meas();

//	To use with add_task functions
void t_bmp_take_meas();
void t_bmp_dep_meas();
void t_bmp_prep_data();
void t_bmp_save_data();

bool pre_bmp_set_listed();

#endif
