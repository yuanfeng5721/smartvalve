/*
 * sensors_task.c
 *
 *  Created on: 2022年1月1日
 *      Author: boboowang
 */

#include "log.h"
#include "cmsis_os.h"
#include "sensors.h"
#include "sensors_task.h"

#define SENSORS_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId sensorsTaskHandle;
osMessageQId sensorsQueueHandle;


void StartSensorsTask(void const * argument)
{
	osEvent event;
	sensors_sample_t *sensors_data = NULL;
	//Sensors_Power(true);
	while (1) {

//		event = osMessageGet (sensorsQueueHandle, osWaitForever);
//		if()
		Sensors_Power(true);
		osDelay(1000);
		sensors_data = Sensors_Sample_Data();
		//Sensors_Power(false);
		osDelay(9000);
	}
}

void Sensors_Task_Init(void)
{
//	osMessageQDef(sensorsQueue, 16, uint16_t);
//	sensorsQueueHandle = osMessageCreate(osMessageQ(sensorsQueue), NULL);


	osThreadDef(sensorsTask, StartSensorsTask, SENSORS_TASK_PRIORITY, 0, 256);
	sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);
}

