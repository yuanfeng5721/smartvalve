/**
  ******************************************************************************
  * @file    log.c
  * @brief   This file provides code for the configuration
  *          of the log instances.
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

/* Includes ------------------------------------------------------------------*/
#include <log.h>
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

uint8_t g_current_dbg_level = LOG_INFO;

void log_init(void)
{
	MX_UART5_Init();
}

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart5, (uint8_t*)&ch,1,HAL_MAX_DELAY);
    return ch;
}
