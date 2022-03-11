/*
 * moto.c
 *
 *  Created on: 2022年2月28日
 *      Author: boboowang
 */


#define LOG_TAG "MOTO"

#include "cmsis_os.h"
#include "tim.h"
#include "moto.h"
#include "board.h"
#include "log.h"
#include "sensors.h"
#include "device_nv.h"


static uint16_t ENCODER_COUNT = 100;
static uint16_t Default_freq = 38000;

bool moto_init(void)
{
	ENCODER_COUNT = nvitem_get_int(NV_ENCODER_COUNT);
	Default_freq = nvitem_get_int(NV_MOTO_TIMER_FREQ);
	LOGD("motor init: encoder count = %d, default_freq = %d \r\n", ENCODER_COUNT, Default_freq);
	return true;
}

bool moto_setfreq(uint16_t freq)
{
	return true;
}

void moto_start(uint16_t freq, Moto_Dir dir)
{
	MX_TIM3_MOTO_Init(freq , dir);
	MX_TIM3_Start();
}

void moto_stop(void)
{
	MX_TIM3_Stop();
}

void moto_run(check_handle handle)
{

}

void moto_encoder_test(uint16_t freq, uint16_t number, Moto_Dir direct)
{
	int8_t count = 0;
	//uint16_t timeout = 0;
	uint32_t old_time,time;

	if(number <= 0 || number > 150) {
		LOGE("input number error (%d)\r\n", number);
		return;
	}
	LOGI("set freq = %d, direct = %d, number = %d!\r\n", freq, direct, number);

	old_time = time = get_ticks();

	//init encoder
	MX_Encoder_Start();
	MX_Encoder_Set_Count(direct?0:number);

	//start up moto
	moto_start(freq, (!direct)?TURN_BACKWARD:TURN_FORWARD);

	while(1)
	{
		if((time-old_time) > TIMEOUT_4S)
		{
			LOGD("MOTO ERROR(time:%d,old_time:%d)!!!!\r\n",time,old_time);
			break;
		}
		count = MX_Encoder_Get_Count();
		LOGD("moto turn around: count = %d!\r\n",count);
		if(direct>0) {
			if(count>=number) {
				moto_stop();
				break;
			}
		} else if(direct < 0) {
			if(count <= 0 ) {
				moto_stop();
				break;
			}
		}
		time = get_ticks();
	}
	moto_stop();
	MX_Encoder_Stop();
}

int16_t moto_ctrl_for_angle(uint16_t freq, float angle)
{
	#define M_ANGLE_BUF 1

	uint8_t count = 0;
	uint8_t count2 = 0;
//	uint32_t timeout = 0;
	float oldAngel = 0.0;
	float Angle = 0.0;
	float dAngle = 0.0;
	float dMAngle = 0.0;
	uint32_t old_time,time;

	LOGD("%s:start ctrl motor\r\n",__FUNCTION__);
	Sensors_Power(1);
	osDelay(1000);

	//sensor sample
	oldAngel = Sensors_Angle_Get();

	dAngle = angle - oldAngel;
	LOGD("%s:old angle = %f, new angel = %f, dAngle = %f\r\n",__FUNCTION__, oldAngel, angle, dAngle);

	if(dAngle == 0)
		return MOTO_CTL_SUCCESS;

	//init encoder
	MX_Encoder_Start();
	MX_Encoder_Set_Count((dAngle>0)?ENCODER_COUNT:0);

	old_time = time = get_ticks();
	//start moto
	moto_start(freq, (dAngle>0)?TURN_FORWARD:TURN_BACKWARD);

	while(1)
	{
		if((time-old_time) > TIMEOUT_4S)  //timeout, motor need turn back to center
		{
			LOGD("MOTOR ERROR(time:%d,old_time:%d)!!!!\r\n",time,old_time);
			moto_stop();
			count2 = MX_Encoder_Get_Count();
			LOGD("motor count = %d!!!!\r\n", count2);
			//turn back to center
			{
				old_time = time = get_ticks();
				moto_start(freq, (dAngle>0)?TURN_BACKWARD:TURN_FORWARD);
				while(1)
				{
					if((time-old_time) > TIMEOUT_4S) {
						LOGD("MOTOR ERROR(time:%d,old_time:%d)!!!!\r\n",time,old_time);
						moto_stop();
						MX_Encoder_Stop();
						break;
					}
					count = MX_Encoder_Get_Count();
					LOGD("motor turn back: count = %d!\r\n",count);

					if(dAngle < 0){
						if(count <= 0) {
							moto_stop();
							MX_Encoder_Stop();
							break;
						}
					}else{
						if(count >= ENCODER_COUNT) {
							moto_stop();
							MX_Encoder_Stop();
							break;
						}
					}
					time = get_ticks();
				}
			}
			MX_Encoder_Stop();
			return MOTO_CTL_MOTO_ERROR;
		}

		count = MX_Encoder_Get_Count();
		LOGD("motor turn around: count = %d!\r\n",count);

		if(dAngle > 0) {
			if(count <= 0) {
				moto_stop();
				break;
			}
		} else {
			if(count >= ENCODER_COUNT) {
				moto_stop();
				break;
			}
		}
		//osDelay(50);
		time = get_ticks();
	}
	//stop motor
	moto_stop();

	//Detection Angle sensor
	//sensor_power(1);
	//osDelay(1000);
	//timeout = 0;
	old_time = time = get_ticks();
	//while(timeout<60*8)
	while((time-old_time) < TIMEOUT_8M)
	{
		//sensor sample
		Angle = Sensors_Angle_Get();
		osDelay(800);
		LOGD("angle value = %f\r\n",Angle);

		if(dAngle>0)
		    dMAngle = Angle - angle + M_ANGLE_BUF;
		else
			dMAngle = angle - Angle + M_ANGLE_BUF;

		LOGD("mangle - angle = %f \r\n",dMAngle);
		if(dMAngle >= 0)
		{
			break;
		}
		//timeout++;
		time = get_ticks();
	}
	//close sensor power
	Sensors_Power(0);
	osDelay(1000);

	//close valve
	//timeout = 0;
	count = 0;

	old_time = time = get_ticks();
	//start up motor
	moto_start(freq , (dAngle>0)?TURN_BACKWARD:TURN_FORWARD);
	while(1)
	{
		if((time-old_time) > TIMEOUT_4S)
		{
			LOGD("MOTOR ERROR(time:%d,old_time:%d)!!!!\r\n",time,old_time);
			moto_stop();
			MX_Encoder_Stop();
			return MOTO_CTL_MOTO_ERROR;
		}
		count = MX_Encoder_Get_Count();
		LOGD("motor turn around: count = %d!\r\n",count);

		if(dAngle < 0) {
			if(count <= 0) {
				moto_stop();
				MX_Encoder_Stop();
				break;
			}
	    } else {
			if(count >= ENCODER_COUNT) {
				moto_stop();
				MX_Encoder_Stop();
				break;
			}
		}
		//osDelay(50);
		//timeout++;
		time = get_ticks();
	}
	//stop moto
	moto_stop();

	//stop encoder
	MX_Encoder_Stop();

	return MOTO_CTL_SUCCESS;
}

uint16_t moto_get_freq(void)
{
	return Default_freq;
}
