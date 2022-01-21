#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "iot_log.h"

#define CONFIG_MQTTS	1

#if CONFIG_MQTTS
#define MQTT_HOST		"studio-mqtts.heclouds.com"
#define MQTT_PORT		8883
#else
#define MQTT_HOST		"studio-mqtt.heclouds.com"
#define MQTT_PORT		1883
#endif

#define MQTT_NAME		"TEST001"//g_IMEI
#define MQTT_USER		"448403"
#define MQTT_PASS		"WIZpdLCOQv1cWYCbrKmwYzM5XeJ6ewZ9Ly0M7lrQLgs="

#define CAVAN_ENDS_WITH(text, len, suffix) \
	cavan_text_ends_with((text), (len), (suffix), sizeof(suffix) - 1)

#define CAVAN_STARTS_WITH(text, prefix) \
	cavan_text_starts_with(text, prefix)

#define println(fmt, args ...) \
	Log_d(fmt, ##args)

#define pr_err_info(fmt, args ...) \
	println(fmt, ##args)

#define pr_red_info(fmt, args ...) \
	println(fmt, ##args)

#define ROR(value, bits) \
	((value) >> (bits) | (value) << ((sizeof(value) << 3) - (bits)))

#define ROL(value, bits) \
	((value) << (bits) | (value) >> ((sizeof(value) << 3) - (bits)))

#define POINTER_ADD(pointer, offset) \
	((void *) (((byte *) (pointer)) + (offset)))

#define POINTER_SUB(pointer, offset) \
	((void *) (((byte *) (pointer)) - (offset)))

#define ADDR_ADD(addr, offset) \
	POINTER_ADD(addr, offset)

#define ADDR_SUB(addr, offset) \
	POINTER_SUB(addr, offset)

#define ADDR_SUB2(addr1, addr2) \
	((byte *) (addr1) - (byte *) (addr2))

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint8_t byte;

char *cavan_url_encode(const char *url, char *buff, int size);
bool cavan_is_space(char value);
const char *cavan_filename(const char *pathname);

char *cavan_text_trim_head(char *text);
char *cavan_text_trim_tail(char *head, char *tail);
int cavan_text_split(char *text, char sep, char *args[], int size);

bool cavan_text_starts_with(const char *text, const char *prefix);
bool cavan_text_ends_with(const char *text, u16 len0, const char *suffix, u16 len1);

