/**
  ******************************************************************************
  * @file
  * @brief 平台状态机
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#include <rtthread.h>

#include <string.h>

#include "app_ofsm.h"
#include "app_hci.h"
#include "app_card.h"
#include "app_osupport.h"
#include "app_support_func.h"
#include "app_data_info_interface.h"

#include "mw_cc1.h"
#include "mw_charge_control.h"
#include "mw_meter.h"
#include "mw_temp.h"
#include "mw_time.h"
#include "mw_module_control.h"

#include "chargepile_config.h"

#define DBG_TAG "app.ofsm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define CARD_NUMBER_COMPARE_LEN_MIN               12    /* 卡号最小对比长度 */

#define LINK_PLATFORM_MAX                         0x02  /* 连接平台总数 */
#define TARGET_PLATFORM_INDEX                     0x00  /* 目标平台下标 */
#define MONITOR_PLATFORM_INDEX                    0x01  /* 监控平台下标 */

typedef void (* ofsm_fun_p)(uint8_t gunno);

static ofsm_fun_p s_ofsm_fun_list[APP_SYSTEM_GUNNO_SIZE][APP_OFSM_STATE_SIZE];
static ofsm_fun_p s_ofsm_fun[APP_SYSTEM_GUNNO_SIZE] = {NULL};
static uint32_t s_debug_count[APP_SYSTEM_GUNNO_SIZE];

static uint32_t s_tiny_current_count[APP_SYSTEM_GUNNO_SIZE];

extern struct rt_messagequeue g_buzzon_mq;
static enum buzzon_state s_buzzon_state = BUZZON_STATE_NULL;

static struct ofsm_info s_ofsm_info[APP_SYSTEM_GUNNO_SIZE];
static int32_t s_current_order_index[LINK_PLATFORM_MAX][APP_SYSTEM_GUNNO_SIZE];
static thaisen_transaction_t s_thaisen_transaction[APP_SYSTEM_GUNNO_SIZE];
static thaisen_transaction_t s_thaisen_transaction_report[LINK_PLATFORM_MAX][APP_SYSTEM_GUNNO_SIZE];
static enum booting_step_t s_booting_step[APP_SYSTEM_GUNNO_SIZE];
static uint32_t s_request_screen_time_tick = 0x00;
static uint8_t s_request_screen_time_step = 0x00;
static uint8_t s_transaction_sending[LINK_PLATFORM_MAX][APP_SYSTEM_GUNNO_SIZE];
static uint8_t s_chargegun_idle_count = 0x00;

static uint8_t s_issue_power_adjust = false, s_chargepile_output_steady[APP_SYSTEM_GUNNO_SIZE];
static uint8_t s_charge_steady_delay[APP_SYSTEM_GUNNO_SIZE], s_power_adjust_delay = 0, s_power_on = 0;
static uint32_t s_system_power_output = 0x00;
static uint16_t s_gun_charging_curr[APP_SYSTEM_GUNNO_SIZE];

static void transaction_record_query_report(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }
    if((s_ofsm_info[gunno].state >= APP_OFSM_STATE_STARTING) && (s_ofsm_info[gunno].state <= APP_OFSM_STATE_STOPING)){
        return;
    }

    if(mw_storage_record_get_userdata_record_num(USER_DATA_TYPE_STORAGE, gunno) > 0x00){
        int32_t index = 0;
        uint8_t save_rentry = 0, check_rentry = 0;
        while(check_rentry < 3){
            if((index = mw_storage_record_get_first_index_userdata(USER_DATA_TYPE_STORAGE, gunno)) < 0x00){
                check_rentry = 3;
                break;
            }

            uint8_t need_check = 0;
            thaisen_transaction_t rtransaction;
            mw_storage_record_get_designate_index_record((uint8_t*)(&rtransaction), sizeof(rtransaction), gunno, index);
            save_rentry = 0;

            if(rtransaction.stop_reason == APP_SYSTEM_STOP_WAY_POWER_OFF){
                uint32_t loss_elect = 0x00, _total_elect = 0x00;

                if(rtransaction.charge_way == APP_CHARGE_WAY_PARACHARGE){
                    for(uint8_t count = 0x00; count < APP_SYSTEM_GUNNO_SIZE; count++){
                        _total_elect += mw_get_meter_total_wh(count);
                    }
                }else{
                    _total_elect = mw_get_meter_total_wh(gunno);
                }

                if(_total_elect >= rtransaction.ammeter_stop){
                    loss_elect = _total_elect - rtransaction.ammeter_stop;
                    rtransaction.ammeter_stop = _total_elect;

                    rtransaction.total_elect = (rtransaction.ammeter_stop - rtransaction.ammeter_start);
                    rtransaction.total_loss_elect = rtransaction.total_elect;
                    rtransaction.charge_fee += ((double)loss_elect *(double)1.05 *100);
                    rtransaction.total_fee += ((double)loss_elect *(double)1.05 *100);

                    rtransaction.end_time = rtransaction.start_time + (15 *60);
                    rtransaction.charge_time += (15 *60);

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||  \
                    defined (APP_INCLUDE_SGCC_PROTOCOL))
                    rtransaction.rate_type_elect[APP_RATE_TYPE_FLAT] += loss_elect;
                    rtransaction.rate_type_amount[APP_RATE_TYPE_FLAT] += ((double)loss_elect *(double)1.05 *100);
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
                    defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
                    if(rtransaction.start_period_number > (APP_BILLING_RULE_PERIOD_MAX - 0x01)){
                        rtransaction.start_period_number = (APP_BILLING_RULE_PERIOD_MAX - 0x01);
                    }
                    rtransaction.period_elect[rtransaction.start_period_number] += loss_elect;
                    rtransaction.period_elect_fees[rtransaction.start_period_number] += ((double)loss_elect *(double)1.05 *100);
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */
                    need_check = 1;
                }
                rtransaction.order_state.is_charging = APP_THA_ENUM_FALSE;
            }else{
                need_check = 1;
            }
            if(need_check){
                while(save_rentry < 5){
                    if(mw_storage_record_designate_index_updated(&rtransaction, sizeof(rtransaction), USER_DATA_TYPE_VERIFIED, 0x00, gunno, index) < 0x00){
                        rt_thread_mdelay(10);
                        save_rentry++;
                        continue;
                    }else{
                        check_rentry = 3;
                        break;
                    }
                    LOG_D("order have adjusted(%d)", check_rentry);
                    break;
                }
            }
        }
        return;
    }

#ifdef APP_INCLUDE_TARGET_PLATFORM
    if(app_nsal_query_transaction_verify_state(gunno, APP_TARGET_PLATFORM_ID) == APP_THA_ENUM_TRUE){
        if(s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] == APP_THA_ENUM_TRUE){
            LOG_D("gunno(%d) target transaction verify", gunno);
            mw_storage_record_designate_index_updated(&(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), sizeof(thaisen_transaction_t), \
                    USER_DATA_TYPE_REPORTED, APP_TARGET_PLATFORM_ID, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
        }
        s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] = APP_THA_ENUM_FALSE;
    }else{
        int32_t resylt = 0;
        if(s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] == APP_THA_ENUM_FALSE){
            if((resylt = mw_storage_record_get_recordverify_record_num(APP_TARGET_PLATFORM_ID, gunno)) > 0x00){
                int32_t index = mw_storage_record_get_first_index_recordverify(APP_TARGET_PLATFORM_ID, gunno), result = STORAGE_ERR_NONE;
                if(index >= 0x00){
                    result = mw_storage_record_get_designate_index_record((uint8_t*)&(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), sizeof(thaisen_transaction_t), gunno, index);
                    if((result == STORAGE_ERR_NONE) || (result == STORAGE_ERR_CHECK_ERROR)){
                        s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] = APP_THA_ENUM_TRUE;
                        s_current_order_index[TARGET_PLATFORM_INDEX][gunno] = index;
                        app_nsal_transaction_record_report_target(gunno, &(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), 0x01);
                    }
                }
            }
        }
    }
#endif /* APP_INCLUDE_TARGET_PLATFORM */

#ifdef APP_INCLUDE_MONITOR_PLATFORM
    if(app_nsal_query_transaction_verify_state(gunno, APP_MONITOR_PLATFORM_ID) == APP_THA_ENUM_TRUE){
        if(s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] == APP_THA_ENUM_TRUE){
            LOG_D("gunno(%d) monitor transaction verify", gunno);
            mw_storage_record_designate_index_updated(&(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), sizeof(thaisen_transaction_t), \
                    USER_DATA_TYPE_REPORTED, APP_MONITOR_PLATFORM_ID, gunno, s_current_order_index[MONITOR_PLATFORM_INDEX][gunno]);
        }
        s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] = APP_THA_ENUM_FALSE;
    }else{
        if(s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] == APP_THA_ENUM_FALSE){
            if(mw_storage_record_get_recordverify_record_num(APP_MONITOR_PLATFORM_ID, gunno) > 0x00){
                int32_t index = mw_storage_record_get_first_index_recordverify(APP_MONITOR_PLATFORM_ID, gunno), result = STORAGE_ERR_NONE;
                int32_t result = mw_storage_record_get_first_index_recordverify(APP_MONITOR_PLATFORM_ID, gunno);
                if(index >= 0x00){
                    result = mw_storage_record_get_designate_index_record((uint8_t*)&(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), sizeof(thaisen_transaction_t), gunno, index);
                    if((result == STORAGE_ERR_NONE) || (result == STORAGE_ERR_CHECK_ERROR)){
                        s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] = APP_THA_ENUM_TRUE;
                        s_current_order_index[MONITOR_PLATFORM_INDEX][gunno] = index;
                        app_nsal_transaction_record_report_monitor(gunno, &(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), 0x01);
                    }
                }
            }
        }
    }
#endif /* APP_INCLUDE_MONITOR_PLATFORM */
}

static void temperature_protect_limitcurr(uint8_t gunno, uint16_t *set_curr)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    uint16_t singlegun_curr = *set_curr;

    if(gunno == APP_SYSTEM_GUNNOA){
        if(s_ofsm_info[gunno].state == APP_OFSM_STATE_CHARGING){
            if(s_ofsm_info[gunno].base.flag.is_overtemp){
#ifdef USING_DOUBLE_GUN
                if(s_ofsm_info[gunno + 0x01].state == APP_OFSM_STATE_CHARGING){
                    if(s_gun_charging_curr[gunno]){
                        singlegun_curr = s_gun_charging_curr[gunno] *OVERTEMP_DECREASE_CURR_PERCENT;
                        s_ofsm_info[gunno].base.flag.is_curr_decreased = APP_THA_ENUM_TRUE;
                        if(singlegun_curr > *set_curr){
                            singlegun_curr = *set_curr;
                        }
                    }
                }else
#endif /* USING_DOUBLE_GUN */
                {
                    if(s_gun_charging_curr[gunno]){
                        singlegun_curr = s_gun_charging_curr[gunno] *OVERTEMP_DECREASE_CURR_PERCENT /APP_SYSTEM_GUNNO_SIZE;
                        s_ofsm_info[gunno].base.flag.is_curr_decreased = APP_THA_ENUM_TRUE;
                        if(singlegun_curr > *set_curr){
                            singlegun_curr = *set_curr;
                        }
                    }
                }
            }
        }else{
#ifdef USING_DOUBLE_GUN
            if(s_ofsm_info[gunno + 0x01].state == APP_OFSM_STATE_CHARGING){
                if(s_ofsm_info[gunno + 0x01].base.flag.is_overtemp){
                    if(s_gun_charging_curr[gunno + 1]){
                        singlegun_curr = s_gun_charging_curr[gunno + 0x01] *OVERTEMP_DECREASE_CURR_PERCENT /APP_SYSTEM_GUNNO_SIZE;
                        s_ofsm_info[gunno].base.flag.is_curr_decreased = APP_THA_ENUM_TRUE;
                        if(singlegun_curr > *set_curr){
                            singlegun_curr = *set_curr;
                        }
                    }
                }
            }
#endif /* USING_DOUBLE_GUN */
        }
    }else{
#ifdef USING_DOUBLE_GUN
        if(s_ofsm_info[gunno].state == APP_OFSM_STATE_CHARGING){
            if(s_ofsm_info[gunno].base.flag.is_overtemp){
                if(s_ofsm_info[gunno - 0x01].state == APP_OFSM_STATE_CHARGING){
                    if(s_gun_charging_curr[gunno]){
                        singlegun_curr = s_gun_charging_curr[gunno] *OVERTEMP_DECREASE_CURR_PERCENT;
                        s_ofsm_info[gunno].base.flag.is_curr_decreased = APP_THA_ENUM_TRUE;
                        if(singlegun_curr > *set_curr){
                            singlegun_curr = *set_curr;
                        }
                    }
                }else{
                    if(s_gun_charging_curr[gunno]){
                        singlegun_curr =  s_gun_charging_curr[gunno] *OVERTEMP_DECREASE_CURR_PERCENT /APP_SYSTEM_GUNNO_SIZE;
                        s_ofsm_info[gunno].base.flag.is_curr_decreased = APP_THA_ENUM_TRUE;
                        if(singlegun_curr > *set_curr){
                            singlegun_curr = *set_curr;
                        }
                    }
                }
            }
        }else{
            if(s_ofsm_info[gunno - 0x01].state == APP_OFSM_STATE_CHARGING){
                if(s_ofsm_info[gunno - 0x01].base.flag.is_overtemp){
                    if(s_gun_charging_curr[gunno - 1]){
                        singlegun_curr = s_gun_charging_curr[gunno - 0x01] *OVERTEMP_DECREASE_CURR_PERCENT /APP_SYSTEM_GUNNO_SIZE;
                        s_ofsm_info[gunno].base.flag.is_curr_decreased = APP_THA_ENUM_TRUE;
                        if(singlegun_curr > *set_curr){
                            singlegun_curr = *set_curr;
                        }
                    }
                }
            }
        }
#endif /* USING_DOUBLE_GUN */
    }
    if(set_curr){
        *set_curr = singlegun_curr;
    }
}

void chargepile_power_adjust(void)
{
#define RUNNING_PERIOD      200
#define ADJUST_PERIOD       30000 /RUNNING_PERIOD

    extern uint32_t get_meter_ua(uint8_t gunno);
    extern uint32_t get_meter_ia(uint8_t gunno);

    extern uint16_t thaisenGetModuleMaxVolt(void);
    extern uint16_t thaisenGetModuleMaxCurr(void);

    extern void thaisenSetModuleMaxChargVolt(uint16_t volt);
    extern uint16_t thaisenGetModuleMaxChargVolt(void);
    extern void thaisenSetModuleMaxChargCurr(uint16_t curr);
    extern uint16_t thaisenGetModuleMaxChargCurr(void);
    extern uint8_t thaisenGetNormalModuleNum(uint8_t gunNum);
    extern void thaisenSetModuleMaxChargCurrGroup(uint8_t groupNum, uint16_t curr);

    static uint32_t original_power[APP_SYSTEM_GUNNO_SIZE], set_gunno = 0x00;
    bool server_adjust_power = false;      /* 服务器下发设置桩工作参数标志 */
    uint8_t gun_idle[APP_SYSTEM_GUNNO_SIZE], gunno = 0x00;
    uint16_t set_volt[APP_SYSTEM_GUNNO_SIZE];                  /* 根据功率百分比得到的需要设置的电流 */
    uint32_t allocation_power[APP_SYSTEM_GUNNO_SIZE];  /* 分配给每个枪的功率 */

    for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        set_volt[gunno] = 0;
        gun_idle[gunno] = 0;
        allocation_power[gunno] = 0;
    }
    /** 服务器下发设置桩工作参数报文，需要进行功率调节 */
    for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        if(s_ofsm_info[gunno].base.system_power_max == 0x00){
            continue;
        }
        if(s_ofsm_info[gunno].base.flag.is_adjust_power){
            if(s_ofsm_info[gunno].base.power_strategy == APP_POWER_STRATEGY_ORDER){
                if((s_ofsm_info[gunno].state != APP_OFSM_STATE_STARTING) && (s_ofsm_info[gunno].state != APP_OFSM_STATE_CHARGING)){
                    s_ofsm_info[gunno].base.flag.is_resume_power = APP_THA_ENUM_TRUE;
                    LOG_D("chargepile is not charging, resume original power(%d, %d, %d)\n", gunno, original_power[gunno], s_ofsm_info[gunno].state);
                }
            }
        }

        if(app_nsal_is_set_power(gunno) || (s_ofsm_info[gunno].base.flag.is_resume_power && s_ofsm_info[gunno].base.flag.is_adjust_power)){
            app_nsal_clear_set_power(gunno);
            if((app_nsal_get_setup_power(gunno) > s_ofsm_info[gunno].base.system_power_max) && (s_ofsm_info[gunno].base.flag.is_resume_power == APP_THA_ENUM_FALSE)){
                server_adjust_power = false;
                app_nsal_report_set_power_result(gunno, 0x00, 0x00);
                LOG_E("power adjust fail deal to power of range|%d, %d\n", app_nsal_get_setup_power(gunno), s_ofsm_info[gunno].base.system_power_max);
            }else{
                set_gunno = gunno;
                server_adjust_power = true;
                s_issue_power_adjust = true;

                if(s_ofsm_info[gunno].base.flag.is_resume_power == APP_THA_ENUM_TRUE){
                    s_system_power_output = original_power[gunno];
                    s_ofsm_info[gunno].base.flag.is_adjust_power = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.flag.is_resume_power = APP_THA_ENUM_FALSE;
                    LOG_D("gunno(%d) resume origin power(%d)", gunno, original_power[gunno]);
                }else{
                    original_power[gunno] = s_system_power_output;
                    s_system_power_output = app_nsal_get_setup_power(gunno) *10;
                    s_ofsm_info[gunno].base.flag.is_adjust_power = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.flag.is_resume_power = APP_THA_ENUM_FALSE;

                    if(s_ofsm_info[gunno].base.power_strategy == APP_POWER_STRATEGY_SET_LIMIT){
                        s_ofsm_info[gunno].base.flag.is_adjust_power = APP_THA_ENUM_FALSE;
                        s_ofsm_info[gunno].base.flag.is_resume_power = APP_THA_ENUM_FALSE;
                        LOG_D("power strategy is order charge, limit power is[%d, %d]", gunno, app_nsal_get_setup_power(gunno));
                    }else{
                        if((s_ofsm_info[gunno].state != APP_OFSM_STATE_STARTING) && (s_ofsm_info[gunno].state != APP_OFSM_STATE_CHARGING)){
                            server_adjust_power = false;
                            s_issue_power_adjust = false;
                            LOG_D("gunno(%d) is not charging, not allow set order power", gunno);
                        }
                    }
                }
                LOG_I("gunno(%d) server adjust system power|%d, %d, %d", gunno, app_nsal_get_setup_power(gunno), s_ofsm_info[gunno].base.system_power_max, s_ofsm_info[gunno].base.power_strategy);
            }
        }
    }
    /** 上电判断之前桩是否被设置过功率，如果是则需要进行功率调节 */
    if(s_power_on == 0 && server_adjust_power == false){
        uint32_t power = *((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_SYSTEM_POWER_TOTAL, 0)));
        uint32_t sys_power_max = 0x00, single_module_power = 0x00;
        uint8_t group = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0));

        single_module_power = ((*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0)))) *  \
                (*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0)))));
        for(uint8_t count = 0; count < group; count++){
            sys_power_max += (sys_get_single_group_module_num(count) *single_module_power);
        }

        if((power >= 1000) && (power <= sys_power_max)){
            s_system_power_output = power *10;
        }else{
            s_system_power_output = sys_power_max *10;
        }
        for(uint8_t count = 0; count < APP_SYSTEM_GUNNO_SIZE; count++){
            original_power[count] = s_system_power_output;
        }

        LOG_I("the system power total|%dw, s_system_power_output|%d, %d", power, s_system_power_output, sys_power_max);
    }

#ifdef APP_USING_DOUBLEGUN
    if(get_ofsm_info(APP_SYSTEM_GUNNOA)->state != APP_OFSM_STATE_CHARGING && get_ofsm_info(APP_SYSTEM_GUNNOB)->state != APP_OFSM_STATE_CHARGING){
        set_volt[APP_SYSTEM_GUNNOA] = thaisenGetModuleMaxChargVolt();  /* 10倍 */
        set_volt[APP_SYSTEM_GUNNOB] = thaisenGetModuleMaxChargVolt();  /* 10倍 */
        gun_idle[APP_SYSTEM_GUNNOA] = 1;
        gun_idle[APP_SYSTEM_GUNNOB] = 1;
    }else if(get_ofsm_info(APP_SYSTEM_GUNNOA)->state == APP_OFSM_STATE_CHARGING && get_ofsm_info(APP_SYSTEM_GUNNOB)->state == APP_OFSM_STATE_CHARGING){
        set_volt[APP_SYSTEM_GUNNOA] = mw_get_meter_ua(APP_SYSTEM_GUNNOA);  /* 10倍 */
        set_volt[APP_SYSTEM_GUNNOB] = mw_get_meter_ua(APP_SYSTEM_GUNNOB);  /* 10倍 */
    }else{
        if(get_ofsm_info(APP_SYSTEM_GUNNOA)->state == APP_OFSM_STATE_CHARGING){
            set_volt[APP_SYSTEM_GUNNOA] = mw_get_meter_ua(APP_SYSTEM_GUNNOA);  /* 10倍 */
            set_volt[APP_SYSTEM_GUNNOB] = mw_get_meter_ua(APP_SYSTEM_GUNNOA);  /* 10倍 */
            gun_idle[APP_SYSTEM_GUNNOB] = 1;
        }else{
            set_volt[APP_SYSTEM_GUNNOA] = mw_get_meter_ua(APP_SYSTEM_GUNNOB);  /* 10倍 */
            set_volt[APP_SYSTEM_GUNNOB] = mw_get_meter_ua(APP_SYSTEM_GUNNOB);  /* 10倍 */
            gun_idle[APP_SYSTEM_GUNNOA] = 1;
        }
    }
#else
    if(get_ofsm_info(APP_SYSTEM_GUNNOA)->state != APP_OFSM_STATE_CHARGING){
        set_volt[APP_SYSTEM_GUNNOA] = thaisenGetModuleMaxChargVolt();
        gun_idle[APP_SYSTEM_GUNNOA] = 1;
    }else if(get_ofsm_info(APP_SYSTEM_GUNNOA)->state == APP_OFSM_STATE_CHARGING){
        set_volt[APP_SYSTEM_GUNNOA] = mw_get_meter_ua(APP_SYSTEM_GUNNOA);
    }
#endif /* APP_USING_DOUBLEGUN */

    /** 等待充电桩输出稳定 */
    for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        if(get_ofsm_info(gunno)->state == APP_OFSM_STATE_CHARGING){
            if(++s_charge_steady_delay[gunno] > ADJUST_PERIOD){
                s_charge_steady_delay[gunno] = ADJUST_PERIOD;
                s_chargepile_output_steady[gunno] = true;
            }
        }else{
            s_charge_steady_delay[gunno] = 0;
            s_chargepile_output_steady[gunno] = false;
        }
    }
//    LOG_W("s_power_adjust_delay|%d, %d, %d, %d", s_power_adjust_delay, set_volt, s_system_power_output *10 /set_volt, charge_type);
    /** 经过一次调整后需等待一定时间使其输出稳定后再进行查询并调节 */
    if((++s_power_adjust_delay > ADJUST_PERIOD) || server_adjust_power || s_power_on == 0 ){
        for(gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            allocation_power[gunno] = s_system_power_output *10 / APP_SYSTEM_GUNNO_SIZE;  /* 100倍 */
            if(set_volt[gunno]){
                if(s_chargepile_output_steady[gunno] || gun_idle[gunno]){
                    s_ofsm_info[gunno].base.gun_set_curr = allocation_power[gunno]/ set_volt[gunno];  /* 10倍 */
                    if(s_ofsm_info[gunno].base.gun_set_curr > ((*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0)))) *10 /APP_SYSTEM_GUNNO_SIZE)){
                        s_ofsm_info[gunno].base.gun_set_curr = ((*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0)))) *10/ APP_SYSTEM_GUNNO_SIZE);
                    }
                    rt_kprintf("gunno(%d) s_set_curr|%d, allocation_power|%d  set_volt|%d  chargecurr_max|%d  s_system_power_output|%d  power_percent|%d\n", gunno,
                            s_ofsm_info[gunno].base.gun_set_curr, allocation_power[gunno], set_volt[gunno], (*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0)))), s_system_power_output, s_system_power_output);
                }
            }
        }
        s_power_on = 1;
        /** 如果是服务器下发了设置桩工作参数报文则需要回复 */
        if(server_adjust_power){
            if(set_gunno < APP_SYSTEM_GUNNO_SIZE){
                if(s_ofsm_info[set_gunno].base.power_strategy == APP_POWER_STRATEGY_SET_LIMIT){
                    uint32_t issue_power = app_nsal_get_setup_power(set_gunno);
                    sys_sync_config_item_content(CONFIG_ITEM_SYSTEM_POWER_TOTAL, &issue_power, sizeof(issue_power));
                    sys_storage_config_item();
                }else{
                    LOG_D("gunno(%d) set order power, is not require storage", set_gunno);
                }

                app_nsal_report_set_power_result(set_gunno, 0x01, 0x00);
            }
        }
        server_adjust_power = false;
        s_power_adjust_delay = 0;
    }
}

static void ofsm_wait_net_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint32_t single_module_power = 0x00;
    uint8_t valid_len = sizeof(s_thaisen_transaction[gunno].chargepile_id), *pile_number = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
    uint8_t group = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0));

    valid_len = valid_len > strlen((char*)pile_number) ? strlen((char*)pile_number) : valid_len;
    memset(s_thaisen_transaction[gunno].chargepile_id, 0x00, sizeof(s_thaisen_transaction[gunno].chargepile_id));
    memcpy(s_thaisen_transaction[gunno].chargepile_id, pile_number, valid_len);
    s_thaisen_transaction[gunno].gunno = gunno;

    s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
    s_ofsm_info[gunno].base.soft_ver_main = SOFTWARE_VERSION;
    s_ofsm_info[gunno].base.soft_ver_sub = SOFTWARE_SUBVERSION;
    s_ofsm_info[gunno].base.soft_ver_revise = SOFTWARE_REVISION;
    s_ofsm_info[gunno].base.bms_data = mw_get_bms_data(gunno);

    s_ofsm_info[gunno].base.system_power_max = 0x00;
    single_module_power = ((*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0)))) *  \
            (*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0)))));
    for(uint8_t count = 0; count < group; count++){
        s_ofsm_info[gunno].base.system_power_max += (sys_get_single_group_module_num(count) *single_module_power);
    }
    LOG_D("gunno(%d) system_power_max(%d) get_single_group_module_num(%d)(%d)\n\n", gunno, s_ofsm_info[gunno].base.system_power_max, \
            sys_get_single_group_module_num(gunno), single_module_power);

    s_ofsm_info[gunno].base.gun_set_curr = (*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0)))) *10;

    app_nsal_message_init(gunno);
    app_nsal_init_charge_data(gunno);

    s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
    switch(s_ofsm_info[gunno].base.cc1_state){
    case CC1_12V :
        s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
        break;
    case CC1_4V :
        s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
        break;
    }

    if(app_get_highest_priority_system_fault(gunno) != APP_SYS_FAULT_NO_ERROR){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
    }

    s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_IDLEING];
    s_ofsm_info[gunno].state = APP_OFSM_STATE_IDLEING;

    s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

    app_nsal_state_charged(gunno);
    app_nsal_event_occurded(gunno);
}

static void ofsm_idleing_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }
    enum charge_state_t charge_state = mw_get_charge_state(gunno);

    if(++s_debug_count[gunno] > (5000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) idle state (%dV, S%d)...", gunno, mw_get_cc1_value(mw_get_cc1(gunno)), charge_state);
    }

    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.is_charge_complete = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;

    if((s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_OVERHAUL) ||
            (s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_FREEZE)){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }

    if(app_get_highest_priority_system_fault(gunno) != APP_SYS_FAULT_NO_ERROR){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }

    switch(s_ofsm_info[gunno].base.cc1_state)
    {
    case CC1_12V:
    case CC1_6V:
        if(s_ofsm_info[gunno].base.cc1_state == CC1_12V){
            if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_HALFWAY){
                s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
                app_nsal_event_occurded(gunno);
            }
        }else{
            if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_DISCONNECT){
                s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
                app_nsal_event_occurded(gunno);
            }
        }

        if(s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_IDLEING){
            s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_IDLEING;
            app_nsal_state_charged(gunno);
        }
        break;
    case CC1_4V:
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_READYING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_READYING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        break;
    default:
        break;
    }

    mw_clear_time_sync_flag(gunno);
    clear_swipe_card_state(gunno);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

    app_nsal_clear_remote_start(gunno);
    app_nsal_clear_remote_stop(gunno);
    app_nsal_clear_remote_card_authorize(gunno);
    app_nsal_clear_remote_vin_authorize(gunno);
}

static void ofsm_readying_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }
    bool is_charging_authorization = APP_THA_ENUM_FALSE;  /* 充电已授权 */
    enum charge_state_t charge_state = mw_get_charge_state(gunno);

    if(++s_debug_count[gunno] > (3000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) readying state (%dV, S%d)...", gunno, mw_get_cc1_value(mw_get_cc1(gunno)), charge_state);
    }

    if((s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_OVERHAUL) ||
            (s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_FREEZE)){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }

    if(app_get_highest_priority_system_fault(gunno) != APP_SYS_FAULT_NO_ERROR){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }
#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
    s_ofsm_info[gunno].base.card_ballance_before = 0x00;
    s_ofsm_info[gunno].base.card_ballance_after = 0x00;
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

                clear_swipe_card_state(gunno);
                app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
                app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
                app_nsal_clear_remote_start(gunno);
                app_nsal_clear_remote_card_authorize(gunno);
                mw_clear_time_sync_flag(gunno);
                app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
                app_nsal_clear_remote_stop(gunno);

                return;
            }
        }
    }

    switch(mw_get_cc1(gunno)) {
    case CC1_12V:
    case CC1_6V:
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_IDLEING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_IDLEING;

        if(s_ofsm_info[gunno].base.cc1_state == CC1_12V){
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
        }else{
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
        }
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
    case CC1_4V:
        if(app_nsal_is_remote_start(gunno)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by APP", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_APP;

            /* 经过平台启动的， 已填充以下字段
             * base->user_number
             * base->transaction_number
             * base->card_number
             * base->card_uid
             * base->account_balance
             * base->charge_strategy
             * base->charge_strategy_para
             * base->card_ballance_before
             * base->card_ballance_after
             * base->device_transaction_number
             * base->offline_chargetime */

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.user_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : valid_len;
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
            memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_APP;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

            is_charging_authorization = true;

        }else if(get_swipe_card_state(gunno)){
            uint8_t need_authorize_online = 0x00;
            clear_swipe_card_state(gunno);
            if(get_card_info_type() == CARD_INFO_TYPE_CARD_NUMBER){
                uint8_t pile_number_len = APP_CARD_NUMBER_COMPARE_LEN, card_number_len = get_card_number_len(),
                        compare_len = 0, count = 0;
                uint8_t *pile_number, *card_number;
                compare_len = pile_number_len > card_number_len ? card_number_len : pile_number_len;
                pile_number = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0);
                card_number = get_card_number();

                if(compare_len >= CARD_NUMBER_COMPARE_LEN_MIN){
                    for(; count < CARD_NUMBER_COMPARE_LEN_MIN; count++){
                        if(pile_number[count] != card_number[count]){
                            break;
                        }
                    }
                    if(count == CARD_NUMBER_COMPARE_LEN_MIN){
                        s_buzzon_state = BUZZON_STATE_OK;
                        rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                        LOG_D("gunno(%d) start charge by local pile number card", gunno);
                    }else{
                        if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                            s_buzzon_state = BUZZON_STATE_OK;
                            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                            LOG_D("gunno(%d) start charge by local whitelist card", gunno);
                        }else{
                            need_authorize_online = 0x01;
                        }
                    }
                }else{
                    if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                        s_buzzon_state = BUZZON_STATE_OK;
                        rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                        LOG_D("gunno(%d) start charge by local whitelist card", gunno);
                    }else{
                        need_authorize_online = 0x01;
                    }
                }

                compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                compare_len = compare_len > card_number_len ? card_number_len : compare_len;
                memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
                memcpy(s_ofsm_info[gunno].base.card_number, card_number, compare_len);

                s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_FALSE;

                if(need_authorize_online == 0x00){
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_ofsm_info[gunno].base.account_balance = 0x00;
                    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                    memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                    memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

                    compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

                    compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                    is_charging_authorization = true;
                }
            }else if(get_card_info_type() == CARD_INFO_TYPE_CARD_UID){
                uint8_t uid_len = get_card_uid_len(), compare_len = 0x00;

                if(sys_card_uid_whitelists_query(get_card_uid(), uid_len) >= 0x00){
                    s_buzzon_state = BUZZON_STATE_OK;
                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                    LOG_D("gunno(%d) start charge by local whitelist card uid", gunno);
                }else{
                    need_authorize_online = 0x01;
                }

                uid_len = uid_len > sizeof(s_ofsm_info[gunno].base.card_uid) ? sizeof(s_ofsm_info[gunno].base.card_uid) : uid_len;
                memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                memcpy(s_ofsm_info[gunno].base.card_uid, get_card_uid(), uid_len);
                s_ofsm_info[gunno].base.card_uid_len = uid_len;
                s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_TRUE;

                if(need_authorize_online == 0x00){
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_ofsm_info[gunno].base.account_balance = 0x00;
                    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                    compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

                    compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                    is_charging_authorization = true;
                }
            }

            if(need_authorize_online){
                if(app_nsal_card_authorize(gunno) < 0x00){
                    s_buzzon_state = BUZZON_STATE_FAILED;
                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                    LOG_W("gunno(%d) swip card authorize fail", gunno);
                    return;
                }
                s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_TRUE;
            }
        }else if(app_nsal_is_card_authorize_success(gunno)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by online card", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_ONLINE_CARD;

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.user_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : valid_len;
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
            memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_ONLINE_CARD;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

            is_charging_authorization = true;

        }else if(app_nsal_is_card_authorize_fail(gunno)){
            LOG_W("gunno(%d) swip card authorize response fail", gunno);
            s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;
            app_nsal_clear_remote_card_authorize(gunno);

            s_buzzon_state = BUZZON_STATE_FAILED;
            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
        }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by screen", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_SCREEN;
            s_ofsm_info[gunno].base.account_balance = 0x00;
            s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
            s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

            app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

            memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
            memset(&(s_ofsm_info[gunno].base.card_uid), 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
            memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_SCREEN;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

            is_charging_authorization = true;

        }else if(app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by VIN", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.vin_is_authorized = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_VIN;
            s_ofsm_info[gunno].base.account_balance = 0x00;
            s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
            s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

            app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

            memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
            memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
            memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
            memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_VIN;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

            is_charging_authorization = true;
        }else if(app_nsal_is_set_reservation(gunno)){
            app_nsal_clear_set_reservation(gunno);
#if 0
            LOG_D("gunno(%d) reservation start", gunno);
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
            s_ofsm_info[gunno].base.flag.is_reservation = APP_THA_ENUM_TRUE;

            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_RESERVATION];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_RESERVATION;
#endif
        }

        /************** 【充电桩已授权】 *************/
        if(is_charging_authorization == true){
            uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;

            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STARTING;

            /* 启动前向屏幕对时 */
            thaisen_request_screen_time();

            s_ofsm_info[gunno].base.main_gunno = gunno;
            s_ofsm_info[gunno].base.charge_way = thaisen_get_charge_way();
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
                if(APP_SYSTEM_GUNNO_SIZE < 0x02){
                    s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 单枪桩不支持并充 */
                }else{
                    if(gunno == APP_SYSTEM_GUNNOA){
                        deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                    }
                    if((s_ofsm_info[deputy_gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT)){
                        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 并充时两把枪都要插上 */
                    }
                    if((s_ofsm_info[deputy_gunno].state != APP_OFSM_STATE_READYING)){
                        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 并充时两把枪都要插上 */
                    }
                }
            }
            s_ofsm_info[deputy_gunno].base.charge_way = s_ofsm_info[gunno].base.charge_way;
            s_ofsm_info[deputy_gunno].base.main_gunno = s_ofsm_info[gunno].base.main_gunno;
            thaisen_set_charge_way(s_ofsm_info[gunno].base.charge_way);

            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
                if(gunno == s_ofsm_info[gunno].base.main_gunno){
                    s_ofsm_info[gunno].base.start_elect = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                }
                s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_PARACHARGE;
            }else{
                s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_SINGLEGUN;
                s_ofsm_info[gunno].base.start_elect = mw_get_meter_total_wh(gunno);
            }

            s_ofsm_info[gunno].base.flag.is_starting = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.voltage_a = 0x00;
            s_ofsm_info[gunno].base.current_a = 0x00;
            s_ofsm_info[gunno].base.power_a = 0x00;
            s_ofsm_info[gunno].base.elect_a = 0x00;
            s_ofsm_info[gunno].base.fees_total = 0x00;
            s_ofsm_info[gunno].base.elect_fees_total = 0x00;
            s_ofsm_info[gunno].base.service_fees_total = 0x00;
            s_ofsm_info[gunno].base.current_elect = mw_get_meter_total_wh(gunno);
            s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.charge_time = 0x00;
            s_ofsm_info[gunno].base.reason_code = APP_SYSTEM_STOP_WAY_SIZE;
            memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
            s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
            s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;
            s_ofsm_info[gunno].base.period_num = 0x01;
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
            s_ofsm_info[gunno].base.charctrl_state.last = APP_CHARGE_CTRL_STATE_IDLE;
            s_ofsm_info[gunno].base.charctrl_state.current = APP_CHARGE_CTRL_STATE_IDLE;
            s_ofsm_info[gunno].base.start_charge_tick = rt_tick_get();

            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
            s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
            s_thaisen_transaction[gunno].charge_time = 0x00;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            memset(s_thaisen_transaction[gunno].car_vin, 0x00, sizeof(s_thaisen_transaction[gunno].car_vin));
            s_thaisen_transaction[gunno].start_soc = 0x00;
            s_thaisen_transaction[gunno].stop_soc = 0x00;
            s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
            s_thaisen_transaction[gunno].ammeter_stop = s_thaisen_transaction[gunno].ammeter_start;
            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_POWER_OFF;
            s_thaisen_transaction[gunno].total_elect = 0x00;
            s_thaisen_transaction[gunno].total_loss_elect = 0x00;
            s_thaisen_transaction[gunno].charge_fee = 0x00;
            s_thaisen_transaction[gunno].service_fee = 0x00;
            s_thaisen_transaction[gunno].total_fee = 0x00;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
            defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_unit[type] = app_billingrule_get_rate_type_price(gunno, type);
            }
            memset(s_thaisen_transaction[gunno].rate_type_elect, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_elect));
            memset(s_thaisen_transaction[gunno].rate_type_amount, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_amount));
            memset(s_thaisen_transaction[gunno].rate_type_loss_elect, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_loss_elect));
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
            defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
            memset(s_thaisen_transaction[gunno].period_elect, 0x00, sizeof(s_thaisen_transaction[gunno].period_elect));
            memset(s_thaisen_transaction[gunno].period_elect_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_elect_fees));
            memset(s_thaisen_transaction[gunno].period_service_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_service_fees));
            memset(s_thaisen_transaction[gunno].period_occupy_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_occupy_fees));
            s_thaisen_transaction[gunno].rule = app_billingrule_get_rule(gunno);
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#ifdef APP_INCLUDE_XJ_PROTOCOL
            s_thaisen_transaction[gunno].delay_fee = 0x00;
#endif /* APP_INCLUDE_XJ_PROTOCOL */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
            s_thaisen_transaction[gunno].card_ballance_before = 0x00;
            s_thaisen_transaction[gunno].card_ballance_after = 0x00;
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_elect_amount[type] = 0x00;
                s_thaisen_transaction[gunno].rate_type_service_amount[type] = 0x00;
            }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */

            s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;
            s_thaisen_transaction[gunno].period_count = s_ofsm_info[gunno].base.period_num;

            s_thaisen_transaction[gunno].order_state.verify_fail = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].order_state.is_start_fail = APP_THA_ENUM_TRUE;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_TRUE;

            app_nsal_init_charge_data(gunno);

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);

            mw_storage_record_create(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                    0x00, gunno);
            s_current_order_index[MONITOR_PLATFORM_INDEX][gunno] = mw_storage_record_get_current_index(gunno);
            s_current_order_index[TARGET_PLATFORM_INDEX][gunno] = s_current_order_index[MONITOR_PLATFORM_INDEX][gunno];
            memset(&(s_thaisen_transaction[gunno].bms_stop_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_stop_reason));
            memset(&(s_thaisen_transaction[gunno].bms_fault_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_fault_reason));
            s_booting_step[gunno] = APP_BOOTING_STEP_IDLE;                 /* 初始化充电步骤 */
        }
        break;
    default:
        break;
    }

    if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT){
        s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
        app_nsal_event_occurded(gunno);
    }
    if((s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_READYING) && (is_charging_authorization == false)){
        s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_READYING;
        app_nsal_state_charged(gunno);
    }

    if(is_charging_authorization == true){
        clear_swipe_card_state(gunno);
        app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
        app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
        app_nsal_clear_remote_start(gunno);
        app_nsal_clear_remote_card_authorize(gunno);
    }else{
        mw_clear_time_sync_flag(gunno);
    }
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_nsal_clear_remote_stop(gunno);
}

static void ofsm_reservation_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }
    bool is_charging_authorization = APP_THA_ENUM_FALSE;  /* 充电已授权 */
    enum charge_state_t charge_state = mw_get_charge_state(gunno);

    if (++s_debug_count[gunno] > (1000  + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) reservation state (%dV | S%d)...", gunno, mw_get_cc1_value(mw_get_cc1(gunno)), charge_state);
    }

    if((s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_OVERHAUL) ||
            (s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_FREEZE)){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }

    if(app_nsal_is_cancel_reservation(gunno)){
        app_nsal_is_cancel_reservation(gunno);

        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_RESERVATION];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_RESERVATION;
    }
    if(s_ofsm_info[gunno].base.reservation_strategy &APP_RESERVATE_STRATEGY_PULLGUN_CANCEL){

    }
    s_ofsm_info[gunno].charge_timeout = rt_tick_get();



    if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT){
        s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
        app_nsal_event_occurded(gunno);
    }
    if((s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_RESERVATION) && (is_charging_authorization == false)){
        s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_RESERVATION;
        app_nsal_state_charged(gunno);
    }

    if(is_charging_authorization == true){
        clear_swipe_card_state(gunno);
        app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
        app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
        app_nsal_clear_remote_start(gunno);
        app_nsal_clear_remote_card_authorize(gunno);
    }else{
        mw_clear_time_sync_flag(gunno);
    }
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_nsal_clear_remote_stop(gunno);
}

static void ofsm_starting_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    enum charge_state_t charge_state = mw_get_charge_state(gunno);
    s_ofsm_info[gunno].base.charctrl_state.current = charge_state;

    if (++s_debug_count[gunno] > (1000  + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) starting state (%dV | S%d)...", gunno, mw_get_cc1_value(mw_get_cc1(gunno)), charge_state);
    }

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
#if 0
                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);
#endif /* 0 */
            }
        }
        if(gunno != s_ofsm_info[gunno].base.main_gunno){
            return;
        }
    }

    uint8_t vin_authentication_complete = APP_THA_ENUM_FALSE;
    enum system_stop_way stop_way = APP_SYSTEM_STOP_WAY_SIZE;
    enum charge_fault_t charge_fault = app_get_highest_priority_charge_fault(gunno);
    enum system_fault_t system_fault = app_get_highest_priority_system_fault(gunno);

    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
    s_tiny_current_count[gunno] = rt_tick_get();

    if(s_ofsm_info[gunno].base.flag.permit_judge_complete == APP_THA_ENUM_FALSE){
        if(mw_module_get_permit_charge_state(gunno) == APP_MODULE_CHARGE_SIZE){
            if(s_ofsm_info[gunno].charge_timeout > rt_tick_get()){
                s_ofsm_info[gunno].charge_timeout = rt_tick_get();
            }

            if((rt_tick_get() - s_ofsm_info[gunno].charge_timeout) > 20000){
                s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_TRUE;
                s_ofsm_info[gunno].charge_timeout = rt_tick_get();
            }
        }else if(mw_module_get_permit_charge_state(gunno) == APP_MODULE_FORBID_CHARGE){
            s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
        }else{
            s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
        }
    }

    if(s_ofsm_info[gunno].base.flag.permit_judge_complete == APP_THA_ENUM_TRUE){
        if(mw_module_get_permit_charge_state(gunno) != APP_MODULE_ALLOW_CHARGE){
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.system_fault = APP_SYS_FAULT_NO_ERROR;
            s_ofsm_info[gunno].base.charge_fault = APP_CHARGE_FAULT_NO_ERROR;
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

            LOG_D("gunno(%d) charge stop deal to stop way(main cabinet forbid)|%d\n", gunno, APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID);

            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID;
            s_ofsm_info[gunno].base.reason_code = APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID;

            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
            s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
            s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
            s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

            /* 对时后时间要修改 */
            if(mw_get_time_sync_flag(gunno)){
                mw_clear_time_sync_flag(gunno);
                uint32_t curr_time = mw_get_current_timestamp();

                s_ofsm_info[gunno].base.stop_time = curr_time;
                if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                    s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
                }else{
                    /* 这种情况是不对的 */
                    s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
                }
                s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
                s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

                app_nsal_time_sync_revise(gunno);

                /** 与时段有关的信息也要更新 */
                LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
            }

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
            return;
        }else{
            if(s_ofsm_info[gunno].base.flag.is_starting == APP_THA_ENUM_FALSE){
                s_ofsm_info[gunno].base.flag.is_starting = APP_THA_ENUM_TRUE;
                /* 开始充电 */
                mw_charge_start_cmd(gunno);
            }
        }
    }

    /** 余额不足判断 */
    if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_MONEY){
        if((rt_tick_get() - s_ofsm_info[gunno].charge_timeout) > 15000){   /* 保证已对时完成 */
            uint8_t period = app_calculate_current_period(s_ofsm_info[gunno].base.current_time);
            uint8_t rate_type = app_billingrule_get_period_rate_number(gunno, period);
            uint32_t unit = app_billingrule_get_rate_type_price(gunno, rate_type);

            if(unit > (s_ofsm_info[gunno].base.charge_strategy_para)){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
                s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

                s_ofsm_info[gunno].base.system_fault = system_fault;
                s_ofsm_info[gunno].base.charge_fault = charge_fault;
                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

                /** 防止状态异常，上层已停止但下层还在充电 */
                mw_charge_stop_cmd(gunno);

                LOG_D("gunno(%d) start fail deal to no ballance|%d, %d, %d, %d\n", gunno, s_ofsm_info[gunno].base.charge_strategy_para,
                        period, rate_type, unit);

                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_NO_BALLANCE;
                s_ofsm_info[gunno].base.reason_code = APP_SYSTEM_STOP_WAY_NO_BALLANCE;

                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
                s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
                s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
                s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
                s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
                s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
                s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
                s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                /* 对时后时间要修改 */
                if(mw_get_time_sync_flag(gunno)){
                    mw_clear_time_sync_flag(gunno);
                    uint32_t curr_time = mw_get_current_timestamp();

                    s_ofsm_info[gunno].base.stop_time = curr_time;
                    if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                        s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
                    }else{
                        /* 这种情况是不对的 */
                        s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
                    }
                    s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
                    s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

                    app_nsal_time_sync_revise(gunno);

                    /** 与时段有关的信息也要更新 */
                    LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
                }

                app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

                app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                        s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);
                return;
            }
        }
    }

    switch(charge_state){
    case APP_CHARGE_STATE_IDLE:
        if(s_ofsm_info[gunno].charge_timeout > rt_tick_get()){
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
        }
        if((rt_tick_get() - s_ofsm_info[gunno].charge_timeout) > 50000){
            stop_way = mw_get_system_stop_way(gunno);
            if(stop_way != APP_SYSTEM_STOP_WAY_NULL){
                /** 已有停充原因，充电已结束，充电失败，退出 */
            }else if((rt_tick_get() - s_ofsm_info[gunno].charge_timeout) <= 90000){
                break;
            }
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.system_fault = system_fault;
            s_ofsm_info[gunno].base.charge_fault = charge_fault;
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

            /** 防止状态异常，上层已停止但下层还在充电 */
            mw_charge_stop_cmd(gunno);
            if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
                uint8_t rentry = 0x00;
                while(rentry <= 200){  /** 最多等待 10s，等下层赋值完停充原因 */
                    rt_thread_mdelay(500);
                    stop_way = mw_get_system_stop_way(gunno);
                    if(stop_way != APP_SYSTEM_STOP_WAY_NULL){
                        break;
                    }
                    rentry++;
                }
            }

            LOG_D("gunno(%d) charge stop deal to stop way(boot timeout)|%d\n", gunno, stop_way);

            s_thaisen_transaction[gunno].stop_reason = stop_way;
            s_ofsm_info[gunno].base.reason_code = stop_way;

            if((stop_way < APP_SYSTEM_STOP_WAY_NULL) &&
                    (stop_way != APP_SYSTEM_STOP_WAY_PULL_GUN) &&
                    (stop_way != APP_SYSTEM_STOP_WAY_CHARGE_FULL) &&
                    (stop_way != APP_SYSTEM_STOP_WAY_PASSIVE) &&
                    (stop_way != APP_SYSTEM_STOP_WAY_BST)){
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;
            }else{
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
            }
            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
            s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
            s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
            s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

            /* 对时后时间要修改 */
            if(mw_get_time_sync_flag(gunno)){
                mw_clear_time_sync_flag(gunno);
                uint32_t curr_time = mw_get_current_timestamp();

                s_ofsm_info[gunno].base.stop_time = curr_time;
                if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                    s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
                }else{
                    /* 这种情况是不对的 */
                    s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
                }
                s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
                s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

                app_nsal_time_sync_revise(gunno);

                /** 与时段有关的信息也要更新 */
                LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
            }

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
            return;
        }
        s_booting_step[gunno] = APP_BOOTING_STEP_IDLE;
        break;
    case APP_CHARGE_STATE_SHAKE_HAND:
        s_booting_step[gunno] = APP_BOOTING_STEP_HAND;
        break;
    case APP_CHARGE_STATE_INSULATION:
        app_nsal_report_bms_message_bmsinfo(gunno);
        s_booting_step[gunno] = APP_BOOTING_STEP_INSULT;
        break;
    case APP_CHARGE_STATE_CONFIGURE:
        if(s_booting_step[gunno] <= APP_BOOTING_STEP_HAND){
            app_nsal_report_bms_message_bmsinfo(gunno);
        }
        s_booting_step[gunno] = APP_BOOTING_STEP_CONFIG;
        break;
    case APP_CHARGE_STATE_CHARGING:
    {
        switch(s_booting_step[gunno]){
        case APP_BOOTING_STEP_IDLE:
        case APP_BOOTING_STEP_HAND:
            app_nsal_report_bms_message_bmsinfo(gunno);
            app_nsal_report_bms_message_bmsparameter(gunno);
            break;
        case APP_BOOTING_STEP_INSULT:
        case APP_BOOTING_STEP_CONFIG:
            if(s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_VIN){
                if(s_ofsm_info[gunno].base.flag.vin_is_authorized == APP_THA_ENUM_FALSE){
                    app_nsal_report_bms_message_bmsparameter(gunno);
                }
            }else{
                app_nsal_report_bms_message_bmsparameter(gunno);
            }
            break;
        default:
            break;
        }

        s_booting_step[gunno] = APP_BOOTING_STEP_CONFIG;
        if(s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_VIN){
            if(s_ofsm_info[gunno].base.flag.vin_is_authorized == APP_THA_ENUM_FALSE){
                struct thaisenBMS_Charger_struct* bms = (struct thaisenBMS_Charger_struct*)(s_ofsm_info[gunno].base.bms_data);
                memcpy(s_ofsm_info[gunno].base.car_vin, bms->BRM.CarDiscern, sizeof(s_ofsm_info[gunno].base.car_vin));
                memcpy(s_thaisen_transaction[gunno].car_vin, bms->BRM.CarDiscern, sizeof(bms->BRM.CarDiscern));
                if(sys_vin_whitelists_query(bms->BRM.CarDiscern, sizeof(bms->BRM.CarDiscern)) >= 0x00){
                    vin_authentication_complete = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_TRUE;
                    LOG_D("gunno(%d) local VIN(%s) start", gunno, bms->BRM.CarDiscern);
                }else{
                    if(s_ofsm_info[gunno].base.net_state != APP_NET_STATE_AUTH_SECCESS){
                        vin_authentication_complete = APP_THA_ENUM_TRUE;
                    }
                    if(app_nsal_vin_authorize(gunno) < 0x00){
                        vin_authentication_complete = APP_THA_ENUM_TRUE;
                    }
                    s_ofsm_info[gunno].base.start_charge_tick = rt_tick_get();
                }
                s_ofsm_info[gunno].base.flag.vin_is_authorized = APP_THA_ENUM_TRUE;
            }
            if((rt_tick_get() - s_ofsm_info[gunno].base.start_charge_tick) > 20 *1000){ /* 超时未接到鉴权回复 */
                LOG_W("gunno(%d) vin charge authentication timeout", gunno);
                vin_authentication_complete = APP_THA_ENUM_TRUE;
            }

            if(app_nsal_is_vin_authorize_success(gunno)){
                uint8_t valid_len = 0x00;
                LOG_D("gunno(%d) vin charge authentication success", gunno);
                s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
                s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_TRUE;

                valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
                memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

                app_nsal_clear_remote_vin_authorize(gunno);
                app_nsal_init_charge_data(gunno);  /* 重新赋值流水号 */
                vin_authentication_complete = APP_THA_ENUM_TRUE;
            }else if(app_nsal_is_vin_authorize_fail(gunno)){
                app_nsal_clear_remote_vin_authorize(gunno);
                LOG_W("gunno(%d) vin charge authentication fail", gunno);
                vin_authentication_complete = APP_THA_ENUM_TRUE;
            }
        }else{
            vin_authentication_complete = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_TRUE;
            struct thaisenBMS_Charger_struct* bms = (struct thaisenBMS_Charger_struct*)(s_ofsm_info[gunno].base.bms_data);
            memcpy(s_ofsm_info[gunno].base.car_vin, bms->BRM.CarDiscern, sizeof(s_ofsm_info[gunno].base.car_vin));
            memcpy(s_thaisen_transaction[gunno].car_vin, bms->BRM.CarDiscern, sizeof(bms->BRM.CarDiscern));
        }

        if((vin_authentication_complete == APP_THA_ENUM_FALSE) && (s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_VIN)){
            break;
        }

        if(s_ofsm_info[gunno].base.flag.vin_authorization_success == APP_THA_ENUM_FALSE){
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            mw_charge_stop_cmd(gunno);

            s_ofsm_info[gunno].base.system_fault = system_fault;
            s_ofsm_info[gunno].base.charge_fault = charge_fault;
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_AUTHEN_FAIL;
            s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;

            LOG_D("gunno(%d) charge stop deal to vin authorization fail", gunno);

            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
            s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
            s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
            s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

            /* 对时后时间要修改 */
            if(mw_get_time_sync_flag(gunno)){
                mw_clear_time_sync_flag(gunno);
                uint32_t curr_time = mw_get_current_timestamp();

                s_ofsm_info[gunno].base.stop_time = curr_time;
                if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                    s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
                }else{
                    /* 这种情况是不对的 */
                    s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
                }
                s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
                s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

                app_nsal_time_sync_revise(gunno);

                LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
            }

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
            return;
        }

        s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_TRUE;
        s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
        s_thaisen_transaction[gunno].order_state.is_start_fail = APP_THA_ENUM_FALSE;

        s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
        s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
        s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
        s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;
        s_ofsm_info[gunno].timing_tick = rt_tick_get();
        s_ofsm_info[gunno].base.offline_tick = rt_tick_get();
        s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
        s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
        s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_CHARGING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_CHARGING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

        app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);

        return;
    }
    case APP_CHARGE_STATE_FINISH:
        break;
    case APP_CHARGE_STATE_FAULTING:
    case APP_CHARGE_STATE_WAIT_PULL_GUN:
    {
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

        s_ofsm_info[gunno].base.system_fault = system_fault;
        s_ofsm_info[gunno].base.charge_fault = charge_fault;
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        stop_way = mw_get_system_stop_way(gunno);

        /** 防止状态异常，上层已停止但下层还在充电 */
        mw_charge_stop_cmd(gunno);
        if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
            uint8_t rentry = 0x00;
            while(rentry <= 200){  /** 最多等待 10s，等下层赋值完停充原因 */
                rt_thread_mdelay(500);
                stop_way = mw_get_system_stop_way(gunno);
                if(stop_way != APP_SYSTEM_STOP_WAY_NULL){
                    break;
                }
                rentry++;
            }
        }

        s_thaisen_transaction[gunno].stop_reason = stop_way;
        s_ofsm_info[gunno].base.reason_code = stop_way;

        LOG_D("gunno(%d) charge stop deal to stop way|%d\n", gunno, stop_way);

        if((stop_way < APP_SYSTEM_STOP_WAY_NULL) &&
                (stop_way != APP_SYSTEM_STOP_WAY_PULL_GUN) &&
                (stop_way != APP_SYSTEM_STOP_WAY_CHARGE_FULL) &&
                (stop_way != APP_SYSTEM_STOP_WAY_PASSIVE) &&
                (stop_way != APP_SYSTEM_STOP_WAY_BST)){
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;
        }else{
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
        }
        s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
        s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
        s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

        s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
        s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
        s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
        s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

        s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
        s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
        s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

        /* 对时后时间要修改 */
        if(mw_get_time_sync_flag(gunno)){
            mw_clear_time_sync_flag(gunno);
            uint32_t curr_time = mw_get_current_timestamp();

            s_ofsm_info[gunno].base.stop_time = curr_time;
            if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
            }else{
                /* 这种情况是不对的 */
                s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
            }
            s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

            app_nsal_time_sync_revise(gunno);

            /** 与时段有关的信息也要更新 */
            LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
        }

        app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

        app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
    }
    default:
        break;
    }

    if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT){
        s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
        app_nsal_event_occurded(gunno);
    }
    if(s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_STARTING){
        s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_STARTING;
        app_nsal_state_charged(gunno);
    }
    if(s_ofsm_info[gunno].base.charctrl_state.last != s_ofsm_info[gunno].base.charctrl_state.current){
        s_ofsm_info[gunno].base.charctrl_state.last = s_ofsm_info[gunno].base.charctrl_state.current;
        app_nsal_state_charged(gunno);
    }

    mw_clear_time_sync_flag(gunno);
    clear_swipe_card_state(gunno);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

    app_nsal_clear_remote_start(gunno);
    app_nsal_clear_remote_card_authorize(gunno);
    app_nsal_clear_remote_vin_authorize(gunno);
}

static void ofsm_charging_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    enum charge_state_t charge_state = mw_get_charge_state(gunno);

    if (++s_debug_count[gunno] > (3000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) charging state (%dV | S%d | E|%u, F|%u V|%d C|%d P|%d charge_time(%d))...", gunno, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state,
                s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.fees_total, s_ofsm_info[gunno].base.voltage_a,
                s_ofsm_info[gunno].base.current_a, s_ofsm_info[gunno].base.power_a, s_ofsm_info[gunno].base.charge_time);
    }

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
#if 0
                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);
#endif /* 0 */
            }
        }
        if(gunno != s_ofsm_info[gunno].base.main_gunno){
            s_ofsm_info[gunno].base.voltage_a = mw_get_meter_ua(gunno) *10;
            return;
        }
    }

    uint32_t current_tick = rt_tick_get(), increase_sec = 0;
    bool is_stop_charge_authorization = APP_THA_ENUM_FALSE;  /* 停充已授权 */
    enum system_stop_way stop_way = APP_SYSTEM_STOP_WAY_SIZE;
    enum charge_fault_t charge_fault = app_get_highest_priority_system_fault(gunno);
    enum system_fault_t system_fault = app_get_highest_priority_charge_fault(gunno);
    struct thaisenBMS_Charger_struct* bms_info = (struct thaisenBMS_Charger_struct*)(s_ofsm_info[gunno].base.bms_data);

    if(s_ofsm_info[gunno].timing_tick > current_tick){
        increase_sec = ((current_tick + 0xFFFFFFFF) - s_ofsm_info[gunno].timing_tick) /1000;
    }else{
        increase_sec = (current_tick - s_ofsm_info[gunno].timing_tick) /1000;
    }

    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
        uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
        if(gunno == APP_SYSTEM_GUNNOA){
            deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
        }
        app_billing_info_calculate((s_ofsm_info[gunno].base.start_time + increase_sec),   \
                (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno)), gunno);
        s_ofsm_info[gunno].base.current_a = (mw_get_meter_ia(gunno) *10 + mw_get_meter_ia(deputy_gunno) *10);
        s_ofsm_info[gunno].base.power_a = (mw_get_meter_pa(gunno) + mw_get_meter_pa(deputy_gunno));
    }else{
        app_billing_info_calculate((s_ofsm_info[gunno].base.start_time + increase_sec), mw_get_meter_total_wh(gunno), gunno);
        s_ofsm_info[gunno].base.current_a = mw_get_meter_ia(gunno) *10;
        s_ofsm_info[gunno].base.power_a = mw_get_meter_pa(gunno);
    }

    s_ofsm_info[gunno].base.voltage_a = mw_get_meter_ua(gunno) *10;
    s_ofsm_info[gunno].base.current_soc = bms_info->BCS.SOC;
    s_ofsm_info[gunno].base.stop_time = (s_ofsm_info[gunno].base.start_time + increase_sec);
    s_ofsm_info[gunno].base.charge_time = s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.start_time;
    s_ofsm_info[gunno].base.current_elect = app_billingrule_get_stop_elcet(gunno);
    s_ofsm_info[gunno].base.fees_total = app_billingrule_get_fees_total(gunno);
    s_ofsm_info[gunno].base.service_fees_total = app_billingrule_get_service_fees_total(gunno);
    if(s_ofsm_info[gunno].base.fees_total > s_ofsm_info[gunno].base.service_fees_total){
        s_ofsm_info[gunno].base.elect_fees_total = (s_ofsm_info[gunno].base.fees_total - s_ofsm_info[gunno].base.service_fees_total);
    }
    s_ofsm_info[gunno].base.elect_a = app_billingrule_get_elcet_total(gunno);
    if(s_ofsm_info[gunno].base.current_period != app_calculate_current_period((s_ofsm_info[gunno].base.start_time + increase_sec))){
        s_ofsm_info[gunno].base.current_period = app_calculate_current_period((s_ofsm_info[gunno].base.start_time + increase_sec));
        if(s_ofsm_info[gunno].base.period_num < APP_BILLING_RULE_PERIOD_MAX){
            s_ofsm_info[gunno].base.period_num++;
        }
    }

    s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
    s_thaisen_transaction[gunno].charge_time = s_ofsm_info[gunno].base.charge_time;
    s_thaisen_transaction[gunno].stop_soc = s_ofsm_info[gunno].base.current_soc;
    s_thaisen_transaction[gunno].ammeter_stop = s_ofsm_info[gunno].base.current_elect;
    s_thaisen_transaction[gunno].total_elect = s_ofsm_info[gunno].base.elect_a;
    s_thaisen_transaction[gunno].total_loss_elect = 0x00;;
    s_thaisen_transaction[gunno].charge_fee = s_ofsm_info[gunno].base.elect_fees_total;
    s_thaisen_transaction[gunno].service_fee = s_ofsm_info[gunno].base.service_fees_total;
    s_thaisen_transaction[gunno].total_fee = s_ofsm_info[gunno].base.fees_total;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
    defined (APP_INCLUDE_SGCC_PROTOCOL))
    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
        s_thaisen_transaction[gunno].rate_type_elect[type] = app_billingrule_get_rate_type_elect(gunno, type);
        s_thaisen_transaction[gunno].rate_type_amount[type] = app_billingrule_get_rate_type_fess(gunno, type);
        s_thaisen_transaction[gunno].rate_type_loss_elect[type] = 0x00;
    }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
    defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
    for(uint8_t period = s_ofsm_info[gunno].base.start_period; period <= s_ofsm_info[gunno].base.current_period; period++){
        s_thaisen_transaction[gunno].period_elect[period] = app_billingrule_get_period_elect(gunno, period);
        s_thaisen_transaction[gunno].period_elect_fees[period] = app_billingrule_get_period_elect_fees(gunno, period);
        s_thaisen_transaction[gunno].period_service_fees[period] = app_billingrule_get_period_service_fees(gunno, period);
    }
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */
    s_thaisen_transaction[gunno].period_count = s_ofsm_info[gunno].base.period_num;

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
        s_thaisen_transaction[gunno].rate_type_elect_amount[type] = app_billingrule_get_rate_type_elect_fess(gunno, type);
        s_thaisen_transaction[gunno].rate_type_service_amount[type] = app_billingrule_get_rate_type_service_fess(gunno, type);
    }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */

    /* 对时后时间要修改 */
    if(mw_get_time_sync_flag(gunno)){
        mw_clear_time_sync_flag(gunno);
        uint32_t curr_time = mw_get_current_timestamp();

        s_ofsm_info[gunno].base.stop_time = curr_time;
        if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
            s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
        }else{
            /* 这种情况是不对的 */
            s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
        }
        s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
        s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

        app_nsal_time_sync_revise(gunno);

        /** 与时段有关的信息也要更新 */
        LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
    }

    app_nsal_padding_charge_data(gunno);
    app_nsal_report_bms_message_bmsrequire_pileoutput(gunno, 0x00);
    app_nsal_report_bms_message_bmsstate(gunno, 0x00);

    switch (charge_state){
    case APP_CHARGE_STATE_IDLE:
    {
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

        s_ofsm_info[gunno].base.system_fault = system_fault;
        s_ofsm_info[gunno].base.charge_fault = charge_fault;
        s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        stop_way = mw_get_system_stop_way(gunno);

        /** 防止状态异常，上层已停止但下层还在充电 */
        mw_charge_stop_cmd(gunno);
        if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
            uint8_t rentry = 0x00;
            while(rentry <= 200){  /** 最多等待 10s，等下层赋值完停充原因 */
                rt_thread_mdelay(500);
                stop_way = mw_get_system_stop_way(gunno);
                if(stop_way != APP_SYSTEM_STOP_WAY_NULL){
                    break;
                }
                rentry++;
            }
        }

        if(stop_way != APP_SYSTEM_STOP_WAY_PASSIVE){
            s_thaisen_transaction[gunno].stop_reason = stop_way;
            s_ofsm_info[gunno].base.reason_code = stop_way;
        }else{
            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_PULL_GUN;
            s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
        }

        LOG_D("gunno(%d) charge finish deal to pull gun\n", gunno);

        /* 对时后时间要修改 */
        if(mw_get_time_sync_flag(gunno)){
            mw_clear_time_sync_flag(gunno);
            uint32_t curr_time = mw_get_current_timestamp();

            s_ofsm_info[gunno].base.stop_time = curr_time;
            if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
            }else{
                /* 这种情况是不对的 */
                s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
            }
            s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

            app_nsal_time_sync_revise(gunno);

            /** 与时段有关的信息也要更新 */
            LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
        }

        s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
        break;
    }
    case APP_CHARGE_STATE_CHARGING:
        break;
    case APP_CHARGE_STATE_FAULTING:
    case APP_CHARGE_STATE_WAIT_PULL_GUN:
    {
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

        s_ofsm_info[gunno].base.system_fault = system_fault;
        s_ofsm_info[gunno].base.charge_fault = charge_fault;
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        stop_way = mw_get_system_stop_way(gunno);

        /** 防止状态异常，上层已停止但下层还在充电 */
        mw_charge_stop_cmd(gunno);
        if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
            uint8_t rentry = 0x00;
            while(rentry <= 200){  /** 最多等待 10s，等下层赋值完停充原因 */
                rt_thread_mdelay(500);
                stop_way = mw_get_system_stop_way(gunno);
                if(stop_way != APP_SYSTEM_STOP_WAY_NULL){
                    break;
                }
                rentry++;
            }
        }

        LOG_D("gunno(%d) charge stop deal to stop way|%d\n", gunno, stop_way);

        s_thaisen_transaction[gunno].stop_reason = stop_way;
        s_ofsm_info[gunno].base.reason_code = stop_way;

        if((stop_way < APP_SYSTEM_STOP_WAY_NULL) &&
                (stop_way != APP_SYSTEM_STOP_WAY_PULL_GUN) &&
                (stop_way != APP_SYSTEM_STOP_WAY_CHARGE_FULL) &&
                (stop_way != APP_SYSTEM_STOP_WAY_PASSIVE) &&
                (stop_way != APP_SYSTEM_STOP_WAY_BST)){
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;
        }else{
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
        }

        /* 对时后时间要修改 */
        if(mw_get_time_sync_flag(gunno)){
            mw_clear_time_sync_flag(gunno);
            uint32_t curr_time = mw_get_current_timestamp();

            s_ofsm_info[gunno].base.stop_time = curr_time;
            if(s_ofsm_info[gunno].base.stop_time >= s_ofsm_info[gunno].base.charge_time){
                s_ofsm_info[gunno].base.start_time = (s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.charge_time);
            }else{
                /* 这种情况是不对的 */
                s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.stop_time;
            }
            s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;

            app_nsal_time_sync_revise(gunno);

            /** 与时段有关的信息也要更新 */
            LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
        }

        s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
        break;
    }
    default:
        break;
    }

    if(app_nsal_is_remote_stop(gunno)) {
        s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_APP_STOP;
        s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
        s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

        app_nsal_report_remote_stop_result(gunno, APP_THA_ENUM_TRUE, s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
        is_stop_charge_authorization = true;
        LOG_D("gunno(%d) charge finish deal to APP stop\n", gunno);

    }else if(get_swipe_card_state(gunno)){
        clear_swipe_card_state(gunno);
        if(get_card_info_type() == CARD_INFO_TYPE_CARD_NUMBER){
            uint8_t compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
            compare_len = compare_len > get_card_number_len() ? get_card_number_len() : compare_len;
            if(compare_len < CARD_NUMBER_COMPARE_LEN_MIN){
                s_buzzon_state = BUZZON_STATE_FAILED;
                rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                LOG_W("card number length is not enough|%d, %d", CARD_NUMBER_COMPARE_LEN_MIN, compare_len);
                return;
            }
            if(memcmp(s_ofsm_info[gunno].base.card_number, get_card_number(), compare_len)){
                s_buzzon_state = BUZZON_STATE_FAILED;
                rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                return;
            }
            s_buzzon_state = BUZZON_STATE_OK;
            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));

            if(s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_ONLINE_CARD){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_ONLINECARD_STOP;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                LOG_D("gunno(%d) charge finish deal to online card stop\n", gunno);
            }else{
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                LOG_D("gunno(%d) charge finish deal to offline card stop\n", gunno);
            }
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
            is_stop_charge_authorization = true;

        }else if(get_card_info_type() == CARD_INFO_TYPE_CARD_UID){
            uint8_t* uid = get_card_uid(), valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
            if(uid){
                valid_len = valid_len > get_card_uid_len() ? get_card_uid_len() : valid_len;
                if(memcmp(s_ofsm_info[gunno].base.card_uid, uid, valid_len) == 0){
                    if(s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_ONLINE_CARD){
                        s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_ONLINECARD_STOP;
                        s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                        LOG_D("gunno(%d) charge finish deal to online card(uid) stop\n", gunno);
                    }else{
                        s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP;
                        s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                        LOG_D("gunno(%d) charge finish deal to offline card(uid) stop\n", gunno);
                    }
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
                    is_stop_charge_authorization = true;

                }else{
                    s_buzzon_state = BUZZON_STATE_FAILED;
                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                    LOG_W("this is not the card of start charge on this gun(%d)", gunno);
                }
            }else{
                LOG_W("gunno error when get card uid in ready state");
            }
        }
    }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1)){
        s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_SCREEN_STOP;
        s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
        s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

        is_stop_charge_authorization = true;
        LOG_D("gunno(%d) charge finish deal to screen stop\n", gunno);

    }else if((s_ofsm_info[gunno].base.charge_strategy != APP_CHARGE_STRATEGY_FULL) &&   \
            (s_ofsm_info[gunno].base.charge_strategy != APP_CHARGE_STRATEGY_RESERVATION)){
        if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_TIME){
            if(increase_sec >= s_ofsm_info[gunno].base.charge_strategy_para){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_REACH_TIME;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to reach target time(%d, %d)\n", gunno, increase_sec, s_ofsm_info[gunno].base.charge_strategy_para);
            }
        }else if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_ELECT){
            if(s_ofsm_info[gunno].base.elect_a >= s_ofsm_info[gunno].base.charge_strategy_para){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_REACH_ELECT;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to reach target elect(%d, %d)\n", gunno, s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.charge_strategy_para);
            }
        }else if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_MONEY){
            if((s_ofsm_info[gunno].base.fees_total + 10000) > s_ofsm_info[gunno].base.charge_strategy_para){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_NO_BALLANCE;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to reach target money(%d, %d)\n", gunno, s_ofsm_info[gunno].base.charge_strategy_para, \
                        s_ofsm_info[gunno].base.fees_total);
            }
        }else if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_SOC){
            if((bms_info->BCS.SOC >= s_ofsm_info[gunno].base.charge_strategy_para) &&
                    (s_ofsm_info[gunno].base.charge_strategy_para != 100)){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_SOC_LIMIT;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to reach target SOC(%d, %d)\n", gunno, bms_info->BCS.SOC, s_ofsm_info[gunno].base.charge_strategy_para);
            }
        }
    }

    if(s_ofsm_info[gunno].base.current_a < TINY_CURRENT_ABNOAMAL_VALUE){
        if(s_tiny_current_count[gunno] > rt_tick_get()){
            s_tiny_current_count[gunno] = rt_tick_get();
        }
        if((rt_tick_get() - s_tiny_current_count[gunno]) > TINY_CURRENT_ABNOAMAL_TIMEOUT){
            s_tiny_current_count[gunno] = rt_tick_get();
            if(is_stop_charge_authorization == false){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to current abnormal\n", gunno);
            }
        }
    }else{
        s_tiny_current_count[gunno] = rt_tick_get();
    }

    if(bms_info->BCS.SOC >= *(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0))){
        if(*(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0)) < 100){
            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_SOC_LIMIT;
            s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

            is_stop_charge_authorization = true;
            LOG_D("gunno(%d) charge finish deal to reach soc protect value(%d)\n", gunno, *(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0)));
        }
    }

#ifdef APP_INCLUDE_NET
    if((s_ofsm_info[gunno].base.flag.is_local_charging == APP_THA_ENUM_FALSE) && app_nsal_offlinecharge_is_limit(gunno)){
        if(app_nsal_get_link_state() == APP_NET_STATE_AUTH_SECCESS){
            s_ofsm_info[gunno].base.offline_tick = rt_tick_get();
        }else{
            if(s_ofsm_info[gunno].base.offline_tick > rt_tick_get()){
                s_ofsm_info[gunno].base.offline_tick = rt_tick_get();
            }
            if((rt_tick_get() - s_ofsm_info[gunno].base.offline_tick) > s_ofsm_info[gunno].base.offline_chargetime *1000){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_OFFLINE_CHARGE_TIME;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to reach offline charge time(%d)\n", gunno, *(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0)));
            }
        }
    }
#endif /* APP_INCLUDE_NET */

    if(is_stop_charge_authorization == true){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

        mw_charge_stop_cmd(gunno);

        s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

        s_ofsm_info[gunno].base.system_fault = APP_SYS_FAULT_NO_ERROR;
        s_ofsm_info[gunno].base.charge_fault = APP_CHARGE_FAULT_NO_ERROR;
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
    }

    if((s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_CHARGING) && (is_stop_charge_authorization == false)){
        s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_CHARGING;
        app_nsal_state_charged(gunno);
    }
}

static void ofsm_stoping_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    bool is_stop_complete = APP_THA_ENUM_FALSE;  /* 充电已停止完成 */
    enum charge_state_t charge_state = mw_get_charge_state(gunno);

    if (++s_debug_count[gunno] > (1000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) stop state (%dV | S%d)...", gunno, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state);
    }

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];

                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_NONE;
            }
        }
        /** 在并充结束后要同步主枪信息给副枪用来在屏幕上显示 */
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].base.main_gunno < APP_SYSTEM_GUNNO_SIZE)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code == APP_SYSTEM_STOP_WAY_AUTHEN_FAIL){
                s_ofsm_info[gunno].base.reason_code = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code;
                s_ofsm_info[gunno].base.fees_total = 0x00;
                s_ofsm_info[gunno].base.elect_a = 0x00;
                s_ofsm_info[gunno].base.charge_time = 0x00;
            }else{
                s_ofsm_info[gunno].base.reason_code = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code;
                s_ofsm_info[gunno].base.fees_total = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.fees_total;
                s_ofsm_info[gunno].base.elect_a = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.elect_a;
                s_ofsm_info[gunno].base.charge_time = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.charge_time;
            }
        }

        if(gunno != s_ofsm_info[gunno].base.main_gunno){
            return;
        }
    }

    switch (charge_state){
    case APP_CHARGE_STATE_IDLE:
    case APP_CHARGE_STATE_SHAKE_HAND:
    case APP_CHARGE_STATE_INSULATION:
    case APP_CHARGE_STATE_CONFIGURE:
        is_stop_complete = APP_THA_ENUM_TRUE;
        break;
    case APP_CHARGE_STATE_FAULTING:
    case APP_CHARGE_STATE_SIZE:
        is_stop_complete = APP_THA_ENUM_TRUE;
        break;
    case APP_CHARGE_STATE_FINISH:
    case APP_CHARGE_STATE_WAIT_PULL_GUN:
        is_stop_complete = APP_THA_ENUM_TRUE;
        break;
    default:
        break;
    }

    if(is_stop_complete){
        switch (charge_state){
        case APP_CHARGE_STATE_IDLE:
        case APP_CHARGE_STATE_SHAKE_HAND:
        case APP_CHARGE_STATE_INSULATION:
        case APP_CHARGE_STATE_CONFIGURE:
        case APP_CHARGE_STATE_FAULTING:
        case APP_CHARGE_STATE_WAIT_PULL_GUN:
        case APP_CHARGE_STATE_SIZE:
            break;
        default:
            return;  /* 要等待充电结束相关报文交互完成 */
        }

        struct thaisenBMS_Charger_struct *bms = mw_get_bms_data(gunno);

        s_thaisen_transaction[gunno].bms_stop_reason.target_soc = bms->BST.SOCGetObj;
        s_thaisen_transaction[gunno].bms_stop_reason.target_tvolt = bms->BST.VoltGetObj;
        s_thaisen_transaction[gunno].bms_stop_reason.target_svolt = bms->BST.CeliVoltGetObj;
        s_thaisen_transaction[gunno].bms_stop_reason.chargerend = bms->BST.ChargInitiStop;

        s_thaisen_transaction[gunno].bms_fault_reason.insultion = bms->BST.InsltFault;
        s_thaisen_transaction[gunno].bms_fault_reason.olink_ot = bms->BST.OutConectOVtemp;
        s_thaisen_transaction[gunno].bms_fault_reason.bms_comp_olink_ot = bms->BST.BMSCompOVtemp;
        s_thaisen_transaction[gunno].bms_fault_reason.clink_fault = bms->BST.Conectfault;
        s_thaisen_transaction[gunno].bms_fault_reason.battery_ot = bms->BST.BatOVtemp;
        s_thaisen_transaction[gunno].bms_fault_reason.hv_relay = bms->BST.HVRelaysFault;
        s_thaisen_transaction[gunno].bms_fault_reason.detect_point2_volt = bms->BST.Check2Ft;
        s_thaisen_transaction[gunno].bms_fault_reason.other = bms->BST.OtherFt;

        app_nsal_report_bms_message_end(gunno);
        app_nsal_report_bms_message_error(gunno);
        app_nsal_report_bms_message_bmsend(gunno);
        app_nsal_report_bms_message_chargerend(gunno);

        memset(s_ofsm_info[gunno].base.transaction_number, 0x00, sizeof(s_ofsm_info[gunno].base.transaction_number));
        memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
        memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
        memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
        memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

        app_nsal_init_charge_data(gunno);

        rt_thread_mdelay(2000);  /* 错峰上报订单 */

        if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code == APP_SYSTEM_STOP_WAY_AUTHEN_FAIL){
            s_ofsm_info[gunno].base.current_elect = s_ofsm_info[gunno].base.start_elect;
            s_ofsm_info[gunno].base.elect_a = 0x00;
            s_ofsm_info[gunno].base.fees_total = 0x00;
            s_ofsm_info[gunno].base.service_fees_total = 0x00;
            s_ofsm_info[gunno].base.elect_fees_total = 0x00;

            s_thaisen_transaction[gunno].ammeter_stop = s_ofsm_info[gunno].base.current_elect;
            s_thaisen_transaction[gunno].total_elect = s_ofsm_info[gunno].base.elect_a;
            s_thaisen_transaction[gunno].total_loss_elect = 0x00;;
            s_thaisen_transaction[gunno].charge_fee = s_ofsm_info[gunno].base.elect_fees_total;
            s_thaisen_transaction[gunno].service_fee = s_ofsm_info[gunno].base.service_fees_total;
            s_thaisen_transaction[gunno].total_fee = s_ofsm_info[gunno].base.fees_total;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
        defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_elect[type] = 0x00;
                s_thaisen_transaction[gunno].rate_type_amount[type] = 0x00;
            }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
        defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t period = s_ofsm_info[gunno].base.start_period; period <= s_ofsm_info[gunno].base.current_period; period++){
                s_thaisen_transaction[gunno].period_elect[period] = 0x00;
                s_thaisen_transaction[gunno].period_elect_fees[period] = 0x00;
                s_thaisen_transaction[gunno].period_service_fees[period] = 0x00;
            }
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_elect_amount[type] = 0x00;
                s_thaisen_transaction[gunno].rate_type_service_amount[type] = 0x00;
            }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */
        }else{
            if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                }
                app_billing_info_calculate(s_ofsm_info[gunno].base.stop_time, (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno)), gunno);
            }else{
                app_billing_info_calculate(s_ofsm_info[gunno].base.stop_time, mw_get_meter_total_wh(gunno), gunno);
            }

            s_ofsm_info[gunno].base.current_elect = app_billingrule_get_stop_elcet(gunno);
            s_ofsm_info[gunno].base.elect_a = app_billingrule_get_elcet_total(gunno);
            s_ofsm_info[gunno].base.fees_total = app_billingrule_get_fees_total(gunno);
            s_ofsm_info[gunno].base.service_fees_total = app_billingrule_get_service_fees_total(gunno);
            if(s_ofsm_info[gunno].base.fees_total > s_ofsm_info[gunno].base.service_fees_total){
                s_ofsm_info[gunno].base.elect_fees_total = (s_ofsm_info[gunno].base.fees_total - s_ofsm_info[gunno].base.service_fees_total);
            }

            s_thaisen_transaction[gunno].ammeter_stop = s_ofsm_info[gunno].base.current_elect;
            s_thaisen_transaction[gunno].total_elect = s_ofsm_info[gunno].base.elect_a;
            s_thaisen_transaction[gunno].total_loss_elect = 0x00;;
            s_thaisen_transaction[gunno].charge_fee = s_ofsm_info[gunno].base.elect_fees_total;
            s_thaisen_transaction[gunno].service_fee = s_ofsm_info[gunno].base.service_fees_total;
            s_thaisen_transaction[gunno].total_fee = s_ofsm_info[gunno].base.fees_total;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
        defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_elect[type] = app_billingrule_get_rate_type_elect(gunno, type);
                s_thaisen_transaction[gunno].rate_type_amount[type] = app_billingrule_get_rate_type_fess(gunno, type);
            }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
        defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t period = s_ofsm_info[gunno].base.start_period; period <= s_ofsm_info[gunno].base.current_period; period++){
                s_thaisen_transaction[gunno].period_elect[period] = app_billingrule_get_period_elect(gunno, period);
                s_thaisen_transaction[gunno].period_elect_fees[period] = app_billingrule_get_period_elect_fees(gunno, period);
                s_thaisen_transaction[gunno].period_service_fees[period] = app_billingrule_get_period_service_fees(gunno, period);
            }
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_elect_amount[type] = app_billingrule_get_rate_type_elect_fess(gunno, type);
                s_thaisen_transaction[gunno].rate_type_service_amount[type] = app_billingrule_get_rate_type_service_fess(gunno, type);
            }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */
        }

        mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_VERIFIED,  \
                0x00, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
        memcpy(&(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), &(s_thaisen_transaction[gunno]), sizeof(thaisen_transaction_t));
        app_nsal_transaction_record_report_target(gunno, &(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), 0x00);
        s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] = APP_THA_ENUM_TRUE;

        memcpy(&(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), &(s_thaisen_transaction[gunno]), sizeof(thaisen_transaction_t));
        app_nsal_transaction_record_report_monitor(gunno, &(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), 0x00);
        s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] = APP_THA_ENUM_TRUE;

        rt_thread_mdelay(4000);  /* 等待电子锁解锁 */

        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FINISHING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FINISHING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_NONE;
        app_nsal_clear_offlinecharge_limit(gunno);
    }

    mw_clear_time_sync_flag(gunno);
    clear_swipe_card_state(gunno);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

    app_nsal_clear_remote_start(gunno);
    app_nsal_clear_remote_stop(gunno);
    app_nsal_clear_remote_card_authorize(gunno);
    app_nsal_clear_remote_vin_authorize(gunno);
}

static void ofsm_finishing_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    bool is_charging_authorization = APP_THA_ENUM_FALSE;  /* 充电已授权 */
    enum charge_state_t charge_state = mw_get_charge_state(gunno);
    enum system_fault_t fault = app_get_highest_priority_system_fault(gunno);

    if (++s_debug_count[gunno] > (3000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) finish state (%dV, S%d)...", gunno, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state);
    }

    s_ofsm_info[gunno].base.flag.is_charge_complete = APP_THA_ENUM_TRUE;

    if((s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_OVERHAUL) ||
            (s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_FREEZE)){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }

    if(fault != APP_SYS_FAULT_NO_ERROR){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FAULTING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        return;
    }
#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
    s_ofsm_info[gunno].base.card_ballance_before = 0x00;
    s_ofsm_info[gunno].base.card_ballance_after = 0x00;
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

                clear_swipe_card_state(gunno);
                app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
                app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
                app_nsal_clear_remote_start(gunno);
                app_nsal_clear_remote_card_authorize(gunno);
                mw_clear_time_sync_flag(gunno);
                app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
                app_nsal_clear_remote_stop(gunno);

                return;
            }
        }
    }

    switch (s_ofsm_info[gunno].base.cc1_state) {
    case CC1_12V:
    case CC1_6V:
    {
        if(charge_state != APP_CHARGE_STATE_IDLE){
            break;                               /* 充电结束必须等待充电状态为空闲时才可响应拔枪动作，已与充电控制同步 */
        }
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_IDLEING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_IDLEING;

        s_ofsm_info[gunno].base.flag.is_charge_complete = APP_THA_ENUM_FALSE;
        s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

        if(s_ofsm_info[gunno].base.cc1_state == CC1_12V){
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
        }else{
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
        }
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
    }
    case CC1_4V:
        if(s_ofsm_info[gunno].base.flag.is_fault_stop == APP_THA_ENUM_TRUE){
            break;          /* 故障停必须要拔枪 */
        }
        if(charge_state != APP_CHARGE_STATE_IDLE){
            break;                               /* 充电结束必须等待充电状态为空闲时才可响应拔枪动作，已与充电控制同步 */
        }
        if(app_nsal_is_remote_start(gunno)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by APP", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_APP;

            /* 经过平台启动的， 已填充以下字段
             * base->user_number
             * base->transaction_number
             * base->card_number
             * base->card_uid
             * base->account_balance
             * base->charge_strategy
             * base->charge_strategy_para
             * base->card_ballance_before
             * base->card_ballance_after
             * base->device_transaction_number
             * base->offline_chargetime */

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.user_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : valid_len;
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
            memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_APP;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

            is_charging_authorization = true;

        }else if(get_swipe_card_state(gunno)){
            uint8_t need_authorize_online = 0x00;
            clear_swipe_card_state(gunno);
            if(get_card_info_type() == CARD_INFO_TYPE_CARD_NUMBER){
                uint8_t pile_number_len = APP_CARD_NUMBER_COMPARE_LEN, card_number_len = get_card_number_len(),
                        compare_len = 0, count = 0;
                uint8_t *pile_number, *card_number;
                compare_len = pile_number_len > card_number_len ? card_number_len : pile_number_len;
                pile_number = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0);
                card_number = get_card_number();

                if(compare_len >= CARD_NUMBER_COMPARE_LEN_MIN){
                    for(; count < CARD_NUMBER_COMPARE_LEN_MIN; count++){
                        if(pile_number[count] != card_number[count]){
                            break;
                        }
                    }
                    if(count == CARD_NUMBER_COMPARE_LEN_MIN){
                        s_buzzon_state = BUZZON_STATE_OK;
                        rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                        LOG_D("gunno(%d) start charge by local pile number card", gunno);
                    }else{
                        if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                            s_buzzon_state = BUZZON_STATE_OK;
                            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                            LOG_D("gunno(%d) start charge by local whitelist card", gunno);
                        }else{
                            need_authorize_online = 0x01;
                        }
                    }
                }else{
                    if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                        s_buzzon_state = BUZZON_STATE_OK;
                        rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                        LOG_D("gunno(%d) start charge by local whitelist card", gunno);
                    }else{
                        need_authorize_online = 0x01;
                    }
                }

                compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                compare_len = compare_len > card_number_len ? card_number_len : compare_len;
                memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
                memcpy(s_ofsm_info[gunno].base.card_number, card_number, compare_len);

                s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_FALSE;

                if(need_authorize_online == 0x00){
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_ofsm_info[gunno].base.account_balance = 0x00;
                    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                    memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                    memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

                    compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

                    compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                    is_charging_authorization = true;
                }
            }else if(get_card_info_type() == CARD_INFO_TYPE_CARD_UID){
                uint8_t uid_len = get_card_uid_len(), compare_len = 0x00;

                if(sys_card_uid_whitelists_query(get_card_uid(), uid_len) >= 0x00){
                    s_buzzon_state = BUZZON_STATE_OK;
                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                    LOG_D("gunno(%d) start charge by local whitelist card uid", gunno);
                }else{
                    need_authorize_online = 0x01;
                }

                uid_len = uid_len > sizeof(s_ofsm_info[gunno].base.card_uid) ? sizeof(s_ofsm_info[gunno].base.card_uid) : uid_len;
                memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                memcpy(s_ofsm_info[gunno].base.card_uid, get_card_uid(), uid_len);
                s_ofsm_info[gunno].base.card_uid_len = uid_len;
                s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_TRUE;

                if(need_authorize_online == 0x00){
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_ofsm_info[gunno].base.account_balance = 0x00;
                    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                    compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

                    compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                    is_charging_authorization = true;
                }
            }

            if(need_authorize_online){
                if(app_nsal_card_authorize(gunno) < 0x00){
                    s_buzzon_state = BUZZON_STATE_FAILED;
                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                    LOG_W("gunno(%d) swip card authorize fail", gunno);
                    return;
                }
                s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_TRUE;
            }
        }else if(app_nsal_is_card_authorize_success(gunno)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by online card", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_ONLINE_CARD;

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.card_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : valid_len;
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, valid_len);

            valid_len = sizeof(s_ofsm_info[gunno].base.user_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : valid_len;
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
            memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_ONLINE_CARD;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

            is_charging_authorization = true;

        }else if(app_nsal_is_card_authorize_fail(gunno)){
            LOG_W("gunno(%d) swip card authorize response fail", gunno);
            s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;
            app_nsal_clear_remote_card_authorize(gunno);

            s_buzzon_state = BUZZON_STATE_FAILED;
            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
        }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by screen", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_SCREEN;
            s_ofsm_info[gunno].base.account_balance = 0x00;
            s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
            s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

            app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

            memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
            memset(&(s_ofsm_info[gunno].base.card_uid), 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
            memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_SCREEN;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

            is_charging_authorization = true;

        }else if(app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1)){
            uint8_t valid_len = 0x00;
            LOG_D("gunno(%d) start charge by VIN", gunno);
            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
            s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.vin_is_authorized = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_VIN;
            s_ofsm_info[gunno].base.account_balance = 0x00;
            s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
            s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

            app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

            memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
            memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
            memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
            memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

            valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
            valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_VIN;
            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

            is_charging_authorization = true;
        }

        /************** 【充电桩已授权】 *************/
        if(is_charging_authorization == true){
            uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;

            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STARTING;

            /* 启动前向屏幕对时 */
            thaisen_request_screen_time();

            s_ofsm_info[gunno].base.main_gunno = gunno;
            s_ofsm_info[gunno].base.charge_way = thaisen_get_charge_way();
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
                if(APP_SYSTEM_GUNNO_SIZE < 0x02){
                    s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 单枪桩不支持并充 */
                }else{
                    if(gunno == APP_SYSTEM_GUNNOA){
                        deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                    }
                    if((s_ofsm_info[deputy_gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT)){
                        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 并充时两把枪都要插上 */
                    }
                    if((s_ofsm_info[deputy_gunno].state != APP_OFSM_STATE_READYING)){
                        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 并充时两把枪都要插上 */
                    }
                }
            }
            s_ofsm_info[deputy_gunno].base.charge_way = s_ofsm_info[gunno].base.charge_way;
            s_ofsm_info[deputy_gunno].base.main_gunno = s_ofsm_info[gunno].base.main_gunno;
            thaisen_set_charge_way(s_ofsm_info[gunno].base.charge_way);

            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
                if(gunno == s_ofsm_info[gunno].base.main_gunno){
                    s_ofsm_info[gunno].base.start_elect = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                }
                s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_PARACHARGE;
            }else{
                s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_SINGLEGUN;
                s_ofsm_info[gunno].base.start_elect = mw_get_meter_total_wh(gunno);
            }

            s_ofsm_info[gunno].base.flag.is_starting = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.voltage_a = 0x00;
            s_ofsm_info[gunno].base.current_a = 0x00;
            s_ofsm_info[gunno].base.power_a = 0x00;
            s_ofsm_info[gunno].base.elect_a = 0x00;
            s_ofsm_info[gunno].base.fees_total = 0x00;
            s_ofsm_info[gunno].base.elect_fees_total = 0x00;
            s_ofsm_info[gunno].base.service_fees_total = 0x00;
            s_ofsm_info[gunno].base.current_elect = mw_get_meter_total_wh(gunno);
            s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
            s_ofsm_info[gunno].base.charge_time = 0x00;
            s_ofsm_info[gunno].base.reason_code = APP_SYSTEM_STOP_WAY_SIZE;
            memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
            s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
            s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;
            s_ofsm_info[gunno].base.period_num = 0x01;
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
            s_ofsm_info[gunno].base.charctrl_state.last = APP_CHARGE_CTRL_STATE_IDLE;
            s_ofsm_info[gunno].base.charctrl_state.current = APP_CHARGE_CTRL_STATE_IDLE;
            s_ofsm_info[gunno].base.start_charge_tick = rt_tick_get();

            s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
            s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
            s_thaisen_transaction[gunno].charge_time = 0x00;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            memset(s_thaisen_transaction[gunno].car_vin, 0x00, sizeof(s_thaisen_transaction[gunno].car_vin));
            s_thaisen_transaction[gunno].start_soc = 0x00;
            s_thaisen_transaction[gunno].stop_soc = 0x00;
            s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
            s_thaisen_transaction[gunno].ammeter_stop = s_thaisen_transaction[gunno].ammeter_start;
            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_POWER_OFF;
            s_thaisen_transaction[gunno].total_elect = 0x00;
            s_thaisen_transaction[gunno].total_loss_elect = 0x00;
            s_thaisen_transaction[gunno].charge_fee = 0x00;
            s_thaisen_transaction[gunno].service_fee = 0x00;
            s_thaisen_transaction[gunno].total_fee = 0x00;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
        defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_unit[type] = app_billingrule_get_rate_type_price(gunno, type);
            }
            memset(s_thaisen_transaction[gunno].rate_type_elect, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_elect));
            memset(s_thaisen_transaction[gunno].rate_type_amount, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_amount));
            memset(s_thaisen_transaction[gunno].rate_type_loss_elect, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_loss_elect));
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
        defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
            memset(s_thaisen_transaction[gunno].period_elect, 0x00, sizeof(s_thaisen_transaction[gunno].period_elect));
            memset(s_thaisen_transaction[gunno].period_elect_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_elect_fees));
            memset(s_thaisen_transaction[gunno].period_service_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_service_fees));
            memset(s_thaisen_transaction[gunno].period_occupy_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_occupy_fees));
            s_thaisen_transaction[gunno].rule = app_billingrule_get_rule(gunno);
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#ifdef APP_INCLUDE_XJ_PROTOCOL
            s_thaisen_transaction[gunno].delay_fee = 0x00;
#endif /* APP_INCLUDE_XJ_PROTOCOL */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
            s_thaisen_transaction[gunno].card_ballance_before = 0x00;
            s_thaisen_transaction[gunno].card_ballance_after = 0x00;
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
            for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                s_thaisen_transaction[gunno].rate_type_elect_amount[type] = 0x00;
                s_thaisen_transaction[gunno].rate_type_service_amount[type] = 0x00;
            }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */

            s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;
            s_thaisen_transaction[gunno].period_count = s_ofsm_info[gunno].base.period_num;

            s_thaisen_transaction[gunno].order_state.verify_fail = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].order_state.is_start_fail = APP_THA_ENUM_TRUE;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_TRUE;

            app_nsal_init_charge_data(gunno);

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);

            mw_storage_record_create(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                    0x00, gunno);
            s_current_order_index[MONITOR_PLATFORM_INDEX][gunno] = mw_storage_record_get_current_index(gunno);
            s_current_order_index[TARGET_PLATFORM_INDEX][gunno] = s_current_order_index[MONITOR_PLATFORM_INDEX][gunno];
            memset(&(s_thaisen_transaction[gunno].bms_stop_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_stop_reason));
            memset(&(s_thaisen_transaction[gunno].bms_fault_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_fault_reason));
            s_booting_step[gunno] = APP_BOOTING_STEP_IDLE;                 /* 初始化充电步骤 */
        }
        break;
    default:
        break;
    }

    switch(s_ofsm_info[gunno].base.cc1_state){
    case CC1_12V:
        if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_HALFWAY){
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
            app_nsal_event_occurded(gunno);
        }
        break;
    case CC1_6V:
        if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_DISCONNECT){
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
            app_nsal_event_occurded(gunno);
        }
        break;
    case CC1_4V:
        if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT){
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
            app_nsal_event_occurded(gunno);
        }
        break;
    default:
        break;
    }

    if((s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_FINISHING) && (is_charging_authorization == false)){
        s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_FINISHING;
        app_nsal_state_charged(gunno);
    }

    if(is_charging_authorization == true){
        clear_swipe_card_state(gunno);
        app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
        app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
        app_nsal_clear_remote_start(gunno);
        app_nsal_clear_remote_card_authorize(gunno);
    }else{
        mw_clear_time_sync_flag(gunno);
    }
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_nsal_clear_remote_stop(gunno);
}

static void ofsm_faulting_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    bool is_charging_authorization = false;
    enum charge_state_t charge_state = mw_get_charge_state(gunno);
    enum charge_fault_t charge_fault = app_get_highest_priority_charge_fault(gunno);
    enum system_fault_t system_fault = app_get_highest_priority_system_fault(gunno);


    if(++s_debug_count[gunno] > (3000 + 500 *gunno) / 100){
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) faulting state (32L: 0x%X, CC1: %d, S%d)...", gunno, (uint32_t)system_fault, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state);
    }

    if(system_fault == APP_SYS_FAULT_NO_ERROR){
        if((s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_OVERHAUL) ||
                (s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_FREEZE)){
            return;
        }
        if(s_ofsm_info[gunno].base.flag.is_charge_complete == APP_THA_ENUM_TRUE){
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FINISHING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_FINISHING;

            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        }else{
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_IDLEING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_IDLEING;

            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
        }

        switch(s_ofsm_info[gunno].base.cc1_state){
        case CC1_12V:
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
            break;
        case CC1_6V:
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
            break;
        case CC1_4V:
            s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
            break;
        default:
            break;
        }
        memset(s_ofsm_info[gunno].base.transaction_number, 0x00, sizeof(s_ofsm_info[gunno].base.transaction_number));
        memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
        memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
        memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
        memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

        app_nsal_init_charge_data(gunno);
        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
    }else{
        s_ofsm_info[gunno].base.system_fault = system_fault;
        s_ofsm_info[gunno].base.charge_fault = charge_fault;

        /** 以下是可充电故障 **/
        if((app_exist_forbid_charge_fault(gunno) == 0x00) && s_ofsm_info[gunno].base.flag.is_charge_complete == APP_THA_ENUM_FALSE){
            s_ofsm_info[gunno].base.flag.is_charge_complete = APP_THA_ENUM_FALSE;
            switch (s_ofsm_info[gunno].base.cc1_state){
            case CC1_4V:
#if 0
                if(s_ofsm_info[gunno].charge_way == APP_CHARGE_WAY_PARACHARGE){
                    if((gunno != s_ofsm_info[gunno].main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].main_gunno].state)){
                        if(s_ofsm_info[s_ofsm_info[gunno].main_gunno].state < APP_OFSM_STATE_SIZE){
                            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].main_gunno].state];
                            s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].main_gunno].state;

                            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

                            clear_swipe_card_state(gunno);
                            app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
                            app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
                            app_nsal_clear_remote_start(gunno);
                            app_nsal_clear_remote_card_authorize(gunno);
                            mw_clear_time_sync_flag(gunno);
                            app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
                            app_nsal_clear_remote_stop(gunno);

                            return;
                        }
                    }
                }
#endif /* 0 */
                if(app_nsal_is_remote_start(gunno)){
                    uint8_t valid_len = 0x00;
                    LOG_D("gunno(%d) start charge by APP", gunno);
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_APP;

                    /* 经过平台启动的， 已填充以下字段
                     * base->user_number
                     * base->transaction_number
                     * base->card_number
                     * base->card_uid
                     * base->account_balance
                     * base->charge_strategy
                     * base->charge_strategy_para
                     * base->card_ballance_before
                     * base->card_ballance_after
                     * base->device_transaction_number
                     * base->offline_chargetime */

                    valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);

                    valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, valid_len);

                    valid_len = sizeof(s_ofsm_info[gunno].base.card_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, valid_len);

                    valid_len = sizeof(s_ofsm_info[gunno].base.user_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
                    memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_APP;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

                    is_charging_authorization = true;

                }else if(get_swipe_card_state(gunno)){
                    uint8_t need_authorize_online = 0x00;
                    clear_swipe_card_state(gunno);
                    if(get_card_info_type() == CARD_INFO_TYPE_CARD_NUMBER){
                        uint8_t pile_number_len = APP_CARD_NUMBER_COMPARE_LEN, card_number_len = get_card_number_len(),
                                compare_len = 0, count = 0;
                        uint8_t *pile_number, *card_number;
                        compare_len = pile_number_len > card_number_len ? card_number_len : pile_number_len;
                        pile_number = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0);
                        card_number = get_card_number();

                        if(compare_len >= CARD_NUMBER_COMPARE_LEN_MIN){
                            for(; count < CARD_NUMBER_COMPARE_LEN_MIN; count++){
                                if(pile_number[count] != card_number[count]){
                                    break;
                                }
                            }
                            if(count == CARD_NUMBER_COMPARE_LEN_MIN){
                                s_buzzon_state = BUZZON_STATE_OK;
                                rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                                LOG_D("gunno(%d) start charge by local pile number card", gunno);
                            }else{
                                if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                                    s_buzzon_state = BUZZON_STATE_OK;
                                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                                    LOG_D("gunno(%d) start charge by local whitelist card", gunno);
                                }else{
                                    need_authorize_online = 0x01;
                                }
                            }
                        }else{
                            if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                                s_buzzon_state = BUZZON_STATE_OK;
                                rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                                LOG_D("gunno(%d) start charge by local whitelist card", gunno);
                            }else{
                                need_authorize_online = 0x01;
                            }
                        }

                        compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                        compare_len = compare_len > card_number_len ? card_number_len : compare_len;
                        memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
                        memcpy(s_ofsm_info[gunno].base.card_number, card_number, compare_len);

                        s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_FALSE;

                        if(need_authorize_online == 0x00){
                            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                            s_ofsm_info[gunno].base.account_balance = 0x00;
                            s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                            s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                            app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                            memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                            memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                            memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

                            compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                            compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
                            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

                            compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                            compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
                            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                            memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

                            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                            is_charging_authorization = true;
                        }
                    }else if(get_card_info_type() == CARD_INFO_TYPE_CARD_UID){
                        uint8_t uid_len = get_card_uid_len(), compare_len = 0x00;

                        if(sys_card_uid_whitelists_query(get_card_uid(), uid_len) >= 0x00){
                            s_buzzon_state = BUZZON_STATE_OK;
                            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                            LOG_D("gunno(%d) start charge by local whitelist card uid", gunno);
                        }else{
                            need_authorize_online = 0x01;
                        }

                        uid_len = uid_len > sizeof(s_ofsm_info[gunno].base.card_uid) ? sizeof(s_ofsm_info[gunno].base.card_uid) : uid_len;
                        memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                        memcpy(s_ofsm_info[gunno].base.card_uid, get_card_uid(), uid_len);
                        s_ofsm_info[gunno].base.card_uid_len = uid_len;
                        s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_TRUE;

                        if(need_authorize_online == 0x00){
                            s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                            s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                            s_ofsm_info[gunno].base.account_balance = 0x00;
                            s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                            s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                            app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                            memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                                    sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                            compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                            compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
                            memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                            memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

                            compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
                            compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
                            memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                            memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

                            memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                            memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                            memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                                    sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                            s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
                            s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                            is_charging_authorization = true;
                        }
                    }

                    if(need_authorize_online){
                        if(app_nsal_card_authorize(gunno) < 0x00){
                            s_buzzon_state = BUZZON_STATE_FAILED;
                            rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                            LOG_W("gunno(%d) swip card authorize fail", gunno);
                            return;
                        }
                        s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_TRUE;
                    }
                }else if(app_nsal_is_card_authorize_success(gunno)){
                    uint8_t valid_len = 0x00;
                    LOG_D("gunno(%d) start charge by online card", gunno);
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_ONLINE_CARD;

                    valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);

                    valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, valid_len);

                    valid_len = sizeof(s_ofsm_info[gunno].base.card_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, valid_len);

                    valid_len = sizeof(s_ofsm_info[gunno].base.user_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
                    memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, valid_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_ONLINE_CARD;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

                    is_charging_authorization = true;

                }else if(app_nsal_is_card_authorize_fail(gunno)){
                    LOG_W("gunno(%d) swip card authorize response fail", gunno);
                    s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;
                    app_nsal_clear_remote_card_authorize(gunno);

                    s_buzzon_state = BUZZON_STATE_FAILED;
                    rt_mq_send(&g_buzzon_mq, &s_buzzon_state, sizeof(s_buzzon_state));
                }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1)){
                    uint8_t valid_len = 0x00;
                    LOG_D("gunno(%d) start charge by screen", gunno);
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_SCREEN;
                    s_ofsm_info[gunno].base.account_balance = 0x00;
                    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                    memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
                    memset(&(s_ofsm_info[gunno].base.card_uid), 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                    memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

                    valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_SCREEN;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                    is_charging_authorization = true;

                }else if(app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1)){
                    uint8_t valid_len = 0x00;
                    LOG_D("gunno(%d) start charge by VIN", gunno);
                    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
                    s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.flag.vin_is_authorized = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_VIN;
                    s_ofsm_info[gunno].base.account_balance = 0x00;
                    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
                    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

                    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

                    memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
                    memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
                    memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
                    memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

                    valid_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
                    valid_len = valid_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : valid_len;
                    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
                    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, valid_len);
                    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
                    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
                    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
                    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
                            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
                    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_VIN;
                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;

                    is_charging_authorization = true;
                }

                /************** 【充电桩已授权】 *************/
                if(is_charging_authorization == true){
#if 0
                    uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
#endif /* 0 */
                    s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING];
                    s_ofsm_info[gunno].state = APP_OFSM_STATE_STARTING;

                    /* 启动前向屏幕对时 */
                    thaisen_request_screen_time();
#if 0
                    s_ofsm_info[gunno].main_gunno = gunno;
                    s_ofsm_info[gunno].charge_way = thaisen_get_charge_way();
                    if(s_ofsm_info[gunno].charge_way == APP_CHARGE_WAY_PARACHARGE){
                        if(APP_SYSTEM_GUNNO_SIZE < 0x02){
                            s_ofsm_info[gunno].charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 单枪桩不支持并充 */
                        }else{
                            if(gunno == APP_SYSTEM_GUNNOA){
                                deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                            }
                            if((s_ofsm_info[deputy_gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT)){
                                s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 并充时两把枪都要插上 */
                            }
                            if((s_ofsm_info[deputy_gunno].state != APP_OFSM_STATE_READYING)){
                                s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_SINGLEGUN;  /** 并充时两把枪都要插上 */
                            }
                        }
                    }
                    s_ofsm_info[deputy_gunno].charge_way = s_ofsm_info[gunno].charge_way;
                    s_ofsm_info[deputy_gunno].main_gunno = s_ofsm_info[gunno].main_gunno;
                    thaisen_set_charge_way(s_ofsm_info[gunno].charge_way);
#endif /* 0 */

#if 0
                    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE){
                        if(gunno == s_ofsm_info[gunno].base.main_gunno){
                            s_ofsm_info[gunno].base.start_elect = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                        }
                        s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_PARACHARGE;
                    }else{
                        s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_SINGLEGUN;
                        s_ofsm_info[gunno].base.start_elect = mw_get_meter_total_wh(gunno);
                    }
#else
                    s_ofsm_info[gunno].base.start_elect = mw_get_meter_total_wh(gunno);
#endif /* 0 */
                    s_ofsm_info[gunno].base.flag.is_starting = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
                    s_ofsm_info[gunno].base.voltage_a = 0x00;
                    s_ofsm_info[gunno].base.current_a = 0x00;
                    s_ofsm_info[gunno].base.power_a = 0x00;
                    s_ofsm_info[gunno].base.elect_a = 0x00;
                    s_ofsm_info[gunno].base.fees_total = 0x00;
                    s_ofsm_info[gunno].base.elect_fees_total = 0x00;
                    s_ofsm_info[gunno].base.service_fees_total = 0x00;
                    s_ofsm_info[gunno].base.current_elect = mw_get_meter_total_wh(gunno);
                    s_ofsm_info[gunno].base.start_time = s_ofsm_info[gunno].base.current_time;
                    s_ofsm_info[gunno].base.stop_time = s_ofsm_info[gunno].base.current_time;
                    s_ofsm_info[gunno].base.charge_time = 0x00;
                    s_ofsm_info[gunno].base.reason_code = APP_SYSTEM_STOP_WAY_SIZE;
                    memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
                    s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                    s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;
                    s_ofsm_info[gunno].base.period_num = 0x01;
                    s_ofsm_info[gunno].charge_timeout = rt_tick_get();
                    s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;
                    s_ofsm_info[gunno].base.charctrl_state.last = APP_CHARGE_CTRL_STATE_IDLE;
                    s_ofsm_info[gunno].base.charctrl_state.current = APP_CHARGE_CTRL_STATE_IDLE;
                    s_ofsm_info[gunno].base.start_charge_tick = rt_tick_get();

                    s_thaisen_transaction[gunno].start_time = s_ofsm_info[gunno].base.start_time;
                    s_thaisen_transaction[gunno].end_time = s_thaisen_transaction[gunno].start_time;
                    s_thaisen_transaction[gunno].charge_time = 0x00;
                    s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
                    memset(s_thaisen_transaction[gunno].car_vin, 0x00, sizeof(s_thaisen_transaction[gunno].car_vin));
                    s_thaisen_transaction[gunno].start_soc = 0x00;
                    s_thaisen_transaction[gunno].stop_soc = 0x00;
                    s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
                    s_thaisen_transaction[gunno].ammeter_stop = s_thaisen_transaction[gunno].ammeter_start;
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_POWER_OFF;
                    s_thaisen_transaction[gunno].total_elect = 0x00;
                    s_thaisen_transaction[gunno].total_loss_elect = 0x00;
                    s_thaisen_transaction[gunno].charge_fee = 0x00;
                    s_thaisen_transaction[gunno].service_fee = 0x00;
                    s_thaisen_transaction[gunno].total_fee = 0x00;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
        defined (APP_INCLUDE_SGCC_PROTOCOL))
                    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                        s_thaisen_transaction[gunno].rate_type_unit[type] = app_billingrule_get_rate_type_price(gunno, type);
                    }
                    memset(s_thaisen_transaction[gunno].rate_type_elect, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_elect));
                    memset(s_thaisen_transaction[gunno].rate_type_amount, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_amount));
                    memset(s_thaisen_transaction[gunno].rate_type_loss_elect, 0x00, sizeof(s_thaisen_transaction[gunno].rate_type_loss_elect));
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
        defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
                    memset(s_thaisen_transaction[gunno].period_elect, 0x00, sizeof(s_thaisen_transaction[gunno].period_elect));
                    memset(s_thaisen_transaction[gunno].period_elect_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_elect_fees));
                    memset(s_thaisen_transaction[gunno].period_service_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_service_fees));
                    memset(s_thaisen_transaction[gunno].period_occupy_fees, 0x00, sizeof(s_thaisen_transaction[gunno].period_occupy_fees));
                    s_thaisen_transaction[gunno].rule = app_billingrule_get_rule(gunno);
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#ifdef APP_INCLUDE_XJ_PROTOCOL
                    s_thaisen_transaction[gunno].delay_fee = 0x00;
#endif /* APP_INCLUDE_XJ_PROTOCOL */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
                    s_thaisen_transaction[gunno].card_ballance_before = 0x00;
                    s_thaisen_transaction[gunno].card_ballance_after = 0x00;
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
                    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                        s_thaisen_transaction[gunno].rate_type_elect_amount[type] = 0x00;
                        s_thaisen_transaction[gunno].rate_type_service_amount[type] = 0x00;
                    }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */

                    s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;
                    s_thaisen_transaction[gunno].period_count = s_ofsm_info[gunno].base.period_num;

                    s_thaisen_transaction[gunno].order_state.verify_fail = APP_THA_ENUM_FALSE;
                    s_thaisen_transaction[gunno].order_state.is_start_fail = APP_THA_ENUM_TRUE;
                    s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_TRUE;

                    app_nsal_init_charge_data(gunno);

                    app_nsal_state_charged(gunno);
                    app_nsal_event_occurded(gunno);

                    mw_storage_record_create(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                            0x00, gunno);
                    s_current_order_index[MONITOR_PLATFORM_INDEX][gunno] = mw_storage_record_get_current_index(gunno);
                    s_current_order_index[TARGET_PLATFORM_INDEX][gunno] = s_current_order_index[MONITOR_PLATFORM_INDEX][gunno];
                    memset(&(s_thaisen_transaction[gunno].bms_stop_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_stop_reason));
                    memset(&(s_thaisen_transaction[gunno].bms_fault_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_fault_reason));
                    s_booting_step[gunno] = APP_BOOTING_STEP_IDLE;                 /* 初始化充电步骤 */
                }
                break;
            default:
                break;
            }
            if(is_charging_authorization == true){
                clear_swipe_card_state(gunno);
                app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
                app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
                app_nsal_clear_remote_start(gunno);
                app_nsal_clear_remote_card_authorize(gunno);
            }else{
                mw_clear_time_sync_flag(gunno);
            }
            app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
            app_nsal_clear_remote_stop(gunno);
        }else{
            mw_clear_time_sync_flag(gunno);
            clear_swipe_card_state(gunno);
            app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
            app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
            app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

            app_nsal_clear_remote_start(gunno);
            app_nsal_clear_remote_stop(gunno);
            app_nsal_clear_remote_card_authorize(gunno);
            app_nsal_clear_remote_vin_authorize(gunno);
        }

        switch(s_ofsm_info[gunno].base.cc1_state){
        case CC1_12V:
            if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_HALFWAY){
                s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_HALFWAY;
                app_nsal_event_occurded(gunno);
            }
            break;
        case CC1_6V:
            if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_DISCONNECT){
                s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_DISCONNECT;
                app_nsal_event_occurded(gunno);
            }
            break;
        case CC1_4V:
            if(s_ofsm_info[gunno].base.flag.connect_state != APP_CONNECT_STATE_CONNECT){
                s_ofsm_info[gunno].base.flag.connect_state = APP_CONNECT_STATE_CONNECT;
                app_nsal_event_occurded(gunno);
            }
            break;
        default:
            break;
        }

        if((s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_FAULTING) && (is_charging_authorization == false)){
            s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_FAULTING;
            app_nsal_state_charged(gunno);
        }
    }
}

void ofsm_fun_list_init(void)
{
    for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_WAIT_NET] = ofsm_wait_net_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_IDLEING] = ofsm_idleing_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_READYING] = ofsm_readying_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_RESERVATION] = ofsm_reservation_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING] = ofsm_starting_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_CHARGING] = ofsm_charging_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING] = ofsm_stoping_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_FINISHING] = ofsm_finishing_fun;
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_FAULTING] = ofsm_faulting_fun;

        s_ofsm_info[gunno].charge_timeout = rt_tick_get();
        memset(&(s_thaisen_transaction[gunno]), 0x00, sizeof(s_thaisen_transaction[gunno]));
        memset(&(s_ofsm_info[gunno].base), 0x00, sizeof(s_ofsm_info[gunno].base));
    }
}

struct ofsm_info *get_ofsm_info(uint8_t gunno)
{
    if(gunno > APP_SYSTEM_GUNNO_SIZE){
        return &(s_ofsm_info[0x00]);
    }
    return &(s_ofsm_info[gunno]);
}

void ofsm_thread_entry(void *parameter)
{
    int32_t chargepile_gun_temp_max = 0;
    uint32_t record_store = 0;
    uint8_t thread_gunno = *((uint8_t*)parameter);
    enum temp_check result = TCHECK_RESULT_NORMAL;

    if(thread_gunno >= APP_SYSTEM_GUNNO_SIZE){
        thread_gunno = APP_SYSTEM_GUNNO_SIZE;
    }
    s_transaction_sending[TARGET_PLATFORM_INDEX][thread_gunno] = APP_THA_ENUM_FALSE;
    s_transaction_sending[MONITOR_PLATFORM_INDEX][thread_gunno] = APP_THA_ENUM_FALSE;
    s_ofsm_fun[thread_gunno] = s_ofsm_fun_list[thread_gunno][APP_OFSM_STATE_WAIT_NET];
    s_ofsm_info[thread_gunno].state = APP_OFSM_STATE_WAIT_NET;

    rt_thread_mdelay(6000);  /** 等待底层驱动正常(电表要获取到电量) */

    while(1){
        uint16_t singlegun_curr = s_ofsm_info[thread_gunno].base.gun_set_curr;
        s_ofsm_info[thread_gunno].base.ota_state = app_nsal_get_ota_state();

        switch (s_ofsm_info[thread_gunno].base.ota_state) {
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
            rt_thread_mdelay(5000);
            continue;
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        if(thaisen_get_screen_timesync_flag()){
            struct tm t = { 0 };
            uint32_t timestamp;
            uint16_t _time[6];
            uint8_t data_valid = 0x01;
            thaisen_get_screen_timesync_time(_time, sizeof(_time));

            if((_time[0] < 2024) || (_time[0] >= 2038)){
                data_valid = 0;
            }else if((_time[1] > 12) || (_time[1] == 0)){
                data_valid = 0;
            }else if((_time[2] > 31) || (_time[2] == 0)){
                data_valid = 0;
            }else if((_time[3] > 23)){
                data_valid = 0;
            }else if((_time[4] >= 60)){
                data_valid = 0;
            }else if((_time[5] >= 60)){
                data_valid = 0;
            }

            t.tm_year = _time[0] - 1900;
            t.tm_mon  = _time[1] - 1;
            t.tm_mday = _time[2];
            t.tm_hour = _time[3];
            t.tm_min  = _time[4];
            t.tm_sec  = _time[5];

            timestamp = mktime(&t);

            s_request_screen_time_step = 1;
            if(data_valid){
                mw_set_datetime_from_timestamp(timestamp);

                extern void mw_set_time_sync_flag(void);
                mw_set_time_sync_flag();
                s_request_screen_time_step = 2;
                LOG_D("time sync :%d, %d, %d, %d, %d, %d", _time[0], _time[1], _time[2], _time[3], _time[4], _time[5]);
            }
        }

        if(s_request_screen_time_step == 1){
            if((rt_tick_get() - s_request_screen_time_tick) >= 3000){
                thaisen_request_screen_time();
                s_request_screen_time_tick = rt_tick_get();
            }
        }

        if(app_nsal_is_time_sync()){
            app_nsal_clear_time_sync();
            extern void mw_set_time_sync_flag(void);
            mw_set_time_sync_flag();
        }

        if(s_ofsm_info[thread_gunno].state == APP_OFSM_STATE_CHARGING){
            if(record_store > rt_tick_get()){
                record_store = rt_tick_get();
            }
            if((rt_tick_get() - record_store) > APP_STORAGE_TRANSATION_INTERVAL){
                mw_storage_record_designate_index_updated(&s_thaisen_transaction[thread_gunno], sizeof(s_thaisen_transaction[thread_gunno]), USER_DATA_TYPE_STORAGE,  \
                        0x00, thread_gunno, s_current_order_index[TARGET_PLATFORM_INDEX][thread_gunno]);
                record_store = rt_tick_get();
            }
        }else{
            record_store = rt_tick_get();
        }

        if(!((s_ofsm_info[thread_gunno].state == APP_OFSM_STATE_STARTING) ||      \
                (s_ofsm_info[thread_gunno].state == APP_OFSM_STATE_CHARGING) ||   \
                (s_ofsm_info[thread_gunno].state == APP_OFSM_STATE_STOPING))){
            if(app_nsal_need_update_billingrule(thread_gunno)){
                app_nsal_clear_update_billingrule_event(thread_gunno);
                app_billingrule_update_billingrule_info(thread_gunno);
            }
        }

        s_chargegun_idle_count = 0x00;
        for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            if((s_ofsm_info[gunno].state == APP_OFSM_STATE_IDLEING) || ((s_ofsm_info[gunno].state == APP_OFSM_STATE_FAULTING))){
                s_chargegun_idle_count++;
            }
            if(s_chargegun_idle_count == APP_SYSTEM_GUNNO_SIZE){
                app_nsal_is_permit_ota(0x01);
            }else{
                app_nsal_is_permit_ota(0x00);
            }
        }

        chargepile_gun_temp_max = s_ofsm_info[thread_gunno].base.gunline_temperature[0x00];
        chargepile_gun_temp_max = chargepile_gun_temp_max > s_ofsm_info[thread_gunno].base.gunline_temperature[0x01] ?   \
                chargepile_gun_temp_max : s_ofsm_info[thread_gunno].base.gunline_temperature[0x01];

        if((result = mw_temperature_protect_check(thread_gunno, chargepile_gun_temp_max /10)) != TCHECK_RESULT_NORMAL){
            if((*(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPRO, 0))) != 1){
                s_ofsm_info[thread_gunno].base.flag.is_overtemp = 0;
                s_ofsm_info[thread_gunno].base.flag.is_curr_decreased = 0;
                s_gun_charging_curr[thread_gunno] = 0;
                thaisenClearSysFaultLib(thaisenFaultOverTemp, thread_gunno);
            }else{
                switch(result){
                case TCHECK_RESULT_WARNNING:
                    s_ofsm_info[thread_gunno].base.flag.is_overtemp = 0;
                    s_ofsm_info[thread_gunno].base.flag.is_curr_decreased = 0;
                    s_gun_charging_curr[thread_gunno] = 0;
                    thaisenClearSysFaultLib(thaisenFaultOverTemp, thread_gunno);
                    break;
                case TCHECK_RESULT_OVERTEMP_1:
                    s_ofsm_info[thread_gunno].base.flag.is_overtemp = 1;
                    if((s_ofsm_info[thread_gunno].state != APP_OFSM_STATE_CHARGING) || (s_issue_power_adjust == true)){
                        if(!(s_ofsm_info[thread_gunno].base.flag.is_curr_decreased)){
                            s_gun_charging_curr[thread_gunno] = 0;
                            s_issue_power_adjust = false;
                        }
                    }
                    if((s_gun_charging_curr[thread_gunno] == 0) && (s_chargepile_output_steady[thread_gunno] == true)){
                        s_gun_charging_curr[thread_gunno] = mw_get_meter_ia(thread_gunno);
                    }
                    thaisenClearSysFaultLib(thaisenFaultOverTemp, thread_gunno);
                    break;
                case TCHECK_RESULT_OVERTEMP_2:
                    s_ofsm_info[thread_gunno].base.flag.is_overtemp = 1;
                    s_gun_charging_curr[thread_gunno] = 0;
                    thaisenSetSysFaultLib(thaisenFaultOverTemp, thread_gunno);
                    break;
                default:
                    s_gun_charging_curr[thread_gunno] = 0;
                    thaisenClearSysFaultLib(thaisenFaultOverTemp, thread_gunno);
                    break;
                }
            }
        }else{
            s_ofsm_info[thread_gunno].base.flag.is_overtemp = 0;
            s_ofsm_info[thread_gunno].base.flag.is_curr_decreased = 0;
            s_gun_charging_curr[thread_gunno] = 0;
            thaisenClearSysFaultLib(thaisenFaultOverTemp, thread_gunno);
        }

        temperature_protect_limitcurr(thread_gunno, &singlegun_curr);
        extern void thaisenSetModuleMaxChargCurrGroup(uint8_t groupNum, uint16_t curr);
        thaisenSetModuleMaxChargCurrGroup(thread_gunno, singlegun_curr);

        s_ofsm_info[thread_gunno].base.ammeter_elect = mw_get_meter_total_wh(thread_gunno);
        s_ofsm_info[thread_gunno].base.net_state = app_nsal_get_link_state();
        s_ofsm_info[thread_gunno].base.cc1_state = mw_get_cc1(thread_gunno);
        s_ofsm_info[thread_gunno].base.current_time = mw_get_current_timestamp();
        s_ofsm_info[thread_gunno].base.gunline_temperature[0x00] = mw_get_temp_dcp(thread_gunno); /* 正极实时获取温度值 */
        s_ofsm_info[thread_gunno].base.gunline_temperature[0x01] = mw_get_temp_dcn(thread_gunno); /* 负极实时获取温度值 */

        if(s_ofsm_fun != NULL) {
            (*(s_ofsm_fun[thread_gunno]))(thread_gunno);
        }

        transaction_record_query_report(thread_gunno);
        app_nsal_realtime_process(thread_gunno);

        set_current_port(thaisen_get_hci_page_pos());

        rt_thread_mdelay(100);
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
