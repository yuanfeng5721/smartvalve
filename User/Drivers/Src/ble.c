/*
 * ble.c
 *
 *  Created on: 2022年2月15日
 *      Author: boboowang
 */
#define LOG_TAG  "BLE"
#include "board.h"
#include "log.h"
#include "usart.h"
#include "ble.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nvitem.h"
#include "device_nv.h"
#include "cmsis_os.h"
#include "utils_timer.h"
#include "utils_ringbuff.h"

sRingbuff g_ble_ringbuff;
static bool ble_inited = false;
static uint8_t ble_print_buff[512];
static bool isbleConnected = false;
static uint32_t old_time = 0, new_time = 0;

void ble_set_powerlevel(uint8_t level);

void ble_hardware_init(void)
{
	LOGI("%s: ble hardware init\r\n", __FUNCTION__);
	ble_reset_pin(true);
	ble_wakeup_mcu_init();
	MX_USART2_UART_Init();
	ble_set_powerlevel(1);
	isbleConnected = false;
}

void ble_set_powerlevel(uint8_t level)
{
	ble_print("AT+LOWL:1");
}

void ble_wakeup_mcu_Handler(uint16_t GPIO_Pin)
{
	if(GPIO_READ(bt_mode_GPIO_Port, bt_mode_Pin) == GPIO_PIN_SET)
	{
		ble_set_connect(true);
		ble_timeout_init();
		LOGI("%s: ble wakeup mcu\r\n", __FUNCTION__);
	}
}

void ble_timeout_init(void)
{
	old_time = new_time = get_ticks();
}

bool ble_check_timeout(uint32_t time)
{
	return ((new_time - old_time)>time)?true:false;
}

void ble_update_time(void)
{
	new_time = get_ticks();
}

void ble_set_connect(bool states)
{
	isbleConnected = states;
}

bool ble_check_connect(void)
{
	return isbleConnected;
}

void ble_resource_init(void)
{
	char *ringBuff = NULL;
	ringBuff = pvPortMalloc(BLE_RING_BUFF_SIZE);
	if (NULL == ringBuff) {
		LOGE("ble malloc ringbuff err \r\n");
		return;
	}
	ring_buff_init(&g_ble_ringbuff, ringBuff, BLE_RING_BUFF_SIZE);
	ble_inited = true;
}

void ble_uart_rx_isr_cb(uint8_t *pdata, uint8_t len)
{
	if(ble_inited) {
		(void)ring_buff_push_data(&g_ble_ringbuff, pdata, len);
	}
}

void ble_IRQHandler(UART_HandleTypeDef *huart)
{
	uint8_t ch;
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == SET) {
        ch = (uint8_t)READ_REG(huart->Instance->DR) & 0xFF;
        /*this callback for at_client*/
        ble_uart_rx_isr_cb(&ch, 1);
        //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    __HAL_UART_CLEAR_PEFLAG(huart);
}

static bool get_char(UART_HandleTypeDef *huart, char *buff, uint32_t timeout)
{
	Timer timer;

	countdown_ms(&timer, timeout);
	do {
#ifndef BLE_UART_RECV_IRQ
		if(HAL_UART_Receive(huart, buff, 1, timeout) != HAL_OK) {
			continue;
		}
#else
        if (0 ==
            ring_buff_pop_data(&g_ble_ringbuff, (uint8_t *)buff, 1)) {  // push data to ringbuff @ AT_UART_IRQHandler
            continue;
        }
#endif
		else {
			break;
		}
	} while (!expired(&timer));

	if (expired(&timer)) {
		return false;
	}

	return true;
}

static uint16_t get_line(UART_HandleTypeDef *huart, char *cmdbuf, size_t size, uint32_t timeout)
{
    int  read_len = 0;
    char ch = 0, last_ch = 0;
    bool is_full = false;
    bool  ret;

    while (1) {
        ret = get_char(huart, &ch, timeout);

        if (!ret) {
            return 0;
        }

        if (read_len < size) {
        	cmdbuf[read_len++] = ch;
        } else {
            is_full = true;
        }

        /* is newline or URC data */
        if ((ch == '\n' && last_ch == '\r')) {
            if (is_full) {
            	LOGE("cmd buffer size small!!!!!!\r\n");
                return 0;
            }
            break;
        }
        last_ch = ch;
    }
    return read_len;
}

uint16_t ble_get_cmd(char * cmdbuf, size_t size)
{
	return get_line(&huart2, cmdbuf, size, 50);
}

void ble_print(const char *format, ...)
{
	size_t length = 0;
	va_list ap;

	memset(ble_print_buff, 0, sizeof(ble_print_buff));
	va_start(ap, format);
	length = vsnprintf(ble_print_buff, sizeof(ble_print_buff), format, ap);
	va_end(ap);

	HAL_UART_Transmit(&huart2, ble_print_buff, length, length * 10);
}
