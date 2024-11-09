/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#include "ykc_message_receive.h"
#include "ykc_message_send.h"
#include "ykc_transceiver.h"
#include "net_operation.h"

#define DBG_TAG "ykc_callback"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YKC

static ykc_qrcode_buf_t s_ykc_qrcode_buf;
static ykc_card_vin_buf_t s_ykc_card_vin_buf;

extern uint8_t s_ykc_current_transaction_number[NET_SYSTEM_GUN_NUMBER][NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];

/**=======================================[服务器请求报文]=======================================*/
/**=======================================[服务器请求报文]=======================================*/
/** 读取实时数据 */
Net_YkcPro_SReq_Query_RealTimeData_t g_ykc_sreq_query_realtime_data[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程控制启机 */
Net_YkcPro_SReq_Remote_StartCharge_t g_ykc_sreq_remote_start_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程停机 */
Net_YkcPro_SReq_Remote_StopCharge_t g_ykc_sreq_remote_stop_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 远程账户余额更新 */
Net_YkcPro_SReq_AccountBallance_Update_t g_ykc_sreq_account_ballance_update[NET_SYSTEM_GUN_NUMBER];   // OK
/** 离线卡数据同步 */
Net_YkcPro_SReq_Sync_OfflineCard_t g_ykc_sreq_sync_offline_card;   // OK
/** 离线卡数据清除 */
Net_YkcPro_SReq_Clear_OfflineCard_t g_ykc_sreq_clear_offline_card;   // OK
/** 离线卡数据查询 */
Net_YkcPro_SReq_Query_OfflineCard_t g_ykc_sreq_query_offline_card;   // OK
/** 充电桩工作参数设置 */
Net_YkcPro_SReq_Set_WorkPara_t g_ykc_sreq_set_work_para;   // OK
/** 对时设置 */
Net_YkcPro_SReq_TimeSync_t g_ykc_sreq_time_sync;   // OK
/** 计费模型设置 */
Net_YkcPro_SRep_BillingModel_Set_t g_ykc_sreq_billing_model_set;   // OK
/** 遥控地锁升锁与降锁命令 */
Net_YkcPro_SReq_GroundLock_Lifting_t g_ykc_sreq_ground_lock_lifting[NET_SYSTEM_GUN_NUMBER];
/** 远程重启 */
Net_YkcPro_SReq_RemoteReboot_t g_ykc_sreq_remote_reboot;
/** 远程更新 */
Net_YkcPro_SReq_RemoteUpdate_t g_ykc_sreq_remote_update;
/** 运营平台远程控制并充启机 */
Net_YkcPro_SReq_Remote_StartMergeCharge_t g_ykc_sreq_remote_start_merge_charge[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(国充) */
Net_YkcPro_SReq_Qrcode_Config_GC_t g_ykc_sreq_qrcode_config_gc[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(云快充1.5) */
Net_YkcPro_SReq_Qrcode_Config_Ykc15_t g_ykc_sreq_qrcode_config_ykc15;
/** 运营平台下发二维码配置(特来电) */
Net_YkcPro_SReq_Qrcode_Config_Tld_t g_ykc_sreq_qrcode_config_tld[NET_SYSTEM_GUN_NUMBER];

/**=======================================[服务器响应报文]=======================================*/
/**=======================================[服务器响应报文]=======================================*/
/** 登录签到响应 */
Net_YkcPro_SRes_LogIn_t g_ykc_sres_login;   // OK
///** 心跳响应 */
//Net_YkcPro_SRes_HeartBeat_t g_ykc_sres_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证请求响应 */
Net_YkcPro_SRes_BillingModel_Verify_t g_ykc_sres_billing_model_verify;   // OK
/** 计费模型请求响应 */
Net_YkcPro_SRes_BillingModel_Request_t g_ykc_sres_billing_model_request;   // OK
/** 交易记录响应 */
Net_YkcPro_SRes_TransactionRecords_t g_ykc_sres_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 充电桩主动申请启动充电响应 */
Net_YkcPro_SRes_ApplyCharge_Active_t g_ykc_sres_apply_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电桩主动申请并充充电响应 */
Net_YkcPro_SRes_ApplyMergeCharge_Active_t g_ykc_sres_apply_merge_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK

/***********************************************
 * 函数名      ykc_get_qrcode_info
* 功能           获取平台下发的二维码信息
 **********************************************/
void* ykc_get_qrcode_info(void)
{
    return &s_ykc_qrcode_buf;
}

/***********************************************
 * 函数名      ykc_get_card_vin_whitelists_info
* 功能           获取平台下发的卡、VIN码白名单信息
 **********************************************/
void* ykc_get_card_vin_whitelists_info(void)
{
    return &s_ykc_card_vin_buf;
}

/*****************************************************************
 * 函数名                   ykc_callback_response_login
 * 功能                       处理登录响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_login(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_response_login");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_LogIn_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_response_login|%d, %d", length,
                sizeof(Net_YkcPro_SRes_LogIn_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SRes_LogIn_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_response_login");
        return;
    }

    memcpy(&g_ykc_sres_login, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_SRES_EVENT_LOGIN);
}

/*****************************************************************
 * 函数名                   ykc_callback_response_heartbeat
 * 功能                       处理上报心跳响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_heartbeat(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_response_heartbeat");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_HeartBeat_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call ykc_callback_response_heartbeat|%d, %d", length,
                sizeof(Net_YkcPro_SRes_HeartBeat_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    Net_YkcPro_SRes_HeartBeat_t *response = (Net_YkcPro_SRes_HeartBeat_t*)data;
    uint8_t gunno = response->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, response->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_response_heartbeat");
        for(uint8_t i = 0x00; i < NET_YKC_CHARGEPILE_LENGTH_DEFAULT; i++){
            printf("%x(%x) ", pile_numberbcd[i], response->body.pile_number[i]);
        }
        printf("\n(%s)\n", pile_number);
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call ykc_callback_response_heartbeat|%d", gunno);
        return;
    }

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_YKC_SRES_EVENT_HEARTBEAT);
}

/*****************************************************************
 * 函数名                   ykc_callback_response_billing_model_verify
 * 功能                       处理计费模型验证响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_billing_model_verify(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_response_billing_model_verify");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_BillingModel_Verify_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call ykc_callback_response_billing_model_verify|%d, %d", length,
                sizeof(Net_YkcPro_SRes_BillingModel_Verify_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SRes_BillingModel_Verify_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_response_billing_model_verify");
        return;
    }

    memcpy(&g_ykc_sres_billing_model_verify, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_clear_message_wait_response_state(0x00, NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY);
}

/*****************************************************************
 * 函数名                   ykc_callback_response_billing_model_request
 * 功能                       处理计费模型请求响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_billing_model_request(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_response_billing_model_request");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_BillingModel_Request_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call ykc_callback_response_billing_model_request|%d, %d", length,
                sizeof(Net_YkcPro_SRes_BillingModel_Request_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SRes_BillingModel_Request_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_response_billing_model_request");
        return;
    }

    memcpy(&g_ykc_sres_billing_model_request, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_clear_message_wait_response_state(0x00, NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST);
}

/*****************************************************************
 * 函数名                   ykc_callback_response_apply_charge_active
 * 功能                       处理桩主动申请充电响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_apply_charge_active(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_response_apply_charge_active");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_ApplyCharge_Active_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call ykc_callback_response_apply_charge_active|%d, %d", length,
                sizeof(Net_YkcPro_SRes_ApplyCharge_Active_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SRes_ApplyCharge_Active_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SRes_ApplyCharge_Active_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_response_apply_charge_active");
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call ykc_callback_response_apply_charge_active|%d", gunno);
        return;
    }

    memcpy(&g_ykc_sres_apply_charge_active[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_clear_message_wait_response_state((gunno - 0x01), NET_YKC_PREQ_EVENT_APPLY_START_CHARGE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_YKC_SRES_EVENT_APPLY_CHARGE_ACTIVE);
}

/*****************************************************************
 * 函数名                   ykc_callback_response_transaction_records
 * 功能                       处理交易记录响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_transaction_records(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_response_transaction_records");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_TransactionRecords_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call ykc_callback_response_transaction_records|%d, %d", length,
                sizeof(Net_YkcPro_SRes_TransactionRecords_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }

    uint8_t count = 0x00, gunno = 0x00;
    Net_YkcPro_SRes_TransactionRecords_t *response = (Net_YkcPro_SRes_TransactionRecords_t*)data;

    if(response->body.result != 0x00){
        LOG_E("ykc response bill report error(%d)", response->body.result);
        for(count = 0x00; count < NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT; count++){
            rt_kprintf("[");
            rt_kprintf("%x ", response->body.serial_number[count]);
            rt_kprintf("]");
        }
        rt_kprintf("\n");
        return;
    }

    for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        if(memcmp(s_ykc_current_transaction_number[gunno], response->body.serial_number, NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT) == 0x00){
            break;
        }
    }

    if(gunno == NET_SYSTEM_GUN_NUMBER){
        LOG_E("ykc response bill serial number error");
        for(count = 0x00; count < NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT; count++){
            rt_kprintf("[");
            rt_kprintf("%x ", response->body.serial_number[count]);
            rt_kprintf("]");
        }
        rt_kprintf("\n");
        return;
    }

    ykc_clear_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_TRANSACTION_RECORD);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_SRES_EVENT_BILL_VERIFY);
}

/*****************************************************************
 * 函数名                   ykc_callback_response_apply_merge_charge_active
 * 功能                       处理充电桩主动申请并充充电响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_response_apply_merge_charge_active(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_response_apply_merge_charge_active");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRes_ApplyMergeCharge_Active_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call ykc_callback_response_apply_merge_charge_active|%d, %d", length,
                sizeof(Net_YkcPro_SRes_ApplyMergeCharge_Active_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SRes_ApplyMergeCharge_Active_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SRes_ApplyMergeCharge_Active_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_response_apply_merge_charge_active");
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call ykc_callback_response_apply_merge_charge_active|%d", gunno);
        return;
    }
    memcpy(&g_ykc_sres_apply_merge_charge_active[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_clear_message_wait_response_state((gunno - 0x01), NET_YKC_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, (gunno - 0x01), NET_YKC_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_query_realtime_data
 * 功能                       处理读取实时数据请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_query_realtime_data(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_query_realtime_data");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_Query_RealTimeData_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_request_query_realtime_data|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Query_RealTimeData_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SReq_Query_RealTimeData_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Query_RealTimeData_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_query_realtime_data");
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call tha_callback_request_query_realtime_data|%d", gunno);
        return;
    }

    memcpy(&g_ykc_sreq_query_realtime_data[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_QUERY_REALTIME_DATA);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_query_realtime_data
 * 功能                       处理运营平台远程控制启机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_remote_start_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_remote_start_charge");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_Remote_StartCharge_t)){
        LOG_E("ykc input length error when call tha_callback_request_remote_start_charge|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Remote_StartCharge_t));
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SReq_Remote_StartCharge_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    Net_YkcPro_SReq_Remote_StartCharge_t *request = (Net_YkcPro_SReq_Remote_StartCharge_t*)data;
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call tha_callback_request_remote_start_charge|%d", gunno);
        return;
    }
    if(memcmp(pile_numberbcd, request->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_remote_start_charge");
        g_ykc_sreq_remote_start_charge[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_sreq_remote_start_charge[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_REMOTE_START_CHARGE);
        return;
    }

    g_ykc_sreq_remote_start_charge[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_sreq_remote_start_charge[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_REMOTE_START_CHARGE);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_remote_stop_charge
 * 功能                       处理运营平台远程停机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_remote_stop_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_remote_stop_charge");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_Remote_StopCharge_t)){
        LOG_E("ykc input length error when call tha_callback_request_remote_stop_charge|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Remote_StopCharge_t));
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SReq_Remote_StopCharge_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call tha_callback_request_remote_stop_charge|%d", gunno);
        return;
    }
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Remote_StopCharge_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_remote_stop_charge");
        g_ykc_sreq_remote_stop_charge[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_sreq_remote_stop_charge[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_REMOTE_START_CHARGE);
        return;
    }

    g_ykc_sreq_remote_stop_charge[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_sreq_remote_stop_charge[gunno - 0x01], data, (length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_REMOTE_STOP_CHARGE);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_account_ballance_update
 * 功能                       处理远程账户余额更新请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_account_ballance_update(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_account_ballance_update");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_AccountBallance_Update_t)){
        LOG_E("ykc input length error when call tha_callback_request_account_ballance_update|%d, %d", length,
                sizeof(Net_YkcPro_SReq_AccountBallance_Update_t));
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SReq_AccountBallance_Update_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call tha_callback_request_account_ballance_update|%d", gunno);
        return;
    }
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_AccountBallance_Update_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_account_ballance_update");
        g_ykc_sreq_account_ballance_update[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_sreq_account_ballance_update[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE);
        return;
    }

    g_ykc_sreq_account_ballance_update[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_sreq_account_ballance_update[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_sync_offline_card
 * 功能                       处理离线卡数据同步请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_sync_offline_card(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_request_sync_offline_card");
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT), count = 0x00, total_len = 0x00;
    uint8_t *card = NULL;
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Sync_OfflineCard_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_request_sync_offline_card");
        return;
    }

    card = (uint8_t*)(&(((Net_YkcPro_SReq_Sync_OfflineCard_t*)data)->body.result));
    count = ((Net_YkcPro_SReq_Sync_OfflineCard_t*)data)->body.count;
    total_len = sizeof(Net_YkcPro_SReq_Sync_OfflineCard_t) + (2 *count *NET_YKC_CARD_NUMBER_LENGTH_MAX);
    if(length != total_len){
        LOG_E("ykc input length error when call ykc_callback_request_sync_offline_card|%d, %d", length, total_len);
        return;
    }
    if(count <= 0x00){
        LOG_E("ykc message data error when call ykc_callback_request_sync_offline_card");
        return;
    }

    g_ykc_sreq_sync_offline_card.body.result = 0x00;
    if(sizeof(s_ykc_card_vin_buf.whitlelist) < (2 *count *NET_YKC_CARD_NUMBER_LENGTH_MAX)){
        g_ykc_sreq_sync_offline_card.body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_callback_request_sync_offline_card");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_SYNC_OFFLINE_CARD);
        return;
    }
    if(s_ykc_card_vin_buf.flag.is_used){
        g_ykc_sreq_sync_offline_card.body.result = 0x02;
        LOG_E("ykc card vin whitelist is used when call ykc_callback_request_sync_offline_card");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_SYNC_OFFLINE_CARD);
        return;
    }

    s_ykc_card_vin_buf.flag.is_used = 0x01;
    s_ykc_card_vin_buf.length = (2 *count *NET_YKC_CARD_NUMBER_LENGTH_MAX);
    s_ykc_card_vin_buf.type = NET_YKC_WHITELIST_TYPE_CARD;
    memcpy(s_ykc_card_vin_buf.whitlelist, card, s_ykc_card_vin_buf.length);
    memcpy(&g_ykc_sreq_sync_offline_card, data, (sizeof(Net_YkcPro_SReq_Sync_OfflineCard_t) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_SYNC_OFFLINE_CARD);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_clear_offline_card
 * 功能                       处理离线卡数据清除请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_clear_offline_card(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_request_clear_offline_card");
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT), count = 0x00, total_len = 0x00;
    uint8_t *card = NULL;
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    Net_YkcPro_SReq_Clear_OfflineCard_t *request = (Net_YkcPro_SReq_Clear_OfflineCard_t*)data;
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, request->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_request_clear_offline_card");
        return;
    }

    card = (uint8_t*)&(request->body.result);
    count = request->body.count;
    total_len = sizeof(Net_YkcPro_SReq_Clear_OfflineCard_t) + count *NET_YKC_CARD_NUMBER_LENGTH_MAX;
    if(length != total_len){
        LOG_E("ykc input length error when call ykc_callback_request_clear_offline_card|%d, %d", length, total_len);
        return;
    }
    if(count <= 0x00){
        LOG_E("ykc message data error when call ykc_callback_request_clear_offline_card");
        return;
    }
    g_ykc_sreq_clear_offline_card.body.result = 0x00;
    if(sizeof(s_ykc_card_vin_buf.whitlelist) < (count *NET_YKC_CARD_NUMBER_LENGTH_MAX)){
        g_ykc_sreq_clear_offline_card.body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_callback_request_clear_offline_card");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_CLEAR_OFFLINE_CARD);
        return;
    }
    if(s_ykc_card_vin_buf.flag.is_used){
        g_ykc_sreq_clear_offline_card.body.result = 0x02;
        LOG_E("ykc card vin whitelist is used when call ykc_callback_request_clear_offline_card");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_CLEAR_OFFLINE_CARD);
        return;
    }
    s_ykc_card_vin_buf.flag.is_used = 0x01;
    s_ykc_card_vin_buf.length = count *NET_YKC_CARD_NUMBER_LENGTH_MAX;
    s_ykc_card_vin_buf.type = NET_YKC_WHITELIST_TYPE_CARD;
    memcpy(s_ykc_card_vin_buf.whitlelist, card, s_ykc_card_vin_buf.length);
    memcpy(&g_ykc_sreq_clear_offline_card, data, (sizeof(g_ykc_sreq_clear_offline_card) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_CLEAR_OFFLINE_CARD);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_query_offline_card
 * 功能                       处理离线卡数据查询请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_query_offline_card(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_request_query_offline_card");
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT), count = 0x00, total_len = 0x00;
    uint8_t *card = NULL;
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    Net_YkcPro_SReq_Query_OfflineCard_t *request = (Net_YkcPro_SReq_Query_OfflineCard_t*)data;
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, request->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_request_query_offline_card");
        return;
    }

    card = (uint8_t*)&(request->body.result);
    count = request->body.count;
    total_len = sizeof(Net_YkcPro_SReq_Query_OfflineCard_t) + count *NET_YKC_CARD_NUMBER_LENGTH_MAX;
    if(length != total_len){
        LOG_E("ykc input length error when call ykc_callback_request_query_offline_card|%d, %d", length, total_len);
        return;
    }
    if(count <= 0x00){
        LOG_E("ykc message data error when call ykc_callback_request_query_offline_card");
        return;
    }
    g_ykc_sreq_query_offline_card.body.result = 0x00;
    if(sizeof(s_ykc_card_vin_buf.whitlelist) < (count *NET_YKC_CARD_NUMBER_LENGTH_MAX)){
        g_ykc_sreq_query_offline_card.body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_callback_request_query_offline_card");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QUERY_OFFLINE_CARD);
        return;
    }
    if(s_ykc_card_vin_buf.flag.is_used){
        g_ykc_sreq_query_offline_card.body.result = 0x02;
        LOG_E("ykc card vin whitelist is used when call ykc_callback_request_query_offline_card");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QUERY_OFFLINE_CARD);
        return;
    }

    s_ykc_card_vin_buf.flag.is_used = 0x01;
    s_ykc_card_vin_buf.length = count *NET_YKC_CARD_NUMBER_LENGTH_MAX;
    s_ykc_card_vin_buf.type = NET_YKC_WHITELIST_TYPE_CARD;
    memcpy(s_ykc_card_vin_buf.whitlelist, card, s_ykc_card_vin_buf.length);
    memcpy(&g_ykc_sreq_query_offline_card, data, (sizeof(g_ykc_sreq_query_offline_card) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QUERY_OFFLINE_CARD);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_set_work_para
 * 功能                       处理充电桩工作参数设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_set_work_para(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_set_work_para");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_Set_WorkPara_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_request_set_work_para|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Set_WorkPara_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Set_WorkPara_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_set_work_para");
        return;
    }

    memcpy(&g_ykc_sreq_set_work_para, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_SET_WORK_PARA);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_time_sync
 * 功能                       处理对时设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_time_sync(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_time_sync");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_TimeSync_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_request_time_sync|%d, %d", length,
                sizeof(Net_YkcPro_SReq_TimeSync_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_TimeSync_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_time_sync");
        return;
    }

    memcpy(&g_ykc_sreq_time_sync, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_TIME_SYNC);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_billing_model_set
 * 功能                       处理计费模型设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_billing_model_set(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_billing_model_set");
        return;
    }
    if(length != sizeof(Net_YkcPro_SRep_BillingModel_Set_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_request_billing_model_set|%d, %d", length,
                sizeof(Net_YkcPro_SRep_BillingModel_Set_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SRep_BillingModel_Set_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_billing_model_set");
        return;
    }

    memcpy(&g_ykc_sreq_billing_model_set, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_BILLING_MODEL_SET);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_ground_lock_lifting
 * 功能                       处理遥控地锁升锁与降锁命令请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_ground_lock_lifting(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_ground_lock_lifting");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_GroundLock_Lifting_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_request_ground_lock_lifting|%d, %d", length,
                sizeof(Net_YkcPro_SReq_GroundLock_Lifting_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SReq_GroundLock_Lifting_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_GroundLock_Lifting_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_ground_lock_lifting");
        return;
    }
    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call tha_callback_request_ground_lock_lifting|%d", gunno);
        return;
    }

    memcpy(&g_ykc_sreq_ground_lock_lifting[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_GROUND_LOCK_LIFTING);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_remote_reboot
 * 功能                       处理远程重启请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_remote_reboot(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_remote_reboot");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_RemoteReboot_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ykc input length error when call tha_callback_request_remote_reboot|%d, %d", length,
                sizeof(Net_YkcPro_SReq_RemoteReboot_t) + NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_RemoteReboot_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_remote_reboot");
        return;
    }

    memcpy(&g_ykc_sreq_remote_reboot, data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_REMOTE_REBOOT);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_remote_update
 * 功能                       处理远程更新求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_remote_update(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_remote_update");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_RemoteUpdate_t)){
        LOG_E("ykc input length error when call tha_callback_request_remote_update|%d, %d", length,
                sizeof(Net_YkcPro_SReq_RemoteUpdate_t));
        return;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_RemoteUpdate_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_remote_update");
        g_ykc_sreq_remote_update.body.result = 0x01;
        memcpy(&g_ykc_sreq_remote_update, data, length - 0x02);
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_REMOTE_UPDATE);
        return;
    }

    g_ykc_sreq_remote_update.body.result = 0x00;
    memcpy(&g_ykc_sreq_remote_update, data, length - 0x02);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_REMOTE_UPDATE);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_remote_start_merge_charge
 * 功能                       处理运营平台远程控制并充启机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_remote_start_merge_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call tha_callback_request_remote_start_merge_charge");
        return;
    }
    if(length != sizeof(Net_YkcPro_SReq_Remote_StartMergeCharge_t)){
        LOG_E("ykc input length error when call tha_callback_request_remote_start_merge_charge|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Remote_StartMergeCharge_t));
        return;
    }
    uint8_t gunno = ((Net_YkcPro_SReq_Remote_StartMergeCharge_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint16_t valid_len = strlen((char*)pile_number);

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call tha_callback_request_remote_start_merge_charge|%d", gunno);
        return;
    }
    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Remote_StartMergeCharge_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call tha_callback_request_remote_start_merge_charge");
        g_ykc_sreq_remote_start_merge_charge[gunno - 0x01].body.result = 0x01;
        memcpy(&g_ykc_sreq_remote_start_merge_charge[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_REMOTE_START_MERGE_CHARGE);
        return;
    }

    g_ykc_sreq_remote_start_merge_charge[gunno - 0x01].body.result = 0x00;
    memcpy(&g_ykc_sreq_remote_start_merge_charge[gunno - 0x01], data, length - NET_YKC_PROTOCOL_CHECK_REGION_SIZE);
    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_REMOTE_START_MERGE_CHARGE);
}


/*****************************************************************
 * 函数名                   ykc_callback_request_qrcode_config_gc
 * 功能                       处理运营平台二维码配置请求(国充)
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_qrcode_config_gc(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_request_qrcode_config_gc");
        return;
    }
    if(length < sizeof(Net_YkcPro_SReq_Qrcode_Config_GC_t)){
        LOG_E("ykc input length too short when call ykc_callback_request_qrcode_config_gc|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Qrcode_Config_GC_t));
        return;
    }

    uint8_t gunno = ((Net_YkcPro_SReq_Qrcode_Config_GC_t*)data)->body.gunno;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint8_t qrcode_len = (length - sizeof(Net_YkcPro_SReq_Qrcode_Config_GC_t));
    uint16_t valid_len = strlen((char*)pile_number);

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call ykc_monitor_callback_request_qrcode_config_gc|%d", gunno);
        memcpy(&(g_ykc_sreq_qrcode_config_gc[0x00]), data, (sizeof(g_ykc_sreq_qrcode_config_gc[0x00]) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));
        g_ykc_sreq_qrcode_config_gc[0x00].body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }

    memcpy(&(g_ykc_sreq_qrcode_config_gc[gunno - 0x01]), data, (sizeof(g_ykc_sreq_qrcode_config_gc[gunno - 0x01]) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));
    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Qrcode_Config_GC_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_monitor_callback_request_qrcode_config_gc");
        g_ykc_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }

    if(qrcode_len == 0x00){
        LOG_E("ykc qrcode content is NULL when call ykc_callback_request_qrcode_config_gc");
        return;
    }
    if(length > (sizeof(Net_YkcPro_SReq_Qrcode_Config_GC_t) + NET_YKC_QRCODE_BUF_MAX)){
        LOG_E("ykc input length error when call ykc_callback_request_qrcode_config_gc|%d, %d", length,
                (sizeof(Net_YkcPro_SReq_Qrcode_Config_GC_t) + NET_YKC_QRCODE_BUF_MAX));
        g_ykc_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }

    g_ykc_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x00;
    if(sizeof(s_ykc_qrcode_buf.qrcode) < qrcode_len){
        g_ykc_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_callback_request_qrcode_config_gc");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }
    if(s_ykc_qrcode_buf.flag.is_used){
        g_ykc_sreq_qrcode_config_gc[gunno - 0x01].body.result = 0x02;
        LOG_E("ykc qrcode buff is used when call ykc_callback_request_qrcode_config_gc");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC);
        return;
    }
    s_ykc_qrcode_buf.flag.is_used = 0x01;
    s_ykc_qrcode_buf.length = qrcode_len + 0x02;
    s_ykc_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    s_ykc_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;

    s_ykc_qrcode_buf.qrcode[0x00] = s_ykc_qrcode_buf.set_type;
    s_ykc_qrcode_buf.qrcode[0x01] = s_ykc_qrcode_buf.general_type;
    memcpy(&(s_ykc_qrcode_buf.qrcode[0x02]), &(((Net_YkcPro_SReq_Qrcode_Config_GC_t*)data)->body.result), s_ykc_qrcode_buf.length);

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_qrcode_config_ykc15
 * 功能                       处理运营平台二维码配置请求(云快充1.5)
 *             data       数据
 *             length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_qrcode_config_ykc15(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_request_qrcode_config_ykc15");
        return;
    }
    if(length < sizeof(Net_YkcPro_SReq_Qrcode_Config_Ykc15_t)){
        LOG_E("ykc input length too short when call ykc_callback_request_qrcode_config_ykc15|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Qrcode_Config_Ykc15_t));
        return;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint8_t pile_numberbcd[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];
    uint8_t qrcode_len = ((Net_YkcPro_SReq_Qrcode_Config_Ykc15_t*)data)->body.length;
    uint16_t valid_len = strlen((char*)pile_number);

    memcpy(&(g_ykc_sreq_qrcode_config_ykc15), data, (sizeof(g_ykc_sreq_qrcode_config_ykc15) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));
    valid_len = valid_len > (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_ascii_to_bcd((uint8_t*)pile_number, valid_len, pile_numberbcd, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    if(memcmp(pile_numberbcd, ((Net_YkcPro_SReq_Qrcode_Config_Ykc15_t*)data)->body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT)){
        LOG_E("ykc chargepile number error when call ykc_callback_request_qrcode_config_ykc15");
        g_ykc_sreq_qrcode_config_ykc15.body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }

    if(qrcode_len == 0x00){
        LOG_E("ykc qrcode content is NULL when call ykc_callback_request_qrcode_config_ykc15");
        g_ykc_sreq_qrcode_config_ykc15.body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }
    if(length > (sizeof(Net_YkcPro_SReq_Qrcode_Config_Ykc15_t) + NET_YKC_QRCODE_BUF_MAX)){
        LOG_E("ykc input length error when call ykc_callback_request_qrcode_config_ykc15|%d, %d", length,
                (sizeof(Net_YkcPro_SReq_Qrcode_Config_Ykc15_t) + NET_YKC_QRCODE_BUF_MAX));
        g_ykc_sreq_qrcode_config_ykc15.body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }

    g_ykc_sreq_qrcode_config_ykc15.body.result = 0x00;
    if(sizeof(s_ykc_qrcode_buf.qrcode) < (qrcode_len + 0x02)){
        g_ykc_sreq_qrcode_config_ykc15.body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_callback_request_qrcode_config_ykc15");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }
    if(s_ykc_qrcode_buf.flag.is_used){
        g_ykc_sreq_qrcode_config_ykc15.body.result = 0x02;
        LOG_E("ykc qrcode buff is used when call ykc_callback_request_qrcode_config_ykc15");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15);
        return;
    }

    if(strstr((const char*)(&(((Net_YkcPro_SReq_Qrcode_Config_Ykc15_t*)data)->body.result)), (const char*)pile_number)){
        s_ykc_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
        s_ykc_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }else{
        s_ykc_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX;
        s_ykc_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }

    s_ykc_qrcode_buf.qrcode[0x00] = s_ykc_qrcode_buf.set_type;
    s_ykc_qrcode_buf.qrcode[0x01] = s_ykc_qrcode_buf.general_type;

    memcpy(&(s_ykc_qrcode_buf.qrcode[0x02]), &(((Net_YkcPro_SReq_Qrcode_Config_Ykc15_t*)data)->body.result), qrcode_len);

    s_ykc_qrcode_buf.flag.is_used = 0x01;
    s_ykc_qrcode_buf.length = qrcode_len + 0x02;

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15);
}

/*****************************************************************
 * 函数名                   ykc_callback_request_qrcode_config_tld
 * 功能                       处理运营平台二维码配置请求(特来电)
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ykc_callback_request_qrcode_config_tld(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ykc input data is null when call ykc_callback_request_qrcode_config_tld");
        return;
    }
    if(length < sizeof(Net_YkcPro_SReq_Qrcode_Config_Tld_t)){
        LOG_E("ykc input length too short when call ykc_callback_request_qrcode_config_tld|%d, %d", length,
                sizeof(Net_YkcPro_SReq_Qrcode_Config_Tld_t));
        return;
    }

    uint8_t gunno = ((Net_YkcPro_SReq_Qrcode_Config_Tld_t*)data)->body.gunno;
    uint8_t qrcode_len = (length - sizeof(Net_YkcPro_SReq_Qrcode_Config_Tld_t));

    if(!((gunno > 0x00) && (gunno <= NET_SYSTEM_GUN_NUMBER))){
        LOG_E("ykc gunno error when call ykc_callback_request_qrcode_config_tld|%d", gunno);
        memcpy(&(g_ykc_sreq_qrcode_config_tld[0x00]), data, (sizeof(g_ykc_sreq_qrcode_config_tld[0x00]) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));
        g_ykc_sreq_qrcode_config_tld[0x00].body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }

    memcpy(&(g_ykc_sreq_qrcode_config_tld[gunno - 0x01]), data, (sizeof(g_ykc_sreq_qrcode_config_tld[gunno - 0x01]) - NET_YKC_PROTOCOL_CHECK_REGION_SIZE));

    if(qrcode_len == 0x00){
        LOG_E("ykc qrcode content is NULL when call ykc_callback_request_qrcode_config_tld");
        return;
    }
    if(length > (sizeof(Net_YkcPro_SReq_Qrcode_Config_Tld_t) + NET_YKC_QRCODE_BUF_MAX)){
        LOG_E("ykc input length error when call ykc_callback_request_qrcode_config_tld|%d, %d", length,
                (sizeof(Net_YkcPro_SReq_Qrcode_Config_Tld_t) + NET_YKC_QRCODE_BUF_MAX));
        g_ykc_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x01;
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }

    g_ykc_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x00;
    if(sizeof(s_ykc_qrcode_buf.qrcode) < qrcode_len){
        g_ykc_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x01;
        LOG_E("ykc message data too long when call ykc_callback_request_qrcode_config_tld");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }
    if(s_ykc_qrcode_buf.flag.is_used){
        g_ykc_sreq_qrcode_config_tld[gunno - 0x01].body.result = 0x02;
        LOG_E("ykc qrcode buff is used when call ykc_callback_request_qrcode_config_tld");
        ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_SREQ_EVENT_QRCODE_CONFIG_TLD);
        return;
    }
    s_ykc_qrcode_buf.flag.is_used = 0x01;
    s_ykc_qrcode_buf.length = qrcode_len + 0x02;
    s_ykc_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    s_ykc_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;

    s_ykc_qrcode_buf.qrcode[0x00] = s_ykc_qrcode_buf.set_type;
    s_ykc_qrcode_buf.qrcode[0x01] = s_ykc_qrcode_buf.general_type;
    memcpy(&(s_ykc_qrcode_buf.qrcode[0x02]), &(((Net_YkcPro_SReq_Qrcode_Config_Tld_t*)data)->body.result), s_ykc_qrcode_buf.length);

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, (gunno - 0x01), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_TLD);
}

int32_t ykc_message_recv_init(void)
{
    ykc_service_callback_register(NETYKC_SRESCMD_SINGIN,                     ykc_callback_response_login);
    ykc_service_callback_register(NETYKC_SRESCMD_HEARTBEAT,                  ykc_callback_response_heartbeat);
    ykc_service_callback_register(NETYKC_SRESCMD_BILLING_MODEL_VERIFY,       ykc_callback_response_billing_model_verify);
    ykc_service_callback_register(NETYKC_SRESCMD_BILLING_MODEL_REQUEST,      ykc_callback_response_billing_model_request);
    ykc_service_callback_register(NETYKC_SRESCMD_APPLY_START_CHARGE,         ykc_callback_response_apply_charge_active);
    ykc_service_callback_register(NETYKC_SRESCMD_TRANSACTION_RECORD_VERIFY,  ykc_callback_response_transaction_records);
    ykc_service_callback_register(NETYKC_SRESCMD_APPLY_START_MERGECHARGE,    ykc_callback_response_apply_merge_charge_active);
    ykc_service_callback_register(NETYKC_SREQCMD_READ_REALTIME_DATA,         ykc_callback_request_query_realtime_data);
    ykc_service_callback_register(NETYKC_SREQCMD_SERVER_START_CHARGE,        ykc_callback_request_remote_start_charge);
    ykc_service_callback_register(NETYKC_SREQCMD_SERVER_STOP_CHARGE,         ykc_callback_request_remote_stop_charge);
    ykc_service_callback_register(NETYKC_SREQCMD_ACCOUNT_BALLANCE_UPDATE,    ykc_callback_request_account_ballance_update);
    ykc_service_callback_register(NETYKC_SREQCMD_SYNC_OFFLINECARD,           ykc_callback_request_sync_offline_card);
    ykc_service_callback_register(NETYKC_SREQCMD_CLEAR_OFFLINECARD,          ykc_callback_request_clear_offline_card);
    ykc_service_callback_register(NETYKC_SREQCMD_QUERY_OFFLINECARD,          ykc_callback_request_query_offline_card);
    ykc_service_callback_register(NETYKC_SREQCMD_SET_WORK_PARA,              ykc_callback_request_set_work_para);
    ykc_service_callback_register(NETYKC_SREQCMD_TIME_SYNC,                  ykc_callback_request_time_sync);
    ykc_service_callback_register(NETYKC_SREQCMD_SET_BILLING_MODEL,          ykc_callback_request_billing_model_set);
    ykc_service_callback_register(NETYKC_SREQCMD_GROUNDLOCK_LIFTING,         ykc_callback_request_ground_lock_lifting);
    ykc_service_callback_register(NETYKC_SREQCMD_REMOTE_REBOOT,              ykc_callback_request_remote_reboot);
    ykc_service_callback_register(NETYKC_SREQCMD_REMOTE_UPDATE,              ykc_callback_request_remote_update);
    ykc_service_callback_register(NETYKC_SREQCMD_SERVER_START_MERGECHARGE,   ykc_callback_request_remote_start_merge_charge);
    ykc_service_callback_register(NETYKC_SREQCMD_QRCODE_CONFIG_GC,           ykc_callback_request_qrcode_config_gc);
    ykc_service_callback_register(NETYKC_SREQCMD_QRCODE_CONFIG_YKC15,        ykc_callback_request_qrcode_config_ykc15);
    ykc_service_callback_register(NETYKC_SREQCMD_QRCODE_CONFIG_TLD,          ykc_callback_request_qrcode_config_tld);

    s_ykc_qrcode_buf.flag.is_used = 0x00;
    s_ykc_card_vin_buf.flag.is_used = 0x00;

    return 0x00;
}

#endif /* NET_PACK_USING_YKC */


