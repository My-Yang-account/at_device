/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */
#include "sgcc_message_send.h"
#include "sgcc_message_receive.h"
#include "sgcc_message_padding.h"
#include "sgcc_device_register.h"

#include "interface.h"

#include "net_operation.h"
#include "protocol.h"

#define DBG_TAG "sgcc_send"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

#define NET_SGCC_HEARTBEAT_TIMEOUT_RENTRY                     3           /* 心跳超时次数 */
#define NET_SGCC_OPEN_SOCKET_RENTRY                           5           /* 打开socket尝试次数 */
#define NET_SGCC_LOGIN_RENTRY                                 5           /* 登录尝试次数 */
#define NET_SGCC_WAIT_LOGIN_RENTRY                            100         /* 等待登录结果尝试次数 */
#define NET_SGCC_WAIT_UNLOCK_TIMEOUT                          (10 *1000)  /* 等待socket 解锁超时时间(单位：ms) */

#define NET_SGCC_LOGIN_OPERATION_INTERVAL                     30000       /* 登录操作间隔(单位ms:30 *1000 = 30s) */
#define NET_SGCC_HEARTBEAT_REPORT_INTERVAL                    10000       /* 心跳间隔(单位ms:10 *1000 = 10s) */

#define NET_SGCC_REALTIME_DATA_IDLE_INTERVAL                  300000      /* 实时数据空闲上报间隔(单位ms:5 *60 *1000 = 5min) */
#define NET_SGCC_REALTIME_DATA_CHARGING_INTERVAL              15000       /* 实时数据充电中上报间隔(单位ms:15 *1000 = 15s) */
#define NET_SGCC_REALTIME_DATA_LINK_INTERVAL                  5000        /* 实时数据刚连上网时上报间隔(单位ms:5 *1000 = 5s) */
#define NET_SGCC_SAME_TRANSATION_REPORT_COUNT_MAX             10          /* 相同订单最大上报次数 */

/** 标志集 */
struct sgcc_flag_set{
    uint8_t disconnect : 1;            /** 断网或刚开始连上网 */
    uint8_t start_responsed : 1;       /** 启动充电已异步响应 */
    uint8_t stop_responsed : 1;        /** 停止充电已异步响应 */
    uint8_t is_time_sync;              /** 已进行时间同步 */
    uint8_t socket_lock;               /** socket已锁 */
};

struct sgcc_wait_response{
    uint32_t message_wait_response_state[NET_SYSTEM_GUN_NUMBER];                        /* 报文已发送发送，等待响应状态 */
    uint32_t message_repeat_time[NET_SYSTEM_GUN_NUMBER][NET_SGCC_CHARGEPILE_PREQ_NUM];  /* 报文重发计时 */
};

static uint32_t s_sgcc_time_sync_count = 0x00;
static struct sgcc_flag_set s_sgcc_flag_set;
static uint32_t s_sgcc_chargepile_event[NET_SGCC_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];
static uint32_t s_sgcc_server_event[NET_SGCC_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];

static uint8_t s_sgcc_same_transaction_report_count[NET_SYSTEM_GUN_NUMBER];
static uint8_t s_sgcc_transaction_verify[NET_SYSTEM_GUN_NUMBER];
static struct sgcc_wait_response s_sgcc_wait_response;
static uint32_t s_sgcc_message_send_state[NET_SYSTEM_GUN_NUMBER];                       /* 报文发送状态 */

static struct rt_thread s_sgcc_message_send_thread;
static uint8_t s_sgcc_message_send_thread_stack[NET_SGCC_MESSAGE_SEND_THREAD_STACK_SIZE];
static struct rt_thread s_sgcc_message_server_thread;
static uint8_t s_sgcc_message_service_thread_stack[NET_SGCC_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE];
static struct rt_thread s_sgcc_connect_thread;
static uint8_t s_sgcc_connect_thread_stack[NET_SGCC_CONNECT_THREAD_STACK_SIZE];
static struct rt_thread s_sgcc_yield_thread;
static uint8_t s_sgcc_yield_thread_stack[NET_SGCC_YIELD_THREAD_STACK_SIZE];

static sgcc_response_message_buf_t s_sgcc_response_buff;
static struct rt_semaphore s_sgcc_response_buff_sem;
static sgcc_socket_info_t s_sgcc_socket_info;

/**=======================================[充电桩公共请求报文]=======================================*/
///设备固件信息
evs_event_fireware_info evs_event_firmware_infos;
///设备版本信息
evs_event_ver_info evs_event_ver_infos;
/////设备组件信息
//evs_event_devmdu_info evs_event_devmdu_infos[NET_SYSTEM_GUN_NUMBER];
/////智能卡信息
//evs_event_card_info evs_event_card_infos[NET_SYSTEM_GUN_NUMBER];
/////智能卡鉴权
//evs_event_card_auth evs_event_card_auths[NET_SYSTEM_GUN_NUMBER];
/////智能卡鉴权结果
//evs_event_card_auth_result evs_event_card_auth_results[NET_SYSTEM_GUN_NUMBER];
///计费模型请求
evs_event_ask_feeModel evs_event_ask_feeModels[NET_SYSTEM_GUN_NUMBER];
///启动充电结果
evs_event_startResult evs_event_startResults[NET_SYSTEM_GUN_NUMBER];
///启动充电鉴权
evs_event_startCharge evs_event_startCharges[NET_SYSTEM_GUN_NUMBER];
///停止充电结果
evs_event_stopCharge evs_event_stopCharges[NET_SYSTEM_GUN_NUMBER];
///交易记录
evs_event_tradeInfo evs_event_tradeInfos[NET_SYSTEM_GUN_NUMBER];
///故障告警
evs_event_alarm evs_event_alarms[NET_SYSTEM_GUN_NUMBER];
///地锁状态变化
evs_event_groundLock_change evs_event_groundLock_changes[NET_SYSTEM_GUN_NUMBER];
///智能门锁状态变化
evs_event_gateLock_change evs_event_gateLock_changes[NET_SYSTEM_GUN_NUMBER];
///充电枪状态变化
evs_event_pile_stutus_change evs_event_pile_stutus_changes[NET_SYSTEM_GUN_NUMBER];
///车辆信息
evs_event_car_info evs_event_car_infos[NET_SYSTEM_GUN_NUMBER];
/////VIN码白名单查询结果
//evs_event_vinList_result evs_event_vinList_results[NET_SYSTEM_GUN_NUMBER];
/////时间同步结果
//evs_event_time_sync_result evs_event_time_sync_results[NET_SYSTEM_GUN_NUMBER];
/////充电中的连接状态
//evs_event_charge_connect_change evs_event_charge_connect_changes[NET_SYSTEM_GUN_NUMBER];
/////卡异常检测信息
//evs_event_card_check_error evs_event_card_check_errors[NET_SYSTEM_GUN_NUMBER];
/////蓝牙信息
//evs_event_ble_plug_charge_info evs_event_ble_plug_charge_infos[NET_SYSTEM_GUN_NUMBER];
/////蓝牙连接状态变化
//evs_event_ble_conn_change evs_event_ble_conn_changes[NET_SYSTEM_GUN_NUMBER];
///日志查询结果
evs_event_logQuery_Result evs_event_logQuery_Results[NET_SYSTEM_GUN_NUMBER];
/////智能枪参数信息
//evs_smart_gun_auth_param evs_smart_gun_auth_params[NET_SYSTEM_GUN_NUMBER];
///输出电表底值
evs_property_meter evs_property_meters[NET_SYSTEM_GUN_NUMBER];

#ifdef NET_SGCC_PRO_USING_DC
/**=======================================[直流充电桩请求报文]=======================================*/
//evs_event_ccu_info evs_event_ccu_infos[NET_SYSTEM_GUN_NUMBER];
//evs_event_pcu_info evs_event_pcu_infos[NET_SYSTEM_GUN_NUMBER];
//evs_event_cu_info evs_event_cu_infos[NET_SYSTEM_GUN_NUMBER];
//evs_event_switch_info evs_event_switch_infos[NET_SYSTEM_GUN_NUMBER];
//evs_event_edas_info evs_event_edas_infos[NET_SYSTEM_GUN_NUMBER];
//evs_event_cu_work_info evs_event_cu_work_infos[NET_SYSTEM_GUN_NUMBER];

evs_property_dcPile evs_property_dcPiles;
evs_property_BMS evs_property_BMSs[NET_SYSTEM_GUN_NUMBER];
evs_property_dc_work evs_property_dc_works[NET_SYSTEM_GUN_NUMBER];
evs_property_dc_nonWork evs_property_dc_nonWorks[NET_SYSTEM_GUN_NUMBER];
///输入电表低值
evs_property_dc_input_meter evs_property_dc_input_meters[NET_SYSTEM_GUN_NUMBER];

#else
/**=======================================[交流充电桩请求报文]=======================================*/
evs_property_acPile evs_property_acPiles;
evs_property_ac_work evs_property_ac_works[NET_SYSTEM_GUN_NUMBER];
evs_property_ac_nonWork evs_property_ac_nonWorks[NET_SYSTEM_GUN_NUMBER];
#endif /* NET_SGCC_PRO_USING_DC */

/**************************************************************************
 * 函数名                 sgcc_get_socket_info
 * 功能                     获取socket信息
 * 说明
 * ***********************************************************************/
sgcc_socket_info_t* sgcc_get_socket_info(void)
{
    return &s_sgcc_socket_info;
}

/**************************************************************************
 * 函数名                 sgcc_response_buff_take_sem_forever
 * 功能                     获取响应缓存互斥量
 * 说明
 * ***********************************************************************/
static int32_t sgcc_response_buff_take_sem_forever(int32_t timeout)
{
    return rt_sem_take(&s_sgcc_response_buff_sem, timeout);
}
/**************************************************************************
 * 函数名                 sgcc_response_buff_release_sem
 * 功能                     释放响应缓存互斥信号量
 * 说明
 * ***********************************************************************/
static void sgcc_response_buff_release_sem(void)
{
    rt_sem_release(&s_sgcc_response_buff_sem);
}
/**************************************************************************
 * 函数名                 sgcc_get_response_buff
 * 功能                     获取响应缓存
 * 说明
 * ***********************************************************************/
static sgcc_response_message_buf_t* sgcc_get_response_buff(int32_t timeout)
{
    if(sgcc_response_buff_take_sem_forever(timeout) < 0){
        return NULL;
    }
    return &s_sgcc_response_buff;
}

/**************************************************************************
 * 函数名                 sgcc_transaction_is_verify
 * 功能                     查询交易是否已确认
 * 说明
 * ***********************************************************************/
uint8_t sgcc_transaction_is_verify(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_sgcc_transaction_verify[gunno];
}

/**************************************************************************
 * 函数名                 sgcc_set_transaction_verify_state
 * 功能                     设置交易确认状态
 * 说明
 * ***********************************************************************/
void sgcc_set_transaction_verify_state(uint8_t gunno, uint8_t state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_sgcc_transaction_verify[gunno] = 0x01;
    }else{
        s_sgcc_transaction_verify[gunno] = 0x00;
    }
}

/**************************************************************************
 * 函数名                 sgcc_net_event_send
 * 功能                     网络事件发送
 * 说明
 * ***********************************************************************/
int32_t sgcc_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event)
{
    if(event_handle >= NET_SGCC_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_SGCC_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }

    if(event_handle == NET_SGCC_EVENT_HANDLE_CHARGEPILE){
        s_sgcc_chargepile_event[event_type][gunno] |= (1 <<event);
    }else{
        s_sgcc_server_event[event_type][gunno] |= (1 <<event);
    }

    return 0;
}
/**************************************************************************
 * 函数名                 sgcc_net_event_receive
 * 功能                     网络事件接收
 * 说明                     若事件发生则对应位为 1，否则为0
 * ***********************************************************************/
int32_t sgcc_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf)
{
    if(event_handle >= NET_SGCC_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_SGCC_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }
    uint32_t (*event_set)[NET_SYSTEM_GUN_NUMBER] = NULL;

    if(event_handle == NET_SGCC_EVENT_HANDLE_CHARGEPILE){
        event_set = s_sgcc_chargepile_event;
    }else{
        event_set = s_sgcc_server_event;
    }

    if(event_buf){
        (*event_buf) = event_set[event_type][gunno];
    }

    if(event_set[event_type][gunno] &(1 <<event)){
        if(option &NET_SGCC_EVENT_OPTION_AND){
            if((event_set[event_type][gunno] &(1 <<event)) != (1 <<event)){
                return -4;
            }
        }
        if(option &NET_SGCC_EVENT_OPTION_CLEAR){
            event_set[event_type][gunno] &= (~(1 <<event));
        }
        return 1;
    }
    return 0;
}

/**************************************************************************
 * 函数名                 sgcc_get_message_send_state
 * 功能                     获取报文发送状态
 * 说明                     若报文正在发送则对应位为 1，否则为0
 * ***********************************************************************/
uint8_t sgcc_get_message_send_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_sgcc_message_send_state[gunno] &(1 <<message_bit)){
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 sgcc_set_message_send_state
 * 功能                     设置报文发送状态
 * 说明                     若报文正在发送则对应位置 1，否则置0
 * ***********************************************************************/
void sgcc_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_sgcc_message_send_state[gunno] |= (1 <<message_bit);
    }else{
        s_sgcc_message_send_state[gunno] &= (~(1 <<message_bit));
    }
}

/**************************************************************************
 * 函数名                 sgcc_set_message_wait_response_state
 * 功能                     设置报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
static void sgcc_set_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_SGCC_CHARGEPILE_PREQ_NUM){
        return;
    }

    s_sgcc_wait_response.message_wait_response_state[gunno] |= (1 <<message_bit);
    s_sgcc_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
}
/**************************************************************************
 * 函数名                 sgcc_clear_message_wait_response_state
 * 功能                     清除报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
void sgcc_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_SGCC_CHARGEPILE_PREQ_NUM){
        return;
    }

    s_sgcc_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
}
/**************************************************************************
 * 函数名                 sgcc_exist_message_wait_response
 * 功能                     检查是否存在已发送的报文等待响应
 * 说明
 * ***********************************************************************/
uint8_t sgcc_exist_message_wait_response(uint8_t gunno, uint32_t *state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_sgcc_wait_response.message_wait_response_state[gunno]){
        if(state){
            *state = s_sgcc_wait_response.message_wait_response_state[gunno];
        }
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 sgcc_get_message_wait_response_timeout_state
 * 功能                     获取报文等待响应超时状态
 * 说明
 * ***********************************************************************/
uint8_t sgcc_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(message_bit >= NET_SGCC_CHARGEPILE_PREQ_NUM){
        return 0x00;
    }
    if(s_sgcc_wait_response.message_wait_response_state[gunno] &(1 <<message_bit)){
        if(s_sgcc_wait_response.message_repeat_time[gunno][message_bit] > rt_tick_get()){
            s_sgcc_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
        }
        if((rt_tick_get() - s_sgcc_wait_response.message_repeat_time[gunno][message_bit]) > timeout){
            s_sgcc_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
            return 0x01;
        }
    }
    return 0x00;
}

/**************************************************************************
 * 函数名                 sgcc_ascii_to_bcd
 * 功能                     将字符码转成BCD
 * 说明
 * ***********************************************************************/
void sgcc_ascii_to_bcd(uint8_t *ascii, uint8_t alen, uint8_t *bcd, uint8_t blen, uint8_t is_order)
{
    uint8_t index, c;

    if(is_order){
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
    }else{
        ascii += (alen - 0x01);
        bcd += (blen - 0x01);
        if(alen %0x02){
            for(index = 0; index < (alen /0x02); index++) {
                c  = (*ascii--) & 0x0F;
                c |= (*ascii--) << 4;
                *bcd-- = c;
            }
            if(blen > (alen %0x02)){
                *bcd  = (*ascii) & 0x0F;
            }
        }else{
            for(index = 0; index < blen; index++) {
                c  = (*ascii--) & 0x0F;
                c |= (*ascii--) << 4;
                *bcd-- = c;
            }
        }
    }
}

static void sgcc_connect_thread_entry(void *parameter)
{
    uint8_t step = NET_SGCC_NET_STATE_OPEN_RESOURCE, is_power_on = 0x01, link_open_step = 0x00, gunno = 0x00;
    uint32_t delay = 0x00, wait_unlock;
    int32_t result = 0x00;

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        net_get_net_handle()->data_updata();
        if((net_get_net_handle()->net_fault) &NET_FAULT_PHYSICAL_LAYER){
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_PHY;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_PHY;
            s_sgcc_socket_info.fd = -0x01;
            step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
            link_open_step = 0x00;
            if(rt_tick_get() > (delay + NET_SGCC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_SGCC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_SIM_CARD){
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_SIM;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_SIM;
            s_sgcc_socket_info.fd = -0x01;
            step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
            link_open_step = 0x00;
            if(rt_tick_get() > (delay + NET_SGCC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_SGCC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_DATA_LINK_LAYER){
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_DATA_LINK;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_DATA_LINK;
            s_sgcc_socket_info.fd = -0x01;
            step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
            link_open_step = 0x00;
            if(rt_tick_get() > (delay + NET_SGCC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_SGCC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((net_get_net_handle()->net_fault) &NET_FAULT_MODULE_INIT){
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_MODULE_INIT;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_MODULE_INIT;
            s_sgcc_socket_info.fd = -0x01;
            step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
            link_open_step = 0x00;
            if(rt_tick_get() > (delay + NET_SGCC_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_SGCC_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }

        extern int get_at_device_appinfo_is_complete(void);
        while (1) {
            if (get_at_device_appinfo_is_complete()) {
//                LOG_D("net init complete \n");
                break;
            }else{
                LOG_D("net wait init... \n");

                rt_thread_mdelay(1000);
            }
        }

        if(link_open_step == 0x00){
            link_open_step = 0x01;
        }
        if(link_open_step == 0x01){
            LOG_D("sgcc linkit new");
            if((result = evs_linkkit_new(0x01, 0x00)) < 0x00){
                LOG_D("linkkit new failed, res code: %d \n", result);
                return RT_ERROR;  /** 是否需要 return,有待考虑*/
            }else{
                LOG_D("linkkit new success \n");
            }

            link_open_step = 0x02;
        }

        /***************************************************** [登录认证] **********************************************************/
        /***************************************************** [登录认证] **********************************************************/
        switch(step){
        case NET_SGCC_NET_STATE_OPEN_RESOURCE:
            if(delay > rt_tick_get()){
                delay = rt_tick_get();
            }
            if(((rt_tick_get() - delay) > NET_SGCC_LOGIN_OPERATION_INTERVAL) || is_power_on){
                LOG_D("sgcc open start");
                if(evs_mainopen() >= 0x00){
                    LOG_D("linkkit open resource success \n");
                    step = NET_SGCC_NET_STATE_LOGIN;
                }
                is_power_on = 0x00;
            }
            break;
        case NET_SGCC_NET_STATE_LOGIN:
        {
            uint8_t vaild_len = 0, rentry = 0, data[21], *sim_no = NULL;
            uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_SGCC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
            struct net_handle* handle = net_get_net_handle();

            memset(data, 0x00, 21);
            (void)(handle->get_system_data(NET_SYSTEM_DATA_NAME_ICCID, data, sizeof(data), option));

            sim_no = data;
            memset(evs_event_firmware_infos.simNo, '\0', sizeof(evs_event_firmware_infos.simNo));
            vaild_len = sizeof(evs_event_firmware_infos.simNo);
            vaild_len = vaild_len > (strlen((char*)sim_no))? strlen((char*)sim_no) : vaild_len;
            memcpy(evs_event_firmware_infos.simNo, sim_no, vaild_len);

            LOG_D("sgcc linkit connect start");
            result = evs_mainconnect();
            if(result >= 0x00){
                LOG_D("linkkit connect success \n");
                step = NET_SGCC_NET_STATE_MONITORING;
                s_sgcc_socket_info.operate_fail.login = 0x00;
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_PREQ_EVENT_REPORT_SDK_VERSION);
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_PREQ_EVENT_REPORT_FIRMWARE_INFO);
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, 0x00, NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE);
            }else{
                evs_mainclose();
                s_sgcc_socket_info.operate_fail.login++;
                delay = rt_tick_get();
                step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
                s_sgcc_socket_info.state = SGCC_SOCKET_STATE_OPEN;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_OPEN;
                LOG_D("linkkit connect failed num(%d)\n", s_sgcc_socket_info.operate_fail.login);
            }
            break;
        }
        case NET_SGCC_NET_STATE_MONITORING:
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_LOGIN_SUCCESS;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_LOGIN_SUCCESS;
            if(net_get_net_handle()->esocket_state == NET_ESOCKET_STATE_CLOSE){
                extern void iotx_linkkit_ctx_close(void);

                iotx_linkkit_ctx_close();

                delay = rt_tick_get();
                wait_unlock = rt_tick_get();
                step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
                s_sgcc_socket_info.state = SGCC_SOCKET_STATE_DATA_LINK;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_DATA_LINK;
                while(s_sgcc_flag_set.socket_lock == NET_ENUM_TRUE){
                    if((rt_tick_get() - wait_unlock) > NET_SGCC_WAIT_UNLOCK_TIMEOUT){
                        break;
                    }
                    rt_thread_mdelay(50);
                }

                evs_mainclose();

                s_sgcc_socket_info.fd = -0x01;
                s_sgcc_socket_info.operate_fail.login = 0x00;
                s_sgcc_socket_info.sync_repeat = 0x00;
                s_sgcc_socket_info.operate_fail.sync = NET_ENUM_FALSE;

                LOG_W("sgcc login rentry = 0x00");
                net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_ALL, 0x00);
            }
            break;
        default:
        {
            extern void iotx_linkkit_ctx_close(void);

            iotx_linkkit_ctx_close();

            delay = rt_tick_get();
            wait_unlock = rt_tick_get();
            step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_DATA_LINK;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_DATA_LINK;
            while(s_sgcc_flag_set.socket_lock == NET_ENUM_TRUE){
                if((rt_tick_get() - wait_unlock) > NET_SGCC_WAIT_UNLOCK_TIMEOUT){
                    break;
                }
                rt_thread_mdelay(50);
            }

            evs_mainclose();

            s_sgcc_socket_info.fd = -0x01;
            s_sgcc_socket_info.sync_repeat = 0x00;
            s_sgcc_socket_info.operate_fail.sync = NET_ENUM_FALSE;
        }
            break;
        }

        if(s_sgcc_socket_info.operate_fail.login > NET_SGCC_LOGIN_RENTRY){
            extern void iotx_linkkit_ctx_close(void);

            iotx_linkkit_ctx_close();

            delay = rt_tick_get();
            wait_unlock = rt_tick_get();
            step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
            s_sgcc_socket_info.state = SGCC_SOCKET_STATE_DATA_LINK;
            net_get_net_handle()->net_state = NET_SOCKET_STATE_DATA_LINK;
            while(s_sgcc_flag_set.socket_lock == NET_ENUM_TRUE){
                if((rt_tick_get() - wait_unlock) > NET_SGCC_WAIT_UNLOCK_TIMEOUT){
                    break;
                }
                rt_thread_mdelay(50);
            }

            evs_mainclose();

            s_sgcc_socket_info.fd = -0x01;
            s_sgcc_socket_info.operate_fail.login = 0x00;
            s_sgcc_socket_info.sync_repeat = 0x00;
            s_sgcc_socket_info.operate_fail.sync = NET_ENUM_FALSE;

            LOG_W("sgcc login rentry = 0x00");
            net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_ALL, 0x00);
        }

        if(s_sgcc_socket_info.state == SGCC_SOCKET_STATE_LOGIN_SUCCESS){
            if(s_sgcc_socket_info.operate_fail.sync == NET_ENUM_TRUE){      /* 相当于心跳超时 */
                extern void iotx_linkkit_ctx_close(void);

                iotx_linkkit_ctx_close();

                delay = rt_tick_get();
                wait_unlock = rt_tick_get();
                step = NET_SGCC_NET_STATE_OPEN_RESOURCE;
                s_sgcc_socket_info.state = SGCC_SOCKET_STATE_DATA_LINK;
                net_get_net_handle()->net_state = NET_SOCKET_STATE_DATA_LINK;
                while(s_sgcc_flag_set.socket_lock == NET_ENUM_TRUE){
                    if((rt_tick_get() - wait_unlock) > NET_SGCC_WAIT_UNLOCK_TIMEOUT){
                        break;
                    }
                    rt_thread_mdelay(50);
                }

                evs_mainclose();

                s_sgcc_socket_info.fd = -0x01;
                s_sgcc_socket_info.sync_repeat = 0x00;
                s_sgcc_socket_info.operate_fail.sync = NET_ENUM_FALSE;

                LOG_D("sgcc sync timeout");
            }
        }

        rt_thread_mdelay(100);
    }
}

static void sgcc_yield_thread_entry(void *parameter)
{
    int32_t result = 0x00;

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        if(s_sgcc_socket_info.state == SGCC_SOCKET_STATE_LOGIN_SUCCESS){
            LOG_D("sgcc main yied start");
            s_sgcc_flag_set.socket_lock = NET_ENUM_TRUE;
            result = evs_mainyield();
            s_sgcc_flag_set.socket_lock = NET_ENUM_FALSE;
            LOG_D("sgcc main yied stop(%d)", result);
        }

        rt_thread_mdelay(10);
    }
}

static void sgcc_message_send_thread_entry(void *parameter)
{
    s_sgcc_flag_set.disconnect = 0x00;

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        if(s_sgcc_socket_info.state != SGCC_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
            s_sgcc_flag_set.disconnect = 0x01;
            rt_thread_mdelay(1000);
            continue;
        }

        if(s_sgcc_flag_set.disconnect){
            s_sgcc_flag_set.disconnect = 0x00;
            s_sgcc_flag_set.is_time_sync = NET_ENUM_FALSE;
            s_sgcc_time_sync_count = rt_tick_get();
            rt_thread_mdelay(3000);
        }

        if(s_sgcc_time_sync_count > rt_tick_get()){
            s_sgcc_time_sync_count = rt_tick_get();
        }
        if(s_sgcc_flag_set.is_time_sync == NET_ENUM_FALSE){
            if((rt_tick_get() - s_sgcc_time_sync_count) > 5000){
                s_sgcc_time_sync_count = rt_tick_get();
                evs_linkkit_time_sync();

                s_sgcc_socket_info.sync_repeat++;
                if(s_sgcc_socket_info.sync_repeat >= 30){
                    s_sgcc_socket_info.sync_repeat = 30;
                    s_sgcc_socket_info.operate_fail.sync = NET_ENUM_TRUE;
                }
            }
        }else{
            if((rt_tick_get() - s_sgcc_time_sync_count) > 50000){
                s_sgcc_time_sync_count = rt_tick_get();
                evs_linkkit_time_sync();

                s_sgcc_socket_info.sync_repeat++;
                if(s_sgcc_socket_info.sync_repeat > 3){
                    s_sgcc_socket_info.sync_repeat = 3;
                    s_sgcc_socket_info.operate_fail.sync = NET_ENUM_TRUE;
                }
            }
        }

        if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, 0x00,
                (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_TIME_SYNC, NULL) > 0){
            s_sgcc_flag_set.is_time_sync = NET_ENUM_TRUE;
            s_sgcc_socket_info.sync_repeat = 0x00;
            s_sgcc_socket_info.operate_fail.sync = NET_ENUM_FALSE;
        }

        /***************************************************** [数据请求] **********************************************************/
        /***************************************************** [数据请求] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint32_t _event = 0;
            sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(!_event){
                continue;   /* 此枪没有请求事件,不进行事件查询 */
            }

            /***** [上送SDK版本信息] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_SDK_VERSION, NULL) > 0){
                rt_kprintf("SDK_VERSION 00000000000000000000000000(%d)\n", gunno);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_SDK_VERSION);
                evs_send_event(EVS_CMD_EVENT_VER_INFO, &evs_event_ver_infos);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_SDK_VERSION);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_SDK_VERSION);
                rt_thread_mdelay(250);
            }
            /***** [上送固件信息] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_FIRMWARE_INFO, NULL) > 0){
                rt_kprintf("FIRMWARE_INFO 00000000000000000000000000(%d)\n", gunno);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_FIRMWARE_INFO);
                evs_send_event(EVS_CMD_EVENT_FIREWARE_INFO, &evs_event_firmware_infos);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_FIRMWARE_INFO);
                rt_thread_mdelay(250);
            }
            /***** [请求计费模型] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE, NULL) > 0){
                rt_kprintf("BILLING_MODE 00000000000000000000000000(%d)\n", gunno);
                sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_BILLING_MODEL_SET, NULL);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE);
                evs_send_event(EVS_CMD_EVENT_ASK_FEEMODEL, &evs_event_ask_feeModels[gunno]);

                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_BILLING_MODEL_SET, NULL) <= 0){
//                    sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE);
                }else{
                    sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE);
                }

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE);
                rt_thread_mdelay(250);
            }
            /***** [充电枪状态变更] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED, NULL) > 0){
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED);
                rt_kprintf("GUNSTATE_CHANGED 00000000000000000000000000(%d)\n", gunno);
#ifdef NET_SGCC_PRO_USING_DC
                evs_send_event(EVS_CMD_EVENT_DCPILE_CHANGE, &evs_event_pile_stutus_changes[gunno]);
#else
                evs_send_event(EVS_CMD_EVENT_ACPILE_CHANGE, &evs_event_pile_stutus_changes[gunno]);
#endif /* NET_SGCC_PRO_USING_DC */
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED);
                rt_thread_mdelay(250);
            }
            /***** [上送实时数据(非充电中)] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING, NULL) > 0){
                rt_kprintf("STATE_DATA_NONCHARGING 00000000000000000000000000(%d)\n", gunno);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
#ifdef NET_SGCC_PRO_USING_DC
                evs_send_property(EVS_CMD_PROPERTY_DC_NONWORK, &evs_property_dc_nonWorks[gunno]);
#else
                evs_send_property(EVS_CMD_PROPERTY_AC_NONWORK, &evs_property_ac_nonWorks[gunno]);
#endif /* NET_SGCC_PRO_USING_DC */
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
                rt_thread_mdelay(250);
            }
            /***** [上送实时数据(充电中)] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING, NULL) > 0){
                rt_kprintf("STATE_DATA_CHARGING 00000000000000000000000000(%d)\n", gunno);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING);
#ifdef NET_SGCC_PRO_USING_DC
                evs_send_property(EVS_CMD_PROPERTY_DC_WORK, &evs_property_dc_works[gunno]);
#else
                evs_send_property(EVS_CMD_PROPERTY_AC_WORK, &evs_property_ac_works[gunno]);
#endif /* NET_SGCC_PRO_USING_DC */
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING);
                rt_thread_mdelay(250);
            }
            /***** [上送交易记录] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD, NULL) > 0){
                rt_kprintf("TRANSACTION_RECORD 00000000000000000000000000(%d)\n", gunno);
                sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SRES_EVENT_BILL_VERIFY, NULL);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
                evs_send_event(EVS_CMD_EVENT_TRADEINFO, &evs_event_tradeInfos[gunno]);

                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_SGCC_EVENT_OPTION_OR), NET_SGCC_SRES_EVENT_BILL_VERIFY, NULL) <= 0){
                    sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);

                    if(++s_sgcc_same_transaction_report_count[gunno] > NET_SGCC_SAME_TRANSATION_REPORT_COUNT_MAX){
                        sgcc_set_transaction_verify_state(gunno, 0x01);
                        s_sgcc_same_transaction_report_count[gunno] = 0x00;
                    }
                }else{
                    s_sgcc_same_transaction_report_count[gunno] = 0x00;
                    sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
                }

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
                rt_thread_mdelay(250);
            }
            /***** [上送电表底值] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE, NULL) > 0){
                rt_kprintf("AMMETER_VALUE 00000000000000000000000000(%d, %d, %d)[%s, %s]\n", gunno,
                        evs_property_meters[gunno].sumMeter, evs_property_meters[gunno].elec,
                        evs_property_meters[gunno].lastTrade, evs_property_meters[gunno].acqTime);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE);
                evs_send_property(EVS_CMD_PROPERTY_DC_OUTMETER, &evs_property_meters[gunno]);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE);
                rt_thread_mdelay(250);
            }
#ifdef NET_SGCC_PRO_USING_DC
            /***** [上送BMS数据] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA, NULL) > 0){
                rt_kprintf("BMS_DATA 00000000000000000000000000(%d)[%d, %d]\n", gunno, evs_property_BMSs[gunno].maxAllowCur,
                        evs_property_BMSs[gunno].socVal);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA);
                evs_send_property(EVS_CMD_PROPERTY_BMS, &evs_property_BMSs[gunno]);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA);
                rt_thread_mdelay(250);
            }
#endif /* NET_SGCC_PRO_USING_DC */
            /***** [上送实时监测属性] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY, NULL) > 0){
                rt_kprintf("MONITOR_PROPERTY 00000000000000000000000000(%d)\n", gunno);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY);
#ifdef NET_SGCC_PRO_USING_DC
                evs_send_property(EVS_CMD_PROPERTY_DCPILE, &evs_property_dcPiles);
#else
                evs_send_property(EVS_CMD_PROPERTY_ACPILE, &evs_property_acPiles);
#endif /* NET_SGCC_PRO_USING_DC */
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY);
                rt_thread_mdelay(250);
            }
            /***** [上送故障告警信息] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING, NULL) > 0){
                rt_kprintf("FAULT_WARNNING 00000000000000000000000000(%d)\n", gunno);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING);
                evs_send_event(EVS_CMD_EVENT_ALARM, &evs_event_alarms[gunno]);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING);
                rt_thread_mdelay(250);
            }
            /***** [启动鉴权请求] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE, NULL) > 0){
                rt_kprintf("APPLY_START_CHARGE 00000000000000000000000000(%d)\n", gunno);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);
                evs_send_event(EVS_CMD_EVENT_STARTCHARGE, &evs_event_startCharges[gunno]);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);
                rt_thread_mdelay(250);
            }
            /***** [上报日志数据请求] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_DEV_RECORD, NULL) > 0){
                rt_kprintf("REPORT_DEV_RECORD 00000000000000000000000000(%d)\n", gunno);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_DEV_RECORD);
                evs_send_event(EVS_CMD_EVENT_LOGQUERY_RESULT, &evs_event_logQuery_Results[gunno]);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_DEV_RECORD);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_DEV_RECORD);
                rt_thread_mdelay(250);
            }
            /***** [上报车辆信息请求] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PREQ_EVENT_REPORT_CAR_INFO, NULL) > 0){
                rt_kprintf("CAR_INFO 00000000000000000000000000(%d)\n", gunno);

                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_ONGOING, NET_SGCC_PREQ_EVENT_REPORT_CAR_INFO);
                evs_send_event(EVS_CMD_EVENT_CAR_INFO, &evs_event_car_infos[gunno]);
//                sgcc_set_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_CAR_INFO);
                sgcc_set_message_send_state(gunno, NET_SGCC_SEND_STATE_COMPLETE, NET_SGCC_PREQ_EVENT_REPORT_CAR_INFO);
                rt_thread_mdelay(250);
            }
        }

        /***************************************************** [数据响应] **********************************************************/
        /***************************************************** [数据响应] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint32_t _event = 0;
            sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);

            if(!_event){
                continue;   /* 此枪没有请求事件,不进行事件查询 */
            }
            /***** [平台启动充电响应] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PRES_EVENT_SERVER_START_CHARGE, NULL) > 0){
                rt_kprintf("START_CHARGE 00000000000000000000000000(%d)\n", gunno);
                evs_send_event(EVS_CMD_EVENT_STARTRESULT, s_sgcc_response_buff.general_transmit_buff);
                sgcc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [平台结束充电响应] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PRES_EVENT_SERVER_STOP_CHARGE, NULL) > 0){
                rt_kprintf("STOP_CHARGE 00000000000000000000000000(%d)\n", gunno);
                evs_send_event(EVS_CMD_EVENT_STOPCHARGE, s_sgcc_response_buff.general_transmit_buff);
                sgcc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [申请充电启动结果响应] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PRES_EVENT_APPLY_CHARGE_RESULT, NULL) > 0){
                rt_kprintf("APPLY_CHARGE_RESULT 00000000000000000000000000(%d)\n", gunno);
                evs_send_event(EVS_CMD_EVENT_STARTRESULT, s_sgcc_response_buff.general_transmit_buff);
                sgcc_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
        }

        rt_thread_mdelay(100);
    }
}

static void sgcc_message_server_thread_entry(void *parameter)
{
    int8_t result = 0x00;
    uint32_t _event = 0x00;
    sgcc_response_message_buf_t *response = NULL;

    while(1)
    {
        if(net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT){
            rt_thread_mdelay(5000);
            continue;
        }

        if(s_sgcc_socket_info.state != SGCC_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
            rt_thread_mdelay(500);
            continue;
        }

        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [[开始充电请求] *****/
                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_START_CHARGE, NULL) > 0){
                    uint8_t pro_result = NET_SGCC_START_RESULT_SUCCESS;
                    uint16_t reason = 0x00;
                    result = 0x00;

                    if(!((evs_service_startCharges[gunno].gunNo > 0x00) && (evs_service_startCharges[gunno].gunNo <= NET_SYSTEM_GUN_NUMBER))){
                        pro_result = NET_SGCC_START_RESULT_FAULTING;
#ifdef NET_SGCC_PRO_USING_DC
                        reason = NETSGCC_DCA_REASON3058_GUN_PORT;
#else
                        reason = NETSGCC_ACA_REASON3015_PORT_ABNORMAL;
#endif /* #ifdef NET_SGCC_PRO_USING_DC */
                    }
                    if(pro_result == NET_SGCC_START_RESULT_SUCCESS){
                        result = sgcc_message_pro_remote_start_charge_request(gunno, &evs_service_startCharges[gunno], sizeof(evs_service_startCharges[gunno]), &reason);
                        if(result >= 0x00){
                            pro_result = result;
                            if(result == NET_SGCC_START_RESULT_SUCCESS){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_START_CHARGE);
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                            }
                        }else{
                            pro_result = NET_SGCC_START_RESULT_FAULTING;
#ifdef NET_SGCC_PRO_USING_DC
                            reason = NETSGCC_DCA_REASON3058_GUN_PORT;
#else
                            reason = NETSGCC_ACA_REASON3015_PORT_ABNORMAL;
#endif /* #ifdef NET_SGCC_PRO_USING_DC */
                        }
                    }

                    if(pro_result != NET_SGCC_START_RESULT_SUCCESS){
                        response = sgcc_get_response_buff(RT_WAITING_FOREVER);

                        result = sgcc_response_padding_remote_start_charge(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((evs_event_startResult*)response->general_transmit_buff)->startResult = pro_result;
                        ((evs_event_startResult*)response->general_transmit_buff)->faultCode = reason;
                        if(result >= 0x00){
                            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_SERVER_START_CHARGE);
                        }else{
                            sgcc_response_buff_release_sem();
                        }
                    }
                }
                /***** [停止充电请求] *****/ // OK
                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_STOP_CHARGE, NULL) > 0){
                    uint8_t pro_result = 0x01, reason = 0x00;
                    result = 0x00;

                    if(!((evs_service_stopCharges[gunno].gunNo > 0x00) && (evs_service_stopCharges[gunno].gunNo <= NET_SYSTEM_GUN_NUMBER))){
                        pro_result = 0x00;
                        reason = 0x01;
                    }
                    if(pro_result == 0x01){
                        result = sgcc_message_pro_remote_stop_charge_request(gunno, &evs_service_stopCharges[gunno], sizeof(evs_service_stopCharges[gunno]));
                        pro_result = 0x00;
                        if(result >= 0x00){
                            if(result == 0x00){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
                                pro_result = 0x01;
                            }
                            reason = result;
                        }
                    }

                    if(pro_result != 0x01){
                        response = sgcc_get_response_buff(RT_WAITING_FOREVER);

                        result = sgcc_response_padding_remote_stop_charge(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((evs_event_stopCharge*)response->general_transmit_buff)->stopResult = 11;
                        ((evs_event_stopCharge*)response->general_transmit_buff)->resultCode = 0x00;
                        ((evs_event_stopCharge*)response->general_transmit_buff)->stopFailReson = 10;
                        if(result >= 0x00){
                            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_SERVER_STOP_CHARGE);
                        }else{
                            sgcc_response_buff_release_sem();
                        }
                    }
                }
                /***** [固件更新请求] *****/
                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_FIRMWARE_UPDATE, NULL) > 0){
                    uint8_t pro_result = 0x01;
                    if(net_get_ota_info()->state == NET_OTA_STATE_NULL){
                        if(sgcc_get_ota_was_requested_flag() == 0x00){
                            sgcc_set_ota_was_requested_flag();
                            pro_result = 0x00;
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_START_UPDATE);
                        }else{
                            pro_result = 0x03;
                        }
                    }else{
                        pro_result = 0x03;
                    }
#if 0
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
#endif
                }
//                /***** [电子锁控制请求] *****/ // OK
//                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_REQUEST, gunno,
//                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SREQ_EVENT_ELOCK_CONTROL, NULL) > 0){
//                    uint8_t pro_result = SGCC_ELOCK_CTRL_SUCCESS, state = SGCC_OPSCTL_ACTION;
//                    result = 0x00;
//
//                    if(sgcc_get_recv_message_item_result(EVS_CTRL_LOCK_SRV, gunno, &result) >= 0x00){
//                        sgcc_clear_recv_message_item(EVS_CTRL_LOCK_SRV, gunno, result);
//                    }else{
//                        pro_result = SGCC_ELOCK_CTRL_NO_ELOCK;
//                        LOG_W("sgcc there is no this message item(%d, %d)\n", gunno, EVS_CTRL_LOCK_SRV);
//                    }
//
//                    if(pro_result == SGCC_ELOCK_CTRL_SUCCESS){
//                        result = sgcc_message_pro_ctrl_elock_request(gunno, &evs_service_lockCtrls[gunno], sizeof(evs_service_lockCtrls[gunno]));
//                        if(result >= 0x00){
//                            if(result == 0x01){
//                                pro_result = SGCC_ELOCK_CTRL_FAIL_FORBID_UNLOCK;
//                                if(evs_service_lockCtrls[gunno].lockParam == SGCC_OPSCTL_ACTION){
//                                    pro_result = SGCC_ELOCK_CTRL_FAIL_FORBID_LOCK;
//                                }
//                            }else if(result == 0x02){
//                                state = SGCC_OPSCTL_SILENT;
//                            }else if(result == 0x03){
//                                state = SGCC_OPSCTL_ACTION;
//                            }else{
//                                state = SGCC_OPSCTL_SILENT;
//                                if(evs_service_lockCtrls[gunno].lockParam == SGCC_OPSCTL_ACTION){
//                                    state = SGCC_OPSCTL_ACTION;
//                                }
//                            }
//                        }
//                    }
//
//                    if(0x01){
//                        response = sgcc_get_response_buff(RT_WAITING_FOREVER);
//
//                        result = sgcc_response_padding_elock_ctrl_result(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
//                        ((evs_service_feedback_lockCtrl*)response->general_transmit_buff)->resCode = pro_result;
//                        ((evs_service_feedback_lockCtrl*)response->general_transmit_buff)->lockStatus = state;
//                        if(result >= 0x00){
//                            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_ELOCK_CTRL_RESULT);
//                        }else{
//                            sgcc_response_buff_release_sem();
//                        }
//                    }
//                }
            }

            sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [交易记录响应响应] *****/
                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SRES_EVENT_BILL_VERIFY, NULL) > 0){
                    sgcc_set_transaction_verify_state(gunno, 0x01);
                }
                /***** [[启动鉴权响应] *****/
                if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_SERVER, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                        (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_SRES_EVENT_APPLY_CHARGE_ACTIVE, NULL) > 0){
                    uint8_t pro_result = SGCC_OPSCTL_SILENT, reason = 0x00;
                    result = 0x01;

                    if(evs_service_authCharges[gunno].result == SGCC_OPSCTL_ACTION){
                        if(sgcc_get_recv_message_item_result(EVS_AUTH_RESULT_SRV, gunno, &result) >= 0x00){
                            sgcc_clear_recv_message_item(EVS_AUTH_RESULT_SRV, gunno, result);
                        }else{
                            result = 0x01;
                            if(evs_event_startCharges[gunno].startType == SGCC_OPSCTL_SILENT){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                            }
#if 0
                            if(evs_event_startCharges[gunno].startType == SGCC_OPSCTL_SILENT){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                            }else{
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
                            }

#endif
                            LOG_W("sgcc there is no this message item(%d, %d)\n", gunno, EVS_AUTH_RESULT_SRV);
                        }
                    }else{
                        if(evs_event_startCharges[gunno].startType == SGCC_OPSCTL_SILENT){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                        }
#if 0
                        if(evs_event_startCharges[gunno].startType == SGCC_OPSCTL_SILENT){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
                        }
#endif
                        LOG_W("sgcc chargepile apply charge fail(%d, %d)\n", gunno, evs_service_authCharges[gunno].result);
                    }

                    if(result == 0x00){
                        result = sgcc_message_pro_apply_charge_response(gunno, &evs_service_authCharges[gunno], sizeof(evs_service_authCharges[gunno]));
                        if(result >= 0x00){
                            if(result == 0x00){
                                if(evs_event_startCharges[gunno].startType == SGCC_OPSCTL_SILENT){
                                    net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS);
                                    net_operation_set_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                                    pro_result = SGCC_OPSCTL_ACTION;
                                }
                            }
                            reason = result;
                        }
                    }

                    if(pro_result != SGCC_OPSCTL_ACTION){
                        response = sgcc_get_response_buff(RT_WAITING_FOREVER);

                        result = sgcc_response_padding_apply_charge_result(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((evs_event_startResult*)response->general_transmit_buff)->startResult = pro_result;
                        ((evs_event_startResult*)response->general_transmit_buff)->faultCode = reason;
                        if(result >= 0x00){
                            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_APPLY_CHARGE_RESULT);
                        }else{
                            sgcc_response_buff_release_sem();
                        }
                    }
                }
            }

            /***** [启动充电异步响应] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = sgcc_get_response_buff(RT_WAITING_FOREVER);
                result = sgcc_response_padding_remote_start_charge(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    ((evs_event_startResult*)response->general_transmit_buff)->startResult = sgcc_get_remotecharge_result(gunno);
                    ((evs_event_startResult*)response->general_transmit_buff)->faultCode = sgcc_get_remotecharge_fail_reason(gunno);

                    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_SERVER_START_CHARGE);
                }else{
                    sgcc_response_buff_release_sem();
                }
            }
            /***** [停止充电异步响应] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = sgcc_get_response_buff(RT_WAITING_FOREVER);
                result = sgcc_response_padding_remote_stop_charge(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    ((evs_event_stopCharge*)response->general_transmit_buff)->stopResult = sgcc_get_remotestop_result(gunno);
                    ((evs_event_stopCharge*)response->general_transmit_buff)->resultCode = sgcc_get_remotestop_fail_reason(gunno);
                    ((evs_event_stopCharge*)response->general_transmit_buff)->stopFailReson = sgcc_get_remotestop_fault_reason(gunno);
                    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_SERVER_STOP_CHARGE);
                }else{
                    sgcc_response_buff_release_sem();
                }
            }
            /***** [申请充电启动结果异步响应] *****/
            if(sgcc_net_event_receive(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno,
                    (NET_SGCC_EVENT_OPTION_OR |NET_SGCC_EVENT_OPTION_CLEAR), NET_SGCC_PRES_EVENT_APPLY_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = sgcc_get_response_buff(RT_WAITING_FOREVER);
                result = sgcc_response_padding_apply_charge_result(gunno, response->general_transmit_buff, NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(sgcc_get_applycharge_result(gunno)){
                        ((evs_event_startResult*)response->general_transmit_buff)->startResult = SGCC_OPSCTL_ACTION;
                    }else{
                        /* 此处需要进行原因码转换 */
                        ((evs_event_startResult*)response->general_transmit_buff)->faultCode = 0x00;
                    }
                    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_APPLY_CHARGE_RESULT);
                }else{
                    sgcc_response_buff_release_sem();
                }
            }
        }

        rt_thread_mdelay(100);
    }
}

int sgcc_message_send_init(void)
{
    if(rt_thread_init(&s_sgcc_message_send_thread, "sgcc_send", sgcc_message_send_thread_entry, NULL,
            s_sgcc_message_send_thread_stack, NET_SGCC_MESSAGE_SEND_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("sgcc message send thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_message_send_thread) != RT_EOK){
        LOG_E("sgcc message send thread startup fail, please check");
        return -0x01;
    }

    if(rt_thread_init(&s_sgcc_message_server_thread, "sgcc_server", sgcc_message_server_thread_entry, NULL,
            s_sgcc_message_service_thread_stack, NET_SGCC_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("sgcc message pro thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_message_server_thread) != RT_EOK){
        LOG_E("gw message pro thread startup fail, please check");
        return -0x01;
    }

    if(rt_thread_init(&s_sgcc_connect_thread, "sgcc_conn", sgcc_connect_thread_entry, NULL,
            s_sgcc_connect_thread_stack, NET_SGCC_CONNECT_THREAD_STACK_SIZE, 10, 10) != RT_EOK){
        LOG_E("sgcc connect thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_connect_thread) != RT_EOK){
        LOG_E("sgcc connect thread startup fail, please check");
        return -0x01;
    }

    if(rt_thread_init(&s_sgcc_yield_thread, "sgcc_yield", sgcc_yield_thread_entry, NULL,
            s_sgcc_yield_thread_stack, NET_SGCC_YIELD_THREAD_STACK_SIZE, 9, 10) != RT_EOK){
        LOG_E("sgcc yield thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_yield_thread) != RT_EOK){
        LOG_E("sgcc yield thread startup fail, please check");
        return -0x01;
    }

    if(rt_sem_init(&s_sgcc_response_buff_sem, "sgcc_tbsem", 0x01, RT_IPC_FLAG_PRIO) != RT_EOK){
        LOG_E("sgcc response buff sem create fail");
    }

    return 0;
}

#endif /* NET_PACK_USING_SGCC */
