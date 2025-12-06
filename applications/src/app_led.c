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

/************************** 灯语0 原公版灯语 **************************/

static void app_led_language_0(uint8_t gunno)
{
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    switch(ofsm->base.state.current){
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
}

/************************** 灯语1 原大气灯语 **************************/

static void app_led_language_1(uint8_t gunno, uint32_t *time_base, uint32_t *_time)
{
    if((time_base == NULL) || (_time == NULL)){
        return;
    }
    uint32_t tick = rt_tick_get();
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    if(tick < time_base[gunno]){
        time_base[gunno] = tick;
    }
    _time[gunno] = tick - time_base[gunno];

    switch(ofsm->base.state.current){
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
}

/************************** 灯语2 未插枪蓝灯,  插枪绿灯常亮,  充电绿灯同频闪烁,  枪故障红灯 **************************/

static void app_led_language_2(uint8_t gunno, uint32_t *time_base, uint32_t *_time)
{
    if((time_base == NULL) || (_time == NULL)){
        return;
    }
    uint32_t tick = rt_tick_get();
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    if(tick < time_base[gunno]){
        time_base[gunno] = tick;
    }
    _time[gunno] = tick - time_base[gunno];

    switch(ofsm->base.state.current){
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
    {
        if(APP_SYSTEM_GUNNO_SIZE >= 0x02){   /** 充电中绿灯同频率闪烁(只有双枪时才判断) */
            uint8_t another_gun = APP_SYSTEM_GUNNOA;
            ofsm = get_ofsm_info(another_gun);
            if((gunno != another_gun) && (ofsm->base.state.current == APP_OFSM_STATE_CHARGING)){  /** 以A枪为主 */
                _time[gunno] = _time[another_gun];
            }
            ofsm = get_ofsm_info(gunno);
        }
        mw_led_off_single(RED_LED, gunno);
        if (_time[gunno] < 1000) {
            mw_led_on_single(GREEN_LED, gunno);
        } else if (_time[gunno] >= 1000 && _time[gunno] < 2000) {
            mw_led_off_single(GREEN_LED, gunno);
        } else if (_time[gunno] >= 2000) {
            time_base[gunno] = rt_tick_get();
        }
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
        mw_led_on_only(RED_LED, gunno);
        break;
    default:
        mw_led_on_single(BLUE_LED, gunno);
        break;
    }
}

/************************** 灯语3 未插枪蓝灯,  插枪绿灯常亮,  充电绿灯常亮,  枪故障红灯 **************************/

static void app_led_language_3(uint8_t gunno, uint32_t *time_base, uint32_t *_time)
{
    if((time_base == NULL) || (_time == NULL)){
        return;
    }
    uint32_t tick = rt_tick_get();
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    if(tick < time_base[gunno]){
        time_base[gunno] = tick;
    }
    _time[gunno] = tick - time_base[gunno];

    switch(ofsm->base.state.current){
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
    case APP_OFSM_STATE_CHARGING:
        mw_led_on_single(GREEN_LED, gunno);
        mw_led_off_single(RED_LED, gunno);
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
        mw_led_on_only(RED_LED, gunno);
        break;
    default:
        mw_led_on_single(BLUE_LED, gunno);
        break;
    }
}

/************************** 灯语4  充电绿灯常亮,  枪故障红灯,  其他全灭 **************************/

static void app_led_language_4(uint8_t gunno)
{
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    switch(ofsm->base.state.current){
    case APP_OFSM_STATE_CHARGING:
        mw_led_on_only(GREEN_LED, gunno);
        break;
    case APP_OFSM_STATE_FAULTING:
        mw_led_on_only(RED_LED, gunno);
        break;
    default:
        mw_led_off_all(gunno);
        break;
    }
}

void app_led_thread_entry(void *parameter)
{
    (void)parameter;

    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

    uint8_t gunno = 0, led_language = CP_LED_LANGUAGE_0;
    uint32_t time_base[APP_SYSTEM_GUNNO_SIZE], _time[APP_SYSTEM_GUNNO_SIZE];

    memset(time_base, 0x00, sizeof(time_base));
    memset(_time, 0x00, sizeof(_time));
    for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        mw_led_on_only(BLUE_LED, gunno);
    }

    while (1)
    {
        led_language = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_LED_LANGUAGE, 0x00));

        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

        switch (get_ofsm_info(0x00)->base.ota_state) {
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
            switch(led_language){
            case CP_LED_LANGUAGE_0:
                app_led_language_0(gunno);
                break;
            case CP_LED_LANGUAGE_1:
                app_led_language_1(gunno, time_base, _time);
                break;
            case CP_LED_LANGUAGE_2:
                app_led_language_2(gunno, time_base, _time);
                break;
            case CP_LED_LANGUAGE_3:
                app_led_language_3(gunno, time_base, _time);
                break;
            case CP_LED_LANGUAGE_4:
                app_led_language_4(gunno);
                break;
            default:
                break;
            }
        }
        rt_thread_mdelay(100);
    }
}


/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
