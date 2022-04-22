/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <cavan.h>
#include <cavan/json.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void _Error_Handler(uint8_t *file, uint32_t line);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define bt_link_status_Pin GPIO_PIN_13
#define bt_link_status_GPIO_Port GPIOC
#define mcu_wakeup_modem_Pin GPIO_PIN_2
#define mcu_wakeup_modem_GPIO_Port GPIOC
#define usm_fb_Pin GPIO_PIN_3
#define usm_fb_GPIO_Port GPIOC
#define temp_sda_Pin GPIO_PIN_1
#define temp_sda_GPIO_Port GPIOA
#define sensor_pwr_en2_Pin GPIO_PIN_4
#define sensor_pwr_en2_GPIO_Port GPIOC
#define mcu_light_Pin GPIO_PIN_5
#define mcu_light_GPIO_Port GPIOC
#define modem_status_Pin GPIO_PIN_0
#define modem_status_GPIO_Port GPIOB
#define modem_pwr_1v8_en_Pin GPIO_PIN_1
#define modem_pwr_1v8_en_GPIO_Port GPIOB
#define mcu_wakeup_bt_Pin GPIO_PIN_2
#define mcu_wakeup_bt_GPIO_Port GPIOB
#define modem_pwr_en_Pin GPIO_PIN_12
#define modem_pwr_en_GPIO_Port GPIOB
#define ocp_alert_Pin GPIO_PIN_13
#define ocp_alert_GPIO_Port GPIOB
#define sensor_pwr_en1_Pin GPIO_PIN_14
#define sensor_pwr_en1_GPIO_Port GPIOB
#define bt_mode_Pin GPIO_PIN_15
#define bt_mode_GPIO_Port GPIOB
#define bt_mode_EXTI_IRQn EXTI15_10_IRQn
#define bt_rst_Pin GPIO_PIN_8
#define bt_rst_GPIO_Port GPIOA
#define uart4_rts_Pin GPIO_PIN_15
#define uart4_rts_GPIO_Port GPIOA
#define modem_3v3_to_1v8_en_Pin GPIO_PIN_4
#define modem_3v3_to_1v8_en_GPIO_Port GPIOB
#define ext_wdt_feed_Pin GPIO_PIN_5
#define ext_wdt_feed_GPIO_Port GPIOB
#define modem_reset_Pin GPIO_PIN_6
#define modem_reset_GPIO_Port GPIOB
#define modem_pwr_key_Pin GPIO_PIN_7
#define modem_pwr_key_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
