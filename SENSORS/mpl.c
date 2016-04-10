/* Functions and variables used for measurement control and getting data from MPL3115A2 */

#include "../general.h"
#include "mpl.h"

int32_t mpl_alt_raw;
uint32_t mpl_pres_raw;
int16_t mpl_temp_raw;

uint8_t mpl_meas_data[MPL_DATA_REGS_NUM];

double mpl_alt;
double mpl_pres;
double mpl_temp;

volatile bool mpl_is_meas_listed;

static inline void mpl_comp_temp() {
	mpl_temp = ((double)mpl_temp_raw)/256.0;
}

static inline void mpl_comp_pres() {
	mpl_pres = ((double)mpl_pres_raw)/64.0/100.0;
}

static inline void mpl_comp_alt() {
	mpl_alt = ((double)mpl_alt_raw)/65536.0;
}

void mpl_init() {
	I2C_write_byte(SLA_MPL_WR, MPL_PT_DATA_CFG, MPL_DREM | MPL_TDEFE | MPL_PDEFE);
}

bool mpl_take_alt_meas()
{
	if((I2C_read_byte(SLA_MPL_WR, MPL_DR_STATUS) & MPL_PDR) != MPL_PDR)
		return false;

	I2C_read_bytes(SLA_MPL_WR, MPL_DATA_REGS, MPL_DATA_REGS_NUM, mpl_meas_data);

	I2C_write_byte(SLA_MPL_WR, MPL_SYSMOD, 0);

	return true;
}

bool mpl_take_p_t_meas() {

	if(I2C_read_byte(SLA_MPL_WR, MPL_DR_STATUS) != (MPL_PDR | MPL_TDR | MPL_PTDR))
		return false;

	I2C_read_bytes(SLA_MPL_WR, MPL_DATA_REGS, MPL_DATA_REGS_NUM, mpl_meas_data);

	I2C_write_byte(SLA_MPL_WR, MPL_SYSMOD, 0);

	return true;
}

void t_mpl_take_p_t_meas() {
	/*	A variable defined to have under control buggy measurement preparing:
	 	when measurement was deputed, but no results can be get from sensor. */
	static uint16_t meas_not_rdy_cnt = 0;
	//	Attempting to get data for 500 milliseconds.
	if(meas_not_rdy_cnt == 500) {
		//	Let GNSS module depute new measurement
		meas_not_rdy_cnt = 0;
		return;
	}
	if(!mpl_take_p_t_meas()) {
		add_asynch_task(t_mpl_take_p_t_meas, 10, false);
		meas_not_rdy_cnt += 10;
	}
	else {
		meas_not_rdy_cnt = 0;
		add_task(t_mpl_prep_p_t_data);
		add_task(t_mpl_dep_alt_meas);
	}
}

bool pre_mpl_set_listed() {
	if(mpl_is_meas_listed) return false;
	mpl_is_meas_listed = true;
	return true;
}

void t_mpl_take_alt_meas() {
	/*	A variable defined to have under control buggy measurement preparing:
	 	when measurement was deputed, but no results can be get from sensor. */
	static uint16_t meas_not_rdy_cnt = 0;
	//	Attempting to get data for 500 milliseconds.
	if(meas_not_rdy_cnt == 500) {
		//	Let GNSS module depute new measurement
		meas_not_rdy_cnt = 0;
		return;
	}
	if(!mpl_take_alt_meas()) {
		add_asynch_task(t_mpl_take_alt_meas, 10, false);
		meas_not_rdy_cnt += 10;
		
	} else {
		meas_not_rdy_cnt = 0;
		add_task(t_mpl_dep_p_t_meas);
		add_task(t_mpl_prep_alt_data);
		mpl_is_meas_listed = false;
	}
}

void t_mpl_dep_alt_meas() {
	I2C_write_byte(SLA_MPL_WR, MPL_CTRL_REG1, MPL_ALT | MPL_SBYB | MPL_SET_OVRSMPL(MPL_OVRSMPL_64));
	I2C_write_byte(SLA_MPL_WR, MPL_CTRL_REG1, MPL_ALT | MPL_OST | MPL_SBYB | MPL_SET_OVRSMPL(MPL_OVRSMPL_64));
	I2C_write_byte(SLA_MPL_WR, MPL_SYSMOD, MPL_SYSMOD_BIT);
}

void t_mpl_dep_p_t_meas() {
	I2C_write_byte(SLA_MPL_WR, MPL_CTRL_REG1, MPL_SBYB| MPL_SET_OVRSMPL(MPL_OVRSMPL_64));
	I2C_write_byte(SLA_MPL_WR, MPL_CTRL_REG1, MPL_OST | MPL_SBYB| MPL_SET_OVRSMPL(MPL_OVRSMPL_64));
	I2C_write_byte(SLA_MPL_WR, MPL_SYSMOD, MPL_SYSMOD_BIT);

	//	258 ms corresponds to actually chosen oversample level (64)
	add_asynch_task(t_mpl_take_p_t_meas, 258, false);
}

void t_mpl_prep_alt_data() {

	mpl_alt_raw = ((int32_t)mpl_meas_data[0] << 24) |
			((int32_t)mpl_meas_data[1] << 16) |
				((int32_t)mpl_meas_data[2] << 8);

	mpl_comp_alt();

	add_task(t_mpl_save_alt_data);
}

void t_mpl_prep_p_t_data() {

	mpl_pres_raw  = ((uint32_t)mpl_meas_data[0] << 16) |
			((uint32_t)mpl_meas_data[1] << 8) |
				((uint32_t)mpl_meas_data[2]);

	mpl_temp_raw  = ((int16_t)mpl_meas_data[3] << 8) |
			((int16_t)mpl_meas_data[4]);

	mpl_comp_temp();
	mpl_comp_pres();

	add_task(t_mpl_save_p_t_data);
}

void t_mpl_save_p_t_data() {
	char *str;
	
	str = conv_meas_data(mpl_pres, pressure);
	SD_put_data_prog(PSTR("mpl_pres "));
	SD_put_data(str);
	free(str);
	
	SD_put_data_prog(PSTR("mpl_temp "));
	str = conv_meas_data(mpl_temp, temperature);
	SD_put_data(str);
	free(str);
}

void t_mpl_save_alt_data() {
	char * str = conv_meas_data(mpl_alt, altitude); 
	SD_put_data_prog(PSTR("mpl_alt "));
	SD_put_data(str);
	free(str);
}
