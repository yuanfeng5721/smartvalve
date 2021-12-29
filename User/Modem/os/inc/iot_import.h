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

#ifndef QCLOUD_IOT_IMPORT_H_
#define QCLOUD_IOT_IMPORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <iot_config.h>
#include "platform.h"
#include "iot_error.h"
#include "iot.h"
//#include "at_uart.h"

#define _IN_ /* indicate an input parameter */
#define _OU_ /* indicate a output parameter */

#define IOT_TRUE  (1) /* indicate boolean value true */
#define IOT_FALSE (0) /* indicate boolean value false */

#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))

/**********************************************************************
 * QCloud IoT C-SDK Hardware Abstraction Layer
 * Platform/OS/IP stack/SSL dependant functions
 * Check platform folder for reference implementaions
 * Require porting when adapt SDK to new platform/OS
 *********************************************************************/

typedef void (*ThreadRunFunc)(void *arg);

typedef struct ThreadParams {
    char *        thread_name;
#ifndef PLATFORM_HAS_CMSIS
	void *        thread_id;
#else
    uint32_t      thread_id;
#endif
    ThreadRunFunc thread_func;
    void *        user_arg;
    uint16_t      priority;
    uint32_t      stack_size;
} ThreadParams;

/**
 * @brief Create a thread/task
 *
 * @param params    thread parameters
 * @return 0 when success, or error code otherwise
 */
int HAL_ThreadCreate(ThreadParams *params);

/**
 * @brief delete a thread/task
 *
 * @param params    thread parameters
 * @return 0 when success, or error code otherwise
 */
int HAL_ThreadDelete(ThreadParams *params);
/**
 * @brief create semaphore
 *
 * @return a valid semaphore handle when success, or NULL otherwise
 */
void *HAL_SemaphoreCreate(void);

/**
 * @brief Destroy semaphore
 * @param sem   semaphore handle
 */
void HAL_SemaphoreDestroy(void *sem);

/**
 * @brief Post semaphore
 * @param sem   semaphore handle
 */
void HAL_SemaphorePost(void *sem);

/**
 * @brief Wait for semaphore
 * @param sem           semaphore handle
 * @param timeout_ms    waiting timeout value (unit: ms)
 * @return QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms);

/**
 * @brief Create mutex
 *
 * @return a valid mutex handle when success, or NULL otherwise
 */
void *HAL_MutexCreate(void);

/**
 * @brief Destroy mutex
 *
 * @param mutex     mutex handle
 */
void HAL_MutexDestroy(_IN_ void *mutex);

/**
 * @brief Lock a mutex in blocking way
 *
 * @param mutex     mutex handle
 */
void HAL_MutexLock(_IN_ void *mutex);

/**
 * @brief Lock a mutex in non-blocking way
 *
 * @param mutex     mutex handle
 * @return 0 for success, or err code for failure
 */
int HAL_MutexTryLock(_IN_ void *mutex);

/**
 * @brief Unlock/release mutex
 *
 * @param mutex     mutex handle
 */
void HAL_MutexUnlock(_IN_ void *mutex);

/**
 * @brief Malloc memory
 *
 * @param size   Expected memory size (unit: byte)
 * @return       pointer to the memory
 */
void *HAL_Malloc(_IN_ uint32_t size);

/**
 * @brief Free memory
 *
 * @param ptr   pointer to the pre-malloc memory
 */
void HAL_Free(_IN_ void *ptr);

/**
 * @brief Print data to console in format
 *
 * @param fmt   print format
 * @param ...   variable number of arguments
 */
void HAL_Printf(_IN_ const char *fmt, ...);

/**
 * @brief Print data to string in format
 *
 * @param str   destination string
 * @param len   Max size of the output
 * @param fmt   print format
 * @param ...   variable number of arguments
 * @return      number of bytes that print successfull
 */
int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...);

/**
 Print data to string in format
  *
  * @param str   destination string
  * @param len   Max size of the output
  * @param fmt   print format
  * @param ap    arguments list
  * @return      number of bytes that print successfull

 */
int HAL_Vsnprintf(_OU_ char *str, _IN_ const int len, _IN_ const char *fmt, _IN_ va_list ap);

/**
 * @brief Get timestamp in millisecond
 *
 * @return   timestamp in millisecond
 */
uint32_t HAL_GetTimeMs(void);

/**
 * @brief Delay operation in blocking way
 *
 * @param ms sleep interval in millisecond
 */
void HAL_DelayMs(_IN_ uint32_t ms);

/**
 * @brief Sleep for a while
 *
 * @param ms sleep interval in millisecond
 */
void HAL_SleepMs(_IN_ uint32_t ms);

/**
 * @brief Set device info to NVS(flash/files)
 *
 * @param pdevInfo reference to device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_SetDevInfo(void *pdevInfo);

/**
 * @brief Get device info from NVS(flash/files)
 *
 * @param pdevInfo reference to device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_GetDevInfo(void *pdevInfo);

/**
 * @brief Get device info from a JSON file
 *
 * @param file_name JSON file path
 * @param pdevInfo reference to device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_GetDevInfoFromFile(const char *file_name, void *dev_info);

#ifdef GATEWAY_ENABLED
/**
 * @brief Get gateway device info from NVS(flash/files)
 *
 * @param pgwDeviceInfo reference to gateway device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_GetGwDevInfo(void *pgwDeviceInfo);
#endif

/**
 * @brief Set the name of file which contain device info
 *
 * @param file_name the name of file which contain device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_SetDevInfoFile(const char *file_name);

/**
 * Define timer structure, platform dependant
 */
struct Timer {
#if defined(__linux__) && defined(__GLIBC__)
    struct timeval end_time;
#else
    uintptr_t end_time;
#endif
};

typedef struct Timer Timer;

/**
 * @brief Check if timer expires or not
 *
 * @param timer     reference to timer
 * @return          true = expired, false = not expired yet
 */
bool HAL_Timer_expired(Timer *timer);

/**
 * @brief Set the countdown/expired value for the timer
 *
 * @param timer         reference to timer
 * @param timeout_ms    countdown/expired value (unit: millisecond)
 */
void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms);

/**
 * @brief Set the countdown/expired value for the timer
 *
 * @param timer         reference to timer
 * @param timeout       countdown/expired value (unit: second)
 */
void HAL_Timer_countdown(Timer *timer, unsigned int timeout);

/**
 * @brief Check the remain time of the timer
 *
 * @param timer     reference to timer
 * @return          0 if expired, or the left time in millisecond
 */
int HAL_Timer_remain(Timer *timer);

/**
 * @brief Init the timer
 *
 * @param timer reference to timer
 */
void HAL_Timer_init(Timer *timer);

#define TIME_FORMAT_STR_LEN (20)
/**
 * @brief Get local time in format: %y-%m-%d %H:%M:%S
 *
 * @return string of formatted time
 */
char *HAL_Timer_current(char *time_str);

/**
 * @brief Get timestamp in second
 *
 * @return   timestamp in second
 */
long HAL_Timer_current_sec(void);

/**
 * @brief Set timestamp in second to systime/rtc
 *
 * @return   0 is success other failed
 */
int HAL_Timer_set_systime_sec(size_t timestamp_sec);

/**
 * @brief Set timestamp in millsecond to systime/rtc
 *
 * @return   0 is success other failed
 */
int HAL_Timer_set_systime_ms(size_t timestamp_ms);

#ifdef AT_TCP_ENABLED
int       HAL_AT_TCP_Init(void);
uintptr_t HAL_AT_TCP_Connect(const char *host, uint16_t port);
int       HAL_AT_TCP_Disconnect(uintptr_t fd);
int HAL_AT_TCP_Write(uintptr_t fd, const unsigned char *buf, uint32_t len, uint32_t timeout_ms, size_t *written_len);
int HAL_AT_TCP_Read(uintptr_t fd, uint8_t *buf, uint32_t len, uint32_t timeout_ms, uint32_t *read_len);
int at_device_init(void);
int at_device_deinit(void);
int HAL_AT_Uart_Buadrate_Set(uint8_t buadrate);
int HAL_AT_Uart_Init(void);
int HAL_AT_Uart_Deinit(void);
int HAL_AT_Uart_Send(void *data, uint32_t size);
int HAL_AT_Uart_Recv(void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout);
#endif


#if defined(__cplusplus)
}
#endif
#endif /* QCLOUD_IOT_IMPORT_H_ */
