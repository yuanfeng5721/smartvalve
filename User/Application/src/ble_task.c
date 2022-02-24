/*
 * ble_task.c
 *
 *  Created on: 2022年2月15日
 *      Author: boboowang
 */
#define LOG_TAG   "BLETSK"
#include "log.h"
#include "cmsis_os.h"
#include "iot_msg.h"
#include "iot_event.h"
#include "ble.h"


#define BLE_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId bleTaskHandle;
osMessageQId bleQueueHandle;


void StartBleTask(void const * argument)
{
	//osMsgStatus status;
	//io_msg_t p_msg;

	ble_hardware_init();

	while (1) {
		osDelay(1000);
	}
}

void BleTaskInit(void)
{
	os_msg_create(&bleQueueHandle, 4, sizeof(io_msg_t));

	osThreadDef(bleTask, StartBleTask, BLE_TASK_PRIORITY, 0, 512);
	bleTaskHandle = osThreadCreate(osThread(bleTask), NULL);
}
