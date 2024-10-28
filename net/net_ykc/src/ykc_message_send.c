/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#include "ykc_message_send.h"
#include "ykc_message_receive.h"
#include "ykc_transceiver.h"
#include "ykc_message_padding.h"

#include "net_operation.h"

#define DBG_TAG "ykc_send"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YKC

#define NET_YKC_HEARTBEAT_TIMEOUT_RENTRY                     3           /* 心跳超时次数 */
#define NET_YKC_OPEN_SOCKET_RENTRY                           5           /* 打开socket尝试次数 */
#define NET_YKC_LOGIN_RENTRY                                 5           /* 登录尝试次数 */
#define NET_YKC_WAIT_LOGIN_RENTRY                            100         /* 等待登录结果尝试次数 */
#define NET_YKC_WAIT_UNLOCK_TIMEOUT                          (10 *1000)  /* 等待socket 解锁超时时间(单位：ms) */

#define NET_YKC_LOGIN_OPERATION_INTERVAL                     30000       /* 登录操作间隔(单位ms:30 *1000 = 30s) */
#define NET_YKC_HEARTBEAT_REPORT_INTERVAL                    10000       /* 心跳间隔(单位ms:10 *1000 = 10s) */

#define NET_YKC_REALTIME_DATA_IDLE_INTERVAL                  300000      /* 实时数据空闲上报间隔(单位ms:5 *60 *1000 = 5min) */
#define NET_YKC_REALTIME_DATA_CHARGING_INTERVAL              15000       /* 实时数据充电中上报间隔(单位ms:15 *1000 = 15s) */
#define NET_YKC_REALTIME_DATA_LINK_INTERVAL                  5000        /* 实时数据刚连上网时上报间隔(单位ms:5 *1000 = 5s) */
#define NET_YKC_SAME_TRANSATION_REPORT_COUNT_MAX             10          /* 相同订单最大上报次数 */

struct ykc_wait_response{
    uint32_t message_wait_response_state[NET_SYSTEM_GUN_NUMBER];                       /* 报文已发送发送，等待响应状态 */
    uint32_t message_repeat_time[NET_SYSTEM_GUN_NUMBER][NET_YKC_CHARGEPILE_PREQ_NUM];  /* 报文重发计时 */
};

uint8_t s_ykc_current_transaction_number[NET_SYSTEM_GUN_NUMBER][NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];
static uint8_t s_ykc_same_transaction_report_count[NET_SYSTEM_GUN_NUMBER];
static uint8_t s_ykc_transaction_verify[NET_SYSTEM_GUN_NUMBER];
static struct ykc_wait_response s_ykc_wait_response;
static ykc_socket_info_t s_ykc_socket_info;
static uint16_t s_ykc_message_serial_number[NET_SYSTEM_GUN_NUMBER];                      /* 报文序列号 */
static uint32_t s_ykc_message_send_state[NET_SYSTEM_GUN_NUMBER];                        /* 报文发送状态 */
static uint32_t s_ykc_chargepile_event[NET_YKC_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];
static uint32_t s_ykc_server_event[NET_YKC_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];

static struct rt_thread s_ykc_message_send_thread;
static uint8_t s_ykc_message_send_thread_stack[NET_YKC_MESSAGE_SEND_THREAD_STACK_SIZE];
static struct rt_thread s_ykc_server_message_pro_thread;
static uint8_t s_ykc_server_message_pro_thread_stack[NET_YKC_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE];
static ykc_response_message_buf_t s_ykc_response_buff;
static struct rt_semaphore s_ykc_response_buff_sem;

/** 登录签到 */
Net_YkcPro_PReq_LogIn_t g_ykc_preq_login;
/** 上报心跳 */
Net_YkcPro_PReq_HeartBeat_t g_ykc_preq_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证 */
Net_YkcPro_PReq_BillingModel_Verify_t g_ykc_preq_billing_model_verify;   // OK
/** 计费模型请求 */
Net_YkcPro_PReq_BillingModel_Request_t g_ykc_preq_billing_model_request;  // OK
/** 上传实时监测数据 */
Net_YkcPro_PRes_Query_PReq_Report_RealTimeData_t g_ykc_preq_report_realtime_data[NET_SYSTEM_GUN_NUMBER];  // OK
/** 充电握手 */
Net_YkcPro_PReq_ShakeHand_t g_ykc_preq_shake_hand[NET_SYSTEM_GUN_NUMBER];   // OK
/** 参数配置 */
Net_YkcPro_PReq_ParameterConfig_t g_ykc_preq_parameter_config[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电结束 */
Net_YkcPro_PReq_ChargeFinish_t g_ykc_preq_charge_finish[NET_SYSTEM_GUN_NUMBER];   // OK
/** 错误报文 */
Net_YkcPro_PReq_ErrorMessage_t g_ykc_preq_error_message[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中 BMS 终止 */
Net_YkcPro_PReq_BmsEnd_t g_ykc_preq_bms_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中充电机终止 */
Net_YkcPro_PReq_ChargerEnd_t g_ykc_preq_charger_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 需求与充电机输出 */
Net_YkcPro_PReq_BmsCommand_ChargerOut_t g_ykc_preq_bmscommand_chargerout[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 信息 */
Net_YkcPro_PReq_BmsInfo_t g_ykc_preq_bms_info[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电桩主动申请启动充电 */
Net_YkcPro_PReq_ApplyCharge_Active_t g_ykc_preq_apply_charge_active[NET_SYSTEM_GUN_NUMBER];  // OK
/** 交易记录 */
Net_YkcPro_PReq_TransactionRecords_t g_ykc_preq_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 地锁数据上送 */
Net_YkcPro_PReq_GroundLock_Info_t g_ykc_preq_ground_lock_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 充电桩主动申请并充充电 */
Net_YkcPro_PReq_ApplyMergeCharge_Active_t g_ykc_preq_apply_merge_charge_active[NET_SYSTEM_GUN_NUMBER];  // OK

/** 升级结果上送 */
Net_YkcPro_PRes_RemoteUpdate_t g_ykc_pres_remote_update;  // OK

/**************************************************************************
 * 函数名                 ykc_get_socket_info
 * 功能                     获取socket信息
 * 说明
 * ***********************************************************************/
ykc_socket_info_t* ykc_get_socket_info(void)
{
    return &s_ykc_socket_info;
}

/**************************************************************************
 * 函数名                 ykc_response_buff_take_sem_forever
 * 功能                     获取响应缓存互斥量
 * 说明
 * ***********************************************************************/
static int32_t ykc_response_buff_take_sem_forever(int32_t timeout)
{
    return rt_sem_take(&s_ykc_response_buff_sem, timeout);
}
/**************************************************************************
 * 函数名                 ykc_response_buff_release_sem
 * 功能                     释放响应缓存互斥信号量
 * 说明
 * ***********************************************************************/
static void ykc_response_buff_release_sem(void)
{
    rt_sem_release(&s_ykc_response_buff_sem);
}
/**************************************************************************
 * 函数名                 ykc_get_response_buff
 * 功能                     获取响应缓存
 * 说明
 * ***********************************************************************/
static ykc_response_message_buf_t* ykc_get_response_buff(int32_t timeout)
{
    if(ykc_response_buff_take_sem_forever(timeout) < 0){
        return NULL;
    }
    return &s_ykc_response_buff;
}

/**************************************************************************
 * 函数名                 ykc_transaction_is_verify
 * 功能                     查询交易是否已确认
 * 说明
 * ***********************************************************************/
uint8_t ykc_transaction_is_verify(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ykc_transaction_verify[gunno];
}

/**************************************************************************
 * 函数名                 ykc_set_transaction_verify_state
 * 功能                     设置交易确认状态
 * 说明
 * ***********************************************************************/
void ykc_set_transaction_verify_state(uint8_t gunno, uint8_t state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_ykc_transaction_verify[gunno] = 0x01;
    }else{
        s_ykc_transaction_verify[gunno] = 0x00;
    }
}

/**************************************************************************
 * 函数名                 ykc_net_event_send
 * 功能                     网络事件发送
 * 说明
 * ***********************************************************************/
int32_t ykc_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event)
{
    if(event_handle >= NET_YKC_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_YKC_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }

    if(event_handle == NET_YKC_EVENT_HANDLE_CHARGEPILE){
        s_ykc_chargepile_event[event_type][gunno] |= (1 <<event);
    }else{
        s_ykc_server_event[event_type][gunno] |= (1 <<event);
    }

    return 0;
}
/**************************************************************************
 * 函数名                 ykc_net_event_receive
 * 功能                     网络事件接收
 * 说明                     若事件发生则对应位为 1，否则为0
 * ***********************************************************************/
int32_t ykc_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf)
{
    if(event_handle >= NET_YKC_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_YKC_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }
    uint32_t (*event_set)[NET_SYSTEM_GUN_NUMBER] = NULL;

    if(event_handle == NET_YKC_EVENT_HANDLE_CHARGEPILE){
        event_set = s_ykc_chargepile_event;
    }else{
        event_set = s_ykc_server_event;
    }

    if(event_buf){
        (*event_buf) = event_set[event_type][gunno];
    }

    if(event_set[event_type][gunno] &(1 <<event)){
        if(option &NET_YKC_EVENT_OPTION_AND){
            if((event_set[event_type][gunno] &(1 <<event)) != (1 <<event)){
                return -4;
            }
        }
        if(option &NET_YKC_EVENT_OPTION_CLEAR){
            event_set[event_type][gunno] &= (~(1 <<event));
        }
        return 1;
    }
    return 0;
}

/**************************************************************************
 * 函数名                 ykc_get_message_send_state
 * 功能                     获取报文发送状态
 * 说明                     若报文正在发送则对应位为 1，否则为0
 * ***********************************************************************/
uint8_t ykc_get_message_send_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_ykc_message_send_state[gunno] &(1 <<message_bit)){
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 ykc_set_message_send_state
 * 功能                     设置报文发送状态
 * 说明                     若报文正在发送则对应位置 1，否则置0
 * ***********************************************************************/
void ykc_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_ykc_message_send_state[gunno] |= (1 <<message_bit);
    }else{
        s_ykc_message_send_state[gunno] &= (~(1 <<message_bit));
    }
}

/**************************************************************************
 * 函数名                 ykc_set_message_wait_response_state
 * 功能                     设置报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
static void ykc_set_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_YKC_CHARGEPILE_PREQ_NUM){
        return;
    }
    s_ykc_wait_response.message_wait_response_state[gunno] |= (1 <<message_bit);
    s_ykc_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
}
/**************************************************************************
 * 函数名                 ykc_clear_message_wait_response_state
 * 功能                     清除报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
void ykc_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_YKC_CHARGEPILE_PREQ_NUM){
        return;
    }
    s_ykc_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
}
/**************************************************************************
 * 函数名                 ykc_exist_message_wait_response
 * 功能                     检查是否存在已发送的报文等待响应
 * 说明
 * ***********************************************************************/
uint8_t ykc_exist_message_wait_response(uint8_t gunno, uint32_t *state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_ykc_wait_response.message_wait_response_state[gunno]){
        if(state){
            *state = s_ykc_wait_response.message_wait_response_state[gunno];
        }
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 ykc_get_message_wait_response_timeout_state
 * 功能                     获取报文等待响应超时状态
 * 说明
 * ***********************************************************************/
uint8_t ykc_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(message_bit >= NET_YKC_CHARGEPILE_PREQ_NUM){
        return 0x00;
    }
    if(s_ykc_wait_response.message_wait_response_state[gunno] &(1 <<message_bit)){
        if(s_ykc_wait_response.message_repeat_time[gunno][message_bit] > rt_tick_get()){
            s_ykc_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
        }
        if((rt_tick_get() - s_ykc_wait_response.message_repeat_time[gunno][message_bit]) > timeout){
            s_ykc_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
            return 0x01;
        }
    }
    return 0x00;
}

/**************************************************************************
 * 函数名                 ykc_ascii_to_bcd
 * 功能                     将字符码转成BCD
 * 说明
 * ***********************************************************************/
void ykc_ascii_to_bcd(uint8_t *ascii, uint8_t alen, uint8_t *bcd, uint8_t blen)
{
    uint8_t index, c;

    if(alen %0x02){
        for(index = 0; index < (alen /0x02); index++) {
            c  = (*ascii++) << 4;
            c |= (*ascii++) & 0x0F;
            *bcd++ = c;
        }
        if(blen > (alen %0x02)){
            *bcd  = (*ascii) << 4;
        }
    }else{
        for(index = 0; index < blen; index++) {
            c  = (*ascii++) << 4;
            c |= (*ascii++) & 0x0F;
            *bcd++ = c;
        }
    }
}


static void net_ykc_message_send_thread_entry(void *parameter)
{
    uint8_t step = NET_YKC_NET_STATE_OPEN_SOCKET, is_power_on = 0x00;
    uint32_t delay = 0x00, wait_unlock = 0x00;
    uint32_t heartbeat_tick[NET_SYSTEM_GUN_NUMBER];

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_OPEN_LINK) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        net_get_net_handle()->data_updata();
        if((net_get_net_handle()->net_fault) &NET_FAULT_PHYSICAL_LAYER){
            s_ykc_socket_info.state = YKC_SOCKET_STATE_PHY;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_PHY;
            s_ykc_socket_info.fd = -0x01;
            step = NET_YKC_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YKC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YKC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_SIM_CARD){
            s_ykc_socket_info.state = YKC_SOCKET_STATE_SIM;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_SIM;
            s_ykc_socket_info.fd = -0x01;
            step = NET_YKC_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YKC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YKC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_DATA_LINK_LAYER){
            s_ykc_socket_info.state = YKC_SOCKET_STATE_DATA_LINK;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_DATA_LINK;
            s_ykc_socket_info.fd = -0x01;
            step = NET_YKC_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YKC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YKC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_MODULE_INIT){
            s_ykc_socket_info.state = YKC_SOCKET_STATE_MODULE_INIT;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_MODULE_INIT;
            s_ykc_socket_info.fd = -0x01;
            step = NET_YKC_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YKC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YKC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        /***************************************************** [登录认证] **********************************************************/
        /***************************************************** [登录认证] **********************************************************/
        if(s_ykc_socket_info.state != YKC_SOCKET_STATE_LOGIN_SUCCESS){
            rt_kprintf("state(%d, %d, %d)\n", s_ykc_socket_info.state, step, (rt_tick_get() - delay));
            switch(step){
            case NET_YKC_NET_STATE_OPEN_SOCKET:
                s_ykc_socket_info.state = YKC_SOCKET_STATE_OPEN;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_OPEN;
                if(delay > rt_tick_get()){
                    delay = rt_tick_get();
                }
                if(((rt_tick_get() - delay) > NET_YKC_LOGIN_OPERATION_INTERVAL) || is_power_on){
                    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                    int32_t result = 0x00;
                    struct net_handle* handle = net_get_net_handle();
                    char *host = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, NULL, 0x00, option));
                    uint16_t port = *((uint16_t*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PORT, NULL, 0x00, option)));

                    if((port == 0x00) || (port == 0xFFFF)){
                        host = "121.43.69.62";
                        port = 8767;
                    }

                    net_operation_set_target_socket_domain(host, strlen(host));
                    net_operation_set_target_socket_port(port);

                    result = ykc_socket_open(&(s_ykc_socket_info.fd), host, strlen(host), port);
                    if(result >= 0){
                        int32_t recv_timeout = 0x0A;

                        LOG_D("ykc socket open success with host[%s] port[%d] fd(%d)", host, port, s_ykc_socket_info.fd);
                        s_ykc_socket_info.operate_fail.open_socket = 0;
                        step = NET_YKC_NET_STATE_LOGIN;

                        ykc_socket_modify_recv_timeout(s_ykc_socket_info.fd, recv_timeout);
                    }else{
                        LOG_W("ykc fail to open socket with host[%s] port[%d] num|%d", host, port, s_ykc_socket_info.operate_fail.open_socket);
                        delay = rt_tick_get();
                        s_ykc_socket_info.operate_fail.open_socket++;
                    }
                    for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                        s_ykc_message_serial_number[gunno] = 0x00;
                    }
                    is_power_on = 0x00;
                }
                break;
            case NET_YKC_NET_STATE_LOGIN:
            {
                uint8_t vaild_len = 0, rentry = 0, data[NET_YKC_SIM_BCD_LENGTH_DEFAULT *0x02 + 0x01], *sim_no = NULL;
                uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                struct net_handle* handle = net_get_net_handle();

                memset(data, 0x00, (NET_YKC_SIM_BCD_LENGTH_DEFAULT *0x02 + 0x01));
                (void)(handle->get_system_data(NET_SYSTEM_DATA_NAME_ICCID, data, sizeof(data), option));

                g_ykc_preq_login.body.operators = *(uint8_t*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_OPERATOR, NULL, 0x00, option));
                memset(g_ykc_preq_login.body.sim_number, '\0', sizeof(g_ykc_preq_login.body.sim_number));
                sim_no = data;
                vaild_len = sizeof(g_ykc_preq_login.body.sim_number);
                vaild_len = vaild_len > (strlen((char*)sim_no) /2)? strlen((char*)sim_no) : vaild_len;

                ykc_ascii_to_bcd(sim_no, strlen((char*)sim_no), g_ykc_preq_login.body.sim_number, NET_YKC_SIM_BCD_LENGTH_DEFAULT);

                switch (g_ykc_preq_login.body.operators) {
                case NET_OPERATOR_NAME_CHINA_MOBILE:
                    g_ykc_preq_login.body.operators = NET_YKC_OPERATOR_MOBILE;
                    break;
                case NET_OPERATOR_NAME_CHINA_TELECOM:
                    g_ykc_preq_login.body.operators = NET_YKC_OPERATOR_TELECOM;
                    break;
                case NET_OPERATOR_NAME_CHINA_UNICOM:
                    g_ykc_preq_login.body.operators = NET_YKC_OPERATOR_UNICOM;
                    break;
                default:
                    g_ykc_preq_login.body.operators = NET_YKC_OPERATOR_OTHER;
                    break;
                }

                ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, 0x00,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SRES_EVENT_LOGIN, NULL);
                s_ykc_socket_info.state = YKC_SOCKET_STATE_LOGIN_WAIT;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_LOGIN_WAIT;
                ykc_message_send_port(NETYKC_PREQCMD_SINGIN, s_ykc_socket_info.fd, &g_ykc_preq_login,
                        sizeof(g_ykc_preq_login));
                while(rentry < NET_YKC_WAIT_LOGIN_RENTRY){
                    if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, 0x00,
                            (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SRES_EVENT_LOGIN, NULL) > 0){
                        LOG_D("ykc login success");
                        s_ykc_socket_info.operate_fail.login = 0;
                        step = NET_YKC_NET_STATE_MONITORING;

                        s_ykc_socket_info.state = YKC_SOCKET_STATE_LOGIN_SUCCESS;
                        net_get_net_handle()->net_state = NET_SOCKET_STATE_LOGIN_SUCCESS;
                        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                            s_ykc_message_serial_number[gunno]++;
                            s_ykc_socket_info.heartbeat[gunno] = 0x00;
                        }
                        break;
                    }
                    rt_thread_mdelay(100);
                    rentry++;
                }
                if(rentry >= NET_YKC_WAIT_LOGIN_RENTRY){
                    delay = rt_tick_get();
                    wait_unlock = rt_tick_get();
                    step = NET_YKC_NET_STATE_OPEN_SOCKET;
                    s_ykc_socket_info.state = YKC_SOCKET_STATE_OPEN;
                    net_get_net_handle()->net_state = NET_SOCKET_STATE_OPEN;
                    while(ykc_socket_is_lock()){
                        if((rt_tick_get() - wait_unlock) > NET_YKC_WAIT_UNLOCK_TIMEOUT){
                            break;
                        }
                        rt_thread_mdelay(50);
                    }

                    ykc_socket_close(s_ykc_socket_info.fd);
                    s_ykc_socket_info.fd = -0x01;
                    s_ykc_socket_info.operate_fail.login++;
                    LOG_D("ykc login fail(timeout) num|%d", s_ykc_socket_info.operate_fail.login);
                }else{
                    rt_thread_mdelay(100);
                    ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, 0x00, NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST);
                }
                break;
            }
            case NET_YKC_NET_STATE_MONITORING:
                s_ykc_socket_info.state = YKC_SOCKET_STATE_LOGIN_SUCCESS;
                break;
            default:
                delay = rt_tick_get();
                wait_unlock = rt_tick_get();
                step = NET_YKC_NET_STATE_OPEN_SOCKET;
                s_ykc_socket_info.state = YKC_SOCKET_STATE_OPEN;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_OPEN;
                while(ykc_socket_is_lock()){
                    if((rt_tick_get() - wait_unlock) > NET_YKC_WAIT_UNLOCK_TIMEOUT){
                        break;
                    }
                    rt_thread_mdelay(50);
                }

                ykc_socket_close(s_ykc_socket_info.fd);
                s_ykc_socket_info.fd = -0x01;
                break;
            }
        }

        if(s_ykc_socket_info.operate_fail.open_socket > NET_YKC_OPEN_SOCKET_RENTRY){
            s_ykc_socket_info.operate_fail.open_socket = 0x00;
            LOG_W("ykc open socket rentry = 0x00");
            net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_TARGET, 0x00);
        }
        if(s_ykc_socket_info.operate_fail.login > NET_YKC_LOGIN_RENTRY){
            s_ykc_socket_info.operate_fail.login = 0x00;
            LOG_W("ykc login rentry = 0x00");
            net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_TARGET, 0x00);
        }

        if(s_ykc_socket_info.state != YKC_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
            for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                heartbeat_tick[gunno] = rt_tick_get();
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_HEARTBEAT);
                ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno, NET_YKC_PREQ_EVENT_HEARTBEAT);
            }
            rt_thread_mdelay(1000);
            continue;
        }

        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_ykc_socket_info.heartbeat[gunno] > NET_YKC_HEARTBEAT_TIMEOUT_RENTRY){
                s_ykc_socket_info.heartbeat[gunno] = 0x00;

                delay = rt_tick_get();
                wait_unlock = rt_tick_get();

                step = NET_YKC_NET_STATE_OPEN_SOCKET;
                s_ykc_socket_info.state = YKC_SOCKET_STATE_OPEN;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_OPEN;
                while(ykc_socket_is_lock()){
                    if((rt_tick_get() - wait_unlock) > NET_YKC_WAIT_UNLOCK_TIMEOUT){
                        break;
                    }
                    rt_thread_mdelay(50);
                }

                ykc_socket_close(s_ykc_socket_info.fd);
                s_ykc_socket_info.fd = -0x01;

                LOG_D("ykc heartbeat timeout");
                break;
            }
        }

        /***************************************************** [定时上报] **********************************************************/
        /***************************************************** [定时上报] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint8_t overreturn = 0x00;
            if(heartbeat_tick[gunno] > rt_tick_get()){
                overreturn = 0x01;
            }
            /* 数据填报只能是连上网后才行 */
            if((rt_tick_get() + overreturn *0xFFFFFFFF - heartbeat_tick[gunno]) >=  NET_YKC_HEARTBEAT_REPORT_INTERVAL){
                heartbeat_tick[gunno] = rt_tick_get();
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_HEARTBEAT);
                ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno, NET_YKC_PREQ_EVENT_HEARTBEAT);
            }
        }
        /***************************************************** [内部消耗事件] **********************************************************/
        /***************************************************** [内部消耗事件] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            /***** [上报心跳响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SRES_EVENT_HEARTBEAT, NULL) > 0){
                s_ykc_socket_info.heartbeat[gunno] = 0x00;
            }
        }
        /***************************************************** [数据请求] **********************************************************/
        /***************************************************** [数据请求] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint32_t _event = 0;
            ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(!_event){
                continue;   /* 此枪没有请求事件,不进行事件查询 */
            }
            /***** [上报心跳] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_HEARTBEAT, NULL) > 0){
                if(s_ykc_socket_info.heartbeat[gunno] < 0xFF){
                    s_ykc_socket_info.heartbeat[gunno]++;
                }
                g_ykc_preq_heartbeat[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_HEARTBEAT, s_ykc_socket_info.fd, &g_ykc_preq_heartbeat[gunno],
                        sizeof(g_ykc_preq_heartbeat[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_HEARTBEAT);

//                LOG_D("ykc gunno(%d) heartbeat timeout count(%d)\n", gunno, s_ykc_socket_info.heartbeat[gunno]);
                rt_thread_mdelay(250);
            }
            /***** [计费模型验证] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY);
                g_ykc_preq_billing_model_verify.head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_BILLING_MODEL_VERIFY, s_ykc_socket_info.fd, &g_ykc_preq_billing_model_verify,
                        sizeof(g_ykc_preq_billing_model_verify));
                ykc_set_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY);
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY);
                rt_thread_mdelay(250);
            }
            /***** [计费模型请求] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST);
                g_ykc_preq_billing_model_request.head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_BILLING_MODEL_REQUEST, s_ykc_socket_info.fd, &g_ykc_preq_billing_model_request,
                        sizeof(g_ykc_preq_billing_model_request));
                ykc_set_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST);
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST);
                rt_thread_mdelay(250);
            }
            /***** [上传实时监测数据] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_REPORT_REALTIME_DATA, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_REPORT_REALTIME_DATA);
                g_ykc_preq_report_realtime_data[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_PRESCMD_REPORT_REALTIME_DATA, s_ykc_socket_info.fd, &g_ykc_preq_report_realtime_data[gunno],
                        sizeof(g_ykc_preq_report_realtime_data[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_REPORT_REALTIME_DATA);
                rt_thread_mdelay(250);
            }
            /***** [充电握手] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_CHARGE_SHAKE_HAND, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_CHARGE_SHAKE_HAND);
                g_ykc_preq_shake_hand[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_CHARGE_SHAKE_HAND, s_ykc_socket_info.fd, &g_ykc_preq_shake_hand[gunno],
                        sizeof(g_ykc_preq_shake_hand[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_CHARGE_SHAKE_HAND);
                rt_thread_mdelay(250);
            }
            /***** [参数配置] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_PARA_CONFIG, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_PARA_CONFIG);
                g_ykc_preq_parameter_config[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_PARA_CONFIG, s_ykc_socket_info.fd, &g_ykc_preq_parameter_config[gunno],
                        sizeof(g_ykc_preq_parameter_config[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_PARA_CONFIG);
                rt_thread_mdelay(250);
            }
            /***** [充电结束] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_CHARGE_END, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_CHARGE_END);
                g_ykc_preq_charge_finish[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_CHARGE_END, s_ykc_socket_info.fd, &g_ykc_preq_charge_finish[gunno],
                        sizeof(g_ykc_preq_charge_finish[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_CHARGE_END);
                rt_thread_mdelay(250);
            }
            /***** [错误报文] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_ERROR_MESSAGE, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_ERROR_MESSAGE);
                g_ykc_preq_error_message[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_ERROR_MESSAGE, s_ykc_socket_info.fd, &g_ykc_preq_error_message[gunno],
                        sizeof(g_ykc_preq_error_message[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_ERROR_MESSAGE);
                rt_thread_mdelay(250);
            }
            /***** [充电过程中 BMS 终止] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_BMS_STOP, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_BMS_STOP);
                g_ykc_preq_bms_end[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_BMS_STOP, s_ykc_socket_info.fd, &g_ykc_preq_bms_end[gunno],
                        sizeof(g_ykc_preq_bms_end[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_BMS_STOP);
                rt_thread_mdelay(250);
            }
            /***** [充电过程中充电机终止] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_CHARGER_STOP, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_CHARGER_STOP);
                g_ykc_preq_charger_end[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_CHARGER_STOP, s_ykc_socket_info.fd, &g_ykc_preq_charger_end[gunno],
                        sizeof(g_ykc_preq_charger_end[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_CHARGER_STOP);
                rt_thread_mdelay(250);
            }
            /***** [充电过程 BMS 需求与充电机输出] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                g_ykc_preq_bmscommand_chargerout[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_CHARGER_OUTPUT_BMS_REQUIRE, s_ykc_socket_info.fd, &g_ykc_preq_bmscommand_chargerout[gunno],
                        sizeof(g_ykc_preq_bmscommand_chargerout[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                rt_thread_mdelay(250);
            }
            /***** [充电过程 BMS 信息] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_BMS_INFO, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_BMS_INFO);
                g_ykc_preq_bms_info[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_BMS_INFO, s_ykc_socket_info.fd, &g_ykc_preq_bms_info[gunno],
                        sizeof(g_ykc_preq_bms_info[gunno]));
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_BMS_INFO);
                rt_thread_mdelay(250);
            }
            /***** [充电桩主动申请启动充电] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_APPLY_START_CHARGE, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_APPLY_START_CHARGE);
                g_ykc_preq_apply_charge_active[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_APPLY_START_CHARGE, s_ykc_socket_info.fd, &g_ykc_preq_apply_charge_active[gunno],
                        sizeof(g_ykc_preq_apply_charge_active[gunno]));
                ykc_set_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_APPLY_START_CHARGE);
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_APPLY_START_CHARGE);
                rt_thread_mdelay(250);
            }
            /***** [交易记录] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_TRANSACTION_RECORD, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_TRANSACTION_RECORD);
                g_ykc_preq_transaction_records[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_TRANSACTION_RECORD, s_ykc_socket_info.fd, &g_ykc_preq_transaction_records[gunno],
                        sizeof(g_ykc_preq_transaction_records[gunno]));
                ykc_set_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_TRANSACTION_RECORD);
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_TRANSACTION_RECORD);

                ykc_set_transaction_verify_state(gunno, 0x00);
                if(memcmp(g_ykc_preq_transaction_records[gunno].body.serial_number, s_ykc_current_transaction_number[gunno], NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT)){
                    s_ykc_same_transaction_report_count[gunno] = 0x00;
                    memcpy(s_ykc_current_transaction_number[gunno], g_ykc_preq_transaction_records[gunno].body.serial_number, NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT);
                }else{
                    if(++s_ykc_same_transaction_report_count[gunno] > NET_YKC_SAME_TRANSATION_REPORT_COUNT_MAX){
                        ykc_set_transaction_verify_state(gunno, 0x01);
                        s_ykc_same_transaction_report_count[gunno] = 0x00;
                        ykc_clear_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_TRANSACTION_RECORD);
                    }
                }

                rt_thread_mdelay(250);
            }
            /***** [地锁数据上送] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_GROUNDLOCK_INFO, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_GROUNDLOCK_INFO);
                g_ykc_preq_ground_lock_info[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_GROUNDLOCK_INFO, s_ykc_socket_info.fd, &g_ykc_preq_ground_lock_info[gunno],
                        sizeof(g_ykc_preq_ground_lock_info[gunno]));
                ykc_set_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_GROUNDLOCK_INFO);
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_GROUNDLOCK_INFO);
                rt_thread_mdelay(250);
            }
            /***** [充电桩主动申请并充充电] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_APPLY_START_MERGECHARGE, NULL) > 0){

                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_ONGOING, NET_YKC_PREQ_EVENT_APPLY_START_MERGECHARGE);
                g_ykc_preq_apply_merge_charge_active[gunno].head.sequence = s_ykc_message_serial_number[gunno]++;
                ykc_message_send_port(NETYKC_PREQCMD_APPLY_START_MERGECHARGE, s_ykc_socket_info.fd, &g_ykc_preq_apply_merge_charge_active[gunno],
                        sizeof(g_ykc_preq_apply_merge_charge_active[gunno]));
                ykc_set_message_wait_response_state(gunno, NET_YKC_PREQ_EVENT_APPLY_START_MERGECHARGE);
                ykc_set_message_send_state(gunno, NET_YKC_SEND_STATE_COMPLETE, NET_YKC_PREQ_EVENT_APPLY_START_MERGECHARGE);
                rt_thread_mdelay(250);
            }
        }
        /***************************************************** [数据响应] **********************************************************/
        /***************************************************** [数据响应] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint32_t _event = 0;
            ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
            if(!_event){
                continue;   /* 此枪没有响应事件,不进行事件查询 */
            }
            /***** [读取实时数据响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_READ_REALTIME_DATA, NULL) > 0){

                Net_YkcPro_PRes_Query_PReq_Report_RealTimeData_t *realtime_data = (Net_YkcPro_PRes_Query_PReq_Report_RealTimeData_t*)(s_ykc_response_buff.general_transmit_buff);

                realtime_data->head.sequence = g_ykc_sreq_query_realtime_data[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PREQCMD_PRESCMD_REPORT_REALTIME_DATA, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台远程控制启机响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SERVER_START_CHARGE, NULL) > 0){

                Net_YkcPro_PRes_Remote_StartCharge_t *start_charge = (Net_YkcPro_PRes_Remote_StartCharge_t*)(s_ykc_response_buff.general_transmit_buff);
                start_charge->head.sequence = g_ykc_sreq_remote_start_charge[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_SERVER_START_CHARGE, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台远程停机响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SERVER_STOP_CHARGE, NULL) > 0){

                Net_YkcPro_PRes_Remote_StopCharge_t *stop_charge = (Net_YkcPro_PRes_Remote_StopCharge_t*)(s_ykc_response_buff.general_transmit_buff);

                stop_charge->head.sequence = g_ykc_sreq_remote_stop_charge[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_SERVER_STOP_CHARGE, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [远程账户余额更新响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_ACCOUNT_BALLANCE_UPDATE, NULL) > 0){
                Net_YkcPro_PRes_AccountBallance_Update_t *ballance_update = (Net_YkcPro_PRes_AccountBallance_Update_t*)(s_ykc_response_buff.general_transmit_buff);
                ballance_update->head.sequence = g_ykc_sreq_account_ballance_update[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_ACCOUNT_BALLANCE_UPDATE, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [离线卡数据同步响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SYNC_OFFLINECARD, NULL) > 0){

                Net_YkcPro_PRes_Sync_OfflineCard_t *sync_card = (Net_YkcPro_PRes_Sync_OfflineCard_t*)(s_ykc_response_buff.general_transmit_buff);
                sync_card->head.sequence = g_ykc_sreq_sync_offline_card.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_SYNC_OFFLINECARD, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [离线卡数据清除响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_CLEAR_OFFLINECARD, NULL) > 0){

                Net_YkcPro_PRes_Clear_OfflineCard_t *clear_card = (Net_YkcPro_PRes_Clear_OfflineCard_t*)(s_ykc_response_buff.general_transmit_buff);
                clear_card->head.sequence = g_ykc_sreq_clear_offline_card.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_CLEAR_OFFLINECARD, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [离线卡数据查询响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_QUERY_OFFLINECARD, NULL) > 0){

                Net_YkcPro_PRes_Query_OfflineCard_t *query_card = (Net_YkcPro_PRes_Query_OfflineCard_t*)(s_ykc_response_buff.general_transmit_buff);
                query_card->head.sequence = g_ykc_sreq_query_offline_card.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_QUERY_OFFLINECARD, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [充电桩工作参数设置响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SET_WORK_PARA, NULL) > 0){

                Net_YkcPro_PRes_Set_WorkPara_t *work_para = (Net_YkcPro_PRes_Set_WorkPara_t*)(s_ykc_response_buff.general_transmit_buff);
                work_para->head.sequence = g_ykc_sreq_set_work_para.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_SET_WORK_PARA, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [对时设置响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_TIME_SYNC, NULL) > 0){

                Net_YkcPro_PRes_TimeSync_t *time_sync = (Net_YkcPro_PRes_TimeSync_t*)(s_ykc_response_buff.general_transmit_buff);
                time_sync->head.sequence = g_ykc_sreq_time_sync.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_TIME_SYNC, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [计费模型设置响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SET_BILLING_MODEL, NULL) > 0){

                Net_YkcPro_PRes_BillingModel_Set_t *billing_model = (Net_YkcPro_PRes_BillingModel_Set_t*)(s_ykc_response_buff.general_transmit_buff);
                billing_model->head.sequence = g_ykc_sreq_billing_model_set.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_SET_BILLING_MODEL, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [遥控地锁升锁与降锁命令响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_GROUNDLOCK_LIFTING, NULL) > 0){

                Net_YkcPro_PRes_GroundLock_Lifting_t *ground_lock = (Net_YkcPro_PRes_GroundLock_Lifting_t*)(s_ykc_response_buff.general_transmit_buff);
                ground_lock->head.sequence = g_ykc_sreq_ground_lock_lifting[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_GROUNDLOCK_LIFTING, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [远程重启响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_REMOTE_REBOOT, NULL) > 0){

                Net_YkcPro_PRes_RemoteReboot_t *reboot = (Net_YkcPro_PRes_RemoteReboot_t*)(s_ykc_response_buff.general_transmit_buff);
                reboot->head.sequence = g_ykc_sreq_remote_reboot.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_REMOTE_REBOOT, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [远程更新响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_REMOTE_UPDATE, NULL) > 0){

                g_ykc_pres_remote_update.head.sequence = g_ykc_sreq_remote_update.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_REMOTE_UPDATE, s_ykc_socket_info.fd, &g_ykc_pres_remote_update,
                        sizeof(g_ykc_pres_remote_update));
                rt_thread_mdelay(250);
            }
            /***** [运营平台远程控制并充启机响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SERVER_START_MERGECHARGE, NULL) > 0){

                Net_YkcPro_PRes_Remote_StartMergeCharge_t *merge_charge = (Net_YkcPro_PRes_Remote_StartMergeCharge_t*)(s_ykc_response_buff.general_transmit_buff);
                merge_charge->head.sequence = g_ykc_sreq_remote_start_merge_charge[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_SERVER_START_MERGECHARGE, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台二维码配置响应(国充)] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_QRCODE_CONFIG_GC, NULL) > 0){

                Net_YkcPro_PRes_Qrcode_Config_GC_t *qrcode = (Net_YkcPro_PRes_Qrcode_Config_GC_t*)(s_ykc_response_buff.general_transmit_buff);
                qrcode->head.sequence = g_ykc_sreq_qrcode_config_gc[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_QRCODE_CONFIG_GC, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台二维码配置响应(云快充1.5)] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_QRCODE_CONFIG_YKC15, NULL) > 0){

                Net_YkcPro_PRes_Qrcode_Config_Ykc15_t *qrcode = (Net_YkcPro_PRes_Qrcode_Config_Ykc15_t*)(s_ykc_response_buff.general_transmit_buff);
                qrcode->head.sequence = g_ykc_sreq_qrcode_config_ykc15.head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_QRCODE_CONFIG_YKC15, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台二维码配置响应(特来电)] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_QRCODE_CONFIG_TLD, NULL) > 0){

                Net_YkcPro_PRes_Qrcode_Config_Tld_t *qrcode = (Net_YkcPro_PRes_Qrcode_Config_Tld_t*)(s_ykc_response_buff.general_transmit_buff);
                qrcode->head.sequence = g_ykc_sreq_qrcode_config_tld[gunno].head.sequence;
                ykc_message_send_port(NETYKC_PRESCMD_QRCODE_CONFIG_TLD, s_ykc_socket_info.fd, s_ykc_response_buff.general_transmit_buff,
                        s_ykc_response_buff.length);
                ykc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
        }
        rt_thread_mdelay(100);
    }
}

static void net_ykc_server_message_pro_entry(void *parameter)
{
    int8_t result = 0x00;
    uint32_t _event = 0x00;
    ykc_response_message_buf_t *response = NULL;

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_OPEN_LINK) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        if(s_ykc_socket_info.state != YKC_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
            rt_thread_mdelay(500);
            continue;
        }

        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [读取实时数据请求] *****/ // OK
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_QUERY_REALTIME_DATA, NULL) > 0){
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    result = ykc_response_padding_query_realtime_data(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_READ_REALTIME_DATA);
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [运营平台远程控制启机请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_REMOTE_START_CHARGE, NULL) > 0){
                    uint8_t pro_result, reason;
                    result = NET_YKC_START_FAIL_REASON_IS_CHARGING;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    if(g_ykc_sreq_remote_start_charge[gunno].body.result == 0x00){
                        result = ykc_message_pro_remote_start_charge_request(gunno, &g_ykc_sreq_remote_start_charge[gunno], sizeof(g_ykc_sreq_remote_start_charge[gunno]));
                        if(result >= NET_YKC_START_FAIL_REASON_NO){
                            pro_result = 0x00;
                            if(result == NET_YKC_START_FAIL_REASON_NO){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_START_CHARGE);
                                net_operation_clear_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                                pro_result = 0x01;
                            }
                            reason = result;
                        }
                    }else{
                        pro_result = 0x00;
                        reason = NET_YKC_START_FAIL_REASON_PILE_NUMBER_NOT_MATCH;
                    }
                    if(result >= NET_YKC_START_FAIL_REASON_NO){
                        result = ykc_response_padding_remote_start_charge(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YkcPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.result = pro_result;
                        ((Net_YkcPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.fail_reason = reason;
                        if(result >= 0x00){
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SERVER_START_CHARGE);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [运营平台远程停机请求] *****/ // OK
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_REMOTE_STOP_CHARGE, NULL) > 0){
                    uint8_t pro_result, reason;
                    result = NET_YKC_STOP_FAIL_REASON_NO;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    if(g_ykc_sreq_remote_stop_charge[gunno].body.result == 0x00){
                        result = ykc_message_pro_remote_stop_charge_request(gunno);
                        if(result >= NET_YKC_STOP_FAIL_REASON_NO){
                            pro_result = 0x00;
                            if(result == NET_YKC_STOP_FAIL_REASON_NO){
                                pro_result = 0x01;
                            }
                            reason = result;
                        }
                    }else{
                        pro_result = 0x00;
                        reason = NET_YKC_STOP_FAIL_REASON_PILE_NUMBER_NOT_MATCH;
                    }
                    if(result >= NET_YKC_STOP_FAIL_REASON_NO){
                        result = ykc_response_padding_remote_stop_charge(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YkcPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.result = pro_result;
                        ((Net_YkcPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.fail_reason = reason;
                        if(result >= 0x00){
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SERVER_STOP_CHARGE);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [远程账户余额更新请求] *****/ // OK
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE, NULL) > 0){
                    result = 0x00;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    if(g_ykc_sreq_account_ballance_update[gunno].body.result == 0x00){
                        result = ykc_message_pro_account_ballance_update_request(gunno, &g_ykc_sreq_account_ballance_update[gunno], sizeof(g_ykc_sreq_account_ballance_update[gunno]));
                        if(result >= 0x00){
                            ((Net_YkcPro_PRes_AccountBallance_Update_t*)response->general_transmit_buff)->body.result = 0x02;
                            if(result == 0x00){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_BALLANCE_UPDATE);
                                ((Net_YkcPro_PRes_AccountBallance_Update_t*)response->general_transmit_buff)->body.result = 0x00;
                            }
                        }
                    }else{
                        ((Net_YkcPro_PRes_AccountBallance_Update_t*)response->general_transmit_buff)->body.result = 0x01;
                    }
                    if(result >= 0x00){
                        result = ykc_response_padding_account_ballance_update(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        if(result >= 0x00){
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_ACCOUNT_BALLANCE_UPDATE);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [离线卡数据同步请求] *****/ // OK
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_SYNC_OFFLINE_CARD, NULL) > 0){
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    struct net_handle* handle = net_get_net_handle();
                    uint8_t pro_result = 0x01, reason = 0x00;
                    ykc_card_vin_buf_t *list = (ykc_card_vin_buf_t*)(ykc_get_card_vin_whitelists_info());
                    uint8_t *card = list->whitlelist;

                    if(list->type == NET_YKC_WHITELIST_TYPE_CARD){
                        list->flag.is_used = 0x00;

                        if(g_ykc_sreq_sync_offline_card.body.result == 0x00){
                            for(uint8_t i = 0x00; i < g_ykc_sreq_sync_offline_card.body.count; i++){
#if 0
                                if(handle->card_vin_whitelists_set(card, NET_YKC_CARD_NUMBER_LENGTH_MAX, NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST) < 0x00){
                                    pro_result = 0x00;
                                    reason = 0x02;
                                    break;
                                }
#endif /* 0 */
                                if(handle->card_vin_whitelists_set((card + NET_YKC_CARD_NUMBER_LENGTH_MAX), NET_YKC_CARD_NUMBER_LENGTH_MAX, \
                                        NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST) < 0x00){
                                    pro_result = 0x00;
                                    reason = 0x02;
                                    break;
                                }
                                if(i < (g_ykc_sreq_sync_offline_card.body.count - 0x01)){
                                    card += (2 *NET_YKC_CARD_NUMBER_LENGTH_MAX);
                                }
                            }
                            if(handle->system_data_storage(NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST) < 0x00){
                                pro_result = 0x00;
                                reason = 0x02;
                            }
#if 0
                            if(handle->system_data_storage(NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST) < 0x00){
                                pro_result = 0x00;
                                reason = 0x02;
                            }
#endif /* 0 */
                        }else if(g_ykc_sreq_sync_offline_card.body.result == 0x01){
                            pro_result = 0x00;
                            reason = 0x02;
                        }else if(g_ykc_sreq_sync_offline_card.body.result == 0x01){
                            pro_result = 0x00;
                            reason = 0x02;
                        }else{
                            pro_result = 0x00;
                            reason = 0x02;
                        }

                        result = ykc_response_padding_sync_offline_card(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        if(result >= 0x00){
                            ((Net_YkcPro_PRes_Sync_OfflineCard_t*)response->general_transmit_buff)->body.result = pro_result;
                            ((Net_YkcPro_PRes_Sync_OfflineCard_t*)response->general_transmit_buff)->body.fail_reason = reason;
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SYNC_OFFLINECARD);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [离线卡数据清除请求] *****/ // OK
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_CLEAR_OFFLINE_CARD, NULL) > 0){
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    uint8_t count = g_ykc_sreq_clear_offline_card.body.count;
                    struct net_handle* handle = net_get_net_handle();
                    ykc_card_vin_buf_t *list = (ykc_card_vin_buf_t*)(ykc_get_card_vin_whitelists_info());
                    uint8_t *card = list->whitlelist;

                    if(list->type == NET_YKC_WHITELIST_TYPE_CARD){
                        list->flag.is_used = 0x00;

                        if((sizeof(Net_YkcPro_PRes_Clear_OfflineCard_t) + sizeof(struct clear_card_block) *count) <= NET_YKC_GENERA_RESPONSE_BUFF_LENGTH){
                            result = ykc_response_padding_clear_offline_card(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                            if(result >= 0x00){
                                struct clear_card_block *info = (struct clear_card_block*)((uint8_t*)(((Net_YkcPro_PRes_Clear_OfflineCard_t*)response->general_transmit_buff)->body.pile_number) \
                                        + NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
                                for(uint8_t i = 0x00; i < count; i++){
                                    memcpy(info[i].physics_card_number, card, NET_YKC_CARD_NUMBER_LENGTH_MAX);
#if 0
                                    if(handle->card_vin_whitelists_delete(card, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX,  \
                                            (NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST |NET_SYSTEM_DATA_OPTION_CARD_NUMBER_JOINT)) < 0x00){
#else
                                    if(handle->card_vin_whitelists_delete(card, NET_YKC_CARD_NUMBER_LENGTH_MAX, NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST) < 0x00){
#endif /* 0 */
                                        info[i].result = 0x00;
                                        info[i].fail_reason = 0x01;
                                    }else{
                                        info[i].result = 0x01;
                                        info[i].fail_reason = 0x02;
                                    }
                                    if(i < (count - 0x01)){
                                        card += NET_YKC_CARD_NUMBER_LENGTH_MAX;
                                    }
                                }
                                if(handle->system_data_storage(NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST) < 0x00){
                                    for(uint8_t i = 0x00; i < count; i++){
                                        info[i].result = 0x00;
                                        info[i].fail_reason = 0x01;
                                    }
                                }
#if 0
                                if(handle->system_data_storage(NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST) < 0x00){
                                    for(uint8_t i = 0x00; i < count; i++){
                                        info[i].result = 0x00;
                                        info[i].fail_reason = 0x01;
                                    }
                                }
#endif /* 0 */
                                response->length += (sizeof(struct clear_card_block) *count);
                                ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_CLEAR_OFFLINECARD);
                            }else{
                                ykc_response_buff_release_sem();
                            }
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }
                }
                /***** [离线卡数据查询请求] *****/ // OK
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_QUERY_OFFLINE_CARD, NULL) > 0){
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    uint8_t count = g_ykc_sreq_query_offline_card.body.count;
                    struct net_handle* handle = net_get_net_handle();
                    ykc_card_vin_buf_t *list = (ykc_card_vin_buf_t*)(ykc_get_card_vin_whitelists_info());
                    uint8_t *card = list->whitlelist;

                    if(list->type == NET_YKC_WHITELIST_TYPE_CARD){
                        list->flag.is_used = 0x00;

                        if((sizeof(Net_YkcPro_PRes_Query_OfflineCard_t) + sizeof(struct query_card_block) *count) <= NET_YKC_GENERA_RESPONSE_BUFF_LENGTH){
                            result = ykc_response_padding_query_offline_card(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                            if(result >= 0x00){
                                struct query_card_block *info = (struct query_card_block*)((uint8_t*)(((Net_YkcPro_PRes_Query_OfflineCard_t*)response->general_transmit_buff)->body.pile_number) \
                                        + NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
                                for(uint8_t i = 0x00; i < count; i++){
                                    memcpy(info[i].physics_card_number, card, NET_YKC_CARD_NUMBER_LENGTH_MAX);
                                    if(handle->card_vin_whitelists_query(card, NET_YKC_CARD_NUMBER_LENGTH_MAX, NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST) < 0x00){
                                        info[i].result = 0x00;
                                    }else{
                                        info[i].result = 0x01;
                                    }
                                    if(i < (count - 0x01)){
                                        card += NET_YKC_CARD_NUMBER_LENGTH_MAX;
                                    }
                                }
                                response->length += (sizeof(struct query_card_block) *count);
                                ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_QUERY_OFFLINECARD);
                            }else{
                                ykc_response_buff_release_sem();
                            }
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }
                }
                /***** [充电桩工作参数设置请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_SET_WORK_PARA, NULL) > 0){
                    if(ykc_message_pro_set_work_para_request(&g_ykc_sreq_set_work_para, sizeof(g_ykc_sreq_set_work_para)) < 0x00){
                        response = ykc_get_response_buff(RT_WAITING_FOREVER);
                        result = ykc_response_padding_set_work_para(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        if(result >= 0x00){
                            ((Net_YkcPro_PRes_Set_WorkPara_t*)response->general_transmit_buff)->body.result = 0x00;
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SET_WORK_PARA);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }else{
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER);
                        if(g_ykc_sreq_set_work_para.body.forbidden == 0x01){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_FORBIDDEN_USE);
                        }
                    }
                }
                /***** [对时设置请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_TIME_SYNC, NULL) > 0){
                    ykc_message_pro_time_sync_request(&g_ykc_sreq_time_sync, sizeof(g_ykc_sreq_time_sync));
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    result = ykc_response_padding_time_sync(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_TIME_SYNC);
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [计费模型设置请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_BILLING_MODEL_SET, NULL) > 0){
                    uint8_t res = 0x00;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);
                    if(ykc_message_pro_billing_model_set_response(&g_ykc_sreq_billing_model_set, sizeof(g_ykc_sreq_billing_model_set)) < 0x00){
                        res = 0x00;
                    }else{
                        res = 0x01;
                    }
                    result = ykc_response_padding_set_billing_model(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    ((Net_YkcPro_PRes_BillingModel_Set_t*)response->general_transmit_buff)->body.result = res;
                    if(result >= 0x00){
                        ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SET_BILLING_MODEL);
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [遥控地锁升锁与降锁命令请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_GROUND_LOCK_LIFTING, NULL) > 0){
                    if(g_ykc_sreq_ground_lock_lifting[gunno].body.control_cmd == 0x55){
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_GROUND_RISE);
                    }else if(g_ykc_sreq_ground_lock_lifting[gunno].body.control_cmd == 0xFF){
                        net_operation_set_event(gunno, NET_OPERATION_EVENT_GROUND_DROP);
                    }else{
                        response = ykc_get_response_buff(RT_WAITING_FOREVER);
                        result = ykc_response_padding_ground_lock_lifting(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        if(result >= 0x00){
                            ((Net_YkcPro_PRes_Set_WorkPara_t*)response->general_transmit_buff)->body.result = 0x00;
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_GROUNDLOCK_LIFTING);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }
                }
                /***** [远程重启请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_REMOTE_REBOOT, NULL) > 0){
                    uint8_t res = 0x00;
                    result = ykc_message_pro_remote_reset_request(&g_ykc_sreq_remote_reboot, sizeof(g_ykc_sreq_remote_reboot));
                    if(result > 0x00){
                        res = 0x01;
                    }
                    if(result >= 0x00){
                        response = ykc_get_response_buff(RT_WAITING_FOREVER);
                        result = ykc_response_padding_remote_reboot(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        if(result >= 0x00){
                            net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                            ((Net_YkcPro_PRes_RemoteReboot_t*)response->general_transmit_buff)->body.result = res;
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_REMOTE_REBOOT);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }
                }
                /***** [远程更新请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_REMOTE_UPDATE, NULL) > 0){
                    uint8_t pro_result;
                    if(g_ykc_sreq_remote_update.body.result == 0x00){
                        if(net_get_ota_info()->state == NET_OTA_STATE_NULL){
                            if(ykc_get_ota_was_requested_flag() == 0x00){
                                ykc_set_ota_was_requested_flag();
                                pro_result = 0x00;
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_START_UPDATE);
                            }else{
                                pro_result = 0x03;
                            }
                        }else{
                            pro_result = 0x03;
                        }
                    }else{
                        pro_result = 0x01;
                    }
                    if(pro_result != 0x00){
                        response = ykc_get_response_buff(RT_WAITING_FOREVER);
                        result = ykc_response_padding_remote_update(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YkcPro_PRes_RemoteUpdate_t*)response->general_transmit_buff)->body.result = pro_result;
                        if(result >= 0x00){
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_REMOTE_UPDATE);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }
                }
                /***** [运营平台远程控制并充启机请求] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_REMOTE_START_MERGE_CHARGE, NULL) > 0){
                    uint8_t pro_result = 0x00, reason = 0x00;
                    result = NET_YKC_START_FAIL_REASON_NO;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);

                    if(g_ykc_sreq_remote_start_merge_charge[gunno].body.result == 0x00){
                        result = ykc_message_pro_remote_start_merge_charge_request(gunno, &g_ykc_sreq_remote_start_merge_charge[gunno], sizeof(g_ykc_sreq_remote_start_merge_charge[gunno]));
                        if(result >= NET_YKC_START_FAIL_REASON_NO){
                            pro_result = 0x00;
                            if(result == NET_YKC_START_FAIL_REASON_NO){
                                pro_result = 0x01;
                            }
                            reason = result;
                        }
                    }else{
                        pro_result = 0x00;
                        reason = NET_YKC_START_FAIL_REASON_PILE_NUMBER_NOT_MATCH;
                    }

                    if(result >= NET_YKC_START_FAIL_REASON_NO){
                        result = ykc_response_padding_remote_start_merge_charge(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YkcPro_PRes_Remote_StartMergeCharge_t*)response->general_transmit_buff)->body.result = pro_result;
                        ((Net_YkcPro_PRes_Remote_StartMergeCharge_t*)response->general_transmit_buff)->body.fail_reason = reason;
                        if(result >= 0x00){
                            ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SERVER_START_MERGECHARGE);
                        }else{
                            ykc_response_buff_release_sem();
                        }
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [运营平台二维码配置请求(国充)] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC, NULL) > 0){
                    uint8_t pro_result = 0x00;
                    ykc_qrcode_buf_t *info = (ykc_qrcode_buf_t*)(ykc_get_qrcode_info());

                    info->flag.is_used = 0x00;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);

                    if(g_ykc_sreq_qrcode_config_gc[gunno].body.result == 0x00){
                        uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                        struct net_handle* handle = net_get_net_handle();

                        if(handle->set_system_data(NET_SYSTEM_DATA_NAME_QRCODE, info->qrcode, info->length, option) >= 0x00){
                            pro_result = 0x01;
                        }
                    }

                    result = ykc_response_padding_qrcode_config_gc(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    ((Net_YkcPro_PRes_Qrcode_Config_GC_t*)response->general_transmit_buff)->body.result = pro_result;
                    if(result >= 0x00){
                        ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_QRCODE_CONFIG_GC);
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [运营平台二维码配置请求(云快充)] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15, NULL) > 0){
                    uint8_t pro_result = 0x01;
                    ykc_qrcode_buf_t *info = (ykc_qrcode_buf_t*)(ykc_get_qrcode_info());

                    info->flag.is_used = 0x00;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);

                    if(g_ykc_sreq_qrcode_config_ykc15.body.result == 0x00){
                        uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                        struct net_handle* handle = net_get_net_handle();

                        if(handle->set_system_data(NET_SYSTEM_DATA_NAME_QRCODE, info->qrcode, info->length, option) >= 0x00){
                            pro_result = 0x00;
                        }
                    }

                    result = ykc_response_padding_qrcode_config_ykc15(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    ((Net_YkcPro_PRes_Qrcode_Config_Ykc15_t*)response->general_transmit_buff)->body.result = pro_result;
                    if(result >= 0x00){
                        ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_QRCODE_CONFIG_YKC15);
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
                /***** [运营平台二维码配置请求(特来电)] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_REQUEST, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SREQ_EVENT_QRCODE_CONFIG_TLD, NULL) > 0){
                    uint8_t pro_result = 0x00;
                    ykc_qrcode_buf_t *info = (ykc_qrcode_buf_t*)(ykc_get_qrcode_info());

                    info->flag.is_used = 0x00;
                    response = ykc_get_response_buff(RT_WAITING_FOREVER);

                    if(g_ykc_sreq_qrcode_config_tld[gunno].body.result == 0x00){
                        uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                        struct net_handle* handle = net_get_net_handle();

                        if(handle->set_system_data(NET_SYSTEM_DATA_NAME_QRCODE, info->qrcode, info->length, option) >= 0x00){
                            pro_result = 0x01;
                        }
                    }

                    result = ykc_response_padding_qrcode_config_tld(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    ((Net_YkcPro_PRes_Qrcode_Config_Tld_t*)response->general_transmit_buff)->body.result = pro_result;
                    if(result >= 0x00){
                        ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_QRCODE_CONFIG_TLD);
                    }else{
                        ykc_response_buff_release_sem();
                    }
                }
            }

            ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [计费模型请求响应] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST, NULL) > 0){
                    ykc_message_pro_billing_model_set_response(&g_ykc_sres_billing_model_request, sizeof(g_ykc_sres_billing_model_request));
                }
                /***** [充电桩主动申请启动充电响应] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SRES_EVENT_APPLY_CHARGE_ACTIVE, NULL) > 0){
                    if(g_ykc_sres_apply_charge_active[gunno].body.authentication_success == 0x01){
                        ykc_message_pro_apply_charge_active_response(gunno, &g_ykc_sres_apply_charge_active[gunno], sizeof(g_ykc_sres_apply_charge_active[gunno]));
                        if(g_ykc_preq_apply_charge_active[gunno].body.start_type == 0x01){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_SUCCESS);
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS);
                        }
                    }else{
                        if(g_ykc_preq_apply_charge_active[gunno].body.start_type == 0x01){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                        }
                        LOG_W("ykc chargepile apply charge active fail reason(%d)", g_ykc_sres_apply_charge_active[gunno].body.fail_reason);
                    }
                    net_operation_clear_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                }
                /***** [交易记录响应响应] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SRES_EVENT_BILL_VERIFY, NULL) > 0){
                    ykc_set_transaction_verify_state(gunno, 0x01);
                }
                /***** [充电桩主动申请并充充电响应] *****/
                if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_SERVER, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE, NULL) > 0){
                    if(g_ykc_sres_apply_merge_charge_active[gunno].body.authentication_success == 0x01){
                        ykc_message_pro_apply_merge_charge_active_response(gunno, &g_ykc_sres_apply_merge_charge_active, sizeof(g_ykc_sres_apply_merge_charge_active));
                        if(g_ykc_preq_apply_merge_charge_active[gunno].body.start_type == 0x01){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_MERGE_SUCCESS);
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_MERGE_SUCCESS);
                        }
                        LOG_D("ykc merge charge sn");
                        for(uint8_t count = 0x00; count < sizeof(g_ykc_sres_apply_merge_charge_active[gunno].body.merge_charge_sn); count++){
                            rt_kprintf("0x%X ", g_ykc_sres_apply_merge_charge_active[gunno].body.merge_charge_sn[count]);
                        }
                        rt_kprintf("\n");
                    }else{
                        if(g_ykc_preq_apply_merge_charge_active[gunno].body.start_type == 0x01){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_MERGE_FAIL);
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_MERGE_FAIL);
                        }
                        LOG_W("ykc chargepile apply merge charge active fail reason(%d)", g_ykc_sres_apply_charge_active[gunno].body.fail_reason);
                    }
                    net_operation_clear_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                }
            }
            /***** [启动充电异步响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = ykc_get_response_buff(RT_WAITING_FOREVER);
                result = ykc_response_padding_remote_start_charge(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= NET_YKC_START_FAIL_REASON_NO){
                    if(ykc_is_start_charge_success(gunno)){
                        ((Net_YkcPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.result = 0x01;
                    }else{
                        ((Net_YkcPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.result = 0x00;
                    }
                    ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SERVER_START_CHARGE);
                }else{
                    ykc_response_buff_release_sem();
                }
            }
            /***** [停止充电异步响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = ykc_get_response_buff(RT_WAITING_FOREVER);
                result = ykc_response_padding_remote_stop_charge(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= NET_YKC_START_FAIL_REASON_NO){
                    if(ykc_is_stop_charge_success(gunno)){
                        ((Net_YkcPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.result = 0x01;
                    }else{
                        ((Net_YkcPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.result = 0x00;
                    }
                    ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SERVER_STOP_CHARGE);
                }else{
                    ykc_response_buff_release_sem();
                }
            }
            /***** [启动充电(并充)异步响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_START_MERGECHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = ykc_get_response_buff(RT_WAITING_FOREVER);
                result = ykc_response_padding_remote_start_merge_charge(gunno, response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= NET_YKC_START_FAIL_REASON_NO){
                    if(ykc_is_start_mergecharge_success(gunno)){
                        ((Net_YkcPro_PRes_Remote_StartMergeCharge_t*)response->general_transmit_buff)->body.result = 0x01;
                    }else{
                        ((Net_YkcPro_PRes_Remote_StartMergeCharge_t*)response->general_transmit_buff)->body.result = 0x00;
                    }
                    ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SERVER_START_MERGECHARGE);
                }else{
                    ykc_response_buff_release_sem();
                }
            }
            /***** [设置功率百分比异步响应] *****/
            if(ykc_net_event_receive(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YKC_EVENT_OPTION_OR |NET_YKC_EVENT_OPTION_CLEAR), NET_YKC_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY, NULL) > 0){
                response = ykc_get_response_buff(RT_WAITING_FOREVER);
                result = ykc_response_padding_set_work_para(response->general_transmit_buff, NET_YKC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= NET_YKC_START_FAIL_REASON_NO){
                    if(ykc_is_set_power_success()){
                        ((Net_YkcPro_PRes_Set_WorkPara_t*)response->general_transmit_buff)->body.result = 0x01;
                    }else{
                        ((Net_YkcPro_PRes_Set_WorkPara_t*)response->general_transmit_buff)->body.result = 0x00;
                    }
                    ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_RESPONSE, gunno, NET_YKC_PRES_EVENT_SET_WORK_PARA);
                }else{
                    ykc_response_buff_release_sem();
                }
            }
        }
        rt_thread_mdelay(100);
    }
}

int32_t ykc_message_send_init(void)
{
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_same_transaction_report_count[gunno] = 0x00;
        s_ykc_transaction_verify[gunno] = 0x00;
    }

    if(rt_thread_init(&s_ykc_message_send_thread, "ykc_send", net_ykc_message_send_thread_entry, NULL,
            s_ykc_message_send_thread_stack, NET_YKC_MESSAGE_SEND_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ykc message send thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ykc_message_send_thread) != RT_EOK){
        LOG_E("ykc message send thread startup fail, please check");
        return -0x01;
    }

    if(rt_thread_init(&s_ykc_server_message_pro_thread, "ykc_server", net_ykc_server_message_pro_entry, NULL,
            s_ykc_server_message_pro_thread_stack, NET_YKC_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ykc server message process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ykc_server_message_pro_thread) != RT_EOK){
        LOG_E("ykc server message process thread startup fail, please check");
        return -0x01;
    }

    if(rt_sem_init(&s_ykc_response_buff_sem, "ykc_tbsem", 0x01, RT_IPC_FLAG_PRIO) != RT_EOK){
        LOG_E("ykc response buff sem create fail");
    }
    return 0x00;
}

#endif /* NET_PACK_USING_YKC */


