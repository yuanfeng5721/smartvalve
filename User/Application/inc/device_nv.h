/*
 * device_nv.h
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_DEVICE_NV_H_
#define APPLICATION_INC_DEVICE_NV_H_

#define DEFAULT_NV_ITEMS  EF_DEFAULT_ENV_ITEM   //22 items

//#define NV_FLAG "nv_flag"
#define NV_VERSION "nv_version"
#define UPDATE_FREQ   "update_freq"
#define NV_IMEI      		"imei"
#define NV_PRODUCT_ID    "product_id"
#define NV_DEVICE_ID     "device_id"
#define NV_AUTH_INFO			"auth_info"
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

/**********************************************************************
 * 						nv value define
 **********************************************************************/
//#define NVFLAG "AA55"
//#define NV_VERSION 1
//#define FORCE_UPDATE_NV "no" ///"yes" is force update nv, "no" don't update nv

#define SW_VERSION "V1.0.0"
#define HW_VERSION "V1.0"


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
#define ANGLE_DEFAULT_NUM 72
/***********************************************************************
 * 						function define
 ***********************************************************************/
void print_software_version(void);
int init_nvitems(void);

#endif /* APPLICATION_INC_DEVICE_NV_H_ */
