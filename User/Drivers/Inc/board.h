

#ifndef __BOARD_H__
#define __BOARD_H__

#include "main.h"

#define GPIO_WRITE(port, pin, level) HAL_GPIO_WritePin(port, pin, level)
#define GPIO_READ(port, pin) HAL_GPIO_ReadPin(port, pin)

void modem_power(bool onoff);
void modem_pwr_key(bool press, uint16_t ms);
void modem_reset(void);
void sensors_power(bool onoff);
void ble_reset_pin(bool onoff);
void ble_wakeup_mcu_init(void);
void mcu_wakeup_ble_pin(bool onoff);

/****************************************************
 *  platform tools
 *****************************************************/
void Delay_MS(uint16_t ms);
void Delay_S(uint16_t s);
void System_Reset(void);
uint32_t get_ticks(void);
#endif
