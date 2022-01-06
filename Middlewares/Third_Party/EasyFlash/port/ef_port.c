/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */

#include <easyflash.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"

/* default environment variables set for user */
//static const ef_env default_env_set[EF_DEFAULT_ENV_ITEM] = {
//		{"SOFTWART_VERSION", "v1.0.0", strlen("v1.0.0")},
//};
extern const ef_env default_env_set[EF_DEFAULT_ENV_ITEM];

osMutexId p_flash_mutex;
/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);
	
    osMutexDef(env);
    p_flash_mutex = osMutexCreate(osMutex(env));

	if (!p_flash_mutex)
    {
        result = EF_ENV_INIT_FAILED;
    }

    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(size % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */
	for (; size > 0; size -= 4, addr += 4, buf++) {
		*buf = *(uint32_t *) addr;
	}
    return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;
	uint32_t erase_count = 0, i;
	HAL_StatusTypeDef flash_status;
    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */
	erase_count = size/EF_ERASE_MIN_SIZE;

	if(size%EF_ERASE_MIN_SIZE != 0)
		erase_count++;

	HAL_FLASHEx_DATAEEPROM_Unlock();

	for (i = 0; i < erase_count; i++) {
		flash_status = HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_WORD, addr + (EF_ERASE_MIN_SIZE * i));
		if (flash_status != HAL_OK) {
			result = EF_ERASE_ERR;
			break;
		}
	}

	HAL_FLASHEx_DATAEEPROM_Lock();

    return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(size % 4 == 0);
    
    /* You can add your code under here. */
    {
    	uint32_t read_data, i;
    	HAL_FLASHEx_DATAEEPROM_Unlock();
		for (i = 0; i < size; i += 4, buf++, addr += 4) {
			/* write data */
			HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, addr, *buf);
			read_data = *(uint32_t *)addr;
			/* check data */
			if (read_data != *buf) {
				result = EF_WRITE_ERR;
				break;
			}
		}
		HAL_FLASHEx_DATAEEPROM_Lock();

    }
    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    
    /* You can add your code under here. */
    osMutexWait (p_flash_mutex, osWaitForever);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    
    /* You can add your code under here. */
    osMutexRelease (p_flash_mutex);
}


/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {

#define filename(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x
#ifdef PRINT_DEBUG
#ifdef MY_PRINT_DEBUG
	va_list list;
	char buf[256]={0};
	
	va_start(list, format);
	vsnprintf(buf,sizeof(buf), format, list);
	va_end(list);

    printf("[Flash](%s:%ld):%s", filename(file), line, buf);
#else
	va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    
    va_end(args);
#endif
#endif
}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;
	char log_buf[128];
    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
	vsprintf(log_buf, format, args);

	printf(log_buf);

    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;
	char log_buf[128];
    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
	vsprintf(log_buf, format, args);

	printf(log_buf);

    va_end(args);
}
