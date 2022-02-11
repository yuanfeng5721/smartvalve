/*
 * utils.c
 *
 *  Created on: 2022年1月1日
 *      Author: boboowang
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

//[a,b,c,d] ==> "a,b,c,d"
size_t array_to_string(char *str, uint32_t *value, size_t value_len)
{
	char *p=str;
	uint16_t i;

	if(!p)
		return 0;

	for(i=0; i<value_len-1; i++) {
		sprintf(p, "%d,", value[i]);
		p+=strlen(p);
	}
	sprintf(p, "%d", value[i]);

	return strlen(str);
}

//"a,b,c,d" ==> [a, b, c, d]
size_t string_to_array(uint32_t *value, char *str, size_t value_len)
{
	char *p;
	uint16_t i=0;

	if(!value || !str)
		return 0;

	p = strtok(str, ",");
	while((p!=NULL) && (i<value_len)) {
		value[i++] = atoi(p);
		p = strtok(NULL, ",");
	}
	return i;
}


/***********************************************************************/
