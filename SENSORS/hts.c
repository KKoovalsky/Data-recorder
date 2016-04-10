/* Functions and variables used for measurement control and getting data from HTS221TR */

#include "../general.h"
#include "hts.h"

hts_meas_t hts_meas_data;
double hts_temp;
double hts_hum;

uint8_t H0_rH_x2;
uint8_t H1_rH_x2;
uint16_t T0_degC_x8;
uint16_t T1_degC_x8;
uint16_t H0_T0_out;
uint16_t H1_T0_out;
int16_t T0_out;
int16_t T1_out;

volatile bool hts_is_meas_listed = false;

void hts_comp_temp() {
	int32_t var1, var2, var3;

	var1 = ((uint32_t)(T1_degC_x8 - T0_degC_x8)) *
			((int32_t)hts_meas_data.meas_data_s.temp_raw);

	var2 = ((uint32_t)T0_degC_x8) * ((int32_t)T1_out);

	var3 = ((int32_t)T0_out) * ((uint32_t)T1_degC_x8);

	hts_temp = (double)((var1 + var2 - var3)/((double)(T1_out - T0_out))/8.0);
}

void hts_comp_hum() {
	uint32_t var1, var2, var3;

	var1 = ((uint32_t)(H1_rH_x2 - H0_rH_x2)) *
			((uint32_t)hts_meas_data.meas_data_s.hum_raw);

	var2 = ((uint32_t)H0_rH_x2) *((uint32_t)H1_T0_out);

	var3 = ((uint32_t)H0_T0_out) * ((uint32_t)H1_rH_x2);

	hts_hum =  (double)((var1 + var2 - var3)/((double)(H1_T0_out - H0_T0_out))/2.0);
}

void hts_init() {
	uint8_t callib_regs[HTS_CALLIB_REGS_NUM];

	/*	Set PD bits (set device active) and BDU bits (data registers
		actualization only after previous were read)				*/
	I2C_write_byte(SLA_HTS_WR, HTS_CTRL_REG1, HTS_PD | HTS_BDU);

	//	Measurement precision possibly the best
	I2C_write_byte(SLA_HTS_WR, HTS_AV_CONF,
			HTS_SET_HUM_OVRSMPL(HTS_OVRSMPL_H512) | HTS_SET_TEMP_OVRSMPL(HTS_OVRSMPL_T256));

	//	Callibration registers reading.
	I2C_read_bytes(SLA_HTS_WR, HTS_CALLIB_REGS | HTS_AUTOINCREMENT,
			HTS_CALLIB_REGS_NUM, callib_regs);

	//	Mapping callibration registers on variables.
	H0_rH_x2 = callib_regs[0];
	H1_rH_x2 = callib_regs[1];
	T0_degC_x8 = callib_regs[2] | ((((uint16_t)callib_regs[5]) & 0x03) << 8);
	T1_degC_x8 = callib_regs[3] | ((((uint16_t)callib_regs[5]) & 0x0C) << 6);
	H0_T0_out = callib_regs[6] | (((uint16_t)callib_regs[7]) << 8);
	H1_T0_out = callib_regs[10] | (((uint16_t)callib_regs[11]) << 8);
	T0_out = callib_regs[12] | (((uint16_t)callib_regs[13]) << 8);
	T1_out = callib_regs[14] | (((uint16_t)callib_regs[15]) << 8);
}

bool hts_take_meas() {

	if(I2C_read_byte(SLA_HTS_WR, HTS_STATUS_REG) != (HTS_H_DA | HTS_T_DA))
		return false;

	I2C_read_bytes(SLA_HTS_WR, HTS_DATA_REGS | HTS_AUTOINCREMENT,
			HTS_DATA_REGS_NUM, hts_meas_data.meas_data);

	return true;
}

void t_hts_dep_meas() {
	I2C_write_byte(SLA_HTS_WR, HTS_CTRL_REG2, HTS_ONE_SHOT);
}


void t_hts_prep_data() {
	hts_comp_temp();
	hts_comp_hum();

	add_task(t_hts_save_data);
}

bool pre_hts_set_listed() {
	if(hts_is_meas_listed) return false;
	hts_is_meas_listed = true;
	return true;
}

void t_hts_take_meas() {
	/*	A variable defined to have under control buggy measurement preparing:
	 	when measurement was deputed, but no results can be get from sensor. */
	static uint16_t meas_not_rdy_cnt = 0;
	//	Attempting to get data for 500 miliseconds.
	if(meas_not_rdy_cnt == 500) {
		//	Let GNSS module depute new measurement
		meas_not_rdy_cnt = 0;
		return;
	}
	if(!hts_take_meas()) {
		add_asynch_task(t_hts_take_meas, 10, false);
		meas_not_rdy_cnt += 10;
	}
	else {
		meas_not_rdy_cnt = 0;
		add_task(t_hts_dep_meas);
		add_task(t_hts_prep_data);
		hts_is_meas_listed = false;
	}
}

void t_hts_save_data() {
	char * str;
	
	str = conv_meas_data(hts_hum, humidity);
	SD_put_data_prog(PSTR("hts_hum "));
	SD_put_data(str);
	free(str);
	
	str = conv_meas_data(hts_temp, temperature);
	SD_put_data_prog(PSTR("hts_temp "));
	SD_put_data(str);
	free(str);
}
