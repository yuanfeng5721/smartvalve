/**
  ******************************************************************************
  * @file    log.h
  * @brief   This file contains all the function prototypes for
  *          the log.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#define LOG_OFF             0 // close all log
#define LOG_ERROR           1
#define LOG_WARN            2
#define LOG_DEBUG           3
#define LOG_INFO            4


#ifndef LOG_TAG
#define LOG_TAG ""
#endif

#define LOG(level, format, args...) \
		{\
			extern uint8_t g_current_dbg_level;\
			if(level<=g_current_dbg_level)\
			{\
				printf("%s: " format, LOG_TAG, ##args);\
			}\
		}\

#define LOGE(format, args...) LOG(LOG_ERROR, format, ##args);
#define LOGW(format, args...) LOG(LOG_WARN,  format, ##args);
#define LOGD(format, args...) LOG(LOG_DEBUG, format, ##args);
#define LOGI(format, args...) LOG(LOG_INFO,  format, ##args);

#define LOG_RAW(format, args...) \
		{\
			printf(format, ##args);\
		}\

void log_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
