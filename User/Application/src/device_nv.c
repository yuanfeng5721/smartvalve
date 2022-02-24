/*
 * device_nv.c
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */
#define LOG_TAG "NV"
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include "device_nv.h"
#include "nvitem.h"
#include "utils.h"
#include "log.h"

uint32_t press_back_default_value[PB_DEFAULT_NUM]=
	{100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,200,\
	 200,200,200,200,200,200,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,\
	 100,100,100,100,100,100,100,100,100,200,200,200,200,200,200,200,200,200,200,100,100,100,\
	 100,100,100,100,100,100};

uint32_t q_default_value[Q_DEFAULT_NUM]=
	{50 ,50 ,50 ,50 ,50 ,50 ,50 ,50 ,50 ,50 ,50 ,50 ,50 ,100,100,100,100,100,200,200,200,200, \
	 200,200,350,350,350,200,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100, \
	 100,100,100,100,100,100,100,100,100,200,200,200,200,200,200,200,200,200,200,100,100,50 , \
	 50 ,50 ,50 ,50 ,50 ,50};

uint32_t zeta_value[ZETA_NUM]=
	{750000,600000,480000,380000,320000,260000,210000,170000,125000,80000,60000,43000,30000,\
	 24500,20000,14800,10600,7000,6300,5844,4900,4110,3650,3170,2588,2180,1790,1540,1380,1250,\
     1105,950,810,730,661,590,521,464,415,375,341,312,280,262,247,235,224,214,206,200};

uint32_t angle_default_value[ANGLE_DEFAULT_NUM] =
	{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,20,20,20,20,20,20,20,20,20,20,20,20,20,20, \
	 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20, \
	 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,0};

const static uint32_t  NVVERSION	= 6;  //default nv version, if need update default nv, please  NV_VERSION + 1
//const static uint8_t  update_freq = 20;
//const static uint32_t wakeup_count = 0;
//const static uint8_t  F_value = 1;
//const static uint8_t  warring_flag = false;
//const static uint8_t  pre_set_angle = 0;
//const static uint16_t f_press_max = 100;
//const static uint16_t f_press_min = 0;
//const static uint16_t b_press_max = 100;
//const static uint16_t b_press_min = 0;
//const static uint8_t  encoder_count = 100;
//const static uint16_t moto_timer_count = 382;
//static BootMode boot_mode = CONFIG_MODE;

const nvitem default_env_set[DEFAULT_NV_ITEMS] = {
		MAKE_NV_ITEM_STR(NV_IMEI,				"865233030271820"),
		MAKE_NV_ITEM_STR(NV_CLIENT_ID,			"TEST001"),
		MAKE_NV_ITEM_STR(NV_USERNAME,			"448403"),
		MAKE_NV_ITEM_STR(NV_PASSWORD,			"WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs="),
#if 0
		MAKE_NV_ITEM_STR(BOOT_MODE,	    "0"),
		MAKE_NV_ITEM_STR(BOOT_COUNT,		"0"),
		MAKE_NV_ITEM_STR(WAKEUP_COUNT,	"0"),
		MAKE_NV_ITEM_STR(SYSTEM_ACTIVE,	"0"),
		MAKE_NV_ITEM_STR(NV_GPRS_NB,    "1"),
		MAKE_NV_ITEM_STR(NV_F,					"1"),
		MAKE_NV_ITEM_STR(MOTO_POSITION_MAX,   "11500"),
		MAKE_NV_ITEM_STR(NV_WARRING_FLAG,  "false"),
		MAKE_NV_ITEM_STR(NV_ONE_CIRCLE_PLUS, "11200"),
		MAKE_NV_ITEM_STR(NV_PRE_SET_ANGLE, "0"),
		MAKE_NV_ITEM_STR(NV_F_PRESS_MAX, "100"),
		MAKE_NV_ITEM_STR(NV_F_PRESS_MIN, "0"),
		MAKE_NV_ITEM_STR(NV_B_PRESS_MAX, "100"),
		MAKE_NV_ITEM_STR(NV_B_PRESS_MIN, "0"),
		MAKE_NV_ITEM_STR(NV_ENCODER_COUNT, "100"),
		MAKE_NV_ITEM_STR(NV_MOTO_TIMER_COUNT, "382")
#else
#if 0
		MAKE_NV_ITEM_INT(NV_VERSION,			&NVVERSION, sizeof(NVVERSION)),
		MAKE_NV_ITEM_INT(UPDATE_FREQ,			&update_freq, sizeof(update_freq)),
		MAKE_NV_ITEM_INT(WAKEUP_COUNT,			&wakeup_count, sizeof(wakeup_count)),
		MAKE_NV_ITEM_INT(NV_F,					&F_value, sizeof(F_value)),
		MAKE_NV_ITEM_INT(NV_WARRING_FLAG,		&warring_flag, sizeof(warring_flag)),
		MAKE_NV_ITEM_INT(NV_PRE_SET_ANGLE,		&pre_set_angle, sizeof(pre_set_angle)),
		MAKE_NV_ITEM_INT(NV_F_PRESS_MAX,		&f_press_max, sizeof(f_press_max)),
		MAKE_NV_ITEM_INT(NV_F_PRESS_MIN,		&f_press_min, sizeof(f_press_min)),
		MAKE_NV_ITEM_INT(NV_B_PRESS_MAX,		&b_press_max, sizeof(b_press_max)),
		MAKE_NV_ITEM_INT(NV_B_PRESS_MIN,		&b_press_min, sizeof(b_press_min)),
		MAKE_NV_ITEM_INT(NV_ENCODER_COUNT,		&encoder_count, sizeof(encoder_count)),
		MAKE_NV_ITEM_INT(NV_MOTO_TIMER_COUNT, 	&moto_timer_count, sizeof(moto_timer_count)),
#else
		MAKE_NV_ITEM_INT(NV_VERSION,			NVVERSION),
		MAKE_NV_ITEM_INT(BOOT_MODE,	    		0),
		MAKE_NV_ITEM_INT(UPDATE_FREQ,			20),
		MAKE_NV_ITEM_INT(SAMPLE_FREQ,			5),
		MAKE_NV_ITEM_INT(WAKEUP_COUNT,			0),
		MAKE_NV_ITEM_INT(NV_F,					1),
		MAKE_NV_ITEM_INT(NV_WARRING_FLAG,		0),
		MAKE_NV_ITEM_INT(NV_PRE_SET_ANGLE,		0),
		MAKE_NV_ITEM_INT(NV_F_PRESS_MAX,		100),
		MAKE_NV_ITEM_INT(NV_F_PRESS_MIN,		0),
		MAKE_NV_ITEM_INT(NV_B_PRESS_MAX,		100),
		MAKE_NV_ITEM_INT(NV_B_PRESS_MIN,		0),
		MAKE_NV_ITEM_INT(NV_ENCODER_COUNT,		0),
		MAKE_NV_ITEM_INT(NV_MOTO_TIMER_COUNT, 	382),
#endif
		MAKE_NV_ITEM_ARRAY(PB_DEFAULT_KEY, press_back_default_value, sizeof(press_back_default_value)/sizeof(press_back_default_value[0])),
		MAKE_NV_ITEM_ARRAY(Q_DEFAULT_KEY, q_default_value, sizeof(q_default_value)/sizeof(q_default_value[0])),
		MAKE_NV_ITEM_ARRAY(ZETA_KEY, zeta_value, sizeof(zeta_value)/sizeof(zeta_value[0])),
		MAKE_NV_ITEM_ARRAY(ANGLE_DEFAULT_KEY, angle_default_value, sizeof(angle_default_value)/sizeof(angle_default_value[0])),
#endif
};


char* print_software_version(void)
{
	char ver_buff[64] = {0};
	LOG_RAW("\r\nHardware Version: %s\r\n", HW_VERSION);
//#ifdef DEBUG
	sprintf(ver_buff, "%d-%02d-%02d_%s_%s", YEAR, MONTH + 1, DAY, __TIME__, SW_VERSION);
//#else
//	sprintf(ver_buff, "%d-%02d-%02d_%s_%s%s", YEAR, MONTH + 1, DAY, __TIME__, RELEASE_STR, SW_VERSION);
//#endif
	//LOG_RAW("Software Version: %d-%02d-%02d_%s_%s\r\n", YEAR, MONTH + 1, DAY, __TIME__, SW_VERSION);
	LOG_RAW("Software Version: %s\r\n", ver_buff);
	return ver_buff;
}

void print_array_nv(const char *key, uint32_t *value, size_t len)
{
	LOG_RAW("%s: [", key);
	for(int i=0; i<len; i++) {
		LOG_RAW("%d,",value[i]);
	}
	LOG_RAW("]\r\n");
}

int init_nvitems(void)
{
	NvErrCode result = NV_NO_ERR;
	uint16_t i, len;
	uint8_t nv_ver = 0;
	uint32_t value[80] = {0};
#if 0
	ef_erase_all();
	//flash_test();
#else
	nvitem_init(NVVERSION);

	nv_ver = nvitem_get_int(NV_VERSION);
	LOGD("old nv version %d , new nv version %d \r\n", nv_ver, NVVERSION);
	if(nv_ver < NVVERSION) {
		nvitem_set_default();
		//nvitem_print();
	}
#if 1
	for(i=0; i<DEFAULT_NV_ITEMS; i++)
	{
		if(default_env_set[i].type == NV_VALUE_STRING) { //print string env
			char *str = nvitem_get_string(default_env_set[i].key);
			LOGD("%s: %s\r\n", default_env_set[i].key, (str==NULL)?"":str);
		} else if(default_env_set[i].type == NV_VALUE_INT) { //print int env
			uint32_t data = nvitem_get_int(default_env_set[i].key);
			LOGD("%s: %d\r\n", default_env_set[i].key, data);
		} else if(default_env_set[i].type == NV_VALUE_ARRAY) { //print array env
			len = nvitem_get_array(default_env_set[i].key, default_env_set[i].body.array, default_env_set[i].size);
			print_array_nv(default_env_set[i].key, default_env_set[i].body.array, len);
		}

	}
#else
	nvitem_print();
#endif
#endif
}

uint32_t get_F(void)
{
	uint32_t value = 0;

	value = nvitem_get_int(NV_F);

	return value;
}

void set_F(uint32_t value)
{
	nvitem_set_int(NV_F, value);
}

bool get_warring_flag(void)
{
	if(nvitem_get_int(NV_WARRING_FLAG))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void set_warring_flag(bool flag)
{
	nvitem_set_int(NV_WARRING_FLAG, flag);
}

BootMode device_get_bootmode(void)
{
	nvitem_get_int(BOOT_MODE);
}

void device_set_bootmode(BootMode mode)
{
	nvitem_set_int(BOOT_MODE, mode);
}
