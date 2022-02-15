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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/semphr.h"
//#include "freertos/task.h"
#include "iot_import.h"
#include "usart.h"

#define PLATFORM_HAS_CMSIS

#ifdef PLATFORM_HAS_CMSIS
#include "cmsis_os.h"
#include "stm32l1xx_hal.h"
#else
#include "os_sync.h"
#include "os_sched.h"
#include "os_mem.h"
#include "os_task.h"
#endif

#ifdef DEBUG_USED_CMD_UART
extern void cmd_uart_print(const char *fmt, ...);
#endif

// TODO platform dependant
void HAL_SleepMs(_IN_ uint32_t ms)
{
	osDelay(ms);
    return;
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
#if 1
	static char buff[512];
	va_list list;
	int length;
	
	va_start(list, fmt);
	length = vsnprintf(buff, sizeof(buff), fmt, list);
	va_end(list);

	if (length < sizeof(buff)) {
		buff[length] = '\n';
		length++;
	}

#if 1
	HAL_UART_Transmit(&huart5, (uint8_t *)buff, length, 0xFFFF);
#else
	HAL_GPIO_WritePin(uart4_rts_GPIO_Port, uart4_rts_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit(&huart4, buff, length, length * 10);
#endif
#else
	va_list list;

	va_start(list, fmt);
	printf(fmt, list);
	va_end(list);
#endif
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return pvPortMalloc(size);
	//return os_mem_zalloc(RAM_TYPE_DATA_ON, size);
}

void HAL_Free(_IN_ void *ptr)
{
	vPortFree(ptr);
}

void *HAL_MutexCreate(void)
{
#ifdef MULTITHREAD_ENABLED
	osMutexId iot_mutex;
	osMutexDef(iot);

	// Create a mutex.
	iot_mutex = osMutexCreate (osMutex(iot));

	if(iot_mutex == NULL)
	{
		HAL_Printf("%s: HAL_MutexCreate failed\n", __FUNCTION__);
		return NULL;
	}
	return iot_mutex;
#else
    return (void *)0xFFFFFFFF;
#endif
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
	osMutexDelete(mutex);
#else
    return;
#endif
}

void HAL_MutexLock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return;
    }

    if (osMutexWait (mutex, osWaitForever) != osOK) {
        HAL_Printf("%s: HAL_MutexLock failed\n", __FUNCTION__);
        return;
    }
#else
    return;
#endif
}

int HAL_MutexTryLock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return -1;
    }

    if (osMutexWait(mutex, 0) != osOK) {
        HAL_Printf("%s: HAL_MutexTryLock failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
#else
    return 0;
#endif
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return;
    }

    if (osMutexRelease(mutex) != osOK) {
        HAL_Printf("%s: HAL_MutexUnlock failed\n", __FUNCTION__);
        return;
    }
#else
    return;
#endif
}

#ifdef MULTITHREAD_ENABLED

// platform-dependant thread routine/entry function
static void _HAL_thread_func_wrapper_(void *ptr)
{
    ThreadParams *params = (ThreadParams *)ptr;

    params->thread_func(params->user_arg);

    osThreadTerminate(NULL);
}

// platform-dependant thread create function
int HAL_ThreadCreate(ThreadParams *params)
{
	osThreadDef_t os_thread_iot;

    if (params == NULL)
        return QCLOUD_ERR_INVAL;

    if (params->thread_name == NULL) {
        HAL_Printf("thread name is required for FreeRTOS platform!\n");
        return QCLOUD_ERR_INVAL;
    }

    os_thread_iot.name = params->thread_name;
    os_thread_iot.pthread = _HAL_thread_func_wrapper_;
    os_thread_iot.tpriority = params->priority;
    os_thread_iot.stacksize = params->stack_size;
    os_thread_iot.instances = 0;

    params->thread_id = osThreadCreate (&os_thread_iot, (void *)params);
    if(params->thread_id == NULL)
    {
		HAL_Printf("%s: xTaskCreate failed\n", __FUNCTION__);
		return QCLOUD_ERR_FAILURE;
    }

    return QCLOUD_RET_SUCCESS;
}

int HAL_ThreadDelete(ThreadParams *params)
{
    if (params == NULL)
        return QCLOUD_ERR_INVAL;

    if (params->thread_id == NULL) {
        HAL_Printf("thread id is required for FreeRTOS platform!\n");
        return QCLOUD_ERR_INVAL;
    }

	if(osThreadTerminate (params->thread_id) != osOK)
	{
		HAL_Printf("%s: xTaskDelete failed\n", __FUNCTION__);
        return QCLOUD_ERR_FAILURE;
	}

    return QCLOUD_RET_SUCCESS;
}
#endif

//#if defined(PLATFORM_HAS_CMSIS) && defined(AT_TCP_ENABLED)
#if defined(AT_OS_USED)
void *HAL_SemaphoreCreate(void)
{
	osSemaphoreDef(iot);
	osSemaphoreId p_sem;
	
	p_sem = osSemaphoreCreate (osSemaphore(iot), 1);
	if (p_sem)
		return p_sem;
	else
		return NULL;
}

void HAL_SemaphoreDestroy(void *sem)
{
	osSemaphoreDelete(sem);
}

void HAL_SemaphorePost(void *sem)
{
	if (osSemaphoreRelease(sem) != osOK)
	{
		HAL_Printf("HAL_SemaphorePost err\n\r");
	}
}

int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
	return (osSemaphoreWait(sem, timeout_ms) == osOK)? 0 : -1;
}
#endif
