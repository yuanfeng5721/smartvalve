#pragma once

/*
 * File:		mqtt.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2021-12-30 12:52:43
 *
 * Copyright (c) 2021 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/json.h>

char *cavan_mqtt_sign(const char *user, const char *pass, const char *device, char *sign, int size);
char *cavan_mqtt_token(const char *user, const char *pass, const char *device, char *token, int size);

void cavan_mqtt_append_property(cavan_json_t *json, const char *name, const char *format, ...);

static inline void cavan_mqtt_append_int(cavan_json_t *json, const char *name, int value)
{
	cavan_mqtt_append_property(json, name, "%d", value);
}

static inline void cavan_mqtt_append_float(cavan_json_t *json, const char *name, float value)
{
	cavan_mqtt_append_property(json, name, "%f", value);
}

static inline void cavan_mqtt_append_text(cavan_json_t *json, const char *name, const char *value)
{
	cavan_mqtt_append_property(json, name, "\"%s\"", value);
}

static inline void cavan_mqtt_append_int2(cavan_json_t *json, const char *name, int value)
{
	cavan_mqtt_append_property2(json, name, "%d", value);
}

static inline void cavan_mqtt_append_float2(cavan_json_t *json, const char *name, float value)
{
	cavan_mqtt_append_property2(json, name, "%.2f", value);
}

static inline void cavan_mqtt_append_text2(cavan_json_t *json, const char *name, const char *value)
{
	cavan_mqtt_append_property2(json, name, "\"%s\"", value);
}

static inline void cavan_mqtt_append_dot(cavan_json_t *json)
{
	cavan_json_append(json, ',');
}
