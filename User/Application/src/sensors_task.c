/*
 * sensors_task.c
 *
 *  Created on: 2022年1月1日
 *      Author: boboowang
 */

#include "log.h"
#include "cmsis_os.h"
#include "iot_msg.h"
#include "sensors.h"
#include "sensors_task.h"
#include "iot_event.h"


#define SENSORS_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId sensorsTaskHandle;
osMessageQId sensorsQueueHandle;


void StartSensorsTask(void const * argument)
{
	osMsgStatus status;
	sensors_sample_t *sensors_data = NULL;
	io_msg_t p_msg;
	//Sensors_Power(true);
	while (1) {
		if(os_msg_recv(sensorsQueueHandle, &p_msg, osWaitForever) == osEventMessage) {
			if(p_msg.type == IO_MSG_TYPE_READ_SENSOR) {
				Sensors_Power(true);
				osDelay(1000);
				sensors_data = Sensors_Sample_Data();
				Sensors_Power(false);
				os_event_set(g_event_handle, IO_EVT_TYPE_SENSORS_COMPLETE);
			}
		}
		osDelay(1000);
	}
}

void SensorsTaskInit(void)
{
	os_msg_create(&sensorsQueueHandle, 4, sizeof(io_msg_t));

	osThreadDef(sensorsTask, StartSensorsTask, SENSORS_TASK_PRIORITY, 0, 256);
	sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);
}

osMessageQId SensorsGetMessageHandle(void)
{
	return sensorsQueueHandle;
}
