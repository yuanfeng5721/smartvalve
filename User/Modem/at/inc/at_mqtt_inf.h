/*
 * at_mqtt_inf.h
 *
 *  Created on: 2021年12月28日
 *      Author: boboowang
 */

#ifndef _AT_MQTT_INF_H_
#define _AT_MQTT_INF_H_
#include <stdint.h>
#include <stdio.h>
#include "iot.h"
#include "utils_list.h"

#define UNUSED_MQTT             (-1)
#define MAX_AT_MQTT_NUM         (5)
#define AT_MQTT_SEND_TIMEOUT_MS (1000)
#define AT_MQTT_RECV_TIMEOUT_MS (1000)
#define HOST_STR_MAX_LEN        (128)

typedef enum {
	eMQTT_ALLOCED = 0,
	eMQTT_CONNECTED,
	eMQTT_CLOSED
} eMqttState;

typedef enum {
    AT_MQTT_EVT_RECV = 0,
    AT_MQTT_EVT_CLOSED,
} at_mqtt_evt_t;

/* AT mqtt receive package list structure */
typedef struct at_mqtt_recv_pkt {
    List   list;
    size_t bfsz_totle;
    size_t bfsz_index;
    char * buff;
} at_mqtt_recv_pkt;

typedef void (*at_mqtt_evt_cb_t)(int id, at_mqtt_evt_t event, char *buff, size_t bfsz);

typedef struct {
	int (*mqtt_connect)(const char *host, int port, const char *client_id, const char *username, const char *password);
    int (*mqtt_disconnect)(int connect_id);
	int (*mqtt_sub)(int connect_id, const char *topic, int packet_id, unsigned char qos);
	int (*mqtt_pub)(int connect_id, const char *topic, char *data, size_t len, int packet_id, unsigned char qos);
	int (*mqtt_unsub)(int connect_id, const char *topic, int packet_id);
    void (*set_event_cb)(at_mqtt_evt_t event, at_mqtt_evt_cb_t cb);
    char *deviceName;
} at_mqtt_op_t;


/*at mqtt context*/
typedef struct {
    int              fd; /** socket fd */
    List *           recvpkt_list;
    char             remote_url[HOST_STR_MAX_LEN];
    uint16_t         remote_port;
    uint32_t         send_timeout_ms;
    uint32_t         recv_timeout_ms;
    void *           recv_lock;
    at_mqtt_op_t *   dev_mqtt_op;
    eMqttState       state;
} at_mqtt_ctx_t;

int at_mqtt_op_register(at_mqtt_op_t *device_op);
// at mqtt api
int at_mqtt_init(void);
int at_mqtt_deinit(void);
int at_mqtt_connect(const char *host, int port, const char *client_id, const char *username, const char *password);
int at_mqtt_disconnect(int fd);
int at_mqtt_pub(int fd, const char *topic, char *data, size_t len, int packet_id, unsigned char qos);
int at_mqtt_sub(int fd, const char *topic, int packet_id, unsigned char qos);
int at_mqtt_unsub(int fd, const char *topic, int packet_id);
__weak int at_mqtt_process_handle(char *buffer, size_t len);

#endif /* _AT_MQTT_INF_H_ */
