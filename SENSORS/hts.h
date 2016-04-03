/* Functions and variables declarations used for measurement control and getting data from HTS221TR */

#ifndef _HTS_H
#define _HTS_H

#define SLA_HTS_WR 0xBE

#define HTS_CALLIB_REGS 0x30
#define HTS_CALLIB_REGS_NUM 16
#define HTS_CTRL_REG1 0x20
#define HTS_CTRL_REG2 0x21
#define HTS_STATUS_REG 0x27
#define HTS_AV_CONF 0x10
#define HTS_DATA_REGS 0x28
#define HTS_DATA_REGS_NUM 4

#define HTS_OVRSMPL_T2 0x00
#define HTS_OVRSMPL_H4 0x00
#define HTS_OVRSMPL_T4 0x01
#define HTS_OVRSMPL_H8 0x01
#define HTS_OVRSMPL_T8 0x02
#define HTS_OVRSMPL_H16 0x02
#define HTS_OVRSMPL_T16 0x03
#define HTS_OVRSMPL_H32 0x03
#define HTS_OVRSMPL_T32 0x04
#define HTS_OVRSMPL_H64 0x04
#define HTS_OVRSMPL_T64 0x05
#define HTS_OVRSMPL_H128 0x05
#define HTS_OVRSMPL_T128 0x06
#define HTS_OVRSMPL_H256 0x06
#define HTS_OVRSMPL_T256 0x07
#define HTS_OVRSMPL_H512 0x07

#define HTS_SET_TEMP_OVRSMPL(x) ((x)<<3)
#define HTS_SET_HUM_OVRSMPL(x) (x)

#define HTS_AUTOINCREMENT 0x80

#define HTS_PD 0x80
#define HTS_BDU 0x04
#define HTS_ODR1 0x02
#define HTS_ODR0 0x01
#define HTS_ONE_SHOT 0x01
#define HTS_H_DA 0x02
#define HTS_T_DA 0x01

typedef struct {
	uint16_t hum_raw;
	int16_t temp_raw;
} hts_meas_s;

typedef union {
	uint8_t meas_data[4];
	hts_meas_s meas_data_s;
} hts_meas_t;


void hts_init();
void hts_comp_temp();
void hts_comp_hum();
bool hts_take_meas();

//	To use with add_task functions
void t_hts_take_meas();
void t_hts_dep_meas();
void t_hts_prep_data();
void t_hts_save_data();

#endif
