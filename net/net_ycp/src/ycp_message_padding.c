/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#include "ycp_message_padding.h"
#include "ycp_message_send.h"
#include "ycp_message_receive.h"

#include "app_ofsm.h"
#include "app_billing_rule.h"
#include "net_operation.h"

#ifdef NET_PACK_USING_YCP

#define YCP_DISPOSABLE_EVENT_STATE                0x00          /* 漏报事件：桩状态 */
#define YCP_DISPOSABLE_EVENT_FAULT                0x01          /* 漏报事件：故障上报 */

#define YCP_STATE_DATA_INTERVAL_INIT              0x05          /* 刚连上网时状态数据上报间隔 */
#define YCP_STATE_DATA_INTERVAL_CHARGING          0x0F          /* 充电中状态数据上报间隔  */
#define YCP_STATE_DATA_INTERVAL_IDLE              0x05 *60      /* 空闲状态数据上报间隔  */

#pragma pack(1)

struct ycp_disposable_info{
    struct{
        uint8_t state : 4;
        uint8_t connect : 2;
        uint8_t reserve : 2;
    }state;                                       /* 桩状态 */
    uint16_t fault_code;                          /* 故障码 */
    uint8_t disposable_event;
};

struct ycp_flag_info{
    uint8_t is_start_charge : 1;                  /*  已启动充电 */
    uint8_t is_stop_charge : 1;                   /*  已停止充电 */
    uint8_t is_set_power : 1;                     /*  已设置功率百分比 */

    uint8_t start_success : 1;                    /*  启机成功 */
    uint8_t stop_success : 1;                     /*  停机成功 */
    uint8_t set_power_success : 1;                /*  设置功率百分比成功 */
};

#pragma pack()

static struct ycp_flag_info s_ycp_flag_info[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_ycp_state_data_interval[NET_SYSTEM_GUN_NUMBER];
static uint32_t s_ycp_state_data_count[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_ycp_local_start_sq;
static struct ycp_disposable_info s_ycp_disposable_info[NET_SYSTEM_GUN_NUMBER];
static struct net_handle* s_ycp_handle = NULL;
static System_BaseData *s_ycp_base = NULL;

static uint16_t ycp_chargepile_stop_reason_converted(uint8_t bit, uint8_t stop_in_starting);
static uint8_t ycp_chargepile_transaction_identity_converted(uint8_t identity);

/*************************************************
 * 函数名      ycp_set_clear_disposable_event
 * 功能          设置漏报报文事件
 * **********************************************/
static void ycp_set_clear_disposable_event(uint8_t gunno, uint8_t event, uint8_t is_clear)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(is_clear){
        s_ycp_disposable_info[gunno].disposable_event &= (~(1 <<event));
    }else{
        s_ycp_disposable_info[gunno].disposable_event |= (1 <<event);
    }
}

/*************************************************
 * 函数名      ycp_get_disposable_event
 * 功能          获取漏报报文事件
 * **********************************************/
static uint8_t ycp_get_disposable_event(uint8_t gunno, uint8_t event, uint8_t *buf)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(buf){
        *buf = s_ycp_disposable_info[gunno].disposable_event;
    }
    if(s_ycp_disposable_info[gunno].disposable_event &(1 <<event)){
        return 0x01;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_get_start_charge_result
 * 功能          启动充电成功
 * **********************************************/
uint8_t ycp_is_start_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ycp_flag_info[gunno].start_success;
}

/*************************************************
 * 函数名      ycp_is_stop_charge_success
 * 功能          停止充电成功
 * **********************************************/
uint8_t ycp_is_stop_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ycp_flag_info[gunno].stop_success;
}

/*************************************************
 * 函数名      ycp_is_set_power_success
 * 功能          设置功率成功
 * **********************************************/
uint8_t ycp_is_set_power_success(void)
{
    return s_ycp_flag_info[0x00].set_power_success;
}

/*************************************************
 * 函数名      ycp_response_padding_query_state_data
 * 功能          组包：查询单枪状态数据响应
 * **********************************************/
int8_t ycp_response_padding_query_state_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint8_t length = 0x00;
    Net_YcpPro_PRes_Query_PReq_Report_PileState_t *response = NULL;

    response = ((Net_YcpPro_PRes_Query_PReq_Report_PileState_t*)buf);
    if(g_ycp_sreq_query_device_state[gunno].body.data_type == 0x01){
        length = (uint8_t)((uint32_t)&g_ycp_preq_report_state_data[gunno].body.plug_gun - (uint32_t)&g_ycp_preq_report_state_data[gunno]) + \
                0x01 + NET_YCP_PROTOCOL_CHECK_REGION_SIZE;
        if(length > ilen){
            return -0x02;
        }
        memset(response, 0x00, length);
        memcpy(response, &g_ycp_sreq_query_device_state[gunno], length);
    }else{
        length = sizeof(Net_YcpPro_PRes_Query_PReq_Report_PileState_t);
        if(length > ilen){
            return -0x02;
        }
        memset(response, 0x00, length);
        memcpy(response, &g_ycp_sreq_query_device_state[gunno], length);
    }

    if(olen){
        *olen = length;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_query_state_data_all
 * 功能          组包：查询所有枪状态数据响应
 * **********************************************/
int8_t ycp_response_padding_query_state_data_all(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }

    uint8_t length = 0x00;
    Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t *response = NULL;

    response = ((Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t*)buf);
    if(g_ycp_sreq_query_device_state_all.body.data_type == 0x01){
        length = ((uint8_t)((uint32_t)&g_ycp_preq_report_state_data[0x00].body.gunno - (uint32_t)&g_ycp_preq_report_state_data[0x00]) + \
                 NET_YCP_PROTOCOL_CHECK_REGION_SIZE + NET_SYSTEM_GUN_NUMBER *sizeof(struct gun_state));
        if(length > ilen){
            return -0x02;
        }
        struct gun_state *_gunno_state = (struct gun_state*)((uint8_t*)&(response->body.gun_num) + 0x01);

        memset(response, 0x00, length);
        response->body.gun_num = NET_SYSTEM_GUN_NUMBER;
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            _gunno_state[gunno].gunno = gunno;
            _gunno_state[gunno].state = g_ycp_preq_report_state_data[gunno].body.state;
            _gunno_state[gunno].homing = g_ycp_preq_report_state_data[gunno].body.homing;
            _gunno_state[gunno].plug_gun = g_ycp_preq_report_state_data[gunno].body.plug_gun;
        }
    }else{
        length = ((uint8_t)((uint32_t)&g_ycp_preq_report_state_data[0x00].body.gunno - (uint32_t)&g_ycp_preq_report_state_data[0x00]) + \
                 NET_YCP_PROTOCOL_CHECK_REGION_SIZE + NET_SYSTEM_GUN_NUMBER *(sizeof(struct gun_data) + sizeof(struct gun_state)));
        if(length > ilen){
            return -0x02;
        }
        uint8_t gun_state_begin = (uint8_t)((uint32_t)&g_ycp_preq_report_state_data[0x00].body.gunno - (uint32_t)&g_ycp_preq_report_state_data[0x00]) + 0x01;
        uint8_t data_info_begin = ((uint8_t)((uint32_t)&g_ycp_preq_report_state_data[0x00].body.gunno - (uint32_t)&g_ycp_preq_report_state_data[0x00])
                + 0x01 + sizeof(struct gun_state));
        uint8_t single_info_len = (sizeof(struct gun_data) + sizeof(struct gun_state));
        struct gun_state *_gunno_state = NULL;
        struct gun_data *_gun_data = NULL;

        memset(response, 0x00, length);
        response->body.gun_num = NET_SYSTEM_GUN_NUMBER;
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            _gunno_state = (struct gun_state*)(buf + gun_state_begin + gunno *single_info_len);
            _gun_data = (struct gun_data*)(buf + data_info_begin + gunno *single_info_len);

            _gunno_state->gunno = gunno;
            _gunno_state->state = g_ycp_preq_report_state_data[gunno].body.state;
            _gunno_state->homing = g_ycp_preq_report_state_data[gunno].body.homing;
            _gunno_state->plug_gun = g_ycp_preq_report_state_data[gunno].body.plug_gun;

            memcpy(_gun_data->serial_number, g_ycp_preq_report_state_data[gunno].body.serial_number, NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT);
            _gun_data->output_voltage = g_ycp_preq_report_state_data[gunno].body.output_voltage;
            _gun_data->output_current = g_ycp_preq_report_state_data[gunno].body.output_current;
            memcpy(_gun_data->gunline_number, g_ycp_preq_report_state_data[gunno].body.gunline_number, NET_YCP_GUNLINE_NUMBER_LENGTH_DEFAULT);
            _gun_data->gun_temperature = g_ycp_preq_report_state_data[gunno].body.gun_temperature;
            _gun_data->soc = g_ycp_preq_report_state_data[gunno].body.soc;
            _gun_data->battery_group_temp_max = g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max;
            _gun_data->charge_time = g_ycp_preq_report_state_data[gunno].body.charge_time;
            _gun_data->remain_time = g_ycp_preq_report_state_data[gunno].body.remain_time;
            _gun_data->charge_elect = g_ycp_preq_report_state_data[gunno].body.charge_elect;
            _gun_data->loss_elect = g_ycp_preq_report_state_data[gunno].body.loss_elect;
            _gun_data->consume_amount = g_ycp_preq_report_state_data[gunno].body.consume_amount;
        }
    }

    if(olen){
        *olen = length;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_general_message
 * 功能          组包：远程启机响应
 * **********************************************/
int8_t ycp_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Remote_StartCharge_t);

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
    Net_YcpPro_PRes_Remote_StartCharge_t *response = NULL;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    response = ((Net_YcpPro_PRes_Remote_StartCharge_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ycp_sreq_remote_start_charge[gunno].body.serial_number);
    valid_len = valid_len > sizeof(response->body.serial_number) ? sizeof(response->body.serial_number) : valid_len;
    memcpy(response->body.serial_number, g_ycp_sreq_remote_start_charge[gunno].body.serial_number, valid_len);

    response->body.gunno = gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_remote_stop_charge
 * 功能          组包：远程停机响应
 * **********************************************/
int8_t ycp_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Remote_StopCharge_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YcpPro_PRes_Remote_StopCharge_t *response = NULL;
    response = ((Net_YcpPro_PRes_Remote_StopCharge_t*)buf);
    memset(response, 0x00, data_len);

    response->body.gunno = gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_set_work_para
 * 功能          组包：设置工作参数响应
 * **********************************************/
int8_t ycp_response_padding_set_work_para(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_ParaSet_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_ParaSet_t *response = NULL;
    response = ((Net_YcpPro_PRes_ParaSet_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_remote_reboot
 * 功能          组包：远程重启响应
 * **********************************************/
int8_t ycp_response_padding_remote_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_RemoteReboot_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_RemoteReboot_t *response = NULL;
    response = ((Net_YcpPro_PRes_RemoteReboot_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_set_billing_model
 * 功能          组包：设置计费模型响应
 * **********************************************/
int8_t ycp_response_padding_set_billing_model(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_BillingModel_Set_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_BillingModel_Set_t *response = NULL;
    response = ((Net_YcpPro_PRes_BillingModel_Set_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_remote_update
 * 功能          组包：远程更新响应
 * **********************************************/
int8_t ycp_response_padding_remote_update(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_RemoteUpdate_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_RemoteUpdate_t *response = NULL;
    response = ((Net_YcpPro_PRes_RemoteUpdate_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_qrcode_config
 * 功能          组包：二维码配置响应
 * **********************************************/
int8_t ycp_response_padding_qrcode_config(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Qrcode_Config_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YcpPro_PRes_Qrcode_Config_t *response = NULL;
    response = ((Net_YcpPro_PRes_Qrcode_Config_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_request_padding_heartbeat
 * 功能          组包：心跳请求
 * **********************************************/
void ycp_request_padding_heartbeat(void)
{
    int32_t temperature = 0x00;
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
        temperature = temperature > s_ycp_base->gunline_temperature[0x00] ? temperature : s_ycp_base->gunline_temperature[0x00];
        temperature = temperature > s_ycp_base->gunline_temperature[0x01] ? temperature : s_ycp_base->gunline_temperature[0x01];
    }
    g_ycp_preq_heartbeat.body.signal_value = 0x16;
    g_ycp_preq_heartbeat.body.state = 0x00;
    g_ycp_preq_heartbeat.body.temp = 0x00;
    if(temperature > 0x00){
        g_ycp_preq_heartbeat.body.temp = temperature /10;
    }
    g_ycp_preq_heartbeat.body.output_voltage = 0x17c;
    g_ycp_preq_heartbeat.body.output_current = 0x00;
    g_ycp_preq_heartbeat.body.input_voltage = 0x00;
    g_ycp_preq_heartbeat.body.input_current = 0x00;
}


/*************************************************
 * 函数名      ycp_message_pro_billing_model_set_response
 * 功能          处理服务器响应(下发)的计费模型
 * **********************************************/
int8_t ycp_message_pro_billing_model_set_response(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_BillingModel_Set_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE;

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YcpPro_SReq_BillingModel_Set_t *request = (Net_YcpPro_SReq_BillingModel_Set_t*)data;

    for(uint8_t _gunno = 0x00; _gunno < NET_SYSTEM_GUN_NUMBER; _gunno++){
        uint8_t gunno = _gunno;
        s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
        if((s_ycp_base->state.current == APP_OFSM_STATE_CHARGING) || (s_ycp_base->state.current == APP_OFSM_STATE_STARTING)){
            net_operation_set_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
            gunno = NET_SYSTEM_GUN_NUMBER;
        }

        rt_kprintf("gunno(%d) billingrule info\n", gunno);
        rt_kprintf("ter(%d) tsr(%d) per(%d) psr(%d) fer(%d) fsr(%d) ver(%d) vsr(%d)\n", request->body.tip_elect_rate,
                request->body.tip_service_rate, request->body.peak_elect_rate, request->body.peak_service_rate,
                request->body.flat_elect_rate, request->body.flat_service_rate, request->body.valley_elect_rate,
                request->body.valley_service_rate);

        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_SHARP, (request->body.tip_elect_rate + request->body.tip_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_PEAK, (request->body.peak_elect_rate + request->body.peak_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_FLAT, (request->body.flat_elect_rate + request->body.flat_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_VALLEY, (request->body.valley_elect_rate + request->body.valley_service_rate) /10);

        for(uint8_t period = 0x00, count = 0x00; period < APP_BILLING_RULE_PERIOD_MAX; period++, count++){
            app_billingrule_set_period_rate_number(gunno, period, request->body.rate_number[period]);
            switch(request->body.rate_number[period]){
            case APP_RATE_TYPE_SHARP :
                app_billingrule_set_period_service_price(gunno, period, request->body.tip_elect_rate /10);
                app_billingrule_set_period_elect_price(gunno, period, request->body.tip_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_PEAK :
                app_billingrule_set_period_service_price(gunno, period, request->body.peak_elect_rate /10);
                app_billingrule_set_period_elect_price(gunno, period, request->body.peak_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_FLAT :
                app_billingrule_set_period_service_price(gunno, period, request->body.flat_elect_rate /10);
                app_billingrule_set_period_elect_price(gunno, period, request->body.flat_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_VALLEY :
                app_billingrule_set_period_service_price(gunno, period, request->body.valley_elect_rate /10);
                app_billingrule_set_period_elect_price(gunno, period, request->body.valley_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            default:
                break;
            }
        }
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_pro_apply_charge_active
 * 功能          处理服务器响应的主动申请充电
 * **********************************************/
int8_t ycp_message_pro_apply_charge_active_response(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_YCP_AS_MONITOR
    uint8_t data_len = sizeof(Net_YcpPro_SRes_ApplyCharge_Active_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE;

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    Net_YcpPro_SRes_ApplyCharge_Active_t *request = (Net_YcpPro_SRes_ApplyCharge_Active_t*)data;

    valid_len = sizeof(request->body.logic_card_number);
    valid_len = valid_len > sizeof(s_ycp_base->card_number) ? sizeof(s_ycp_base->card_number) : valid_len;
    memset(s_ycp_base->card_number, 0x00, sizeof(s_ycp_base->card_number));
    memcpy(s_ycp_base->card_number, request->body.logic_card_number, valid_len);
    memset(s_ycp_base->card_uid, 0x00, sizeof(s_ycp_base->card_uid));

    valid_len = sizeof(request->body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(s_ycp_base->transaction_number, 0x00, sizeof(s_ycp_base->transaction_number));
    memcpy(s_ycp_base->transaction_number, request->body.serial_number, valid_len);

    valid_len = sizeof(s_ycp_base->card_number);
    valid_len = valid_len > sizeof(s_ycp_base->user_number) ? sizeof(s_ycp_base->user_number) : valid_len;
    memset(s_ycp_base->user_number, 0x00, sizeof(s_ycp_base->user_number));
    memcpy(s_ycp_base->user_number, s_ycp_base->card_number, valid_len);

    s_ycp_base->account_balance = 0x00;

    s_ycp_base->charge_strategy = request->body.strategy;
    s_ycp_base->charge_strategy_para = request->body.strategy_para;
    return 0x00;
#else
    return -0x01;
#endif /* NET_YCP_AS_MONITOR */
}

/*************************************************
 * 函数名      ycp_message_pro_remote_start_charge_request
 * 功能          处理服务器下发的远程启机
 * **********************************************/
int16_t ycp_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_Remote_StartCharge_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE;

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
        return NET_YCP_FAULT_CODE_BILLING;
    }

    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    Net_YcpPro_SReq_Remote_StartCharge_t *request = (Net_YcpPro_SReq_Remote_StartCharge_t*)data;

    switch(s_ycp_base->state.current){
    case APP_OFSM_STATE_IDLEING:
        return NET_YCP_FAULT_CODE_NO_GUN;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        memset(s_ycp_base->card_number, 0x00, sizeof(s_ycp_base->card_number));
        memset(s_ycp_base->card_uid, 0x00, sizeof(s_ycp_base->card_uid));
        memset(s_ycp_base->user_number, 0x00, sizeof(s_ycp_base->user_number));

        valid_len = sizeof(request->body.serial_number);
        valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
        memset(s_ycp_base->transaction_number, 0x00, sizeof(s_ycp_base->transaction_number));
        memcpy(s_ycp_base->transaction_number, request->body.serial_number, valid_len);

        s_ycp_base->account_balance = 0x00;

        s_ycp_base->charge_strategy = request->body.strategy;
        s_ycp_base->charge_strategy_para = request->body.strategy_para;

        s_ycp_flag_info[gunno].is_start_charge = 0x01;

        return 0x00;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        return NET_YCP_FAULT_CODE_CHARGING;
        break;
    default:
        return NET_YCP_FAULT_CODE_NO_GUN;
        break;
    }
    return NET_YCP_FAULT_CODE_NO_GUN;
}

/*************************************************
 * 函数名      ycp_message_pro_remote_stop_charge_request
 * 功能          处理服务器下发的远程停机
 * **********************************************/
int16_t ycp_message_pro_remote_stop_charge_request(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }

    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(s_ycp_base->state.current != APP_OFSM_STATE_CHARGING){
        s_ycp_flag_info[gunno].is_stop_charge = 0x01;
        return 0x00;
    }
    return NET_YCP_FAULT_CODE_NO_CHARGE;
}

/*************************************************
 * 函数名      ycp_message_pro_set_para_request
 * 功能          处理服务器下发的设置桩参数请求
 * **********************************************/
int8_t ycp_message_pro_set_para_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_ParaSet_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE;

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YcpPro_SReq_ParaSet_t *request = (Net_YcpPro_SReq_ParaSet_t*)data;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(0x00));

    rt_kprintf("ycp_message_pro_set_work_para_request(%d, %d, %d)\n", request->body.power_percent,
            s_ycp_base->system_power_max, (s_ycp_base->system_power_max * request->body.power_percent /100));
    if((request->body.power_percent > 0x00) && (request->body.power_percent <= 0x64)){
        uint32_t power = (s_ycp_base->system_power_max * request->body.power_percent /100);
        net_operation_set_total_power(power);
        s_ycp_flag_info[0x00].is_set_power = 0x01;
    }else{
        return -0x03;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_pro_time_sync_response
 * 功能          处理服务器的对时响应
 * **********************************************/
int8_t ycp_message_pro_time_sync_response(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SRes_TimeSync_t);
    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YcpPro_SRes_TimeSync_t *response = (Net_YcpPro_SRes_TimeSync_t*)data;
    uint32_t timestamp = ycp_timebcd_to_timestamp(response->body.current_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
    s_ycp_handle->time_sync(timestamp);

    rt_kprintf("ycp_message_pro_time_sync_request(%d)[%x, %x, %x, %x, %x, %x, %x]\n", timestamp,
            response->body.current_time[0x06], response->body.current_time[0x05],
            response->body.current_time[0x04], response->body.current_time[0x03],
            response->body.current_time[0x02], response->body.current_time[0x01],
            response->body.current_time[0x00]);

    net_operation_set_event(0x00, NET_OPERATION_EVENT_TIME_SYNC);

    return 0x00;
}





/*************************************************
 * 函数名      ycp_message_pro_remote_reset_request
 * 功能          处理服务器的远程重启请求
 * **********************************************/
int8_t ycp_message_pro_remote_reset_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_RemoteReboot_t);

    if(data_len > len){
        return -0x02;
    }

    uint8_t gunno = 0x00;
    Net_YcpPro_SReq_RemoteReboot_t *request = (Net_YcpPro_SReq_RemoteReboot_t*)data;

    if(request->body.control_cmd == 0x01){
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            if(s_ycp_base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
    }else if(request->body.control_cmd == 0x02){
        return 0x01;
    }

    if(gunno == NET_SYSTEM_GUN_NUMBER){
        return 0x01;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_info_init
 * 功能          越城报文信息初始化
 * **********************************************/
void ycp_message_info_init(void)  ///////// 这是网络部分外部调用的第一个函数(可以在里面进行相关初始化)
{
    static uint8_t ycp_is_init = 0x00;
    if(ycp_is_init){
        return;
    }

    s_ycp_handle = net_get_net_handle();
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(0x00));
    ycp_is_init = 0x01;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ycp_state_data_interval[gunno] = YCP_STATE_DATA_INTERVAL_INIT;
        s_ycp_state_data_count[gunno] = rt_tick_get();
    }
    memset(&s_ycp_flag_info, 0x00, sizeof(s_ycp_flag_info));

    /** 初始化登录签到 */
    g_ycp_preq_login.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
    g_ycp_preq_login.head.sequence = 0x00;

    g_ycp_preq_login.body.pile_type = NET_YCP_PILE_TYPE_DC;
    g_ycp_preq_login.body.gun_count = NET_SYSTEM_GUN_NUMBER;
    g_ycp_preq_login.body.protocol_ver[0x00] = NET_YCP_PROTOCOL_VERSION_MAIN;
    g_ycp_preq_login.body.protocol_ver[0x01] = NET_YCP_PROTOCOL_VERSION_SUB;
    g_ycp_preq_login.body.protocol_ver[0x02] = NET_YCP_PROTOCOL_VERSION_REV;

    memset(g_ycp_preq_login.body.software_ver, '\0', sizeof(g_ycp_preq_login.body.software_ver));
    g_ycp_preq_login.body.software_ver[0] = s_ycp_base->soft_ver_main + '0';
    g_ycp_preq_login.body.software_ver[1] = '.';
    g_ycp_preq_login.body.software_ver[2] = s_ycp_base->soft_ver_sub + '0';
    g_ycp_preq_login.body.software_ver[3] = '.';
    sprintf((char *)&g_ycp_preq_login.body.software_ver[3 + 1], "%2d", s_ycp_base->soft_ver_revise);

    g_ycp_preq_login.body.net_link_type = NET_YCP_NET_LINK_TYPE_SIM;
    memset(g_ycp_preq_login.body.communicate_module_sn, '\0', sizeof(g_ycp_preq_login.body.communicate_module_sn));
    memcpy(g_ycp_preq_login.body.communicate_module_sn, "EC800M", strlen("EC800M"));

    memset(g_ycp_preq_login.body.network_keys, '\0', sizeof(g_ycp_preq_login.body.network_keys));

    g_ycp_preq_login.body.transmit_type = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        /** 初始化状态数据响应(实时数据请求) */
        g_ycp_preq_report_state_data[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_report_state_data[gunno].body.gunno = gunno;

        /** 初始化充电桩主动申请启动充电请求 */
        g_ycp_preq_apply_charge_active[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_apply_charge_active[gunno].body.gunno = gunno;

        /** 初始化交易记录请求 */
        g_ycp_preq_transaction_records[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_transaction_records[gunno].body.gunno = gunno;

        /** 初始化充电握手请求 */
        g_ycp_preq_shake_hand[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_shake_hand[gunno].body.gunno = gunno;

        /** 初始化参数配置请求 */
        g_ycp_preq_parameter_config[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_parameter_config[gunno].body.gunno = gunno;

        /** 初始化充电结束请求 */
        g_ycp_preq_charge_finish[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_charge_finish[gunno].body.gunno = gunno;

        /** 初始化错误报文请求 */
        g_ycp_preq_error_message[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_error_message[gunno].body.gunno = gunno;

        /** 初始化充电过程中 BMS 终止请求 */
        g_ycp_preq_bms_end[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_bms_end[gunno].body.gunno = gunno;

        /** 初始化充电过程中充电机终止请求 */
        g_ycp_preq_charger_end[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_charger_end[gunno].body.gunno = gunno;

        /** 初始化充电过程 BMS 需求与充电机输出请求 */
        g_ycp_preq_bmscommand_chargerout[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_bmscommand_chargerout[gunno].body.gunno = gunno;

        /** 初始化充电过程 BMS 信息请求 */
        g_ycp_preq_bms_info[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_bms_info[gunno].body.gunno = gunno;
    }
    /** 初始化设备故障上报请求 */
    g_ycp_preq_report_device_fault.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    /** 初始化计费模型验证 */
    g_ycp_preq_billing_model_verify.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
    g_ycp_preq_billing_model_verify.body.model_sn = 0x00;

    /** 初始化升级结果响应 */
    g_ycp_pres_remote_update.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    /** 初始化心跳请求 */
    g_ycp_preq_heartbeat.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
    g_ycp_preq_heartbeat.body.state = 0x00;     /* 心跳状态默认正常 */
    g_ycp_preq_heartbeat.body.signal_value = 0x00;
    g_ycp_preq_heartbeat.body.temp = 250;
    g_ycp_preq_heartbeat.body.output_voltage = 0x00;
    g_ycp_preq_heartbeat.body.output_current = 0x00;
    g_ycp_preq_heartbeat.body.input_voltage = 0x00;
    g_ycp_preq_heartbeat.body.input_current = 0x00;

    /** 初始化对时 */
    g_ycp_preq_time_sync.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    /** 初始化桩参数 */
    g_ycp_sreq_set_para.body.heartbeat_interval = 0x1E;
    g_ycp_sreq_set_para.body.bmsdata_switch = 0x01;
    g_ycp_sreq_set_para.body.chargedata_type = 0x02;
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_state_data
 * 功能          充电桩请求报文填报：状态数据
 * **********************************************/
void ycp_chargepile_request_padding_state_data(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(g_ycp_sreq_set_para.body.chargedata_type == 0x01){
        g_ycp_preq_report_state_data[gunno].head.length = ((uint32_t)(g_ycp_preq_report_state_data[gunno].body.serial_number) -  \
                (uint32_t)&(g_ycp_preq_report_state_data[gunno]) + 0x01 - 0x04);
    }else{
        g_ycp_preq_report_state_data[gunno].head.length = sizeof(g_ycp_preq_report_state_data[gunno]) - 0x04;
    }
    if(is_init){
        uint8_t valid_len = sizeof(g_ycp_preq_report_state_data[gunno].body.serial_number);
        valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
        memset(g_ycp_preq_report_state_data[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_report_state_data[gunno].body.serial_number));
        memcpy(g_ycp_preq_report_state_data[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

        g_ycp_preq_report_state_data[gunno].body.homing = 0x02;
        g_ycp_preq_report_state_data[gunno].body.output_voltage = 0x00;
        g_ycp_preq_report_state_data[gunno].body.output_current = 0x00;
        g_ycp_preq_report_state_data[gunno].body.gun_temperature = 0x00;
        g_ycp_preq_report_state_data[gunno].body.soc = 0x00;
        g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max = 0x00;
        g_ycp_preq_report_state_data[gunno].body.charge_time = 0x00;
        g_ycp_preq_report_state_data[gunno].body.remain_time = 0x00;
        g_ycp_preq_report_state_data[gunno].body.charge_elect = 0x00;
        g_ycp_preq_report_state_data[gunno].body.loss_elect = 0x00;
        g_ycp_preq_report_state_data[gunno].body.consume_amount = 0x00;

        ycp_chargepile_request_padding_bmscommand_chargerout(gunno, 0x01);
        ycp_chargepile_request_padding_bmsinfo_duringcharge(gunno, 0x01);
    }else{
        if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD) == NET_YCP_SEND_STATE_COMPLETE){
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

            g_ycp_preq_report_state_data[gunno].body.output_voltage = s_ycp_base->voltage_a /10;
            g_ycp_preq_report_state_data[gunno].body.output_current = s_ycp_base->current_a /10;
            if(s_ycp_base->gunline_temperature[0] > s_ycp_base->gunline_temperature[1]){
                g_ycp_preq_report_state_data[gunno].body.gun_temperature = (s_ycp_base->gunline_temperature[0] /10 + 0x00);
            }else{
                g_ycp_preq_report_state_data[gunno].body.gun_temperature = (s_ycp_base->gunline_temperature[1] /10 + 0x00);
            }
            g_ycp_preq_report_state_data[gunno].body.soc = s_ycp_base->current_soc;
            g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max = bms->BSM.HigTemp;
            g_ycp_preq_report_state_data[gunno].body.charge_time = s_ycp_base->charge_time /60;
            g_ycp_preq_report_state_data[gunno].body.remain_time = bms->BCS.SurplChgTime;
            g_ycp_preq_report_state_data[gunno].body.charge_elect = s_ycp_base->elect_a *10;
            g_ycp_preq_report_state_data[gunno].body.loss_elect = 0x00;
            g_ycp_preq_report_state_data[gunno].body.consume_amount = s_ycp_base->fees_total;
        }
    }
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_shakehand
 * 功能          充电桩请求报文填报：BMS握手
 * **********************************************/
void ycp_chargepile_request_padding_bms_shakehand(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }

    uint16_t year;
    uint8_t valid_len = 0x00, yearh, yearl, byteh, bytel;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

    valid_len = sizeof(g_ycp_preq_shake_hand[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(g_ycp_preq_shake_hand[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_shake_hand[gunno].body.serial_number));
    memcpy(g_ycp_preq_shake_hand[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_protocol_ver, bms->BRM.BMSVer, sizeof(bms->BRM.BMSVer));
    g_ycp_preq_shake_hand[gunno].body.bms_bat_type = bms->BRM.BatType;
    g_ycp_preq_shake_hand[gunno].body.bms_bat_rated_capacity = bms->BRM.BatRateCap;
    g_ycp_preq_shake_hand[gunno].body.bms_bat_rated_volt = bms->BRM.BatRateVolt;
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_bat_maker_name, bms->BRM.BatFirm, sizeof(bms->BRM.BatFirm));
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_bat_sn, bms->BRM.SerialNum, sizeof(bms->BRM.SerialNum));

    byteh = (uint8_t)((bms->BRM.BatBuldday &0xF0) >> 0x04);
    bytel = (uint8_t)(bms->BRM.BatBuldday &0x0F);
    g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x00] = byteh *10 + bytel;

    byteh = (uint8_t)((bms->BRM.BatBuldmonth &0xF0) >> 0x04);
    bytel = (uint8_t)(bms->BRM.BatBuldmonth &0x0F);
    g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x01] = byteh *10 + bytel;

    year = bms->BRM.BatBuldyear + 1985;
    yearh = year %100;
    yearl = year /100;

    byteh = (uint8_t)((yearl &0xF0) >> 0x04);
    bytel = (uint8_t)(yearl &0x0F);
    g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x02] = byteh *10 + bytel;

    byteh = (uint8_t)((yearh &0xF0) >> 0x04);
    bytel = (uint8_t)(yearh &0x0F);
    g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x03] = byteh *10 + bytel;

    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_bat_charge_num, bms->BRM.Chagtimer, sizeof(bms->BRM.Chagtimer));
    g_ycp_preq_shake_hand[gunno].body.bms_bat_title_identification = bms->BRM.BatProperty;
    memcpy(g_ycp_preq_shake_hand[gunno].body.vin, bms->BRM.CarDiscern, NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX);
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_software_ver, bms->BRM.BMSVerNum, sizeof(bms->BRM.BMSVerNum));

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGE_SHAKE_HAND);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_paraconfig
 * 功能          充电桩请求报文填报：BMS参数配置
 * **********************************************/
void ycp_chargepile_request_padding_bms_paraconfig(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

    valid_len = sizeof(g_ycp_preq_parameter_config[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(g_ycp_preq_parameter_config[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_parameter_config[gunno].body.serial_number));
    memcpy(g_ycp_preq_parameter_config[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

    g_ycp_preq_parameter_config[gunno].body.bms_single_bat_allow_volt_max = bms->BCP.CellAlowHigVolt;
    g_ycp_preq_parameter_config[gunno].body.bms_allow_curr_max = bms->BCP.AlowCurlt;
    g_ycp_preq_parameter_config[gunno].body.bms_bat_nominal_energy_all = bms->BCP.BatRateKW;
    g_ycp_preq_parameter_config[gunno].body.bms_allow_volt_max = bms->BCP.BatAlowHigVolt;
    g_ycp_preq_parameter_config[gunno].body.bms_allow_temp_max = bms->BCP.BatAlowHigTemp;
    g_ycp_preq_parameter_config[gunno].body.soc = bms->BCP.SOC;
    g_ycp_preq_parameter_config[gunno].body.bms_bat_current_volt = bms->BCP.BatVolt;

    g_ycp_preq_parameter_config[gunno].body.pile_output_volt_max = bms->CML.ChagHigOutVolt;
    g_ycp_preq_parameter_config[gunno].body.pile_output_volt_min = bms->CML.ChagLowOutVolt;
    g_ycp_preq_parameter_config[gunno].body.pile_output_curr_max = bms->CML.ChagHigOutCurlt;
    g_ycp_preq_parameter_config[gunno].body.pile_output_curr_min = bms->CML.ChagLowOutCurlt;

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_PARA_CONFIG);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_chargeend
 * 功能          充电桩请求报文填报：BMS充电结束
 * **********************************************/
void ycp_chargepile_request_padding_bms_chargeend(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

    valid_len = sizeof(g_ycp_preq_charge_finish[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(g_ycp_preq_charge_finish[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_charge_finish[gunno].body.serial_number));
    memcpy(g_ycp_preq_charge_finish[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

    g_ycp_preq_charge_finish[gunno].body.bms_end_soc = bms->BSD.StopSOC *10;
    g_ycp_preq_charge_finish[gunno].body.bms_single_bat_volt_min = bms->BSD.CellLowVolt;
    g_ycp_preq_charge_finish[gunno].body.bms_single_bat_volt_max = bms->BSD.CellHigVolt;
    g_ycp_preq_charge_finish[gunno].body.bms_bat_temp_min = bms->BSD.LowTemp;
    g_ycp_preq_charge_finish[gunno].body.bms_bat_temp_max = bms->BSD.HigTemp;
    g_ycp_preq_charge_finish[gunno].body.total_charge_time = bms->CSD.TotalChgTime *60;
    g_ycp_preq_charge_finish[gunno].body.charge_elect = bms->CSD.OutputKWh;
    memcpy(g_ycp_preq_charge_finish[gunno].body.charger_number, bms->CSD.ChgNum, sizeof(bms->CSD.ChgNum));

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGE_END);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_error
 * 功能          充电桩请求报文填报：BMS错误报文
 * **********************************************/
void ycp_chargepile_request_padding_bms_error(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

    valid_len = sizeof(g_ycp_preq_error_message[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(g_ycp_preq_error_message[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_error_message[gunno].body.serial_number));
    memcpy(g_ycp_preq_error_message[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

    g_ycp_preq_error_message[gunno].body.timeout.spn2560_00_identify = bms->BEM.CRM00OVtime;
    g_ycp_preq_error_message[gunno].body.timeout.spn2560_aa_identify = bms->BEM.CRMAAOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_sync_and_output_max = bms->BEM.CTSCMLOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_charge_ready_ok = bms->BEM.CROOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_charge_state = bms->BEM.CCSOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_end_charge = bms->BEM.CSTOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_charge_statistics = bms->BEM.CSDOVtime;

    g_ycp_preq_error_message[gunno].body.timeout.bms_and_car = bms->CEM.BRMOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bat_parameter = bms->CEM.BCPOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bms_charge_ready_ok = bms->CEM.BROOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bat_state_all = bms->CEM.BCSOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bat_charge_conmand = bms->CEM.BCLOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bms_end_charge = bms->CEM.BSTOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bms_charge_count = bms->CEM.BSDOVtime;

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_ERROR_MESSAGE);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bmsend_duringcharge
 * 功能          充电桩请求报文填报：充电过程中 BMS 终止
 * **********************************************/
void ycp_chargepile_request_padding_bmsend_duringcharge(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

    valid_len = sizeof(g_ycp_preq_bms_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(g_ycp_preq_bms_end[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_bms_end[gunno].body.serial_number));
    memcpy(g_ycp_preq_bms_end[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

    if(bms->BST.SOCGetObj){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x01;
    }else if(bms->BST.VoltGetObj){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x02;
    }else if(bms->BST.CeliVoltGetObj){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x03;
    }else if(bms->BST.ChargInitiStop){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x04;
    }else{
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x00;
    }

    if(bms->BST.InsltFault){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.OutConectOVtemp){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.BMSCompOVtemp){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.Conectfault){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.BatOVtemp){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.HVRelaysFault){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.Check2Ft){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.OtherFt){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else{
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }

    if(bms->BST.OtherFt){
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x00;
    }else if(bms->BST.OverCurlt){
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x01;
    }else if(bms->BST.Voltfault){
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x02;
    }else{
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x00;
    }

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_BMS_STOP);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_chargerend_duringcharge
 * 功能          充电桩请求报文填报：充电过程中充电机终止
 * **********************************************/
void ycp_chargepile_request_padding_chargerend_duringcharge(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

    valid_len = sizeof(g_ycp_preq_charger_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
    memset(g_ycp_preq_charger_end[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_charger_end[gunno].body.serial_number));
    memcpy(g_ycp_preq_charger_end[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);

    if(bms->CST.AutoStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x01;
    }else if(bms->CST.ManStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x02;
    }else if(bms->CST.FaultStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x03;
    }else if(bms->CST.BMSInitiStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x04;
    }else{
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x00;
    }

    if(bms->CST.ChgOVtemp){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.ChgConectfault){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.ChgOVtempIn){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.EnergyTranFault){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.EmgcyStop){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.OtherFault){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else{
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }

    if(bms->CST.Curltfault){
        g_ycp_preq_charger_end[gunno].body.stop_error = 0x01;
    }else if(bms->CST.Voltfault){
        g_ycp_preq_charger_end[gunno].body.stop_error = 0x02;
    }else{
        g_ycp_preq_charger_end[gunno].body.stop_error = 0x00;
    }

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGER_STOP);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bmscommand_chargerout
 * 功能          充电桩请求报文填报：充电过程 BMS 需求与充电机输出
 * **********************************************/
void ycp_chargepile_request_padding_bmscommand_chargerout(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(is_init){
        uint8_t valid_len = 0x00;
        s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

        valid_len = sizeof(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number);
        valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
        memset(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number));
        memcpy(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);
    }else{
        if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
            return;
        }
        if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE) == NET_YCP_SEND_STATE_COMPLETE){
            s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_volt_command = bms->BCL.BMSneedVolt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_curr_command = bms->BCL.BMSneedCurlt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = bms->BCL.ChagModel;

            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = bms->BCS.ChargVolt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = bms->BCS.ChargCurlt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_max_single_bat_volt = bms->BCS.CellHigVolt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.max_single_bat_volt_gn = bms->BCS.HigVoltCellNum;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_current_soc = bms->BCS.SOC;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_remain_charge_time = bms->BCS.SurplChgTime;
            g_ycp_preq_bmscommand_chargerout[gunno].body.pile_output_volt = (s_ycp_base->voltage_a /10);
            g_ycp_preq_bmscommand_chargerout[gunno].body.pile_output_curr = (s_ycp_base->current_a /10);
            g_ycp_preq_bmscommand_chargerout[gunno].body.charge_time = s_ycp_base->charge_time;
        }
    }
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bmsinfo_duringcharge
 * 功能          充电桩请求报文填报：充电过程 BMS 信息
 * **********************************************/
void ycp_chargepile_request_padding_bmsinfo_duringcharge(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(is_init){
        uint8_t valid_len = 0x00;
        s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

        valid_len = sizeof(g_ycp_preq_bms_info[gunno].body.serial_number);
        valid_len = valid_len > sizeof(s_ycp_base->transaction_number) ? sizeof(s_ycp_base->transaction_number) : valid_len;
        memset(g_ycp_preq_bms_info[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_bms_info[gunno].body.serial_number));
        memcpy(g_ycp_preq_bms_info[gunno].body.serial_number, s_ycp_base->transaction_number, valid_len);
    }else{
        if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
            return;
        }
        if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_BMS_INFO) == NET_YCP_SEND_STATE_COMPLETE){
            s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ycp_base->bms_data);

            g_ycp_preq_bms_info[gunno].body.bms_max_single_volt_bat_number = bms->BSM.HigVoltCellNum;
            g_ycp_preq_bms_info[gunno].body.bms_bat_temp_max = (bms->BSM.HigTemp + 0x00);
            g_ycp_preq_bms_info[gunno].body.bat_temp_max_measure_number = bms->BSM.HigTempNum;
            g_ycp_preq_bms_info[gunno].body.bms_bat_temp_min = (bms->BSM.LowTemp + 0x00);
            g_ycp_preq_bms_info[gunno].body.bat_temp_min_measure_number = bms->BSM.LowTempNum;
            g_ycp_preq_bms_info[gunno].body.bms_single_volt = bms->BSM.CellOverVolt;
            g_ycp_preq_bms_info[gunno].body.bms_bat_soc = bms->BSM.SOCState;
            g_ycp_preq_bms_info[gunno].body.bms_bat_curr = bms->BSM.BatOverCurlt;
            g_ycp_preq_bms_info[gunno].body.bms_bat_temp = bms->BSM.BatOverTemp;
            g_ycp_preq_bms_info[gunno].body.bms_bat_isolate = bms->BSM.Insulat;
            g_ycp_preq_bms_info[gunno].body.bms_bat_output_linker = bms->BSM.OutConect;
            g_ycp_preq_bms_info[gunno].body.charge_forbid = bms->BSM.AllowChg;
        }
    }
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_card_authority
 * 功能          充电桩请求报文填报：刷卡权限认证
 * **********************************************/
int8_t ycp_chargepile_request_padding_card_authority(uint8_t gunno)
{
#ifndef NET_YCP_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }
    if(ycp_get_socket_info()->state != YCP_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    uint8_t valid_len = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(s_ycp_base->flag.card_info_is_uid){
        return -0x01;
    }
    g_ycp_preq_apply_charge_active[gunno].body.start_type = 0x01;

    valid_len = sizeof(s_ycp_base->card_number);
    valid_len = valid_len > NET_YCP_PHYCARD_NUMBER_LENGTH_MAX ? NET_YCP_PHYCARD_NUMBER_LENGTH_MAX : valid_len;
    memset(g_ycp_preq_apply_charge_active[gunno].body.phycard_number, 0x00, NET_YCP_PHYCARD_NUMBER_LENGTH_MAX);
    memcpy(g_ycp_preq_apply_charge_active[gunno].body.phycard_number, s_ycp_base->card_number, valid_len);

    memset(g_ycp_preq_apply_charge_active[gunno].body.password, 0x00, NET_YCP_PASSWORD_LENGTH_DEFAULT);

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YCP_AS_MONITOR */
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_vin_authority
 * 功能          充电桩请求报文填报：VIN 码权限认证
 * **********************************************/
int8_t ycp_chargepile_request_padding_vin_authority(uint8_t gunno)
{
#ifndef NET_YCP_AS_MONITOR
    /* 该协议暂时不支持VIN请求充电 */
    return -0x01;

    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }
    if(ycp_get_socket_info()->state != YCP_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    g_ycp_preq_apply_charge_active[gunno].body.start_type = 0x03;
    memset(g_ycp_preq_apply_charge_active[gunno].body.phycard_number, 0x00, NET_YCP_PHYCARD_NUMBER_LENGTH_MAX);
    memset(g_ycp_preq_apply_charge_active[gunno].body.password, 0x00, NET_YCP_PASSWORD_LENGTH_DEFAULT);
//    for(uint8_t count = 0x00; count < NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX; count++){
//        g_ycp_preq_apply_charge_active[gunno].body.vin[count] = s_ycp_base->car_vin[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
//    }

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YCP_AS_MONITOR */
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_transaction_record
 * 功能          充电桩请求报文填报：交易记录信息
 * **********************************************/
uint8_t ycp_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    uint8_t valid_len = 0x00;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)transaction;

    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    ycp_set_transaction_verify_state(gunno, 0x00);
    if((ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD) == NET_YCP_SEND_STATE_COMPLETE) || !is_repeat){
        valid_len = sizeof(g_ycp_preq_transaction_records[gunno].body.serial_number);
        valid_len = valid_len > sizeof(_transaction->serial_number) ? sizeof(_transaction->serial_number) : valid_len;
        memset(g_ycp_preq_transaction_records[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_transaction_records[gunno].body.serial_number));
        memcpy(g_ycp_preq_transaction_records[gunno].body.serial_number, _transaction->serial_number, valid_len);
        ycp_timestamp_to_timebcd(_transaction->start_time, g_ycp_preq_transaction_records[gunno].body.start_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
        ycp_timestamp_to_timebcd(_transaction->end_time, g_ycp_preq_transaction_records[gunno].body.end_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);

        g_ycp_preq_transaction_records[gunno].body.tip_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_SHARP];
        g_ycp_preq_transaction_records[gunno].body.tip_elect = _transaction->rate_type_elect[APP_RATE_TYPE_SHARP] *10;
        g_ycp_preq_transaction_records[gunno].body.tip_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_SHARP];

        g_ycp_preq_transaction_records[gunno].body.peak_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_PEAK];
        g_ycp_preq_transaction_records[gunno].body.peak_elect = _transaction->rate_type_elect[APP_RATE_TYPE_PEAK] *10;
        g_ycp_preq_transaction_records[gunno].body.peak_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_PEAK];

        g_ycp_preq_transaction_records[gunno].body.flat_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_FLAT];
        g_ycp_preq_transaction_records[gunno].body.flat_elect = _transaction->rate_type_elect[APP_RATE_TYPE_FLAT] *10;
        g_ycp_preq_transaction_records[gunno].body.flat_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_FLAT];

        g_ycp_preq_transaction_records[gunno].body.valley_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_VALLEY];
        g_ycp_preq_transaction_records[gunno].body.valley_elect = _transaction->rate_type_elect[APP_RATE_TYPE_VALLEY] *10;
        g_ycp_preq_transaction_records[gunno].body.valley_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_VALLEY];

        g_ycp_preq_transaction_records[gunno].body.consume_amount = _transaction->total_fee;

        valid_len = sizeof(_transaction->car_vin);
        valid_len = valid_len > NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX ? NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX : valid_len;
        memcpy(g_ycp_preq_transaction_records[gunno].body.vin, &(_transaction->car_vin), valid_len);

        g_ycp_preq_transaction_records[gunno].body.pay_way = 0x00;        /* 待确认 *///////////////////////////////////
        g_ycp_preq_transaction_records[gunno].body.stop_reason = 0x00;    /* 待确认 *///////////////////////////////////

        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
        return 0x01;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_start_charge_response_asynchronously
 * 功能          充电桩报文响应事件：启动充电异步响应
 * **********************************************/
void ycp_start_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_ycp_flag_info[gunno].is_start_charge == 0x00){
        return;
    }
    if(result){
        s_ycp_flag_info[gunno].start_success = 0x01;
    }else{
        s_ycp_flag_info[gunno].start_success = 0x00;
    }

    s_ycp_flag_info[gunno].is_start_charge = 0x00;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ycp_stop_charge_response_asynchronously
 * 功能          充电桩报文响应事件：停止充电异步响应
 * **********************************************/
void ycp_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_ycp_flag_info[gunno].is_stop_charge == 0x00){
        return;
    }
    if(result){
        s_ycp_flag_info[gunno].stop_success = 0x01;
    }else{
        s_ycp_flag_info[gunno].stop_success = 0x00;
    }

    s_ycp_flag_info[gunno].is_stop_charge = 0x00;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ycp_set_power_percent_response_asynchronously
 * 功能          充电桩报文响应事件：设置功率百分比响应
 * **********************************************/
void ycp_set_power_percent_response_asynchronously(uint8_t result)
{
    if(s_ycp_flag_info[0x00].is_set_power == 0x00){
        return;
    }
    if(result){
        s_ycp_flag_info[0x00].set_power_success = 0x01;
    }else{
        s_ycp_flag_info[0x00].set_power_success = 0x00;
    }

    s_ycp_flag_info[0x00].is_set_power = 0x00;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY);
}

void ycp_chargepile_state_changed(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t state = 0xFF;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(s_ycp_base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        s_ycp_disposable_info[gunno].state.connect = 0x01;
    }else{
        s_ycp_disposable_info[gunno].state.connect = 0x00;
    }

    switch(s_ycp_base->state.current){
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_STARTING:
        state = 0x02;
        break;
    case APP_OFSM_STATE_CHARGING:
        state = 0x03;
        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        state = 0x02;
        break;
    case APP_OFSM_STATE_FAULTING:
        state = 0x01;
        break;
    default:
        break;
    }

    if(state == 0xFF){
        return;
    }

    s_ycp_disposable_info[gunno].state.state = state;
    if((s_ycp_disposable_info[gunno].state.state == g_ycp_preq_report_state_data[gunno].body.state) &&
            (s_ycp_disposable_info[gunno].state.connect == g_ycp_preq_report_state_data[gunno].body.plug_gun)){
        return;
    }
    if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA) == NET_YCP_SEND_STATE_ONGOING){
        ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_STATE, 0x00);
        return;
    }
    g_ycp_preq_report_state_data[gunno].body.plug_gun = s_ycp_disposable_info[gunno].state.connect;
    g_ycp_preq_report_state_data[gunno].body.state = s_ycp_disposable_info[gunno].state.state;

    ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_STATE, 0x01);
    if(1/*g_ycp_preq_report_state_data[gunno].body.state != 0x01*/){
        s_ycp_state_data_count[gunno] = rt_tick_get();
        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
    }
}

void ycp_chargepile_update_result_report(uint8_t result, uint8_t reason)
{
    g_ycp_pres_remote_update.body.result = result;
    g_ycp_pres_remote_update.body.reason = reason;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_PRES_EVENT_REMOTE_UPDATE);
}

void ycp_chargepile_fault_report(uint8_t gunno, uint16_t code)
{
#if 0
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_ycp_disposable_info[gunno].fault_code = code;

    if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_REALTIME_DATA) == NET_YCP_SEND_STATE_ONGOING){
        ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_FAULT, 0x00);
        return;
    }
    ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_FAULT, 0x01);
    g_ycp_preq_report_realtime_data[gunno].body.hardware_fault = code;
    s_ycp_realtime_data_count[gunno] = rt_tick_get();
    rt_kprintf("1111111111111111nnnnnnnnnnnnn(%d, %d)\n", gunno, code);
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_REALTIME_DATA);
#endif /* 0 */
}

/*************************************************
 * 函数名      ycp_chargepile_state_detect
 * 功能          桩状态检测并修正
 * **********************************************/
void ycp_chargepile_state_detect(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t cstate = 0x02, cconnect = 0x00;
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    switch(s_ycp_base->state.current){
    case APP_OFSM_STATE_IDLEING:
        cconnect = 0x00;
        cstate = 0x02;
        break;
    case APP_OFSM_STATE_READYING:
        cconnect = 0x01;
        cstate = 0x02;
        break;
    case APP_OFSM_STATE_STARTING:
        cconnect = 0x01;
        cstate = 0x02;
        break;
    case APP_OFSM_STATE_CHARGING:
        cconnect = 0x01;
        cstate = 0x03;
        break;
    case APP_OFSM_STATE_STOPING:
        cconnect = 0x01;
        cstate = 0x02;
        break;
    case APP_OFSM_STATE_FINISHING:
        cconnect = 0x01;
        cstate = 0x02;
        break;
    case APP_OFSM_STATE_FAULTING:
        cconnect = 0x00;
        if(s_ycp_base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
            cconnect = 0x01;
        }
        cstate = 0x01;
        break;
    default:
        break;
    }
    if((s_ycp_disposable_info[gunno].state.connect != cconnect) || (s_ycp_disposable_info[gunno].state.state != cstate)){
        ycp_chargepile_state_changed(gunno);
    }
}

/*************************************************
 * 函数名      ycp_chargepile_create_local_transaction_number
 * 功能          创建本地交易号
 * **********************************************/
int8_t ycp_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if((vector == NULL) || (len == 0x00)){
        return -0x02;
    }
    if(len < NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT){
        return -0x03;
    }

    uint8_t sn_len = 0x00, *ptr = (uint8_t*)vector;
    struct tm *_tm = NULL;

    s_ycp_local_start_sq++;
//    sn_len = sizeof(g_ycp_preq_transaction_records[gunno].body.pile_number);
    sn_len = 0x07;  /* 需要确认本地流水号创建规则 */////////////////////////////////////////////
    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    _tm = localtime((const time_t*)&(s_ycp_base->current_time));

    rt_kprintf("ycp_chargepile_create_local_transaction_number[%d](%d, %d, %d, %d, %d)\n", s_ycp_base->current_time, _tm->tm_year, _tm->tm_mon,
            _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
//    memcpy(ptr, g_ycp_preq_transaction_records[gunno].body.pile_number, sn_len);   /* 桩号 */
    ptr[sn_len++] = gunno + 1;                                                     /* 枪号 */
    ptr[sn_len++] = (((_tm->tm_year - 100) /10) *16) + ((_tm->tm_year - 100) %10); /* 年 */
    ptr[sn_len++] = (((_tm->tm_mon + 0x01) /10) *16) + ((_tm->tm_mon + 0x01) %10); /* 月 */
    ptr[sn_len++] = ((_tm->tm_mday /10) *16) +  (_tm->tm_mday %10);                /* 日 */
    ptr[sn_len++] = ((_tm->tm_hour /10) *16) + (_tm->tm_hour %10);                 /* 时 */
    ptr[sn_len++] = ((_tm->tm_min /10) *16) + (_tm->tm_min %10);                   /* 分 */
    ptr[sn_len++] = ((_tm->tm_sec /10) *16) + (_tm->tm_sec %10);                   /* 秒 */
    memcpy((ptr + sn_len), &s_ycp_local_start_sq, sizeof(s_ycp_local_start_sq));   /* 自增序列号 */


    return 0x00;
}

/*************************************************
 * 函数名      ycp_chargepile_time_sync_revise
 * 功能          时间同步修正
 * **********************************************/
void ycp_chargepile_time_sync_revise(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    ycp_timestamp_to_timebcd(s_ycp_base->start_time, g_ycp_preq_transaction_records[gunno].body.start_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
    ycp_timestamp_to_timebcd(s_ycp_base->stop_time, g_ycp_preq_transaction_records[gunno].body.end_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
}

/*************************************************
 * 函数名      ycp_chargepile_fault_converted
 * 功能          故障转换
 * **********************************************/
uint8_t ycp_chargepile_fault_converted(uint8_t bit)
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
 * 函数名      ycp_chargepile_transaction_identity_converted
 * 功能          交易标识转换
 * **********************************************/
static uint8_t ycp_chargepile_transaction_identity_converted(uint8_t identity)
{
    switch(identity){
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
 * 函数名      ycp_chargepile_stop_reason_converted
 * 功能          停充原因转换
 * **********************************************/
static uint16_t ycp_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting)
{
#if 0
    uint16_t _reason = DEV_ABORT_CHARGING_CODE_90;

    switch(reason){
    /* 急停 */
    case APP_SYSTEM_STOP_WAY_SCRAM:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_50;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_72;
        }
        break;
    /* 读卡器、门禁 */
    case APP_SYSTEM_STOP_WAY_CARDREADER:
    case APP_SYSTEM_STOP_WAY_DOOR:
        _reason = DEV_START_CHARGING_FAILED_CODE_4A;
        break;
    /* 电表 */
    case APP_SYSTEM_STOP_WAY_AMMETER:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_4D;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_6D;
        }
        break;
    /* 充电模块 */
    case APP_SYSTEM_STOP_WAY_CHARGEMODULE:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_4F;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_71;
        }
        break;
    /* 过温 */
    case APP_SYSTEM_STOP_WAY_OVERTEMP:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_53;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_74;
        }
        break;
    /* 过、欠压 */
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        _reason = DEV_ABORT_CHARGING_CODE_79;
        break;
    /* 过流 */
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        _reason = DEV_ABORT_CHARGING_CODE_7A;
        break;
    /* 继电器 */
    case APP_SYSTEM_STOP_WAY_RELAY:
    case APP_SYSTEM_STOP_WAY_PARALLEL_RELAY:
    case APP_SYSTEM_STOP_WAY_AC_RELAY:
        _reason = DEV_ABORT_CHARGING_CODE_6C;
        break;
    /* 充满 */
    case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
        _reason = DEV_STOP_CHARGING_CODE_41;
        break;
    /* 拔枪 */
    case APP_SYSTEM_STOP_WAY_PULL_GUN:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_4B;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_6B;
        }
        break;
    /* 电子锁 */
    case APP_SYSTEM_STOP_WAY_ELECTRY_LOCK:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_55;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_77;
        }
        break;
    /* 通讯 */
    case APP_SYSTEM_STOP_WAY_COMMINICATION:
        if(stop_in_starting){
            _reason = DEV_START_CHARGING_FAILED_CODE_59;
        }else{
            _reason = DEV_ABORT_CHARGING_CODE_84;
        }
        break;
    /* 辅源 */
    case APP_SYSTEM_STOP_WAY_AUXPOWER:
        _reason = DEV_START_CHARGING_FAILED_CODE_56;
        break;
    /* 断电 */
    case APP_SYSTEM_STOP_WAY_POWER_OFF:
        _reason = DEV_ABORT_CHARGING_CODE_83;
        break;
    /* 存储芯片 */
    case APP_SYSTEM_STOP_WAY_FLASH:
    case APP_SYSTEM_STOP_WAY_EEPROM:
        _reason = DEV_START_CHARGING_FAILED_CODE_4A;
        break;
    /* 短路 */
    case APP_SYSTEM_STOP_WAY_SHORTS:
        _reason = DEV_ABORT_CHARGING_CODE_6C;
        break;
    /* 枪电压 */
    case APP_SYSTEM_STOP_WAY_GUNVOLT:
        _reason = DEV_START_CHARGING_FAILED_CODE_60;
        break;
    /* 绝缘 */
    case APP_SYSTEM_STOP_WAY_INSULT:
        _reason = DEV_START_CHARGING_FAILED_CODE_57;
        break;
    /* 电池电压 */
    case APP_SYSTEM_STOP_WAY_BATTERY_VOLT:
        _reason = DEV_START_CHARGING_FAILED_CODE_61;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = DEV_ABORT_CHARGING_CODE_8A;
        break;
    /* 准备电压 */
    case APP_SYSTEM_STOP_WAY_READY_VOLT:
        _reason = DEV_ABORT_CHARGING_CODE_8B;
        break;
    /* 绝缘电压 */
    case APP_SYSTEM_STOP_WAY_INSULT_VOLT:
        _reason = DEV_START_CHARGING_FAILED_CODE_65;
        break;
//    /* BSM */
//    case APP_SYSTEM_STOP_WAY_BSM:
//        _reason = DEV_START_CHARGING_FAILED_CODE_66;
//        break;
    /* APP */
    case APP_SYSTEM_STOP_WAY_APP_STOP:
        _reason = DEV_STOP_CHARGING_CODE_40;
        break;
    /* 刷卡 */
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = DEV_STOP_CHARGING_CODE_45;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        _reason = DEV_ABORT_CHARGING_CODE_6E;
        break;
    /* 到达设定电量 */
    case APP_SYSTEM_STOP_WAY_REACH_ELECT:
        _reason = DEV_STOP_CHARGING_CODE_42;
        break;
    /* 到达设定时间 */
    case APP_SYSTEM_STOP_WAY_REACH_TIME:
        _reason = DEV_STOP_CHARGING_CODE_44;
        break;
    /* 到达设定余额 */
    case APP_SYSTEM_STOP_WAY_REACH_MONEY:
        _reason = DEV_STOP_CHARGING_CODE_43;
        break;
    /* 充电电流异常 */
    case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
        _reason = DEV_ABORT_CHARGING_CODE_8C;
        break;
    /* 达到SOC 限定值 */
    case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
        _reason = DEV_STOP_CHARGING_CODE_46;
        break;
    default:
        break;
    }
    return _reason;
#endif /* 0 */
    return 0x00;
}

/*************************************************
 * 函数名      ycp_query_transaction_verify_state
 * 功能          查询订单确认状态
 * **********************************************/
uint8_t ycp_query_transaction_verify_state(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    if(ycp_transaction_is_verify(gunno)){
        ycp_set_transaction_verify_state(gunno, 0x00);
        return 0x01;
    }
    return 0x00;
}

static void ycp_request_message_repeat(uint8_t gunno)
{
    uint8_t event = 0;
    if(ycp_exist_message_wait_response(gunno, NULL)){
        for(event = 0; event < NET_YCP_CHARGEPILE_PREQ_NUM; event++){
            if(ycp_get_message_wait_response_timeout_state(gunno, NET_YCP_WAIT_RESPONSE_TIMEOUT, event)){
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, event);
            }
        }
    }
}

void ycp_data_realtime_process(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ycp_base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(ycp_get_socket_info()->state == YCP_SOCKET_STATE_LOGIN_SUCCESS){
        ycp_request_message_repeat(gunno);

        if(s_ycp_base->state.current == APP_OFSM_STATE_CHARGING){
            if(s_ycp_state_data_count[gunno] > rt_tick_get()){
                s_ycp_state_data_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_ycp_state_data_count[gunno]) > YCP_STATE_DATA_INTERVAL_CHARGING *1000){
                s_ycp_state_data_count[gunno] = rt_tick_get();
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
                if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x01){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_BMS_INFO);
                }
            }
        }else{
            if(s_ycp_state_data_count[gunno] > rt_tick_get()){
                s_ycp_state_data_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_ycp_state_data_count[gunno]) > s_ycp_state_data_interval[gunno] *1000){
                s_ycp_state_data_count[gunno] = rt_tick_get();
                if(s_ycp_state_data_interval[gunno] < YCP_STATE_DATA_INTERVAL_IDLE){
                    s_ycp_state_data_interval[gunno] = YCP_STATE_DATA_INTERVAL_IDLE;
                }
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
            }
        }
    }else{
        s_ycp_state_data_count[gunno] = rt_tick_get();
        s_ycp_state_data_interval[gunno] = YCP_STATE_DATA_INTERVAL_INIT;
        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
    }

    if((s_ycp_base->state.current == APP_OFSM_STATE_STARTING) || (s_ycp_base->state.current == APP_OFSM_STATE_CHARGING)){
        ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
    }
}

/*
 * 用于检测只上报一次的报文是否有漏报
 * */
void ycp_disposable_message_check(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t _event = 0x00;
    ycp_get_disposable_event(gunno, 0x00, &_event);
    if(_event){
        if(_event &(1 <<YCP_DISPOSABLE_EVENT_STATE)){
            if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA) == NET_YCP_SEND_STATE_COMPLETE){
                g_ycp_preq_report_state_data[gunno].body.plug_gun = s_ycp_disposable_info[gunno].state.connect;
                g_ycp_preq_report_state_data[gunno].body.state = s_ycp_disposable_info[gunno].state.state;

                ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_STATE, 0x01);
                s_ycp_state_data_count[gunno] = rt_tick_get();
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
            }
        }
        if(_event &(1 <<YCP_DISPOSABLE_EVENT_FAULT)){
            if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA) == NET_YCP_SEND_STATE_COMPLETE){
//                g_ycp_preq_report_state_data[gunno].body.hardware_fault = s_ycp_disposable_info[gunno].fault_code;

                ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_FAULT, 0x01);
                s_ycp_state_data_count[gunno] = rt_tick_get();
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
            }
        }
    }
}

#endif /* NET_PACK_USING_YCP */
