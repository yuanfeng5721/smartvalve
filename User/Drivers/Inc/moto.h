/*
 * moto.h
 *
 *  Created on: 2022年2月28日
 *      Author: boboowang
 */

#ifndef DRIVERS_INC_MOTO_H_
#define DRIVERS_INC_MOTO_H_
#include <stdbool.h>
#include <stdint.h>
#include "tim.h"

typedef enum {
	TURN_FORWARD = true,
	TURN_BACKWARD = false,
}Moto_Dir;

typedef enum {
	MOTO_CTL_SUCCESS = 0,
	MOTO_CTL_MOTO_ERROR = -1,
}MOTO_RET_CODE;

#define TIMEOUT 4000
#define TIMEOUT_4S 4000
#define TIMEOUT_8M (1000*60*8)

typedef bool (*check_handle)(uint16_t);

bool moto_init(void);
void moto_encoder_test(uint16_t freq, uint16_t number, Moto_Dir direct);
int16_t moto_ctrl_for_angle(uint16_t freq, float angle);
uint16_t moto_get_freq(void);

#endif /* DRIVERS_INC_MOTO_H_ */
