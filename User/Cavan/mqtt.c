/*
 * File:		mqtt.c
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
#include <cavan/hmac.h>
#include <cavan/mqtt.h>
#include <cavan/base64.h>

#define MQTT_TIME		"2600000000"
#define MQTT_METHOD		"md5"
#define MQTT_VERSION	"2018-10-31"

char *cavan_mqtt_sign(const char *user, const char *pass, const char *device, char *sign, int size)
{
	char resource[64];
	char buff[128];
	u8 digest[16];
	u8 key[32];
	int length;
	int klen;

	snprintf(resource, sizeof(resource), "products/%s/devices/%s", user, device);
	length = snprintf(buff, sizeof(buff), MQTT_TIME "\n" MQTT_METHOD "\n%s\n" MQTT_VERSION, resource);

	klen = cavan_base64_decode_text(pass, key, sizeof(key)) - key;
	cavan_hmac_md5_check(key, klen, buff, length, digest);

	return cavan_base64_encode(sign, size, digest, sizeof(digest));
}

static char *cavan_mqtt_token_append(char *token, char *token_end, const char *name, const char *value)
{
	while (token < token_end && *name != 0) {
		*token++ = *name++;
	}

	if (token < token_end) {
		*token++ = '=';
	}

	return cavan_url_encode(value, token, token_end - token);
}

char *cavan_mqtt_token(const char *user, const char *pass, const char *device, char *token, int size)
{
	char *token_end = token + size;
	char resource[64];
	char buff[128];
	u8 digest[16];
	char sign[32];
	u8 key[32];
	int length;
	int klen;

	snprintf(resource, sizeof(resource), "products/%s/devices/%s", user, device);
	length = snprintf(buff, sizeof(buff), MQTT_TIME "\n" MQTT_METHOD "\n%s\n" MQTT_VERSION, resource);

	klen = cavan_base64_decode_text(pass, key, sizeof(key)) - key;
	cavan_hmac_md5_check(key, klen, buff, length, digest);

	cavan_base64_encode(sign, size, digest, sizeof(digest));

	token = cavan_mqtt_token_append(token, token_end, "version", MQTT_VERSION);
	if (token < token_end) {
		*token++ = '&';
	}

	token = cavan_mqtt_token_append(token, token_end, "res", resource);
	if (token < token_end) {
		*token++ = '&';
	}

	token = cavan_mqtt_token_append(token, token_end, "et", MQTT_TIME);
	if (token < token_end) {
		*token++ = '&';
	}

	token = cavan_mqtt_token_append(token, token_end, "method", MQTT_METHOD);
	if (token < token_end) {
		*token++ = '&';
	}

	return cavan_mqtt_token_append(token, token_end, "sign", sign);
}

void cavan_mqtt_append_property(cavan_json_t *json, const char *name, const char *format, ...)
{
	va_list ap;

	cavan_json_append_name(json, name);
	cavan_json_begin(json);

	cavan_json_append_name(json, "value");

	va_start(ap, format);
	cavan_json_vprintf(json, format, ap);
	va_end(ap);

	cavan_json_end(json);
}

