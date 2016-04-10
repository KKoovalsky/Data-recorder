/* Functions and variables used for measurement control and getting data from BMP280 */

#include "../general.h"
#include "bmp.h"

bmp_comp_regs_t bmp_c_regs;

BMP_temp_raw_t bmp_temp_raw;
BMP_pres_raw_t bmp_pres_raw;

int32_t temp_fine;

uint8_t temp_meas_data[BMP_MEAS_REGS_NUM];

double bmp_temp;
double bmp_pres;

/*	A variable defined to prevent multiple task listing	*/
volatile bool bmp_is_meas_listed = false;

void bmp_init() {
	I2C_read_bytes(SLA_BMP_WR, BMP_COMP_PARAMS, BMP_COMP_REG_NUM, bmp_c_regs.comp_reg);
}

int32_t conv_bmp_meas_data(uint8_t XLSB, uint8_t LSB, uint8_t MSB) {
	return ((((int32_t)XLSB) >> 4) | (((int32_t)LSB) << 4) | (((int32_t)MSB) << 12));
}

void bmp_comp_temp(BMP_temp_raw_t temp_data) {
	BMP_temp_raw_t temp1, temp2, T;

	temp1 = ((((temp_data>>3) - ((BMP_temp_raw_t)bmp_c_regs.comp_regs.dig_T1<<1))) *
			((BMP_temp_raw_t)bmp_c_regs.comp_regs.dig_T2)) >> 11;

	temp2 = (((((temp_data>>4) - ((BMP_temp_raw_t)bmp_c_regs.comp_regs.dig_T1)) *
		((temp_data>>4) - ((BMP_temp_raw_t)bmp_c_regs.comp_regs.dig_T1))) >> 12) *
				((BMP_temp_raw_t)bmp_c_regs.comp_regs.dig_T3)) >> 14;

	temp_fine = temp1 + temp2;
	T = (temp_fine * 5 + 128) >> 8;
	bmp_temp = ((double)T)/100.0;
}

void bmp_comp_pres(BMP_pres_raw_t pres_data) {
	int32_t var1, var2;
	BMP_pres_raw_t p;

	var1 = (((int32_t)temp_fine)>>1) - (int32_t)64000;

	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)bmp_c_regs.comp_regs.dig_P6);

	var2 = var2 + ((var1*((int32_t)bmp_c_regs.comp_regs.dig_P5))<<1);

	var2 = (var2>>2)+(((int32_t)bmp_c_regs.comp_regs.dig_P4)<<16);

	var1 = (((bmp_c_regs.comp_regs.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) +
			((((int32_t)bmp_c_regs.comp_regs.dig_P2) * var1)>>1))>>18;

	var1 =((((32768+var1))*((int32_t)bmp_c_regs.comp_regs.dig_P1))>>15);

	if (var1 == 0)
		return; // avoid exception caused by division by zero

	p = (((BMP_pres_raw_t)(((int32_t)1048576)-pres_data)-(var2>>12)))*3125;
	if (p < 0x80000000)
		p = (p << 1) / ((BMP_pres_raw_t)var1);
	else
		p = (p / (BMP_pres_raw_t)var1) * 2;

	var1 = (((int32_t)bmp_c_regs.comp_regs.dig_P9) *
			((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)bmp_c_regs.comp_regs.dig_P8))>>13;
	p = (BMP_pres_raw_t)((int32_t)p + ((var1 + var2 + bmp_c_regs.comp_regs.dig_P7) >> 4));
	bmp_pres = ((double)p)/100.0;
}

bool bmp_take_meas() {
	if((I2C_read_byte(SLA_BMP_WR, BMP_STATUS_ADDR)
				& BMP_STATUS_MEAS) == BMP_STATUS_MEAS)
		return false;

	I2C_read_bytes(SLA_BMP_WR, BMP_MEAS_REGS, BMP_MEAS_REGS_NUM, temp_meas_data);

	return true;
}

void t_bmp_dep_meas() {
	I2C_write_byte(SLA_BMP_WR, BMP_CTRL_MEAS,
			BMP_FORCED_MODE | BMP_SET_PRES_OVRSMPL(BMP_OVRSMPL_16) | BMP_SET_TEMP_OVRSMPL(BMP_OVRSMPL_16));
}


void t_bmp_prep_data() {

	bmp_temp_raw = (BMP_temp_raw_t)conv_bmp_meas_data(
			temp_meas_data[5], temp_meas_data[4], temp_meas_data[3]);

	bmp_pres_raw = (BMP_pres_raw_t)conv_bmp_meas_data(
			temp_meas_data[2], temp_meas_data[1], temp_meas_data[0]);

	bmp_comp_temp(bmp_temp_raw);
	bmp_comp_pres(bmp_pres_raw);

	add_task(t_bmp_save_data);
}

bool pre_bmp_set_listed() {
	if(bmp_is_meas_listed) return false;
	bmp_is_meas_listed = true;
	return true;
}

void t_bmp_take_meas() {
	/*	A variable defined to have under control buggy measurement preparing:
	 	when measurement was deputed, but no results can be get from sensor. */
	static uint16_t meas_not_rdy_cnt = 0;
	//	Attempting to get data for 500 milliseconds.
	if(meas_not_rdy_cnt == 500) {
		//	Let GNSS module depute new measurement
		meas_not_rdy_cnt = 0;
		return;
	}
	if(!bmp_take_meas()) {
		add_asynch_task(t_bmp_take_meas, 10, false);
		meas_not_rdy_cnt += 10;
	}
	else {
		meas_not_rdy_cnt = 0;
		add_task(t_bmp_dep_meas);
		add_task(t_bmp_prep_data);
		bmp_is_meas_listed = false;		
	}
}

void t_bmp_save_data() {
	SD_put_data_prog(PSTR("bmp_pres "));
	SD_put_data(conv_meas_data(bmp_pres, pressure));
	SD_put_data_prog(PSTR("bmp_temp "));
	SD_put_data(conv_meas_data(bmp_temp, temperature));
}
