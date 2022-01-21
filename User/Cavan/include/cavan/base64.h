#pragma once

/*
 * File:		base64.h
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

void cavan_base64_decode_table(void);
char *cavan_base64_encode(char *text, int size, const u8 *buff, int length);
char *cavan_base64_encode_text(char *text, int size, const char *buff);
u8 *cavan_base64_decode(const char *text, int length, u8 *buff, int size);
u8 *cavan_base64_decode_text(const char *text, u8 *buff, int size);

