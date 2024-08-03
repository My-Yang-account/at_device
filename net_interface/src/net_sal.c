/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#include "net_sal.h"
#include "app_ofsm.h"

static void app_nsal_data_update(void)
{

}

int32_t app_nsal_operation_function_config(void)
{

}

/*******************************************
 * 函数名    app_nsal_realtime_process
 * 功能        实时处理数据
 *****************************************/
void app_nsal_realtime_process(uint8_t gunno)
{
#if 0
#ifdef NET_PACK_USING_THA
    tha_fault_detect_report(gunno);
    tha_data_realtime_process(gunno);
    tha_disposable_message_check(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    ykc_fault_detect_report(gunno);
    ykc_data_realtime_process(gunno);
    ykc_disposable_message_check(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_fault_detect_report(gunno);
    ykc_monitor_data_realtime_process(gunno);
    ykc_monitor_disposable_message_check(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_fault_detect_report(gunno);
    ycp_data_realtime_process(gunno);
    ycp_disposable_message_check(gunno);
#endif /* NET_PACK_USING_YCP */
#endif /* 0 */
}

/*******************************************
 * 函数名    app_nsal_realtime_process
 * 功能        实时处理数据
 *****************************************/
void app_nsal_system_fault_report(uint8_t gunno, uint8_t code, uint32_t timestamp, uint8_t is_resume)
{
#ifdef NET_PACK_USING_THA
    tha_fault_event_detect_callback(gunno, code, timestamp, is_resume);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    ykc_fault_event_detect_callback(gunno, code, is_resume);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_fault_event_detect_callback(gunno, code, is_resume);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_fault_event_detect_callback(gunno, code, is_resume);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_get_link_state
 * 功能        获取网络连接状态
 *****************************************/
uint8_t app_nsal_get_link_state(void)
{
    switch(net_get_net_handle()->net_state){
    case NET_SOCKET_STATE_PHY:
        return APP_NET_STATE_NULL;
        break;
    case NET_SOCKET_STATE_DATA_LINK:
        return APP_NET_STATE_UP;
        break;
    case NET_SOCKET_STATE_OPEN:
        return APP_NET_STATE_INTERNET_UP;
        break;
    case NET_SOCKET_STATE_LOGIN_WAIT:
        return APP_OTA_STATE_UP;
        break;
    case NET_SOCKET_STATE_LOGIN_SUCCESS:
        return APP_NET_STATE_AUTH_SECCESS;
        break;
    default:
        break;
    }
    return APP_NET_STATE_NULL;
}

/********************************************** OTA 相关 OTA **********************************************/
/********************************************** OTA 相关 OTA **********************************************/
/*******************************************
 * 函数名    app_nsal_get_ota_start_flag
 * 功能        获取OTA开始标志
 *****************************************/
uint8_t app_nsal_get_ota_start_flag(void)
{
    return net_get_ota_info()->flag.start_ota;
}

/*******************************************
 * 函数名    app_nsal_get_ota_state
 * 功能        获取OTA状态
 *****************************************/
uint8_t app_nsal_get_ota_state(void)
{
    uint8_t state = APP_OTA_STATE_NULL;
    switch(net_get_ota_info()->state){
    case NET_OTA_STATE_NULL:
        break;
    case NET_OTA_STATE_READY:
        state = APP_OTA_STATE_UP;
        break;
    case NET_OTA_STATE_OPEN_LINK:
        state = APP_OTA_STATE_INTERNET_UP;
        break;
    case NET_OTA_STATE_LOGIN_WAIT:
        state = APP_OTA_STATE_AUTHING;
        break;
    case NET_OTA_STATE_UPDATING:
        state = APP_OTA_STATE_UPDATEING;
        break;
    case NET_OTA_STATE_SUCCESS:
        state = APP_OTA_STATE_UPDATE_SECCESS;
        break;
    case NET_OTA_STATE_FAIL:
        state = APP_OTA_STATE_UPDATE_FAILED;
        break;
    default:
        break;
    }
    return state;
}


/*******************************************
 * 函数名    app_nsal_get_ota_result
 * 功能        获取OTA结果
 *****************************************/
uint8_t app_nsal_get_ota_result(void)
{
    return net_get_ota_info()->result;
}
/*******************************************
 * 函数名    app_nsal_get_ota_progress
 * 功能        获取OTA进度
 *****************************************/
uint32_t app_nsal_get_ota_progress(void)
{
    return net_get_ota_info()->progress;
}
/**********************************************OTA 相关 OTA **********************************************/

/********************************************** 远程重启相关 **********************************************/
/********************************************** 远程重启相关 **********************************************/
/*******************************************
 * 函数名    app_nsal_is_remote_reset
 * 功能        检测是否有远程重启事件
 *****************************************/
uint8_t app_nsal_is_remote_reset(void)
{
    return net_operation_get_event(0x00, NET_OPERATION_EVENT_REBOOT);
}
/********************************************** 远程重启相关 **********************************************/

/*******************************************
 * 函数名   app_nsal_message_init
 * 功能        初始化网络报文
 *****************************************/
void app_nsal_message_init(void)
{
#ifdef NET_PACK_USING_THA
    tha_message_info_init();
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    ykc_message_info_init();
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_message_info_init();
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_message_info_init();
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_SGCC
    extern void sgcc_message_info_init(void);
    sgcc_message_info_init();
#endif /* NET_PACK_USING_SGCC */
}
/*******************************************
 * 函数名    app_nsal_state_charged
 * 功能        充电桩状态发生改变：上报变化状态
 *****************************************/
void app_nsal_state_charged(uint8_t gunno)
{
#ifdef NET_PACK_USING_THA
    tha_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    ykc_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_SGCC
    sgcc_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_SGCC */
}
/*******************************************
 * 函数名    app_nsal_event_occurded
 * 功能        充电桩有事件产生：上报事件
 *****************************************/
void app_nsal_event_occurded(uint8_t gunno)
{
#ifdef NET_PACK_USING_THA
    tha_chargepile_event_occurred(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    ykc_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_state_changed(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*********************************************** [远程启停 相关信息] **************************************/
/*********************************************** [远程启停 相关信息] **************************************/
/*******************************************
 * 函数名    app_nsal_clear_remote_start
 * 功能        清除远程启动事件
 *****************************************/
void app_nsal_clear_remote_start(uint8_t gunno)
{
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_START_CHARGE);
}

/*******************************************
 * 函数名    app_nsal_clear_remote_stop
 * 功能        清除远程停止事件
 *****************************************/
void app_nsal_clear_remote_stop(uint8_t gunno)
{
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
}

/*******************************************
 * 函数名    app_nsal_is_remote_start
 * 功能        检测是否有远程启动事件
 *****************************************/
uint8_t app_nsal_is_remote_start(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_START_CHARGE);
}

/*******************************************
 * 函数名    app_nsal_clear_remote_stop
 * 功能        检测是否有远程停止事件
 *****************************************/
uint8_t app_nsal_is_remote_stop(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
}

/*******************************************
 * 函数名    app_nsal_report_remote_start_result
 * 功能        上报远程启动结果
 *****************************************/
void app_nsal_report_remote_start_result(uint8_t gunno, uint8_t result, uint8_t reason)
{
#ifdef NET_PACK_USING_YKC
    ykc_start_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_start_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_start_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_start_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_remote_stop_result
 * 功能        上报远程停止结果
 *****************************************/
void app_nsal_report_remote_stop_result(uint8_t gunno, uint8_t result, uint8_t reason)
{
#ifdef NET_PACK_USING_YKC
    ykc_stop_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_stop_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_stop_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_stop_charge_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_YCP */
}

/*********************************************** [计费相关] **************************************/
/*********************************************** [计费相关] **************************************/
/*******************************************
 * 函数名    app_nsal_need_update_billingrule
 * 功能        检测是否有需要更新计费规则事件
 *****************************************/
uint8_t app_nsal_need_update_billingrule(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
}

/*******************************************
 * 函数名    app_nsal_clear_update_billingrule_event
 * 功能        清除需要更新计费规则事件
 *****************************************/
void app_nsal_clear_update_billingrule_event(uint8_t gunno)
{
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
}

/*********************************************** [充电前报文初始化] **************************************/
/*********************************************** [充电前报文初始化] **************************************/
/*******************************************
 * 函数名    app_nsal_time_sync_revise
 * 功能        时间同步修正
 *****************************************/
void app_nsal_time_sync_revise(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_time_sync_revise(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_chargepile_time_sync_revise(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_time_sync_revise(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_time_sync_revise(gunno);
#endif /* NET_PACK_USING_YCP */
}
/*******************************************
 * 函数名    app_nsal_init_charge_data
 * 功能        初始化充电数据
 *****************************************/
void app_nsal_init_charge_data(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_realtime_data(gunno, 0x01);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_chargepile_request_padding_charge_data(gunno, 0x01);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_realtime_data(gunno, 0x01);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_state_data(gunno, 0x01);
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_SGCC
    sgcc_chargepile_request_padding_state_data(gunno, 0x01);
#endif /* NET_PACK_USING_SGCC */
}

/*******************************************
 * 函数名    app_nsal_padding_charge_data
 * 功能        填充充电数据
 *****************************************/
void app_nsal_padding_charge_data(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_realtime_data(gunno, 0x00);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_chargepile_request_padding_charge_data(gunno, 0x00);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_realtime_data(gunno, 0x00);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_state_data(gunno, 0x00);
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_SGCC
    sgcc_chargepile_request_padding_state_data(gunno, 0x00);
#endif /* NET_PACK_USING_SGCC */
}

/*******************************************
 * 函数名    app_nsal_transaction_record_report_monitor
 * 功能        上送交易记录(监控平台)
 *****************************************/
void app_nsal_transaction_record_report_monitor(uint8_t gunno, void *transaction, uint8_t is_repeat)
{
#ifdef NET_PACK_USING_YKC
#ifdef NET_YKC_AS_MONITOR
    ykc_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_YKC_AS_MONITOR */
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
#ifdef NET_THA_AS_MONITOR
    if(is_repeat){
        tha_chargepile_request_padding_history_bill(gunno, transaction);
    }else{
        tha_chargepile_request_padding_transaction_bill(gunno, transaction);
    }
#endif /* NET_THA_AS_MONITOR */
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
#ifdef NET_YKC_MONITOR_AS_MONITOR
    ykc_monitor_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_YKC_MONITOR_AS_MONITOR */
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
#ifdef NET_YCP_AS_MONITOR
    ycp_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_YCP_AS_MONITOR */
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_SGCC
#ifdef NET_SGCC_AS_MONITOR
    sgcc_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_SGCC_AS_MONITOR */
#endif /* NET_PACK_USING_SGCC */
}

/*******************************************
 * 函数名    app_nsal_transaction_record_report_target
 * 功能        上送交易记录(目标平台)
 *****************************************/
void app_nsal_transaction_record_report_target(uint8_t gunno, void *transaction, uint8_t is_repeat)
{
#ifdef NET_PACK_USING_YKC
#ifdef NET_YKC_AS_TARGET
    ykc_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_YKC_AS_TARGET */
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
#ifdef NET_THA_AS_TARGET
    if(is_repeat){
        tha_chargepile_request_padding_history_bill(gunno, transaction);
    }else{
        tha_chargepile_request_padding_transaction_bill(gunno, transaction);
    }
#endif /* NET_THA_AS_TARGET */
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
#ifdef NET_YKC_MONITOR_AS_TARGET
    ykc_monitor_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_YKC_MONITOR_AS_TARGET */
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
#ifdef NET_YCP_AS_TARGET
    ycp_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_YCP_AS_TARGET */
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_SGCC
#ifdef NET_SGCC_AS_TARGET
    sgcc_chargepile_request_padding_transaction_record(gunno, transaction, is_repeat);
#endif /* NET_SGCC_AS_TARGET */
#endif /* NET_PACK_USING_SGCC */
}


/*********************************************** [鉴权 相关信息] **************************************/
/*********************************************** [鉴权 相关信息] **************************************/
/*******************************************
 * 函数名    app_nsal_card_authorize
 * 功能        上送刷卡鉴权
 *****************************************/
int8_t app_nsal_card_authorize(uint8_t gunno)
{
    int8_t result = -0x01;
#ifdef NET_PACK_USING_YKC
#ifndef NET_YKC_AS_MONITOR
    result = ykc_chargepile_request_padding_card_authority(gunno);
#endif /* NET_YKC_AS_MONITOR */
#endif /* NET_PACK_USING_YKC */
    if(result > 0x00){
        return result;
    }

#ifdef NET_PACK_USING_YKC_MONITOR
#ifndef NET_YKC_MONITOR_AS_MONITOR
    ykc_monitor_chargepile_request_padding_card_authority(gunno);
#endif /* NET_YKC_MONITOR_AS_MONITOR */
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_THA
    result = tha_chargepile_request_padding_card_authority(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YCP
#ifndef NET_YCP_AS_MONITOR
    result = ycp_chargepile_request_padding_card_authority(gunno);
#endif /* NET_YCP_AS_MONITOR */
#endif /* NET_PACK_USING_YCP */
    return result;
}
/*******************************************
 * 函数名    app_nsal_vin_authorize
 * 功能        上送VIN码鉴权
 *****************************************/
int8_t app_nsal_vin_authorize(uint8_t gunno)
{
    int8_t result = -0x01;
#ifdef NET_PACK_USING_YKC
#ifndef NET_YKC_AS_MONITOR
    result = ykc_chargepile_request_padding_vin_authority(gunno);
#endif /* NET_YKC_AS_MONITOR */
#endif /* NET_PACK_USING_YKC */
    if(result > 0x00){
        return result;
    }

#ifdef NET_PACK_USING_YKC_MONITOR
#ifndef NET_YKC_MONITOR_AS_MONITOR
    ykc_monitor_chargepile_request_padding_vin_authority(gunno);
#endif /* NET_YKC_MONITOR_AS_MONITOR */
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_THA
    result = tha_chargepile_request_padding_vin_authority(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YCP
#ifndef NET_YCP_AS_MONITOR
    result = ycp_chargepile_request_padding_vin_authority(gunno);
#endif /* NET_YCP_AS_MONITOR */
#endif /* NET_PACK_USING_YCP */
    return result;
}

/*******************************************
 * 函数名    app_nsal_clear_remote_card_authorize
 * 功能        清除远程刷卡授权事件
 *****************************************/
void app_nsal_clear_remote_card_authorize(uint8_t gunno)
{
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_SUCCESS);
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
}
/*******************************************
 * 函数名    app_nsal_clear_remote_vin_authorize
 * 功能        清除远程VIN码授权事件
 *****************************************/
void app_nsal_clear_remote_vin_authorize(uint8_t gunno)
{
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS);
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
}

/*******************************************
 * 函数名    app_nsal_is_card_authorize_success
 * 功能        检测刷卡鉴权是否成功
 *****************************************/
uint8_t app_nsal_is_card_authorize_success(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_SUCCESS);
}
/*******************************************
 * 函数名   app_nsal_is_vin_authorize_success
 * 功能        检测VIN码鉴权是否成功
 *****************************************/
uint8_t app_nsal_is_vin_authorize_success(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS);
}
/*******************************************
 * 函数名    app_nsal_is_card_authorize_fail
 * 功能        检测刷卡鉴权是否失败
 *****************************************/
uint8_t app_nsal_is_card_authorize_fail(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
}
/*******************************************
 * 函数名    app_nsal_is_vin_authorize_fail
 * 功能        检测VIN码鉴权是否失败
 *****************************************/
uint8_t app_nsal_is_vin_authorize_fail(uint8_t gunno)
{
    return net_operation_get_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
}

/*********************************************** [BMS 相关信息] **************************************/
/*********************************************** [BMS 相关信息] **************************************/
/*******************************************
 * 函数名    app_nsal_report_bms_message_bmsinfo
 * 功能        上报BMS报文：BMS信息
 *****************************************/
void app_nsal_report_bms_message_bmsinfo(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bms_shakehand(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bms_shakehand(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bms_shakehand(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_bmsparameter
 * 功能        上报BMS报文：BMS参数
 *****************************************/
void app_nsal_report_bms_message_bmsparameter(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bms_paraconfig(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_chargepile_request_padding_battery_info(gunno);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bms_paraconfig(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bms_paraconfig(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_end
 * 功能        上报BMS报文：充电结束
 *****************************************/
void app_nsal_report_bms_message_end(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bms_chargeend(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bms_chargeend(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bms_chargeend(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_error
 * 功能        上报BMS报文：错误报文
 *****************************************/
void app_nsal_report_bms_message_error(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bms_error(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bms_error(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bms_error(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_bmsend
 * 功能        上报BMS报文：BMS 终止
 *****************************************/
void app_nsal_report_bms_message_bmsend(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bmsend_duringcharge(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bmsend_duringcharge(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bmsend_duringcharge(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_chargerend
 * 功能        上报BMS报文：充电机终止
 *****************************************/
void app_nsal_report_bms_message_chargerend(uint8_t gunno)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_chargerend_duringcharge(gunno);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_chargerend_duringcharge(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_chargerend_duringcharge(gunno);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_bmsrequire_pileoutput
 * 功能        上报BMS报文：充电过程 BMS 需求与充电机输出
 *****************************************/
void app_nsal_report_bms_message_bmsrequire_pileoutput(uint8_t gunno, uint8_t is_init)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bmscommand_chargerout(gunno, is_init);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bmscommand_chargerout(gunno, is_init);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bmscommand_chargerout(gunno, is_init);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_report_bms_message_bmsstate
 * 功能        上报BMS报文：充电过程 BMS 状态信息
 *****************************************/
void app_nsal_report_bms_message_bmsstate(uint8_t gunno, uint8_t is_init)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_request_padding_bmsinfo_duringcharge(gunno, is_init);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_chargepile_request_padding_battery_state(gunno, 0x00);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_chargepile_request_padding_bmsinfo_duringcharge(gunno, is_init);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_request_padding_bmsinfo_duringcharge(gunno, is_init);
#endif /* NET_PACK_USING_YCP */
}




/*******************************************
 * 函数名    app_nsal_create_local_transaction_number
 * 功能        创建本地交易号
 *****************************************/
void app_nsal_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
#ifdef NET_PACK_USING_YKC
    ykc_chargepile_create_local_transaction_number(gunno, vector, len);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_chargepile_create_local_transaction_number(gunno, vector, len);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
#ifndef NET_YKC_MONITOR_AS_MONITOR
    ykc_monitor_chargepile_create_local_transaction_number(gunno, vector, len);
#endif /* NET_YKC_MONITOR_AS_MONITOR */
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_chargepile_create_local_transaction_number(gunno, vector, len);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_query_transaction_verify_state
 * 功能        查询订单确认状态
 *****************************************/
uint8_t app_nsal_query_transaction_verify_state(uint8_t gunno, uint16_t platform_id)
{
    uint32_t state = 0x00;
    switch(platform_id){
    case APP_PLATFORM_ID_THA:
#ifdef NET_PACK_USING_THA
        state = tha_query_transaction_verify_state(gunno);
#endif /* NET_PACK_USING_THA */
        break;
    case APP_PLATFORM_ID_YKC:
#ifdef NET_PACK_USING_YKC
        state = ykc_query_transaction_verify_state(gunno);
#endif /* NET_PACK_USING_YKC */
        break;
    case APP_PLATFORM_ID_XJ:
#ifdef NET_PACK_USING_XJ
        state = 0x00;
#endif /* NET_PACK_USING_XJ */
        break;
    case APP_PLATFORM_ID_SL:
#ifdef NET_PACK_USING_SL
        state = 0x00;
#endif /* NET_PACK_USING_SL */
        break;
    case APP_PLATFORM_ID_YKC_MONITOR:
#ifdef NET_PACK_USING_YKC_MONITOR
        state = ykc_monitor_query_transaction_verify_state(gunno);
#endif /* NET_PACK_USING_YKC_MONITOR */
        break;
    case APP_PLATFORM_ID_YCP:
#ifdef NET_PACK_USING_YCP
        state = ycp_query_transaction_verify_state(gunno);
#endif /* NET_PACK_USING_YCP */
        break;
    default:
        break;
    }

    return state;
}

/*******************************************
 * 函数名    app_nsal_is_permit_ota
 * 功能        允许OTA
 *****************************************/
void app_nsal_is_permit_ota(uint8_t state)
{
    net_get_ota_info()->flag.permit_ota = state;
}

/*******************************************
 * 函数名    app_nsal_report_set_power_result
 * 功能        上报功率设置结果
 *****************************************/
void app_nsal_report_set_power_result(uint8_t gunno, uint8_t result, uint8_t reason)
{
#ifdef NET_PACK_USING_YKC
    ykc_set_power_percent_response_asynchronously(result);
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_THA
    tha_set_power_response_asynchronously(gunno, result);
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC_MONITOR
    ykc_monitor_set_power_percent_response_asynchronously(result);
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    ycp_set_power_percent_response_asynchronously(result);
#endif /* NET_PACK_USING_YCP */
}

/*******************************************
 * 函数名    app_nsal_is_set_power
 * 功能        检查是否有设置功率事件
 *****************************************/
uint8_t app_nsal_is_set_power(uint8_t gunno)
{
    for(uint8_t count = 0x00; count < APP_SYSTEM_GUNNO_SIZE; count++){
        if(net_operation_get_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER)){
            return 0x01;
        }
    }
    return 0x00;
}

/*******************************************
 * 函数名    app_nsal_clear_set_power
 * 功能        清除设置功率事件
 *****************************************/
void app_nsal_clear_set_power(uint8_t gunno)
{
    net_operation_clear_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER);
}

/*******************************************
 * 函数名    app_nsal_get_setup_power
 * 功能        获取设置的总功率
 *****************************************/
uint32_t app_nsal_get_setup_power(void)
{
    return net_operation_get_total_power();
}

/*******************************************
 * 函数名    app_nsal_is_time_sync
 * 功能        查询是否进行了时间同步
 *****************************************/
uint8_t app_nsal_is_time_sync(void)
{
    return net_operation_get_event(0x00, NET_OPERATION_EVENT_TIME_SYNC);
}

/*******************************************
 * 函数名    app_nsal_clear_time_sync
 * 功能        清除时间同步事件
 *****************************************/
void app_nsal_clear_time_sync(void)
{
    net_operation_clear_event(0x00, NET_OPERATION_EVENT_TIME_SYNC);
}

