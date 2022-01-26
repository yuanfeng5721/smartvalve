/*
 * data_process.h
 *
 *  Created on: 2022年1月25日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_DATA_PROCESS_H_
#define APPLICATION_INC_DATA_PROCESS_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "mqtt.h"

#define DP_MAX_NUMBER       8
#define STROE_MAX_NUMBER    12

typedef struct {
	float v;
	time_t t;
	bool isOk;
}d_t;

typedef struct {
	const char *name;
	d_t dt[STROE_MAX_NUMBER];
}dp_t;

cavan_json_t *make_report_json(void);

void DataProcessTaskInit(void);
#endif /* APPLICATION_INC_DATA_PROCESS_H_ */
