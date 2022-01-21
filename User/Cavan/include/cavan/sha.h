#pragma once

/*
 * File:		sha.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-28 17:50:42
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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

#define SHA_FLAG_SWAP		(1 << 0)

#define SHA1_DIGEST_SIZE	20
#define MD5_DIGEST_SIZE		16

struct cavan_sha_context {
	u64 size;
	u32 digest[8];
	u8 digest_size;
	u8 remain;
	u8 flags;

	union {
		u8 buff[64];
		u16 wbuff[32];
		u32 dwbuff[16];
	};

	void (*init)(struct cavan_sha_context *context, u32 *digest);
	void (*transform)(u32 *digest, const u32 *buff);
};

int cavan_sha_init(struct cavan_sha_context *context);
void cavan_sha_update(struct cavan_sha_context *context, const void *buff, size_t size);
int cavan_sha_update2(struct cavan_sha_context *context, int fd);
int cavan_sha_update3(struct cavan_sha_context *context, const char *pathname);
void cavan_sha_finish(struct cavan_sha_context *context, u8 *digest);
int cavan_sha_check(struct cavan_sha_context *context, const void *buff, size_t size, u8 *digest);
int cavan_file_sha_mmap(struct cavan_sha_context *context, const char *pathname, u8 *digest);
int cavan_file_sha(struct cavan_sha_context *context, const char *pathname, u8 *digest);
void cavan_sha_show_digest(struct cavan_sha_context *context, void *digest);

void cavan_md5_init_context(struct cavan_sha_context *context);
void cavan_md5_check(const void *buff, size_t length, void *digest);

void cavan_sha1_init_context(struct cavan_sha_context *context);
void cavan_sha1_check(const void *buff, size_t length, void *digest);
