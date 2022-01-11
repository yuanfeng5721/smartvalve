/*
 * nvitem.h
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */

#ifndef DRIVERS_INC_NVITEM_H_
#define DRIVERS_INC_NVITEM_H_

#define NV_USE_EASYFLASH

#include <string.h>

#ifdef NV_USE_EASYFLASH
#include <easyflash.h>
#else
#include "kv_api.h"
#endif
/* nv error code */
typedef enum {
    NV_NO_ERR,
    NV_DEL_ERR,
    NV_READ_ERR,
    NV_WRITE_ERR,
    NV_ENV_NAME_ERR,
    NV_ENV_NAME_EXIST,
    NV_ENV_FULL,
    NV_ENV_INIT_FAILED,
} NvErrCode;

/* nv value type */
typedef enum {
	NV_VALUE_INT,
    NV_VALUE_STRING,
    NV_VALUE_ARRAY,
} NvValueType;

#ifdef NV_USE_EASYFLASH
typedef struct _nvitem{
	NvValueType type;
    char *key;
    union {
    	char *str;
    	uint32_t data;
    	uint32_t *array;
    }body;
    size_t size;
}nvitem, *nvitem_t;
#else
typedef struct _nvitem{
    char *key;
    void *value;
    size_t size;
}nvitem, *nvitem_t;
#endif

#ifdef NV_USE_EASYFLASH
//#define DEFAULT_NV_ITEM_NUM EF_DEFAULT_ENV_ITEM
#define DEFAULT_NV_ITEM_NUM 20
#define MAKE_NV_ITEM_STR(key,value)  {NV_VALUE_STRING, key, .body.str=value, strlen(value)}
#define MAKE_NV_ITEM_INT(key,value) {NV_VALUE_INT, key, .body.data=value, 0}
#define MAKE_NV_ITEM_ARRAY(key,value,size) {NV_VALUE_ARRAY, key, .body.array=value, size}
#else
#define DEFAULT_NV_ITEM_NUM 20
#define MAKE_NV_ITEM_STR(key,value)  {key, value, strlen(value)}
#define MAKE_NV_ITEM_INT(key,value,size) {key, value, size}
#define MAKE_NV_ITEM_ARRAY(key,value,size) {key, value, size}
#endif

NvErrCode nvitem_init(uint32_t version);
NvErrCode nvitem_set_default(void);
void nvitem_print(void);
size_t nvitem_get_array(const char *key, uint32_t *value, uint32_t value_len);
uint32_t nvitem_get_int(const char *key);
char *nvitem_get_string(const char *key);

NvErrCode nvitem_set_array_raw(const char *key, const uint32_t *value, uint32_t value_len);
NvErrCode nvitem_set_int_raw(const char *key, uint32_t value);
NvErrCode nvitem_set_string_raw(const char *key, const char *value);

NvErrCode nvitem_set_array(const char *key, const uint32_t *value, uint32_t value_len);
NvErrCode nvitem_set_int(const char *key, uint32_t value);
NvErrCode nvitem_set_string(const char *key, const char *value);
NvErrCode nvitem_flush(void);
NvErrCode nvitem_del(const char *key);

#endif /* DRIVERS_INC_NVITEM_H_ */
