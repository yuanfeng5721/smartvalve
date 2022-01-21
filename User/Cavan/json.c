#include "cavan/json.h"

void cavan_json_append(cavan_json_t *json, char value)
{
	if (json->length < sizeof(json->buff)) {
		json->buff[json->length] = value;
		json->length++;
	}
}

void cavan_json_vprintf(cavan_json_t *json, const char *format, va_list ap)
{
	char *buff = json->buff + json->length;
	u16 length = sizeof(json->buff) - json->length;
	json->length += vsnprintf(buff, length, format, ap);
}

void cavan_json_printf(cavan_json_t *json, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	cavan_json_vprintf(json, format, ap);
	va_end(ap);
}

void cavan_json_continue(cavan_json_t *json)
{
	u16 length = json->length;

	if (length > 0 && json->buff[length - 1] != '{') {
		cavan_json_append(json, ',');
	}
}

void cavan_json_array_append(cavan_json_t *json, const char *format, ...)
{
	u16 length = json->length;
	va_list ap;

	if (length > 0 && json->buff[length - 1] != '[') {
		cavan_json_append(json, ',');
	}

	va_start(ap, format);
	cavan_json_vprintf(json, format, ap);
	va_end(ap);
}

