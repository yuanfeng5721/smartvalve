/*
 * sensors.h
 *
 *  Created on: 2021年12月30日
 *      Author: boboowang
 */
#include "stm32l1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef DRIVERS_INC_SENSORS_H_
#define DRIVERS_INC_SENSORS_H_

#define ADC_CHANNEL_NAME_LEN   15
#define SAMPLE_ADC_CHANNEL     7
#define AVERAGE_MAX_NUM        12
typedef enum{
	MOTO_BAT_CH = 0,
	VALVE_ANGLE_CH,
	PRESS_REAR_CH,
	PRESS_FRONT_CH,
	SYS_BAT_CH,
	SYS_TEMP_CH,
	MOTO_TEMP_CH,
	MAX_SAMPLE_CH
}sample_channel_e;

#define MOTO_BATTERY    "moto_battery"
#define VALVE_ANGLE     "angle"
#define STRESS_FRONT    "front_press"
#define STRESS_REAR     "rear_press"
#define SYSTEM_BATTERY  "sys_battery"
#define SYSTEM_TEMP     "sys_temp"

typedef struct linear_equs_t{
	float sploe;
	float intercept;
}linear_equs_t;

typedef struct sensors_sample_t{
	const char * const name;
	uint32_t channel;
	float voltage;
	float value;
} sensors_sample_t;

sensors_sample_t* Sensors_Sample_Data(void);
void Sensors_Power(bool onoff);
#endif /* DRIVERS_INC_SENSORS_H_ */
