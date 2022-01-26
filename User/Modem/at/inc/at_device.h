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

#ifndef _AT_DEVICE_H_
#define _AT_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "iot_import.h"
#include "at_socket_inf.h"

#define MAX_SOCKET_IPADRESS_LEN   20
#define MAX_SOCKET_DOMAIN_LEN     128

#define AT_NO_CONNECTED_FD 0xffffffff
/*
 * Type of network interface
 */

typedef enum { NETWORK_TCP = 0, NETWORK_UDP = 1/*, NETWORK_TLS = 2, NETWORK_DTLS = 3*/ } Network_Type;

/**
 * @brief Define structure for network stack
 */
//typedef struct Network Network;

typedef struct ip4_addr
{
    uint32_t addr;
} ip4_addr_t;

typedef struct sockaddr{
	int port;
	char host[MAX_SOCKET_IPADRESS_LEN];
	Network_Type type;
}sockaddr_t;

/*at device api */
int modem_init(void);
int modem_deinit(void);
int at_device_get_info(ue_info *pInfo);
int modem_ntp(time_t *t);
extern int at_device_get_rssi(void);
char *at_device_get_imei(void);

/*socket api*/
size_t  socket_read(int fd, unsigned char *data, size_t datalen, uint32_t timeout_ms);
size_t  socket_write(int fd, unsigned char *data, size_t datalen, uint32_t timeout_ms);
int socket_disconnect(int fd);
int socket_connect(const sockaddr_t *addr);
int socket_parse_domain(const char *domain, sockaddr_t *addr);

/*mqtt api*/
int mqtt_connect(const char *clientid, const char *username, const char *passwd);
int mqtt_publish(const char *topic, const void *buff, uint16_t length);
int mqtt_disconnect(void);

#ifdef __cplusplus
}
#endif
#endif /* _AT_DEVICE_H_ */
