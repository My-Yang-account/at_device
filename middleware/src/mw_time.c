/**
 ******************************************************************************
 * @file mw_time.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include <sys/time.h>

#include "mw_time.h"
#include "string.h"
#include "app_ofsm.h"

static uint8_t s_time_sync_flag = 0;

static uint8_t if_leap_year(uint16_t year)
{
    if(year % 4) {
        return 0;
    }

    if(year % 100) {
        return 1;
    }

    if(year % 400) {
        return 0;
    }

    return 1;
}

static uint8_t get_month_days(uint16_t year, uint8_t mon)
{
    switch(mon)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
            if(if_leap_year(year)) {
                return 29;
            } else {
                return 28;
            }
        default:
            break;
    }

    return 0;
}

uint32_t mw_get_timestamp_from_date(thaisenRTCSt _date)
{
    uint32_t _timestamp = 0;
    struct tm t = { 0 };

    t.tm_year = _date.thaisenGetData.Year + 2000 - 1900;
    t.tm_mon  = _date.thaisenGetData.Month - 1;
    t.tm_mday = _date.thaisenGetData.Date;
    t.tm_hour = _date.thaisenGetTime.Hours;
    t.tm_min  = _date.thaisenGetTime.Minutes;
    t.tm_sec  = _date.thaisenGetTime.Seconds;

    _timestamp = mktime(&t);

    return _timestamp;
}

void mw_set_datetime_from_timestamp(uint32_t timestamp)
{
    uint16_t temp = 0;

    uint16_t year = 0, mon = 0;
    uint32_t total_day = 0, day_sec = 0;
    uint32_t syear = 0, smon = 0, sday = 0, shour = 0, smin = 0, ssec = 0; /* 算出来的时间为 UTC+8 时间 */

    timestamp += (3600 * 8);
    total_day = timestamp / (3600 * 24);
    day_sec = timestamp % (3600 * 24);

    shour = day_sec / 3600;
    smin = day_sec % 3600 / 60;
    ssec = day_sec % 3600 % 60;

    year = 1970;
    temp = if_leap_year(year) ? 366 : 365;
    while (total_day >= temp) {
        total_day -= temp;
        year++;
        temp = if_leap_year(year) ? 366 : 365;
    }
    syear = year;

    mon = 1;
    temp = get_month_days(year, mon);
    while (total_day >= temp) {
        total_day -= temp;
        mon++;
        temp = get_month_days(year, mon);
    }
    smon = mon;
    sday = total_day + 1;

    thaisenRTCSt t = { 0 };

    t.thaisenGetData.Year  = (uint8_t)(syear - 2000);
    t.thaisenGetData.Month = (uint8_t)(smon);
    t.thaisenGetData.Date  = (uint8_t)(sday);

    t.thaisenGetTime.Hours   = (uint8_t)(shour);
    t.thaisenGetTime.Minutes = (uint8_t)(smin);
    t.thaisenGetTime.Seconds = (uint8_t)(ssec);

    thaisenSetRTC(t);
}

time_t mw_get_current_timestamp(void)
{
    time_t timestamp;
    timestamp = time(NULL);
    return timestamp;
}

void mw_set_time_sync_flag(void)
{
    for(uint8_t count = 0; count < APP_TIME_SYNC_FLAG_SIZE; count++){
        s_time_sync_flag |= (1 <<count);
    }
}

int8_t mw_get_time_sync_flag(uint8_t index)
{
    if(index >= APP_TIME_SYNC_FLAG_SIZE){
        return 0;
    }

    if(s_time_sync_flag &(1 <<index)){
        return 1;
    }
    return 0;
}

void mw_clear_time_sync_flag(uint8_t index)
{
    if(index >= APP_TIME_SYNC_FLAG_SIZE){
        return;
    }
    s_time_sync_flag &= (~(1 <<index));
}




