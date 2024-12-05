/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#include "ycp_message_receive.h"
#include "ycp_message_send.h"
#include "ycp_transceiver.h"
#include "net_operation.h"

#define DBG_TAG "ycp_callback"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YCP

static ycp_device_sn_buf_t s_ycp_device_sn_buf;
static ycp_qrcode_buf_t s_ycp_qrcode_buf;
static ycp_service_phone_buf_t s_ycp_service_phone_buf;
extern uint8_t s_ycp_current_transaction_number[NET_SYSTEM_GUN_NUMBER][NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];

/**=======================================[服务器请求报文]=======================================*/
/**=======================================[服务器请求报文]=======================================*/
/** 充电桩参数设置 */
Net_YcpPro_SReq_ParaSet_t g_ycp_sreq_set_para;
/** 运营平台下发二维码配置 */
Net_YcpPro_SReq_Qrcode_Config_t g_ycp_sreq_qrcode_config;
/** 客服电话设置 */
Net_YcpPro_SReq_ServicePhone_t g_ycp_sreq_set_service_phone;
/** 计费模型下发 */
Net_YcpPro_SReq_BillingModel_Set_t g_ycp_sreq_billing_model_set;
/** 查询单个枪状态 */
Net_YcpPro_SReq_Query_PileState_t g_ycp_sreq_query_device_state[NET_SYSTEM_GUN_NUMBER];
/** 查询所有枪状态 */
Net_YcpPro_SReq_Query_PileState_All_t g_ycp_sreq_query_device_state_all;
/** 远程开启充电 */
Net_YcpPro_SReq_Remote_StartCharge_t g_ycp_sreq_remote_start_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 远程结束充电 */
Net_YcpPro_SReq_Remote_StopCharge_t g_ycp_sreq_remote_stop_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 远程升级 */
Net_YcpPro_SReq_RemoteUpdate_t g_ycp_sreq_remote_update;
/** 远程重启 */
Net_YcpPro_SReq_RemoteReboot_t g_ycp_sreq_remote_reboot;
/** 远程修改联网地址 */
Net_YcpPro_SReq_Modify_ServerAddr_t g_ycp_sreq_modify_server_addr;
/** 获取当前设备故障信息 */
Net_YcpPro_SReq_Query_DeviceFault_t g_ycp_sreq_query_device_fault;

/**=======================================[服务器响应报文]=======================================*/
/**=======================================[服务器响应报文]=======================================*/
/** 登录签到响应 */
Net_YcpPro_SRes_LogIn_t g_ycp_sres_login;   // OK
/** 对时设置 */
Net_YcpPro_SRes_TimeSync_t g_ycp_sres_time_sync;   // OK
///** 心跳响应 */
//Net_YcpPro_SRes_HeartBeat_t g_ycp_sres_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证请求响应 */
Net_YcpPro_SRes_BillingModel_Verify_t g_ycp_sres_billing_model_verify;   // OK
/** 服务器回复充电桩主动请求充电请求 */
Net_YcpPro_SRes_ApplyCharge_Active_t g_ycp_sres_apply_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电结束报告应答 */
Net_YcpPro_SRes_TransactionRecords_t g_ycp_sres_transaction_records[NET_SYSTEM_GUN_NUMBER];

/***********************************************
 * 函数名      ycp_get_qrcode_info
* 功能           获取平台下发的二维码信息
 **********************************************/
void* ycp_get_qrcode_info(void)
{
    return &s_ycp_qrcode_buf;
}

/***********************************************
 * 函数名      ycp_get_service_phone_info
* 功能           获取平台下发的客服电话信息
 **********************************************/
void* ycp_get_service_phone_info(void)
{
    return &s_ycp_service_phone_buf;
}

/***********************************************
 * 函数名      ycp_get_device_sn_info
* 功能           获取设备编号信息
 **********************************************/
void* ycp_get_device_sn_info(void)
{
    return &s_ycp_device_sn_buf;
}

/*****************************************************************
 * 函数名                   ycp_callback_response_login
 * 功能                       处理登录响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_response_login(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_response_login");
        return;
    }
    if(length < (sizeof(Net_YcpPro_SRes_LogIn_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error(not enough) when call tha_callback_response_login|%d, %d", length,
                sizeof(Net_YcpPro_SRes_LogIn_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }

    uint8_t *device_sn = NULL;
    Net_YcpPro_SRes_LogIn_t *response = (Net_YcpPro_SRes_LogIn_t*)data;

    if(response->body.result != 0x01){
        LOG_E("ycp login fail");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SRes_LogIn_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE + (response->body.device_sn_length /0x02))){
        LOG_E("ycp input length error when call tha_callback_response_login|%d, %d", length,
                (sizeof(Net_YcpPro_SRes_LogIn_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE + (response->body.device_sn_length /0x02)));
        return;
    }

    device_sn = (data + sizeof(Net_YcpPro_SRes_LogIn_t));

    memset(s_ycp_device_sn_buf.device_sn, 0x00, YCP_DEVICE_SN_LENGTH_MAX);
    ycp_bcd_to_ascii(s_ycp_device_sn_buf.device_sn, (YCP_DEVICE_SN_LENGTH_MAX - 0x01), device_sn, (response->body.device_sn_length /0x02));
    s_ycp_device_sn_buf.device_sn_length = strlen((char*)s_ycp_device_sn_buf.device_sn);

    LOG_D("s_ycp_device_sn_buf.device_sn(%s)\n", s_ycp_device_sn_buf.device_sn);
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_SRES_EVENT_LOGIN);
}

/*****************************************************************
 * 函数名                   ycp_callback_response_time_sync
 * 功能                       处理对时设置响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_response_time_sync(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_time_sync");
        return;
    }
    if(length != sizeof(Net_YcpPro_SRes_TimeSync_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ycp input length error when call tha_callback_request_time_sync|%d, %d", length,
                sizeof(Net_YcpPro_SRes_TimeSync_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }

    memcpy(&g_ycp_sres_time_sync, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_clear_message_wait_response_state(0x00, NET_YCP_PREQ_EVENT_TIME_SYNC);
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_SRES_EVENT_TIME_SYNC);
}

/*****************************************************************
 * 函数名                   ycp_callback_response_heartbeat
 * 功能                       处理上报心跳响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_response_heartbeat(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_response_heartbeat");
        return;
    }
    if(length != sizeof(Net_YcpPro_SRes_HeartBeat_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE){
        LOG_E("ycp input length error when call ycp_callback_response_heartbeat|%d, %d", length,
                sizeof(Net_YcpPro_SRes_HeartBeat_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE);
        return;
    }

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_SRES_EVENT_HEARTBEAT);
}

/*****************************************************************
 * 函数名                   ycp_callback_response_billing_model_verify
 * 功能                       处理计费模型验证响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_response_billing_model_verify(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_response_billing_model_verify");
        return;
    }
    if(length < (sizeof(Net_YcpPro_Head_t) + 0x01 + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error(not enough) when call ycp_callback_response_billing_model_verify|%d, %d", length,
                (sizeof(Net_YcpPro_Head_t) + 0x01 + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sres_billing_model_verify, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_clear_message_wait_response_state(0x00, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_SRES_EVENT_BILLING_MODEL_VERIFY);
}


/*****************************************************************
 * 函数名                   ycp_callback_response_apply_charge_active
 * 功能                       处理桩主动申请充电响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_response_apply_charge_active(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_response_apply_charge_active");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SRes_ApplyCharge_Active_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call ycp_callback_response_apply_charge_active|%d, %d", length,
                (sizeof(Net_YcpPro_SRes_ApplyCharge_Active_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }
    Net_YcpPro_SRes_ApplyCharge_Active_t *response = (Net_YcpPro_SRes_ApplyCharge_Active_t*)data;

    if((response->body.gunno <= 0x00) || (response->body.gunno > NET_SYSTEM_GUN_NUMBER)){
        LOG_E("ycp gunno error when call ycp_callback_response_apply_charge_active|%d, %d", response->body.gunno,
                NET_SYSTEM_GUN_NUMBER);
        return;
    }

    memcpy(&g_ycp_sres_apply_charge_active[response->body.gunno - 0x01], data, length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE);
    ycp_clear_message_wait_response_state((response->body.gunno - 0x01), NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, (response->body.gunno - 0x01), NET_YCP_SRES_EVENT_APPLY_CHARGE_ACTIVE);
}

/*****************************************************************
 * 函数名                   ycp_callback_response_transaction_records
 * 功能                       处理交易记录响应
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_response_transaction_records(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_response_transaction_records");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SRes_TransactionRecords_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call ycp_callback_response_transaction_records|%d, %d", length,
                (sizeof(Net_YcpPro_SRes_TransactionRecords_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    uint8_t count = 0x00, gunno = 0x00;
    Net_YcpPro_SRes_TransactionRecords_t *response = (Net_YcpPro_SRes_TransactionRecords_t*)data;
    /** 只要回复了就是确认了，不管成功还是失败 */
#if 0
    if(response->body.result != 0x01){
        LOG_E("ycp response bill report error(%d)", response->body.result);
        for(count = 0x00; count < NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT; count++){
            rt_kprintf("[");
            rt_kprintf("%x ", response->body.serial_number[count]);
            rt_kprintf("]");
        }
        rt_kprintf("\n");
        return;
    }
#endif
    for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        if(memcmp(s_ycp_current_transaction_number[gunno], response->body.serial_number, NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT) == 0x00){
            break;
        }
    }

    if(gunno == NET_SYSTEM_GUN_NUMBER){
        LOG_E("ycp response bill serial number error");
        for(count = 0x00; count < NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT; count++){
            rt_kprintf("[");
            rt_kprintf("%x ", response->body.serial_number[count]);
            rt_kprintf("]");
        }
        rt_kprintf("\n");
        return;
    }

    ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_SRES_EVENT_BILL_VERIFY);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_set_para
 * 功能                       处理充电桩工作参数设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_set_para(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_set_para");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_ParaSet_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call tha_callback_request_set_para|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_ParaSet_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_set_para, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_SET_PARA);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_qrcode_config
 * 功能                       处理运营平台二维码配置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_qrcode_config(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_request_qrcode_config");
        return;
    }
    if(length < sizeof(Net_YcpPro_SReq_Qrcode_Config_t)){
        LOG_E("ycp input length error when call ycp_callback_request_qrcode_config|%d, %d", length,
                sizeof(Net_YcpPro_SReq_Qrcode_Config_t));
        return;
    }

    Net_YcpPro_SReq_Qrcode_Config_t *request = (Net_YcpPro_SReq_Qrcode_Config_t*)data;

    if(request->body.qrcode_len == 0x00){
        LOG_E("ycp qrcode content is NULL when call ycp_callback_request_qrcode_config");
        if(s_ycp_qrcode_buf.flag.is_used == 0x00){
            g_ycp_sreq_qrcode_config.body.result = 0x01;
            ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_QRCODE_CONFIG);
            return;
        }
    }
    memcpy(&(g_ycp_sreq_qrcode_config), data, (sizeof(g_ycp_sreq_qrcode_config) - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));

    if(length > (sizeof(Net_YcpPro_SReq_Qrcode_Config_t) + NET_YCP_QRCODE_BUF_MAX)){
        LOG_E("ycp input length error when call ycp_callback_request_qrcode_config|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Qrcode_Config_t) + NET_YCP_QRCODE_BUF_MAX));
        if(s_ycp_qrcode_buf.flag.is_used == 0x00){
            g_ycp_sreq_qrcode_config.body.result = 0x01;
            ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_QRCODE_CONFIG);
            return;
        }
    }

    g_ycp_sreq_qrcode_config.body.result = 0x00;
    if(sizeof(s_ycp_qrcode_buf.qrcode) < request->body.qrcode_len){
        LOG_E("ycp message data too long when call ycp_callback_request_qrcode_config");
        if(s_ycp_qrcode_buf.flag.is_used == 0x00){
            g_ycp_sreq_qrcode_config.body.result = 0x01;
            ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_QRCODE_CONFIG);
            return;
        }
    }
    if(s_ycp_qrcode_buf.flag.is_used){
        LOG_E("ycp qrcode buff is used when call ycp_callback_request_qrcode_config(%d)", s_ycp_qrcode_buf.flag.is_used);
//        return;
    }

    s_ycp_qrcode_buf.flag.is_used = 0x01;
    s_ycp_qrcode_buf.length = request->body.qrcode_len + 0x02;
    if(g_ycp_sreq_qrcode_config.body.qrcode_type == 0x00){
        s_ycp_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
        s_ycp_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }else{
        s_ycp_qrcode_buf.set_type = NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN;
        s_ycp_qrcode_buf.general_type = NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    }

    s_ycp_qrcode_buf.qrcode[0x00] = s_ycp_qrcode_buf.set_type;
    s_ycp_qrcode_buf.qrcode[0x01] = s_ycp_qrcode_buf.general_type;
    memcpy(&(s_ycp_qrcode_buf.qrcode[0x02]), &(((Net_YcpPro_SReq_Qrcode_Config_t*)data)->body.result), s_ycp_qrcode_buf.length);

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_QRCODE_CONFIG);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_set_service_phone
 * 功能                       处理运营平台设置客服电话请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_set_service_phone(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_request_set_service_phone");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_ServicePhone_t) + ((Net_YcpPro_SReq_ServicePhone_t*)data)->body.service_phone_len)){
        LOG_E("ycp input length error when call ycp_callback_request_set_service_phone|%d, %d", length, \
                (sizeof(Net_YcpPro_SReq_ServicePhone_t) + ((Net_YcpPro_SReq_ServicePhone_t*)data)->body.service_phone_len));
        return;
    }

    Net_YcpPro_SReq_ServicePhone_t *request = (Net_YcpPro_SReq_ServicePhone_t*)data;
    g_ycp_sreq_set_service_phone.body.result = 0x00;

    if(request->body.service_phone_len > NET_YCP_SERVICE_PHONE_BUF_MAX){
        g_ycp_sreq_set_service_phone.body.result = 0x01;
    }else{
        memset(s_ycp_service_phone_buf.service_phone, 0x00, NET_YCP_SERVICE_PHONE_BUF_MAX);
        memcpy(s_ycp_service_phone_buf.service_phone, &(request->body.result), request->body.service_phone_len);
        memcpy(&g_ycp_sreq_set_service_phone, request, (sizeof(g_ycp_sreq_set_service_phone) - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    }
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_SET_SERVICE_PHONE);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_billing_model_set
 * 功能                       处理计费模型设置请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_billing_model_set(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_billing_model_set");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_BillingModel_Set_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call tha_callback_request_billing_model_set|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_BillingModel_Set_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_billing_model_set, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_BILLING_MODEL_SET);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_query_state_data
 * 功能                       处理查询单个枪状态数据请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_query_state_data(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_request_query_state_data");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_Query_PileState_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call ycp_callback_request_query_state_data|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Query_PileState_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    Net_YcpPro_SReq_Query_PileState_t *request = (Net_YcpPro_SReq_Query_PileState_t*)data;

    if((request->body.gunno <= 0x00) || (request->body.gunno > NET_SYSTEM_GUN_NUMBER)){
        LOG_E("ycp gunno error when call ycp_callback_request_query_state_data|%d, %d", request->body.gunno,
                NET_SYSTEM_GUN_NUMBER);
        return;
    }

    memcpy(&g_ycp_sreq_query_device_state[request->body.gunno], data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, (request->body.gunno - 0x01), NET_YCP_SREQ_EVENT_QUERY_DEVICE_STATE);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_query_state_data_all
 * 功能                       处理查询所有枪状态数据请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_query_state_data_all(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_request_query_state_data_all");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_Query_PileState_All_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call ycp_callback_request_query_state_data_all|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Query_PileState_All_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_query_device_state_all, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_QUERY_DEVICE_STATE_ALL);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_query_realtime_data
 * 功能                       处理运营平台远程控制启机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_remote_start_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_remote_start_charge");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_Remote_StartCharge_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call tha_callback_request_remote_start_charge|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Remote_StartCharge_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    Net_YcpPro_SReq_Remote_StartCharge_t *request = (Net_YcpPro_SReq_Remote_StartCharge_t*)data;

    if((request->body.gunno <= 0x00) || (request->body.gunno > NET_SYSTEM_GUN_NUMBER)){
        LOG_E("ycp gunno error when call tha_callback_request_remote_start_charge|%d, %d", request->body.gunno,
                NET_SYSTEM_GUN_NUMBER);
        return;
    }

    memcpy(&g_ycp_sreq_remote_start_charge[request->body.gunno - 0x01], data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, (request->body.gunno - 0x01), NET_YCP_SREQ_EVENT_REMOTE_START_CHARGE);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_remote_stop_charge
 * 功能                       处理运营平台远程停机请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_remote_stop_charge(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_remote_stop_charge");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_Remote_StopCharge_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call tha_callback_request_remote_stop_charge|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Remote_StopCharge_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    Net_YcpPro_SReq_Remote_StopCharge_t *request = (Net_YcpPro_SReq_Remote_StopCharge_t*)data;

    if((request->body.gunno <= 0x00) || (request->body.gunno > NET_SYSTEM_GUN_NUMBER)){
        LOG_E("ycp gunno error when call tha_callback_request_remote_stop_charge|%d, %d", request->body.gunno,
                NET_SYSTEM_GUN_NUMBER);
        return;
    }

    memcpy(&g_ycp_sreq_remote_stop_charge[request->body.gunno - 0x01], data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, (request->body.gunno - 0x01), NET_YCP_SREQ_EVENT_REMOTE_STOP_CHARGE);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_remote_update
 * 功能                       处理远程更新求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_remote_update(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_remote_update");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_RemoteUpdate_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call tha_callback_request_remote_update|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_RemoteUpdate_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_remote_update, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_REMOTE_UPDATE);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_remote_reboot
 * 功能                       处理远程重启请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_remote_reboot(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call tha_callback_request_remote_reboot");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_RemoteReboot_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call tha_callback_request_remote_reboot|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_RemoteReboot_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_remote_reboot, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_REMOTE_REBOOT);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_set_server_addr
 * 功能                       处理修改服务器地址请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_set_server_addr(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_request_set_server_addr");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_Modify_ServerAddr_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call ycp_callback_request_set_server_addr|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Modify_ServerAddr_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_modify_server_addr, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_MODIFY_SERVICE_ADDR);
}

/*****************************************************************
 * 函数名                   ycp_callback_request_query_device_fault
 * 功能                       处理查询设备故障信息请求
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void ycp_callback_request_query_device_fault(uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("ycp input data is null when call ycp_callback_request_query_device_fault");
        return;
    }
    if(length != (sizeof(Net_YcpPro_SReq_Query_DeviceFault_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE)){
        LOG_E("ycp input length error when call ycp_callback_request_query_device_fault|%d, %d", length,
                (sizeof(Net_YcpPro_SReq_Query_DeviceFault_t) + NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
        return;
    }

    memcpy(&g_ycp_sreq_query_device_fault, data, (length - NET_YCP_PROTOCOL_CHECK_REGION_SIZE));
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_SREQ_EVENT_QUERY_DEVICE_FAULT);
}

int32_t ycp_message_recv_init(void)
{
    ycp_service_callback_register(NETYCP_SRESCMD_SINGIN,                     ycp_callback_response_login);
    ycp_service_callback_register(NETYCP_SRESCMD_TIME_SYNC,                  ycp_callback_response_time_sync);
    ycp_service_callback_register(NETYCP_SRESCMD_HEARTBEAT,                  ycp_callback_response_heartbeat);
    ycp_service_callback_register(NETYCP_SRESCMD_BILLING_MODEL_VERIFY,       ycp_callback_response_billing_model_verify);
    ycp_service_callback_register(NETYCP_SRESCMD_APPLY_START_CHARGE,         ycp_callback_response_apply_charge_active);
    ycp_service_callback_register(NETYCP_SRESCMD_TRANSACTION_RECORD_VERIFY,  ycp_callback_response_transaction_records);
    ycp_service_callback_register(NETYCP_SREQCMD_SET_PARA,                   ycp_callback_request_set_para);
    ycp_service_callback_register(NETYCP_SREQCMD_QRCODE_CONFIG,              ycp_callback_request_qrcode_config);
    ycp_service_callback_register(NETYCP_SREQCMD_SET_SERVER_PHONE,           ycp_callback_request_set_service_phone);
    ycp_service_callback_register(NETYCP_SREQCMD_BILLING_MODEL_SET,          ycp_callback_request_billing_model_set);
    ycp_service_callback_register(NETYCP_SREQCMD_QUERY_STATE_DATA,           ycp_callback_request_query_state_data);
    ycp_service_callback_register(NETYCP_SREQCMD_QUERY_STATE_DATA_ALL,       ycp_callback_request_query_state_data_all);
    ycp_service_callback_register(NETYCP_SREQCMD_SERVER_START_CHARGE,        ycp_callback_request_remote_start_charge);
    ycp_service_callback_register(NETYCP_SREQCMD_SERVER_STOP_CHARGE,         ycp_callback_request_remote_stop_charge);
    ycp_service_callback_register(NETYCP_SREQCMD_REMOTE_UPDATE,              ycp_callback_request_remote_update);
    ycp_service_callback_register(NETYCP_SREQCMD_REMOTE_REBOOT,              ycp_callback_request_remote_reboot);
    ycp_service_callback_register(NETYCP_SREQCMD_MODIFY_SERVER_ADDR,         ycp_callback_request_set_server_addr);
    ycp_service_callback_register(NETYCP_SREQCMD_QUERY_DEVICE_FAULT,         ycp_callback_request_query_device_fault);

    s_ycp_qrcode_buf.flag.is_used = 0x00;

    return 0x00;
}

#endif /* NET_PACK_USING_YCP */
