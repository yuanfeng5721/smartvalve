/*
 * iot_event.h
 *
 *  Created on: 2022年1月24日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_IOT_EVENT_H_
#define APPLICATION_INC_IOT_EVENT_H_

typedef osStatus osEvtStatus;
typedef osEventFlagsId_t event_handle_t;

#define BIT(n)    (1<<n)
typedef enum
{
	IO_EVT_TYPE_SENSORS_COMPLETE = BIT(0),
	IO_EVT_TYPE_MOTO_COMPLETE = BIT(1),
	IO_EVT_TYPE_SEND_MSG_COMPLETE = BIT(2),
	IO_EVT_TYPE_MQTT_RECV_COMPLETE = BIT(3),
}io_evt_type_t;



osEventFlagsId_t os_event_create(void);
uint32_t os_event_set(osEventFlagsId_t ef_id, uint32_t flags);
uint32_t os_event_get(osEventFlagsId_t ef_id);
uint32_t os_event_clear(osEventFlagsId_t ef_id, uint32_t flags);
uint32_t os_event_wait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);
osEvtStatus os_event_delete(osEventFlagsId_t ef_id);

int globle_event_init(void);
event_handle_t globle_event_get_handle(void);

#define g_event_handle globle_event_get_handle()

#endif /* APPLICATION_INC_IOT_EVENT_H_ */
