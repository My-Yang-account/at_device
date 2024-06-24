/**
 ******************************************************************************
 * @file mw_time.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_TIME_H_
#define MW_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/time.h>
#include <stdint.h>
#include "thaisen7102Public.h"

#ifdef APP_USING_DOUBLEGUN
#define APP_TIME_SYNC_FLAG_GUNNOA       0
#define APP_TIME_SYNC_FLAG_GUNNOB       1
#define APP_TIME_SYNC_FLAG_SCREEN       2
#else
#define APP_TIME_SYNC_FLAG_GUNNOA       0
#define APP_TIME_SYNC_FLAG_SCREEN       1
#endif /* APP_USING_DOUBLEGUN */

void mw_set_datetime_from_timestamp(uint32_t timestamp);
uint32_t mw_get_timestamp_from_date(thaisenRTCSt _date);
time_t mw_get_current_timestamp(void);

void mw_set_time_sync_flag(void);
int8_t mw_get_time_sync_flag(uint8_t gunno);
void mw_clear_time_sync_flag(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* MW_TIME_H_ */
