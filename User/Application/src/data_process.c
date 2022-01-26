/*
 * data_process.c
 *
 *  Created on: 2022年1月25日
 *      Author: boboowang
 */
#define LOG_TAG "DATA_PROC"
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "cmsis_os.h"
#include "iot_msg.h"
#include "sensors.h"
#include "iot_event.h"
#include "mqtt.h"
#include "at_device.h"
#include "data_process.h"
#include "sensors_task.h"
#include "device_nv.h"
#include "rtc_wakeup.h"

#define DATA_PROCESS_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId dataProcessTaskHandle;
osMessageQId dataProcessQueueHandle;
//int temperature;
//int front_press;
//int back_press;
//int battery;
//int openning;
//int battery_usm;
//int overflow;
//int signal;
dp_t dp_data[DP_MAX_NUMBER] = {
		{.name = "battery_usm",},
		{.name = "openning"},
		{.name = "back_press",},
		{.name = "front_press",},
		{.name = "battery",},
		{.name = "temperature",},
		{.name = "signal",},
		{.name = "overflow",}
};

static cavan_json_t report_json;
static uint32_t msg_id = 0;

//{
//    "id": 123,
//    "dp": {
//        "temperatrue": [{
//            "v": 30,
//            "t": 1552289676
//        }],
//        "power": [{
//            "v": 4.5,
//            "t": 1552289676
//        }],
//        "status": [{
//                "v": {
//                    "color": "blue"
//                },
//                "t": 1552289677
//            },
//            {
//                "v": {
//                    "color": "red"
//                },
//                "t": 1552289678
//            }
//        ]
//    }
//}
cavan_json_t *make_report_json(void)
{
	uint8_t i = 0;
	cavan_json_t *json = &report_json;

	cavan_json_init(json);

	cavan_json_begin(json);
	cavan_json_printf(json, "\"id\":%d", msg_id);

	cavan_json_append_name(json, "dp");
	cavan_json_begin(json);
	for(i=0; i<DP_MAX_NUMBER-1; i++)
	{
		cavan_json_append_name(json, dp_data[i].name);
		cavan_json_array_begin(json);
		cavan_json_begin(json);
		cavan_mqtt_append_float2(json, "v", dp_data[i].dt[0].v);
		cavan_mqtt_append_int2(json, "t", dp_data[i].dt[0].t);
		cavan_json_end(json);
		cavan_json_array_end(json);
	}
	cavan_json_end(json);
	cavan_json_end(json);
	LOGD("report(%d):%s\r\n", json->length, json->buff);

	return json;
}

void sensor_data_clear(uint8_t item)
{
	for(int i=0; i<DP_MAX_NUMBER; i++) {
		if(item>0 && item<STROE_MAX_NUMBER) {
			dp_data[i].dt[item].v = 0.0;
			dp_data[i].dt[item].t = 0;
			dp_data[i].dt[item].isOk = true;
		}
	}
}

void record_sensor_data(time_t t)
{
	extern float sensor_data[SAMPLE_ADC_CHANNEL];
	uint8_t i = 0;
	time_t time = LOCALTIME_TO_UNIXTIME(t, 8);

	sensor_data_clear(0);
	for(i=0; i<SAMPLE_ADC_CHANNEL-1; i++)
	{
		dp_data[i].dt[0].v = sensor_data[i];
		dp_data[i].dt[0].t = time;
		dp_data[i].dt[0].isOk = true;
	}
	dp_data[i].dt[0].v = at_device_get_rssi();
	dp_data[i].dt[0].t = time;
	dp_data[i].dt[0].isOk = true;
}

void run_process(void)
{
	char *clientid = "TEST001";
	char *username = "448403";
	char *passwd = "WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs=";
	cavan_json_t *report_json = NULL;

	if(!modem_init()) {
		//send data to mqtt server
		if(!mqtt_connect(clientid, username, passwd)) {
			msg_id++;
			report_json = make_report_json();
			mqtt_publish("dp/post/json", report_json->buff, strlen(report_json->buff));
			mqtt_disconnect();
		}
	}

	//close modem
	modem_deinit();
}

void DataProcessTask(void const * argument)
{
	//osMsgStatus status;
	time_t next_time;
	io_msg_t p_msg =
	{
	.type = IO_MSG_TYPE_READ_SENSOR,
	.subtype = 0x1234,
	.u.buf = "567890",
	};

	// init nvitem
	init_nvitems();

	//first init modem, sync time
	if(!modem_init())
	{
	  //sync net time
	  modem_ntp(NULL);
	}

	//close modem
	modem_deinit();
//	  set_local_time(make_data_time(22, 1, 24, 15, 20, 0, 0));
//	  osDelay(S_TO_TICKS(5));
	make_report_json();
	while (1) {
		next_time = SleepAndWakeUp(MIN_TO_SECONDS(10));

		//send sensor sample message
		os_msg_send(sensorsQueueHandle, &p_msg, 0);
		//wait sample task complete
		if(os_event_wait(g_event_handle, IO_EVT_TYPE_SENSORS_COMPLETE, osFlagsWaitAll, S_TO_TICKS(5*60)) != IO_EVT_TYPE_SENSORS_COMPLETE)
		{
			LOGE("some task exec error\r\n");
		}
		else
		{
			//record sensor data
			record_sensor_data(next_time);
			//wait send task complete
			LOGI("sensor sample ok, next task run\r\n");
			run_process();
		}
		osDelay(100);
	}
}

void DataProcessTaskInit(void)
{
	os_msg_create(&dataProcessQueueHandle, 4, sizeof(io_msg_t));

	osThreadDef(dataProcessTask, DataProcessTask, DATA_PROCESS_TASK_PRIORITY, 0, 512);
	dataProcessTaskHandle = osThreadCreate(osThread(dataProcessTask), NULL);
}

osMessageQId DataProcessGetMessageHandle(void)
{
	return dataProcessQueueHandle;
}
