/**
  ******************************************************************************
  * @file
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#include <rtthread.h>

#include "app_led.h"
#include "app_ofsm.h"

#include "mw_led.h"
#include "mw_cc1.h"

void app_led_thread_entry(void *parameter)
{
    (void)parameter;

    uint8_t gunno = 0;
    uint32_t time_base[APP_SYSTEM_GUNNO_SIZE], _time[APP_SYSTEM_GUNNO_SIZE];

    memset(time_base, 0x00, sizeof(time_base));
    memset(_time, 0x00, sizeof(_time));
    for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        mw_led_on_only(BLUE_LED, gunno);
    }

    while (1)
    {
        switch (get_ofsm_info(gunno)->base.ota_state) {
        case APP_OTA_STATE_NULL:
            break;
        case APP_OTA_STATE_UP:
        case APP_OTA_STATE_LINK_UP:
        case APP_OTA_STATE_INTERNET_UP:
            break;
        case APP_OTA_STATE_AUTHING:
            break;
        case APP_OTA_STATE_AUTH_SUCCESS:
        case APP_OTA_STATE_UPDATEING:
            for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                mw_led_on_all(gunno);
            }
            rt_thread_mdelay(3000);
            continue;
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
#if 1
            switch(get_ofsm_info(gunno)->base.state.current)
            {
            case APP_OFSM_STATE_CHARGING:
                mw_led_on_only(GREEN_LED, gunno);
                break;
            case APP_OFSM_STATE_FAULTING:
                mw_led_on_only(RED_LED, gunno);
                break;
            default:
                mw_led_on_only(BLUE_LED, gunno);
                break;
            }
#else
            if(rt_tick_get() < time_base[gunno]){
                time_base[gunno] = rt_tick_get();
            }
            _time[gunno] = rt_tick_get() - time_base[gunno];

            switch(get_ofsm_info(gunno)->base.state.current)
            {
            case APP_OFSM_STATE_IDLEING:
                mw_led_off_single(RED_LED, gunno);
                if(mw_get_cc1(gunno) == CC1_4V){
                    mw_led_on_single(GREEN_LED, gunno);
                }else{
                    mw_led_off_single(GREEN_LED, gunno);
                }
                break;
            case APP_OFSM_STATE_READYING:
            case APP_OFSM_STATE_STARTING:
                mw_led_on_single(GREEN_LED, gunno);
                mw_led_off_single(RED_LED, gunno);
                break;
            case APP_OFSM_STATE_CHARGING:
                mw_led_off_single(RED_LED, gunno);
                if (_time[gunno] < 1000) {
                    mw_led_on_single(GREEN_LED, gunno);
                } else if (_time[gunno] >= 1000 && _time[gunno] < 2000) {
                    mw_led_off_single(GREEN_LED, gunno);
                } else if (_time[gunno] >= 2000) {
                    time_base[gunno] = rt_tick_get();
                }
                break;
            case APP_OFSM_STATE_STOPING:
            case APP_OFSM_STATE_FINISHING:
                mw_led_off_single(RED_LED, gunno);
                if(mw_get_cc1(gunno) == CC1_4V){
                    mw_led_on_single(GREEN_LED, gunno);
                }else{
                    mw_led_off_single(GREEN_LED, gunno);
                }
                break;
            case APP_OFSM_STATE_FAULTING:
                mw_led_on_single(RED_LED, gunno);
                if(mw_get_cc1(gunno) == CC1_4V){
                    mw_led_on_single(GREEN_LED, gunno);
                }else{
                    mw_led_off_single(GREEN_LED, gunno);
                }
                break;
            default:
                mw_led_on_single(BLUE_LED, gunno);
                break;
            }
#endif
        }
        rt_thread_mdelay(100);
    }
}


/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
