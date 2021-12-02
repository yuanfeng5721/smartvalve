

#include <stdint.h>
#include <stdbool.h>
#include "stm32l1xx_hal.h"
#include "cmsis_os.h"
#include "board.h"

/** @brief  modem power control
  * @param  onoff 1: power on; 0: power off
  * @retval None
  */
void modem_power(bool onoff)
{
	GPIO_WRITE(modem_pwr_en_GPIO_Port, modem_pwr_en_Pin, onoff?GPIO_PIN_SET:GPIO_PIN_RESET);
}

/** @brief  modem power key control
  * @param  press 1: key press; 0: key release
  * @param  ms    0xffff: key always press, other: press time, unit ms
  * @retval None
  */
void modem_pwr_key(bool press, uint16_t ms)
{
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
	GPIO_WRITE(modem_reset_GPIO_Port, modem_reset_Pin, GPIO_PIN_SET);
	osDelay(100);
	GPIO_WRITE(modem_reset_GPIO_Port, modem_reset_Pin, GPIO_PIN_RESET);
}
