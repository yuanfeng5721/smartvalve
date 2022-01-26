/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "iwdg.h"
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define LOG_TAG "freertos"
#include "log.h"
#include "iot_msg.h"
#include "iot_event.h"
#include "at_device.h"
#include "usart.h"
#include "sensors_task.h"
#include "device_nv.h"
#include "rtc_wakeup.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId modemTaskHandle;
osThreadId bleTaskHandle;
osThreadId rs485TaskHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}

__weak void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}

void StartModemTask(void const * argument)
{
	//modem_init();

	while (1) {

		osDelay(2000);
	}
}

u16 CavanUartRead(UART_HandleTypeDef *huart, u8 *buff, u16 size, u32 timeout)
{
	u8 *buff_bak, *buff_end;
	u32 ticks;

	/* Check that a Rx process is not already ongoing */
	if (huart->RxState != HAL_UART_STATE_READY) {
		return 0;
	}

	/* Process Locked */
	__HAL_LOCK(huart);

	huart->ErrorCode = HAL_UART_ERROR_NONE;
	huart->RxState = HAL_UART_STATE_BUSY_RX;
	huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

	huart->RxXferSize = size;
	huart->RxXferCount = size;

	/* Process Unlocked */
	__HAL_UNLOCK(huart);

	buff_bak = buff;
	buff_end = buff + size;
	ticks = HAL_GetTick() + timeout;

	while (buff < buff_end) {
		if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != 0) {
			ticks = HAL_GetTick() + timeout;
			*buff++ = huart->Instance->DR;
		} else if (buff == buff_bak || ticks > HAL_GetTick()) {
			osThreadYield();
		} else {
			break;
		}
	}

    huart->RxState = HAL_UART_STATE_READY;

	return buff - buff_bak;
}

void StartBleTask(void const * argument)
{
	u8 buff[512];

	while (1) {
		u16 length = CavanUartRead(&huart2, buff, sizeof(buff), 20);
		if (length > 0) {
			HAL_UART_Transmit(&huart2, buff, length, length * 10);
		}
	}
}

void StartRs485Task(void const * argument)
{
	u8 buff[512];

	HAL_GPIO_WritePin(uart4_rts_GPIO_Port, uart4_rts_Pin, GPIO_PIN_RESET);

	while (1) {
		u16 length = CavanUartRead(&huart4, buff, sizeof(buff), 20);
		if (length > 0) {
			HAL_GPIO_WritePin(uart4_rts_GPIO_Port, uart4_rts_Pin, GPIO_PIN_SET);
			HAL_UART_Transmit(&huart4, buff, length, length * 10);
			HAL_GPIO_WritePin(uart4_rts_GPIO_Port, uart4_rts_Pin, GPIO_PIN_RESET);
		}
	}
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityAboveNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  osThreadDef(modemTask, StartModemTask, osPriorityNormal, 0, 512);
  modemTaskHandle = osThreadCreate(osThread(modemTask), NULL);

  osThreadDef(bleTask, StartBleTask, osPriorityNormal, 0, 256);
  bleTaskHandle = osThreadCreate(osThread(bleTask), NULL);

  osThreadDef(rs485Task, StartRs485Task, osPriorityNormal, 0, 256);
  rs485TaskHandle = osThreadCreate(osThread(rs485Task), NULL);

  globle_event_init();

  SensorsTaskInit();
  DataProcessTaskInit();
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for(;;)
  {
	//check system task, if idle into sleep
	osDelay(100);
	HAL_IWDG_Refresh(&hiwdg);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

