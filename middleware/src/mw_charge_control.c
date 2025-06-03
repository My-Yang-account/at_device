/**
 ******************************************************************************
 * @file mw_charge_control.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_charge_control.h"
#include "app_ofsm.h"

static uint8_t s_charge_state_filter[APP_SYSTEM_GUNNO_SIZE] = {0};
static enum charge_state_t s_charge_state_last[APP_SYSTEM_GUNNO_SIZE] = {APP_CHARGE_STATE_IDLE};

void mw_charge_start_cmd(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        thaisen_start_charg(gunno);
    }
}

void mw_charge_stop_cmd(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        thaisen_stop_charg(gunno);
    }
}

enum charge_state_t mw_get_charge_state(uint8_t gunno)
{
    enum charge_state_t current_state;
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        current_state = thaisenGetChargWorkStatus(gunno);
        if(current_state != s_charge_state_last[gunno]){
            if(++s_charge_state_filter[gunno] > 3){
                s_charge_state_filter[gunno] = 0;
                s_charge_state_last[gunno] = current_state;
            }
        }
        return (enum charge_state_t)s_charge_state_last[gunno];
    }
    return APP_CHARGE_STATE_SIZE;
}

struct thaisenBMS_Charger_struct* mw_get_bms_data(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return thaisen_get_bms_data(gunno);
    }

    return NULL;
}

void mw_open_auxiliary_power(void)
{
    thaisenAux_Enable();
}

void mw_close_auxiliary_power(void)
{
    thaisenAux_Disable();
}

enum aux_state_t mw_get_auxiliary_power_state(void)
{
    return (enum aux_state_t)thaisenGetAuxStatus();
}

#if 0
typedef enum thaisenChargStatusEnum
{
    thaisenChargIdle,             /* 空闲 */
    thaisenChargAuxPowerOn,       /* 闭合辅源 */
    thaisenChargCHM,              /* 握手 */
    thaisenChargInsult,           /* 绝缘 */
    thaisenChargInsultFinish,     /* 绝缘结束 */
    thaisenChargCRM,              /* CRM 辨识 */
    thaisenChargCTSCML,           /* 时间同步、充电机最大允许 */
    thaisenChargCRO,              /* CRO */
    thaisenChargCROAA,            /* CROAA */
    thaisenChargCCS,              /* CCS 充电 */
    thaisenChargCST,              /* CST */
    thaisenChargCSD,              /* BSD */
    thaisenChargStop,             /* 停止 */
    thaisenChargWaitGun,          /* 等待拔枪 */
    thaisenChargFault,            /* 故障 */
    thaisenChargingFault,         /* 充电故障 */
    thaisenChargCommonFault,      /* 通讯(进行重连) */
    thaisenChargCommonEndFault,   /* 通讯故障 */
    thaisenChargAll,              /* 无 */
}thaisenChargStatusEn;
#endif

/**
 * 获取充电状态
 */
uint8_t mw_get_charge_library_state(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return thaisenChargGetStatus(gunno);
}
/**
 * 获取BCP电池电压  精度：0.1
 */
int16_t mw_get_bcp_voltage(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return (mw_get_bms_data(gunno)->BCP.BatVolt);
}
/**
 * 获取BHM最大允许电压   精度：0.1
 */
int16_t mw_get_bhm_voltage(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return (mw_get_bms_data(gunno)->BHM.MaxAllowVol);
}

int16_t mw_get_sampling_voltage(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            return TH_get_A_Insult_Volt();
        }else{
#ifdef APP_USING_DOUBLEGUN
            return TH_get_B_Insult_Volt();
#endif /* APP_USING_DOUBLEGUN */
        }
    }
}

void mw_enable_dcrelay(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            thaisen_relay_on_A();
        }else{
#ifdef APP_USING_DOUBLEGUN
            thaisen_relay_on_B();
#endif /* APP_USING_DOUBLEGUN */
        }
    }
}

void mw_disable_dcrelay(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            thaisen_relay_off_A();
        }else{
#ifdef APP_USING_DOUBLEGUN
            thaisen_relay_off_B();
#endif /* APP_USING_DOUBLEGUN */
        }
    }
}

