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

enum system_stop_way mw_get_system_stop_way(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        thaisenChargeCtlStopWayEn stop_way;
        stop_way = thaisenGetChargCtlStopWay(gunno);
        rt_kprintf("gunno(%d) stop charge, reason is(%d)\n", gunno, stop_way);
        return (enum system_stop_way)stop_way;
    }
    return APP_SYSTEM_STOP_WAY_SIZE;
}


