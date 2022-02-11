/*
 * shell.h
 *
 *  Created on: 2022年1月28日
 *      Author: boboowang
 */

#ifndef APPLICATION_INC_SHELL_H_
#define APPLICATION_INC_SHELL_H_
#include "stdint.h"
#include "stdbool.h"
#include "string.h"


typedef bool (*pFunHook)(char *args[], uint8_t argc, uint16_t len );

typedef struct
{
	const char* const pCmdStr; //cmd
	uint8_t uExpParam; //parames number
	const char* const pHelpStr; ///help
	const pFunHook pxCmdHook; // cmd callback function
	char* pResult; //back result(string)
}cli_cmd;


void ShellTaskInit(void);

#endif /* APPLICATION_INC_SHELL_H_ */
