

#ifndef __BOARD_H__
#define __BOARD_H__

#include "main.h"

#define GPIO_WRITE(port, pin, level) HAL_GPIO_WritePin(port, pin, level)
#define GPIO_READ(port, pin) HAL_GPIO_ReadPin(port, pin)

void modem_power(bool onoff);
void modem_pwr_key(bool press, uint16_t ms);
void modem_reset(void);

#endif
