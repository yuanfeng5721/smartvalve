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

#include "iot.h"
#include "iot_import.h"

#ifdef AT_TCP_ENABLED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "at_socket_inf.h"
#include "at_device.h"
#include "utils_param_check.h"
#include "utils_timer.h"

uint8_t device_has_initialled = 0;

extern int at_device_sim7070_init(void);
extern int at_device_ml302_init(void);

/*at device driver must realize this api which called by HAL_AT_TCP_Init*/
int at_device_init(void)
{
#ifdef AT_DEVICE_SIM7070
    return at_device_sim7070_init();
#endif

#ifdef AT_DEVICE_ML302
    return at_device_ml302_init();
#endif
}

int at_device_deinit(void)
{
#ifdef AT_DEVICE_SIM7070
	extern int at_device_sim7070_deinit(void);
	return at_device_sim7070_deinit();
#endif

#ifdef AT_DEVICE_ML302
	extern int at_device_ml302_deinit(void);
	return at_device_ml302_deinit();
#endif
}

int modem_init(void)
{
    int rc;

	if(device_has_initialled) {
		// Log_i("at device has been initialled!!!");
		return QCLOUD_RET_SUCCESS;
	}
		
    /*at device init entry: at_client init, device driver register to at_socket*/
    rc = at_device_init();
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("at device init fail,rc:%d", rc);
        device_has_initialled = 0;
        return rc;
    }

    /*do after at device init*/
    rc = at_socket_init();
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("at socket init fail,rc:%d", rc);
        device_has_initialled = 0;
    } else {
		Log_i("at socket init success,rc:%d", rc);
		device_has_initialled = 1;
	}
    return rc;
}
int modem_deinit(void)
{
	if(!device_has_initialled) {
		Log_i("at device not initialled!!!");
		return QCLOUD_RET_SUCCESS;
	}
		
	at_socket_deinit();
	at_device_deinit();
	device_has_initialled = 0;
	Log_i("at device deinit success");
	
    return QCLOUD_RET_SUCCESS;
}

int modem_get_info(ue_info *pInfo)
{
	int rc;

	rc = at_socket_get_info(pInfo);

	if(QCLOUD_RET_SUCCESS != rc) {
		Log_e("get ue info error\n");
	}

	return rc;
}

int socket_connect(const sockaddr_t *addr)
{
    POINTER_SANITY_CHECK(addr, QCLOUD_ERR_INVAL);

    int fd = at_socket_connect(addr->host, addr->port, eNET_TCP);

    if (fd < 0) {
        Log_e("fail to connect with TCP server: %s:%u", STRING_PTR_PRINT_SANITY_CHECK(addr->host), addr->port);
        fd = AT_NO_CONNECTED_FD;
        //return -1;
    } else {
        Log_d("connected with TCP server: %s:%u", addr->host, addr->port);
        //pNetwork->handle = fd;
        //return 0;
    }
	
	return fd;
}

size_t socket_read(int fd, unsigned char *data, size_t datalen, uint32_t timeout_ms)
{
    int      ret, err_code;
    size_t  len_recv;
    Timer    timer;
	//size_t read_len;
	
    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);

    len_recv = 0;
    err_code = 0;

    do {
        if (expired(&timer)) {
            err_code = QCLOUD_ERR_TCP_READ_TIMEOUT;
            break;
        }

        ret = at_socket_recv(fd, data + len_recv, datalen - len_recv);

        if (ret > 0) {
            len_recv += ret;
        } else if (ret == 0) {
            err_code = QCLOUD_ERR_TCP_NOTHING_TO_READ;
        } else {  // ret < 0
            Log_e("recv fail\n");
            err_code = QCLOUD_ERR_TCP_READ_FAIL;
            break;
        }

    } while ((len_recv < datalen));

    if (err_code == QCLOUD_ERR_TCP_READ_TIMEOUT && len_recv == 0) {
        err_code = QCLOUD_ERR_TCP_NOTHING_TO_READ;
    }

    //read_len = len_recv;

    //return (datalen == len_recv) ? QCLOUD_RET_SUCCESS : err_code;
	return len_recv;
}

size_t socket_write(int fd, unsigned char *data, size_t datalen, uint32_t timeout_ms)
{
    int      ret;
    uint32_t len_sent;
    Timer    timer;
    int      net_err = 0;

    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);

    len_sent = 0;
    ret      = 1; /* send one time if timeout_ms is value 0 */

    do {
        ret = at_socket_send(fd, data + len_sent, datalen - len_sent);

        if (ret > 0) {
            len_sent += ret;
        } else if (0 == ret) {
            Log_e("No data be sent\n");
        } else {
            Log_e("send fail, ret:%d\n", ret);
            net_err = 1;
            break;
        }
    } while (!net_err && (len_sent < datalen) && (!expired(&timer)));

    //*written_len = (size_t)len_sent;

    //return (len_sent > 0 && net_err == 0) ? QCLOUD_RET_SUCCESS : QCLOUD_ERR_TCP_WRITE_FAIL;
	return len_sent;
}

int socket_parse_domain(const char *domain, sockaddr_t *addr)
{
    int ret;
	POINTER_SANITY_CHECK(domain, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(addr, QCLOUD_ERR_INVAL);

    ret = at_socket_parse_domain(domain, addr->host, 16);

    if (ret < 0) {
        Log_e("fail to parse domain: %s", STRING_PTR_PRINT_SANITY_CHECK(domain));
        return -1;
    } else {
        Log_d("success parse domain: %s -> %s", domain, addr->host);
        return 0;
    }
}

int socket_disconnect(int fd)
{
    int rc;

    rc = at_socket_close(fd);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("socket close error\n");
    }

    return rc;
}

#endif
