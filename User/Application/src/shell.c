/*
 * shell.c
 *
 *  Created on: 2022年1月28日
 *      Author: boboowang
 */
#define LOG_TAG "SHELL"
#include "log.h"
#include "board.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nvitem.h"
#include "device_nv.h"
#include "sensors.h"
#include "shell.h"
#include "cmsis_os.h"
#include "utils_timer.h"
#include "utils_ringbuff.h"

#define SHELL_UART_RECV_IRQ
#define SHELL_RING_BUFF_SIZE   512
//#define SHELL_PRINT_BUFF_SIZE  256
#define Shell_IRQHandler HAL_UART5_IrqCallback
#define shell_print(format, args...) printf(format, ##args)

//static uint8_t print_buff[SHELL_PRINT_BUFF_SIZE] = {0};
static bool g_connected = false;
sRingbuff g_shell_ringbuff;
static bool shell_inited = false;

/************************************************************************************************/
bool nv_write_cmd(char *args[], uint8_t argc, uint16_t len);
bool nv_read_cmd(char *args[], uint8_t argc, uint16_t len);
bool help_cmd(char *args[], uint8_t argc, uint16_t len);
bool sync_cmd(char *args[], uint8_t argc, uint16_t len);
bool moto_control_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_mode_cmd(char *args[], uint8_t argc, uint16_t len);
bool read_sendor_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_network_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_network_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_version_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_angle_zero(char *args[], uint8_t argc, uint16_t len);
bool set_moto_zero(char *args[], uint8_t argc, uint16_t len);
bool set_f_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_f_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_sample_interval_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_update_interval_cmd(char *args[], uint8_t argc, uint16_t len );
bool set_plus_pre_circle_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_plus_pre_circle_cmd(char *args[], uint8_t argc, uint16_t len);
bool reset_bootcount_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_z_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_z_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_p_d_cmd(char *args[], uint8_t argc, uint16_t len);
bool get_q_d_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_q_d_cmd(char *args[], uint8_t argc, uint16_t len);
bool encoder_test_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_valve_angle_cmd(char *args[], uint8_t argc, uint16_t len);
bool set_angle_cmd(char *args[], uint8_t argc, uint16_t len );
bool get_angle_cmd(char *args[], uint8_t argc, uint16_t len );
bool set_encoder_count_cmd(char *args[], uint8_t argc, uint16_t len );
bool get_encoder_count_cmd(char *args[], uint8_t argc, uint16_t len );
bool set_moto_timer_count_cmd(char *args[], uint8_t argc, uint16_t len );
bool get_moto_timer_count_cmd(char *args[], uint8_t argc, uint16_t len );
bool erase_nv_cmd(char *args[], uint8_t argc, uint16_t len );

static const cli_cmd g_cmd[]=
{
	{"set_mode",1,"set device mode", set_mode_cmd, NULL},
	{"nv_wr",2,"write nv item", nv_write_cmd, NULL},
	{"nv_rd",1,"read nv item", nv_read_cmd, NULL},
	{"moto_ctl",2,"control moto", moto_control_cmd, NULL},
	{"read_sensor",1,"read sensor value", read_sendor_cmd, NULL},
//	{"set_network",1,"set network mode", set_network_cmd, NULL},
//	{"get_network",0,"get network mode", get_network_cmd, NULL},
	{"get_version",0,"get software version", get_version_cmd, NULL},
	{"set_angle_zero", 0, "set angle sensor zero point", set_angle_zero, NULL},
//	{"set_moto_zero", 0, "set moto zero point", set_moto_zero, NULL},
	{"set_F",1,"set f", set_f_cmd, NULL},
	{"get_F",0,"get f", get_f_cmd, NULL},
	{"set_sample_freq",1,"set sample freq", set_sample_interval_cmd, NULL},
	{"set_update_freq",1,"set update freq", set_update_interval_cmd, NULL},
//	{"set_plus_pre_circle",1,"set plus value for one circle", set_plus_pre_circle_cmd, NULL},
//	{"get_plus_pre_circle",0,"get plus value for one circle", get_plus_pre_circle_cmd, NULL},
	{"set_z",1,"set zeta", set_z_cmd, NULL},
	{"get_z",0,"get zeta", get_z_cmd, NULL},
	{"get_p_d",0,"get press default", get_p_d_cmd, NULL},
	{"get_q_d",0,"get Q default", get_q_d_cmd, NULL},
	{"set_q_d",0,"set Q default", set_q_d_cmd, NULL},
	{"set_a_d",1,"set opening value", set_angle_cmd, NULL},
	{"get_a_d",0,"get opening value", get_angle_cmd, NULL},
//	{"set_encoder_count",1,"set encoder count", set_encoder_count_cmd, NULL},
//	{"get_encoder_count",0,"get encoder count", get_encoder_count_cmd, NULL},
//	{"set_moto_timer_count",1,"set moto timer count", set_moto_timer_count_cmd, NULL},
//	{"get_moto_timer_count",0,"get moto timer count", get_moto_timer_count_cmd, NULL},
//	{"reset_bootcount_cmd",0,"reset bootcount", reset_bootcount_cmd, NULL},
//	{"encoder_test",1,"encoder test", encoder_test_cmd, NULL},
	{"set_valve_angle",1,"set valve angel",set_valve_angle_cmd, NULL },
	{"erase_nv",0,"erase nv items",erase_nv_cmd, NULL },
	{"help",0,"this is help", help_cmd, NULL},
	{"sync",1,"pc connect device", sync_cmd, NULL}
};

void set_connect_flag(bool flag)
{
	g_connected = flag;
}

bool get_connect_flag(void)
{
	return g_connected;
}

//void shell_print(const char *format, ...)
//{
//	//size_t length = 0;
//	va_list ap;
//
//	va_start(ap, format);
//	printf(format, ap);
//	//length = vsnprintf(print_buff, SHELL_PRINT_BUFF_SIZE, format, ap);
//	va_end(ap);
//
//	//HAL_UART_Transmit(&huart5, print_buff, length, length * 10);
//}

bool sync_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char state;

	state = args[0][0];

	shell_print("%s: state=%c\r\n",__FUNCTION__,state);

	//c is connect, d is disconnect
	if(state == 'c')
		set_connect_flag(true);
	else
		set_connect_flag(false);

	shell_print("g_connected=%d\r\n",get_connect_flag());
	shell_print("\r\nok\r\n");
	return true;
}

bool nv_write_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *key = NULL, *value = NULL;

	key = args[0];
	value = args[1];

	shell_print("%s: key=%s, value=%s\r\n",__FUNCTION__, key, value);

	if(key == NULL || value == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		ef_set_and_save_env(key, value);
		shell_print("\r\nok\r\n");
	}

	return true;
}

bool nv_read_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *key = NULL, *value = NULL;
	key = args[0];

	shell_print("%s: key=%s\r\n",__FUNCTION__,key);

	if(key == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		value = ef_get_env(key);
		if(value == NULL)
			shell_print("\r\nfaile\r\n");
		else
		{
			shell_print("\r\n%s\r\n\r\nok\r\n", value);
		}
	}
	return true;
}

bool set_mode_cmd(char *args[], uint8_t argc, uint16_t len )
{
	BootMode mode = '0';

	shell_print("%s: \r\n",__FUNCTION__);
	mode = args[0][0] - '0';
	shell_print("%s: mode = %d\r\n",__FUNCTION__, mode);

	if(mode == CONFIG_MODE || mode == NO_ACTIVITE_MODE || mode == NORMAL_MODE)
	{
		device_set_bootmode(mode);
		shell_print("\r\nok\r\n");
		System_Reset();
	}
	else
		shell_print("\r\nfail\r\n");

	return true;
}

bool help_cmd(char *args[], uint8_t argc, uint16_t len )
{
	shell_print("%s: \r\n",__FUNCTION__);
	return true;
}

bool moto_control_cmd(char *args[], uint8_t argc, uint16_t len)
{
	char driect = 'f';
	uint32_t circleNumber = 0;

	shell_print("%s: %d\r\n",__FUNCTION__, argc);

	driect = args[0][0];
	circleNumber = atoi(args[1]);

	shell_print("driect: %c, circlenumber: %d\r\n",driect, circleNumber);

	if(circleNumber<=0 || circleNumber>150)
	{
		shell_print("circlenumber is error!!!\r\n");
		return false;
	}

#if 0
	if(driect == 'f')
		//moto_turn_number(DRIVER_FREQ, TURN_FORWARD, circleNumber);
		moto_encoder_set_angle(circleNumber, 1);
	else
		//moto_turn_number(DRIVER_FREQ, TURN_BACK, circleNumber);
		moto_encoder_set_angle(circleNumber, -1);
#endif

	shell_print("\r\nok\r\n");
	return true;
}

bool get_version_cmd(char *args[], uint8_t argc, uint16_t len)
{
	print_software_version();
	return true;
}

bool read_sendor_cmd(char *args[], uint8_t argc, uint16_t len)
{
	uint8_t ch = 0;
	char c = '0';
	sensors_sample_t *sensors_data = NULL;

	c = args[0][0];
	shell_print("%s: ch = %c\r\n",__FUNCTION__, c);
	if(c<'0' || c>'9'){
		shell_print("\r\nfaile\r\n");
		return false;
	}
	ch = c - '0';
	Sensors_Power(true);
	osDelay(500);
	sensors_data = Sensors_Sample_Data();
	Sensors_Power(false);

	shell_print("\r\nvalue:%.2f,voltage:%.2f\r\n\r\nok\r\n",sensors_data[ch].value, sensors_data[ch].voltage);
	return true;
}

bool set_angle_zero(char *args[], uint8_t argc, uint16_t len)
{
	shell_print("%s: set angle seneor zero point\r\n",__FUNCTION__);

	shell_print("\r\nstart setting zero ....\r\n");
	Sensors_Power(true);
	osDelay(15000);
	Sensors_Power(false);

	shell_print("\r\nok\r\n");
	return true;
}

bool set_f_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *value = NULL;

	value = args[0];

	shell_print("%s: value=%s\r\n",__FUNCTION__, value);

	if(value == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		set_F(atoi(value));
		shell_print("\r\nok\r\n");
	}

	return true;
}

bool get_f_cmd(char *args[], uint8_t argc, uint16_t len )
{
	uint32_t value = 0;

	shell_print("%s\r\n",__FUNCTION__);
	{
		value = get_F();
		{
			shell_print("\r\n%d\r\n\r\nok\r\n",value);
		}
	}
	return true;
}

bool set_sample_interval_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *mode = NULL;
	uint8_t interval = 0;

	mode = args[0];
	shell_print("%s: mode = %s\r\n",__FUNCTION__, mode);

	interval = atoi(mode);
	if(interval == 5 || interval == 10 || interval == 20)
	{
		nvitem_set_int(SAMPLE_FREQ, interval);
		shell_print("\r\nok\r\n");
	}
	else
		shell_print("\r\nfaile\r\n");

	return true;
}

bool set_update_interval_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *mode = NULL;
	uint8_t interval = 0;

	mode = args[0];
	shell_print("%s: mode = %s\r\n", __FUNCTION__, mode);

	interval = atoi(mode);
	if(interval == 5 || interval == 10 || interval == 20)
	{
		nvitem_set_int(UPDATE_FREQ, interval);
		shell_print("\r\nok\r\n");
	}
	else
		shell_print("\r\nfaile\r\n");

	return true;
}

bool set_z_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *value = NULL;
	char *ptr = NULL;
	uint8_t i = 0;
	uint32_t data[ZETA_NUM] = {0};

	value = args[0];

	shell_print("%s: value=%s\r\n",__FUNCTION__, value);

	shell_print("\r\nstart seting zeta ....\r\n");

	if(value == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		i = 0;
		ptr = strtok(value, ",");
		shell_print("z[");
		while (ptr != NULL) {
			data[i] = atoi(ptr);
			shell_print("%d,",data[i]);
			ptr = strtok(NULL, ",");
			i++;
		}
		shell_print("]\r\n");
		nvitem_set_array(ZETA_KEY, data, ZETA_NUM);
		shell_print("\r\nok\r\n");
	}

	return true;
}

bool get_z_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *value;

	shell_print("%s\r\n",__FUNCTION__);
	value = nvitem_get_string(ZETA_KEY);
	shell_print("%s\r\n", value);
	shell_print("\r\nok\r\n");
	return true;
}

bool get_p_d_cmd(char *args[], uint8_t argc, uint16_t len)
{
	char *value;

	shell_print("%s\r\n",__FUNCTION__);

	value = nvitem_get_string(PB_DEFAULT_KEY);
	shell_print("%s\r\n", value);
	shell_print("\r\nok\r\n");

	return true;
}

bool set_q_d_cmd(char *args[], uint8_t argc, uint16_t len)
{
	char *value = NULL;
	char *ptr = NULL;
	uint8_t i = 0;
	uint32_t data[Q_DEFAULT_NUM] = {0};

	value = args[0];

	shell_print("%s: value=%s\r\n",__FUNCTION__, value);

	shell_print("\r\nstart seting Q ....\r\n");

	if(value == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		i = 0;
		ptr = strtok(value, ",");
		shell_print("Q[");
		while (ptr != NULL) {
			data[i] = atoi(ptr);
			shell_print("%d,",data[i]);
			ptr = strtok(NULL, ",");
			i++;
		}
		shell_print("]\r\n");
		nvitem_set_array(Q_DEFAULT_KEY, data, Q_DEFAULT_NUM);

		shell_print("\r\nok\r\n");
	}

	return true;
}

bool get_q_d_cmd(char *args[], uint8_t argc, uint16_t len)
{
	char *value;

	shell_print("%s\r\n",__FUNCTION__);
	value = nvitem_get_string(Q_DEFAULT_KEY);
	shell_print("%s\r\n", value);
	shell_print("\r\nok\r\n");
	return true;
}

bool set_valve_angle_cmd(char *args[], uint8_t argc, uint16_t len)
{
	char *value = NULL;
	uint8_t angle = 0;

	value = args[0];

	shell_print("%s: value=%s\r\n",__FUNCTION__, value);

	if(value == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		angle = atoi(value);
		//moto_ctrl_for_angle(angle);
		shell_print("\r\nok\r\n");
	}

	return true;
}

bool set_angle_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *value = NULL;
	char *ptr = NULL;
	uint8_t i = 0;
	uint32_t data[ANGLE_DEFAULT_NUM] = {0};

	value = args[0];

	shell_print("%s: value=%s\r\n",__FUNCTION__, value);

	shell_print("\r\nstart setting opening value ....\r\n");

	if(value == NULL)
		shell_print("\r\nfaile\r\n");
	else
	{
		i = 0;
		ptr = strtok(value, ",");
		shell_print("a_d[");
		while (ptr != NULL) {
			data[i] = atoi(ptr);
			shell_print("%d,",data[i]);
			ptr = strtok(NULL, ",");
			i++;
		}
		shell_print("]\r\n");
		nvitem_set_array(ANGLE_DEFAULT_KEY, data, ANGLE_DEFAULT_NUM);
		shell_print("\r\nok\r\n");
	}

	return true;
}

bool get_angle_cmd(char *args[], uint8_t argc, uint16_t len )
{
	char *value;

	shell_print("%s\r\n",__FUNCTION__);
	value = nvitem_get_string(ANGLE_DEFAULT_KEY);
	shell_print("%s\r\n", value);
	shell_print("\r\nok\r\n");
	return true;
}

bool erase_nv_cmd(char *args[], uint8_t argc, uint16_t len )
{
	EfErrCode result = EF_NO_ERR;

	shell_print("%s\r\n",__FUNCTION__);
	result = ef_port_erase(EF_START_ADDR,ENV_USER_SETTING_SIZE);
	shell_print("easyflash erase nv env resualt(%d)\r\n",result);
	if (result != EF_NO_ERR) {
		shell_print("\r\nfail\r\n");
		return false;
	}

	shell_print("\r\nOK\r\n");

	return true;
}
/**************************************************************************************************/

void shell_uart_rx_isr_cb(uint8_t *pdata, uint8_t len)
{
	if(shell_inited) {
		(void)ring_buff_push_data(&g_shell_ringbuff, pdata, len);
	}
}

void Shell_IRQHandler(UART_HandleTypeDef *huart)
{
	uint8_t ch;
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == SET) {
        ch = (uint8_t)READ_REG(huart->Instance->DR) & 0xFF;
        /*this callback for at_client*/
        shell_uart_rx_isr_cb(&ch, 1);
        //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    __HAL_UART_CLEAR_PEFLAG(huart);
}

//u16 ShellUartRead(UART_HandleTypeDef *huart, u8 *buff, u16 size, u32 timeout)
//{
//	u8 *buff_bak, *buff_end;
//	u32 ticks;
//
//	/* Check that a Rx process is not already ongoing */
//	if (huart->RxState != HAL_UART_STATE_READY) {
//		return 0;
//	}
//
//	/* Process Locked */
//	__HAL_LOCK(huart);
//
//	huart->ErrorCode = HAL_UART_ERROR_NONE;
//	huart->RxState = HAL_UART_STATE_BUSY_RX;
//	huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
//
//	huart->RxXferSize = size;
//	huart->RxXferCount = size;
//
//	/* Process Unlocked */
//	__HAL_UNLOCK(huart);
//
//	buff_bak = buff;
//	buff_end = buff + size;
//	ticks = HAL_GetTick() + timeout;
//
//	while (buff < buff_end) {
//		if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != 0) {
//			ticks = HAL_GetTick() + timeout;
//			*buff++ = huart->Instance->DR;
//		} else if (buff == buff_bak || ticks > HAL_GetTick()) {
//			osThreadYield();
//		} else {
//			break;
//		}
//	}
//
//    huart->RxState = HAL_UART_STATE_READY;
//
//	return buff - buff_bak;
//}

bool get_char(UART_HandleTypeDef *huart, char *buff, uint32_t timeout)
{
	Timer timer;

	countdown_ms(&timer, timeout);
	do {
#ifndef SHELL_UART_RECV_IRQ
		if(HAL_UART_Receive(huart, buff, 1, timeout) != HAL_OK) {
			continue;
		}
#else
        if (0 ==
            ring_buff_pop_data(&g_shell_ringbuff, (uint8_t *)buff, 1)) {  // push data to ringbuff @ AT_UART_IRQHandler
            continue;
        }
#endif
		else {
			break;
		}
	} while (!expired(&timer));

	if (expired(&timer)) {
		return false;
	}

	return true;
}

uint16_t get_line(UART_HandleTypeDef *huart, char *cmdbuf, size_t size, uint32_t timeout)
{
    int  read_len = 0;
    char ch = 0, last_ch = 0;
    bool is_full = false;
    bool  ret;

    while (1) {
        ret = get_char(huart, &ch, timeout);

        if (!ret) {
            return 0;
        }

        if (read_len < size) {
        	cmdbuf[read_len++] = ch;
        } else {
            is_full = true;
        }

        /* is newline or URC data */
        if ((ch == '\n' && last_ch == '\r')) {
            if (is_full) {
            	LOGE("cmd buffer size small!!!!!!\r\n");
                return 0;
            }
            break;
        }
        last_ch = ch;
    }
    return read_len;
}

uint16_t get_cmd(char * cmdbuf, size_t size)
{
	return get_line(&huart5, cmdbuf, size, 50);
}

bool StrComp(void * buffer,void * StrCmd)
{
    uint8_t i;
    uint8_t * ptBuf;
    uint8_t * ptCmd;

    ptBuf = (uint8_t *)buffer;
    ptCmd = (uint8_t *)StrCmd;
    for(i=0; i<255; i++)
    {
        if(ptCmd[i])
        {
            if(ptBuf[i] != ptCmd[i])return false;
        }
        else
        {
            if(i)return true;
            else return false;
        }
    }
    return false;
}
void cli_cmd_service(void)
{
	char cmd[300] = {0}, *param = NULL, *argv[5]={NULL};
	uint16_t len = 0, param_len = 0,j = 0;
	char delims[] = " ";
	char *str = NULL;

	BootMode bootmode = device_get_bootmode();

	do
	{
	//1. get cmd and parameter
		memset(cmd,0,300);
		len = get_cmd(cmd, 300);
	//2. check that the command length is correct
		if( (len < 6) || ('\r' != cmd[len-2]) || ('\n' != cmd[len-1]))
		{

		}
		else
		{
	//3. search cmd and exec cmd
			cmd[len-1] = '\0';
			cmd[len-2] = '\0';
			len = len - 2;
			LOGD("cmd >  %s \r\n",cmd);
			for(uint8_t i=0; i<sizeof(g_cmd)/sizeof(cli_cmd); i++)
			{
				j = 0;
				if(StrComp(cmd,(void *)g_cmd[i].pCmdStr) == true)
				{
					param = cmd+strlen(g_cmd[i].pCmdStr)+1;
					param_len = len-strlen(g_cmd[i].pCmdStr)-1;
					if(g_cmd[i].uExpParam > 0)
					{
						str = strtok(param, delims);
						while(str!=NULL)
						{
							argv[j] = str;
							LOGD( "arg[%d] : %s\r\n", j, argv[j]);
							j++;
							str = strtok( NULL, delims );
						}
					}

					if(((bootmode != CONFIG_MODE) && (i == 0)) || (bootmode == CONFIG_MODE)) {
						g_cmd[i].pxCmdHook(argv, g_cmd[i].uExpParam, param_len);
					}
					//send_result(g_cmd[i].pResult);
					break;
				}
			}
		}
		osDelay(200);
	}while(true);
}



#define SHELL_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId shellTaskHandle;


void StartShellTask(void const * argument)
{
	char *ringBuff = NULL;
	ringBuff = pvPortMalloc(SHELL_RING_BUFF_SIZE);
	if (NULL == ringBuff) {
		LOGE("malloc ringbuff err \r\n");
		return;
	}
	ring_buff_init(&g_shell_ringbuff, ringBuff, SHELL_RING_BUFF_SIZE);
	shell_inited = true;
	while (1) {
		cli_cmd_service();
	}
}

void ShellTaskInit(void)
{
	shell_inited = false;
	osThreadDef(shellTask, StartShellTask, SHELL_TASK_PRIORITY, 0, 512);
	shellTaskHandle = osThreadCreate(osThread(shellTask), NULL);
}
