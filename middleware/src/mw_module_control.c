/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-31     我的杨yang       the first version
 */
#include "mw_module_control.h"
#include "app_ofsm.h"

void mw_module_set_permit_charge_state(uint8_t state, uint8_t gunno)
{
    thaisenModuleSetAllowChargeState(state, gunno);
}

uint8_t mw_module_get_permit_charge_state(uint8_t gunno)
{
#ifdef APP_USING_CYCLE_MATRIX
    uint8_t state = thaisen_get_gun_serversta(gunno + 0x01);

    if(state == server_disable){
        return APP_MODULE_FORBID_CHARGE;
    }else if(state == server_enable){
        return APP_MODULE_ALLOW_CHARGE;
    }
    return thaisenAllowCharge_size;
#else
    uint8_t state = thaisenModuleGetAllowChargeState(gunno);
    if(state > thaisenAllowCharge_size){
        return thaisenAllowCharge_size;
    }else{
        return state;
    }
#endif /* APP_USING_CYCLE_MATRIX */
}

uint8_t mw_module_is_starting(uint8_t gunno)
{
#ifdef APP_USING_CYCLE_MATRIX
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        thaisen_moduleallo_chargeReq(gunno + 0x01);
    }
#else
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(get_ofsm_info(gunno)->state == APP_OFSM_STATE_STARTING){
            return 0x01;
        }
    }
#endif /* APP_USING_CYCLE_MATRIX */
    return 0x00;
}

void mw_module_starting_finish(uint8_t gunno)
{
#ifdef APP_USING_CYCLE_MATRIX
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        thaisen_moduleallo_chargeReqFinish(gunno + 0x01);
    }
#endif /* APP_USING_CYCLE_MATRIX */
}

