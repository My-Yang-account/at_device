/**
  ******************************************************************************
  * @file
  * @brief 平台状态机
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/**
  ******************************************************************************
      * 注：并充时停充逻辑：
  *   1.所有的人为停止都按主枪停止(给主枪发停止指令)，副枪根据主枪状态跳转
  *   2.对于由于充电状态变为非充电时的停止场景，副枪根据主枪状态跳转
  *   3.对于由于副枪自身故障而停充的场景，如果这个故障只有副枪有，则置位标志让主枪停止
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
#include "app_rfid_reader.h"
#include "app_terminal.h"

#include "mw_cc1.h"
#include "mw_charge_control.h"
#include "mw_meter.h"
#include "mw_temp.h"
#include "mw_time.h"
#include "mw_module_control.h"
#include "mw_can_control.h"


#define DBG_TAG "app.ofsm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LINK_PLATFORM_MAX                         0x02  /* 连接平台总数 */
#define TARGET_PLATFORM_INDEX                     0x00  /* 目标平台下标 */
#define MONITOR_PLATFORM_INDEX                    0x01  /* 监控平台下标 */

typedef void (* ofsm_fun_p)(uint8_t gunno);

APP_DEF_SRAM1 static ofsm_fun_p s_ofsm_fun_list[APP_SYSTEM_GUNNO_SIZE][APP_OFSM_STATE_SIZE];
APP_DEF_SRAM1 static ofsm_fun_p s_ofsm_fun[APP_SYSTEM_GUNNO_SIZE] = {NULL};
APP_DEF_SRAM1 static uint32_t s_debug_count[APP_SYSTEM_GUNNO_SIZE];

APP_DEF_SRAM1 static uint32_t s_tiny_current_count[APP_SYSTEM_GUNNO_SIZE];

APP_DEF_SRAM1 static struct ofsm_info s_ofsm_info[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static int32_t s_current_order_index[LINK_PLATFORM_MAX][APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static thaisen_transaction_t s_thaisen_transaction[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static thaisen_transaction_t s_thaisen_transaction_report[LINK_PLATFORM_MAX][APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static enum booting_step_t s_booting_step[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static uint32_t s_request_screen_time_tick = 0x00;
APP_DEF_SRAM1 static uint8_t s_request_screen_time_step = 0x01;
APP_DEF_SRAM1 static uint8_t s_transaction_sending[LINK_PLATFORM_MAX][APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static uint8_t s_chargegun_idle_count = 0x00;
APP_DEF_SRAM1 static uint32_t s_timestamp_base = 0x00, s_tick_base = 0x00;

APP_DEF_SRAM1 static uint8_t s_issue_power_adjust = false, s_chargepile_output_steady[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static uint8_t s_charge_steady_delay[APP_SYSTEM_GUNNO_SIZE], s_power_adjust_delay = 0, s_power_on = 0;
APP_DEF_SRAM1 static uint32_t s_system_power_output = 0x00;
APP_DEF_SRAM1 static uint16_t s_gun_charging_curr[APP_SYSTEM_GUNNO_SIZE];

APP_DEF_SRAM1 static uint32_t s_compare_module_bcl_count[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static uint32_t s_compare_ccs_module_count[APP_SYSTEM_GUNNO_SIZE];

APP_DEF_SRAM1 static uint32_t s_bms_require_curr_last[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static uint8_t s_bms_reqcurr_changed_count[APP_SYSTEM_GUNNO_SIZE];

#ifdef APP_DESIGNATE_REGION
/*************************************
 * 函数名       app_ofsm_info_init
 * 功能           业务信息、变量初始化
 * 参数
 * 返回
 ************************************/
void app_ofsm_info_init(void)
{
    for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        memset(s_ofsm_fun_list[gunno], 0x00, sizeof(s_ofsm_fun_list[gunno]));
        s_ofsm_fun[gunno] = 0x00;

        s_debug_count[gunno] = 0x00;
        s_tiny_current_count[gunno] = 0x00;

        for(uint8_t plat = 0x00; plat < LINK_PLATFORM_MAX; plat++){
            s_current_order_index[plat][gunno] = 0x00;
            s_transaction_sending[plat][gunno] = 0x00;
            memset(&(s_thaisen_transaction_report[plat][gunno]), 0x00, sizeof(s_thaisen_transaction_report[plat][gunno]));
        }

        s_booting_step[gunno] = 0x00;

        s_chargepile_output_steady[gunno] = 0x00;
        s_charge_steady_delay[gunno] = 0x00;
        s_gun_charging_curr[gunno] = 0x00;

        s_compare_module_bcl_count[gunno] = 0x00;
        s_compare_ccs_module_count[gunno] = 0x00;
        s_bms_require_curr_last[gunno] = 0x00;
        s_bms_reqcurr_changed_count[gunno] = 0x00;

        memset(&(s_thaisen_transaction[gunno]), 0x00, sizeof(s_thaisen_transaction[gunno]));
        memset(&(s_ofsm_info[gunno]), 0x00, sizeof(s_ofsm_info[gunno]));
    }

    s_request_screen_time_tick = 0x00;
    s_request_screen_time_step = 0x01;

    s_chargegun_idle_count = 0x00;
    s_issue_power_adjust = false;
    s_power_adjust_delay = 0x00;
    s_power_on = 0x00;
    s_system_power_output = 0x00;

    s_timestamp_base = 0x00;
    s_tick_base = 0x00;
}
#endif /* APP_DESIGNATE_REGION */

/*************************************
 * 函数名       ofsm_bms_can_send
 * 功能           发送BMS CAN 数据
 * 参数           gunno   枪号
 *        data    要发送的数据
 * 返回
 ************************************/
static void ofsm_bms_can_send(uint8_t gunno, mw_can_info *data)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    if(data == NULL){
        return;
    }

    data->id = APP_PARACHARGE_IDENTIFY_CAN_ID;
    data->length = 0x08;
    memset(data->data, APP_PARACHARGE_IDENTIFY_CAN_DATA, data->length);
    if(gunno == APP_SYSTEM_GUNNOA){
        mw_bmsa_can_send(data->id, data->data, data->length);
    }
#ifdef APP_USING_DOUBLEGUN
    else if(gunno == APP_SYSTEM_GUNNOB){
        mw_bmsb_can_send(data->id, data->data, data->length);
    }
#endif /* APP_USING_DOUBLEGUN */
}

/*************************************
 * 函数名       ofsm_is_belong_one_car
 * 功能           并充自动识别，判断两把枪是否插在同一辆车上(CNA 线相连)
 * 参数           gunno   枪号
 * 返回           1：是  0：否
 ************************************/
static uint8_t ofsm_is_belong_one_car(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_THA_ENUM_FALSE;
    }
    /** 按两把枪做 */
    if(APP_SYSTEM_GUNNO_SIZE >= 0x02){
        if(gunno == APP_SYSTEM_GUNNOA){
            gunno = (APP_SYSTEM_GUNNOA + 0x01);
        }else{
            gunno = APP_SYSTEM_GUNNOA;
        }
    }else{
        return APP_THA_ENUM_FALSE;
    }

    mw_can_info buf;
    uint8_t compare_data[0x08];
    memset(compare_data, APP_PARACHARGE_IDENTIFY_CAN_DATA, sizeof(compare_data));
    memset(&buf, 0x00, sizeof(mw_can_info));

    if(gunno == APP_SYSTEM_GUNNOA){
        mw_bmsa_can_recv(&buf);
    }
#ifdef APP_USING_DOUBLEGUN
    else if(gunno == APP_SYSTEM_GUNNOB){
        mw_bmsb_can_recv(&buf);
    }
#endif /* APP_USING_DOUBLEGUN */

    if(buf.id != APP_PARACHARGE_IDENTIFY_CAN_ID){
        return APP_THA_ENUM_FALSE;
    }
    if(buf.length > sizeof(compare_data)){
        if(memcmp(compare_data, buf.data, sizeof(compare_data))){
            return APP_THA_ENUM_FALSE;
        }
    }else{
        if(memcmp(compare_data, buf.data, buf.length)){
            return APP_THA_ENUM_FALSE;
        }
    }

    return APP_THA_ENUM_TRUE;
}

/*************************************
 * 函数名       ofsm_get_current_period
 * 功能           获取当前时段号
 * 参数
 * 返回           当前时段号
 ************************************/
uint8_t ofsm_get_current_period(void)
{
    uint8_t period;
    uint32_t current_time = s_timestamp_base, tick = rt_tick_get();
    struct tm *_tm;

    if(s_tick_base > tick){
        current_time += ((tick + 0xFFFFFFFF - s_tick_base) /1000);
    }else{
        current_time += ((tick - s_tick_base) /1000);
    }

    rt_enter_critical();
    _tm = localtime((time_t*)&current_time);
    current_time = _tm->tm_hour *60 *60 + _tm->tm_min *60 + _tm->tm_sec;
    rt_exit_critical();

    period = current_time / (24 * 60 / APP_BILLING_RULE_PERIOD_MAX * 60);

    return (period + 0x01);
}

/**********************************************************************************
 * 函数名       ofsm_get_current_period_time_hm
 * 功能           获取当前时段对应时间
 * 参数           period     时段
 *        buf        缓存
 *        blen       缓存长度
 * 返回           >=0:成功，<0:失败
 *********************************************************************************/
int32_t ofsm_get_current_period_time_hm(uint8_t period, uint8_t *buf, uint8_t blen)
{
    if((period == 0x00) || (period > APP_BILLING_RULE_PERIOD_MAX)){   /** 时段号从1开始 */
        return -0x01;
    }
    if((buf == NULL) || (blen < 0x04)){    /** 时间格式：小时：分钟-小时：分钟，缓存最小长度是4 */
        return -0x01;
    }

    uint8_t hour, min;
    hour = (period - 0x01) /0x04;          /** 每个小时分为4个时段 */
    min = ((period - 0x01) %0x04) *15;     /** 每个时段15分钟 */

    buf[0x00] = (hour %24);                /** 开始时间：小时 */
    buf[0x01] = (min %60);                 /** 开始时间：分钟 */
    if((min + 15) /60){
        buf[0x02] = ((hour + 0x01) %24);   /** 结束时间：小时 */
    }else{
        buf[0x02] = (hour %24);            /** 结束时间：小时 */
    }
    buf[0x03] = (min + 15) %60;            /** 结束时间：分钟 */

    return 0x00;
}

/************************************************************
 * 函数名       ofsm_get_period_price
 * 功能           获取指定枪时段电费单价
 * 参数           gunno      枪号(此时传入的枪号参数不使用，枪号按更新了最新计费规则的来)
 *        period     时段
 * 返回           >=0:成功，<0:失败
 ***********************************************************/
uint32_t ofsm_get_period_price(uint8_t gunno, uint8_t period)
{
    gunno = app_billingrule_query_rule_update_gunno();

    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return 0x00;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){   /* 时段号不对 */
        return 0x00;
    }

    uint32_t price = app_billingrule_get_period_elect_price(gunno, period) + \
            app_billingrule_get_period_service_price(gunno, period) + \
            app_billingrule_get_period_delay_price(gunno, period);

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        uint32_t current_time = s_timestamp_base, tick = rt_tick_get();
        if(s_tick_base > tick){
            current_time += ((tick + 0xFFFFFFFF - s_tick_base) /1000);
        }else{
            current_time += ((tick - s_tick_base) /1000);
        }
        price = sys_get_offbilling_unit_price(current_time);
    }

    return price;
}

/************************************************************
 * 函数名       transaction_record_query_report
 * 功能           查询指定枪号订单并上报
 * 参数           gunno      枪号
 * 返回
 ***********************************************************/
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
        uint8_t save_rentry = 0;
        if((index = mw_storage_record_get_first_index_userdata(USER_DATA_TYPE_STORAGE, gunno)) < 0x00){
            return;
        }

        uint8_t need_check = 0, is_force = APP_THA_ENUM_FALSE;
        uint8_t _period = 0x00, _rated_number = APP_RATE_TYPE_FLAT;
        double _unit_price = 10.5;   /** 断电时电费单价(默认1.05元，精度：0.1) */
        thaisen_transaction_t rtransaction;
        mw_storage_record_get_designate_index_record((uint8_t*)(&rtransaction), sizeof(rtransaction), gunno, index);
        save_rentry = 0;

        if(rtransaction.stop_reason == APP_SYSTEM_STOP_WAY_POWER_OFF){
            uint32_t loss_elect = 0x00, _total_elect = 0x00, _price = 0x00;

            if(rtransaction.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
                for(uint8_t count = 0x00; count < APP_SYSTEM_GUNNO_SIZE; count++){
                    _total_elect += mw_get_meter_total_wh(count);
                }
            }else{
                _total_elect = mw_get_meter_total_wh(gunno);
            }

            if(s_ofsm_info[gunno].base.order_fixes_tick > rt_tick_get()){
                s_ofsm_info[gunno].base.order_fixes_tick = rt_tick_get();
            }
            if((rt_tick_get() - s_ofsm_info[gunno].base.order_fixes_tick) > APP_ORDER_FIXES_WAIT_TIME){
                is_force = APP_THA_ENUM_TRUE;   /** 对于断电订单：如果当前电表电量小于订单结束电表电量持续30s，就强制进行电量修正 */
            }

            if((_total_elect >= rtransaction.ammeter_stop) || (is_force == APP_THA_ENUM_TRUE)){
#ifdef APP_INCLUDE_YKC17_PROTOCOL
                if(s_ofsm_info[gunno].base.meter_step == APP_AMMETER_ENCRY_STEP_NULL){
                    s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_START;
                    s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();

                    LOG_D("meter encrypt:start[%d](poweroff bill)", gunno);
                    thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_START);
                    return;
                }
                /** 等待电表加密开始充电指令响应 */
                else if(s_ofsm_info[gunno].base.meter_step == APP_AMMETER_ENCRY_STEP_START){
                    if((rt_tick_get() - s_ofsm_info[gunno].base.meter_reading_tick) < APP_AMMETER_ENCRY_WAIT_START_REPLY_MS){
                        if(thaisen_ammeter_is_replied(gunno) == APP_THA_ENUM_FALSE){
                            return;
                        }
                    }
                    s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_STOP;
                    s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();

                    LOG_D("meter encrypt:stop[%d, %d](poweroff bill)", gunno, (rt_tick_get() - s_ofsm_info[gunno].base.meter_reading_tick));
                    thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
                    return;
                }
                /** 等待电表加密结束充电指令响应 */
                else if(s_ofsm_info[gunno].base.meter_step == APP_AMMETER_ENCRY_STEP_STOP){
                    if((rt_tick_get() - s_ofsm_info[gunno].base.meter_reading_tick) < APP_AMMETER_ENCRY_WAIT_STOP_REPLY_MS){
                        if(thaisen_ammeter_is_replied(gunno) == APP_THA_ENUM_FALSE){
                            return;
                        }
                    }
                    s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_METER_READING;
                    s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();

                    LOG_D("meter encrypt:meter reading[%d, %d](poweroff bill)", gunno, (rt_tick_get() - s_ofsm_info[gunno].base.meter_reading_tick));
                    thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_METER_READING);
                    return;
                }
                /** 等待电表加密抄表指令响应 */
                else if(s_ofsm_info[gunno].base.meter_step == APP_AMMETER_ENCRY_STEP_METER_READING){
                    if((rt_tick_get() - s_ofsm_info[gunno].base.meter_reading_tick) < APP_AMMETER_ENCRY_WAIT_READING_REPLY_MS){
                        if(thaisen_ammeter_is_replied(gunno) == APP_THA_ENUM_TRUE){
                            uint8_t _compare_len = 0x00;
                            LOG_D("meter encrypt:meter reading is replied[%d, %d](poweroff bill)\n", gunno, (rt_tick_get() - s_ofsm_info[gunno].base.meter_reading_tick));
                            thaisen_encry_data_t *_encry_data = (thaisen_encry_data_t*)thaisen_ammeter_query_encrypt_data(gunno);
                            if(_encry_data){
                                rtransaction.meter_ver = _encry_data->ver;
                                rtransaction.meter_encry_type = _encry_data->encry_type;

                                _compare_len = sizeof(_encry_data->trade_number);
                                _compare_len = _compare_len > sizeof(rtransaction.meter_trade_number) ? sizeof(rtransaction.meter_trade_number) : _compare_len;
                                memset(rtransaction.meter_trade_number, 0x00, sizeof(rtransaction.meter_trade_number));
                                memcpy(rtransaction.meter_trade_number, _encry_data->trade_number, _compare_len);

                                _compare_len = sizeof(_encry_data->dev_sn);
                                _compare_len = _compare_len > sizeof(rtransaction.meter_dev_sn) ? sizeof(rtransaction.meter_dev_sn) : _compare_len;
                                memset(rtransaction.meter_dev_sn, 0x00, sizeof(rtransaction.meter_dev_sn));
                                memcpy(rtransaction.meter_dev_sn, _encry_data->dev_sn, _compare_len);

                                _compare_len = sizeof(_encry_data->port_identify_sn);
                                _compare_len = _compare_len > sizeof(rtransaction.meter_port_identify_sn) ? sizeof(rtransaction.meter_port_identify_sn) : _compare_len;
                                memset(rtransaction.meter_port_identify_sn, 0x00, sizeof(rtransaction.meter_port_identify_sn));
                                memcpy(rtransaction.meter_port_identify_sn, _encry_data->port_identify_sn, _compare_len);

                                rtransaction.meter_stimestamp = _encry_data->stimestamp;
                                rtransaction.meter_etimestamp = _encry_data->etimestamp;
                                rtransaction.meter_positive_elect = _encry_data->positive_elect;
                                rtransaction.meter_install_timestamp = _encry_data->install_timestamp;
                                rtransaction.meter_history_state = _encry_data->history_state;
                            }
                        }else{
                            return;
                        }
                    }
                    s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_NULL;
                    s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();
                }else{
                    s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_NULL;
                    s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();
                    return;
                }
#endif /* APP_INCLUDE_YKC17_PROTOCOL */
                /** 这是不对的情况 */
                if(_total_elect < rtransaction.ammeter_stop){
                    _total_elect = rtransaction.ammeter_stop;
                }

                if(rtransaction.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                    _rated_number = sys_get_offbilling_rate_number(rtransaction.end_time);
                    if(_rated_number > CP_RATED_TYPE_MAX){
                        _rated_number = CP_PERIOD_RATED_NUMBER_DEFAULT;
                    }

                    _price = sys_get_offbilling_unit_price(rtransaction.end_time);
                    if((_price > CP_UNIT_PRICE_MAX) || (_price < CP_UNIT_PRICE_MIN)){
                        _price = CP_UNIT_PRICE_DEFAULT;
                    }
                    _unit_price = _price;
                    _unit_price /= 1000;   /** 单价是4位小数，此处只需保留一位 */
                }

                loss_elect = _total_elect - rtransaction.ammeter_stop;
                rtransaction.ammeter_stop = _total_elect;

                rtransaction.total_elect = (rtransaction.ammeter_stop - rtransaction.ammeter_start);
                rtransaction.charge_fee += ((double)loss_elect *(double)_unit_price);
                rtransaction.total_fee += ((double)loss_elect *(double)_unit_price);

                rtransaction.end_time += (15 *60);
                rtransaction.charge_time += (15 *60);
                if(rtransaction.end_time > rtransaction.charge_time){
                    rtransaction.start_time = rtransaction.end_time - rtransaction.charge_time;
                }else{
                    rtransaction.start_time = rtransaction.end_time;
                    rtransaction.charge_time = 0x00;
                }
                if(rtransaction.charge_fee > APP_SPEND_AMOUNT_MAX){
                    rtransaction.charge_fee = APP_SPEND_AMOUNT_MAX;
                }
                if(rtransaction.total_fee > APP_SPEND_AMOUNT_MAX){
                    rtransaction.total_fee = APP_SPEND_AMOUNT_MAX;
                }
                if(rtransaction.total_elect > APP_CHARGE_ELECT_MAX){
                    rtransaction.total_elect = APP_CHARGE_ELECT_MAX;
                }

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||  \
                defined (APP_INCLUDE_SGCC_PROTOCOL))
                rtransaction.rate_type_elect[_rated_number] += loss_elect;
                rtransaction.rate_type_amount[_rated_number] += ((double)loss_elect *(double)_unit_price);
                if(rtransaction.rate_type_amount[_rated_number] > APP_SPEND_AMOUNT_MAX){
                    rtransaction.rate_type_amount[_rated_number] = APP_SPEND_AMOUNT_MAX;
                }
                if(rtransaction.rate_type_elect[_rated_number] > APP_CHARGE_ELECT_MAX){
                    rtransaction.rate_type_elect[_rated_number] = APP_CHARGE_ELECT_MAX;
                }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
                defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
                if(rtransaction.start_period_number > (APP_BILLING_RULE_PERIOD_MAX - 0x01)){
                    rtransaction.start_period_number = (APP_BILLING_RULE_PERIOD_MAX - 0x01);
                }
                rtransaction.period_elect[rtransaction.start_period_number] += loss_elect;
                rtransaction.period_elect_fees[rtransaction.start_period_number] += ((double)loss_elect *(double)_unit_price);

                if(rtransaction.period_elect_fees[rtransaction.start_period_number] > APP_SPEND_AMOUNT_MAX){
                    rtransaction.period_elect_fees[rtransaction.start_period_number] = APP_SPEND_AMOUNT_MAX;
                }
                if(rtransaction.period_elect[rtransaction.start_period_number] > APP_CHARGE_ELECT_MAX){
                    rtransaction.period_elect[rtransaction.start_period_number] = APP_CHARGE_ELECT_MAX;
                }
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */
                need_check = 1;
            }
            rtransaction.order_state.is_charging = APP_THA_ENUM_FALSE;
        }else{
            need_check = 1;
            s_ofsm_info[gunno].base.order_fixes_tick = rt_tick_get();
        }
        if(need_check){
            int32_t ret = 0x00;
            while(save_rentry < 5){
                ret = 0x00;
                if((s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING) && (rtransaction.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD)){
                    ret = mw_storage_record_designate_index_updated(&rtransaction, sizeof(rtransaction), USER_DATA_TYPE_VERIFIED, 0x00, APP_THA_ENUM_FALSE, gunno, index);
                }else{
                    ret = mw_storage_record_designate_index_updated(&rtransaction, sizeof(rtransaction), USER_DATA_TYPE_VERIFIED, 0x00, APP_THA_ENUM_TRUE, gunno, index);
                }
                if(ret < 0x00){
                    rt_thread_mdelay(10);
                    save_rentry++;
                    continue;
                }else{
                    break;
                    s_ofsm_info[gunno].base.order_fixes_tick = rt_tick_get();
                }
                LOG_D("order have adjusted");
                break;
            }
        }
#ifdef APP_INCLUDE_YKC17_PROTOCOL
            s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_NULL;
            s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();
#endif /* APP_INCLUDE_YKC17_PROTOCOL */
        return;
    }else{
        s_ofsm_info[gunno].base.order_fixes_tick = rt_tick_get();
#ifdef APP_INCLUDE_YKC17_PROTOCOL
        s_ofsm_info[gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_NULL;
        s_ofsm_info[gunno].base.meter_reading_tick = rt_tick_get();
#endif /* APP_INCLUDE_YKC17_PROTOCOL */
    }

#ifdef APP_INCLUDE_TARGET_PLATFORM
    if(app_nsal_query_transaction_verify_state(gunno, APP_TARGET_PLATFORM_ID) == APP_THA_ENUM_TRUE){
        if(s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] == APP_THA_ENUM_TRUE){
            LOG_D("gunno(%d) target transaction verify", gunno);
            mw_storage_record_designate_index_updated(&(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), sizeof(thaisen_transaction_t), \
                    USER_DATA_TYPE_REPORTED, APP_TARGET_PLATFORM_ID, APP_THA_ENUM_TRUE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
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
                    USER_DATA_TYPE_REPORTED, APP_MONITOR_PLATFORM_ID, APP_THA_ENUM_TRUE, gunno, s_current_order_index[MONITOR_PLATFORM_INDEX][gunno]);
        }
        s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] = APP_THA_ENUM_FALSE;
    }else{
        if(s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] == APP_THA_ENUM_FALSE){
            if(mw_storage_record_get_recordverify_record_num(APP_MONITOR_PLATFORM_ID, gunno) > 0x00){
                int32_t index = mw_storage_record_get_first_index_recordverify(APP_MONITOR_PLATFORM_ID, gunno), result = STORAGE_ERR_NONE;
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

/***************************************************************************
 * 函数名       temperature_protect_limitcurr
 * 功能           检查是否过温并降流
 * 参数           gunno      枪号
 *        set_curr   被修改电流值指针
 * 返回
 **************************************************************************/
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

/***************************************************************************
 * 函数名       chargepile_power_adjust
 * 功能           功率调整
 * 参数
 * 返回
 **************************************************************************/
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
                    s_ofsm_info[gunno].base.flag.is_resume_power = APP_THA_ENUM_FALSE;

                    if(s_ofsm_info[gunno].base.power_strategy == APP_POWER_STRATEGY_SET_LIMIT){
                        s_system_power_output = app_nsal_get_setup_power(gunno) *10;
                        s_ofsm_info[gunno].base.flag.is_adjust_power = APP_THA_ENUM_FALSE;
                        s_ofsm_info[gunno].base.flag.is_resume_power = APP_THA_ENUM_FALSE;
                        LOG_D("power strategy is not order charge, limit power is[%d, %d]", gunno, app_nsal_get_setup_power(gunno));
                    }else{
                        if(s_ofsm_info[gunno].base.flag.is_adjust_power == APP_THA_ENUM_FALSE){
                            s_ofsm_info[gunno].base.flag.is_adjust_power = APP_THA_ENUM_TRUE;
                            original_power[gunno] = s_system_power_output;
                        }
                        s_system_power_output = app_nsal_get_setup_power(gunno) *10;
                        if((s_ofsm_info[gunno].state != APP_OFSM_STATE_STARTING) && (s_ofsm_info[gunno].state != APP_OFSM_STATE_CHARGING)){
                            server_adjust_power = false;
                            s_issue_power_adjust = false;
                            LOG_D("gunno(%d) is not charging, not allow set order power", gunno);
                        }
                    }
#ifdef CP_CONFIG_USING_DUPU
                    if(s_system_power_output > terminal_get_ems_set_power() *1000){
                        s_system_power_output =  terminal_get_ems_set_power() *1000;
                    }
#endif /* CP_CONFIG_USING_DUPU */
                }
                LOG_I("gunno(%d) server adjust system power|%d, %d, %d", gunno, app_nsal_get_setup_power(gunno), s_ofsm_info[gunno].base.system_power_max, s_ofsm_info[gunno].base.power_strategy);
            }
        }
    }
    /** 上电判断之前桩是否被设置过功率，如果是则需要进行功率调节 */
    if((thaisen_is_set_power_percent() || terminal_is_ems_adjust_power() || s_power_on == 0) && server_adjust_power == false){
        uint32_t power = *((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_SYSTEM_POWER_TOTAL, 0)));
        uint32_t sys_power_max = 0x00, single_module_power = 0x00;
        uint8_t group = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0));

        single_module_power = ((*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0)))) *  \
                (*((uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0)))));
        for(uint8_t count = 0; count < group; count++){
            sys_power_max += (sys_get_single_group_module_num(count) *single_module_power);
        }
#ifdef CP_CONFIG_USING_DUPU
        if(terminal_is_ems_adjust_power()){
            if((terminal_get_ems_set_power() *100 < power) && (terminal_get_ems_set_power() != 0x00)){
                power = terminal_get_ems_set_power() *100;
            }
        }else if((power > terminal_get_ems_set_power() *100) && (terminal_get_ems_set_power() != 0x00)){
            power =  terminal_get_ems_set_power() *100;
        }
#endif /* CP_CONFIG_USING_DUPU */
        if((power >= sys_power_max /100) && (power <= sys_power_max)){
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
    if((++s_power_adjust_delay > ADJUST_PERIOD) || server_adjust_power || s_power_on == 0 || terminal_is_ems_adjust_power()){
        if(terminal_is_ems_adjust_power()){
            terminal_clear_is_ems_adjust_power();
        }
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

/**************************************************************************************************************************
 *                                               以下是启动信息填充
 *************************************************************************************************************************/
/***************************************************************
 * 函数名          ofsm_start_info_padding_plug_and_play
 * 功能               即插即充启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_plug_and_play(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t valid_len = 0x00;

    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_PLUG_AND_CHARGE;
    s_ofsm_info[gunno].base.account_ballance_before = 0x00;
    s_ofsm_info[gunno].base.account_ballance_after = 0x00;
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
    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_PLUG_AND_CHARGE;
    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_app
 * 功能               app启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_app(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t valid_len = 0x00;

    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_APP;

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
    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_APP;
    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_offline_card
 * 功能               离线卡启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_offline_card(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t compare_len = 0x00;

    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
    s_ofsm_info[gunno].base.account_ballance_before = 0x00;
    s_ofsm_info[gunno].base.account_ballance_after = 0x00;
    s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_FULL;
    s_ofsm_info[gunno].base.charge_strategy_para = 0x00;

    app_nsal_create_local_transaction_number(gunno, &(s_ofsm_info[gunno].base.transaction_number),  \
            sizeof(s_ofsm_info[gunno].base.transaction_number));
#ifdef APP_INCLUDE_SGCC_PROTOCOL
    memcpy(s_ofsm_info[gunno].base.device_transaction_number, s_ofsm_info[gunno].base.transaction_number, \
            sizeof(s_ofsm_info[gunno].base.transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

    memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

    compare_len = sizeof(s_ofsm_info[gunno].base.card_uid);
    compare_len = compare_len > rfidr_query_uuid_len() ? rfidr_query_uuid_len() : compare_len;
    memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
    memcpy(s_ofsm_info[gunno].base.card_uid, rfidr_query_uuid(), compare_len);

    compare_len = sizeof(s_ofsm_info[gunno].base.transaction_number);
    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].serial_number) ? sizeof(s_thaisen_transaction[gunno].serial_number) : compare_len;
    memset(s_thaisen_transaction[gunno].serial_number, 0x00, sizeof(s_thaisen_transaction[gunno].serial_number));
    memcpy(s_thaisen_transaction[gunno].serial_number, s_ofsm_info[gunno].base.transaction_number, compare_len);

    compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].logic_card_number) ? sizeof(s_thaisen_transaction[gunno].logic_card_number) : compare_len;
    memset(s_thaisen_transaction[gunno].logic_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].logic_card_number));
    memcpy(s_thaisen_transaction[gunno].logic_card_number, s_ofsm_info[gunno].base.card_number, compare_len);

    compare_len = sizeof(s_ofsm_info[gunno].base.card_uid);
    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].physics_card_number) ? sizeof(s_thaisen_transaction[gunno].physics_card_number) : compare_len;
    memset(s_thaisen_transaction[gunno].physics_card_number, 0x00, sizeof(s_thaisen_transaction[gunno].physics_card_number));
    memcpy(s_thaisen_transaction[gunno].physics_card_number, s_ofsm_info[gunno].base.card_uid, compare_len);

    compare_len = sizeof(s_ofsm_info[gunno].base.user_number);
    compare_len = compare_len > sizeof(s_thaisen_transaction[gunno].user_number) ? sizeof(s_thaisen_transaction[gunno].user_number) : compare_len;
    memset(s_thaisen_transaction[gunno].user_number, 0x00, sizeof(s_thaisen_transaction[gunno].user_number));
    memcpy(s_thaisen_transaction[gunno].user_number, s_ofsm_info[gunno].base.user_number, compare_len);
#ifdef APP_INCLUDE_SGCC_PROTOCOL
    memcpy(s_thaisen_transaction[gunno].device_serial_number, s_ofsm_info[gunno].base.device_transaction_number, \
            sizeof(s_ofsm_info[gunno].base.device_transaction_number));
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_OFFLINE_CARD;
    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_online_card
 * 功能               在线卡启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_online_card(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t valid_len = 0x00;

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
    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_ONLINE_CARD;
    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_screen
 * 功能               屏幕启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_screen(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t valid_len = 0x00;

    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_SCREEN;
    s_ofsm_info[gunno].base.account_ballance_before = 0x00;
    s_ofsm_info[gunno].base.account_ballance_after = 0x00;
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
    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_SCREEN;
    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_vin
 * 功能               VIN码启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_vin(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t valid_len = 0x00;

    s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;
    s_ofsm_info[gunno].base.flag.vin_authorization_success = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.vin_is_authorized = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.start_type = APP_CHARGE_START_WAY_VIN;
    s_ofsm_info[gunno].base.account_ballance_before = 0x00;
    s_ofsm_info[gunno].base.account_ballance_after = 0x00;
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
    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

    s_thaisen_transaction[gunno].start_type = APP_CHARGE_START_WAY_VIN;
    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_FALSE;
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_password
 * 功能               密码启动  信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_password(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }
}

/***************************************************************
 * 函数名          ofsm_start_info_padding_public
 * 功能               启动充电  公用信息填充
 * 参数              gunno   枪号
 * 返回
 **************************************************************/
static void ofsm_start_info_padding_public(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
    thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_START);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

    /** 云端并充时 main_gunno charge_way 这两个字段已被赋值 */
    if(s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD){
        s_ofsm_info[gunno].base.main_gunno = gunno;
        s_ofsm_info[gunno].base.charge_way = thaisen_get_charge_way();
    }

    /** 这主要是判是否可以并充并设置充电模式，云端并充时已在网络部分判断了并充的可行性 */
    if(s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD){
        if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
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

        if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
            if(gunno == s_ofsm_info[gunno].base.main_gunno){
                s_ofsm_info[gunno].base.start_elect = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
            }
            s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_PARACHARGE_LOCAL;
        }else{
            s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_SINGLEGUN;
            s_ofsm_info[gunno].base.start_elect = mw_get_meter_total_wh(gunno);
        }
    }else{
        s_ofsm_info[gunno].base.start_elect = mw_get_meter_total_wh(gunno);
    }

    thaisen_set_charge_way(s_ofsm_info[gunno].base.charge_way);

    s_ofsm_info[gunno].base.flag.is_starting = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.permit_judge_complete = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.paracharge_is_identified = APP_THA_ENUM_FALSE;
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
    s_ofsm_info[gunno].base.charge_elect_last = s_ofsm_info[gunno].base.start_elect;
    if((s_ofsm_info[gunno].base.start_elect == 0x00) || (s_ofsm_info[gunno].base.current_elect == 0x00)){
        s_ofsm_info[gunno].base.flag.is_ammeter_elect_error = APP_THA_ENUM_TRUE;
    }else{
        s_ofsm_info[gunno].base.flag.is_ammeter_elect_error = APP_THA_ENUM_FALSE;
    }

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
    s_thaisen_transaction[gunno].run_mode = s_ofsm_info[gunno].base.run_mode;

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
    memset(s_thaisen_transaction[gunno].elect_model_sn, 0x00, (APP_BILLING_MODEL_SN_LEN + 1));
    memset(s_thaisen_transaction[gunno].service_model_sn, 0x00, (APP_BILLING_MODEL_SN_LEN + 1));

    memcpy(s_thaisen_transaction[gunno].elect_model_sn, app_billingrule_get_elect_model_sn(gunno), APP_BILLING_MODEL_SN_LEN);
    memcpy(s_thaisen_transaction[gunno].service_model_sn, app_billingrule_get_service_model_sn(gunno), APP_BILLING_MODEL_SN_LEN);
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

    memset(&(s_thaisen_transaction[gunno].bms_stop_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_stop_reason));
    memset(&(s_thaisen_transaction[gunno].bms_fault_reason), 0x00, sizeof(s_thaisen_transaction[gunno].bms_fault_reason));
    s_booting_step[gunno] = APP_BOOTING_STEP_IDLE;                 /* 初始化充电步骤 */

    app_nsal_init_charge_data(gunno);

    app_nsal_state_charged(gunno);
    app_nsal_event_occurded(gunno);

    mw_storage_record_create(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
            0x00, APP_THA_ENUM_FALSE, gunno);
    s_current_order_index[MONITOR_PLATFORM_INDEX][gunno] = mw_storage_record_get_current_index(gunno);
    s_current_order_index[TARGET_PLATFORM_INDEX][gunno] = s_current_order_index[MONITOR_PLATFORM_INDEX][gunno];
}

/***************************************************************
 * 函数名          ofsm_swip_card_judge
 * 功能               刷卡判断
 * 参数              gunno   枪号
 * 返回              1: 鉴权成功，0: 鉴权失败
 **************************************************************/
static uint8_t ofsm_swip_card_judge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return APP_THA_ENUM_FALSE;
    }

    uint8_t need_authorize_online = APP_THA_ENUM_FALSE;

    /***************************************************************
     * 此次刷卡得到的卡信息： 卡号
     * ************************************************************/
    if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_CARD_NUMBER){
        uint8_t pile_number_len = APP_CARD_NUMBER_COMPARE_LEN, card_number_len = rfidr_query_card_number_len(),
                compare_len = 0x00, count = 0x00, compare_count = APP_CARD_NUMBER_COMPARE_LEN_MIN;    /** 桩号卡对比长度 */
        uint8_t *pile_number = NULL, *card_number = NULL;

        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            if(app_card_event_recv(APP_CARD_EVENT_CHARGE_START, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) < APP_THA_ENUM_FALSE){
                if(app_card_event_recv(APP_CARD_EVENT_IS_NOT_SAME_PORT, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) < APP_THA_ENUM_FALSE){
                    LOG_D("gunno(%d) is not receive card start charge event in offline billing", gunno);
                    app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                }
                return APP_THA_ENUM_FALSE;
            }
        }
        /** 离线计费模式只判断卡号前6位 */
        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            compare_count = APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING;
        }

        compare_len = pile_number_len > card_number_len ? card_number_len : pile_number_len;
        pile_number = sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0);
        card_number = rfidr_query_card_number();

        /** 判断此卡是否符合充电要求 */
        if(compare_len >= compare_count){
            for(count = 0x00; count < compare_count; count++){
                if(pile_number[count] != card_number[count]){
                    break;
                }
            }
            if(count >= compare_count){
                app_rfidr_send_mail(APP_BUZZON_STATE_OK);
                LOG_D("gunno(%d) start charge by local pile number card", gunno);
            }
        }

        if((compare_len < compare_count) || (count < compare_count)){
            if(sys_card_number_whitelists_query(card_number, card_number_len) >= 0x00){
                app_rfidr_send_mail(APP_BUZZON_STATE_OK);
                LOG_D("gunno(%d) start charge by local whitelist card", gunno);
            }else{
                if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                    /** 提示鉴权失败, 离线计费必须是本地卡 */;
                    LOG_D("gunno(%d) invalid card in offline billing mode, compare length 00", gunno);
                    app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                    thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);
                    return APP_THA_ENUM_FALSE;
                }
                need_authorize_online = APP_THA_ENUM_TRUE;
            }
        }

        compare_len = sizeof(s_ofsm_info[gunno].base.card_number);
        compare_len = compare_len > card_number_len ? card_number_len : compare_len;
        memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
        memcpy(s_ofsm_info[gunno].base.card_number, card_number, compare_len);

        compare_len = sizeof(s_ofsm_info[gunno].base.card_uid);
        compare_len = compare_len > rfidr_query_uuid_len() ? rfidr_query_uuid_len() : compare_len;
        memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
        memcpy(s_ofsm_info[gunno].base.card_uid, rfidr_query_uuid(), compare_len);

        s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_FALSE;
        s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;

        /** 桩号卡、白名单卡；不需要平台鉴权 */
        if(need_authorize_online == APP_THA_ENUM_FALSE){
            ofsm_start_info_padding_offline_card(gunno);

            if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                s_ofsm_info[gunno].base.account_ballance_before = app_card_query_ballance(gunno);
                s_ofsm_info[gunno].base.account_ballance_after = app_card_query_ballance(gunno);

                s_ofsm_info[gunno].base.charge_strategy = APP_CHARGE_STRATEGY_MONEY;
                s_ofsm_info[gunno].base.charge_strategy_para = s_ofsm_info[gunno].base.account_ballance_before *100;
            }
            return APP_THA_ENUM_TRUE;
        }
    }
    /***************************************************************
     * 此次刷卡得到的卡信息： 卡UUID
     * ************************************************************/
    else if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_UUID){
        uint8_t uid_len = rfidr_query_uuid_len();

        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            /** 提示鉴权失败, 离线计费获取到的卡信息类型必须是卡号 */;
            LOG_D("gunno(%d) invalid card in offline billing mode, key authen fail", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_KEY_AUTHEN_FAIL, 0x05, APP_THA_ENUM_TRUE, gunno);
            return APP_THA_ENUM_FALSE;
        }

        if(sys_card_uid_whitelists_query(rfidr_query_uuid(), uid_len) >= 0x00){
            app_rfidr_send_mail(APP_BUZZON_STATE_OK);
            LOG_D("gunno(%d) start charge by local whitelist card uid", gunno);
        }else{
            need_authorize_online = APP_THA_ENUM_TRUE;
        }

        uid_len = uid_len > sizeof(s_ofsm_info[gunno].base.card_uid) ? sizeof(s_ofsm_info[gunno].base.card_uid) : uid_len;
        memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
        memcpy(s_ofsm_info[gunno].base.card_uid, rfidr_query_uuid(), uid_len);

        memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));

        s_ofsm_info[gunno].base.card_uid_len = uid_len;
        s_ofsm_info[gunno].base.flag.card_info_is_uid = APP_THA_ENUM_TRUE;
        s_ofsm_info[gunno].base.flag.is_local_charging = APP_THA_ENUM_TRUE;

        /** 白名单卡；不需要平台鉴权 */
        if(need_authorize_online == APP_THA_ENUM_FALSE){
            ofsm_start_info_padding_offline_card(gunno);
            return APP_THA_ENUM_TRUE;
        }
    }
    /***************************************************************
     * 此次刷卡得到的卡信息： 读取卡号失败
     * ************************************************************/
    else if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_ERROR){
        /** 提示：无效卡 */
        LOG_D("gunno(%d) invalid card in offline billing mode, read card number fail", gunno);
        app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);

        need_authorize_online = APP_THA_ENUM_FALSE;
    }
    /***************************************************************
     * 此次刷卡得到的卡信息： 卡读、写信息操作失败
     * ************************************************************/
    else if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_RW_FAIL){
        switch(app_card_query_operate_ret(gunno)){
        case APP_CARD_OPERATE_RET_NO_BALLANCE:
            LOG_D("gunno(%d) no ballance in offline billing mode", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_CARD_NOBALLANCE, 0x05, APP_THA_ENUM_TRUE, gunno);
            break;
        case APP_CARD_OPERATE_RET_IS_LOCKED:
            LOG_D("gunno(%d) card is locked in offline billing mode", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_CARD_LOCKED, 0x05, APP_THA_ENUM_TRUE, gunno);
            break;
        case APP_CARD_OPERATE_RET_HISTORY_BILL_ERROR:
            LOG_D("gunno(%d) card is locked(no bill) in offline billing mode", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_CARD_LOCKED, 0x05, APP_THA_ENUM_TRUE, gunno);
            break;
        case APP_CARD_OPERATE_RET_IS_CHARGING:
            LOG_D("gunno(%d) this card is start in offline billing mode", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_IS_CHARGING, 0x05, APP_THA_ENUM_TRUE, gunno);
            break;
        case APP_CARD_OPERATE_RET_PAYED:
            LOG_D("gunno(%d) card is payed in offline billing mode 555", gunno);
            break;
        case APP_CARD_OPERATE_RET_NULL:
            LOG_D("gunno(%d) current state is not allow stop in offline billing mode", gunno);
            break;
        default:
            LOG_D("gunno(%d) invalid card in offline billing mode 111", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);
            break;
        }
        need_authorize_online = APP_THA_ENUM_FALSE;
    }
    /***************************************************************
     * 此次刷卡得到的卡信息： 其它
     * ************************************************************/
    else{
        need_authorize_online = APP_THA_ENUM_FALSE;
        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            /** 提示鉴权失败, 信息有误 */;
            LOG_D("gunno(%d) operate card error in offline billing mode 00", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_KEY_AUTHEN_FAIL, 0x05, APP_THA_ENUM_TRUE, gunno);
            return APP_THA_ENUM_FALSE;
        }else{
            /** 信息有误, 蜂鸣器提示 */
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
        }
    }

    if(need_authorize_online){
        if(app_nsal_card_authorize(gunno) < 0x00){
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            LOG_W("gunno(%d) swip card authorize fail", gunno);
            return APP_THA_ENUM_FALSE;
        }
        s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_TRUE;
    }

    return APP_THA_ENUM_FALSE;
}

/**************************************************************************************************************************
 *                                               以下是业务状态机状态
 *************************************************************************************************************************/
/*****************************************************
 * 函数名          ofsm_info_init_fun
 * 功能               业务状态机信息初始化状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
static void ofsm_info_init_fun(uint8_t gunno)
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

/*****************************************************
 * 函数名          ofsm_idleing_fun
 * 功能               业务状态机 空闲 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
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
    s_ofsm_info[gunno].base.flag.is_deputygun_stop = APP_THA_ENUM_FALSE;

    memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
    memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));

    s_ofsm_info[gunno].base.main_gunno = 0x00;
    s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_NONE;

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

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        if(app_card_event_recv(APP_CARD_EVENT_PAY_COMPLETE, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) >= APP_THA_ENUM_FALSE){
            LOG_D("gunno(%d) idle card is payed in offline billing", gunno);
            LOG_D("time:%ds  elect:%d  money:%d  ballance:%d  reason:%d", s_ofsm_info[gunno].base.charge_time,
                    s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.fees_total, s_ofsm_info[gunno].base.account_ballance_after,
                    s_ofsm_info[gunno].base.reason_code);

            rfidr_clear_swipe_state(gunno);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAY_COMPLETE, 30, APP_THA_ENUM_TRUE, gunno);
        }
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
        if(rfidr_query_swipe_state(gunno)){
            rfidr_clear_swipe_state(gunno);
            if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                /** 提示先插枪再刷卡 */
                if(app_card_event_recv(APP_CARD_EVENT_IS_NOT_SAME_PORT, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) < APP_THA_ENUM_FALSE){
                    LOG_D("gunno(%d) please insert the gun first", gunno);
                    thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INSERT_GUN, 0x05, APP_THA_ENUM_TRUE, gunno);
                    app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                }
            }else{
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            }
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
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

    app_nsal_clear_remote_start(gunno);
    app_nsal_clear_remote_stop(gunno);
    app_nsal_clear_remote_card_authorize(gunno);
    app_nsal_clear_remote_vin_authorize(gunno);

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    }
}

/*****************************************************
 * 函数名          ofsm_readying_fun
 * 功能               业务状态机 插枪准备 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
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

    s_ofsm_info[gunno].base.flag.is_deputygun_stop = APP_THA_ENUM_FALSE;

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

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        if(app_card_event_recv(APP_CARD_EVENT_PAY_COMPLETE, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) >= APP_THA_ENUM_FALSE){
            LOG_D("gunno(%d) ready card is payed in offline billing", gunno);
            LOG_D("time:%ds  elect:%d  money:%d  ballance:%d  reason:%d", s_ofsm_info[gunno].base.charge_time,
                    s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.fees_total, s_ofsm_info[gunno].base.account_ballance_after,
                    s_ofsm_info[gunno].base.reason_code);

            rfidr_clear_swipe_state(gunno);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAY_COMPLETE, 30, APP_THA_ENUM_TRUE, gunno);
        }
    }

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

                rfidr_clear_swipe_state(gunno);
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
        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            app_card_event_send(APP_CARD_EVENT_CHARGEPILE_READY, gunno, NULL);
        }else{
            app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
        }

        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_PLUG_AND_PLAY){
            LOG_D("gunno(%d) start charge by plug and play", gunno);
            ofsm_start_info_padding_plug_and_play(gunno);
            is_charging_authorization = true;

        }else if(app_nsal_is_remote_start(gunno)){
            app_nsal_clear_remote_start(gunno);
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
#ifndef APP_USING_DOUBLEGUN
                break;      /** 单枪不允许并充 */
#endif /* APP_USING_DOUBLEGUN */
            }

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
             * base->offline_chargetime
             * base->main_gunno(并充时)
             * base->charge_way(并充时)*/

            LOG_D("gunno(%d) start charge by APP", gunno);
            ofsm_start_info_padding_app(gunno);
            is_charging_authorization = true;

        }else if(rfidr_query_swipe_state(gunno)){
            rfidr_clear_swipe_state(gunno);
            if(s_ofsm_info[gunno].base.run_mode != APP_RUN_MODE_OFFLINE_BILLING){
                if(thaisen_is_not_allow_swip_card()){
                    LOG_D("gunno(%d) current page is not allow swip card charge", gunno);
                    return;
                }
            }
            if(ofsm_swip_card_judge(gunno)){
                is_charging_authorization = true;
            }else{
                return;
            }
        }else if(app_nsal_is_card_authorize_success(gunno)){
            app_nsal_clear_remote_card_authorize(gunno);

            LOG_D("gunno(%d) start charge by online card", gunno);
            ofsm_start_info_padding_online_card(gunno);
            is_charging_authorization = true;

        }else if(app_nsal_is_card_authorize_fail(gunno)){
            app_nsal_clear_remote_card_authorize(gunno);
            LOG_W("gunno(%d) swip card authorize response fail", gunno);
            s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;

            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);

        }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, APP_THA_ENUM_TRUE)){
            LOG_D("gunno(%d) start charge by screen", gunno);
            ofsm_start_info_padding_screen(gunno);
            is_charging_authorization = true;

        }else if(app_get_hci_event(gunno, HCI_EVENT_VIN_START, APP_THA_ENUM_TRUE)){
            LOG_D("gunno(%d) start charge by VIN", gunno);
            ofsm_start_info_padding_vin(gunno);
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
            /** 启动前向屏幕对时 */
            thaisen_request_screen_time();

            ofsm_start_info_padding_public(gunno);

            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STARTING;
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
        rfidr_clear_swipe_state(gunno);
        app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
        app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);
        app_nsal_clear_remote_start(gunno);
        app_nsal_clear_remote_card_authorize(gunno);
    }else{
        mw_clear_time_sync_flag(gunno);
    }
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_nsal_clear_remote_stop(gunno);

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    }
}

/*****************************************************
 * 函数名          ofsm_reservation_fun
 * 功能               业务状态机 预约 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
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
        rfidr_clear_swipe_state(gunno);
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
/*****************************************************
 * 函数名          ofsm_starting_fun
 * 功能               业务状态机 启动中 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
static void ofsm_starting_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    uint8_t vin_authentication_complete = APP_THA_ENUM_FALSE;
    enum system_stop_way stop_way = APP_SYSTEM_STOP_WAY_SIZE;
    enum charge_state_t charge_state = mw_get_charge_state(gunno);
    enum charge_fault_t charge_fault = app_get_highest_priority_charge_fault(gunno);
    enum system_fault_t system_fault = app_get_highest_priority_system_fault(gunno);

    s_ofsm_info[gunno].base.charctrl_state.current = charge_state;

    if (++s_debug_count[gunno] > (1000  + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) starting state (%dV | S%d)...", gunno, mw_get_cc1_value(mw_get_cc1(gunno)), charge_state);
    }

    s_ofsm_info[gunno].base.flag.is_pay_complete = APP_THA_ENUM_FALSE;
    s_ofsm_info[gunno].base.flag.bms_require_decrease = APP_THA_ENUM_FALSE;
    s_compare_module_bcl_count[gunno] = rt_tick_get();
    s_compare_ccs_module_count[gunno] = rt_tick_get();
    s_tiny_current_count[gunno] = rt_tick_get();
    s_bms_require_curr_last[gunno] = 0x00;
    s_bms_reqcurr_changed_count[gunno] = 0x00;

    /*******************************************************************************
     ********************************  并充自动识别    ***********************************
     ******************************************************************************/
    if(s_ofsm_info[gunno].base.flag.paracharge_is_identified == APP_THA_ENUM_FALSE){
#ifdef APP_USING_DOUBLEGUN
        /** 并充自动识别前提条件1：本次屏幕选择充电模式为单枪单车(选双枪单车时就按并充模式来)、已启用并充功能、设备类型不是动态双枪 */
        if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_SINGLEGUN) &&   \
                ((*sys_read_config_item_content(CONFIG_ITEM_SUPORT_PARALLEL, 0x00)) == APP_THA_ENUM_TRUE) &&  \
                ((*sys_read_config_item_content(CONFIG_ITEM_DEVICE_TYPE, 0x00)) != SYSTEM_FUNCTION_DYNAMIC_SWITCH)){
            mw_can_info data;
            uint8_t rentry = 0x00, deputy_gunno = APP_SYSTEM_GUNNOA, deputy_gun_enum = THAISEN_BMS_A_CAN_RECV;

            if(gunno == APP_SYSTEM_GUNNOA){
                deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                deputy_gun_enum = THAISEN_BMS_B_CAN_RECV;
            }
            /** 并充自动识别前提条件2：并充时两把枪都要插上  副枪要处于未充电状态 */
            if((s_ofsm_info[deputy_gunno].base.flag.connect_state == APP_CONNECT_STATE_CONNECT) &&  \
                    (s_ofsm_info[deputy_gunno].state == APP_OFSM_STATE_READYING)){
                mw_clear_can_recved(deputy_gun_enum);
                /** 等待响应，最多等待500ms */
                while(1){
                    ofsm_bms_can_send(gunno, &data);          /** 发送并充识别报文 */
                    if(++rentry > 10){
                        break;
                    }

                    if(mw_is_can_recved(deputy_gun_enum)){
                        mw_clear_can_recved(deputy_gun_enum);
                        if(ofsm_is_belong_one_car(gunno) != APP_THA_ENUM_TRUE){
                            LOG_D("gunno(%d) identify paracharge, both gun is not belong to the same car", gunno);
                            break;
                        }
                        /** 这是两把枪插在一辆车上了，启动并充模式 */
                        s_ofsm_info[gunno].base.main_gunno = gunno;
                        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_PARACHARGE_LOCAL;

                        s_ofsm_info[deputy_gunno].base.charge_way = s_ofsm_info[gunno].base.charge_way;
                        s_ofsm_info[deputy_gunno].base.main_gunno = s_ofsm_info[gunno].base.main_gunno;

                        s_ofsm_info[gunno].base.start_elect = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                        thaisen_set_charge_way(s_ofsm_info[gunno].base.charge_way);

                        s_thaisen_transaction[gunno].charge_way = APP_CHARGE_WAY_PARACHARGE_LOCAL;
                        s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
                        s_thaisen_transaction[gunno].ammeter_stop = s_thaisen_transaction[gunno].ammeter_start;

                        /** 此处不再次保存订单，由时间同步修正是统一再次保存，目前程序，启动时都会校时一次，如果不校时则需要在此处保存一次 */
                        break;
                    }
                    rt_thread_mdelay(50);
                }
            }
        }
#endif /* APP_USING_DOUBLEGUN */
        s_ofsm_info[gunno].base.flag.paracharge_is_identified = APP_THA_ENUM_TRUE;
    }

    /** 时间已进行同步，需要修改订单和BASE数据中与时间有关字段，并重新保存一次订单 */
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

        s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
        s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

        s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

        app_nsal_time_sync_revise(gunno);

        mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                0x00, APP_THA_ENUM_FALSE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
        /** 与时段有关的信息也要更新 */
        LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
    }

    /******************************[并充部分,并充启动时副枪不能往下执行,副枪靠主枪状态跳转]******************************/
    /******************************[并充部分,并充启动时副枪不能往下执行,副枪靠主枪状态跳转]******************************/
    if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) || (s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)) && \
            (s_ofsm_info[gunno].base.main_gunno != gunno)){
        /** 副枪有故障导致整机停止 */
        if(s_ofsm_info[gunno].base.flag.is_deputygun_stop == APP_THA_ENUM_FALSE){
            if(system_fault != APP_SYS_FAULT_NO_ERROR){
                s_ofsm_info[gunno].base.system_fault = system_fault;
                s_ofsm_info[gunno].base.charge_fault = charge_fault;

                s_thaisen_transaction[gunno].stop_reason = mw_system_stop_way_convert(system_fault);
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
                s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
                s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                    s_ofsm_info[i].base.flag.is_deputygun_stop = APP_THA_ENUM_TRUE;
                }
                LOG_D("deputy gunno is fault in parallel charge mode(%d, %d)", gunno, system_fault);
            }

            /******************************[启动阶段,判断主机柜是否允许充电]******************************/
            /******************************[启动阶段,判断主机柜是否允许充电]******************************/
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
                    s_ofsm_info[gunno].base.system_fault = APP_SYS_FAULT_NO_ERROR;
                    s_ofsm_info[gunno].base.charge_fault = APP_CHARGE_FAULT_NO_ERROR;

                    LOG_D("gunno(%d) charge stop deal to stop way(main cabinet forbid)|%d\n", gunno, APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID);

                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                    s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
                    s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
                    s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        s_ofsm_info[i].base.flag.is_deputygun_stop = APP_THA_ENUM_TRUE;
                    }
                    LOG_D("deputy gunno is fault in parallel charge mode(%d, %d)", gunno, system_fault);
                }
            }
        }
    }

    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) || (s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
        /** 主枪停止，副枪获取主枪信息，副枪根据主枪状态跳转 */
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

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

                    s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                    s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                    s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                    /** 与时段有关的信息也要更新 */
                    LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
                }

                if(s_ofsm_info[gunno].state == APP_OFSM_STATE_CHARGING){
                    s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_TRUE;
                    s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
                    s_thaisen_transaction[gunno].order_state.is_start_fail = APP_THA_ENUM_FALSE;

                    s_ofsm_info[gunno].timing_tick = rt_tick_get();
                    s_ofsm_info[gunno].base.offline_tick = rt_tick_get();
                }else{
                    /** 如果不是副枪故障导致的停机，则副枪需要获取主枪的信息 */
                    if(s_ofsm_info[gunno].base.flag.is_deputygun_stop != APP_THA_ENUM_TRUE){
                        s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                        s_ofsm_info[gunno].base.system_fault = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.system_fault;
                        s_ofsm_info[gunno].base.charge_fault = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.charge_fault;

                        s_thaisen_transaction[gunno].stop_reason = s_thaisen_transaction[s_ofsm_info[gunno].base.main_gunno].stop_reason;
                        s_ofsm_info[gunno].base.reason_code = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code;
                        s_ofsm_info[gunno].base.flag.is_fault_stop = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.flag.is_fault_stop;

                        s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
                        s_thaisen_transaction[gunno].boot_result = s_thaisen_transaction[s_ofsm_info[gunno].base.main_gunno].boot_result;

                        LOG_D("gunno(%d) charge finish deal main gun stop in parallel charge mode\n", gunno);
                    }
                }

                app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

                app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                        s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);

                return;
            }
        }
        if(gunno != s_ofsm_info[gunno].base.main_gunno){
            s_ofsm_info[gunno].base.voltage_a = mw_get_meter_ua(gunno) *10;
            return;
        }
    }

    /******************************[启动阶段,判断主机柜是否允许充电]******************************/
    /******************************[启动阶段,判断主机柜是否允许充电]******************************/
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

#ifdef APP_INCLUDE_YKC17_PROTOCOL
            thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

            if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                }
                s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
            }else{
                s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
            }

            LOG_D("gunno(%d) charge stop deal to stop way(main cabinet forbid)|%d\n", gunno, APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID);

            s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID;
            s_ofsm_info[gunno].base.reason_code = APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID;

            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

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

                s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                app_nsal_time_sync_revise(gunno);

                /** 与时段有关的信息也要更新 */
                LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
            }

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

            app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
            return;
        }else{
            if(s_ofsm_info[gunno].base.flag.is_starting == APP_THA_ENUM_FALSE){
                s_ofsm_info[gunno].base.flag.is_starting = APP_THA_ENUM_TRUE;
                /* 开始充电 */
                if((s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_LOCAL) || (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
                    mw_charge_start_cmd(gunno);
                }else{
                    if(s_ofsm_info[gunno].base.main_gunno == gunno){
                        mw_charge_start_cmd(gunno);
                    }
                }
            }
        }
    }

    /******************************[余额不足判断]******************************/
    /******************************[余额不足判断]******************************/
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

#ifdef APP_INCLUDE_YKC17_PROTOCOL
                thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

                if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                    uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                    if(gunno == APP_SYSTEM_GUNNOA){
                        deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                    }
                    s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                }else{
                    s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
                }

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

                    s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                    s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                    s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                    /** 与时段有关的信息也要更新 */
                    LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
                }

                app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

                app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                        s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

                app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);
                return;
            }
        }
    }

    if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||  \
            ((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (s_ofsm_info[gunno].base.main_gunno == gunno)) || \
            ((s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_LOCAL))){
        if(s_ofsm_info[gunno].charge_timeout > rt_tick_get()){
            s_ofsm_info[gunno].charge_timeout = rt_tick_get();
        }
        if((rt_tick_get() - s_ofsm_info[gunno].charge_timeout) > 50000){
            stop_way = mw_get_system_stop_way(gunno);
            if(((rt_tick_get() - s_ofsm_info[gunno].charge_timeout) >= 90000) || (stop_way != APP_SYSTEM_STOP_WAY_NULL)){
                if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
                    uint8_t rentry = 0x00;
                    while((rentry <= 100) && (thaisen_is_countdown_finish(gunno) == APP_THA_ENUM_FALSE)){  /** 最多等待 60s，等下层赋值完停充原因 */
                        rt_thread_mdelay(600);
                        stop_way = mw_get_system_stop_way(gunno);
                        if(stop_way != APP_SYSTEM_STOP_WAY_NULL){
                            break;
                        }
                        rentry++;
                    }
                }
                /** 注：启动超时时间计算如此：
                                             *  屏幕上的倒计时时长为90s，但是计时用的是线程定时，有一定误差，实际测试屏幕的90s大概为实际的145s(2分24s48)，为了使业务状态跳转与屏幕倒计时一致，实际倒计时时间必须大于145s，但不能相差过大，故定150s = 90s + ((100 *600) /1000)s
                 */
                /** 防止状态异常，上层已停止但下层还在充电 */
                mw_charge_stop_cmd(gunno);

                LOG_D("gunno(%d) charge stop deal to stop way(boot timeout)|%d\n", gunno, stop_way);

                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
                s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

                s_ofsm_info[gunno].base.system_fault = system_fault;
                s_ofsm_info[gunno].base.charge_fault = charge_fault;
                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
                thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

                if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                    uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                    if(gunno == APP_SYSTEM_GUNNOA){
                        deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                    }
                    s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                }else{
                    s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
                }

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

                    s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                    s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                    s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                    /** 与时段有关的信息也要更新 */
                    LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
                }

                app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

                app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                        s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

                app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);
                return;
            }
        }
    }

    switch(charge_state){
    case APP_CHARGE_STATE_IDLE:
        s_booting_step[gunno] = APP_BOOTING_STEP_IDLE;
        break;
    case APP_CHARGE_STATE_SHAKE_HAND:
        s_booting_step[gunno] = APP_BOOTING_STEP_HAND;
        break;
    case APP_CHARGE_STATE_INSULATION:
        if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                    app_nsal_report_bms_message_bmsinfo(i);
                }
            }else{
                app_nsal_report_bms_message_bmsinfo(gunno);
            }
        }
        s_booting_step[gunno] = APP_BOOTING_STEP_INSULT;
        break;
    case APP_CHARGE_STATE_CONFIGURE:
        if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            if(s_booting_step[gunno] <= APP_BOOTING_STEP_HAND){
                if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        app_nsal_report_bms_message_bmsinfo(i);
                    }
                }else{
                    app_nsal_report_bms_message_bmsinfo(gunno);
                }
            }
        }
        s_booting_step[gunno] = APP_BOOTING_STEP_CONFIG;
        break;
    case APP_CHARGE_STATE_CHARGING:
    {
        if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            switch(s_booting_step[gunno]){
            case APP_BOOTING_STEP_IDLE:
            case APP_BOOTING_STEP_HAND:
                if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        app_nsal_report_bms_message_bmsinfo(i);
                        app_nsal_report_bms_message_bmsparameter(i);
                    }
                }else{
                    app_nsal_report_bms_message_bmsinfo(gunno);
                    app_nsal_report_bms_message_bmsparameter(gunno);
                }
                break;
            case APP_BOOTING_STEP_INSULT:
            case APP_BOOTING_STEP_CONFIG:
                if(s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_VIN){
                    if(s_ofsm_info[gunno].base.flag.vin_is_authorized == APP_THA_ENUM_FALSE){
                        if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                            for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                                app_nsal_report_bms_message_bmsparameter(i);
                            }
                        }else{
                            app_nsal_report_bms_message_bmsparameter(gunno);
                        }
                    }
                }else{
                    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                        for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                            app_nsal_report_bms_message_bmsparameter(i);
                        }
                    }else{
                        app_nsal_report_bms_message_bmsparameter(gunno);
                    }
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
                    s_thaisen_transaction[gunno].account_ballance_before = s_ofsm_info[gunno].base.account_ballance_before;
                    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

                    s_thaisen_transaction[gunno].order_state.online_order = APP_THA_ENUM_TRUE;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
                    defined (APP_INCLUDE_SGCC_PROTOCOL))
                    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                        s_thaisen_transaction[gunno].rate_type_unit[type] = app_billingrule_get_rate_type_price(gunno, type);
                    }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
    defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
                    memcpy(s_thaisen_transaction[gunno].elect_model_sn, app_billingrule_get_elect_model_sn(gunno), APP_BILLING_MODEL_SN_LEN);
                    memcpy(s_thaisen_transaction[gunno].service_model_sn, app_billingrule_get_service_model_sn(gunno), APP_BILLING_MODEL_SN_LEN);
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

                    app_nsal_clear_remote_vin_authorize(gunno);
                    app_nsal_init_charge_data(gunno);  /* 重新赋值流水号 */
                    vin_authentication_complete = APP_THA_ENUM_TRUE;

                    mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                            0x00, APP_THA_ENUM_FALSE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
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

#ifdef APP_INCLUDE_YKC17_PROTOCOL
                thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

                if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                    uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                    if(gunno == APP_SYSTEM_GUNNOA){
                        deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                    }
                    s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
                }else{
                    s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
                }

                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_AUTHEN_FAIL;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;

                LOG_D("gunno(%d) charge stop deal to vin authorization fail", gunno);

                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
                s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
                s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
                s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

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

                    s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                    s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                    s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                    LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
                }

                app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

                app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                        s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

                app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);
                return;
            }

            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_TRUE;
            s_thaisen_transaction[gunno].boot_result = s_ofsm_info[gunno].base.flag.start_result;
            s_thaisen_transaction[gunno].order_state.is_start_fail = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].timing_tick = rt_tick_get();
            s_ofsm_info[gunno].base.offline_tick = rt_tick_get();
            s_ofsm_info[gunno].elect_calculate_tick = rt_tick_get();

            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_CHARGING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_CHARGING;

            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);
            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
        }
        return;
    }
    case APP_CHARGE_STATE_FINISH:
        break;
    case APP_CHARGE_STATE_FAULTING:
    case APP_CHARGE_STATE_WAIT_PULL_GUN:
    {
        if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.system_fault = system_fault;
            s_ofsm_info[gunno].base.charge_fault = charge_fault;
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
            thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

            if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                }
                s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
            }else{
                s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
            }

            stop_way = mw_get_system_stop_way(gunno);

            /** 防止状态异常，上层已停止但下层还在充电 */
            mw_charge_stop_cmd(gunno);
            if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
                uint8_t rentry = 0x00;
                while(rentry <= 20){  /** 最多等待 10s，等下层赋值完停充原因 */
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

                s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                /** 与时段有关的信息也要更新 */
                LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
            }

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

            app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
        }
        return;
    }
    default:
        break;
    }

    /******************************[并充部分,这是副枪故障导致的主枪停止,主枪要获取副枪的信息]******************************/
    /******************************[并充部分,这是副枪故障导致的主枪停止,主枪要获取副枪的信息]******************************/
    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) || (s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
        if((s_ofsm_info[gunno].base.flag.is_deputygun_stop == APP_THA_ENUM_TRUE) && (s_ofsm_info[gunno].base.main_gunno == gunno)){
            uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
            if(gunno == APP_SYSTEM_GUNNOA){
                deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
            }
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
            thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

            mw_charge_stop_cmd(gunno);

            s_ofsm_info[gunno].base.system_fault = s_ofsm_info[deputy_gunno].base.system_fault;
            s_ofsm_info[gunno].base.charge_fault = s_ofsm_info[deputy_gunno].base.charge_fault;

            s_thaisen_transaction[gunno].stop_reason = s_thaisen_transaction[deputy_gunno].stop_reason;
            s_ofsm_info[gunno].base.reason_code = s_ofsm_info[deputy_gunno].base.reason_code;

            s_ofsm_info[gunno].base.flag.is_fault_stop = s_ofsm_info[deputy_gunno].base.flag.is_fault_stop;
            s_ofsm_info[gunno].base.flag.start_result = APP_THA_ENUM_FALSE;
            s_thaisen_transaction[gunno].boot_result = s_thaisen_transaction[deputy_gunno].boot_result;
            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

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

                s_ofsm_info[gunno].base.start_period = app_calculate_current_period(s_ofsm_info[gunno].base.start_time);
                s_ofsm_info[gunno].base.current_period = s_ofsm_info[gunno].base.start_period;

                s_thaisen_transaction[gunno].start_period_number = s_ofsm_info[gunno].base.start_period;

                /** 与时段有关的信息也要更新 */
                LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
            }

            app_billing_info_init(s_ofsm_info[gunno].base.start_elect, gunno);

            app_nsal_report_remote_start_result(gunno, s_ofsm_info[gunno].base.flag.start_result,  \
                    s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

            app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);

            LOG_D("gunno(%d) charge finish deal deputy gun fault stop in parallel charge mode\n", gunno);
            return;
        }
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
    rfidr_clear_swipe_state(gunno);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

    app_nsal_clear_remote_start(gunno);
    app_nsal_clear_remote_card_authorize(gunno);
    app_nsal_clear_remote_vin_authorize(gunno);

    app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    app_card_event_recv(APP_CARD_EVENT_CHARGE_START, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    if(s_ofsm_info[gunno].base.reason_code != APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP){
        app_card_event_recv(APP_CARD_EVENT_PAY_COMPLETE, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    }
}
/*****************************************************
 * 函数名          ofsm_charging_fun
 * 功能               业务状态机 充电中 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
static void ofsm_charging_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    extern uint32_t thaisen_get_module_curr(uint8_t gunNum);
    uint32_t current_tick = rt_tick_get(), increase_sec = 0, bms_ccs_curr = 4000;
    bool is_stop_charge_authorization = APP_THA_ENUM_FALSE;  /* 停充已授权 */
    enum system_stop_way stop_way = APP_SYSTEM_STOP_WAY_SIZE;
    enum charge_state_t charge_state = mw_get_charge_state(gunno);
    enum system_fault_t system_fault = app_get_highest_priority_system_fault(gunno);
    enum charge_fault_t charge_fault = app_get_highest_priority_charge_fault(gunno);
    struct thaisenBMS_Charger_struct* bms_info = (struct thaisenBMS_Charger_struct*)(s_ofsm_info[gunno].base.bms_data);

    if (++s_debug_count[gunno] > (3000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) charging state (%dV | S%d | E|%u, F|%u V|%d C|%d P|%d charge_time(%d))...", gunno, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state,
                s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.fees_total, s_ofsm_info[gunno].base.voltage_a,
                s_ofsm_info[gunno].base.current_a, s_ofsm_info[gunno].base.power_a, s_ofsm_info[gunno].base.charge_time);
    }

    /************************************************************************************************************/
    /** 这是本地选择并充的场景 */
    /************************************************************************************************************/
    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (s_ofsm_info[gunno].base.main_gunno != gunno)){
        /** 副枪有故障导致整机停止 */
        if(s_ofsm_info[gunno].base.flag.is_deputygun_stop == APP_THA_ENUM_FALSE){
            if(system_fault != APP_SYS_FAULT_NO_ERROR){
                s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                s_ofsm_info[gunno].base.system_fault = system_fault;
                s_ofsm_info[gunno].base.charge_fault = charge_fault;

                s_thaisen_transaction[gunno].stop_reason = mw_system_stop_way_convert(system_fault);
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                    s_ofsm_info[i].base.flag.is_deputygun_stop = APP_THA_ENUM_TRUE;
                }
            }
        }
    }
    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        /** 主枪停止，副枪获取主枪信息，副枪根据主枪状态跳转 */
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

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

                /** 如果不是副枪故障导致的停机，则副枪需要获取主枪的信息 */
                if(s_ofsm_info[gunno].base.flag.is_deputygun_stop != APP_THA_ENUM_TRUE){
                    s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                    s_ofsm_info[gunno].base.system_fault = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.system_fault;
                    s_ofsm_info[gunno].base.charge_fault = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.charge_fault;

                    s_thaisen_transaction[gunno].stop_reason = s_thaisen_transaction[s_ofsm_info[gunno].base.main_gunno].stop_reason;
                    s_ofsm_info[gunno].base.reason_code = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.flag.is_fault_stop;

                    LOG_D("gunno(%d) charge finish deal main gun stop in parallel charge mode\n", gunno);
                }

                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);

                return;
            }
        }
        if(gunno != s_ofsm_info[gunno].base.main_gunno){
            s_ofsm_info[gunno].base.voltage_a = mw_get_meter_ua(gunno) *10;
            return;
        }
    }
    /************************************************************************************************************/
    /************************************************************************************************************/

    /************************************************************************************************************/
    /** 这是云端选择并充的场景 */
    /************************************************************************************************************/
    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        /** 主枪停止，副枪获取主枪信息，副枪根据主枪状态跳转 */
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

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

                /** 如果不是副枪故障导致的停机，则副枪需要获取主枪的信息 */
                if(s_ofsm_info[gunno].base.flag.is_deputygun_stop != APP_THA_ENUM_TRUE){
                    s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                    s_ofsm_info[gunno].base.system_fault = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.system_fault;
                    s_ofsm_info[gunno].base.charge_fault = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.charge_fault;

                    s_thaisen_transaction[gunno].stop_reason = s_thaisen_transaction[s_ofsm_info[gunno].base.main_gunno].stop_reason;
                    s_ofsm_info[gunno].base.reason_code = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.reason_code;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.flag.is_fault_stop;

                    LOG_D("gunno(%d) charge finish deal main gun stop in parallel charge mode\n", gunno);
                }

                app_nsal_state_charged(gunno);
                app_nsal_event_occurded(gunno);

                return;
            }
        }
    }
    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno != gunno)){
        /** 副枪有故障导致整机停止 */
        if(s_ofsm_info[gunno].base.flag.is_deputygun_stop == APP_THA_ENUM_FALSE){
            if(system_fault != APP_SYS_FAULT_NO_ERROR){
                s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

                s_ofsm_info[gunno].base.system_fault = system_fault;
                s_ofsm_info[gunno].base.charge_fault = charge_fault;

                s_thaisen_transaction[gunno].stop_reason = mw_system_stop_way_convert(system_fault);
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                    s_ofsm_info[i].base.flag.is_deputygun_stop = APP_THA_ENUM_TRUE;
                }
                return;
            }
        }else{
            return;
        }

        charge_state = mw_get_charge_state(s_ofsm_info[gunno].base.main_gunno);
        bms_info = (struct thaisenBMS_Charger_struct*)(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].base.bms_data);
    }
    /************************************************************************************************************/
    /************************************************************************************************************/

    if(s_ofsm_info[gunno].timing_tick > current_tick){
        increase_sec = ((current_tick + 0xFFFFFFFF) - s_ofsm_info[gunno].timing_tick) /1000;
    }else{
        increase_sec = (current_tick - s_ofsm_info[gunno].timing_tick) /1000;
    }

    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
        uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
        if(gunno == APP_SYSTEM_GUNNOA){
            deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
        }

        /** 按600KW算，一小时600度，一分钟10度，一秒钟 1/6 度，三位小数，大概是：166， 约等于170， 每次计算两次之间的差值不能大于一定值 */
        if(s_ofsm_info[gunno].elect_calculate_tick > current_tick){
            if((current_tick + 0xFFFFFFFF - s_ofsm_info[gunno].elect_calculate_tick) > 1000){  /** 一秒计算一次 */
                uint32_t _elect = mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno);
                if((_elect <= (s_ofsm_info[gunno].base.charge_elect_last + APP_CALCULATE_ELECT_DIFF_MAX)) && (_elect > s_ofsm_info[gunno].base.charge_elect_last)){ /** 按三秒的电量来算 */
                    app_billing_info_calculate((s_ofsm_info[gunno].base.start_time + increase_sec), _elect, (_elect - s_ofsm_info[gunno].base.charge_elect_last), gunno);
                }
                s_ofsm_info[gunno].elect_calculate_tick = current_tick;
                s_ofsm_info[gunno].base.charge_elect_last = _elect;
            }
        }else{
            if((current_tick - s_ofsm_info[gunno].elect_calculate_tick) > 1000){  /** 一秒计算一次 */
                uint32_t _elect = mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno);
                if((_elect <= (s_ofsm_info[gunno].base.charge_elect_last + APP_CALCULATE_ELECT_DIFF_MAX)) && (_elect > s_ofsm_info[gunno].base.charge_elect_last)){
                    app_billing_info_calculate((s_ofsm_info[gunno].base.start_time + increase_sec), _elect, (_elect - s_ofsm_info[gunno].base.charge_elect_last), gunno);
                }
                s_ofsm_info[gunno].elect_calculate_tick = current_tick;
                s_ofsm_info[gunno].base.charge_elect_last = _elect;
            }
        }

        s_ofsm_info[gunno].base.current_a = (mw_get_meter_ia(gunno) *10 + mw_get_meter_ia(deputy_gunno) *10);
        s_ofsm_info[gunno].base.power_a = (mw_get_meter_pa(gunno) + mw_get_meter_pa(deputy_gunno));
    }else{
        /** 按600KW算，一小时600度，一分钟10度，一秒钟 1/6 度，三位小数，大概是：166， 约等于170， 每次计算两次之间的差值不能大于一定值 */
        if(s_ofsm_info[gunno].elect_calculate_tick > current_tick){
            if((current_tick + 0xFFFFFFFF - s_ofsm_info[gunno].elect_calculate_tick) > 1000){  /** 一秒计算一次 */
                uint32_t _elect = mw_get_meter_total_wh(gunno);
                if((_elect <= (s_ofsm_info[gunno].base.charge_elect_last + APP_CALCULATE_ELECT_DIFF_MAX)) && (_elect > s_ofsm_info[gunno].base.charge_elect_last)){ /** 按三秒的电量来算 */
                    app_billing_info_calculate((s_ofsm_info[gunno].base.start_time + increase_sec), _elect, (_elect - s_ofsm_info[gunno].base.charge_elect_last), gunno);
                }
                s_ofsm_info[gunno].elect_calculate_tick = current_tick;
                s_ofsm_info[gunno].base.charge_elect_last = _elect;
            }
        }else{
            if((current_tick - s_ofsm_info[gunno].elect_calculate_tick) > 1000){  /** 一秒计算一次 */
                uint32_t _elect = mw_get_meter_total_wh(gunno);
                if((_elect <= (s_ofsm_info[gunno].base.charge_elect_last + APP_CALCULATE_ELECT_DIFF_MAX)) && (_elect > s_ofsm_info[gunno].base.charge_elect_last)){
                    app_billing_info_calculate((s_ofsm_info[gunno].base.start_time + increase_sec), _elect, (_elect - s_ofsm_info[gunno].base.charge_elect_last), gunno);
                }
                s_ofsm_info[gunno].elect_calculate_tick = current_tick;
                s_ofsm_info[gunno].base.charge_elect_last = _elect;
            }
        }

        s_ofsm_info[gunno].base.current_a = mw_get_meter_ia(gunno) *10;
        s_ofsm_info[gunno].base.power_a = mw_get_meter_pa(gunno);
    }

    s_ofsm_info[gunno].base.voltage_a = mw_get_meter_ua(gunno) *10;
    s_ofsm_info[gunno].base.current_soc = bms_info->BCS.SOC;
    s_ofsm_info[gunno].base.stop_time = (s_ofsm_info[gunno].base.start_time + increase_sec);
    s_ofsm_info[gunno].base.charge_time = s_ofsm_info[gunno].base.stop_time - s_ofsm_info[gunno].base.start_time;
    s_ofsm_info[gunno].base.start_elect = app_billingrule_get_start_elcet(gunno);
    s_ofsm_info[gunno].base.current_elect = app_billingrule_get_stop_elcet(gunno);
    s_ofsm_info[gunno].base.fees_total = app_billingrule_get_fees_total(gunno);
    s_ofsm_info[gunno].base.service_fees_total = app_billingrule_get_service_fees_total(gunno);

    if(s_ofsm_info[gunno].base.fees_total > APP_SPEND_AMOUNT_MAX){
        s_ofsm_info[gunno].base.fees_total = APP_SPEND_AMOUNT_MAX;
    }
    if(s_ofsm_info[gunno].base.service_fees_total > APP_SPEND_AMOUNT_MAX){
        s_ofsm_info[gunno].base.service_fees_total = APP_SPEND_AMOUNT_MAX;
    }
    if(s_ofsm_info[gunno].base.fees_total > s_ofsm_info[gunno].base.service_fees_total){
        s_ofsm_info[gunno].base.elect_fees_total = (s_ofsm_info[gunno].base.fees_total - s_ofsm_info[gunno].base.service_fees_total);
        if(s_ofsm_info[gunno].base.elect_fees_total > APP_SPEND_AMOUNT_MAX){
            s_ofsm_info[gunno].base.elect_fees_total = APP_SPEND_AMOUNT_MAX;
        }
    }
    if(s_ofsm_info[gunno].base.account_ballance_before > (s_ofsm_info[gunno].base.fees_total /100)){
        s_ofsm_info[gunno].base.account_ballance_after = s_ofsm_info[gunno].base.account_ballance_before - (s_ofsm_info[gunno].base.fees_total /100);
    }
    s_ofsm_info[gunno].base.elect_a = app_billingrule_get_elcet_total(gunno);
    if(s_ofsm_info[gunno].base.elect_a > APP_CHARGE_ELECT_MAX){
        s_ofsm_info[gunno].base.elect_a = APP_CHARGE_ELECT_MAX;
    }
    if(s_ofsm_info[gunno].base.current_period != app_calculate_current_period((s_ofsm_info[gunno].base.start_time + increase_sec))){
        s_ofsm_info[gunno].base.current_period = app_calculate_current_period((s_ofsm_info[gunno].base.start_time + increase_sec));
        if(s_ofsm_info[gunno].base.period_num < APP_BILLING_RULE_PERIOD_MAX){
            s_ofsm_info[gunno].base.period_num++;
        }
    }

    s_thaisen_transaction[gunno].end_time = s_ofsm_info[gunno].base.stop_time;
    s_thaisen_transaction[gunno].charge_time = s_ofsm_info[gunno].base.charge_time;
    s_thaisen_transaction[gunno].stop_soc = s_ofsm_info[gunno].base.current_soc;
    s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
    s_thaisen_transaction[gunno].ammeter_stop = s_ofsm_info[gunno].base.current_elect;
    s_thaisen_transaction[gunno].total_elect = s_ofsm_info[gunno].base.elect_a;
    s_thaisen_transaction[gunno].total_loss_elect = 0x00;;
    s_thaisen_transaction[gunno].charge_fee = s_ofsm_info[gunno].base.elect_fees_total;
    s_thaisen_transaction[gunno].service_fee = s_ofsm_info[gunno].base.service_fees_total;
    s_thaisen_transaction[gunno].total_fee = s_ofsm_info[gunno].base.fees_total;
    s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
    defined (APP_INCLUDE_SGCC_PROTOCOL))
    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
        s_thaisen_transaction[gunno].rate_type_elect[type] = app_billingrule_get_rate_type_elect(gunno, type);
        s_thaisen_transaction[gunno].rate_type_amount[type] = app_billingrule_get_rate_type_fess(gunno, type);
        s_thaisen_transaction[gunno].rate_type_loss_elect[type] = 0x00;

        if(s_thaisen_transaction[gunno].rate_type_amount[type] > APP_SPEND_AMOUNT_MAX){
            s_thaisen_transaction[gunno].rate_type_amount[type] = APP_SPEND_AMOUNT_MAX;
        }
        if(s_thaisen_transaction[gunno].rate_type_elect[type] > APP_CHARGE_ELECT_MAX){
            s_thaisen_transaction[gunno].rate_type_elect[type] = APP_CHARGE_ELECT_MAX;
        }
    }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
    defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
    for(uint8_t period = s_ofsm_info[gunno].base.start_period; period <= s_ofsm_info[gunno].base.current_period; period++){
        s_thaisen_transaction[gunno].period_elect[period] = app_billingrule_get_period_elect(gunno, period);
        s_thaisen_transaction[gunno].period_elect_fees[period] = app_billingrule_get_period_elect_fees(gunno, period);
        s_thaisen_transaction[gunno].period_service_fees[period] = app_billingrule_get_period_service_fees(gunno, period);

        if(s_thaisen_transaction[gunno].period_elect_fees[period] > APP_SPEND_AMOUNT_MAX){
            s_thaisen_transaction[gunno].period_elect_fees[period] = APP_SPEND_AMOUNT_MAX;
        }
        if(s_thaisen_transaction[gunno].period_service_fees[period] > APP_SPEND_AMOUNT_MAX){
            s_thaisen_transaction[gunno].period_service_fees[period] = APP_SPEND_AMOUNT_MAX;
        }
        if(s_thaisen_transaction[gunno].period_elect[period] > APP_CHARGE_ELECT_MAX){
            s_thaisen_transaction[gunno].period_elect[period] = APP_CHARGE_ELECT_MAX;
        }
    }
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */
    s_thaisen_transaction[gunno].period_count = s_ofsm_info[gunno].base.period_num;

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
    for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
        s_thaisen_transaction[gunno].rate_type_elect_amount[type] = app_billingrule_get_rate_type_elect_fess(gunno, type);
        s_thaisen_transaction[gunno].rate_type_service_amount[type] = app_billingrule_get_rate_type_service_fess(gunno, type);

        if(s_thaisen_transaction[gunno].rate_type_elect_amount[type] > APP_SPEND_AMOUNT_MAX){
            s_thaisen_transaction[gunno].rate_type_elect_amount[type] = APP_SPEND_AMOUNT_MAX;
        }
        if(s_thaisen_transaction[gunno].rate_type_service_amount[type] > APP_SPEND_AMOUNT_MAX){
            s_thaisen_transaction[gunno].rate_type_service_amount[type] = APP_SPEND_AMOUNT_MAX;
        }
    }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */

    /** 电表开始电量不对, 要重新赋值 */
    if(s_ofsm_info[gunno].base.flag.is_ammeter_elect_error == APP_THA_ENUM_TRUE){
        if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
            uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
            if(gunno == APP_SYSTEM_GUNNOA){
                deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
            }
            if((mw_get_meter_total_wh(gunno) != 0x00) && (mw_get_meter_total_wh(deputy_gunno) != 0x00)){
                if(app_billingrule_get_start_elcet(gunno) != 0x00){
                    s_ofsm_info[gunno].base.start_elect = app_billingrule_get_start_elcet(gunno);
                    s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
                    s_ofsm_info[gunno].base.flag.is_ammeter_elect_error = APP_THA_ENUM_FALSE;
                }
            }
        }else{
            if(mw_get_meter_total_wh(gunno) != 0x00){
                if(app_billingrule_get_start_elcet(gunno) != 0x00){
                    s_ofsm_info[gunno].base.start_elect = app_billingrule_get_start_elcet(gunno);
                    s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
                    s_ofsm_info[gunno].base.flag.is_ammeter_elect_error = APP_THA_ENUM_FALSE;
                }
            }
        }
        /** 电表开始电量修改后要重新保存 */
        if(s_ofsm_info[gunno].base.flag.is_ammeter_elect_error == APP_THA_ENUM_FALSE){
            mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                    0x00, APP_THA_ENUM_FALSE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
        }
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
#if 0
        mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_STORAGE,  \
                0x00, APP_THA_ENUM_FALSE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
#endif /* 0 */
        /** 与时段有关的信息也要更新 */
        LOG_I("chargepile is synchronized, modify correlation time|%x\n", curr_time);
    }

    app_nsal_padding_charge_data(gunno);
    app_nsal_report_bms_message_bmsrequire_pileoutput(gunno, 0x00);
    app_nsal_report_bms_message_bmsstate(gunno, 0x00);

    switch (charge_state){
    case APP_CHARGE_STATE_IDLE:
    {
        if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.system_fault = system_fault;
            s_ofsm_info[gunno].base.charge_fault = charge_fault;
            s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
            thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

            if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                }
                s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
            }else{
                s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
            }

            stop_way = mw_get_system_stop_way(gunno);

            /** 防止状态异常，上层已停止但下层还在充电 */
            mw_charge_stop_cmd(gunno);
            if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
                uint8_t rentry = 0x00;
                while(rentry <= 20){  /** 最多等待 10s，等下层赋值完停充原因 */
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

            app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

            app_nsal_report_remote_stop_result(gunno, APP_THA_ENUM_TRUE, s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
            return;
        }
        break;
    }
    case APP_CHARGE_STATE_CHARGING:
        break;
    case APP_CHARGE_STATE_FAULTING:
    case APP_CHARGE_STATE_WAIT_PULL_GUN:
    {
        if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.system_fault = system_fault;
            s_ofsm_info[gunno].base.charge_fault = charge_fault;
            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
            thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

            if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                }
                s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
            }else{
                s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
            }

            stop_way = mw_get_system_stop_way(gunno);

            /** 防止状态异常，上层已停止但下层还在充电 */
            mw_charge_stop_cmd(gunno);
            if(stop_way == APP_SYSTEM_STOP_WAY_NULL){
                uint8_t rentry = 0x00;
                while(rentry <= 20){  /** 最多等待 10s，等下层赋值完停充原因 */
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

            app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

            app_nsal_report_remote_stop_result(gunno, APP_THA_ENUM_TRUE, s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);
            return;
        }
        break;
    }
    default:
        break;
    }

    if(app_nsal_is_remote_stop(gunno)) {
        s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_APP_STOP;
        s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
        s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

        is_stop_charge_authorization = true;
        LOG_D("gunno(%d) charge finish deal to APP stop\n", gunno);

    }else if(rfidr_query_swipe_state(gunno)){
        rfidr_clear_swipe_state(gunno);
        if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_CARD_NUMBER){
            uint8_t compare_len = sizeof(s_ofsm_info[gunno].base.card_number), compare_count = APP_CARD_NUMBER_COMPARE_LEN_MIN;
            compare_len = compare_len > rfidr_query_card_number_len() ? rfidr_query_card_number_len() : compare_len;

            if((s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING) && (s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD)){
                compare_count = APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING;
            }

            if(compare_len < compare_count){
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                LOG_W("gunno(%d) card number length is not enough|%d, %d", gunno, compare_count, compare_len);
                return;
            }
            if(memcmp(s_ofsm_info[gunno].base.card_number, rfidr_query_card_number(), compare_count)){
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                LOG_W("gunno(%d) card number is not match|%s, %s", gunno, s_ofsm_info[gunno].base.card_number, rfidr_query_card_number());
                return;
            }

            compare_count = sizeof(s_ofsm_info[gunno].base.card_uid);
            compare_count = compare_count > rfidr_query_uuid_len() ? rfidr_query_uuid_len() : compare_count;
            if(memcmp(s_ofsm_info[gunno].base.card_uid, rfidr_query_uuid(), compare_count)){
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                LOG_W("gunno(%d) card uuid is not match", gunno);
                return;
            }
            app_rfidr_send_mail(APP_BUZZON_STATE_OK);

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

        }else if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_UUID){
            uint8_t* uid = rfidr_query_uuid(), valid_len = sizeof(s_ofsm_info[gunno].base.card_uid);

            if((s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING) && (s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD)){
                /** 提示鉴权失败, 离线计费获取到的卡信息类型必须是卡号 */;
                LOG_D("gunno(%d) invalid card in offline billing mode, key authen fail", gunno);
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                thaisen_set_trigger_event(THAISEN_TRIG_EVENT_KEY_AUTHEN_FAIL, 0x05, APP_THA_ENUM_TRUE, gunno);
            }else{
                if(uid){
                    valid_len = valid_len > rfidr_query_uuid_len() ? rfidr_query_uuid_len() : valid_len;
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
                        app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                        LOG_W("this is not the card of start charge on this gun(%d)", gunno);
                    }
                }else{
                    LOG_W("gunno error when get card uid in ready state");
                }
            }
        }else if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_ERROR){
            /** 提示：无效卡 */
            LOG_D("gunno(%d) invalid card in offline billing mode, read card number fail", gunno);
            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);

        }else if(rfidr_query_info_type() == APP_RFIDR_INFO_TYPE_RW_FAIL){
            switch(app_card_query_operate_ret(gunno)){
            case APP_CARD_OPERATE_RET_PAYED:
                break;
            case APP_CARD_OPERATE_RET_NOT_START_CARD:
                LOG_D("gunno(%d) not start card in offline billing mode", gunno);
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                thaisen_set_trigger_event(THAISEN_TRIG_EVENT_SWITCH_GUN, 0x05, APP_THA_ENUM_TRUE, gunno);
                break;
            case APP_CARD_OPERATE_RET_IS_CHARGING:
                LOG_D("gunno(%d) this card is start in offline billing mode", gunno);
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                thaisen_set_trigger_event(THAISEN_TRIG_EVENT_IS_CHARGING, 0x05, APP_THA_ENUM_TRUE, gunno);
                break;
            case APP_CARD_OPERATE_RET_NULL:
                LOG_D("gunno(%d) current state is not allow stop in offline billing mode", gunno);
                break;
            default:
                LOG_D("gunno(%d) invalid card in offline billing mode 333", gunno);
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);
                break;
            }

        }else{
            if((s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING) && (s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD)){
                /** 提示鉴权失败, 信息有误 */;
                LOG_D("gunno(%d) operate card error in offline billing mode 00", gunno);
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                thaisen_set_trigger_event(THAISEN_TRIG_EVENT_KEY_AUTHEN_FAIL, 0x05, APP_THA_ENUM_TRUE, gunno);
            }else{
                /** 信息有误, 蜂鸣器提示 */
                app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
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
                if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                        (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_REACH_TIME;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                    is_stop_charge_authorization = true;
                    LOG_D("gunno(%d) charge finish deal to reach target time(%d, %d)\n", gunno, increase_sec, s_ofsm_info[gunno].base.charge_strategy_para);
                }
            }
        }else if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_ELECT){
            uint32_t _elect = s_ofsm_info[gunno].base.elect_a;
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                uint8_t another_gun = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    another_gun = APP_SYSTEM_GUNNOA + 0x01;
                }
                _elect += s_ofsm_info[another_gun].base.elect_a;
            }
            if(_elect >= s_ofsm_info[gunno].base.charge_strategy_para){
                if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                        (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_REACH_ELECT;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                    is_stop_charge_authorization = true;
                    LOG_D("gunno(%d) charge finish deal to reach target elect(%d, %d)\n", gunno, _elect, s_ofsm_info[gunno].base.charge_strategy_para);
                }
            }
        }else if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_MONEY){
            uint32_t _money = s_ofsm_info[gunno].base.fees_total;
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                uint8_t another_gun = APP_SYSTEM_GUNNOA;
                if(gunno == APP_SYSTEM_GUNNOA){
                    another_gun = APP_SYSTEM_GUNNOA + 0x01;
                }
                _money += s_ofsm_info[another_gun].base.fees_total;
            }
            if((_money + 10000) > s_ofsm_info[gunno].base.charge_strategy_para){
                if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                        (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_NO_BALLANCE;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                    is_stop_charge_authorization = true;
                    LOG_D("gunno(%d) charge finish deal to reach target money(%d, %d)\n", gunno, s_ofsm_info[gunno].base.charge_strategy_para, _money);
                }
            }
        }else if(s_ofsm_info[gunno].base.charge_strategy == APP_CHARGE_STRATEGY_SOC){
            if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
                    (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
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
    }
#ifdef CP_CONFIG_USING_DUPU
    if(terminal_get_ems_set_power() < APP_EMS_STOP_POWER_MIN){   /** 0.5KW */
        s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_REACH_ELECT;
        s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
        s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

        is_stop_charge_authorization = true;
        LOG_D("gunno(%d) charge finish deal to reach target elect(ems)\n", gunno);
    }
#endif /* CP_CONFIG_USING_DUPU */
    if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
            (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
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
    }

    if(((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) && (s_ofsm_info[gunno].base.main_gunno == gunno)) ||
            (s_ofsm_info[gunno].base.charge_way != APP_CHARGE_WAY_PARACHARGE_CLOUD)){
        if(bms_info->BCS.SOC >= *(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0))){
            if(*(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0)) < 100){
                if(is_stop_charge_authorization == false){
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_SOC_LIMIT;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_FALSE;

                    is_stop_charge_authorization = true;
                    LOG_D("gunno(%d) charge finish deal to reach soc protect value(%d)\n", gunno, *(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0)));
                }
            }
        }
    }

#if 0
    if(bms_info->CCS.OutputCurlt <= 4000){
        bms_ccs_curr = (4000 - bms_info->CCS.OutputCurlt);    /** 目前国标电流偏移是 -400 */
    }

    if(s_ofsm_info[gunno].base.flag.bms_require_decrease == APP_THA_ENUM_FALSE){
        if((s_bms_require_curr_last[gunno]) > ((bms_info->BCL.BMSneedCurlt *10) + APP_CURR_COMPARE_CCSBCL_MAX)){
            s_bms_reqcurr_changed_count[gunno] = 0x00;
            s_bms_require_curr_last[gunno] = (bms_info->BCL.BMSneedCurlt *10);
            s_ofsm_info[gunno].base.flag.bms_require_decrease = APP_THA_ENUM_TRUE;
        }else{
            if(++s_bms_reqcurr_changed_count[gunno] > 10){  /** 防止1s内多次变化，每次变化都小于APP_CURR_COMPARE_CCSBCL_MAX */
                s_bms_reqcurr_changed_count[gunno] = 0x00;
                s_bms_require_curr_last[gunno] = (bms_info->BCL.BMSneedCurlt *10);
            }
        }
    }else{
        s_bms_reqcurr_changed_count[gunno] = 0x00;
        s_bms_require_curr_last[gunno] = (bms_info->BCL.BMSneedCurlt *10);
    }

    if(thaisen_get_module_curr(gunno) *10 > (bms_info->BCL.BMSneedCurlt *10 + APP_CURR_COMPARE_CCSBCL_MAX)){
        if(s_compare_module_bcl_count[gunno] > rt_tick_get()){
            s_compare_module_bcl_count[gunno] = rt_tick_get();
        }
        if(s_ofsm_info[gunno].base.flag.bms_require_decrease == APP_THA_ENUM_TRUE){
            if((rt_tick_get() - s_compare_module_bcl_count[gunno]) > 2 *APP_CURR_CCSBCL_ABNORMAL_TIMEOUT){
                if(is_stop_charge_authorization == false){
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                    is_stop_charge_authorization = true;
                    LOG_D("gunno(%d) charge finish deal to module BCL current abnormal(%d, %d)\n", gunno, thaisen_get_module_curr(gunno), bms_info->BCL.BMSneedCurlt);
                }
            }
        }else{
            if((rt_tick_get() - s_compare_module_bcl_count[gunno]) > APP_CURR_CCSBCL_ABNORMAL_TIMEOUT){
                if(is_stop_charge_authorization == false){
                    s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL;
                    s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                    s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                    is_stop_charge_authorization = true;
                    LOG_D("gunno(%d) charge finish deal to module BCL current abnormal(%d, %d)\n", gunno, thaisen_get_module_curr(gunno), bms_info->BCL.BMSneedCurlt);
                }
            }
        }
    }else{
        s_compare_module_bcl_count[gunno] = rt_tick_get();
        s_ofsm_info[gunno].base.flag.bms_require_decrease = APP_THA_ENUM_FALSE;
    }
#endif
#if 0
    if(abs(bms_ccs_curr *10 - thaisen_get_module_curr(gunno) *10) > APP_CURR_COMPARE_CCSBCS_MAX){
        if(s_compare_ccs_module_count[gunno] > rt_tick_get()){
            s_compare_ccs_module_count[gunno] = rt_tick_get();
        }
        if((rt_tick_get() - s_compare_ccs_module_count[gunno]) > APP_CURR_CCSBCS_ABNORMAL_TIMEOUT){
            if(is_stop_charge_authorization == false){
                s_thaisen_transaction[gunno].stop_reason = APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL;
                s_ofsm_info[gunno].base.reason_code = s_thaisen_transaction[gunno].stop_reason;
                s_ofsm_info[gunno].base.flag.is_fault_stop = APP_THA_ENUM_TRUE;

                is_stop_charge_authorization = true;
                LOG_D("gunno(%d) charge finish deal to CCS module current abnormal(%d, %d)\n", gunno, bms_ccs_curr, thaisen_get_module_curr(gunno));
            }
        }
    }else{
        s_compare_ccs_module_count[gunno] = rt_tick_get();
    }
#endif

#ifdef APP_INCLUDE_NET
#if 0
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
#endif
#endif /* APP_INCLUDE_NET */

    /** 这是副枪故障导致的主枪停止，主枪要获取副枪的信息 */
    if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) || (s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
        if((s_ofsm_info[gunno].base.flag.is_deputygun_stop == APP_THA_ENUM_TRUE) && (s_ofsm_info[gunno].base.main_gunno == gunno)){
            uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
            if(gunno == APP_SYSTEM_GUNNOA){
                deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
            }
            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

            s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
            thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

            mw_charge_stop_cmd(gunno);

            s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

            s_ofsm_info[gunno].base.system_fault = s_ofsm_info[deputy_gunno].base.system_fault;
            s_ofsm_info[gunno].base.charge_fault = s_ofsm_info[deputy_gunno].base.charge_fault;

            s_thaisen_transaction[gunno].stop_reason = s_thaisen_transaction[deputy_gunno].stop_reason;
            s_ofsm_info[gunno].base.reason_code = s_ofsm_info[deputy_gunno].base.reason_code;
            s_ofsm_info[gunno].base.flag.is_fault_stop = s_ofsm_info[deputy_gunno].base.flag.is_fault_stop;

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

            app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

            app_nsal_report_remote_stop_result(gunno, APP_THA_ENUM_TRUE, s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

            app_nsal_state_charged(gunno);
            app_nsal_event_occurded(gunno);

            LOG_D("gunno(%d) charge finish deal deputy gun fault stop in parallel charge mode\n", gunno);
            return;
        }
    }

    if(is_stop_charge_authorization == true){
        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STOPING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_STOPING;

#ifdef APP_INCLUDE_YKC17_PROTOCOL
        thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_STOP);
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

        mw_charge_stop_cmd(gunno);

        s_thaisen_transaction[gunno].order_state.is_charging = APP_THA_ENUM_FALSE;

        s_ofsm_info[gunno].base.system_fault = APP_SYS_FAULT_NO_ERROR;
        s_ofsm_info[gunno].base.charge_fault = APP_CHARGE_FAULT_NO_ERROR;
        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
            uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
            if(gunno == APP_SYSTEM_GUNNOA){
                deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
            }
            s_ofsm_info[gunno].base.charge_elect_last = (mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno));
        }else{
            s_ofsm_info[gunno].base.charge_elect_last = mw_get_meter_total_wh(gunno);
        }

        app_charge_fault_occur(gunno, s_ofsm_info[gunno].base.reason_code, (*mw_get_charge_fault_set(gunno)));

        app_nsal_report_remote_stop_result(gunno, APP_THA_ENUM_TRUE, s_ofsm_info[gunno].base.reason_code, s_ofsm_info[gunno].base.reason_code);

        app_nsal_state_charged(gunno);
        app_nsal_event_occurded(gunno);
        return;
    }

    if((s_ofsm_info[gunno].base.state.current != APP_OFSM_STATE_CHARGING) && (is_stop_charge_authorization == false)){
        s_ofsm_info[gunno].base.state.current = APP_OFSM_STATE_CHARGING;
        app_nsal_state_charged(gunno);
    }

    app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    app_card_event_recv(APP_CARD_EVENT_CHARGE_START, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
}
/*****************************************************
 * 函数名          ofsm_stoping_fun
 * 功能               业务状态机 停止中 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
static void ofsm_stoping_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    bool is_stop_complete = APP_THA_ENUM_FALSE;  /* 充电已停止完成 */
    enum charge_state_t charge_state = mw_get_charge_state(gunno);
#ifdef APP_INCLUDE_YKC17_PROTOCOL
    uint32_t meter_reading_tick = 0x00;
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        charge_state = mw_get_charge_state(s_ofsm_info[gunno].base.main_gunno);
    }

    if (++s_debug_count[gunno] > (1000 + 500 *gunno) / 100) {
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) stop state (%dV | S%d)...", gunno, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state);
    }

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
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
        if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
            bms = mw_get_bms_data(s_ofsm_info[gunno].base.main_gunno);
        }
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
        memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

        app_nsal_init_charge_data(gunno);

        rt_thread_mdelay(1000);  /* 错峰上报订单 */

#ifdef APP_INCLUDE_YKC17_PROTOCOL
        /** 电表加密 */
        meter_reading_tick = rt_tick_get();
        /** 等待停止充电响应 */
        while((rt_tick_get() - meter_reading_tick) < APP_AMMETER_ENCRY_WAIT_STOP_REPLY_MS){
            if(thaisen_ammeter_is_replied(gunno)){
                LOG_D("ammeter stop charge cmd replied(%d)", gunno);
                break;
            }
        }

        s_thaisen_transaction[gunno].meter_ver = 0x00;
        s_thaisen_transaction[gunno].meter_encry_type = 0x00;
        memset(s_thaisen_transaction[gunno].meter_trade_number, 0x00, sizeof(s_thaisen_transaction[gunno].meter_trade_number));
        memset(s_thaisen_transaction[gunno].meter_dev_sn, 0x00, sizeof(s_thaisen_transaction[gunno].meter_dev_sn));
        memset(s_thaisen_transaction[gunno].meter_port_identify_sn, 0x00, sizeof(s_thaisen_transaction[gunno].meter_port_identify_sn));
        s_thaisen_transaction[gunno].meter_stimestamp = 0x00;
        s_thaisen_transaction[gunno].meter_etimestamp = 0x00;
        s_thaisen_transaction[gunno].meter_positive_elect = 0x00;
        s_thaisen_transaction[gunno].meter_install_timestamp = 0x00;
        s_thaisen_transaction[gunno].meter_history_state = 0x00;

        meter_reading_tick = rt_tick_get();

        LOG_D("app ofsm meter reading start...");
        thaisen_ammeter_encry_scmd(gunno, THAISEN_AMMETER_ENCRY_TYPE_METER_READING);

        while((rt_tick_get() - meter_reading_tick) < APP_AMMETER_ENCRY_WAIT_READING_REPLY_MS){
            if(thaisen_ammeter_is_replied(gunno)){
                uint8_t _compare_len = 0x00;
                LOG_D("app ofsm meter reading is replied(%d)\n", (rt_tick_get() - meter_reading_tick));
                thaisen_encry_data_t *_encry_data = (thaisen_encry_data_t*)thaisen_ammeter_query_encrypt_data(gunno);
                if(_encry_data){
                    s_thaisen_transaction[gunno].meter_ver = _encry_data->ver;
                    s_thaisen_transaction[gunno].meter_encry_type = _encry_data->encry_type;

                    _compare_len = sizeof(_encry_data->trade_number);
                    _compare_len = _compare_len > sizeof(s_thaisen_transaction[gunno].meter_trade_number) ? sizeof(s_thaisen_transaction[gunno].meter_trade_number) : _compare_len;
                    memset(s_thaisen_transaction[gunno].meter_trade_number, 0x00, sizeof(s_thaisen_transaction[gunno].meter_trade_number));
                    memcpy(s_thaisen_transaction[gunno].meter_trade_number, _encry_data->trade_number, _compare_len);

                    _compare_len = sizeof(_encry_data->dev_sn);
                    _compare_len = _compare_len > sizeof(s_thaisen_transaction[gunno].meter_dev_sn) ? sizeof(s_thaisen_transaction[gunno].meter_dev_sn) : _compare_len;
                    memset(s_thaisen_transaction[gunno].meter_dev_sn, 0x00, sizeof(s_thaisen_transaction[gunno].meter_dev_sn));
                    memcpy(s_thaisen_transaction[gunno].meter_dev_sn, _encry_data->dev_sn, _compare_len);

                    _compare_len = sizeof(_encry_data->port_identify_sn);
                    _compare_len = _compare_len > sizeof(s_thaisen_transaction[gunno].meter_port_identify_sn) ? sizeof(s_thaisen_transaction[gunno].meter_port_identify_sn) : _compare_len;
                    memset(s_thaisen_transaction[gunno].meter_port_identify_sn, 0x00, sizeof(s_thaisen_transaction[gunno].meter_port_identify_sn));
                    memcpy(s_thaisen_transaction[gunno].meter_port_identify_sn, _encry_data->port_identify_sn, _compare_len);

                    s_thaisen_transaction[gunno].meter_stimestamp = _encry_data->stimestamp;
                    s_thaisen_transaction[gunno].meter_etimestamp = _encry_data->etimestamp;
                    s_thaisen_transaction[gunno].meter_positive_elect = _encry_data->positive_elect;
                    s_thaisen_transaction[gunno].meter_install_timestamp = _encry_data->install_timestamp;
                    s_thaisen_transaction[gunno].meter_history_state = _encry_data->history_state;
                }
                break;
            }
        }
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

        if(s_ofsm_info[gunno].base.flag.start_result == APP_THA_ENUM_FALSE){
            s_ofsm_info[gunno].base.current_elect = s_ofsm_info[gunno].base.start_elect;
            s_ofsm_info[gunno].base.elect_a = 0x00;
            s_ofsm_info[gunno].base.fees_total = 0x00;
            s_ofsm_info[gunno].base.service_fees_total = 0x00;
            s_ofsm_info[gunno].base.elect_fees_total = 0x00;
            if(s_ofsm_info[gunno].base.account_ballance_before > (s_ofsm_info[gunno].base.fees_total /100)){
                s_ofsm_info[gunno].base.account_ballance_after = s_ofsm_info[gunno].base.account_ballance_before - s_ofsm_info[gunno].base.fees_total;
            }
            s_thaisen_transaction[gunno].ammeter_stop = s_ofsm_info[gunno].base.current_elect;
            s_thaisen_transaction[gunno].total_elect = s_ofsm_info[gunno].base.elect_a;
            s_thaisen_transaction[gunno].total_loss_elect = 0x00;;
            s_thaisen_transaction[gunno].charge_fee = s_ofsm_info[gunno].base.elect_fees_total;
            s_thaisen_transaction[gunno].service_fee = s_ofsm_info[gunno].base.service_fees_total;
            s_thaisen_transaction[gunno].total_fee = s_ofsm_info[gunno].base.fees_total;
            s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

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
            if((s_ofsm_info[gunno].base.run_mode != APP_RUN_MODE_OFFLINE_BILLING) ||
                    ((s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING) && (s_ofsm_info[gunno].base.start_type != APP_CHARGE_START_WAY_OFFLINE_CARD))){
                if((s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) && (gunno == s_ofsm_info[gunno].base.main_gunno)){
                    uint8_t deputy_gunno = APP_SYSTEM_GUNNOA;
                    if(gunno == APP_SYSTEM_GUNNOA){
                        deputy_gunno = APP_SYSTEM_GUNNOA + 0x01;
                    }
                    uint32_t _elect = mw_get_meter_total_wh(gunno) + mw_get_meter_total_wh(deputy_gunno);
                    /** 按600KW算，一小时600度，一分钟10度，一秒钟 1/6 度，三位小数，大概是：166， 约等于170， 每次计算两次之间的差值不能大于一定值 */
                    if((_elect <= (s_ofsm_info[gunno].base.charge_elect_last + APP_CALCULATE_ELECT_DIFF_MAX)) && (_elect > s_ofsm_info[gunno].base.charge_elect_last)){
                        app_billing_info_calculate(s_ofsm_info[gunno].base.stop_time, _elect, (_elect - s_ofsm_info[gunno].base.charge_elect_last), gunno);
                    }
                }else{
                    uint32_t _elect = mw_get_meter_total_wh(gunno);
                    /** 按600KW算，一小时600度，一分钟10度，一秒钟 1/6 度，三位小数，大概是：166， 约等于170， 每次计算两次之间的差值不能大于一定值 */
                    if((_elect <= (s_ofsm_info[gunno].base.charge_elect_last + APP_CALCULATE_ELECT_DIFF_MAX)) && (_elect > s_ofsm_info[gunno].base.charge_elect_last)){ /** 按三秒的电量来算 */
                        app_billing_info_calculate(s_ofsm_info[gunno].base.stop_time, _elect, (_elect - s_ofsm_info[gunno].base.charge_elect_last), gunno);
                    }
                }

                s_ofsm_info[gunno].base.start_elect = app_billingrule_get_start_elcet(gunno);
                s_ofsm_info[gunno].base.current_elect = app_billingrule_get_stop_elcet(gunno);
                s_ofsm_info[gunno].base.elect_a = app_billingrule_get_elcet_total(gunno);
                s_ofsm_info[gunno].base.fees_total = app_billingrule_get_fees_total(gunno);
                s_ofsm_info[gunno].base.service_fees_total = app_billingrule_get_service_fees_total(gunno);

                if(s_ofsm_info[gunno].base.fees_total > APP_SPEND_AMOUNT_MAX){
                    s_ofsm_info[gunno].base.fees_total = APP_SPEND_AMOUNT_MAX;
                }
                if(s_ofsm_info[gunno].base.service_fees_total > APP_SPEND_AMOUNT_MAX){
                    s_ofsm_info[gunno].base.service_fees_total = APP_SPEND_AMOUNT_MAX;
                }
                if(s_ofsm_info[gunno].base.elect_a > APP_CHARGE_ELECT_MAX){
                    s_ofsm_info[gunno].base.elect_a = APP_CHARGE_ELECT_MAX;
                }
                if(s_ofsm_info[gunno].base.fees_total > s_ofsm_info[gunno].base.service_fees_total){
                    s_ofsm_info[gunno].base.elect_fees_total = (s_ofsm_info[gunno].base.fees_total - s_ofsm_info[gunno].base.service_fees_total);
                    if(s_ofsm_info[gunno].base.elect_fees_total > APP_SPEND_AMOUNT_MAX){
                        s_ofsm_info[gunno].base.elect_fees_total = APP_SPEND_AMOUNT_MAX;
                    }
                }
                if(s_ofsm_info[gunno].base.account_ballance_before > (s_ofsm_info[gunno].base.fees_total /100)){
                    s_ofsm_info[gunno].base.account_ballance_after = s_ofsm_info[gunno].base.account_ballance_before - (s_ofsm_info[gunno].base.fees_total /100);
                }
                s_thaisen_transaction[gunno].ammeter_start = s_ofsm_info[gunno].base.start_elect;
                s_thaisen_transaction[gunno].ammeter_stop = s_ofsm_info[gunno].base.current_elect;
                s_thaisen_transaction[gunno].total_elect = s_ofsm_info[gunno].base.elect_a;
                s_thaisen_transaction[gunno].total_loss_elect = 0x00;;
                s_thaisen_transaction[gunno].charge_fee = s_ofsm_info[gunno].base.elect_fees_total;
                s_thaisen_transaction[gunno].service_fee = s_ofsm_info[gunno].base.service_fees_total;
                s_thaisen_transaction[gunno].total_fee = s_ofsm_info[gunno].base.fees_total;
                s_thaisen_transaction[gunno].account_ballance_after = s_ofsm_info[gunno].base.account_ballance_after;

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) || \
            defined (APP_INCLUDE_SGCC_PROTOCOL))
                for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                    s_thaisen_transaction[gunno].rate_type_elect[type] = app_billingrule_get_rate_type_elect(gunno, type);
                    s_thaisen_transaction[gunno].rate_type_amount[type] = app_billingrule_get_rate_type_fess(gunno, type);

                    if(s_thaisen_transaction[gunno].rate_type_amount[type] > APP_SPEND_AMOUNT_MAX){
                        s_thaisen_transaction[gunno].rate_type_amount[type] = APP_SPEND_AMOUNT_MAX;
                    }
                    if(s_thaisen_transaction[gunno].rate_type_elect[type] > APP_CHARGE_ELECT_MAX){
                        s_thaisen_transaction[gunno].rate_type_elect[type] = APP_CHARGE_ELECT_MAX;
                    }
                }
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || defined (APP_INCLUDE_YCP_PROTOCOL) ||
            defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
                for(uint8_t period = s_ofsm_info[gunno].base.start_period; period <= s_ofsm_info[gunno].base.current_period; period++){
                    s_thaisen_transaction[gunno].period_elect[period] = app_billingrule_get_period_elect(gunno, period);
                    s_thaisen_transaction[gunno].period_elect_fees[period] = app_billingrule_get_period_elect_fees(gunno, period);
                    s_thaisen_transaction[gunno].period_service_fees[period] = app_billingrule_get_period_service_fees(gunno, period);

                    if(s_thaisen_transaction[gunno].period_elect_fees[period] > APP_SPEND_AMOUNT_MAX){
                        s_thaisen_transaction[gunno].period_elect_fees[period] = APP_SPEND_AMOUNT_MAX;
                    }
                    if(s_thaisen_transaction[gunno].period_service_fees[period] > APP_SPEND_AMOUNT_MAX){
                        s_thaisen_transaction[gunno].period_service_fees[period] = APP_SPEND_AMOUNT_MAX;
                    }
                    if(s_thaisen_transaction[gunno].period_elect[period] > APP_CHARGE_ELECT_MAX){
                        s_thaisen_transaction[gunno].period_elect[period] = APP_CHARGE_ELECT_MAX;
                    }
                }
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
                for(uint8_t type = 0x00; type < APP_BILLING_RULE_RATE_TYPE_MAX; type++){
                    s_thaisen_transaction[gunno].rate_type_elect_amount[type] = app_billingrule_get_rate_type_elect_fess(gunno, type);
                    s_thaisen_transaction[gunno].rate_type_service_amount[type] = app_billingrule_get_rate_type_service_fess(gunno, type);

                    if(s_thaisen_transaction[gunno].rate_type_elect_amount[type] > APP_SPEND_AMOUNT_MAX){
                        s_thaisen_transaction[gunno].rate_type_elect_amount[type] = APP_SPEND_AMOUNT_MAX;
                    }
                    if(s_thaisen_transaction[gunno].rate_type_service_amount[type] > APP_SPEND_AMOUNT_MAX){
                        s_thaisen_transaction[gunno].rate_type_service_amount[type] = APP_SPEND_AMOUNT_MAX;
                    }
                }
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL)) */
            }
        }

        s_ofsm_info[gunno].base.account_ballance_before = s_ofsm_info[gunno].base.account_ballance_after;

        if(s_thaisen_transaction[gunno].end_time > s_thaisen_transaction[gunno].charge_time){
            s_thaisen_transaction[gunno].start_time = s_thaisen_transaction[gunno].end_time - s_thaisen_transaction[gunno].charge_time;
        }else{
            s_thaisen_transaction[gunno].start_time = s_thaisen_transaction[gunno].end_time;
            s_thaisen_transaction[gunno].charge_time = 0x00;
        }
        /** 如果是离线计费模式下的刷卡停止，则订单就是已经确认了的 */
        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            if((s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD) && \
                    (s_ofsm_info[gunno].base.reason_code != APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP)){
                mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_VERIFIED,  \
                        0x00, APP_THA_ENUM_FALSE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
            }else{
                mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_VERIFIED,  \
                        0x00, APP_THA_ENUM_TRUE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
            }
        }else{
            mw_storage_record_designate_index_updated(&s_thaisen_transaction[gunno], sizeof(s_thaisen_transaction[gunno]), USER_DATA_TYPE_VERIFIED,  \
                    0x00, APP_THA_ENUM_TRUE, gunno, s_current_order_index[TARGET_PLATFORM_INDEX][gunno]);
        }

        memcpy(&(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), &(s_thaisen_transaction[gunno]), sizeof(thaisen_transaction_t));
        app_nsal_transaction_record_report_target(gunno, &(s_thaisen_transaction_report[TARGET_PLATFORM_INDEX][gunno]), 0x00);
        s_transaction_sending[TARGET_PLATFORM_INDEX][gunno] = APP_THA_ENUM_TRUE;

        memcpy(&(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), &(s_thaisen_transaction[gunno]), sizeof(thaisen_transaction_t));
        app_nsal_transaction_record_report_monitor(gunno, &(s_thaisen_transaction_report[MONITOR_PLATFORM_INDEX][gunno]), 0x00);
        s_transaction_sending[MONITOR_PLATFORM_INDEX][gunno] = APP_THA_ENUM_TRUE;

        rt_thread_mdelay(3000);  /* 等待电子锁解锁 */

        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            app_card_event_send(APP_CARD_EVENT_CHARGE_STOP, gunno, NULL);
            if(s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD){   /** 离线计费：只有离线启动时才弹出请刷卡结算页面 */
                if(s_ofsm_info[gunno].base.flag.start_result == APP_THA_ENUM_FALSE){
                    /** 提示充电结束, 启动失败, 请刷卡结算 */;
                    LOG_D("gunno(%d) start fail in offline billing mode");
                    if(s_thaisen_transaction[gunno].stop_reason != APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP){
                        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_FINISH, 0x64, APP_THA_ENUM_TRUE, gunno);
                    }else{
                        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAY_COMPLETE, 0x1E, APP_THA_ENUM_TRUE, gunno);
                    }
                }else{
                    /** 提示充电结束, 请刷卡结算 */;
                    LOG_D("gunno(%d) charge finish in offline billing mode", gunno);
                    if(s_thaisen_transaction[gunno].stop_reason != APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP){   /** 离线计费：非刷卡停时才弹出请刷卡结算页面 */
                        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_FINISH, 0x64, APP_THA_ENUM_TRUE, gunno);
                    }else{
                        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAY_COMPLETE, 0x1E, APP_THA_ENUM_TRUE, gunno);
                    }
                }
            }
        }

        s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_FINISHING];
        s_ofsm_info[gunno].state = APP_OFSM_STATE_FINISHING;

        s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

        app_nsal_state_charged(gunno);

        s_ofsm_info[gunno].base.main_gunno = 0x00;
        s_ofsm_info[gunno].base.charge_way = APP_CHARGE_WAY_NONE;    /** 复位充电模式 */
        app_nsal_clear_offlinecharge_limit(gunno);

        s_ofsm_info[gunno].base.flag.is_pay_complete = APP_THA_ENUM_FALSE;
    }

    app_charge_fault_resume(gunno, s_ofsm_info[gunno].base.reason_code, 0x00);

    mw_clear_time_sync_flag(gunno);
    rfidr_clear_swipe_state(gunno);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, 1);
    app_get_hci_event(gunno, HCI_EVENT_SCREEN_STOP, 1);
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 1);

    app_nsal_clear_remote_start(gunno);
    app_nsal_clear_remote_stop(gunno);
    app_nsal_clear_remote_card_authorize(gunno);
    app_nsal_clear_remote_vin_authorize(gunno);

    app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    if(s_ofsm_info[gunno].base.reason_code != APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP){
        app_card_event_recv(APP_CARD_EVENT_PAY_COMPLETE, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
    }
}
/*****************************************************
 * 函数名          ofsm_stoping_fun
 * 功能               业务状态机 充电结束 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
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
    s_ofsm_info[gunno].base.flag.is_deputygun_stop = APP_THA_ENUM_FALSE;

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

    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if((gunno != s_ofsm_info[gunno].base.main_gunno) && (s_ofsm_info[gunno].state != s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state)){
            if(s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state < APP_OFSM_STATE_SIZE){
                s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state];
                s_ofsm_info[gunno].state = s_ofsm_info[s_ofsm_info[gunno].base.main_gunno].state;

                s_ofsm_info[gunno].base.state.current = s_ofsm_info[gunno].state;

                rfidr_clear_swipe_state(gunno);
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

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        if(app_card_event_recv(APP_CARD_EVENT_PAY_COMPLETE, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) >= APP_THA_ENUM_FALSE){
            LOG_D("gunno(%d) finish card is payed in offline billing", gunno);
            LOG_D("time:%ds  elect:%d  money:%d  ballance:%d  reason:%d", s_ofsm_info[gunno].base.charge_time,
                    s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.fees_total, s_ofsm_info[gunno].base.account_ballance_after,
                    s_ofsm_info[gunno].base.reason_code);

            rfidr_clear_swipe_state(gunno);
            s_ofsm_info[gunno].base.flag.is_pay_complete = APP_THA_ENUM_TRUE;
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAY_COMPLETE, 30, APP_THA_ENUM_TRUE, gunno);

            memset(s_ofsm_info[gunno].base.card_uid, 0x00, sizeof(s_ofsm_info[gunno].base.card_uid));
            memset(s_ofsm_info[gunno].base.card_number, 0x00, sizeof(s_ofsm_info[gunno].base.card_number));
        }
    }
    /** 离线计费：未接收到订单结算完成前要一直发送充电结束事件 */
    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        if((s_ofsm_info[gunno].base.flag.is_pay_complete == APP_THA_ENUM_FALSE) && (s_ofsm_info[gunno].base.start_type == APP_CHARGE_START_WAY_OFFLINE_CARD)){
            app_card_event_send(APP_CARD_EVENT_CHARGE_STOP, gunno, NULL);
        }else{
            app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
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

        rfidr_clear_swipe_state(gunno);
        return;
    }
    case CC1_4V:
        if(s_ofsm_info[gunno].base.flag.is_fault_stop == APP_THA_ENUM_TRUE){
            if(rfidr_query_swipe_state(gunno)){
                rfidr_clear_swipe_state(gunno);
                if(app_card_query_operate_ret(gunno) != APP_CARD_OPERATE_RET_NULL){
                    app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_FAULT_STOP, 0x05, APP_THA_ENUM_TRUE, gunno);
                    }
                }
                LOG_D("gunno(%d)[%d] is fault stop, please pull and insert gun first", gunno, THAISEN_TRIG_EVENT_FAULT_STOP);
            }
            break;          /* 故障停必须要拔枪 */
        }
        if(charge_state != APP_CHARGE_STATE_IDLE){
            break;                               /* 充电结束必须等待充电状态为空闲时才可响应拔枪动作，已与充电控制同步 */
        }

        /** 离线计费模式下支持2次启动，2次启动条件：必须先接收到卡已结算信号量，然后发送充电桩已准备好事件 */
        if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
            if((s_ofsm_info[gunno].base.flag.is_pay_complete == APP_THA_ENUM_TRUE) || (s_ofsm_info[gunno].base.start_type != APP_CHARGE_START_WAY_OFFLINE_CARD)){
                app_card_event_send(APP_CARD_EVENT_CHARGEPILE_READY, gunno, NULL);
            }else{
                app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
                if(rfidr_query_swipe_state(gunno)){
                    rfidr_clear_swipe_state(gunno);
                    switch(rfidr_query_info_type()){
                    case APP_RFIDR_INFO_TYPE_RW_FAIL:
                        switch(app_card_query_operate_ret(gunno)){
                        case APP_CARD_OPERATE_RET_NO_BALLANCE:
                            LOG_D("gunno(%d) card no ballance in offline billing mode(finish)", gunno);
                            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_CARD_NOBALLANCE, 0x05, APP_THA_ENUM_TRUE, gunno);
                            break;
                        case APP_CARD_OPERATE_RET_IS_LOCKED:
                            LOG_D("gunno(%d) card is locked in offline billing mode(finish)", gunno);
                            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_CARD_LOCKED, 0x05, APP_THA_ENUM_TRUE, gunno);
                            break;
                        case APP_CARD_OPERATE_RET_PAYED:
                            break;
                        case APP_CARD_OPERATE_RET_NOT_START_CARD:
                            LOG_D("gunno(%d) not start card in offline billing mode", gunno);
                            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_SWITCH_GUN, 0x05, APP_THA_ENUM_TRUE, gunno);
                            break;
                        case APP_CARD_OPERATE_RET_IS_CHARGING:
                            LOG_D("gunno(%d) this card is start in offline billing mode", gunno);
                            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_IS_CHARGING, 0x05, APP_THA_ENUM_TRUE, gunno);
                            break;
                        case APP_CARD_OPERATE_RET_NULL:
                            LOG_D("gunno(%d) current state is not allow stop in offline billing mode", gunno);
                            break;
                        default:
                            LOG_D("gunno(%d) invalid card in offline billing mode 333", gunno);
                            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);
                            break;
                        }
                        break;
                    default:
                        LOG_D("gunno(%d) invalid card in offline billing mode 222", gunno);
                        app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);
                        thaisen_set_trigger_event(THAISEN_TRIG_EVENT_INVALID, 0x05, APP_THA_ENUM_TRUE, gunno);
                        break;
                    }
                }
                break;
            }
        }else{
            app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
        }

        if(app_nsal_is_remote_start(gunno)){
            app_nsal_clear_remote_start(gunno);
            if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
#ifndef APP_USING_DOUBLEGUN
                break;      /** 单枪不允许并充 */
#endif /* APP_USING_DOUBLEGUN */
            }

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
             * base->offline_chargetime
             * base->main_gunno(并充时)
             * base->charge_way(并充时)*/

            LOG_D("gunno(%d) start charge by APP", gunno);
            ofsm_start_info_padding_app(gunno);
            is_charging_authorization = true;

        }else if(rfidr_query_swipe_state(gunno)){
            rfidr_clear_swipe_state(gunno);
            if(s_ofsm_info[gunno].base.run_mode != APP_RUN_MODE_OFFLINE_BILLING){
                if(thaisen_is_not_allow_swip_card()){
                    LOG_D("gunno(%d) current page is not allow swip card charge", gunno);
                    return;
                }
            }
            if(ofsm_swip_card_judge(gunno)){
                is_charging_authorization = true;
            }else{
                return;
            }
        }else if(app_nsal_is_card_authorize_success(gunno)){
            app_nsal_clear_remote_card_authorize(gunno);

            LOG_D("gunno(%d) start charge by online card", gunno);
            ofsm_start_info_padding_online_card(gunno);
            is_charging_authorization = true;

        }else if(app_nsal_is_card_authorize_fail(gunno)){
            app_nsal_clear_remote_card_authorize(gunno);
            LOG_W("gunno(%d) swip card authorize response fail", gunno);
            s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;

            app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);

        }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, APP_THA_ENUM_TRUE)){
            LOG_D("gunno(%d) start charge by screen", gunno);
            ofsm_start_info_padding_screen(gunno);
            is_charging_authorization = true;

        }else if(app_get_hci_event(gunno, HCI_EVENT_VIN_START, APP_THA_ENUM_TRUE)){
            LOG_D("gunno(%d) start charge by VIN", gunno);
            ofsm_start_info_padding_vin(gunno);
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
            if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
            }
            /** 启动前向屏幕对时 */
            thaisen_request_screen_time();

            ofsm_start_info_padding_public(gunno);

            s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING];
            s_ofsm_info[gunno].state = APP_OFSM_STATE_STARTING;
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
        rfidr_clear_swipe_state(gunno);
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
/*****************************************************
 * 函数名          ofsm_faulting_fun
 * 功能               业务状态机 故障 状态
 * 参数              gunno   枪号
 * 返回
 ****************************************************/
static void ofsm_faulting_fun(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){   /* 枪号不对 */
        return;
    }

    bool is_charging_authorization = false;
    enum charge_state_t charge_state = mw_get_charge_state(gunno);
    enum charge_fault_t charge_fault = app_get_highest_priority_charge_fault(gunno);
    enum system_fault_t system_fault = app_get_highest_priority_system_fault(gunno);

    s_ofsm_info[gunno].base.flag.is_deputygun_stop = APP_THA_ENUM_FALSE;

    if(++s_debug_count[gunno] > (3000 + 500 *gunno) / 100){
        s_debug_count[gunno] = 0;
        LOG_I("gunno(%d) faulting state (32L: 0x%X, CC1: %d, S%d)...", gunno, (uint32_t)system_fault, mw_get_cc1_value(s_ofsm_info[gunno].base.cc1_state), charge_state);
    }

    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
        if(app_card_event_recv(APP_CARD_EVENT_PAY_COMPLETE, 0x00, gunno, NULL, APP_THA_ENUM_TRUE) >= APP_THA_ENUM_FALSE){
            LOG_D("gunno(%d) faulting card is payed in offline billing", gunno);
            LOG_D("time:%ds  elect:%d  money:%d  ballance:%d  reason:%d", s_ofsm_info[gunno].base.charge_time,
                    s_ofsm_info[gunno].base.elect_a, s_ofsm_info[gunno].base.fees_total, s_ofsm_info[gunno].base.account_ballance_after,
                    s_ofsm_info[gunno].base.reason_code);

            rfidr_clear_swipe_state(gunno);
            thaisen_set_trigger_event(THAISEN_TRIG_EVENT_PAY_COMPLETE, 30, APP_THA_ENUM_TRUE, gunno);
        }
    }

    if(system_fault == APP_SYS_FAULT_NO_ERROR){
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

        if((s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_OVERHAUL) ||
                (s_ofsm_info[gunno].base.device_state == APP_DEVICE_STATE_FREEZE)){
            memset(s_ofsm_info[gunno].base.transaction_number, 0x00, sizeof(s_ofsm_info[gunno].base.transaction_number));
            memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
            memset(s_ofsm_info[gunno].base.user_number, 0x00, sizeof(s_ofsm_info[gunno].base.user_number));

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

        memset(s_ofsm_info[gunno].base.transaction_number, 0x00, sizeof(s_ofsm_info[gunno].base.transaction_number));
        memset(s_ofsm_info[gunno].base.car_vin, 0x00, sizeof(s_ofsm_info[gunno].base.car_vin));
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
                if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                    app_card_event_send(APP_CARD_EVENT_CHARGEPILE_READY, gunno, NULL);
                }else{
                    app_card_event_recv(APP_CARD_EVENT_CHARGEPILE_READY, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
                }
#if 0
                if(s_ofsm_info[gunno].charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
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
                    app_nsal_clear_remote_start(gunno);
                    if(s_ofsm_info[gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
#ifndef APP_USING_DOUBLEGUN
                        break;      /** 单枪不允许并充 */
#endif /* APP_USING_DOUBLEGUN */
                    }

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
                     * base->offline_chargetime
                     * base->main_gunno(并充时)
                     * base->charge_way(并充时)*/

                    LOG_D("gunno(%d) start charge by APP", gunno);
                    ofsm_start_info_padding_app(gunno);
                    is_charging_authorization = true;

                }else if(rfidr_query_swipe_state(gunno)){
                    rfidr_clear_swipe_state(gunno);
                    if(s_ofsm_info[gunno].base.run_mode != APP_RUN_MODE_OFFLINE_BILLING){
                        if(thaisen_is_not_allow_swip_card()){
                            LOG_D("gunno(%d) current page is not allow swip card charge", gunno);
                            return;
                        }
                    }
                    if(ofsm_swip_card_judge(gunno)){
                        is_charging_authorization = true;
                    }else{
                        return;
                    }
                }else if(app_nsal_is_card_authorize_success(gunno)){
                    app_nsal_clear_remote_card_authorize(gunno);

                    LOG_D("gunno(%d) start charge by online card", gunno);
                    ofsm_start_info_padding_online_card(gunno);
                    is_charging_authorization = true;

                }else if(app_nsal_is_card_authorize_fail(gunno)){
                    app_nsal_clear_remote_card_authorize(gunno);
                    LOG_W("gunno(%d) swip card authorize response fail", gunno);
                    s_ofsm_info[gunno].base.flag.card_authorization = APP_THA_ENUM_FALSE;

                    app_rfidr_send_mail(APP_BUZZON_STATE_FAILED);

                }else if(app_get_hci_event(gunno, HCI_EVENT_SCREEN_START, APP_THA_ENUM_TRUE)){
                    LOG_D("gunno(%d) start charge by screen", gunno);
                    ofsm_start_info_padding_screen(gunno);
                    is_charging_authorization = true;

                }else if(app_get_hci_event(gunno, HCI_EVENT_VIN_START, APP_THA_ENUM_TRUE)){
                    LOG_D("gunno(%d) start charge by VIN", gunno);
                    ofsm_start_info_padding_vin(gunno);
                    is_charging_authorization = true;

                }

                /************** 【充电桩已授权】 *************/
                if(is_charging_authorization == true){
                    if(s_ofsm_info[gunno].base.run_mode == APP_RUN_MODE_OFFLINE_BILLING){
                        app_card_event_recv(APP_CARD_EVENT_CHARGE_STOP, 0x00, gunno, NULL, APP_THA_ENUM_TRUE);
                    }
                    /** 启动前向屏幕对时 */
                    thaisen_request_screen_time();

                    ofsm_start_info_padding_public(gunno);

                    s_ofsm_fun[gunno] = s_ofsm_fun_list[gunno][APP_OFSM_STATE_STARTING];
                    s_ofsm_info[gunno].state = APP_OFSM_STATE_STARTING;
                }
                break;
            default:
                break;
            }
            if(is_charging_authorization == true){
                rfidr_clear_swipe_state(gunno);
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
            rfidr_clear_swipe_state(gunno);
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
        s_ofsm_fun_list[gunno][APP_OFSM_STATE_WAIT_NET] = ofsm_info_init_fun;
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
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
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
    thaisenChargeGunInfo info;

    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

    memset(&info, 0x00, sizeof(thaisenChargeGunInfo));
    if(thread_gunno >= APP_SYSTEM_GUNNO_SIZE){
        thread_gunno = APP_SYSTEM_GUNNO_SIZE;
    }

    s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_4G_ETH;
    if(*(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, APP_THA_ENUM_FALSE))){
        s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_OFFLINE_BILLING;
    }else if(*(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PLUGCHARGE, APP_THA_ENUM_FALSE))){
        s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_PLUG_AND_PLAY;
    }else if(*(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_NET_TYPE, APP_THA_ENUM_FALSE)) == CP_NETTYPE_OFFLINE){
        s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_OFFLINE;
    }

    LOG_D("system run mode is:%d-%d", thread_gunno, s_ofsm_info[thread_gunno].base.run_mode);
    s_transaction_sending[TARGET_PLATFORM_INDEX][thread_gunno] = APP_THA_ENUM_FALSE;
    s_transaction_sending[MONITOR_PLATFORM_INDEX][thread_gunno] = APP_THA_ENUM_FALSE;
    s_ofsm_fun[thread_gunno] = s_ofsm_fun_list[thread_gunno][APP_OFSM_STATE_WAIT_NET];
    s_ofsm_info[thread_gunno].state = APP_OFSM_STATE_WAIT_NET;

    s_ofsm_info[thread_gunno].base.net_state = app_nsal_get_link_state();
    app_billingrule_set_eloss_proportion(*((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_ELOSS_PROPORTION, 0x00)));

    rt_thread_mdelay(6000);  /** 等待底层驱动正常(电表要获取到电量) */
    s_request_screen_time_tick = rt_tick_get();
    s_ofsm_info[thread_gunno].base.order_fixes_tick = rt_tick_get();

#ifdef APP_INCLUDE_YKC17_PROTOCOL
    s_ofsm_info[thread_gunno].base.meter_step = APP_AMMETER_ENCRY_STEP_NULL;
    s_ofsm_info[thread_gunno].base.meter_reading_tick = rt_tick_get();
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

    while(1){
        uint16_t singlegun_curr = s_ofsm_info[thread_gunno].base.gun_set_curr;
        s_ofsm_info[thread_gunno].base.ota_state = app_nsal_get_ota_state();

        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

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

        if(s_ofsm_info[thread_gunno].base.state.current == APP_OFSM_STATE_CHARGING){         /** 进入充电时才可设置BMS是否禁止充电 */
            if((s_ofsm_info[thread_gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) ||  \
                    (s_ofsm_info[thread_gunno].base.charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
                if(s_ofsm_info[thread_gunno].base.main_gunno == thread_gunno){      /** 并充时只有主枪才可设置BMS是否禁止充电 */
                    /** 设置BMS是否禁止充电 */
                    thaisenModuleSetBMSAllowCharge(thaisen_get_charging_pause_activate(thread_gunno), thread_gunno);
                }
            }else{
                /** 设置BMS是否禁止充电 */
                thaisenModuleSetBMSAllowCharge(thaisen_get_charging_pause_activate(thread_gunno), thread_gunno);
            }
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

        if(mw_get_time_sync_flag(APP_TIME_SYNC_FLAG_FEES)){
            mw_clear_time_sync_flag(APP_TIME_SYNC_FLAG_FEES);

            s_timestamp_base = mw_get_current_timestamp();
            s_tick_base = rt_tick_get();
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

        info.soc = 0x00;
        info.voltage = 0x00;
        info.current = 0x00;
        info.fault_code = 0x00;
        info.chargeElect = 0x00;
        info.chargeTime = 0x00;

        switch (s_ofsm_info[thread_gunno].base.state.current){
        case APP_OFSM_STATE_WAIT_NET:
            info.state = THAISEN_GUNSTATE_SELFCHECK;
            break;
        case APP_OFSM_STATE_IDLEING:
            info.state = THAISEN_GUNSTATE_IDLE;
            break;
        case APP_OFSM_STATE_READYING:
            info.state = THAISEN_GUNSTATE_READY;
            break;
        case APP_OFSM_STATE_RESERVATION:
            info.state = THAISEN_GUNSTATE_READY;
            break;
        case APP_OFSM_STATE_STARTING:
            info.state = THAISEN_GUNSTATE_STARTING;
            break;
        case APP_OFSM_STATE_CHARGING:
            info.soc = s_ofsm_info[thread_gunno].base.current_soc *10;
            info.voltage = s_ofsm_info[thread_gunno].base.voltage_a /10;
            info.current = s_ofsm_info[thread_gunno].base.current_a /10;
            info.chargeElect = s_ofsm_info[thread_gunno].base.elect_a /100;
            info.chargeTime = s_ofsm_info[thread_gunno].base.charge_time /60;
            info.state = THAISEN_GUNSTATE_CHARGING;
            break;
        case APP_OFSM_STATE_STOPING:
            info.soc = s_ofsm_info[thread_gunno].base.current_soc *10;
            info.chargeElect = s_ofsm_info[thread_gunno].base.elect_a /100;
            info.chargeTime = s_ofsm_info[thread_gunno].base.charge_time /60;
            info.state = THAISEN_GUNSTATE_STOPING;
            break;
        case APP_OFSM_STATE_FINISHING:
            info.soc = s_ofsm_info[thread_gunno].base.current_soc *10;
            info.chargeElect = s_ofsm_info[thread_gunno].base.elect_a /100;
            info.chargeTime = s_ofsm_info[thread_gunno].base.charge_time /60;
            info.state = THAISEN_GUNSTATE_FINISH;
            break;
        case APP_OFSM_STATE_FAULTING:
            info.fault_code = app_get_highest_priority_system_fault(thread_gunno) + 0x01;  /** + 1是因为急停故障码是0 */
            info.state = THAISEN_GUNSTATE_FAULTING;
            break;
        default:
            info.fault_code = app_get_highest_priority_system_fault(thread_gunno) + 0x01;
            info.state = THAISEN_GUNSTATE_FAULTING;
            break;
        }

        info.gunTemp = s_ofsm_info[thread_gunno].base.gunline_temperature[0x00];
        info.gunLineTemp = s_ofsm_info[thread_gunno].base.gunline_temperature[0x00];
        if(s_ofsm_info[thread_gunno].base.gunline_temperature[0x01] > s_ofsm_info[thread_gunno].base.gunline_temperature[0x00]){
            info.gunTemp = s_ofsm_info[thread_gunno].base.gunline_temperature[0x01];
            info.gunLineTemp = s_ofsm_info[thread_gunno].base.gunline_temperature[0x01];
        }

        thaisenSetGunState(thread_gunno, info);
        extern uint32_t thaisen_get_module_volt(uint8_t gunNum);
        extern uint32_t thaisen_get_module_curr(uint8_t gunNum);
        thaisen_set_gunVolt(thaisen_get_module_volt(thread_gunno), thread_gunno);
        thaisen_set_gunCurr(thaisen_get_module_curr(thread_gunno), thread_gunno);

        if(s_ofsm_info[thread_gunno].state == APP_OFSM_STATE_CHARGING){
            if(record_store > rt_tick_get()){
                record_store = rt_tick_get();
            }
            if((rt_tick_get() - record_store) > APP_STORAGE_TRANSATION_INTERVAL){
                mw_storage_record_designate_index_updated(&s_thaisen_transaction[thread_gunno], sizeof(s_thaisen_transaction[thread_gunno]), USER_DATA_TYPE_STORAGE,  \
                        0x00, APP_THA_ENUM_FALSE, thread_gunno, s_current_order_index[TARGET_PLATFORM_INDEX][thread_gunno]);
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

        if(s_ofsm_info[thread_gunno].base.run_mode >= APP_RUN_MODE_SIZE){
            s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_4G_ETH;
            if(*(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, APP_THA_ENUM_FALSE))){
                s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_OFFLINE_BILLING;
            }else if(*(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PLUGCHARGE, APP_THA_ENUM_FALSE))){
                s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_PLUG_AND_PLAY;
            }else if(*(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_NET_TYPE, APP_THA_ENUM_FALSE)) == CP_NETTYPE_OFFLINE){
                s_ofsm_info[thread_gunno].base.run_mode = APP_RUN_MODE_OFFLINE;
            }
            LOG_D("system run mode is:%d-%d", thread_gunno, s_ofsm_info[thread_gunno].base.run_mode);
        }
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

        if(thaisen_is_set_eloss_proportion()){
            app_billingrule_set_eloss_proportion(*((uint16_t*)sys_read_config_item_content(CONFIG_ITEM_ELOSS_PROPORTION, 0x00)));
        }

        rt_thread_mdelay(100);
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
