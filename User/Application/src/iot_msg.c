/*
 * iot_msg.c
 *
 *  Created on: 2022年1月1日
 *      Author: boboowang
 */

#include "log.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "iot_msg.h"

osMsgStatus os_msg_create(os_msg_handle *pp_handle, uint16_t msg_num, uint16_t msg_size)
{
	const osMessageQDef_t _os_messageQ_def = { msg_num, msg_size, NULL, NULL};
	*pp_handle = osMessageCreate (&_os_messageQ_def, NULL);
	if(*pp_handle == NULL)
		return osErrorResource;
	else
		return osOK;
}

osMsgStatus os_msg_recv(os_msg_handle p_handle, io_msg_t *p_msg, uint32_t wait_ms)
{
	osEvent event;
	event = osMessageGet (p_handle, wait_ms);
	//p_msg = (io_msg_t *)event.value.p;
	memcpy((void *)p_msg, event.value.p, sizeof(io_msg_t));
	return event.status;
}

osMsgStatus os_msg_send(os_msg_handle p_handle, io_msg_t *p_msg, uint32_t wait_ms)
{
	return osMessagePut (p_handle, (uint32_t)p_msg, wait_ms);
}

