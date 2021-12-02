/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "iot_import.h"


#ifdef AT_TCP_ENABLED
#define USE_STM_MCU

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "at_client.h"
#ifdef USE_STM_MCU
#include "stm32l1xx_hal.h"
#else
//#include "at_uart.h"
#endif
#include "utils_ringbuff.h"

#ifdef USE_STM_MCU
#define HAL_AT_UART_IRQHandler HAL_USART1_IrqCallback
extern UART_HandleTypeDef  huart1;
static UART_HandleTypeDef *pAtUart = &huart1;
#else
#define HAL_AT_UART_IRQHandler Uart0_IRQHandler
//static M0P_UART_TypeDef *pAtUart = M0P_UART0;
#endif

extern void at_uart_init(void (*func)(uint8_t *, uint8_t));
//extern void AT_Uart_Init(void);
#ifndef USE_STM_MCU
//extern HAL_StatusTypeDef AT_UART_Transmit(M0P_UART_TypeDef* UARTx, uint8_t *pData, uint16_t Size, uint32_t Timeout);
#endif
extern void at_client_uart_rx_isr_cb(uint8_t *pdata, uint8_t len);

//#include "board.h"
/**
 * @brief This function handles AT UART global interrupt,push recv char to ringbuff.
 */
void HAL_AT_UART_IRQHandler(UART_HandleTypeDef *huart)
{
#ifdef USE_STM_MCU
	uint8_t ch;
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == SET) {
        ch = (uint8_t)READ_REG(huart->Instance->DR) & 0xFF;
        /*this callback for at_client*/
        at_client_uart_rx_isr_cb(&ch, 1);
        //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    __HAL_UART_CLEAR_PEFLAG(huart);
#else
	//uint8_t ch;
	//if(Uart_GetStatus(pAtUart, UartRC))         //UART0数据接收
    //{
    //    Uart_ClrStatus(pAtUart, UartRC);        //清中断状态位
    //    ch = Uart_ReceiveData(pAtUart);   //接收数据字节
    //    at_client_uart_rx_isr_cb(&ch, 1);
    //}
#endif
}

/**
 *pdata: pointer of data for receive
 *len:  len of data to be sent
 *return: the len of data receive success
 * @brief hal api for at data send
 */
int HAL_AT_Uart_Recv(void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout)
{

#ifdef USE_STM_MCU
    if (HAL_OK == HAL_UART_Receive(pAtUart, data, expect_size, timeout)) {
        return expect_size;
    } else {
        return 0;
    }
#endif
}

/**
 *pdata: pointer of data for send
 *len:  len of data to be sent
 *return: the len of data send success
 * @brief hal api for at data send
 */
int HAL_AT_Uart_Send(void *data, uint32_t size)
{

#ifdef USE_STM_MCU
    if (HAL_OK == HAL_UART_Transmit(pAtUart, data, size, 0xFFFF)) {
        return size;
#else
	//if (HAL_OK == AT_UART_Transmit(pAtUart, data, size, 0xFFFF)) {
    //    return size;
	
	if(at_uart_send_data(data, size)) {
		return size;
#endif
    } else {
        return 0;
    }
}

int HAL_AT_Uart_Buadrate_Set(uint8_t buadrate)
{
	//at_uart_set_buadrate(buadrate);
	return 0;
}

int HAL_AT_Uart_Init(void)
{
    //AT_Uart_Init();
	//at_uart_init(at_client_uart_rx_isr_cb);
    return QCLOUD_RET_SUCCESS;
}

int HAL_AT_Uart_Deinit(void)
{
	//at_uart_deinit();
    return QCLOUD_RET_SUCCESS;
}
#endif
