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
#include "valve_process.h"
#include "cJSON.h"


#define ONENET_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId onenetTaskHandle;
osMessageQId onenetQueueHandle;


#define D_CLIENT_ID "TEST001"
#define D_USERNAME  "448403"
#define D_PASSWD    "WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs="

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
			}
			jsValue = cJSON_GetObjectItem(jsItem, "uploadIntvl");
			if(jsValue!=NULL || jsValue->type == cJSON_Number) {
				LOGD("uploadIntvl: %d \r\n", jsValue->valueint);
				update_update_freq(jsValue->valueint);
			}
			jsValue = cJSON_GetObjectItem(jsItem, "sampParamFlow");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamFlow: %s \r\n", jsValue->valuestring);
				update_flow_data(jsValue->valuestring, strlen(jsValue->valuestring));
			}
			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu4");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu4: %s \r\n", jsValue->valuestring);
				update_angle_data(3, jsValue->valuestring, strlen(jsValue->valuestring));
			}
			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu3");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu3: %s \r\n", jsValue->valuestring);
				update_angle_data(2, jsValue->valuestring, strlen(jsValue->valuestring));
			}
			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu2");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu2: %s \r\n", jsValue->valuestring);
				update_angle_data(1, jsValue->valuestring, strlen(jsValue->valuestring));
			}
			jsValue = cJSON_GetObjectItem(jsItem, "sampParamKaidu1");
			if(jsValue!=NULL || jsValue->type == cJSON_String) {
				LOGD("sampParamKaidu1: %s \r\n", jsValue->valuestring);
				update_angle_data(0, jsValue->valuestring, strlen(jsValue->valuestring));
			}
		}
	}
	cJSON_Delete(jsRoot);
}

void test_json(void)
{
	update_delta_handle(message, strlen(message));
}

static int onenet_trigger_delta(void)
{
	char message[64] = {0};

	if(mqtt_subscribe(ONENET_TOPIC_UPDATE_DELTA, update_delta_handle) < 0) {
		LOGE("onenet subscribe %s fail!! \r\n", ONENET_TOPIC_UPDATE_DELTA);
		return -1;
	}
	snprintf(message, sizeof(message), ONENET_UPDATE_MESSAGE, "");
	if(mqtt_publish(ONENET_TOPIC_UPDATE, message, strlen(message)) < 0) {
		LOGE("onenet publish %s fail!! \r\n", message);
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

int onenet_publish_device_image(void)
{
	return 0;
}

void onenet_process(void)
{
	if(!modem_init()) {
		//send data to mqtt server
		if(!mqtt_connect(clientid, username, passwd)) {
			//pub dp
			onenet_publish_dp();
			//get delta
			onenet_trigger_delta();

			//update need propertys

			//disconnect mqtt server
			mqtt_disconnect();
		}
	}
	//close modem
	modem_deinit();
}
