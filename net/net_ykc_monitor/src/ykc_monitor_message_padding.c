/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#include "ykc_monitor_message_padding.h"
#include "ykc_monitor_message_send.h"
#include "ykc_monitor_message_receive.h"

#include "app_ofsm.h"
#include "chargepile_config.h"
#include "net_operation.h"
#include "thaisenChargModuleLib.h"

#define DBG_TAG "ykc_mrl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YKC_MONITOR

#define YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT           0x05          /* 刚连上网时实时数据上报间隔 */
#define YKC_MONITOR_REALTIME_DATA_INTERVAL_CHARGING       0x0F          /* 充电中实时数据上报间隔  */
#define YKC_MONITOR_REALTIME_DATA_INTERVAL_IDLE           0x05 *60      /* 空闲实时数据上报间隔  */

#define YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE    1536          /* 实时处理线程栈大小 */

#pragma pack(1)

struct ykc_monitor_state_info{
    struct{
        uint8_t state : 4;
        uint8_t connect : 2;
        uint8_t reserve : 2;
    }state;                                       /* 桩状态 */
    uint16_t fault_code;                          /* 故障码 */
};

struct ykc_monitor_flag_info{
    uint8_t is_start_charge : 1;                  /*  已启动充电 */
    uint8_t is_stop_charge : 1;                   /*  已停止充电 */
    uint8_t is_start_mergecharge : 1;             /*  已启动并充充电 */
    uint8_t is_set_power : 1;                     /*  已设置功率百分比 */

    uint8_t start_success : 1;                    /*  启机成功 */
    uint8_t stop_success : 1;                     /*  停机成功 */
    uint8_t mergestart_success : 1;               /*  并充启机成功 */
    uint8_t set_power_success : 1;                /*  设置功率百分比成功 */
};

#pragma pack()

static struct ykc_monitor_flag_info s_ykc_monitor_flag_info[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_ykc_monitor_realtime_data_interval[NET_SYSTEM_GUN_NUMBER];
static uint32_t s_ykc_monitor_realtime_data_count[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_ykc_monitor_local_start_sq;
static struct ykc_monitor_state_info s_ykc_monitor_state_info[NET_SYSTEM_GUN_NUMBER];
static struct rt_thread s_ykc_monitor_realtime_process_thread;
static uint8_t s_ykc_monitor_realtime_process_thread_stack[YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE];
static struct net_handle* s_ykc_monitor_handle = NULL;
static System_BaseData *s_ykc_monitor_base = NULL;

static uint16_t ykc_monitor_chargepile_stop_reason_converted(uint8_t bit, uint8_t stop_in_starting);
static uint8_t ykc_monitor_chargepile_transaction_identity_converted(uint8_t identity);

cp56time2a_monitor_t ykc_monitor_get_cp56time2a_from_timestamp(uint32_t timestamp)
{
    struct tm *_tm;
    time_t _time = timestamp;
    cp56time2a_monitor_t cp56time2a;

    _tm = localtime(&_time);
    memset(&cp56time2a, 0x00, sizeof(cp56time2a));

    cp56time2a.cp56time2a_tm.year  = _tm->tm_year + 1900 - 2000;
    cp56time2a.cp56time2a_tm.month = _tm->tm_mon + 1;
    cp56time2a.cp56time2a_tm.mday  = _tm->tm_mday;
    cp56time2a.cp56time2a_tm.wday  = _tm->tm_wday;
    cp56time2a.cp56time2a_tm.hour  = _tm->tm_hour;
    cp56time2a.cp56time2a_tm.min   = _tm->tm_min;
    cp56time2a.cp56time2a_tm.msec  = _tm->tm_sec * 1000;
    cp56time2a.cp56time2a_tm.iv    = 0;
    cp56time2a.cp56time2a_tm.su    = 0;

    return cp56time2a;
}

uint32_t ykc_monitor_get_timestamp_from_cp56time2a(cp56time2a_monitor_t _cp56time2a)
{
    struct tm _tm;
    uint32_t timestamp = 0;

    memset(&_tm, 0x00, sizeof(struct tm));

    _tm.tm_year = _cp56time2a.cp56time2a_tm.year + 2000 - 1900;
    _tm.tm_mon = _cp56time2a.cp56time2a_tm.month - 1;
    _tm.tm_mday = _cp56time2a.cp56time2a_tm.mday;
    _tm.tm_wday = _cp56time2a.cp56time2a_tm.wday;
    _tm.tm_hour = _cp56time2a.cp56time2a_tm.hour;
    _tm.tm_min = _cp56time2a.cp56time2a_tm.min;
    _tm.tm_sec = _cp56time2a.cp56time2a_tm.msec /1000;

    timestamp = mktime(&_tm);

    return (timestamp - 28800);
}

/*************************************************
 * 函数名      ykc_monitor_get_start_charge_result
 * 功能          启动充电成功
 * **********************************************/
uint8_t ykc_monitor_is_start_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ykc_monitor_flag_info[gunno].start_success;
}

/*************************************************
 * 函数名      ykc_monitor_is_stop_charge_success
 * 功能          停止充电成功
 * **********************************************/
uint8_t ykc_monitor_is_stop_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ykc_monitor_flag_info[gunno].stop_success;
}

/*************************************************
 * 函数名      ykc_monitor_is_start_mergecharge_success
 * 功能          启动并充充电成功
 * **********************************************/
uint8_t ykc_monitor_is_start_mergecharge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ykc_monitor_flag_info[gunno].mergestart_success;
}

/*************************************************
 * 函数名      ykc_monitor_is_set_power_success
 * 功能          设置功率成功
 * **********************************************/
uint8_t ykc_monitor_is_set_power_success(void)
{
    return s_ykc_monitor_flag_info[0x00].set_power_success;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_query_realtime_data
 * 功能          组包：查询实时数据响应
 * **********************************************/
int8_t ykc_monitor_response_padding_query_realtime_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Query_PReq_Report_RealTimeData_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_PRes_Query_PReq_Report_RealTimeData_t *response = NULL;

    response = ((Net_YkcMonitorPro_PRes_Query_PReq_Report_RealTimeData_t*)buf);
    memset(response, 0x00, data_len);
    memcpy(response, &g_ykc_monitor_preq_report_realtime_data[gunno], sizeof(Net_YkcMonitorPro_PRes_Query_PReq_Report_RealTimeData_t));

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_general_message
 * 功能          组包：远程启机响应
 * **********************************************/
int8_t ykc_monitor_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Remote_StartCharge_t);

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
    Net_YkcMonitorPro_PRes_Remote_StartCharge_t *response = NULL;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    response = ((Net_YkcMonitorPro_PRes_Remote_StartCharge_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_start_charge[gunno].body.serial_number);
    valid_len = valid_len > sizeof(response->body.serial_number) ? sizeof(response->body.serial_number) : valid_len;
    memcpy(response->body.serial_number, g_ykc_monitor_sreq_remote_start_charge[gunno].body.serial_number, valid_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_start_charge[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_remote_start_charge[gunno].body.pile_number, valid_len);

    response->body.gunno = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_remote_stop_charge
 * 功能          组包：远程停机响应
 * **********************************************/
int8_t ykc_monitor_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Remote_StopCharge_t);

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
    Net_YkcMonitorPro_PRes_Remote_StopCharge_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Remote_StopCharge_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_stop_charge[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_remote_stop_charge[gunno].body.pile_number, valid_len);

    response->body.gunno = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_account_ballance_update
 * 功能          组包：账户余额更新响应
 * **********************************************/
int8_t ykc_monitor_response_padding_account_ballance_update(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_AccountBallance_Update_t);

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
    Net_YkcMonitorPro_PRes_AccountBallance_Update_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_AccountBallance_Update_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_account_ballance_update[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_account_ballance_update[gunno].body.pile_number, valid_len);

    valid_len = sizeof(g_ykc_monitor_sreq_account_ballance_update[gunno].body.physics_card_number);
    valid_len = valid_len > sizeof(response->body.physics_card_number) ? sizeof(response->body.physics_card_number) : valid_len;
    memcpy(response->body.physics_card_number, g_ykc_monitor_sreq_account_ballance_update[gunno].body.physics_card_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_sync_offline_card
 * 功能          组包：同步离线卡响应
 * **********************************************/
int8_t ykc_monitor_response_padding_sync_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Sync_OfflineCard_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_Sync_OfflineCard_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Sync_OfflineCard_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_sync_offline_card.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_sync_offline_card.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_clear_offline_card
 * 功能          组包：清除离线卡响应
 * **********************************************/
int8_t ykc_monitor_response_padding_clear_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Clear_OfflineCard_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_Clear_OfflineCard_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Clear_OfflineCard_t*)buf);
    memset(response, 0x00, sizeof(Net_YkcMonitorPro_PRes_Clear_OfflineCard_t));

    valid_len = sizeof(g_ykc_monitor_sreq_clear_offline_card.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_clear_offline_card.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_query_offline_card
 * 功能          组包：查询离线卡响应
 * **********************************************/
int8_t ykc_monitor_response_padding_query_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Query_OfflineCard_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_Query_OfflineCard_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Query_OfflineCard_t*)buf);
    memset(response, 0x00, sizeof(Net_YkcMonitorPro_PRes_Query_OfflineCard_t));

    valid_len = sizeof(g_ykc_monitor_sreq_query_offline_card.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_query_offline_card.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_set_work_para
 * 功能          组包：设置工作参数响应
 * **********************************************/
int8_t ykc_monitor_response_padding_set_work_para(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Set_WorkPara_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_Set_WorkPara_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Set_WorkPara_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_set_work_para.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_set_work_para.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_time_sync
 * 功能          组包：对时设置响应
 * **********************************************/
int8_t ykc_monitor_response_padding_time_sync(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_TimeSync_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_TimeSync_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_TimeSync_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_time_sync.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_time_sync.body.pile_number, valid_len);
    response->body.current_time = ykc_monitor_get_cp56time2a_from_timestamp(time(NULL));

    rt_kprintf("ykc_monitor_response_padding_time_sync(%d)[%d, %d, %d, %d, %d, %d]\n", time(NULL),
            response->body.current_time.cp56time2a_tm.year, response->body.current_time.cp56time2a_tm.month,
            response->body.current_time.cp56time2a_tm.mday, response->body.current_time.cp56time2a_tm.hour,
            response->body.current_time.cp56time2a_tm.min, response->body.current_time.cp56time2a_tm.msec /1000);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_remote_reboot
 * 功能          组包：远程重启响应
 * **********************************************/
int8_t ykc_monitor_response_padding_remote_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_RemoteReboot_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_RemoteReboot_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_RemoteReboot_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_reboot.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_remote_reboot.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_set_billing_model
 * 功能          组包：设置计费模型响应
 * **********************************************/
int8_t ykc_monitor_response_padding_set_billing_model(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_BillingModel_Set_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_BillingModel_Set_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_BillingModel_Set_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_billing_model_set.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_billing_model_set.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_ground_lock_lifting
 * 功能          组包：地锁升降控制响应
 * **********************************************/
int8_t ykc_monitor_response_padding_ground_lock_lifting(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_GroundLock_Lifting_t);

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
    Net_YkcMonitorPro_PRes_GroundLock_Lifting_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_GroundLock_Lifting_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_ground_lock_lifting[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_ground_lock_lifting[gunno].body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_remote_update
 * 功能          组包：远程更新响应
 * **********************************************/
int8_t ykc_monitor_response_padding_remote_update(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_RemoteUpdate_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_RemoteUpdate_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_RemoteUpdate_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_update.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_remote_update.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_remote_start_merge_charge
 * 功能          组包：远程并充启机响应
 * **********************************************/
int8_t ykc_monitor_response_padding_remote_start_merge_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Remote_StartMergeCharge_t);

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
    Net_YkcMonitorPro_PRes_Remote_StartMergeCharge_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Remote_StartMergeCharge_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.serial_number);
    valid_len = valid_len > sizeof(response->body.serial_number) ? sizeof(response->body.serial_number) : valid_len;
    memcpy(response->body.serial_number, g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.serial_number, valid_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.pile_number, valid_len);

    response->body.main_auxiliary_gun_flag = NET_ENUM_FALSE;
    memcpy(response->body.merge_charge_sn, g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.merge_charge_sn, NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT);

    response->body.gunno = gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_qrcode_config_gc
 * 功能          组包：二维码配置响应(国充)
 * **********************************************/
int8_t ykc_monitor_response_padding_qrcode_config_gc(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Qrcode_Config_GC_t);

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
    Net_YkcMonitorPro_PRes_Qrcode_Config_GC_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Qrcode_Config_GC_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_qrcode_config_gc[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_qrcode_config_gc[gunno].body.pile_number, valid_len);

    response->body.gunno = g_ykc_monitor_sreq_qrcode_config_gc[gunno].body.gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_qrcode_config_tld
 * 功能          组包：二维码配置响应(特来电)
 * **********************************************/
int8_t ykc_monitor_response_padding_qrcode_config_tld(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Qrcode_Config_Tld_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, option));
    Net_YkcMonitorPro_PRes_Qrcode_Config_Tld_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Qrcode_Config_Tld_t*)buf);
    memset(response, 0x00, data_len);

    ykc_monitor_ascii_to_bcd(pile_number, strlen((char*)pile_number), response->body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    response->body.gunno = g_ykc_monitor_sreq_qrcode_config_tld[gunno].body.gunno;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_qrcode_config_ykc15
 * 功能          组包：二维码配置响应(云快充1.5)
 * **********************************************/
int8_t ykc_monitor_response_padding_qrcode_config_ykc15(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_PRes_Qrcode_Config_Ykc15_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_PRes_Qrcode_Config_Ykc15_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Qrcode_Config_Ykc15_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_qrcode_config_ykc15.body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_qrcode_config_ykc15.body.pile_number, valid_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}


/*************************************************
 * 函数名      ykc_monitor_message_pro_billing_model_set_response
 * 功能          处理服务器响应(下发)的计费模型
 * **********************************************/
int8_t ykc_monitor_message_pro_billing_model_set_response(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SRes_BillingModel_Request_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YkcMonitorPro_SRes_BillingModel_Request_t *request = (Net_YkcMonitorPro_SRes_BillingModel_Request_t*)data;

    for(uint8_t _gunno = 0x00; _gunno < NET_SYSTEM_GUN_NUMBER; _gunno++){
        uint8_t gunno = _gunno;
        s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
        if((s_ykc_monitor_base->state.current == APP_OFSM_STATE_CHARGING) || (s_ykc_monitor_base->state.current == APP_OFSM_STATE_STARTING)){
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
            app_billingrule_set_period_rate_number(gunno, period, request->body.rate_number[period /0x02]);
            switch(request->body.rate_number[period /0x02]){
            case APP_RATE_TYPE_SHARP :
                app_billingrule_set_period_elect_price(gunno, period, request->body.tip_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, request->body.tip_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_PEAK :
                app_billingrule_set_period_elect_price(gunno, period, request->body.peak_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, request->body.peak_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_FLAT :
                app_billingrule_set_period_elect_price(gunno, period, request->body.flat_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, request->body.flat_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_VALLEY :
                app_billingrule_set_period_elect_price(gunno, period, request->body.valley_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, request->body.valley_service_rate /10);
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
 * 函数名      ykc_monitor_message_pro_apply_charge_active
 * 功能          处理服务器响应的主动申请充电
 * **********************************************/
int8_t ykc_monitor_message_pro_apply_charge_active_response(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SRes_ApplyCharge_Active_t);

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
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    Net_YkcMonitorPro_SRes_ApplyCharge_Active_t *request = (Net_YkcMonitorPro_SRes_ApplyCharge_Active_t*)data;

    valid_len = sizeof(request->body.logic_card_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->card_number) ? sizeof(s_ykc_monitor_base->card_number) : valid_len;
    memset(s_ykc_monitor_base->card_number, 0x00, sizeof(s_ykc_monitor_base->card_number));
    memcpy(s_ykc_monitor_base->card_number, request->body.logic_card_number, valid_len);

    valid_len = sizeof(request->body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(s_ykc_monitor_base->transaction_number, 0x00, sizeof(s_ykc_monitor_base->transaction_number));
    memcpy(s_ykc_monitor_base->transaction_number, request->body.serial_number, valid_len);

    valid_len = sizeof(s_ykc_monitor_base->card_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->user_number) ? sizeof(s_ykc_monitor_base->user_number) : valid_len;
    memset(s_ykc_monitor_base->user_number, 0x00, sizeof(s_ykc_monitor_base->user_number));
    memcpy(s_ykc_monitor_base->user_number, s_ykc_monitor_base->card_number, valid_len);

    s_ykc_monitor_base->account_balance = request->body.account_ballance;

    s_ykc_monitor_base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
    s_ykc_monitor_base->charge_strategy_para = request->body.account_ballance *100;
    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_remote_start_charge_request
 * 功能          处理服务器下发的远程启机
 * **********************************************/
int8_t ykc_monitor_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_Remote_StartCharge_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return NET_YKC_MONITOR_START_FAIL_REASON_BILLING;
    }

    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    Net_YkcMonitorPro_SReq_Remote_StartCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StartCharge_t*)data;

    /** 并充时不能让平台启动副枪 */
    if(s_ykc_monitor_base->charge_way == APP_CHARGE_WAY_PARACHARGE){
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
    }

    switch(s_ykc_monitor_base->state.current){
    case APP_OFSM_STATE_IDLEING:
        return NET_YKC_MONITOR_START_FAIL_REASON_NO_GUN;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        valid_len = sizeof(request->body.logic_card_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->card_number) ? sizeof(s_ykc_monitor_base->card_number) : valid_len;
        memset(s_ykc_monitor_base->card_number, 0x00, sizeof(s_ykc_monitor_base->card_number));
        memcpy(s_ykc_monitor_base->card_number, request->body.logic_card_number, valid_len);

        valid_len = sizeof(request->body.physics_card_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->card_uid) ? sizeof(s_ykc_monitor_base->card_uid) : valid_len;
        memset(s_ykc_monitor_base->card_uid, 0x00, sizeof(s_ykc_monitor_base->card_uid));
        memcpy(s_ykc_monitor_base->card_uid, request->body.physics_card_number, valid_len);

        valid_len = sizeof(request->body.serial_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
        memset(s_ykc_monitor_base->transaction_number, 0x00, sizeof(s_ykc_monitor_base->transaction_number));
        memcpy(s_ykc_monitor_base->transaction_number, request->body.serial_number, valid_len);

        valid_len = sizeof(s_ykc_monitor_base->card_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->user_number) ? sizeof(s_ykc_monitor_base->user_number) : valid_len;
        memset(s_ykc_monitor_base->user_number, 0x00, sizeof(s_ykc_monitor_base->user_number));
        memcpy(s_ykc_monitor_base->user_number, s_ykc_monitor_base->card_number, valid_len);

        s_ykc_monitor_base->account_balance = request->body.account_ballance;

        s_ykc_monitor_base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
        s_ykc_monitor_base->charge_strategy_para = request->body.account_ballance *100;

        s_ykc_monitor_flag_info[gunno].is_start_charge = NET_ENUM_TRUE;

        return NET_YKC_MONITOR_START_FAIL_REASON_NO;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
        break;
    default:
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_FAULTING;
        break;
    }
    return NET_YKC_MONITOR_START_FAIL_REASON_NO;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_remote_stop_charge_request
 * 功能          处理服务器下发的远程停机
 * **********************************************/
int8_t ykc_monitor_message_pro_remote_stop_charge_request(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }

    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    /** 并充时不能让平台停止副枪 */
    if(s_ykc_monitor_base->charge_way == APP_CHARGE_WAY_PARACHARGE){
        if(gunno != s_ykc_monitor_base->main_gunno){
            return NET_YKC_MONITOR_STOP_FAIL_REASON_NOT_CHARGING;
        }
    }

    if((s_ykc_monitor_base->state.current == APP_OFSM_STATE_CHARGING) ||
            (s_ykc_monitor_base->state.current == APP_OFSM_STATE_STARTING)){
        s_ykc_monitor_flag_info[gunno].is_stop_charge = 0x01;
        return NET_YKC_MONITOR_STOP_FAIL_REASON_NO;
    }
    return NET_YKC_MONITOR_STOP_FAIL_REASON_NOT_CHARGING;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_account_ballance_update_request
 * 功能          处理服务器下发的账户余额更新
 * **********************************************/
int8_t ykc_monitor_message_pro_account_ballance_update_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_AccountBallance_Update_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_SReq_AccountBallance_Update_t *request = (Net_YkcMonitorPro_SReq_AccountBallance_Update_t*)data;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    s_ykc_monitor_base->account_balance = request->body.account_amount;

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_set_work_para_request
 * 功能          处理服务器下发的设置桩工作参数请求
 * **********************************************/
int8_t ykc_monitor_message_pro_set_work_para_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_Set_WorkPara_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YkcMonitorPro_SReq_Set_WorkPara_t *request = (Net_YkcMonitorPro_SReq_Set_WorkPara_t*)data;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(0x00));

    rt_kprintf("ykc_message_pro_set_work_para_request(%d, %d, %d)\n", request->body.power_max_percent,
            s_ykc_monitor_base->system_power_max, (s_ykc_monitor_base->system_power_max * request->body.power_max_percent /100));
    if((request->body.power_max_percent > 0x00) && (request->body.power_max_percent <= 0x64)){
        uint32_t power = (s_ykc_monitor_base->system_power_max * request->body.power_max_percent /100);
        net_operation_set_total_power(power, 0x00);
        s_ykc_monitor_flag_info[0x00].is_set_power = NET_ENUM_TRUE;
        s_ykc_monitor_base->power_strategy = APP_POWER_STRATEGY_SET_LIMIT;
        s_ykc_monitor_base->power_strategy_para = 0x00;
    }else{
        return -0x03;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_time_sync_request
 * 功能          处理服务器下发的对时请求
 * **********************************************/
int8_t ykc_monitor_message_pro_time_sync_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_TimeSync_t);
    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YkcMonitorPro_SReq_TimeSync_t *request = (Net_YkcMonitorPro_SReq_TimeSync_t*)data;
    uint32_t timestamp = ykc_monitor_get_timestamp_from_cp56time2a(request->body.current_time);
    s_ykc_monitor_handle->time_sync(timestamp + 28800);

    rt_kprintf("ykc_monitor_message_pro_time_sync_request(%d)[%d, %d, %d, %d, %d, %d]\n", timestamp,
            request->body.current_time.cp56time2a_tm.year, request->body.current_time.cp56time2a_tm.month,
            request->body.current_time.cp56time2a_tm.mday, request->body.current_time.cp56time2a_tm.hour,
            request->body.current_time.cp56time2a_tm.min, request->body.current_time.cp56time2a_tm.msec /1000);

    net_operation_set_event(0x00, NET_OPERATION_EVENT_TIME_SYNC);

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_remote_reset_request
 * 功能          处理服务器的远程重启请求
 * **********************************************/
int8_t ykc_monitor_message_pro_remote_reset_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_RemoteReboot_t);

    if(data_len > len){
        return -0x02;
    }

    uint8_t gunno = 0x00;
    Net_YkcMonitorPro_SReq_RemoteReboot_t *request = (Net_YkcMonitorPro_SReq_RemoteReboot_t*)data;

    if(request->body.control_cmd == 0x01){
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            if(s_ykc_monitor_base->state.current != APP_OFSM_STATE_IDLEING){
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
 * 函数名      ykc_monitor_message_pro_apply_merge_charge_active_response
 * 功能          处理服务器响应的主动申请并充充电
 * **********************************************/
int8_t ykc_monitor_message_pro_apply_merge_charge_active_response(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t);

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
    Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t *request = (Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t*)data;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    valid_len = sizeof(request->body.logic_card_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->card_number) ? sizeof(s_ykc_monitor_base->card_number) : valid_len;
    memset(&(s_ykc_monitor_base->card_number), 0x00, sizeof(s_ykc_monitor_base->card_number));
    memcpy(&(s_ykc_monitor_base->card_number), request->body.logic_card_number, valid_len);

    valid_len = sizeof(request->body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(&(s_ykc_monitor_base->transaction_number), 0x00, sizeof(s_ykc_monitor_base->transaction_number));
    memcpy(&(s_ykc_monitor_base->transaction_number), request->body.serial_number, valid_len);

    valid_len = sizeof(s_ykc_monitor_base->card_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->user_number) ? sizeof(s_ykc_monitor_base->user_number) : valid_len;
    memset(s_ykc_monitor_base->user_number, 0x00, sizeof(s_ykc_monitor_base->user_number));
    memcpy(s_ykc_monitor_base->user_number, s_ykc_monitor_base->card_number, valid_len);
    s_ykc_monitor_base->account_balance = request->body.account_ballance;

    s_ykc_monitor_base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
    s_ykc_monitor_base->charge_strategy_para = request->body.account_ballance *100;
    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_remote_start_merge_charge_request
 * 功能          处理服务器下发的远程并充启机
 * **********************************************/
int8_t ykc_monitor_message_pro_remote_start_merge_charge_request(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return NET_YKC_MONITOR_START_FAIL_REASON_BILLING;
    }

    uint8_t valid_len = 0x00;
    Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t*)data;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    /** 并充时不能让平台启动副枪 */
    if(s_ykc_monitor_base->charge_way == APP_CHARGE_WAY_PARACHARGE){
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
    }

    switch(s_ykc_monitor_base->state.current){
    case APP_OFSM_STATE_IDLEING:
        return NET_YKC_MONITOR_START_FAIL_REASON_NO_GUN;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        valid_len = sizeof(request->body.logic_card_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->card_number) ? sizeof(s_ykc_monitor_base->card_number) : valid_len;
        memset(&(s_ykc_monitor_base->card_number), 0x00, sizeof(s_ykc_monitor_base->card_number));
        memcpy(&(s_ykc_monitor_base->card_number), request->body.logic_card_number, valid_len);

        valid_len = sizeof(request->body.physics_card_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->card_uid) ? sizeof(s_ykc_monitor_base->card_uid) : valid_len;
        memset(&(s_ykc_monitor_base->card_uid), 0x00, sizeof(s_ykc_monitor_base->card_uid));
        memcpy(&(s_ykc_monitor_base->card_uid), request->body.physics_card_number, valid_len);

        valid_len = sizeof(request->body.serial_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
        memset(&(s_ykc_monitor_base->transaction_number), 0x00, sizeof(s_ykc_monitor_base->transaction_number));
        memcpy(&(s_ykc_monitor_base->transaction_number), request->body.serial_number, valid_len);

        valid_len = sizeof(s_ykc_monitor_base->card_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->user_number) ? sizeof(s_ykc_monitor_base->user_number) : valid_len;
        memset(s_ykc_monitor_base->user_number, 0x00, sizeof(s_ykc_monitor_base->user_number));
        memcpy(s_ykc_monitor_base->user_number, s_ykc_monitor_base->card_number, valid_len);
        s_ykc_monitor_base->account_balance = request->body.account_ballance;

        s_ykc_monitor_base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
        s_ykc_monitor_base->charge_strategy_para = request->body.account_ballance *100;

        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_TRUE;

        return NET_YKC_MONITOR_START_FAIL_REASON_NO;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
        break;
    default:
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_FAULTING;
        break;
    }
    return NET_YKC_MONITOR_START_FAIL_REASON_NO;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_message_info_init
 * 功能          云快充报文信息初始化
 * **********************************************/
void ykc_monitor_message_info_init(uint8_t gunno)  ///////// 这是网络部分外部调用的第一个函数(可以在里面进行相关初始化)
{
    static uint8_t ykc_monitor_is_init = NET_ENUM_FALSE;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(ykc_monitor_is_init){
        return;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = NULL;
    s_ykc_monitor_handle = net_get_net_handle();
    pile_number = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, option));
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    ykc_monitor_is_init = NET_ENUM_TRUE;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_monitor_realtime_data_interval[gunno] = YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT;
        s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
    }
    memset(&s_ykc_monitor_flag_info, 0x00, sizeof(s_ykc_monitor_flag_info));

    /** 初始化登录签到 */
    g_ykc_monitor_preq_login.head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    g_ykc_monitor_preq_login.head.sequence = 0x00;

    ykc_monitor_ascii_to_bcd(pile_number, strlen((char*)pile_number), g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    g_ykc_monitor_preq_login.body.pile_type = NET_YKC_MONITOR_PILE_TYPE_DC;
    g_ykc_monitor_preq_login.body.gun_count = NET_SYSTEM_GUN_NUMBER;
    g_ykc_monitor_preq_login.body.protocol_ver = NET_YKC_MONITOR_PROTOCOL_VERSION;

    memset(g_ykc_monitor_preq_login.body.software_ver, '\0', sizeof(g_ykc_monitor_preq_login.body.software_ver));
    g_ykc_monitor_preq_login.body.software_ver[0] = s_ykc_monitor_base->soft_ver_main + '0';
    g_ykc_monitor_preq_login.body.software_ver[1] = '.';
    g_ykc_monitor_preq_login.body.software_ver[2] = s_ykc_monitor_base->soft_ver_sub + '0';
    g_ykc_monitor_preq_login.body.software_ver[3] = '.';
    sprintf((char *)&g_ykc_monitor_preq_login.body.software_ver[3 + 1], "%2d", s_ykc_monitor_base->soft_ver_revise);

    g_ykc_monitor_preq_login.body.net_link_type = NET_YKC_MONITOR_NET_LINK_TYPE_SIM;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        /** 初始化心跳请求 */
        g_ykc_monitor_preq_heartbeat[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_heartbeat[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_heartbeat[gunno].body.gunno = gunno + 0x01;
        g_ykc_monitor_preq_heartbeat[gunno].body.state = 0x00;     /* 心跳状态默认正常 */

        /** 初始化读取实时数据响应(实时数据请求) */
        g_ykc_monitor_preq_report_realtime_data[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_report_realtime_data[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_report_realtime_data[gunno].body.gunno = gunno + 0x01;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault = 0x00;

        /** 初始化充电握手请求 */
        g_ykc_monitor_preq_shake_hand[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_shake_hand[gunno].body.gunno = gunno + 0x01;

        /** 初始化参数配置请求 */
        g_ykc_monitor_preq_parameter_config[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_parameter_config[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_parameter_config[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电结束请求 */
        g_ykc_monitor_preq_charge_finish[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_charge_finish[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_charge_finish[gunno].body.gunno = gunno + 0x01;

        /** 初始化错误报文请求 */
        g_ykc_monitor_preq_error_message[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_error_message[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_error_message[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程中 BMS 终止请求 */
        g_ykc_monitor_preq_bms_end[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_bms_end[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_bms_end[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程中充电机终止请求 */
        g_ykc_monitor_preq_charger_end[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_charger_end[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_charger_end[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程 BMS 需求与充电机输出请求 */
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程 BMS 信息请求 */
        g_ykc_monitor_preq_bms_info[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_bms_info[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_bms_info[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电桩主动申请启动充电请求 */
        g_ykc_monitor_preq_apply_charge_active[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_apply_charge_active[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_apply_charge_active[gunno].body.gunno = gunno + 0x01;

        /** 初始化交易记录请求 */
        g_ykc_monitor_preq_transaction_records[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_transaction_records[gunno].body.gunno = gunno + 0x01;

        /** 初始化地锁数据上送请求 */
        g_ykc_monitor_preq_ground_lock_info[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_ground_lock_info[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_ground_lock_info[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电桩主动申请并充充电请求 */
        g_ykc_monitor_preq_apply_merge_charge_active[gunno].head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
        memcpy(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.gunno = gunno + 0x01;

    }
    /** 初始化计费模型验证请求 */
    g_ykc_monitor_preq_billing_model_verify.head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    memcpy(g_ykc_monitor_preq_billing_model_verify.body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    g_ykc_monitor_preq_billing_model_verify.body.model_number = 0x00;

    /** 初始化计费模型请求 */
    g_ykc_monitor_preq_billing_model_request.head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    memcpy(g_ykc_monitor_preq_billing_model_request.body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    /** 初始化升级结果响应 */
    g_ykc_monitor_pres_remote_update.head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    memcpy(g_ykc_monitor_pres_remote_update.body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_realtime_data
 * 功能          充电桩请求报文填报：实时数据
 * **********************************************/
void ykc_monitor_chargepile_request_padding_realtime_data(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(is_init){
        uint8_t valid_len = sizeof(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

        g_ykc_monitor_preq_report_realtime_data[gunno].body.homing = 0x02;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.output_voltage = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.output_current = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.soc = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.battery_group_temp_max = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_time = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.remain_time = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_elect = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.loss_elect = 0x00;
        g_ykc_monitor_preq_report_realtime_data[gunno].body.consume_amount = 0x00;

        ykc_monitor_chargepile_request_padding_bmscommand_chargerout(gunno, NET_ENUM_TRUE);
        ykc_monitor_chargepile_request_padding_bmsinfo_duringcharge(gunno, NET_ENUM_TRUE);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            if((s_ykc_monitor_base->state.current == APP_OFSM_STATE_CHARGING) || (s_ykc_monitor_base->state.current == APP_OFSM_STATE_STARTING)){
                struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_voltage = s_ykc_monitor_base->voltage_a /10;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_current = s_ykc_monitor_base->current_a /10;
                if(s_ykc_monitor_base->gunline_temperature[0] > s_ykc_monitor_base->gunline_temperature[1]){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = (s_ykc_monitor_base->gunline_temperature[0] /10 + 50);
                }else{
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = (s_ykc_monitor_base->gunline_temperature[1] /10 + 50);
                }
                g_ykc_monitor_preq_report_realtime_data[gunno].body.soc = s_ykc_monitor_base->current_soc;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.battery_group_temp_max = (bms->BSM.HigTemp + 50);
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_time = s_ykc_monitor_base->charge_time /60;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.remain_time = bms->BCS.SurplChgTime;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_elect = s_ykc_monitor_base->elect_a *10;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.loss_elect = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.consume_amount = s_ykc_monitor_base->fees_total;
            }else{
                g_ykc_monitor_preq_report_realtime_data[gunno].body.homing = 0x02;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_voltage = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_current = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.soc = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.battery_group_temp_max = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_time = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.remain_time = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_elect = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.loss_elect = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.consume_amount = 0x00;
            }
        }
    }
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_bms_shakehand
 * 功能          充电桩请求报文填报：BMS握手
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bms_shakehand(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.bms_protocol_ver, bms->BRM.BMSVer, sizeof(bms->BRM.BMSVer));
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_type = bms->BRM.BatType;
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_rated_capacity = bms->BRM.BatRateCap;
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_rated_volt = bms->BRM.BatRateVolt;
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_maker_name, bms->BRM.BatFirm, sizeof(bms->BRM.BatFirm));
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_sn, bms->BRM.SerialNum, sizeof(bms->BRM.SerialNum));
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_date_year = bms->BRM.BatBuldyear;
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_date_month = bms->BRM.BatBuldmonth;
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_date_day = bms->BRM.BatBuldday;
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_charge_num, bms->BRM.Chagtimer, sizeof(bms->BRM.Chagtimer));
    g_ykc_monitor_preq_shake_hand[gunno].body.bms_bat_title_identification = bms->BRM.BatProperty;
    g_ykc_monitor_preq_shake_hand[gunno].body.reserve = 0x00;
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.vin, bms->BRM.CarDiscern, NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX);
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.bms_software_ver, bms->BRM.BMSVerNum, sizeof(bms->BRM.BMSVerNum));

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGE_SHAKE_HAND);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_bms_paraconfig
 * 功能          充电桩请求报文填报：BMS参数配置
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bms_paraconfig(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

    g_ykc_monitor_preq_parameter_config[gunno].body.bms_single_bat_allow_volt_max = bms->BCP.CellAlowHigVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_curr_max = (4000 - bms->BCP.AlowCurlt);
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_bat_nominal_energy_all = bms->BCP.BatRateKW;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_volt_max = bms->BCP.BatAlowHigVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_temp_max = (bms->BCP.BatAlowHigTemp + 50);
    g_ykc_monitor_preq_parameter_config[gunno].body.soc = bms->BCP.SOC;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_bat_current_volt = bms->BCP.BatVolt;

    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_volt_max = bms->CML.ChagHigOutVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_volt_min = bms->CML.ChagLowOutVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_curr_max = (4000 - bms->CML.ChagHigOutCurlt);
    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_curr_min = (4000 - bms->CML.ChagLowOutCurlt);

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_PARA_CONFIG);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_bms_chargeend
 * 功能          充电桩请求报文填报：BMS充电结束
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bms_chargeend(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

    g_ykc_monitor_preq_charge_finish[gunno].body.bms_end_soc = bms->BSD.StopSOC;
    g_ykc_monitor_preq_charge_finish[gunno].body.bms_single_bat_volt_min = bms->BSD.CellLowVolt;
    g_ykc_monitor_preq_charge_finish[gunno].body.bms_single_bat_volt_max = bms->BSD.CellHigVolt;
    g_ykc_monitor_preq_charge_finish[gunno].body.bms_bat_temp_min = (bms->BSD.LowTemp + 50);
    g_ykc_monitor_preq_charge_finish[gunno].body.bms_bat_temp_max = (bms->BSD.HigTemp + 50);
    g_ykc_monitor_preq_charge_finish[gunno].body.total_charge_time = bms->CSD.TotalChgTime;
    g_ykc_monitor_preq_charge_finish[gunno].body.charge_elect = bms->CSD.OutputKWh;
    memcpy(g_ykc_monitor_preq_charge_finish[gunno].body.charger_number, bms->CSD.ChgNum, sizeof(bms->CSD.ChgNum));

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGE_END);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_bms_error
 * 功能          充电桩请求报文填报：BMS错误报文
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bms_error(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_error_message[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_error_message[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_error_message[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_error_message[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

    g_ykc_monitor_preq_error_message[gunno].body.timeout.spn2560_00_identify = bms->BEM.CRM00OVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.spn2560_aa_identify = bms->BEM.CRMAAOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.reserve_1 = 0x00;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.charger_sync_and_output_max = bms->BEM.CTSCMLOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.charger_charge_ready_ok = bms->BEM.CROOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.reserve_2 = 0x00;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.charger_charge_state = bms->BEM.CCSOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.charger_end_charge = bms->BEM.CSTOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.reserve_3 = 0x00;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.charger_charge_statistics = bms->BEM.CSDOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bms_others = 0x00;

    g_ykc_monitor_preq_error_message[gunno].body.timeout.bms_and_car = bms->CEM.BRMOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.reserve_4 = 0x00;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bat_parameter = bms->CEM.BCPOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bms_charge_ready_ok = bms->CEM.BROOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.reserve_5 = 0x00;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bat_state_all = bms->CEM.BCSOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bat_charge_conmand = bms->CEM.BCLOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bms_end_charge = bms->CEM.BSTOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.reserve_6 = 0x00;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.bms_charge_count = bms->CEM.BSDOVtime;
    g_ykc_monitor_preq_error_message[gunno].body.timeout.charger_others = 0x00;

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_ERROR_MESSAGE);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_bmsend_duringcharge
 * 功能          充电桩请求报文填报：充电过程中 BMS 终止
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bmsend_duringcharge(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_bms_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_bms_end[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bms_end[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_bms_end[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

    g_ykc_monitor_preq_bms_end[gunno].body.reason.reach_soc = bms->BST.SOCGetObj;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.reach_volt_all = bms->BST.VoltGetObj;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.reach_single_volt = bms->BST.CeliVoltGetObj;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.charger_end = bms->BST.ChargInitiStop;

    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_insulation = bms->BST.InsltFault;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_out_linker_overtemp = bms->BST.OutConectOVtemp;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_bms_out_linker_overtemp = bms->BST.BMSCompOVtemp;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_charge_linker = bms->BST.Conectfault;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_bat_group_overtemp = bms->BST.BatOVtemp;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_high_volt_relay = bms->BST.HVRelaysFault;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_test_2_point_volt = bms->BST.Check2Ft;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.fault_other = bms->BST.OtherFt;

    g_ykc_monitor_preq_bms_end[gunno].body.reason.overcurr = bms->BST.OverCurlt;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.volt_abnormal = bms->BST.Voltfault;
    g_ykc_monitor_preq_bms_end[gunno].body.reason.reserve = 0x00;

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_BMS_STOP);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_chargerend_duringcharge
 * 功能          充电桩请求报文填报：充电过程中充电机终止
 * **********************************************/
void ykc_monitor_chargepile_request_padding_chargerend_duringcharge(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_charger_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_charger_end[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_charger_end[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_charger_end[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);

    g_ykc_monitor_preq_charger_end[gunno].body.reason.reach_set_adition = bms->CST.AutoStop;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.manual_end = bms->CST.ManStop;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.exception_end = bms->CST.FaultStop;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.bms_end = bms->CST.BMSInitiStop;

    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_charger_overtemp = bms->CST.ChgOVtemp;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_charge_linker = bms->CST.ChgConectfault;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_charger_internal_overtemp = bms->CST.ChgOVtempIn;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_elect_can_not_report = bms->CST.EnergyTranFault;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_charger_scram = bms->CST.EmgcyStop;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_other = bms->CST.OtherFault;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.fault_reserve = 0x00;

    g_ykc_monitor_preq_charger_end[gunno].body.reason.curr_no_match = bms->CST.Curltfault;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.volt_exception = bms->CST.Voltfault;
    g_ykc_monitor_preq_charger_end[gunno].body.reason.reserve = 0x00;

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGER_STOP);
}

/*************************************************
 * 函数名      ykc_chargepile_request_padding_bmscommand_chargerout
 * 功能          充电桩请求报文填报：充电过程 BMS 需求与充电机输出
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bmscommand_chargerout(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(is_init){
        uint8_t valid_len = 0x00;
        s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

        valid_len = sizeof(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_command = bms->BCL.BMSneedVolt;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_command = (4000 - bms->BCL.BMSneedCurlt);
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = bms->BCL.ChagModel;

            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = bms->BCS.ChargVolt;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = (4000 - bms->BCS.ChargCurlt);
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.max_volt_and_gn.bms_max_single_bat_volt = bms->BCS.CellHigVolt;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.max_volt_and_gn.max_single_bat_volt_gn = bms->BCS.HigVoltCellNum;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_current_soc = bms->BCS.SOC;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_remain_charge_time = bms->BCS.SurplChgTime;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_volt = (s_ykc_monitor_base->voltage_a /10);
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_curr = (4000 - (s_ykc_monitor_base->current_a /10));
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.charge_time = s_ykc_monitor_base->charge_time /60;
        }
    }
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_bmsinfo_duringcharge
 * 功能          充电桩请求报文填报：充电过程 BMS 信息
 * **********************************************/
void ykc_monitor_chargepile_request_padding_bmsinfo_duringcharge(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(is_init){
        uint8_t valid_len = 0x00;
        s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

        valid_len = sizeof(g_ykc_monitor_preq_bms_info[gunno].body.serial_number);
        valid_len = valid_len > sizeof(s_ykc_monitor_base->transaction_number) ? sizeof(s_ykc_monitor_base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_bms_info[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bms_info[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_bms_info[gunno].body.serial_number, s_ykc_monitor_base->transaction_number, valid_len);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_BMS_INFO) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_ykc_monitor_base->bms_data);

            g_ykc_monitor_preq_bms_info[gunno].body.bms_max_single_volt_bat_number = bms->BSM.HigVoltCellNum;
            g_ykc_monitor_preq_bms_info[gunno].body.bms_bat_temp_max = (bms->BSM.HigTemp + 50);
            g_ykc_monitor_preq_bms_info[gunno].body.bat_temp_max_measure_number = bms->BSM.HigTempNum;
            g_ykc_monitor_preq_bms_info[gunno].body.bms_bat_temp_min = (bms->BSM.LowTemp + 50);
            g_ykc_monitor_preq_bms_info[gunno].body.bat_temp_min_measure_number = bms->BSM.LowTempNum;
            g_ykc_monitor_preq_bms_info[gunno].body.state.bms_single_volt = bms->BSM.CellOverVolt;
            g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_soc = bms->BSM.SOCState;
            g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_curr = bms->BSM.BatOverCurlt;
            g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_temp = bms->BSM.BatOverTemp;
            g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_isolate = bms->BSM.Insulat;
            g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_output_linker = bms->BSM.OutConect;
            g_ykc_monitor_preq_bms_info[gunno].body.state.charge_forbid = bms->BSM.AllowChg;
            g_ykc_monitor_preq_bms_info[gunno].body.state.reserve = 0x00;
        }
    }
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_card_authority
 * 功能          充电桩请求报文填报：刷卡权限认证
 * **********************************************/
int8_t ykc_monitor_chargepile_request_padding_card_authority(uint8_t gunno)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return -0x01;
    }
    if(ykc_monitor_get_socket_info()->state != YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(!s_ykc_monitor_base->flag.card_info_is_uid){
        return -0x01;
    }
    g_ykc_monitor_preq_apply_charge_active[gunno].body.start_type = 0x01;
    g_ykc_monitor_preq_apply_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;

    valid_len = sizeof(s_ykc_monitor_base->card_uid);
    valid_len = valid_len > NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX : valid_len;
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    if(valid_len > s_ykc_monitor_base->card_uid_len){
        memcpy((g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number + (valid_len - s_ykc_monitor_base->card_uid_len)),
                s_ykc_monitor_base->card_uid, s_ykc_monitor_base->card_uid_len);
    }else{
        memcpy(g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number, s_ykc_monitor_base->card_uid, valid_len);
    }
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.vin, 0x00, NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX);

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_vin_authority
 * 功能          充电桩请求报文填报：VIN 码权限认证
 * **********************************************/
int8_t ykc_monitor_chargepile_request_padding_vin_authority(uint8_t gunno)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return -0x01;
    }
    if(ykc_monitor_get_socket_info()->state != YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_apply_charge_active[gunno].body.start_type = 0x03;
    g_ykc_monitor_preq_apply_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    for(uint8_t count = 0x00; count < NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX; count++){
        g_ykc_monitor_preq_apply_charge_active[gunno].body.vin[count] = s_ykc_monitor_base->car_vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
    }

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_transaction_record
 * 功能          充电桩请求报文填报：交易记录信息
 * **********************************************/
uint8_t ykc_monitor_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    uint8_t valid_len = 0x00;
    uint64_t elect = 0x00;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)transaction;

    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    ykc_monitor_set_transaction_verify_state(gunno, 0x00);

    if((ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD) == NET_YKC_MONITOR_SEND_STATE_COMPLETE) || !is_repeat){
        valid_len = sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number);
        valid_len = valid_len > sizeof(_transaction->serial_number) ? sizeof(_transaction->serial_number) : valid_len;
        memset(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number, _transaction->serial_number, valid_len);
        g_ykc_monitor_preq_transaction_records[gunno].body.start_time = ykc_monitor_get_cp56time2a_from_timestamp(_transaction->start_time);
        g_ykc_monitor_preq_transaction_records[gunno].body.stop_time = ykc_monitor_get_cp56time2a_from_timestamp(_transaction->end_time);

        g_ykc_monitor_preq_transaction_records[gunno].body.tip_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_SHARP] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.tip_elect = _transaction->rate_type_elect[APP_RATE_TYPE_SHARP] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.tip_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_SHARP] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.tip_amount = _transaction->rate_type_amount[APP_RATE_TYPE_SHARP];

        g_ykc_monitor_preq_transaction_records[gunno].body.peak_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_PEAK] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.peak_elect = _transaction->rate_type_elect[APP_RATE_TYPE_PEAK] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.peak_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_PEAK] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.peak_amount = _transaction->rate_type_amount[APP_RATE_TYPE_PEAK];

        g_ykc_monitor_preq_transaction_records[gunno].body.flat_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_FLAT] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.flat_elect = _transaction->rate_type_elect[APP_RATE_TYPE_FLAT] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.flat_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_FLAT] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.flat_amount = _transaction->rate_type_amount[APP_RATE_TYPE_FLAT];

        g_ykc_monitor_preq_transaction_records[gunno].body.valley_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_VALLEY] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.valley_elect = _transaction->rate_type_elect[APP_RATE_TYPE_VALLEY] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.valley_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_VALLEY] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.valley_amount = _transaction->rate_type_amount[APP_RATE_TYPE_VALLEY];

        elect = _transaction->ammeter_start *10;
        memset(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val));
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val, &elect, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val));
        elect = _transaction->ammeter_stop *10;
        memset(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val));
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val, &elect, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val));

        g_ykc_monitor_preq_transaction_records[gunno].body.total_elect = _transaction->total_elect *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.total_loss_elect = _transaction->total_loss_elect *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.consume_amount = _transaction->total_fee;

        valid_len = sizeof(_transaction->car_vin);
        valid_len = valid_len > NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX : valid_len;
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.vin, &(_transaction->car_vin), valid_len);

        g_ykc_monitor_preq_transaction_records[gunno].body.transaction_identity = ykc_monitor_chargepile_transaction_identity_converted(_transaction->start_type);
        if(g_ykc_monitor_preq_transaction_records[gunno].body.transaction_identity == NETYKC_MONITOR_START_WAY_OFFLINE){
            valid_len = sizeof(_transaction->logic_card_number);
            valid_len = valid_len > NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX : valid_len;
            memset(g_ykc_monitor_preq_transaction_records[gunno].body.physics_card_number, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.physics_card_number));
            memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.physics_card_number, &(_transaction->logic_card_number), valid_len);
        }else{
            valid_len = sizeof(_transaction->physics_card_number);
            valid_len = valid_len > NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX : valid_len;
            memset(g_ykc_monitor_preq_transaction_records[gunno].body.physics_card_number, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.physics_card_number));
            memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.physics_card_number, &(_transaction->physics_card_number), valid_len);
        }

        g_ykc_monitor_preq_transaction_records[gunno].body.transaction_date = ykc_monitor_get_cp56time2a_from_timestamp(s_ykc_monitor_base->current_time);
        g_ykc_monitor_preq_transaction_records[gunno].body.stop_reason = ykc_monitor_chargepile_stop_reason_converted(_transaction->stop_reason, _transaction->order_state.is_start_fail);

        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD);
        return 0x01;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_mergecharge_card_authority
 * 功能          充电桩请求报文填报：刷卡权限认证(并充)
 * **********************************************/
int8_t ykc_monitor_chargepile_request_padding_mergecharge_card_authority(uint8_t gunno)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return -0x01;
    }
    if(ykc_monitor_get_socket_info()->state != YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    struct tm *_tm;
    time_t _time = time(NULL);
    uint8_t valid_len = 0x00;
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.start_type = 0x01;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;

    valid_len = sizeof(s_ykc_monitor_base->card_uid);
    valid_len = valid_len > NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX : valid_len;
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    memcpy(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.account_or_phycard_number, s_ykc_monitor_base->card_uid, valid_len);
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.vin, 0x00, NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX);

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.main_auxiliary_gun_flag = NET_ENUM_FALSE;   /* 默认主枪申请 */
    _tm = localtime(&_time);
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[0] = _tm->tm_year;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[1] = _tm->tm_mon;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[2] = _tm->tm_mday;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[3] = _tm->tm_hour;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[4] = _tm->tm_min;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[5] = _tm->tm_sec;

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_MERGECHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_mergecharge_vin_authority
 * 功能          充电桩请求报文填报：VIN 码权限认证(并充)
 * **********************************************/
int8_t ykc_monitor_chargepile_request_padding_mergecharge_vin_authority(uint8_t gunno)
{
#ifndef NET_YKC_MONITOR_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return -0x01;
    }
    if(ykc_monitor_get_socket_info()->state != YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    struct tm *_tm;
    time_t _time = time(NULL);
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.start_type = 0x03;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    for(uint8_t count = 0x00; count < NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX; count++){
        g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.vin[count] = s_ykc_monitor_base->car_vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
    }
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.main_auxiliary_gun_flag = NET_ENUM_FALSE;   /* 默认主枪申请 */
    _tm = localtime(&_time);
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[0] = _tm->tm_year;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[1] = _tm->tm_mon;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[2] = _tm->tm_mday;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[3] = _tm->tm_hour;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[4] = _tm->tm_min;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[5] = _tm->tm_sec;

    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_MERGECHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_start_charge_response_asynchronously
 * 功能          充电桩报文响应事件：启动充电异步响应
 * **********************************************/
void ykc_monitor_start_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_ykc_monitor_flag_info[gunno].is_start_charge == NET_ENUM_FALSE){
        return;
    }
    if(result){
        s_ykc_monitor_flag_info[gunno].start_success = NET_ENUM_TRUE;
    }else{
        s_ykc_monitor_flag_info[gunno].start_success = NET_ENUM_FALSE;
    }
    s_ykc_monitor_flag_info[gunno].is_start_charge = NET_ENUM_FALSE;
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, gunno, NET_YKC_MONITOR_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ykc_monitor_monitor_stop_charge_response_asynchronously
 * 功能          充电桩报文响应事件：停止充电异步响应
 * **********************************************/
void ykc_monitor_monitor_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_ykc_monitor_flag_info[gunno].is_stop_charge == NET_ENUM_FALSE){
        return;
    }
    if(result){
        s_ykc_monitor_flag_info[gunno].stop_success = NET_ENUM_TRUE;
    }else{
        s_ykc_monitor_flag_info[gunno].stop_success = NET_ENUM_FALSE;
    }
    s_ykc_monitor_flag_info[gunno].is_stop_charge = NET_ENUM_FALSE;
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, gunno, NET_YKC_MONITOR_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ykc_monitor_start_mergecharge_response_asynchronously
 * 功能          充电桩报文响应事件：启动充电异步响应(并充)
 * **********************************************/
void ykc_monitor_start_mergecharge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_ykc_monitor_flag_info[gunno].is_start_mergecharge == NET_ENUM_FALSE){
        return;
    }
    if(result){
        s_ykc_monitor_flag_info[gunno].mergestart_success = NET_ENUM_TRUE;
    }else{
        s_ykc_monitor_flag_info[gunno].mergestart_success = NET_ENUM_FALSE;
    }
    s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, gunno, NET_YKC_MONITOR_PRES_EVENT_START_MERGECHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ykc_monitor_set_power_percent_response_asynchronously
 * 功能          充电桩报文响应事件：设置功率百分比响应
 * **********************************************/
void ykc_monitor_set_power_percent_response_asynchronously(uint8_t result)
{
    if(s_ykc_monitor_flag_info[0x00].is_set_power == NET_ENUM_FALSE){
        return;
    }
    if(result){
        s_ykc_monitor_flag_info[0x00].set_power_success = NET_ENUM_TRUE;
    }else{
        s_ykc_monitor_flag_info[0x00].set_power_success = NET_ENUM_FALSE;
    }
    s_ykc_monitor_flag_info[0x00].is_set_power = NET_ENUM_FALSE;
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY);
}

void ykc_monitor_chargepile_state_changed(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(s_ykc_monitor_base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        s_ykc_monitor_state_info[gunno].state.connect = NET_ENUM_TRUE;
    }else{
        s_ykc_monitor_state_info[gunno].state.connect = NET_ENUM_FALSE;
    }
    switch(s_ykc_monitor_base->state.current){
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_STARTING:
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_IDLE;
        break;
    case APP_OFSM_STATE_CHARGING:
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_CHARGING;
        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_IDLE;
        break;
    case APP_OFSM_STATE_FAULTING:
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_FAULTING;
        break;
    default:
        break;
    }
}

void ykc_monitor_chargepile_update_result_report(uint8_t result)
{
    g_ykc_monitor_pres_remote_update.body.result = result;
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_PRES_EVENT_REMOTE_UPDATE);
}

void ykc_monitor_chargepile_fault_report(uint8_t gunno, uint16_t code)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ykc_monitor_state_info[gunno].fault_code = code;
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_create_local_transaction_number
 * 功能          创建本地交易号
 * **********************************************/
int8_t ykc_monitor_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if((vector == NULL) || (len == 0x00)){
        return -0x02;
    }
    if(len < NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT){
        return -0x03;
    }

    uint8_t sn_len = 0x00, *ptr = (uint8_t*)vector;
    struct tm *_tm = NULL;

    s_ykc_monitor_local_start_sq++;
    sn_len = sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.pile_number);
    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    _tm = localtime((const time_t*)&(s_ykc_monitor_base->current_time));

    rt_kprintf("ykc_chargepile_create_local_transaction_number[%d](%d, %d, %d, %d, %d)\n", s_ykc_monitor_base->current_time, _tm->tm_year, _tm->tm_mon,
            _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
    memcpy(ptr, g_ykc_monitor_preq_transaction_records[gunno].body.pile_number, sn_len);   /* 桩号 */
    ptr[sn_len++] = gunno + 1;                                                     /* 枪号 */
    ptr[sn_len++] = (((_tm->tm_year - 100) /10) *16) + ((_tm->tm_year - 100) %10); /* 年 */
    ptr[sn_len++] = (((_tm->tm_mon + 0x01) /10) *16) + ((_tm->tm_mon + 0x01) %10); /* 月 */
    ptr[sn_len++] = ((_tm->tm_mday /10) *16) +  (_tm->tm_mday %10);                /* 日 */
    ptr[sn_len++] = ((_tm->tm_hour /10) *16) + (_tm->tm_hour %10);                 /* 时 */
    ptr[sn_len++] = ((_tm->tm_min /10) *16) + (_tm->tm_min %10);                   /* 分 */
    ptr[sn_len++] = ((_tm->tm_sec /10) *16) + (_tm->tm_sec %10);                   /* 秒 */
    memcpy((ptr + sn_len), &s_ykc_monitor_local_start_sq, sizeof(s_ykc_monitor_local_start_sq));   /* 自增序列号 */


    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_time_sync_revise
 * 功能          时间同步修正
 * **********************************************/
void ykc_monitor_chargepile_time_sync_revise(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_transaction_records[gunno].body.start_time = ykc_monitor_get_cp56time2a_from_timestamp(s_ykc_monitor_base->start_time);
    g_ykc_monitor_preq_transaction_records[gunno].body.stop_time = ykc_monitor_get_cp56time2a_from_timestamp(s_ykc_monitor_base->stop_time);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_fault_converted
 * 功能          故障转换
 * **********************************************/
uint16_t ykc_monitor_chargepile_fault_converted(uint16_t bit)
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
    case APP_SYS_FAULT_LIGHT_PRPTECT :
        return NET_GENERAL_FAULT_LIGHT_PRPTECT;
    case APP_SYS_FAULT_GUN_SITE :
        return NET_GENERAL_FAULT_GUN_SITE;
    case APP_SYS_FAULT_CIRCUIT_BREAKER :
        return NET_GENERAL_FAULT_CIRCUIT_BREAKER;
    case APP_SYS_FAULT_FLOODING :
        return NET_GENERAL_FAULT_FLOODING;
    case APP_SYS_FAULT_SMOKE :
        return NET_GENERAL_FAULT_SMOKE;
    case APP_SYS_FAULT_POUR :
        return NET_GENERAL_FAULT_POUR;
    case APP_SYS_FAULT_LIQUID_COOLING :
        return NET_GENERAL_FAULT_LIQUID_COOLING;
    case APP_SYS_FAULT_FUSE :
        return NET_GENERAL_FAULT_FUSE;
    default:
        return NET_GENERAL_FAULT_SIZE;
    }
    return NET_GENERAL_FAULT_SIZE;
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_transaction_identity_converted
 * 功能          交易标识转换
 * **********************************************/
static uint8_t ykc_monitor_chargepile_transaction_identity_converted(uint8_t identity)
{
    switch(identity){
    case APP_CHARGE_START_WAY_APP:
        return NETYKC_MONITOR_START_WAY_APP;
        break;
    case APP_CHARGE_START_WAY_ONLINE_CARD:
        return NETYKC_MONITOR_START_WAY_ONLINE;
        break;
    case APP_CHARGE_START_WAY_OFFLINE_CARD:
        return NETYKC_MONITOR_START_WAY_OFFLINE;
        break;
    case APP_CHARGE_START_WAY_VIN:
        return NETYKC_MONITOR_START_WAY_VIN;
        break;
    default:
        break;
    }
    return NETYKC_MONITOR_START_WAY_OFFLINE;
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_stop_reason_converted
 * 功能          停充原因转换
 * **********************************************/
static uint16_t ykc_monitor_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting)
{
    uint16_t _reason = NETYKC_MONITOR_AS_REASON90_UNKNOW;

    switch(reason){
    /* 急停 */
    case APP_SYSTEM_STOP_WAY_SCRAM:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON50_EMERGENCY_STOP;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON72_EMERGENCY_STOP;
        }
        break;
    /* 读卡器、门禁 */
    case APP_SYSTEM_STOP_WAY_CARDREADER:
    case APP_SYSTEM_STOP_WAY_DOOR:
        _reason = NETYKC_MONITOR_AS_REASON8A_CARDREADER;
        break;
    /* 电表 */
    case APP_SYSTEM_STOP_WAY_AMMETER:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON4D_AMMETER_COMMUNICATION;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON6D_AMMETER_COMMUNICATE;
        }
        break;
    /* 充电模块 */
    case APP_SYSTEM_STOP_WAY_CHARGEMODULE:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON4F_CHARGE_MODULE;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON71_CHARGE_MODULE;
        }
        break;
    /* 过温 */
    case APP_SYSTEM_STOP_WAY_OVERTEMP:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON53_ABNORMAL_TEMP;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON74_TEMPERATURE_ABNORMAL;
        }
        break;
    /* 过、欠压 */
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        _reason = NETYKC_MONITOR_AS_REASON79_CHARGE_TVOLTAGE_ABNORMAL;
        break;
    /* 过流 */
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        _reason = NETYKC_MONITOR_AS_REASON7A_CHARGE_TCURRENT_ABNORMAL;
        break;
    /* DC 继电器 */
    case APP_SYSTEM_STOP_WAY_RELAY:
        _reason = NETYKC_MONITOR_AS_REASON8B_DC_RELAY;
        break;
    /* 并联 继电器 */
    case APP_SYSTEM_STOP_WAY_PARALLEL_RELAY:
        _reason = NETYKC_MONITOR_AS_REASON8D_PARALLEL_RELAY;
        break;
    /* AC 继电器 */
    case APP_SYSTEM_STOP_WAY_AC_RELAY:
        _reason = NETYKC_MONITOR_AS_REASON8C_AC_RELAY;
        break;
    /* 充满 */
    case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
        _reason = NETYKC_MONITOR_CC_REASON41_CHARGE_FULL;
        break;
    /* 拔枪 */
    case APP_SYSTEM_STOP_WAY_PULL_GUN:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON4B_GUIDE_DISCONNECT;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON6B_GUIDANCE_DISCONNECT;
        }
        break;
    /* 电子锁 */
    case APP_SYSTEM_STOP_WAY_ELECTRY_LOCK:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON55_ELECT_LOCK;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON77_ELOCK_ABNORMAL;
        }
        break;
    /* 通讯 */
    case APP_SYSTEM_STOP_WAY_COMMINICATION:
        if(stop_in_starting){
            _reason = NETYKC_MONITOR_SF_REASON5A_RECV_BRM_TIMEOUT;
        }else{
            _reason = NETYKC_MONITOR_AS_REASON84_RECV_BCS_TIMEOUT;
        }
        break;
    /* 辅源 */
    case APP_SYSTEM_STOP_WAY_AUXPOWER:
        _reason = NETYKC_MONITOR_SF_REASON66_AUXPOWER;
        break;
    /* 断电 */
    case APP_SYSTEM_STOP_WAY_POWER_OFF:
        _reason = NETYKC_MONITOR_AS_REASON83_POWER_OFF;
        break;
    /* 存储芯片 */
    case APP_SYSTEM_STOP_WAY_FLASH:
    case APP_SYSTEM_STOP_WAY_EEPROM:
        _reason = NETYKC_MONITOR_AS_REASON8E_STORAGE_CHIP;
        break;
    /* 短路 */
    case APP_SYSTEM_STOP_WAY_SHORTS:
        _reason = NETYKC_MONITOR_AS_REASON6C_CIRCUIT_BREAKER_ACTION;
        break;
    /* 枪电压 */
    case APP_SYSTEM_STOP_WAY_GUNVOLT:
        _reason = NETYKC_MONITOR_SF_REASON60_BHM_STAGE_VOLT_OVERRANGE;
        break;
    /* 绝缘 */
    case APP_SYSTEM_STOP_WAY_INSULT:
        _reason = NETYKC_MONITOR_SF_REASON57_INSULATION_ABNORMAL;
        break;
    /* 电池电压 */
    case APP_SYSTEM_STOP_WAY_BATTERY_VOLT:
        _reason = NETYKC_MONITOR_SF_REASON61_BRO_AA_STAGE_VOLT_OVERRANGE;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = NETYKC_MONITOR_AS_REASON82_CAR_COMMAND_STOP;
        break;
    /* 准备电压 */
    case APP_SYSTEM_STOP_WAY_READY_VOLT:
        _reason = NETYKC_MONITOR_SF_REASON67_READY_VOLTAGE;
        break;
    /* 绝缘电压 */
    case APP_SYSTEM_STOP_WAY_INSULT_VOLT:
        _reason = NETYKC_MONITOR_SF_REASON68_INSULT_VOLTAGE;
        break;
    /* BSM */
    case APP_SYSTEM_STOP_WAY_BSM:
        _reason = NETYKC_MONITOR_AS_REASON8F_BSM_WARNNING;
        break;
    /* APP */
    case APP_SYSTEM_STOP_WAY_APP_STOP:
        _reason = NETYKC_MONITOR_CC_REASON40_APP;
        break;
    /* 刷卡 */
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = NETYKC_MONITOR_CC_REASON45_MANUAL_STOP;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        _reason = NETYKC_MONITOR_AS_REASON6E_NO_BALLANCE;
        break;
    /* 屏幕 */
    case APP_SYSTEM_STOP_WAY_SCREEN_STOP:
        _reason = NETYKC_MONITOR_CC_REASON45_MANUAL_STOP;
        break;
    /* 到达设定电量 */
    case APP_SYSTEM_STOP_WAY_REACH_ELECT:
        _reason = NETYKC_MONITOR_CC_REASON42_TARGET_ELECT;
        break;
    /* 到达设定时间 */
    case APP_SYSTEM_STOP_WAY_REACH_TIME:
        _reason = NETYKC_MONITOR_CC_REASON44_TARGET_TIME;
        break;
    /* 到达设定余额 */
    case APP_SYSTEM_STOP_WAY_REACH_MONEY:
        _reason = NETYKC_MONITOR_CC_REASON43_TARGET_MONEY;
        break;
    /* 充电电流异常 */
    case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
        _reason = NETYKC_MONITOR_AS_REASON76_CURRENT_ABNORMAL;
        break;
    /* 达到SOC 限定值 */
    case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
        _reason = NETYKC_MONITOR_CC_REASON46_RESERVE;
        break;
    default:
        break;
    }
    return _reason;
}

/*************************************************
 * 函数名      ykc_monitor_query_transaction_verify_state
 * 功能          查询订单确认状态
 * **********************************************/
uint8_t ykc_monitor_query_transaction_verify_state(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    if(ykc_monitor_transaction_is_verify(gunno)){
        ykc_monitor_set_transaction_verify_state(gunno, NET_ENUM_FALSE);
        return 0x01;
    }
    return 0x00;
}

static void ykc_monitor_request_message_repeat(uint8_t gunno)
{
    uint8_t event = 0;
    if(ykc_monitor_exist_message_wait_response(gunno, NULL)){
        for(event = 0; event < NET_YKC_MONITOR_CHARGEPILE_PREQ_NUM; event++){
            if(ykc_monitor_get_message_wait_response_timeout_state(gunno, NET_YKC_MONITOR_WAIT_RESPONSE_TIMEOUT, event)){
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, event);
            }
        }
    }
}

void ykc_monitor_data_realtime_process(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ykc_monitor_base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(ykc_monitor_get_socket_info()->state == YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
        ykc_monitor_request_message_repeat(gunno);

        if(s_ykc_monitor_base->state.current == APP_OFSM_STATE_CHARGING){
            if(s_ykc_monitor_realtime_data_count[gunno] > rt_tick_get()){
                s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_ykc_monitor_realtime_data_count[gunno]) > YKC_MONITOR_REALTIME_DATA_INTERVAL_CHARGING *1000){
                s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_BMS_INFO);
            }
        }else{
            if(s_ykc_monitor_realtime_data_count[gunno] > rt_tick_get()){
                s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_ykc_monitor_realtime_data_count[gunno]) > s_ykc_monitor_realtime_data_interval[gunno] *1000){
                s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
                if(s_ykc_monitor_realtime_data_interval[gunno] < YKC_MONITOR_REALTIME_DATA_INTERVAL_IDLE){
                    s_ykc_monitor_realtime_data_interval[gunno] = YKC_MONITOR_REALTIME_DATA_INTERVAL_IDLE;
                }
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
            }
        }
    }else{
        s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
        s_ykc_monitor_realtime_data_interval[gunno] = YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
    }

    if((s_ykc_monitor_base->state.current == APP_OFSM_STATE_STARTING) || (s_ykc_monitor_base->state.current == APP_OFSM_STATE_CHARGING)){
        ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD);
    }
}

/*
 * 用于检测到状态有变化时上报
 * */
static void ykc_monitor_state_changed_check(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if((g_ykc_monitor_preq_report_realtime_data[gunno].body.plug_gun != s_ykc_monitor_state_info[gunno].state.connect) ||
            (g_ykc_monitor_preq_report_realtime_data[gunno].body.state != s_ykc_monitor_state_info[gunno].state.state) ||
            (g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault != s_ykc_monitor_state_info[gunno].fault_code)){

        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            /** 由于 s_ykc_monitor_state_info[gunno].state.connect 和 s_ykc_monitor_state_info[gunno].state.state 和
             *  s_ykc_monitor_state_info[gunno].fault_code 会在其它线程被赋值，为了防止用这几个值做判断时和赋值时可能存在的不一致而导致
                            *     状态错乱问题，将这几个值进行临时存储用于判断和赋值*/
            uint8_t _connect = s_ykc_monitor_state_info[gunno].state.connect;
            uint8_t _state = s_ykc_monitor_state_info[gunno].state.state;
            uint16_t _fault = s_ykc_monitor_state_info[gunno].fault_code;

            if(_state == NETYKC_MONITOR_DEVICE_STATE_FAULTING){
                if(_fault != 0x00){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault = _fault;
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.plug_gun = _connect;
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.state = _state;

                    s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
                }
            }else{
                if(_fault == 0x00){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault = _fault;
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.plug_gun = _connect;
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.state = _state;

                    s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
                }
            }
        }
    }
}

static void ykc_monitor_realtime_process_thread_entry(void *parameter)
{
    uint8_t gunno = 0x00;

    while(1){
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }
        if((s_ykc_monitor_handle == NULL) || (s_ykc_monitor_base == NULL)){
            rt_thread_mdelay(100);
            continue;
        }

        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            ykc_monitor_fault_detect_report(gunno);
            ykc_monitor_data_realtime_process(gunno);
            ykc_monitor_state_changed_check(gunno);
        }

        rt_thread_mdelay(100);
    }
}

int32_t ykc_monitor_realtime_process_init(void)
{
    if(rt_thread_init(&s_ykc_monitor_realtime_process_thread, "ykc_mrl_pro", ykc_monitor_realtime_process_thread_entry, NULL,
            s_ykc_monitor_realtime_process_thread_stack, YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ykc monitor realtime process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ykc_monitor_realtime_process_thread) != RT_EOK){
        LOG_E("ykc monitor realtime process thread startup fail, please check");
        return -0x01;
    }
    return 0x00;
}

/******************************** 以下是监控报文 *******************************/
/******************************** 以下是监控报文 *******************************/
/*************************************************
 * 函数名      ykc_monitor_message_padding_module_info
 * 功能          组包：充电模块信息
 * **********************************************/
int8_t ykc_monitor_message_padding_module_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }

    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_Preq_ModuleInfo_t);
    uint8_t num = 0x00, padding_num = 0x00, group_num = 0x00, *module_num = NULL;
    thaisenModuleVoltCurrStruct * voltcurr = NULL;
    Net_YkcMonitorPro_PRes_Preq_ModuleInfo_t *message = (Net_YkcMonitorPro_PRes_Preq_ModuleInfo_t*)buf;
    struct single_module_info *_info = NULL;

    extern uint8_t sys_get_module_model(void);
    extern uint8_t sys_get_module_group_num(void);
    extern uint8_t* sys_get_module_num_single_group(void);

    group_num = sys_get_module_group_num();
    module_num = sys_get_module_num_single_group();

    message->head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    message->body.num = 0x00;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    _info = (struct single_module_info*)(buf + (total - 0x02));

    for(uint8_t group = 0x00; group < group_num; group++){
        voltcurr = thaisenGetModuleVoltCurrInfo(&num, group);
        if(num > module_num[group]){
            num = module_num[group];
        }
        message->body.num += num;
        for(uint8_t count = 0x00; count < num; count++){
            _info[padding_num + count].address = voltcurr[count].addr;
            _info[padding_num + count].voltage = voltcurr[count].voltage;
            _info[padding_num + count].current = voltcurr[count].current;
            _info[padding_num + count].state = 0x00;

            rt_kprintf("group(%d) count(%d) padding_num(%d) addr(%x) voltage(%d) current(%d) state(%x)\n",
                    group, count, padding_num, _info[padding_num + count].address, _info[padding_num + count].voltage,
                    _info[padding_num + count].current, _info[padding_num + count].state);
        }
        padding_num += num;
        total += (num *sizeof(struct single_module_info));
    }

    if(olen){
        *olen = total;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_module_setup_info
 * 功能          组包：充电模块配置信息
 * **********************************************/
int8_t ykc_monitor_message_padding_module_setup_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_Preq_ModuleSetupInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    uint8_t *module_num = NULL;
    Net_YkcMonitorPro_PRes_Preq_ModuleSetupInfo_t *message = (Net_YkcMonitorPro_PRes_Preq_ModuleSetupInfo_t*)buf;

    extern uint8_t sys_get_module_model(void);
    extern uint8_t sys_get_module_group_num(void);
    extern uint8_t* sys_get_module_num_single_group(void);

    module_num = sys_get_module_num_single_group();

    message->head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    memset(message->body.module_num, 0x00, sizeof(message->body.module_num));

    message->body.model = sys_get_module_model();
    message->body.group = sys_get_module_group_num();
    for(uint8_t count = 0x00; count < message->body.group; count++){
        message->body.module_num[count] = module_num[count];
    }
    message->body.mrated_volt = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0x00));
    message->body.mrated_curr = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0x00));
    message->body.poutvolt_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, 0x00));
    message->body.poutcurr_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0x00));
    message->body.poutvolt_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0x00));
    message->body.poutcurr_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MIN_LIMIT_CURRENT, 0x00));

    if(olen){
        *olen = total;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_fault_record
 * 功能          组包：故障记录信息
 * **********************************************/
int8_t ykc_monitor_message_padding_fault_record(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_Preq_ModuleSetupInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }

    return -0x02;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_charge_record
 * 功能          组包：充电记录信息
 * **********************************************/
int8_t ykc_monitor_message_padding_charge_record(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_Preq_ModuleSetupInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }

    return -0x02;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_input_info_setup
 * 功能          组包：输入信息配置
 * **********************************************/
int8_t ykc_monitor_message_padding_input_info_setup(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_InInfo_Setup_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    Net_YkcMonitorPro_PRes_InInfo_Setup_t *message = (Net_YkcMonitorPro_PRes_InInfo_Setup_t*)buf;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    message->body.emergency_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_SCRAM, 0x00));
    message->body.emergency_reversal = *(sys_read_config_item_content(CONFIG_ITEM_INNEG_SCRAM, 0x00));

    message->body.gate_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_GATE, 0x00));
    message->body.gate_reversal = *(sys_read_config_item_content(CONFIG_ITEM_INNEG_GATE, 0x00));

    message->body.acrelay_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_ACRELAY, 0x00));
    message->body.acrelay_reversal = *(sys_read_config_item_content(CONFIG_ITEM_INNEG_ACRELAY, 0x00));

    message->body.dcrelay_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_DCRELAY, 0x00));
    message->body.dcrelay_reversal = *(sys_read_config_item_content(CONFIG_ITEM_INNEG_DCRELAY, 0x00));

    message->body.fan_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_FAN, 0x00));
    message->body.fan_reversal = *(sys_read_config_item_content(CONFIG_ITEM_INNEG_FAN, 0x00));

    message->body.elock_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_ELOCK, 0x00));
    message->body.elock_reversal = *(sys_read_config_item_content(CONFIG_ITEM_INNEG_ELOCK, 0x00));

    message->body.temppro_enable = *(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPRO, 0x00));
    message->body.temppro_reversal = 0x00;

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_protect_info_setup
 * 功能          组包：保护信息配置
 * **********************************************/
int8_t ykc_monitor_message_padding_protect_info_setup(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_ProtectInfo_Setup_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    Net_YkcMonitorPro_PRes_ProtectInfo_Setup_t *message = (Net_YkcMonitorPro_PRes_ProtectInfo_Setup_t*)buf;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    message->body.inover_volt = (uint16_t)(*((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_OVERVOL, 0x00))));
    message->body.inundver_volt = (uint16_t)(*((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_UNDERVOL, 0x00))));
    message->body.ouover_volt = (uint16_t)(*((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERVOL, 0x00))));
    message->body.ouundver_volt = (uint16_t)(*((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_UNDERVOL, 0x00))));
    message->body.ouover_curr = (uint16_t)(*((uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERCUR, 0x00))));
    message->body.stop_soc = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0x00));
    message->body.ot_warnning = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_WARN, 0x00));
    message->body.ot_stop = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_STOP, 0x00));
    message->body.ot_resume = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_RECOVER, 0x00));
    message->body.ot_limit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_SETCUR, 0x00));
    message->body.gun_volt = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUNVOLT_LIMIT, 0x00));
    message->body.power_percent = sys_get_power_percent();

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_function_setup
 * 功能          组包：功能配置
 * **********************************************/
int8_t ykc_monitor_message_padding_function_setup(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PRes_FunctionSetup_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    Net_YkcMonitorPro_PRes_FunctionSetup_t *message = (Net_YkcMonitorPro_PRes_FunctionSetup_t*)buf;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    message->body.insulation = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_INSULATION, 0x00));
    message->body.vin_charge = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_VIN, 0x00));
    message->body.plug_and_play = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PLUGCHARGE, 0x00));
    message->body.card_reader = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0x00));
    message->body.local_charge = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_LOCAL, 0x00));
    message->body.cc4_uplimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_CC14V_MAX, 0x00));
    message->body.cc4_downlimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_CC14V_MIN, 0x00));
    message->body.cc6_uplimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_CC16V_MAX, 0x00));
    message->body.cc6_downlimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_CC16V_MIN, 0x00));
    message->body.cc12_uplimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_CC112V_MAX, 0x00));
    message->body.cc12_downlimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_CC112V_MIN, 0x00));

    if(olen){
        *olen = total;
    }

    return 0x00;
}

#endif /* NET_PACK_USING_YKC_MONITOR */
