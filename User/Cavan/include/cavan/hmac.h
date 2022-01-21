#pragma once

/*
 * File:		hmac.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2021-12-30 13:09:59
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
#include <cavan/sha.h>

#define HMAC_KEY_SIZE		64
#define HMAC_INPUT_PAD		0x36
#define HMAC_OUTPUT_PAD		0x5C

struct cavan_hmac_context {
	struct cavan_sha_context context;
	u8 key[HMAC_KEY_SIZE];
};

void cavan_hmac_init(struct cavan_hmac_context *context, const void *key, int length);
void cavan_hmac_update(struct cavan_hmac_context *context, const void *buff, size_t length);
void cavan_hmac_finish(struct cavan_hmac_context *context, void *digest);
void cavan_hmac_check(struct cavan_hmac_context *context, const void *key, int klen, const void *buff, size_t length, void *digest);
void cavan_hmac_check_text(struct cavan_hmac_context *context, const void *key, int klen, const char *text, void *digest);

void cavan_hmac_md5_init_context(struct cavan_hmac_context *context);
void cavan_hamc_md5_init(struct cavan_hmac_context *context, const void *key, int length);
void cavan_hmac_md5_check(const void *key, int klen, const void *buff, size_t length, void *digest);
void cavan_hmac_md5_check_text(const void *key, int klen, const char *text, void *digest);

void cavan_hmac_sha1_init_context(struct cavan_hmac_context *context);
void cavan_hamc_sha1_init(struct cavan_hmac_context *context, const void *key, int length);
void cavan_hmac_sha1_check(const void *key, int klen, const void *buff, size_t length, void *digest);
void cavan_hmac_sha1_check_text(const void *key, int klen, const char *text, void *digest);

