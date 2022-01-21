#ifndef __IOT_MSG_H__
#define __IOT_MSG_H__


#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

typedef osMessageQId os_msg_handle;
typedef osStatus osMsgStatus;

/**  @brief IO message definition for communications between tasks*/
typedef struct
{
    uint16_t type;
    uint16_t subtype;
    union
    {
        uint32_t  param;
        void     *buf;
    } u;
} io_msg_t;

/**  @brief IO type definitions for IO message, may extend as requested */
typedef enum
{
    IO_MSG_TYPE_TIMER,           /**< App timer message with subtype @ref T_IO_MSG_TIMER */
    IO_MSG_TYPE_RESET_WDG_TIMER, /**< reset watch dog timer*/
	IO_MSG_TYPE_READ_SENSOR,
} io_msg_type_t;

osMsgStatus os_msg_create(os_msg_handle *pp_handle, uint16_t msg_num, uint16_t msg_size);
osMsgStatus os_msg_recv(os_msg_handle p_handle, io_msg_t *p_msg, uint32_t wait_ms);
osMsgStatus os_msg_send(os_msg_handle p_handle, io_msg_t *p_msg, uint32_t wait_ms);
#endif
