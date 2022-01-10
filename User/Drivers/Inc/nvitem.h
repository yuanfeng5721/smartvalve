/*
 * nvitem.h
 *
 *  Created on: 2022年1月6日
 *      Author: boboowang
 */

#ifndef DRIVERS_INC_NVITEM_H_
#define DRIVERS_INC_NVITEM_H_

#include <string.h>
#include <easyflash.h>
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
	NV_VALUE_BYTE,
	NV_VALUE_WORD,
    NV_VALUE_DWORD,
    NV_VALUE_STRING,
    NV_VALUE_BYTES,
} NvValueType;

#define DEFAULT_NV_ITEM_NUM EF_DEFAULT_ENV_ITEM

#define MAKE_NV_ITEM_STR(key,value)  {key, value}//{key, value, 0}
#define MAKE_NV_ITEM_INT(key,value,size) {key, value, size}
#define MAKE_NV_ITEM_ARRAY(key,value,size) {key, value, size}

NvErrCode nvitem_init(uint32_t version);
NvErrCode nvitem_set_default(void);
size_t nvitem_get_array(const char *key, uint8_t *value, uint32_t value_len);
uint32_t nvitem_get_int(const char *key);
char *nvitem_get_string(const char *key);
size_t nvitem_set_array(const char *key, const uint8_t *value, uint32_t value_len);
size_t nvitem_set_int(const char *key, uint32_t value);
size_t nvitem_set_string(const char *key, const char *value);

#endif /* DRIVERS_INC_NVITEM_H_ */
