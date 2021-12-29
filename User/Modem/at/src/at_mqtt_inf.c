/*
 * at_mqtt_inf.c
 *
 *  Created on: 2021年12月28日
 *      Author: boboowang
 */

#include "at_mqtt_inf.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "iot.h"
#include "iot_import.h"
#include "utils_param_check.h"

/** The global array of available at */
static at_mqtt_ctx_t at_mqtt_ctxs[MAX_AT_MQTT_NUM];

/**at mqtt operation mutex */
static void *sg_at_mqtt_mutex;

/**at device driver ops*/
static at_mqtt_op_t *sg_at_mqtt_ops = NULL;

#define MAX_RECV_PKT_PER_CHAIN (10)

static at_mqtt_op_t *_at_mqtt_op_get(void)
{
    return sg_at_mqtt_ops;
}

static int _at_mqtt_ctx_free(at_mqtt_ctx_t *pCtx)
{
    POINTER_SANITY_CHECK(pCtx, QCLOUD_ERR_INVAL);

    pCtx->fd       = UNUSED_MQTT;
    //pCtx->net_type = eNET_DEFAULT;

    if (pCtx->recvpkt_list) {
        list_destroy(pCtx->recvpkt_list);
        pCtx->recvpkt_list = NULL;
    }

    if (pCtx->recv_lock) {
        HAL_MutexDestroy(pCtx->recv_lock);
        pCtx->recv_lock = NULL;
    }

    return QCLOUD_RET_SUCCESS;
}

static at_mqtt_ctx_t *_at_mqtt_ctx_alloc(void)
{
    int i;

    for (i = 0; i < MAX_AT_MQTT_NUM; i++) {
        if (at_mqtt_ctxs[i].state == eMQTT_CLOSED) {
            //at_mqtt_ctxs[i].net_type        = eNET_DEFAULT;
        	at_mqtt_ctxs[i].send_timeout_ms = AT_MQTT_SEND_TIMEOUT_MS;
        	at_mqtt_ctxs[i].recv_timeout_ms = AT_MQTT_RECV_TIMEOUT_MS;
        	at_mqtt_ctxs[i].dev_mqtt_op          = _at_mqtt_op_get();

        	at_mqtt_ctxs[i].recv_lock = HAL_MutexCreate();
            if (NULL == at_mqtt_ctxs[i].recv_lock) {
                Log_e("create recv lock fail");
                goto exit;
            }
            at_mqtt_ctxs[i].recvpkt_list = list_new();
            if (NULL != at_mqtt_ctxs[i].recvpkt_list) {
            	at_mqtt_ctxs[i].recvpkt_list->free = HAL_Free;
            } else {
                Log_e("no memory to allocate recvpkt_list");
                goto exit;
            }

            at_mqtt_ctxs[i].state = eMQTT_ALLOCED;
            return &at_mqtt_ctxs[i];
        }
    }

exit:

    if (i < MAX_AT_MQTT_NUM) {
    	_at_mqtt_ctx_free(&at_mqtt_ctxs[i]);
    }

    return NULL;
}

static at_mqtt_ctx_t *_at_mqtt_find(int fd)
{
    int i;

    for (i = 0; i < MAX_AT_MQTT_NUM; i++) {
        if (at_mqtt_ctxs[i].fd == fd) {
            return &at_mqtt_ctxs[i];
        }
    }

    return NULL;
}

/* get a block to the AT socket receive list*/
static int _at_recvpkt_put(List *rlist, const char *ptr, size_t length)
{
	at_mqtt_recv_pkt *pkt = NULL;

    if (rlist->len > MAX_RECV_PKT_PER_CHAIN) {
        Log_e("Too many recv packets wait for read");
        HAL_Free(pkt);
        return QCLOUD_ERR_FAILURE;
    }

    pkt = (at_mqtt_recv_pkt *)HAL_Malloc(sizeof(struct at_mqtt_recv_pkt));
    if (pkt == NULL) {
        Log_e("No memory for receive packet table!");
        return QCLOUD_ERR_FAILURE;
    }

    pkt->bfsz_totle = length;
    pkt->bfsz_index = 0;
    pkt->buff       = (char *)ptr;

    ListNode *node = list_node_new(pkt);
    if (NULL == node) {
        Log_e("run list_node_new is error!");
        HAL_Free(pkt);
        return QCLOUD_ERR_FAILURE;
    }

    list_rpush(rlist, node);

    return length;
}

/* get a block from AT socket receive list */
static int _at_recvpkt_get(List *pkt_list, char *buff, size_t len)
{
    ListIterator *iter;
    ListNode *    node = NULL;
    at_mqtt_recv_pkt * pkt;
    size_t        readlen = 0, page_len = 0;
    POINTER_SANITY_CHECK(buff, QCLOUD_ERR_INVAL);

    if (pkt_list->len) {
        iter = list_iterator_new(pkt_list, LIST_HEAD);
        if (NULL == iter) {
            Log_e("new listiterator fail");
            return QCLOUD_ERR_TCP_READ_FAIL;
        }

        /*traverse recv pktlist*/
        do {
            node = list_iterator_next(iter);
            if (!node) {
                break;
            }

            /*get recv packet*/
            pkt = (at_mqtt_recv_pkt *)(node->val);
            if (!pkt) {
                Log_e("pkt is invalid!");
                list_remove(pkt_list, node);
                continue;
            }

            page_len = pkt->bfsz_totle - pkt->bfsz_index;
            if (page_len >= (len - readlen)) {
                memcpy(buff + readlen, pkt->buff + pkt->bfsz_index, (len - readlen));
                pkt->bfsz_index += len - readlen;
                readlen = len;
                break;
            } else {
                memcpy(buff + readlen, pkt->buff + pkt->bfsz_index, page_len);
                readlen += page_len;

                /*delete pkt after read*/
                HAL_Free(pkt->buff);
                list_remove(pkt_list, node);
            }
        } while (1);

        list_iterator_destroy(iter);
    }

    return readlen;
}

static void _at_mqtt_recv_cb(int fd, at_mqtt_evt_t event, char *buff, size_t bfsz)
{
    POINTER_SANITY_CHECK_RTN(buff);
    at_mqtt_ctx_t *pAtMqttt;

    if (event == AT_MQTT_EVT_RECV) {
        HAL_MutexLock(sg_at_mqtt_mutex);
        pAtMqttt = _at_mqtt_find(fd + MAX_AT_MQTT_NUM);
//        if (_at_recvpkt_put(pAtMqttt->recvpkt_list, buff, bfsz) < 0) {
//            Log_e("put recv package to list fail");
//            HAL_Free(buff);
//        }
        at_mqtt_process_handle(buff, bfsz);
        HAL_MutexUnlock(sg_at_mqtt_mutex);
    }
}

static void _at_mqtt_closed_cb(int fd, at_mqtt_evt_t event, char *buff, size_t bfsz)
{
	at_mqtt_ctx_t *pAtMqttt;
	pAtMqttt = _at_mqtt_find(fd + MAX_AT_MQTT_NUM);

    if (pAtMqttt != NULL && event == AT_MQTT_EVT_CLOSED) {
        HAL_MutexLock(sg_at_mqtt_mutex);
        pAtMqttt->state = eMQTT_CLOSED;
        _at_mqtt_ctx_free(pAtMqttt);
        HAL_MutexUnlock(sg_at_mqtt_mutex);
    }
}

int at_mqtt_op_register(at_mqtt_op_t *device_op)
{
    int rc;

    if (NULL == sg_at_mqtt_ops) {
    	sg_at_mqtt_ops = device_op;
        rc               = QCLOUD_RET_SUCCESS;
    } else {
        Log_e("pre device mqtt op already register");
        rc = QCLOUD_ERR_FAILURE;
    }

    return rc;
}

int at_mqtt_init(void)
{
    int i;
    int rc = QCLOUD_RET_SUCCESS;

    for (i = 0; i < MAX_AT_MQTT_NUM; i++) {
        at_mqtt_ctxs[i].fd           = UNUSED_MQTT;
        at_mqtt_ctxs[i].state        = eMQTT_CLOSED;
        at_mqtt_ctxs[i].dev_mqtt_op       = NULL;
        at_mqtt_ctxs[i].recvpkt_list = NULL;
    }

    sg_at_mqtt_mutex = HAL_MutexCreate();
    if (sg_at_mqtt_mutex == NULL) {
        Log_e("create sg_at_mqtt_mutex fail \n");
        rc = QCLOUD_ERR_FAILURE;
    }

    if (NULL != sg_at_mqtt_ops) {
            Log_d("at mqtt %s register evnet callback \r\n",
                  (NULL == sg_at_mqtt_ops->deviceName) ? "noname" : sg_at_mqtt_ops->deviceName);
            sg_at_mqtt_ops->set_event_cb(AT_MQTT_EVT_RECV, _at_mqtt_recv_cb);
            sg_at_mqtt_ops->set_event_cb(AT_MQTT_EVT_CLOSED, _at_mqtt_closed_cb);
    }

    return rc;
}

int at_mqtt_deinit(void)
{
	int i;
    int rc = QCLOUD_RET_SUCCESS;

    if (NULL != sg_at_mqtt_ops) {
            Log_d("at mqtt %s unregister success",
                  (NULL == sg_at_mqtt_ops->deviceName) ? "noname" : sg_at_mqtt_ops->deviceName);
            sg_at_mqtt_ops->set_event_cb(AT_MQTT_EVT_RECV, NULL);
            sg_at_mqtt_ops->set_event_cb(AT_MQTT_EVT_CLOSED, NULL);
    }

	if(sg_at_mqtt_mutex)
		HAL_MutexDestroy(sg_at_mqtt_mutex);

	for (i = 0; i < MAX_AT_MQTT_NUM; i++) {
        at_mqtt_ctxs[i].fd           = UNUSED_MQTT;
        at_mqtt_ctxs[i].state        = eMQTT_CLOSED;
        at_mqtt_ctxs[i].dev_mqtt_op       = NULL;
        at_mqtt_ctxs[i].recvpkt_list = NULL;
    }
    return rc;
}


int at_mqtt_connect(const char *host, int port, const char *client_id, const char *username, const char *password)
{
    at_mqtt_ctx_t *pAtMqtt;
    int              fd;

    HAL_MutexLock(sg_at_mqtt_mutex);
    pAtMqtt = _at_mqtt_ctx_alloc();
    HAL_MutexUnlock(sg_at_mqtt_mutex);

    if ((NULL == pAtMqtt) || (NULL == pAtMqtt->dev_mqtt_op) || (NULL == pAtMqtt->dev_mqtt_op->mqtt_connect)) {
        Log_e("alloc mqtt fail");
        return QCLOUD_ERR_FAILURE;
    }

    fd = pAtMqtt->dev_mqtt_op->mqtt_connect(host, port, client_id, username, password);
    if (fd < 0) {
        Log_e("dev_mqtt_op connect fail,pls check at device driver!");
        _at_mqtt_ctx_free(pAtMqtt);
    } else {
        pAtMqtt->fd    = fd + MAX_AT_MQTT_NUM;
        pAtMqtt->state = eMQTT_CONNECTED;
    }

    return pAtMqtt->fd;
}

int at_mqtt_disconnect(int fd)
{
    at_mqtt_ctx_t *pAtMqtt;

    pAtMqtt = _at_mqtt_find(fd);
    if (NULL == pAtMqtt) {  // server close the connection
        Log_e("mqtt was closed");
        return QCLOUD_ERR_MQTT_NO_CONN;
    }

    int rc;
    if ((eMQTT_CONNECTED == pAtMqtt->state) && (NULL != pAtMqtt->dev_mqtt_op) && (NULL != pAtMqtt->dev_mqtt_op->mqtt_disconnect)) {
        rc = pAtMqtt->dev_mqtt_op->mqtt_disconnect(pAtMqtt->fd - MAX_AT_MQTT_NUM);
    } else {
        rc = QCLOUD_ERR_FAILURE;
    }
    return rc;
}

int at_mqtt_pub(int fd, const char *topic, char *data, size_t len, int packet_id, unsigned char qos)
{
	at_mqtt_ctx_t *pAtMqtt;
	int connect_id, ret;

	pAtMqtt = _at_mqtt_find(fd);
	POINTER_SANITY_CHECK(pAtMqtt, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pAtMqtt->dev_mqtt_op, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pAtMqtt->dev_mqtt_op->mqtt_pub, QCLOUD_ERR_INVAL);

	if (pAtMqtt->state != eMQTT_CONNECTED) {
		Log_e("mqtt was closed");
		return QCLOUD_ERR_MQTT_NO_CONN;
	} else {
		connect_id = pAtMqtt->fd - MAX_AT_MQTT_NUM;
		return pAtMqtt->dev_mqtt_op->mqtt_pub(connect_id, topic, data, len, packet_id, qos);
	}
}

int at_mqtt_sub(int fd, const char *topic, int packet_id, unsigned char qos)
{
	at_mqtt_ctx_t *pAtMqtt;
	int connect_id, ret;

	pAtMqtt = _at_mqtt_find(fd);
	POINTER_SANITY_CHECK(pAtMqtt, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pAtMqtt->dev_mqtt_op, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pAtMqtt->dev_mqtt_op->mqtt_sub, QCLOUD_ERR_INVAL);

	if (pAtMqtt->state != eMQTT_CONNECTED) {
		Log_e("mqtt was closed");
		return QCLOUD_ERR_MQTT_NO_CONN;
	} else {
		connect_id = pAtMqtt->fd - MAX_AT_MQTT_NUM;
		return pAtMqtt->dev_mqtt_op->mqtt_sub(connect_id, topic, packet_id, qos);
	}
}

int at_mqtt_unsub(int fd, const char *topic, int packet_id)
{
	at_mqtt_ctx_t *pAtMqtt;
	int connect_id, ret;

	pAtMqtt = _at_mqtt_find(fd);
	POINTER_SANITY_CHECK(pAtMqtt, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pAtMqtt->dev_mqtt_op, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pAtMqtt->dev_mqtt_op->mqtt_unsub, QCLOUD_ERR_INVAL);

	if (pAtMqtt->state != eMQTT_CONNECTED) {
		Log_e("mqtt was closed");
		return QCLOUD_ERR_MQTT_NO_CONN;
	} else {
		connect_id = pAtMqtt->fd - MAX_AT_MQTT_NUM;
		return pAtMqtt->dev_mqtt_op->mqtt_unsub(connect_id, topic, packet_id);
	}
}

__weak int at_mqtt_process_handle(char *buffer, size_t len)
{

}
