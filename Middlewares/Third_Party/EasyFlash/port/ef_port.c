/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"

/* default environment variables set for user */
//static const ef_env default_env_set[EF_DEFAULT_ENV_ITEM] = {
//		{"SOFTWART_VERSION", "v1.0.0", strlen("v1.0.0")},
//};
extern const ef_env default_env_set[EF_DEFAULT_ENV_ITEM];
static char log_buf[80];
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
    ef_log_info("default env size: %d \r\n", *default_env_size);
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
    uint8_t *buf_8 = (uint8_t *)buf;
    size_t i;
	
	//EF_ASSERT(addr % 4 == 0);
    /* You can add your code under here. */
    for (i = 0; i < size; i++, addr ++, buf_8++) {
		*buf_8 = *(uint8_t *) addr;
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
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError = 0;
	uint8_t retry = 0;
    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    //EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */
	//erase_count = size/EF_ERASE_MIN_SIZE;

	//if(size%EF_ERASE_MIN_SIZE != 0)
	//	erase_count++;
#ifdef USE_EEPROM
	HAL_FLASHEx_DATAEEPROM_Unlock();
#if 0
	for (i = 0; i < erase_count*EF_ERASE_MIN_SIZE; i+=4) {
		flash_status = HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_WORD, addr + i);
//		if (flash_status != HAL_OK) {
//			result = EF_ERASE_ERR;
//			break;
//		}
	}
#else
	for (i = 0; i < size; i++) {
		retry = 3;
		while(retry--)
		{
			flash_status = HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_BYTE, addr + i);
			if (flash_status != HAL_OK) {
				result = EF_ERASE_ERR;
				//break;
			} else {
				result = EF_NO_ERR;
			}
		}
	}
#endif
	HAL_FLASHEx_DATAEEPROM_Lock();
#else
	HAL_FLASH_Unlock();
	 /* Fill EraseInit structure*/
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = addr;
	EraseInitStruct.NbPages     = (erase_count*EF_ERASE_MIN_SIZE) / FLASH_PAGE_SIZE;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		result = EF_ERASE_ERR;
	}

	HAL_FLASH_Lock();
#endif
    return result;
}

void ef_erase_all(void)
{
	HAL_FLASHEx_DATAEEPROM_Unlock();
	for(int j=0; j<ENV_AREA_SIZE*2; j=j+4) {
		HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_WORD, EF_START_ADDR + j);
	}
	HAL_FLASHEx_DATAEEPROM_Lock();
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
    uint32_t i;
    uint8_t read_data;
    uint8_t *w_data = (uint8_t *)buf;
    uint8_t retry = 0;
    /* You can add your code under here. */
    //EF_ASSERT(addr % 4 == 0);

#ifdef USE_EEPROM
	HAL_FLASHEx_DATAEEPROM_Unlock();
	for (i = 0; i < size; i++, w_data++, addr++) {
		retry = 3;
		while(retry--)
		{
			/* write data */
			//HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_BYTE, addr);
			HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_FASTBYTE, addr, *w_data);
			read_data = *(__IO uint8_t *)addr;
			/* check data */
			if (read_data != *w_data) {
				result = EF_WRITE_ERR;
			} else {
				result = EF_NO_ERR;
				break;
			}

		}
	}
	HAL_FLASHEx_DATAEEPROM_Lock();
#else
	HAL_FLASH_Unlock();

	for (i = 0; i < size; i=i+4, buf++, addr=addr+4) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *buf);
		read_data = *(__IO uint32_t *)addr;
		/* check data */
		if (read_data != *buf) {
			result = EF_WRITE_ERR;
			break;
		}
	}

	HAL_FLASH_Lock();
#endif
    return result;
}


void flash_test(void)
{
	int i=0;
	uint32_t data = 0x12345678;
	uint32_t r_data = 0;
	data = 0x11111111;

	ef_port_erase(EF_START_ADDR,256);
	for(i=0; i<20; i++)
	{
		ef_port_write(EF_START_ADDR+i*4, &data, 4);
	}

	for(i=0; i<20; i++)
	{
		r_data = *(__IO uint32_t *)(EF_START_ADDR+i*4);
		printf("%d ", r_data);
	}
	printf("----------------");
	data = 0x22222222;
	ef_port_erase(EF_START_ADDR,256);
	for(i=0; i<20; i++)
	{
		ef_port_write(EF_START_ADDR+i*4, &data, 4);
	}

	for(i=0; i<20; i++)
	{
		r_data = *(__IO uint32_t *)(EF_START_ADDR+i*4);
		printf("%d ", r_data);
	}
	printf("----------------");
	data = 0x33333333;
	ef_port_erase(EF_START_ADDR,256);
	for(i=0; i<20; i++)
	{
		ef_port_write(EF_START_ADDR+i*4, &data, 4);
	}
	for(i=0; i<20; i++)
	{
		r_data = *(__IO uint32_t *)(EF_START_ADDR+i*4);
		printf("%d ", r_data);
	}
	printf("----------------");
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
 * set ENV version number
 */
void ef_port_set_env_ver_num(uint32_t num)
{
	//EF_ENV_VER_NUM = num;
}

/**
 * set ENV version number
 */
uint32_t ef_port_get_env_ver_num(void)
{
	return 0;
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

#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x
#ifdef PRINT_DEBUG
#ifdef MY_PRINT_DEBUG
	va_list list;
	
	va_start(list, format);
	vsnprintf(log_buf,sizeof(log_buf), format, list);
	va_end(list);

    printf("[Flash](%s:%d):%s", filename(file), line, log_buf);
#else
	va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    /* You can add your code under here. */
    printf("[EasyFlash](%s:%ld):", filename(file), line);
    printf(format, args);

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

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    vsnprintf(log_buf,sizeof(log_buf), format, args);
	printf("%s",log_buf);

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

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    vsnprintf(log_buf,sizeof(log_buf), format, args);
    printf("%s",log_buf);

    va_end(args);
}
