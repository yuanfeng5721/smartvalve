/*
 * sensors.c
 *
 *  Created on: 2021年12月30日
 *      Author: boboowang
 */
#define LOG_TAG "SENSORS"
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "sensors.h"
#include "board.h"
#include "device_nv.h"
#include "log.h"
#include "cmsis_os.h"
#include "math.h"

#define VDDA_APPLI        ((uint32_t) 3300)    /* Value of analog voltage supply Vdda (unit: mV) */
#define RANGE_12BITS      ((uint32_t) 4095)    /* Max digital value with a full range of 12 bits */

#define DIGITAL_12BITS_TO_VOLTAGE(ADC_DATA)     \
  ( ((ADC_DATA) * VDDA_APPLI) / RANGE_12BITS)


osSemaphoreId adcDmaCompleteBinarySemHandle;

static sensors_sample_t ADCSample[SAMPLE_ADC_CHANNEL] = {
	{"moto_battery", ADC_CHANNEL_4,  0.0, 0.0},
	{"angle",        ADC_CHANNEL_5,  0.0, 0.0},
	{"back_press",   ADC_CHANNEL_6,  0.0, 0.0},
	{"front_press",  ADC_CHANNEL_7,  0.0, 0.0},
	{"sys_battery",  ADC_CHANNEL_10, 0.0, 0.0},
	{"moto_temp",    ADC_CHANNEL_11, 0.0, 0.0},
	{"sys_temp",     0xFFFFFFFF,     0.0, 0.0}
};

float sensor_data[SAMPLE_ADC_CHANNEL] = {0};
//static char *SampleChName[SAMPLEADC_CHANNEL] = {"moto_battery","angle","front_stress","rear_stress","sys_battery","sys_temp","moto_temp"};
static linear_equs_t f_press_param;
static linear_equs_t b_press_param;

static float calc_press_frome_voltage(float voltage, linear_equs_t param);

static float get_ntc_resistance(float voltage)
{
	return 10000.0*voltage/(3.3-voltage);
}

// Rt = R *EXP(B*(1/T1-1/T2))
// 这里T1和T2指的是K度即开尔文温度，K度=273.15(绝对温度)+摄氏度；其中T2=(273.15+25)
// Rt 是热敏电阻在T1温度下的阻值；
// R是热敏电阻在T2常温下的标称阻值；
// B值是热敏电阻的重要参数；
// EXP是e的n次方；
static float get_ntc_temp(float voltage)
{
	const float Rp = 10000.0; //10K
	const float T2 = (273.15+25.0);//T2
	const float Bx = 3380.0;//B
	const float Ka = 273.15;

	float Rt;
	float temp;
	Rt = get_ntc_resistance(voltage);
	//like this R=5000, T2=273.15+25,B=3470, RT=5000*EXP(3470*(1/T1-1/(273.15+25)),
	temp = Rt/Rp;
	temp = log(temp);//ln(Rt/Rp)
	temp/=Bx;//ln(Rt/Rp)/B
	temp+=(1/T2);
	temp = 1/(temp);
	temp-=Ka;
	LOGD("NTC: resitance = %.2f , temp = %.2f \r\n",Rt, temp);
	return temp;
}

static float get_sys_battery(float voltage)
{
	const float R1 = 47000.0; //100k
	const float R2 = 10000.0; //10k

	return voltage*(R1+R2)/R2;
}

static float get_usm_battery(float voltage)
{
	const float R1 = 100000.0; //100k
	const float R2 = 10000.0; //10k

	return voltage*(R1+R2)/R2;
}

static float get_font_press(float voltage)
{
	float press = 0; //0kpa

	if(voltage < 0.5)
		press = 0;
	else{
		#if 0
		press = 250.0*voltage-125.0;
	  #else
		press = calc_press_frome_voltage(voltage, f_press_param);
	  #endif
	}

	return press; //KPA
}

static float get_back_press(float voltage)
{
	float press = 0; //0kpa

	if(voltage < 0.5)
		press = 0;
	else{
		#if 0
		press = 175.0*voltage-87.5;
		#else
			press = calc_press_frome_voltage(voltage, b_press_param);
		#endif
	}

	return press; //KPA
}

static float get_angle_sensor(float voltage)
{
	float angle = 0; //
	if(voltage < 0.5)
	{
		angle = 0;
	}
	else
		angle = 22.5*voltage-11.25;

	return angle;
}

static uint8_t calc_press_sensor_slope(float max_press, float min_press, linear_equs_t * param)
{
	#define MAX_VOLTAGE 4.5
	#define MIN_VOLTAGE 0.5

	//linear_equs_t param= {0.0,0.0};

	if(param == NULL)
		return 0;

	param->sploe = (max_press - min_press)/(MAX_VOLTAGE - MIN_VOLTAGE);
	param->intercept = max_press - param->sploe * MAX_VOLTAGE;

	LOGD("%s: max_press = %f, min_press = %f, slope = %f, intercept = %f \r\n", __FUNCTION__, max_press, min_press, param->sploe, param->intercept);

	return 1;
}

static float calc_press_frome_voltage(float voltage, linear_equs_t param)
{
	float press = 0.0;

	press = param.sploe*voltage + param.intercept;

	return press;
}

static void press_init(void)
{
	uint16_t f_press_max = 0, f_press_min = 0, b_press_max = 0, b_press_min = 0;

	f_press_max = nvitem_get_int(NV_F_PRESS_MAX);
	f_press_min = nvitem_get_int(NV_F_PRESS_MIN);
	b_press_max = nvitem_get_int(NV_B_PRESS_MAX);
	b_press_min = nvitem_get_int(NV_B_PRESS_MIN);

	calc_press_sensor_slope(f_press_max,f_press_min,&f_press_param);
	calc_press_sensor_slope(b_press_max,b_press_min,&b_press_param);
}

static float sensor_convert(sample_channel_e ch, float voltage)
{
	switch(ch)
	{
		case MOTO_TEMP_CH:
			ADCSample[ch].value = get_ntc_temp(ADCSample[SYS_TEMP_CH].voltage);//DS18B20_Get_Temp(0);//CT1820_Get_Temp(0);
		break;

		case VALVE_ANGLE_CH:
			#if TEST_ADC
			ADCSample[ch].value = angle[count];
			#else
			ADCSample[ch].value = get_angle_sensor(voltage);
			#endif
		break;

		case PRESS_FRONT_CH:
			#if TEST_ADC
			ADCSample[ch].value = b_press[count];
			#else
			ADCSample[ch].value = get_font_press(voltage);
			#endif
		break;

		case PRESS_REAR_CH:
			#if TEST_ADC
			ADCSample[ch].value = f_press[count];
		  #else
			ADCSample[ch].value = get_back_press(voltage);
			#endif
		break;

		case SYS_BAT_CH:
			ADCSample[ch].value = get_sys_battery(voltage);
		break;

		case SYS_TEMP_CH:
			ADCSample[ch].value = get_ntc_temp(ADCSample[ch].voltage);
		break;

		case MOTO_BAT_CH:
			ADCSample[ch].value = get_usm_battery(voltage);
		break;

		default:
		break;
	}
	return ADCSample[ch].value;
}

sensors_sample_t* Sensors_Sample_Data(void)
{
	uint16_t data[AVERAGE_MAX_NUM][SAMPLE_ADC_CHANNEL-1] = {0};
	uint32_t i = 0, ch = 0;
	volatile uint32_t value=0;
	float vcc = 0;

	if(!adcDmaCompleteBinarySemHandle)
	{
		//first use, need create semaphore
		osSemaphoreDef(SampleADC);
		adcDmaCompleteBinarySemHandle = osSemaphoreCreate(osSemaphore(SampleADC), 1);
		osSemaphoreRelease(adcDmaCompleteBinarySemHandle);
	}

	if(adcDmaCompleteBinarySemHandle != NULL)
	{
		MX_ADC_Start((uint32_t *)&data, AVERAGE_MAX_NUM*(SAMPLE_ADC_CHANNEL-1));
		if(osSemaphoreWait(adcDmaCompleteBinarySemHandle, osWaitForever) == osOK)
		{
			//Delay_MS(1);
			for(ch=0; ch<SAMPLE_ADC_CHANNEL; ch++)
			{
				if(ch != MOTO_TEMP_CH)
				{
					value = 0;
					vcc = 0;
					for(i=0; i<AVERAGE_MAX_NUM; i++)
					{
						value = value + data[i][ch];
					}
					//vcc = value*3.3/4095/AVERAGE_MAX_NUM;
					vcc = DIGITAL_12BITS_TO_VOLTAGE(value)/AVERAGE_MAX_NUM/1000.0;
					if(ch == VALVE_ANGLE_CH || ch == PRESS_FRONT_CH || ch == PRESS_REAR_CH)  //1/2 div voltage
						vcc = ((float)((int)((vcc*2.0+0.005)*100)))/100;//vcc;
					else
						vcc = ((float)((int)((vcc+0.005)*100)))/100;//vcc;

					ADCSample[ch].voltage = vcc;
				}
				sensor_convert(ch, ADCSample[ch].voltage);
				sensor_data[ch] = ADCSample[ch].value;
				osDelay(10);
				LOGI("Read Voltage:vcc[%s] = %0.2fv , value = %.2f\r\n",ADCSample[ch].name, ADCSample[ch].voltage, ADCSample[ch].value);
			}
			#if TEST_ADC
			count = (count++)%10;
			#endif
		}
		//sensor_power(0);
	}
	return ADCSample;
}

void MX_ADC_ConvCpltCallback(void)
{
	osSemaphoreRelease(adcDmaCompleteBinarySemHandle);
	LOGI("adc conv complete! \r\n");
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adcHandle)
{
  /* Report to main program that ADC sequencer has reached its end */
	//if(adcHandle == &hadc)
	{
		HAL_ADC_Stop_DMA(adcHandle);
		MX_ADC_ConvCpltCallback();
	}
}
void Sensors_Power(bool onoff)
{
	sensors_power(onoff);
	//osDelay(1000);
}

void Sensors_param_init(void)
{
	press_init();
}

float Sensors_Angle_Get(void)
{
	sensors_sample_t *sensors_data = NULL;

	sensors_data = Sensors_Sample_Data();

	return sensors_data[VALVE_ANGLE_CH].value;
}
