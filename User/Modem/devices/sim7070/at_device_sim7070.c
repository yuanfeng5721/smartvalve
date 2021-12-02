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
#ifdef AT_DEVICE_SIM7070
#include "at_device_sim7070.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "at_client.h"
#include "at_socket_inf.h"
#include "iot.h"
#include "iot_import.h"
#include "utils_param_check.h"
#include "rtc.h"
#include "log.h"
#include "utils_timer.h"

//#define USING_RTC


#undef DEVICE_NAME
#define DEVICE_NAME    "SIM7070"
#define NET_CONN_OK_FLAG (1 << 0)
#define NET_CONN_FAIL_FLAG (1 << 1)
#define SEND_OK_FLAG   (1 << 2)
#define SEND_FAIL_FLAG (1 << 3)

volatile uint8_t sg_SocketBitMap = 0;

static at_evt_cb_t at_evt_cb_table[] = {
    [AT_SOCKET_EVT_RECV]   = NULL,
    [AT_SOCKET_EVT_CLOSED] = NULL,
};

#define IMEI_MAX_LEN 16
static char g_IMEI[IMEI_MAX_LEN] = {0};
/* power up sim7070 modem */
void sim7070_power_on(void)
{
#if 0
//	uint8_t pinState = 0;
	
	
	modem_power(RESET);
	HAL_SleepMs(2000);
	modem_power(SET);
	HAL_SleepMs(100);
	
//	pinState = GPIO_ReadInputDataBit(MODEM_STATUS_INPUT);
//	if (pinState == SET)
//    {
//        return;
//    }
	
    GPIO_WriteBit(MODEM_PWR_KEY_OUTPUT, Bit_SET);
//    while (GPIO_ReadInputDataBit(MODEM_STATUS_INPUT) == RESET)
//    {
//        HAL_SleepMs(100);
//    }
	HAL_SleepMs(1500);
    GPIO_WriteBit(MODEM_PWR_KEY_OUTPUT, Bit_RESET);
	HAL_SleepMs(1000);
	//while(1);
#endif
}

/* power off sim7070 modem */
static void sim7070_power_off(void)
{
#if 0
	modem_power(RESET);
    GPIO_WriteBit(MODEM_PWR_KEY_OUTPUT, Bit_RESET);
	//HAL_SleepMs(2000);
#endif
}

static int alloc_fd(void)
{
    uint8_t i = 0;

    for (i = 0; i < SIM7070_MAX_SOCKET_NUM; i++) {
        if (0 == ((sg_SocketBitMap >> i) & 0x01)) {
            sg_SocketBitMap |= (1 << i) & 0xff;
			Log_d("SocketBitMap = 0x%x ", sg_SocketBitMap);
            break;
        }
    }

    return (i < SIM7070_MAX_SOCKET_NUM) ? i : UNUSED_SOCKET;
}

static void free_fd(int fd)
{
    //uint8_t i = fd;

    if ((fd != UNUSED_SOCKET) && fd < SIM7070_MAX_SOCKET_NUM) {
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
//            for (i = 0; i < SIM7070_MAX_SOCKET_NUM; i++) {
//                at_evt_cb_table[AT_SOCKET_EVT_CLOSED](i, AT_SOCKET_EVT_CLOSED, NULL, 0);
//                free_fd(i);
//            }
//        }
//    }
}

static at_urc urc_table[] = 
{
	//{"+CAACK",   "\r\n", urc_send_func},
	{"+CAOPEN",  "\r\n", urc_connect_func},
	{"+CACLOSE", "\r\n", urc_close_func},
	{"+CAURC",   "\r\n", urc_recv_func},
	{"+SGNSCMD", "\r\n", urc_gnss_func},
	{"+CPSI",    "\r\n", urc_net_info_func},
};

static void sim7070_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb)
{
    if (event < sizeof(at_evt_cb_table) / sizeof(at_evt_cb_table[0])) {
        at_evt_cb_table[event] = cb;
    }
}

static int sim7070_init(void)
{
#define INIT_RETRY                     10
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
	/* power-up sim7070 */
	//sim7070_power_on();
	
	while(retry_num--)
	{	
		/* power-up sim7070 */
		sim7070_power_on();
		memset(g_IMEI, 0, IMEI_MAX_LEN);
		/* wait SIM7070 startup finish, Send AT every 5s, if receive OK, SYNC success*/
		if(at_obj_wait_connect(SIM7070_WAIT_CONNECT_TIME))
		{
			Log_e("sim7070 connect timeout!");
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
		/* get module version */
		ret = at_exec_cmd(resp, "ATI");
		if (QCLOUD_RET_SUCCESS != ret) {
			Log_e("cmd ATI exec err");
			// goto exit;
		}

		/* show module version */
		for (i = 0; i < resp->line_counts - 1; i++) {
			Log_d("%s", at_resp_get_line(resp, i + 1));
		}

		/* get module version */
		ret = at_exec_cmd(resp, "AT+SIMCOMATI");
		if (QCLOUD_RET_SUCCESS != ret) {
			Log_e("cmd SIMCOMATI exec err");
			// goto exit;
		}

		/* show module version */
		for (i = 0; i < resp->line_counts - 1; i++) {
			const char *str = at_resp_get_line(resp, i + 1);
			Log_d("%s", str);
			/* get imei */
			if(strstr(str, "IMEI"))
				sscanf(str, "IMEI:%s", g_IMEI);
		}
		/* check SIM card */
		at_delayms(1000);
		for (i = 0; i < CPIN_RETRY; i++)
		{
			at_exec_cmd(resp, "AT+CPIN?");
			if (at_resp_get_line_by_kw(resp, "READY"))
			{
				Log_d("%s device SIM card detection success.", DEVICE_NAME);
				break;
			}
			Log_i("\"AT+CPIN\" commands send retry...");
			at_delayms(1000);
		}

		if (i == CPIN_RETRY)
		{
			ret = QCLOUD_ERR_FAILURE;
			Log_i("no sim card, pleast insert...");
			//goto __exit;
			break;
		}
		
		/* set GSM+LTE mode */
		ret = at_exec_cmd(resp, "AT+CNMP=13");
		if (QCLOUD_RET_SUCCESS != ret) {
			Log_e("cmd AT+CNMP exec err");
			// goto exit;
		}
//		ret = at_exec_cmd(resp, "AT+CMNB=2");
//		if (QCLOUD_RET_SUCCESS != ret) {
//			Log_e("cmd AT+CMNB exec err");
//			// goto exit;
//		}
		
		/* waiting for dirty data to be digested */
		at_delayms(10);
		/* check signal strength */
		for (i = 0; i < CSQ_RETRY; i++)
		{
			at_exec_cmd(resp, "AT+CSQ");
			at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %d,%d", &qi_arg[0], &qi_arg[1]);
			if (qi_arg[0] != 99)
			{
				Log_d("%s device signal strength: %d  Channel bit error rate: %d", DEVICE_NAME, qi_arg[0], qi_arg[1]);
				break;
			}
			at_delayms(2000);
		}

		if (i == CSQ_RETRY)
		{
			Log_e("%s device signal strength check failed (%s).", DEVICE_NAME, parsed_data);
			ret = QCLOUD_ERR_FAILURE;
			goto __exit;
		}
		at_delayms(2000);
		
		/* check the GPRS network is registered */
        for (i = 0; i < CGREG_RETRY; i++)
        {
            at_exec_cmd(resp, "AT+CGREG?");
            at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %s", &parsed_data);
            if (!strncmp(parsed_data, "0,1", sizeof(parsed_data)) || !strncmp(parsed_data, "0,5", sizeof(parsed_data)))
            {
                Log_d("%s device GPRS is registered(%s).", DEVICE_NAME, parsed_data);
                break;
            }
            at_delayms(2000);
        }

        if (i == CGREG_RETRY)
        {
            Log_e("%s device GPRS is register failed(%s).", DEVICE_NAME, parsed_data);
            ret = QCLOUD_ERR_FAILURE;
            goto __exit;
        }
		/* do not show the prompt when receiving data */
		//at_exec_cmd(resp, "AT+CIPSRIP=0");
		
		/* check packet domain attach or detach */
        for (i = 0; i < CGATT_RETRY; i++)
        {
            at_exec_cmd(resp, "AT+CGATT?");
            at_resp_parse_line_args_by_kw(resp, "+CGATT:", "+CGATT: %s", &parsed_data);
            if (!strncmp(parsed_data, "1", 1))
            {
                Log_d("%s device Packet domain attach.", DEVICE_NAME);
                break;
            }

            at_delayms(2000);
        }

        if (i == CGATT_RETRY)
        {
            Log_e("%s device GPRS attach failed.", DEVICE_NAME);
            ret = QCLOUD_ERR_FAILURE;
            goto __exit;		
        }
		at_delayms(1000);
		
		ret = at_exec_cmd(resp, "AT+CPSI?");
		if (QCLOUD_RET_SUCCESS != ret) {
			Log_e("cmd AT+CPSI? exec err");
			// goto exit;
		}

		/* show module version */
		for (i = 0; i < resp->line_counts - 1; i++) {
			Log_d("%s", at_resp_get_line(resp, i + 1));
		}
		
        /* get apn */
		at_exec_cmd(resp, "AT+CGNAPN");
		{
			char APN[12] = {0};
			uint8_t valid_apn = 0;
			at_resp_parse_line_args_by_kw(resp, "+CGNAPN:", "+CGNAPN: %d,\"%[^\"]\"", &valid_apn, APN);
			if(strlen(APN) == 0)
			{
				Log_e("%s device GPRS is register failed(%s).", DEVICE_NAME, APN);
				ret = QCLOUD_ERR_FAILURE;
				//goto __exit;
			}
			else
			{
				if(strlen(APN) < 2) {
					memcpy(APN, "cmiot", strlen("cmiot"));
				}
			}
			Log_d("apn = %s", APN);
			/* configure context */
			at_exec_cmd(resp, "AT+CNCFG=0,1,\"%s\"",APN);
		}

        /* activate context */
		at_exec_cmd(resp, "AT+CNACT=0,1");
        {
            uint8_t net_status = 0, i;
			uint8_t pdp_idx = 0;
			char ip_addr[20] = {0};
			
			for(i=0; i<10; i++)
			{
				at_delayms(500);
				at_exec_cmd(resp, "AT+CNACT?");
				at_resp_parse_line_args_by_kw(resp, "+CNACT:", "+CNACT: %d,%d,\"%[^\"]\"", &pdp_idx, &net_status, ip_addr);
				/* 0 - netwoek close, 1 - network open, 2 - in operate */
				if (net_status == 0)
				{
					at_exec_cmd(resp, "AT+NETOPEN");
				} 
				else if(net_status == 2)
				{
					continue;
				}
				else
					break;
			}
			Log_d("pdp idx = %d, net status = %d, ip address = %s", pdp_idx, net_status, ip_addr);
        }
		at_delayms(2000);
        /* set active PDP context's profile number */
        //AT_SEND_CMD(client, resp, "AT+CSOCKSETPN=1");

#ifdef USING_RTC
		{
			/* get real time */
			volatile int year, month, day, hour, min, sec;
			char str_timezone[5] = {0};
			volatile int timezone = 0;
			
			at_obj_exec_cmd(resp, "AT+CLTS=1");
			at_delayms(2000);
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
				set_data_time(year, month, day, hour, min, sec, timezone);
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
        /* initialize successfully  */
        ret = QCLOUD_RET_SUCCESS;
        break;

    __exit:
        if (ret != QCLOUD_RET_SUCCESS)
        {
            /* power off the sim76xx device */
            sim7070_power_off();
            at_delayms(1000);

            Log_i("%s device initialize retry...", DEVICE_NAME);
        }
	}
	
    if (resp) {
        at_delete_resp(resp);
    }
	
    return ret;
}

static int sim7070_deinit(void)
{
    /* power off the sim76xx device */
	sim7070_power_off();

	Log_i("%s device deinit", DEVICE_NAME);
	return QCLOUD_RET_SUCCESS;
}

static int sim7070_close(int fd)
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

static int sim7070_connect(const char *ip, uint16_t port, eNetProto proto)
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
        Log_e("sim7070 support max %d chain", SIM7070_MAX_SOCKET_NUM);
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
        if (QCLOUD_RET_SUCCESS != sim7070_close(fd)) {
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

static int sim7070_send(int fd, const void *buff, size_t len)
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
        if (len - sent_size < SIM7070_SEND_MAX_LEN_ONCE) {
            cur_pkt_size = len - sent_size;
        } else {
            cur_pkt_size = SIM7070_SEND_MAX_LEN_ONCE;
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

static int sim7070_recv_timeout(int fd, void *buf, size_t len, uint32_t timeout)
{
#ifndef AT_OS_USED
    at_client_yeild(NULL, timeout);
#endif
    return QCLOUD_RET_SUCCESS;
}

static int sim7070_parse_domain(const char *host_name, char *host_ip, size_t host_ip_len)
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

static int sim7070_query_info(ue_info *info)
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

at_device_op_t at_ops_sim7070 = {
    .init         = sim7070_init,
	.deinit       = sim7070_deinit,
	.get_info     = sim7070_query_info,
    .connect      = sim7070_connect,
    .send         = sim7070_send,
    .recv_timeout = sim7070_recv_timeout,
    .close        = sim7070_close,
    .parse_domain = sim7070_parse_domain,
    .set_event_cb = sim7070_set_event_cb,
    .deviceName   = "sim7070",
};

int at_device_sim7070_init(void)
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

    ret = at_device_op_register(&at_ops_sim7070);
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

int at_device_sim7070_deinit(void)
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
#endif /*AT_DEVICE_SIM7070*/
