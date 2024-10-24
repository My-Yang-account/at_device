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
    AUXILIARY_POWER_STATE_CLOSE,
    AUXILIARY_POWER_STATE_OPEN,
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

void mw_charge_start_cmd(uint8_t gunno);
void mw_charge_stop_cmd(uint8_t gunno);
enum charge_state_t mw_get_charge_state(uint8_t gunno);

struct thaisenBMS_Charger_struct* mw_get_bms_data(uint8_t gunno);

void mw_open_auxiliary_power(void);
void mw_close_auxiliary_power(void);
enum aux_state_t mw_get_auxiliary_power_state(void);

uint8_t mw_get_charge_library_state(uint8_t gunno);
int16_t mw_get_bcp_voltage(uint8_t gunno);
int16_t mw_get_bhm_voltage(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* MW_CHARGE_CONTROL_H_ */
