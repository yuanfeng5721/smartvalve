/*
 * ble.h
 *
 *  Created on: 2022年2月15日
 *      Author: boboowang
 */

#ifndef DRIVERS_INC_BLE_H_
#define DRIVERS_INC_BLE_H_

#define BLE_UART_RECV_IRQ
#define BLE_RING_BUFF_SIZE   512

#define ble_IRQHandler HAL_USART2_IrqCallback
#define ble_wakeup_mcu_Handler HAL_GPIO_EXTI_Callback

void ble_hardware_init(void);
void ble_resource_init(void);
uint16_t ble_get_cmd(char * cmdbuf, size_t size);
void ble_print(const char *format, ...);
void ble_set_connect(bool states);
bool ble_check_connect(void);
void ble_set_powerlevel(uint8_t level);
void ble_timeout_init(void);
bool ble_check_timeout(uint32_t time);
void ble_update_time(void);

#endif /* DRIVERS_INC_BLE_H_ */
