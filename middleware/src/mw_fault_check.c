/**
 ******************************************************************************
 * @file mw_fault_check.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_fault_check.h"

uint32_t* mw_get_system_fault_set(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
//        rt_kprintf("mw_get_system_fault_set gunno(%d)(%x)\n", gunno, *thaisenGetSysFault(gunno));
        return thaisenGetSysFault(gunno);
    }
    return NULL;
}

uint32_t* mw_get_charge_fault_set(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
//        rt_kprintf("mw_get_charge_fault_set gunno(%d)(%x)\n", gunno, *thaisenGetChargFault(gunno));
        return thaisenGetChargFault(gunno);
    }
    return NULL;
}

uint16_t mw_get_system_stop_way(uint8_t gunno)
{
    enum system_stop_way __way;
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        uint16_t stop_way;
        __way = thaisenGetChargCtlStopWay(gunno);
        stop_way = mw_system_stop_way_convert(__way);

        rt_kprintf("gunno(%d) stop charge, reason is(%d, %d)\n", gunno, stop_way, __way);
        return stop_way;
    }
    return APP_SYSTEM_STOP_WAY_SIZE;
}


uint16_t mw_system_fault_convert(uint16_t code)
{
    if(code >= APP_SYS_FAULT_NO_ERROR){
        return APP_SYS_FAULT_NO_ERROR;
    }
    if(code < APP_ORIGIN_SYSFAULT_MAX){
        return code;
    }

    return (code + APP_SYSFAULT_OFFSET_MIN);
}

uint16_t mw_system_stop_way_convert(uint16_t stopway)
{
    uint16_t diff = 0x00;

    if(stopway >= APP_SYSTEM_STOP_WAY_SIZE){
        return APP_SYSTEM_STOP_WAY_SIZE;
    }

    /** 和系统故障码一样的停充原因码 */
    if(stopway < APP_ORIGIN_SYSFAULT_STOPWAY_MAX){
        return stopway;
    }else if(stopway < APP_SYS_FAULT_NO_ERROR){  /** 这里的 APP_SYS_FAULT_NO_ERROR相当于当前的因系统故障停充的停充原因最大值 */
        return (stopway + APP_SYSFAULT_STOPWAY_OFFSET);
    }

    /** 和系统故障码不一样的停充原因码 */
    if((stopway >= APP_SYS_FAULT_NO_ERROR)){  /** 这里的 APP_SYS_FAULT_NO_ERROR相当于当前的因系统故障停充的停充原因最大值 */
        if((stopway - APP_SYS_FAULT_NO_ERROR) <= (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN)){
            diff = (stopway - APP_SYS_FAULT_NO_ERROR);
            return (diff + APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN);
        }else if((stopway < APP_SYSTEM_STOP_WAY_NULL)){ /** 若 stopway == APP_SYSTEM_STOP_WAY_NULL，则返回APP_SYSTEM_STOP_WAY_SIZE */
            diff = ((stopway - APP_SYS_FAULT_NO_ERROR) - (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX + 0x01 - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN));
            return (diff + APP_NONE_SYSFAULT_STOPWAY_OFFSET + APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX);
        }else if(stopway == APP_SYSTEM_STOP_WAY_NULL){
            return (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX + 0x01);
        }
    }

    /** 自定义故障码 */
    if((stopway > APP_SYSTEM_STOP_WAY_NULL)){
        if((stopway - (APP_SYSTEM_STOP_WAY_NULL + 0x01)) <= APP_USER_STOPWAY_NUM_MAX){
            diff = (stopway - (APP_SYSTEM_STOP_WAY_NULL + 0x01));
            return (diff + APP_ORIGIN_USER_STOPWAY_MIN);
        }
    }

    return APP_SYSTEM_STOP_WAY_SIZE;
}



