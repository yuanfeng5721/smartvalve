/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include "utils_timer.h"

static const uint8_t sg_day_num[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void timestamp_to_date(size_t timestamp_sec, RTC_DATE_TIME *date_time, int8_t time_zone)
{
    date_time->sec = timestamp_sec % 60;

    timestamp_sec /= 60;
    date_time->minute = timestamp_sec % 60;
    timestamp_sec += (size_t)time_zone * 60;  // time_zone add
    timestamp_sec /= 60;                      // to all hour
    date_time->hour = timestamp_sec % 24;     // calc curr hour

    int day_num = timestamp_sec / 24;  // to all day_num

    int leap_num = (day_num + 365) / 1461;  // calc leap year num

    if (((day_num + 366) % 1461) == 0) {
        date_time->year  = (day_num / 366) + 1970 - 0;
        date_time->month = 12;
        date_time->day   = 31;
    } else {
        day_num -= leap_num;  // all year to no leap
        date_time->year = (day_num / 365) + 1970 - 0;
        day_num %= 365;
        day_num += 1;
        // curr leap year
        if (((date_time->year % 4) == 0) && (day_num == 60)) {
            date_time->month = 2;
            date_time->day   = 29;
        } else {
            if (((date_time->year % 4) == 0) && (day_num > 60)) {
                day_num -= 1;  // leap year day_num to not leap year day_num
            }
            // calc month and day
            int month_index;
            for (month_index = 0; sg_day_num[month_index] < day_num; month_index++) {
                day_num -= sg_day_num[month_index];
            }
            date_time->month = (month_index + 1);
            date_time->day   = day_num;
        }
    }

    return;
}

size_t date_to_timestamp(RTC_DATE_TIME *date_time, int8_t time_zone)
{
#if 1
#define MINUTE 60        
#define HOUR (60*MINUTE)
//#define DAY (24*HOUR)
//#define YEAR (365*DAY)
	
	volatile int year, mon, day, hour, min, sec;
	
	year = date_time->year + 2000;
	mon = date_time->month;
	day = date_time->day;
	hour = date_time->hour;
	min = date_time->minute;
	sec = date_time->sec;
	
    if (0 >= (int) (mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
        mon += 12;      /* Puts Feb last since it has leap day */
        year -= 1;
    }
 
    return (((
        (unsigned long) (year/4 - year/100 + year/400 + 367*mon/12 + day) +
        year*365 - 719499
        )*24 + hour /* now have hours */
        )*60 + min /* now have minutes */
        )*60 + sec - time_zone*HOUR; /* finally seconds */
#else
	struct tm time;
	time_t timestamp;
	
	time.tm_year = date_time->year + 2000 - 1900;
	time.tm_mon = date_time->month - 1;
	time.tm_mday = date_time->day;
	time.tm_hour = date_time->hour;
	time.tm_min = date_time->minute;
	time.tm_sec = date_time->sec;
	
	timestamp = mktime(&time)+time_zone*60*60;
	return timestamp;
#endif
}

bool expired(Timer *timer)
{
    return HAL_Timer_expired(timer);
}

void countdown_ms(Timer *timer, unsigned int timeout_ms)
{
    HAL_Timer_countdown_ms(timer, timeout_ms);
}

void countdown(Timer *timer, unsigned int timeout)
{
    HAL_Timer_countdown(timer, timeout);
}

int left_ms(Timer *timer)
{
    return HAL_Timer_remain(timer);
}

void InitTimer(Timer *timer)
{
    HAL_Timer_init(timer);
}

#ifdef __cplusplus
}
#endif
