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

#ifdef NET_YKC_MONITOR_AS_MONITOR
/** 启动中报文 */
#define YKC_MONITOR_STARTING_BMS_MESSAGE_BHM              (0x01 <<0x00)         /* 是否接收到了BHM报文 */
#define YKC_MONITOR_STARTING_BMS_MESSAGE_BRM              (0x01 <<0x01)         /* 是否接收到了BRM报文 */
#define YKC_MONITOR_STARTING_BMS_MESSAGE_BCP              (0x01 <<0x02)         /* 是否接收到了BCP报文 */
#define YKC_MONITOR_STARTING_BMS_MESSAGE_BRO              (0x01 <<0x03)         /* 是否接收到了BRO报文 */
#define YKC_MONITOR_STARTING_BMS_IS_READY                 (0x01 <<0x04)         /* BMS是否已准备好 */
/** 充电中报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BCL              (0x01 <<0x00)         /* 是否接收到了BCL报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BCS              (0x01 <<0x01)         /* 是否接收到了BCS报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BSM              (0x01 <<0x02)         /* 是否接收到了BSM报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BMV              (0x01 <<0x03)         /* 是否接收到了BMV报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BMT              (0x01 <<0x04)         /* 是否接收到了BMT报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BSP              (0x01 <<0x05)         /* 是否接收到了BSP报文 */
#define YKC_MONITOR_CHARGING_BMS_MESSAGE_BEM              (0x01 <<0x06)         /* 是否接收到了BEM报文 */
#define YKC_MONITOR_CHARGING_BMS_IS_ALLOW                 (0x01 <<0x07)         /* BMS 允许充电 */
/** 充电结束报文 */
#define YKC_MONITOR_FINISH_BMS_MESSAGE_BST                (0x01 <<0x00)         /* 是否接收到了BST报文 */
#define YKC_MONITOR_FINISH_BMS_MESSAGE_BSD                (0x01 <<0x01)         /* 是否接收到了BSD报文 */

#define YKC_MONITOR_BUF_PUBLIC_LENGTH                     0xFF                  /* 充电数据公用缓存长度  */
#define YKC_MONITOR_MODULE_GROUP_MAX                      0x04                  /* 最大模块组数  */
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#define YKC_MONITOR_CHARGE_ELECT_MAX                      500000                /* 最大充电电量值(精度：0.001) */
#define YKC_MONITOR_SPEND_AMOUNT_MAX                      5000000               /* 最大消费金额值(精度：0.0001) */

#define YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT           0x05                  /* 刚连上网时实时数据上报间隔 */
#define YKC_MONITOR_REALTIME_DATA_INTERVAL_CHARGING       0x0F                  /* 充电中实时数据上报间隔  */
#define YKC_MONITOR_REALTIME_DATA_INTERVAL_IDLE           0x05 *60              /* 空闲实时数据上报间隔  */

#define YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE    1536                  /* 实时处理线程栈大小 */

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
    uint16_t is_start_charge : 1;                 /*  已启动充电 */
    uint16_t is_stop_charge : 1;                  /*  已停止充电 */
    uint16_t is_start_mergecharge : 1;            /*  已启动并充充电 */
    uint16_t is_request_mergecharge : 1;          /*  已请求并充充电 */
    uint16_t is_refuse_mergecharge : 1;           /*  已拒绝并充充电 */
    uint16_t is_set_power : 1;                    /*  已设置功率百分比 */

    uint16_t start_success : 1;                   /*  启机成功 */
    uint16_t stop_success : 1;                    /*  停机成功 */
    uint16_t mergestart_success : 1;              /*  并充启机成功 */
    uint16_t set_power_success : 1;               /*  设置功率百分比成功 */
    uint16_t is_charge_finish : 1;                /*  充电结束 */
};

#ifdef NET_YKC_MONITOR_AS_MONITOR
/** 给模块设置的电压、电流信息 */
typedef struct{
    uint32_t base_tick;                           /* 时间时基 */
    uint32_t timestamp;                           /* 采样时间 */
    uint8_t count;                                /* 已采样数 */
    uint8_t is_locked;                            /*  */
    struct voltcurr_pair pair[YKC_MONITOR_MODULE_GROUP_MAX][NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX]; /* 连续采14次，1.5秒采一次(目前按4组模块计算) */
}ykc_monitor_setvoltcurr;
/** 启动过程中的信息 */
typedef struct{
    uint32_t base_tick;                           /* 时间时基 */
    uint32_t timestamp;                           /* 采样时间 */
    uint8_t count;                                /* 已采样数 */
    uint8_t is_locked;                            /*  */
    struct starting_info info[NET_YKC_MONITOR_STARTING_INFO_MAX]; /* 连续采10次，1秒采一次 */
}ykc_monitor_starting_info;
/** 充电过程中的信息 */
typedef struct{
    uint32_t base_tick;                           /* 时间时基 */
    uint32_t timestamp;                           /* 采样时间 */
    uint8_t count;                                /* 已采样数 */
    uint8_t is_locked;                            /*  */
    struct charging_info info[NET_YKC_MONITOR_CHARGING_INFO_MAX]; /* 连续采14次，1.5秒采一次 */
}ykc_monitor_charging_info;
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#pragma pack()

#ifdef NET_YKC_MONITOR_AS_MONITOR
NET_DEF_SRAM2 static ykc_monitor_setvoltcurr s_ykc_monitor_setvoltcurr;
NET_DEF_SRAM2 static ykc_monitor_starting_info s_ykc_monitor_starting_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static ykc_monitor_charging_info s_ykc_monitor_charging_info[NET_SYSTEM_GUN_NUMBER];
#endif /* NET_YKC_MONITOR_AS_MONITOR */

NET_DEF_SRAM2 static struct ykc_monitor_flag_info s_ykc_monitor_flag_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_ykc_monitor_realtime_data_interval[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_ykc_monitor_realtime_data_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_ykc_monitor_local_start_sq;
NET_DEF_SRAM2 static struct ykc_monitor_state_info s_ykc_monitor_state_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct rt_thread s_ykc_monitor_realtime_process_thread;
NET_DEF_SRAM0 static uint8_t s_ykc_monitor_realtime_process_thread_stack[YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE];
NET_DEF_SRAM2 static struct net_handle* s_ykc_monitor_handle = NULL;

static uint16_t ykc_monitor_chargepile_stop_reason_converted(uint8_t bit, uint8_t stop_in_starting);
static uint8_t ykc_monitor_chargepile_transaction_identity_converted(uint8_t identity);

/*******************************************************
 * 函数名               ykc_monitor_enter_critical
 * 功能                  进入临界区
 * 参数
 * 返回
 ******************************************************/
static void ykc_monitor_enter_critical(void)
{
    rt_enter_critical();
}

/*******************************************************
 * 函数名               ykc_monitor_exit_critical
 * 功能                  退出临界区
 * 参数
 * 返回
 ******************************************************/
static void ykc_monitor_exit_critical(void)
{
    rt_exit_critical();
}

cp56time2a_monitor_t ykc_monitor_get_cp56time2a_from_timestamp(uint32_t timestamp)
{
    struct tm _tm;
    time_t _time = timestamp;
    cp56time2a_monitor_t cp56time2a;

    ykc_monitor_enter_critical();
    _tm = *(localtime(&_time));
    ykc_monitor_exit_critical();

    memset(&cp56time2a, 0x00, sizeof(cp56time2a));

    cp56time2a.cp56time2a_tm.year  = _tm.tm_year + 1900 - 2000;
    cp56time2a.cp56time2a_tm.month = _tm.tm_mon + 1;
    cp56time2a.cp56time2a_tm.mday  = _tm.tm_mday;
    cp56time2a.cp56time2a_tm.wday  = _tm.tm_wday;
    cp56time2a.cp56time2a_tm.hour  = _tm.tm_hour;
    cp56time2a.cp56time2a_tm.min   = _tm.tm_min;
    cp56time2a.cp56time2a_tm.msec  = _tm.tm_sec * 1000;
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

#ifdef NET_YKC_MONITOR_AS_MONITOR
/*************************************************
 * 函数名      ykc_monitor_storage_data_check
 * 功能          校验存储的平台数据
 * **********************************************/
static void ykc_monitor_storage_data_check(void)
{
    uint8_t verify_success = 0x01;
    System_BaseData *base = NULL;
    ykc_monitor_storage_struct *config = (ykc_monitor_storage_struct*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT));

    if(config == NULL){
        verify_success = 0x00;
    }else if((config->verify_result == 0x00) || (config->storage_init_flag != NET_YKC_MONITOR_STORAGE_INIT_FLAG)){
        verify_success = 0x00;
    }

    if(verify_success){
        ykc_monitor_function_switch_set(config);
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            if(config->fswitch.lock == NET_ENUM_TRUE){
                base->device_state = APP_DEVICE_STATE_FREEZE;
            }else {
                base->device_state = APP_DEVICE_STATE_COMMISSIONING;
            }
        }
    }else{

    }

    LOG_D("ykc_monitor_storage_data_check(%d, %d)\n", config->fswitch.tplat_log, config->fswitch.lock);
}
#endif /* NET_YKC_MONITOR_AS_MONITOR */

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
    System_BaseData *base = NULL;
    Net_YkcMonitorPro_PRes_Remote_StartMergeCharge_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Remote_StartMergeCharge_t*)buf);
    base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.serial_number);
    valid_len = valid_len > sizeof(response->body.serial_number) ? sizeof(response->body.serial_number) : valid_len;
    memcpy(response->body.serial_number, g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.serial_number, valid_len);

    valid_len = sizeof(g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.pile_number);
    valid_len = valid_len > sizeof(response->body.pile_number) ? sizeof(response->body.pile_number) : valid_len;
    memcpy(response->body.pile_number, g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.pile_number, valid_len);

    if(base->main_gunno == gunno){
        response->body.main_auxiliary_gun_flag = NET_ENUM_FALSE;
    }else{
        response->body.main_auxiliary_gun_flag = NET_ENUM_TRUE;
    }
    memcpy(response->body.merge_charge_sn, g_ykc_monitor_sreq_remote_start_merge_charge[gunno].body.merge_charge_sn, NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT);

    response->body.gunno = gunno + 0x01;

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
    uint8_t *pile_number = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    uint16_t valid_len = strlen((char*)pile_number);
    Net_YkcMonitorPro_PRes_Qrcode_Config_Tld_t *response = NULL;
    response = ((Net_YkcMonitorPro_PRes_Qrcode_Config_Tld_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = valid_len > (NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_monitor_ascii_to_bcd(pile_number, valid_len, response->body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
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

    System_BaseData *base = NULL;
    Net_YkcMonitorPro_SRes_BillingModel_Request_t *request = (Net_YkcMonitorPro_SRes_BillingModel_Request_t*)data;

    for(uint8_t _gunno = 0x00; _gunno < NET_SYSTEM_GUN_NUMBER; _gunno++){
        uint8_t gunno = _gunno;
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
        if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING) ||
                (base->state.current == APP_OFSM_STATE_STOPING)){
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    Net_YkcMonitorPro_SRes_ApplyCharge_Active_t *request = (Net_YkcMonitorPro_SRes_ApplyCharge_Active_t*)data;

    valid_len = sizeof(request->body.logic_card_number);
    valid_len = valid_len > sizeof(base->card_number) ? sizeof(base->card_number) : valid_len;
    memset(base->card_number, 0x00, sizeof(base->card_number));
    memcpy(base->card_number, request->body.logic_card_number, valid_len);

    valid_len = sizeof(request->body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
    memcpy(base->transaction_number, request->body.serial_number, valid_len);

    valid_len = sizeof(base->card_number);
    valid_len = valid_len > sizeof(base->user_number) ? sizeof(base->user_number) : valid_len;
    memset(base->user_number, 0x00, sizeof(base->user_number));
    memcpy(base->user_number, base->card_number, valid_len);

    base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
    base->charge_strategy_para = request->body.account_ballance *100;

    base->account_ballance_before = request->body.account_ballance;
    base->account_ballance_after = request->body.account_ballance;
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    Net_YkcMonitorPro_SReq_Remote_StartCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StartCharge_t*)data;

    /** 并充时不能让平台启动副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
    }

    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
        return NET_YKC_MONITOR_START_FAIL_REASON_NO_GUN;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        valid_len = sizeof(request->body.logic_card_number);
        valid_len = valid_len > sizeof(base->card_number) ? sizeof(base->card_number) : valid_len;
        memset(base->card_number, 0x00, sizeof(base->card_number));
        memcpy(base->card_number, request->body.logic_card_number, valid_len);

        valid_len = sizeof(request->body.physics_card_number);
        valid_len = valid_len > sizeof(base->card_uid) ? sizeof(base->card_uid) : valid_len;
        memset(base->card_uid, 0x00, sizeof(base->card_uid));
        memcpy(base->card_uid, request->body.physics_card_number, valid_len);

        valid_len = sizeof(request->body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
        memcpy(base->transaction_number, request->body.serial_number, valid_len);

        valid_len = sizeof(base->card_number);
        valid_len = valid_len > sizeof(base->user_number) ? sizeof(base->user_number) : valid_len;
        memset(base->user_number, 0x00, sizeof(base->user_number));
        memcpy(base->user_number, base->card_number, valid_len);

        base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
        base->charge_strategy_para = request->body.account_ballance *100;

        base->account_ballance_before = request->body.account_ballance;
        base->account_ballance_after = request->body.account_ballance;

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

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    /** 并充时不能让平台停止副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if(gunno != base->main_gunno){
            return NET_YKC_MONITOR_STOP_FAIL_REASON_NOT_CHARGING;
        }
    }

    if((base->state.current == APP_OFSM_STATE_CHARGING) ||
            (base->state.current == APP_OFSM_STATE_STARTING)){
        s_ykc_monitor_flag_info[gunno].is_stop_charge = NET_ENUM_TRUE;

        if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
            net_operation_set_event(base->main_gunno, NET_OPERATION_EVENT_STOP_CHARGE);
        }else{
            net_operation_set_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
        }


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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    base->account_ballance_before = request->body.account_amount;
    base->account_ballance_after = request->body.account_amount;

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_set_work_para_request
 * 功能          处理服务器下发的设置桩工作参数请求
 * **********************************************/
int8_t ykc_monitor_message_pro_set_work_para_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YkcMonitorPro_SReq_Set_WorkPara_t), gunno = 0x00;

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YkcMonitorPro_SReq_Set_WorkPara_t *request = (Net_YkcMonitorPro_SReq_Set_WorkPara_t*)data;
    ykc_monitor_storage_struct *config = (ykc_monitor_storage_struct*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT));
    System_BaseData *base = NULL;

    if(request->body.forbidden == NET_ENUM_TRUE){
        /** 启动或充电中不能停用 */
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            if((base->state.current == APP_OFSM_STATE_STARTING) || (base->state.current == APP_OFSM_STATE_CHARGING)){
                return -0x03;
            }
        }
        /** 此处锁桩只是填充信息，具体是否保存成功有设置功率百分比异步响应决定 */
        if(config){
            config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG;
            config->fswitch.lock = NET_ENUM_TRUE;
        }
    }else{
        /** 此处锁桩只是填充信息，具体是否保存成功有设置功率百分比异步响应决定 */
        if(config){
            config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG;
            config->fswitch.lock = NET_ENUM_FALSE;
        }
    }

    base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(0x00));

    LOG_D("ykc_monitor_message_pro_set_work_para_request(%d, %d, %d)\n", request->body.power_max_percent,
            base->system_power_max, (base->system_power_max * request->body.power_max_percent /100));
    if((request->body.power_max_percent > 0x00) && (request->body.power_max_percent <= 0x64)){
        uint32_t power = (base->system_power_max * request->body.power_max_percent /100);
        net_operation_set_total_power(power, 0x00);
        s_ykc_monitor_flag_info[0x00].is_set_power = NET_ENUM_TRUE;
        base->power_strategy = APP_POWER_STRATEGY_SET_LIMIT;
        base->power_strategy_para = 0x00;
    }else{
        return -0x04;
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

    LOG_D("ykc_monitor_message_pro_time_sync_request(%d)[%d, %d, %d, %d, %d, %d]", timestamp,
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
    System_BaseData *base = NULL;
    Net_YkcMonitorPro_SReq_RemoteReboot_t *request = (Net_YkcMonitorPro_SReq_RemoteReboot_t*)data;

    if(request->body.control_cmd == 0x01){
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    valid_len = sizeof(request->body.logic_card_number);
    valid_len = valid_len > sizeof(base->card_number) ? sizeof(base->card_number) : valid_len;
    memset(&(base->card_number), 0x00, sizeof(base->card_number));
    memcpy(&(base->card_number), request->body.logic_card_number, valid_len);

    valid_len = sizeof(request->body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(&(base->transaction_number), 0x00, sizeof(base->transaction_number));
    memcpy(&(base->transaction_number), request->body.serial_number, valid_len);

    valid_len = sizeof(base->card_number);
    valid_len = valid_len > sizeof(base->user_number) ? sizeof(base->user_number) : valid_len;
    memset(base->user_number, 0x00, sizeof(base->user_number));
    memcpy(base->user_number, base->card_number, valid_len);

    base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
    base->charge_strategy_para = request->body.account_ballance *100;

    base->account_ballance_before = request->body.account_ballance;
    base->account_ballance_after = request->body.account_ballance;
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
    if((gunno >= NET_SYSTEM_GUN_NUMBER) && (NET_SYSTEM_GUN_NUMBER >= 0x02)){    /** 并充至少要有两把枪 */
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return NET_YKC_MONITOR_START_FAIL_REASON_BILLING;
    }

    uint8_t valid_len = 0x00, another_gun = 0x01;
    Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t *request = (Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t*)data;
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    /** 并充时不能让平台启动副枪 */
    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
    }

    /** 并充暂时按照两把枪的形式做 */
    if(gunno == 0x01){
        another_gun = 0x00;
    }

    /** 平台并充启动时另一把枪必须处于待充电状态，第一把接收到并充报文的枪规定为主枪 */
    base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(another_gun));
    if((base->state.current != APP_OFSM_STATE_READYING) && (base->state.current != APP_OFSM_STATE_FINISHING)){
        s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_TRUE;
        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
        return NET_YKC_MONITOR_START_FAIL_REASON_GUN_STATE;
    }else{
        if(s_ykc_monitor_flag_info[another_gun].is_request_mergecharge != NET_ENUM_TRUE){
            for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
                base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(i));
                base->main_gunno = gunno;
            }
        }
    }

    base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
        s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_TRUE;
        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
        return NET_YKC_MONITOR_START_FAIL_REASON_NO_GUN;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        valid_len = sizeof(request->body.logic_card_number);
        valid_len = valid_len > sizeof(base->card_number) ? sizeof(base->card_number) : valid_len;
        memset(&(base->card_number), 0x00, sizeof(base->card_number));
        memcpy(&(base->card_number), request->body.logic_card_number, valid_len);

        valid_len = sizeof(request->body.physics_card_number);
        valid_len = valid_len > sizeof(base->card_uid) ? sizeof(base->card_uid) : valid_len;
        memset(&(base->card_uid), 0x00, sizeof(base->card_uid));
        memcpy(&(base->card_uid), request->body.physics_card_number, valid_len);

        valid_len = sizeof(request->body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(&(base->transaction_number), 0x00, sizeof(base->transaction_number));
        memcpy(&(base->transaction_number), request->body.serial_number, valid_len);

        valid_len = sizeof(base->card_number);
        valid_len = valid_len > sizeof(base->user_number) ? sizeof(base->user_number) : valid_len;
        memset(base->user_number, 0x00, sizeof(base->user_number));
        memcpy(base->user_number, base->card_number, valid_len);

        base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
        base->charge_strategy_para = request->body.account_ballance *100;

        base->account_ballance_before = request->body.account_ballance;
        base->account_ballance_after = request->body.account_ballance;

        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_TRUE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_TRUE;

        /** 这是第二把接收到并充报文的前，两把枪都接收到了并充报文，可以启动并充了，并充相当于两把枪单独充电 */
        if(s_ykc_monitor_flag_info[another_gun].is_request_mergecharge == NET_ENUM_TRUE){
            for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
                base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(i));
                net_operation_set_event(i, NET_OPERATION_EVENT_START_CHARGE);
                net_operation_clear_event(i, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
                base->charge_way = APP_CHARGE_WAY_PARACHARGE_CLOUD;
            }
        }

        return NET_YKC_MONITOR_START_FAIL_REASON_NO;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_TRUE;
        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_CHARGING;
        break;
    default:
        s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_TRUE;
        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
        return NET_YKC_MONITOR_START_FAIL_REASON_IS_FAULTING;
        break;
    }

    s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_TRUE;
    s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
    s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;

    return NET_YKC_MONITOR_START_FAIL_REASON_NO;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_AS_MONITOR */
}

/*************************************************
 * 函数名      ykc_monitor_message_field_init
 * 功能          云快充监控报文字段初始化
 * **********************************************/
void ykc_monitor_message_field_init(uint8_t gun)
{
    if(gun >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = NULL;
    uint16_t valid_len = 0x00;
    s_ykc_monitor_handle = net_get_net_handle();
    pile_number = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gun));
    valid_len = strlen((char*)pile_number);

    /** 初始化登录签到 */
    g_ykc_monitor_preq_login.head.encrypt = NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE;
    g_ykc_monitor_preq_login.head.sequence = 0x00;

#ifdef NET_YKC_MONITOR_AS_MONITOR
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint8_t used_len = 0x00;
    ykc_monitor_socket_info_t *socket_info = ykc_monitor_get_socket_info();

    valid_len = valid_len > (used_len + (strlen((char*)pile_number) + 0x01)) ? (used_len + (strlen((char*)pile_number) + 0x01)) : valid_len;
    if(valid_len){
        sprintf((char*)g_ykc_monitor_preq_login.body.pile_number, "%s_", pile_number);
    }
    used_len += (used_len + (strlen((char*)pile_number) + 0x01));

    valid_len = valid_len > (used_len + strlen((char*)socket_info->target_plat_ip)) ? (used_len + strlen((char*)socket_info->target_plat_ip)) : valid_len;
    if(valid_len){
        sprintf((char*)(g_ykc_monitor_preq_login.body.pile_number + used_len), "%s", socket_info->target_plat_ip);
    }
#else
    valid_len = valid_len > (NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_monitor_ascii_to_bcd(pile_number, valid_len, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#else
    valid_len = valid_len > (NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ykc_monitor_ascii_to_bcd(pile_number, valid_len, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
#endif /* NET_YKC_MONITOR_AS_MONITOR */

    g_ykc_monitor_preq_login.body.pile_type = NET_YKC_MONITOR_PILE_TYPE_DC;
    g_ykc_monitor_preq_login.body.gun_count = NET_SYSTEM_GUN_NUMBER;
    g_ykc_monitor_preq_login.body.protocol_ver = NET_YKC_MONITOR_PROTOCOL_VERSION;

    memset(g_ykc_monitor_preq_login.body.software_ver, '\0', sizeof(g_ykc_monitor_preq_login.body.software_ver));
    g_ykc_monitor_preq_login.body.software_ver[0] = base->soft_ver_main + '0';
    g_ykc_monitor_preq_login.body.software_ver[1] = '.';
    g_ykc_monitor_preq_login.body.software_ver[2] = base->soft_ver_sub + '0';
    g_ykc_monitor_preq_login.body.software_ver[3] = '.';
    sprintf((char *)&g_ykc_monitor_preq_login.body.software_ver[3 + 1], "%02d", base->soft_ver_revise);

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
 * 函数名      ykc_monitor_message_info_init
 * 功能          云快充监控报文信息初始化
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

    s_ykc_monitor_handle = net_get_net_handle();

    ykc_monitor_is_init = NET_ENUM_TRUE;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_monitor_realtime_data_interval[gunno] = YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT;
        s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
    }
    memset(&s_ykc_monitor_flag_info, 0x00, sizeof(s_ykc_monitor_flag_info));

    ykc_monitor_message_field_init(gunno);
#ifdef NET_YKC_MONITOR_AS_MONITOR
    ykc_monitor_storage_data_check();
#endif /* NET_YKC_MONITOR_AS_MONITOR */
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

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(is_init){
        uint8_t valid_len = sizeof(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_report_realtime_data[gunno].body.serial_number, base->transaction_number, valid_len);

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
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING)){
                struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_voltage = base->voltage_a /10;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_current = base->current_a /10;
                if(base->gunline_temperature[0] > base->gunline_temperature[1]){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = (base->gunline_temperature[0] /10 + 50);
                }else{
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = (base->gunline_temperature[1] /10 + 50);
                }
                g_ykc_monitor_preq_report_realtime_data[gunno].body.soc = base->current_soc;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.battery_group_temp_max = (bms->BSM.HigTemp + 50);
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_time = base->charge_time /60;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.remain_time = bms->BCS.SurplChgTime;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_elect = base->elect_a *10;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.loss_elect = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.consume_amount = base->fees_total;
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_shake_hand[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_parameter_config[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_single_bat_allow_volt_max = bms->BCP.CellAlowHigVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_curr_max = (4000 - bms->BCP.AlowCurlt);
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_bat_nominal_energy_all = bms->BCP.BatRateKW;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_volt_max = bms->BCP.BatAlowHigVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_temp_max = (bms->BCP.BatAlowHigTemp + 50);
    g_ykc_monitor_preq_parameter_config[gunno].body.soc = bms->BCP.SOC;
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_bat_current_volt = bms->BCP.BatVolt;

    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_volt_max = bms->CML.ChagHigOutVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_volt_min = bms->CML.ChagLowOutVolt;
    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_curr_max = bms->CML.ChagHigOutCurlt;
    g_ykc_monitor_preq_parameter_config[gunno].body.pile_output_curr_min = bms->CML.ChagLowOutCurlt;

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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_charge_finish[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_error_message[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_error_message[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_error_message[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_error_message[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_bms_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_bms_end[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bms_end[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_bms_end[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ykc_monitor_preq_charger_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ykc_monitor_preq_charger_end[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_charger_end[gunno].body.serial_number));
    memcpy(g_ykc_monitor_preq_charger_end[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
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
        System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

        valid_len = sizeof(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number, base->transaction_number, valid_len);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_volt = (base->voltage_a /10);
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_curr = (4000 - (base->current_a /10));
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.charge_time = base->charge_time /60;

            if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_command = bms->BCL.BMSneedVolt;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_command = (4000 - bms->BCL.BMSneedCurlt);
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = bms->BCL.ChagModel;

            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = bms->BCS.ChargVolt;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = (4000 - bms->BCS.ChargCurlt);
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.max_volt_and_gn.bms_max_single_bat_volt = bms->BCS.CellHigVolt;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.max_volt_and_gn.max_single_bat_volt_gn = bms->BCS.HigVoltCellNum;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_current_soc = bms->BCS.SOC;
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_remain_charge_time = bms->BCS.SurplChgTime;
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
        System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

        valid_len = sizeof(g_ykc_monitor_preq_bms_info[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_bms_info[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bms_info[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_bms_info[gunno].body.serial_number, base->transaction_number, valid_len);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_BMS_INFO) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

            if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
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
    System_BaseData *base =  (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(!base->flag.card_info_is_uid){
        return -0x01;
    }
    g_ykc_monitor_preq_apply_charge_active[gunno].body.start_type = 0x01;
    g_ykc_monitor_preq_apply_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;

    valid_len = sizeof(base->card_uid);
    valid_len = valid_len > NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX : valid_len;
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    if(valid_len > base->card_uid_len){
        memcpy((g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number + (valid_len - base->card_uid_len)),
                base->card_uid, base->card_uid_len);
    }else{
        memcpy(g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number, base->card_uid, valid_len);
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
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_apply_charge_active[gunno].body.start_type = 0x03;
    g_ykc_monitor_preq_apply_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    memset(g_ykc_monitor_preq_apply_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    for(uint8_t count = 0x00; count < NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX; count++){
        g_ykc_monitor_preq_apply_charge_active[gunno].body.vin[count] = base->car_vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
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

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    ykc_monitor_set_transaction_verify_state(gunno, 0x00);

    if((ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD) == NET_YKC_MONITOR_SEND_STATE_COMPLETE)){
        valid_len = sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number);
        valid_len = valid_len > sizeof(_transaction->serial_number) ? sizeof(_transaction->serial_number) : valid_len;
        memset(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.serial_number, _transaction->serial_number, valid_len);

        if(_transaction->end_time > _transaction->charge_time){
            _transaction->start_time = _transaction->end_time - _transaction->charge_time;
        }else{
            _transaction->start_time = _transaction->end_time;
            _transaction->charge_time = 0x00;
        }
        g_ykc_monitor_preq_transaction_records[gunno].body.start_time = ykc_monitor_get_cp56time2a_from_timestamp(_transaction->start_time);
        g_ykc_monitor_preq_transaction_records[gunno].body.stop_time = ykc_monitor_get_cp56time2a_from_timestamp(_transaction->end_time);

        g_ykc_monitor_preq_transaction_records[gunno].body.tip_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_SHARP] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.tip_elect = _transaction->rate_type_elect[APP_RATE_TYPE_SHARP] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.tip_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_SHARP] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.tip_amount = _transaction->rate_type_amount[APP_RATE_TYPE_SHARP];
        if(g_ykc_monitor_preq_transaction_records[gunno].body.tip_amount > YKC_MONITOR_SPEND_AMOUNT_MAX){
            g_ykc_monitor_preq_transaction_records[gunno].body.tip_amount = YKC_MONITOR_SPEND_AMOUNT_MAX;
        }
        if(g_ykc_monitor_preq_transaction_records[gunno].body.tip_elect > YKC_MONITOR_CHARGE_ELECT_MAX *10){
            g_ykc_monitor_preq_transaction_records[gunno].body.tip_elect = YKC_MONITOR_CHARGE_ELECT_MAX *10;
        }

        g_ykc_monitor_preq_transaction_records[gunno].body.peak_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_PEAK] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.peak_elect = _transaction->rate_type_elect[APP_RATE_TYPE_PEAK] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.peak_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_PEAK] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.peak_amount = _transaction->rate_type_amount[APP_RATE_TYPE_PEAK];
        if(g_ykc_monitor_preq_transaction_records[gunno].body.peak_amount > YKC_MONITOR_SPEND_AMOUNT_MAX){
            g_ykc_monitor_preq_transaction_records[gunno].body.peak_amount = YKC_MONITOR_SPEND_AMOUNT_MAX;
        }
        if(g_ykc_monitor_preq_transaction_records[gunno].body.peak_elect > YKC_MONITOR_CHARGE_ELECT_MAX *10){
            g_ykc_monitor_preq_transaction_records[gunno].body.peak_elect = YKC_MONITOR_CHARGE_ELECT_MAX *10;
        }

        g_ykc_monitor_preq_transaction_records[gunno].body.flat_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_FLAT] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.flat_elect = _transaction->rate_type_elect[APP_RATE_TYPE_FLAT] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.flat_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_FLAT] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.flat_amount = _transaction->rate_type_amount[APP_RATE_TYPE_FLAT];
        if(g_ykc_monitor_preq_transaction_records[gunno].body.flat_amount > YKC_MONITOR_SPEND_AMOUNT_MAX){
            g_ykc_monitor_preq_transaction_records[gunno].body.flat_amount = YKC_MONITOR_SPEND_AMOUNT_MAX;
        }
        if(g_ykc_monitor_preq_transaction_records[gunno].body.flat_elect > YKC_MONITOR_CHARGE_ELECT_MAX *10){
            g_ykc_monitor_preq_transaction_records[gunno].body.flat_elect = YKC_MONITOR_CHARGE_ELECT_MAX *10;
        }

        g_ykc_monitor_preq_transaction_records[gunno].body.valley_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_VALLEY] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.valley_elect = _transaction->rate_type_elect[APP_RATE_TYPE_VALLEY] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.valley_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_VALLEY] *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.valley_amount = _transaction->rate_type_amount[APP_RATE_TYPE_VALLEY];
        if(g_ykc_monitor_preq_transaction_records[gunno].body.valley_amount > YKC_MONITOR_SPEND_AMOUNT_MAX){
            g_ykc_monitor_preq_transaction_records[gunno].body.valley_amount = YKC_MONITOR_SPEND_AMOUNT_MAX;
        }
        if(g_ykc_monitor_preq_transaction_records[gunno].body.valley_elect > YKC_MONITOR_CHARGE_ELECT_MAX *10){
            g_ykc_monitor_preq_transaction_records[gunno].body.valley_elect = YKC_MONITOR_CHARGE_ELECT_MAX *10;
        }

        elect = _transaction->ammeter_start *10;
        memset(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val));
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val, &elect, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_start_val));
        elect = _transaction->ammeter_stop *10;
        memset(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val, 0x00, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val));
        memcpy(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val, &elect, sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.ammeter_end_val));

        g_ykc_monitor_preq_transaction_records[gunno].body.total_elect = _transaction->total_elect *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.total_loss_elect = _transaction->total_loss_elect *10;
        g_ykc_monitor_preq_transaction_records[gunno].body.consume_amount = _transaction->total_fee;
        if(g_ykc_monitor_preq_transaction_records[gunno].body.consume_amount > YKC_MONITOR_SPEND_AMOUNT_MAX){
            g_ykc_monitor_preq_transaction_records[gunno].body.consume_amount = YKC_MONITOR_SPEND_AMOUNT_MAX;
        }
        if(g_ykc_monitor_preq_transaction_records[gunno].body.total_elect > YKC_MONITOR_CHARGE_ELECT_MAX *10){
            g_ykc_monitor_preq_transaction_records[gunno].body.total_elect = YKC_MONITOR_CHARGE_ELECT_MAX *10;
        }

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

        g_ykc_monitor_preq_transaction_records[gunno].body.transaction_date = ykc_monitor_get_cp56time2a_from_timestamp(base->current_time);
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
    struct tm _tm;
    time_t _time = time(NULL);
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.start_type = 0x01;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;

    valid_len = sizeof(base->card_uid);
    valid_len = valid_len > NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX ? NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX : valid_len;
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    memcpy(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.account_or_phycard_number, base->card_uid, valid_len);
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.vin, 0x00, NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX);

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.main_auxiliary_gun_flag = NET_ENUM_FALSE;   /* 默认主枪申请 */

    ykc_monitor_enter_critical();
    _tm = *(localtime(&_time));
    ykc_monitor_exit_critical();

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[0] = _tm.tm_year;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[1] = _tm.tm_mon;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[2] = _tm.tm_mday;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[3] = _tm.tm_hour;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[4] = _tm.tm_min;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[5] = _tm.tm_sec;

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
    struct tm _tm;
    time_t _time = time(NULL);
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.start_type = 0x03;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);
    memset(g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.password, 0x00, NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT);
    for(uint8_t count = 0x00; count < NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX; count++){
        g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.vin[count] = base->car_vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
    }
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.main_auxiliary_gun_flag = NET_ENUM_FALSE;   /* 默认主枪申请 */

    ykc_monitor_enter_critical();
    _tm = *(localtime(&_time));
    ykc_monitor_exit_critical();

    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[0] = _tm.tm_year;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[1] = _tm.tm_mon;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[2] = _tm.tm_mday;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[3] = _tm.tm_hour;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[4] = _tm.tm_min;
    g_ykc_monitor_preq_apply_merge_charge_active[gunno].body.merge_charge_sn[5] = _tm.tm_sec;

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
 * 函数名      ykc_monitor_stop_charge_response_asynchronously
 * 功能          充电桩报文响应事件：停止充电异步响应
 * **********************************************/
void ykc_monitor_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        gunno = base->main_gunno;
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

    if(s_ykc_monitor_flag_info[0x00].set_power_success == NET_ENUM_TRUE){
        System_BaseData *base = NULL;
        ykc_monitor_storage_struct *config = (ykc_monitor_storage_struct*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT));
        if(config){
            /** 功率修改与锁桩功能在同一个报文中，为了提高效率，锁桩是否成功都有功率是否修改成功来决定(信息要存flash-耗时) */
            for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
                if(config->fswitch.lock == NET_ENUM_TRUE){
                    base->device_state = APP_DEVICE_STATE_FREEZE;
                }else {
                    base->device_state = APP_DEVICE_STATE_COMMISSIONING;
                }
            }
        }
    }

    s_ykc_monitor_flag_info[0x00].is_set_power = NET_ENUM_FALSE;
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00, NET_YKC_MONITOR_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY);
}

void ykc_monitor_chargepile_state_changed(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    if(base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        s_ykc_monitor_state_info[gunno].state.connect = NET_ENUM_TRUE;
    }else{
        s_ykc_monitor_state_info[gunno].state.connect = NET_ENUM_FALSE;
    }
    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_STARTING:
        s_ykc_monitor_flag_info[gunno].is_charge_finish = NET_ENUM_FALSE;
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_IDLE;
        break;
    case APP_OFSM_STATE_CHARGING:
        s_ykc_monitor_flag_info[gunno].is_charge_finish = NET_ENUM_FALSE;
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_CHARGING;
        break;
    case APP_OFSM_STATE_STOPING:
        s_ykc_monitor_flag_info[gunno].is_charge_finish = NET_ENUM_FALSE;
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_IDLE;
        break;
    case APP_OFSM_STATE_FINISHING:
        if(s_ykc_monitor_flag_info[gunno].is_charge_finish == NET_ENUM_FALSE){
#ifdef NET_YKC_MONITOR_AS_MONITOR
            ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                    gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_CHARGE_FINISH);
#endif /* NET_YKC_MONITOR_AS_MONITOR */
        }
        s_ykc_monitor_flag_info[gunno].is_charge_finish = NET_ENUM_TRUE;
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_IDLE;
        break;
    case APP_OFSM_STATE_FAULTING:
        s_ykc_monitor_flag_info[gunno].is_charge_finish = NET_ENUM_FALSE;
        s_ykc_monitor_state_info[gunno].state.state = NETYKC_MONITOR_DEVICE_STATE_FAULTING;
        break;
    default:
        s_ykc_monitor_flag_info[gunno].is_charge_finish = NET_ENUM_FALSE;
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
    struct tm _tm;

    s_ykc_monitor_local_start_sq++;
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    sn_len = 0x07;    /** 受限于云快充协议 */
#else /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
    sn_len = sizeof(g_ykc_monitor_preq_transaction_records[gunno].body.pile_number);
#endif
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    ykc_monitor_enter_critical();
    _tm = *(localtime((const time_t*)&(base->current_time)));
    ykc_monitor_exit_critical();

    LOG_D("ykc_chargepile_create_local_transaction_number[%d](%d/%d/%d %d:%d:%d)", base->current_time, _tm.tm_year, _tm.tm_mon,
            _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
    memcpy(ptr, g_ykc_monitor_preq_transaction_records[gunno].body.pile_number, sn_len);   /* 桩号 */
    ptr[sn_len++] = gunno + 1;                                                     /* 枪号 */
    ptr[sn_len++] = (((_tm.tm_year - 100) /10) *16) + ((_tm.tm_year - 100) %10); /* 年 */
    ptr[sn_len++] = (((_tm.tm_mon + 0x01) /10) *16) + ((_tm.tm_mon + 0x01) %10); /* 月 */
    ptr[sn_len++] = ((_tm.tm_mday /10) *16) +  (_tm.tm_mday %10);                /* 日 */
    ptr[sn_len++] = ((_tm.tm_hour /10) *16) + (_tm.tm_hour %10);                 /* 时 */
    ptr[sn_len++] = ((_tm.tm_min /10) *16) + (_tm.tm_min %10);                   /* 分 */
    ptr[sn_len++] = ((_tm.tm_sec /10) *16) + (_tm.tm_sec %10);                   /* 秒 */
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

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    g_ykc_monitor_preq_transaction_records[gunno].body.start_time = ykc_monitor_get_cp56time2a_from_timestamp(base->start_time);
    g_ykc_monitor_preq_transaction_records[gunno].body.stop_time = ykc_monitor_get_cp56time2a_from_timestamp(base->stop_time);
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
    /* 读卡器 */
    case APP_SYSTEM_STOP_WAY_CARDREADER:
        _reason = NETYKC_MONITOR_AS_REASON8A_CARDREADER;
        break;
    /* 门禁 */
    case APP_SYSTEM_STOP_WAY_DOOR:
        _reason = NETYKC_MONITOR_AS_REASON91_GATE;
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

static void ykc_monitor_data_realtime_process(uint8_t gunno, System_BaseData* base)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(base == NULL){
        return;
    }

    if(ykc_monitor_get_socket_info()->state == YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
        ykc_monitor_request_message_repeat(gunno);

        if(base->state.current == APP_OFSM_STATE_CHARGING){
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

#ifdef NET_YKC_MONITOR_AS_MONITOR
        if((rt_tick_get() - s_ykc_monitor_setvoltcurr.base_tick) > 1500){   /** 1.5秒采一次数据 */
            ykc_monitor_padding_setvoltcurr_data();
            s_ykc_monitor_setvoltcurr.base_tick = rt_tick_get();
        }
        if(s_ykc_monitor_setvoltcurr.count >= NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX){
            if(s_ykc_monitor_setvoltcurr.is_locked == NET_ENUM_FALSE){
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                        0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_SET_VOLTCURR);
                s_ykc_monitor_setvoltcurr.is_locked = NET_ENUM_TRUE;
            }
        }else{
            s_ykc_monitor_setvoltcurr.is_locked = NET_ENUM_FALSE;
        }

        /*********************************** 启动中信息 ************************************/
        /*********************************** 启动中信息 ************************************/
        if((base->state.current == APP_OFSM_STATE_STARTING) ||
                ((base->state.current == APP_OFSM_STATE_STOPING) && (base->flag.start_result == NET_ENUM_FALSE))){  /** 启动失败时停止阶段也采样，防止状态变化不同步 */
            if((rt_tick_get() - s_ykc_monitor_starting_info[gunno].base_tick) > 1000){   /** 1秒采一次数据 */
                ykc_monitor_padding_starting_info(gunno);
                s_ykc_monitor_starting_info[gunno].base_tick = rt_tick_get();
            }
            if(s_ykc_monitor_starting_info[gunno].count >= NET_YKC_MONITOR_STARTING_INFO_MAX){
                if(s_ykc_monitor_starting_info[gunno].is_locked == NET_ENUM_FALSE){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_STARTING_INFO);
                    s_ykc_monitor_starting_info[gunno].is_locked = NET_ENUM_TRUE;
                }
            }else{
                s_ykc_monitor_starting_info[gunno].is_locked = NET_ENUM_FALSE;
            }
        }else{
            if(s_ykc_monitor_starting_info[gunno].count != 0x00){
                if(s_ykc_monitor_starting_info[gunno].is_locked == NET_ENUM_FALSE){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_STARTING_INFO);
                    s_ykc_monitor_starting_info[gunno].is_locked = NET_ENUM_TRUE;
                }
            }else{
                s_ykc_monitor_starting_info[gunno].count = 0x00;
                s_ykc_monitor_starting_info[gunno].is_locked = NET_ENUM_FALSE;
            }
        }

        /*********************************** 充电中信息 ************************************/
        /*********************************** 充电中信息 ************************************/
        if(base->state.current == APP_OFSM_STATE_CHARGING){
            if((rt_tick_get() - s_ykc_monitor_charging_info[gunno].base_tick) > 1500){   /** 1.5秒采一次数据 */
                ykc_monitor_padding_charging_info(gunno);
                s_ykc_monitor_charging_info[gunno].base_tick = rt_tick_get();
            }
            if(s_ykc_monitor_charging_info[gunno].count >= NET_YKC_MONITOR_CHARGING_INFO_MAX){
                if(s_ykc_monitor_charging_info[gunno].is_locked == NET_ENUM_FALSE){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_CHARGING_INFO);
                    s_ykc_monitor_charging_info[gunno].is_locked = NET_ENUM_TRUE;
                }
            }else{
                s_ykc_monitor_charging_info[gunno].is_locked = NET_ENUM_FALSE;
            }
        }else{
            if(s_ykc_monitor_charging_info[gunno].count != 0x00){
                if(s_ykc_monitor_charging_info[gunno].is_locked == NET_ENUM_FALSE){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_CHARGING_INFO);
                    s_ykc_monitor_charging_info[gunno].is_locked = NET_ENUM_TRUE;
                }
            }else{
                s_ykc_monitor_charging_info[gunno].count = 0x00;
                s_ykc_monitor_charging_info[gunno].is_locked = NET_ENUM_FALSE;
            }
        }
#endif /* NET_YKC_MONITOR_AS_MONITOR */
    }else{
#ifdef NET_YKC_MONITOR_AS_MONITOR
        s_ykc_monitor_setvoltcurr.base_tick = rt_tick_get();
        s_ykc_monitor_starting_info[gunno].base_tick = rt_tick_get();
        s_ykc_monitor_charging_info[gunno].base_tick = rt_tick_get();
#endif /* NET_YKC_MONITOR_AS_MONITOR */
        s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
        s_ykc_monitor_realtime_data_interval[gunno] = YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT;
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
    }

    switch(base->state.current){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_STOPING:
        s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
        ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_CHARGE);
        break;
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        s_ykc_monitor_flag_info[gunno].is_refuse_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_request_mergecharge = NET_ENUM_FALSE;
        s_ykc_monitor_flag_info[gunno].is_start_mergecharge = NET_ENUM_FALSE;
        break;
    case APP_OFSM_STATE_STARTING:
        if(base->start_type != APP_CHARGE_START_WAY_VIN){
            ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_CHARGE);
        }
        ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD);
        break;
    case APP_OFSM_STATE_CHARGING:
        ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_APPLY_START_CHARGE);
        ykc_monitor_clear_message_wait_response_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_TRANSACTION_RECORD);
        break;
    default:
        break;
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
    System_BaseData *base = NULL;
    uint8_t gunno = 0x00;

    while(1){
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }
        if(s_ykc_monitor_handle == NULL){
            rt_thread_mdelay(100);
            continue;
        }

        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            ykc_monitor_fault_detect_report(gunno);
            ykc_monitor_data_realtime_process(gunno, base);
            ykc_monitor_state_changed_check(gunno);
        }

        rt_thread_mdelay(100);
    }
}

int32_t ykc_monitor_realtime_process_init(void)
{
#ifdef NET_DESIGNATE_REGION
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_monitor_realtime_data_interval[gunno] = 0x00;
        s_ykc_monitor_realtime_data_count[gunno] = 0x00;

        memset(&s_ykc_monitor_flag_info[gunno], 0x00, sizeof(s_ykc_monitor_flag_info[gunno]));
        memset(&s_ykc_monitor_state_info[gunno], 0x00, sizeof(s_ykc_monitor_state_info[gunno]));

#ifdef NET_YKC_MONITOR_AS_MONITOR
        memset(&s_ykc_monitor_starting_info[gunno], 0x00, sizeof(s_ykc_monitor_starting_info[gunno]));
        memset(&s_ykc_monitor_charging_info[gunno], 0x00, sizeof(s_ykc_monitor_charging_info[gunno]));
#endif /* NET_YKC_MONITOR_AS_MONITOR */
    }

#ifdef NET_YKC_MONITOR_AS_MONITOR
    memset(&s_ykc_monitor_setvoltcurr, 0x00, sizeof(s_ykc_monitor_setvoltcurr));
#endif /* NET_YKC_MONITOR_AS_MONITOR */
    s_ykc_monitor_local_start_sq = 0x00;
    s_ykc_monitor_handle = NULL;
#endif /* NET_DESIGNATE_REGION */

    if(rt_thread_init(&s_ykc_monitor_realtime_process_thread, "ykc_mrl_pro", ykc_monitor_realtime_process_thread_entry, NULL,
            s_ykc_monitor_realtime_process_thread_stack, YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ykc monitor realtime process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ykc_monitor_realtime_process_thread) != RT_EOK){
        LOG_E("ykc monitor realtime process thread startup fail, please check");
        return -0x01;
    }

#ifdef NET_YKC_MONITOR_AS_MONITOR
    s_ykc_monitor_setvoltcurr.count = 0x00;
    s_ykc_monitor_setvoltcurr.is_locked = NET_ENUM_FALSE;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_monitor_starting_info[gunno].count = 0x00;
        s_ykc_monitor_starting_info[gunno].is_locked = NET_ENUM_FALSE;

        s_ykc_monitor_charging_info[gunno].count = 0x00;
        s_ykc_monitor_charging_info[gunno].is_locked = NET_ENUM_FALSE;
    }
#endif /* NET_YKC_MONITOR_AS_MONITOR */

    return 0x00;
}

/******************************** 以下是监控报文 *******************************/
/******************************** 以下是监控报文 *******************************/
#ifdef NET_YKC_MONITOR_AS_MONITOR
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

/*************************************************
 * 函数名      ykc_monitor_message_padding_tsocket_info
 * 功能          组包：目标socket信息
 * **********************************************/
int8_t ykc_monitor_message_padding_tsocket_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = NULL;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
    uint8_t valid_len = 0x00;
    net_plat_socket_info_t *socket = net_operation_get_target_socket_info();
    Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t *message = (Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t*)buf;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    pile_number = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    valid_len = sizeof(message->body.pile_number_whole);
    valid_len = valid_len > strlen((char*)pile_number) ? strlen((char*)pile_number) : valid_len;
    memset(message->body.pile_number_whole, 0x00, sizeof(message->body.pile_number_whole));
    memcpy(message->body.pile_number_whole, pile_number, valid_len);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

    valid_len = sizeof(message->body.domain);
    valid_len = valid_len > sizeof(socket->domain) ? sizeof(socket->domain) : valid_len;
    memcpy(message->body.domain, socket->domain, valid_len);

    message->body.port = socket->port;
    message->body.state = socket->state;
    message->body.open_count = socket->open_count;
    message->body.login_count = socket->login_count;

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_log_info
 * 功能          组包：目标平台日志信息
 * **********************************************/
int8_t ykc_monitor_message_padding_log_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_Preq_TargetPlat_Log_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = NULL, valid_len;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
    Net_YkcMonitorPro_Preq_TargetPlat_Log_t *message = (Net_YkcMonitorPro_Preq_TargetPlat_Log_t*)buf;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    pile_number = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    valid_len = sizeof(message->body.pile_number_whole);
    valid_len = valid_len > strlen((char*)pile_number) ? strlen((char*)pile_number) : valid_len;
    memset(message->body.pile_number_whole, 0x00, sizeof(message->body.pile_number_whole));
    memcpy(message->body.pile_number_whole, pile_number, valid_len);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_dev_info
 * 功能          组包：设备信息
 * **********************************************/
int8_t ykc_monitor_message_padding_dev_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_Preq_PRes_DevInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t valid_len = 0x00, *data = NULL;
    Net_YkcMonitorPro_Preq_PRes_DevInfo_t *message = (Net_YkcMonitorPro_Preq_PRes_DevInfo_t*)buf;
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    data = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));

#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    valid_len = sizeof(message->body.pile_number_whole);
    valid_len = valid_len > strlen((char*)data) ? strlen((char*)data) : valid_len;
    memset(message->body.pile_number_whole, 0x00, sizeof(message->body.pile_number_whole));
    memcpy(message->body.pile_number_whole, data, valid_len);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

    data = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_NETWORKED_WAY, NULL, 0x00, option));
    if(*data == *(uint8_t*)(NET_NETWORKED_WAY_4G)){
        message->body.interconnecting_way = 0x01;
    }else{
        message->body.interconnecting_way = 0x02;
    }

    data = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_HARDWARE_INFO, NULL, 0x00, option));
    valid_len = sizeof(message->body.hardware);
    valid_len = valid_len > strlen((char*)data) ? strlen((char*)data) : valid_len;
    memset(message->body.hardware, 0x00, sizeof(message->body.hardware));
    memcpy(message->body.hardware, data, valid_len);

    data = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_SOFTWARE_MODEL, NULL, 0x00, option));
    valid_len = sizeof(message->body.soft_model);
    valid_len = valid_len > strlen((char*)data) ? strlen((char*)data) : valid_len;
    memset(message->body.soft_model, 0x00, sizeof(message->body.soft_model));
    memcpy(message->body.soft_model, data, valid_len);

    data = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_DEV_TYPE, NULL, 0x00, option));
    if(*data == *(uint8_t*)(NET_DEV_TYPE_DYNAMIC_DOUBLEGUN)){
        message->body.dev_type = 0x02;
    }else if(*data == *(uint8_t*)(NET_DEV_TYPE_SINGLEGUN_TERMINAL)){
        message->body.dev_type = 0x03;
    }else if(*data == *(uint8_t*)(NET_DEV_TYPE_DOUBLEGUN_TERMINAL)){
        message->body.dev_type = 0x04;
    }else if(*data == *(uint8_t*)(NET_DEV_TYPE_MAIN_CABINET)){
        message->body.dev_type = 0x05;
    }else if(*data == *(uint8_t*)(NET_DEV_TYPE_SUPER_SINGLEGUN)){
        message->body.dev_type = 0x03;
    }else{
        message->body.dev_type = 0x01;
    }

#ifdef NET_INCLUDE_TARGET_PLATFORM
#if (NET_TARGET_PLATFORM_ID == NET_YKC_PRO_ID)
#ifdef NET_YKC_DERIVE_PRO_XXCD
    message->body.target_plat_protocol = 0x07;
#elif defined(NET_YKC_DERIVE_PRO_TLD)
    message->body.target_plat_protocol = 0x08;
#elif defined(NET_YKC_DERIVE_PRO_DUPU)
    message->body.target_plat_protocol = 0x09;
#else
    message->body.target_plat_protocol = 0x0A;
#endif /* NET_YKC_DERIVE_PRO_XXCD */

#elif (NET_TARGET_PLATFORM_ID == NET_YCP_PRO_ID)
    message->body.target_plat_protocol = 0x02;
#elif (NET_TARGET_PLATFORM_ID == NET_YND_PRO_ID)
    message->body.target_plat_protocol = 0x04;
#elif (NET_TARGET_PLATFORM_ID == NET_XJ_PRO_ID)
    message->body.target_plat_protocol = 0x05;
#elif (NET_TARGET_PLATFORM_ID == NET_SL_PRO_ID)
    message->body.target_plat_protocol = 0x06;
#elif (NET_TARGET_PLATFORM_ID == NET_SGCC_PRO_ID)
    message->body.target_plat_protocol = 0x03;
#else
    message->body.target_plat_protocol = 0x01;
#endif

#else
    message->body.target_plat_protocol = 0x01;
#endif /* NET_INCLUDE_TARGET_PLATFORM */

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_padding_setvoltcurr_data
 * 功能         按枪填写给模块设置的输出电压、电流数据
 * 参数
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_padding_setvoltcurr_data(void)
{
    if(s_ykc_monitor_setvoltcurr.count >= NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX){
        return -0x01;
    }

    extern uint8_t thaisenGetModuleGroupOpenState(uint8_t groupNum);
    extern uint32_t thaisenGetModuleSetVoltage(uint8_t groupNum);
    extern uint32_t thaisenGetModuleSetCurrent(uint8_t groupNum);

    if(s_ykc_monitor_setvoltcurr.count == 0x00){
        System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(0x00));
        s_ykc_monitor_setvoltcurr.timestamp = base->current_time;
    }

    for(uint8_t group = 0x00; group < YKC_MONITOR_MODULE_GROUP_MAX; group++){
        s_ykc_monitor_setvoltcurr.pair[group][s_ykc_monitor_setvoltcurr.count].voltage = thaisenGetModuleSetVoltage(group);
        s_ykc_monitor_setvoltcurr.pair[group][s_ykc_monitor_setvoltcurr.count].current = thaisenGetModuleSetCurrent(group);
        s_ykc_monitor_setvoltcurr.pair[group][s_ykc_monitor_setvoltcurr.count].is_open = 0x00;
        if(thaisenGetModuleGroupOpenState(group)){
            s_ykc_monitor_setvoltcurr.pair[group][s_ykc_monitor_setvoltcurr.count].is_open = 0x01;
        }
    }
    s_ykc_monitor_setvoltcurr.count++;

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_setvoltcurr
 * 功能         组包：填充给模块设置的电压、电流数据
 * 参数         gunno   枪号
 *       buf      缓存
 *       ilen    输入缓存长度
 *       olen    填写数据总长度
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_message_padding_setvoltcurr(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = (sizeof(Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t) + sizeof(s_ykc_monitor_setvoltcurr.pair));

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    struct voltcurr_pair *pair = (struct voltcurr_pair*)(buf + sizeof(Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t *message = (Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t*)buf;
    uint8_t group_num = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));

    if(group_num > YKC_MONITOR_MODULE_GROUP_MAX){
        group_num = YKC_MONITOR_MODULE_GROUP_MAX;
    }
    memset(message, 0x00, sizeof(Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t));
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    message->body.timestamp = s_ykc_monitor_setvoltcurr.timestamp;
    message->body.group_num = group_num;

    for(uint8_t group = 0x00; group < message->body.group_num; group++){
        for(uint8_t count = 0x00; count < NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX; count++){
            pair[count] = s_ykc_monitor_setvoltcurr.pair[group][count];
        }
        pair += NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX;
    }

    memset(s_ykc_monitor_setvoltcurr.pair, 0x00, sizeof(s_ykc_monitor_setvoltcurr.pair));
    s_ykc_monitor_setvoltcurr.count = 0x00;

    if(olen){
        *olen = (sizeof(Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t) + message->body.group_num *sizeof(struct voltcurr_pair) *NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX);
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_padding_starting_info
 * 功能         按枪填写启动中信息数据
 * 参数         gunno    枪号
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_padding_starting_info(uint8_t gunno)
{
    if(s_ykc_monitor_starting_info[gunno].count >= NET_YKC_MONITOR_STARTING_INFO_MAX){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    extern uint8_t mw_get_charge_library_state(uint8_t gunno);
    extern uint32_t thaisen_get_module_volt(uint8_t gunNum);
    extern uint16_t thaisen_get_Insult_ResPos(uint8_t gunNum);
    extern uint16_t thaisen_get_Insult_ResCat(uint8_t gunNum);

    int32_t value0 = 0x00, value1 = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    if(s_ykc_monitor_starting_info[gunno].count == 0x00){
        s_ykc_monitor_starting_info[gunno].timestamp = base->current_time;
    }

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].state = mw_get_charge_library_state(gunno);
    if(bms->BHM.rev_info){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message |= YKC_MONITOR_STARTING_BMS_MESSAGE_BHM;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message &= (~YKC_MONITOR_STARTING_BMS_MESSAGE_BHM);
    }

    if(bms->BRM.rev_info){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message |= YKC_MONITOR_STARTING_BMS_MESSAGE_BRM;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message &= (~YKC_MONITOR_STARTING_BMS_MESSAGE_BRM);
    }

    if(bms->BCP.rev_info){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message |= YKC_MONITOR_STARTING_BMS_MESSAGE_BCP;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message &= (~YKC_MONITOR_STARTING_BMS_MESSAGE_BCP);
    }

    if(bms->BRO.rev_info){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message |= YKC_MONITOR_STARTING_BMS_MESSAGE_BRO;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message &= (~YKC_MONITOR_STARTING_BMS_MESSAGE_BRO);
    }

    if(bms->BRO.BMSReady == 0xAA){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message |= YKC_MONITOR_STARTING_BMS_IS_READY;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].bms_message &= (~YKC_MONITOR_STARTING_BMS_IS_READY);
    }

    /** 暂时按双枪做 */
    if(gunno == 0x00){
        extern int16_t TH_get_A_Insult_Volt(void);
        value0 = TH_get_A_Insult_Volt();
    }else{
        extern int16_t TH_get_B_Insult_Volt(void);
        value0 = TH_get_B_Insult_Volt();
    }
    if(value0 < 0x00){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sampling_voltage.symbol = 0x01;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sampling_voltage.data = 0x00 - value0;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sampling_voltage.symbol = 0x00;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sampling_voltage.data = value0;
    }

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].max_alllow_voltage.symbol = 0x00;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].max_alllow_voltage.data = bms->BHM.MaxAllowVol;

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].battery_voltage.symbol = 0x00;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].battery_voltage.data = bms->BCP.BatVolt;

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].module_voltage.symbol = 0x00;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].module_voltage.data = thaisen_get_module_volt(gunno);

    /** 暂时按双枪做 */
    if(gunno == 0x00){
        extern int16_t TH_get_A_Insult_Positive_PE_Volt(void);
        extern int16_t TH_get_A_Insult_Cathode_PE_Volt(void);
        value0 = TH_get_A_Insult_Positive_PE_Volt();
        value1 = TH_get_A_Insult_Cathode_PE_Volt();
    }else{
        extern int16_t TH_get_B_Insult_Positive_PE_Volt(void);
        extern int16_t TH_get_B_Insult_Cathode_PE_Volt(void);
        value0 = TH_get_B_Insult_Positive_PE_Volt();
        value1 = TH_get_B_Insult_Cathode_PE_Volt();
    }
    if(value0 < 0x00){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_volt.symbol = 0x01;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_volt.data = 0x00 - value0;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_volt.symbol = 0x00;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_volt.data = value0;
    }
    if(value1 < 0x00){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_volt.symbol = 0x01;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_volt.data = 0x00 - value1;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_volt.symbol = 0x00;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_volt.data = value1;
    }

    value0 = thaisen_get_Insult_ResPos(gunno);
    value1 = thaisen_get_Insult_ResCat(gunno);
    if(value0 < 0x00){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_resistance.symbol = 0x01;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_resistance.data = 0x00 - value0;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_resistance.symbol = 0x00;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_resistance.data = value0;
    }
    if(value1 < 0x00){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_resistance.symbol = 0x01;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_resistance.data = 0x00 - value1;
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_resistance.symbol = 0x00;
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_resistance.data = value1;
    }

    s_ykc_monitor_starting_info[gunno].count++;

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_starting_info
 * 功能         组包：填充启动中信息
 * 参数         gunno   枪号
 *       buf      缓存
 *       ilen    输入缓存长度
 *       olen    填写数据总长度
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_message_padding_starting_info(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = (sizeof(Net_YkcMonitorPro_Preq_ProcessInfo_t) + sizeof(s_ykc_monitor_starting_info[gunno].info));

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_Preq_ProcessInfo_t *message = (Net_YkcMonitorPro_Preq_ProcessInfo_t*)buf;
    struct starting_info *info = (struct starting_info*)(&message->body.group_num + 0x01);

    memset(message, 0x00, data_len);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    message->body.timestamp = s_ykc_monitor_starting_info[gunno].timestamp;
    message->body.info_type = NET_YKC_MONITOR_PROCESS_INFO_TYPE_STARTING;
    message->body.gunno = (gunno + 0x01);
    if(s_ykc_monitor_starting_info[gunno].count > NET_YKC_MONITOR_STARTING_INFO_MAX){
        message->body.group_num = NET_YKC_MONITOR_STARTING_INFO_MAX;
    }else{
        message->body.group_num = s_ykc_monitor_starting_info[gunno].count;
    }

    for(uint8_t count = 0x00; count < message->body.group_num; count++){
        memcpy(&info[count], &s_ykc_monitor_starting_info[gunno].info[count], sizeof(struct starting_info));
    }

    memset(s_ykc_monitor_starting_info[gunno].info, 0x00, sizeof(s_ykc_monitor_starting_info[gunno].info));
    s_ykc_monitor_starting_info[gunno].count = 0x00;

    if(olen){
        *olen = (sizeof(Net_YkcMonitorPro_Preq_ProcessInfo_t) + message->body.group_num *sizeof(struct starting_info));
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_padding_charging_info
 * 功能         按枪填写充电中信息数据
 * 参数         gunno    枪号
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_padding_charging_info(uint8_t gunno)
{
    if(s_ykc_monitor_charging_info[gunno].count >= NET_YKC_MONITOR_CHARGING_INFO_MAX){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    extern uint8_t mw_get_charge_library_state(uint8_t gunno);
    extern uint32_t thaisen_get_module_volt(uint8_t gunNum);
    extern uint32_t thaisen_get_module_curr(uint8_t gunNum);

    int32_t value = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    if(s_ykc_monitor_charging_info[gunno].count == 0x00){
        s_ykc_monitor_charging_info[gunno].timestamp = base->current_time;
    }

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].state = mw_get_charge_library_state(gunno);
    if(bms->BCL.rev_info){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BCL;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BCL);
    }

    if(bms->BCS.rev_info){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BCS;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BCS);
    }

    if(bms->BSM.rev_info){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BSM;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BSM);
    }

    if(0/*bms->BMV.rev_info*/){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BMV;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BMV);
    }

    if(0/*bms->BMT.rev_info*/){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BMT;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BMT);
    }

    if(0/*bms->BSP.rev_info*/){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BSP;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BSP);
    }

    if(bms->BEM.rev_info){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_MESSAGE_BEM;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_MESSAGE_BEM);
    }

    if(bms->BSM.AllowChg){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message |= YKC_MONITOR_CHARGING_BMS_IS_ALLOW;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_message &= (~YKC_MONITOR_CHARGING_BMS_IS_ALLOW);
    }

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].require_voltage.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].require_voltage.data = bms->BCL.BMSneedVolt;

    value = bms->BCL.BMSneedCurlt;
    if(value < 0x00){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].require_current.symbol = 0x01;
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].require_current.data = 0x00 - value;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].require_current.symbol = 0x00;
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].require_current.data = value;
    }

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_voltage.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_voltage.data = thaisen_get_module_volt(gunno);

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_current.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_current.data = thaisen_get_module_curr(gunno);

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_measure_voltage.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_measure_voltage.data = bms->BCS.ChargVolt;

    value = bms->BCS.ChargCurlt;
    if(value < 0x00){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_measure_current.symbol = 0x01;
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_measure_current.data = 0x00 - value;
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_measure_current.symbol = 0x00;
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].bms_measure_current.data = value;
    }

//    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_voltage.symbol = 0x01;
//    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_voltage.data = base->voltage_a /10;

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_current.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_current.data = base->current_a /10;

    s_ykc_monitor_charging_info[gunno].count++;

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_charging_info
 * 功能         组包：填充充电中信息
 * 参数         gunno   枪号
 *       buf      缓存
 *       ilen    输入缓存长度
 *       olen    填写数据总长度
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_message_padding_charging_info(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = (sizeof(Net_YkcMonitorPro_Preq_ProcessInfo_t) + sizeof(s_ykc_monitor_charging_info[gunno].info));

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_Preq_ProcessInfo_t *message = (Net_YkcMonitorPro_Preq_ProcessInfo_t*)buf;
    struct charging_info *info = (struct charging_info*)(&message->body.group_num + 0x01);

    memset(message, 0x00, data_len);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    message->body.timestamp = s_ykc_monitor_charging_info[gunno].timestamp;
    message->body.info_type = NET_YKC_MONITOR_PROCESS_INFO_TYPE_CHARGING;
    message->body.gunno = (gunno + 0x01);
    if(s_ykc_monitor_charging_info[gunno].count > NET_YKC_MONITOR_CHARGING_INFO_MAX){
        message->body.group_num = NET_YKC_MONITOR_CHARGING_INFO_MAX;
    }else{
        message->body.group_num = s_ykc_monitor_charging_info[gunno].count;
    }

    for(uint8_t count = 0x00; count < message->body.group_num; count++){
        memcpy(&info[count], &s_ykc_monitor_charging_info[gunno].info[count], sizeof(struct charging_info));
    }

    memset(s_ykc_monitor_charging_info[gunno].info, 0x00, sizeof(s_ykc_monitor_charging_info[gunno].info));
    s_ykc_monitor_charging_info[gunno].count = 0x00;

    if(olen){
        *olen = (sizeof(Net_YkcMonitorPro_Preq_ProcessInfo_t) + message->body.group_num *sizeof(struct charging_info));
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_charge_finish_info
 * 功能         组包：填充充电结束信息
 * 参数         gunno   枪号
 *       buf      缓存
 *       ilen    输入缓存长度
 *       olen    填写数据总长度
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_message_padding_charge_finish_info(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = (sizeof(Net_YkcMonitorPro_Preq_ProcessInfo_t) + sizeof(struct finish_info));

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    extern uint8_t mw_get_charge_library_state(uint8_t gunno);

    Net_YkcMonitorPro_Preq_ProcessInfo_t *message = (Net_YkcMonitorPro_Preq_ProcessInfo_t*)buf;
    struct finish_info *info = (struct finish_info*)(&message->body.group_num + 0x01);
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    memset(message, 0x00, data_len);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    message->body.timestamp = base->current_time;
    message->body.info_type = NET_YKC_MONITOR_PROCESS_INFO_TYPE_FINISH;
    message->body.gunno = (gunno + 0x01);
    message->body.group_num = NET_YKC_MONITOR_FINISH_INFO_MAX;

    info->state = mw_get_charge_library_state(gunno);

    if(bms->BST.rev_info){
        info->bms_message |= YKC_MONITOR_FINISH_BMS_MESSAGE_BST;
    }else{
        info->bms_message &= (~YKC_MONITOR_FINISH_BMS_MESSAGE_BST);
    }

    if(bms->BSD.rev_info){
        info->bms_message |= YKC_MONITOR_FINISH_BMS_MESSAGE_BSD;
    }else{
        info->bms_message &= (~YKC_MONITOR_FINISH_BMS_MESSAGE_BSD);
    }

    info->bsm.msingle_bat_sn = bms->BSM.HigVoltCellNum;
    info->bsm.max_bat_temp = bms->BSM.HigTemp;
    info->bsm.max_bat_temp_sn = bms->BSM.HigTempNum;
    info->bsm.min_bat_temp = bms->BSM.LowTemp;
    info->bsm.min_bat_temp_sn = bms->BSM.LowTempNum;

    info->bsm.state.bms_single_volt = bms->BSM.CellOverVolt;
    info->bsm.state.bms_bat_soc = bms->BSM.SOCState;
    info->bsm.state.bms_bat_curr = bms->BSM.BatOverCurlt;
    info->bsm.state.bms_bat_temp = bms->BSM.BatOverTemp;
    info->bsm.state.bms_bat_isolate = bms->BSM.Insulat;
    info->bsm.state.bms_bat_output_linker = bms->BSM.OutConect;
    info->bsm.state.charge_forbid = bms->BSM.AllowChg;
    info->bsm.state.reserve = 0x00;

    if(olen){
        *olen = (sizeof(Net_YkcMonitorPro_Preq_ProcessInfo_t) + message->body.group_num *sizeof(struct finish_info));
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_function_switch
 * 功能          组包：功能开关控制响应
 * **********************************************/
int8_t ykc_monitor_response_padding_function_switch(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = sizeof(Net_YkcMonitorPro_Pres_FunctionSwitch_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < data_len){
        return -0x02;
    }

    Net_YkcMonitorPro_Pres_FunctionSwitch_t *fswitch = (Net_YkcMonitorPro_Pres_FunctionSwitch_t*)buf;

    memset(fswitch, 0x00, sizeof(Net_YkcMonitorPro_Pres_FunctionSwitch_t));
    fswitch->body.result = 0x01;

    if(olen){
        *olen = data_len;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_info_para_confirm
 * 功能          组包：确认修改的桩信息、参数
 * **********************************************/
int8_t ykc_monitor_response_padding_info_para_confirm(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = sizeof(Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < data_len){
        return -0x02;
    }

    Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t *info_para = (Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t*)buf;

    memset(info_para, 0x00, sizeof(Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t));
    memcpy(info_para, &g_ykc_monitor_sreq_pres_info_para_modify_confirm, sizeof(Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t));

    if(olen){
        *olen = data_len;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_response_padding_modify_dev_info
 * 功能          组包：修改设备信息响应
 * **********************************************/
int8_t ykc_monitor_response_padding_modify_dev_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = sizeof(Net_YkcMonitorPro_Sres_InfoPara_ConfirmResult_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < data_len){
        return -0x02;
    }

    if(olen){
        *olen = data_len;
    }

    return 0x00;
}





/** 监控报文处理 */

/*************************************************
 * 函数名      ykc_monitor_message_pro_function_switch
 * 功能          处理服务器下发的功能开关控制请求
 * **********************************************/
int8_t ykc_monitor_message_pro_function_switch(void *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if(len < sizeof(Net_YkcMonitorPro_Sreq_FunctionSwitch_t)){
        return -0x02;
    }

    Net_YkcMonitorPro_Sreq_FunctionSwitch_t *fswitch = (Net_YkcMonitorPro_Sreq_FunctionSwitch_t*)data;
    ykc_monitor_storage_struct *config = (ykc_monitor_storage_struct*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT));
    if(config == NULL){
        return -0x03;
    }

    config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG;
    config->fswitch.tplat_log = fswitch->body.tplat_log;

    if(s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT) < 0x00){
        config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG - 0x01;
        return -0x04;
    }

    ykc_monitor_function_switch_set(config);

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_info_para_modify
 * 功能          执行桩信息、参数修改
 * **********************************************/
int8_t ykc_monitor_message_pro_info_para_modify(void *data, uint16_t len)
{
    uint16_t data_len = sizeof(Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t);

    if(data == NULL){
        return -0x01;
    }
    if(len < data_len){
        return -0x02;
    }

    uint8_t item_len = 0x00;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t *info_para = (Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t*)data;

    item_len = strlen((char*)info_para->body.new_pile_number);
    if(item_len){
        LOG_D("ykc monitor modify pile number:%s", info_para->body.new_pile_number);
        s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, (uint8_t*)info_para->body.new_pile_number, item_len, option);
    }

    item_len = strlen((char*)info_para->body.domain);
    if(item_len){
        LOG_D("ykc monitor modify domain:%s", info_para->body.domain);
        s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, (uint8_t*)info_para->body.domain, item_len, option);
    }

    if(info_para->body.port){
        LOG_D("ykc monitor modify port:%s", info_para->body.port);
        s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_PORT, (uint8_t*)&info_para->body.port, sizeof(info_para->body.port), option);
    }

    return s_ykc_monitor_handle->system_data_storage(0x00);
}

/*************************************************
 * 函数名      ykc_monitor_message_pro_modify_dev_info
 * 功能          执行设备信息修改
 * **********************************************/
int8_t ykc_monitor_message_pro_modify_dev_info(uint8_t info_type, void *data, uint16_t len)
{
    if(data == NULL){
        return -0x01;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);

    /** 屏幕密码 */
    if(info_type == 0x00){
        uint8_t count = 0x00;
        for(count = 0; count < len; count++){
            if((*((uint8_t*)data + count) < 0x20) || (*((uint8_t*)data + count) > 0x7E)){        /** 不能是控制字符 */
                break;
            }
        }
        if(count == len){
            if(s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_SCREEN_PW, (uint8_t*)data, len, option) < 0x00){
                LOG_W("ykc_monitor_message_pro_modify_dev_info modify screen password fail(storage sync)");
                return -0x01;
            }
        }else{
            LOG_W("ykc_monitor_message_pro_modify_dev_info modify screen password fail(ctrl char)");
            return -0x01;
        }
    }else{
        LOG_W("ykc_monitor_message_pro_modify_dev_info not this info type(%d)", info_type);
        return -0x01;
    }

    return s_ykc_monitor_handle->system_data_storage(0x00);
}


#endif /* NET_YKC_MONITOR_AS_MONITOR */

#endif /* NET_PACK_USING_YKC_MONITOR */
