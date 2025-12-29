/**
 ******************************************************************************
 * @file mw_meter.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_meter.h"
#include "thaisenChargLib.h"
#include "app_ofsm.h"

//#define AMMETER_DATA_DEBUG
#ifdef AMMETER_DATA_DEBUG
static uint8_t s_ammeter_data_count[4];
#endif /* AMMETER_DATA_DEBUG */

int32_t mw_get_meter_ua(uint8_t gunno)
{
#ifdef AMMETER_DATA_DEBUG
    if(s_ammeter_data_count[0]++ > 15){
        s_ammeter_data_count[0] = 0;
        rt_kprintf("gunno(%d) ammeter data voltage|%d, %d, %d\n", gunno, thaisen_get_ammeterVolt(gunno),
                thaisen_get_ammeterVolt(0), thaisen_get_ammeterVolt(1));
    }
#endif /* AMMETER_DATA_DEBUG */
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        int32_t voltage = thaisen_get_ammeterVolt(gunno);

        if(voltage < 0x00){
            voltage = 0x00 - voltage;
        }
        return voltage;
    }
    return 0;
}
int32_t mw_get_meter_ub(uint8_t gunno)
{
    return 0;
}
int32_t mw_get_meter_uc(uint8_t gunno)
{
    return 0;
}

int32_t mw_get_meter_uab(uint8_t gunno)
{
    return 0;
}
int32_t mw_get_meter_ubc(uint8_t gunno)
{
    return 0;
}
int32_t mw_get_meter_uca(uint8_t gunno)
{
    return 0;
}

int32_t mw_get_meter_ia(uint8_t gunno)
{
#ifdef AMMETER_DATA_DEBUG
    if(s_ammeter_data_count[1]++ > 15){
        s_ammeter_data_count[1] = 0;
        rt_kprintf("gunno(%d) ammeter data current|%d, %d, %d\n", gunno, thaisen_get_ammeterCurrent(gunno),
                thaisen_get_ammeterCurrent(0), thaisen_get_ammeterCurrent(1));
    }
#endif /* AMMETER_DATA_DEBUG */
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
#ifdef APP_INCLUDE_V2G
        return thaisen_get_ammeterCurrent(gunno);
#else
        int32_t current = thaisen_get_ammeterCurrent(gunno);

        if(current < 0x00){
            current = 0x00;
        }
        return current;
#endif /* APP_INCLUDE_V2G */
    }
    return 0;
}
int32_t mw_get_meter_ib(uint8_t gunno)
{
    return 0;
}
int32_t mw_get_meter_ic(uint8_t gunno)
{
    return 0;
}

uint32_t mw_get_meter_pa(uint8_t gunno)
{
#ifdef AMMETER_DATA_DEBUG
    if(s_ammeter_data_count[2]++ > 15){
        s_ammeter_data_count[2] = 0;
        rt_kprintf("gunno(%d) ammeter data power|%d, %d, %d\n", gunno, thaisen_get_ammeterPower(gunno),
                thaisen_get_ammeterPower(0), thaisen_get_ammeterPower(1));
    }
#endif /* AMMETER_DATA_DEBUG */
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return (uint32_t)(thaisen_get_ammeterPower(gunno));
    }
    return 0;
}
uint32_t mw_get_meter_pb(uint8_t gunno)
{
    return 0;
}
uint32_t mw_get_meter_pc(uint8_t gunno)
{
    return 0;
}
uint32_t mw_get_meter_p(uint8_t gunno)
{
    return 0;
}

uint32_t mw_get_meter_qa(uint8_t gunno)
{
    return 0;
}
uint32_t mw_get_meter_qb(uint8_t gunno)
{
    return 0;
}
uint32_t mw_get_meter_qc(uint8_t gunno)
{
    return 0;
}
uint32_t mw_get_meter_q(uint8_t gunno)
{
    return 0;
}

uint32_t mw_get_meter_total_wh(uint8_t gunno)
{
#ifdef AMMETER_DATA_DEBUG
    if(s_ammeter_data_count[3]++ > 15){
        s_ammeter_data_count[3] = 0;
        rt_kprintf("gunno(%d) ammeter data elect|%d, %d, %d\n", gunno, thaisen_get_ammeterEnergy(gunno),
                thaisen_get_ammeterEnergy(0), thaisen_get_ammeterEnergy(1));
    }
#endif /* AMMETER_DATA_DEBUG */
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return thaisen_get_ammeterEnergy(gunno);
    }
    return 0;
}

uint32_t mw_get_meter_reserve_total_wh(uint8_t gunno)
{
#ifdef APP_INCLUDE_V2G
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return thaisen_get_ammeterReverseEnergy(gunno);
    }
#endif /* APP_INCLUDE_V2G */
    return 0;
}

