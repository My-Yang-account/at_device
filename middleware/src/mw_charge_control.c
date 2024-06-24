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





