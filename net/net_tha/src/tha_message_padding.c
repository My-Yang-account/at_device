/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#include "tha_message_padding.h"
#include "tha_message_send.h"
#include "tha_message_receive.h"

#include "app_ofsm.h"
#include "app_billing_rule.h"

#ifdef NET_PACK_USING_THA

#define THA_DISPOSABLE_EVENT_HEARTBEAT                0x00          /* 漏报事件：桩心跳 */
#define THA_DISPOSABLE_EVENT_EVENT                    0x01          /* 漏报事件：桩事件 */

#define THA_CHARGE_DATA_INTERVAL_INIT                 0x05          /* 开始充电时充电数据上报间隔 */

#pragma pack(1)

struct tha_disposable_info{
    struct{
        uint8_t heartbeat : 4;                  /* 心跳状态 */
        uint8_t event : 4;                      /* 事件状态 */
    }state;
    uint8_t disposable_event;
};

struct tha_flag_info{
    uint8_t is_start_charge : 1;                  /*  已启动充电 */
    uint8_t is_stop_charge : 1;                   /*  已停止充电 */
    uint8_t is_set_power : 1;                     /*  已设置功率百分比 */

    uint8_t start_success : 1;                    /*  启机成功 */
    uint8_t stop_success : 1;                     /*  停机成功 */
    uint8_t set_power_success : 1;                /*  设置功率百分比成功 */
};

#pragma pack()

static struct tha_flag_info s_tha_flag_info[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_tha_charge_data_interval[NET_SYSTEM_GUN_NUMBER];
static uint32_t s_tha_charge_data_count[NET_SYSTEM_GUN_NUMBER];
static struct tha_disposable_info s_tha_disposable_info[NET_SYSTEM_GUN_NUMBER];
static struct net_handle* s_tha_handle = NULL;
static System_BaseData *s_tha_base = NULL;

static uint8_t tha_chargepile_stop_reason_converted(uint8_t bit);
static uint16_t tha_chargepile_start_way_converted(uint8_t bit, uint8_t stop_in_starting);

/*************************************************
 * 函数名      tha_set_clear_disposable_event
 * 功能          设置漏报报文事件
 * **********************************************/
static void tha_set_clear_disposable_event(uint8_t gunno, uint8_t event, uint8_t is_clear)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(is_clear){
        s_tha_disposable_info[gunno].disposable_event &= (~(1 <<event));
    }else{
        s_tha_disposable_info[gunno].disposable_event |= (1 <<event);
    }
}

/*************************************************
 * 函数名      tha_get_disposable_event
 * 功能          获取漏报报文事件
 * **********************************************/
static uint8_t tha_get_disposable_event(uint8_t gunno, uint8_t event, uint8_t *buf)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(buf){
        *buf = s_tha_disposable_info[gunno].disposable_event;
    }
    if(s_tha_disposable_info[gunno].disposable_event &(1 <<event)){
        return 0x01;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_get_start_charge_result
 * 功能          启动充电成功
 * **********************************************/
uint8_t tha_is_start_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_tha_flag_info[gunno].start_success;
}

/*************************************************
 * 函数名      tha_is_stop_charge_success
 * 功能          停止充电成功
 * **********************************************/
uint8_t tha_is_stop_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_tha_flag_info[gunno].stop_success;
}

/*************************************************
 * 函数名      tha_is_set_power_success
 * 功能          设置功率成功
 * **********************************************/
uint8_t tha_is_set_power_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_tha_flag_info[gunno].set_power_success;
}

/*************************************************
 * 函数名      tha_response_padding_general_message
 * 功能          组包：通用响应报文
 * **********************************************/
int8_t tha_response_padding_general_message(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_GeneralResReq_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_ThaPro_GeneralResReq_t *response = NULL;
    response = ((Net_ThaPro_GeneralResReq_t*)buf);
    memset(response, 0x00, data_len);
    response->body.gunno = gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_time_sync
 * 功能          组包：时间同步响应
 * **********************************************/
int8_t tha_response_padding_time_sync(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReqPRes_TimeSync_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_ThaPro_SReqPRes_TimeSync_t *response = NULL;
    response = ((Net_ThaPro_SReqPRes_TimeSync_t*)buf);
    memset(response, 0x00, data_len);
    response->body.timpstamp = time(NULL);
    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_system_reboot
 * 功能          组包：系统重启响应
 * **********************************************/
int8_t tha_response_padding_system_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReqPRes_TimeSync_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_ThaPro_SReqPRes_SystemReboot_t *response = NULL;
    response = ((Net_ThaPro_SReqPRes_SystemReboot_t*)buf);
    memset(response, 0x00, data_len);
    response->body.timpstamp = time(NULL);
    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_query_charge_data
 * 功能          组包：查询充电数据响应
 * **********************************************/
int8_t tha_response_padding_query_charge_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReqPRes_TimeSync_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint8_t valid_len = 0x00;
    Net_ThaPro_PRes_Query_ChargeData_t *response = NULL;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    response = ((Net_ThaPro_PRes_Query_ChargeData_t*)buf);
    memset(response, 0x00, data_len);

    if(s_tha_base->state.current != APP_OFSM_STATE_CHARGING){
        response->head.status = NET_THA_STATUS_CODE_REFUSE_SERVICE;
    }
    response->body.start_time = s_tha_base->start_time;
    response->body.elect_total = s_tha_base->elect_a;

    valid_len = sizeof(s_tha_base->transaction_number);
    valid_len = valid_len > sizeof(response->body.trade_no) ? sizeof(response->body.trade_no) : valid_len;
    memset(&(s_tha_base->transaction_number), 0x00, sizeof(s_tha_base->transaction_number));
    memcpy(&response->body.trade_no, s_tha_base->transaction_number, valid_len);
    response->body.elect = s_tha_base->elect_a;
    response->body.volatge = s_tha_base->voltage_a;
    response->body.current = s_tha_base->current_a;
    response->body.charge_time = s_tha_base->charge_time;
    response->body.gunno = gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_query_system_info
 * 功能          组包：查询系统信息响应
 * **********************************************/
int8_t tha_response_padding_query_system_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }

    uint8_t valid_len = 0x00, data_len = 0x00;
    Net_ThaPro_PRes_Query_SystemInfo_t *response = NULL;
    uint8_t *data = NULL;
    uint32_t length = 0x00, option = (NET_SYSTEM_DATA_OPTION_PLAT_THA |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(0x00));
    data = s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, NULL, 0x00, option);

    data_len = (sizeof(Net_ThaPro_PRes_Query_SystemInfo_t) + strlen((char*)(data)));
    if(data_len > ilen){
        return -0x02;
    }
    response = ((Net_ThaPro_PRes_Query_SystemInfo_t*)buf);
    memset(response, 0x00, data_len);

    response->body.software_ver[0] = s_tha_base->soft_ver_main;
    response->body.software_ver[1] = s_tha_base->soft_ver_sub;
    response->body.software_ver[2] = s_tha_base->soft_ver_revise;

    response->body.server_domain_length = length;
    memcpy((response->body.operator_name + sizeof(response->body.operator_name)), data, response->body.server_domain_length);

    data = s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option);
    response->body.pile_name_length = length;
    valid_len = sizeof(response->body.pile_name);
    valid_len = valid_len > response->body.pile_name_length ? response->body.pile_name_length : valid_len;
    memcpy(response->body.pile_name, data, valid_len);

    data = s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_OPERATOR, NULL, 0x00, option);
    response->body.operator_name_length = length;
    valid_len = sizeof(response->body.operator_name);
    valid_len = valid_len > response->body.operator_name_length ? response->body.operator_name_length : valid_len;
    memcpy(response->body.operator_name, data, valid_len);

    response->body.port = *(uint16_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_PORT, NULL, 0x00, option));

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_query_billing_rule
 * 功能          组包：查询计费规则响应
 * **********************************************/
int8_t tha_response_padding_query_billing_rule(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t *response = NULL;

    response = ((Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t*)buf);
    memset(response, 0x00, data_len);

    response->body.rate_1 = app_billingrule_get_rate_price(gunno, APP_RATE_TYPE_SHARP);
    response->body.rate_2 = app_billingrule_get_rate_price(gunno, APP_RATE_TYPE_PEAK);
    response->body.rate_3 = app_billingrule_get_rate_price(gunno, APP_RATE_TYPE_FLAT);
    response->body.rate_4 = app_billingrule_get_rate_price(gunno, APP_RATE_TYPE_VALLEY);
    response->body.service_rate = app_billingrule_get_period_service_price(gunno, 0x00);

    for(uint8_t period = 0, count = 0; period < APP_BILLING_RULE_PERIOD_MAX; period += 2, count++){
        response->body.period_rate_number[count] = app_billingrule_get_period_rate_number(gunno, period);
    }
    response->body.gunno = gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_query_charge_system_set
 * 功能          组包：查询充电系统设置响应
 * **********************************************/
int8_t tha_response_padding_query_charge_system_set(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_THA |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t *response = NULL;
    uint32_t length = 0x00;

    response = ((Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t*)buf);
    memset(response, 0x00, data_len);

    response->body.voltage_max = *(uint16_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_VOLTAGE_MAX, NULL, 0x00, option));
    response->body.voltage_min = *(uint16_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_VOLTAGE_MIN, NULL, 0x00, option));
    response->body.current_max = *(uint16_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_CURRENT_MAX, NULL, 0x00, option));
    response->body.power_module_num = *(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_MODULE_NUM, NULL, 0x00, option));
    response->body.power_of_single_module = *(uint16_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_SINGLE_MODULE_POWER, NULL, 0x00, option));
    response->body.power_limit_of_ac_charge = 0x00;
#ifdef USING_EXPAND_SECTION
    response->body.expand_voltage_max = *(uint32_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MAX, NULL, 0x00, option));
    response->body.expand_current_min = *(uint32_t*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MIN, NULL, 0x00, option));
#endif /* USING_EXPAND_SECTION */

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

//////////////////// incomplete
/*************************************************
 * 函数名      tha_response_padding_query_device_fault_info
 * 功能          组包：查询设备故障信息响应
 * **********************************************/
int8_t tha_response_padding_query_device_fault_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_PReq_Report_PRes_Query_Device_FaultInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(0x00));
    Net_ThaPro_PReq_Report_PRes_Query_Device_FaultInfo_t *response = NULL;

    response = ((Net_ThaPro_PReq_Report_PRes_Query_Device_FaultInfo_t*)buf);
    memset(response, 0x00, data_len);

    response->body.ac_device_fault_state = 0x00;
    response->body.dc_device_fault_state = 0x00;
    response->body.dc_chargemodule_fault_state = 0x00;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_query_battery_charge_info
 * 功能          组包：查询电池充电信息响应
 * **********************************************/
int8_t tha_response_padding_query_battery_charge_info(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_tha_base->bms_data);
    Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_t *response = NULL;

    response = ((Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_t*)buf);
    memset(response, 0x00, data_len);

    response->body.battery_type = bms->BRM.BatType;
//    response->body.allow_temp_max = ((bms->BCP.BatAlowHigTemp /10) + 50);
    response->body.allow_temp_max = (bms->BCP.BatAlowHigTemp + 50);
    response->body.allow_chargevolt_max_ofbms = bms->BCP.BatAlowHigVolt;
    response->body.allow_chargevolt_max_ofbsingle = bms->BCP.CellAlowHigVolt;
    if(bms->BCP.AlowCurlt > 4000){
        response->body.allow_chargecurr_max = 0x00;
    }else{
        response->body.allow_chargecurr_max = (4000 - bms->BCP.AlowCurlt);
    }
    response->body.battery_ratedvolt_total = bms->BRM.BatRateVolt;
    response->body.battery_voltage_current = bms->BCP.BatVolt;
    response->body.battery_rated_capacity = bms->BRM.BatRateCap;
    response->body.battery_nominal_power = bms->BCP.BatRateKW;
    response->body.gunno = gunno;
    memcpy(response->body.vin, bms->BRM.CarDiscern, NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}
//////////////////// incomplete
/*************************************************
 * 函数名      tha_response_padding_query_order_state
 * 功能          组包：查询订单状态响应
 * **********************************************/
int8_t tha_response_padding_query_order_state(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_ThaPro_PRes_Query_OrderState_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_ThaPro_PRes_Query_OrderState_t *response = NULL;
    struct thaisenBMS_Charger_struct *bms = NULL;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(0x00));
    bms = (struct thaisenBMS_Charger_struct*)(s_tha_base->bms_data);

    response = ((Net_ThaPro_PRes_Query_OrderState_t*)buf);
    memset(response, 0x00, data_len);

    response->body.trade_no = 0x00;
    response->body.state = 0x00;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_update_billing_rule_request
 * 功能          处理服务器下发的更新计费规则请求
 * **********************************************/
int8_t tha_message_pro_update_billing_rule_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    uint16_t elect_fees = 0x00, rate_price = 0x00;
    Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t *request = (Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t*)data;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    if((s_tha_base->state.current == APP_OFSM_STATE_CHARGING) || (s_tha_base->state.current == APP_OFSM_STATE_STARTING)){
        net_operation_set_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
        gunno = NET_SYSTEM_GUN_NUMBER;
    }
    app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_SHARP, request->body.rate_1);
    app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_PEAK, request->body.rate_2);
    app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_FLAT, request->body.rate_3);
    app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_VALLEY, request->body.rate_4);

    for(uint8_t period = 0x00, count = 0x00; period < APP_BILLING_RULE_PERIOD_MAX; period += 2, count++){
        app_billingrule_set_period_rate_number(gunno, period, request->body.period_rate_number[count]);
        app_billingrule_set_period_rate_number(gunno, (period + 1), request->body.period_rate_number[count]);

        app_billingrule_set_period_service_price(gunno, period, request->body.service_rate);
        rate_price = app_billingrule_get_rate_price(gunno, request->body.period_rate_number[count]);
        if(rate_price > request->body.service_rate){
            elect_fees = rate_price - request->body.service_rate;
        }else{
            elect_fees = 0x00;
        }
        app_billingrule_set_period_elect_price(gunno, period, elect_fees);
        app_billingrule_set_period_delay_price(gunno, period, 0x00);
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_set_reservation_request
 * 功能          处理服务器下发的设置定时充电请求
 * **********************************************/
int8_t tha_message_pro_set_reservation_request(uint8_t gunno, void *data, uint8_t len)
{
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_cancel_reservation_request
 * 功能          处理服务器下发的取消定时请求
 * **********************************************/
int8_t tha_message_pro_cancel_reservation_request(uint8_t gunno, void *data, uint8_t len)
{
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_stop_charge_request
 * 功能          处理服务器下发的停止充电请求
 * **********************************************/
int8_t tha_message_pro_stop_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_StopCharge_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));
    Net_ThaPro_SReq_StopCharge_t *request = (Net_ThaPro_SReq_StopCharge_t*)data;

    if(s_tha_base->state.current != APP_OFSM_STATE_CHARGING){
        return -0x04;
    }
#ifndef NET_THA_AS_MONITOR
    uint8_t valid_len = sizeof(s_tha_base->user_number);
    valid_len = valid_len > sizeof(request->body.user_id) ? sizeof(request->body.user_id) : valid_len;
    if(memcmp(s_tha_base->user_number, &(request->body.user_id), valid_len)){
        return -0x05;
    }
#endif /* NET_THA_AS_MONITOR */
    s_tha_flag_info[gunno].is_stop_charge = 0x01;

    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_remote_update_request
 * 功能          处理服务器下发的远程升级请求
 * **********************************************/
int8_t tha_message_pro_remote_update_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_StartUpdate_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_ThaPro_SReq_StartUpdate_t *request = (Net_ThaPro_SReq_StartUpdate_t*)data;

    if(request->body.segment_len > NET_OTA_SEGMENT_LEN){
        return -0x03;
    }
    if(request->body.updated_way != 0x01){
        return -0x05;
    }
    if(request->body.update_pack_len > NET_OTA_DATA_REGION_SIZE){
        return -0x06;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_unlock_charge_request
 * 功能          处理服务器下发的解锁充电请求
 * **********************************************/
int8_t tha_message_pro_unlock_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_UnlockCharge_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }

    uint8_t valid_len = 0x00;
    Net_ThaPro_SReq_UnlockCharge_t *request = (Net_ThaPro_SReq_UnlockCharge_t*)data;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    switch(s_tha_base->state.current){
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        valid_len = sizeof(s_tha_base->user_number);
        valid_len = valid_len > sizeof(request->body.user_id) ? sizeof(request->body.user_id) : valid_len;
        memcpy(s_tha_base->user_number, &(request->body.user_id), valid_len);

        valid_len = sizeof(s_tha_base->transaction_number);
        valid_len = valid_len > sizeof(request->body.trade_no) ? sizeof(request->body.trade_no) : valid_len;
        memcpy(&(s_tha_base->transaction_number), &(request->body.trade_no), valid_len);

        memset(s_tha_base->card_number, 0x00, sizeof(s_tha_base->card_number));
        memset(s_tha_base->card_uid, 0x00, sizeof(s_tha_base->card_uid));
        s_tha_base->account_balance = 0x00;

        s_tha_flag_info[gunno].is_start_charge = 0x01;
        return 0x00;
        break;
    default:
        return -0x04;
        break;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_authority_response
 * 功能          处理服务器响应的权限认证
 * **********************************************/
int8_t tha_message_pro_authority_response(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_THA_AS_MONITOR
    uint8_t data_len = sizeof(Net_ThaPro_SRes_Authority_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    uint8_t valid_len = 0x00;
    Net_ThaPro_SRes_Authority_t *request = (Net_ThaPro_SRes_Authority_t*)data;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    valid_len = sizeof(s_tha_base->transaction_number);
    valid_len = valid_len > sizeof(request->body.trade_no) ? sizeof(request->body.trade_no) : valid_len;
    memset(&(s_tha_base->transaction_number), 0x00, sizeof(s_tha_base->transaction_number));
    memcpy(&(s_tha_base->transaction_number), &request->body.trade_no, sizeof(request->body.trade_no));
    s_tha_base->account_balance = request->body.account_ballance;

    valid_len = sizeof(s_tha_base->user_number);
    valid_len = valid_len > sizeof(request->body.user_id) ? sizeof(request->body.user_id) : valid_len;
    memcpy(s_tha_base->user_number, &(request->body.user_id), valid_len);
#endif /* NET_THA_AS_MONITOR */
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_vin_authority_response
 * 功能          处理服务器响应的VIN码权限认证
 * **********************************************/
int8_t tha_message_pro_vin_authority_response(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_THA_AS_MONITOR
    uint8_t data_len = sizeof(Net_ThaPro_SRes_VinAuthorize_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    uint8_t valid_len = 0x00;
    Net_ThaPro_SRes_VinAuthorize_t *request = (Net_ThaPro_SRes_VinAuthorize_t*)data;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    valid_len = sizeof(s_tha_base->transaction_number);
    valid_len = valid_len > sizeof(request->body.trade_no) ? sizeof(request->body.trade_no) : valid_len;
    memset(&(s_tha_base->transaction_number), 0x00, sizeof(s_tha_base->transaction_number));
    memcpy(&(s_tha_base->transaction_number), &request->body.trade_no, sizeof(request->body.trade_no));

    valid_len = sizeof(s_tha_base->user_number);
    valid_len = valid_len > sizeof(request->body.user_id) ? sizeof(request->body.user_id) : valid_len;
    memcpy(s_tha_base->user_number, &(request->body.user_id), valid_len);
#endif /* NET_THA_AS_MONITOR */
    return 0x00;
}

/*************************************************
 * 函数名      tha_message_pro_set_power_response
 * 功能          处理服务器的设置功率请求
 * **********************************************/
int8_t tha_message_pro_set_power_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_ThaPro_SReq_Set_Pile_ChargePower_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    Net_ThaPro_SReq_Set_Pile_ChargePower_t *request = (Net_ThaPro_SReq_Set_Pile_ChargePower_t*)data;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    rt_kprintf("tha_message_pro_set_power_request(%d, %d)\n", s_tha_base->system_power_max, request->body.charge_power);
    if(s_tha_base->system_power_max >= (request->body.charge_power *1000)){
        net_operation_set_total_power(request->body.charge_power *1000);
        s_tha_flag_info[gunno].is_set_power = 0x01;
        s_tha_base->power_strategy = APP_POWER_STRATEGY_SET_LIMIT;
        s_tha_base->power_strategy_para = 0x00;
    }else{
        return -0x03;
    }

    return 0x00;
}

/*************************************************
 * 函数名      tha_message_info_init
 * 功能          钛享报文信息初始化
 * **********************************************/
void tha_message_info_init(uint8_t gunno)  ///////// 这是网络部分外部调用的第一个函数(可以在里面进行相关初始化)
{
    static uint8_t tha_is_init = 0x00;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(tha_is_init){
        return;
    }

    uint8_t valid_len = 0x00;
    char *pile_number = NULL;
    uint32_t length = 0x00, option = (NET_SYSTEM_DATA_OPTION_PLAT_THA |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    s_tha_handle = net_get_net_handle();
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));
    pile_number = (char*)(s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));

    tha_is_init = 0x01;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_tha_charge_data_count[gunno] = rt_tick_get();
        s_tha_charge_data_interval[gunno] = THA_CHARGE_DATA_INTERVAL_INIT;
    }

    memset(&s_tha_flag_info, 0x00, sizeof(s_tha_flag_info));

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        /** 初始化登录签到 */
        g_tha_preq_login[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_login[gunno].head.cmd = NET_THA_MESSAGE_CMD_LOGIN;
        g_tha_preq_login[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
        g_tha_preq_login[gunno].head.sequence = 0x00;
        g_tha_preq_login[gunno].head.length = sizeof(Net_ThaPro_PReq_LogIn_t) - sizeof(Net_ThaPro_Head_t);
        g_tha_preq_login[gunno].head.flag = NET_THA_MESSAGE_FLAG;
        g_tha_preq_login[gunno].head.check_sum = 0x00;

        memset(g_tha_preq_login[gunno].body.pile_number, '\0', sizeof(g_tha_preq_login[gunno].body.pile_number));
        valid_len = sizeof(g_tha_preq_login[gunno].body.pile_number);
        valid_len = valid_len > length ? length : valid_len;
        memcpy(g_tha_preq_login[gunno].body.pile_number,  pile_number, valid_len);

        if(NET_SYSTEM_GUN_NUMBER > 0x01){
            if((valid_len + 1) < sizeof(g_tha_preq_login[gunno].body.pile_number)){
                sprintf((char*)(g_tha_preq_login[gunno].body.pile_number + valid_len), "%c", ('A' + gunno));
            }
        }
        g_tha_preq_login[gunno].body.pile_type = NET_THA_PILE_TYPE_SINGLE_GUN;
        g_tha_preq_login[gunno].body.client_mark = NET_THA_USER_PROVIDER;
        g_tha_preq_login[gunno].body.magic_number = NET_THA_USER_MAGIC_NUMBER;
        g_tha_preq_login[gunno].body.pile_info.protocol_ver = NET_THA_PROTOCOL_VERSION;
        g_tha_preq_login[gunno].body.pile_info.software_ver_main = s_tha_base->soft_ver_main;
        g_tha_preq_login[gunno].body.pile_info.software_ver_sub = s_tha_base->soft_ver_sub;
        g_tha_preq_login[gunno].body.pile_info.software_ver_revise = s_tha_base->soft_ver_revise;

        valid_len = sizeof(g_tha_preq_login[gunno].body.pile_info.software_model);
        valid_len = valid_len > strlen(NET_THA_SOFTWARE_MODEL) ? strlen(NET_THA_SOFTWARE_MODEL) : valid_len;
        memcpy(g_tha_preq_login[gunno].body.pile_info.software_model, NET_THA_SOFTWARE_MODEL, strlen(NET_THA_SOFTWARE_MODEL));
        g_tha_preq_login[gunno].body.random_sign = 0xAA;

        /** 初始化权限认证 */
        g_tha_preq_authority[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_authority[gunno].body.gunno = 0x00;
        /** 初始化上报充电数据 */
        g_tha_preq_charge_data[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_charge_data[gunno].body.gunno = 0x00;
        /** 初始化上报历史订单 */
        g_tha_preq_history_order[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_history_order[gunno].body.gunno = 0x00;
        /** 初始化上报心跳 */
        g_tha_preq_heartbeat[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_heartbeat[gunno].body.gunno = 0x00;
        /** 初始化上报电池充电状态 */
        g_tha_preq_report_battery_charge_state[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_report_battery_charge_state[gunno].body.gunno = 0x00;
        /** 初始化停止充电动作完成，上报信息 */
        g_tha_preq_stop_charge_complete[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_stop_charge_complete[gunno].body.gunno = 0x00;
        /** 初始化事件状态上报 */
        g_tha_preq_event_state[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_event_state[gunno].body.gunno = 0x00;
        /** 初始化错误信息上报 */
        g_tha_preq_error_info[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_error_info[gunno].body.gunno = 0x00;
        /** 初始化VIN码鉴权 */
        g_tha_preq_vin_authorize[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_vin_authorize[gunno].body.gunno = 0x00;
        /** 初始化上报电池信息 */
        g_tha_preq_report_battery_info[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_report_battery_info[gunno].body.gunno = 0x00;
        /** 初始化上报车辆告警信息 */
        g_tha_preq_car_warnning_info[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_car_warnning_info[gunno].body.gunno = 0x00;
        /** 初始化上报充电中充电枪的实时温度 */
        g_tha_preq_report_gun_realtemp[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_report_gun_realtemp[gunno].body.gunno = 0x00;
        /** 初始化桩运行参数 */
        g_tha_sreq_set_pile_running_para[gunno].body.heartbeat_interval = 0x3C;
        g_tha_sreq_set_pile_running_para[gunno].body.charge_data_interval = 0x3C;

        /** 初始化电池信息(扩展) */
        g_tha_preq_pres_battery_info_expand[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_pres_battery_info_expand[gunno].body.gunno = 0x00;
        /** 初始化充电数据(扩展) */
        g_tha_preq_charge_data_expand[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_charge_data_expand[gunno].body.gunno = 0x00;
        /** 初始化电池充电状态(扩展) */
        g_tha_preq_battery_state_expand[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
        g_tha_preq_battery_state_expand[gunno].body.gunno = 0x00;
        /** 初始化设备网络信息上传 */
        g_pile_request_net_info[gunno].head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
    }
    /** 初始化请求下载升级包 */
    g_tha_preq_update_pack_load.head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
    /** 初始化上报设备故障信息 */
    g_tha_preq_report_device_fault_info.head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
    /** 初始化升级包下载结果 */
    g_pile_request_updatepack_loadresult.head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
    /** 设备查询最新可用升级包软件版本号 */
    g_pres_query_available_update_pack_ver.head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
    /** 设备发起远程升级请求 */
    g_preq_start_update.head.start_code = NET_THA_MESSAGE_START_CODE_REQ;
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_card_authority
 * 功能          充电桩请求报文填报：刷卡权限认证
 * **********************************************/
int8_t tha_chargepile_request_padding_card_authority(uint8_t gunno)
{
#ifndef NET_THA_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(tha_get_socket_info()->state != THA_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }
    uint8_t valid_len = 0x00;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    if(s_tha_base->flag.card_info_is_uid){
        return -0x01;
    }

    g_tha_preq_authority[gunno].body.card_type = 0x00;
    g_tha_preq_authority[gunno].body.card_number_length = sizeof(s_tha_base->card_number);

    valid_len = sizeof(g_tha_preq_authority[gunno].body.card_number);
    valid_len = valid_len > sizeof(s_tha_base->card_number) ? sizeof(s_tha_base->card_number) : valid_len;
    memset(g_tha_preq_authority[gunno].body.card_number, 0x00, sizeof(g_tha_preq_authority[gunno].body.card_number));
    memcpy(g_tha_preq_authority[gunno].body.card_number, &(s_tha_base->card_number), valid_len);
    g_tha_preq_authority[gunno].body.card_ballance = s_tha_base->card_balance;
    g_tha_preq_authority[gunno].body.unpay_num = 0x00;

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_AUTHORIZE);
    return 0x00;
#endif /* NET_THA_AS_MONITOR */
    return -0x01;
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_charge_data
 * 功能          充电桩请求报文填报：充电数据
 * **********************************************/
void tha_chargepile_request_padding_charge_data(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    if(is_init){
        uint8_t valid_len = 0x00;
        valid_len = sizeof(g_tha_preq_charge_data[gunno].body.trade_no);
        valid_len = valid_len > sizeof(s_tha_base->transaction_number) ? sizeof(s_tha_base->transaction_number) : valid_len;
        memset(&g_tha_preq_charge_data[gunno].body.trade_no, 0x00, sizeof(g_tha_preq_charge_data[gunno].body.trade_no));
        memcpy(&g_tha_preq_charge_data[gunno].body.trade_no, &(s_tha_base->transaction_number), valid_len);
        g_tha_preq_charge_data[gunno].body.elect = 0x00;
        g_tha_preq_charge_data[gunno].body.voltage = 0x00;
        g_tha_preq_charge_data[gunno].body.current = 0x00;
        g_tha_preq_charge_data[gunno].body.charge_time = 0x00;
        g_tha_preq_charge_data[gunno].body.expand_voltage = 0x00;  // 需要注意
        memcpy(g_tha_preq_charge_data[gunno].body.vin, &(s_tha_base->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
        g_tha_preq_charge_data[gunno].head.length = sizeof(g_tha_preq_charge_data[gunno]) - sizeof(Net_ThaPro_Head_t) - sizeof(g_tha_preq_charge_data[gunno].body.expand_voltage);
    }else{
        if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA) == NET_THA_SEND_STATE_COMPLETE){
            g_tha_preq_charge_data[gunno].body.elect = s_tha_base->elect_a /10;
            g_tha_preq_charge_data[gunno].body.current = s_tha_base->current_a;
            g_tha_preq_charge_data[gunno].body.charge_time = s_tha_base->charge_time;

            if(s_tha_base->voltage_a > 65500){
                g_tha_preq_charge_data[gunno].body.voltage = 0xFFFF;
                g_tha_preq_charge_data[gunno].body.expand_voltage = s_tha_base->voltage_a;
                g_tha_preq_charge_data[gunno].body = 0x00;
                memcpy(g_tha_preq_charge_data[gunno].body.vin, &(s_tha_base->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
                g_tha_preq_charge_data[gunno].head.length = sizeof(g_tha_preq_charge_data[gunno]) - sizeof(Net_ThaPro_Head_t);
            }else{
                g_tha_preq_charge_data[gunno].body.voltage = s_tha_base->voltage_a;
                *((uint8_t*)(&g_tha_preq_charge_data[gunno].charge_time) + 4) = 0x00;
                memcpy(((uint8_t*)(&g_tha_preq_charge_data[gunno].charge_time) + 5), &(s_tha_base->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
                g_tha_preq_charge_data[gunno].head.length = sizeof(g_tha_preq_charge_data[gunno]) - sizeof(Net_ThaPro_Head_t) - sizeof(g_tha_preq_charge_data[gunno].body.expand_voltage);
            }
        }
    }
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_battery_state
 * 功能          充电桩请求报文填报：电池充电状态
 * **********************************************/
void tha_chargepile_request_padding_battery_state(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    if(is_init){
        uint8_t valid_len = 0x00;
        struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_tha_base->bms_data);

        valid_len = sizeof(g_tha_preq_report_battery_charge_state[gunno].body.trade_no);
        valid_len = valid_len > sizeof(s_tha_base->transaction_number) ? sizeof(s_tha_base->transaction_number) : valid_len;
        memset(&g_tha_preq_report_battery_charge_state[gunno].body.trade_no, 0x00, sizeof(g_tha_preq_report_battery_charge_state[gunno].body.trade_no));
        memcpy(&g_tha_preq_report_battery_charge_state[gunno].body.trade_no, &(s_tha_base->transaction_number), valid_len);
        g_tha_preq_report_battery_charge_state[gunno].body.charge_mode = bms->BCL.ChagModel;
        g_tha_preq_report_battery_charge_state[gunno].body.remain_time = bms->BCS.SurplChgTime;
        g_tha_preq_report_battery_charge_state[gunno].body.soc = bms->BCS.SOC;
        g_tha_preq_report_battery_charge_state[gunno].body.battery_temp = bms->BSM.HigTemp;
        g_tha_preq_report_battery_charge_state[gunno].body.single_voltage_max = bms->BCS.CellHigVolt;
        memcpy(g_tha_preq_report_battery_charge_state[gunno].body.vin, &(s_tha_base->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
    }else{
        if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE) == NET_THA_SEND_STATE_COMPLETE){
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_tha_base->bms_data);

            g_tha_preq_report_battery_charge_state[gunno].body.charge_mode = bms->BCL.ChagModel;
            g_tha_preq_report_battery_charge_state[gunno].body.remain_time = bms->BCS.SurplChgTime;
            g_tha_preq_report_battery_charge_state[gunno].body.soc = bms->BCS.SOC;
            g_tha_preq_report_battery_charge_state[gunno].body.battery_temp = bms->BSM.HigTemp;
            g_tha_preq_report_battery_charge_state[gunno].body.single_voltage_max = bms->BCS.CellHigVolt;
        }
    }
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_history_bill
 * 功能          充电桩请求报文填报：历史交易账单
 * **********************************************/
void tha_chargepile_request_padding_history_bill(uint8_t gunno, void *transaction)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t valid_len = 0x00;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)transaction;

    tha_set_transaction_verify_state(gunno, 0x00);

    valid_len = sizeof(_transaction->serial_number);
    valid_len = valid_len > sizeof(g_tha_preq_history_order[gunno].body.trade_no) ? sizeof(g_tha_preq_history_order[gunno].body.trade_no) : valid_len;
    memset(&g_tha_preq_history_order[gunno].body.trade_no, 0x00, sizeof(g_tha_preq_history_order[gunno].body.trade_no));
    memcpy(&g_tha_preq_history_order[gunno].body.trade_no, _transaction->serial_number, valid_len);
    g_tha_preq_history_order[gunno].body.elect = _transaction->total_elect;
    g_tha_preq_history_order[gunno].body.start_timestamp = _transaction->start_time;
    g_tha_preq_history_order[gunno].body.stop_timestamp = _transaction->end_time;
    g_tha_preq_history_order[gunno].body.charge_time = _transaction->charge_time;
    g_tha_preq_history_order[gunno].body.charge_fee = _transaction->total_fee;
    g_tha_preq_history_order[gunno].body.is_pay_by_card = 0x00;

    if(_transaction->start_type == APP_CHARGE_START_WAY_APP){
        g_tha_preq_history_order[gunno].body.user_mark_type = 0x01;
        g_tha_preq_history_order[gunno].body.user_mark_type_length = sizeof(g_tha_preq_history_order[gunno].body.user_mark.user_id);
        memset(&(g_tha_preq_history_order[gunno].body.user_mark), 0x00, sizeof(g_tha_preq_history_order[gunno].body.user_mark));

        valid_len = sizeof(_transaction->user_number);
        valid_len = valid_len > sizeof( g_tha_preq_history_order[gunno].body.user_mark.user_id) ?  \
                sizeof( g_tha_preq_history_order[gunno].body.user_mark.user_id) : valid_len;
        memcpy(&(g_tha_preq_history_order[gunno].body.user_mark.user_id),_transaction->user_number, valid_len);
    }else if(_transaction->start_type == APP_CHARGE_START_WAY_VIN){
        g_tha_preq_history_order[gunno].body.user_mark_type = 0x03;
        g_tha_preq_history_order[gunno].body.user_mark_type_length = sizeof(g_tha_preq_history_order[gunno].body.user_mark.vin);
        memset(&(g_tha_preq_history_order[gunno].body.user_mark), 0x00, sizeof(g_tha_preq_history_order[gunno].body.user_mark));
        memcpy(g_tha_preq_history_order[gunno].body.user_mark.vin, &(_transaction->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
    }else{
        g_tha_preq_history_order[gunno].body.user_mark_type = 0x02;
        g_tha_preq_history_order[gunno].body.user_mark_type_length = sizeof(g_tha_preq_history_order[gunno].body.user_mark.card_number);
        memset(&(g_tha_preq_history_order[gunno].body.user_mark), 0x00, sizeof(g_tha_preq_history_order[gunno].body.user_mark));
        valid_len = sizeof(g_tha_preq_history_order[gunno].body.user_mark.card_number);
        valid_len = valid_len > sizeof(_transaction->logic_card_number) ? sizeof(_transaction->logic_card_number) : valid_len;
        memcpy(g_tha_preq_history_order[gunno].body.user_mark.card_number, &(_transaction->logic_card_number), valid_len);
    }
    memcpy(g_tha_preq_history_order[gunno].body.vin, &(_transaction->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER);
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_transaction_bill
 * 功能          充电桩请求报文填报：交易账单
 * **********************************************/
void tha_chargepile_request_padding_transaction_bill(uint8_t gunno, void *transaction)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t valid_len = 0x00;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)transaction;

    tha_set_transaction_verify_state(gunno, 0x00);

    valid_len = sizeof(_transaction->serial_number);
    valid_len = valid_len > sizeof(g_tha_preq_stop_charge_complete[gunno].body.trade_no) ? sizeof(g_tha_preq_stop_charge_complete[gunno].body.trade_no) : valid_len;
    memset(&g_tha_preq_stop_charge_complete[gunno].body.trade_no, 0x00, sizeof(g_tha_preq_stop_charge_complete[gunno].body.trade_no));
    memcpy(&g_tha_preq_stop_charge_complete[gunno].body.trade_no, _transaction->serial_number, valid_len);
    g_tha_preq_stop_charge_complete[gunno].body.elect = _transaction->total_elect;
    g_tha_preq_stop_charge_complete[gunno].body.start_timestamp = _transaction->start_time;
    g_tha_preq_stop_charge_complete[gunno].body.stop_timestamp = _transaction->end_time;
    g_tha_preq_stop_charge_complete[gunno].body.charge_time = _transaction->charge_time;
    g_tha_preq_stop_charge_complete[gunno].body.charge_fee = _transaction->total_fee;
    g_tha_preq_stop_charge_complete[gunno].body.is_pay_by_card = 0x00;

    if(_transaction->start_type == APP_CHARGE_START_WAY_APP){
        g_tha_preq_stop_charge_complete[gunno].body.user_mark_type = 0x01;
        g_tha_preq_stop_charge_complete[gunno].body.user_mark_type_length = sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark.user_id);
        memset(&(g_tha_preq_stop_charge_complete[gunno].body.user_mark), 0x00, sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark));

        valid_len = sizeof(_transaction->user_number);
        valid_len = valid_len > sizeof( g_tha_preq_stop_charge_complete[gunno].body.user_mark.user_id) ?  \
                sizeof( g_tha_preq_stop_charge_complete[gunno].body.user_mark.user_id) : valid_len;
        memcpy(&(g_tha_preq_stop_charge_complete[gunno].body.user_mark.user_id),_transaction->user_number, valid_len);
    }else if(_transaction->start_type == APP_CHARGE_START_WAY_VIN){
        g_tha_preq_stop_charge_complete[gunno].body.user_mark_type = 0x03;
        g_tha_preq_stop_charge_complete[gunno].body.user_mark_type_length = sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark.vin);
        memset(&(g_tha_preq_stop_charge_complete[gunno].body.user_mark), 0x00, sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark));
        memcpy(g_tha_preq_stop_charge_complete[gunno].body.user_mark.vin, &(_transaction->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
    }else{
        g_tha_preq_stop_charge_complete[gunno].body.user_mark_type = 0x02;
        g_tha_preq_stop_charge_complete[gunno].body.user_mark_type_length = sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark.card_number);
        memset(&(g_tha_preq_stop_charge_complete[gunno].body.user_mark), 0x00, sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark));
        valid_len = sizeof(g_tha_preq_stop_charge_complete[gunno].body.user_mark.card_number);
        valid_len = valid_len > sizeof(_transaction->logic_card_number) ? sizeof(_transaction->logic_card_number) : valid_len;
        memcpy(g_tha_preq_stop_charge_complete[gunno].body.user_mark.card_number, &(_transaction->logic_card_number), valid_len);
    }
    g_tha_preq_stop_charge_complete[gunno].body.bms_stop_reason.reach_target_soc = _transaction->bms_stop_reason.target_soc;
    g_tha_preq_stop_charge_complete[gunno].body.bms_stop_reason.reach_set_value_volt = _transaction->bms_stop_reason.target_tvolt;
    g_tha_preq_stop_charge_complete[gunno].body.bms_stop_reason.reach_set_value_single_volt = _transaction->bms_stop_reason.target_svolt;
    g_tha_preq_stop_charge_complete[gunno].body.bms_stop_reason.charger_stop = _transaction->bms_stop_reason.chargerend;

    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.insultion = _transaction->bms_fault_reason.insultion;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.olinker_overtemp = _transaction->bms_fault_reason.olink_ot;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.bmsunit_olinker_overtemp = _transaction->bms_fault_reason.bms_comp_olink_ot;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.charge_linker = _transaction->bms_fault_reason.clink_fault;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.battery_group_overtemp = _transaction->bms_fault_reason.battery_ot;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.hv_relay = _transaction->bms_fault_reason.hv_relay;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.volt_detect_point_2 = _transaction->bms_fault_reason.detect_point2_volt;
    g_tha_preq_stop_charge_complete[gunno].body.bms_fault_reason.other = _transaction->bms_fault_reason.other;

    memcpy(g_tha_preq_stop_charge_complete[gunno].body.vin, &(_transaction->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);
    g_tha_preq_stop_charge_complete[gunno].body.finish_reason = tha_chargepile_stop_reason_converted(_transaction->stop_reason);

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE);
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_vin_authority
 * 功能          充电桩请求报文填报：VIN码权限认证
 * **********************************************/
int8_t tha_chargepile_request_padding_vin_authority(uint8_t gunno)
{
#ifndef NET_THA_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(tha_get_socket_info()->state != THA_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }

    uint8_t valid_len = 0x00;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    g_tha_preq_vin_authorize[gunno].body.vin_state = 0x01;

    valid_len = sizeof(g_tha_preq_vin_authorize[gunno].body.vin);
    valid_len = valid_len > sizeof(s_tha_base->car_vin) ? sizeof(s_tha_base->car_vin) : valid_len;
    memset(g_tha_preq_vin_authorize[gunno].body.vin, 0x00, sizeof(g_tha_preq_vin_authorize[gunno].body.vin));
    memcpy(g_tha_preq_vin_authorize[gunno].body.vin, &(s_tha_base->car_vin), valid_len);
    g_tha_preq_vin_authorize[gunno].body.vin_length = valid_len;

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_VIN_AUTHORIZE);
    return 0x00;
#endif /* NET_THA_AS_MONITOR */
    return -0x01;
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_net_info
 * 功能          充电桩请求报文填报：网络信息
 * **********************************************/
void tha_chargepile_request_padding_net_info(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    uint8_t valid_len = 0x00, *data;
    uint32_t length = 0x00, option = (NET_SYSTEM_DATA_OPTION_PLAT_THA |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_NETWORKED_WAY, NULL, 0x00, option));
    g_pile_request_net_info[gunno].body.networked_way = *data;                          /* 联网方式 */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_MCC, NULL, 0x00, option));
    g_pile_request_net_info[gunno].body.mcc = *((uint16_t*)data);                       /* 国家码 */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_MNC, NULL, 0x00, option));
    g_pile_request_net_info[gunno].body.mnc = *data;                                    /*  */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_LAC, NULL, 0x00, option));
    g_pile_request_net_info[gunno].body.lac = *((uint16_t*)data);                       /*  */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_CI, NULL, 0x00, option));
    g_pile_request_net_info[gunno].body.ci = *((uint16_t*)data);                        /*  */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_IMEI, NULL, 0x00, option));
    valid_len = sizeof(g_pile_request_net_info[gunno].body.imei);
    valid_len = valid_len > length ? length : valid_len;
    memset(g_pile_request_net_info[gunno].body.imei, '\0', sizeof(g_pile_request_net_info[gunno].body.imei));
    memcpy(g_pile_request_net_info[gunno].body.imei, data, valid_len);                  /* 移动设备身份码 */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_RAM, NULL, 0x00, option));
    valid_len = sizeof(g_pile_request_net_info[gunno].body.ram);
    valid_len = valid_len > length ? length : valid_len;
    memset(g_pile_request_net_info[gunno].body.ram, '\0', sizeof(g_pile_request_net_info[gunno].body.ram));
    memcpy(g_pile_request_net_info[gunno].body.ram, data, sizeof(g_pile_request_net_info[gunno].body.ram)); /* 充电设备RAM大小 */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_CI, NULL, 0x00, option));
    valid_len = sizeof(g_pile_request_net_info[gunno].body.rom);
    valid_len = valid_len > length ? length : valid_len;
    memset(g_pile_request_net_info[gunno].body.rom, '\0', sizeof(g_pile_request_net_info[gunno].body.rom));
    memcpy(g_pile_request_net_info[gunno].body.rom, data, sizeof(g_pile_request_net_info[gunno].body.rom)); /* 充电设备ROM大小 */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_CI, NULL, 0x00, option));
    valid_len = sizeof(g_pile_request_net_info[gunno].body.mac);
    valid_len = valid_len > length ? length : valid_len;
    memset(g_pile_request_net_info[gunno].body.mac, '\0', sizeof(g_pile_request_net_info[gunno].body.mac));
    memcpy(g_pile_request_net_info[gunno].body.mac, data, sizeof(g_pile_request_net_info[gunno].body.mac));

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_ICCID, NULL, 0x00, option));
    valid_len = sizeof(g_pile_request_net_info[gunno].body.iccid);
    valid_len = valid_len > strlen((char*)data) ? strlen((char*)data) : valid_len;
    memset(g_pile_request_net_info[gunno].body.iccid, '\0', sizeof(g_pile_request_net_info[gunno].body.iccid));
    memcpy(g_pile_request_net_info[gunno].body.iccid, data, valid_len);                             /* SIM卡卡号 */

    data = (s_tha_handle->get_system_data(NET_SYSTEM_DATA_NAME_IMSI, NULL, 0x00, option));
    valid_len = sizeof(g_pile_request_net_info[gunno].body.imsi);
    valid_len = valid_len > strlen((char*)data) ? strlen((char*)data) : valid_len;
    memset(g_pile_request_net_info[gunno].body.imsi, '\0', sizeof(g_pile_request_net_info[gunno].body.imsi));
    memcpy(g_pile_request_net_info[gunno].body.imsi, data, valid_len);                              /*  */

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_DEVICE_NET_INFO);
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_battery_info
 * 功能          充电桩请求报文填报：电池信息
 * **********************************************/
void tha_chargepile_request_padding_battery_info(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    struct thaisenBMS_Charger_struct *bms = NULL;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));
    bms = (struct thaisenBMS_Charger_struct*)(s_tha_base->bms_data);

    g_tha_preq_report_battery_info[gunno].body.battery_type = bms->BRM.BatType;
    g_tha_preq_report_battery_info[gunno].body.allow_temp_max = bms->BCP.BatAlowHigTemp;
    g_tha_preq_report_battery_info[gunno].body.allow_chargevolt_max_ofbms = bms->BCP.BatAlowHigVolt;
    g_tha_preq_report_battery_info[gunno].body.allow_chargevolt_max_ofbsingle = bms->BCP.CellAlowHigVolt;
    g_tha_preq_report_battery_info[gunno].body.allow_chargecurr_max = bms->BCP.AlowCurlt;
    g_tha_preq_report_battery_info[gunno].body.battery_ratedvolt_total = bms->BRM.BatRateVolt;
    g_tha_preq_report_battery_info[gunno].body.battery_voltage_current = bms->BCP.BatVolt;
    g_tha_preq_report_battery_info[gunno].body.battery_rated_capacity = bms->BRM.BatRateCap;
    g_tha_preq_report_battery_info[gunno].body.battery_nominal_power = bms->BRM.BatRateCap;
    memcpy(g_tha_preq_report_battery_info[gunno].body.vin, &(s_tha_base->car_vin), NET_THA_CAR_VIN_NUMBER_LENGTH_MAX);

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO);
}

/*************************************************
 * 函数名      tha_chargepile_request_padding_gun_realtime_temp
 * 功能          充电桩请求报文填报：充电过程中实时温度
 * **********************************************/
void tha_chargepile_request_padding_gun_realtime_temp(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    g_tha_preq_report_gun_realtemp[gunno].body.real_temp = s_tha_base->gunline_process_temp[1];
    if(s_tha_base->gunline_process_temp[0] > s_tha_base->gunline_process_temp[1]){
        g_tha_preq_report_gun_realtemp[gunno].body.real_temp = s_tha_base->gunline_process_temp[0];
    }
    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_GUN_REALTEMP);
}

/*************************************************
 * 函数名      tha_start_charge_response_asynchronously
 * 功能          充电桩报文响应事件：启动充电异步响应
 * **********************************************/
void tha_start_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_tha_flag_info[gunno].is_start_charge == 0x00){
        return;
    }
    if(result){
        s_tha_flag_info[gunno].start_success = 0x01;
    }else{
        s_tha_flag_info[gunno].start_success = 0x00;
    }
    s_tha_flag_info[gunno].is_start_charge = 0x00;
    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      tha_stop_charge_response_asynchronously
 * 功能          充电桩报文响应事件：停止充电异步响应
 * **********************************************/
void tha_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_tha_flag_info[gunno].is_stop_charge == 0x00){
        return;
    }
    if(result){
        s_tha_flag_info[gunno].stop_success = 0x01;
    }else{
        s_tha_flag_info[gunno].stop_success = 0x00;
    }
    s_tha_flag_info[gunno].is_stop_charge = 0x00;
    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      tha_set_power_percent_response_asynchronously
 * 功能          充电桩报文响应事件：设置功率百分比响应
 * **********************************************/
void tha_set_power_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_tha_flag_info[gunno].is_set_power == 0x00){
        return;
    }
    if(result){
        s_tha_flag_info[gunno].set_power_success = 0x01;
    }else{
        s_tha_flag_info[gunno].set_power_success = 0x00;
    }
    s_tha_flag_info[0x00].is_set_power = 0x00;

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_SET_POWER_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      tha_chargepile_state_changed
 * 功能          桩状态变化上报
 * **********************************************/
void tha_chargepile_state_changed(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t heartbeat_state = 0xFF;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    switch(s_tha_base->state.current){
    case APP_OFSM_STATE_IDLEING:
        heartbeat_state = 0x00;
        break;
    case APP_OFSM_STATE_READYING:
        heartbeat_state = 0x11;
        break;
    case APP_OFSM_STATE_STARTING:
        break;
    case APP_OFSM_STATE_CHARGING:
        heartbeat_state = 0x06;
        break;
    case APP_OFSM_STATE_STOPING:
        break;
    case APP_OFSM_STATE_FINISHING:
        heartbeat_state = 0x08;
        break;
    case APP_OFSM_STATE_FAULTING:
        heartbeat_state = 0x0F;
        break;
    default:
        break;
    }

    if(heartbeat_state == 0xFF){
        return;
    }
    s_tha_disposable_info[gunno].state.heartbeat = heartbeat_state;
    if(s_tha_disposable_info[gunno].state.heartbeat == g_tha_preq_heartbeat[gunno].body.device_state){
        return;
    }
    if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_HEARTBEAT) == NET_THA_SEND_STATE_ONGOING){
        tha_set_clear_disposable_event(gunno, THA_DISPOSABLE_EVENT_HEARTBEAT, 0x00);
        return;
    }
    tha_set_clear_disposable_event(gunno, THA_DISPOSABLE_EVENT_HEARTBEAT, 0x01);
    g_tha_preq_heartbeat[gunno].body.device_state = s_tha_disposable_info[gunno].state.heartbeat;
    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_HEARTBEAT);
}

/*************************************************
 * 函数名      tha_chargepile_event_occurred
 * 功能          桩事件发生上报
 * **********************************************/
void tha_chargepile_event_occurred(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t event_state = 0xFF;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    if(s_tha_base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        event_state = 0x03;
    }else{
        event_state = 0x09;
    }
    if(s_tha_base->state.current == APP_OFSM_STATE_CHARGING){
        event_state = 0x011;
    }

    s_tha_disposable_info[gunno].state.event = event_state;
    if(s_tha_disposable_info[gunno].state.event == g_tha_preq_event_state[gunno].body.device_event){
        return;
    }
    if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_EVENT_STATE) == NET_THA_SEND_STATE_ONGOING){
        tha_set_clear_disposable_event(gunno, THA_DISPOSABLE_EVENT_EVENT, 0x00);
        return;
    }
    tha_set_clear_disposable_event(gunno, THA_DISPOSABLE_EVENT_EVENT, 0x01);
    g_tha_preq_event_state[gunno].body.device_event = s_tha_disposable_info[gunno].state.event;
    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_EVENT_STATE);
}

void tha_chargepile_update_result_report(uint8_t result, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    g_pile_request_updatepack_loadresult.body.result = result;
    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_UPDATE_RESULT);
}

void tha_chargepile_fault_report(uint8_t gunno, char str[3], uint8_t rank, uint32_t timestamp, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(str == NULL){
        return;
    }
    if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_ERROR_INFO)){
        return;
    }
    g_tha_preq_error_info[gunno].body.device_type = 0x02;
    memcpy(g_tha_preq_error_info[gunno].body.error_code, str, strlen(str));
    g_tha_preq_error_info[gunno].body.error_mark = 0x00;
    if(is_resume){
        g_tha_preq_error_info[gunno].body.error_mark = 0x01;
    }
    g_tha_preq_error_info[gunno].body.error_type = rank;
    g_tha_preq_error_info[gunno].body.timestamp = timestamp;

    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_ERROR_INFO);
}

/*************************************************
 * 函数名      tha_chargepile_state_detect
 * 功能          桩状态检测并修正
 * **********************************************/
void tha_chargepile_state_detect(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t cstate = 0x00, cevent = 0x00;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    switch(s_tha_base->state.current){
    case APP_OFSM_STATE_IDLEING:
        cevent = 0x09;
        cstate = 0x00;
        break;
    case APP_OFSM_STATE_READYING:
        cevent = 0x03;
        cstate = 0x11;
        break;
    case APP_OFSM_STATE_STARTING:
        cevent = 0x03;
        break;
    case APP_OFSM_STATE_CHARGING:
        cevent = 0x11;
        cstate = 0x06;
        break;
    case APP_OFSM_STATE_STOPING:
        cevent = 0x03;
        cstate = 0x08;
        break;
    case APP_OFSM_STATE_FINISHING:
        cevent = 0x03;
        cstate = 0x08;
        break;
    case APP_OFSM_STATE_FAULTING:
        cstate = 0x0F;
        break;
    default:
        cevent = 0x09;
        break;
    }
    if(s_tha_disposable_info[gunno].state.heartbeat != cstate){
        tha_chargepile_state_changed(gunno);
    }
    if(s_tha_disposable_info[gunno].state.event != cevent){
        tha_chargepile_event_occurred(gunno);
    }
}

/*************************************************
 * 函数名      tha_chargepile_create_local_transaction_number
 * 功能          创建本地交易号
 * **********************************************/
int8_t tha_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
#ifndef NET_THA_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if((vector == NULL) || (len == 0x00)){
        return -0x02;
    }

    uint8_t valid_len = len;
    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));
    valid_len = valid_len > sizeof(s_tha_base->current_time) ? sizeof(s_tha_base->current_time) : valid_len;
    memset(vector, 0x00, len);
    memcpy(vector, &(s_tha_base->current_time), valid_len);
#endif /* NET_THA_AS_MONITOR */
    return 0x00;
}

/*************************************************
 * 函数名      tha_chargepile_fault_converted
 * 功能          故障转换
 * **********************************************/
uint8_t tha_chargepile_fault_converted(uint8_t bit)
{
    switch(bit){
    case APP_SYS_FAULT_SCRAM :
        return NET_GENERAL_FAULT_SCRAM;
    case APP_SYS_FAULT_CARD_READER :
        return NET_GENERAL_FAULT_CARD_READER;
    case APP_SYS_FAULT_DOOR :
        return NET_GENERAL_FAULT_DOOR;
    case APP_SYS_FAULT_AMMETER :
        return NET_GENERAL_FAULT_AMMETER;
    case APP_SYS_FAULT_CHARGE_MODULE :
        return NET_GENERAL_FAULT_CHARGE_MODULE;
    case APP_SYS_FAULT_OVER_TEMP :
        return NET_GENERAL_FAULT_OVER_TEMP;
    case APP_SYS_FAULT_OVER_VOLT :
        return NET_GENERAL_FAULT_OVER_VOLT;
    case APP_SYS_FAULT_UNDER_VOLT :
        return NET_GENERAL_FAULT_UNDER_VOLT;
    case APP_SYS_FAULT_OVER_CURR :
        return NET_GENERAL_FAULT_OVER_CURR;
    case APP_SYS_FAULT_RELAY :
        return NET_GENERAL_FAULT_MAIN_RELAY;
    case APP_SYS_FAULT_PARALLEL_RELAY :
        return NET_GENERAL_FAULT_PARALLEL_RELAY;
    case APP_SYS_FAULT_AC_RELAY :
        return NET_GENERAL_FAULT_AC_RELAY;
    case APP_SYS_FAULT_ELOCK :
        return NET_GENERAL_FAULT_ELOCK;
    case APP_SYS_FAULT_AUXPOWER :
        return NET_GENERAL_FAULT_AUXPOWER;
    case APP_SYS_FAULT_FLASH :
        return NET_GENERAL_FAULT_FLASH;
    case APP_SYS_FAULT_EEPROM :
        return NET_GENERAL_FAULT_EEPROM;
    default:
        return NET_GENERAL_FAULT_SIZE;
    }
    return NET_GENERAL_FAULT_SIZE;
}

/*************************************************
 * 函数名      tha_chargepile_time_sync_revise
 * 功能          时间同步修正
 * **********************************************/
void tha_chargepile_time_sync_revise(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    g_tha_preq_stop_charge_complete[gunno].body.start_timestamp = s_tha_base->start_time;
    g_tha_preq_stop_charge_complete[gunno].body.stop_timestamp = s_tha_base->stop_time;
}

/*************************************************
 * 函数名      tha_chargepile_stop_reason_converted
 * 功能          停充原因转换
 * **********************************************/
static uint8_t tha_chargepile_stop_reason_converted(uint8_t reason)
{
    uint8_t _reason = 0x04;

    switch(reason){
    /* 充满 */
    case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
        _reason = 0x03;
        break;
    /* 拔枪 */
    case APP_SYSTEM_STOP_WAY_PULL_GUN:
        _reason = 0x01;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = 0x05;
        break;
    /* APP */
    case APP_SYSTEM_STOP_WAY_APP_STOP:
        _reason = 0x00;
        break;
    /* 刷卡 */
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = 0x02;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        _reason = 0x07;
        break;
    /* 到达设定电量 */
    case APP_SYSTEM_STOP_WAY_REACH_ELECT:
        _reason = 0x07;
        break;
    /* 到达设定时间 */
    case APP_SYSTEM_STOP_WAY_REACH_TIME:
        _reason = 0x07;
        break;
    /* 到达设定余额 */
    case APP_SYSTEM_STOP_WAY_REACH_MONEY:
        _reason = 0x07;
        break;
    /* 充电电流异常 */
    case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
        _reason = 0x07;
        break;
    /* 达到SOC 限定值 */
    case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
        _reason = 0x07;
        break;
        /* 断电 */
    case APP_SYSTEM_STOP_WAY_POWER_OFF:
        _reason = 0x08;
        break;
    default:
        break;
    }
    return _reason;
}

/*************************************************
 * 函数名      tha_chargepile_start_way_converted
 * 功能          启动方式转换
 * **********************************************/
static uint16_t tha_chargepile_start_way_converted(uint8_t bit, uint8_t stop_in_starting)
{
    switch(bit){
    case APP_CHARGE_START_WAY_APP:
        return 0x01;
        break;
    case APP_CHARGE_START_WAY_ONLINE_CARD:
        return 0x02;
        break;
    case APP_CHARGE_START_WAY_OFFLINE_CARD:
        return 0x04;
        break;
    case APP_CHARGE_START_WAY_VIN:
        return 0x05;
        break;
    default:
        break;
    }
    return 0x04;
}

/*************************************************
 * 函数名      tha_query_transaction_verify_state
 * 功能          查询订单确认状态
 * **********************************************/
uint8_t tha_query_transaction_verify_state(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    if(tha_transaction_is_verify(gunno)){
        tha_set_transaction_verify_state(gunno, 0x00);
        return 0x01;
    }
    return 0x00;
}

static void tha_request_message_repeat(uint8_t gunno)
{
    uint8_t event = 0;
    if(tha_exist_message_wait_response(gunno, NULL)){
        for(event = 0; event < NET_THA_CHARGEPILE_PREQ_NUM; event++){
            if(tha_get_message_wait_response_timeout_state(gunno, NET_THA_WAIT_RESPONSE_TIMEOUT, event)){
                tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, event);
            }
        }
    }
}

void tha_data_realtime_process(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_tha_base = (System_BaseData*)(s_tha_handle->get_base_data(gunno));

    if(tha_get_socket_info()->state[gunno] == THA_SOCKET_STATE_LOGIN_SUCCESS){
        tha_request_message_repeat(gunno);

        if(s_tha_base->state.current == APP_OFSM_STATE_CHARGING){
            if(s_tha_charge_data_count[gunno] > rt_tick_get()){
                s_tha_charge_data_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_tha_charge_data_count[gunno]) > s_tha_charge_data_interval[gunno] *1000){
                s_tha_charge_data_count[gunno] = rt_tick_get();
                if(s_tha_charge_data_interval[gunno] == THA_CHARGE_DATA_INTERVAL_INIT){
                    s_tha_charge_data_interval[gunno] = g_tha_sreq_set_pile_running_para[gunno].body.charge_data_interval;
                }
                tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA);
            }
        }else{
            s_tha_charge_data_interval[gunno] = THA_CHARGE_DATA_INTERVAL_INIT;
        }
    }else{
        s_tha_charge_data_count[gunno] = rt_tick_get();
    }

    if((s_tha_base->state.current == APP_OFSM_STATE_STARTING) || (s_tha_base->state.current == APP_OFSM_STATE_CHARGING)){
        tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER);
        tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE);
    }
}

/*
 * 用于检测只上报一次的报文是否有漏报
 * */
void tha_disposable_message_check(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t _event = 0x00;
    tha_get_disposable_event(gunno, 0x00, &_event);
    if(_event){
        if(_event &(1 <<THA_DISPOSABLE_EVENT_EVENT)){
            if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_EVENT_STATE) == NET_THA_SEND_STATE_COMPLETE){
                g_tha_preq_event_state[gunno].body.device_event = s_tha_disposable_info[gunno].state.event;

                tha_set_clear_disposable_event(gunno, THA_DISPOSABLE_EVENT_EVENT, 0x01);
                tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_EVENT_STATE);
            }
        }
        if(_event &(1 <<THA_DISPOSABLE_EVENT_HEARTBEAT)){
            if(tha_get_message_send_state(gunno, NET_THA_PREQ_EVENT_HEARTBEAT) == NET_THA_SEND_STATE_COMPLETE){
                g_tha_preq_heartbeat[gunno].body.device_state = s_tha_disposable_info[gunno].state.heartbeat;

                tha_set_clear_disposable_event(gunno, THA_DISPOSABLE_EVENT_HEARTBEAT, 0x01);
                tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_HEARTBEAT);
            }
        }
    }
}

#endif /* NET_PACK_USING_THA */

