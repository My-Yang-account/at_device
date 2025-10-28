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

/*****************************************************
 * 函数名    mw_charglib_clear_before_charge
 * 功能        启动前清除充电库指定信息
 * 参数        gunno    枪号
 * 返回        1：成功      0：失败
 ****************************************************/
uint8_t mw_charglib_clear_before_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    thaisenChargCtrlHandle_t *handle = thaisenChargGetCtrlHandle();

    if((handle == NULL) || (handle->FunctionExecute == NULL)){
        return 0x00;
    }
    handle->FunctionExecute(gunno, thaisenChargFunctionExecute_Init);
}

/*****************************************************
* 函数名        mw_charglib_set_function_enable
* 功能            设置功能使能状态
* 参数            port       指定枪口
*          function   功能
*          state      使能状态(1：使能、0：不使能)
* 返回
 ****************************************************/
void mw_charglib_set_function_enable(uint8_t port, app_funcenable_t function, uint8_t state)
{
    if(port >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    thaisenChargCtrlHandle_t *handle = thaisenChargGetCtrlHandle();

    if((handle == NULL) || (handle->SetupFunctionEnable == NULL))
        return;

    state = state > 0x01 ? 0x01 : state;
    switch(function){
    case APP_FUNCTION_NO_OFFSET:
        handle->SetupFunctionEnable(port, thaisenChargFunctionEnable_NoOffset, state);
        break;
    case APP_FUNCTION_YUTONG:
        handle->SetupFunctionEnable(port, thaisenChargFunctionEnable_YuTong, state);
        break;
    case APP_FUNCTION_BAY_AREA:
        handle->SetupFunctionEnable(port, thaisenChargFunctionEnable_BayArea, state);
        break;
    case APP_FUNCTION_BATVOLT_DETECT:
        handle->SetupFunctionEnable(port, thaisenChargFunctionEnable_BatVolt, state);
        break;
    case APP_FUNCTION_BCLTIMEOUT_DETECT:
        handle->SetupFunctionEnable(port, thaisenChargFunctionEnable_BCLTimeout, state);
        break;
    default:
        break;
    }
}

/*****************************************************
* 函数名        mw_charglib_get_function_enable
* 功能            获取功能使能状态
* 参数            port       指定枪口
*          function   功能
* 返回            使能状态(1：使能、0：不使能)
 ****************************************************/
uint8_t mw_charglib_get_function_enable(uint8_t port, app_funcenable_t function)
{
    if(port >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    thaisenChargCtrlHandle_t *handle = thaisenChargGetCtrlHandle();

    if((handle == NULL) || (handle->QueryFunctionEnable == NULL))
        return 0x00;

    switch(function){
    case APP_FUNCTION_NO_OFFSET:
        return handle->QueryFunctionEnable(port, thaisenChargFunctionEnable_NoOffset);
        break;
    case APP_FUNCTION_YUTONG:
        return handle->QueryFunctionEnable(port, thaisenChargFunctionEnable_YuTong);
        break;
    case APP_FUNCTION_BAY_AREA:
        return handle->QueryFunctionEnable(port, thaisenChargFunctionEnable_BayArea);
        break;
    case APP_FUNCTION_BATVOLT_DETECT:
        return handle->QueryFunctionEnable(port, thaisenChargFunctionEnable_BatVolt);
        break;
    case APP_FUNCTION_BCLTIMEOUT_DETECT:
        return handle->QueryFunctionEnable(port, thaisenChargFunctionEnable_BCLTimeout);
        break;
    default:
        break;
    }
    return 0x00;
}
