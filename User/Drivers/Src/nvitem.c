/*
 * nvitem.c
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */


#include <easyflash.h>
#include <string.h>
#include "nvitem.h"

//const ef_env default_env_set[DEFAULT_NV_ITEM_NUM] = {
//	MAKE_NV_ITEM("SoftwareVersion", "1.0.0"),
//};
static char env_buf[256] = {0};

NvErrCode nvitem_init(uint32_t version)
{
	NvErrCode ret = NV_NO_ERR;

	//ef_env_set_ver(version);

	if(easyflash_init() != EF_NO_ERR)
		ret = NV_ENV_INIT_FAILED;

	ef_print_env();
	return ret;
}

NvErrCode nvitem_set_default(void)
{
	return ef_env_set_default();
}

#if 0
size_t nvitem_get_array(const char *key, uint8_t *value, uint32_t value_len)
{
	size_t get_size, copy_size;

	get_size = ef_get_env_blob(key, (void *)value, value_len, NULL);

	if(get_size)
	{
		if(get_size <= value_len)
			copy_size = get_size;
		else
			copy_size = value_len;
		return copy_size;
	}
	else
		return 0;
}

uint32_t nvitem_get_int(const char *key)
{
	uint32_t data = 0;
	size_t get_size;
	get_size = ef_get_env_blob(key, (uint8_t *)&data, 4, NULL);

	if(get_size == 1)
		return data&0xFF;
	else if(get_size == 2)
		return data&0xFFFF;
	else if(get_size == 4)
		return data&0xFFFFFFFF;
	else
		return 0;
}
#endif
char *nvitem_get_string(const char *key)
{
//	size_t get_size;
//
//	memset(env_buf, 0, 256);
//	get_size = ef_get_env_blob(key, (void *)env_buf, 256, NULL);
//	if(!get_size)
//		return NULL;
//
//	return env_buf;
	return ef_get_env(key);
}

#if 0
size_t nvitem_set_array(const char *key, const uint8_t *value, uint32_t value_len)
{
	if(ef_set_env_blob(key, (const void *)value, value_len) == EF_NO_ERR)
		return value_len;
	else
		return 0;
}

size_t nvitem_set_int(const char *key, uint32_t value)
{
	if(ef_set_env_blob(key, (const void *)&value, 4) == EF_NO_ERR)
		return 1;
	else
		return 0;
}
#endif
size_t nvitem_set_string(const char *key, const char *value)
{
//	if(ef_set_env_blob(key, (const void *)value, strlen(value)) == EF_NO_ERR)
//		return strlen(value);
//	else
//		return 0;
	return ef_set_env(key,value);
}
