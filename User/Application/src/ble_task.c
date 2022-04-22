/*
 * ble_task.c
 *
 *  Created on: 2022年2月15日
 *      Author: boboowang
 */
#define LOG_TAG   "BLETSK"
#include "log.h"
#include "cmsis_os.h"
#include "iot_msg.h"
#include "iot_event.h"
#include "ble.h"
#include "device_nv.h"
#include "shell.h"

#define BLE_TASK_PRIORITY      (osPriorityNormal+1)

osThreadId bleTaskHandle;
osMessageQId bleQueueHandle;

extern void cli_cmd_process(char cmd, uint16_t len, BootMode bootmode);
extern const cli_cmd g_cmd[22];

void ble_cmd_service(void)
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
		len = ble_get_cmd(cmd, 300);


		if(ble_check_connect())
		{
			ble_update_time();
			if(ble_check_timeout(1000*60*5))
			{
				LOGD("ble connect timeout!\r\n");
				ble_print("ble connect timeout!\r\n");
				ble_print("BLE CONNECT TIMEOUT\r\n");
				osDelay(100);
				ble_set_powerlevel(1);
				ble_set_connect(false);
				ble_timeout_init();
			}
		}
	//2. check that the command length is correct
		if((len < 6) || ('\r' != cmd[len-2]) || ('\n' != cmd[len-1]))
		{

		}
		else
		{
			ble_timeout_init();
	//3. search cmd and exec cmd
			cmd[len-1] = '\0';
			cmd[len-2] = '\0';
			len = len - 2;
			LOGD("cmd >  %s \r\n",cmd);
			ble_print("cmd >  %s \r\n",cmd);

			if(StrComp(cmd,"BLE CONNECT") == true)
			{
				ble_set_connect(true);
				ble_print("BLE CONNECTED\r\n");
				continue;
			}
			else if(StrComp(cmd,"BLE DISCONNECT") == true)
			{
				ble_print("BLE DISCONNECTED\r\n");
				osDelay(100);
				ble_set_powerlevel(1);
				ble_set_connect(false);
				ble_timeout_init();
				continue;
			}
			//cli_cmd_process(cmd, len, bootmode);
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
		osDelay(100);
	}while(true);
}

void StartBleTask(void const * argument)
{
	//osMsgStatus status;
	//io_msg_t p_msg;

	ble_hardware_init();
	ble_resource_init();

	while (1) {
		ble_cmd_service();
		osDelay(200);
	}
}

void BleTaskInit(void)
{
	os_msg_create(&bleQueueHandle, 4, sizeof(io_msg_t));

	osThreadDef(bleTask, StartBleTask, BLE_TASK_PRIORITY, 0, 512);
	bleTaskHandle = osThreadCreate(osThread(bleTask), NULL);
}
