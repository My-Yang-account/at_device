/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#include "tha_message_receive.h"
#include "tha_message_send.h"
#include "tha_transceiver.h"

#define DBG_TAG "tha_callback"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_THA

#define THA_RECV_MESSAGE_ITEM_NUM          0x05

struct tha_recv_message{
    uint8_t cmd;
    uint8_t status;
    uint8_t gunno;
    uint8_t serial_number;
};

static struct tha_recv_message s_tha_recv_message[THA_RECV_MESSAGE_ITEM_NUM];

/** 登录签到响应 */
Net_ThaPro_SRes_LogIn_t g_tha_sres_login[NET_SYSTEM_GUN_NUMBER];   // OK
/** 权限认证响应 */
Net_ThaPro_SRes_Authority_t g_tha_sres_authority[NET_SYSTEM_GUN_NUMBER];   // OK
/** 请求下载升级包响应 */
Net_ThaPro_SRes_UpdatePack_Load_t g_tha_sres_update_pack_load;   /* 请求下载升级包同时最多一把枪 */  // OK
/** 查询ICCID对应的桩编号响应 */
Net_ThaPro_SRes_Query_PileNumberOfIccid_t g_tha_sres_query_pile_number_of_iccid;        // OK
/** VIN码鉴权响应 */
Net_ThaPro_SRes_VinAuthorize_t g_tha_sres_vin_authorize[NET_SYSTEM_GUN_NUMBER];        // OK
/** 时间同步 */
Net_ThaPro_SReqPRes_TimeSync_t g_tha_sreq_time_sync;
/** 系统复位 */
Net_ThaPro_SReqPRes_SystemReboot_t g_tha_sreq_system_reboot;
/** 用户绑定 */
Net_ThaPro_SReq_User_Binding_t g_tha_sreq_user_binding;
/** 更新系统信息 */
Net_ThaPro_SReq_Updated_SystemInfo_t g_tha_sreq_updated_system_info;
/** 更新计费规则 */
Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t g_tha_sreq_updated_billing_rule[NET_SYSTEM_GUN_NUMBER];
/** 更新充电系统信息 */
Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t g_tha_sreq_updated_charge_system_config;
/** 查询订单状态 */
Net_ThaPro_SReq_Query_OrderState_t g_tha_sreq_query_order_state;
/** 停止充电 */
Net_ThaPro_SReq_StopCharge_t g_tha_sreq_stop_charge[NET_SYSTEM_GUN_NUMBER];
/** 启动远程升级 */
Net_ThaPro_SReq_StartUpdate_t g_tha_sreq_start_update[NET_SYSTEM_GUN_NUMBER];
/** 设置桩运行参数 */
Net_ThaPro_SReq_Set_Pile_RunningPara_t g_tha_sreq_set_pile_running_para[NET_SYSTEM_GUN_NUMBER];
/** 设置充电枪充电功率 */
Net_ThaPro_SReq_Set_Pile_ChargePower_t g_tha_sreq_set_pile_charge_power[NET_SYSTEM_GUN_NUMBER];
/** 解锁充电 */
Net_ThaPro_SReq_UnlockCharge_t g_tha_sreq_unlock_charge[NET_SYSTEM_GUN_NUMBER];
/** 平台下发处理用参数 */
Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t g_tha_sreq_issued_process_para;
/** 设备查询最新可用升级包软件版本号响应帧 */
Net_ThaPro_SRes_Query_Available_UpdatePackVer_t g_tha_sres_query_available_update_pack_ver;

#ifdef NET_THA_PRO_USING_AC
/** 设置定时充电 */
Net_ThaPro_SReq_Set_Reservation_t g_tha_sreq_set_reservation[NET_SYSTEM_GUN_NUMBER];
/** 取消定时充电 */
Net_ThaPro_SReq_Cancel_Reservation_t g_tha_sreq_cancel_reservation[NET_SYSTEM_GUN_NUMBER];
/** 私桩定时充电预约 */
Net_ThaPro_SReq_Set_PrivateReservation_t g_tha_sreq_set_private_reservation;
/** 私桩取消特定ID的定时充电预约 */
Net_ThaPro_SReq_Cancel_PrivateReservation_t g_tha_sreq_cancel_private_reservatione;
/** 私桩(桩端)查询定时充电预约响应 */
Net_ThaPro_SRes_Query_PrivateReservation_t g_tha_sres_query_private_reservation;
/** 私桩PWM电流值下发 */
Net_ThaPro_SReq_IssudePwm_t g_tha_sreq_issude_pwm;
#endif /* NET_THA_PRO_USING_AC */

/***********************************************
 * 函数名      tha_clear_recv_message_item
* 功能           根据信息项的指令码清除指定的接收信息项
 **********************************************/
void tha_clear_recv_message_item(uint8_t cmd, uint8_t gunno)
{
    uint8_t item = 0x00, index = 0x00;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    for(index = 0x00; index < THA_RECV_MESSAGE_ITEM_NUM; index++){
        if((s_tha_recv_message[index].cmd == cmd) && (s_tha_recv_message[index].gunno == gunno)){
            item = index;
            break;
        }
    }
    if(index < THA_RECV_MESSAGE_ITEM_NUM){
        for(item = index; item < THA_RECV_MESSAGE_ITEM_NUM; item++){
            if(item < (THA_RECV_MESSAGE_ITEM_NUM - 1)){
                s_tha_recv_message[item].cmd = s_tha_recv_message[item + 1].cmd;
                s_tha_recv_message[item].status = s_tha_recv_message[item + 1].status;
                s_tha_recv_message[item].gunno = s_tha_recv_message[item + 1].gunno;
                s_tha_recv_message[item].serial_number = s_tha_recv_message[item + 1].serial_number;
            }
        }
        memset(&s_tha_recv_message[THA_RECV_MESSAGE_ITEM_NUM - 1], 0x00, sizeof(s_tha_recv_message[THA_RECV_MESSAGE_ITEM_NUM - 1]));
    }
}
/***********************************************
 * 函数名      tha_get_recv_message_item_status
* 功能           根据信息项的指令码获取信息项的状态码
 **********************************************/
int16_t tha_get_recv_message_item_status(uint8_t cmd, uint8_t gunno, uint8_t *status)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    for(uint8_t index = 0; index < THA_RECV_MESSAGE_ITEM_NUM; index++){
        if((s_tha_recv_message[index].cmd == cmd) && (s_tha_recv_message[index].gunno == gunno)){
            if(status){
                *status = s_tha_recv_message[index].status;
            }
            return 0x00;
        }
    }
    return -0x01;
}
/***********************************************
 * 函数名      tha_get_recv_message_item_serial_number
* 功能           根据信息项的指令码获取信息项的序列号
 **********************************************/
int16_t tha_get_recv_message_item_serial_number(uint8_t cmd, uint8_t gunno, uint8_t *serial_number)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    for(uint8_t index = 0; index < THA_RECV_MESSAGE_ITEM_NUM; index++){
        if((s_tha_recv_message[index].cmd == cmd) && (s_tha_recv_message[index].gunno == gunno)){
            if(serial_number){
                *serial_number = s_tha_recv_message[index].serial_number;
            }
            return 0x00;
        }
    }
    return -0x01;
}
/***********************************************
 * 函数名      tha_input_recv_message_item
* 功能           向接收信息队列中插入一个信息项
 **********************************************/
static int8_t tha_input_recv_message_item(uint8_t cmd, uint8_t status, uint8_t serial_number, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    for(uint8_t index = 0x00; index < THA_RECV_MESSAGE_ITEM_NUM; index++){
        if(s_tha_recv_message[index].cmd == 0x00){
            s_tha_recv_message[index].cmd = cmd;
            s_tha_recv_message[index].status = status;
            s_tha_recv_message[index].gunno = gunno;
            s_tha_recv_message[index].serial_number = serial_number;
            return 0x00;
        }
    }
    return -0x01;
}

/***********************************************
 * 函数名      tha_message_status_check
* 功能           对接收到的报文进行初次检查
 **********************************************/
static uint32_t tha_message_status_check(uint8_t gunno, uint8_t* data, uint16_t length)
{
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_message_status_check");
        return NET_THA_STATUS_CODE_EXECUTE_FAIL;
    }
    if(((Net_ThaPro_Head_t*)data)->status != NET_THA_STATUS_CODE_NORMAL){
        LOG_E("thaisen message status code error|%x cmd|%x", ((Net_ThaPro_Head_t*)data)->status, ((Net_ThaPro_Head_t*)data)->cmd);
        return ((Net_ThaPro_Head_t*)data)->status;
    }
    if(length < sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input data len is too short when call tha_message_status_check|%d", length);
        return NET_THA_STATUS_CODE_FORMAT_ERROR;
    }

    if(((Net_ThaPro_Head_t*)data)->cmd != NET_THA_MESSAGE_CMD_UPDATE_PACK_LOAD){
        uint8_t check_code = 0;
        check_code = tha_get_check_code(g_tha_sres_login[gunno].body.random_sign, data, sizeof(Net_ThaPro_Head_t) - 1);
        check_code = tha_get_check_code(check_code, (data + sizeof(Net_ThaPro_Head_t)), (length - sizeof(Net_ThaPro_Head_t)));
        if(check_code != ((Net_ThaPro_Head_t*)data)->check_sum){
            LOG_E("thaisen message check error(%x, %x) cmd|%x", check_code, ((Net_ThaPro_Head_t*)data)->check_sum,
                    ((Net_ThaPro_Head_t*)data)->cmd);
            return NET_THA_STATUS_CODE_REFUSE_SERVICE;
        }
    }
    return NET_THA_STATUS_CODE_NORMAL;
}

/*****************************************************************
 * 函数名                   tha_callback_response_login
 * 功能                       处理登录响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 *           status     报文状态
 * 返回                        无
 ****************************************************************/

static void tha_callback_response_login(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_login");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(length != sizeof(Net_ThaPro_SRes_LogIn_t)){
        LOG_E("thaisen input length error when call tha_callback_response_login|%d, %d", length,
                sizeof(Net_ThaPro_SRes_LogIn_t));
        g_tha_sres_login[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_LOGIN);
        return;
    }

    g_tha_sres_login[gunno].body.random_sign = ((Net_ThaPro_SRes_LogIn_t*)data)->body.random_sign;

    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sres_login[gunno].head.status = status;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_LOGIN);
        return;
    }

    memcpy(&g_tha_sres_login[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_LOGIN);
}
/*****************************************************************
 * 函数名                   tha_callback_response_authority
 * 功能                       处理权限认证响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_authority(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_authority");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_SRes_Authority_t)){
        LOG_E("thaisen input length error when call tha_callback_response_authority|%d, %d", length,
                sizeof(Net_ThaPro_SRes_Authority_t));
        return;
    }
    if(gunno != ((Net_ThaPro_SRes_Authority_t*)data)->body.gunno){
        LOG_E("thaisen gunno error|%d, %d when call tha_callback_response_authority", gunno, ((Net_ThaPro_SRes_Authority_t*)data)->body.gunno);
        return;
    }

    memcpy(&g_tha_sres_authority[gunno], data, length);
    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_AUTHORIZE);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_AUTHORITY);
}
/*****************************************************************
 * 函数名                   tha_callback_response_report_charge_data
 * 功能                       处理上报充电数据响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_report_charge_data(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_report_charge_data");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_report_charge_data|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_CHARGE_DATA);
}
/*****************************************************************
 * 函数名                   tha_callback_response_history_order
 * 功能                       处理上报历史订单响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_history_order(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL, count = 0x00;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_history_order");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_history_order|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    const tha_transaction_info_t* transaction = (const tha_transaction_info_t*)(tha_get_transaction_info(gunno));
    for(count = 0x00; count < NET_THA_TRANSACTION_REPORT_COUNT_MAX; count++){
        if(transaction[count].sn == ((Net_ThaPro_Head_t*)data)->sequence){
            break;
        }
    }
    if(count < NET_THA_TRANSACTION_REPORT_COUNT_MAX){
        tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_HISTORY_ORDER);
    }
}
/*****************************************************************
 * 函数名                   tha_callback_response_heartbeat
 * 功能                       处理上报心跳响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_heartbeat(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_heartbeat");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_heartbeat|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_HEARTBEAT);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_HEARTBEAT);
}
/*****************************************************************
 * 函数名                   tha_callback_response_update_pack_load
 * 功能                       处请求下载升级包响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_update_pack_load(uint8_t gunno, uint8_t* data, uint16_t length)
{
//    net_message_event_send(DEVICE_LINK_PLATFORM_TARGET, EVENT_HANDLE_SERVER, EVENT_TYPE_RESPONSE, DEVICE_GUNNO_A,
//            EVENT_SERVER_RESPONSE_UPDATEPACK_LOAD);
}
/*****************************************************************
 * 函数名                   tha_callback_response_report_battery_state
 * 功能                       处理上报上报电池状态响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_report_battery_state(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_report_battery_state");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_report_battery_state|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_BATTERY_STATE);
}
/*****************************************************************
 * 函数名                   tha_callback_response_stop_charge_complete
 * 功能                       处理上报停止充电完成响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_stop_charge_complete(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL, count = 0x00;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_stop_charge_complete");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_stop_charge_complete|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    const tha_transaction_info_t* transaction = (const tha_transaction_info_t*)(tha_get_transaction_info(gunno));
    for(count = 0x00; count < NET_THA_TRANSACTION_REPORT_COUNT_MAX; count++){
        if(transaction[count].sn == ((Net_ThaPro_Head_t*)data)->sequence){
            break;
        }
    }
    if(count < NET_THA_TRANSACTION_REPORT_COUNT_MAX){
        tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_STOP_CHARGE_COMPLETE);
    }
}
/*****************************************************************
 * 函数名                   tha_callback_response_event_state
 * 功能                       处理上报事件响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_event_state(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_event_state");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_event_state|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_EVENT_STATE);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_EVENT_STATE);
}
/*****************************************************************
 * 函数名                   tha_callback_response_error_info
 * 功能                       处理上报错误信息响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_error_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_error_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_GeneralResReq_t)){
        LOG_E("thaisen input length error when call tha_callback_response_error_info|%d, %d", length,
                sizeof(Net_ThaPro_GeneralResReq_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_ERROR_INFO);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_ERROR_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_response_updatepack_loadresult
 * 功能                       处理上报升级包下载结果响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_updatepack_loadresult(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_updatepack_loadresult");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_updatepack_loadresult|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_UPDATE_RESULT);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_UPDATE_RESULT);
}
/*****************************************************************
 * 函数名                   tha_callback_response_query_pile_number_of_iccid
 * 功能                       处理查询ICCID对应的桩编号响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_query_pile_number_of_iccid(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_query_pile_number_of_iccid");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_SRes_Query_PileNumberOfIccid_t)){
        LOG_E("thaisen input length error when call tha_callback_response_query_pile_number_of_iccid|%d, %d", length,
                sizeof(Net_ThaPro_SRes_Query_PileNumberOfIccid_t));
        return;
    }

    memcpy(&g_tha_sres_query_pile_number_of_iccid, data, length);
    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_AUTHORIZE);   // 待修改
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_PILENUMBER_OF_ICCID);
}
#if 0
/*****************************************************************
 * 函数名                   tha_callback_response_report_process_para
 * 功能                       处理设备上报处理用参数响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_report_process_para(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_report_process_para");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_report_process_para|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_EVENT_STATE);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_PROCESS_PARA);
}
#endif
/*****************************************************************
 * 函数名                   tha_callback_response_vin_authorize
 * 功能                       处理VIN 码鉴权响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_vin_authorize(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_vin_authorize");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_SRes_VinAuthorize_t)){
        LOG_E("thaisen input length error when call tha_callback_response_vin_authorize|%d, %d", length,
                sizeof(Net_ThaPro_SRes_VinAuthorize_t));
        return;
    }
    if(gunno != ((Net_ThaPro_SRes_VinAuthorize_t*)data)->body.gunno){
        LOG_E("thaisen gunno error|%d, %d when call tha_callback_response_vin_authorize", gunno, ((Net_ThaPro_SRes_VinAuthorize_t*)data)->body.gunno);
        return;
    }

    memcpy(&g_tha_sres_vin_authorize[gunno], data, length);
    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_VIN_AUTHORIZE);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_VIN_AUTHORIZE);
}
/*****************************************************************
 * 函数名                   tha_callback_response_net_info
 * 功能                       处理上报网络信息响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_net_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_net_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_net_info|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_DEVICE_NET_INFO);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_NET_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_response_report_battery_info
 * 功能                       处理上报电池信息响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_report_battery_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_report_battery_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_GeneralResReq_t)){
        LOG_E("thaisen input length error when call tha_callback_response_report_battery_info|%d, %d", length,
                sizeof(Net_ThaPro_GeneralResReq_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_BATTERY_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_response_device_fault_info
 * 功能                       处理上报设备故障信息响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_device_fault_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_device_fault_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_device_fault_info|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_DEVICE_FAULT_INFO);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_DEVICE_FAULT_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_response_report_car_warnning_info
 * 功能                       处理上报车辆告警信息响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_report_car_warnning_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_report_car_warnning_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_report_car_warnning_info|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }
    if(gunno != ((Net_ThaPro_SRes_Authority_t*)data)->body.gunno){
        LOG_E("thaisen gunno error|%d, %d when call tha_callback_response_report_car_warnning_info", gunno, ((Net_ThaPro_SRes_Authority_t*)data)->body.gunno);
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_CAR_WARNNING_INFO);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_CAR_WARNNING_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_response_report_real_temperation
 * 功能                       处理上报充电中充电枪的实时温度响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_report_real_temperation(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_report_real_temperation");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_report_real_temperation|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }
    if(gunno != ((Net_ThaPro_SRes_Authority_t*)data)->body.gunno){
        LOG_E("thaisen gunno error|%d, %d when call tha_callback_response_report_real_temperation", gunno, ((Net_ThaPro_SRes_Authority_t*)data)->body.gunno);
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_GUN_REALTEMP);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_REPORT_REAL_TEMPERATION);
}
/*****************************************************************
 * 函数名                   tha_callback_request_time_sync
 * 功能                       处理时间同步请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_time_sync(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_time_sync");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_time_sync.head.status = status;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_TIME_SYNC);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReqPRes_TimeSync_t)){
        LOG_E("thaisen input length error when call tha_callback_request_time_sync|%d, %d", length,
                sizeof(Net_ThaPro_SReqPRes_TimeSync_t));
        g_tha_sreq_time_sync.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_TIME_SYNC);
        return;
    }

    memcpy(&g_tha_sreq_time_sync, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_TIME_SYNC);
}
/*****************************************************************
 * 函数名                   tha_callback_request_system_reboot
 * 功能                       处理系统重启请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_system_reboot(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_system_reboot");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_system_reboot.head.status = status;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SYSTEM_REBOOT);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReqPRes_SystemReboot_t)){
        LOG_E("thaisen input length error when call tha_callback_request_system_reboot|%d, %d", length,
                sizeof(Net_ThaPro_SReqPRes_SystemReboot_t));
        g_tha_sreq_system_reboot.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SYSTEM_REBOOT);
        return;
    }

    memcpy(&g_tha_sreq_system_reboot, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SYSTEM_REBOOT);
}
/*****************************************************************
 * 函数名                   tha_callback_request_query_charge_data
 * 功能                       处理查询充电数据请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_charge_data(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_charge_data");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_CHARGE_DATA);
        return;
    }
    if(length != sizeof(Net_ThaPro_GeneralResReq_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_charge_data|%d, %d", length,
                sizeof(Net_ThaPro_GeneralResReq_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_CHARGE_DATA);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_CHARGE_DATA);
}
#if 0
/*****************************************************************
 * 函数名                   tha_callback_request_user_binding
 * 功能                       处理用户绑定请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_user_binding(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_user_binding");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_READ_CHARGE_DATA);
        return;
    }
    if(length != sizeof(Net_ThaPro_GeneralResReq_t)){
        LOG_E("thaisen input length error when call tha_callback_request_user_binding|%d, %d", length,
                sizeof(Net_ThaPro_GeneralResReq_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_READ_CHARGE_DATA);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_READ_CHARGE_DATA);
}
#endif
/*****************************************************************
 * 函数名                   tha_callback_request_query_system_info
 * 功能                       处理查询系统信息请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_system_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_system_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_SYSTEM_INFO);
        return;
    }
    if(length != sizeof(Net_ThaPro_GeneralResReq_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_system_info|%d, %d", length,
                sizeof(Net_ThaPro_GeneralResReq_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_SYSTEM_INFO);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_SYSTEM_INFO);
}
#if 0
/*****************************************************************
 * 函数名                   tha_callback_request_update_system_info
 * 功能                       处理更新系统信息请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_update_system_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_update_system_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_updated_system_info.head.status = status;
        g_tha_sreq_updated_system_info.head.sequence = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_SYSTEM_INFO);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Updated_SystemInfo_t)){
        LOG_E("thaisen input length error when call tha_callback_request_update_system_info|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Updated_SystemInfo_t));
        g_tha_sreq_updated_system_info.head.status = status;
        g_tha_sreq_updated_system_info.head.sequence = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_SYSTEM_INFO);
        return;
    }

    memcpy(&g_tha_sreq_updated_system_info, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_SYSTEM_INFO);
}
#endif
/*****************************************************************
 * 函数名                   tha_callback_request_query_billing_rule
 * 功能                       处理查询计费规则请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_billing_rule(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_billing_rule");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_BILLING_RULE);
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_billing_rule|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_BILLING_RULE);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_BILLING_RULE);
}
/*****************************************************************
 * 函数名                   tha_callback_request_update_billing_rule
 * 功能                       处理更新计费规则请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_update_billing_rule(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_update_billing_rule");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_updated_billing_rule[gunno].head.status = status;
        g_tha_sreq_updated_billing_rule[gunno].head.sequence = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_BILLING_RULE);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t)){
        LOG_E("thaisen input length error when call tha_callback_request_update_billing_rule|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t));
        g_tha_sreq_updated_billing_rule[gunno].head.status = status;
        g_tha_sreq_updated_billing_rule[gunno].head.sequence = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_BILLING_RULE);
        return;
    }

    memcpy(&g_tha_sreq_updated_billing_rule[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_BILLING_RULE);
}
/*****************************************************************
 * 函数名                   tha_callback_request_query_system_set_info
 * 功能                       处理查询系统设置信息请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_system_set_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_system_set_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_SYSTEM_SET_INFO);
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_system_set_info|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_SYSTEM_SET_INFO);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_SYSTEM_SET_INFO);
}
#if 0
/*****************************************************************
 * 函数名                   tha_callback_request_update_system_set_info
 * 功能                       处理更新系统设置信息请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_update_system_set_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_update_system_set_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_updated_charge_system_config.head.status = status;
        g_tha_sreq_updated_charge_system_config.head.sequence = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_SYSTEM_SET_INFO);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t)){
        LOG_E("thaisen input length error when call tha_callback_request_update_system_set_info|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t));
        g_tha_sreq_updated_charge_system_config.head.status = status;
        g_tha_sreq_updated_charge_system_config.head.sequence = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_SYSTEM_SET_INFO);
        return;
    }

    memcpy(&g_tha_sreq_updated_charge_system_config, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UPDATE_SYSTEM_SET_INFO);
}
#endif
/*****************************************************************
 * 函数名                   tha_callback_request_query_device_fault_info
 * 功能                       处理查询设备故障信息请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_device_fault_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_device_fault_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_DEVICE_FAULT_INFO);
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_device_fault_info|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_DEVICE_FAULT_INFO);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_DEVICE_FAULT_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_request_query_battery_charge_info
 * 功能                       处理查询电池充电信息请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_battery_charge_info(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_battery_charge_info");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_BATTERY_CHARGE_INFO);
        return;
    }
    if(length != sizeof(Net_ThaPro_GeneralResReq_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_battery_charge_info|%d, %d", length,
                sizeof(Net_ThaPro_GeneralResReq_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_BATTERY_CHARGE_INFO);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_BATTERY_CHARGE_INFO);
}
/*****************************************************************
 * 函数名                   tha_callback_request_query_order_state
 * 功能                       处理查询订单状态
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_query_order_state(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_query_order_state");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_query_order_state.head.status = status;
        g_tha_sreq_query_order_state.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_ORDER_STATE);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Query_OrderState_t)){
        LOG_E("thaisen input length error when call tha_callback_request_query_order_state|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Query_OrderState_t));
        g_tha_sreq_query_order_state.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_query_order_state.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_ORDER_STATE);
        return;
    }

    memcpy(&g_tha_sreq_query_order_state, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_QUERY_ORDER_STATE);
}
/*****************************************************************
 * 函数名                   tha_callback_request_set_reservation
 * 功能                       处理设置预约充电请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_set_reservation(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_set_reservation");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_set_reservation[gunno].head.status = status;
        g_tha_sreq_set_reservation[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_RESERVATION);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Set_Reservation_t)){
        LOG_E("thaisen input length error when call tha_callback_request_set_reservation|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Set_Reservation_t));
        g_tha_sreq_set_reservation[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_set_reservation[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_RESERVATION);
        return;
    }

    memcpy(&g_tha_sreq_set_reservation[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_RESERVATION);
}
/*****************************************************************
 * 函数名                   tha_callback_request_cancel_reservation
 * 功能                       处理取消预约充电请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_cancel_reservation(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_cancel_reservation");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_cancel_reservation[gunno].head.status = status;
        g_tha_sreq_cancel_reservation[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_CANCEL_RESERVATION);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Cancel_Reservation_t)){
        LOG_E("thaisen input length error when call tha_callback_request_cancel_reservation|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Cancel_Reservation_t));
        g_tha_sreq_cancel_reservation[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_cancel_reservation[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_CANCEL_RESERVATION);
        return;
    }

    memcpy(&g_tha_sreq_cancel_reservation[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_CANCEL_RESERVATION);
}
/*****************************************************************
 * 函数名                   tha_callback_request_stop_charge
 * 功能                       处理停止充电请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_stop_charge(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_stop_charge");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_stop_charge[gunno].head.status = status;
        g_tha_sreq_stop_charge[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_STOP_CHARGE);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_StopCharge_t)){
        LOG_E("thaisen input length error when call tha_callback_request_stop_charge|%d, %d", length,
                sizeof(Net_ThaPro_SReq_StopCharge_t));
        g_tha_sreq_stop_charge[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_stop_charge[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_STOP_CHARGE);
        return;
    }
    if(gunno != ((Net_ThaPro_SReq_StopCharge_t*)data)->body.gunno){
        g_tha_sreq_stop_charge[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
        g_tha_sreq_stop_charge[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_STOP_CHARGE);
        LOG_E("thaisen gunno error|%d, %d when call tha_callback_request_query_battery_charge_info", gunno, ((Net_ThaPro_GeneralResReq_t*)data)->body.gunno);
        return;
    }

    memcpy(&g_tha_sreq_stop_charge[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_STOP_CHARGE);
}
/*****************************************************************
 * 函数名                   tha_callback_request_start_update
 * 功能                       处理开始升级请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_start_update(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_start_update");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_start_update[gunno].head.status = status;
        g_tha_sreq_start_update[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_START_UPDATE);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_StartUpdate_t)){
        LOG_E("thaisen input length error when call tha_callback_request_start_update|%d, %d", length,
                sizeof(Net_ThaPro_SReq_StartUpdate_t));
        g_tha_sreq_start_update[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_start_update[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_START_UPDATE);
        return;
    }

    memcpy(&g_tha_sreq_start_update[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_START_UPDATE);
}
/*****************************************************************
 * 函数名                   tha_callback_request_set_pile_running_para
 * 功能                       处理设置桩运行参数请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_set_pile_running_para(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_set_pile_running_para");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_set_pile_running_para[gunno].head.status = status;
        g_tha_sreq_set_pile_running_para[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_RUNNING_PARA);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Set_Pile_RunningPara_t)){
        LOG_E("thaisen input length error when call tha_callback_request_set_pile_running_para|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Set_Pile_RunningPara_t));
        g_tha_sreq_set_pile_running_para[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_set_pile_running_para[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_RUNNING_PARA);
        return;
    }

    memcpy(&g_tha_sreq_set_pile_running_para[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_RUNNING_PARA);
}
/*****************************************************************
 * 函数名                   tha_callback_request_set_pile_charge_power
 * 功能                       处理设置桩充电功率请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_set_pile_charge_power(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_set_pile_charge_power");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_set_pile_charge_power[gunno].head.status = status;
        g_tha_sreq_set_pile_charge_power[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_CHARGE_POWER);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Set_Pile_ChargePower_t)){
        LOG_E("thaisen input length error when call tha_callback_request_set_pile_charge_power|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Set_Pile_ChargePower_t));
        g_tha_sreq_set_pile_charge_power[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_set_pile_charge_power[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_CHARGE_POWER);
        return;
    }

    rt_kprintf("8888888888888(%d)\n", gunno);
    memcpy(&g_tha_sreq_set_pile_charge_power[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_CHARGE_POWER);
}
/*****************************************************************
 * 函数名                   tha_callback_request_unlock_charge
 * 功能                       处理解锁充电请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_unlock_charge(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_unlock_charge");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_unlock_charge[gunno].head.status = status;
        g_tha_sreq_unlock_charge[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UNLOCK_CHARGE);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_UnlockCharge_t)){
        LOG_E("thaisen input length error when call tha_callback_request_unlock_charge|%d, %d", length,
                sizeof(Net_ThaPro_SReq_UnlockCharge_t));
        g_tha_sreq_unlock_charge[gunno].head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_unlock_charge[gunno].head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UNLOCK_CHARGE);
        return;
    }

    memcpy(&g_tha_sreq_unlock_charge[gunno], data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_UNLOCK_CHARGE);
}
/*****************************************************************
 * 函数名                   tha_callback_request_issued_process_para
 * 功能                       处理平台下发处理用参数请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_issued_process_para(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_issued_process_para");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_issued_process_para.head.status = status;
        g_tha_sreq_issued_process_para.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_ISSUED_PROCESS_PARA);
        return;
    }
    if(length != sizeof(Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t)){
        LOG_E("thaisen input length error when call tha_callback_request_issued_process_para|%d, %d", length,
                sizeof(Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t));
        g_tha_sreq_issued_process_para.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_issued_process_para.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_ISSUED_PROCESS_PARA);
        return;
    }

    memcpy(&g_tha_sreq_issued_process_para, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_ISSUED_PROCESS_PARA);
}
/*****************************************************************
 * 函数名                   tha_callback_request_set_private_reservation
 * 功能                       处理设置私桩定时充电预约请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_set_private_reservation(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_set_private_reservation");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_set_private_reservation.head.status = status;
        g_tha_sreq_set_private_reservation.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_PRIVATE_RESERVATION);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Set_PrivateReservation_t)){
        LOG_E("thaisen input length error when call tha_callback_request_set_private_reservation|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Set_PrivateReservation_t));
        g_tha_sreq_set_private_reservation.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_set_private_reservation.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_PRIVATE_RESERVATION);
        return;
    }

    memcpy(&g_tha_sreq_set_private_reservation, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_SET_PRIVATE_RESERVATION);
}
/*****************************************************************
 * 函数名                   tha_callback_request_cancel_private_reservation
 * 功能                       处理取消私桩定时充电预约请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_cancel_private_reservation(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_request_cancel_private_reservation");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_cancel_private_reservatione.head.status = status;
        g_tha_sreq_cancel_private_reservatione.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_CANCEL_PRIVATE_RESERVATION);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_Cancel_PrivateReservation_t)){
        LOG_E("thaisen input length error when call tha_callback_request_cancel_private_reservation|%d, %d", length,
                sizeof(Net_ThaPro_SReq_Cancel_PrivateReservation_t));
        g_tha_sreq_cancel_private_reservatione.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_cancel_private_reservatione.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_CANCEL_PRIVATE_RESERVATION);
        return;
    }

    memcpy(&g_tha_sreq_cancel_private_reservatione, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_CANCEL_PRIVATE_RESERVATION);
}
/*****************************************************************
 * 函数名                   tha_callback_response_query_available_update_pack_ver
 * 功能                       处理查询可用最新升级包版本响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_query_available_update_pack_ver(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_query_available_update_pack_ver");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sres_query_available_update_pack_ver.head.status = status;
        g_tha_sres_query_available_update_pack_ver.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_AVAILABLE_UPDATE_PACK_VER);
        return;
    }
    if(length != sizeof(Net_ThaPro_SRes_Query_Available_UpdatePackVer_t)){
        LOG_E("thaisen input length error when call tha_callback_response_query_available_update_pack_ver|%d, %d", length,
                sizeof(Net_ThaPro_SRes_Query_Available_UpdatePackVer_t));
        g_tha_sres_query_available_update_pack_ver.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sres_query_available_update_pack_ver.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_AVAILABLE_UPDATE_PACK_VER);
        return;
    }

    memcpy(&g_tha_sres_query_available_update_pack_ver, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_AVAILABLE_UPDATE_PACK_VER);
}
/*****************************************************************
 * 函数名                   tha_callback_response_device_start_update
 * 功能                       处理设备端请求升级响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_device_start_update(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_device_start_update");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_device_start_update|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        return;
    }

    tha_clear_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_DEVICE_REQUEST_UPDATE);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_DEVICE_REQUEST_UPDATE);
}
/*****************************************************************
 * 函数名                   tha_callback_response_query_private_reservation
 * 功能                       处理查询私桩预约信息响应
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_response_query_private_reservation(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call tha_callback_response_query_private_reservation");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_RES){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_PRIVATE_RESERVATION, status, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_PRIVATE_RESERVATION);
        return;
    }
    if(length != sizeof(Net_ThaPro_Head_t)){
        LOG_E("thaisen input length error when call tha_callback_response_query_private_reservation|%d, %d", length,
                sizeof(Net_ThaPro_Head_t));
        tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_PRIVATE_RESERVATION, NET_THA_STATUS_CODE_FORMAT_ERROR, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_PRIVATE_RESERVATION);
        return;
    }

    tha_input_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_PRIVATE_RESERVATION, NET_THA_STATUS_CODE_NORMAL, ((Net_ThaPro_Head_t*)data)->sequence, gunno);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SRES_EVENT_QUERY_PRIVATE_RESERVATION);
}
/*****************************************************************
 * 函数名                   tha_callback_request_issude_pwm
 * 功能                       处理下发pwm值请求
 * 参数                        link       连接
 *           data       数据
 *           length     数据长度
 * 返回                        无
 ****************************************************************/
static void tha_callback_request_issude_pwm(uint8_t gunno, uint8_t* data, uint16_t length)
{
    uint8_t status = NET_THA_STATUS_CODE_NORMAL;
    if(data == NULL){
        LOG_E("thaisen input data is null when call message_callback_request_issude_pwm");
        return;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        LOG_E("thaisen there is not this gunno|%d cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    if(((Net_ThaPro_Head_t*)data)->start_code != NET_THA_MESSAGE_START_CODE_REQ){
        LOG_E("thaisen gunno(%d) message start_code error cmd|%d", gunno, ((Net_ThaPro_Head_t*)data)->cmd);
        return ;
    }
    status = tha_message_status_check(gunno, data, length);
    if(status != NET_THA_STATUS_CODE_NORMAL){
        g_tha_sreq_issude_pwm.head.status = status;
        g_tha_sreq_issude_pwm.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_ISSUED_PWM);
        return;
    }
    if(length != sizeof(Net_ThaPro_SReq_IssudePwm_t)){
        LOG_E("thaisen input length error when call message_callback_request_issude_pwm|%d, %d", length,
                sizeof(Net_ThaPro_SReq_IssudePwm_t));
        g_tha_sreq_issude_pwm.head.status = NET_THA_STATUS_CODE_FORMAT_ERROR;
        g_tha_sreq_issude_pwm.head.sequence  = ((Net_ThaPro_Head_t*)data)->sequence;
        tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_ISSUED_PWM);
        return;
    }

    memcpy(&g_tha_sreq_issude_pwm, data, length);
    tha_net_event_send(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_SREQ_EVENT_ISSUED_PWM);
}


int32_t tha_message_recv_init(void)
{
    tha_service_callback_register(NET_THA_MESSAGE_CMD_LOGIN,                      tha_callback_response_login);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_AUTHORIZE,                  tha_callback_response_authority);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_CHARGE_DATA,                tha_callback_response_report_charge_data);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_HISTORY_ORDER,              tha_callback_response_history_order);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_HEARTBEAT,                  tha_callback_response_heartbeat);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_UPDATE_PACK_LOAD,           tha_callback_response_update_pack_load);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_REPORT_BATTERY_STATE,       tha_callback_response_report_battery_state);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_STOP_CHARGE_COMPLETE,       tha_callback_response_stop_charge_complete);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_EVENT_STATE,                tha_callback_response_event_state);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_ERROR_INFO,                 tha_callback_response_error_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_UPDATEPACK_LOADRESULT,      tha_callback_response_updatepack_loadresult);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_PILE_NUMBER_OF_ICCID, tha_callback_response_query_pile_number_of_iccid);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_VIN_AUTHORIZE,              tha_callback_response_vin_authorize);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_NET_INFO,                   tha_callback_response_net_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_REPORT_BATTERY_INFO,        tha_callback_response_report_battery_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_DEVICE_FAULT_INFO,          tha_callback_response_device_fault_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_CAR_WARNNING_INFO,          tha_callback_response_report_car_warnning_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_GUN_REALTEMP,               tha_callback_response_report_real_temperation);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_TIME_SYNC,                  tha_callback_request_time_sync);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_SYSTEM_REBOOT,              tha_callback_request_system_reboot);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA,          tha_callback_request_query_charge_data);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO,          tha_callback_request_query_system_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE,         tha_callback_request_query_billing_rule);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_UPDATED_BILLING_RULE,       tha_callback_request_update_billing_rule);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET,    tha_callback_request_query_system_set_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO,    tha_callback_request_query_device_fault_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO,  tha_callback_request_query_battery_charge_info);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_ORDER_STATE,          tha_callback_request_query_order_state);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_SET_RESERVATION,            tha_callback_request_set_reservation);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_CANCEL_RESERVATION,         tha_callback_request_cancel_reservation);

    tha_service_callback_register(NET_THA_MESSAGE_CMD_STOP_CHARGE,                tha_callback_request_stop_charge);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_SERVER_START_UPDATE,        tha_callback_request_start_update);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_SET_PILE_RUNNING_PARA,      tha_callback_request_set_pile_running_para);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_SET_PILE_CHARGE_POWER,      tha_callback_request_set_pile_charge_power);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_UNLOCK_CHARGE,              tha_callback_request_unlock_charge);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_SET_PRIVATE_RESERVATION,    tha_callback_request_set_private_reservation);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_CANCEL_PRIVATE_RESERVATION, tha_callback_request_cancel_private_reservation);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_AVAILABLE_PACK_VER,   tha_callback_response_query_available_update_pack_ver);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_DEVICE_START_UPDATE,        tha_callback_response_device_start_update);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_QUERY_PRIVATE_RESERVATION,  tha_callback_response_query_private_reservation);
    tha_service_callback_register(NET_THA_MESSAGE_CMD_ISSUDE_PWM,                 tha_callback_request_issude_pwm);

    return 0x00;
}

#endif /* NET_PACK_USING_THA */

