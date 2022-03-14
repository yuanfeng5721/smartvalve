/*
 * mqtt_task.h
 *
 *  Created on: 2022年3月2日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_ONENET_H_
#define APPLICATION_INC_ONENET_H_
#include <stdint.h>
#include <stdbool.h>

#define ONENET_TOPIC_UPDATE_DELTA "image/update/delta"
#define ONENET_TOPIC_DP_DATA      "dp/post/json"
#define ONENET_TOPIC_UPDATE       "image/update"


#define ONENET_UPDATE_MESSAGE     "{\"state\":{\"reported\":{%s}}}"


typedef struct {
	uint16_t type;
	bool isneed;
	union {
		float fvalue;
		uint16_t uvalue;
		char *array;
	}Data;
}Need_update_Data;


#define BIT(b) (1<<b)

typedef enum {
	SAMPLE_FREQ_FLAG = BIT(0),
	UPDATE_FREQ_FLAG = BIT(1),
	FLOW_DEFAULT_FLAG = BIT(2),
	ANGLE_DEFAULT_FLAG = BIT(3),
	DP_DATA_FLAG = BIT(3),
}Need_Update_Flag;

void init_onenet_account(void);
int onenet_subscribe_topic(void);
void onenet_process(void);

#endif /* APPLICATION_INC_ONENET_H_ */
