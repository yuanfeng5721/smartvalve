

#include <stdint.h>
#include <stdbool.h>
#include "stm32l1xx_hal.h"
#include "cmsis_os.h"
#include "board.h"
#include "log.h"

/** @brief  modem power control
  * @param  onoff 1: power on; 0: power off
  * @retval None
  */
void modem_power(bool onoff)
{
	LOGD("modem_power: %d \r\n", onoff);
	GPIO_WRITE(modem_pwr_en_GPIO_Port, modem_pwr_en_Pin | modem_3v3_to_1v8_en_Pin | modem_pwr_1v8_en_Pin, (GPIO_PinState) onoff);
}

/** @brief  modem power key control
  * @param  press 1: key press; 0: key release
  * @param  ms    0xffff: key always press, other: press time, unit ms
  * @retval None
  */
void modem_pwr_key(bool press, uint16_t ms)
{
	LOGD("modem_pwr_key: press = %d, ms = %d \r\n", press, ms);

	if(press)
	{
		if(ms == 0xFFFF)
		{
			GPIO_WRITE(modem_pwr_key_GPIO_Port, modem_pwr_key_Pin, GPIO_PIN_SET);
		}
		else
		{
			GPIO_WRITE(modem_pwr_key_GPIO_Port, modem_pwr_key_Pin, GPIO_PIN_SET);
			osDelay(ms);
			GPIO_WRITE(modem_pwr_key_GPIO_Port, modem_pwr_key_Pin, GPIO_PIN_RESET);
		}
	}
	else
	{
		GPIO_WRITE(modem_pwr_key_GPIO_Port, modem_pwr_key_Pin, GPIO_PIN_RESET);
	}
}

/** @brief  modem reset
  * @param  none
  * @retval None
  */
void modem_reset(void)
{
	LOGD("modem_reset \r\n");
	GPIO_WRITE(modem_reset_GPIO_Port, modem_reset_Pin, GPIO_PIN_SET);
	osDelay(500);
	GPIO_WRITE(modem_reset_GPIO_Port, modem_reset_Pin, GPIO_PIN_RESET);
}

/** @brief  sensors power control
  * @param  onoff 1: power on; 0: power off
  * @retval None
  */
void sensors_power(bool onoff)
{
	LOGD("sensors_power: %d \r\n", onoff);
	GPIO_WRITE(sensor_pwr_en2_GPIO_Port, sensor_pwr_en2_Pin, (GPIO_PinState) onoff);
	GPIO_WRITE(sensor_pwr_en1_GPIO_Port, sensor_pwr_en1_Pin, (GPIO_PinState) onoff);
}
/******************************************************************************
 *                                platform tools
 ******************************************************************************/
void Delay_MS(uint16_t ms)
{
	HAL_Delay(ms);
}

void Delay_S(uint16_t s)
{
	HAL_Delay(s*1000);
}

void System_Reset(void)
{
	NVIC_SystemReset();
}
