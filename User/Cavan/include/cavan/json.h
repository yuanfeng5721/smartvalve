#pragma once

#include "cavan.h"

typedef struct {
	char buff[2048];
	u16 length;
} cavan_json_t;

void cavan_json_append(cavan_json_t *json, char value);
void cavan_json_vprintf(cavan_json_t *json, const char *format, va_list ap);
void cavan_json_printf(cavan_json_t *json, const char *format, ...);
void cavan_json_continue(cavan_json_t *json);
void cavan_json_array_append(cavan_json_t *json, const char *format, ...);

static inline void cavan_json_begin(cavan_json_t *json)
{
	cavan_json_append(json, '{');
}

static inline void cavan_json_end(cavan_json_t *json)
{
	cavan_json_append(json, '}');
}

static inline void cavan_json_array_begin(cavan_json_t *json)
{
	cavan_json_append(json, '[');
}

static inline void cavan_json_array_end(cavan_json_t *json)
{
	cavan_json_append(json, ']');
}

static inline void cavan_json_init(cavan_json_t *json)
{
	memset(json->buff,0,sizeof(json->buff));
	json->length = 0;
}

static inline void cavan_json_append_name(cavan_json_t *json, const char *name)
{
	cavan_json_continue(json);
	cavan_json_printf(json, "\"%s\":", name);
}

static inline void cavan_json_append_int(cavan_json_t *json, const char *name, int value)
{
	cavan_json_continue(json);
	cavan_json_printf(json, "\"%s\":%d", name, value);
}

static inline void cavan_json_append_float(cavan_json_t *json, const char *name, float value)
{
	cavan_json_continue(json);
	cavan_json_printf(json, "\"%s\":%f", name, value);
}

static inline void cavan_json_append_text(cavan_json_t *json, const char *name, const char *text)
{
	cavan_json_continue(json);
	cavan_json_printf(json, "\"%s\":\"%s\"", name, text);
}

static inline void cavan_json_array_append_text(cavan_json_t *json, const char *value)
{
	cavan_json_array_append(json, "\"%s\"", value);
}

