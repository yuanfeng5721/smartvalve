/*
 * rtc_wakeup.c
 *
 *  Created on: 2022年1月17日
 *      Author: boboowang
 */

#define LOG_TAG "WAKEUP"
#include <rtc_wakeup.h>
#include "rtc.h"
#include "time.h"
#include "log.h"
#include "iwdg.h"

time_t time (time_t *_time)
{
    struct tm ts;
	time_t time;
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	ts.tm_hour = sTime.Hours;
	ts.tm_min = sTime.Minutes;
	ts.tm_sec = sTime.Seconds;

	if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	ts.tm_wday =  sDate.WeekDay;
	ts.tm_mon = sDate.Month-1;
	ts.tm_mday = sDate.Date;
	ts.tm_year = sDate.Year + 2000-1900;

	time = mktime(&ts);

	if(_time != NULL)
    {
		*_time = time;
    }
    return time;
}

//"18/12/22,20:34:00+32"
time_t make_local_time(char *time)
{
	struct tm ts = {0};
	uint8_t yy=0,MM=0,dd=0,hh=0,mm=0,ss=0;

	LOGD("%s\r\n", time);

	sscanf(time, "%d/%d/%d,%d:%d:%d+", yy, MM, dd, hh, mm, ss);

	ts.tm_hour = hh;
	ts.tm_min = mm;
	ts.tm_sec = ss;

	ts.tm_mon = MM-1;
	ts.tm_mday = dd;
	ts.tm_year = yy+2000-1900;

	return mktime(&ts);
}

time_t make_data_time(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec, uint16_t timezone)
{
	struct tm ts = {0};
	time_t t;
	//char buf[22];

	ts.tm_hour = hour;
	ts.tm_min = min;
	ts.tm_sec = sec;

	ts.tm_mon = month-1;
	ts.tm_mday = day;
	ts.tm_year = year+2000-1900;

	//strftime(buf, 20, "%Y-%m-%d %H:%M:%S", &ts);
	//LOGD("%s:time: %s!!!!\r\n", __FUNCTION__, buf);
	//t = mktime(&ts);
	//LOGD("%s:make time: %lu!!!!\r\n", __FUNCTION__, t);
	//return t;
	return mktime(&ts);
}

void set_local_time(time_t t)
{
	time_t t1;
	t1 = time(NULL);

	LOGD("local time = %lu \r\n", t1);
	LOGD("net time = %lu \r\n", t);
	LOGD("diff time = %lu \r\n", abs(t-t1));
	if(abs(t-t1) > 2) {
		MX_RTC_Set_Time(t);
	}
}

int calc_wakeup_count(uint16_t interval_s)
{
	time_t t1,t2;
	struct tm *gt;
	uint16_t wakeupcount = 0;
	uint16_t interval_min = interval_s/60;

	t1=time(NULL);
	gt = localtime(&t1);
	gt->tm_min = ((gt->tm_min+1)/interval_min)*interval_min;
	gt->tm_sec = 0;
	t2 = mktime(gt);

	LOGD("t2 = %d, t2|DAY_SECONDS_MASK = %d \r\n", (int)t2, (int)t2%DAY_SECONDS_MASK);

	wakeupcount = ((((t2%DAY_SECONDS_MASK)==0)?(interval_min*60*((24*60)/interval_min)):(t2%DAY_SECONDS_MASK))/(interval_min*60) - 1)%((24*60)/interval_min);
	LOGD("wakeupcount = %d, wakeup interval = %d min\r\n", wakeupcount, interval_min);

	return wakeupcount;
}

int calc_wakeup_time(time_t *t, uint16_t interval_s)
{
	time_t t1,t2;
	struct tm *gt;
	uint16_t sleeptime = 0;
	char buf[22];
	uint16_t interval_min = interval_s/60;

	if(interval_s<60)
	{
		t1=time(NULL);
		t1 += interval_s;
		if(t != NULL)
			*t = t1;
		gt = localtime(&t1);
		strftime(buf, 20, "%Y-%m-%d %H:%M:%S", gt);
		LOGD("next wakeup time: %s!!!!\r\n",buf);
		sleeptime = interval_s;
	}
	else
	{
		t1=time(NULL);
		gt = localtime(&t1);
		gt->tm_min = (gt->tm_min/interval_min+1)*interval_min;
		gt->tm_sec = 0;
		t2 = mktime(gt);
		if(t != NULL)
			*t = t2;

		strftime(buf, 20, "%Y-%m-%d %H:%M:%S", gt);
		LOGD("next wakeup time: %s!!!!\r\n",buf);
		t1=time(NULL);
		sleeptime = t2-t1;
	}
	LOGD("sleep time: time=%ds\r\n",sleeptime);

	return sleeptime;
}

void SuspendTick(void)
{
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}

void ResumeTick(void)
{
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}

void enter_stop_mode(void)
{
	SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
	SysTick->CTRL  = 0;
	//SuspendTick();
	//SystemPower_Config();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
				   SysTick_CTRL_TICKINT_Msk   |
				   SysTick_CTRL_ENABLE_Msk;
	//ResumeTick();
	//System_Reinit();
}
time_t SleepAndWakeUp(uint32_t interval_s)
{
	uint16_t feeddog_count = 0, i;
	uint32_t remain_sleep_time = 0;
	uint32_t sleeptime;
	time_t next_time;

	sleeptime = calc_wakeup_time(&next_time, interval_s);

	feeddog_count = sleeptime/FEED_DOG_INTERVAL;
	remain_sleep_time = sleeptime%FEED_DOG_INTERVAL;

	LOGD("into sleep(count:%d, remain time:%d)!!!!!!\r\n", feeddog_count, remain_sleep_time);
	HAL_IWDG_Refresh(&hiwdg);
	for(i=0; i<feeddog_count; i++) {
		MX_RTC_Wakeup_Start(FEED_DOG_INTERVAL);
		enter_stop_mode();
		MX_RTC_Wakeup_Stop();
		HAL_IWDG_Refresh(&hiwdg);
	}
	if(remain_sleep_time > 0) {
		MX_RTC_Wakeup_Start(remain_sleep_time);
		enter_stop_mode();
		MX_RTC_Wakeup_Stop();
		HAL_IWDG_Refresh(&hiwdg);
	}
	SystemClock_Config();
	osDelay(300);
	LOGD("wakeup(%d)!!!!!!\r\n", i);
	return next_time;
}
