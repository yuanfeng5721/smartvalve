/*
 * rtc_wakeup.h
 *
 *  Created on: 2022年1月17日
 *      Author: boboowang
 */

#ifndef DRIVERS_INC_RTC_WAKEUP_H_
#define DRIVERS_INC_RTC_WAKEUP_H_

#include <stdint.h>
#include <time.h>
#include "cmsis_os.h"



#define DAY_SECONDS_MASK   (60*60*24)
#define MIN_TO_SECONDS(m)  (60*m)
#define S_TO_TICKS(s)      (s*configTICK_RATE_HZ)
#define LOCALTIME_TO_UNIXTIME(t, z) (t - (z)*60*60)

time_t make_local_time(char *time);
time_t make_data_time(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec, uint16_t timezone);
void set_local_time(time_t t);
int calc_wakeup_time(time_t *t, uint16_t interval_s);
int calc_wakeup_count(uint16_t interval_s);

time_t SleepAndWakeUp(uint32_t interval_s);
void enter_stop_mode(void);

#endif /* DRIVERS_INC_RTC_WAKEUP_H_ */
