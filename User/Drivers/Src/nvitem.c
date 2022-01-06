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

NvErrCode nv_item_init(void)
{
	NvErrCode ret = NV_NO_ERR;

	if(easyflash_init() != EF_NO_ERR)
		ret = NV_ENV_INIT_FAILED;

	return ret;
}

size_t nv_item_read_array(const char *key, uint8_t *value, uint32_t value_len)
{
	char buf[EF_STR_ENV_VALUE_MAX_SIZE+1] = {0};
	size_t get_size, copy_size;

	get_size = ef_get_env_blob(key, (uint8_t *)buf, EF_STR_ENV_VALUE_MAX_SIZE, NULL);

	if(get_size)
	{
		if(get_size <= value_len)
			copy_size = get_size;
		else
			copy_size = value_len;
		memcpy((void *)value, (void *)buf, copy_size);
		return copy_size;
	}
	else
		return 0;
}

uint32_t nv_item_read_int(const char *key)
{
	uint32_t data = 0;
	size_t get_size;
	get_size = ef_get_env_blob(key, (uint8_t *)&data, 4, NULL);

	if(get_size)
		return data;
	else
		return 0;
}

char *nv_item_read_string(const char *key)
{
	return ef_get_env(key);
}

size_t nv_item_write_array(const char *key, const uint8_t *value, uint32_t value_len)
{
	if(ef_set_env_blob(key, (const void *)value, value_len) == EF_NO_ERR)
		return value_len;
	else
		return 0;
}

size_t nv_item_write_int(const char *key, uint32_t value)
{
	if(ef_set_env_blob(key, (const void *)&value, 4) == EF_NO_ERR)
		return 1;
	else
		return 0;
}

size_t nv_item_write_string(const char *key, const char *value)
{
	if(ef_set_env(key, value) == EF_NO_ERR)
		return strlen(value);
	else
		return 0;
}
