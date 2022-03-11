/*
 * device_nv.h
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_DEVICE_NV_H_
#define APPLICATION_INC_DEVICE_NV_H_

#ifndef DEBUG
#include "version.h"
#endif
#include "nvitem.h"

#define DEFAULT_NV_ITEMS  DEFAULT_NV_ITEM_NUM   //22 items

//#define NV_FLAG "nv_flag"
#define NV_VERSION "nv_version"
#define UPDATE_FREQ   "update_freq"
#define SAMPLE_FREQ   "sample_freq"
#define NV_IMEI      		"imei"
#define NV_CLIENT_ID    "clientid"
#define NV_USERNAME     "username"
#define NV_PASSWORD		"passwd"
#define BOOT_MODE			"boot_mode"
#define BOOT_COUNT    "boot_count"
#define WAKEUP_COUNT  "wakeup_count"
#define SYSTEM_ACTIVE "system_active"
#define MOTO_POSITION "moto_p"       //moto position
#define NV_GPRS_NB		"GPRS_NB"
#define NV_F          "F"
#define MOTO_POSITION_MAX   "moto_p_max"
#define NV_WARRING_FLAG "warring_flag"
#define NV_ONE_CIRCLE_PLUS  "circle_plus"
#define NV_PRE_SET_ANGLE "pre_angle"
#define NV_F_PRESS_MAX   "press_f_max"
#define NV_F_PRESS_MIN   "press_f_min"
#define NV_B_PRESS_MAX   "press_b_max"
#define NV_B_PRESS_MIN   "press_b_min"
#define NV_ENCODER_COUNT "encoder_count"
#define NV_MOTO_TIMER_COUNT "moto_tim_cnt"
#define NV_MOTO_TIMER_FREQ "moto_tim_freq"

/**********************************************************************
 * 						nv value define
 **********************************************************************/
//#define NVFLAG "AA55"
//#define NV_VERSION 1
//#define FORCE_UPDATE_NV "no" ///"yes" is force update nv, "no" don't update nv

#define RELEASE_STR "Release_"
#define DEBUG_STR 	"Debug"

#define TOSTRING(p) #p
#define JOINER(n, m) n TOSTRING(m)

#ifndef DEBUG
#define SW_VERSION JOINER(RELEASE_STR, BUILD_VERSION)
#else

#define SW_VERSION DEBUG_STR
#endif

#define HW_VERSION "V1.0"


//boot mode
typedef enum{
	CONFIG_MODE = 0,
	NO_ACTIVITE_MODE = 1,
	NORMAL_MODE = 2,
	MAX_MODE = 3,
}BootMode;

typedef enum{
	SAMPLE_5MIN 	= 5,
	SAMPLE_10MIN  	= 10,
	SAMPLE_20MIN 	= 20,
}SampleFreq;

typedef enum{
	UPDATE_5MIN 	= 5,
	UPDATE_10MIN  	= 10,
	UPDATE_20MIN 	= 20,
}UpdateFreq;

#define CHECK_FREQ(f) (!(f != SAMPLE_5MIN && f != SAMPLE_10MIN && f != SAMPLE_20MIN))

#define CHECK_SAMPLE_FREQ(f) ((f != SAMPLE_5MIN && f != SAMPLE_10MIN && f != SAMPLE_20MIN)?SAMPLE_5MIN:f)
#define CHECK_UPDATE_FREQ(f) ((f != UPDATE_5MIN && f != UPDATE_10MIN && f != UPDATE_20MIN)?UPDATE_20MIN:f)

/*********************************************************************
 *
 *********************************************************************/
#define PB_DEFAULT_KEY  "PRESS_BACK_DEFAULT"
#define PB_DEFAULT_NUM  72

#define Q_DEFAULT_KEY   "Q_DEFAULT"
#define Q_DEFAULT_NUM   72

#define ZETA_KEY        "ZETA"
#define ZETA_NUM        50

#define ANGLE_DEFAULT_KEY "ANGLE_DEFAULT"
#define ANGLE_DEFAULT_NUM 288
#define ANGLE_NUM_QUARTER  ANGLE_DEFAULT_NUM/4

/***********************************************************************
 * 						extern variable
 ***********************************************************************/
extern uint32_t press_back_default_value[PB_DEFAULT_NUM];
extern uint32_t q_default_value[Q_DEFAULT_NUM];
extern uint32_t zeta_value[ZETA_NUM];
extern uint32_t angle_default_value[ANGLE_DEFAULT_NUM];
/***********************************************************************
 * 						function define
 ***********************************************************************/
char* print_software_version(void);
int init_nvitems(void);

uint32_t get_F(void);
void set_F(uint32_t value);
bool get_warring_flag(void);
void set_warring_flag(bool flag);
BootMode device_get_bootmode(void);
void device_set_bootmode(BootMode mode);

void update_angle_data(uint8_t index, char *str, uint16_t size);
void update_flow_data(char *str, uint16_t size);
void update_sample_freq(SampleFreq freq);
void update_update_freq(UpdateFreq freq);
#endif /* APPLICATION_INC_DEVICE_NV_H_ */
