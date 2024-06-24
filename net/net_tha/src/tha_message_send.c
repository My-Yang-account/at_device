/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#include "tha_message_send.h"
#include "tha_message_receive.h"
#include "tha_transceiver.h"
#include "tha_message_padding.h"

#include "net_operation.h"

#define DBG_TAG "tha_send"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_THA

#define NET_THA_SAME_TRANSATION_REPORT_COUNT_MAX             10          /* 相同订单最大上报次数 */

struct tha_wait_response{
    uint32_t message_wait_response_state[NET_SYSTEM_GUN_NUMBER];                       /* 报文已发送发送，等待响应状态 */
    uint32_t message_repeat_time[NET_SYSTEM_GUN_NUMBER][NET_THA_CHARGEPILE_PREQ_NUM];  /* 报文重发计时 */
};

static tha_transaction_info_t s_tha_transaction_info[NET_SYSTEM_GUN_NUMBER][NET_THA_TRANSACTION_REPORT_COUNT_MAX];
static uint8_t s_tha_current_transaction_number[NET_SYSTEM_GUN_NUMBER];
static uint8_t s_tha_transaction_info_num[NET_SYSTEM_GUN_NUMBER];
static uint8_t s_tha_same_transaction_report_count[NET_SYSTEM_GUN_NUMBER];
static uint8_t s_tha_transaction_verify[NET_SYSTEM_GUN_NUMBER];
static struct tha_wait_response s_tha_wait_response;
static tha_socket_info_t s_tha_socket_info;
static uint8_t s_tha_message_serial_number[NET_SYSTEM_GUN_NUMBER];                      /* 报文序列号 */
static uint32_t s_tha_message_send_state[NET_SYSTEM_GUN_NUMBER];                        /* 报文发送状态 */
static uint32_t s_tha_chargepile_event[NET_THA_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];
static uint32_t s_tha_server_event[NET_THA_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];

static struct rt_thread s_tha_message_send_thread;
static uint8_t s_tha_message_send_thread_stack[NET_THA_MESSAGE_SEND_THREAD_STACK_SIZE];
static struct rt_thread s_tha_server_message_pro_thread;
static uint8_t s_tha_server_message_pro_thread_stack[NET_THA_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE];
static tha_response_message_buf_t s_tha_response_buff;
static struct rt_mutex s_tha_response_buff_mutex;

/** 登录签到 */
Net_ThaPro_PReq_LogIn_t g_tha_preq_login[NET_SYSTEM_GUN_NUMBER];
/** 权限认证 */
Net_ThaPro_PReq_Authority_t g_tha_preq_authority[NET_SYSTEM_GUN_NUMBER];   // OK
/** 上报充电数据 */
Net_ThaPro_PReq_Report_ChargeData_t g_tha_preq_charge_data[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报历史订单 */
Net_ThaPro_PReq_Report_HistoryOrder_t g_tha_preq_history_order[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报心跳 */
Net_ThaPro_PReq_Report_HeartBeat_t g_tha_preq_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 请求下载升级包 */
Net_ThaPro_PReq_UpdatePack_Load_t g_tha_preq_update_pack_load;
/** 上报电池充电状态 */
Net_ThaPro_PReq_Battery_ChargeState_t g_tha_preq_report_battery_charge_state[NET_SYSTEM_GUN_NUMBER];  // OK
/** 停止充电动作完成，上报信息 */
Net_ThaPro_PReq_StopCharge_Complete_t g_tha_preq_stop_charge_complete[NET_SYSTEM_GUN_NUMBER];  // OK
/** 事件状态上报 */
Net_ThaPro_PReq_Report_EventState_t g_tha_preq_event_state[NET_SYSTEM_GUN_NUMBER];  // OK
/** 错误信息上报 */
Net_ThaPro_PReq_Report_ErrorInfo_t g_tha_preq_error_info[NET_SYSTEM_GUN_NUMBER];   // OK
/** 查询ICCID对应的桩编号 */
Net_ThaPro_SRes_Query_PileNumberOfIccid_t g_tha_preq_query_pile_number_of_iccid;
/** 设备上报处理用参数 */
Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t g_tha_preq_report_process_para;
/** 设备上报桩编号和ICCID的绑定关系（工厂模式） */
Net_ThaPro_PReq_Report_BindTie_IccidAndPileNumber_t g_tha_preq_report_bindtie_of_iccid_and_number;
/** VIN码鉴权 */
Net_ThaPro_PReq_VinAuthorize_t g_tha_preq_vin_authorize[NET_SYSTEM_GUN_NUMBER];   // OK
/** 升级包下载结果 */
Net_ThaPro_PReq_UpdatePack_LoadResult_t g_pile_request_updatepack_loadresult;     // OK
/** 设备网络信息上传 */
Net_ThaPro_PReq_Report_NetInfo_t g_pile_request_net_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报电池信息 */
Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_t g_tha_preq_report_battery_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报设备故障信息 */
Net_ThaPro_PReq_Report_PRes_Query_Device_FaultInfo_t g_tha_preq_report_device_fault_info;  // OK
/** 上报车辆告警信息 */
Net_ThaPro_PReq_Report_Car_WarnningInfo_t  g_tha_preq_car_warnning_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报充电中充电枪的实时温度 */
Net_ThaPro_PReq_Report_Gun_RealTemp_t  g_tha_preq_report_gun_realtemp[NET_SYSTEM_GUN_NUMBER];  // OK
/** 查询订单状态响应 */
Net_ThaPro_PRes_Query_OrderState_t g_pres_query_order_state;
/** 设备查询最新可用升级包软件版本号 */
Net_ThaPro_PReq_Query_Available_UpdatePackVer_t g_pres_query_available_update_pack_ver;  // OK
/** 设备发起远程升级请求 */
Net_ThaPro_PReq_Start_Update_t g_preq_start_update;  // OK

/** 上报电池信息（充电电流大于655A时用）、查询电池信息（充电电流大于655A时用） */
Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_Expand_t g_tha_preq_pres_battery_info_expand[NET_SYSTEM_GUN_NUMBER];
/** 上报充电数据（充电电流大于655A时用） */
Net_ThaPro_PReq_ChargeData_Expand_t g_tha_preq_charge_data_expand[NET_SYSTEM_GUN_NUMBER];
/** 上报电池充电状态（充电电流大于655A时用） */
Net_ThaPro_PReq_BatteryState_Expand_t g_tha_preq_battery_state_expand[NET_SYSTEM_GUN_NUMBER];

/**************************************************************************
 * 函数名                 tha_get_socket_info
 * 功能                     获取socket信息
 * 说明
 * ***********************************************************************/
tha_socket_info_t* tha_get_socket_info(void)
{
    return &s_tha_socket_info;
}

/**************************************************************************
 * 函数名                 tha_response_buff_take_mutex_forever
 * 功能                     获取响应缓存互斥量
 * 说明
 * ***********************************************************************/
static int32_t tha_response_buff_take_mutex_forever(int32_t timeout)
{
    return rt_mutex_take(&s_tha_response_buff_mutex, timeout);
}
/**************************************************************************
 * 函数名                 tha_response_buff_release_mutex
 * 功能                     释放响应缓存互斥信号量
 * 说明
 * ***********************************************************************/
void tha_response_buff_release_mutex(void)
{
    rt_mutex_release(&s_tha_response_buff_mutex);
}
/**************************************************************************
 * 函数名                 tha_get_response_buff
 * 功能                     获取响应缓存
 * 说明
 * ***********************************************************************/
tha_response_message_buf_t* tha_get_response_buff(int32_t timeout)
{
    if(tha_response_buff_take_mutex_forever(timeout) < 0){
        return NULL;
    }
    return &s_tha_response_buff;
}

/**************************************************************************
 * 函数名                 tha_transaction_is_verify
 * 功能                     查询交易是否已确认
 * 说明
 * ***********************************************************************/
uint8_t tha_transaction_is_verify(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_tha_transaction_verify[gunno];
}

/**************************************************************************
 * 函数名                 tha_set_transaction_verify_state
 * 功能                     设置交易确认状态
 * 说明
 * ***********************************************************************/
void tha_set_transaction_verify_state(uint8_t gunno, uint8_t state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_tha_transaction_verify[gunno] = 0x01;
    }else{
        s_tha_transaction_verify[gunno] = 0x00;
    }
}

/**************************************************************************
 * 函数名                 tha_get_transaction_info
 * 功能                     获取交易信息
 * 说明
 * ***********************************************************************/
tha_transaction_info_t* tha_get_transaction_info(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return &s_tha_transaction_info[gunno];
}

/**************************************************************************
 * 函数名                 tha_net_event_send
 * 功能                     网络事件发送
 * 说明
 * ***********************************************************************/
int32_t tha_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event)
{
    if(event_handle >= NET_THA_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_THA_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }

    if(event_handle == NET_THA_EVENT_HANDLE_CHARGEPILE){
        s_tha_chargepile_event[event_type][gunno] |= (1 <<event);
    }else{
        s_tha_server_event[event_type][gunno] |= (1 <<event);
    }
    return 0;
}
/**************************************************************************
 * 函数名                 tha_net_event_receive
 * 功能                     网络事件接收
 * 说明                     若事件发生则对应位为 1，否则为0
 * ***********************************************************************/
int32_t tha_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf)
{
    if(event_handle >= NET_THA_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_THA_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }
    uint32_t (*event_set)[NET_SYSTEM_GUN_NUMBER] = NULL;

    if(event_handle == NET_THA_EVENT_HANDLE_CHARGEPILE){
        event_set = s_tha_chargepile_event;
    }else{
        event_set = s_tha_server_event;
    }

    if(event_buf){
        (*event_buf) = event_set[event_type][gunno];
    }
    if(event_set[event_type][gunno] &(1 <<event)){
        if(option &NET_THA_EVENT_OPTION_AND){
            if((event_set[event_type][gunno] &(1 <<event)) != (1 <<event)){
                return -4;
            }
        }
        if(option &NET_THA_EVENT_OPTION_CLEAR){
            event_set[event_type][gunno] &= (~(1 <<event));
        }
        return 1;
    }
    return 0;
}

/**************************************************************************
 * 函数名                 tha_get_message_send_state
 * 功能                     获取报文发送状态
 * 说明                     若报文正在发送则对应位为 1，否则为0
 * ***********************************************************************/
uint8_t tha_get_message_send_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_tha_message_send_state[gunno] &(1 <<message_bit)){
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 tha_set_message_send_state
 * 功能                     设置报文发送状态
 * 说明                     若报文正在发送则对应位置 1，否则置0
 * ***********************************************************************/
void tha_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_tha_message_send_state[gunno] |= (1 <<message_bit);
    }else{
        s_tha_message_send_state[gunno] &= (~(1 <<message_bit));
    }
}

/**************************************************************************
 * 函数名                 tha_set_message_wait_response_state
 * 功能                     设置报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
static void tha_set_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_THA_CHARGEPILE_PREQ_NUM){
        return;
    }
    s_tha_wait_response.message_wait_response_state[gunno] |= (1 <<message_bit);
    s_tha_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
}
/**************************************************************************
 * 函数名                 tha_clear_message_wait_response_state
 * 功能                     清除报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
void tha_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_THA_CHARGEPILE_PREQ_NUM){
        return;
    }
    s_tha_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
}
/**************************************************************************
 * 函数名                 tha_exist_message_wait_response
 * 功能                     检查是否存在已发送的报文等待响应
 * 说明
 * ***********************************************************************/
uint8_t tha_exist_message_wait_response(uint8_t gunno, uint32_t *state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_tha_wait_response.message_wait_response_state[gunno]){
        if(state){
            *state = s_tha_wait_response.message_wait_response_state[gunno];
        }
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 tha_get_message_wait_response_timeout_state
 * 功能                     获取报文等待响应超时状态
 * 说明
 * ***********************************************************************/
uint8_t tha_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(message_bit >= NET_THA_CHARGEPILE_PREQ_NUM){
        return 0x00;
    }
    if(s_tha_wait_response.message_wait_response_state[gunno] &(1 <<message_bit)){
        if(s_tha_wait_response.message_repeat_time[gunno][message_bit] > rt_tick_get()){
            s_tha_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
        }
        if((rt_tick_get() - s_tha_wait_response.message_repeat_time[gunno][message_bit]) > timeout){
            s_tha_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
            return 0x01;
        }
    }
    return 0x00;
}

static void net_tha_message_send_thread_entry(void *parameter)
{
    uint8_t step[NET_SYSTEM_GUN_NUMBER];
    uint16_t delay[NET_SYSTEM_GUN_NUMBER];
    uint32_t heartbeat_tick[NET_SYSTEM_GUN_NUMBER];

    while(1)
    {
        if(net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT){
            rt_thread_mdelay(5000);
            continue;
        }

        net_get_net_handle()->data_updata();
        if((net_get_net_handle()->net_fault) &NET_FAULT_PHYSICAL_LAYER){
            for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                s_tha_socket_info.state[gunno] = THA_SOCKET_STATE_PHY;
                s_tha_socket_info.fd[gunno] = -0x01;
                step[gunno] = NET_THA_NET_STATE_OPEN_SOCKET;
                delay[gunno] = 0;
            }
            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_DATA_LINK_LAYER){
            for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                s_tha_socket_info.state[gunno] = THA_SOCKET_STATE_DATA_LINK;
                s_tha_socket_info.fd[gunno] = -0x01;
                step[gunno] = NET_THA_NET_STATE_OPEN_SOCKET;
                delay[gunno] = 0;
            }
            rt_thread_mdelay(1000);
            continue;
        }

        /***************************************************** [登录认证] **********************************************************/
        /***************************************************** [登录认证] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_tha_socket_info.state[gunno] != THA_SOCKET_STATE_LOGIN_SUCCESS){
                switch(step[gunno]){
                case NET_THA_NET_STATE_OPEN_SOCKET:
                    s_tha_socket_info.state[gunno] = THA_SOCKET_STATE_OPEN;
                    if((delay[gunno] %(NET_THA_LOGIN_OPERATION_INTERVAL /100)) == 0){
                        int32_t result = 0x00;
                        tha_socket_close(s_tha_socket_info.fd[gunno]);
#ifndef NET_THA_AS_MONITOR
                        uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_THA |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                        struct net_handle* handle = net_get_net_handle();
                        char *host = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, NULL, option));
                        uint16_t port = *((uint16_t*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PORT, NULL, option)));

                        result = tha_socket_open(&(s_tha_socket_info.fd[gunno]), host, strlen(host), port);
                        if(result >= 0){
                            LOG_D("thaisen(%d) socket open success with host[%s] port[%d]", gunno, host, port);
                            s_tha_socket_info.operate_fail[gunno].open_socket = 0;
                            step[gunno] = NET_THA_NET_STATE_LOGIN;
                        }else{
                            LOG_W("thaisen(%d) fail to open socket with host[%s] port[%d] num|%d", gunno, host, port, s_tha_socket_info.operate_fail[gunno].open_socket);
                            delay[gunno]++;
                            s_tha_socket_info.operate_fail[gunno].open_socket++;
                        }
#else
                        result = tha_socket_open(&(s_tha_socket_info.fd[gunno]), "139.198.163.108", strlen("139.198.163.108"), 8003);
                        if(result >= 0){
                            LOG_D("thaisen(%d) socket open success with host[%s] port[%d]", gunno, "139.198.163.108", 8003);
                            s_tha_socket_info.operate_fail[gunno].open_socket = 0;
                            step[gunno] = NET_THA_NET_STATE_LOGIN;
                        }else{
                            LOG_W("thaisen(%d) fail to open socket with host[%s] port[%d] num|%d", gunno, "139.198.163.108", 8003, s_tha_socket_info.operate_fail[gunno].open_socket);
                            delay[gunno]++;
                            s_tha_socket_info.operate_fail[gunno].open_socket++;
                        }
#endif /*  */
                        s_tha_message_serial_number[gunno] = 0x00;
                    }else{
                        delay[gunno]++;
                    }
                    break;
                case NET_THA_NET_STATE_LOGIN:
                {
                    uint8_t rentry = 0;
                    s_tha_socket_info.state[gunno] = THA_SOCKET_STATE_LOGIN_WAIT;
                    tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, 0x00,
                            (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_LOGIN, NULL);
                    tha_message_send_port(NET_THA_MESSAGE_CMD_LOGIN, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_login[gunno],
                            sizeof(g_tha_preq_login[gunno]));
                    while(rentry < 100){
                        if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                                (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_LOGIN, NULL) > 0){
                            if(g_tha_sres_login[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                                LOG_D("thaisen(%d) login success", gunno);
                                struct net_handle* handle = net_get_net_handle();
                                s_tha_socket_info.operate_fail[gunno].login = 0;
                                step[gunno] = NET_THA_NET_STATE_MONITORING;
                                handle->time_sync(g_tha_sres_login[gunno].body.timestamp);
                                s_tha_message_serial_number[gunno]++;

                                rt_thread_mdelay(250);
                                tha_chargepile_request_padding_net_info(gunno);
                            }else{
                                delay[gunno] = 0;
                                step[gunno] = NET_THA_NET_STATE_OPEN_SOCKET;
                                s_tha_socket_info.fd[gunno] = -0x01;
                                s_tha_socket_info.operate_fail[gunno].login++;
                                LOG_D("thaisen(%d) login fail(status:%d) num|%d", gunno, g_tha_sres_login[gunno].head.status, s_tha_socket_info.operate_fail[gunno].login);
                            }
                            break;
                        }
                        rt_thread_mdelay(30);
                        rentry++;
                    }
                    if(rentry >= 100){
                        delay[gunno] = 0;
                        step[gunno] = NET_THA_NET_STATE_OPEN_SOCKET;
                        s_tha_socket_info.fd[gunno] = -0x01;
                        s_tha_socket_info.operate_fail[gunno].login++;
                        LOG_D("thaisen(%d) login fail(timeout) num|%d", gunno, s_tha_socket_info.operate_fail[gunno].login);
                    }
                    break;
                }
                case NET_THA_NET_STATE_MONITORING:
                    s_tha_socket_info.state[gunno] = THA_SOCKET_STATE_LOGIN_SUCCESS;
                    break;
                default:
                    step[gunno] = NET_THA_NET_STATE_OPEN_SOCKET;
                    delay[gunno] = 0;
                    s_tha_socket_info.fd[gunno] = -0x01;
                    s_tha_socket_info.state[gunno] = THA_SOCKET_STATE_OPEN;
                    break;
                }
                if(s_tha_socket_info.operate_fail[gunno].open_socket > 0x05){

                    s_tha_socket_info.operate_fail[gunno].open_socket = 0x00;
                }
                if(s_tha_socket_info.operate_fail[gunno].login > 0x05){

                    s_tha_socket_info.operate_fail[gunno].login = 0x00;
                }
            }
        }
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_tha_socket_info.state[gunno] != THA_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
                heartbeat_tick[gunno] = rt_tick_get();
                tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_HEARTBEAT);
                tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_HEARTBEAT);
            }
        }

        /***************************************************** [数据请求] **********************************************************/
        /***************************************************** [数据请求] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_tha_socket_info.state[gunno] == THA_SOCKET_STATE_LOGIN_SUCCESS){
                uint32_t _event = 0;
                tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
                if(!_event){
                    continue;   /* 此枪没有请求事件,不进行事件查询 */
                }
                /***** [权限认证] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_AUTHORIZE, NULL) > 0){
#ifndef NET_THA_AS_MONITOR
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_AUTHORIZE);
                    g_tha_preq_authority[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_authority[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_authority[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_AUTHORIZE, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_authority[gunno],
                            sizeof(g_tha_preq_authority[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_AUTHORIZE);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_AUTHORIZE);
                    rt_thread_mdelay(250);
#endif /* NET_THA_AS_MONITOR */
                }
                /***** [上报充电数据] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA);
                    g_tha_preq_charge_data[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_charge_data[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_charge_data[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_CHARGE_DATA, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_charge_data[gunno],
                            (g_tha_preq_charge_data[gunno].head.length + sizeof(Net_ThaPro_Head_t)));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA);
                    rt_thread_mdelay(250);
                }
                /***** [上报历史订单] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER);
                    g_tha_preq_history_order[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_history_order[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_history_order[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_HISTORY_ORDER, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_history_order[gunno],
                            sizeof(g_tha_preq_history_order[gunno]));

                    s_tha_transaction_verify[gunno] = 0x00;
                    if(s_tha_current_transaction_number[gunno] != g_tha_preq_history_order[gunno].body.trade_no){
                        s_tha_same_transaction_report_count[gunno] = 0x00;
                        s_tha_current_transaction_number[gunno] = g_tha_preq_history_order[gunno].body.trade_no;
                    }else{
                        if(++s_tha_same_transaction_report_count[gunno] > NET_THA_SAME_TRANSATION_REPORT_COUNT_MAX){
                            s_tha_transaction_verify[gunno] = 0x01;
                            s_tha_same_transaction_report_count[gunno] = 0x00;
                        }
                    }

                    s_tha_transaction_info[gunno][s_tha_transaction_info_num[gunno]].gunno = gunno;
                    s_tha_transaction_info[gunno][s_tha_transaction_info_num[gunno]].sn = g_tha_preq_history_order[gunno].head.sequence;
                    s_tha_transaction_info_num[gunno]++;
                    if(s_tha_transaction_info_num[gunno] >= (NET_THA_TRANSACTION_REPORT_COUNT_MAX - 0x01)){
                        s_tha_transaction_info_num[gunno] = 0x00;
                    }

                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER);
                    rt_thread_mdelay(250);
                }
                /***** [上报心跳] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_HEARTBEAT, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_HEARTBEAT);
                    s_tha_socket_info.operate_fail[gunno].heartbeat++;
                    g_tha_preq_heartbeat[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_heartbeat[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_heartbeat[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_HEARTBEAT, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_heartbeat[gunno],
                            sizeof(g_tha_preq_heartbeat[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_HEARTBEAT);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_HEARTBEAT);
                    rt_thread_mdelay(250);
                }
                /***** [上报电池状态] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE);
                    g_tha_preq_report_battery_charge_state[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_report_battery_charge_state[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_report_battery_charge_state[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_REPORT_BATTERY_STATE, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_report_battery_charge_state[gunno],
                            sizeof(g_tha_preq_report_battery_charge_state[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE);
                    rt_thread_mdelay(250);
                }
                /***** [停止充电动作完成，上报信息] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE);
                    g_tha_preq_stop_charge_complete[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_stop_charge_complete[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_stop_charge_complete[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_STOP_CHARGE_COMPLETE, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_stop_charge_complete[gunno],
                            sizeof(g_tha_preq_stop_charge_complete[gunno]));

                    s_tha_transaction_verify[gunno] = 0x00;
                    if(s_tha_current_transaction_number[gunno] != g_tha_preq_history_order[gunno].body.trade_no){
                        s_tha_same_transaction_report_count[gunno] = 0x00;
                        s_tha_current_transaction_number[gunno] = g_tha_preq_history_order[gunno].body.trade_no;
                    }else{
                        if(++s_tha_same_transaction_report_count[gunno] > NET_THA_SAME_TRANSATION_REPORT_COUNT_MAX){
                            s_tha_transaction_verify[gunno] = 0x01;
                            s_tha_same_transaction_report_count[gunno] = 0x00;
                        }
                    }

                    s_tha_transaction_info[gunno][s_tha_transaction_info_num[gunno]].gunno = gunno;
                    s_tha_transaction_info[gunno][s_tha_transaction_info_num[gunno]].sn = g_tha_preq_history_order[gunno].head.sequence;
                    s_tha_transaction_info_num[gunno]++;
                    if(s_tha_transaction_info_num[gunno] >= (NET_THA_TRANSACTION_REPORT_COUNT_MAX - 0x01)){
                        s_tha_transaction_info_num[gunno] = 0x00;
                    }

                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE);
                    rt_thread_mdelay(250);
                }
                /***** [上报事件状态] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_EVENT_STATE, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_EVENT_STATE);
                    g_tha_preq_event_state[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_event_state[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_event_state[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_EVENT_STATE, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_event_state[gunno],
                            sizeof(g_tha_preq_event_state[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_EVENT_STATE);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_EVENT_STATE);
                    rt_thread_mdelay(250);
                }
                /***** [上报错误信息] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_ERROR_INFO, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_ERROR_INFO);
                    g_tha_preq_error_info[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_error_info[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_error_info[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_ERROR_INFO, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_error_info[gunno],
                            sizeof(g_tha_preq_error_info[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_ERROR_INFO);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_ERROR_INFO);
                    rt_thread_mdelay(250);
                }
                /***** [vin 码鉴权] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_VIN_AUTHORIZE, NULL) > 0){
    #ifndef NET_THA_AS_MONITOR
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_VIN_AUTHORIZE);
                    g_tha_preq_vin_authorize[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_vin_authorize[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_vin_authorize[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_VIN_AUTHORIZE, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_vin_authorize[gunno],
                            sizeof(g_tha_preq_vin_authorize[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_VIN_AUTHORIZE);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_VIN_AUTHORIZE);
                    rt_thread_mdelay(250);
    #endif /* NET_THA_AS_MONITOR */
                }
                /***** [升级包下载结果] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_UPDATE_RESULT, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_UPDATE_RESULT);
                    g_pile_request_updatepack_loadresult.head.sequence = s_tha_message_serial_number[gunno]++;
                    g_pile_request_updatepack_loadresult.head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_pile_request_updatepack_loadresult.head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_UPDATEPACK_LOADRESULT, gunno, s_tha_socket_info.fd[gunno], &g_pile_request_updatepack_loadresult,
                            sizeof(g_pile_request_updatepack_loadresult));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_UPDATE_RESULT);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_UPDATE_RESULT);
                    rt_thread_mdelay(250);
                }
                /***** [上传设备网络信息] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_DEVICE_NET_INFO, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_DEVICE_NET_INFO);
                    g_pile_request_net_info[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_pile_request_net_info[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_pile_request_net_info[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_NET_INFO, gunno, s_tha_socket_info.fd[gunno], &g_pile_request_net_info[gunno],
                            sizeof(g_pile_request_net_info[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_DEVICE_NET_INFO);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_DEVICE_NET_INFO);
                    rt_thread_mdelay(250);
                }
                /***** [上报电池信息] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO);
                    g_tha_preq_report_battery_info[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_report_battery_info[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_report_battery_info[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_REPORT_BATTERY_INFO, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_report_battery_info[gunno],
                            sizeof(g_tha_preq_report_battery_info[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO);
                    rt_thread_mdelay(250);
                }
                /***** [上报设备故障信息] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_DEVICE_FAULT_INFO, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_DEVICE_FAULT_INFO);
                    g_tha_preq_report_device_fault_info.head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_report_device_fault_info.head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_report_device_fault_info.head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_DEVICE_FAULT_INFO, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_report_device_fault_info,
                            sizeof(g_tha_preq_report_device_fault_info));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_DEVICE_FAULT_INFO);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_DEVICE_FAULT_INFO);
                    rt_thread_mdelay(250);
                }
                /***** [上报车辆告警信息] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_CAR_WARNNING_INFO, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_CAR_WARNNING_INFO);
                    g_tha_preq_car_warnning_info[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_car_warnning_info[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_car_warnning_info[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_CAR_WARNNING_INFO, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_car_warnning_info[gunno],
                            sizeof(g_tha_preq_car_warnning_info[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_CAR_WARNNING_INFO);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_CAR_WARNNING_INFO);
                    rt_thread_mdelay(250);
                }
                /***** [上报充电中充电枪的实时温度] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_GUN_REALTEMP, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_GUN_REALTEMP);
                    g_tha_preq_report_gun_realtemp[gunno].head.sequence = s_tha_message_serial_number[gunno]++;
                    g_tha_preq_report_gun_realtemp[gunno].head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_tha_preq_report_gun_realtemp[gunno].head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_GUN_REALTEMP, gunno, s_tha_socket_info.fd[gunno], &g_tha_preq_report_gun_realtemp[gunno],
                            sizeof(g_tha_preq_report_gun_realtemp[gunno]));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_GUN_REALTEMP);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_GUN_REALTEMP);
                    rt_thread_mdelay(250);
                }
                /***** [设备查询最新可用升级包软件版本号] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_QUERY_AVAILABLE_PACK_VER, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_QUERY_AVAILABLE_PACK_VER);
                    g_pres_query_available_update_pack_ver.head.sequence = s_tha_message_serial_number[gunno]++;
                    g_pres_query_available_update_pack_ver.head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_pres_query_available_update_pack_ver.head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_AVAILABLE_PACK_VER, gunno, s_tha_socket_info.fd[gunno], &g_pres_query_available_update_pack_ver,
                            sizeof(g_pres_query_available_update_pack_ver));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_QUERY_AVAILABLE_PACK_VER);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_QUERY_AVAILABLE_PACK_VER);
                    rt_thread_mdelay(250);
                }
                /***** [设备发起远程升级请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PREQ_EVENT_DEVICE_REQUEST_UPDATE, NULL) > 0){

                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_DEVICE_REQUEST_UPDATE);
                    g_preq_start_update.head.sequence = s_tha_message_serial_number[gunno]++;
                    g_preq_start_update.head.check_sum = g_tha_preq_login[gunno].body.random_sign;
                    g_preq_start_update.head.status = NET_THA_STATUS_CODE_NORMAL;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_DEVICE_START_UPDATE, gunno, s_tha_socket_info.fd[gunno], &g_preq_start_update,
                            sizeof(g_preq_start_update));
                    tha_set_message_wait_response_state(gunno, NET_THA_PREQ_EVENT_DEVICE_REQUEST_UPDATE);
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_COMPLETE, NET_THA_PREQ_EVENT_DEVICE_REQUEST_UPDATE);
                    rt_thread_mdelay(250);
                }
            }
        }
        /***************************************************** [数据响应] **********************************************************/
        /***************************************************** [数据响应] **********************************************************/

        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_tha_socket_info.state[gunno] == THA_SOCKET_STATE_LOGIN_SUCCESS){
                uint32_t _event = 0;
                tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
                if(!_event){
                    continue;   /* 此枪没有响应事件,不进行事件查询 */
                }
                /***** [时间同步响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_TIME_SYNC, NULL) > 0){

                    Net_ThaPro_Head_t *time_sync = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    time_sync->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    time_sync->sequence = g_tha_sreq_time_sync.head.sequence;
                    time_sync->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    time_sync->status = g_tha_sreq_time_sync.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_TIME_SYNC, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [系统复位响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_REBOOT, NULL) > 0){

                    Net_ThaPro_Head_t *reboot = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    reboot->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    reboot->sequence = g_tha_sreq_system_reboot.head.sequence;
                    reboot->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    reboot->status = g_tha_sreq_system_reboot.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_SYSTEM_REBOOT, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [查询充电数据响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_READ_CHARGE_DATA, NULL) > 0){

                    uint8_t value = 0x00;
                    int16_t result = 0x00;
                    Net_ThaPro_Head_t *charge_data = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    charge_data->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    result = tha_get_recv_message_item_serial_number(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    charge_data->sequence = value;
                    charge_data->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    result = tha_get_recv_message_item_status(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    if(value != NET_THA_STATUS_CODE_NORMAL){
                        charge_data->status = value;
                    }

                    tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, gunno);
                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
    #if 0
                /***** [用户绑定] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_USER_BINDING, NULL) > 0){

                    Net_ThaPro_Head_t *binding = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    binding->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    binding->sequence = g_tha_sreq_user_binding.head.sequence;
                    binding->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    binding->status = g_tha_sreq_user_binding.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_TIME_SYNC, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
    #endif
                /***** [查询系统信息响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_QUERY_SYSTEM_INFO, NULL) > 0){

                    uint8_t value = 0x00;
                    int16_t result = 0x00;
                    Net_ThaPro_Head_t *system_info = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    system_info->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    result = tha_get_recv_message_item_serial_number(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    system_info->sequence = value;
                    system_info->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    result = tha_get_recv_message_item_status(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    system_info->status = value;

                    tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, gunno);
                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
    #if 0
                /***** [更新系统信息响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_UPDATE_SYSTEM_INFO, NULL) > 0){

                    Net_ThaPro_Head_t *system_info = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    system_info->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    system_info->sequence = g_tha_sreq_updated_system_info.head.sequence;
                    system_info->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    system_info->status = g_tha_sreq_updated_system_info.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_TIME_SYNC, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
    #endif
                /***** [查询计费规则响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_QUERY_BILLING_RULE, NULL) > 0){

                    uint8_t value = 0x00;
                    int16_t result = 0x00;
                    Net_ThaPro_Head_t *query_billing_rule = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    query_billing_rule->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    result = tha_get_recv_message_item_serial_number(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_billing_rule->sequence = value;
                    query_billing_rule->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    result = tha_get_recv_message_item_status(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_billing_rule->status = value;

                    tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, gunno);
                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [更新计费规则响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_UPDATE_BILLING_RULE, NULL) > 0){

                    Net_ThaPro_Head_t *update_billing_rule = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    update_billing_rule->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    update_billing_rule->sequence = g_tha_sreq_updated_billing_rule[gunno].head.sequence;
                    update_billing_rule->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    update_billing_rule->status = g_tha_sreq_updated_billing_rule[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_UPDATED_BILLING_RULE, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [查询充电系统设置响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_QUERY_CHARGE_SYSTEM_SET, NULL) > 0){

                    uint8_t value = 0x00;
                    int16_t result = 0x00;
                    Net_ThaPro_Head_t *query_charge_system = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    query_charge_system->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    result = tha_get_recv_message_item_serial_number(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_charge_system->sequence = value;
                    query_charge_system->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    result = tha_get_recv_message_item_status(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_charge_system->status = value;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
    #if 0
                /***** [更新充电系统信息响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_UPDATE_CHARGE_SYSTEM_SET, NULL) > 0){

                    Net_ThaPro_Head_t *update_charge_system = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    update_charge_system->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    update_charge_system->sequence = g_tha_sreq_updated_charge_system_config.head.sequence;
                    update_charge_system->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    update_charge_system->status = g_tha_sreq_updated_charge_system_config.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
    #endif
                /***** [查询设备故障信息响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_QUERY_DEVICE_FAULT_INFO, NULL) > 0){

                    uint8_t value = 0x00;
                    int16_t result = 0x00;
                    Net_ThaPro_Head_t *query_fault_info = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    query_fault_info->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    result = tha_get_recv_message_item_serial_number(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_fault_info->sequence = value;
                    query_fault_info->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    result = tha_get_recv_message_item_status(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_fault_info->status = value;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [查询电池充电信息响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_QUERY_BATTERY_CHARGE_INFO, NULL) > 0){

                    uint8_t value = 0x00;
                    int16_t result = 0x00;
                    Net_ThaPro_Head_t *query_battery_charge_info = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    query_battery_charge_info->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    result = tha_get_recv_message_item_serial_number(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_battery_charge_info->sequence = value;
                    query_battery_charge_info->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    result = tha_get_recv_message_item_status(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, gunno, &value);
                    if(result < 0){
                        tha_clear_recv_message_item(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, gunno);
                        tha_response_buff_release_mutex();
                        continue;
                    }
                    query_battery_charge_info->status = value;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [查询订单状态响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_QUERY_ORDER_STATE, NULL) > 0){

                    Net_ThaPro_Head_t *query_order_state = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    query_order_state->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    query_order_state->sequence = g_tha_sreq_query_order_state.head.sequence;
                    query_order_state->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    query_order_state->status = g_tha_sreq_query_order_state.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_QUERY_ORDER_STATE, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [设置定时响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_SET_RESERVATION, NULL) > 0){

                    Net_ThaPro_Head_t *set_reservation = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    set_reservation->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    set_reservation->sequence = g_tha_sreq_set_reservation[gunno].head.sequence;
                    set_reservation->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    set_reservation->status = g_tha_sreq_set_reservation[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_SET_RESERVATION, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [取消定时响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_CANCEL_RESERVATION, NULL) > 0){

                    Net_ThaPro_Head_t *cancel_reservation = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    cancel_reservation->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    cancel_reservation->sequence = g_tha_sreq_cancel_reservation[gunno].head.sequence;
                    cancel_reservation->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    cancel_reservation->status = g_tha_sreq_cancel_reservation[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_CANCEL_RESERVATION, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [停止充电响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_STOP_CHARGE, NULL) > 0){

                    Net_ThaPro_Head_t *stop_charge = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    stop_charge->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    stop_charge->sequence = g_tha_sreq_stop_charge[gunno].head.sequence;
                    stop_charge->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    stop_charge->status = g_tha_sreq_stop_charge[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_STOP_CHARGE, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [启动远程升级响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_START_UPDATE, NULL) > 0){

                    Net_ThaPro_Head_t *start_update = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    start_update->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    start_update->sequence = g_tha_sreq_start_update[gunno].head.sequence;
                    start_update->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    start_update->status = g_tha_sreq_start_update[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_SERVER_START_UPDATE, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [设置桩运行参数响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_SET_PILE_RUNNING_PARA, NULL) > 0){

                    Net_ThaPro_Head_t *set_running_para = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    set_running_para->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    set_running_para->sequence = g_tha_sreq_set_pile_running_para[gunno].head.sequence;
                    set_running_para->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    set_running_para->status = g_tha_sreq_set_pile_running_para[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_SET_PILE_RUNNING_PARA, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [设置充电枪充电功率响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_SET_CHARGE_POWER, NULL) > 0){

                    Net_ThaPro_Head_t *set_charge_power = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    set_charge_power->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    set_charge_power->sequence = g_tha_sreq_set_pile_charge_power[gunno].head.sequence;
                    set_charge_power->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    set_charge_power->status = g_tha_sreq_set_pile_charge_power[gunno].head.status;

                    rt_kprintf("6666666666666666666666666dd(%d)\n", gunno);
                    tha_message_send_port(NET_THA_MESSAGE_CMD_SET_PILE_CHARGE_POWER, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [解锁充电响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_UNLOCK_CHARGE, NULL) > 0){

                    Net_ThaPro_Head_t *unlock_charge = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    unlock_charge->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    unlock_charge->sequence = g_tha_sreq_unlock_charge[gunno].head.sequence;
                    unlock_charge->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    unlock_charge->status = g_tha_sreq_unlock_charge[gunno].head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_UNLOCK_CHARGE, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [平台下发处理用参数响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_ISSUED_PROCESS_PARA, NULL) > 0){

                    Net_ThaPro_Head_t *process_para = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    process_para->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    process_para->sequence = g_tha_sreq_issued_process_para.head.sequence;
                    process_para->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    process_para->status = g_tha_sreq_issued_process_para.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_ISSUED_PROCESS_PARA, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [私桩定时充电预约响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_SET_PRIVATE_RESERVATION, NULL) > 0){

                    Net_ThaPro_Head_t *set_private_reservation = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    set_private_reservation->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    set_private_reservation->sequence = g_tha_sreq_set_private_reservation.head.sequence;
                    set_private_reservation->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    set_private_reservation->status = g_tha_sreq_set_private_reservation.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_SET_PRIVATE_RESERVATION, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [私桩取消特定ID的定时充电预约响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_CANCEL_PRIVATE_RESERVATION, NULL) > 0){

                    Net_ThaPro_Head_t *cancel_private_reservation = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    cancel_private_reservation->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    cancel_private_reservation->sequence = g_tha_sreq_cancel_private_reservatione.head.sequence;
                    cancel_private_reservation->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    cancel_private_reservation->status = g_tha_sreq_cancel_private_reservatione.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_CANCEL_PRIVATE_RESERVATION, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
                /***** [私桩PWM电流值下发响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_ISSUED_PWM, NULL) > 0){

                    Net_ThaPro_Head_t *issued_pwm = (Net_ThaPro_Head_t*)(s_tha_response_buff.general_transmit_buff);
                    issued_pwm->start_code = NET_THA_MESSAGE_START_CODE_RES;
                    issued_pwm->sequence = g_tha_sreq_issude_pwm.head.sequence;
                    issued_pwm->check_sum = g_tha_preq_login[gunno].body.random_sign;
                    issued_pwm->status = g_tha_sreq_issude_pwm.head.status;

                    tha_message_send_port(NET_THA_MESSAGE_CMD_ISSUDE_PWM, gunno, s_tha_socket_info.fd[gunno], s_tha_response_buff.general_transmit_buff,
                            s_tha_response_buff.length);
                    tha_response_buff_release_mutex();
                    rt_thread_mdelay(250);
                }
            }
        }
        /***************************************************** [定时上报] **********************************************************/
        /***************************************************** [定时上报] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_tha_socket_info.state[gunno] == THA_SOCKET_STATE_LOGIN_SUCCESS){
                uint8_t overreturn = 0x00;
                if(heartbeat_tick[gunno] > rt_tick_get()){
                    overreturn = 0x01;
                }
                /* 数据填报只能是连上网后才行 */
                if((rt_tick_get() + overreturn *0xFFFFFFFF - heartbeat_tick[gunno]) >= (g_tha_sreq_set_pile_running_para[gunno].body.heartbeat_interval *1000)){
                    heartbeat_tick[gunno] = rt_tick_get();
                    tha_set_message_send_state(gunno, NET_THA_SEND_STATE_ONGOING, NET_THA_PREQ_EVENT_HEARTBEAT);
                    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_REQUEST, gunno, NET_THA_PREQ_EVENT_HEARTBEAT);
                }
            }
        }
        /***************************************************** [内部消耗事件] **********************************************************/
        /***************************************************** [内部消耗事件] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            /***** [上报心跳响应] *****/
            if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                    (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_HEARTBEAT, NULL) > 0){
                s_tha_socket_info.operate_fail[gunno].heartbeat = 0x00;
            }
            if(s_tha_socket_info.operate_fail[gunno].heartbeat > 0x03){

                s_tha_socket_info.operate_fail[gunno].heartbeat = 0x00;
            }
        }
        rt_thread_mdelay(100);
    }
}

static void net_tha_server_message_pro_entry(void *parameter)
{
    int8_t result = 0x00;
    uint32_t _event = 0x00;
    struct net_handle* handle = net_get_net_handle();
    tha_response_message_buf_t *response = NULL;

    while(1)
    {
        if(net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT){
            rt_thread_mdelay(5000);
            continue;
        }

        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [时间同步请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_TIME_SYNC, NULL) > 0){
                    if(g_tha_sreq_time_sync.head.status == NET_THA_STATUS_CODE_NORMAL){
                        handle->time_sync(g_tha_sreq_time_sync.body.timpstamp);
                    }
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_time_sync(response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_TIME_SYNC);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [系统复位请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_SYSTEM_REBOOT, NULL) > 0){
                    if(g_tha_sreq_system_reboot.head.status == NET_THA_STATUS_CODE_NORMAL){
                        net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                    }
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_system_reboot(response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_REBOOT);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [读取充电数据请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_CHARGE_DATA, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_charge_data(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_READ_CHARGE_DATA);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [查询系统信息请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_SYSTEM_INFO, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_system_info(response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_QUERY_SYSTEM_INFO);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [更新系统信息请求] *****/
#if 0
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_UPDATE_SYSTEM_INFO, NULL) > 0){
                    result = tha_response_padding_general_head(gunno, NET_THA_PRES_EVENT_UPDATE_SYSTEM_INFO);
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_UPDATE_SYSTEM_INFO);
                    }
                }
#endif
                /***** [查询计费规则请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_BILLING_RULE, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_billing_rule(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_QUERY_BILLING_RULE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [更新计费规则请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_UPDATE_BILLING_RULE, NULL) > 0){
                    if(g_tha_sreq_updated_billing_rule[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        tha_message_pro_update_billing_rule_request(gunno, &g_tha_sreq_updated_billing_rule[gunno], sizeof(g_tha_sreq_updated_billing_rule[gunno]));
                    }
                    result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_UPDATE_BILLING_RULE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [查询系统设置信息请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_SYSTEM_SET_INFO, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_charge_system_set(response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_QUERY_CHARGE_SYSTEM_SET);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
#if 0
                /***** [更新系统设置信息请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_UPDATE_SYSTEM_SET_INFO, NULL) > 0){
                    result = tha_response_padding_general_head(gunno, NET_THA_PRES_EVENT_UPDATE_CHARGE_SYSTEM_SET);
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_UPDATE_CHARGE_SYSTEM_SET);
                    }
                }
#endif
                /***** [查询设备故障信息请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_DEVICE_FAULT_INFO, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_device_fault_info(response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_QUERY_DEVICE_FAULT_INFO);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [查询电池充电信息请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_BATTERY_CHARGE_INFO, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_battery_charge_info(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_QUERY_BATTERY_CHARGE_INFO);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [查询订单状态请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_QUERY_ORDER_STATE, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_query_order_state(response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_QUERY_ORDER_STATE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [设置定时充电请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_SET_RESERVATION, NULL) > 0){
                    result = 0x00;
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    if(g_tha_sreq_set_reservation[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        result = tha_message_pro_set_reservation_request(gunno, &g_tha_sreq_set_reservation[gunno], sizeof(g_tha_sreq_set_reservation[gunno]));
                    }
                    if(result < 0x00){
                        g_tha_sreq_set_reservation[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                    }
                    result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_SET_RESERVATION);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [取消定时充电请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_CANCEL_RESERVATION, NULL) > 0){
                    result = 0x00;
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    if(g_tha_sreq_cancel_reservation[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        result = tha_message_pro_cancel_reservation_request(gunno, &g_tha_sreq_cancel_reservation[gunno], sizeof(g_tha_sreq_cancel_reservation[gunno]));
                    }
                    if(result < 0x00){
                        g_tha_sreq_cancel_reservation[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                    }
                    result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_CANCEL_RESERVATION);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [停止充电请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_STOP_CHARGE, NULL) > 0){
                    result = 0x00;
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    if(g_tha_sreq_stop_charge[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        result = tha_message_pro_stop_charge_request(gunno, &g_tha_sreq_stop_charge[gunno], sizeof(g_tha_sreq_stop_charge[gunno]));
                        if(result < 0x00){
                            g_tha_sreq_stop_charge[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
                        }
                    }
                    result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_STOP_CHARGE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /** 启动远程升级 */ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_START_UPDATE, NULL) > 0){
                    rt_kprintf("aaaaaaaaaaaaaaaaa(%d, %d, %d, %d)\n", gunno, g_tha_sreq_start_update[gunno].head.status,
                            net_get_ota_info()->state, tha_get_ota_was_requested_flag());
                    if(g_tha_sreq_start_update[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        rt_kprintf("bbbbbbbbbbbb\n");
                        if(net_get_ota_info()->state == NET_OTA_STATE_NULL){
                            rt_kprintf("cccccccccccccc\n");
                            if(tha_message_pro_remote_update_request(&g_tha_sreq_start_update[gunno], sizeof(g_tha_sreq_start_update[gunno])) < 0x00){
                                rt_kprintf("ddddddddddddd\n");
                                g_tha_sreq_start_update[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                            }else{
                                rt_kprintf("eeeeeeeeeee\n");
                                if(tha_get_ota_was_requested_flag() == 0x00){
                                    rt_kprintf("ffffffffffff\n");
                                    tha_set_ota_was_requested_flag();
                                    net_operation_set_event(gunno, NET_OPERATION_EVENT_START_UPDATE);
                                }else{
                                    rt_kprintf("ggggggggggggg\n");
                                    g_tha_sreq_start_update[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                                }
                            }
                        }else{
                            rt_kprintf("hhhhhhhhhhhhhh\n");
                            g_tha_sreq_start_update[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                        }
                    }
                    rt_kprintf("iiiiiiiiiiiiii\n");
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    rt_kprintf("jjjjjjjjjjjjj\n");
                    tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_SREQ_EVENT_START_UPDATE);
                }
                /***** [设置桩运行参数请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_SET_RUNNING_PARA, NULL) > 0){
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_SET_PILE_RUNNING_PARA);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
                /***** [设置充电功率请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_SET_CHARGE_POWER, NULL) > 0){
                    if(g_tha_sreq_set_pile_charge_power[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        if(tha_message_pro_set_power_request(gunno, &g_tha_sreq_set_pile_charge_power[gunno], sizeof(g_tha_sreq_set_pile_charge_power[gunno])) >= 0x00){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER);
                        }
                    }
                }
                /***** [解锁充电请求] *****/ // OK
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_UNLOCK_CHARGE, NULL) > 0){
                    result = 0x00;
                    response = tha_get_response_buff(RT_WAITING_FOREVER);
                    if(g_tha_sreq_unlock_charge[gunno].head.status == NET_THA_STATUS_CODE_NORMAL){
                        result = tha_message_pro_unlock_charge_request(gunno, &g_tha_sreq_unlock_charge[gunno], sizeof(g_tha_sreq_unlock_charge[gunno]));
                        if(result < 0x00){
                            g_tha_sreq_unlock_charge[gunno].head.status = NET_THA_STATUS_CODE_EXECUTE_FAIL;
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_START_CHARGE);
                        }
                    }
                    result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_UNLOCK_CHARGE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }
#if 0
                /***** [下发处理用参数请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_ISSUED_PROCESS_PARA, NULL) > 0){

                    result = tha_response_padding_general_head(gunno, NET_THA_PRES_EVENT_ISSUED_PROCESS_PARA);
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_ISSUED_PROCESS_PARA);
                    }
                }
                /***** [私桩预约充电求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_SET_PRIVATE_RESERVATION, NULL) > 0){

                    result = tha_response_padding_general_head(gunno, NET_THA_PRES_EVENT_SET_PRIVATE_RESERVATION);
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_SET_PRIVATE_RESERVATION);
                    }
                }
                /***** [取消私桩预约充电请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_CANCEL_PRIVATE_RESERVATION, NULL) > 0){

                    result = tha_response_padding_general_head(gunno, NET_THA_PRES_EVENT_CANCEL_PRIVATE_RESERVATION);
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_CANCEL_PRIVATE_RESERVATION);
                    }
                }
                /***** [私桩PWM电流值下发请求] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_REQUEST, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SREQ_EVENT_ISSUED_PWM, NULL) > 0){

                    result = tha_response_padding_general_head(gunno, NET_THA_PRES_EVENT_ISSUED_PWM);
                    if(result){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_ISSUED_PWM);
                    }
                }
#endif
            }

            tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [权限认证响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_AUTHORITY, NULL) > 0){
#ifndef NET_THA_AS_MONITOR
                    result = tha_message_pro_authority_response(gunno, &g_tha_sres_authority[gunno], sizeof(g_tha_sres_authority[gunno]));
                    if(result >= 0x00){
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_SUCCESS);
                    }else{
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
                    }
#endif /* NET_THA_AS_MONITOR */
                }
                /***** [上报历史订单响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_HISTORY_ORDER, NULL) > 0){
                    memset(&(s_tha_transaction_info[gunno]), 0x00, sizeof(s_tha_transaction_info[gunno]));
                    s_tha_transaction_verify[gunno] = 0x01;
                }
                /***** [停止充电动作完成，上报信息响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_HISTORY_ORDER, NULL) > 0){
                    memset(&(s_tha_transaction_info[gunno]), 0x00, sizeof(s_tha_transaction_info[gunno]));
                    s_tha_transaction_verify[gunno] = 0x01;
                }
                /***** [vin码鉴权响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_VIN_AUTHORIZE, NULL) > 0){
#ifndef NET_THA_AS_MONITOR
                    result = tha_message_pro_vin_authority_response(gunno, &g_tha_sres_authority[gunno], sizeof(g_tha_sres_authority[gunno]));
                    if(result >= 0x00){
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS);
                    }else{
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                    }
#endif /* NET_THA_AS_MONITOR */
                }
#if 0
                /***** [上报升级结果响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_UPDATE_RESULT, NULL) > 0){

                }
                /***** [查询ICCID对应的桩编号响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_QUERY_PILENUMBER_OF_ICCID, NULL) > 0){

                }
                /***** [设备查询最新可用升级包软件版本号响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_QUERY_AVAILABLE_UPDATE_PACK_VER, NULL) > 0){

                }
                /***** [设备发起远程升级请求响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_DEVICE_REQUEST_UPDATE, NULL) > 0){

                }
                /***** [私桩(桩端)查询定时充电预约响应] *****/
                if(tha_net_event_receive(NET_THA_EVENT_HANDLE_SERVER, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                        (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_SRES_EVENT_QUERY_PRIVATE_RESERVATION, NULL) > 0){

                }
#endif
            }
            /***** [解锁充电异步响应] *****/
            if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                    (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = tha_get_response_buff(RT_WAITING_FOREVER);
                result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(tha_is_start_charge_success(gunno)){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_UNLOCK_CHARGE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }else{
                    tha_response_buff_release_mutex();
                }
            }
            /***** [停止充电异步响应] *****/
            if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                    (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = tha_get_response_buff(RT_WAITING_FOREVER);
                result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(tha_is_stop_charge_success(gunno)){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_STOP_CHARGE);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }else{
                    tha_response_buff_release_mutex();
                }
            }
            /***** [设置功率异步响应] *****/
            if(tha_net_event_receive(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno,
                    (NET_THA_EVENT_OPTION_OR |NET_THA_EVENT_OPTION_CLEAR), NET_THA_PRES_EVENT_SET_POWER_ASYNCHRONOUSLY, NULL) > 0){
                response = tha_get_response_buff(RT_WAITING_FOREVER);
                result = tha_response_padding_general_message(gunno, response->general_transmit_buff, NET_THA_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(tha_is_set_power_success(gunno)){
                        tha_net_event_send(NET_THA_EVENT_HANDLE_CHARGEPILE, NET_THA_EVENT_TYPE_RESPONSE, gunno, NET_THA_PRES_EVENT_SET_CHARGE_POWER);
                    }else{
                        tha_response_buff_release_mutex();
                    }
                }else{
                    tha_response_buff_release_mutex();
                }
            }
        }
        rt_thread_mdelay(200);
    }

}

int32_t net_tha_message_send_init(void)
{
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        memset(&(s_tha_transaction_info[gunno]), 0x00, sizeof(s_tha_transaction_info[gunno]));
        s_tha_transaction_info_num[gunno] = 0x00;
        s_tha_same_transaction_report_count[gunno] = 0x00;
        s_tha_transaction_verify[gunno] = 0x00;
    }
    if(rt_thread_init(&s_tha_message_send_thread, "tha_send", net_tha_message_send_thread_entry, NULL,
            s_tha_message_send_thread_stack, NET_THA_MESSAGE_SEND_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("thaisen message send thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_tha_message_send_thread) != RT_EOK){
        LOG_E("thaisen message send thread startup fail, please check");
        return -0x01;
    }

    if(rt_thread_init(&s_tha_server_message_pro_thread, "tha_server", net_tha_server_message_pro_entry, NULL,
            s_tha_server_message_pro_thread_stack, NET_THA_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("thaisen server message process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_tha_server_message_pro_thread) != RT_EOK){
        LOG_E("thaisen server message process thread startup fail, please check");
        return -0x01;
    }

    if(rt_mutex_init(&s_tha_response_buff_mutex, "tha_tbmutex", RT_IPC_FLAG_PRIO) != RT_EOK){
        LOG_E("thaisen response buff mutex create fail");
    }
    rt_kprintf("fFFFFFFFFFFFFF\n");
    return 0x00;
}

#endif /* NET_PACK_USING_THA */

