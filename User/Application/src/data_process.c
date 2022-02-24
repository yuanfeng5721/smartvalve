/*
 * data_process.c
 *
 *  Created on: 2022年1月25日
 *      Author: boboowang
 */
#define LOG_TAG "DATA_PROC"
#include <stdint.h>
#include <string.h>
#include <math.h>
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
#define IS_UPDATE_SENSOR_DATA(sample_count, sample_freq, update_freq) (((sample_count+1)%(update_freq/sample_freq)) == 0)
#define RECORD_SENSOR_ITEM(sample_count, sample_freq) (sample_count%(60/sample_freq))
#define MAX_SAMPLE_ITEM(sample_freq) (24*60/sample_freq)

osThreadId dataProcessTaskHandle;
osMessageQId dataProcessQueueHandle;
//int temperature;
//int front_press;
//int back_press;
//int battery;
//int openning;
//int battery_usm;
//int flow
//int signal;
//int overflow;

dp_t dp_data[DP_MAX_NUMBER] = {
		{.name = "battery_usm",},
		{.name = "openning"},
		{.name = "back_press",},
		{.name = "front_press",},
		{.name = "battery",},
		{.name = "temperature",},
		{.name = "flow",},
		{.name = "signal",},
		{.name = "overflow",}
};

#define D_CLIENT_ID "TEST001"
#define D_USERNAME  "448403"
#define D_PASSWD    "WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs="

static char clientid[20];
static char username[20];
static char passwd[50];
static UpdateFreq update_freq = 5;
static SampleFreq sample_freq = 5;
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
void init_mqtt_account(void)
{
	memset(clientid, 0, sizeof(clientid));
	memset(username, 0, sizeof(username));
	memset(passwd, 0, sizeof(passwd));

	strcpy(clientid, nvitem_get_string(NV_CLIENT_ID));
	strcpy(username, nvitem_get_string(NV_USERNAME));
	strcpy(passwd, nvitem_get_string(NV_PASSWORD));

	LOGD("client id: %s\r\n", clientid);
	LOGD("user name: %s\r\n", username);
	LOGD("pass word: %s\r\n", passwd);
}

void init_device_params(void)
{
	update_freq = CHECK_UPDATE_FREQ(nvitem_get_int(UPDATE_FREQ));
	sample_freq = CHECK_SAMPLE_FREQ(nvitem_get_int(SAMPLE_FREQ));

	LOGD("update_freq = %d min\r\n",update_freq);
	LOGD("sample_freq = %d min\r\n",sample_freq);
}

void sensor_data_clear(uint8_t num, uint8_t item)
{
	//for(int i=0; i<DP_MAX_NUMBER; i++) {
	if(num>=0 && num<DP_MAX_NUMBER) {
		if(item>=0 && item<STROE_MAX_NUMBER) {
			dp_data[num].dt[item].v = 0.0;
			dp_data[num].dt[item].t = 0;
			dp_data[num].dt[item].isOk = false;
		}
	}
	//}
}

void sensor_data_clear_all(void)
{
	for(int i=0; i<DP_MAX_NUMBER; i++) {
		for(int j=0; j<STROE_MAX_NUMBER; j++) {
			dp_data[i].dt[j].v = 0.0;
			dp_data[i].dt[j].t = 0;
			dp_data[i].dt[j].isOk = false;
		}
	}
	//}
}
cavan_json_t *make_report_json(void)
{
	uint8_t i = 0, j = 0;
	uint8_t need_dot = 0;

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
		for(j=0; j<STROE_MAX_NUMBER; j++)
		{
			if(dp_data[i].dt[j].isOk)
			{
				if(need_dot++){
					cavan_json_append(json, ',');
				}
				cavan_json_begin(json);
				cavan_mqtt_append_float2(json, "v", dp_data[i].dt[j].v);
				cavan_mqtt_append_int2(json, "t", dp_data[i].dt[j].t);
				cavan_json_end(json);
				sensor_data_clear(i, j);
			}
		}
		cavan_json_array_end(json);
	}
	cavan_json_end(json);
	cavan_json_end(json);
	LOGD("report(%d):%s\r\n", json->length, json->buff);
	return json;
}

int32_t calc_q(float p_b, float p_f, float angle)
{
	#define P   1  //水的密度
	//#define F   1
	int32_t dp = (p_f-p_b);
	uint32_t Q=0;
	uint8_t pos = 0;
	uint32_t F = 1000;//1000为单位
	uint32_t z = 0;
	uint8_t degree = 0;

	degree = ((angle/1.8)*10+5)/10;

	if(degree == 0)
	{
		Q = 0;
		LOGI("calc q: Q = %d,degree = %d \r\n", Q, degree);
	}
	else
	{
		pos = (degree>1)?(degree/2 - 1):0;
		z = zeta_value[pos];
		F = nvitem_get_int(NV_F);

		Q = F*sqrt(2*dp/P)/sqrt(z);
		LOGI("degree = %d, pos = %d\r\n", degree, pos);
		LOGI("sqrt(2*dp/P) = %f\r\n",sqrt(2*dp/P));
		LOGI("sqrt(zeta_value[pos] = %f\r\n",sqrt(z));

		LOGI("calc q: pf=%d, pb=%d, degree=%d, zeta=%d, Q=%d\r\n", p_f, p_b, degree, z, Q);
	}

	return Q;
}

void record_sensor_data(time_t t, uint16_t record_item)
{
	extern float sensor_data[SAMPLE_ADC_CHANNEL];
	uint8_t i = 0;
	float Q = 0, p_b = 0, p_f = 0, angle = 0;
	time_t time = LOCALTIME_TO_UNIXTIME(t, 8);

	angle = sensor_data[1];
	p_b = sensor_data[2];
	p_f = sensor_data[3];

	Q = calc_q(p_b, p_f, angle);

	for(i=0; i<SAMPLE_ADC_CHANNEL-1; i++)
	{
		dp_data[i].dt[record_item].v = sensor_data[i];
		dp_data[i].dt[record_item].t = time;
		dp_data[i].dt[record_item].isOk = true;
	}
	dp_data[i].dt[record_item].v = Q;
	dp_data[i].dt[record_item].t = time;
	dp_data[i].dt[record_item].isOk = true;
	i++;
	dp_data[i].dt[record_item].v = at_device_get_rssi();
	dp_data[i].dt[record_item].t = time;
	dp_data[i].dt[record_item].isOk = true;
}

int valve_control(uint16_t sample_count, SampleFreq sample_freq)
{
	uint16_t wakeupcount = sample_count;
	uint8_t next_degree = 0;
	uint8_t pre_degree = 0;
	float angle = 0;

	if(wakeupcount == 0)
	{
		pre_degree = angle_default_value[MAX_SAMPLE_ITEM(sample_freq) - 1];
	}
	else
	{
		pre_degree = angle_default_value[wakeupcount-1];
	}
	next_degree = angle_default_value[wakeupcount];
	LOGD("%s:wakeupcount = %d, next degree = %d, pre_degree = %d\r\n",__FUNCTION__,wakeupcount, next_degree, pre_degree);

	//open valve to degree
	if(pre_degree != next_degree){
		angle = (next_degree*90.0/100.0);
		LOGD("%s:angle = %f\r\n", __FUNCTION__, angle);
		//return moto_ctrl_for_angle(angle);
		//need control moto for angle
		return true;
	}else{
		LOGD("%s:nothing to do\r\n", __FUNCTION__);
	}

	return 0;
}

void run_process(void)
{
//	char *clientid = "TEST001";
//	char *username = "448403";
//	char *passwd = "WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs=";
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
	BootMode bootmode;
	time_t next_time;
	io_msg_t p_msg =
	{
		.type = IO_MSG_TYPE_READ_SENSOR,
		.subtype = 0x1234,
		.u.buf = "567890",
	};
	uint16_t sample_count = 0;
	//float Q = 0.0;

	// init nvitem
	init_nvitems();

	// init sensors parama
	Sensors_param_init();

	//init mqtt account
	init_mqtt_account();

	//init device params
	init_device_params();

	// check boot mode
	bootmode = device_get_bootmode();
	LOGI("--- device boot mode = %d ---\r\n", bootmode);
	if(bootmode == CONFIG_MODE) {
		LOGI("--- device into configure mode ---\r\n");
		while(1)
		{
			osDelay(2000);
		}
	} else if (bootmode == NO_ACTIVITE_MODE) {
		LOGI("--- device into low power mode ---\r\n");
		osDelay(20000);
		enter_stop_mode();
	} else {
		LOGI("--- device into normal mode ---\r\n");
		//first init modem, sync time
		if(!modem_init())
		{
		  //sync net time
		  modem_ntp(NULL);
		}

		//close modem
		modem_deinit();

		while (1) {
			next_time = SleepAndWakeUp(MIN_TO_SECONDS(sample_freq));
			osDelay(300);
			sample_count = calc_wakeup_count(MIN_TO_SECONDS(sample_freq));
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
				record_sensor_data(next_time, RECORD_SENSOR_ITEM(sample_count, sample_freq));
				//valve control
				valve_control(sample_count, sample_freq);
				//check whether update sensor data
				if(IS_UPDATE_SENSOR_DATA(sample_count, sample_freq, update_freq)) {
					LOGI("sensor sample ok, next task run\r\n");
					run_process();
				}
			}
			osDelay(100);
		}
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
