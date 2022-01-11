/*
 * nvitem.c
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */


#include <easyflash.h>
#include <string.h>
#include "nvitem.h"
#include "log.h"

//const ef_env default_env_set[DEFAULT_NV_ITEM_NUM] = {
//	MAKE_NV_ITEM("SoftwareVersion", "1.0.0"),
//};
#define ENV_VALUE_MAX_LEN   900
static char env_buf[ENV_VALUE_MAX_LEN] = {0};

NvErrCode nvitem_init(uint32_t version)
{
#ifdef NV_USE_EASYFLASH
	NvErrCode ret = NV_NO_ERR;
	//ef_env_set_ver(version);

	if(easyflash_init() != EF_NO_ERR)
		ret = NV_ENV_INIT_FAILED;
#else
	int32_t ret = NV_NO_ERR;
	ret = kv_init();
	if(ret == KV_OK) {
		ret = NV_NO_ERR;
	} else {
		LOGI("nvitem init %d \r\n", ret);
		ret = NV_ENV_INIT_FAILED;
	}
#endif
	return ret;
}

NvErrCode nvitem_set_default(void)
{
extern const nvitem default_env_set[DEFAULT_NV_ITEM_NUM];
#ifdef NV_USE_EASYFLASH
	for(int i=0; i<DEFAULT_NV_ITEM_NUM; i++)
	{
		if(default_env_set[i].type == NV_VALUE_INT) {
			nvitem_set_int_raw(default_env_set[i].key, default_env_set[i].body.data);
		} else if(default_env_set[i].type == NV_VALUE_STRING) {
			nvitem_set_string_raw(default_env_set[i].key, default_env_set[i].body.str);
		} else if(default_env_set[i].type == NV_VALUE_ARRAY) {
			nvitem_set_array_raw(default_env_set[i].key, default_env_set[i].body.array, default_env_set[i].size);
		}
	}
	nvitem_flush();
#else
	for(int i=0; i<DEFAULT_NV_ITEM_NUM; i++)
	{
		kv_item_set(default_env_set[i].key, default_env_set[i].value, default_env_set[i].size);
	}
#endif
}

void nvitem_print(void)
{
	ef_print_env();
}

size_t nvitem_get_array(const char *key, uint32_t *value, uint32_t value_len)
{
#ifdef NV_USE_EASYFLASH
	char *p;

	p = ef_get_env(key);
	if(p) {
		return string_to_array(value, p, value_len);
	} else {
		return 0;
	}

#else
	size_t buffer_size = value_len, copy_size;
	if(!kv_item_get(key, (void *)value, &buffer_size))
	{
		if(buffer_size)
		{
			if(buffer_size <= value_len)
				copy_size = buffer_size;
			else
				copy_size = value_len;
			return copy_size;
		}
		else
			return 0;
	}
	return 0;
#endif
}

uint32_t nvitem_get_int(const char *key)
{
#ifdef NV_USE_EASYFLASH
	char *p;

	p = ef_get_env(key);
	if(p)
		return atoi(p);
	return 0;
#else
	uint32_t data = 0;
	size_t get_size = 4;
	if(!kv_item_get(key, (void *)&data, &get_size))
	{
		if(get_size == 1)
			return data&0xFF;
		else if(get_size == 2)
			return data&0xFFFF;
		else if(get_size == 4)
			return data&0xFFFFFFFF;
		else
			return 0;
	}
	return 0;
#endif
}

char *nvitem_get_string(const char *key)
{
#ifdef NV_USE_EASYFLASH
	return ef_get_env(key);
#else
	size_t get_size = 256;
	memset(env_buf, 0, sizeof(env_buf));
	if(!kv_item_get(key, (void *)env_buf, &get_size))
	{
		if(get_size > 0)
			return env_buf;
		else
			return NULL;
	}
	return NULL;
#endif
}

NvErrCode nvitem_set_array_raw(const char *key, const uint32_t *value, uint32_t value_len)
{
#ifdef NV_USE_EASYFLASH
	size_t str_len;
	memset(env_buf, 0, sizeof(env_buf));
	str_len = array_to_string(env_buf, value, value_len);

	if(str_len)
		return ef_set_env(key,env_buf);
	return NV_WRITE_ERR;

#else
	if(kv_item_set(key, (void *)value, value_len))
	{
		return value_len;
	}
	return 0;
#endif
}

NvErrCode nvitem_set_array(const char *key, const uint32_t *value, uint32_t value_len)
{
#ifdef NV_USE_EASYFLASH
	size_t str_len;
	memset(env_buf, 0, sizeof(env_buf));
	str_len = array_to_string(env_buf, value, value_len);

	if(str_len)
		return ef_set_and_save_env(key,env_buf);
	return NV_WRITE_ERR;

#else
	if(kv_item_set(key, (void *)value, value_len))
	{
		return value_len;
	}
	return 0;
#endif
}

NvErrCode nvitem_set_int_raw(const char *key, uint32_t value)
{
#ifdef NV_USE_EASYFLASH
	char *p;
	size_t str_len;
	memset(env_buf, 0, sizeof(env_buf));
	//p = utoa(value, env_buf, 10);
	str_len = sprintf(env_buf, "%d", value);

	if(str_len)
		return ef_set_env(key,env_buf);

	return NV_WRITE_ERR;
#else
	if(kv_item_set(key, (void *)&value, sizeof(value)))
	{
		return sizeof(value);
	}
	return 0;
#endif
}

NvErrCode nvitem_set_int(const char *key, uint32_t value)
{
#ifdef NV_USE_EASYFLASH
	size_t str_len;
	char *p;

	memset(env_buf, 0, sizeof(env_buf));
	//p = utoa(value, env_buf, 10);
	str_len = sprintf(env_buf, "%d", value);

	if(str_len)
		return ef_set_and_save_env(key,env_buf);
	return NV_WRITE_ERR;
#else
	if(kv_item_set(key, (void *)&value, sizeof(value)))
	{
		return sizeof(value);
	}
	return 0;
#endif
}

NvErrCode nvitem_set_string_raw(const char *key, const char *value)
{
#ifdef NV_USE_EASYFLASH
	return ef_set_env(key,value);
#else
	if(kv_item_set(key, (void *)value, strlen(value)))
	{
		return strlen(value);
	}
	return 0;
#endif
}

NvErrCode nvitem_set_string(const char *key, const char *value)
{
#ifdef NV_USE_EASYFLASH
	return ef_set_and_save_env(key,value);
#else
	if(kv_item_set(key, (void *)value, strlen(value)))
	{
		return strlen(value);
	}
	return 0;
#endif
}

NvErrCode nvitem_flush(void)
{
	return ef_save_env();
}

NvErrCode nvitem_del(const char *key)
{
#ifdef NV_USE_EASYFLASH
	return ef_set_and_save_env(key,NULL);
#else
	return kv_item_delete(key);
#endif
}
