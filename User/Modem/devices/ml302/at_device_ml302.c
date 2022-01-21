/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifdef AT_DEVICE_ML302
#include "at_device_ml302.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cavan/mqtt.h>

#include "at_client.h"
#include "at_socket_inf.h"
#include "iot.h"
#include "iot_import.h"
#include "utils_param_check.h"
#include "rtc.h"
#include "log.h"
#include "utils_timer.h"
#include "rtc_wakeup.h"

//#define USING_RTC

#define MQTT_CONN		0
#define AUTO_ACTIVE		1

#undef DEVICE_NAME
#define DEVICE_NAME    "ML302"
#define NET_CONN_OK_FLAG (1 << 0)
#define NET_CONN_FAIL_FLAG (1 << 1)
#define SEND_OK_FLAG   (1 << 2)
#define SEND_FAIL_FLAG (1 << 3)
#define MQTT_OK_FLAG   (1 << 4)
#define MQTT_FAIL_FLAG (1 << 5)
#define NET_INPUT_FLAG (1 << 6)

volatile uint8_t sg_SocketBitMap = 0;

static at_evt_cb_t at_evt_cb_table[] = {
    [AT_SOCKET_EVT_RECV]   = NULL,
    [AT_SOCKET_EVT_CLOSED] = NULL,
};

static char g_IMEI[IMEI_MAX_LEN] = {0};
static int ml302_rssi;
static int ml302_ber;
static u32 ml302_msgid;
static cavan_json_t ml302_send_json;

static const char mqtt_cert[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIDOzCCAiOgAwIBAgIJAPCCNfxANtVEMA0GCSqGSIb3DQEBCwUAMDQxCzAJBgNV\n"
	"BAYTAkNOMQ4wDAYDVQQKDAVDTUlPVDEVMBMGA1UEAwwMT25lTkVUIE1RVFRTMB4X\n"
	"DTE5MDUyOTAxMDkyOFoXDTQ5MDUyMTAxMDkyOFowNDELMAkGA1UEBhMCQ04xDjAM\n"
	"BgNVBAoMBUNNSU9UMRUwEwYDVQQDDAxPbmVORVQgTVFUVFMwggEiMA0GCSqGSIb3\n"
	"DQEBAQUAA4IBDwAwggEKAoIBAQC/VvJ6lGWfy9PKdXKBdzY83OERB35AJhu+9jkx\n"
	"5d4SOtZScTe93Xw9TSVRKrFwu5muGgPusyAlbQnFlZoTJBZY/745MG6aeli6plpR\n"
	"r93G6qVN5VLoXAkvqKslLZlj6wXy70/e0GC0oMFzqSP0AY74icANk8dUFB2Q8usS\n"
	"UseRafNBcYfqACzF/Wa+Fu/upBGwtl7wDLYZdCm3KNjZZZstvVB5DWGnqNX9HkTl\n"
	"U9NBMS/7yph3XYU3mJqUZxryb8pHLVHazarNRppx1aoNroi+5/t3Fx/gEa6a5PoP\n"
	"ouH35DbykmzvVE67GUGpAfZZtEFE1e0E/6IB84PE00llvy3pAgMBAAGjUDBOMB0G\n"
	"A1UdDgQWBBTTi/q1F2iabqlS7yEoX1rbOsz5GDAfBgNVHSMEGDAWgBTTi/q1F2ia\n"
	"bqlS7yEoX1rbOsz5GDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAL\n"
	"aqJ2FgcKLBBHJ8VeNSuGV2cxVYH1JIaHnzL6SlE5q7MYVg+Ofbs2PRlTiWGMazC7\n"
	"q5RKVj9zj0z/8i3ScWrWXFmyp85ZHfuo/DeK6HcbEXJEOfPDvyMPuhVBTzuBIRJb\n"
	"41M27NdIVCdxP6562n6Vp0gbE8kN10q+ksw8YBoLFP0D1da7D5WnSV+nwEIP+F4a\n"
	"3ZX80bNt6tRj9XY0gM68mI60WXrF/qYL+NUz+D3Lw9bgDSXxpSN8JGYBR85BxBvR\n"
	"NNAhsJJ3yoAvbPUQ4m8J/CoVKKgcWymS1pvEHmF47pgzbbjm5bdthlIx+swdiGFa\n"
	"WzdhzTYwVkxBaU+xf/2w\n"
	"-----END CERTIFICATE-----\n";

static const void *cavan_net_input_buff;
static u16 cavan_net_input_length;

void cavan_net_input_set(const void *buff, u16 length)
{
	cavan_net_input_length = length;
	cavan_net_input_buff = buff;
}

void cavan_net_input_clear(void)
{
	cavan_net_input_set(NULL, 0);
}

static int alloc_fd(void)
{
    uint8_t i = 0;

    for (i = 0; i < ML302_MAX_SOCKET_NUM; i++) {
        if (0 == ((sg_SocketBitMap >> i) & 0x01)) {
            sg_SocketBitMap |= (1 << i) & 0xff;
			Log_d("SocketBitMap = 0x%x ", sg_SocketBitMap);
            break;
        }
    }

    return (i < ML302_MAX_SOCKET_NUM) ? i : UNUSED_SOCKET;
}

static void free_fd(int fd)
{
    //uint8_t i = fd;

    if ((fd != UNUSED_SOCKET) && fd < ML302_MAX_SOCKET_NUM) {
        sg_SocketBitMap &= ~((1 << fd) & 0xff);
    }
}

/* unsolicited TCP/IP command<err> codes */
static void at_tcp_ip_errcode_parse(int result)
{
    switch(result)
    {
//    case 0 : Log_d("%d : operation succeeded ",             result); break;
//    case 1 : Log_e("%d : UNetwork failure",                 result); break;
//    case 2 : Log_e("%d : Network not opened",               result); break;
//    case 3 : Log_e("%d : Wrong parameter",                  result); break;
//    case 4 : Log_d("%d : Operation not supported",          result); break;
//    case 5 : Log_e("%d : Failed to create socket",          result); break;
//    case 6 : Log_e("%d : Failed to bind socket",            result); break;
//    case 7 : Log_e("%d : TCP server is already listening",  result); break;
//    case 8 : Log_e("%d : Busy",                             result); break;
//    case 9 : Log_e("%d : Sockets opened",                   result); break;
//    case 10 : Log_e("%d : Timeout ",                        result); break;
//    case 11 : Log_e("%d : DNS parse failed for AT+CIPOPEN", result); break;
//    case 255 : Log_e("%d : Unknown error",                  result); break;
    }
}

static void urc_input_func(const char *data, size_t size)
{
	const char *buff = cavan_net_input_buff;
	u16 length = cavan_net_input_length;

	cavan_net_input_clear();

	if (buff != NULL && length > 0) {
		println("input = %d", length);
		at_client_send(at_client_get(), buff, length, AT_RESP_TIMEOUT_MS);
		at_setFlag(NET_INPUT_FLAG);
	}
}

static void urc_connect_func(const char *data, size_t size)
{
    int device_socket = 0, result = 0;
 

    POINTER_SANITY_CHECK_RTN(data && size);

    sscanf(data, "+CAOPEN: %d,%d", &device_socket, &result);

    if (result == 0)
    {
        //sim76xx_socket_event_send(device, SET_EVENT(device_socket, SIM76XX_EVENT_CONN_OK));
		at_setFlag(NET_CONN_OK_FLAG);
    }
    else
    {
        at_tcp_ip_errcode_parse(result);
        //sim76xx_socket_event_send(device, SET_EVENT(device_socket, SIM76XX_EVENT_CONN_FAIL));
		at_setFlag(NET_CONN_FAIL_FLAG);
    }
}

static void urc_send_func(const char *data, size_t size)
{
	int totalsize, unacksize;
	
    POINTER_SANITY_CHECK_RTN(data && size);

    sscanf(data, "+CAACK: %d,%d", &totalsize, &unacksize);

    if (unacksize == 0) {
        at_setFlag(SEND_OK_FLAG);
    } 
//	else if (strstr(data, "SEND FAIL")) {
//        at_setFlag(SEND_FAIL_FLAG);
//    }
}

static void urc_close_func(const char *data, size_t size)
{
    int fd = 0;
    POINTER_SANITY_CHECK_RTN(data);

    sscanf(data, "+CACLOSE: %d", &fd);

    /* notice the socket is disconnect by remote */
    if (at_evt_cb_table[AT_SOCKET_EVT_CLOSED]) {
        at_evt_cb_table[AT_SOCKET_EVT_CLOSED](fd, AT_SOCKET_EVT_CLOSED, NULL, 0);
    }

    free_fd(fd);
}

static void urc_recv_func(const char *data, size_t size)
{
    int      fd;
    size_t   bfsz = 0, temp_size = 0;
    uint32_t timeout;
    char *   recv_buf, temp[8];

    POINTER_SANITY_CHECK_RTN(data);

    /* get the current socket and receive buffer size by receive data */
    sscanf(data, "+CAURC: \"recv\",%d,%d,", &fd, (int *)&bfsz);

    /* get receive timeout by receive buffer length */
    timeout = bfsz;

    if (fd < 0 || bfsz == 0)
        return;

    recv_buf = HAL_Malloc(bfsz);
    if (NULL == recv_buf) {
        Log_e("no memory for URC receive buffer (%d)!", bfsz);
        /* read and clean the coming data */
        while (temp_size < bfsz) {
            if (bfsz - temp_size > sizeof(temp)) {
                at_client_recv(temp, sizeof(temp), timeout);
            } else {
                at_client_recv(temp, bfsz - temp_size, timeout);
            }
            temp_size += sizeof(temp);
        }
        return;
    }

    /* sync receive data */
    if (at_client_recv(recv_buf, bfsz, timeout) != bfsz) {
        Log_e("receive size(%d) data failed!", bfsz);
        HAL_Free(recv_buf);
        return;
    }

    /* notice the receive buffer and buffer size */
    if (at_evt_cb_table[AT_SOCKET_EVT_RECV]) {
        at_evt_cb_table[AT_SOCKET_EVT_RECV](fd, AT_SOCKET_EVT_RECV, recv_buf, bfsz);
    }
}

static void urc_gnss_func(const char *data, size_t size)
{
	POINTER_SANITY_CHECK_RTN(data);
	//+SGNSCMD: 2,08:14:43,31.16211,121.30720,13.12,50.54,41.15,0.00,0.00,0x17bba2c7d38,375  
    //Log_d("GNSS: %s \r\n", data);
}

static void urc_net_info_func(const char *data, size_t size)
{
	POINTER_SANITY_CHECK_RTN(data);
	Log_i("Net Info: %s \r\n", data);
}

static void urc_func(const char *data, size_t size)
{
    POINTER_SANITY_CHECK_RTN(data);
    int i;

//    if (strstr(data, "WIFI CONNECTED")) {
//        at_setFlag(WIFI_CONN_FLAG);
//    } else if (strstr(data, "WIFI DISCONNECT")) {
//        /* notice the socket is disconnect by remote */
//        if (at_evt_cb_table[AT_SOCKET_EVT_CLOSED]) {
//            for (i = 0; i < ML302_MAX_SOCKET_NUM; i++) {
//                at_evt_cb_table[AT_SOCKET_EVT_CLOSED](i, AT_SOCKET_EVT_CLOSED, NULL, 0);
//                free_fd(i);
//            }
//        }
//    }
}

static void urc_mqtt_conn_func(const char *data, size_t size)
{
	int conn = 0, result = 0;

	if (sscanf(data, "+MMQTTCON: %d,%d", &conn, &result) == 2) {
		println("MMQTTCON: conn = %d, result = %d", conn, result);

		if (result == 0) {
			at_setFlag(MQTT_OK_FLAG);
		} else {
			at_setFlag(MQTT_FAIL_FLAG);
		}
	}
}

static void urc_mqtt_disconn_func(const char *data, size_t size)
{
	int conn = 0, result = 0;

	if (sscanf(data, "+MMQTTDISCON: %d,%d", &conn, &result) == 2) {
		println("MMQTTDISCON: conn = %d, result = %d", conn, result);

		if (result == 0) {
			at_setFlag(MQTT_OK_FLAG);
		} else {
			at_setFlag(MQTT_FAIL_FLAG);
		}
	}
}

static void urc_mqtt_sub_func(const char *data, size_t size)
{
	int conn = 0, result = 0;

	if (sscanf(data, "+MMQTTSUB: %d,%d", &conn, &result) == 2) {
		println("MMQTTSUB: conn = %d, result = %d", conn, result);

		if (result == 0) {
			at_setFlag(MQTT_OK_FLAG);
		} else {
			at_setFlag(MQTT_FAIL_FLAG);
		}
	}
}

static void urc_mqtt_pub_func(const char *data, size_t size)
{
	int conn = 0, result = 0;

	if (sscanf(data, "+MMQTTPUB: %d,%d", &conn, &result) == 2) {
		println("MMQTTPUB: conn = %d, result = %d", conn, result);

		if (result == 0) {
			at_setFlag(MQTT_OK_FLAG);
		} else {
			at_setFlag(MQTT_FAIL_FLAG);
		}
	}
}

static int cavan_mqtt_readline(char *buff, u16 size)
{
	at_client_t client = at_client_get();
	char *text;
	int length;

	length = at_recv_readline(client);
	if (length < 0) {
		return length;
	}

	text = client->recv_buffer;

	while (length > 0 && cavan_is_space(text[length - 1])) {
		length--;
	}

	if (length >= size) {
		length = size - 1;
	}

	memcpy(buff, text, length);
	buff[length] = 0;

	return length;
}

static bool process_property_set(const char *buff, u16 length)
{
	println("set: %s", buff);
	return false;
}

static bool process_property_get(const char *buff, u16 length)
{
	println("get: %s", buff);
	return false;
}

static bool process_property_post_reply(const char *buff, u16 length)
{
	println("reply: %s", buff);
	return false;
}

static bool process_property_desired_reply(const char *buff, u16 length)
{
	println("desired: %s", buff);
	return false;
}

static bool process_mqtt_publish(char *argv[], int argc)
{
	static char line1[1024];
	static char line0[64];
	const char *name;
	int len0, len1;

	len0 = cavan_mqtt_readline(line0, sizeof(line0));
	if (len0 < 0) {
		return false;
	}

	len1 = cavan_mqtt_readline(line1, sizeof(line1));
	if (len1 < 0) {
		return false;
	}

	if (CAVAN_ENDS_WITH(line0, len0, "/set")) {
		return process_property_set(line1, len1);
	}

	if (CAVAN_ENDS_WITH(line0, len0, "/get")) {
		return process_property_get(line1, len1);
	}

	if (CAVAN_ENDS_WITH(line0, len0, "/post/reply")) {
		return process_property_post_reply(line1, len1);
	}

	if (CAVAN_ENDS_WITH(line0, len0, "/desired/get/reply")) {
		return process_property_desired_reply(line1, len1);
	}

	return false;
}

static void urc_mqtt_urc_func(const char *data, size_t size)
{
	static char buff[1024];
	char *args[16];
	int count;
	int index;

	if (size < 10) {
		return;
	}

	data += 10;
	size -= 10;

	if (size > sizeof(buff) - 1) {
		size = sizeof(buff) - 1;
	}

	memcpy(buff, data, size);
	buff[size] = 0;

	count = cavan_text_split(buff, ',', args, NELEM(args));
	if (count < 2) {
		return;
	}

	if (strcmp(args[1], "\"publish\"") == 0) {
		process_mqtt_publish(args + 2, count - 2);
	}
}

static at_urc urc_table[] = 
{
	{ ">",				"\0",		urc_input_func },
	{ "+CAOPEN",		"\r\n",		urc_connect_func },
	{ "+CACLOSE",		"\r\n",		urc_close_func },
	{ "+CAURC",			"\r\n",		urc_recv_func },
	{ "+SGNSCMD",		"\r\n",		urc_gnss_func },
	{ "+CPSI",			"\r\n",		urc_net_info_func },
	{ "+MMQTTCON",	 	"\r\n",		urc_mqtt_conn_func },
	{ "+MMQTTDISCON",	"\r\n",		urc_mqtt_disconn_func },
	{ "+MMQTTSUB",		"\r\n",		urc_mqtt_sub_func },
	{ "+MMQTTPUB",		"\r\n",		urc_mqtt_pub_func },
	{ "+MMQTTURC",		"\r\n", 	urc_mqtt_urc_func },
};

static void ml302_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    if (event < sizeof(at_evt_cb_table) / sizeof(at_evt_cb_table[0])) {
        at_evt_cb_table[event] = cb;
    }
}

static int ml302_power(bool status)
{
	Log_d("ml302_power: %d", status);

	if(status)
	{
		modem_power(true);
		at_delayms(200);
		modem_pwr_key(true, 3000);
		if(at_obj_wait_connect(ML302_WAIT_CONNECT_TIME))
		{
			Log_e("ml302 power on error!");
			return QCLOUD_ERR_TIMEOUT;
		}
	}
	else
	{
		//modem_pwr_key(true, 4000);
		modem_pwr_key(false, 0);
		modem_power(false);
	}
	return QCLOUD_RET_SUCCESS;
}

static int ml302_reset(void)
{
	modem_reset();
	at_delayms(200);
	if(at_obj_wait_connect(ML302_WAIT_CONNECT_TIME))
	{
		Log_e("ml302 reset error!");
		return QCLOUD_ERR_TIMEOUT;
	}

	return QCLOUD_RET_SUCCESS;
}

bool cavan_send_VERCTRL(at_response_t rsp, u8 times)
{
	while (times > 0) {
#if AUTO_ACTIVE == 0
		at_exec_cmd(rsp, "AT+VERCTRL=0,0");
#else
		at_exec_cmd(rsp, "AT+VERCTRL=0,1");
#endif

		if (at_resp_get_line_by_kw(rsp, "OK")) {
			Log_d("%s auto active success.", DEVICE_NAME);
			return true;
		}

		Log_i("\"AT+VERCTRL\" commands send retry...");
		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_CGSN(at_response_t rsp, u8 times)
{
	char imei[32];

	while (times > 0) {
		at_exec_cmd(rsp, "AT+CGSN=1");

		if (at_resp_parse_line_args_by_kw(rsp, "+CGSN:", "+CGSN: %s", imei) == 1) {
			memcpy(g_IMEI, imei, sizeof(g_IMEI));
			Log_d("imei = %s", imei);
			return true;
		}

		Log_i("\"AT+CGSN\" commands send retry...");
		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_CPIN(at_response_t rsp, u8 times)
{
	while (times > 0) {
		at_exec_cmd(rsp, "AT+CPIN?");

		if (at_resp_get_line_by_kw(rsp, "READY")) {
			Log_d("%s device SIM card detection success.", DEVICE_NAME);
			return true;
		}

		Log_i("\"AT+CPIN\" commands send retry...");
		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_CIMI(at_response_t rsp, u8 times)
{
	while (times > 0) {
		at_exec_cmd(rsp, "AT+CIMI");

		if (at_resp_get_line_by_kw(rsp, "OK")) {
			return true;
		}

		Log_i("\"AT+CIMI\" commands send retry...");
		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_CFUN(at_response_t rsp, u8 times)
{
	int value;

	while (times > 0) {
		at_exec_cmd(rsp, "AT+CFUN?");

		if (at_resp_parse_line_args_by_kw(rsp, "+CFUN:", "+CFUN: %d", &value) == 1) {
			Log_d("value = %d", value);
			if (value != 0) {
				Log_d("%s stack opened.", DEVICE_NAME, value);
				return true;
			}

			at_exec_cmd(rsp, "AT+CFUN=1");
			at_resp_get_line_by_kw(rsp, "OK");
		} else {
			Log_i("\"AT+CFUN\" commands send retry...");
			at_delayms(1000);
		}

		times--;
	}

	return false;
}

bool cavan_send_CSQ(at_response_t rsp, u8 times)
{
	int value0, value1;

	while (times > 0) {
		at_exec_cmd(rsp, "AT+CSQ");

		if (at_resp_parse_line_args_by_kw(rsp, "+CSQ:", "+CSQ: %d,%d", &value0, &value1) == 2) {
			Log_d("value0 = %d, value1 = %d", value0, value1);

			ml302_rssi = value0;
			ml302_ber = value1;

			if (value0 != 99) {
				Log_d("%s device signal strength: %d  Channel bit error rate: %d", DEVICE_NAME, value0, value1);
				return true;
			}
		}

		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_CGDCONT(at_response_t rsp, u8 times)
{
	while (times > 0) {
		at_exec_cmd(rsp, "AT+CGDCONT=1,\"IP\",\"CMIOT\"");

		if (at_resp_get_line_by_kw(rsp, "OK")) {
			return true;
		}

		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_CGACT(at_response_t rsp, u8 times)
{
	int value0, value1;
	int err_times = 0;

	while (times > 0) {
		at_exec_cmd(rsp, "AT+CGACT?");

		if (at_resp_parse_line_args_by_kw(rsp, "+CGACT:", "+CGACT: %d,%d", &value0, &value1) == 2) {
			Log_d("value0 = %d, value1 = %d", value0, value1);

			if (value1 == 1) {
				return true;
			}

#if AUTO_ACTIVE == 0
			at_exec_cmd(rsp, "AT+CGACT=1,1");

			if (at_resp_parse_line_args_by_kw(rsp, "+CGACT:", "+CGACT: %d,%d", &value0, &value1) == 2) {
				Log_d("value0 = %d, value1 = %d", value0, value1);

				if (value0 == 1 && value1 == 1) {
					return true;
				}
			}
#endif

			err_times = 0;
		} else if (++err_times > 10) {
			break;
		}

		println("times = %d, err = %d", times, err_times);
		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_wait_conn_complete(u32 timeout)
{
	if (at_waitFlag(MQTT_OK_FLAG | MQTT_FAIL_FLAG, timeout)) {
		if (at_waitFlag(MQTT_FAIL_FLAG, 100)) {
			Log_e("connect fail!!!");
			return false;
		}
	} else {
		Log_e("connect timeout!!!");
		return false;
	}

	return true;
}

bool cavan_wait_send_complete(u32 timeout)
{
	bool success = at_waitFlag(NET_INPUT_FLAG, timeout);
	cavan_net_input_clear();
	return success;
}

bool cavan_send_MSSLCFG(at_response_t rsp, u8 times)
{
	while (times > 0) {
		at_exec_cmd(rsp, "AT+MSSLCFG=0,1");

		if (at_resp_get_line_by_kw(rsp, "OK")) {
			return true;
		}

		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_MSSLCTXCFG(at_response_t rsp, u8 times)
{
	while (times > 0) {
#if CONFIG_MQTTS
		at_exec_cmd(rsp, "AT+MSSLCTXCFG=1,0,1");
#else
		at_exec_cmd(rsp, "AT+MSSLCTXCFG=1,0,0");
#endif

		if (at_resp_get_line_by_kw(rsp, "OK")) {
			return true;
		}

		at_delayms(1000);
		times--;
	}

	return false;
}

bool cavan_send_MSSLSECWR(at_response_t rsp, u8 times)
{
	while (times > 0) {
		at_clearFlag(NET_INPUT_FLAG);

		cavan_net_input_set(mqtt_cert, sizeof(mqtt_cert) - 1);
		at_exec_cmd(rsp, "AT+MSSLSECWR=1,0,%d", cavan_net_input_length);

		if (cavan_wait_send_complete(2000) && at_resp_get_line_by_kw(rsp, "+MSSLSECWR")) {
			return true;
		}

		at_delayms(1000);
		times--;
	}

	return false;
}

/* AT+MMQTTDISCON=<connect_id>
 * +MMQTTDISCON=<connect_id>,<success>
 */

bool cavan_send_MMQTTDISCON(at_response_t rsp)
{
	at_clearFlag(MQTT_OK_FLAG | MQTT_FAIL_FLAG);
	at_exec_cmd(rsp, "AT+MMQTTDISCON=%d", MQTT_CONN);
	return cavan_wait_conn_complete(500);
}

/* AT+MMQTTCON=<connect_id>,<ip/host_name>,<port>,<client_id>,<username>,<password>
 * +MMQTTCON=<connect_id>,<success>
 */

bool cavan_send_MMQTTCON(at_response_t rsp)
{
	char token[200];

	cavan_mqtt_token(MQTT_USER, MQTT_PASS, MQTT_NAME, token, sizeof(token));

	at_clearFlag(MQTT_OK_FLAG | MQTT_FAIL_FLAG);
	at_exec_cmd(rsp, "AT+MMQTTCON=%d,\"%s\",%d,\"%s\",\"%s\",\"%s\"",
		MQTT_CONN, MQTT_HOST, MQTT_PORT, MQTT_NAME, MQTT_USER, token);

	return cavan_wait_conn_complete(AT_RESP_TIMEOUT_MS);
}

bool cavan_send_MMQTTPUB(at_response_t rsp, const char *topic, const void *buff, u16 length)
{
	ml302_msgid++;

	at_clearFlag(NET_INPUT_FLAG);

	cavan_net_input_set(buff, length);
	at_exec_cmd(rsp, "AT+MMQTTPUB=0,\"$sys/" MQTT_USER "/%s/thing/property/%s\",%d", MQTT_NAME, topic, cavan_net_input_length);

	return cavan_wait_send_complete(2000) && at_resp_get_line_by_kw(rsp, "+MMQTTPUB:");
}

bool cavan_property_post(at_response_t rsp, const void *buff, u16 length)
{
	return cavan_send_MMQTTPUB(rsp, "post", buff, length);
}

bool cavan_property_report(at_response_t rsp)
{
	cavan_json_t *json = &ml302_send_json;

	cavan_json_init(json);

	cavan_json_begin(json);
	cavan_json_printf(json, "\"id\":\"%d\"", ml302_msgid);

	cavan_json_append_name(json, "params");
	cavan_json_begin(json);
	cavan_mqtt_append_int(json, "RSSI", ml302_rssi);
	cavan_mqtt_append_int(json, "BER", ml302_ber);
	cavan_json_end(json);

	cavan_json_end(json);

	return cavan_property_post(rsp, json->buff, json->length);
}

bool cavan_property_get_desired(at_response_t rsp)
{
	cavan_json_t *json = &ml302_send_json;

	cavan_json_init(json);

	cavan_json_begin(json);
	cavan_json_printf(json, "\"id\":\"%d\"", ml302_msgid);

	cavan_json_append_name(json, "params");
	cavan_json_array_begin(json);
	cavan_json_array_append_text(json, "front_press");
	cavan_json_array_append_text(json, "back_press");
	cavan_json_array_end(json);

	cavan_json_end(json);

	return cavan_send_MMQTTPUB(rsp, "desired/get", json->buff, json->length);
}

bool cavan_property_get_reply(at_response_t rsp, const void *buff, u16 length)
{
	return cavan_send_MMQTTPUB(rsp, "get_reply", buff, length);
}

bool cavan_property_set_reply(at_response_t rsp, const void *buff, u16 length)
{
	return cavan_send_MMQTTPUB(rsp, "set_reply", buff, length);
}

/*
 * AT+MMQTTSUB=<connect_id>,<topicfilter>
 */

bool cavan_send_MMQTTSUB(at_response_t rsp, const char *topic)
{
	at_clearFlag(MQTT_OK_FLAG | MQTT_FAIL_FLAG);
	at_exec_cmd(rsp, "AT+MMQTTSUB=0,\"$sys/%s/%s/thing/property/%s\"", MQTT_USER, MQTT_NAME, topic);
	return cavan_wait_conn_complete(AT_RESP_TIMEOUT_MS);
}

static bool ml302_init_MQTT(at_response_t rsp, u8 times)
{
	if (!cavan_send_MSSLCFG(rsp, 10)) {
		return false;
	}

	at_delayms(100);

	if (!cavan_send_MSSLCTXCFG(rsp, 10)) {
		return false;
	}

	at_delayms(100);

	while (1) {
		if (cavan_send_MMQTTCON(rsp)) {
			break;
		}

		if (times < 1) {
			return false;
		}

		cavan_send_MMQTTDISCON(rsp);
		times--;
	}

	at_delayms(100);

	if (!cavan_send_MMQTTSUB(rsp, "post/reply")) {
		return false;
	}

	at_delayms(100);

	if (!cavan_send_MMQTTSUB(rsp, "set")) {
		return false;
	}

	at_delayms(100);

	if (!cavan_send_MMQTTSUB(rsp, "get")) {
		return false;
	}

	at_delayms(100);

	if (!cavan_send_MMQTTSUB(rsp, "desired/get/reply")) {
		return false;
	}

	at_delayms(100);

	cavan_property_report(rsp);
	at_delayms(500);

	cavan_property_get_desired(rsp);
	at_delayms(100);

	return true;
}

static int ml302_init(void)
{
#define INIT_RETRY                     3
#define CPIN_RETRY                     5
#define CSQ_RETRY                      50
#define CREG_RETRY                     10
#define CGREG_RETRY                    50
#define CGATT_RETRY                    50
#define CCLK_RETRY                     10
    at_response_t resp = NULL;
    int           ret;
    int           i;
	int 		  qi_arg[3] = {0};
	char          parsed_data[20] = {0};
	int           retry_num = INIT_RETRY;
	
    resp = at_create_resp(160, 0, AT_RESP_TIMEOUT_MS);
    if (NULL == resp) {
        Log_e("No memory for response structure!");
        ret = QCLOUD_ERR_FAILURE;
        goto __exit;
    }
	
    ml302_power(true);

	while(retry_num--) {
		/* power-up ml302 */
		memset(g_IMEI, 0, IMEI_MAX_LEN);
		/* wait ML302 startup finish, Send AT every 5s, if receive OK, SYNC success*/
		if(at_obj_wait_connect(ML302_WAIT_CONNECT_TIME))
		{
			Log_e("ml302 connect timeout!");
			ret = QCLOUD_ERR_TIMEOUT;
			goto __exit;
		}

		/* disable echo */

		ret = at_exec_cmd(resp, "ATE0");
		if (QCLOUD_RET_SUCCESS != ret) {
			Log_e("cmd ATE0 exec err");
			// goto exit;
		}

		at_delayms(100);

		if (!cavan_send_VERCTRL(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

		if (!cavan_send_CPIN(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

		if (!cavan_send_CFUN(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

		if (!cavan_send_CIMI(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

		if (!cavan_send_CGSN(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

#if CONFIG_MQTTS
		if (!cavan_send_MSSLSECWR(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);
#endif

		if (!cavan_send_CSQ(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

#if AUTO_ACTIVE == 0
		if (!cavan_send_CGDCONT(resp, 10)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);
#endif

		if (!cavan_send_CGACT(resp, 180)) {
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}

		at_delayms(100);

#ifdef USING_RTC
		{
			/* get real time */
			volatile int year, month, day, hour, min, sec;
			char str_timezone[5] = {0};
			volatile int timezone = 0;
			
//			at_obj_exec_cmd(resp, "AT+CLTS=1");
//			at_delayms(2000);
			for (i = 0; i < CCLK_RETRY; i++)
			{
				if (at_obj_exec_cmd(resp, "AT+CCLK?") < 0)
				{
					at_delayms(500);
					continue;
				}

				/* +CCLK: "18/12/22,18:33:12+32" */
				if (at_resp_parse_line_args_by_kw(resp, "+CCLK:", "+CCLK: \"%d/%d/%d,%d:%d:%d%s",
												  &year, &month, &day, &hour, &min, &sec, str_timezone) < 0)
				{
					at_delayms(500);
					continue;
				}
				timezone = atoi(str_timezone)/4;
				//set_date(year + 2000, month, day);
				//set_time(hour, min, sec);
				//set_data_time(year, month, day, hour, min, sec, timezone);
				set_local_time(make_data_time(year, month, day, hour, min, sec, timezone));
				break;
			}

			if (i == CCLK_RETRY)
			{
				Log_e("%s device GPRS attach failed.", DEVICE_NAME);
				ret = QCLOUD_ERR_FAILURE;
				goto __exit;
			}
		}
#endif /* USING_RTC */

//		if (!ml302_init_MQTT(resp, 3)) {
//			ret = QCLOUD_ERR_FAILURE;
//			goto __exit;
//		}
//
//		at_delayms(100);

        /* initialize successfully  */
        ret = QCLOUD_RET_SUCCESS;
        break;

    __exit:
        if (ret != QCLOUD_RET_SUCCESS)
        {
            /* reset the ml302 device */
        	ml302_reset();
            // at_delayms(1000);

            Log_i("%s device initialize retry...", DEVICE_NAME);
        }
	}

    if (resp) {
        at_delete_resp(resp);
    }

	Log_d("ret = %d", ret);
	
    return ret;
}

static int ml302_deinit(void)
{
    /* power off the sim76xx device */
	//ml302_power_off();
	ml302_power(false);
	Log_i("%s device deinit", DEVICE_NAME);
	return QCLOUD_RET_SUCCESS;
}

static int ml302_close(int fd)
{
    at_response_t resp;
    int           ret;

    resp = at_create_resp(64, 0, 4*AT_RESP_TIMEOUT_MS);
    if (NULL == resp) {
        Log_e("No memory for response structure!");
        return QCLOUD_ERR_FAILURE;
    }

    ret = at_exec_cmd(resp, "AT+CACLOSE=%d", fd);

    if (QCLOUD_RET_SUCCESS != ret) {  
        Log_e("close socket(%d) fail", fd);
		goto __exit;
    }
	
	if (at_evt_cb_table[AT_SOCKET_EVT_CLOSED]) {
        at_evt_cb_table[AT_SOCKET_EVT_CLOSED](fd, AT_SOCKET_EVT_CLOSED, NULL, 0);
    }
	
	free_fd(fd);
	
__exit:	
    if (resp) {
        at_delete_resp(resp);
    }

    return ret;
}

static int ml302_connect(const char *ip, uint16_t port, eNetProto proto)
{
    at_response_t resp;
    bool          retryed = false;
    int           fd = 0, ret;
	uint8_t       recv_mode = 1;
	int           cid, result = 1;

    POINTER_SANITY_CHECK(ip, QCLOUD_ERR_INVAL);
    resp = at_create_resp(128, 0, 2*AT_RESP_TIMEOUT_MS);
    if (NULL == resp) {
        Log_e("No memory for response structure!");
        return QCLOUD_ERR_FAILURE;
    }

    fd = alloc_fd();
    if (fd < 0) {
        Log_e("ml302 support max %d chain", ML302_MAX_SOCKET_NUM);
        return QCLOUD_ERR_FAILURE;
    }

__retry:
	at_clearFlag(NET_CONN_OK_FLAG);
	at_clearFlag(NET_CONN_FAIL_FLAG);
    switch (proto) {
        case eNET_TCP:
            /* send AT commands to connect TCP server */
            ret = at_exec_cmd(resp, "AT+CAOPEN=%d,0,\"TCP\",\"%s\",%d,%d", fd, ip, port, recv_mode);
            if (QCLOUD_RET_SUCCESS != ret) {
                Log_e("start tcp connect failed, fd=%d,ip(%s),port(%d)", fd, ip, port);
            }
            break;

        case eNET_UDP:
            ret = at_exec_cmd(resp, "AT+CAOPEN=%d,0,\"UDP\",\"%s\",%d,%d", fd, ip, port, recv_mode);
            if (QCLOUD_RET_SUCCESS != ret) {
                Log_e("start udp connect failed, fd=%d,ip(%s),port(%d)", fd, ip, port);
            }
            break;

        default:
            Log_e("Not supported connect type : %d", proto);
            ret = QCLOUD_ERR_FAILURE;
            goto __exit;
    }
	if(QCLOUD_RET_SUCCESS == ret) {
		if(at_waitFlag(NET_CONN_OK_FLAG|NET_CONN_FAIL_FLAG, AT_RESP_TIMEOUT_MS))
		{
			if(at_waitFlag(NET_CONN_FAIL_FLAG, 100))
			{
				Log_e("socket connect fail!!!");
				ret = QCLOUD_ERR_FAILURE;
				//goto __exit;
			}
		}
		else
		{
			Log_e("socket connect timeout!!!");
			ret = QCLOUD_ERR_FAILURE;
			//goto __exit;
		}
	}
    if ((QCLOUD_RET_SUCCESS != ret)&& !retryed) {
        Log_e("socket(%d) connect failed, maybe the socket was not be closed at the last time and now will retry.", fd);
        if (QCLOUD_RET_SUCCESS != ml302_close(fd)) {
            goto __exit;
        }
        retryed = true;
        at_delayms(100);
        goto __retry;
    }

__exit:

    if (resp) {
        at_delete_resp(resp);
    }

    if (QCLOUD_RET_SUCCESS != ret) {
        free_fd(fd);
        fd = UNUSED_SOCKET;
    }

    at_delayms(200);
    return fd;
}

static int ml302_send(int fd, const void *buff, size_t len)
{
    int           ret;
    at_response_t resp;
    size_t        cur_pkt_size = 0;
    size_t        sent_size    = 0;
    size_t        temp_size    = 0;
	size_t        remain_size  = 0;
	
    POINTER_SANITY_CHECK(buff, QCLOUD_ERR_INVAL);
    resp = at_create_resp(128, 2, AT_RESP_TIMEOUT_MS);
    if (NULL == resp) {
        Log_e("No memory for response structure!");
        return QCLOUD_ERR_FAILURE;
    }

    /* set AT client end sign to deal with '>' sign.*/
    at_set_end_sign('>');

    while (sent_size < len) {
        if (len - sent_size < ML302_SEND_MAX_LEN_ONCE) {
            cur_pkt_size = len - sent_size;
        } else {
            cur_pkt_size = ML302_SEND_MAX_LEN_ONCE;
        }

        at_clearFlag(SEND_OK_FLAG);
        at_clearFlag(SEND_FAIL_FLAG);
        /* send the "AT+CASEND" commands to AT server than receive the '>' response on the first line. */
        ret = at_exec_cmd(resp, "AT+CASEND=%d,%d", fd, cur_pkt_size);
        if (QCLOUD_RET_SUCCESS != ret) {
            Log_e("cmd AT+CASEND exec err");
            goto __exit;
        }
        at_set_end_sign(0);
        /* send the real data to server or client */
        temp_size = at_client_send(at_client_get(), (char *)buff + sent_size, cur_pkt_size, AT_RESP_TIMEOUT_MS);

        if (cur_pkt_size != temp_size) {
            Log_e("at send real data failed");
            goto __exit;
        }

#ifndef AT_OS_USED
        at_urc urc_send = {.cmd_prefix = "SEND OK", .cmd_suffix = "\r\n", NULL};
        at_client_yeild(&urc_send, resp->timeout);
        if (at_client_get()->resp_status != AT_RESP_OK) {
            Log_e("send fail");
            ret = QCLOUD_ERR_FAILURE;
            goto __exit;
        }
#else
		//at_exec_cmd(resp, "AT+CAACK=%d", fd);
        //at_resp_parse_line_args_by_kw(resp, "+CAACK:", "+CAACK: %d,%d", &temp_size, &remain_size);
		{
			Timer timer;

			countdown_ms(&timer, 2*AT_RESP_TIMEOUT_MS);
			do{
				if(at_obj_exec_cmd(resp, "AT+CAACK=%d", fd) < 0)
				{
					Log_e("send fail");
					ret = QCLOUD_ERR_FAILURE;
					goto __exit;
				}
				if(at_resp_parse_line_args_by_kw(resp, "+CAACK:", "+CAACK: %d,%d", &temp_size, &remain_size) < 0)
				{
					continue;
				}
				
				if(remain_size == 0)
					break;
				at_delayms(100);
			}while(!expired(&timer));
		}
//        if (at_waitFlag(SEND_OK_FLAG | SEND_FAIL_FLAG, AT_RESP_TIMEOUT_MS)) {
//            if (at_waitFlag(SEND_FAIL_FLAG, AT_RESP_TIMEOUT_MS)) {
//                Log_e("send fail");
//                ret = QCLOUD_ERR_FAILURE;
//                goto __exit;
//            }
//        }
#endif
        sent_size += cur_pkt_size;
    }

    ret = QCLOUD_RET_SUCCESS;

__exit:
    /* reset the end sign for data */
    if (resp) {
        at_delete_resp(resp);
    }

    return sent_size;  // fancyxu
}

static int ml302_recv_timeout(int fd, void *buf, size_t len, uint32_t timeout)
{
#ifndef AT_OS_USED
    at_client_yeild(NULL, timeout);
#endif
    return QCLOUD_RET_SUCCESS;
}

static int ml302_parse_domain(const char *host_name, char *host_ip, size_t host_ip_len)
{
#define RESOLVE_RETRY 5

    char          recv_ip[16] = {0};
    at_response_t resp;
    int           ret, i, status = 0;
	uint8_t retry = 5;
	int timeout = 10000;
	char temp[20] = {0};

    POINTER_SANITY_CHECK(host_name, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(host_ip, QCLOUD_ERR_INVAL);

    if (host_ip_len < 16) {
        Log_e("host ip buff too short");
        return QCLOUD_ERR_FAILURE;
    }

    resp = at_create_resp(128, 4, 2 * AT_RESP_TIMEOUT_MS);
    if (NULL == resp) {
        Log_e("No memory for response structure!");
        return QCLOUD_ERR_FAILURE;
    }

    for (i = 0; i < RESOLVE_RETRY; i++) {
        ret = at_obj_exec_cmd(resp, "AT+CDNSGIP=\"%s\",%d,%d", host_name, retry, timeout);
        if (QCLOUD_RET_SUCCESS != ret) {
            Log_e("exec AT+CDNSGIP=\"%s\" fail", host_name);
            goto __exit;
        }
		//at_delayms(5000);
        /* parse the third line of response data, get the IP address */
        if (at_resp_parse_line_args_by_kw(resp, "+CDNSGIP:", "+CDNSGIP: %d,%[^,],\"%[^\"]", &status, temp, recv_ip) < 0) {
            at_delayms(100);
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        } else {
			if(!status)
			{
				at_delayms(100);
				continue;
			}
		}

        if (strlen(recv_ip) < 8) {
            at_delayms(100);
            /* resolve failed, maybe receive an URC CRLF */
            continue;
        } else {
            strncpy(host_ip, recv_ip, 15);
            host_ip[15] = '\0';
            break;
        }
    }
	
	if (i == RESOLVE_RETRY)
	{
		Log_e("%s device parse domain failed.", DEVICE_NAME);
		ret = QCLOUD_ERR_FAILURE;
		goto __exit;
	}
		
__exit:

    if (resp) {
        at_delete_resp(resp);
    }

#undef RESOLVE_RETRY

    return ret;
}

static int ml302_query_info(ue_info *info)
{
	at_response_t resp = NULL;
	int ret;
	
	resp = at_create_resp(160, 0, AT_RESP_TIMEOUT_MS);
	if (NULL == resp) 
	{
		Log_e("No memory for response structure!");
		ret = QCLOUD_ERR_FAILURE;
		goto __exit;
	}
	
	ret = at_exec_cmd(resp, "AT+CPSI?");
	if (QCLOUD_RET_SUCCESS != ret)
	{
		Log_e("cmd AT+CPSI? exec err");
	}
	
	if (resp) 
	{
		at_delete_resp(resp);
	}
	
__exit:
	return ret;
}

at_device_op_t at_ops_ml302 = {
    .init         = ml302_init,
	.deinit       = ml302_deinit,
	.power		  = ml302_power,
	.get_info     = ml302_query_info,
    .connect      = ml302_connect,
    .send         = ml302_send,
    .recv_timeout = ml302_recv_timeout,
    .close        = ml302_close,
    .parse_domain = ml302_parse_domain,
    .set_event_cb = ml302_set_event_cb,
    .deviceName   = "ml302",
};

int at_device_ml302_init(void)
{
    int         i;
    int         ret;
    at_client_t p_client;

    ret = HAL_AT_Uart_Init();
    if (QCLOUD_RET_SUCCESS != ret) {
        Log_e("at uart init fail!");
    } else {
        Log_d("at uart init success!");
    }

    /* initialize AT client */
    ret = at_client_init(&p_client);

    if (QCLOUD_RET_SUCCESS != ret) {
        Log_e("at client init fail,ret:%d", ret);
        goto exit;
    } else {
        Log_d("at client init success");
    }

    /* register URC data execution function  */
    at_set_urc_table(p_client, urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

    Log_d("urc table addr:%p, size:%d", p_client->urc_table, p_client->urc_table_size);
    for (i = 0; i < p_client->urc_table_size; i++) {
        Log_d("%s", p_client->urc_table[i].cmd_prefix);
    }

    ret = at_device_op_register(&at_ops_ml302);
    if (QCLOUD_RET_SUCCESS != ret) {
        Log_e("at device driver register fail");
		goto exit;
    }

exit:
    if (QCLOUD_RET_SUCCESS != ret) {
        if (NULL != p_client) {
            at_client_deinit(&p_client);
        }
    }

    return ret;
}

int at_device_ml302_deinit(void)
{
    at_client_deinit(NULL);
	HAL_AT_Uart_Deinit();
    return QCLOUD_RET_SUCCESS;
}

char *at_device_get_imei(void)
{
	if(strlen(g_IMEI) >= 15)
		return g_IMEI;
	else
		return NULL;
}
#endif /*AT_DEVICE_ML302*/
