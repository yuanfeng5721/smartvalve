/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#define TIME_CLOCK_FREQ  32000000

#define MOTO_MAX_FREQ 39000
#define MOTO_MIN_FREQ 37000

#define MOTO_MAX_FREQ2 42000
#define MOTO_MIN_FREQ2 44000

#define MOTO_FREQ_STEP 500

#define CHECK_FREQ(f) (f<MOTO_MIN_FREQ)?MOTO_MIN_FREQ:((f>MOTO_MAX_FREQ)?MOTO_MAX_FREQ:f)
#define CALC_PERIOD(freq) ((TIME_CLOCK_FREQ>>1)/CHECK_FREQ(freq)-2)

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM6_Init(void);
void MX_TIM7_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */
void freqset(u16 freq);
void MX_TIM3_MOTO_Init(uint32_t freq , bool direct);
void MX_TIM3_Stop(void);
void MX_TIM3_Start(void);

void MX_Encoder_Start(void);
void MX_Encoder_Stop(void);
void MX_Encoder_Set_Count(uint16_t count);
uint16_t MX_Encoder_Get_Count(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

