/*
 * mqtt_task.c
 *
 *  Created on: 2022年3月2日
 *      Author: boboowang
 */

#define LOG_TAG "MQTT_TASK"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "log.h"
#include "cmsis_os.h"
#include "iot_msg.h"
#include "iot_event.h"
#include "mqtt.h"
#include "at_device.h"
#include "device_nv.h"
#include "onenet.h"
#include "rtc_wakeup.h"
#include "valve_process.h"
#include "cJSON.h"
#include "utils.h"


#define ONENET_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId onenetTaskHandle;
osMessageQId onenetQueueHandle;


#define D_CLIENT_ID "TEST001"
#define D_USERNAME  "448403"
#define D_PASSWD    "WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs="

static uint8_t need_update_flag = 0;
extern cavan_json_t report_json;
extern UpdateFreq update_freq;
extern SampleFreq sample_freq;
extern uint32_t q_default_value[Q_DEFAULT_NUM];
extern uint32_t angle_default_value[ANGLE_DEFAULT_NUM];
extern dp_t dp[DP_MAX_NUMBER];

static bool recv_flag = false;

static char clientid[20];
static char username[20];
static char passwd[50];

int onenet_recv_deal_process(uint16_t type, const char *msg)
{
	return 0;
}

void StartOnenetTask(void const * argument)
{
	//osMsgStatus status;
	io_msg_t p_msg;

	while (1) {
		if(os_msg_recv(onenetQueueHandle, &p_msg, osWaitForever) == osEventMessage) {
			if(p_msg.type == IO_MSG_TYPE_MQTT_PUBLISH) {
				uint16_t type = p_msg.subtype;
				char *msg = p_msg.u.buf;
				onenet_recv_deal_process(type, msg);
			}
		}
		osDelay(100);
	}
}

void OnenetTaskInit(void)
{
	os_msg_create(&onenetQueueHandle, 4, sizeof(io_msg_t));

	osThreadDef(onenetTask, StartOnenetTask, ONENET_TASK_PRIORITY, 0, 512);
	onenetTaskHandle = osThreadCreate(osThread(onenetTask), NULL);
}

osMessageQId OnenetGetMessageHandle(void)
{
	return onenetQueueHandle;
}

void init_onenet_account(void)
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

//{
//	"state":{"sampleIntvl":5},
//	"metadata":{"sampleIntvl":{"timestamp":1646210722}},
//	"version":21,"timestamp":1646965516
//}
static char *message = "{\"state\":{\"sampParamKaidu4\":\"99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99\",\"sampleIntvl\":5,\"sampParamKaidu2\":\"250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,1312\",\"sampParamFlow\":\"939,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,3312\",\"sampParamKaidu3\":\"20,10,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99\",\"sampParamKaidu1\":\"88887,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250\"},\"metadata\":{\"sampParamKaidu4\":{\"timestamp\":1646981827},\"sampleIntvl\":{\"timestamp\":1646981827},\"sampParamKaidu2\":{\"timestamp\":1646981827},\"sampParamFlow\":{\"timestamp\":1646981827},\"sampParamKaidu3\":{\"timestamp\":1646981827},\"sampParamKaidu1\":{\"timestamp\":1646981827}},\"version\":25,\"timestamp\":1646986817}";
static void update_delta_handle(char *msg, size_t size)
{
	cJSON *jsRoot = NULL;
	cJSON *jsItem = NULL;
	cJSON *jsValue = NULL;

	LOGD("msg: %s \r\n", msg);

	jsRoot = cJSON_Parse(msg);
	if(jsRoot != NULL) {
		jsItem = cJSON_GetObjectItem(jsRoot, "state");
		if(jsItem != NULL) {
			jsValue = cJSON_GetObjectItem(jsItem, "sampleIntvl");
			if(jsValue!=NULL || jsValue->type == cJSON_Number) {
				LOGD("sampleIntvl: %d \r\n", jsValue->valueint);
				update_sample_freq(jsValue->valueint);
				need_update_flag |= SAMPLE_FREQ_FLAG;
			}
			jsValue = cJSON_GetObjectItem(jsItem, "uploadIntvl");
			if(jsValue!=NULL || jsValue->type == cJSON_Number) {
				LOGD("uploadIntvl: %d \r\n", jsValue->valueint);
				update_update_freq(jsValue->valueint);
				need_update_flag |= UPDATE_FREQ_FLAG;
			}
			jsValue = cJSON_GetObjectItem(jsItem, "sampParamFlow");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamFlow: %s \r\n", jsValue->valuestring);
				update_flow_data(jsValue->valuestring, strlen(jsValue->valuestring));
				need_update_flag |= FLOW_DEFAULT_FLAG;
			}

			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu1");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu1: %s \r\n", jsValue->valuestring);
				update_angle_data(0, jsValue->valuestring, strlen(jsValue->valuestring));
				need_update_flag |= ANGLE_DEFAULT_FLAG;
			}

			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu2");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu2: %s \r\n", jsValue->valuestring);
				update_angle_data(1, jsValue->valuestring, strlen(jsValue->valuestring));
			}

			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu3");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu3: %s \r\n", jsValue->valuestring);
				update_angle_data(2, jsValue->valuestring, strlen(jsValue->valuestring));
			}

			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu4");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu4: %s \r\n", jsValue->valuestring);
				update_angle_data(3, jsValue->valuestring, strlen(jsValue->valuestring));
			}
		}
	}
	cJSON_Delete(jsRoot);

	//os_event_set(g_event_handle, IO_EVT_TYPE_MQTT_RECV_COMPLETE);
	recv_flag = true;
}

static cavan_json_t *make_image_dp_json(void)
{
	cavan_json_t *json = &report_json;
	uint8_t need_dot = false;
	uint8_t i = 0;

	LOGI("%s \r\n", __FUNCTION__);

	cavan_json_init(json);

	cavan_json_begin(json);
	cavan_json_append_name(json, "state");
	cavan_json_begin(json);
	cavan_json_append_name(json, "reported");
	cavan_json_begin(json);

	for(i=0; i<DP_MAX_NUMBER-1; i++)
	{
		if(need_dot++){
			cavan_json_append(json, ',');
		}
		cavan_mqtt_append_float2(json, dp[i].name, dp[i].dt.v);
		dp[i].dt.v = 0;
		dp[i].dt.isOk = false;
		dp[i].dt.t = 0;
	}

	cavan_json_end(json);
	cavan_json_end(json);
	cavan_json_end(json);
	LOGD("report(%d):%s\r\n", json->length, json->buff);
	return json;

}

int onenet_subscribe_topic(void)
{
	LOGI("%s \r\n", __FUNCTION__);

	if(mqtt_subscribe(ONENET_TOPIC_UPDATE_DELTA, update_delta_handle) < 0) {
		LOGE("onenet subscribe %s fail!! \r\n", ONENET_TOPIC_UPDATE_DELTA);
		return -1;
	}
	return 0;
}

static int onenet_publish_last_dp(void)
{
	cavan_json_t *json = NULL;

	LOGI("%s \r\n", __FUNCTION__);

	//snprintf(message, sizeof(message), ONENET_UPDATE_MESSAGE, "");
	json = make_image_dp_json();
	//if(mqtt_publish(ONENET_TOPIC_UPDATE, message, strlen(message)) < 0) {
	if(mqtt_publish(ONENET_TOPIC_UPDATE, json->buff, strlen(json->buff)) < 0) {
		LOGE("onenet publish %s fail!! \r\n", json->buff);
		return -1;
	}
}

static int onenet_trigger_delta(void)
{
	char message[64] = {0};
	cavan_json_t *json = NULL;
	need_update_flag = 0;

	LOGI("%s \r\n", __FUNCTION__);
	recv_flag = false;

	snprintf(message, sizeof(message), ONENET_UPDATE_MESSAGE, "");
	if(mqtt_publish(ONENET_TOPIC_UPDATE, message, strlen(message)) < 0) {
		LOGE("onenet trigger delta %s fail!! \r\n", message);
		return -1;
	}
	return 0;
}

int onenet_publish_dp(void)
{
	cavan_json_t *report_json = NULL;

	report_json = make_dp_report_json();
	if(mqtt_publish(ONENET_TOPIC_DP_DATA, report_json->buff, strlen(report_json->buff)) < 0) {
		return -1;
	}
	return 0;
}

char temp[512];

static cavan_json_t *make_image_json(void)
{
	cavan_json_t *json = &report_json;
//	bool need_dot = false;

	LOGI("%s \r\n", __FUNCTION__);
	cavan_json_init(json);

	cavan_json_begin(json);
	cavan_json_append_name(json, "state");
	cavan_json_begin(json);
	cavan_json_append_name(json, "reported");
	cavan_json_begin(json);

	if(need_update_flag & SAMPLE_FREQ_FLAG) {
		cavan_mqtt_append_int2(json, "sampleIntvl", sample_freq);
//		need_dot = true;
	}
	if(need_update_flag & UPDATE_FREQ_FLAG) {
//		if(need_dot){
//			cavan_json_append(json, ',');
//		}
		cavan_mqtt_append_int2(json, "uploadIntvl", update_freq);
	}
	if(need_update_flag & FLOW_DEFAULT_FLAG) {
//		if(need_dot){
//			cavan_json_append(json, ',');
//		}
		memset(temp, 0, sizeof(temp));
		array_to_string(temp, q_default_value, Q_DEFAULT_NUM);
		cavan_mqtt_append_text2(json, "sampParamFlow", temp);
	}
	if(need_update_flag & ANGLE_DEFAULT_FLAG) {
		uint32_t *ptr = angle_default_value;
//		if(need_dot){
//			cavan_json_append(json, ',');
//		}
		memset(temp, 0, sizeof(temp));
		array_to_string(temp, ptr, ANGLE_NUM_QUARTER);
		cavan_mqtt_append_text2(json, "sampParamKaidu1", temp);
		//cavan_json_append(json, ',');
		memset(temp, 0, sizeof(temp));
		array_to_string(temp, ptr+ANGLE_NUM_QUARTER, ANGLE_NUM_QUARTER);
		cavan_mqtt_append_text2(json, "sampParamKaidu2", temp);
		//cavan_json_append(json, ',');
		memset(temp, 0, sizeof(temp));
		array_to_string(temp, ptr+ANGLE_NUM_QUARTER*2, ANGLE_NUM_QUARTER);
		cavan_mqtt_append_text2(json, "sampParamKaidu3", temp);
		//cavan_json_append(json, ',');
		memset(temp, 0, sizeof(temp));
		array_to_string(temp, ptr+ANGLE_NUM_QUARTER*3, ANGLE_NUM_QUARTER);
		cavan_mqtt_append_text2(json, "sampParamKaidu4", temp);
	}

	cavan_json_end(json);
	cavan_json_end(json);
	cavan_json_end(json);
	LOGD("report(%d):%s\r\n", json->length, json->buff);
	return json;
}

int onenet_publish_device_image(void)
{
	cavan_json_t *report_json = NULL;
	uint32_t old_time,new_time;

	LOGI("%s \r\n", __FUNCTION__);

	old_time = new_time = get_ticks();

	//if(os_event_wait(g_event_handle, IO_EVT_TYPE_MQTT_RECV_COMPLETE, osFlagsWaitAll, S_TO_TICKS(60)) != IO_EVT_TYPE_MQTT_RECV_COMPLETE)
	while(!recv_flag)
	{
		if((new_time-old_time) > 15*1000)
		{
			LOGE("mqtt recv timeout\r\n");
			break;
		}
		osDelay(100);
		new_time = get_ticks();
	}

	if(recv_flag)
	{
		report_json = make_image_json();
		if(mqtt_publish(ONENET_TOPIC_UPDATE, report_json->buff, strlen(report_json->buff)) < 0) {
			return -1;
		}
	}
	return 0;
}

int save_update_params(void)
{
	LOGI("%s \r\n", __FUNCTION__);
	if(need_update_flag & SAMPLE_FREQ_FLAG) {
		nvitem_set_int(SAMPLE_FREQ, sample_freq);
	}
	if(need_update_flag & UPDATE_FREQ_FLAG) {
		nvitem_set_int(UPDATE_FREQ, update_freq);
	}
	if(need_update_flag & FLOW_DEFAULT_FLAG) {
		nvitem_set_array(Q_DEFAULT_KEY, q_default_value, Q_DEFAULT_NUM);
	}
	if(need_update_flag & ANGLE_DEFAULT_FLAG) {
		nvitem_set_array(ANGLE_DEFAULT_KEY, angle_default_value, ANGLE_DEFAULT_NUM);
	}
}

void onenet_process(void)
{
	if(!modem_init()) {
		//send data to mqtt server
		if(!mqtt_connect(clientid, username, passwd)) {
			//pub dp
			onenet_publish_dp();

			//pub last dp to device images
			onenet_publish_last_dp();

			//sub topic
			onenet_subscribe_topic();

			//get delta
			onenet_trigger_delta();

			//update need propertys
			onenet_publish_device_image();

			//disconnect mqtt server
			mqtt_disconnect();
		}
	}
	//close modem
	modem_deinit();

	//save need update param
	save_update_params();
}

void test_json(void)
{
	need_update_flag = 0xf;
	make_image_json();
}
