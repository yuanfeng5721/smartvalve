/*
 * sensors_task.h
 *
 *  Created on: 2022年1月1日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_SENSORS_TASK_H_
#define APPLICATION_INC_SENSORS_TASK_H_


void SensorsTaskInit(void);
extern osMessageQId sensorsQueueHandle;

#endif /* APPLICATION_INC_SENSORS_TASK_H_ */
