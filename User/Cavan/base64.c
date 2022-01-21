/*
 * File:		base64.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2021-12-30 01:32:13
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
#include <cavan/base64.h>

const char cavan_base64_encode_map[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};

const u8 cavan_base64_decode_map[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35,
	0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04,
	0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C,
	0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
	0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
	0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static inline char cavan_base64_encode_value(u8 value)
{
	return cavan_base64_encode_map[value & 0x3F];
}

static inline u8 cavan_base64_decode_value(char value)
{
	return cavan_base64_decode_map[value & 0x7F];
}

static u8 cavan_base64_decode_find_value(char ch)
{
	for (u8 value = 0; value < sizeof(cavan_base64_encode_map); value++) {
		if (cavan_base64_encode_map[value] == ch) {
			return value;
		}
	}

	return 0xFF;
}

void cavan_base64_decode_table(void)
{
	for (int ch = 0; ch < 128; ch++) {
		u8 value = cavan_base64_decode_find_value(ch);

		printf("0x%02X, ", value);

		if (ch % 10 == 9) {
			putchar('\n');
		}
	}

	putchar('\n');
}

char *cavan_base64_encode(char *text, int size, const u8 *buff, int length)
{
	const u8 *buff_end = buff + length - 2;
	char *text_end = text + size - 3;

	while (buff < buff_end && text < text_end) {
		u8 value0 = *buff++;
		u8 value1 = *buff++;
		u8 value2 = *buff++;

		*text++ = cavan_base64_encode_value(value0 >> 2);
		*text++ = cavan_base64_encode_value(value0 << 4 | value1 >> 4);
		*text++ = cavan_base64_encode_value(value1 << 2 | value2 >> 6);
		*text++ = cavan_base64_encode_value(value2);
	}

	buff_end += 2;

	if (buff < buff_end && text < text_end) {
		u8 value0 = *buff++;

		*text++ = cavan_base64_encode_value(value0 >> 2);

		if (buff < buff_end) {
			u8 value1 = *buff++;
			*text++ = cavan_base64_encode_value(value0 << 4 | value1 >> 4);
			*text++ = cavan_base64_encode_value(value1 << 2);
		} else {
			*text++ = cavan_base64_encode_value(value0 << 4);
			*text++ = '=';
		}

		*text++ = '=';
	}

	if (text < text_end) {
		*text = 0;
	}

	return text;
}

char *cavan_base64_encode_text(char *text, int size, const char *buff)
{
	return cavan_base64_encode(text, size, (u8 *) buff, strlen(buff));
}

u8 *cavan_base64_decode(const char *text, int length, u8 *buff, int size)
{
	const char *text_end = text + length - 4;
	u8 *buff_end = buff + size - 2;

	while (text < text_end && buff < buff_end) {
		u8 value0 = cavan_base64_decode_value(*text++);
		u8 value1 = cavan_base64_decode_value(*text++);
		u8 value2 = cavan_base64_decode_value(*text++);
		u8 value3 = cavan_base64_decode_value(*text++);

		*buff++ = value0 << 2 | value1 >> 4;
		*buff++ = value1 << 4 | value2 >> 2;
		*buff++ = value2 << 6 | value3;
	}

	text_end += 4;
	buff_end += 2;

	while (text < text_end) {
		u8 value0, value1, value2, value3;

		value0 = cavan_base64_decode_value(*text++);
		if (value0 == 0xFF) {
			break;
		}

		if (text < text_end) {
			value1 = cavan_base64_decode_value(*text++);
			if (value1 == 0xFF) {
				break;
			}
		} else {
			break;
		}

		if (buff < buff_end) {
			*buff++ = value0 << 2 | value1 >> 4;
		} else {
			break;
		}

		if (text < text_end) {
			value2 = cavan_base64_decode_value(*text++);
			if (value2 == 0xFF) {
				break;
			}
		} else {
			break;
		}

		if (buff < buff_end) {
			*buff++ = value1 << 4 | value2 >> 2;
		} else {
			break;
		}

		if (text < text_end) {
			value3 = cavan_base64_decode_value(*text++);
			if (value3 == 0xFF) {
				break;
			}
		} else {
			break;
		}

		if (buff < buff_end) {
			*buff++ = value2 << 6 | value3;
		} else {
			break;
		}
	}

	return buff;
}

u8 *cavan_base64_decode_text(const char *text, u8 *buff, int size)
{
	return cavan_base64_decode(text, strlen(text), buff, size);
}

