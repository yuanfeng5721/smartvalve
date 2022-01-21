/*
 * File:		hmac.c
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
#include <cavan/hmac.h>

static inline void cavan_hmac_padding(struct cavan_hmac_context *context, u8 value)
{
	u8 buff[HMAC_KEY_SIZE];
	int index;

	for (index = 0; index < HMAC_KEY_SIZE; index++) {
		buff[index] = context->key[index] ^ value;
	}

	cavan_sha_update(&context->context, buff, sizeof(buff));
}

void cavan_hmac_init(struct cavan_hmac_context *context, const void *key, int length)
{
	if (length > HMAC_KEY_SIZE) {
		cavan_sha_check(&context->context, key, length, context->key);
		length = context->context.digest_size;
	} else {
		memcpy(context->key, key, length);
	}

	memset(context->key + length, 0x00, sizeof(context->key) - length);

	cavan_sha_init(&context->context);
	cavan_hmac_padding(context, HMAC_INPUT_PAD);
}

void cavan_hmac_update(struct cavan_hmac_context *context, const void *buff, size_t length)
{
	cavan_sha_update(&context->context, buff, length);
}

void cavan_hmac_finish(struct cavan_hmac_context *context, void *digest)
{
	cavan_sha_finish(&context->context, digest);

	cavan_sha_init(&context->context);
	cavan_hmac_padding(context, HMAC_OUTPUT_PAD);
	cavan_sha_update(&context->context, digest, context->context.digest_size);
	cavan_sha_finish(&context->context, digest);
}

void cavan_hmac_check(struct cavan_hmac_context *context, const void *key, int klen, const void *buff, size_t length, void *digest)
{
	cavan_hmac_init(context, key, klen);
	cavan_hmac_update(context, buff, length);
	cavan_hmac_finish(context, digest);
}

void cavan_hmac_check_text(struct cavan_hmac_context *context, const void *key, int klen, const char *text, void *digest)
{
	cavan_hmac_check(context, key, klen, text, strlen(text), digest);
}

// ================================================================================

void cavan_hmac_md5_init_context(struct cavan_hmac_context *context)
{
	cavan_md5_init_context(&context->context);
}

void cavan_hamc_md5_init(struct cavan_hmac_context *context, const void *key, int length)
{
	cavan_hmac_md5_init_context(context);
	cavan_hmac_init(context, key, length);
}

void cavan_hmac_md5_check(const void *key, int klen, const void *buff, size_t length, void *digest)
{
	struct cavan_hmac_context context;
	cavan_hmac_md5_init_context(&context);
	cavan_hmac_check(&context, key, klen, buff, length, digest);
}

void cavan_hmac_md5_check_text(const void *key, int klen, const char *text, void *digest)
{
	cavan_hmac_md5_check(key, klen, text, strlen(text), digest);
}

// ================================================================================

void cavan_hmac_sha1_init_context(struct cavan_hmac_context *context)
{
	cavan_sha1_init_context(&context->context);
}

void cavan_hamc_sha1_init(struct cavan_hmac_context *context, const void *key, int length)
{
	cavan_hmac_sha1_init_context(context);
	cavan_hmac_init(context, key, length);
}

void cavan_hmac_sha1_check(const void *key, int klen, const void *buff, size_t length, void *digest)
{
	struct cavan_hmac_context context;
	cavan_hmac_sha1_init_context(&context);
	cavan_hmac_check(&context, key, klen, buff, length, digest);
}

void cavan_hmac_sha1_check_text(const void *key, int klen, const char *text, void *digest)
{
	cavan_hmac_sha1_check(key, klen, text, strlen(text), digest);
}

