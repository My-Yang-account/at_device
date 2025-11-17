/**
 ******************************************************************************
 * @file mw_temp.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_temp.h"
#include "chargepile_config.h"
#include "thaisen7102Public.h"
#include "app_ofsm.h"

#define GUN_TEMP_VALUE_MAX                 1000    /* 枪头温度最大值 */
#define GUN_TEMP_VALUE_MIN                 -50    /* 枪头温度最小值 */
#define GUN_TEMP_DETECT_TIME               5000   /* 枪头温度检测判定时长 */

#ifdef APP_USING_DOUBLEGUN
static uint8_t s_state[APP_SYSTEM_GUNNO_SIZE] = {TCHECK_RESULT_NORMAL, TCHECK_RESULT_NORMAL};
#else
static uint8_t s_state[APP_SYSTEM_GUNNO_SIZE] = {TCHECK_RESULT_NORMAL};
#endif /* APP_USING_DOUBLEGUN */
static uint32_t s_overtemp_time_base[APP_SYSTEM_GUNNO_SIZE] = {0};
static enum temp_check s_result[APP_SYSTEM_GUNNO_SIZE] = {TCHECK_RESULT_NORMAL};

int32_t mw_get_temp_dcp(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            return thaisen_get_DCPos_tempA();
        }else{
#ifdef APP_USING_DOUBLEGUN
            return thaisen_get_DCPos_tempB();
#endif /* APP_USING_DOUBLEGUN */
        }
    }
    return -55;    /* 当枪号不对时返回此值 */
}

int32_t mw_get_temp_dcn(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            return thaisen_get_DCCat_tempA();
        }else{
#ifdef APP_USING_DOUBLEGUN
            return thaisen_get_DCCat_tempB();
#endif /* APP_USING_DOUBLEGUN */
        }
    }
    return -55;    /* 当枪号不对时返回此值 */
}

enum temp_check mw_temperature_protect_check(uint8_t gunno, int32_t temperature)
{
    if((temperature >= GUN_TEMP_VALUE_MAX) || (temperature <= GUN_TEMP_VALUE_MIN)) {
        return TCHECK_RESULT_OVERTEMP_2; /* 系统不可能达到临界值，这样的值按照不确定处理 */
    }
    if(gunno >= (APP_SYSTEM_GUNNO_SIZE)){
        return TCHECK_RESULT_OVERTEMP_2;
    }

    if(rt_tick_get() < s_overtemp_time_base[gunno]){
        s_overtemp_time_base[gunno] = rt_tick_get();
    }

    switch(s_state[gunno]){
    case TCHECK_RESULT_NORMAL:
        if(temperature > *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_WARN, 0x00))) {
            s_state[gunno] = TCHECK_RESULT_WARNNING;
        }
        s_result[gunno] = TCHECK_RESULT_NORMAL;
        s_overtemp_time_base[gunno] = rt_tick_get();
        break;
    case TCHECK_RESULT_WARNNING:
        if(temperature > *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_SETCUR, 0x00))) {
            s_state[gunno] = TCHECK_RESULT_OVERTEMP_1;
            s_overtemp_time_base[gunno] = rt_tick_get();
        }else if(temperature > *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_WARN, 0x00))){
            if ((rt_tick_get() - s_overtemp_time_base[gunno]) > GUN_TEMP_DETECT_TIME) {
                s_state[gunno] = TCHECK_RESULT_WARNNING;
                s_result[gunno] = TCHECK_RESULT_WARNNING;
                s_overtemp_time_base[gunno] = rt_tick_get();
            }
        }else if(temperature <= *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_RECOVER, 0x00))){
            if ((rt_tick_get() - s_overtemp_time_base[gunno]) > GUN_TEMP_DETECT_TIME) {
                s_state[gunno] = TCHECK_RESULT_NORMAL;
                s_result[gunno] = TCHECK_RESULT_NORMAL;
                s_overtemp_time_base[gunno] = rt_tick_get();
            }
        }
        break;
    case TCHECK_RESULT_OVERTEMP_1:
        if(temperature > *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_STOP, 0x00))) {
            s_state[gunno] = TCHECK_RESULT_OVERTEMP_2;
            s_overtemp_time_base[gunno] = rt_tick_get();
        }else if(temperature > *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_SETCUR, 0x00))){
            if ((rt_tick_get() - s_overtemp_time_base[gunno]) > GUN_TEMP_DETECT_TIME) {
                s_state[gunno] = TCHECK_RESULT_OVERTEMP_1;
                s_result[gunno] = TCHECK_RESULT_OVERTEMP_1;
                s_overtemp_time_base[gunno] = rt_tick_get();
            }
        }else if(temperature <= *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_WARN, 0x00))){
            if ((rt_tick_get() - s_overtemp_time_base[gunno]) > GUN_TEMP_DETECT_TIME) {
                s_state[gunno] = TCHECK_RESULT_WARNNING;
                s_result[gunno] = TCHECK_RESULT_WARNNING;
                s_overtemp_time_base[gunno] = rt_tick_get();
            }
        }
        break;
    case TCHECK_RESULT_OVERTEMP_2:
        if(temperature > *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_STOP, 0x00))) {
            if ((rt_tick_get() - s_overtemp_time_base[gunno]) > GUN_TEMP_DETECT_TIME) {
                s_state[gunno] = TCHECK_RESULT_OVERTEMP_2;
                s_result[gunno] = TCHECK_RESULT_OVERTEMP_2;
                s_overtemp_time_base[gunno] = rt_tick_get();
            }
        }else if(temperature <= *((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_SETCUR, 0x00))){
            if ((rt_tick_get() - s_overtemp_time_base[gunno]) > GUN_TEMP_DETECT_TIME) {
                s_state[gunno] = TCHECK_RESULT_OVERTEMP_1;
                s_result[gunno] = TCHECK_RESULT_OVERTEMP_1;
                s_overtemp_time_base[gunno] = rt_tick_get();
            }
        }
        break;
    default:
        break;
    }

    return s_result[gunno];
}
