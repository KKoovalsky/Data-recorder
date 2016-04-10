/* Functions and variables declarations used for measurement control and getting data from BMP280 */

#ifndef _MPL_H
#define _MPL_H

#define SLA_MPL_WR 0xC0

#define MPL_DR_STATUS 0x06
#define MPL_PTDR 0x08
#define MPL_PDR 0x04
#define MPL_TDR 0x02

#define MPL_DATA_REGS 0x01
#define MPL_DATA_REGS_NUM 5
#define MPL_ALT_DATA_REGS_NUM 3

#define MPL_SYSMOD 0x11
#define MPL_SYSMOD_BIT 0x01

#define MPL_PT_DATA_CFG 0x13
#define MPL_DREM 0x04
#define MPL_PDEFE 0x02
#define MPL_TDEFE 0x01

#define MPL_CTRL_REG1 0x26
#define MPL_ALT 0x80 			//Altimeter mode | 0x01; Pressure mode - & ~ 0x01
#define MPL_OST 0x02
#define MPL_SBYB 0x01

									// Minimum time between samples
#define MPL_OVRSMPL_1 0x00			// 6 ms
#define MPL_OVRSMPL_2 0x01			// 10 ms
#define MPL_OVRSMPL_4 0x02			// 18 ms
#define MPL_OVRSMPL_8 0x03			// 34 ms
#define MPL_OVRSMPL_16 0x04			// 66 ms
#define MPL_OVRSMPL_32 0x05			// 130 ms
#define MPL_OVRSMPL_64 0x06			// 258 ms
#define MPL_OVRSMPL_128 0x07		// 512 ms
#define MPL_SET_OVRSMPL(x) ((x)<<3)

void mpl_init();
bool mpl_take_alt_meas();
bool mpl_take_p_t_meas();

//	To use with add_task functions
void t_mpl_take_p_t_meas();
void t_mpl_take_alt_meas();
void t_mpl_dep_alt_meas();
void t_mpl_dep_p_t_meas();
void t_mpl_prep_alt_data();
void t_mpl_prep_p_t_data();
void t_mpl_save_p_t_data();
void t_mpl_save_alt_data();

bool pre_mpl_set_listed();

#endif
