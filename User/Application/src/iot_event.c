/*
 * iot_event.c
 *
 *  Created on: 2022年1月24日
 *      Author: boboowang
 */
#define LOG_TAG "EVNET"
#include "log.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "iot_event.h"

osEventFlagsId_t g_event_group = NULL;

osEventFlagsId_t os_event_create(void)
{
	return osEventFlagsNew();
}

uint32_t os_event_set(osEventFlagsId_t ef_id, uint32_t flags)
{
	return osEventFlagsSet (ef_id, flags);
}

uint32_t os_event_get(osEventFlagsId_t ef_id)
{
	return osEventFlagsGet(ef_id);
}

uint32_t os_event_clear(osEventFlagsId_t ef_id, uint32_t flags)
{
	return osEventFlagsClear (ef_id, flags);
}

uint32_t os_event_wait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout)
{
	return osEventFlagsWait (ef_id, flags, options, timeout);
}

osEvtStatus os_event_delete(osEventFlagsId_t ef_id)
{
	osEventFlagsDelete (ef_id);
}

int globle_event_init(void)
{
	g_event_group = os_event_create();

	if(!g_event_group)
	{
		LOGE("globle event group create error!!!!\r\n");
		return -1;
	}
	LOGE("globle event group create success!!!!\r\n");
	return 0;
}

event_handle_t globle_event_get_handle(void)
{
	return g_event_group;
}
