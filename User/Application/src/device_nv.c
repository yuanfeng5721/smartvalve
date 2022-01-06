/*
 * device_nv.c
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */


#include "device_nv.h"
#include "nvitem.h"
#include "utils.h"

const ef_env default_env_set[DEFAULT_NV_ITEMS] = {
		MAKE_NV_ITEM(NV_FLAG,			NVFLAG),
		MAKE_NV_ITEM(NV_VERSION,		NVVERSION),
		MAKE_NV_ITEM(UPDATE_FREQ,		"20"),
		MAKE_NV_ITEM(NV_IMEI,			"865233030271820"),
		MAKE_NV_ITEM(NV_PRODUCT_ID,		"199761"),
		MAKE_NV_ITEM(NV_DEVICE_ID,		"520256916"),
		MAKE_NV_ITEM(NV_AUTH_INFO,		"865233030271820"),
		MAKE_NV_ITEM(BOOT_MODE,			"0"),
		MAKE_NV_ITEM(BOOT_COUNT,		"0"),
		MAKE_NV_ITEM(WAKEUP_COUNT,		"0"),
		MAKE_NV_ITEM(SYSTEM_ACTIVE,		"0"),
		MAKE_NV_ITEM(MOTO_POSITION, 	"0"),
		MAKE_NV_ITEM(NV_GPRS_NB,    	"1"),
		MAKE_NV_ITEM(NV_F,				"1"),
		MAKE_NV_ITEM(MOTO_POSITION_MAX,	"11500"),
		MAKE_NV_ITEM(NV_WARRING_FLAG,	"false"),
		MAKE_NV_ITEM(NV_ONE_CIRCLE_PLUS,"11200"),
		MAKE_NV_ITEM(NV_PRE_SET_ANGLE,	"0"),
		MAKE_NV_ITEM(NV_F_PRESS_MAX,	"100"),
		MAKE_NV_ITEM(NV_F_PRESS_MIN,	"0"),
		MAKE_NV_ITEM(NV_B_PRESS_MAX,	"100"),
		MAKE_NV_ITEM(NV_B_PRESS_MIN,	"0"),
		MAKE_NV_ITEM(NV_ENCODER_COUNT,	"100"),
		MAKE_NV_ITEM(NV_MOTO_TIMER_COUNT, "382")
};

void print_software_version(void)
{
	printf("\r\nHardware Version: %s\r\n", HW_VERSION);
	printf("Software Version: %d-%02d-%02d_%s_%s\r\n", YEAR, MONTH + 1, DAY, __TIME__, SW_VERSION);
}
