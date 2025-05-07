/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#include "ykc_monitor_message_receive.h"
#include "ykc_monitor_message_send.h"
#include "ykc_monitor_transceiver.h"
#include "net_operation.h"

#define DBG_TAG "mykc_callback"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YKC_MONITOR

#ifdef NET_YKC_MONITOR_AS_MONITOR
#define YKC_RECV_MONITOR_MESSAGE_ITEM_NUM       0x03

struct ykc_monitor_recv_message{
    uint8_t cmd;
    uint8_t gunno;
    uint8_t result;
    uint16_t serial_number;
};

NET_DEF_SRAM2 static struct ykc_monitor_recv_message s_ykc_monitor_recv_message[YKC_RECV_MONITOR_MESSAGE_ITEM_NUM];
#endif /* NET_YKC_MONITOR_AS_MONITOR */

NET_DEF_SRAM2 static ykc_monitor_qrcode_buf_t s_ykc_monitor_qrcode_buf;
NET_DEF_SRAM2 static ykc_monitor_card_vin_buf_t s_ykc_monitor_card_vin_buf;
NET_DEF_SRAM2 static ykc_monitor_dev_info_buf_t s_ykc_monitor_dev_info_buf;
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
NET_DEF_SRAM2 static ykc_monitor_config_info_buf_t s_ykc_monitor_config_info_buf;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

extern uint8_t s_ykc_monitor_current_transaction_number[NET_SYSTEM_GUN_NUMBER][NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];

/**=======================================[服务器请求报文]=======================================*/
/**=======================================[服务器请求报文]=======================================*/
/** 读取实时数据 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Query_RealTimeData_t g_ykc_monitor_sreq_query_realtime_data[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程控制启机 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Remote_StartCharge_t g_ykc_monitor_sreq_remote_start_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程停机 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Remote_StopCharge_t g_ykc_monitor_sreq_remote_stop_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 远程账户余额更新 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_AccountBallance_Update_t g_ykc_monitor_sreq_account_ballance_update[NET_SYSTEM_GUN_NUMBER];   // OK
/** 离线卡数据同步 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Sync_OfflineCard_t g_ykc_monitor_sreq_sync_offline_card;   // OK
/** 离线卡数据清除 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Clear_OfflineCard_t g_ykc_monitor_sreq_clear_offline_card;   // OK
/** 离线卡数据查询 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Query_OfflineCard_t g_ykc_monitor_sreq_query_offline_card;   // OK
/** 充电桩工作参数设置 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Set_WorkPara_t g_ykc_monitor_sreq_set_work_para;   // OK
/** 对时设置 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_TimeSync_t g_ykc_monitor_sreq_time_sync;   // OK
/** 计费模型设置 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRep_BillingModel_Set_t g_ykc_monitor_sreq_billing_model_set;   // OK
/** 遥控地锁升锁与降锁命令 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_GroundLock_Lifting_t g_ykc_monitor_sreq_ground_lock_lifting[NET_SYSTEM_GUN_NUMBER];
/** 远程重启 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_RemoteReboot_t g_ykc_monitor_sreq_remote_reboot;
/** 远程更新 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_RemoteUpdate_t g_ykc_monitor_sreq_remote_update;
/** 运营平台远程控制并充启机 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t g_ykc_monitor_sreq_remote_start_merge_charge[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(国充) */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t g_ykc_monitor_sreq_qrcode_config_gc[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(云快充1.5) */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t g_ykc_monitor_sreq_qrcode_config_ykc15;
/** 运营平台下发二维码配置(特来电) */
NET_DEF_SRAM2 Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t g_ykc_monitor_sreq_qrcode_config_tld[NET_SYSTEM_GUN_NUMBER];

/**=======================================[服务器响应报文]=======================================*/
/**=======================================[服务器响应报文]=======================================*/
/** 登录签到响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_LogIn_t g_ykc_monitor_sres_login;   // OK
///** 心跳响应 */
//NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_HeartBeat_t g_ykc_monitor_sres_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证请求响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_BillingModel_Verify_t g_ykc_monitor_sres_billing_model_verify;   // OK
/** 计费模型请求响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_BillingModel_Request_t g_ykc_monitor_sres_billing_model_request;   // OK
/** 交易记录响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_TransactionRecords_t g_ykc_monitor_sres_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 充电桩主动申请启动充电响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_ApplyCharge_Active_t g_ykc_monitor_sres_apply_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电桩主动申请并充充电响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t g_ykc_monitor_sres_apply_merge_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK

#ifdef NET_YKC_MONITOR_AS_MONITOR
/** 充电桩目标平台日志上报响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_Sres_TargetPlat_Log_t g_ykc_monitor_sres_target_plat_log;
/** 充电桩设备信息上报响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_Sres_DevInfo_t g_ykc_monitor_sres_dev_info;
/** 服务器下发功能开关控制请求 */
NET_DEF_SRAM2 Net_YkcMonitorPro_Sreq_FunctionSwitch_t g_ykc_monitor_sreq_function_switch;
/** IP、端口、桩号修改(信息确认)请求 */
NET_DEF_SRAM2 Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t g_ykc_monitor_sreq_pres_info_para_modify_confirm;
/** IP、服务器IP、端口、桩号信息确认结果响应 */
NET_DEF_SRAM2 Net_YkcMonitorPro_Sres_InfoPara_ConfirmResult_t g_ykc_monitor_sres_info_para_confirm_result;
/** 服务器下发修改设备信息请求 */
NET_DEF_SRAM2 Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t g_ykc_monitor_sreq_modify_device_info;
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#ifdef NET_YKC_MONITOR_AS_MONITOR

#ifdef NET_DESIGNATE_REGION
/*************************************************
 * 函数名      ykc_monitor_mreceive_info_init
 * 功能          监控平台报文接收信息、变量初始化
 * **********************************************/
void ykc_monitor_mreceive_info_init(void)
{
    /**=======================================[服务器请求报文]=======================================*/
    /**=======================================[服务器请求报文]=======================================*/
    memset(g_ykc_monitor_sreq_query_realtime_data, 0x00, sizeof(g_ykc_monitor_sreq_query_realtime_data));
    memset(g_ykc_monitor_sreq_remote_start_charge, 0x00, sizeof(g_ykc_monitor_sreq_remote_start_charge));
    memset(g_ykc_monitor_sreq_remote_stop_charge, 0x00, sizeof(g_ykc_monitor_sreq_remote_stop_charge));
    memset(g_ykc_monitor_sreq_account_ballance_update, 0x00, sizeof(g_ykc_monitor_sreq_account_ballance_update));
    memset(g_ykc_monitor_sreq_ground_lock_lifting, 0x00, sizeof(g_ykc_monitor_sreq_ground_lock_lifting));
    memset(g_ykc_monitor_sreq_remote_start_merge_charge, 0x00, sizeof(g_ykc_monitor_sreq_remote_start_merge_charge));
    memset(g_ykc_monitor_sreq_qrcode_config_gc, 0x00, sizeof(g_ykc_monitor_sreq_qrcode_config_gc));
    memset(g_ykc_monitor_sreq_qrcode_config_tld, 0x00, sizeof(g_ykc_monitor_sreq_qrcode_config_tld));

    memset(&g_ykc_monitor_sreq_sync_offline_card, 0x00, sizeof(g_ykc_monitor_sreq_sync_offline_card));
    memset(&g_ykc_monitor_sreq_clear_offline_card, 0x00, sizeof(g_ykc_monitor_sreq_clear_offline_card));
    memset(&g_ykc_monitor_sreq_query_offline_card, 0x00, sizeof(g_ykc_monitor_sreq_query_offline_card));
    memset(&g_ykc_monitor_sreq_set_work_para, 0x00, sizeof(g_ykc_monitor_sreq_set_work_para));
    memset(&g_ykc_monitor_sreq_time_sync, 0x00, sizeof(g_ykc_monitor_sreq_time_sync));
    memset(&g_ykc_monitor_sreq_billing_model_set, 0x00, sizeof(g_ykc_monitor_sreq_billing_model_set));
    memset(&g_ykc_monitor_sreq_remote_reboot, 0x00, sizeof(g_ykc_monitor_sreq_remote_reboot));
    memset(&g_ykc_monitor_sreq_remote_update, 0x00, sizeof(g_ykc_monitor_sreq_remote_update));
    memset(&g_ykc_monitor_sreq_qrcode_config_ykc15, 0x00, sizeof(g_ykc_monitor_sreq_qrcode_config_ykc15));


    /**=======================================[服务器响应报文]=======================================*/
    /**=======================================[服务器响应报文]=======================================*/
    memset(g_ykc_monitor_sres_transaction_records, 0x00, sizeof(g_ykc_monitor_sres_transaction_records));
    memset(g_ykc_monitor_sres_apply_charge_active, 0x00, sizeof(g_ykc_monitor_sres_apply_charge_active));
    memset(g_ykc_monitor_sres_apply_merge_charge_active, 0x00, sizeof(g_ykc_monitor_sres_apply_merge_charge_active));
//    memset(g_ykc_monitor_sres_heartbeat, 0x00, sizeof(g_ykc_monitor_sres_heartbeat));

    memset(&g_ykc_monitor_sres_login, 0x00, sizeof(g_ykc_monitor_sres_login));
    memset(&g_ykc_monitor_sres_billing_model_verify, 0x00, sizeof(g_ykc_monitor_sres_billing_model_verify));
    memset(&g_ykc_monitor_sres_billing_model_request, 0x00, sizeof(g_ykc_monitor_sres_billing_model_request));

#ifdef NET_YKC_MONITOR_AS_MONITOR
    memset(s_ykc_monitor_recv_message, 0x00, sizeof(s_ykc_monitor_recv_message));

    memset(&g_ykc_monitor_sres_target_plat_log, 0x00, sizeof(g_ykc_monitor_sres_target_plat_log));
    memset(&g_ykc_monitor_sres_dev_info, 0x00, sizeof(g_ykc_monitor_sres_dev_info));
    memset(&g_ykc_monitor_sreq_function_switch, 0x00, sizeof(g_ykc_monitor_sreq_function_switch));
    memset(&g_ykc_monitor_sreq_pres_info_para_modify_confirm, 0x00, sizeof(g_ykc_monitor_sreq_pres_info_para_modify_confirm));
    memset(&g_ykc_monitor_sres_info_para_confirm_result, 0x00, sizeof(g_ykc_monitor_sres_info_para_confirm_result));
    memset(&g_ykc_monitor_sreq_modify_device_info, 0x00, sizeof(g_ykc_monitor_sreq_modify_device_info));
#endif /* NET_YKC_MONITOR_AS_MONITOR */

    memset(&s_ykc_monitor_qrcode_buf, 0x00, sizeof(s_ykc_monitor_qrcode_buf));
    memset(&s_ykc_monitor_card_vin_buf, 0x00, sizeof(s_ykc_monitor_card_vin_buf));
    memset(&s_ykc_monitor_dev_info_buf, 0x00, sizeof(s_ykc_monitor_dev_info_buf));
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    memset(&s_ykc_monitor_config_info_buf, 0x00, sizeof(s_ykc_monitor_config_info_buf));
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
}
#endif /* NET_DESIGNATE_REGION */

/***********************************************
 * 函数名      ykc_monitor_clear_recv_message_item
* 功能           根据信息项的指令码清除指定的接收信息项
 **********************************************/
void ykc_monitor_clear_recv_message_item(uint8_t cmd, uint8_t gunno)
{
    uint8_t item = 0x00, index = 0x00;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    for(index = 0x00; index < YKC_RECV_MONITOR_MESSAGE_ITEM_NUM; index++){
        if((s_ykc_monitor_recv_message[index].cmd == cmd) && (s_ykc_monitor_recv_message[index].gunno == gunno)){
            item = index;
            break;
        }
    }
    if(index < YKC_RECV_MONITOR_MESSAGE_ITEM_NUM){
        for(item = index; item < YKC_RECV_MONITOR_MESSAGE_ITEM_NUM; item++){
            if(item < (YKC_RECV_MONITOR_MESSAGE_ITEM_NUM - 1)){
                s_ykc_monitor_recv_message[item].cmd = s_ykc_monitor_recv_message[item + 1].cmd;
                s_ykc_monitor_recv_message[item].gunno = s_ykc_monitor_recv_message[item + 1].gunno;
                s_ykc_monitor_recv_message[item].serial_number = s_ykc_monitor_recv_message[item + 1].serial_number;
                s_ykc_monitor_recv_message[item].result = s_ykc_monitor_recv_message[item + 1].result;
            }
        }
        memset(&s_ykc_monitor_recv_message[YKC_RECV_MONITOR_MESSAGE_ITEM_NUM - 1], 0x00, sizeof(struct ykc_monitor_recv_message));
    }
}

/***********************************************
 * 函数名      ykc_monitor_get_recv_message_item_serial_number
* 功能           根据信息项的指令码获取信息项的序列号
 **********************************************/
uint16_t ykc_monitor_get_recv_message_item_serial_number(uint8_t cmd, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    for(uint8_t index = 0; index < YKC_RECV_MONITOR_MESSAGE_ITEM_NUM; index++){
        if((s_ykc_monitor_recv_message[index].cmd == cmd) && (s_ykc_monitor_recv_message[index].gunno == gunno)){
            return s_ykc_monitor_recv_message[index].serial_number;
        }
    }
    return 0x00;
}

/***********************************************
 * 函数名      ykc_monitor_get_recv_message_item_result
* 功能           根据信息项的指令码获取信息项的结果
 **********************************************/
uint8_t ykc_monitor_get_recv_message_item_result(uint8_t cmd, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    for(uint8_t index = 0; index < YKC_RECV_MONITOR_MESSAGE_ITEM_NUM; index++){
        if((s_ykc_monitor_recv_message[index].cmd == cmd) && (s_ykc_monitor_recv_message[index].gunno == gunno)){
            return s_ykc_monitor_recv_message[index].result;
        }
    }
    return 0x00;
}

/***********************************************
 * 函数名      ykc_monitor_input_recv_message_item
* 功能           向接收信息队列中插入一个信息项
 **********************************************/
static int8_t ykc_monitor_input_recv_message_item(uint8_t cmd, uint8_t serial_number, uint8_t result, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    for(uint8_t index = 0x00; index < YKC_RECV_MONITOR_MESSAGE_ITEM_NUM; index++){
        if(s_ykc_monitor_recv_message[index].cmd == 0x00){
            s_ykc_monitor_recv_message[index].cmd = cmd;
            s_ykc_monitor_recv_message[index].gunno = gunno;
            s_ykc_monitor_recv_message[index].serial_number = serial_number;
            s_ykc_monitor_recv_message[index].result = result;
            return 0x00;
        }
    }
    return -0x01;
}

#endif /* NET_YKC_MONITOR_AS_MONITOR */

/***********************************************
 * 函数名      ykc_monitor_get_qrcode_info
* 功能           获取平台下发的二维码信息
 **********************************************/
void* ykc_monitor_get_qrcode_info(void)
{
    return &s_ykc_monitor_qrcode_buf;
}

/***********************************************
 * 函数名      ykc_monitor_get_card_vin_whitelists_info
* 功能           获取平台下发的卡、VIN码白名单信息
 **********************************************/
void* ykc_monitor_get_card_vin_whitelists_info(void)
{
    return &s_ykc_monitor_card_vin_buf;
}

/***********************************************
 * 函数名      ykc_monitor_get_device_info
* 功能           获取平台下发修改的设备信息
 **********************************************/
void* ykc_monitor_get_device_info(void)
{
    return &s_ykc_monitor_dev_info_buf;
}

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
/***********************************************
 * 函数名      ykc_monitor_get_config_info
* 功能           获取平台下发的修改、查询设备配置信息
 **********************************************/
void* ykc_monitor_get_config_info(void)
{
    return &s_ykc_monitor_config_info_buf;
}
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

/***********************************************
 * 函数名      ykc_monitor_pile_number_invalid
* 功能           判断报文桩号字段是否有效
* 参数            number   报文所含桩号
*        code      报文指令码
* 返回           1：无效       0：有效
 **********************************************/
static uint8_t ykc_monitor_pile_number_invalid(uint8_t *number, uint16_t code)
{
    if(memcmp(number, g_ykc_monitor_preq_login.body.pile_number, strlen((char*)g_ykc_monitor_preq_login.body.pile_number))){
        LOG_E("ykc monitor chargepile number error cmd|%04X [%s|%s", code, number, g_ykc_monitor_preq_login.body.pile_number);
        return 0x01;
    }

    return 0x00;
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_login
 * 功能                       处理登录响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_login(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_response_login");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_LogIn_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_response_login|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_LogIn_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRes_LogIn_t *response = (Net_YkcMonitorPro_SRes_LogIn_t*)data;

    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sres_login, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_SRES_EVENT_LOGIN);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_heartbeat
 * 功能                       处理上报心跳响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_heartbeat(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_heartbeat");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_HeartBeat_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_heartbeat|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_HeartBeat_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRes_HeartBeat_t *response = (Net_YkcMonitorPro_SRes_HeartBeat_t*)data;
    uint8_t gunno = response->body.gunno;

    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call ykc_monitor_callback_response_heartbeat|%d", gunno);
        return;
    }

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_YKC_MONITOR_SRES_EVENT_HEARTBEAT);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_billing_model_verify
 * 功能                       处理计费模型验证响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_billing_model_verify(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_billing_model_verify");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_BillingModel_Verify_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_billing_model_verify|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_BillingModel_Verify_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRes_BillingModel_Verify_t *response = (Net_YkcMonitorPro_SRes_BillingModel_Verify_t*)data;
    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sres_billing_model_verify, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_clear_message_wait_response_state(0x00, NET_YKC_MONITOR_PREQ_EVENT_BILLING_MODEL_VERIFY);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_PREQ_EVENT_BILLING_MODEL_VERIFY);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_billing_model_request
 * 功能                       处理计费模型请求响应
 *             data       数据
 *             length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_billing_model_request(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_billing_model_request");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_BillingModel_Request_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_billing_model_request|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_BillingModel_Request_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRes_BillingModel_Request_t *response = (Net_YkcMonitorPro_SRes_BillingModel_Request_t*)data;
    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sres_billing_model_request, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_clear_message_wait_response_state(0x00, NET_YKC_MONITOR_PREQ_EVENT_BILLING_MODEL_REQUEST);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_PREQ_EVENT_BILLING_MODEL_REQUEST);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_apply_charge_active
 * 功能                       处理桩主动申请充电响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_apply_charge_active(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_apply_charge_active");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_ApplyCharge_Active_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_apply_charge_active|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_ApplyCharge_Active_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRes_ApplyCharge_Active_t *response = (Net_YkcMonitorPro_SRes_ApplyCharge_Active_t*)data;
    uint8_t gunno = response->body.gunno;

    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call ykc_monitor_callback_response_apply_charge_active|%d", gunno);
        return;
    }

    memcpy(&g_ykc_monitor_sres_apply_charge_active[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_clear_message_wait_response_state((gunno - 0x01), NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_CHARGE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_YKC_MONITOR_SRES_EVENT_APPLY_CHARGE_ACTIVE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_transaction_records
 * 功能                       处理交易记录响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_transaction_records(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_transaction_records");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_TransactionRecords_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_transaction_records|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_TransactionRecords_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }

    uint8_t count = 0x00, gunno = 0x00;
    Net_YkcMonitorPro_SRes_TransactionRecords_t *response = (Net_YkcMonitorPro_SRes_TransactionRecords_t*)data;

    if(response->body.result != 0x00){
        LOG_E("ykc monitor response bill report error(%d)", response->body.result);
        for(count = 0x00; count < NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT; count++){
            rt_kprintf("[");
            rt_kprintf("%x ", response->body.serial_number[count]);
            rt_kprintf("]");
        }
        rt_kprintf("\n");
        return;
    }

    for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        if(memcmp(s_ykc_monitor_current_transaction_number[gunno], response->body.serial_number, NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT) == 0x00){
            break;
        }
    }

    if(gunno == NET_SYSTEM_GUN_NUMBER){
        LOG_E("ykc monitor response bill serial number error");
        for(count = 0x00; count < NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT; count++){
            rt_kprintf("[");
            rt_kprintf("%x ", response->body.serial_number[count]);
            rt_kprintf("]");
        }
        rt_kprintf("\n");
        return;
    }

    rt_kprintf("SDBFBvzXvDS's(%d)\n", gunno);
    ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, gunno, NET_YKC_MONITOR_SRES_EVENT_BILL_VERIFY);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_apply_merge_charge_active
 * 功能                       处理充电桩主动申请并充充电响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_apply_merge_charge_active(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_apply_merge_charge_active");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_apply_merge_charge_active|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t *response = (Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t*)data;
    uint8_t gunno = response->body.gunno;

    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call ykc_monitor_callback_response_apply_merge_charge_active|%d", gunno);
        return;
    }

    memcpy(&g_ykc_monitor_sres_apply_merge_charge_active[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_clear_message_wait_response_state((gunno - 0x01), NET_YKC_MONITOR_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_YKC_MONITOR_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_realtime_data
 * 功能                       处理读取实时数据请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_realtime_data(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_query_realtime_data");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_Query_RealTimeData_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_request_query_realtime_data|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Query_RealTimeData_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SReq_Query_RealTimeData_t *request = (Net_YkcMonitorPro_SReq_Query_RealTimeData_t*)data;
    uint8_t gunno = request->body.gunno;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call tha_callback_request_query_realtime_data|%d", gunno);
        return;
    }

    memcpy(&g_ykc_monitor_sreq_query_realtime_data[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QUERY_REALTIME_DATA);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_realtime_data
 * 功能                       处理运营平台远程控制启机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_remote_start_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_remote_start_charge");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_Remote_StartCharge_t)){
        LOG_E("ykc monitor input length error when call tha_callback_request_remote_start_charge|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Remote_StartCharge_t));
        return;
    }
    Net_YkcMonitorPro_SReq_Remote_StartCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StartCharge_t*)data;
    uint8_t gunno = request->body.gunno;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call tha_callback_request_remote_start_charge|%d", gunno);
        return;
    }
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_remote_start_charge[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_monitor_sreq_remote_start_charge[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_CHARGE);
        return;
    }

    g_ykc_monitor_sreq_remote_start_charge[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_monitor_sreq_remote_start_charge[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_CHARGE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_remote_stop_charge
 * 功能                       处理运营平台远程停机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_remote_stop_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_remote_stop_charge");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_Remote_StopCharge_t)){
        LOG_E("ykc monitor input length error when call tha_callback_request_remote_stop_charge|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Remote_StopCharge_t));
        return;
    }
    Net_YkcMonitorPro_SReq_Remote_StopCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StopCharge_t*)data;
    uint8_t gunno = request->body.gunno;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call tha_callback_request_remote_stop_charge|%d", gunno);
        return;
    }
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_remote_stop_charge[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_monitor_sreq_remote_stop_charge[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_CHARGE);
        return;
    }

    g_ykc_monitor_sreq_remote_stop_charge[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_monitor_sreq_remote_stop_charge[gunno - 0x01], data, (length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_REMOTE_STOP_CHARGE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_account_ballance_update
 * 功能                       处理远程账户余额更新请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_account_ballance_update(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_account_ballance_update");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_AccountBallance_Update_t)){
        LOG_E("ykc monitor input length error when call tha_callback_request_account_ballance_update|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_AccountBallance_Update_t));
        return;
    }
    Net_YkcMonitorPro_SReq_AccountBallance_Update_t *request = (Net_YkcMonitorPro_SReq_AccountBallance_Update_t*)data;
    uint8_t gunno = ((Net_YkcMonitorPro_SReq_AccountBallance_Update_t*)data)->body.gunno;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call tha_callback_request_account_ballance_update|%d", gunno);
        return;
    }
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_account_ballance_update[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_monitor_sreq_account_ballance_update[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE);
        return;
    }

    g_ykc_monitor_sreq_account_ballance_update[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_monitor_sreq_account_ballance_update[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_sync_offline_card
 * 功能                       处理离线卡数据同步请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_sync_offline_card(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_sync_offline_card");
        return;
    }
    uint32_t count = 0x00, total_len = 0x00;
    uint8_t *card = NULL;
    Net_YkcMonitorPro_SReq_Sync_OfflineCard_t *request = (Net_YkcMonitorPro_SReq_Sync_OfflineCard_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    card = (uint8_t*)(&(request->body.result));
    count = request->body.count;
    total_len = sizeof(Net_YkcMonitorPro_SReq_Sync_OfflineCard_t) + (2 *count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    if(length != total_len){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_sync_offline_card|%d, %d", length, total_len);
        return;
    }
    if(count <= 0x00){
        LOG_E("ykc monitor message data error when call ykc_monitor_callback_request_sync_offline_card");
        return;
    }

    g_ykc_monitor_sreq_sync_offline_card.body.result = 0x00;
    if(sizeof(s_ykc_monitor_card_vin_buf.whitlelist) < (2 *count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX)){
        g_ykc_monitor_sreq_sync_offline_card.body.result = 0x01;
        LOG_E("ykc monitor message data too long when call ykc_monitor_callback_request_sync_offline_card");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_SYNC_OFFLINE_CARD);
        return;
    }
    if(s_ykc_monitor_card_vin_buf.flag.is_used){
        g_ykc_monitor_sreq_sync_offline_card.body.result = 0x02;
        LOG_E("ykc monitor card vin whitelist is used when call ykc_monitor_callback_request_sync_offline_card");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_SYNC_OFFLINE_CARD);
        return;
    }

    s_ykc_monitor_card_vin_buf.flag.is_used = 0x01;
    s_ykc_monitor_card_vin_buf.length = (2 *count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    s_ykc_monitor_card_vin_buf.type = NET_YKC_MONITOR_WHITELIST_TYPE_CARD;
    memcpy(s_ykc_monitor_card_vin_buf.whitlelist, card, s_ykc_monitor_card_vin_buf.length);
    memcpy(&g_ykc_monitor_sreq_sync_offline_card, data, (sizeof(Net_YkcMonitorPro_SReq_Sync_OfflineCard_t) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_SYNC_OFFLINE_CARD);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_clear_offline_card
 * 功能                       处理离线卡数据清除请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_clear_offline_card(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_clear_offline_card");
        return;
    }
    uint32_t count = 0x00, total_len = 0x00;
    uint8_t *card = NULL;
    Net_YkcMonitorPro_SReq_Clear_OfflineCard_t *request = (Net_YkcMonitorPro_SReq_Clear_OfflineCard_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    card = (uint8_t*)&(request->body.result);
    count = request->body.count;
    total_len = sizeof(Net_YkcMonitorPro_SReq_Clear_OfflineCard_t) + count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX;
    if(length != total_len){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_clear_offline_card|%d, %d", length, total_len);
        return;
    }
    if(count <= 0x00){
        LOG_E("ykc monitor message data error when call ykc_monitor_callback_request_clear_offline_card");
        return;
    }
    g_ykc_monitor_sreq_clear_offline_card.body.result = 0x00;
    if(sizeof(s_ykc_monitor_card_vin_buf.whitlelist) < (count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX)){
        g_ykc_monitor_sreq_clear_offline_card.body.result = 0x01;
        LOG_E("ykc monitor message data too long when call ykc_monitor_callback_request_clear_offline_card");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_CLEAR_OFFLINE_CARD);
        return;
    }
    if(s_ykc_monitor_card_vin_buf.flag.is_used){
        g_ykc_monitor_sreq_clear_offline_card.body.result = 0x02;
        LOG_E("ykc monitor card vin whitelist is used when call ykc_monitor_callback_request_clear_offline_card");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_CLEAR_OFFLINE_CARD);
        return;
    }
    s_ykc_monitor_card_vin_buf.flag.is_used = 0x01;
    s_ykc_monitor_card_vin_buf.length = count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX;
    s_ykc_monitor_card_vin_buf.type = NET_YKC_MONITOR_WHITELIST_TYPE_CARD;
    memcpy(s_ykc_monitor_card_vin_buf.whitlelist, card, s_ykc_monitor_card_vin_buf.length);
    memcpy(&g_ykc_monitor_sreq_clear_offline_card, data, (sizeof(g_ykc_monitor_sreq_clear_offline_card) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_CLEAR_OFFLINE_CARD);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_offline_card
 * 功能                       处理离线卡数据查询请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_offline_card(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_offline_card");
        return;
    }
    uint32_t count = 0x00, total_len = 0x00;
    uint8_t *card = NULL;
    Net_YkcMonitorPro_SReq_Query_OfflineCard_t *request = (Net_YkcMonitorPro_SReq_Query_OfflineCard_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    card = (uint8_t*)&(request->body.result);
    count = request->body.count;
    total_len = sizeof(Net_YkcMonitorPro_SReq_Query_OfflineCard_t) + count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX;
    if(length != total_len){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_offline_card|%d, %d", length, total_len);
        return;
    }
    if(count <= 0x00){
        LOG_E("ykc monitor message data error when call ykc_monitor_callback_request_query_offline_card");
        return;
    }
    g_ykc_monitor_sreq_query_offline_card.body.result = 0x00;
    if(sizeof(s_ykc_monitor_card_vin_buf.whitlelist) < (count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX)){
        g_ykc_monitor_sreq_query_offline_card.body.result = 0x01;
        LOG_E("ykc monitor message data too long when call ykc_monitor_callback_request_query_offline_card");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QUERY_OFFLINE_CARD);
        return;
    }
    if(s_ykc_monitor_card_vin_buf.flag.is_used){
        g_ykc_monitor_sreq_query_offline_card.body.result = 0x02;
        LOG_E("ykc monitor card vin whitelist is used when call ykc_monitor_callback_request_query_offline_card");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QUERY_OFFLINE_CARD);
        return;
    }

    s_ykc_monitor_card_vin_buf.flag.is_used = 0x01;
    s_ykc_monitor_card_vin_buf.length = count *NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX;
    s_ykc_monitor_card_vin_buf.type = NET_YKC_MONITOR_WHITELIST_TYPE_CARD;
    memcpy(s_ykc_monitor_card_vin_buf.whitlelist, card, s_ykc_monitor_card_vin_buf.length);
    memcpy(&g_ykc_monitor_sreq_query_offline_card, data, (sizeof(g_ykc_monitor_sreq_query_offline_card) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QUERY_OFFLINE_CARD);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_set_work_para
 * 功能                       处理充电桩工作参数设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_set_work_para(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_set_work_para");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_Set_WorkPara_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_request_set_work_para|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Set_WorkPara_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SReq_Set_WorkPara_t *request = (Net_YkcMonitorPro_SReq_Set_WorkPara_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sreq_set_work_para, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_SET_WORK_PARA);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_time_sync
 * 功能                       处理对时设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_time_sync(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_time_sync");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_TimeSync_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_request_time_sync|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_TimeSync_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SReq_TimeSync_t *request = (Net_YkcMonitorPro_SReq_TimeSync_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sreq_time_sync, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_TIME_SYNC);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_billing_model_set
 * 功能                       处理计费模型设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_billing_model_set(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_billing_model_set");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SRep_BillingModel_Set_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_request_billing_model_set|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SRep_BillingModel_Set_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SRep_BillingModel_Set_t *request = (Net_YkcMonitorPro_SRep_BillingModel_Set_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sreq_billing_model_set, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_BILLING_MODEL_SET);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_ground_lock_lifting
 * 功能                       处理遥控地锁升锁与降锁命令请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_ground_lock_lifting(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_ground_lock_lifting");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_GroundLock_Lifting_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_request_ground_lock_lifting|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_GroundLock_Lifting_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcMonitorPro_SReq_GroundLock_Lifting_t *request = (Net_YkcMonitorPro_SReq_GroundLock_Lifting_t*)data;
    uint8_t gunno = ((Net_YkcMonitorPro_SReq_GroundLock_Lifting_t*)data)->body.gunno;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call tha_callback_request_ground_lock_lifting|%d", gunno);
        return;
    }

    memcpy(&g_ykc_monitor_sreq_ground_lock_lifting[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_GROUND_LOCK_LIFTING);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_remote_reboot
 * 功能                       处理远程重启请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_remote_reboot(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_remote_reboot");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_RemoteReboot_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc monitor input length error when call tha_callback_request_remote_reboot|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_RemoteReboot_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }

    Net_YkcMonitorPro_SReq_RemoteReboot_t *request = (Net_YkcMonitorPro_SReq_RemoteReboot_t*)data;
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    memcpy(&g_ykc_monitor_sreq_remote_reboot, data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_REMOTE_REBOOT);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_remote_update
 * 功能                       处理远程更新求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_remote_update(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_remote_update");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_RemoteUpdate_t)){
        LOG_E("ykc monitor input length error when call tha_callback_request_remote_update|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_RemoteUpdate_t));
        return;
    }
    Net_YkcMonitorPro_SReq_RemoteUpdate_t *request = (Net_YkcMonitorPro_SReq_RemoteUpdate_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_remote_update.body.result = 0x01;
        memcpy(&g_ykc_monitor_sreq_remote_update, data, length - 0x02);
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_REMOTE_UPDATE);
        return;
    }

    g_ykc_monitor_sreq_remote_update.body.result = 0x00;
    memcpy(&g_ykc_monitor_sreq_remote_update, data, length - 0x02);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_REMOTE_UPDATE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_remote_start_merge_charge
 * 功能                       处理运营平台远程控制并充启机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_remote_start_merge_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call tha_callback_request_remote_start_merge_charge");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t)){
        LOG_E("ykc monitor input length error when call tha_callback_request_remote_start_merge_charge|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t));
        return;
    }
    Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t*)data;
    uint8_t gunno = request->body.gunno;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call tha_callback_request_remote_start_merge_charge|%d", gunno);
        return;
    }
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_remote_start_merge_charge[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_monitor_sreq_remote_start_merge_charge[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_MERGE_CHARGE);
        return;
    }

    g_ykc_monitor_sreq_remote_start_merge_charge[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_monitor_sreq_remote_start_merge_charge[gunno - 0x01], data, length - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_MERGE_CHARGE);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_qrcode_config_gc
 * 功能                       处理运营平台二维码配置请求(国充)
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_qrcode_config_gc(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_qrcode_config_gc");
        return;
    }
    if(length < sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t)){
        LOG_E("ykc monitor input length too short when call ykc_monitor_callback_request_qrcode_config_gc|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t));
        return;
    }
    Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t *request = (Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t*)data;
    uint8_t gunno = request->body.gunno;
    uint8_t qrcode_len = (length - sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t));

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        memcpy(&(g_ykc_monitor_sreq_qrcode_config_gc[0x00]), data, (sizeof(g_ykc_monitor_sreq_qrcode_config_gc[0x00]) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
        g_ykc_monitor_sreq_qrcode_config_gc[0x00].body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
        LOG_E("ykc monitor gunno error when call ykc_monitor_callback_request_qrcode_config_gc|%d", gunno);
        return;
    }
    memcpy(&(g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01]), data, (sizeof(g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01]) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }

    if(qrcode_len == 0x00){
        LOG_E("ykc monitor qrcode content is NULL when call ykc_monitor_callback_request_qrcode_config_gc");
        g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }
    if(length > (sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t) + NET_YKC_MONITOR_QRCODE_BUF_MAX)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_qrcode_config_gc|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t) + NET_YKC_MONITOR_QRCODE_BUF_MAX));
        g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }

    g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x00;
    if(sizeof(s_ykc_monitor_qrcode_buf.qrcode) < qrcode_len){
        g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        LOG_E("ykc monitor message data too long when call ykc_monitor_callback_request_qrcode_config_gc");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }
    if(s_ykc_monitor_qrcode_buf.flag.is_used){
        g_ykc_monitor_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x02;
        LOG_E("ykc monitor qrcode buff is used when call ykc_monitor_callback_request_qrcode_config_gc");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }
    s_ykc_monitor_qrcode_buf.flag.is_used = 0x01;
    s_ykc_monitor_qrcode_buf.length = qrcode_len + 0x02;
    s_ykc_monitor_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    s_ykc_monitor_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;

    s_ykc_monitor_qrcode_buf.qrcode[0x00] = s_ykc_monitor_qrcode_buf.set_type;
    s_ykc_monitor_qrcode_buf.qrcode[0x01] = s_ykc_monitor_qrcode_buf.general_type;
    memcpy(&(s_ykc_monitor_qrcode_buf.qrcode[0x02]), &(((Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t*)data)->body.result), s_ykc_monitor_qrcode_buf.length);

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_qrcode_config_ykc15
 * 功能                       处理运营平台二维码配置请求(云快充1.5)
 *             data       数据
 *             length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_qrcode_config_ykc15(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_qrcode_config_ykc15");
        return;
    }
    if(length < sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t)){
        LOG_E("ykc monitor input length too short when call ykc_monitor_callback_request_qrcode_config_ykc15|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t));
        return;
    }
    Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t *request = (Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t*)data;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t qrcode_len = ((Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t*)data)->body.length;

    memcpy(&(g_ykc_monitor_sreq_qrcode_config_ykc15), data, (sizeof(g_ykc_monitor_sreq_qrcode_config_ykc15) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        g_ykc_monitor_sreq_qrcode_config_ykc15.body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }

    if(qrcode_len == 0x00){
        LOG_E("ykc monitor qrcode content is NULL when call ykc_monitor_callback_request_qrcode_config_ykc15");
        g_ykc_monitor_sreq_qrcode_config_ykc15.body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }
    if(length > (sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t) + NET_YKC_MONITOR_QRCODE_BUF_MAX)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_qrcode_config_ykc15|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t) + NET_YKC_MONITOR_QRCODE_BUF_MAX));
        g_ykc_monitor_sreq_qrcode_config_ykc15.body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }

    g_ykc_monitor_sreq_qrcode_config_ykc15.body.result = 0x00;
    if(sizeof(s_ykc_monitor_qrcode_buf.qrcode) < (qrcode_len + 0x02)){
        g_ykc_monitor_sreq_qrcode_config_ykc15.body.result = 0x01;
        LOG_E("ykc monitor message data too long when call ykc_monitor_callback_request_qrcode_config_ykc15");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }
    if(s_ykc_monitor_qrcode_buf.flag.is_used){
        g_ykc_monitor_sreq_qrcode_config_ykc15.body.result = 0x02;
        LOG_E("ykc monitor qrcode buff is used when call ykc_monitor_callback_request_qrcode_config_ykc15");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(g_ykc_monitor_sreq_qrcode_config_ykc15.body.format == 0x00){
        s_ykc_monitor_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX;
        s_ykc_monitor_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN;
    }else{
        s_ykc_monitor_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX;
        s_ykc_monitor_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }
#else
    if(strstr((const char*)(&(request->body.result)), (const char*)pile_number)){
        s_ykc_monitor_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
        s_ykc_monitor_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }else{
        s_ykc_monitor_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX;
        s_ykc_monitor_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

    s_ykc_monitor_qrcode_buf.qrcode[0x00] = s_ykc_monitor_qrcode_buf.set_type;
    s_ykc_monitor_qrcode_buf.qrcode[0x01] = s_ykc_monitor_qrcode_buf.general_type;

    memcpy(&(s_ykc_monitor_qrcode_buf.qrcode[0x02]), &(((Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t*)data)->body.result), s_ykc_monitor_qrcode_buf.length);

    s_ykc_monitor_qrcode_buf.flag.is_used = 0x01;
    s_ykc_monitor_qrcode_buf.length = qrcode_len + 0x02;

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_qrcode_config_tld
 * 功能                       处理运营平台二维码配置请求(特来电)
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_qrcode_config_tld(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_monitor_callback_request_qrcode_config_tld");
        return;
    }
    if(length < sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t)){
        LOG_E("ykc input length too short when call ykc_monitor_callback_request_qrcode_config_tld|%d, %d", length,
                sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t));
        return;
    }
    Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t *request = (Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t*)data;
    uint8_t gunno = request->body.gunno;
    uint8_t qrcode_len = (length - sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t));

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call ykc_monitor_callback_request_qrcode_config_tld|%d", gunno);
        memcpy(&(g_ykc_monitor_sreq_qrcode_config_tld[0x00]), data, (sizeof(g_ykc_monitor_sreq_qrcode_config_tld[0x00]) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
        g_ykc_monitor_sreq_qrcode_config_tld[0x00].body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }

    memcpy(&(g_ykc_monitor_sreq_qrcode_config_tld[gunno - 0x01]), data, (sizeof(g_ykc_monitor_sreq_qrcode_config_tld[gunno - 0x01]) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));

    if(qrcode_len == 0x00){
        LOG_E("ykc qrcode content is NULL when call ykc_monitor_callback_request_qrcode_config_tld");
        return;
    }
    if(length > (sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t) + NET_YKC_MONITOR_QRCODE_BUF_MAX)){
        LOG_E("ykc input length error when call ykc_monitor_callback_request_qrcode_config_tld|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t) + NET_YKC_MONITOR_QRCODE_BUF_MAX));
        g_ykc_monitor_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x01;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }

    g_ykc_monitor_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x00;
    if(sizeof(s_ykc_monitor_qrcode_buf.qrcode) < qrcode_len){
        g_ykc_monitor_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_monitor_callback_request_qrcode_config_tld");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }
    if(s_ykc_monitor_qrcode_buf.flag.is_used){
        g_ykc_monitor_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x02;
        LOG_E("ykc qrcode buff is used when call ykc_monitor_callback_request_qrcode_config_tld");
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }
    s_ykc_monitor_qrcode_buf.flag.is_used = 0x01;
    s_ykc_monitor_qrcode_buf.length = qrcode_len + 0x02;
    s_ykc_monitor_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    s_ykc_monitor_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;

    s_ykc_monitor_qrcode_buf.qrcode[0x00] = s_ykc_monitor_qrcode_buf.set_type;
    s_ykc_monitor_qrcode_buf.qrcode[0x01] = s_ykc_monitor_qrcode_buf.general_type;
    memcpy(&(s_ykc_monitor_qrcode_buf.qrcode[0x02]), &(((Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t*)data)->body.result), s_ykc_monitor_qrcode_buf.length);

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_TLD);
}

/******************************** 以下是监控服务回调 *******************************/
/******************************** 以下是监控服务回调 *******************************/
#ifdef NET_YKC_MONITOR_AS_MONITOR
/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_module_info
 * 功能                       处理运营平台查询模块信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_module_info(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_qrcode_config");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_module_info|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_MODULE_INFO, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_MODULE_INFO);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_module_setupinfo
 * 功能                       处理运营平台查询模块配置信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_module_setupinfo(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_module_setupinfo");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_module_setupinfo|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_MODULE_SETUPINFO, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_MODULE_SETUPINFO);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_fault_record
 * 功能                       处理运营平台查询故障记录请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_fault_record(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_fault_record");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_fault_record|%d, %d", length,
                 (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;
    uint8_t gunno = request->body.gunno;

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call ykc_monitor_callback_request_query_fault_record|%d", gunno);
        return;
    }
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_FAULT_RECORD, request->head.sequence, 0x01, (gunno - 0x01));
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_FAULT_RECORD);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_charge_record
 * 功能                       处理运营平台查询充电记录请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_charge_record(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_charge_record");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_charge_record|%d, %d", length,
                 (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;
    uint8_t gunno = request->body.gunno;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc monitor gunno error when call ykc_monitor_callback_request_query_charge_record|%d", gunno);
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_CHARGE_RECORD, request->head.sequence, 0x01, (gunno - 0x01));
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_CHARGE_RECORD);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_inputinfo_setup
 * 功能                       处理运营平台查询输入信息配置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_inputinfo_setup(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_inputinfo_setup");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_inputinfo_setup|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_INPUTINFO_SETUP, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_ININFO_SETUP);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_protectinfo_setup
 * 功能                       处理运营平台查询保护信息配置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_protectinfo_setup(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_protectinfo_setup");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_protectinfo_setup|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_PROTECTINFO_SETUP, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_PROTECTINFO_SETUP);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_function_setup
 * 功能                       处理运营平台查询功能配置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_function_setup(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_function_setup");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_function_setup|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_FUNCTION_SETUP, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_FUNCTION_SETUP);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_tsocket_info
 * 功能                       处理运营平台查询目标socket信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_tsocket_info(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_tsocket_info");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_tsocket_info|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_TSOCKET_INFO, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_TSOCKET_INFO);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_report_tplat_log
 * 功能                       处理运营平台上报目标平台日志响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_report_tplat_log(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_report_log");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_Sres_TargetPlat_Log_t)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_report_log|%d, %d", length,
                sizeof(Net_YkcMonitorPro_Sres_TargetPlat_Log_t));
        return;
    }
    uint16_t valid_len = 0x00;
    Net_YkcMonitorPro_Sres_TargetPlat_Log_t *response = (Net_YkcMonitorPro_Sres_TargetPlat_Log_t*)data;

    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    valid_len = strlen((char*)pile_number);

    valid_len = sizeof(response->body.pile_number_whole);
    valid_len = valid_len > strlen(pile_number) ? strlen(pile_number) : valid_len;
    if(memcmp(pile_number, response->body.pile_number_whole, valid_len)){
        LOG_E("ykc monitor whole chargepile number error when call ykc_monitor_callback_response_report_log");
        return;
    }
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

    memcpy(&g_ykc_monitor_sres_target_plat_log, data, length);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_USER_SRES_EVENT_REPORT_TPLAT_LOG);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_dev_info
 * 功能                       处理运营平台查询设备信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_dev_info(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_dev_info");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_dev_info|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE - 0x01));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_DVE_INFO, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_DEV_INFO);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_report_dev_info
 * 功能                       处理运营平台上报设备信息响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_report_dev_info(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_report_dev_info");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_Sres_DevInfo_t)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_report_dev_info|%d, %d", length,
                sizeof(Net_YkcMonitorPro_Sres_DevInfo_t));
        return;
    }
    uint16_t valid_len = 0x00;
    Net_YkcMonitorPro_Sres_DevInfo_t *response = (Net_YkcMonitorPro_Sres_DevInfo_t*)data;

    if(ykc_monitor_pile_number_invalid(response->body.pile_number, response->head.type)){
        return;
    }

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    valid_len = strlen((char*)pile_number);

    valid_len = sizeof(response->body.pile_number_whole);
    valid_len = valid_len > strlen(pile_number) ? strlen(pile_number) : valid_len;
    if(memcmp(pile_number, response->body.pile_number_whole, valid_len)){
        LOG_E("ykc monitor whole chargepile number error when call ykc_monitor_callback_response_report_log");
        return;
    }
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

    memcpy(&g_ykc_monitor_sres_dev_info, data, length);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_USER_SRES_EVENT_REPORT_DEV_INFO);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_function_switch
 * 功能                       处理运营平台下发的功能开关控制指令
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_function_switch(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_function_switch");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_Sreq_FunctionSwitch_t)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_function_switch|%d, %d", length,
                sizeof(Net_YkcMonitorPro_Sreq_FunctionSwitch_t));
        return;
    }

    memcpy(&g_ykc_monitor_sreq_function_switch, data, length);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_FUNCTION_SWITCH);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_info_para_modify
 * 功能                       处理运营平台下发的修改桩信息、参数指令
 *             data       数据
 *             length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_info_para_modify(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_info_para_modify");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_info_para_modify|%d, %d", length,
                sizeof(Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t));
        return;
    }

    if(g_ykc_monitor_sreq_pres_info_para_modify_confirm.ongoing){
        LOG_W("ykc monitor modify info para confirm......");
        return;
    }
    g_ykc_monitor_sreq_pres_info_para_modify_confirm.ongoing = 0x01;
    memcpy(&g_ykc_monitor_sreq_pres_info_para_modify_confirm, data, length);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_INFOPARA_MODIFY);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_info_para_confirm_result
 * 功能                       处理运营平台响应的桩信息、参数确认结果
 *             data       数据
 *             length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_info_para_confirm_result(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_info_para_confirm_result");
        return;
    }
    if(length != sizeof(Net_YkcMonitorPro_Sres_InfoPara_ConfirmResult_t)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_response_info_para_confirm_result|%d, %d", length,
                sizeof(Net_YkcMonitorPro_Sres_InfoPara_ConfirmResult_t));
        return;
    }

    if(g_ykc_monitor_sres_info_para_confirm_result.ongoing){
        LOG_W("ykc monitor modify info para confirm(result)......");
        return;
    }
    g_ykc_monitor_sres_info_para_confirm_result.ongoing = 0x01;
    memcpy(&g_ykc_monitor_sres_info_para_confirm_result, data, length);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_USER_SRES_EVENT_INFOPARA_CONFIRM_RES);
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_modify_device_info
 * 功能                       处理运营平台下发的修改设备信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_modify_device_info(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_modify_device_info");
        return;
    }
    if(length < sizeof(Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t)){
        LOG_E("ykc monitor input length too short when call ykc_monitor_callback_request_modify_device_info|%d, %d", length,
                sizeof(Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t));
        return;
    }

    Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t *info = (Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t*)data;
    if(s_ykc_monitor_dev_info_buf.flag.is_used){
        LOG_W("ykc monitor device info processing......");
        return;
    }

    memcpy(&g_ykc_monitor_sreq_modify_device_info, info, sizeof(Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t));
    /** 屏幕密码 */
    if(info->body.info_type == 0x00){
        struct screen_pw *pw = (struct screen_pw*)(&info->body.info_type + 0x01);   /** 0x01(info_type) */

        s_ykc_monitor_dev_info_buf.flag.is_used = 0x01;
        s_ykc_monitor_dev_info_buf.info_type = info->body.info_type;
        s_ykc_monitor_dev_info_buf.length = pw->dlen;
        if(s_ykc_monitor_dev_info_buf.length > NET_YKC_MONITOR_DEV_INFO_BUF_MAX){
            s_ykc_monitor_dev_info_buf.length = NET_YKC_MONITOR_DEV_INFO_BUF_MAX;
        }
        memcpy(s_ykc_monitor_dev_info_buf.info, pw->password, s_ykc_monitor_dev_info_buf.length);
        ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_MODIFY_DEV_INFO);
    }else{
        /*
        s_ykc_monitor_dev_info_buf.flag.is_used = 0x01;
        */
    }
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_billing_rule_info
 * 功能                       处理运营平台下发的查询计费规则信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_billing_rule_info(uint8_t* data, uint16_t length)
{
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_billing_rule_info");
        return;
    }
    if(length != (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ykc monitor input length error when call ykc_monitor_callback_request_query_billing_rule_info|%d, %d", length,
                (sizeof(Net_YkcMonitorPro_SReq_General_t) + NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }
    Net_YkcMonitorPro_SReq_General_t *request = (Net_YkcMonitorPro_SReq_General_t*)data;

    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        return;
    }

    ykc_monitor_input_recv_message_item(NETYKC_MONITOR_SREQCMD_QUERY_BILLING_RULE, request->head.sequence, 0x01, 0x00);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_BILLING_RULE);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_request_query_set_config_info
 * 功能                       处理运营平台下发的查询、设置配置信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_request_query_set_config_info(uint8_t* data, uint16_t length)
{
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_request_query_set_config_info");
        return;
    }
    if(length > sizeof(s_ykc_monitor_config_info_buf.info)){
        LOG_E("ykc monitor input data length out of range when call ykc_monitor_callback_request_query_set_config_info|%d, %d", \
                length, sizeof(s_ykc_monitor_config_info_buf.info));
        return;
    }

    if(s_ykc_monitor_config_info_buf.flag.is_used){
        LOG_W("ykc monitor config info processing......");
        return;
    }

    Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t *request = (Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t*)data;
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        LOG_W("ykc monitor config info pile number error(%s)", request->body.pile_number);
        return;
    }

    s_ykc_monitor_config_info_buf.flag.is_used = 0x01;
    s_ykc_monitor_config_info_buf.length = length;
    memset(s_ykc_monitor_config_info_buf.info, 0x00, sizeof(s_ykc_monitor_config_info_buf.info));
    memcpy(s_ykc_monitor_config_info_buf.info, data, length);

    ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_SET_CONFIG_INFO);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
}

/*****************************************************************
 * 函数名                   ykc_monitor_callback_response_module_fault_info
 * 功能                       处理运营平台的上报模块故障信息响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_monitor_callback_response_module_fault_info(uint8_t* data, uint16_t length)
{
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(data == NULL){
        LOG_E("ykc monitor input data is null when call ykc_monitor_callback_response_module_fault_info");
        return;
    }
    if(length < sizeof(Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t)){
        LOG_E("ykc monitor input data length out of range when call ykc_monitor_callback_response_module_fault_info|%d, %d", \
                length, sizeof(Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t));
        return;
    }

    Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t *request = (Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t*)data;
    if(ykc_monitor_pile_number_invalid(request->body.pile_number, request->head.type)){
        LOG_W("ykc monitor module fault info pile number error(%s)", request->body.pile_number);
        return;
    }
    if(request->body.info_type == 0x01){
        ykc_monitor_net_event_send(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, 0x00, NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_SET_CONFIG_INFO);
    }
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
}

#endif /* NET_PACK_USING_YKC_MONITOR */

int32_t ykc_monitor_message_recv_init(void)
{
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_SINGIN,                     ykc_monitor_callback_response_login);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_HEARTBEAT,                  ykc_monitor_callback_response_heartbeat);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_BILLING_MODEL_VERIFY,       ykc_monitor_callback_response_billing_model_verify);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_BILLING_MODEL_REQUEST,      ykc_monitor_callback_response_billing_model_request);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_APPLY_START_CHARGE,         ykc_monitor_callback_response_apply_charge_active);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_TRANSACTION_RECORD_VERIFY,  ykc_monitor_callback_response_transaction_records);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_APPLY_START_MERGECHARGE,    ykc_monitor_callback_response_apply_merge_charge_active);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_READ_REALTIME_DATA,         ykc_monitor_callback_request_query_realtime_data);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_SERVER_START_CHARGE,        ykc_monitor_callback_request_remote_start_charge);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_SERVER_STOP_CHARGE,         ykc_monitor_callback_request_remote_stop_charge);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_ACCOUNT_BALLANCE_UPDATE,    ykc_monitor_callback_request_account_ballance_update);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_SYNC_OFFLINECARD,           ykc_monitor_callback_request_sync_offline_card);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_CLEAR_OFFLINECARD,          ykc_monitor_callback_request_clear_offline_card);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_OFFLINECARD,          ykc_monitor_callback_request_query_offline_card);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_SET_WORK_PARA,              ykc_monitor_callback_request_set_work_para);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_TIME_SYNC,                  ykc_monitor_callback_request_time_sync);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_SET_BILLING_MODEL,          ykc_monitor_callback_request_billing_model_set);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_GROUNDLOCK_LIFTING,         ykc_monitor_callback_request_ground_lock_lifting);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_REMOTE_REBOOT,              ykc_monitor_callback_request_remote_reboot);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_REMOTE_UPDATE,              ykc_monitor_callback_request_remote_update);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_SERVER_START_MERGECHARGE,   ykc_monitor_callback_request_remote_start_merge_charge);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QRCODE_CONFIG_GC,           ykc_monitor_callback_request_qrcode_config_gc);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QRCODE_CONFIG_YKC15,        ykc_monitor_callback_request_qrcode_config_ykc15);
#ifdef NET_YKC_MONITOR_AS_MONITOR
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_MODULE_INFO,          ykc_monitor_callback_request_query_module_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_MODULE_SETUPINFO,     ykc_monitor_callback_request_query_module_setupinfo);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_FAULT_RECORD,         ykc_monitor_callback_request_query_fault_record);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_CHARGE_RECORD,        ykc_monitor_callback_request_query_charge_record);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_INPUTINFO_SETUP,      ykc_monitor_callback_request_query_inputinfo_setup);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_PROTECTINFO_SETUP,    ykc_monitor_callback_request_query_protectinfo_setup);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_FUNCTION_SETUP,       ykc_monitor_callback_request_query_function_setup);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_TSOCKET_INFO,         ykc_monitor_callback_request_query_tsocket_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_TARGET_PLAT_LOG,            ykc_monitor_callback_response_report_tplat_log);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_DVE_INFO,             ykc_monitor_callback_request_query_dev_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_REPORT_DVE_INFO,            ykc_monitor_callback_response_report_dev_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_FUNCTION_SWITCH,            ykc_monitor_callback_request_function_switch);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_INFOPARA_MODIFY,            ykc_monitor_callback_request_info_para_modify);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SRESCMD_INFOPARA_CONFIRM_RESULT,    ykc_monitor_callback_response_info_para_confirm_result);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_MODIFY_PILE_INFO,           ykc_monitor_callback_request_modify_device_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_BILLING_RULE,         ykc_monitor_callback_request_query_billing_rule_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_SREQCMD_QUERY_SET_CONFIG_INFO,      ykc_monitor_callback_request_query_set_config_info);
    ykc_monitor_service_callback_register(NETYKC_MONITOR_PREQ_SRESCMD_MFAULT_INFO,           ykc_monitor_callback_response_module_fault_info);
#endif /* #ifdef NET_YKC_MONITOR_AS_MONITOR */

    s_ykc_monitor_qrcode_buf.flag.is_used = 0x00;
    s_ykc_monitor_card_vin_buf.flag.is_used = 0x00;
#ifdef NET_YKC_MONITOR_AS_MONITOR
    g_ykc_monitor_sreq_pres_info_para_modify_confirm.ongoing = 0x00;
    g_ykc_monitor_sres_info_para_confirm_result.ongoing = 0x00;
#endif /* NET_YKC_MONITOR_AS_MONITOR */

    return 0x00;
}

#endif /* NET_PACK_USING_YKC_MONITOR */
