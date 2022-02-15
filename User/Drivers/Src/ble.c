/*
 * ble.c
 *
 *  Created on: 2022年2月15日
 *      Author: boboowang
 */
#define LOG_TAG  "BLE"
#include "board.h"
#include "log.h"

void ble_hardware_init(void)
{
	LOGI("%s: ble hardware init\r\n", __FUNCTION__);
	ble_reset_pin(true);
}
