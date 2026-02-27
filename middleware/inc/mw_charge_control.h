/**
 ******************************************************************************
 * @file mw_charge_control.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_CHARGE_CONTROL_H_
#define MW_CHARGE_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "thaisenBMS.h"
#include "thaisenChargLib.h"

enum aux_state_t{
    AUXILIARY_POWER_STATE_ENABLE,
    AUXILIARY_POWER_STATE_DISABLE,
};

/** 充电状态 */
enum charge_state_t{
    APP_CHARGE_STATE_IDLE = thaisen_charg_Idle,                /* 空闲  */
    APP_CHARGE_STATE_SHAKE_HAND = thaisen_charg_Handshake,     /* 握手  */
    APP_CHARGE_STATE_INSULATION = thaisen_charg_Insult,        /* 绝缘  */
    APP_CHARGE_STATE_CONFIGURE = thaisen_charg_Config,         /* 配置  */
    APP_CHARGE_STATE_CHARGING = thaisen_charg_Charg,           /* 充电  */
    APP_CHARGE_STATE_FINISH = thaisen_charg_End,               /* 结束  */
    APP_CHARGE_STATE_FAULTING = thaisen_charg_Fault,           /* 故障  */
    APP_CHARGE_STATE_WAIT_PULL_GUN = thaisen_charg_WaitGun,    /* 等待拔枪  */
    APP_CHARGE_STATE_SIZE,                                     /* 枪号不对时返回此值 */
};

/** 功能使能 */
typedef enum{
    APP_FUNCTION_NO_OFFSET,                                    /* 电流无偏移协议 */
    APP_FUNCTION_YUTONG,                                       /* 宇通 协议 */
    APP_FUNCTION_BAY_AREA,                                     /* 湾区 协议 */
    APP_FUNCTION_BATVOLT_DETECT,                               /* 预充电池电压检测 */
    APP_FUNCTION_BCLTIMEOUT_DETECT,                            /* BCL报文超时检测 */
    APP_FUNCTION_BMS_SEVERAL_FRAME,                            /* BMS多帧 */
    APP_FUNCTION_SIZE,                                         /* 功能使能 */
}app_funcenable_t;

void mw_charge_start_cmd(uint8_t gunno);
void mw_charge_stop_cmd(uint8_t gunno);
enum charge_state_t mw_get_charge_state(uint8_t gunno);

struct thaisenBMS_Charger_struct* mw_get_bms_data(uint8_t gunno);

void mw_enable_auxiliary_power(uint8_t gunno);
void mw_disable_auxiliary_power(uint8_t gunno);
enum aux_state_t mw_get_auxiliary_power_state(uint8_t gunno);

uint8_t mw_get_charge_library_state(uint8_t gunno);
int16_t mw_get_bcp_voltage(uint8_t gunno);
int16_t mw_get_bhm_voltage(uint8_t gunno);

int16_t mw_get_sampling_voltage(uint8_t gunno);

void mw_enable_dcrelay(uint8_t gunno);
void mw_disable_dcrelay(uint8_t gunno);

void mw_enable_dcrelay_directly(uint8_t gunno);
void mw_disable_dcrelay_directly(uint8_t gunno);

/*****************************************************
 * 函数名    mw_charglib_clear_before_charge
 * 功能        启动前清除充电库指定信息
 * 参数        gunno    枪号
 * 返回        1：成功      0：失败
 ****************************************************/
uint8_t mw_charglib_clear_before_charge(uint8_t gunno);

/*****************************************************
* 函数名        mw_charglib_set_function_enable
* 功能            设置功能使能状态
* 参数            port       指定枪口
*          function   功能
*          state      使能状态(1：使能、0：不使能)
* 返回
 ****************************************************/
void mw_charglib_set_function_enable(uint8_t port, app_funcenable_t function, uint8_t state);

/*****************************************************
* 函数名        mw_charglib_get_function_enable
* 功能            获取功能使能状态
* 参数            port       指定枪口
*          function   功能
* 返回            使能状态(1：使能、0：不使能)
 ****************************************************/
uint8_t mw_charglib_get_function_enable(uint8_t port, app_funcenable_t function);

/*****************************************************
* 函数名        mw_charglib_register_get_sysdata_cb
* 功能            注册获取系统数据回调函数
* 参数            cb       回调函数句柄
* 返回
 ****************************************************/
void mw_charglib_register_get_sysdata_cb(void *cb);

#ifdef __cplusplus
}
#endif

#endif /* MW_CHARGE_CONTROL_H_ */
