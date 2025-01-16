/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#include "ycp_message_send.h"
#include "ycp_message_receive.h"
#include "ycp_transceiver.h"
#include "ycp_message_padding.h"

#include "net_operation.h"

#define DBG_TAG "ycp_send"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YCP

#define NET_YCP_HEARTBEAT_TIMEOUT_RENTRY                     3           /* 心跳超时次数 */
#define NET_YCP_OPEN_SOCKET_RENTRY                           5           /* 打开socket尝试次数 */
#define NET_YCP_LOGIN_RENTRY                                 5           /* 登录尝试次数 */
#define NET_YCP_WAIT_LOGIN_RENTRY                            100         /* 等待登录结果尝试次数 */
#define NET_YCP_WAIT_UNLOCK_TIMEOUT                          (10 *1000)  /* 等待socket 解锁超时时间(单位：ms) */

#define NET_YCP_REQ_BILLINGRULE_INTERVAL                     (30 *1000)  /* 未接收到计费规则时，重新请求间隔(单位ms:30 *1000 = 30s) */

#define NET_YCP_LOGIN_OPERATION_INTERVAL                     30000       /* 登录操作间隔(单位ms:30 *1000 = 30s) */

#define NET_YCP_REALTIME_DATA_IDLE_INTERVAL                  300000      /* 实时数据空闲上报间隔(单位ms:5 *60 *1000 = 5min) */
#define NET_YCP_REALTIME_DATA_CHARGING_INTERVAL              15000       /* 实时数据充电中上报间隔(单位ms:15 *1000 = 15s) */
#define NET_YCP_REALTIME_DATA_LINK_INTERVAL                  5000        /* 实时数据刚连上网时上报间隔(单位ms:5 *1000 = 5s) */
#define NET_YCP_SAME_TRANSATION_REPORT_COUNT_MAX             10          /* 相同订单最大上报次数 */

#define NET_YCP_TIME_SYNC_PERIOD_DEF                         (43200000)  /* 向平台对时的时间间隔(12 *60 *60 *1000) */

struct ycp_wait_response{
    uint32_t message_wait_response_state[NET_SYSTEM_GUN_NUMBER];                       /* 报文已发送发送，等待响应状态 */
    uint32_t message_repeat_time[NET_SYSTEM_GUN_NUMBER][NET_YCP_CHARGEPILE_PREQ_NUM];  /* 报文重发计时 */
};

struct ycp_assistant_flag{
    uint16_t is_timesync : 1;
    uint16_t is_verify_billingrule : 1;
    uint16_t message_recv_error : 1;
    uint16_t message_send_error : 1;
    uint16_t modify_server_addr : 1;
    uint16_t attemp_open_success : 1;
    uint16_t attemp_login_success : 1;
    uint16_t is_attemping_open : 1;
    uint16_t is_attemping_login : 1;
    uint16_t is_storaging : 1;
    uint16_t req_billingrule_again : 1;
};

NET_DEF_SRAM2 static uint32_t s_ycp_heartbeat_tick;
NET_DEF_SRAM2 static uint32_t s_ycp_billing_rule_tick;
NET_DEF_SRAM2 uint8_t s_ycp_current_transaction_number[NET_SYSTEM_GUN_NUMBER][NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];
NET_DEF_SRAM2 static struct ycp_assistant_flag s_ycp_assistant_flag;
NET_DEF_SRAM2 static uint8_t s_ycp_same_transaction_report_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint8_t s_ycp_transaction_verify[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct ycp_wait_response s_ycp_wait_response;
NET_DEF_SRAM2 static ycp_socket_info_t s_ycp_socket_info;
NET_DEF_SRAM2 static uint16_t s_ycp_message_serial_number[NET_SYSTEM_GUN_NUMBER];                      /* 报文序列号 */
NET_DEF_SRAM2 static uint32_t s_ycp_message_send_state[NET_SYSTEM_GUN_NUMBER];                        /* 报文发送状态 */
NET_DEF_SRAM2 static uint32_t s_ycp_chargepile_event[NET_YCP_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_ycp_server_event[NET_YCP_EVENT_TYPE_SIZE][NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_ycp_wait_storage_tick;

NET_DEF_SRAM2 uint32_t g_net_target_platform_tick = 0x00;

NET_DEF_SRAM2 static struct rt_thread s_ycp_message_send_thread;
NET_DEF_SRAM0 static uint8_t s_ycp_message_send_thread_stack[NET_YCP_MESSAGE_SEND_THREAD_STACK_SIZE];
NET_DEF_SRAM2 static struct rt_thread s_ycp_server_message_pro_thread;
NET_DEF_SRAM0 static uint8_t s_ycp_server_message_pro_thread_stack[NET_YCP_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE];
NET_DEF_SRAM2 static ycp_response_message_buf_t s_ycp_response_buff;
NET_DEF_SRAM2 static struct rt_semaphore s_ycp_response_buff_sem;

/** 登录签到 */
NET_DEF_SRAM2 Net_YcpPro_PReq_LogIn_t g_ycp_preq_login;
/** 对时 */
NET_DEF_SRAM2 Net_YcpPro_PReq_TimeSync_t g_ycp_preq_time_sync;  // OK
/** 上报心跳 */
NET_DEF_SRAM2 Net_YcpPro_PReq_HeartBeat_t g_ycp_preq_heartbeat;   // OK
/** 计费模型验证 */
NET_DEF_SRAM2 Net_YcpPro_PReq_BillingModel_Verify_t g_ycp_preq_billing_model_verify;   // OK
/** 上报单枪状态数据 */
NET_DEF_SRAM2 Net_YcpPro_PRes_Query_PReq_Report_PileState_t g_ycp_preq_report_state_data[NET_SYSTEM_GUN_NUMBER];  // OK
/** 充电桩主动申请启动充电 */
NET_DEF_SRAM2 Net_YcpPro_PReq_ApplyCharge_Active_t g_ycp_preq_apply_charge_active[NET_SYSTEM_GUN_NUMBER];  // OK
/** 交易记录 */
NET_DEF_SRAM2 Net_YcpPro_PReq_TransactionRecords_t g_ycp_preq_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 上报设备故障 */
NET_DEF_SRAM2 Net_YcpPro_PReq_Report_DeviceFault_t g_ycp_preq_report_device_fault;  // OK
/** 充电握手 */
NET_DEF_SRAM2 Net_YcpPro_PReq_ShakeHand_t g_ycp_preq_shake_hand[NET_SYSTEM_GUN_NUMBER];   // OK
/** 参数配置 */
NET_DEF_SRAM2 Net_YcpPro_PReq_ParameterConfig_t g_ycp_preq_parameter_config[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电结束 */
NET_DEF_SRAM2 Net_YcpPro_PReq_ChargeFinish_t g_ycp_preq_charge_finish[NET_SYSTEM_GUN_NUMBER];   // OK
/** 错误报文 */
NET_DEF_SRAM2 Net_YcpPro_PReq_ErrorMessage_t g_ycp_preq_error_message[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中 BMS 终止 */
NET_DEF_SRAM2 Net_YcpPro_PReq_BmsEnd_t g_ycp_preq_bms_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中充电机终止 */
NET_DEF_SRAM2 Net_YcpPro_PReq_ChargerEnd_t g_ycp_preq_charger_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 需求与充电机输出 */
NET_DEF_SRAM2 Net_YcpPro_PReq_BmsCommand_ChargerOut_t g_ycp_preq_bmscommand_chargerout[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 信息 */
NET_DEF_SRAM2 Net_YcpPro_PReq_BmsInfo_t g_ycp_preq_bms_info[NET_SYSTEM_GUN_NUMBER];   // OK
/** 升级结果上送 */
NET_DEF_SRAM2 Net_YcpPro_PRes_RemoteUpdate_t g_ycp_pres_remote_update;  // OK


/*******************************************************
 * 函数名               ycp_enter_critical
 * 功能                  进入临界区
 * 参数
 * 返回
 ******************************************************/
void ycp_enter_critical(void)
{
    rt_enter_critical();
}

/*******************************************************
 * 函数名               ycp_exit_critical
 * 功能                  退出临界区
 * 参数
 * 返回
 ******************************************************/
void ycp_exit_critical(void)
{
    rt_exit_critical();
}

/**************************************************************************
 * 函数名                 ycp_is_interact_normally
 * 功能                     判断是否已经可以正常交互数据(越城公用协议需要接收到计费规则(响应)后才能交互其它报文)
 * 说明                     1：可以    0：不可
 * ***********************************************************************/
uint8_t ycp_is_interact_normally(void)
{
    return s_ycp_assistant_flag.is_verify_billingrule;
}

/**************************************************************************
 * 函数名                 ycp_request_billingrule_again
 * 功能                     用于外部触发再次请求计费规则
 * 说明
 * ***********************************************************************/
void ycp_request_billingrule_again(void)
{
    s_ycp_assistant_flag.req_billingrule_again = 0x01;
}

/**************************************************************************
 * 函数名                 ycp_get_socket_info
 * 功能                     获取socket信息
 * 说明
 * ***********************************************************************/
ycp_socket_info_t* ycp_get_socket_info(void)
{
    return &s_ycp_socket_info;
}

/**************************************************************************
 * 函数名                 ycp_response_buff_take_sem_forever
 * 功能                     获取响应缓存互斥量
 * 说明
 * ***********************************************************************/
static int32_t ycp_response_buff_take_sem_forever(int32_t timeout)
{
    return rt_sem_take(&s_ycp_response_buff_sem, timeout);
}
/**************************************************************************
 * 函数名                 ycp_response_buff_release_sem
 * 功能                     释放响应缓存互斥信号量
 * 说明
 * ***********************************************************************/
static void ycp_response_buff_release_sem(void)
{
    rt_sem_release(&s_ycp_response_buff_sem);
}
/**************************************************************************
 * 函数名                 ycp_get_response_buff
 * 功能                     获取响应缓存
 * 说明
 * ***********************************************************************/
static ycp_response_message_buf_t* ycp_get_response_buff(int32_t timeout)
{
    if(ycp_response_buff_take_sem_forever(timeout) < 0){
        return NULL;
    }
    return &s_ycp_response_buff;
}

/**************************************************************************
 * 函数名                 ycp_transaction_is_verify
 * 功能                     查询交易是否已确认
 * 说明
 * ***********************************************************************/
uint8_t ycp_transaction_is_verify(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ycp_transaction_verify[gunno];
}

/**************************************************************************
 * 函数名                 ycp_set_transaction_verify_state
 * 功能                     设置交易确认状态
 * 说明
 * ***********************************************************************/
void ycp_set_transaction_verify_state(uint8_t gunno, uint8_t state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_ycp_transaction_verify[gunno] = 0x01;
    }else{
        s_ycp_transaction_verify[gunno] = 0x00;
    }
}

/**************************************************************************
 * 函数名                 ycp_net_event_send
 * 功能                     网络事件发送
 * 说明
 * ***********************************************************************/
int32_t ycp_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event)
{
    if(event_handle >= NET_YCP_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_YCP_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }

    if(event_handle == NET_YCP_EVENT_HANDLE_CHARGEPILE){
        s_ycp_chargepile_event[event_type][gunno] |= (1 <<event);
    }else{
        s_ycp_server_event[event_type][gunno] |= (1 <<event);
    }

    return 0;
}
/**************************************************************************
 * 函数名                 ycp_net_event_receive
 * 功能                     网络事件接收
 * 说明                     若事件发生则对应位为 1，否则为0
 * ***********************************************************************/
int32_t ycp_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf)
{
    if(event_handle >= NET_YCP_EVENT_HANDLE_SIZE){
        return -1;
    }
    if(event_type >= NET_YCP_EVENT_TYPE_SIZE){
        return -2;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -3;
    }
    uint32_t (*event_set)[NET_SYSTEM_GUN_NUMBER] = NULL;

    if(event_handle == NET_YCP_EVENT_HANDLE_CHARGEPILE){
        event_set = s_ycp_chargepile_event;
    }else{
        event_set = s_ycp_server_event;
    }

    if(event_buf){
        (*event_buf) = event_set[event_type][gunno];
    }

    if(event_set[event_type][gunno] &(1 <<event)){
        if(option &NET_YCP_EVENT_OPTION_AND){
            if((event_set[event_type][gunno] &(1 <<event)) != (1 <<event)){
                return -4;
            }
        }
        if(option &NET_YCP_EVENT_OPTION_CLEAR){
            event_set[event_type][gunno] &= (~(1 <<event));
        }
        return 1;
    }
    return 0;
}

/**************************************************************************
 * 函数名                 ycp_get_message_send_state
 * 功能                     获取报文发送状态
 * 说明                     若报文正在发送则对应位为 1，否则为0
 * ***********************************************************************/
uint8_t ycp_get_message_send_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_ycp_message_send_state[gunno] &(1 <<message_bit)){
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 ycp_set_message_send_state
 * 功能                     设置报文发送状态
 * 说明                     若报文正在发送则对应位置 1，否则置0
 * ***********************************************************************/
void ycp_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(state){
        s_ycp_message_send_state[gunno] |= (1 <<message_bit);
    }else{
        s_ycp_message_send_state[gunno] &= (~(1 <<message_bit));
    }
}

/**************************************************************************
 * 函数名                 ycp_set_message_wait_response_state
 * 功能                     设置报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
static void ycp_set_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_YCP_CHARGEPILE_PREQ_NUM){
        return;
    }
    s_ycp_wait_response.message_wait_response_state[gunno] |= (1 <<message_bit);
    s_ycp_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
}
/**************************************************************************
 * 函数名                 ycp_clear_message_wait_response_state
 * 功能                     清除报文已发送，等待响应状态
 * 说明
 * ***********************************************************************/
void ycp_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(message_bit >= NET_YCP_CHARGEPILE_PREQ_NUM){
        return;
    }
    s_ycp_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
}
/**************************************************************************
 * 函数名                 ycp_exist_message_wait_response
 * 功能                     检查是否存在已发送的报文等待响应
 * 说明
 * ***********************************************************************/
uint8_t ycp_exist_message_wait_response(uint8_t gunno, uint32_t *state)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_ycp_wait_response.message_wait_response_state[gunno]){
        if(state){
            *state = s_ycp_wait_response.message_wait_response_state[gunno];
        }
        return 0x01;
    }
    return 0x00;
}
/**************************************************************************
 * 函数名                 ycp_get_message_wait_response_timeout_state
 * 功能                     获取报文等待响应超时状态
 * 说明
 * ***********************************************************************/
uint8_t ycp_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(message_bit >= NET_YCP_CHARGEPILE_PREQ_NUM){
        return 0x00;
    }
    if(s_ycp_wait_response.message_wait_response_state[gunno] &(1 <<message_bit)){
        if(s_ycp_wait_response.message_repeat_time[gunno][message_bit] > rt_tick_get()){
            s_ycp_wait_response.message_repeat_time[gunno][message_bit] = rt_tick_get();
        }
        if((rt_tick_get() - s_ycp_wait_response.message_repeat_time[gunno][message_bit]) > timeout){
            s_ycp_wait_response.message_wait_response_state[gunno] &= (~(1 <<message_bit));
            return 0x01;
        }
    }
    return 0x00;
}

/**************************************************************************
 * 函数名                 ycp_bcd_to_ascii
 * 功能                     将BCD转成字符码
 * 说明
 * ***********************************************************************/
void ycp_bcd_to_ascii(uint8_t *ascii, uint8_t alen, uint8_t *bcd, uint8_t blen)
{
    if((ascii == NULL) || (bcd == NULL) || (alen == 0x00) || (blen == 0x00)){
        return;
    }
    int16_t aindex, bindex;

    memset(ascii, 0x00, alen);

    for(aindex = 0, bindex = 0; ((aindex + 1) < alen) && (bindex < blen); aindex += 2, bindex++){
        ascii[aindex] = (uint8_t)((bcd[bindex] &0xf0) >>4);
        ascii[aindex] += 0x30;
        ascii[aindex + 1] = (bcd[bindex] &0x0f);
        ascii[aindex + 1] += 0x30;
    }
}

/**************************************************************************
 * 函数名                 ycp_ascii_to_bcd
 * 功能                     将字符码转成BCD
 * 说明
 * ***********************************************************************/
void ycp_ascii_to_bcd(uint8_t *ascii, uint8_t alen, uint8_t *bcd, uint8_t blen, uint8_t is_order)
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

/**************************************************************************
 * 函数名                 ycp_timestamp_to_timebcd
 * 功能                     将时间戳转成时间BCD码
 * 说明
 * ***********************************************************************/
void ycp_timestamp_to_timebcd(uint32_t timestamp, uint8_t *bcd, uint8_t len)
{
    if(bcd == NULL || len < 0x07){
        return;
    }
    struct tm _tm;
    uint8_t value, yhigh, ylow;

    ycp_enter_critical();
    _tm = *(localtime((time_t*)(&timestamp)));
    ycp_exit_critical();

    /** 年 */
    _tm.tm_year += 1900;
    bcd[0x01] = (uint8_t)(_tm.tm_year %10);
    _tm.tm_year /= 10;
    bcd[0x01] |= (uint8_t)((_tm.tm_year %10) <<0x04);
    _tm.tm_year /= 10;

    bcd[0x00] = (uint8_t)(_tm.tm_year %10);
    _tm.tm_year /= 10;
    bcd[0x00] |= (uint8_t)((_tm.tm_year %10) <<0x04);

    /** 月 */
    _tm.tm_mon += 0x01;
    bcd[0x02] = (uint8_t)(_tm.tm_mon %10);
    _tm.tm_mon /= 10;
    bcd[0x02] |= (uint8_t)((_tm.tm_mon %10) <<0x04);
    _tm.tm_mon /= 10;

    /** 日 */
    bcd[0x03] = (uint8_t)(_tm.tm_mday %10);
    _tm.tm_mday /= 10;
    bcd[0x03] |= (uint8_t)((_tm.tm_mday %10) <<0x04);
    _tm.tm_mday /= 10;

    /** 时 */
    bcd[0x04] = (uint8_t)(_tm.tm_hour %10);
    _tm.tm_hour /= 10;
    bcd[0x04] |= (uint8_t)((_tm.tm_hour %10) <<0x04);
    _tm.tm_hour /= 10;

    /** 分 */
    bcd[0x05] = (uint8_t)(_tm.tm_min %10);
    _tm.tm_min /= 10;
    bcd[0x05] |= (uint8_t)((_tm.tm_min %10) <<0x04);
    _tm.tm_min /= 10;

    /** 秒 */
    bcd[0x06] = (uint8_t)(_tm.tm_sec %10);
    _tm.tm_sec /= 10;
    bcd[0x06] |= (uint8_t)((_tm.tm_sec %10) <<0x04);
    _tm.tm_sec /= 10;


}

/**************************************************************************
 * 函数名                 ycp_ascii_to_bcd
 * 功能                     将时间BCD码转成时间戳
 * 说明
 * ***********************************************************************/
uint32_t ycp_timebcd_to_timestamp(uint8_t *bcd, uint8_t len)
{
    if(bcd == NULL || len < 0x07){
        return time(NULL);
    }
    uint32_t timestamp;
    uint8_t byteh, bytel;
    struct tm t = { 0 };

    /** 秒 */
    byteh = (uint8_t)((bcd[0x06] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x06] &0x0F);
    t.tm_sec = byteh *10 + bytel;

    /**分 */
    byteh = (uint8_t)((bcd[0x05] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x05] &0x0F);
    t.tm_min = byteh *10 + bytel;

    /** 时 */
    byteh = (uint8_t)((bcd[0x04] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x04] &0x0F);
    t.tm_hour = byteh *10 + bytel;

    /** 日 */
    byteh = (uint8_t)((bcd[0x03] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x03] &0x0F);
    t.tm_mday = byteh *10 + bytel;

    /** 月 */
    byteh = (uint8_t)((bcd[0x02] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x02] &0x0F);
    t.tm_mon = byteh *10 + bytel;
    if(t.tm_mon > 0x00){
        t.tm_mon -= 0x01;
    }

    /** 年 */
    byteh = (uint8_t)((bcd[0x01] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x01] &0x0F);
    t.tm_year = byteh *10 + bytel;

    byteh = (uint8_t)((bcd[0x00] &0xF0) >> 0x04);
    bytel = (uint8_t)(bcd[0x00] &0x0F);
    t.tm_year += byteh *1000 + bytel *100;
    if(t.tm_year > 1900){
        t.tm_year -= 1900;
    }else{
        t.tm_year = 0x00;
    }

    timestamp = mktime(&t);
    return timestamp;
}

static void net_ycp_message_send_thread_entry(void *parameter)
{
    uint8_t step = NET_YCP_NET_STATE_OPEN_SOCKET, is_power_on = 0x01;
    uint32_t delay = 0x00, wait_unlock = 0x00;
    uint32_t time_sync_tick;

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YCP |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *host = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, NULL, 0x00, option));
    uint16_t port = *((uint16_t*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PORT, NULL, 0x00, option)));

    if((port == 0x00) || (port == 0xFFFF)){
        host = "121.229.203.34";
        port = 6002;
    }
    LOG_D("ycp current link ip[%s:%d]", host, port);

    s_ycp_socket_info.fd = -0x01;
    s_ycp_socket_info.domain_is_prase = 0x00;

    while(1)
    {
        s_ycp_socket_info.program_state = step;
        g_net_target_platform_tick = rt_tick_get();
        net_thread_running(rt_thread_self(), NULL, 0x00, 0x00);
        if((net_get_ota_info()->state >= NET_OTA_STATE_OPEN_LINK) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        handle->data_updata();
        if((handle->net_fault) &NET_FAULT_PHYSICAL_LAYER){
            if((s_ycp_socket_info.fd >= 0x00) && (step >= NET_YCP_NET_STATE_LOGIN)){   /** 平台已建立连接，但是通信模块出错(关机) */
                ycp_socket_close(s_ycp_socket_info.fd);
                s_ycp_socket_info.fd = -0x01;
            }
            s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_PHY;
            handle->net_state = NET_SOCKET_STATE_PHY;
            s_ycp_socket_info.fd = -0x01;
            step = NET_YCP_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YCP_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YCP_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((handle->net_fault) &NET_FAULT_SIM_CARD){
            if((s_ycp_socket_info.fd >= 0x00) && (step >= NET_YCP_NET_STATE_LOGIN)){   /** 平台已建立连接，但是通信模块出错(关机) */
                ycp_socket_close(s_ycp_socket_info.fd);
                s_ycp_socket_info.fd = -0x01;
            }
            s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_SIM;
            handle->net_state = NET_SOCKET_STATE_SIM;
            s_ycp_socket_info.fd = -0x01;
            step = NET_YCP_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YCP_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YCP_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((handle->net_fault) &NET_FAULT_DATA_LINK_LAYER){
            if((s_ycp_socket_info.fd >= 0x00) && (step >= NET_YCP_NET_STATE_LOGIN)){   /** 平台已建立连接，但是通信模块出错(关机) */
                ycp_socket_close(s_ycp_socket_info.fd);
                s_ycp_socket_info.fd = -0x01;
            }
            s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_DATA_LINK;
            handle->net_state = NET_SOCKET_STATE_DATA_LINK;
            s_ycp_socket_info.fd = -0x01;
            step = NET_YCP_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YCP_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YCP_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }
        if((handle->net_fault) &NET_FAULT_MODULE_INIT){
            if((s_ycp_socket_info.fd >= 0x00) && (step >= NET_YCP_NET_STATE_LOGIN)){   /** 平台已建立连接，但是通信模块出错(关机) */
                ycp_socket_close(s_ycp_socket_info.fd);
                s_ycp_socket_info.fd = -0x01;
            }
            s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_MODULE_INIT;
            handle->net_state = NET_SOCKET_STATE_MODULE_INIT;
            s_ycp_socket_info.fd = -0x01;
            step = NET_YCP_NET_STATE_OPEN_SOCKET;
            if(rt_tick_get() > (delay + NET_YCP_LOGIN_OPERATION_INTERVAL)){
                delay = (rt_tick_get() - NET_YCP_LOGIN_OPERATION_INTERVAL);
            }

            rt_thread_mdelay(1000);
            continue;
        }

        /** 进行域名解析 */
        if(s_ycp_socket_info.domain_is_prase == 0x00){
            char ip_str[0x10];   /** 点分十进制式IP，最大长度15，预留一位 */
            if(ycp_socket_domain_parse(0x00, host, strlen(host), ip_str, sizeof(ip_str)) >= 0x00){
                net_operation_set_target_socket_domain(ip_str, strlen(ip_str));
                net_operation_set_target_socket_port(port);
                s_ycp_socket_info.domain_is_prase = 0x01;

                LOG_D("ycp domain prase success[%s:%d]", ip_str, port);
            }
        }
        /***************************************************** [登录认证] **********************************************************/
        /***************************************************** [登录认证] **********************************************************/
        if(s_ycp_socket_info.socket_state != YCP_SOCKET_STATE_LOGIN_SUCCESS){
            rt_kprintf("ycp state(%d, %d, %d)\n", s_ycp_socket_info.socket_state, step, (rt_tick_get() - delay));
            switch(step){
            case NET_YCP_NET_STATE_OPEN_SOCKET:
                s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_OPEN;
                handle->net_state = NET_SOCKET_STATE_OPEN;
                if(delay > rt_tick_get()){
                    delay = rt_tick_get();
                }
                if(((rt_tick_get() - delay) > NET_YCP_LOGIN_OPERATION_INTERVAL) || is_power_on){
                    int32_t result = 0x00;

                    result = ycp_socket_open(&(s_ycp_socket_info.fd), host, strlen(host), port);
                    if(result >= 0){
                        int32_t recv_timeout = 0x0A;

                        LOG_D("ycp socket open success with host[%s] port[%d] fd(%d)", host, port, s_ycp_socket_info.fd);
                        s_ycp_socket_info.operate_fail.open_socket = 0;
                        step = NET_YCP_NET_STATE_LOGIN;
                        /** 状态变化，提前改变状态 */
                        s_ycp_socket_info.program_state = step;
                        s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_LOGIN_WAIT;
                        handle->net_state = NET_SOCKET_STATE_LOGIN_WAIT;

                        ycp_socket_modify_recv_timeout(s_ycp_socket_info.fd, recv_timeout);
                    }else{
                        LOG_W("ycp fail to open socket with host[%s] port[%d] num|%d", host, port, s_ycp_socket_info.operate_fail.open_socket);
                        delay = rt_tick_get();
                        s_ycp_socket_info.operate_fail.open_socket++;
                        s_ycp_socket_info.fd = -0x01;
                    }
                    for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                        s_ycp_message_serial_number[gunno] = 0x00;
                    }
                    is_power_on = 0x00;
                    /** socket 状态可能有变，上报一次 */
                    net_operation_tplat_info_trigger(0x01);
                }
                break;
            case NET_YCP_NET_STATE_LOGIN:
            {
                uint8_t rentry = 0x00, data[NET_YCP_SIM_BCD_LENGTH_DEFAULT *0x02 + 0x01], *sim_no = NULL, *imei = NULL;

                ycp_device_sn_buf_t *device_sn = NULL;
                memset(data, 0x00, (NET_YCP_SIM_BCD_LENGTH_DEFAULT *0x02 + 0x01));
                (void)(handle->get_system_data(NET_SYSTEM_DATA_NAME_ICCID, data, sizeof(data), option));

                g_ycp_preq_login.body.operators = *(uint8_t*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_OPERATOR, NULL, 0x00, option));
                memset(g_ycp_preq_login.body.sim_number, '\0', NET_YCP_SIM_BCD_LENGTH_DEFAULT);
                sim_no = data;
                ycp_ascii_to_bcd(sim_no, strlen((char*)sim_no), g_ycp_preq_login.body.sim_number, NET_YCP_SIM_BCD_LENGTH_DEFAULT, 0x01);

                switch (g_ycp_preq_login.body.operators) {
                case NET_OPERATOR_NAME_CHINA_MOBILE:
                    g_ycp_preq_login.body.operators = NET_YCP_OPERATOR_MOBILE;
                    break;
                case NET_OPERATOR_NAME_CHINA_TELECOM:
                    g_ycp_preq_login.body.operators = NET_YCP_OPERATOR_TELECOM;
                    break;
                case NET_OPERATOR_NAME_CHINA_UNICOM:
                    g_ycp_preq_login.body.operators = NET_YCP_OPERATOR_UNICOM;
                    break;
                default:
                    g_ycp_preq_login.body.operators = NET_YCP_OPERATOR_OTHER;
                    break;
                }

                memset(g_ycp_preq_login.body.communicate_module_sn, '\0', NET_YCP_COMMUNICATE_MODULE_SN_LENGTH_DEFAULT);
                memset(data, 0x00, (NET_YCP_SIM_BCD_LENGTH_DEFAULT *0x02 + 0x01));
                (void)(handle->get_system_data(NET_SYSTEM_DATA_NAME_IMEI, data, sizeof(data), option));
                imei = data;
                ycp_ascii_to_bcd(imei, strlen((char*)imei), g_ycp_preq_login.body.communicate_module_sn, NET_YCP_COMMUNICATE_MODULE_SN_LENGTH_DEFAULT, 0x00);

                ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_LOGIN, NULL);
                s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_LOGIN_WAIT;
                handle->net_state = NET_SOCKET_STATE_LOGIN_WAIT;
                ycp_message_send_port(NETYCP_PREQCMD_SINGIN, s_ycp_socket_info.fd, &g_ycp_preq_login,
                        sizeof(g_ycp_preq_login));
                while(rentry < NET_YCP_WAIT_LOGIN_RENTRY){
                    if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00,
                            (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_LOGIN, NULL) > 0){
                        LOG_D("ycp login success");
                        s_ycp_socket_info.operate_fail.login = 0;
                        step = NET_YCP_NET_STATE_MONITORING;
                        /** 状态变化，提前改变状态 */
                        s_ycp_socket_info.program_state = step;

                        device_sn = (ycp_device_sn_buf_t*)(ycp_get_device_sn_info());
                        if(handle->set_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, (uint8_t*)device_sn->device_sn, device_sn->device_sn_length, option) >= 0x00){
                            handle->system_data_storage(0x00);
                        }
                        net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_TARGET, 0x01);
                        s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_LOGIN_SUCCESS;
                        handle->net_state = NET_SOCKET_STATE_LOGIN_SUCCESS;
                        s_ycp_socket_info.heartbeat = 0x00;
                        break;
                    }
                    rt_thread_mdelay(100);
                    rentry++;
                }
                if(rentry >= NET_YCP_WAIT_LOGIN_RENTRY){
                    delay = rt_tick_get();
                    wait_unlock = rt_tick_get();
                    step = NET_YCP_NET_STATE_OPEN_SOCKET;
                    /** 状态变化，提前改变状态 */
                    s_ycp_socket_info.program_state = step;
                    s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_OPEN;
                    handle->net_state = NET_SOCKET_STATE_OPEN;
                    while(ycp_socket_is_lock()){
                        if((rt_tick_get() - wait_unlock) > NET_YCP_WAIT_UNLOCK_TIMEOUT){
                            break;
                        }
                        rt_thread_mdelay(50);
                    }

                    ycp_socket_close(s_ycp_socket_info.fd);
                    s_ycp_socket_info.fd = -0x01;
                    s_ycp_socket_info.operate_fail.login++;
                    LOG_D("ycp login fail(timeout) num|%d", s_ycp_socket_info.operate_fail.login);
                }else{
                    rt_thread_mdelay(250);
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_TIME_SYNC);
                    rt_thread_mdelay(250);
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
                }
                /** socket 状态可能有变，上报一次 */
                net_operation_tplat_info_trigger(0x01);
                break;
            }
            case NET_YCP_NET_STATE_MONITORING:
                s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_LOGIN_SUCCESS;
                handle->net_state = NET_SOCKET_STATE_LOGIN_SUCCESS;
                break;
            default:
                delay = rt_tick_get();
                wait_unlock = rt_tick_get();
                step = NET_YCP_NET_STATE_OPEN_SOCKET;
                /** 状态变化，提前改变状态 */
                s_ycp_socket_info.program_state = step;
                s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_OPEN;
                handle->net_state = NET_SOCKET_STATE_OPEN;
                while(ycp_socket_is_lock()){
                    if((rt_tick_get() - wait_unlock) > NET_YCP_WAIT_UNLOCK_TIMEOUT){
                        break;
                    }
                    rt_thread_mdelay(50);
                }

                ycp_socket_close(s_ycp_socket_info.fd);
                s_ycp_socket_info.fd = -0x01;
                /** socket 状态可能有变，上报一次 */
                net_operation_tplat_info_trigger(0x01);
                break;
            }
        }

        if(s_ycp_socket_info.operate_fail.open_socket > NET_YCP_OPEN_SOCKET_RENTRY){
            /** socket 状态有变，上报一次 */
            net_operation_tplat_info_trigger(0x01);
            s_ycp_socket_info.operate_fail.open_socket = 0x00;
            LOG_W("ycp open socket rentry = 0x00");
            rt_thread_mdelay(5000);
            net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_TARGET, 0x00);
        }
        if(s_ycp_socket_info.operate_fail.login > NET_YCP_OPEN_SOCKET_RENTRY){
            /** socket 状态有变，上报一次 */
            net_operation_tplat_info_trigger(0x01);
            s_ycp_socket_info.operate_fail.login = 0x00;
            LOG_W("ycp login rentry = 0x00");
            rt_thread_mdelay(5000);
            net_set_clear_ndev_reset_state(NET_PLATFORM_MASK_TARGET, 0x00);
        }

        if(s_ycp_socket_info.socket_state != YCP_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
            s_ycp_assistant_flag.is_timesync = 0x00;
            s_ycp_assistant_flag.is_verify_billingrule = 0x00;
            s_ycp_assistant_flag.req_billingrule_again = 0x00;
            s_ycp_heartbeat_tick = rt_tick_get();
            s_ycp_billing_rule_tick = rt_tick_get();
            rt_thread_mdelay(1000);
            continue;
        }


        if(s_ycp_socket_info.heartbeat > NET_YCP_HEARTBEAT_TIMEOUT_RENTRY){
            delay = rt_tick_get();
            wait_unlock = rt_tick_get();

            step = NET_YCP_NET_STATE_OPEN_SOCKET;
            /** 状态变化，提前改变状态 */
            s_ycp_socket_info.program_state = step;
            s_ycp_socket_info.socket_state = YCP_SOCKET_STATE_OPEN;
            handle->net_state = NET_SOCKET_STATE_OPEN;
            while(ycp_socket_is_lock()){
                if((rt_tick_get() - wait_unlock) > NET_YCP_WAIT_UNLOCK_TIMEOUT){
                    break;
                }
                rt_thread_mdelay(50);
            }

            ycp_socket_close(s_ycp_socket_info.fd);
            s_ycp_socket_info.fd = -0x01;
            /** socket 状态可能有变，上报一次 */
            net_operation_tplat_info_trigger(0x01);
            s_ycp_socket_info.heartbeat = 0x00;

            LOG_D("ycp heartbeat timeout");
        }

        if(s_ycp_assistant_flag.is_timesync){
            if(time_sync_tick > rt_tick_get()){
                if((rt_tick_get() + 0xFFFFFFFF - time_sync_tick) > NET_YCP_TIME_SYNC_PERIOD_DEF){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_TIME_SYNC);
                    time_sync_tick = rt_tick_get();
                    s_ycp_assistant_flag.is_timesync = 0x00;
                }
            }else{
                if((rt_tick_get() - time_sync_tick) > NET_YCP_TIME_SYNC_PERIOD_DEF){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_TIME_SYNC);
                    time_sync_tick = rt_tick_get();
                    s_ycp_assistant_flag.is_timesync = 0x00;
                }
            }
        }else{
            time_sync_tick = rt_tick_get();
        }

        /** 超过一定时间未接收到计费规则需要再次请求 */
        if(s_ycp_assistant_flag.is_verify_billingrule == 0x00){
            if(s_ycp_billing_rule_tick > rt_tick_get()){
                s_ycp_billing_rule_tick = rt_tick_get();
            }
            if((rt_tick_get() - s_ycp_billing_rule_tick) > NET_YCP_REQ_BILLINGRULE_INTERVAL){
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
                s_ycp_billing_rule_tick = rt_tick_get();
            }
        }

        /** 外部触发再次请求计费模型 */
        if(s_ycp_assistant_flag.req_billingrule_again){
            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
            s_ycp_assistant_flag.req_billingrule_again = 0x00;
        }

        /***************************************************** [数据请求] **********************************************************/
        /***************************************************** [数据请求] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint32_t _event = 0;
            ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(!_event){
                continue;   /* 此枪没有请求事件,不进行事件查询 */
            }

            /***** [对时设置] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_TIME_SYNC, NULL) > 0){
                uint32_t timestamp = time(NULL);
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_TIME_SYNC);
                g_ycp_preq_time_sync.head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_timestamp_to_timebcd(timestamp, g_ycp_preq_time_sync.body.current_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
                ycp_message_send_port(NETYCP_PREQCMD_TIME_SYNC, s_ycp_socket_info.fd, &g_ycp_preq_time_sync,
                        sizeof(g_ycp_preq_time_sync));
                ycp_set_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TIME_SYNC);
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_TIME_SYNC);
                rt_thread_mdelay(250);
            }
            /***** [计费模型验证] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR), NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY, NULL) > 0){
                if(s_ycp_assistant_flag.is_timesync){
                    ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY, NULL);

                    ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
                    g_ycp_preq_billing_model_verify.head.sequence = s_ycp_message_serial_number[gunno]++;
                    ycp_message_send_port(NETYCP_PREQCMD_BILLING_MODEL_VERIFY, s_ycp_socket_info.fd, &g_ycp_preq_billing_model_verify,
                            sizeof(g_ycp_preq_billing_model_verify));
                    ycp_set_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
                    ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY);
                    rt_thread_mdelay(250);
                }
            }
            /***** [上报心跳] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_HEARTBEAT, NULL) > 0){
                if(s_ycp_socket_info.heartbeat < 0xFF){
                    s_ycp_socket_info.heartbeat++;
                }
                ycp_request_padding_heartbeat();
                g_ycp_preq_heartbeat.head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_HEARTBEAT, s_ycp_socket_info.fd, &g_ycp_preq_heartbeat,
                        sizeof(g_ycp_preq_heartbeat));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_HEARTBEAT);

                LOG_D("ycp heartbeat timeout count(%d, %d)\n", gunno, s_ycp_socket_info.heartbeat);
                rt_thread_mdelay(250);
            }
            /***** [上报单枪状态数据] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR), NET_YCP_PREQ_EVENT_REPORT_STATE_DATA, NULL) > 0){
                if(s_ycp_assistant_flag.is_verify_billingrule == 0x01){
                    ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_REPORT_STATE_DATA, NULL);
                    ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
                    g_ycp_preq_report_state_data[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                    ycp_message_send_port(NETYCP_PREQCMD_PRESCMD_REPORT_STATE_DATA, s_ycp_socket_info.fd, &g_ycp_preq_report_state_data[gunno],
                            (g_ycp_preq_report_state_data[gunno].head.length + 0x04));
                    ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
                    rt_thread_mdelay(250);
                }
            }
            /***** [充电桩主动申请启动充电] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_APPLY_START_CHARGE, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
                g_ycp_preq_apply_charge_active[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_APPLY_START_CHARGE, s_ycp_socket_info.fd, &g_ycp_preq_apply_charge_active[gunno],
                        sizeof(g_ycp_preq_apply_charge_active[gunno]));
                ycp_set_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
                rt_thread_mdelay(250);
            }
            /***** [交易记录] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_TRANSACTION_RECORD, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
                g_ycp_preq_transaction_records[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_TRANSACTION_RECORD, s_ycp_socket_info.fd, &g_ycp_preq_transaction_records[gunno],
                        sizeof(g_ycp_preq_transaction_records[gunno]));
                ycp_set_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);

                ycp_set_transaction_verify_state(gunno, 0x00);
                if(memcmp(g_ycp_preq_transaction_records[gunno].body.serial_number, s_ycp_current_transaction_number[gunno], NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT)){
                    s_ycp_same_transaction_report_count[gunno] = 0x00;
                    memcpy(s_ycp_current_transaction_number[gunno], g_ycp_preq_transaction_records[gunno].body.serial_number, NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT);
                }else{
                    if(++s_ycp_same_transaction_report_count[gunno] > NET_YCP_SAME_TRANSATION_REPORT_COUNT_MAX){
                        ycp_set_transaction_verify_state(gunno, 0x01);
                        s_ycp_same_transaction_report_count[gunno] = 0x00;
                        ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
                    }
                }

                rt_thread_mdelay(250);
            }
            /***** [上报设备故障] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_REPORT_DEVICE_FAULT, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_REPORT_DEVICE_FAULT);
                g_ycp_preq_report_device_fault.head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_PRESCMD_REPORT_DEVICE_FAULT, s_ycp_socket_info.fd, &g_ycp_preq_report_device_fault,
                        (g_ycp_preq_report_device_fault.head.length + 0x04));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_REPORT_DEVICE_FAULT);
                rt_thread_mdelay(250);
            }
            /***** [充电握手] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_CHARGE_SHAKE_HAND, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_CHARGE_SHAKE_HAND);
                g_ycp_preq_shake_hand[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_CHARGE_SHAKE_HAND, s_ycp_socket_info.fd, &g_ycp_preq_shake_hand[gunno],
                        sizeof(g_ycp_preq_shake_hand[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_CHARGE_SHAKE_HAND);
                rt_thread_mdelay(250);
            }
            /***** [参数配置] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_PARA_CONFIG, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_PARA_CONFIG);
                g_ycp_preq_parameter_config[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_PARA_CONFIG, s_ycp_socket_info.fd, &g_ycp_preq_parameter_config[gunno],
                        sizeof(g_ycp_preq_parameter_config[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_PARA_CONFIG);
                rt_thread_mdelay(250);
            }
            /***** [充电结束] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_CHARGE_END, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_CHARGE_END);
                g_ycp_preq_charge_finish[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_CHARGE_END, s_ycp_socket_info.fd, &g_ycp_preq_charge_finish[gunno],
                        sizeof(g_ycp_preq_charge_finish[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_CHARGE_END);
                rt_thread_mdelay(250);
            }
            /***** [错误报文] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_ERROR_MESSAGE, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_ERROR_MESSAGE);
                g_ycp_preq_error_message[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_ERROR_MESSAGE, s_ycp_socket_info.fd, &g_ycp_preq_error_message[gunno],
                        sizeof(g_ycp_preq_error_message[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_ERROR_MESSAGE);
                rt_thread_mdelay(250);
            }
            /***** [充电过程中 BMS 终止] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_BMS_STOP, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_BMS_STOP);
                g_ycp_preq_bms_end[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_BMS_STOP, s_ycp_socket_info.fd, &g_ycp_preq_bms_end[gunno],
                        sizeof(g_ycp_preq_bms_end[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_BMS_STOP);
                rt_thread_mdelay(250);
            }
            /***** [充电过程中充电机终止] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_CHARGER_STOP, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_CHARGER_STOP);
                g_ycp_preq_charger_end[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_CHARGER_STOP, s_ycp_socket_info.fd, &g_ycp_preq_charger_end[gunno],
                        sizeof(g_ycp_preq_charger_end[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_CHARGER_STOP);
                rt_thread_mdelay(250);
            }
            /***** [充电过程 BMS 需求与充电机输出] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                g_ycp_preq_bmscommand_chargerout[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_CHARGER_OUTPUT_BMS_REQUIRE, s_ycp_socket_info.fd, &g_ycp_preq_bmscommand_chargerout[gunno],
                        sizeof(g_ycp_preq_bmscommand_chargerout[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                rt_thread_mdelay(250);
            }
            /***** [充电过程 BMS 信息] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PREQ_EVENT_BMS_INFO, NULL) > 0){

                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_BMS_INFO);
                g_ycp_preq_bms_info[gunno].head.sequence = s_ycp_message_serial_number[gunno]++;
                ycp_message_send_port(NETYCP_PREQCMD_BMS_INFO, s_ycp_socket_info.fd, &g_ycp_preq_bms_info[gunno],
                        sizeof(g_ycp_preq_bms_info[gunno]));
                ycp_set_message_send_state(gunno, NET_YCP_SEND_STATE_COMPLETE, NET_YCP_PREQ_EVENT_BMS_INFO);
                rt_thread_mdelay(250);
            }
        }
        /***************************************************** [数据响应] **********************************************************/
        /***************************************************** [数据响应] **********************************************************/
        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            uint32_t _event = 0;
            ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
            if(!_event){
                continue;   /* 此枪没有响应事件,不进行事件查询 */
            }
            /***** [查询单枪状态数据响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_QUERY_STATE_DATA, NULL) > 0){

                Net_YcpPro_PRes_Query_PReq_Report_PileState_t *state_data = (Net_YcpPro_PRes_Query_PReq_Report_PileState_t*)(s_ycp_response_buff.general_transmit_buff);
                state_data->head.sequence = g_ycp_sreq_query_device_state[gunno].head.sequence;
                ycp_message_send_port(NETYCP_PREQCMD_PRESCMD_REPORT_STATE_DATA, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [查询所有枪状态数据响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_QUERY_STATE_DATA_ALL, NULL) > 0){

                Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t *state_data = (Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t*)(s_ycp_response_buff.general_transmit_buff);
                state_data->head.sequence = g_ycp_sreq_query_device_state_all.head.sequence;
                ycp_message_send_port(NETYCP_PREQCMD_PRESCMD_REPORT_STATE_DATA_ALL, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台远程控制启机响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_SERVER_START_CHARGE, NULL) > 0){

                Net_YcpPro_PRes_Remote_StartCharge_t *start_charge = (Net_YcpPro_PRes_Remote_StartCharge_t*)(s_ycp_response_buff.general_transmit_buff);
                start_charge->head.sequence = g_ycp_sreq_remote_start_charge[gunno].head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_SERVER_START_CHARGE, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台远程停机响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_SERVER_STOP_CHARGE, NULL) > 0){

                Net_YcpPro_PRes_Remote_StopCharge_t *stop_charge = (Net_YcpPro_PRes_Remote_StopCharge_t*)(s_ycp_response_buff.general_transmit_buff);

                stop_charge->head.sequence = g_ycp_sreq_remote_stop_charge[gunno].head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_SERVER_STOP_CHARGE, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [充电桩参数设置响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_SET_PARA, NULL) > 0){
                Net_YcpPro_PRes_ParaSet_t *work_para = (Net_YcpPro_PRes_ParaSet_t*)(s_ycp_response_buff.general_transmit_buff);
                work_para->head.sequence = g_ycp_sreq_set_para.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_SET_PARA, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [计费模型设置响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_SET_BILLING_MODEL, NULL) > 0){
                Net_YcpPro_PRes_BillingModel_Set_t *billing_model = (Net_YcpPro_PRes_BillingModel_Set_t*)(s_ycp_response_buff.general_transmit_buff);
                billing_model->head.sequence = g_ycp_sreq_billing_model_set.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_BILLING_MODEL_SET, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [远程重启响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_REMOTE_REBOOT, NULL) > 0){

                Net_YcpPro_PRes_RemoteReboot_t *reboot = (Net_YcpPro_PRes_RemoteReboot_t*)(s_ycp_response_buff.general_transmit_buff);
                reboot->head.sequence = g_ycp_sreq_remote_reboot.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_REMOTE_REBOOT, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [远程更新响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_REMOTE_UPDATE, NULL) > 0){

                g_ycp_pres_remote_update.head.sequence = g_ycp_sreq_remote_update.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_REMOTE_UPDATE, s_ycp_socket_info.fd, &g_ycp_pres_remote_update,
                        sizeof(g_ycp_pres_remote_update));
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [运营平台二维码配置响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_QRCODE_CONFIG, NULL) > 0){
                Net_YcpPro_PRes_Qrcode_Config_t *qrcode = (Net_YcpPro_PRes_Qrcode_Config_t*)(s_ycp_response_buff.general_transmit_buff);
                qrcode->head.sequence = g_ycp_sreq_qrcode_config.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_QRCODE_CONFIG, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);
                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [设置客服电话响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_SET_SERVICE_PHONE, NULL) > 0){
                Net_YcpPro_PRes_ServicePhone_t *service_phone = (Net_YcpPro_PRes_ServicePhone_t*)(s_ycp_response_buff.general_transmit_buff);
                service_phone->head.sequence = g_ycp_sreq_set_service_phone.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_SET_SERVER_PHONE, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);

                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [修改联网地址响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_MODIFY_SERVER_ADDR, NULL) > 0){

                Net_YcpPro_PRes_Modify_ServerAddr_t *server_addr = (Net_YcpPro_PRes_Modify_ServerAddr_t*)(s_ycp_response_buff.general_transmit_buff);
                server_addr->head.sequence = g_ycp_sreq_modify_server_addr.head.sequence;
                ycp_message_send_port(NETYCP_PRESCMD_MODIFY_SERVER_ADDR, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);

                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
            /***** [查询设备故障响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_QUERY_DEVICE_FAULT, NULL) > 0){

                Net_YcpPro_PReq_Report_DeviceFault_t *device_fault = (Net_YcpPro_PReq_Report_DeviceFault_t*)(s_ycp_response_buff.general_transmit_buff);
                device_fault->head.sequence = g_ycp_sreq_query_device_fault.head.sequence;
                ycp_message_send_port(NETYCP_PREQCMD_PRESCMD_REPORT_DEVICE_FAULT, s_ycp_socket_info.fd, s_ycp_response_buff.general_transmit_buff,
                        s_ycp_response_buff.length);

                ycp_response_buff_release_sem();
                rt_thread_mdelay(250);
            }
        }

        /***************************************************** [定时上报] **********************************************************/
        /***************************************************** [定时上报] **********************************************************/
        if(s_ycp_assistant_flag.is_verify_billingrule == 0x01){
            uint8_t overreturn = 0x00;
            if(s_ycp_heartbeat_tick > rt_tick_get()){
                overreturn = 0x01;
            }
            /* 数据填报只能是连上网后才行 */
            if((rt_tick_get() + overreturn *0xFFFFFFFF - s_ycp_heartbeat_tick) >=  (1000 *g_ycp_sreq_set_para.body.heartbeat_interval)){
                s_ycp_heartbeat_tick = rt_tick_get();
                ycp_set_message_send_state(0x00, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_HEARTBEAT);
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_HEARTBEAT);
            }
        }else{
            uint8_t overreturn = 0x00;
            if(s_ycp_heartbeat_tick > rt_tick_get()){
                overreturn = 0x01;
            }

            /* 数据填报只能是连上网后才行 */
            if((rt_tick_get() + overreturn *0xFFFFFFFF - s_ycp_heartbeat_tick) >=  (1000 *g_ycp_sreq_set_para.body.heartbeat_interval)){
                s_ycp_heartbeat_tick = rt_tick_get();
                if(s_ycp_socket_info.heartbeat < 0xFF){
                    s_ycp_socket_info.heartbeat++;
                }
            }
        }
        /***************************************************** [内部消耗事件] **********************************************************/
        /***************************************************** [内部消耗事件] **********************************************************/
        /***** [上报心跳响应] *****/
        if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00,
                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_HEARTBEAT, NULL) > 0){
            s_ycp_socket_info.heartbeat = 0x00;
        }

        rt_thread_mdelay(10);
    }
}

static void net_ycp_server_message_pro_entry(void *parameter)
{
    int fd = -1;
    uint8_t attemp_open_rentry = 0x00;
    int16_t result = 0x00;
    uint32_t _event = 0x00, attemp_open_interval = 0x00, wait_attemp_login_interval = 0x00;
    ycp_response_message_buf_t *response = NULL;

    while(1)
    {
        net_thread_running(rt_thread_self(), NULL, 0x00, 0x00);
        if((net_get_ota_info()->state >= NET_OTA_STATE_OPEN_LINK) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }

        if(s_ycp_socket_info.socket_state != YCP_SOCKET_STATE_LOGIN_SUCCESS){   /* 未登录上服务器前不进行网络数据交互事件处理 */
            rt_thread_mdelay(500);
            continue;
        }

        if(s_ycp_assistant_flag.modify_server_addr){
            if(s_ycp_assistant_flag.attemp_open_success == 0x00){
                if(((rt_tick_get() - attemp_open_interval) > 10 *1000) || (s_ycp_assistant_flag.is_attemping_open == 0x00)){
                    fd = -1;
                    if(ycp_socket_open(&fd, (char*)(g_ycp_sreq_modify_server_addr.body.server_addr),  \
                            strlen((char*)(g_ycp_sreq_modify_server_addr.body.server_addr)), g_ycp_sreq_modify_server_addr.body.port) >= 0x00){
                        s_ycp_assistant_flag.attemp_open_success = 0x01;
                        LOG_D("modify server addr open success, addr(%s:%d), fd(%d)", g_ycp_sreq_modify_server_addr.body.server_addr,  \
                                g_ycp_sreq_modify_server_addr.body.port, fd);
                    }else{
                        if(++attemp_open_rentry >= 0x02){
                            s_ycp_assistant_flag.modify_server_addr = 0x00;
                            LOG_D("modify server addr open fail, addr(%s:%d), rentry(%d)", g_ycp_sreq_modify_server_addr.body.server_addr,  \
                                    g_ycp_sreq_modify_server_addr.body.port, attemp_open_rentry);
                        }
                    }
                    s_ycp_assistant_flag.is_attemping_open = 0x01;
                    attemp_open_interval = rt_tick_get();
                }
                s_ycp_assistant_flag.attemp_login_success = 0x00;
            }else{
                if(s_ycp_assistant_flag.attemp_login_success == 0x00){
                    if(s_ycp_assistant_flag.is_attemping_login == 0x00){
                        ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00,
                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_LOGIN, NULL);
                        ycp_message_send_port(NETYCP_PREQCMD_SINGIN, fd, &g_ycp_preq_login, sizeof(g_ycp_preq_login));

                        s_ycp_assistant_flag.is_attemping_login = 0x01;
                        wait_attemp_login_interval = rt_tick_get();
                    }else{
                        if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, 0x00,
                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_LOGIN, NULL) > 0){
                            struct net_handle* handle = net_get_net_handle();
                            uint32_t option = NET_SYSTEM_DATA_OPTION_DATA_CONTENT |NET_SYSTEM_DATA_OPTION_PLAT_YCP;

                            s_ycp_assistant_flag.attemp_login_success = 0x01;
                            s_ycp_assistant_flag.modify_server_addr = 0x00;
                            LOG_D("modify server addr login success");

                            if(handle->set_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, g_ycp_sreq_modify_server_addr.body.server_addr, \
                                    strlen((char*)g_ycp_sreq_modify_server_addr.body.server_addr), option) < 0x00){
                                s_ycp_assistant_flag.attemp_login_success = 0x00;
                                LOG_D("modify server addr data sync fail");
                            }
                            if(s_ycp_assistant_flag.attemp_login_success){
                                if(handle->set_system_data(NET_SYSTEM_DATA_NAME_PORT, (uint8_t*)&(g_ycp_sreq_modify_server_addr.body.port), \
                                        sizeof(g_ycp_sreq_modify_server_addr.body.port), option) < 0x00){
                                    s_ycp_assistant_flag.attemp_login_success = 0x00;
                                    LOG_D("modify server port data sync fail");
                                }
                            }

                            if(handle->system_data_storage(0x00) < 0x00){
                                s_ycp_assistant_flag.attemp_login_success = 0x00;
                                LOG_D("modify server addr port data sync fail...");
                            }

                            if(s_ycp_assistant_flag.attemp_login_success){
                                if(handle->system_data_storage(0x00) < 0x00){
                                    s_ycp_assistant_flag.attemp_login_success = 0x00;
                                    LOG_D("modify server addr port storage fail");
                                }
                            }
                        }

                        if(s_ycp_assistant_flag.attemp_login_success == 0x00){
                            if((rt_tick_get() - wait_attemp_login_interval) > 10 *500){
                                s_ycp_assistant_flag.modify_server_addr = 0x00;
                                LOG_D("modify server addr login fail");
                            }
                        }
                    }
                }
            }
        }else{
            if((s_ycp_assistant_flag.is_attemping_open == 0x01) || (s_ycp_assistant_flag.is_attemping_login == 0x01)){
                s_ycp_assistant_flag.is_attemping_open = 0x00;
                s_ycp_assistant_flag.is_attemping_login = 0x00;

                ycp_socket_close(fd);

                response = ycp_get_response_buff(RT_WAITING_FOREVER);

                result = ycp_response_padding_modify_server_addr(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                ((Net_YcpPro_PRes_Modify_ServerAddr_t*)response->general_transmit_buff)->body.result = 0x00;
                if(s_ycp_assistant_flag.attemp_login_success){
                    ((Net_YcpPro_PRes_Modify_ServerAddr_t*)response->general_transmit_buff)->body.result = 0x01;
                }
                if(result >= 0x00){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_PRES_EVENT_MODIFY_SERVER_ADDR);
                }else{
                    ycp_response_buff_release_sem();
                }
            }
        }

        for(uint8_t gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [读取实时数据请求] *****/ // OK
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_QUERY_DEVICE_STATE, NULL) > 0){
                    response = ycp_get_response_buff(RT_WAITING_FOREVER);
                    result = ycp_response_padding_query_state_data(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_QUERY_STATE_DATA);
                    }else{
                        ycp_response_buff_release_sem();
                    }
                }
                /***** [读取实时数据请求] *****/ // OK
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_QUERY_DEVICE_STATE_ALL, NULL) > 0){
                    response = ycp_get_response_buff(RT_WAITING_FOREVER);
                    result = ycp_response_padding_query_state_data_all(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_QUERY_STATE_DATA_ALL);
                    }else{
                        ycp_response_buff_release_sem();
                    }
                }
                /***** [远程开启充电] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_REMOTE_START_CHARGE, NULL) > 0){
                    uint8_t pro_result, reason;
                    response = ycp_get_response_buff(RT_WAITING_FOREVER);
                    result = ycp_message_pro_remote_start_charge_request(gunno, &g_ycp_sreq_remote_start_charge[gunno], sizeof(g_ycp_sreq_remote_start_charge[gunno]));
                    if(result >= NET_YCP_START_FAIL_CODE_NONE){
                        pro_result = NET_YCP_START_FAIL_CODE_NONE;
                        if(result == NET_YCP_START_FAIL_CODE_NONE){
                            pro_result = 0x01;
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_START_CHARGE);
                            net_operation_clear_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                        }
                        reason = result;
                    }else{
                        pro_result = 0x00;
                        reason = 0x00;
                    }

                    result = ycp_response_padding_remote_start_charge(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    ((Net_YcpPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.result = pro_result;
                    ((Net_YcpPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.reason = reason;
                    if(result >= 0x00){
                        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SERVER_START_CHARGE);
                    }else{
                        ycp_response_buff_release_sem();
                    }
                }
                /***** [远程停机请求] *****/ // OK
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_REMOTE_STOP_CHARGE, NULL) > 0){
                    uint8_t pro_result, reason;
                    response = ycp_get_response_buff(RT_WAITING_FOREVER);
                    result = ycp_message_pro_remote_stop_charge_request(gunno);
                    if(result >= NET_YCP_STOP_FAIL_CODE_NONE){
                        pro_result = NET_YCP_STOP_FAIL_CODE_NONE;
                        if(result == NET_YCP_STOP_FAIL_CODE_NONE){
                            pro_result = 0x01;
                        }
                        reason = result;
                    }else{
                        pro_result = 0x00;
                        reason = NET_YCP_STOP_FAIL_CODE_UNKNOW;
                    }

                    result = ycp_response_padding_remote_stop_charge(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    ((Net_YcpPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.result = pro_result;
                    ((Net_YcpPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.reason = reason;
                    if(result >= 0x00){
                        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SERVER_STOP_CHARGE);
                    }else{
                        ycp_response_buff_release_sem();
                    }
                }
                /***** [充电桩参数设置请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR), NET_YCP_SREQ_EVENT_SET_PARA, NULL) > 0){
                    if(s_ycp_assistant_flag.is_storaging == 0x00){
                        s_ycp_assistant_flag.is_storaging = 0x01;
                        if(ycp_message_pro_set_para_request(&g_ycp_sreq_set_para, sizeof(g_ycp_sreq_set_para)) < 0x00){
                            response = ycp_get_response_buff(RT_WAITING_FOREVER);
                            result = ycp_response_padding_set_work_para(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                            if(result >= 0x00){
                                ((Net_YcpPro_PRes_ParaSet_t*)response->general_transmit_buff)->body.result = 0x00;
                                ((Net_YcpPro_PRes_ParaSet_t*)response->general_transmit_buff)->body.reason = NET_YCP_PARASET_RESULT_FAIL_POWER;
                                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SET_PARA);
                            }else{
                                ycp_response_buff_release_sem();
                            }
                            s_ycp_assistant_flag.is_storaging = 0x00;
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER);
                        }
                        ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_SET_PARA, NULL);
                    }
                }
                /***** [计费模型设置请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR), NET_YCP_SREQ_EVENT_BILLING_MODEL_SET, NULL) > 0){
                    if(s_ycp_assistant_flag.is_storaging == 0x00){
                        s_ycp_assistant_flag.is_storaging = 0x01;

                        uint8_t res = 0x00;
                        response = ycp_get_response_buff(RT_WAITING_FOREVER);
                        if(ycp_message_pro_billing_model_set_response(&g_ycp_sreq_billing_model_set, sizeof(g_ycp_sreq_billing_model_set), 0x00) < 0x00){
                            res = 0x00;
                        }else{
                            res = 0x01;
                        }
                        result = ycp_response_padding_set_billing_model(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YcpPro_PRes_BillingModel_Set_t*)response->general_transmit_buff)->body.result = res;
                        if(result >= 0x00){
                            g_ycp_preq_billing_model_verify.body.model_sn = g_ycp_sreq_billing_model_set.body.model_number;
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SET_BILLING_MODEL);
                        }else{
                            ycp_response_buff_release_sem();
                        }
                        ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_BILLING_MODEL_SET, NULL);
                        s_ycp_assistant_flag.is_storaging = 0x00;
                    }
                }
                /***** [远程重启请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_REMOTE_REBOOT, NULL) > 0){
                    uint8_t res = 0x00;
                    result = ycp_message_pro_remote_reset_request(&g_ycp_sreq_remote_reboot, sizeof(g_ycp_sreq_remote_reboot));
                    if(result > 0x00){
                        res = 0x01;
                    }
                    if(result >= 0x00){
                        response = ycp_get_response_buff(RT_WAITING_FOREVER);
                        result = ycp_response_padding_remote_reboot(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        if(result >= 0x00){
                            net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                            ((Net_YcpPro_PRes_RemoteReboot_t*)response->general_transmit_buff)->body.result = res;
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_REMOTE_REBOOT);
                        }else{
                            ycp_response_buff_release_sem();
                        }
                    }
                }
                /***** [远程更新请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_REMOTE_UPDATE, NULL) > 0){
                    uint8_t pro_result, reason;
                    if(g_ycp_sreq_remote_update.body.update_way == 0x01){
                        if(net_get_ota_info()->state == NET_OTA_STATE_NULL){
                            if(ycp_get_ota_was_requested_flag() == 0x00){
                                ycp_set_ota_was_requested_flag();
                                pro_result = 0x01;
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_START_UPDATE);
                            }else{
                                pro_result = 0x00;
                                reason = 0x01;
                            }
                        }else{
                            pro_result = 0x00;
                            reason = 0x01;
                        }
                    }else{
                        pro_result = 0x00;
                        reason = 0x01;
                        LOG_W("system is not support this update way");
                    }
                    if(pro_result == 0x00){
                        response = ycp_get_response_buff(RT_WAITING_FOREVER);
                        result = ycp_response_padding_remote_update(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YcpPro_PRes_RemoteUpdate_t *)response->general_transmit_buff)->body.result = pro_result;
                        ((Net_YcpPro_PRes_RemoteUpdate_t *)response->general_transmit_buff)->body.result = reason;
                        if(result >= 0x00){
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_REMOTE_UPDATE);
                        }else{
                            ycp_response_buff_release_sem();
                        }
                    }
                }
                /***** [运营平台二维码配置请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR), NET_YCP_SREQ_EVENT_QRCODE_CONFIG, NULL) > 0){
                    if(s_ycp_assistant_flag.is_storaging == 0x00){
                        s_ycp_assistant_flag.is_storaging = 0x01;

                        uint8_t pro_result = 0x00;
                        ycp_qrcode_buf_t *info = (ycp_qrcode_buf_t*)(ycp_get_qrcode_info());

                        info->flag.is_used = 0x00;
                        if(g_ycp_sreq_qrcode_config.body.result == 0x00){
                            uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YCP |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                            struct net_handle* handle = net_get_net_handle();

                            if(handle->set_system_data(NET_SYSTEM_DATA_NAME_QRCODE, info->qrcode, info->length, option) >= 0x00){
                                pro_result = 0x01;
                            }
                        }

                        response = ycp_get_response_buff(RT_WAITING_FOREVER);
                        result = ycp_response_padding_qrcode_config(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YcpPro_PRes_Qrcode_Config_t*)response->general_transmit_buff)->body.result = pro_result;
                        if(result >= 0x00){
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_QRCODE_CONFIG);
                        }else{
                            ycp_response_buff_release_sem();
                        }
                        ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_QRCODE_CONFIG, NULL);
                        s_ycp_assistant_flag.is_storaging = 0x00;
                    }
                }
                /***** [客服电话设置请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR), NET_YCP_SREQ_EVENT_SET_SERVICE_PHONE, NULL) > 0){
                    if(s_ycp_assistant_flag.is_storaging == 0x00){
                        s_ycp_assistant_flag.is_storaging = 0x01;

                        uint8_t pro_result = 0x00;
                        response = ycp_get_response_buff(RT_WAITING_FOREVER);
                        if(g_ycp_sreq_set_service_phone.body.result == 0x00){
                            uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YCP |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
                            struct net_handle* handle = net_get_net_handle();
                            ycp_service_phone_buf_t *phone = (ycp_service_phone_buf_t*)ycp_get_service_phone_info();

                            if(handle->set_system_data(NET_SYSTEM_DATA_NAME_HELP_PHONE, phone->service_phone,
                                    g_ycp_sreq_set_service_phone.body.service_phone_len, option) >= 0x00){
                                pro_result = 0x01;
                            }
                        }

                        result = ycp_response_padding_set_service_phone(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                        ((Net_YcpPro_PRes_ServicePhone_t*)response->general_transmit_buff)->body.result = pro_result;
                        if(result >= 0x00){
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SET_SERVICE_PHONE);
                        }else{
                            ycp_response_buff_release_sem();
                        }
                        ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_SET_SERVICE_PHONE, NULL);
                        s_ycp_assistant_flag.is_storaging = 0x00;
                    }
                }
                /***** [修改联网地址请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_MODIFY_SERVICE_ADDR, NULL) > 0){
                    s_ycp_assistant_flag.modify_server_addr = 0x01;
                    s_ycp_assistant_flag.attemp_open_success = 0x00;
                    s_ycp_assistant_flag.attemp_login_success = 0x00;
                    s_ycp_assistant_flag.is_attemping_open = 0x00;
                    s_ycp_assistant_flag.is_attemping_login = 0x00;

                    attemp_open_rentry = 0x00;
                }
                /***** [查询设备故障请求] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_REQUEST, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SREQ_EVENT_QUERY_DEVICE_FAULT, NULL) > 0){
                    response = ycp_get_response_buff(RT_WAITING_FOREVER);

                    result = ycp_response_padding_query_device_fault(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                    if(result >= 0x00){
                        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_QUERY_DEVICE_FAULT);
                    }else{
                        ycp_response_buff_release_sem();
                    }
                }
            }

            ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno, 0x00, 0x00, &_event);
            if(_event){
                /***** [计费模型请求响应] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YCP_EVENT_OPTION_OR), NET_YCP_SRES_EVENT_BILLING_MODEL_VERIFY, NULL) > 0){
                    if(g_ycp_sres_billing_model_verify.body.result == 0x00){
                        if(s_ycp_assistant_flag.is_storaging == 0x00){
                            s_ycp_assistant_flag.is_storaging = 0x01;

                            memcpy(&(g_ycp_sreq_billing_model_set.body.model_number), &(g_ycp_sres_billing_model_verify.body.model_number), \
                                    (sizeof(g_ycp_sreq_billing_model_set) - sizeof(Net_YcpPro_Head_t)));
                            ycp_message_pro_billing_model_set_response(&g_ycp_sreq_billing_model_set, sizeof(g_ycp_sreq_billing_model_set), 0x00);

                            g_ycp_preq_billing_model_verify.body.model_sn = g_ycp_sres_billing_model_verify.body.model_number;
                            s_ycp_assistant_flag.is_verify_billingrule = 0x01;

                            ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                                                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_BILLING_MODEL_VERIFY, NULL);

                            s_ycp_heartbeat_tick = rt_tick_get();
                            ycp_set_message_send_state(0x00, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_HEARTBEAT);
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_HEARTBEAT);

                            s_ycp_assistant_flag.is_storaging = 0x00;
                        }
                    }else{
                        g_ycp_preq_billing_model_verify.body.model_sn = g_ycp_sres_billing_model_verify.body.model_number;
                        s_ycp_assistant_flag.is_verify_billingrule = 0x01;

                        s_ycp_heartbeat_tick = rt_tick_get();
                        ycp_set_message_send_state(0x00, NET_YCP_SEND_STATE_ONGOING, NET_YCP_PREQ_EVENT_HEARTBEAT);
                        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_HEARTBEAT);

                        ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                                                (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_BILLING_MODEL_VERIFY, NULL);

                    }
                }
                /***** [充电桩主动申请启动充电响应] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_APPLY_CHARGE_ACTIVE, NULL) > 0){
                    if(g_ycp_sres_apply_charge_active[gunno].body.result == 0x01){
                        if(ycp_message_pro_apply_charge_active_response(gunno, &g_ycp_sres_apply_charge_active[gunno], sizeof(g_ycp_sres_apply_charge_active[gunno])) >= 0x00){
                            if(g_ycp_preq_apply_charge_active[gunno].body.start_type == 0x01){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_SUCCESS);
                            }else if(g_ycp_preq_apply_charge_active[gunno].body.start_type == 0x02){
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_PASSWORD_AUTHORITY_SUCCESS);
                            }else{
                                net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS);
                            }
                        }
                    }else{
                        if(g_ycp_preq_apply_charge_active[gunno].body.start_type == 0x01){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_CARDAUTHORITY_FAIL);
                        }else if(g_ycp_preq_apply_charge_active[gunno].body.start_type == 0x02){
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_PASSWORD_AUTHORITY_FAIL);
                        }else{
                            net_operation_set_event(gunno, NET_OPERATION_EVENT_VINAUTHORITY_FAIL);
                        }
                        LOG_W("ycp chargepile apply charge active fail reason(%d)", g_ycp_sres_apply_charge_active[gunno].body.reason);
                    }
                    net_operation_clear_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                }
                /***** [交易记录响应响应] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_BILL_VERIFY, NULL) > 0){
                    ycp_set_transaction_verify_state(gunno, 0x01);
                }
                /***** [对时设置响应] *****/
                if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_SERVER, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                        (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_SRES_EVENT_TIME_SYNC, NULL) > 0){
                    ycp_message_pro_time_sync_response(&g_ycp_sres_time_sync, sizeof(g_ycp_sres_time_sync));
                    s_ycp_assistant_flag.is_timesync = 0x01;
                }
            }
            /***** [启动充电异步响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = ycp_get_response_buff(RT_WAITING_FOREVER);
                result = ycp_response_padding_remote_start_charge(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(ycp_is_start_charge_success(gunno)){
                        ((Net_YcpPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.result = 0x01;
                    }else{
                        ((Net_YcpPro_PRes_Remote_StartCharge_t*)response->general_transmit_buff)->body.result = 0x00;
                    }
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SERVER_START_CHARGE);
                }else{
                    ycp_response_buff_release_sem();
                }
            }
            /***** [停止充电异步响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY, NULL) > 0){
                response = ycp_get_response_buff(RT_WAITING_FOREVER);
                result = ycp_response_padding_remote_stop_charge(gunno, response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(ycp_is_stop_charge_success(gunno)){
                        ((Net_YcpPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.result = 0x01;
                    }else{
                        ((Net_YcpPro_PRes_Remote_StopCharge_t*)response->general_transmit_buff)->body.result = 0x00;
                    }
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SERVER_STOP_CHARGE);
                }else{
                    ycp_response_buff_release_sem();
                }
            }
            /***** [设置功率百分比异步响应] *****/
            if(ycp_net_event_receive(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno,
                    (NET_YCP_EVENT_OPTION_OR |NET_YCP_EVENT_OPTION_CLEAR), NET_YCP_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY, NULL) > 0){
                response = ycp_get_response_buff(RT_WAITING_FOREVER);
                result = ycp_response_padding_set_work_para(response->general_transmit_buff, NET_YCP_GENERA_RESPONSE_BUFF_LENGTH, &(response->length));
                if(result >= 0x00){
                    if(ycp_is_set_power_success()){
                        ((Net_YcpPro_PRes_ParaSet_t*)response->general_transmit_buff)->body.result = 0x01;
                        ((Net_YcpPro_PRes_ParaSet_t*)response->general_transmit_buff)->body.reason = NET_YCP_PARASET_RESULT_SUCCESS;
                    }else{
                        ((Net_YcpPro_PRes_ParaSet_t*)response->general_transmit_buff)->body.result = 0x00;
                        ((Net_YcpPro_PRes_ParaSet_t*)response->general_transmit_buff)->body.reason = NET_YCP_PARASET_RESULT_FAIL_POWER;
                    }
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_SET_PARA);
                }else{
                    ycp_response_buff_release_sem();
                }
                s_ycp_assistant_flag.is_storaging = 0x00;
            }
        }

        if(s_ycp_assistant_flag.is_storaging == 0x01){
            if(s_ycp_wait_storage_tick > rt_tick_get()){
                s_ycp_wait_storage_tick = rt_tick_get();
            }
            if((rt_tick_get() - s_ycp_wait_storage_tick) > (10 *1000)){
                s_ycp_wait_storage_tick = rt_tick_get();
                s_ycp_assistant_flag.is_storaging = 0x00;
            }
        }else{
            s_ycp_wait_storage_tick = rt_tick_get();
        }

        rt_thread_mdelay(10);
    }
}

int32_t ycp_message_send_init(void)
{
    uint8_t entry = 0x03, name[NET_THREAD_MONITOR_NAME_MAX];

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ycp_same_transaction_report_count[gunno] = 0x00;
        s_ycp_transaction_verify[gunno] = 0x00;
#ifdef NET_DESIGNATE_REGION
        s_ycp_message_serial_number[gunno] = 0x00;
        s_ycp_message_send_state[gunno] = 0x00;

        for(uint8_t event = 0x00; event < NET_YCP_EVENT_TYPE_SIZE; event++){
            s_ycp_chargepile_event[event][gunno] = 0x00;
            s_ycp_server_event[event][gunno] = 0x00;
        }
        memset(&s_ycp_current_transaction_number[gunno], 0x00, sizeof(s_ycp_current_transaction_number[gunno]));
#endif /* NET_DESIGNATE_REGION */
    }

#ifdef NET_DESIGNATE_REGION
    s_ycp_heartbeat_tick = 0x00;
    s_ycp_billing_rule_tick = 0x00;
    g_net_target_platform_tick = 0x00;
    s_ycp_wait_storage_tick = 0x00;
    memset(&s_ycp_assistant_flag, 0x00, sizeof(s_ycp_assistant_flag));
    memset(&s_ycp_wait_response, 0x00, sizeof(s_ycp_wait_response));
    memset(&s_ycp_socket_info, 0x00, sizeof(s_ycp_socket_info));
    memset(&s_ycp_response_buff, 0x00, sizeof(s_ycp_response_buff));


    memset(g_ycp_preq_report_state_data, 0x00, sizeof(g_ycp_preq_report_state_data));
    memset(g_ycp_preq_apply_charge_active, 0x00, sizeof(g_ycp_preq_apply_charge_active));
    memset(g_ycp_preq_transaction_records, 0x00, sizeof(g_ycp_preq_transaction_records));
    memset(g_ycp_preq_shake_hand, 0x00, sizeof(g_ycp_preq_shake_hand));
    memset(g_ycp_preq_parameter_config, 0x00, sizeof(g_ycp_preq_parameter_config));
    memset(g_ycp_preq_charge_finish, 0x00, sizeof(g_ycp_preq_charge_finish));
    memset(g_ycp_preq_error_message, 0x00, sizeof(g_ycp_preq_error_message));
    memset(g_ycp_preq_bms_end, 0x00, sizeof(g_ycp_preq_bms_end));
    memset(g_ycp_preq_charger_end, 0x00, sizeof(g_ycp_preq_charger_end));
    memset(g_ycp_preq_bmscommand_chargerout, 0x00, sizeof(g_ycp_preq_bmscommand_chargerout));
    memset(g_ycp_preq_bms_info, 0x00, sizeof(g_ycp_preq_bms_info));

    memset(&g_ycp_preq_login, 0x00, sizeof(g_ycp_preq_login));
    memset(&g_ycp_preq_time_sync, 0x00, sizeof(g_ycp_preq_time_sync));
    memset(&g_ycp_preq_heartbeat, 0x00, sizeof(g_ycp_preq_heartbeat));
    memset(&g_ycp_preq_billing_model_verify, 0x00, sizeof(g_ycp_preq_billing_model_verify));
    memset(&g_ycp_preq_report_device_fault, 0x00, sizeof(g_ycp_preq_report_device_fault));
    memset(&g_ycp_pres_remote_update, 0x00, sizeof(g_ycp_pres_remote_update));
#endif /* NET_DESIGNATE_REGION */

    if(rt_thread_init(&s_ycp_message_send_thread, "ycp_send", net_ycp_message_send_thread_entry, NULL,
            s_ycp_message_send_thread_stack, NET_YCP_MESSAGE_SEND_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ycp message send thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ycp_message_send_thread) != RT_EOK){
        LOG_E("ycp message send thread startup fail, please check");
        return -0x01;
    }

    net_thread_init_hook(&s_ycp_message_send_thread, &entry, sizeof(entry), NET_THREAD_RUNNING_OPTION_ENTRY_MAX);

    memset(name, 0x00, NET_THREAD_MONITOR_NAME_MAX);
    memcpy(name, "yc_pms", strlen("yc_pms"));
    net_thread_init_hook(&s_ycp_message_send_thread, name, strlen((char*)name), NET_THREAD_RUNNING_OPTION_NAME);

    if(rt_thread_init(&s_ycp_server_message_pro_thread, "ycp_server", net_ycp_server_message_pro_entry, NULL,
            s_ycp_server_message_pro_thread_stack, NET_YCP_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ycp server message process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ycp_server_message_pro_thread) != RT_EOK){
        LOG_E("ycp server message process thread startup fail, please check");
        return -0x01;
    }
    net_thread_init_hook(&s_ycp_server_message_pro_thread, &entry, sizeof(entry), NET_THREAD_RUNNING_OPTION_ENTRY_MAX);

    memset(name, 0x00, NET_THREAD_MONITOR_NAME_MAX);
    memcpy(name, "yc_smp", strlen("yc_smp"));
    net_thread_init_hook(&s_ycp_server_message_pro_thread, name, strlen((char*)name), NET_THREAD_RUNNING_OPTION_NAME);

    if(rt_sem_init(&s_ycp_response_buff_sem, "ycp_tbsem", 0x01, RT_IPC_FLAG_PRIO) != RT_EOK){
        LOG_E("ycp response buff sem create fail");
    }
    return 0x00;
}

#endif /* NET_PACK_USING_YCP */

