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
#include "thaisenChargLib.h"
#include "thaisen7102Public.h"
#include "app_data_info_interface.h"

#define DBG_TAG "ykc_mrl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YKC_MONITOR

#ifdef NET_YKC_MONITOR_AS_MONITOR

#define YKC_MONITOR_FIXED_CMD_MSG_VER                     0x00                  /* 固定类型指令报文版本 */

#define YKC_MONITOR_DYNAMIC_CMD_MSG_VER                   0x00                  /* 动态类型指令报文版本 */
#define YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM                0x06                  /* 单次操作动态类型指令最大个数，超过的不执行，也不报错 */

/** 接收BMS报文 */
#define YKC_MONITOR_RECVED_MSG_BHM                       (0x01 <<0x00)          /* 是否接收到了BHM报文 */
#define YKC_MONITOR_RECVED_MSG_BRM                       (0x01 <<0x01)          /* 是否接收到了BRM报文 */
#define YKC_MONITOR_RECVED_MSG_BFC                       (0x01 <<0x02)          /* 是否接收到了BFC报文 */
#define YKC_MONITOR_RECVED_MSG_BCP                       (0x01 <<0x03)          /* 是否接收到了BCP报文 */
#define YKC_MONITOR_RECVED_MSG_BRO_00                    (0x01 <<0x04)          /* 是否接收到了BRO_00报文 */
#define YKC_MONITOR_RECVED_MSG_BRO_AA                    (0x01 <<0x05)          /* 是否接收到了BRO_AA报文 */
#define YKC_MONITOR_RECVED_MSG_BCL                       (0x01 <<0x06)          /* 是否接收到了BCL报文 */
#define YKC_MONITOR_RECVED_MSG_BCS                       (0x01 <<0x07)          /* 是否接收到了BCS报文 */
#define YKC_MONITOR_RECVED_MSG_BEM                       (0x01 <<0x08)          /* 是否接收到了BEM报文 */
#define YKC_MONITOR_RECVED_MSG_BSM                       (0x01 <<0x09)          /* 是否接收到了BSM报文 */
#define YKC_MONITOR_RECVED_MSG_BST                       (0x01 <<0x0A)          /* 是否接收到了BST报文 */
#define YKC_MONITOR_RECVED_MSG_BSD                       (0x01 <<0x0B)          /* 是否接收到了BSD报文 */
/** 发送充电机报文报文 */
#define YKC_MONITOR_SENDED_MSG_CHM                       (0x01 <<0x00)          /* 是否发送了CHM报文 */
#define YKC_MONITOR_SENDED_MSG_CRM                       (0x01 <<0x01)          /* 是否发送了CRM报文 */
#define YKC_MONITOR_SENDED_MSG_CRM_AA                    (0x01 <<0x02)          /* 是否发送了CRM_AA报文 */
#define YKC_MONITOR_SENDED_MSG_CFC                       (0x01 <<0x03)          /* 是否发送了CFC报文 */
#define YKC_MONITOR_SENDED_MSG_CTS                       (0x01 <<0x04)          /* 是否发送了CTS报文 */
#define YKC_MONITOR_SENDED_MSG_CML                       (0x01 <<0x05)          /* 是否发送了CML报文 */
#define YKC_MONITOR_SENDED_MSG_CRO                       (0x01 <<0x06)          /* 是否发送了CRO报文 */
#define YKC_MONITOR_SENDED_MSG_CRO_AA                    (0x01 <<0x07)          /* 是否发送了CRO_AA报文 */
#define YKC_MONITOR_SENDED_MSG_CCS                       (0x01 <<0x08)          /* 是否发送了CCS报文 */
#define YKC_MONITOR_SENDED_MSG_CEM                       (0x01 <<0x09)          /* 是否发送了CEM报文 */
#define YKC_MONITOR_SENDED_MSG_CST                       (0x01 <<0x0A)          /* 是否发送了CST报文 */
#define YKC_MONITOR_SENDED_MSG_CSD                       (0x01 <<0x0B)          /* 是否发送了CSD报文 */

#define YKC_MONITOR_BUF_PUBLIC_LENGTH                     0xFF                  /* 充电数据公用缓存长度  */
#define YKC_MONITOR_MODULE_GROUP_MAX                      0x04                  /* 最大模块组数  */

#define YKC_MONITOR_MFAULT_CHECK_PERIOD                   500                   /* 模块故障检测周期(ms)  */
#define YKC_MONITOR_MFAULT_REPEAT_FAST_PERIOD             5000                  /* 模块故障快速上报周期(ms，用于上报信息无响应时)  */
#define YKC_MONITOR_MFAULT_REPEAT_NORMAL_PERIOD           (5 *60 *1000)         /* 模块故障正常上报周期(ms， 用于有故障时定时上报)  */

#endif /* NET_YKC_MONITOR_AS_MONITOR */

#define YKC_MONITOR_CHARGE_ELECT_MAX                      1000000               /* 最大充电电量值(精度：0.001) */
#define YKC_MONITOR_SPEND_AMOUNT_MAX                      30000000              /* 最大消费金额值(精度：0.0001) */

#define YKC_MONITOR_REALTIME_DATA_INTERVAL_INIT           0x05                  /* 刚连上网时实时数据上报间隔 */
#define YKC_MONITOR_REALTIME_DATA_INTERVAL_CHARGING       0x0F                  /* 充电中实时数据上报间隔  */
#define YKC_MONITOR_REALTIME_DATA_INTERVAL_IDLE           0x05 *60              /* 空闲实时数据上报间隔  */

#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
#define YKC_MONITOR_DEVICE_FAULT_LOCK_DEVICE              (0x01 <<25)           /* 实时故障：锁桩(这个要和 ykc_monitor_fault_analyse.c  YKC_MONITOR_REALTIME_FAULT_LOCK_DEVICE 一样) */
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */

#define YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE    1536                  /* 实时处理线程栈大小 */

#pragma pack(1)

struct ykc_monitor_state_info{
    struct{
        uint8_t state : 4;
        uint8_t connect : 2;
        uint8_t reserve : 2;
    }state;                                       /* 桩状态 */
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
    uint32_t fault_code[NET_YKC_MONITOR_FAULT_SET_NUM]; /* 故障码 */
#else
    uint16_t fault_code;                          /* 故障码 */
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
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

/** 模块故障信息 */
typedef struct{
    uint32_t check_tick;                          /* 故障检测时基 */
    uint32_t report_tick;                         /* 故障上报时基 */
    uint8_t faddr;                                /* 检测到有故障的第一个模块的地址(只要一个模块有故障就上报) */
    struct{
        uint8_t is_report : 1;                    /* 已执行上报 */
        uint8_t is_resume : 1;                    /* 1:是故障恢复  0：是故障发生 */
        uint8_t is_waiting_response : 1;          /* 1:正在等待响应  0：已响应 */
    }flag;
}ykc_monitor_mfault_info;

/** 模块故障信息 */
typedef struct{
    uint32_t base_tick;                          /* 定时时基 */
    struct{
        uint8_t is_wait_response : 1;            /* 正在等待响应 */
        uint8_t operate_result : 1;              /* 操作结果(1：成功，0：失败) */
    }flag;
}ykc_monitor_lock_module;

/** 导引状态变化信息 */
typedef struct{
    uint8_t count;                               /* 有效数量 */
    uint8_t is_sending;                          /* 数据正在发送 */
    struct{
        uint32_t timestamp;                      /* 变化时的时间(时间戳) */
        int voltage;                             /* 导引当前电压值(0.001V) */
        int voltage_last;                        /* 导引前一次电压值(0.001V) */
        uint16_t diff_positive_adc;              /* 当前差分正ADC */
        uint16_t diff_negtive_adc;               /* 当前差分负ADC */
        uint16_t diff_positive_adc_last;         /* 前一次差分正ADC */
        uint16_t diff_negtive_adc_last;          /* 前一次差分负ADC */
        uint8_t flag;                            /* 标志 */
    }data[NET_YKC_MONITOR_GUIDANCE_CHANGED_INFO_MAX];
}ykc_monitor_guidance_changed;

/** 器件控制状态变化信息 */
typedef struct{
    uint8_t delay_count;                         /* 延时上报(预防有多个器件接连发生变化：提高报文中有效数据的占比) */
    uint8_t count;                               /* 有效数量 */
    uint8_t is_sending;                          /* 数据正在发送 */
    struct control_segment segment[NET_YKC_MONITOR_DEVICE_CTRL_CHANGED_INFO_MAX]; /* 数据段 */
}ykc_monitor_device_control_changed;

/** 器件控制状态变化信息 */
typedef struct{
    uint8_t delay_count;                         /* 延时上报(预防有多个器件接连发生变化：提高报文中有效数据的占比) */
    uint8_t is_recved;                           /* 已接收到变化 */
}ykc_monitor_device_status_changed;

/** 断网原因 */
typedef struct{
    uint8_t count;
    struct socket_dis_group group;
}ykcm_socket_disconnect_t;

typedef struct{
    uint8_t count;
    struct module_dis_group group;
}ykcm_module_disconnect_t;

/** 液冷故障 */
struct liquid_group{
    struct{
        uint8_t sequence : 3;                     /* 液冷序号 */
        uint8_t type : 3;                         /* 液冷类型 */
        uint8_t is_offline : 1;                   /* 已离线 */
        uint8_t reserve : 1;                      /* 预留 */
    }info;
    uint32_t f_value;                             /* 液冷故障值 */
    uint32_t f_value_last;                        /* 液冷故障值(前一次) */
};

typedef struct{
    struct liquid_group group[NET_YKC_MONITOR_LIQUID_F_INFO_MAX];
    struct{
        uint8_t is_lock : 1;                      /* 已上锁 */
        uint8_t wait_response : 1;                /* 等待服务器响应 */
        uint8_t reserve : 6;                      /* 预留 */
    }info;
    uint8_t wait_unlock_time;                     /* 等待解锁时间 */
    uint8_t wait_response_time;                   /* 等待服务器响应时间 */
}ykcm_liquid_f_info_t;

#endif /* NET_YKC_MONITOR_AS_MONITOR */

#pragma pack()

#ifdef NET_YKC_MONITOR_AS_MONITOR
NET_DEF_SRAM2 static ykc_monitor_setvoltcurr s_ykc_monitor_setvoltcurr;
NET_DEF_SRAM2 static ykc_monitor_starting_info s_ykc_monitor_starting_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static ykc_monitor_charging_info s_ykc_monitor_charging_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static ykc_monitor_mfault_info s_ykc_monitor_mfault_info;
NET_DEF_SRAM2 static ykc_monitor_lock_module s_ykc_monitor_lock_module;
#endif /* NET_YKC_MONITOR_AS_MONITOR */

NET_DEF_SRAM2 static struct ykc_monitor_flag_info s_ykc_monitor_flag_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_ykc_monitor_realtime_data_interval[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_ykc_monitor_realtime_data_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_ykc_monitor_local_start_sq;
NET_DEF_SRAM2 static struct ykc_monitor_state_info s_ykc_monitor_state_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct rt_thread s_ykc_monitor_realtime_process_thread;
NET_DEF_SRAM0 static uint8_t s_ykc_monitor_realtime_process_thread_stack[YKC_MONITOR_REALTIME_PROCESS_THREAD_STACK_SIZE];
NET_DEF_SRAM2 static struct net_handle* s_ykc_monitor_handle = NULL;
NET_DEF_SRAM2 static ykc_monitor_guidance_changed s_ykc_monitor_guidance_changed[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static ykc_monitor_device_control_changed s_ykc_monitor_device_control_changed[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static ykc_monitor_device_status_changed s_ykc_monitor_device_status_changed[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static ykcm_socket_disconnect_t s_ykc_monitor_close_passive;                     /** socket 被动关闭 */
NET_DEF_SRAM2 static ykcm_socket_disconnect_t s_ykc_monitor_close_active;                      /** socket 主动关闭 */
NET_DEF_SRAM2 static ykcm_socket_disconnect_t s_ykc_monitor_heartbeat_timeout;                 /** socket 心跳超时 */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_socket_pdp;                        /** socket PDP场景失效 */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_close_module;                      /** 通信模块关闭 */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_at_physice;                        /** AT指令(以太网物理层：查是否在线、查版本、修改波特率) */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_cpin_lk_mac;                       /** 查找SIM卡(以太网数据链路层：初始化芯片、寻线、开DHCP) */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_cimi_lk_mac;                       /** 查找CIMI号(以太网数据链路层：初始化芯片、寻线、开DHCP) */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_signal_strength_lk_mac;            /** 查询信号强度(以太网数据链路层：初始化芯片、寻线、开DHCP) */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_gsm_registered;                    /** 注册GSM网络(以太网网络层：判断DHCP是否启动、获取IP信息、查询MAC地址) */
NET_DEF_SRAM2 static ykcm_module_disconnect_t s_ykc_monitor_gprs_registered;                   /** 注册GPRS网络(以太网网络层：判断DHCP是否启动、获取IP信息、查询MAC地址) */

NET_DEF_SRAM2 static ykcm_liquid_f_info_t s_ykcm_liquid_f_info;                                /** 液冷故障信息 */

static uint16_t ykc_monitor_chargepile_stop_reason_converted(void *handle, uint16_t bit, uint8_t stop_in_starting);
static uint8_t ykc_monitor_chargepile_transaction_identity_converted(uint8_t identity);
static void ykc_monitor_module_fault_check(void);
static void ykc_monitor_liquid_fault_check(void);

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
            if(config->fswitch.lock == NET_ENUM_FALSE){
                base->device_state = APP_DEVICE_STATE_FREEZE;
            }else {
                base->device_state = APP_DEVICE_STATE_COMMISSIONING;
            }
        }
    }else{

    }

    if(config->flag.is_thread_error == NET_ENUM_FALSE){
        memset(config->reset_lable, 0x00, sizeof(config->reset_lable));
    }
    base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(0x00));

    if(config->reset_count == 0xFFFFFFFF){
        config->reset_count = 0x00;
    }
    config->reset_count++;
    config->reset_reason = base->reset_reason;
    config->flag.is_thread_error = NET_ENUM_FALSE;
//    /** 重新保存一次 */
//    s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT);

    LOG_D("ykc monitor tplat_log lock:%d, pile lock:%d", !config->fswitch.tplat_log, !config->fswitch.lock);
    LOG_D("ykc monitor reset count:%d, reset reason:%X", config->reset_count, config->reset_reason);
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

#ifdef NET_YKC_MONITOR_AS_TARGET
    uint8_t is_updated = NET_ENUM_FALSE;
#endif /* NET_YKC_MONITOR_AS_TARGET */
    System_BaseData *base = NULL;
    Net_YkcMonitorPro_SRes_BillingModel_Request_t *request = (Net_YkcMonitorPro_SRes_BillingModel_Request_t*)data;

#ifdef NET_YKC_MONITOR_AS_TARGET
    net_operation_set_fees_gunno(0xFF, 0x00);    /** 先清除信息 */
#endif /* NET_YKC_MONITOR_AS_TARGET */
    for(uint8_t _gunno = 0x00; _gunno < NET_SYSTEM_GUN_NUMBER; _gunno++){
        uint8_t gunno = _gunno;
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
        if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING) ||
                (base->state.current == APP_OFSM_STATE_STOPING)){
            net_operation_set_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
            gunno = NET_SYSTEM_GUN_NUMBER;
        }
#ifdef NET_YKC_MONITOR_AS_TARGET
        else{
            is_updated = NET_ENUM_TRUE;
            net_operation_set_fees_gunno(_gunno, 0x01);
        }
#endif /* NET_YKC_MONITOR_AS_TARGET */

        rt_kprintf("gunno(%d) billingrule info\n", gunno);
        rt_kprintf("ter(%d) tsr(%d) per(%d) psr(%d) fer(%d) fsr(%d) ver(%d) vsr(%d)\n", request->body.tip_elect_rate,
                request->body.tip_service_rate, request->body.peak_elect_rate, request->body.peak_service_rate,
                request->body.flat_elect_rate, request->body.flat_service_rate, request->body.valley_elect_rate,
                request->body.valley_service_rate);

        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_SHARP, (request->body.tip_elect_rate + request->body.tip_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_PEAK, (request->body.peak_elect_rate + request->body.peak_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_FLAT, (request->body.flat_elect_rate + request->body.flat_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_VALLEY, (request->body.valley_elect_rate + request->body.valley_service_rate) /10);
        app_billingrule_set_elect_model_sn(gunno, (uint8_t*)&request->body.model_number, sizeof(request->body.model_number));
        app_billingrule_set_service_model_sn(gunno, (uint8_t*)&request->body.model_number, sizeof(request->body.model_number));

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
#ifdef NET_YKC_MONITOR_AS_TARGET
    if(is_updated){
        net_operation_updated_billing_trigger();
    }
#endif /* NET_YKC_MONITOR_AS_TARGET */

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
    ykc_monitor_bcd_to_ascii(base->card_number, sizeof(base->card_number), request->body.logic_card_number, NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX);

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
    case APP_OFSM_STATE_RESERVATION:
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
            config->fswitch.lock = NET_ENUM_FALSE;
        }
    }else{
        /** 此处锁桩只是填充信息，具体是否保存成功有设置功率百分比异步响应决定 */
        if(config){
            config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG;
            config->fswitch.lock = NET_ENUM_TRUE;
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
    net_operation_set_event(0x00, NET_OPERATION_EVENT_SYNC_MONITOR);

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
    sprintf((char *)&g_ykc_monitor_preq_login.body.software_ver[3 + 1], "%c", (base->soft_ver_revise + 'A'));

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
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        memset(g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set, 0x00, sizeof(g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set));
#else
        g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault = 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */

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

        g_ykc_monitor_preq_report_realtime_data[gunno].body.platform_id = 0x00;
        /** 这是并充 */
        if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
            if(base->main_gunno != gunno){
                g_ykc_monitor_preq_report_realtime_data[gunno].body.belong_main_gun = (base->main_gunno + 0x01);
                g_ykc_monitor_preq_report_realtime_data[gunno].body.is_parallel_deputy = NET_ENUM_TRUE;
            }else{
                g_ykc_monitor_preq_report_realtime_data[gunno].body.belong_main_gun = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.is_parallel_deputy = NET_ENUM_FALSE;
            }
        }else{
            g_ykc_monitor_preq_report_realtime_data[gunno].body.belong_main_gun = 0x00;
            g_ykc_monitor_preq_report_realtime_data[gunno].body.is_parallel_deputy = NET_ENUM_FALSE;
        }
        g_ykc_monitor_preq_report_realtime_data[gunno].body.is_v2g = 0x00;
        if(base->gun_running_mode == APP_GUN_RUNNING_MODE_V2G){
            g_ykc_monitor_preq_report_realtime_data[gunno].body.is_v2g = 0x01;
        }
        g_ykc_monitor_preq_report_realtime_data[gunno].body.bms_protocol_type = 0xFF;
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING)){
                struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

                g_ykc_monitor_preq_report_realtime_data[gunno].body.platform_id = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_voltage = base->voltage_a /10;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.output_current = base->current_a /10;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_elect = base->elect_a *10;
                if(base->gunline_temperature[0] > base->gunline_temperature[1]){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = (base->gunline_temperature[0] /10 + 50);
                }else{
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.gun_temperature = (base->gunline_temperature[1] /10 + 50);
                }
                g_ykc_monitor_preq_report_realtime_data[gunno].body.is_v2g = 0x00;
                if(base->gun_running_mode == APP_GUN_RUNNING_MODE_V2G){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.is_v2g = 0x01;
                }
                /** 这是并充 */
                if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
                    if(base->main_gunno != gunno){
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.belong_main_gun = (base->main_gunno + 0x01);
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.is_parallel_deputy = NET_ENUM_TRUE;
                    }else{
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.belong_main_gun = 0x00;
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.is_parallel_deputy = NET_ENUM_FALSE;
                    }
                    base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
                }else{
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.belong_main_gun = 0x00;
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.is_parallel_deputy = NET_ENUM_FALSE;
                }

                g_ykc_monitor_preq_report_realtime_data[gunno].body.soc = base->current_soc;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.battery_group_temp_max = (bms->BSM.HigTemp + 50);
                g_ykc_monitor_preq_report_realtime_data[gunno].body.charge_time = base->charge_time /60;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.remain_time = bms->BCS.SurplChgTime;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.loss_elect = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.consume_amount = base->fees_total;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.bms_protocol_type = 0xFF;
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
                g_ykc_monitor_preq_report_realtime_data[gunno].body.is_v2g = 0x00;
                g_ykc_monitor_preq_report_realtime_data[gunno].body.bms_protocol_type = 0xFF;
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

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
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
    g_ykc_monitor_preq_shake_hand[gunno].body.reserve = bms->BRM.reserved;
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

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    g_ykc_monitor_preq_parameter_config[gunno].body.bms_single_bat_allow_volt_max = bms->BCP.CellAlowHigVolt;
    if(bms->BCP.AlowCurlt > 4000){
        g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_curr_max = 0x00;
    }else{
        g_ykc_monitor_preq_parameter_config[gunno].body.bms_allow_curr_max = (4000  - bms->BCP.AlowCurlt);
    }
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

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
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

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
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

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
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

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
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

        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_command = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_command = 4000;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = 4000;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.max_volt_and_gn.bms_max_single_bat_volt = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.max_volt_and_gn.max_single_bat_volt_gn = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_current_soc = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_remain_charge_time = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_volt = 0x00;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_curr = 4000;
        g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.charge_time = 0x00;

        valid_len = sizeof(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.serial_number, base->transaction_number, valid_len);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_volt = (base->voltage_a /10);
            if((base->current_a /10) > 4000){
                g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_curr = 0x00;
            }else{
                g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.pile_output_curr = (4000  - (base->current_a /10));
            }
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.charge_time = base->charge_time /60;

            if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
                base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_command = bms->BCL.BMSneedVolt;
            if(bms->BCL.BMSneedCurlt > 4000){
                g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_command = 0x00;
            }else{
                g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_command = (4000  - bms->BCL.BMSneedCurlt);
            }
            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = bms->BCL.ChagModel;

            g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = bms->BCS.ChargVolt;
            if(bms->BCS.ChargCurlt > 4000){
                g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = 0x00;
            }else{
                g_ykc_monitor_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = (4000  - bms->BCS.ChargCurlt);
            }
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

        g_ykc_monitor_preq_bms_info[gunno].body.bms_max_single_volt_bat_number = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.bms_bat_temp_max = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.bat_temp_max_measure_number = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.bms_bat_temp_min = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.bat_temp_min_measure_number = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.bms_single_volt = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_soc = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_curr = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_temp = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_isolate = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.bms_bat_output_linker = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.charge_forbid = 0;
        g_ykc_monitor_preq_bms_info[gunno].body.state.reserve = 0;

        valid_len = sizeof(g_ykc_monitor_preq_bms_info[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ykc_monitor_preq_bms_info[gunno].body.serial_number, 0x00, sizeof(g_ykc_monitor_preq_bms_info[gunno].body.serial_number));
        memcpy(g_ykc_monitor_preq_bms_info[gunno].body.serial_number, base->transaction_number, valid_len);
    }else{
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_BMS_INFO) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

            if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
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
 * 函数名      ykc_monitor_transaction_record_time_updata
 * 功能          更新账单交易时间（防止断定订单在未联网对时情况下时间有误）
 * **********************************************/
void ykc_monitor_transaction_record_time_updata(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    g_ykc_monitor_preq_transaction_records[gunno].body.transaction_date = ykc_monitor_get_cp56time2a_from_timestamp(base->current_time);
}

/*************************************************
 * 函数名      ykc_monitor_chargepile_request_padding_heartbeat
 * 功能          心跳数据填充
 * **********************************************/
void ykc_monitor_chargepile_request_padding_heartbeat(uint8_t gunno, void *data)
{
    if((gunno >= NET_SYSTEM_GUN_NUMBER) || (data == NULL)){
        return;
    }
    Net_YkcMonitorPro_PReq_HeartBeat_t *msg = (Net_YkcMonitorPro_PReq_HeartBeat_t*)data;
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));

    msg->body.meter_value = (base->ammeter_elect /10);
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
        g_ykc_monitor_preq_transaction_records[gunno].body.stop_reason = ykc_monitor_chargepile_stop_reason_converted(_transaction, _transaction->stop_reason, _transaction->order_info.is_start_fail);

        g_ykc_monitor_preq_transaction_records[gunno].body.is_v2g = 0x00;
        if(_transaction->gun_running_mode == APP_GUN_RUNNING_MODE_V2G){
            g_ykc_monitor_preq_transaction_records[gunno].body.is_v2g = 0x01;
        }
        g_ykc_monitor_preq_transaction_records[gunno].body.bms_protocol_type = 0xFF;

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
                if(config->fswitch.lock == NET_ENUM_FALSE){
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
    case APP_OFSM_STATE_RESERVATION:
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
            ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                    gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_BMS_MESSAGE);
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

#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
void ykc_monitor_chargepile_fault_report(uint8_t gunno, uint32_t *code)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(code == NULL){
        return;
    }

    for(uint8_t i = 0x00; i < NET_YKC_MONITOR_FAULT_SET_NUM; i++){
        s_ykc_monitor_state_info[gunno].fault_code[i] = code[i];
    }
}
#else
void ykc_monitor_chargepile_fault_report(uint8_t gunno, uint16_t code)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_ykc_monitor_state_info[gunno].fault_code = code;
}
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */

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
    case APP_SYS_FAULT_MAIN_CABINET_OFFLINE :
        return NET_GENERAL_FAULT_MAIN_CABINET_OFFLINE;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_1 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_1;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_2 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_2;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_3 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_3;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN2_1 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_1;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN2_2 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_2;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN3_1 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN3_1;
    case APP_SYSTEM_FAULT_SLAVE_DEVICE_OFFLINE :
        return NET_GENERAL_FAULT_SLAVE_DEVICE_OFFLINE;
    case APP_SYSTEM_FAULT_FAN :
        return NET_GENERAL_FAULT_FAN;
    case APP_SYSTEM_FAULT_MAINCABINET_SCRAM :
        return NET_GENERAL_FAULT_MAINCABINET_SCRAM;
    case APP_SYSTEM_FAULT_MAINCABINET_GATE :
        return NET_GENERAL_FAULT_MAINCABINET_GATE;
    case APP_SYSTEM_FAULT_MAINCABINET_PDUFAULT :
        return NET_GENERAL_FAULT_MAINCABINET_PDUFAULT;
    case APP_SYSTEM_FAULT_MAINCABINET_MODULEFAULT :
        return NET_GENERAL_FAULT_MAINCABINET_MODULEFAULT;
    case APP_SYSTEM_FAULT_MAINCABINET_CONFIG :
        return NET_GENERAL_FAULT_MAINCABINET_CONFIG;
    case APP_SYSTEM_FAULT_MAINCABINET_ACRELAY :
        return NET_GENERAL_FAULT_MAINCABINET_ACRELAY;
    case APP_SYSTEM_FAULT_MAINCABINET_SMOKE :
        return NET_GENERAL_FAULT_MAINCABINET_SMOKE;
    case APP_SYSTEM_FAULT_MAINCABINET_POUR :
        return NET_GENERAL_FAULT_MAINCABINET_POUR;
    case APP_SYSTEM_FAULT_MAINCABINET_FLOODING :
        return NET_GENERAL_FAULT_MAINCABINET_FLOODING;
    case APP_SYSTEM_FAULT_MAINCABINET_OTHER :
        return NET_GENERAL_FAULT_MAINCABINET_OTHER;
    case APP_SYSTEM_FAULT_MAINCABINET_LIGHT_PROTECT :
        return NET_GENERAL_FAULT_MAINCABINET_LIGHT_PROTECT;
    case APP_SYSTEM_FAULT_DEVICE_IS_LOCKED :
        return NET_GENERAL_FAULT_DEVICE_IS_LOCKED;
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
static uint16_t ykc_monitor_chargepile_stop_reason_converted(void *handle, uint16_t reason, uint8_t stop_in_starting)
{
    uint16_t _reason = NETYKC_MONITOR_AS_REASON90_UNKNOW;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)handle;

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
    /* 过压 */
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
        _reason = NETYKC_MONITOR_AS_REASONCC_PILE_OVERVOLT;
        break;
    /* 欠压 */
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        _reason = NETYKC_MONITOR_AS_REASONCD_PILE_UNDERVOLT;
        break;
    /* 过流 */
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        _reason = NETYKC_MONITOR_AS_REASONCE_PILE_OVERCURR;
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
    /* 接收BRM超时 */
    case APP_SYSTEM_STOP_WAY_BRM_TIMEOUT:
        _reason = NETYKC_MONITOR_SF_REASON5A_RECV_BRM_TIMEOUT;
        break;
    /* 接收BCP超时 */
    case APP_SYSTEM_STOP_WAY_BCP_TIMEOUT:
        _reason = NETYKC_MONITOR_SF_REASON5B_RECV_BCP_TIMEOUT;
        break;
    /* 接收BRO超时 */
    case APP_SYSTEM_STOP_WAY_BRO_TIMEOUT:
        _reason = NETYKC_MONITOR_AS_REASONCF_WAIT_BRO;
        break;
    /* 接收BRO_AA超时 */
    case APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT:
        _reason = NETYKC_MONITOR_SF_REASON5C_RECV_BRO_AA_TIMEOUT;
        break;
    /* 启动中接收BCS超时 */
    case APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT:
        _reason = NETYKC_MONITOR_SF_REASON5D_RECV_BCS_TIMEOUT;
        break;
    /* 启动中接收BCL超时 */
    case APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT:
        _reason = NETYKC_MONITOR_SF_REASON5E_RECV_BCL_TIMEOUT;
        break;
    /* 充电中接收BCS超时 */
    case APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT:
        _reason = NETYKC_MONITOR_AS_REASON84_RECV_BCS_TIMEOUT;
        break;
    /* 充电中接收BCL超时 */
    case APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT:
        _reason = NETYKC_MONITOR_AS_REASON85_RECV_BCL_TIMEOUT;
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
        _reason = NETYKC_MONITOR_AS_REASON92_BATTERY_VOLTAGE;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = NETYKC_MONITOR_AS_REASONA7_CAR_STOP;
        break;
    /* 准备电压 */
    case APP_SYSTEM_STOP_WAY_READY_VOLT:
        _reason = NETYKC_MONITOR_SF_REASON61_BRO_AA_STAGE_VOLT_OVERRANGE;
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
    /* 刷离线卡 */
    case APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP:
        _reason = NETYKC_MONITOR_AS_REASOND1_OFFLINE_CARD;
        break;
    /* 刷在线卡 */
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = NETYKC_MONITOR_AS_REASOND0_ONLINE_CARD;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        _reason = NETYKC_MONITOR_AS_REASON6E_NO_BALLANCE;
        break;
    /* 屏幕 */
    case APP_SYSTEM_STOP_WAY_SCREEN_STOP:
        _reason = NETYKC_MONITOR_AS_REASONBA_SCREEN;
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
    /* VIN 码鉴权失败 */
    case APP_SYSTEM_STOP_WAY_AUTHEN_FAIL:
        _reason = NETYKC_MONITOR_AS_REASON93_VIN_AUTHEN_FAIL;
        break;
    /* 主机柜禁止充电 */
    case APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID:
        _reason = NETYKC_MONITOR_AS_REASON9C_MAIN_CABINET_FORBID;
        break;
    default:
    {
        uint16_t _way = mw_system_stop_way_convert(reason);
        switch(_way){
        /* 防雷器 */
        case APP_SYSTEM_STOP_WAY_LIGHTPROTECT:
            _reason = NETYKC_MONITOR_AS_REASON94_LIGHTPROTECT;
            break;
        /* 枪座 */
        case APP_SYSTEM_STOP_WAY_GUNSITE:
            _reason = NETYKC_MONITOR_AS_REASON95_GUNSITE;
            break;
        /* 断路器 */
        case APP_SYSTEM_STOP_WAY_CIRCUIT_BREAKER:
            _reason = NETYKC_MONITOR_AS_REASON96_CIRCUIT_BREAKER;
            break;
        /* 水浸 */
        case APP_SYSTEM_STOP_WAY_FLOODING:
            _reason = NETYKC_MONITOR_AS_REASON97_FLOODING;
            break;
        /* 烟感 */
        case APP_SYSTEM_STOP_WAY_SMOKE:
            _reason = NETYKC_MONITOR_AS_REASON98_SMOKE;
            break;
        /* 倾倒 */
        case APP_SYSTEM_STOP_WAY_POUR:
            _reason = NETYKC_MONITOR_AS_REASON99_POUR;
            break;
        /* 液冷 */
        case APP_SYSTEM_STOP_WAY_LIQUIDCOOLING:
            _reason = NETYKC_MONITOR_AS_REASON9A_LIQUIDCOOLING;
            break;
        /* 熔断器 */
        case APP_SYSTEM_STOP_WAY_FUSE:
            _reason = NETYKC_MONITOR_AS_REASON9B_FUSE;
            break;
        /* 主机柜故障 */
        case APP_SYSTEM_STOP_WAY_MAIN_CABINET_OFFLINE:
            _reason = NETYKC_MONITOR_AS_REASON9D_MAIN_CABINET_FAULT;
            break;
        /* 矩阵正负接触器KPN1-1 */
        case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN1_1:
            _reason = NETYKC_MONITOR_AS_REASON9F_MATRIX_RELAY_KPN1_1;
            break;
        /* 矩阵正负接触器KPN1-2 */
        case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN1_2:
            _reason = NETYKC_MONITOR_AS_REASONA0_MATRIX_RELAY_KPN1_2;
            break;
        /* 矩阵正负接触器KPN1-3 */
        case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN1_3:
            _reason = NETYKC_MONITOR_AS_REASONA1_MATRIX_RELAY_KPN1_3;
            break;
        /* 矩阵正负接触器KPN2-1 */
        case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN2_1:
            _reason = NETYKC_MONITOR_AS_REASONA2_MATRIX_RELAY_KPN2_1;
            break;
        /* 矩阵正负接触器KPN2-2 */
        case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN2_2:
            _reason = NETYKC_MONITOR_AS_REASONA3_MATRIX_RELAY_KPN2_2;
            break;
        /* 矩阵正负接触器KPN3-1 */
        case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN3_1:
            _reason = NETYKC_MONITOR_AS_REASONA4_MATRIX_RELAY_KPN3_1;
            break;
        /* 从设备离线 */
        case APP_SYSTEM_STOP_WAY_SLAVE_DEVICE_OFFLINE:
            _reason = NETYKC_MONITOR_AS_REASONA5_SLAVE_DEVICE_OFFLINE;
            break;
        /* 风扇 */
        case APP_SYSTEM_STOP_WAY_FAN:
            _reason = NETYKC_MONITOR_AS_REASONCA_FAN_FAULT;
            break;
        /* 主机柜急停 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_SCRAM:
            _reason = NETYKC_MONITOR_AS_REASONBF_MAIN_CABINET_SCRAM;
            break;
        /* 主机柜门禁 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_GATE:
            _reason = NETYKC_MONITOR_AS_REASONC0_MAIN_CABINET_DOOR;
            break;
        /* 主机柜开关板故障 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_PDUFAULT:
            _reason = NETYKC_MONITOR_AS_REASONC1_MAIN_CABINET_PDU;
            break;
        /* 主机柜模块 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_MODULEFAULT:
            _reason = NETYKC_MONITOR_AS_REASONC2_MAIN_CABINET_MODULEFAULT;
            break;
        /* 主机柜配置项 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_CONFIG:
            _reason = NETYKC_MONITOR_AS_REASONC3_MAIN_CABINET_CONFIG;
            break;
        /* 主机柜交流接触器 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_ACRELAY:
            _reason = NETYKC_MONITOR_AS_REASONC4_MAIN_CABINET_ACRELAY;
            break;
        /* 主机柜烟感报警 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_SMOKE:
            _reason = NETYKC_MONITOR_AS_REASONC5_MAIN_CABINET_SMOKE;
            break;
        /* 主机柜倾倒 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_POUR:
            _reason = NETYKC_MONITOR_AS_REASONC6_MAIN_CABINET_POUR;
            break;
        /* 主机柜水浸 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_FLOODING:
            _reason = NETYKC_MONITOR_AS_REASONC7_MAIN_CABINET_FLOODING;
            break;
        /* 主机柜其它故障 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_OTHER:
            _reason = NETYKC_MONITOR_AS_REASONC8_MAIN_CABINET_OTHER;
            break;
        /* 主机柜防雷故障 */
        case APP_SYSTEM_STOP_WAY_MAINCABINET_LIGHT_PROTECT:
            _reason = NETYKC_MONITOR_AS_REASONC9_MAIN_CABINET_LIGHTPROTECT;
            break;
        /* 设备已锁定 */
        case APP_SYSTEM_STOP_WAY_DEVICE_IS_LOCKED:
            _reason = NETYKC_MONITOR_AS_REASONCB_IS_LOCKED;
            break;
        /* 宇通BFC */
        case APP_SYSTEM_STOP_WAY_YT_BFC:
            _reason = NETYKC_MONITOR_AS_REASON9D_YT_BFC;
            break;
        /* 车端停详细原因：SOC达到目标值  */
        case APP_SYSTEM_STOP_WAY_BST_TARGET_SOC:
            _reason = NETYKC_MONITOR_AS_REASONA8_BST_TARGET_SOC;
            break;
        /* 车端停详细原因：总电压达到目标值  */
        case APP_SYSTEM_STOP_WAY_BST_TARGET_TOTAL_VOLT:
            _reason = NETYKC_MONITOR_AS_REASONA9_BST_TARGET_TVOLT;
            break;
        /* 车端停详细原因：单体电压达到目标值  */
        case APP_SYSTEM_STOP_WAY_BST_TARGET_SINGLE_VOLT:
            _reason = NETYKC_MONITOR_AS_REASONAA_BST_TARGET_SVOLT;
            break;
        /* 车端停详细原因：充电机主动停止  */
        case APP_SYSTEM_STOP_WAY_BST_CHARGER_END:
            _reason = NETYKC_MONITOR_AS_REASONAB_BST_CHARGER_END;
            break;
        /* 车端停详细原因：绝缘故障  */
        case APP_SYSTEM_STOP_WAY_BST_INSULATION_FAULT:
            _reason = NETYKC_MONITOR_AS_REASONAC_BST_INSULT;
            break;
        /* 车端停详细原因：输出连接器故障  */
        case APP_SYSTEM_STOP_WAY_BST_OUT_LINKER_FAULT:
            _reason = NETYKC_MONITOR_AS_REASONAD_BST_OUT_LINKER;
            break;
        /* 车端停详细原因：BMS元件故障  */
        case APP_SYSTEM_STOP_WAY_BST_BMS_ELEMENT:
            _reason = NETYKC_MONITOR_AS_REASONAE_BST_ELEMENT;
            break;
        /* 车端停详细原因：充电连接故障  */
        case APP_SYSTEM_STOP_WAY_BST_CHARGE_LINKER_FAULT:
            _reason = NETYKC_MONITOR_AS_REASONAF_BST_CHARGE_LINKER;
            break;
        /* 车端停详细原因：电池组温度故障  */
        case APP_SYSTEM_STOP_WAY_BST_BAT_GROUP_OT:
            _reason = NETYKC_MONITOR_AS_REASON7C_BATTERY_GROUP_OVERTEMP;
            break;
        /* 车端停详细原因：高压继电器故障  */
        case APP_SYSTEM_STOP_WAY_BST_HV_RELAY:
            _reason = NETYKC_MONITOR_AS_REASONB0_BST_HV_RELAY;
            break;
        /* 车端停详细原因：检测点2电压检测故障  */
        case APP_SYSTEM_STOP_WAY_BST_DETECT_PIONT_2:
            _reason = NETYKC_MONITOR_AS_REASONB1_BST_POINT_2;
            break;
        /* 车端停详细原因：充电电流过流  */
        case APP_SYSTEM_STOP_WAY_BST_OVER_CURRENT:
            _reason = NETYKC_MONITOR_AS_REASON7A_CHARGE_TCURRENT_ABNORMAL;
            break;
        /* 车端停详细原因：充电电压异常  */
        case APP_SYSTEM_STOP_WAY_BST_ABNORMAL_VOLTAGE:
            _reason = NETYKC_MONITOR_AS_REASON79_CHARGE_TVOLTAGE_ABNORMAL;
            break;
        /* BSM详细原因：单体电压异常 */
        case APP_SYSTEM_STOP_WAY_BSM_SINGLE_BAT_OV:
            _reason = NETYKC_MONITOR_AS_REASONB2_BSM_SVOLT;
            break;
        /* BSM详细原因：SOC状态异常 */
        case APP_SYSTEM_STOP_WAY_BSM_ABNORMAL_SOC:
            _reason = NETYKC_MONITOR_AS_REASONB3_BSM_SOC_STATE;
            break;
        /* BSM详细原因：电池充电过流 */
        case APP_SYSTEM_STOP_WAY_BSM_OVER_CURRENT:
            _reason = NETYKC_MONITOR_AS_REASONB4_BSM_OVERCURR;
            break;
        /* BSM详细原因：电池温度过高 */
        case APP_SYSTEM_STOP_WAY_BSM_BATTERY_OT:
            _reason = NETYKC_MONITOR_AS_REASONB5_BSM_BATGRP_OT;
            break;
        /* BSM详细原因：电池绝缘状态异常 */
        case APP_SYSTEM_STOP_WAY_BSM_BAT_INSULATION_ABNORMAL:
            _reason = NETYKC_MONITOR_AS_REASONB6_BSM_BAT_INSULT;
            break;
        /* BSM详细原因：输出连接器状态异常 */
        case APP_SYSTEM_STOP_WAY_BSM_OUT_LINKER_ABNORMAL:
            _reason = NETYKC_MONITOR_AS_REASONB7_BSM_OUT_LINKER;
            break;
        /* BSM详细原因：禁止充电 */
        case APP_SYSTEM_STOP_WAY_BSM_FORBID:
            _reason = NETYKC_MONITOR_AS_REASON82_CAR_COMMAND_STOP;
            break;
        default:
            break;
        }
    }
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

    if(ykc_monitor_get_socket_info()->socket_state == YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
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
        if((base->state.current == APP_OFSM_STATE_CHARGING) || \
                ((base->state.current == APP_OFSM_STATE_STOPING) && (base->flag.start_result == NET_ENUM_TRUE))){
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
    case APP_OFSM_STATE_RESERVATION:
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
static void ykc_monitor_state_changed_check(uint8_t gunno, System_BaseData * base)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(base == NULL){
        return;
    }

#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
    if((g_ykc_monitor_preq_report_realtime_data[gunno].body.plug_gun != s_ykc_monitor_state_info[gunno].state.connect) ||
            (g_ykc_monitor_preq_report_realtime_data[gunno].body.state != s_ykc_monitor_state_info[gunno].state.state) ||
            (memcmp(g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set, s_ykc_monitor_state_info[gunno].fault_code, sizeof(s_ykc_monitor_state_info[gunno].fault_code)))){
#else
    if((g_ykc_monitor_preq_report_realtime_data[gunno].body.plug_gun != s_ykc_monitor_state_info[gunno].state.connect) ||
            (g_ykc_monitor_preq_report_realtime_data[gunno].body.state != s_ykc_monitor_state_info[gunno].state.state) ||
            (g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault != s_ykc_monitor_state_info[gunno].fault_code)){
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        if(ykc_monitor_get_message_send_state(gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA) == NET_YKC_MONITOR_SEND_STATE_COMPLETE){
            /** 由于 s_ykc_monitor_state_info[gunno].state.connect 和 s_ykc_monitor_state_info[gunno].state.state 和
             *  s_ykc_monitor_state_info[gunno].fault_code 会在其它线程被赋值，为了防止用这几个值做判断时和赋值时可能存在的不一致而导致
                            *     状态错乱问题，将这几个值进行临时存储用于判断和赋值*/
            uint8_t _connect = s_ykc_monitor_state_info[gunno].state.connect;
            uint8_t _state = s_ykc_monitor_state_info[gunno].state.state;
            uint8_t is_faulting = NET_ENUM_FALSE;        /** 是故障状态 */
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
            uint32_t _fault[NET_YKC_MONITOR_FAULT_SET_NUM];
            for(uint8_t i = 0x00; i < NET_YKC_MONITOR_FAULT_SET_NUM; i++){
                _fault[i] = s_ykc_monitor_state_info[gunno].fault_code[i];
                if(_fault[i] != 0x00){
                    is_faulting = NET_ENUM_TRUE;
                }
            }
#else
            uint16_t _fault = s_ykc_monitor_state_info[gunno].fault_code;
            if(_fault){
                is_faulting = NET_ENUM_TRUE;
            }
#endif /* #ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND */

            if(_state == NETYKC_MONITOR_DEVICE_STATE_FAULTING){
                if((is_faulting == NET_ENUM_TRUE) || (base->device_state == APP_DEVICE_STATE_OVERHAUL) || (base->device_state == APP_DEVICE_STATE_FREEZE)){
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
                    for(uint8_t i = 0x00; i < NET_YKC_MONITOR_FAULT_SET_NUM; i++){
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set[i] = _fault[i];
                    }
#else
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault = _fault;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.plug_gun = _connect;
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.state = _state;

                    s_ykc_monitor_realtime_data_count[gunno] = rt_tick_get();
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EVENT_HANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST, gunno, NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA);
                }
                if((base->device_state == APP_DEVICE_STATE_OVERHAUL) || (base->device_state == APP_DEVICE_STATE_FREEZE)){
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set[NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_DEVICE_FAULT_LOCK_DEVICE;
                }
            }else{
                if(is_faulting == NET_ENUM_FALSE){
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
                    for(uint8_t i = 0x00; i < NET_YKC_MONITOR_FAULT_SET_NUM; i++){
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set[i] = _fault[i];
                        g_ykc_monitor_preq_report_realtime_data[gunno].body.fault_set[i] &= ~(YKC_MONITOR_DEVICE_FAULT_LOCK_DEVICE);
                    }
#else
                    g_ykc_monitor_preq_report_realtime_data[gunno].body.hardware_fault = _fault;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
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
    extern uint8_t thaisenGetEnableModuleOperateState(void);
    extern uint8_t thaisenGetEnableModuleOperateResult(void);

    System_BaseData *base = NULL;
    uint8_t gunno = 0x00, system_is_idle = 0x00;

    s_ykc_monitor_mfault_info.check_tick = rt_tick_get();

    while(1){
        net_thread_running(rt_thread_self(), NULL, 0x00, 0x00);
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }
        if(s_ykc_monitor_handle == NULL){
            rt_thread_mdelay(100);
            continue;
        }

        system_is_idle = 0x00;   /** 系统是空闲状态 */
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
            ykc_monitor_fault_detect_report(gunno);
            ykc_monitor_data_realtime_process(gunno, base);
            ykc_monitor_state_changed_check(gunno, base);

            if((rt_tick_get() - s_ykc_monitor_mfault_info.check_tick) > YKC_MONITOR_MFAULT_CHECK_PERIOD){
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
                ykc_monitor_module_fault_check();
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
                s_ykc_monitor_mfault_info.check_tick = rt_tick_get();
            }
            if(!((base->state.current >= APP_OFSM_STATE_STARTING) && (base->state.current <= APP_OFSM_STATE_CHARGING))){
                system_is_idle++;
            }
        }
#ifdef NET_YKC_MONITOR_AS_MONITOR
        /** 液冷故障检测 */
        ykc_monitor_liquid_fault_check();

        /************************************** 上报给每个模块设置的电压、电流 **************************************/
        /******************* 系统非空闲，有枪在充电 ********************/
        if(system_is_idle < NET_SYSTEM_GUN_NUMBER){
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
        }else{
            if(s_ykc_monitor_setvoltcurr.count != 0x00){
                if(s_ykc_monitor_setvoltcurr.is_locked == NET_ENUM_FALSE){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_SET_VOLTCURR);
                    s_ykc_monitor_setvoltcurr.is_locked = NET_ENUM_TRUE;
                }
            }else{
                s_ykc_monitor_setvoltcurr.count = 0x00;
                s_ykc_monitor_setvoltcurr.is_locked = NET_ENUM_FALSE;
            }
        }

        /********************************************** 凌康锁模块处理 **********************************************/

        if(s_ykc_monitor_lock_module.flag.is_wait_response == NET_ENUM_TRUE){
            if((rt_tick_get() - s_ykc_monitor_lock_module.base_tick) > 10000){
                s_ykc_monitor_lock_module.flag.operate_result = 0x00;
                s_ykc_monitor_lock_module.flag.is_wait_response = NET_ENUM_FALSE;
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                        0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_LOCK_MODULE_RESPONSE);
            }
            if(thaisenGetEnableModuleOperateState()){
                if(thaisenGetEnableModuleOperateResult()){
                    s_ykc_monitor_lock_module.flag.operate_result = 0x01;
                }else{
                    s_ykc_monitor_lock_module.flag.operate_result = 0x00;
                }
                s_ykc_monitor_lock_module.flag.is_wait_response = NET_ENUM_FALSE;
                ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                        0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_LOCK_MODULE_RESPONSE);
            }
        }else{
            s_ykc_monitor_lock_module.base_tick = rt_tick_get();
        }

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            /********************** 导引变化信息 **********************/
            if(s_ykc_monitor_guidance_changed[gunno].is_sending == NET_ENUM_FALSE){
                if(s_ykc_monitor_guidance_changed[gunno].count){
                    s_ykc_monitor_guidance_changed[gunno].is_sending = NET_ENUM_TRUE;
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_GUIDANCE_CHANGED);
                }
            }
            /********************** 器件控制变化信息 **********************/
            if(s_ykc_monitor_device_control_changed[gunno].is_sending == NET_ENUM_FALSE){
                if(s_ykc_monitor_device_control_changed[gunno].count){
                    if(s_ykc_monitor_device_control_changed[gunno].delay_count < (0xFF - 0x01)){
                        s_ykc_monitor_device_control_changed[gunno].delay_count++;
                    }
                    /** 一次上报3s内器件的变化(线程运行时间100ms) */
                    if(s_ykc_monitor_device_control_changed[gunno].delay_count > (3000 /100)){
                        s_ykc_monitor_device_control_changed[gunno].is_sending = NET_ENUM_TRUE;
                        ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                                gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_DEVICE_CTRL_CHANGED);
                    }
                }else{
                    s_ykc_monitor_device_control_changed[gunno].delay_count = 0x00;
                }
            }else{
                s_ykc_monitor_device_control_changed[gunno].delay_count = 0x00;
            }

            /********************** 器件反馈变化信息 **********************/
            if(s_ykc_monitor_device_status_changed[gunno].is_recved == NET_ENUM_TRUE){
                if(s_ykc_monitor_device_status_changed[gunno].delay_count < (0xFF - 0x01)){
                    s_ykc_monitor_device_status_changed[gunno].delay_count++;
                }
                /** 连续4s内器件控制状态无变化再上报反馈状态(线程运行时间100ms) */
                if(s_ykc_monitor_device_status_changed[gunno].delay_count > (4000 /100)){
                    rt_enter_critical();
                    /** 防止其它线程修改 delay_count */
                    if(s_ykc_monitor_device_status_changed[gunno].delay_count > (2000 /100)){
                        s_ykc_monitor_device_status_changed[gunno].is_recved = NET_ENUM_FALSE;
                        ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                                gunno, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_DEVICE_FB_CHANGED);
                    }
                    rt_exit_critical();
                }
            }else{
                s_ykc_monitor_device_status_changed[gunno].delay_count = 0x00;
            }
        }
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_DESIGNATE_REGION */


        rt_thread_mdelay(100);
    }
}

/****************************************************
 * 函数名            ykc_monitor_clear_guidance_changed_sending
 * 功能               清除导引状态变化数据正在发送标志
 ***************************************************/
void ykc_monitor_clear_guidance_changed_sending(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_ykc_monitor_guidance_changed[gunno].is_sending = NET_ENUM_FALSE;
}

/****************************************************
 * 函数名            ykc_monitor_clear_dev_control_changed_sending
 * 功能               清除器件控制状态变化数据正在发送标志
 ***************************************************/
void ykc_monitor_clear_dev_control_changed_sending(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_ykc_monitor_device_control_changed[gunno].is_sending = NET_ENUM_FALSE;
}

/*******************************************************
 * 函数名               ykc_monitor_clear_disconnect_reason
 * 功能                  清除断网原因信息
 * 参数
 * 返回
 ******************************************************/
void ykc_monitor_clear_disconnect_reason(void)
{
    memset(&s_ykc_monitor_close_passive, 0x00, sizeof(s_ykc_monitor_close_passive));
    memset(&s_ykc_monitor_close_active, 0x00, sizeof(s_ykc_monitor_close_active));
    memset(&s_ykc_monitor_heartbeat_timeout, 0x00, sizeof(s_ykc_monitor_heartbeat_timeout));
    memset(&s_ykc_monitor_socket_pdp, 0x00, sizeof(s_ykc_monitor_socket_pdp));
    memset(&s_ykc_monitor_close_module, 0x00, sizeof(s_ykc_monitor_close_module));
    memset(&s_ykc_monitor_at_physice, 0x00, sizeof(s_ykc_monitor_at_physice));
    memset(&s_ykc_monitor_cpin_lk_mac, 0x00, sizeof(s_ykc_monitor_cpin_lk_mac));
    memset(&s_ykc_monitor_cimi_lk_mac, 0x00, sizeof(s_ykc_monitor_cimi_lk_mac));
    memset(&s_ykc_monitor_signal_strength_lk_mac, 0x00, sizeof(s_ykc_monitor_signal_strength_lk_mac));
    memset(&s_ykc_monitor_gsm_registered, 0x00, sizeof(s_ykc_monitor_gsm_registered));
    memset(&s_ykc_monitor_gprs_registered, 0x00, sizeof(s_ykc_monitor_gprs_registered));
}

int32_t ykc_monitor_realtime_process_init(void)
{
    uint8_t entry = 0x03, name[NET_THREAD_MONITOR_NAME_MAX];

#ifdef NET_DESIGNATE_REGION
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_monitor_realtime_data_interval[gunno] = 0x00;
        s_ykc_monitor_realtime_data_count[gunno] = 0x00;

        memset(&s_ykc_monitor_flag_info[gunno], 0x00, sizeof(s_ykc_monitor_flag_info[gunno]));
        memset(&s_ykc_monitor_state_info[gunno], 0x00, sizeof(s_ykc_monitor_state_info[gunno]));

#ifdef NET_YKC_MONITOR_AS_MONITOR
        memset(&s_ykc_monitor_starting_info[gunno], 0x00, sizeof(s_ykc_monitor_starting_info[gunno]));
        memset(&s_ykc_monitor_charging_info[gunno], 0x00, sizeof(s_ykc_monitor_charging_info[gunno]));
        memset(&s_ykc_monitor_mfault_info, 0x00, sizeof(s_ykc_monitor_mfault_info));
        memset(&s_ykc_monitor_lock_module, 0x00, sizeof(s_ykc_monitor_lock_module));
#endif /* NET_YKC_MONITOR_AS_MONITOR */
    }

#ifdef NET_YKC_MONITOR_AS_MONITOR
    memset(&s_ykc_monitor_setvoltcurr, 0x00, sizeof(s_ykc_monitor_setvoltcurr));
#endif /* NET_YKC_MONITOR_AS_MONITOR */
    memset(s_ykc_monitor_guidance_changed, 0x00, sizeof(s_ykc_monitor_guidance_changed));
    memset(s_ykc_monitor_device_control_changed, 0x00, sizeof(s_ykc_monitor_device_control_changed));
    memset(s_ykc_monitor_device_status_changed, 0x00, sizeof(s_ykc_monitor_device_status_changed));
    memset(&s_ykcm_liquid_f_info, 0x00, sizeof(s_ykcm_liquid_f_info));
	ykc_monitor_clear_disconnect_reason();
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

    net_thread_init_hook(&s_ykc_monitor_realtime_process_thread, &entry, sizeof(entry), NET_THREAD_RUNNING_OPTION_ENTRY_MAX);

    memset(name, 0x00, NET_THREAD_MONITOR_NAME_MAX);
    memcpy(name, "ym_rlp", strlen("ym_rlp"));
    net_thread_init_hook(&s_ykc_monitor_realtime_process_thread, name, strlen((char*)name), NET_THREAD_RUNNING_OPTION_NAME);

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
    message->body.cc4_uplimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC14V_MAX, 0x00));
    message->body.cc4_downlimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC14V_MIN, 0x00));
    message->body.cc6_uplimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC16V_MAX, 0x00));
    message->body.cc6_downlimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC16V_MIN, 0x00));
    message->body.cc12_uplimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC112V_MAX, 0x00));
    message->body.cc12_downlimit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC112V_MIN, 0x00));

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
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint16_t total = sizeof(Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    uint8_t *domain = NULL, valid_len = 0x00;
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    net_plat_socket_info_t *socket = net_operation_get_target_socket_info(NET_ENUM_FALSE);
    Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t *message = (Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t*)buf;

    memset(message, 0x00, ilen);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    domain = (uint8_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_DOMAIN, NULL, 0x00, option));
    valid_len = strlen((char*)domain);
    valid_len = valid_len > sizeof(message->body.domain) ? sizeof(message->body.domain) : valid_len;
    memcpy(message->body.domain, domain, valid_len);

    message->body.port = *(uint16_t*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PORT, NULL, 0x00, option));

    message->body.program_state = socket->program_state;
    message->body.socket_state = socket->socket_state;
    message->body.open_count = socket->open_count;
    message->body.login_count = socket->login_count;
    message->body.heartbeat_count = socket->heartbeat_count;

    if(olen){
        *olen = total;
    }

    LOG_D("net target socket info:");
    LOG_D("pile_number|%s", message->body.pile_number);
    LOG_D("domain|%s", message->body.domain);
    LOG_D("port|%d", message->body.port);
    LOG_D("program_state|%d", message->body.program_state);
    LOG_D("socket_state|%d", message->body.socket_state);
    LOG_D("open_count|%d", message->body.open_count);
    LOG_D("login_count|%d", message->body.login_count);
    LOG_D("heartbeat_count|%d", message->body.heartbeat_count);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
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
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    ykc_monitor_storage_struct *config = (ykc_monitor_storage_struct*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT));
#endif /* #ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
    memset(message, 0x00, sizeof(Net_YkcMonitorPro_Preq_PRes_DevInfo_t));
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
    message->body.target_plat_protocol = 0x06;
#elif defined(NET_YKC_DERIVE_PRO_TLD)
    message->body.target_plat_protocol = 0x07;
#elif defined(NET_YKC_DERIVE_PRO_DUPU)
    message->body.target_plat_protocol = 0x0F;
#elif defined(NET_YKC_DERIVE_PRO_XDT)
    message->body.target_plat_protocol = 0x10;
#elif defined(NET_YKC_DERIVE_PRO_TT)
    message->body.target_plat_protocol = 0x11;
#elif defined(NET_YKC_DERIVE_PRO_XJ)
    message->body.target_plat_protocol = 0x0E;
#else
    message->body.target_plat_protocol = 0x00;
#endif /* NET_YKC_DERIVE_PRO_XXCD */

#elif (NET_TARGET_PLATFORM_ID == NET_YCP_PRO_ID)
    message->body.target_plat_protocol = 0x02;
#elif (NET_TARGET_PLATFORM_ID == NET_YND_PRO_ID)
    message->body.target_plat_protocol = 0x03;
#elif (NET_TARGET_PLATFORM_ID == NET_XJ_PRO_ID)
    message->body.target_plat_protocol = 0x04;
#elif (NET_TARGET_PLATFORM_ID == NET_SL_PRO_ID)
    message->body.target_plat_protocol = 0x05;

#elif (NET_TARGET_PLATFORM_ID == NET_CDW_PRO_ID)
    message->body.target_plat_protocol = 0x0A;
#elif (NET_TARGET_PLATFORM_ID == NET_YD_PRO_ID)
    message->body.target_plat_protocol = 0x0B;
#elif (NET_TARGET_PLATFORM_ID == NET_JR_PRO_ID)
    message->body.target_plat_protocol = 0x0C;
#elif (NET_TARGET_PLATFORM_ID == NET_WXN_PRO_ID)
    message->body.target_plat_protocol = 0x0D;

#elif (NET_TARGET_PLATFORM_ID == NET_SGCC_PRO_ID)
    message->body.target_plat_protocol = 0x01;
#elif (NET_TARGET_PLATFORM_ID == NET_QBJ_PRO_ID)
    message->body.target_plat_protocol = 0x13;
#else
    message->body.target_plat_protocol = 0x00;
#endif

#else
    message->body.target_plat_protocol = 0x00;
#endif /* NET_INCLUDE_TARGET_PLATFORM */

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(config){
        message->body.flag.verify_result = config->verify_result;
        message->body.reset_count = config->reset_count;
        message->body.reset_reason = config->reset_reason;
        if(strlen((char*)config->reset_lable) > sizeof(message->body.reset_lable)){
            memcpy(message->body.reset_lable, config->reset_lable, sizeof(message->body.reset_lable));
        }else{
            memcpy(message->body.reset_lable, config->reset_lable, strlen((char*)config->reset_lable));
        }
    }

    (void)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_ICCID, message->body.sim_no, sizeof(message->body.sim_no), option));
#endif /* #ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

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
    thaisenMsgRecved_t msg_recved;
    thaisenMsgSended_t msg_sended;

    if(s_ykc_monitor_starting_info[gunno].count == 0x00){
        s_ykc_monitor_starting_info[gunno].timestamp = base->current_time;
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

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].module_voltage.symbol = 0x00;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].module_voltage.data = thaisen_get_module_volt(gunno);

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].pile_measure_voltage = base->voltage_a /10;

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
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].positive_insul_resistance = value0;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].negative_insul_resistance = value1;

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].state = mw_get_charge_library_state(base->main_gunno);
    }else{
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].state = mw_get_charge_library_state(gunno);
    }

    rt_enter_critical();
    msg_recved = thaisenGetMsgRecved(gunno);
    msg_sended = thaisenGetMsgSended(gunno);
    rt_exit_critical();
    /********************************************** 报文接收 **********************************************/
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message = 0x00;
    /** BHM */
    if(msg_recved.BHM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BHM;
    }
    /** BRM */
    if(msg_recved.BRM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BRM;
    }
    /** BCP */
    if(msg_recved.BCP){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BCP;
    }
    /** BRO_00 */
    if(msg_recved.BRO){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BRO_00;
    }
    /** BRO_AA */
    if(msg_recved.BRO_AA){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BRO_AA;
    }
    /** BCL */
    if(msg_recved.BCL){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BCL;
    }
    /** BCS */
    if(msg_recved.BCS){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BCS;
    }
    /** BSM */
    if(msg_recved.BSM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BSM;
    }
    /** BST */
    if(msg_recved.BST){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BST;
    }
    /** BSD */
    if(msg_recved.BSD){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BSD;
    }
    /** BEM */
    if(msg_recved.BEM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BEM;
    }
    /** BFC */
    if(msg_recved.BFC){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BFC;
    }

    /********************************************** 报文发送 **********************************************/
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message = 0x00;
    /** CHM */
    if(msg_sended.CHM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CHM;
    }
    /** CRM */
    if(msg_sended.CRM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRM;
    }
    /** CRM_AA */
    if(msg_sended.CRM_AA){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRM_AA;
    }
    /** CFC */
    if(msg_sended.CFC){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CFC;
    }
    /** CTS */
    if(msg_sended.CTS){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CTS;
    }
    /** CML */
    if(msg_sended.CML){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CML;
    }
    /** CRO */
    if(msg_sended.CRO){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRO;
    }
    /** CRO_AA */
    if(msg_sended.CRO_AA){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRO_AA;
    }
    /** CCS */
    if(msg_sended.CCS){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CCS;
    }
    /** CST */
    if(msg_sended.CST){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CST;
    }
    /** CSD */
    if(msg_sended.CSD){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CSD;
    }
    /** CEM */
    if(msg_sended.CEM){
        s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CEM;
    }

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].max_alllow_voltage.symbol = 0x00;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].max_alllow_voltage.data = bms->BHM.MaxAllowVol;

    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].battery_voltage.symbol = 0x00;
    s_ykc_monitor_starting_info[gunno].info[s_ykc_monitor_starting_info[gunno].count].battery_voltage.data = bms->BCP.BatVolt;

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
    message->body.info.msg_version = 0x01;
    message->body.info.info_type = NET_YKC_MONITOR_PROCESS_INFO_TYPE_STARTING;
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
    thaisenMsgRecved_t msg_recved;
    thaisenMsgSended_t msg_sended;

    if(s_ykc_monitor_charging_info[gunno].count == 0x00){
        s_ykc_monitor_charging_info[gunno].timestamp = base->current_time;
    }

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_voltage.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_voltage.data = thaisen_get_module_volt(gunno);

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_current.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].module_current.data = thaisen_get_module_curr(gunno);

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_voltage = base->voltage_a /10;

    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_current.symbol = 0x00;
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].pile_measure_current.data = base->current_a /10;

    if((base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD) || (base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL)){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].state = mw_get_charge_library_state(base->main_gunno);
    }else{
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].state = mw_get_charge_library_state(gunno);
    }

    rt_enter_critical();
    msg_recved = thaisenGetMsgRecved(gunno);
    msg_sended = thaisenGetMsgSended(gunno);
    rt_exit_critical();

    /********************************************** 报文接收 **********************************************/
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message = 0x00;
    /** BHM */
    if(msg_recved.BHM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BHM;
    }
    /** BRM */
    if(msg_recved.BRM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BRM;
    }
    /** BCP */
    if(msg_recved.BCP){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BCP;
    }
    /** BRO_00 */
    if(msg_recved.BRO){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BRO_00;
    }
    /** BRO_AA */
    if(msg_recved.BRO_AA){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BRO_AA;
    }
    /** BCL */
    if(msg_recved.BCL){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BCL;
    }
    /** BCS */
    if(msg_recved.BCS){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BCS;
    }
    /** BSM */
    if(msg_recved.BSM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BSM;
    }
    /** BST */
    if(msg_recved.BST){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BST;
    }
    /** BSD */
    if(msg_recved.BSD){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BSD;
    }
    /** BEM */
    if(msg_recved.BEM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BEM;
    }
    /** BFC */
    if(msg_recved.BFC){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].recved_message |= YKC_MONITOR_RECVED_MSG_BFC;
    }

    /********************************************** 报文发送 **********************************************/
    s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message = 0x00;
    /** CHM */
    if(msg_sended.CHM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CHM;
    }
    /** CRM */
    if(msg_sended.CRM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRM;
    }
    /** CRM_AA */
    if(msg_sended.CRM_AA){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRM_AA;
    }
    /** CFC */
    if(msg_sended.CFC){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CFC;
    }
    /** CTS */
    if(msg_sended.CTS){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CTS;
    }
    /** CML */
    if(msg_sended.CML){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CML;
    }
    /** CRO */
    if(msg_sended.CRO){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRO;
    }
    /** CRO_AA */
    if(msg_sended.CRO_AA){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CRO_AA;
    }
    /** CCS */
    if(msg_sended.CCS){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CCS;
    }
    /** CST */
    if(msg_sended.CST){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CST;
    }
    /** CSD */
    if(msg_sended.CSD){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CSD;
    }
    /** CEM */
    if(msg_sended.CEM){
        s_ykc_monitor_charging_info[gunno].info[s_ykc_monitor_charging_info[gunno].count].sended_message |= YKC_MONITOR_SENDED_MSG_CEM;
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
    message->body.info.msg_version = 0x01;
    message->body.info.info_type = NET_YKC_MONITOR_PROCESS_INFO_TYPE_CHARGING;
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

#define YKC_MONITOR_RECVED_MSG_OFFSET       8
#define YKC_MONITOR_SENDED_MSG_OFFSET       9

    extern uint8_t mw_get_charge_library_state(uint8_t gunno);

    Net_YkcMonitorPro_Preq_ProcessInfo_t *message = (Net_YkcMonitorPro_Preq_ProcessInfo_t*)buf;
    struct finish_info *info = (struct finish_info*)(&message->body.group_num + 0x01);
    System_BaseData *base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    thaisenMsgRecved_t msg_recved;
    thaisenMsgSended_t msg_sended;

    memset(message, 0x00, data_len);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    message->body.timestamp = base->current_time;
    message->body.info.msg_version = 0x01;
    message->body.info.info_type = NET_YKC_MONITOR_PROCESS_INFO_TYPE_FINISH;
    message->body.gunno = (gunno + 0x01);
    message->body.group_num = NET_YKC_MONITOR_FINISH_INFO_MAX;

    info->state = mw_get_charge_library_state(gunno);

    rt_enter_critical();
    msg_recved = thaisenGetMsgRecved(gunno);
    msg_sended = thaisenGetMsgSended(gunno);
    rt_exit_critical();

    /********************************************** 报文接收 **********************************************/
    info->recved_message = 0x00;
    /** BEM */
    if(msg_recved.BEM){
        info->recved_message |= (YKC_MONITOR_RECVED_MSG_BEM >>YKC_MONITOR_RECVED_MSG_OFFSET);
    }
    /** BSM */
    if(msg_recved.BSM){
        info->recved_message |= (YKC_MONITOR_RECVED_MSG_BSM >>YKC_MONITOR_RECVED_MSG_OFFSET);
    }
    /** BST */
    if(msg_recved.BST){
        info->recved_message |= (YKC_MONITOR_RECVED_MSG_BST >>YKC_MONITOR_RECVED_MSG_OFFSET);
    }
    /** BSD */
    if(msg_recved.BSD){
        info->recved_message |= (YKC_MONITOR_RECVED_MSG_BSD >>YKC_MONITOR_RECVED_MSG_OFFSET);
    }
    /********************************************** 报文发送 **********************************************/
    info->sended_message = 0x00;
    /** CEM */
    if(msg_sended.CEM){
        info->sended_message |= (YKC_MONITOR_SENDED_MSG_CEM >>YKC_MONITOR_SENDED_MSG_OFFSET);
    }
    /** CST */
    if(msg_sended.CST){
        info->sended_message |= (YKC_MONITOR_SENDED_MSG_CST >>YKC_MONITOR_SENDED_MSG_OFFSET);
    }
    /** CSD */
    if(msg_sended.CSD){
        info->sended_message |= (YKC_MONITOR_SENDED_MSG_CSD >>YKC_MONITOR_SENDED_MSG_OFFSET);
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
 * 函数名      ykc_monitor_response_padding_lock_module
 * 功能          组包：锁、解锁模块控制结果响应
 * **********************************************/
int8_t ykc_monitor_response_padding_lock_module(uint8_t *buf, uint16_t ilen, uint16_t *olen)
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
    if(s_ykc_monitor_lock_module.flag.operate_result){
        fswitch->body.result = 0x01;
    }

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

/*************************************************
 * 函数名      ykc_monitor_message_padding_billing_rule_info
 * 功能          组包：计费信息
 * **********************************************/
int8_t ykc_monitor_message_padding_billing_rule(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(buf == NULL){
        return -0x01;
    }
    if(ilen < sizeof(Net_YkcMonitorPro_Preq_Pres_BillingRule_t)){
        return -0x04;
    }

    uint8_t is_locked_fees_type = NET_ENUM_FALSE, gunno = 0x00, count = 0x00;
    uint16_t total_len = 0x00;
    Net_YkcMonitorPro_Preq_Pres_BillingRule_t *message = (Net_YkcMonitorPro_Preq_Pres_BillingRule_t*)buf;

#ifdef NET_INCLUDE_TARGET_PLATFORM
#if ((NET_TARGET_PLATFORM_ID == NET_YKC_PRO_ID) || (NET_TARGET_PLATFORM_ID == NET_YCP_PRO_ID))
    is_locked_fees_type = NET_ENUM_TRUE;
    total_len = (sizeof(Net_YkcMonitorPro_Preq_Pres_BillingRule_t) + sizeof(struct fees_type_ykc16));
    if(ilen < total_len){
        LOG_W("ykc monitor buf too short when padding billing rule(yck/ycp)[%d, %d]", total_len, ilen);
        return -0x03;
    }
#elif (NET_TARGET_PLATFORM_ID == NET_YKC20_PRO_ID)
    is_locked_fees_type = NET_ENUM_FALSE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_XJ_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_SL_PRO_ID)
    is_locked_fees_type = NET_ENUM_FALSE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_CDW_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_YD_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_JR_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_WXN_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_QBJ_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    /** 总长度判断 */
#elif (NET_TARGET_PLATFORM_ID == NET_SGCC_PRO_ID)
    is_locked_fees_type = NET_ENUM_TRUE;
    total_len = (sizeof(Net_YkcMonitorPro_Preq_Pres_BillingRule_t) + sizeof(struct fees_type_period15min) *APP_BILLING_RULE_PERIOD_MAX);
    if(ilen < total_len){
        LOG_W("ykc monitor buf too short when padding billing rule(sgcc)[%d, %d]", total_len, ilen);
        return -0x03;
    }
#else
    LOG_D("ykc monitor no found match target platform(padding billing rule)");
    return -0x03;
#endif /* (NET_TARGET_PLATFORM_ID == NET_YKC_PRO_ID) */
#else
    LOG_D("ykc monitor not include target platform(padding billing rule)");
    return -0x03;
#endif /* NET_INCLUDE_TARGET_PLATFORM */

    /**************** 费率信息组包 ********************/
    if(is_locked_fees_type == NET_ENUM_TRUE){
        memset(message, 0x00, total_len);
        memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
        for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            if(net_operation_is_gunno_updated_fees(i)){
                count++;
                gunno = i;
            }
        }
#ifdef NET_INCLUDE_TARGET_PLATFORM
#if ((NET_TARGET_PLATFORM_ID == NET_YKC_PRO_ID) || (NET_TARGET_PLATFORM_ID == NET_YCP_PRO_ID))
        /** 云快充1.6/越城公用协议费率信息组包 */
        struct fees_type_ykc16 *info = (struct fees_type_ykc16*)(buf + sizeof(Net_YkcMonitorPro_Preq_Pres_BillingRule_t) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        sprintf((char*)message->body.emodel_sn, "%d", *(uint16_t*)app_billingrule_get_elect_model_sn(gunno));
        sprintf((char*)message->body.smodel_sn, "%d", *(uint16_t*)app_billingrule_get_service_model_sn(gunno));
        message->body.group_num = NET_YKC_MONITOR_RATE_PERIOD_COUNT_MAX;
        message->body.fees_type = NET_YKC_MONITOR_FEES_TYPE_YKC15;

        for(uint8_t i = 0x00; i < APP_BILLING_RULE_PERIOD_MAX; i += 0x02){
            info->rate_number[i /0x02] = app_billingrule_get_period_rate_number(gunno, i);
            /****************** 判尖电费 **********************/
            if(info->tip_elect_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_SHARP){
                    info->tip_elect_rate = app_billingrule_get_period_elect_price(gunno, i) *10;
                }
            }
            /** 判尖服务费 */
            if(info->tip_service_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_SHARP){
                    info->tip_service_rate = app_billingrule_get_period_service_price(gunno, i) *10;
                }
            }

            /****************** 判峰电费 **********************/
            if(info->peak_elect_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_PEAK){
                    info->peak_elect_rate = app_billingrule_get_period_elect_price(gunno, i) *10;
                }
            }
            /** 判峰服务费 */
            if(info->peak_service_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_PEAK){
                    info->peak_service_rate = app_billingrule_get_period_service_price(gunno, i) *10;
                }
            }

            /****************** 判平电费 *********************/
            if(info->flat_elect_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_FLAT){
                    info->flat_elect_rate = app_billingrule_get_period_elect_price(gunno, i) *10;
                }
            }
            /** 判平服务费 */
            if(info->flat_service_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_FLAT){
                    info->flat_service_rate = app_billingrule_get_period_service_price(gunno, i) *10;
                }
            }

            /****************** 判谷电费 *********************/
            if(info->valley_elect_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_VALLEY){
                    info->valley_elect_rate = app_billingrule_get_period_elect_price(gunno, i) *10;
                }
            }
            /** 判谷服务费 */
            if(info->valley_service_rate == 0x00){
                if(info->rate_number[i /0x02] == APP_RATE_TYPE_VALLEY){
                    info->valley_service_rate = app_billingrule_get_period_service_price(gunno, i) *10;
                }
            }
        }
        info->loss_proportion = app_billingrule_query_eloss_proportion();
        if(count >= NET_SYSTEM_GUN_NUMBER){
            gunno = 0xFF;
        }
        message->body.gunno = gunno;
#elif (NET_TARGET_PLATFORM_ID == NET_YKC20_PRO_ID)
        /** 云快充2.0协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_XJ_PRO_ID)
        /** 小桔协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_SL_PRO_ID)
        /** 阳关乐通协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_CDW_PRO_ID)
        /** 车电网协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_YD_PRO_ID)
        /** 一电协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_JR_PRO_ID)
        /** 久融协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_WXN_PRO_ID)
        /** 皖小能协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_QBJ_PRO_ID)
        /** 柒捌玖协议费率信息组包 */
#elif (NET_TARGET_PLATFORM_ID == NET_SGCC_PRO_ID)
        /** 国网协议费率信息组包 */
        struct fees_type_period15min *info = (struct fees_type_period15min*)(buf + sizeof(Net_YkcMonitorPro_Preq_Pres_BillingRule_t) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
        sprintf((char*)message->body.emodel_sn, "%s", app_billingrule_get_elect_model_sn(gunno));
        sprintf((char*)message->body.smodel_sn, "%s", app_billingrule_get_service_model_sn(gunno));
        message->body.group_num = APP_BILLING_RULE_PERIOD_MAX;
        message->body.fees_type = NET_YKC_MONITOR_FEES_TYPE_PERIOD_15MIN;

        for(uint8_t i = 0x00; i < APP_BILLING_RULE_PERIOD_MAX; i++){
            info[i].elect_fees = app_billingrule_get_period_elect_price(gunno, i);
            info[i].service_fees = app_billingrule_get_period_service_price(gunno, i);
            info[i].delay_fees = app_billingrule_get_period_delay_price(gunno, i);
            info[i].reserve = 0x00;
        }
        if(count >= NET_SYSTEM_GUN_NUMBER){
            gunno = 0xFF;
        }
        message->body.gunno = gunno;
#else
        LOG_D("ykc monitor match target platform error(padding billing rule)");
        return -0x03;
#endif /* (NET_TARGET_PLATFORM_ID == NET_YKC_PRO_ID) */
#endif /* NET_INCLUDE_TARGET_PLATFORM */
    }else{
        LOG_D("ykc monitor there is no fees type to be locked(padding billing rule)");
        return -0x03;
    }

    if(olen){
        *olen = total_len;
    }

    return 0x00;
#else
    return -0x01;
#endif /* #ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
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

    /** 0xFF为无效值 */
    if(fswitch->body.tplat_log != 0xFF){
        if(config == NULL){
            return -0x03;
        }
        config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG;
        if(fswitch->body.tplat_log){
            config->fswitch.tplat_log = NET_ENUM_FALSE;
        }else{
            config->fswitch.tplat_log = NET_ENUM_TRUE;
        }

        if(s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT) < 0x00){
            config->storage_init_flag = NET_YKC_MONITOR_STORAGE_INIT_FLAG - 0x01;
            return -0x04;
        }

        ykc_monitor_function_switch_set(config);
    }
    /** 0xFF为无效值 */
    if(fswitch->body.lock_module != 0xFF){
        s_ykc_monitor_lock_module.flag.is_wait_response = NET_ENUM_TRUE;
        s_ykc_monitor_lock_module.base_tick = rt_tick_get();
        if(fswitch->body.lock_module){
            thaisenSetEnableModuleState(0x00);
        }else{
            thaisenSetEnableModuleState(0x01);
        }
    }
    /** 除了0x01,其他值为无效值 */
    if(fswitch->body.clear_record == 0x01){
        if(SerialScreen_BtnClearAll() < 0x00){
            return -0x05;
        }
    }

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

/*********************************************************************************
 * 设备配置信息报文处理
 ********************************************************************************/
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
/********************************************************************
 * 函数名      ykc_monitor_is_config_data_valid
 * 功能          检查下发的配置数据是否有效(字符型数据全为空字符为无效；数值型数据全为0xFF为无效)
 * *****************************************************************/
static uint8_t ykc_monitor_is_config_data_valid(void *data, uint8_t dlen, uint8_t is_string)
{
    if(is_string){
        if(strlen((char*)data)){
            return 0x01;
        }
    }else{
        for(uint8_t i = 0x00; i < dlen; i++){
            if(*((uint8_t*)data + i) != 0xFF){
                return 0x01;
            }
        }
    }
    return 0x00;
}

/********************************************************************
 * 函数名      ykc_monitor_config_execute
 * 功能          执行配置操作
 * *****************************************************************/
static int32_t ykc_monitor_config_execute(uint8_t gunno, uint8_t page, void *data, void *sub_data, void *sub_sub_data)
{
    int32_t ret = thaisen_trigger_config_execute(gunno, page, data, sub_data, sub_sub_data);
    if(ret == THAISEN_CONFIG_SUCCESS){
        return NETYKCM_CONFIG_RES_SUCCESS;                               /** 配置成功 */
    }else if(ret == THAISEN_CONFIG_FAIL_STORAGR){
        return NETYKCM_CONFIG_RES_FAIL_STORAGE;                          /** 配置保存失败 */
    }else if(ret <= THAISEN_CONFIG_SYSTEM_ASSERT){
        return (NETYKCM_CONFIG_RES_EXTERN_INVOKE_ASSERT_BASE + (THAISEN_CONFIG_SYSTEM_ASSERT - ret));  /** 外部调用断言失败 */
    }else{
        return (NETYKCM_CONFIG_RES_ITEM_FAIL_BASE + (ret - THAISEN_CONFIG_FAIL_OFFSET));               /** 配置条目失败 */
    }
}

/** 系统信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_sys_info
 * 功能          处理服务器下发的系统信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_sys_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_sys_info))){
        LOG_E("ykcm input buf invalid with sys info|%d,%d", blen, sizeof(struct ykcm_sys_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_sys_info *response = (struct ykcm_sys_info*)buf;

        memset(response, 0x00, sizeof(struct ykcm_sys_info));

        response->allocate_way = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_ALLOCATION_WAY, 0x00));
        response->dev_function = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_DEVICE_TYPE, 0x00));
        response->terminal_addr[0x00] = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_TEMINAL_ADDRA, 0x00));
        response->terminal_addr[0x01] = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_TEMINAL_ADDRB, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_sys_info))){
            LOG_E("ykcm input data invalid with sys info|%d,%d", dlen, sizeof(struct ykcm_sys_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_sys_info *info = (struct ykcm_sys_info*)data;

        if(ykc_monitor_is_config_data_valid(&info->allocate_way, sizeof(info->allocate_way), 0x00) == NET_ENUM_FALSE){
            info->allocate_way = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_ALLOCATION_WAY, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->dev_function, sizeof(info->dev_function), 0x00) == NET_ENUM_FALSE){
            info->dev_function = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_DEVICE_TYPE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->terminal_addr[0x00], sizeof(info->terminal_addr[0x00]), 0x00) == NET_ENUM_FALSE){
            info->terminal_addr[0x00] = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_TEMINAL_ADDRA, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->terminal_addr[0x01], sizeof(info->terminal_addr[0x01]), 0x00) == NET_ENUM_FALSE){
            info->terminal_addr[0x01] = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_TEMINAL_ADDRB, 0x00));
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_SYSTEM_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** 桩信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_pile_info
 * 功能          处理服务器下发的桩信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_pile_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_pile_info))){
        LOG_E("ykcm input buf invalid with pile info|%d,%d", blen, sizeof(struct ykcm_pile_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    uint16_t valid_len = 0x00;
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        char *config_item = NULL;
        struct ykcm_pile_info *response = (struct ykcm_pile_info*)buf;

        memset(response, 0x00, sizeof(struct ykcm_pile_info));

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_QRCODE_PRE, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->qrcode_prefix) ? sizeof(response->qrcode_prefix) : valid_len;
        memcpy(response->qrcode_prefix, config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_QRCODE_SUF, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->qrcode_suffix) ? sizeof(response->qrcode_suffix) : valid_len;
        memcpy(response->qrcode_suffix, config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_HELP_PHONE, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->help_number) ? sizeof(response->help_number) : valid_len;
        memcpy(response->help_number, config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_SCREEN_PASSWORD, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->screen_password) ? sizeof(response->screen_password) : valid_len;
        memcpy(response->screen_password, config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_CARD_KEY, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->card_key) ? sizeof(response->card_key) : valid_len;
        memcpy(response->card_key, config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_REGISTER_CODE, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->register_code) ? sizeof(response->register_code) : valid_len;
        memcpy(response->register_code, config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_USER_IDENTITY, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->manufacturer_sn) ? sizeof(response->manufacturer_sn) : valid_len;
        memcpy(response->manufacturer_sn, config_item, valid_len);
#if 0
        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_SCREEN_PASSWORD, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->random_str) ? sizeof(response->random_str) : valid_len;
        memcpy(response->random_str, config_item, valid_len);
#endif
        response->cardnumber_block = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_CARD_BLOCK_SN, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_pile_info))){
            LOG_E("ykcm input data invalid with pile info|%d,%d", dlen, sizeof(struct ykcm_pile_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_PILE_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** 服务器信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_server_info
 * 功能          处理服务器下发的服务器信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_server_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
#define YKCM_NET_MODE_POSITION              0x02         /** 网络模式(配置条目)在结构体 thaisen_cfg_info_server 中的成员次序(从0开始)*/

    if((buf == NULL) || (blen < sizeof(struct ykcm_server_info))){
        LOG_E("ykcm input buf invalid with server info|%d,%d", blen, sizeof(struct ykcm_server_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    uint16_t valid_len = 0x00;

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        char *config_item = NULL;
        struct ykcm_server_info *response = (struct ykcm_server_info*)buf;

        memset(response, 0x00, sizeof(struct ykcm_server_info));

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_IP_DOMAIN, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > sizeof(response->domain) ? sizeof(response->domain) : valid_len;
        memcpy(response->domain, config_item, valid_len);

        response->port = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_PORT, 0x00));
        response->net_mode = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_NET_TYPE, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_server_info))){
            LOG_E("ykcm input data invalid with server info|%d,%d", dlen, sizeof(struct ykcm_server_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_server_info *info = (struct ykcm_server_info*)data;

        if(ykc_monitor_is_config_data_valid(&info->net_mode, sizeof(info->net_mode), 0x00) == NET_ENUM_FALSE){
            info->net_mode = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_NET_TYPE, 0x00));
        }
        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_SERVER_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** 电表信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_ammeter_info
 * 功能          处理服务器下发的电表信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_ammeter_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_ammeter_info))){
        LOG_E("ykcm input buf invalid with ammeter info|%d,%d", blen, sizeof(struct ykcm_ammeter_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    uint16_t valid_len = 0x00;

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        char *config_item = NULL;
        struct ykcm_ammeter_info *response = (struct ykcm_ammeter_info*)buf;

        memset(response, 0x00, sizeof(struct ykcm_ammeter_info));

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_METER_NOA, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > (sizeof(response->ammeter_addr[0x00]) - 0x01) ? (sizeof(response->ammeter_addr[0x00]) - 0x01) : valid_len;
        memcpy(response->ammeter_addr[0x00], config_item, valid_len);

        config_item = (char*)(sys_read_config_item_content(CONFIG_ITEM_METER_NOB, 0x00));
        valid_len = strlen(config_item);
        valid_len = valid_len > (sizeof(response->ammeter_addr[0x01]) - 0x01) ? (sizeof(response->ammeter_addr[0x01]) - 0x01) : valid_len;
        memcpy(response->ammeter_addr[0x01], config_item, valid_len);

        response->baudrate = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_METER_BAUDRATE, 0x00));
        response->check_way = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_METER_CHECK_WAY, 0x00));
        response->ammeter_model = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_METER_MODEL, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_ammeter_info))){
            LOG_E("ykcm input data invalid with ammeter info|%d,%d", dlen, sizeof(struct ykcm_ammeter_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_ammeter_info *info = (struct ykcm_ammeter_info*)data;

        if(ykc_monitor_is_config_data_valid(&info->ammeter_model, sizeof(info->ammeter_model), 0x00) == NET_ENUM_FALSE){
            info->ammeter_model = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_METER_MODEL, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->check_way, sizeof(info->check_way), 0x00) == NET_ENUM_FALSE){
            info->check_way = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_METER_CHECK_WAY, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->baudrate, sizeof(info->baudrate), 0x00) == NET_ENUM_FALSE){
            info->baudrate = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_METER_BAUDRATE, 0x00));
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_AMMETER_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** 模块信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_module_info
 * 功能          处理服务器下发的模块信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_module_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_module_info))){
        LOG_E("ykcm input buf invalid with module info|%d,%d", blen, sizeof(struct ykcm_module_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_module_info *response = (struct ykcm_module_info*)buf;

        memset(response, 0x00, sizeof(struct ykcm_module_info));

        response->module_protocol = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_MODEL, 0x00));
        response->module_group = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));
        response->module_num_single[0x00] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_1, 0x00));
        if(response->module_group > 0x01){
            response->module_num_single[0x01] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_2, 0x00));
        }
        if(response->module_group > 0x02){
            response->module_num_single[0x02] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_3, 0x00));
        }
        if(response->module_group > 0x03){
            response->module_num_single[0x03] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_4, 0x00));
        }
        response->lowpower_module = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_LP_MODULE, 0x00));
        response->module_rated_voltage = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0x00));
        response->module_rated_current = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0x00));
        response->pile_outvoltage_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, 0x00));
        response->pile_outvoltage_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0x00));
        response->pile_outcurrent_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0x00));
        response->pile_outcurrent_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MIN_LIMIT_CURRENT, 0x00));
        response->module_outcurrent_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_SMODULE_OUTCURR_MAX, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_module_info))){
            LOG_E("ykcm input data invalid with module info|%d,%d", dlen, sizeof(struct ykcm_module_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_module_info *info = (struct ykcm_module_info*)data;

        if(ykc_monitor_is_config_data_valid(&info->module_protocol, sizeof(info->module_protocol), 0x00) == NET_ENUM_FALSE){
            info->module_protocol = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_MODEL, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_group, sizeof(info->module_group), 0x00) == NET_ENUM_FALSE){
            info->module_group = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_num_single[0x00], sizeof(info->module_num_single[0x00]), 0x00) == NET_ENUM_FALSE){
            info->module_num_single[0x00] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_1, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_num_single[0x01], sizeof(info->module_num_single[0x01]), 0x00) == NET_ENUM_FALSE){
            info->module_num_single[0x01] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_2, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_num_single[0x02], sizeof(info->module_num_single[0x02]), 0x00) == NET_ENUM_FALSE){
            info->module_num_single[0x02] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_3, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_num_single[0x03], sizeof(info->module_num_single[0x03]), 0x00) == NET_ENUM_FALSE){
            info->module_num_single[0x03] = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODULE_NUM_GROUP_4, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_rated_voltage, sizeof(info->module_rated_voltage), 0x00) == NET_ENUM_FALSE){
            info->module_rated_voltage = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_rated_current, sizeof(info->module_rated_current), 0x00) == NET_ENUM_FALSE){
            info->module_rated_current = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->pile_outvoltage_max, sizeof(info->pile_outvoltage_max), 0x00) == NET_ENUM_FALSE){
            info->pile_outvoltage_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->pile_outvoltage_min, sizeof(info->pile_outvoltage_min), 0x00) == NET_ENUM_FALSE){
            info->pile_outvoltage_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->pile_outcurrent_max, sizeof(info->pile_outcurrent_max), 0x00) == NET_ENUM_FALSE){
            info->pile_outcurrent_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->pile_outcurrent_min, sizeof(info->pile_outcurrent_min), 0x00) == NET_ENUM_FALSE){
            info->pile_outcurrent_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_MIN_LIMIT_CURRENT, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->lowpower_module, sizeof(info->lowpower_module), 0x00) == NET_ENUM_FALSE){
            info->lowpower_module = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_LP_MODULE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->module_outcurrent_max, sizeof(info->module_outcurrent_max), 0x00) == NET_ENUM_FALSE){
            info->module_outcurrent_max = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SMODULE_OUTCURR_MAX, 0x00));
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_MODULE_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** VIN码信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_vin_info
 * 功能          处理服务器下发的VIN码信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_vin_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_vin_info))){
        LOG_E("ykcm input buf invalid with vin info|%d,%d", blen, sizeof(struct ykcm_vin_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        uint8_t *vin = (uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_VIN_WHITELIST, 0x00));
        struct ykcm_vin_info *response = (struct ykcm_vin_info*)buf;

        memset(response->vin_whitelist, 0x00, sizeof(response->vin_whitelist));

        for(uint8_t i = 0x00; i < NET_YKC_MONITOR_VIN_COUNT_MAX; i++){
            memcpy(response->vin_whitelist[i], vin, (sizeof(response->vin_whitelist[i]) - 0x01));
            if((i + 0x01) < NET_YKC_MONITOR_VIN_COUNT_MAX){
                vin += sizeof(response->vin_whitelist[i]);
            }
        }
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_vin_info))){
            LOG_E("ykcm input data invalid with vin info|%d,%d", dlen, sizeof(struct ykcm_vin_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        uint8_t count = NET_YKC_MONITOR_VIN_COUNT_MAX, valid_count = 0x00, i = 0x00;
        struct ykcm_vin_info *info = (struct ykcm_vin_info*)data;

        count = count > CP_INFO_VIN_WHITELIST_NUM_MAX ? CP_INFO_VIN_WHITELIST_NUM_MAX : count;
        /** VIN码有效性判断 */
        for(i = 0x00; i < count; i++){
            if(ykc_monitor_is_config_data_valid(info->vin_whitelist[i], 0x00, 0x01)){
                if(strlen((char*)data) != 0x11){     /** VIN码必须17位 */
                    break;
                }
                valid_count++;
            }
        }
        /** VIN码格式不对 */
        if(i < count){
            LOG_W("ykcm vin format error with config_info_process_vin_info");
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        if(valid_count){
            return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_VIN_INFO, data, &valid_count, NULL);
        }else{
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
        }
    }

    return 0x00;
}

/** 保护信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_protect_info
 * 功能          处理服务器下发的保护信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_protect_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_protect_info))){
        LOG_E("ykcm input buf invalid with protect info|%d,%d", blen, sizeof(struct ykcm_protect_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_protect_info *response = (struct ykcm_protect_info*)buf;

        memset(response, 0x00, sizeof(struct ykcm_protect_info));

        response->overtemp_alarm = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_WARN, 0x00));
        response->overtemp_stop = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_STOP, 0x00));
        response->overtemp_recovery = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_RECOVER, 0x00));
        response->overtemp_limitcur = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_SETCUR, 0x00));
        response->gunvolt_limit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUNVOLT_LIMIT, 0x00));
        response->soc_stop = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0x00));
        response->power_percent = sys_get_power_percent();
        response->eloss_proportion = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_ELOSS_PROPORTION, 0x00));
        response->cc1_12_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC112V_MAX, 0x00));
        response->cc1_12_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC112V_MIN, 0x00));
        response->cc1_6_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC16V_MAX, 0x00));
        response->cc1_6_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC16V_MIN, 0x00));
        response->cc1_4_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC14V_MAX, 0x00));
        response->cc1_4_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC14V_MIN, 0x00));
        response->out_overvolt = (*(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERVOL, 0x00)) /10);
        response->out_undervolt = (*(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_UNDERVOL, 0x00)) /10);
        response->in_overvolt = (*(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_OVERVOL, 0x00)) /10);
        response->in_undervolt = (*(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_UNDERVOL, 0x00)) /10);
        response->out_overcurr = (*(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERCUR, 0x00)) /10);
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_protect_info))){
            LOG_E("ykcm input data invalid with protect info|%d,%d", dlen, sizeof(struct ykcm_protect_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_protect_info *info = (struct ykcm_protect_info*)data;

        if(ykc_monitor_is_config_data_valid(&info->overtemp_alarm, sizeof(info->overtemp_alarm), 0x00) == NET_ENUM_FALSE){
            info->overtemp_alarm = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_WARN, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->overtemp_stop, sizeof(info->overtemp_stop), 0x00) == NET_ENUM_FALSE){
            info->overtemp_stop = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_STOP, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->overtemp_recovery, sizeof(info->overtemp_recovery), 0x00) == NET_ENUM_FALSE){
            info->overtemp_recovery = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_RECOVER, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->overtemp_limitcur, sizeof(info->overtemp_limitcur), 0x00) == NET_ENUM_FALSE){
            info->overtemp_limitcur = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_OVERTEMP_SETCUR, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->gunvolt_limit, sizeof(info->gunvolt_limit), 0x00) == NET_ENUM_FALSE){
            info->gunvolt_limit = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUNVOLT_LIMIT, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->soc_stop, sizeof(info->soc_stop), 0x00) == NET_ENUM_FALSE){
            info->soc_stop = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_SOC_STOP, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->power_percent, sizeof(info->power_percent), 0x00) == NET_ENUM_FALSE){
            info->power_percent = sys_get_power_percent();
        }
        if(ykc_monitor_is_config_data_valid(&info->eloss_proportion, sizeof(info->eloss_proportion), 0x00) == NET_ENUM_FALSE){
            info->eloss_proportion = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_ELOSS_PROPORTION, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->cc1_12_max, sizeof(info->cc1_12_max), 0x00) == NET_ENUM_FALSE){
            info->cc1_12_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC112V_MAX, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->cc1_12_min, sizeof(info->cc1_12_min), 0x00) == NET_ENUM_FALSE){
            info->cc1_12_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC112V_MIN, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->cc1_6_max, sizeof(info->cc1_6_max), 0x00) == NET_ENUM_FALSE){
            info->cc1_6_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC16V_MAX, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->cc1_6_min, sizeof(info->cc1_6_min), 0x00) == NET_ENUM_FALSE){
            info->cc1_6_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC16V_MIN, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->cc1_4_max, sizeof(info->cc1_4_max), 0x00) == NET_ENUM_FALSE){
            info->cc1_4_max = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC14V_MAX, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->cc1_4_min, sizeof(info->cc1_4_min), 0x00) == NET_ENUM_FALSE){
            info->cc1_4_min = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_GUN1_CC14V_MIN, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->out_overvolt, sizeof(info->out_overvolt), 0x00) == NET_ENUM_FALSE){
            info->out_overvolt = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERVOL, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->out_undervolt, sizeof(info->out_undervolt), 0x00) == NET_ENUM_FALSE){
            info->out_undervolt = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_UNDERVOL, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->in_overvolt, sizeof(info->in_overvolt), 0x00) == NET_ENUM_FALSE){
            info->in_overvolt = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_OVERVOL, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->in_undervolt, sizeof(info->in_undervolt), 0x00) == NET_ENUM_FALSE){
            info->in_undervolt = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_UNDERVOL, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->out_overcurr, sizeof(info->out_overcurr), 0x00) == NET_ENUM_FALSE){
            info->out_overcurr = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERCUR, 0x00));
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_PROTECT_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** 功能配置信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_function_config_info
 * 功能          处理服务器下发的功能配置信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_function_config_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_function_config))){
        LOG_E("ykcm input buf invalid with function config info|%d,%d", blen, sizeof(struct ykcm_function_config));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_function_config *response = (struct ykcm_function_config*)buf;

        memset(response, 0x00, sizeof(struct ykcm_function_config));

        response->insult_detect = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_INSULATION, 0x00));
        response->card_reader = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0x00));
        response->parallel_charge = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PARALLEL, 0x00));
        response->vin_charge = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_VIN, 0x00));
        response->parallel_relay = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PARALLELRELAY, 0x00));
        response->module_silence = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_MODULE_SLIENCE, 0x00));
        response->plug_charge = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PLUGCHARGE, 0x00));
        response->local_start = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_LOCAL, 0x00));
        response->local_stop = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_LOCAL_STOP, 0x00));
        response->auxpower_24V = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_AUXPOWER24V, 0x00));
        response->offline_billing = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, 0x00));
        response->password_start = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PASSWORD_START, 0x00));
        response->mode_select = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_MODE_SELECT, 0x00));
        response->offline_card = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_OFFLINE_CARD, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_function_config))){
            LOG_E("ykcm input data invalid with function config info|%d,%d", dlen, sizeof(struct ykcm_function_config));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_FUNCTION_INFO, data, NULL, NULL);
    }

    return 0x00;
}

/** 离线计费信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_offline_billing_info
 * 功能          处理服务器下发的离线计费信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_offline_billing_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
#ifdef APP_USING_OFFLINE_BILLING
    if((buf == NULL) || (blen < sizeof(struct ykcm_offline_billing))){
        LOG_E("ykcm input buf invalid with offline billing info|%d,%d", blen, sizeof(struct ykcm_offline_billing));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    struct sys_billing_rule *rule = (struct sys_billing_rule*)(sys_read_config_item_content(CONFIG_ITEM_BILLING_RULE, 0x00));

    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_offline_billing *response = (struct ykcm_offline_billing*)buf;

        memset(response, 0x00, sizeof(struct ykcm_offline_billing));

        response->service_price = rule->rate_service_price[0x00];
        /** 尖尖 */
        response->sharp_sharp_price = rule->rate_elect_price[CP_RATED_TYPE_SHARP_SHARP];
        memcpy(&response->sstime1, &rule->time[CP_RATED_TYPE_SHARP_SHARP][0x00], sizeof(response->sstime1));
        memcpy(&response->sstime2, &rule->time[CP_RATED_TYPE_SHARP_SHARP][0x01], sizeof(response->sstime2));
        /** 尖 */
        response->sharp_price = rule->rate_elect_price[CP_RATED_TYPE_SHARP];
        memcpy(&response->stime1, &rule->time[CP_RATED_TYPE_SHARP][0x00], sizeof(response->stime1));
        memcpy(&response->stime2, &rule->time[CP_RATED_TYPE_SHARP][0x01], sizeof(response->stime2));
        /** 峰 */
        response->peak_price = rule->rate_elect_price[CP_RATED_TYPE_PEAK];
        memcpy(&response->ptime1, &rule->time[CP_RATED_TYPE_PEAK][0x00], sizeof(response->ptime1));
        memcpy(&response->ptime2, &rule->time[CP_RATED_TYPE_PEAK][0x01], sizeof(response->ptime2));
        /** 平 */
        response->flat_price = rule->rate_elect_price[CP_RATED_TYPE_FLAT];
        memcpy(&response->ftime1, &rule->time[CP_RATED_TYPE_FLAT][0x00], sizeof(response->ftime1));
        memcpy(&response->ftime2, &rule->time[CP_RATED_TYPE_FLAT][0x01], sizeof(response->ftime2));
        /** 谷 */
        response->valley_price = rule->rate_elect_price[CP_RATED_TYPE_VALLEY];
        memcpy(&response->vtime1, &rule->time[CP_RATED_TYPE_VALLEY][0x00], sizeof(response->vtime1));
        memcpy(&response->vtime2, &rule->time[CP_RATED_TYPE_VALLEY][0x01], sizeof(response->vtime2));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_offline_billing))){
            LOG_E("ykcm input data invalid with offline billing info|%d,%d", dlen, sizeof(struct ykcm_offline_billing));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        System_BaseData *base = NULL;
        struct ykcm_offline_billing *info = (struct ykcm_offline_billing*)data;

        /************* 充电时不能修改费率 ************/
        for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(i));
            if((base->state.current >= APP_OFSM_STATE_STARTING) && (base->state.current <= APP_OFSM_STATE_STOPING)){
                LOG_W("ykcm gunno(%d) is charging, not allow modify billing rule", i);
                return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
            }
        }
        /** 服务费 */
        if(ykc_monitor_is_config_data_valid(&info->service_price, sizeof(info->service_price), 0x00) == NET_ENUM_FALSE){
            info->service_price = rule->rate_service_price[0x00];
        }
        /** 尖尖电费 */
        if(ykc_monitor_is_config_data_valid(&info->sharp_sharp_price, sizeof(info->sharp_sharp_price), 0x00) == NET_ENUM_FALSE){
            info->sharp_sharp_price = rule->rate_elect_price[CP_RATED_TYPE_SHARP_SHARP];
        }
        /** 尖电费 */
        if(ykc_monitor_is_config_data_valid(&info->sharp_price, sizeof(info->sharp_price), 0x00) == NET_ENUM_FALSE){
            info->sharp_price = rule->rate_elect_price[CP_RATED_TYPE_SHARP];
        }
        /** 峰电费 */
        if(ykc_monitor_is_config_data_valid(&info->peak_price, sizeof(info->peak_price), 0x00) == NET_ENUM_FALSE){
            info->peak_price = rule->rate_elect_price[CP_RATED_TYPE_PEAK];
        }
        /** 平电费 */
        if(ykc_monitor_is_config_data_valid(&info->flat_price, sizeof(info->flat_price), 0x00) == NET_ENUM_FALSE){
            info->flat_price = rule->rate_elect_price[CP_RATED_TYPE_FLAT];
        }
        /** 谷电费 */
        if(ykc_monitor_is_config_data_valid(&info->valley_price, sizeof(info->valley_price), 0x00) == NET_ENUM_FALSE){
            info->valley_price = rule->rate_elect_price[CP_RATED_TYPE_VALLEY];
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_OFFLINE_BILLING_INFO, data, NULL, NULL);
    }

    return 0x00;
#else
    return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x04);
#endif /* APP_USING_OFFLINE_BILLING */
}

/************************************* 7103/7101 *********************************************/
/** 输入信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_input_7103_7101_info
 * 功能          处理服务器下发的输入信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_input_7103_7101_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_input_info_7103_7101))){
        LOG_E("ykcm input buf invalid with input 7103/7101 info|%d,%d", blen, sizeof(struct ykcm_input_info_7103_7101));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_input_info_7103_7101 *response = (struct ykcm_input_info_7103_7101*)buf;

        memset(response, 0x00, sizeof(struct ykcm_input_info_7103_7101));

        response->scram.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_SCRAM, 0x00));
        response->scram.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_SCRAM, 0x00));

        response->door.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_GATE, 0x00));
        response->door.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_GATE, 0x00));

        response->acrelay.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_ACRELAY, 0x00));
        response->acrelay.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_ACRELAY, 0x00));

        response->dcrelay.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_DCRELAY, 0x00));
        response->dcrelay.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_DCRELAY, 0x00));

        response->fan.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FAN, 0x00));
        response->fan.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FAN, 0x00));

        response->elock.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_ELOCK, 0x00));
        response->elock.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_ELOCK, 0x00));

        response->tempprotect.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPRO, 0x00));
        response->tempprotect.reversal = 0x00;

        response->pour.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_POUR, 0x00));
        response->pour.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_POUR, 0x00));

        response->protect_light.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_PROTECT_LIGHT, 0x00));
        response->protect_light.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_PROTECT_LIGHT, 0x00));

        response->flood.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FLOOD, 0x00));
        response->flood.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FLOOD, 0x00));

        response->smoke.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_SMOKE, 0x00));
        response->smoke.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_SMOKE, 0x00));

        response->gunsite.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_GUNSITE, 0x00));
        response->gunsite.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_GUNSITE, 0x00));

        response->fuse.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FUSE, 0x00));
        response->fuse.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FUSE, 0x00));

        response->liquid.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_LIQUID, 0x00));
        response->liquid.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_LIQUID, 0x00));

        response->circuit_breaker.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_CIRCUIT_BREAKER, 0x00));
        response->circuit_breaker.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_CIRCUIT_BREAKER, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_input_info_7103_7101))){
            LOG_E("ykcm input data invalid with input 7103/7101 info|%d,%d", dlen, sizeof(struct ykcm_input_info_7103_7101));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_INPUT_7103_7101_INFO, data, NULL, NULL);
    }

    return 0x00;
}


/************************************* 7104 *********************************************/
/** 通用输入信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_public_input_7104_info
 * 功能          处理服务器下发的通用输入信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_public_input_7104_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
/******************************* 这是7104的配置 **********************************/
#if 0
    if((buf == NULL) || (blen < sizeof(struct ykcm_public_input_info_7104))){
        LOG_E("ykcm input buf invalid with public input 7104 info|%d,%d", blen, sizeof(struct ykcm_public_input_info_7104));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_public_input_info_7104 *response = (struct ykcm_public_input_info_7104*)buf;

        memset(response, 0x00, sizeof(struct ykcm_public_input_info_7104));

        response->protectlight.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PROTECTLIGHT, 0x00));
        response->protectlight.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_PROTECTLIGHT, 0x00));
        response->protectlight.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_PROTECTLIGHT, 0x00));

        response->parallel_relay1.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PARALLEL1, 0x00));
        response->parallel_relay1.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_PRARALLEL1, 0x00));
        response->parallel_relay1.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_PARALLEL1, 0x00));

        response->parallel_relay2.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PARALLEL2, 0x00));
        response->parallel_relay2.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_PRARALLEL2, 0x00));
        response->parallel_relay2.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_PARALLEL2, 0x00));

        response->parallel_relay3.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PARALLEL3, 0x00));
        response->parallel_relay3.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_PRARALLEL3, 0x00));
        response->parallel_relay3.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_PARALLEL3, 0x00));

        response->scram.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_SCRAM, 0x00));
        response->scram.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_SCRAM, 0x00));
        response->scram.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_SCRAM, 0x00));

        response->breaker.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_BREAKERS, 0x00));
        response->breaker.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_BREAKERS, 0x00));
        response->breaker.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_BREAKERS, 0x00));

        response->acrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_AC, 0x00));
        response->acrelay.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_ACRELAY, 0x00));
        response->acrelay.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_ACRELAY, 0x00));

        response->fan.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FAN, 0x00));
        response->fan.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FAN, 0x00));
        response->fan.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FAN, 0x00));

        response->flooding.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_WATER, 0x00));
        response->flooding.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_WATER, 0x00));
        response->flooding.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_WATER, 0x00));

        response->door.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_GATE, 0x00));
        response->door.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_GATE, 0x00));
        response->door.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_GATE, 0x00));

        response->smoke.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_SMOKE, 0x00));
        response->smoke.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_SMOKE, 0x00));
        response->smoke.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_SMOKE, 0x00));

        response->fall.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FALL, 0x00));
        response->fall.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FALL, 0x00));
        response->fall.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FALL, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_public_input_info_7104))){
            LOG_E("ykcm input data invalid with public input 7104 info|%d,%d", dlen, sizeof(struct ykcm_public_input_info_7104));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_public_input_info_7104 *info = (struct ykcm_public_input_info_7104*)data;

        if(ykc_monitor_is_config_data_valid(&info->protectlight.port_number, sizeof(info->protectlight.port_number), 0x00) == NET_ENUM_FALSE){
            info->protectlight.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PROTECTLIGHT, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->parallel_relay1.port_number, sizeof(info->parallel_relay1.port_number), 0x00) == NET_ENUM_FALSE){
            info->parallel_relay1.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PARALLEL1, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->parallel_relay2.port_number, sizeof(info->parallel_relay2.port_number), 0x00) == NET_ENUM_FALSE){
            info->parallel_relay2.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PARALLEL2, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->parallel_relay3.port_number, sizeof(info->parallel_relay3.port_number), 0x00) == NET_ENUM_FALSE){
            info->parallel_relay3.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_PARALLEL3, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->scram.port_number, sizeof(info->scram.port_number), 0x00) == NET_ENUM_FALSE){
            info->scram.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_SCRAM, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->breaker.port_number, sizeof(info->breaker.port_number), 0x00) == NET_ENUM_FALSE){
            info->breaker.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_BREAKERS, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->acrelay.port_number, sizeof(info->acrelay.port_number), 0x00) == NET_ENUM_FALSE){
            info->acrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_AC, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->fan.port_number, sizeof(info->fan.port_number), 0x00) == NET_ENUM_FALSE){
            info->fan.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FAN, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->flooding.port_number, sizeof(info->flooding.port_number), 0x00) == NET_ENUM_FALSE){
            info->flooding.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_WATER, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->door.port_number, sizeof(info->door.port_number), 0x00) == NET_ENUM_FALSE){
            info->door.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_GATE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->smoke.port_number, sizeof(info->smoke.port_number), 0x00) == NET_ENUM_FALSE){
            info->smoke.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_SMOKE, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->fall.port_number, sizeof(info->fall.port_number), 0x00) == NET_ENUM_FALSE){
            info->fall.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FALL, 0x00));
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_PUBLIC_INPUT_7104_INFO, data, NULL, NULL);
    }

    return 0x00;
#else
    return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
#endif
}

/** 枪输入信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_gun_input_7104_info
 * 功能          处理服务器下发的枪输入信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_gun_input_7104_info(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
/******************************* 这是7104的配置 **********************************/
#if 0
    if((buf == NULL) || (blen < sizeof(struct ykcm_gun_input_info_7104))){
        LOG_E("ykcm input buf invalid with gun input 7104 info|%d,%d", blen, sizeof(struct ykcm_gun_input_info_7104));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_gun_input_info_7104 *response = (struct ykcm_gun_input_info_7104*)buf;

        memset(response, 0x00, sizeof(struct ykcm_gun_input_info_7104));

        if(gunno == 0x01){
            response->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_DCA, 0x00));
            response->dcrelay.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_DCRELAYA, 0x00));
            response->dcrelay.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_DCRELAYA, 0x00));

            response->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_ELOCKA, 0x00));
            response->elock.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_ELOCKA, 0x00));
            response->elock.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_ELOCKA, 0x00));

            response->gunsite.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_GUNSITEA, 0x00));
            response->gunsite.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_GUNSITEA, 0x00));
            response->gunsite.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_GUNSITEA, 0x00));

            response->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_LIQIDA, 0x00));
            response->liquid.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_LIQIDA, 0x00));
            response->liquid.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_LIQIDA, 0x00));

            response->fuse.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FUSEA, 0x00));
            response->fuse.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FUSEA, 0x00));
            response->fuse.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FUSEA, 0x00));

            response->temp_detect.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPROA, 0x00));
            response->temp_detect.state.enable = 0x00;
            response->temp_detect.state.reversal = 0x00;
        }else if(gunno == 0x02){
            response->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_DCB, 0x00));
            response->dcrelay.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_DCRELAYB, 0x00));
            response->dcrelay.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_DCRELAYB, 0x00));

            response->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_ELOCKB, 0x00));
            response->elock.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_ELOCKB, 0x00));
            response->elock.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_ELOCKB, 0x00));

            response->gunsite.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_GUNSITEB, 0x00));
            response->gunsite.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_GUNSITEB, 0x00));
            response->gunsite.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_GUNSITEB, 0x00));

            response->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_LIQIDB, 0x00));
            response->liquid.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_LIQIDB, 0x00));
            response->liquid.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_LIQIDB, 0x00));

            response->fuse.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FUSEB, 0x00));
            response->fuse.state.enable = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_FUSEB, 0x00));
            response->fuse.state.reversal = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INNEG_FUSEB, 0x00));

            response->temp_detect.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPROB, 0x00));
            response->temp_detect.state.enable = 0x00;
            response->temp_detect.state.reversal = 0x00;
        }else{
            LOG_W("ykcm config set input gun port gunno error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_gun_input_info_7104))){
            LOG_E("ykcm input data invalid with gun input 7104 info|%d,%d", dlen, sizeof(struct ykcm_gun_input_info_7104));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        struct ykcm_gun_input_info_7104 *info = (struct ykcm_gun_input_info_7104*)data;

        if(gunno == 0x01){
            if(ykc_monitor_is_config_data_valid(&info->dcrelay.port_number, sizeof(info->dcrelay.port_number), 0x00) == NET_ENUM_FALSE){
                info->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_DCA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->elock.port_number, sizeof(info->elock.port_number), 0x00) == NET_ENUM_FALSE){
                info->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_ELOCKA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->gunsite.port_number, sizeof(info->gunsite.port_number), 0x00) == NET_ENUM_FALSE){
                info->gunsite.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_GUNSITEA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->liquid.port_number, sizeof(info->liquid.port_number), 0x00) == NET_ENUM_FALSE){
                info->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_LIQIDA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->fuse.port_number, sizeof(info->fuse.port_number), 0x00) == NET_ENUM_FALSE){
                info->fuse.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FUSEA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->temp_detect.port_number, sizeof(info->temp_detect.port_number), 0x00) == NET_ENUM_FALSE){
                info->temp_detect.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPROA, 0x00));
            }
        }else if(gunno == 0x02){
            if(ykc_monitor_is_config_data_valid(&info->dcrelay.port_number, sizeof(info->dcrelay.port_number), 0x00) == NET_ENUM_FALSE){
                info->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_DCB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->elock.port_number, sizeof(info->elock.port_number), 0x00) == NET_ENUM_FALSE){
                info->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_ELOCKB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->gunsite.port_number, sizeof(info->gunsite.port_number), 0x00) == NET_ENUM_FALSE){
                info->gunsite.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_GUNSITEB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->liquid.port_number, sizeof(info->liquid.port_number), 0x00) == NET_ENUM_FALSE){
                info->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_LIQIDB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->fuse.port_number, sizeof(info->fuse.port_number), 0x00) == NET_ENUM_FALSE){
                info->fuse.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INPUT_FUSEB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->temp_detect.port_number, sizeof(info->temp_detect.port_number), 0x00) == NET_ENUM_FALSE){
                info->temp_detect.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_INEN_TEMPPROB, 0x00));
            }
        }else{
            LOG_W("ykcm config set input gun port gunno error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
        }

        return ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_GUN_INPUT_7104_INFO, data, NULL, NULL);
    }

    return 0x00;
#else
    return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x05);
#endif
}

/** 通用输出信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_public_output_7104_info
 * 功能          处理服务器下发的通用输出信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_public_output_7104_info(uint8_t option, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
/******************************* 这是7104的配置 **********************************/
#if 0
    if((buf == NULL) || (blen < sizeof(struct ykcm_public_output_info_7104))){
        LOG_E("ykcm input buf invalid with public output 7104 info|%d,%d", blen, sizeof(struct ykcm_public_output_info_7104));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_public_output_info_7104 *response = (struct ykcm_public_output_info_7104*)buf;

        memset(response, 0x00, sizeof(struct ykcm_public_output_info_7104));

        response->fan.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_FAN, 0x00));
        response->fan.enable = 0x01;

        response->parallel_relay1.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_PARALLEL1, 0x00));
        response->parallel_relay1.enable = 0x01;

        response->parallel_relay2.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_PARALLEL2, 0x00));
        response->parallel_relay2.enable = 0x01;

        response->parallel_relay3.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_PARALLEL3, 0x00));
        response->parallel_relay3.enable = 0x01;

        response->acrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AC, 0x00));
        response->acrelay.enable = 0x01;
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_public_output_info_7104))){
            LOG_E("ykcm input data invalid with public output 7104 info|%d,%d", dlen, sizeof(struct ykcm_public_output_info_7104));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
        struct ykcm_public_output_info_7104 *info = (struct ykcm_public_output_info_7104*)data;

        if(ykc_monitor_is_config_data_valid(&info->fan.port_number, sizeof(info->fan.port_number), 0x00) == NET_ENUM_FALSE){
            info->fan.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_FAN, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->parallel_relay1.port_number, sizeof(info->parallel_relay1.port_number), 0x00) == NET_ENUM_FALSE){
            info->parallel_relay1.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_PARALLEL1, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->parallel_relay2.port_number, sizeof(info->parallel_relay2.port_number), 0x00) == NET_ENUM_FALSE){
            info->parallel_relay2.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_PARALLEL2, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->parallel_relay3.port_number, sizeof(info->parallel_relay3.port_number), 0x00) == NET_ENUM_FALSE){
            info->parallel_relay3.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_PARALLEL3, 0x00));
        }
        if(ykc_monitor_is_config_data_valid(&info->acrelay.port_number, sizeof(info->acrelay.port_number), 0x00) == NET_ENUM_FALSE){
            info->acrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AC, 0x00));
        }

        return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_PUBLIC_OUTPUT_7104_INFO, data, NULL, NULL);
    }

    return 0x00;
#else
    return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
#endif
}

/** 枪输出信息 */
/*************************************************
 * 函数名      ykc_monitor_config_info_process_gun_output_7104_info
 * 功能          处理服务器下发的枪输出信息配置修改、查询请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_process_gun_output_7104_info(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
/******************************* 这是7104的配置 **********************************/
#if 0
    if((buf == NULL) || (blen < sizeof(struct ykcm_gun_output_info_7104))){
        LOG_E("ykcm input buf invalid with gun output 7104 info|%d,%d", blen, sizeof(struct ykcm_gun_output_info_7104));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_gun_output_info_7104 *response = (struct ykcm_gun_output_info_7104*)buf;

        memset(response, 0x00, sizeof(struct ykcm_gun_output_info_7104));

        if(gunno == 0x01){
            response->auxpower_24V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX24A, 0x00));
            response->auxpower_24V.enable = 0x01;

            response->auxpower_12V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX12A, 0x00));
            response->auxpower_12V.enable = 0x01;

            response->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_DCA, 0x00));
            response->dcrelay.enable = 0x01;

            response->relief.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_RELIEFA, 0x00));
            response->relief.enable = 0x01;

            response->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_ELOCKA, 0x00));
            response->elock.enable = 0x01;

            response->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_LIQIDA, 0x00));
            response->liquid.enable = 0x01;
        }else if(gunno == 0x02){
            response->auxpower_24V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX24B, 0x00));
            response->auxpower_24V.enable = 0x01;

            response->auxpower_12V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX12B, 0x00));
            response->auxpower_12V.enable = 0x01;

            response->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_DCB, 0x00));
            response->dcrelay.enable = 0x01;

            response->relief.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_RELIEFB, 0x00));
            response->relief.enable = 0x01;

            response->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_ELOCKB, 0x00));
            response->elock.enable = 0x01;

            response->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_LIQIDB, 0x00));
            response->liquid.enable = 0x01;
        }else{
            LOG_W("ykcm config query output gun port gunno error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_gun_output_info_7104))){
            LOG_E("ykcm input data invalid with gun output 7104 info|%d,%d", dlen, sizeof(struct ykcm_gun_output_info_7104));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        struct ykcm_gun_output_info_7104 *info = (struct ykcm_gun_output_info_7104*)data;

        if(gunno == 0x01){
            if(ykc_monitor_is_config_data_valid(&info->auxpower_24V.port_number, sizeof(info->auxpower_24V.port_number), 0x00) == NET_ENUM_FALSE){
                info->auxpower_24V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX24A, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->auxpower_12V.port_number, sizeof(info->auxpower_12V.port_number), 0x00) == NET_ENUM_FALSE){
                info->auxpower_12V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX12A, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->dcrelay.port_number, sizeof(info->dcrelay.port_number), 0x00) == NET_ENUM_FALSE){
                info->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_DCA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->relief.port_number, sizeof(info->relief.port_number), 0x00) == NET_ENUM_FALSE){
                info->relief.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_RELIEFA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->elock.port_number, sizeof(info->elock.port_number), 0x00) == NET_ENUM_FALSE){
                info->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_ELOCKA, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->liquid.port_number, sizeof(info->liquid.port_number), 0x00) == NET_ENUM_FALSE){
                info->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_LIQIDA, 0x00));
            }
        }else if(gunno == 0x02){
            if(ykc_monitor_is_config_data_valid(&info->auxpower_24V.port_number, sizeof(info->auxpower_24V.port_number), 0x00) == NET_ENUM_FALSE){
                info->auxpower_24V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX24B, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->auxpower_12V.port_number, sizeof(info->auxpower_12V.port_number), 0x00) == NET_ENUM_FALSE){
                info->auxpower_12V.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_AUX12B, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->dcrelay.port_number, sizeof(info->dcrelay.port_number), 0x00) == NET_ENUM_FALSE){
                info->dcrelay.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_DCB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->relief.port_number, sizeof(info->relief.port_number), 0x00) == NET_ENUM_FALSE){
                info->relief.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_RELIEFB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->elock.port_number, sizeof(info->elock.port_number), 0x00) == NET_ENUM_FALSE){
                info->elock.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_ELOCKB, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->liquid.port_number, sizeof(info->liquid.port_number), 0x00) == NET_ENUM_FALSE){
                info->liquid.port_number = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_LIQIDB, 0x00));
            }
        }else{
            LOG_W("ykcm config set output gun port gunno error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
        }

        return ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_GUN_OUTPUT_7104_INFO, data, NULL, NULL);
    }

    return 0x00;
#else
    return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x04);
#endif
}

/** 模式选择：正常模式 */
/*************************************************
 * 函数名      ykc_monitor_config_info_mode_select_normal
 * 功能          处理服务器下发的正常模式配置请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_mode_select_normal(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_mode_select_normal))){
        LOG_E("ykcm input buf invalid with mode select normal|%d,%d", blen, sizeof(struct ykcm_mode_select_normal));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_mode_select_normal *response = (struct ykcm_mode_select_normal*)buf;

        memset(response, 0x00, sizeof(struct ykcm_mode_select_normal));

        if(gunno == 0x01){
            response->mode = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_CURRENT_MODE_A, 0x00));
            response->mode_parameter = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_MODE_PARAMETER_A, 0x00));
        }else if(gunno == 0x02){
            response->mode = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_CURRENT_MODE_B, 0x00));
            response->mode_parameter = *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_MODE_PARAMETER_B, 0x00));
        }else{
            LOG_W("ykcm config query mode select normal error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_mode_select_normal))){
            LOG_E("ykcm input data invalid with mode select normal|%d,%d", dlen, sizeof(struct ykcm_mode_select_normal));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        struct ykcm_mode_select_normal *info = (struct ykcm_mode_select_normal*)data;

        if(gunno == 0x01){
            if(ykc_monitor_is_config_data_valid(&info->mode, sizeof(info->mode), 0x00) == NET_ENUM_FALSE){
                info->mode = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_CURRENT_MODE_A, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->mode_parameter, sizeof(info->mode_parameter), 0x00) == NET_ENUM_FALSE){
                info->mode_parameter = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODE_PARAMETER_A, 0x00));
            }
        }else if(gunno == 0x02){
            if(ykc_monitor_is_config_data_valid(&info->mode, sizeof(info->mode), 0x00) == NET_ENUM_FALSE){
                info->mode = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_CURRENT_MODE_B, 0x00));
            }
            if(ykc_monitor_is_config_data_valid(&info->mode_parameter, sizeof(info->mode_parameter), 0x00) == NET_ENUM_FALSE){
                info->mode_parameter = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_MODE_PARAMETER_B, 0x00));
            }
        }else{
            LOG_W("ykcm config set mode select normal error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
        }

        return ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_MODE_SELECT_NORMAL, data, NULL, NULL);
    }

    return 0x00;
}

/** 模式选择：V2G模式 */
/*************************************************
 * 函数名      ykc_monitor_config_info_mode_select_v2g
 * 功能          处理服务器下发的V2G模式配置请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_mode_select_v2g(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_mode_select_v2g))){
        LOG_E("ykcm input buf invalid with mode select v2g|%d,%d", blen, sizeof(struct ykcm_mode_select_v2g));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_mode_select_v2g *response = (struct ykcm_mode_select_v2g*)buf;

        memset(response, 0x00, sizeof(struct ykcm_mode_select_v2g));

        if(gunno == 0x01){
            response->mode = 0x00;
        }else if(gunno == 0x02){
            response->mode = 0x00;
        }else{
            LOG_W("ykcm config query mode select v2g error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x01);
        }
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_mode_select_v2g))){
            LOG_E("ykcm input data invalid with mode select v2g|%d,%d", dlen, sizeof(struct ykcm_mode_select_v2g));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        struct ykcm_mode_select_v2g *info = (struct ykcm_mode_select_v2g*)data;

        if(gunno == 0x01){
            if(ykc_monitor_is_config_data_valid(&info->mode, sizeof(info->mode), 0x00) == NET_ENUM_FALSE){
                info->mode = 0x00;
            }
        }else if(gunno == 0x02){
            if(ykc_monitor_is_config_data_valid(&info->mode, sizeof(info->mode), 0x00) == NET_ENUM_FALSE){
                info->mode = 0x00;
            }
        }else{
            LOG_W("ykcm config set mode select v2g error(%d)", gunno);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x03);
        }

//        return ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_MODE_SELECT_V2G, data, NULL, NULL);
    }

    return 0x00;
}

/** 其它配置 */
/*************************************************
 * 函数名      ykc_monitor_config_info_other
 * 功能          处理服务器下发的其它配置请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_other(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_other_config))){
        LOG_E("ykcm input buf invalid with other config|%d,%d", blen, sizeof(struct ykcm_other_config));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        uint8_t *_config_data = NULL, i = 0x00;
        struct ykcm_other_config *response = (struct ykcm_other_config*)buf;

        memset(response, 0x00, sizeof(struct ykcm_other_config));
#if 0
        _config_data = sys_read_config_item_content(CONFIG_ITEM_LIQUID_ENABLE, 0x00);
        if(_config_data){
            for(i = 0x00; i < (sizeof(response->liquid) /sizeof(response->liquid[0x00])); i++){
                if(_config_data[i]){
                    response->liquid[i].used = 0x01;
                }
            }
        }
        _config_data = sys_read_config_item_content(CONFIG_ITEM_LIQUID_DETECT, 0x00);
        if(_config_data){
            for(i = 0x00; i < (sizeof(response->liquid) /sizeof(response->liquid[0x00])); i++){
                if(_config_data[i]){
                    response->liquid[i].fdetect = 0x01;
                }
            }
        }
        _config_data = sys_read_config_item_content(CONFIG_ITEM_LIQUID_ADDR, 0x00);
        if(_config_data){
            for(i = 0x00; i < (sizeof(response->liquid) /sizeof(response->liquid[0x00])); i++){
                response->liquid[i].address = _config_data[i];
            }
        }
#endif
        response->fan_work_time = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_FAN_WORK_TIME, 0x00));
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_other_config))){
            LOG_E("ykcm input data invalid with other config|%d,%d", dlen, sizeof(struct ykcm_other_config));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        uint8_t *_config_data = NULL, i = 0x00;
        struct ykcm_other_config *info = (struct ykcm_other_config*)data;
#if 0
        _config_data = sys_read_config_item_content(CONFIG_ITEM_LIQUID_ADDR, 0x00);
        for(i = 0x00; i < (sizeof(info->liquid) /sizeof(info->liquid[0x00])); i++){
            if(ykc_monitor_is_config_data_valid(&info->liquid[i].address, sizeof(info->liquid[i].address), 0x00) == NET_ENUM_FALSE){
                if(_config_data){
                    info->liquid[i].address = _config_data[i];
                }
            }
        }
#endif
        if(ykc_monitor_is_config_data_valid(&info->fan_work_time, sizeof(info->fan_work_time), 0x00) == NET_ENUM_FALSE){
            info->fan_work_time = *(uint16_t*)(sys_read_config_item_content(CONFIG_ITEM_FAN_WORK_TIME, 0x00));
        }
        /** 针对所有枪 */
        if(gunno == 0xFF){
            return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_OTHER_CONFIG, data, NULL, NULL);
        }else{
            return ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_OTHER_CONFIG, data, NULL, NULL);
        }
    }

    return 0x00;
}

/** 固定类型指令信息配置 */
/*************************************************
 * 函数名      ykc_monitor_config_info_fixed_cmd
 * 功能          处理服务器下发的固定类型指令信息请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_fixed_cmd(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_fixed_cmd_info))){
        LOG_E("ykcm input buf invalid with fixed cmd config|%d,%d", blen, sizeof(struct ykcm_fixed_cmd_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        uint8_t data = 0x00;
        struct ykcm_fixed_cmd_info *response = (struct ykcm_fixed_cmd_info*)buf;

        /** 读的时候不限制版本信息 */
        memset(response, 0x00, sizeof(struct ykcm_fixed_cmd_info));
        response->msg_version = YKC_MONITOR_FIXED_CMD_MSG_VER;

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_BATVOLT_DETECT, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.batvolt_detect = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.batvolt_detect = NET_ENUM_FALSE;
        }else{
            response->info.batvolt_detect = NET_ENUM_TRUE;
        }

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_BCLTIMOUT_DETECT, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.bcltimeout_detect = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.bcltimeout_detect = NET_ENUM_FALSE;
        }else{
            response->info.bcltimeout_detect = NET_ENUM_TRUE;
        }

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_FAST_PROTOCOL, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.fast_protocol = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.fast_protocol = NET_ENUM_FALSE;
        }else{
            response->info.fast_protocol = NET_ENUM_TRUE;
        }

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_YT_PROTOCOL, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.cfc_protocol = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.cfc_protocol = NET_ENUM_FALSE;
        }else{
            response->info.cfc_protocol = NET_ENUM_TRUE;
        }

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_BAY_PROTOCOL, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.bay_area_protocol = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.bay_area_protocol = NET_ENUM_FALSE;
        }else{
            response->info.bay_area_protocol = NET_ENUM_TRUE;
        }

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PROTOCOL_GB_T, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.protocol_gb_t = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.protocol_gb_t = NET_ENUM_FALSE;
        }else{
            response->info.protocol_gb_t = NET_ENUM_FALSE;
        }

        data = *(sys_read_config_item_content(CONFIG_ITEM_SUPORT_BMS_SEVERAL_FRAME, 0x00));
        if(data == CONFIG_ENABLE_ENUM){
            response->info.bms_several_frame = NET_ENUM_TRUE;
        }else if(data == CONFIG_DISABLE_ENUM){
            response->info.bms_several_frame = NET_ENUM_FALSE;
        }else{
            response->info.bms_several_frame = NET_ENUM_TRUE;
        }
    }
    /** 配置信息设置 */
    else{
        if((data == NULL) || (dlen < sizeof(struct ykcm_fixed_cmd_info))){
            LOG_E("ykcm input data invalid with fixed cmd config|%d,%d", dlen, sizeof(struct ykcm_fixed_cmd_info));
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        struct ykcm_fixed_cmd_info *info = (struct ykcm_fixed_cmd_info*)data;

        if(YKC_MONITOR_FIXED_CMD_MSG_VER != info->msg_version){
            LOG_E("ykcm fixed cmd config msg version error|%d,%d", YKC_MONITOR_FIXED_CMD_MSG_VER, info->msg_version);
            return (NETYKCM_CONFIG_RES_ITEM_FAIL_BASE + 0x00);
        }

        /** 针对所有枪 */
        if(gunno == 0xFF){
            return ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_FIXED_CMD_INFO, data, NULL, NULL);
        }else{
            return ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_FIXED_CMD_INFO, data, NULL, NULL);
        }
    }

    return 0x00;
}

/** 动态类型指令信息配置 */
/*************************************************
 * 函数名      ykc_monitor_config_info_dynamic_cmd
 * 功能          处理服务器下发的动态类型指令信息请求
 * 返回          <0：失败(无效数据-系统故障，不执行响应)
 *      =0：成功
 *      >0：失败(作为失败原因进行响应)
 * **********************************************/
static int32_t ykc_monitor_config_info_dynamic_cmd(uint8_t option, uint8_t gunno, void *data, uint16_t dlen, void *buf, uint16_t blen)
{
    if((buf == NULL) || (blen < sizeof(struct ykcm_fixed_cmd_info))){
        LOG_E("ykcm input buf invalid with dynamic cmd config|%d,%d", blen, sizeof(struct ykcm_fixed_cmd_info));
        return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x00);
    }
    /** 配置信息查询 */
    if(option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
        struct ykcm_dynamic_cmd_read *info = (struct ykcm_dynamic_cmd_read*)data;
        struct ykcm_dynamic_cmd_modify *response = (struct ykcm_dynamic_cmd_modify*)buf;

        if(info->cmd_num > YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM){
            info->cmd_num = YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM;
        }
        /** 读的时候不限制版本信息 */
        memset(response, 0x00, blen);
        response->msg_version = YKC_MONITOR_DYNAMIC_CMD_MSG_VER;
        response->cmd_num = info->cmd_num;

        /** 在此处回调用以从业务获取指令信息 */
        /** 针对所有枪 */
        if(gunno == 0xFF){
            ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_DYNAMIC_CMD_INFO_READ, response, &blen, data);
        }else{
            ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_DYNAMIC_CMD_INFO_READ, response, &blen, data);
        }
    }
    /** 配置信息设置 */
    else{
        if(data == NULL){
            LOG_E("00000 ykcm input data invalid with dynamic cmd config|%d", data);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }
        int32_t ret = 0x00;
        struct ykcm_dynamic_cmd_modify *info = (struct ykcm_dynamic_cmd_modify*)data;
        /** 检查版本信息 */
        if(YKC_MONITOR_DYNAMIC_CMD_MSG_VER != info->msg_version){
            LOG_E("ykcm dynamic cmd config msg version error|%d,%d", YKC_MONITOR_DYNAMIC_CMD_MSG_VER, info->msg_version);
            return (NETYKCM_CONFIG_RES_ITEM_FAIL_BASE + 0x00);
        }
        /** 限制单次操作指令数量 */
        if(info->cmd_num > YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM){
            info->cmd_num = YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM;
        }
        /** 报文总长度不对 */
        if(dlen < (sizeof(struct ykcm_dynamic_cmd_modify) + (info->cmd_num *sizeof(struct cmd_modify_segment)))){
            LOG_E("11111 ykcm input data invalid with dynamic cmd config|%d, %d", dlen, info->cmd_num);
            return (NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE + 0x02);
        }

        /** 针对所有枪 */
        if(gunno == 0xFF){
            ret = ykc_monitor_config_execute(0x00, THAISEN_CONFIG_PAGE_DYNAMIC_CMD_INFO_ISSUE, data, NULL, NULL);
        }else{
            ret = ykc_monitor_config_execute((gunno - 0x01), THAISEN_CONFIG_PAGE_DYNAMIC_CMD_INFO_ISSUE, data, NULL, NULL);
        }
        if(ret >= NETYKCM_CONFIG_RES_ITEM_FAIL_BASE){
            ret++;
        }
        return ret;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_config_info_process
 * 功能          处理服务器下发的配置信息修改、查询请求
 * **********************************************/
int8_t ykc_monitor_config_info_process(void *data, uint16_t dlen, void *buf, uint16_t blen, uint16_t *olen)
{
    if((data == NULL) || (dlen < sizeof(Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t))){
        LOG_E("ykcm input data error with config_info_process|%d |%d, %d",  \
                data, dlen, sizeof(Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t));
        return -0x01;
    }
    if((buf == NULL) || (blen < sizeof(Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t))){
        LOG_E("ykcm input buff error with config_info_process|%d |%d, %d",  \
                buf, blen, sizeof(Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t));
        return -0x01;
    }

    int32_t ret = 0x00;
    uint16_t out_len = sizeof(Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t),
             cdata_len = (dlen - sizeof(Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t)),
             rbuf_len = (blen - sizeof(Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t));
    Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t *request = (Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t*)data;
    Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t *response = (Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t*)buf;

    if((request->body.option >= NETYKCM_CONFIG_INFO_OPTION_SIZE) || (request->body.option < 0x00)){
        LOG_E("ykcm config info process option error|%d", request->body.option);
        return -0x01;
    }
    memcpy(response, request, sizeof(Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t));

    switch(request->body.info_type){
    case NETYKCM_CONFIG_INFO_TYPE_SYSTEM:
        LOG_D("ykcm config info query set --- system info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_sys_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_sys_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_PILE:
        LOG_D("ykcm config info query set --- pile info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_pile_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_pile_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_SERVER:
        LOG_D("ykcm config info query set --- server info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_server_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_server_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_AMMETER:
        LOG_D("ykcm config info query set --- ammeter info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_ammeter_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_ammeter_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_MODULE:
        LOG_D("ykcm config info query set --- module info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_module_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_module_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_VIN:
        LOG_D("ykcm config info query set --- vin info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_vin_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_vin_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_PROTECT_INFO:
        LOG_D("ykcm config info query set --- protect info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_protect_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_protect_info);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_FUNCTION_CONFIG:
        LOG_D("ykcm config info query set --- function config info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_function_config_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_function_config);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_OFFLINE_BILLING:
        LOG_D("ykcm config info query set --- offline billing info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_offline_billing_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_offline_billing);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_INPUT_7103_7101:
        LOG_D("ykcm config info query set --- input 7103/7101 info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_input_7103_7101_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_input_info_7103_7101);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_PUBLIC_INPUT_7104:
        LOG_D("ykcm config info query set --- public input 7104 info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_public_input_7104_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_public_input_info_7104);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_GUN_INPUT_7104:
        LOG_D("ykcm config info query set --- gun input 7104 info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_gun_input_7104_info(request->body.option, request->body.gunno,
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_gun_input_info_7104);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_PUBLIC_OUTPUT_7104:
        LOG_D("ykcm config info query set --- public output 7104 info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_public_output_7104_info(request->body.option, ((uint8_t*)&request->body.option + 0x01), \
                cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_public_output_info_7104);
        }
        break;
    case NETYKCM_CONFIG_INFO_TYPE_GUN_OUTPUT_7104:
        LOG_D("ykcm config info query set --- gun output 7104 info(%d)", request->body.option);
        ret = ykc_monitor_config_info_process_gun_output_7104_info(request->body.option, request->body.gunno, \
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_gun_output_info_7104);
        }
        break;
    case NETYKCM_CONFIG_INFO_MODE_SELECT_NORMAL:
        LOG_D("ykcm config info query set --- mode select normal info(%d)", request->body.option);
        ret = ykc_monitor_config_info_mode_select_normal(request->body.option, request->body.gunno, \
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_mode_select_normal);
        }
        break;
    case NETYKCM_CONFIG_INFO_MODE_SELECT_V2G:
        LOG_D("ykcm config info query set --- mode select V2G info(%d)", request->body.option);
        ret = ykc_monitor_config_info_mode_select_v2g(request->body.option, request->body.gunno, \
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_mode_select_v2g);
        }
        break;
    case NETYKCM_CONFIG_INFO_OTHER_CONFIG:
        LOG_D("ykcm config info query set --- other info(%d)", request->body.option);
        ret = ykc_monitor_config_info_other(request->body.option, request->body.gunno, \
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_other_config);
        }
        break;
    case NETYKCM_CONFIG_INFO_DYNAMIC_CMD_INFO:
        LOG_D("ykcm config info query set --- dynamic cmd info(%d)", request->body.option);
        ret = ykc_monitor_config_info_dynamic_cmd(request->body.option, request->body.gunno, \
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            /** 获取报文数据部分长度 */
            struct ykcm_dynamic_cmd_modify *info = (struct ykcm_dynamic_cmd_modify*)((uint8_t*)&response->body.option + 0x01);
            if(info->cmd_num > YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM){
                info->cmd_num = YKC_MONITOR_DYNAMIC_CMD_SINGLE_NUM;
            }
            out_len += (sizeof(struct ykcm_dynamic_cmd_modify) + (info->cmd_num *sizeof(struct cmd_modify_segment)));
        }
        break;
    case NETYKCM_CONFIG_INFO_FIXED_CMD_INFO:
        LOG_D("ykcm config info query set --- fixed cmd info(%d)", request->body.option);
        ret = ykc_monitor_config_info_fixed_cmd(request->body.option, request->body.gunno, \
                ((uint8_t*)&request->body.option + 0x01), cdata_len, ((uint8_t*)&response->body.option + 0x01), rbuf_len);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_QUERY){
            out_len += sizeof(struct ykcm_fixed_cmd_info);
        }
        break;
    default:
        LOG_D("ykcm config info query set --- info type error(%d)", request->body.info_type);
        if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_SET){
            struct ykcm_response_result *result = (struct ykcm_response_result*)((uint8_t*)&response->body.option + 0x01);

            result->result = 0x01;
            result->fail_reason = (NETYKCM_CONFIG_RES_SYS_ASSERT_BASE + 0x00);
            out_len += sizeof(struct ykcm_response_result);
            return 0x00;
        }
        return -0x01;
    }

    if(request->body.option == NETYKCM_CONFIG_INFO_OPTION_SET){
        struct ykcm_response_result *result = (struct ykcm_response_result*)((uint8_t*)&response->body.option + 0x01);

        result->result = NETYKCM_CONFIG_RES_SUCCESS;
        result->fail_reason = NETYKCM_CONFIG_RES_SUCCESS;
        if(ret != NETYKCM_CONFIG_RES_SUCCESS){
            result->result = 0x01;
            result->fail_reason = ret;
        }
        out_len += sizeof(struct ykcm_response_result);
    }

    if(olen){
        *(uint16_t*)olen = out_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_module_fault_check
 * 功能          模块故障检测
 * **********************************************/
static void ykc_monitor_module_fault_check(void)
{
    static uint8_t in_power_connect = NET_ENUM_TRUE, in_power_connect_delay = 0x00;
    thaisenModuleFaultInfoStruct * fault = NULL;
    uint8_t group = 0x00, number = 0x00, i, j;

    extern uint8_t thaisenModule_IsInPowerConnected(void);

    /** 模块输入电源已断开，不再上报模块相关故障 */
    if(thaisenModule_IsInPowerConnected() == NET_ENUM_TRUE){
        in_power_connect = NET_ENUM_TRUE;
    }else{
        in_power_connect = NET_ENUM_FALSE;
    }
    /** 模块输入电源连接后再延迟一定时间再检测模块故障(要等待模块通讯建立) */
    if(in_power_connect == NET_ENUM_TRUE){
        if(in_power_connect_delay < (0xFF - 0x01)){
            in_power_connect_delay++;
        }
        /** 外部调用此函数时基是500ms，大概2s */
        if(in_power_connect_delay <= 0x04){
            return;
        }
    }else{
        in_power_connect_delay = 0x00;
        return;
    }

    if(s_ykc_monitor_mfault_info.flag.is_report != NET_ENUM_TRUE){
        s_ykc_monitor_mfault_info.report_tick = rt_tick_get();
    }
    group = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));

    for(i = 0; i < group; i++){
        fault = thaisenGetModuleFaultInfo(&number, i);
        if(fault){
            for(j = 0; j < number; j++){
                if((fault[j].state.fault.fault_val) || (fault[j].state.warn.warn_val)){   /** 模块有告警或故障 */
                    /** 触发上报模块故障 */
                    s_ykc_monitor_mfault_info.faddr = fault[j].addr;
                    s_ykc_monitor_mfault_info.flag.is_resume = NET_ENUM_FALSE;
                    if(s_ykc_monitor_mfault_info.flag.is_report == NET_ENUM_FALSE){
                        s_ykc_monitor_mfault_info.flag.is_waiting_response = NET_ENUM_TRUE;
                        ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                                0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_MFAULT_INFO);
                    }
                    s_ykc_monitor_mfault_info.flag.is_report = NET_ENUM_TRUE;
                    break;
                }
            }
            if(j < number){
                break;
            }
        }
    }
    if(ykc_monitor_net_event_receive(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00,
            (NET_YKC_MONITOR_EVENT_OPTION_OR |NET_YKC_MONITOR_EVENT_OPTION_CLEAR), NET_YKC_MONITOR_USER_SRES_EVENT_MFAULT_RES, NULL) > 0){
        s_ykc_monitor_mfault_info.flag.is_waiting_response = NET_ENUM_FALSE;
        if(s_ykc_monitor_mfault_info.flag.is_resume == NET_ENUM_TRUE){
            s_ykc_monitor_mfault_info.flag.is_report = NET_ENUM_FALSE;
        }
    }

    if(i >= group){
        /** 故障已恢复，上报信息 */
        if(s_ykc_monitor_mfault_info.flag.is_report == NET_ENUM_TRUE){
            if(s_ykc_monitor_mfault_info.flag.is_resume == NET_ENUM_FALSE){
                s_ykc_monitor_mfault_info.flag.is_resume = NET_ENUM_TRUE;
                s_ykc_monitor_mfault_info.flag.is_waiting_response = NET_ENUM_TRUE;

                ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                        0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_MFAULT_INFO);
            }else{
                if(s_ykc_monitor_mfault_info.flag.is_waiting_response == NET_ENUM_TRUE){
                    if((rt_tick_get() - s_ykc_monitor_mfault_info.report_tick) > YKC_MONITOR_MFAULT_REPEAT_FAST_PERIOD){
                        ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                                0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_MFAULT_INFO);
                        s_ykc_monitor_mfault_info.flag.is_waiting_response = NET_ENUM_TRUE;
                        s_ykc_monitor_mfault_info.report_tick = rt_tick_get();
                    }
                }
            }
        }
    }else{
        /** 有故障时定时上报 */
        if(s_ykc_monitor_mfault_info.flag.is_report == NET_ENUM_TRUE){
            if(s_ykc_monitor_mfault_info.flag.is_waiting_response == NET_ENUM_TRUE){
                if((rt_tick_get() - s_ykc_monitor_mfault_info.report_tick) > YKC_MONITOR_MFAULT_REPEAT_FAST_PERIOD){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_MFAULT_INFO);
                    s_ykc_monitor_mfault_info.flag.is_waiting_response = NET_ENUM_TRUE;
                    s_ykc_monitor_mfault_info.report_tick = rt_tick_get();
                }
            }else{
                if((rt_tick_get() - s_ykc_monitor_mfault_info.report_tick) > YKC_MONITOR_MFAULT_REPEAT_NORMAL_PERIOD){
                    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                            0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_MFAULT_INFO);
                    s_ykc_monitor_mfault_info.flag.is_waiting_response = NET_ENUM_TRUE;
                    s_ykc_monitor_mfault_info.report_tick = rt_tick_get();
                }
            }
        }
    }
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_module_fault_info
 * 功能         组包：填充模块故障信息
 * 参数         gunno   枪号
 *       buf      缓存
 *       ilen    输入缓存长度
 *       olen    填写数据总长度
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_message_padding_module_fault_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }

    thaisenModuleFaultSetStruct info;
    thaisenModuleFaultInfoStruct * fault = NULL;
    struct ykcm_mfault_pre_process_info *pre_head = NULL;
    struct ykcm_mfault_info *body = NULL;
    Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t *message = (Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t*)buf;
    uint32_t total_len = sizeof(Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t) + sizeof(struct ykcm_mfault_pre_process_info);
    uint8_t group = 0x00, num = 0x00, count = 0x00, i, j, num_item = CONFIG_ITEM_MODULE_NUM_GROUP_1;

    memset(buf, 0x00, ilen);
    pre_head = (struct ykcm_mfault_pre_process_info*)(buf + sizeof(Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE);
    body = (struct ykcm_mfault_info*)(buf + sizeof(Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t) - NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE + sizeof(struct ykcm_mfault_pre_process_info));

    if(total_len > ilen){
        return -0x02;
    }
    group = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));
    for(i = 0; i < group; i++){
        num = *(sys_read_config_item_content((num_item + i), 0x00));
        total_len += (num *sizeof(struct ykcm_mfault_info));
        pre_head->group_num += num;
    }
    if(total_len > ilen){
        return -0x02;
    }

    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    message->body.info_type = 0x00;

    pre_head->timestamp = time(NULL);
    pre_head->faddr = s_ykc_monitor_mfault_info.faddr;
    pre_head->is_resume = s_ykc_monitor_mfault_info.flag.is_resume;
    pre_head->module_protocol = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_MODEL, 0x00));
    num_item = CONFIG_ITEM_MODULE_NUM_GROUP_1;
    for(i = 0; i < group; i++){
        num = *(sys_read_config_item_content((num_item + i), 0x00));
        fault = thaisenGetModuleFaultInfo(NULL, i);
        for(j = 0; j < num; j++){
            info = thaisenGetModuleFaultSetInfo(i, j);
            if(fault){
                body[count].addr = fault[j].addr;
            }
            body[count].main_fault = info.main_fault;
            body[count].sub_fault = info.sub_fault;
            count++;
        }
    }

    s_ykc_monitor_mfault_info.report_tick = rt_tick_get();

    if(olen){
        *olen = total_len;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_request_server_info
 * 功能          组包：向服务器请求信息
 * **********************************************/
int8_t ykc_monitor_message_padding_request_server_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint16_t total = sizeof(Net_YkcMonitorPro_Preq_RequestServerInfo_t);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    uint8_t gunno = 0x00;
    System_BaseData *base = NULL;
    Net_YkcMonitorPro_Preq_RequestServerInfo_t *message = (Net_YkcMonitorPro_Preq_RequestServerInfo_t*)buf;

    for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        base = (System_BaseData*)(s_ykc_monitor_handle->get_base_data(gunno));
        if((base->state.current >= APP_OFSM_STATE_STARTING) && (base->state.current <= APP_OFSM_STATE_STOPING)){
            break;
        }
    }
    if(gunno < NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    memset(message, 0x00, ilen);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    if(olen){
        *olen = total;
    }

    return 0x00;
#else
    return -0x01;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
}

/*************************************************
 * 函数名      ykc_monitor_guidance_changed_info_padding
 * 功能          组包：填充导引状态变化信息
 * **********************************************/
int8_t ykc_monitor_guidance_changed_info_padding(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t) + 0x01;

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < (total + (NET_YKC_MONITOR_GUIDANCE_CHANGED_INFO_MAX *sizeof(struct guidance_segment)))){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t *message = (Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t*)buf;
    struct running_data_info *running_data = (struct running_data_info*)((uint8_t*)&(message->body.msg_version) + 0x01);
    struct guidance_segment *segment = (struct guidance_segment*)((uint8_t*)&(running_data->segment_num) + 0x01);

    memset(message, 0x00, sizeof(ilen));

    rt_enter_critical();

    if(s_ykc_monitor_guidance_changed[gunno].count == 0x00){
        rt_exit_critical();
        return -0x03;
    }
    if(s_ykc_monitor_guidance_changed[gunno].count > NET_YKC_MONITOR_GUIDANCE_CHANGED_INFO_MAX){
        s_ykc_monitor_guidance_changed[gunno].count = NET_YKC_MONITOR_GUIDANCE_CHANGED_INFO_MAX;
    }
    running_data->segment_num = s_ykc_monitor_guidance_changed[gunno].count;
    for(uint8_t i = 0x00; i < running_data->segment_num; i++){
        segment[i].timestamp = s_ykc_monitor_guidance_changed[gunno].data[i].timestamp;
        segment[i].voltage = s_ykc_monitor_guidance_changed[gunno].data[i].voltage;
        segment[i].voltage_last = s_ykc_monitor_guidance_changed[gunno].data[i].voltage_last;
        segment[i].diff_positive_adc = s_ykc_monitor_guidance_changed[gunno].data[i].diff_positive_adc;
        segment[i].diff_negtive_adc = s_ykc_monitor_guidance_changed[gunno].data[i].diff_negtive_adc;
        segment[i].diff_positive_adc_last = s_ykc_monitor_guidance_changed[gunno].data[i].diff_positive_adc_last;
        segment[i].diff_negtive_adc_last = s_ykc_monitor_guidance_changed[gunno].data[i].diff_negtive_adc_last;
        segment[i].flag = s_ykc_monitor_guidance_changed[gunno].data[i].flag;
    }
    s_ykc_monitor_guidance_changed[gunno].count = 0x00;

    rt_exit_critical();

    message->body.info_type = NETYKCM_DEV_RUNNING_DATA_INFO_GUIDANCE;
    message->body.option = 0x01;        /** 数据上报 */
    message->body.msg_version = 0x00;   /** 报文版本 */
    message->body.gunno = (gunno + 0x01);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    total += (running_data->segment_num *sizeof(struct guidance_segment));

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_guidance_changed_callback
 * 功能         导引状态信息变化回调
 * **********************************************/
void ykc_monitor_guidance_changed_callback(uint8_t gunno, uint8_t flag, uint32_t timestamp, int voltage, int voltage_last, uint16_t diff_positive_adc, \
        uint16_t diff_negtive_adc, uint16_t diff_positive_adc_last, uint16_t diff_negtive_adc_last)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(s_ykc_monitor_guidance_changed[gunno].count >= NET_YKC_MONITOR_GUIDANCE_CHANGED_INFO_MAX){
        return;
    }
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].timestamp = timestamp;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].voltage = voltage;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].voltage_last = voltage_last;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].diff_positive_adc = diff_positive_adc;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].diff_negtive_adc = diff_negtive_adc;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].diff_positive_adc_last = diff_positive_adc_last;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].diff_negtive_adc_last = diff_negtive_adc_last;
    s_ykc_monitor_guidance_changed[gunno].data[s_ykc_monitor_guidance_changed[gunno].count].flag = flag;
    s_ykc_monitor_guidance_changed[gunno].count++;
}

/*************************************************
 * 函数名      ykc_monitor_dev_control_changed_info_padding
 * 功能          组包：填充器件控制状态变化信息
 * **********************************************/
int8_t ykc_monitor_dev_control_changed_info_padding(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t) + 0x01;

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < (total + (NET_YKC_MONITOR_DEVICE_CTRL_CHANGED_INFO_MAX *sizeof(struct control_segment)))){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t *message = (Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t*)buf;
    struct running_control_info *running_data = (struct running_control_info*)((uint8_t*)&(message->body.msg_version) + 0x01);
    struct control_segment *segment = (struct control_segment*)((uint8_t*)&(running_data->segment_num) + 0x01);

    memset(message, 0x00, sizeof(ilen));

    rt_enter_critical();

    if(s_ykc_monitor_device_control_changed[gunno].count == 0x00){
        rt_exit_critical();
        return -0x03;
    }
    if(s_ykc_monitor_device_control_changed[gunno].count > NET_YKC_MONITOR_DEVICE_CTRL_CHANGED_INFO_MAX){
        s_ykc_monitor_device_control_changed[gunno].count = NET_YKC_MONITOR_DEVICE_CTRL_CHANGED_INFO_MAX;
    }
    running_data->segment_num = s_ykc_monitor_device_control_changed[gunno].count;
    for(uint8_t i = 0x00; i < running_data->segment_num; i++){
        segment[i].timestamp = s_ykc_monitor_device_control_changed[gunno].segment[i].timestamp;
        segment[i].info.ctrl = s_ykc_monitor_device_control_changed[gunno].segment[i].info.ctrl;
        segment[i].info.is_debug = s_ykc_monitor_device_control_changed[gunno].segment[i].info.is_debug;
        segment[i].info.result = s_ykc_monitor_device_control_changed[gunno].segment[i].info.result;
        segment[i].info.type = s_ykc_monitor_device_control_changed[gunno].segment[i].info.type;
        segment[i].device = NETYKCM_DEVICE_ENUM_SIZE;
        switch(s_ykc_monitor_device_control_changed[gunno].segment[i].device){
        case THAISEN_DEVICE_ENUM_DCRELAY:
            segment[i].device = NETYKCM_DEVICE_ENUM_POS_DCRELAY;
            break;
        case THAISEN_DEVICE_ENUM_ACRELAY:
            segment[i].device = NETYKCM_DEVICE_ENUM_ACRELAY;
            break;
        case THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_0:
            segment[i].device = NETYKCM_DEVICE_ENUM_POS_PARALLEL_RELAY_0;
            break;
        case THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_0:
            segment[i].device = NETYKCM_DEVICE_ENUM_NEG_PARALLEL_RELAY_0;
            break;
        case THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_1:
            segment[i].device = NETYKCM_DEVICE_ENUM_POS_PARALLEL_RELAY_1;
            break;
        case THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_1:
            segment[i].device = NETYKCM_DEVICE_ENUM_NEG_PARALLEL_RELAY_1;
            break;
        case THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_2:
            segment[i].device = NETYKCM_DEVICE_ENUM_POS_PARALLEL_RELAY_2;
            break;
        case THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_2:
            segment[i].device = NETYKCM_DEVICE_ENUM_NEG_PARALLEL_RELAY_2;
            break;
        case THAISEN_DEVICE_ENUM_AUXPOWER_12V:
            segment[i].device = NETYKCM_DEVICE_ENUM_AUXPOWER_12V;
            break;
        case THAISEN_DEVICE_ENUM_AUXPOWER_24V:
            segment[i].device = NETYKCM_DEVICE_ENUM_AUXPOWER_24V;
            break;
        case THAISEN_DEVICE_ENUM_ELOCK:
            segment[i].device = NETYKCM_DEVICE_ENUM_ELOCK;
            break;
        case THAISEN_DEVICE_ENUM_FAN:
            segment[i].device = NETYKCM_DEVICE_ENUM_FAN;
            break;
        case THAISEN_DEVICE_ENUM_LIQUID:
            segment[i].device = NETYKCM_DEVICE_ENUM_LIQUID;
            break;
        default:
            break;
        }
    }
    s_ykc_monitor_device_control_changed[gunno].count = 0x00;

    rt_exit_critical();

    message->body.info_type = NETYKCM_DEV_RUNNING_CONTROL_INFO;
    message->body.option = 0x01;        /** 数据上报 */
    message->body.msg_version = 0x00;   /** 报文版本 */
    message->body.gunno = (gunno + 0x01);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    total += (running_data->segment_num *sizeof(struct control_segment));

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_dev_control_changed_callback
 * 功能         器件控制状态信息变化回调
 * **********************************************/
void ykc_monitor_dev_control_changed_callback(uint8_t gunno, uint8_t device, uint8_t type, uint8_t is_debug, uint8_t control, \
        uint8_t result, uint32_t timestamp)
{
    uint8_t count = 0x00;

    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    rt_enter_critical();

    s_ykc_monitor_device_status_changed[gunno].delay_count = 0x00;
    s_ykc_monitor_device_status_changed[gunno].is_recved = NET_ENUM_TRUE;
    if(s_ykc_monitor_device_control_changed[gunno].count >= NET_YKC_MONITOR_DEVICE_CTRL_CHANGED_INFO_MAX){
        rt_exit_critical();
        return;
    }

    count = s_ykc_monitor_device_control_changed[gunno].count;
    s_ykc_monitor_device_control_changed[gunno].count++;

    s_ykc_monitor_device_control_changed[gunno].segment[count].timestamp = timestamp;
    s_ykc_monitor_device_control_changed[gunno].segment[count].device = device;
    s_ykc_monitor_device_control_changed[gunno].segment[count].info.ctrl = control;
    s_ykc_monitor_device_control_changed[gunno].segment[count].info.is_debug = is_debug;
    s_ykc_monitor_device_control_changed[gunno].segment[count].info.result = result;
    s_ykc_monitor_device_control_changed[gunno].segment[count].info.type = type;

    rt_exit_critical();
}

/*************************************************
 * 函数名      ykc_monitor_dev_feedback_changed_info_padding
 * 功能          组包：填充器件反馈状态变化信息
 * **********************************************/
int8_t ykc_monitor_dev_feedback_changed_info_padding(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = (sizeof(Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t) + sizeof(struct running_status_info));

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t *message = (Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t*)buf;
    struct running_status_info *running_data = (struct running_status_info*)((uint8_t*)&(message->body.msg_version) + 0x01);

    memset(message, 0x00, sizeof(ilen));

    running_data->segment_num = 0x01;
    running_data->segment.data.timestamp = time(NULL);

    /********************* 直流继电器 *********************/
    if(thaisenDcRelay_StateQuery(gunno, thaisenRelayClose)){
        running_data->segment.data.info.dcrelay_positive_status = 0x01;
        running_data->segment.data.info.dcrelay_negtive_status = 0x01;
    }else{
        running_data->segment.data.info.dcrelay_positive_status = 0x00;
        running_data->segment.data.info.dcrelay_negtive_status = 0x00;
    }
    /********************* 电子锁 *********************/
    if(thaisenElectLock_StateQuery(gunno) == thaisen_elock_close){
        running_data->segment.data.info.elock_status = 0x01;
    }else{
        running_data->segment.data.info.elock_status = 0x00;
    }
    /********************* 液冷 *********************/
    running_data->segment.data.info.liquid_status = 0x00;
    /********************* 风扇 *********************/
    running_data->segment.data.info.fan_status = 0x00;
    /********************* 12V辅源 *********************/
    /********************* 24V辅源 *********************/
    running_data->segment.data.info.auxpower_12v_status = 0x00;
    running_data->segment.data.info.auxpower_24v_status = 0x00;
    if(gunno == 0x00){
        if(thaisenGetAux_A_Status_Debug() == thaisen_auxPower_ok){
            running_data->segment.data.info.auxpower_12v_status = 0x01;
            running_data->segment.data.info.auxpower_24v_status = 0x01;
        }
    }else{
        if(thaisenGetAux_B_Status_Debug() == thaisen_auxPower_ok){
            running_data->segment.data.info.auxpower_12v_status = 0x01;
            running_data->segment.data.info.auxpower_24v_status = 0x01;
        }
    }
    /********************* 交流接触器 *********************/
    running_data->segment.data.info.acrelay_status = 0x00;
    if(thaisenAcRelay_StateQuery() == thaisenRelayClose){
        running_data->segment.data.info.acrelay_status = 0x01;
    }
    /********************* 母联继电器 *********************/
    running_data->segment.data.info.parallelrelay_0_positive_status = 0x00;
    running_data->segment.data.info.parallelrelay_0_negtive_status = 0x00;
    if(thaisen_relay_parallel_FB_Z() == thaisenRelayClose){
        running_data->segment.data.info.parallelrelay_0_positive_status = 0x01;
    }
    if(thaisen_relay_parallel_FB_F() == thaisenRelayClose){
        running_data->segment.data.info.parallelrelay_0_negtive_status = 0x01;
    }
    running_data->segment.data.info.parallelrelay_1_positive_status = 0x00;
    running_data->segment.data.info.parallelrelay_1_negtive_status = 0x00;
    if(thaisen_relay_K7_FB() == thaisenRelayClose){
        running_data->segment.data.info.parallelrelay_1_positive_status = 0x01;
    }
    if(thaisen_relay_K8_FB() == thaisenRelayClose){
        running_data->segment.data.info.parallelrelay_1_negtive_status = 0x01;
    }
    running_data->segment.data.info.parallelrelay_2_positive_status = 0x00;
    running_data->segment.data.info.parallelrelay_2_negtive_status = 0x00;
    if(thaisen_relay_K9_FB() == thaisenRelayClose){
        running_data->segment.data.info.parallelrelay_2_positive_status = 0x01;
    }
    if(thaisen_relay_K10_FB() == thaisenRelayClose){
        running_data->segment.data.info.parallelrelay_2_negtive_status = 0x01;
    }

    /********************* 矩阵继电器 *********************/
    running_data->segment.data.info.matrixrelay_1_1_positive_status = 0x00;
    running_data->segment.data.info.matrixrelay_1_1_negtive_status = 0x00;
    running_data->segment.data.info.matrixrelay_1_2_positive_status = 0x00;
    running_data->segment.data.info.matrixrelay_1_2_negtive_status = 0x00;
    running_data->segment.data.info.matrixrelay_1_3_positive_status = 0x00;
    running_data->segment.data.info.matrixrelay_1_3_negtive_status = 0x00;
    running_data->segment.data.info.matrixrelay_2_1_positive_status = 0x00;
    running_data->segment.data.info.matrixrelay_2_1_negtive_status = 0x00;
    running_data->segment.data.info.matrixrelay_2_2_positive_status = 0x00;
    running_data->segment.data.info.matrixrelay_2_2_negtive_status = 0x00;
    running_data->segment.data.info.matrixrelay_3_1_positive_status = 0x00;
    running_data->segment.data.info.matrixrelay_3_1_negtive_status = 0x00;
    running_data->segment.data.info.reserve = 0x00;

    message->body.info_type = NETYKCM_DEV_RUNNING_STATUS_INFO;
    message->body.option = 0x01;        /** 数据上报 */
    message->body.msg_version = 0x00;   /** 报文版本 */
    message->body.gunno = (gunno + 0x01);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_request_disconnect_reason
 * 功能          组包：填充断网原因信息
 * **********************************************/
int8_t ykc_monitor_message_padding_request_disconnect_reason(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t) + 0x01 + sizeof(struct disconnect_reason_segment);

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t *message = (Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t*)buf;
    struct running_data_info *running_data = (struct running_data_info*)((uint8_t*)&(message->body.msg_version) + 0x01);
    struct disconnect_reason_segment *segment = (struct disconnect_reason_segment*)((uint8_t*)&(running_data->segment_num) + 0x01);

    memset(message, 0x00, sizeof(ilen));

    rt_enter_critical();

    running_data->segment_num = 0x01;
    for(uint8_t i = 0x00; i< NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX; i++){
        segment->close_passive.info[i].fd = s_ykc_monitor_close_passive.group.info[i].fd;
        segment->close_passive.info[i].timestamp = s_ykc_monitor_close_passive.group.info[i].timestamp;

        segment->close_active.info[i].fd = s_ykc_monitor_close_active.group.info[i].fd;
        segment->close_active.info[i].timestamp = s_ykc_monitor_close_active.group.info[i].timestamp;

        segment->heartbeat_timeout.info[i].fd = s_ykc_monitor_heartbeat_timeout.group.info[i].fd;
        segment->heartbeat_timeout.info[i].timestamp = s_ykc_monitor_heartbeat_timeout.group.info[i].timestamp;

        segment->socket_pdp.timestamp[i] = s_ykc_monitor_socket_pdp.group.timestamp[i];
        segment->close_module.timestamp[i] = s_ykc_monitor_close_module.group.timestamp[i];
        segment->at_physics.timestamp[i] = s_ykc_monitor_at_physice.group.timestamp[i];
        segment->cpin_lk_mac.timestamp[i] = s_ykc_monitor_cpin_lk_mac.group.timestamp[i];
        segment->cimi_lk_mac.timestamp[i] = s_ykc_monitor_cimi_lk_mac.group.timestamp[i];
        segment->signal_strength_lk_mac.timestamp[i] = s_ykc_monitor_signal_strength_lk_mac.group.timestamp[i];
        segment->gsm_registered.timestamp[i] = s_ykc_monitor_gsm_registered.group.timestamp[i];
        segment->gprs_registered.timestamp[i] = s_ykc_monitor_gprs_registered.group.timestamp[i];
    }

    ykc_monitor_clear_disconnect_reason();

    rt_exit_critical();

    message->body.info_type = NETYKCM_DEV_RUNNING_DATA_DISCONNECT_REASON;
    message->body.option = 0x01;        /** 数据上报 */
    message->body.msg_version = 0x00;   /** 报文版本 */
    message->body.gunno = (0x00 + 0x01);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    rt_exit_critical();

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*******************************************************
 * 函数名               ykc_monitor_disconnect_reason_callback
 * 功能                  断网原因回调
 * 参数                  fd         文件描述符
 * 返回                  reason_en  原因枚举
 ******************************************************/
void ykc_monitor_disconnect_reason_callback(int8_t fd, uint8_t reason_en)
{
    rt_enter_critical();

    switch(reason_en){
    case NET_DIS_REASON_CLOSE_PASSIVE:
        if(s_ykc_monitor_close_passive.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_close_passive.group.info[s_ykc_monitor_close_passive.count].fd = fd;
            s_ykc_monitor_close_passive.group.info[s_ykc_monitor_close_passive.count].timestamp = time(NULL);
            if(s_ykc_monitor_close_passive.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_close_passive.count++;
            }
        }
        break;
    case NET_DIS_REASON_CLOSE_ACTIVE:
        if(s_ykc_monitor_close_active.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_close_active.group.info[s_ykc_monitor_close_active.count].fd = fd;
            s_ykc_monitor_close_active.group.info[s_ykc_monitor_close_active.count].timestamp = time(NULL);
            if(s_ykc_monitor_close_active.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_close_active.count++;
            }
        }
        break;
    case NET_DIS_REASON_HEARTBEAT_TIMEOUT:
        if(s_ykc_monitor_heartbeat_timeout.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_heartbeat_timeout.group.info[s_ykc_monitor_heartbeat_timeout.count].fd = fd;
            s_ykc_monitor_heartbeat_timeout.group.info[s_ykc_monitor_heartbeat_timeout.count].timestamp = time(NULL);
            if(s_ykc_monitor_heartbeat_timeout.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_heartbeat_timeout.count++;
            }
        }
        break;
    case NET_DIS_REASON_SOCKET_PDP:
        if(s_ykc_monitor_socket_pdp.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_socket_pdp.group.timestamp[s_ykc_monitor_socket_pdp.count] = time(NULL);
            if(s_ykc_monitor_socket_pdp.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_socket_pdp.count++;
            }
        }
        break;
    case NET_DIS_REASON_CLOSE_MODULE:
        if(s_ykc_monitor_close_module.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_close_module.group.timestamp[s_ykc_monitor_close_module.count] = time(NULL);
            if(s_ykc_monitor_close_module.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_close_module.count++;
            }
        }
        break;
    case NET_DIS_REASON_AT_PHYSICS:
        if(s_ykc_monitor_at_physice.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_at_physice.group.timestamp[s_ykc_monitor_at_physice.count] = time(NULL);
            if(s_ykc_monitor_at_physice.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_at_physice.count++;
            }
        }
        break;
    case NET_DIS_REASON_CPIN_LK_MAC:
        if(s_ykc_monitor_cpin_lk_mac.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_cpin_lk_mac.group.timestamp[s_ykc_monitor_cpin_lk_mac.count] = time(NULL);
            if(s_ykc_monitor_cpin_lk_mac.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_cpin_lk_mac.count++;
            }
        }
        break;
    case NET_DIS_REASON_CIMI_LK_MAC:
        if(s_ykc_monitor_cimi_lk_mac.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_cimi_lk_mac.group.timestamp[s_ykc_monitor_cimi_lk_mac.count] = time(NULL);
            if(s_ykc_monitor_cimi_lk_mac.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_cimi_lk_mac.count++;
            }
        }
        break;
    case NET_DIS_REASON_SIGNAL_STRENGTH_LK_MAC:
        if(s_ykc_monitor_signal_strength_lk_mac.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_signal_strength_lk_mac.group.timestamp[s_ykc_monitor_signal_strength_lk_mac.count] = time(NULL);
            if(s_ykc_monitor_signal_strength_lk_mac.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_signal_strength_lk_mac.count++;
            }
        }
        break;
    case NET_DIS_REASON_GSM_REGISTERED:
        if(s_ykc_monitor_gsm_registered.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_gsm_registered.group.timestamp[s_ykc_monitor_gsm_registered.count] = time(NULL);
            if(s_ykc_monitor_gsm_registered.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_gsm_registered.count++;
            }
        }
        break;
    case NET_DIS_REASON_GPRS_REGISTERED:
        if(s_ykc_monitor_gprs_registered.count < NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX){
            s_ykc_monitor_gprs_registered.group.timestamp[s_ykc_monitor_gprs_registered.count] = time(NULL);
            if(s_ykc_monitor_gprs_registered.count < (NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX - 1)){
                s_ykc_monitor_gprs_registered.count++;
            }
        }
        break;
    default:
        break;
    }

    rt_exit_critical();
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_request_bms_message
 * 功能          组包：填充BMS报文信息
 * **********************************************/
int8_t ykc_monitor_message_padding_request_bms_message(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t total = sizeof(Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t) + 0x01;

    /** 计算所有报文长度 */
    total += (sizeof(struct yt_cfc) + sizeof(struct yt_bfc) + sizeof(struct start_crm) + sizeof(struct end_crm) + sizeof(struct brm) + \
            sizeof(struct bst) + sizeof(struct bsm) + sizeof(struct bem) + sizeof(struct bsd) + sizeof(struct other_data));
    /** 计算所有报文头的长度(目前10个报文) */
    total += (0x0A *sizeof(struct bms_msg_info_head));

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint8_t max_len = 0x00, head_len = sizeof(struct bms_msg_info_head);
    Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t *message = (Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t*)buf;
    struct running_data_info *running_data = (struct running_data_info*)((uint8_t*)&(message->body.msg_version) + 0x01);
    uint8_t *segment = ((uint8_t*)&(running_data->segment_num) + 0x01);
    struct bms_msg_info_head *head = NULL;
    struct brm *brm_info = NULL;
    thaisenSuperCurrProtocol *info = thaisenGetSuperCurrProtocolInfo(gunno);
    thaisenBSDDetailed_t bsd = thaisenGetBSDDetailed(gunno);
    thaisenBEMDetailed_t bem = thaisenGetBEMDetailed(gunno);
    thaisenBSMDetailed_t bsm = thaisenGetBSMDetailed(gunno);
    thaisenBSTDetailed_t bst = thaisenGetBSTDetailed(gunno);

    memset(message, 0x00, sizeof(ilen));
    /** 固定10个段 */
    running_data->segment_num = 0x0A;
    /********************************** 宇通CFC报文 **********************************/
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_YT_CFC;
    head->msg_len = sizeof(struct yt_cfc);
    segment += head_len;
    ((struct yt_cfc*)segment)->curr_offset = info->YT_CFC.CurrOffset;
    ((struct yt_cfc*)segment)->gun_num = info->YT_CFC.GunNum;
    ((struct yt_cfc*)segment)->ack = info->YT_CFC.Ack;
    ((struct yt_cfc*)segment)->reserve0 = info->YT_CFC.Reserve0;
    ((struct yt_cfc*)segment)->reserve1 = info->YT_CFC.Reserve1;
    ((struct yt_cfc*)segment)->reserve2 = info->YT_CFC.Reserve2;
    ((struct yt_cfc*)segment)->reserve3 = info->YT_CFC.Reserve3;
    ((struct yt_cfc*)segment)->reserve4 = info->YT_CFC.Reserve4;
    ((struct yt_cfc*)segment)->reserve5 = info->YT_CFC.Reserve5;
    ((struct yt_cfc*)segment)->reserve6 = info->YT_CFC.Reserve6;

    /********************************** 宇通BFC报文 **********************************/
    segment += sizeof(struct yt_cfc);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_YT_BFC;
    head->msg_len = sizeof(struct yt_bfc);
    segment += head_len;
    ((struct yt_bfc*)segment)->curr_offset = info->YT_BFC.CurrOffset;
    ((struct yt_bfc*)segment)->gun_num = info->YT_BFC.GunNum;
    ((struct yt_bfc*)segment)->ack = info->YT_BFC.Ack;
    ((struct yt_bfc*)segment)->reserve0 = info->YT_BFC.Reserve0;
    ((struct yt_bfc*)segment)->reserve1 = info->YT_BFC.Reserve1;
    ((struct yt_bfc*)segment)->reserve2 = info->YT_BFC.Reserve2;
    ((struct yt_bfc*)segment)->reserve3 = info->YT_BFC.Reserve3;
    ((struct yt_bfc*)segment)->reserve4 = info->YT_BFC.Reserve4;
    ((struct yt_bfc*)segment)->reserve5 = info->YT_BFC.Reserve5;
    ((struct yt_bfc*)segment)->reserve6 = info->YT_BFC.Reserve6;
    ((struct yt_bfc*)segment)->is_recved = info->YT_BFC.Recved;

    /********************************** 起始CRM报文 **********************************/
    segment += sizeof(struct yt_bfc);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_CRM_START;
    head->msg_len = sizeof(struct start_crm);
    segment += head_len;
    ((struct start_crm*)segment)->discern = info->CRM_Start.Discern;
    ((struct start_crm*)segment)->chage_number = info->CRM_Start.ChagNum;
    memcpy(((struct start_crm*)segment)->chage_place, info->CRM_Start.ChagPlace, sizeof(((struct start_crm*)segment)->chage_place));

    /********************************** 结束CRM报文 **********************************/
    segment += sizeof(struct start_crm);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_CRM_END;
    head->msg_len = sizeof(struct end_crm);
    segment += head_len;
    ((struct end_crm*)segment)->discern = info->CRM_End.Discern;
    ((struct end_crm*)segment)->chage_number = info->CRM_End.ChagNum;
    memcpy(((struct end_crm*)segment)->chage_place, info->CRM_End.ChagPlace, sizeof(((struct end_crm*)segment)->chage_place));

    /********************************** BRM报文 **********************************/
    segment += sizeof(struct end_crm);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_BRM;
    head->msg_len = sizeof(struct brm);
    segment += head_len;
    brm_info = (struct brm*)segment;

    if(head->msg_len > sizeof(info->BRM.BMSVer)){
        memcpy(brm_info->bms_version, &(info->BRM.BMSVer), sizeof(info->BRM.BMSVer));
    }else{
        memcpy(brm_info->bms_version, &(info->BRM.BMSVer), head->msg_len);
    }

    brm_info->bat_type = info->BRM.BatType;
    brm_info->bat_rate_capacity = info->BRM.BatRateCap;
    brm_info->bat_rate_volt = info->BRM.BatRateVolt;

    max_len = sizeof(brm_info->bat_firm);
    if(max_len > sizeof(info->BRM.BatFirm)){
        memcpy(brm_info->bat_firm, &(info->BRM.BatFirm), sizeof(info->BRM.BatFirm));
    }else{
        memcpy(brm_info->bat_firm, &(info->BRM.BatFirm), max_len);
    }

    max_len = sizeof(brm_info->serial_number);
    if(max_len > sizeof(info->BRM.SerialNum)){
        memcpy(brm_info->serial_number, &(info->BRM.SerialNum), sizeof(info->BRM.SerialNum));
    }else{
        memcpy(brm_info->serial_number, &(info->BRM.SerialNum), max_len);
    }

    brm_info->bat_buld_year = info->BRM.BatBuldyear;
    brm_info->bat_buld_month = info->BRM.BatBuldmonth;
    brm_info->bat_buld_day = info->BRM.BatBuldday;

    max_len = sizeof(brm_info->chage_timer);
    if(max_len > sizeof(info->BRM.Chagtimer)){
        memcpy(brm_info->chage_timer, &(info->BRM.Chagtimer), sizeof(info->BRM.Chagtimer));
    }else{
        memcpy(brm_info->chage_timer, &(info->BRM.Chagtimer), max_len);
    }

    brm_info->bat_property = info->BRM.BatProperty;
    brm_info->reserved = info->BRM.reserved;

    max_len = sizeof(brm_info->car_vin);
    if(max_len > sizeof(info->BRM.CarDiscern)){
        memcpy(brm_info->car_vin, &(info->BRM.CarDiscern), sizeof(info->BRM.CarDiscern));
    }else{
        memcpy(brm_info->car_vin, &(info->BRM.CarDiscern), max_len);
    }

    max_len = sizeof(brm_info->bms_ver_number);
    if(max_len > sizeof(info->BRM.BMSVerNum)){
        memcpy(brm_info->bms_ver_number, &(info->BRM.BMSVerNum), sizeof(info->BRM.BMSVerNum));
    }else{
        memcpy(brm_info->bms_ver_number, &(info->BRM.BMSVerNum), max_len);
    }

    rt_enter_critical();

    /********************************** BST报文 **********************************/
    segment += sizeof(struct brm);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_BST;
    head->msg_len = sizeof(struct bst);
    segment += head_len;
    ((struct bst*)segment)->data.target_soc = bst.data.SOCGetObj;
    ((struct bst*)segment)->data.target_total_volt = bst.data.VoltGetObj;
    ((struct bst*)segment)->data.target_single_volt = bst.data.CeliVoltGetObj;
    ((struct bst*)segment)->data.charger_end = bst.data.ChargInitiStop;
    ((struct bst*)segment)->data.insultion_fault = bst.data.InsltFault;
    ((struct bst*)segment)->data.outlinker_fault = bst.data.OutConectOVtemp;
    ((struct bst*)segment)->data.bms_element_fault = bst.data.BMSCompOVtemp;
    ((struct bst*)segment)->data.charge_linker_fault = bst.data.Conectfault;
    ((struct bst*)segment)->data.bat_group_fault = bst.data.BatOVtemp;
    ((struct bst*)segment)->data.hv_relay_fault = bst.data.HVRelaysFault;
    ((struct bst*)segment)->data.detect_point_2 = bst.data.Check2Ft;
    ((struct bst*)segment)->data.other_fault = bst.data.OtherFt;
    ((struct bst*)segment)->data.over_curr = bst.data.OverCurlt;
    ((struct bst*)segment)->data.volt_abnormal = bst.data.Voltfault;
    ((struct bst*)segment)->data.reserve = bst.data.Reserve;
    ((struct bst*)segment)->is_recved = bst.Recved;

    /********************************** BSM报文 **********************************/
    segment += sizeof(struct bst);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_BSM;
    head->msg_len = sizeof(struct bsm);
    segment += head_len;
    ((struct bsm*)segment)->data.max_singlevolt_sn = bsm.data.HigVoltCellNum;
    ((struct bsm*)segment)->data.highest_temp = bsm.data.HigTemp;
    ((struct bsm*)segment)->data.highest_temp_sn = bsm.data.HigTempNum;
    ((struct bsm*)segment)->data.lowest_temp = bsm.data.LowTemp;
    ((struct bsm*)segment)->data.lowest_temp_sn = bsm.data.LowTempNum;
    ((struct bsm*)segment)->data.singlevolt_over = bsm.data.CellOverVolt;
    ((struct bsm*)segment)->data.soc_state = bsm.data.SOCState;
    ((struct bsm*)segment)->data.bat_overcurrr = bsm.data.BatOverCurlt;
    ((struct bsm*)segment)->data.bat_overtemp = bsm.data.BatOverTemp;
    ((struct bsm*)segment)->data.insultion_state = bsm.data.Insulat;
    ((struct bsm*)segment)->data.outlinker_state = bsm.data.OutConect;
    ((struct bsm*)segment)->data.is_allow_charge = bsm.data.AllowChg;
    ((struct bsm*)segment)->data.reserve = bsm.data.Reserve;
    ((struct bsm*)segment)->is_recved = bsm.Recved;

    /********************************** BEM报文 **********************************/
    segment += sizeof(struct bsm);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_BEM;
    head->msg_len = sizeof(struct bem);
    segment += head_len;
    ((struct bem*)segment)->data.crm_00_timeout = bem.data.CRM00OVtime;
    ((struct bem*)segment)->data.crm_aa_timeout = bem.data.CRMAAOVtime;
    ((struct bem*)segment)->data.cts_cml_timeout = bem.data.CTSCMLOVtime;
    ((struct bem*)segment)->data.cro_aa_timeout = bem.data.CROOVtime;
    ((struct bem*)segment)->data.ccs_timeout = bem.data.CCSOVtime;
    ((struct bem*)segment)->data.cst_timeout = bem.data.CSTOVtime;
    ((struct bem*)segment)->data.csd_timeout = bem.data.CSDOVtime;
    ((struct bem*)segment)->is_recved = bem.Recved;

    /********************************** BSD报文 **********************************/
    segment += sizeof(struct bem);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_BSD;
    head->msg_len = sizeof(struct bsd);
    segment += head_len;
    ((struct bsd*)segment)->end_soc = bsd.StopSOC;
    ((struct bsd*)segment)->singlevolt_lowest = bsd.CellLowVolt;
    ((struct bsd*)segment)->singlevolt_highest = bsd.CellHigVolt;
    ((struct bsd*)segment)->temp_lowest = bsd.LowTemp;
    ((struct bsd*)segment)->temp_highest = bsd.HigTemp;
    ((struct bsd*)segment)->is_recved = bsd.Recved;

    rt_exit_critical();

    /********************************** 其它报文 **********************************/
    segment += sizeof(struct bsd);
    head = (struct bms_msg_info_head*)segment;

    head->msg_type = NETYKCM_BMS_MSG_OTHER;
    head->msg_len = sizeof(struct other_data);
    segment += head_len;
    ((struct other_data*)segment)->protocol_type = info->ProtocolType;
    ((struct other_data*)segment)->current_offset = info->CurrOffset;

    message->body.info_type = NETYKCM_DEV_RUNNING_DATA_BMS_MESSAGE;
    message->body.option = 0x01;        /** 数据上报 */
    message->body.msg_version = 0x00;   /** 报文版本 */
    message->body.gunno = (gunno + 0x01);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);

    if(olen){
        *olen = total;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ykc_monitor_liquid_fault_check
 * 功能          液冷故障检测
 * **********************************************/
static void ykc_monitor_liquid_fault_check(void)
{
    extern thaisenLiquidDevType thaisenLiquid_get_LiquidDev(void);
    extern thaisenLiquidSt *thaisenGetLiquidPara(uint8_t gunNum);
    extern uint8_t thaisenGetLiquidNum(void);
    uint8_t liquid_num = 0x00;
    thaisenLiquidSt *liquid_info = NULL;

    /************************************ 如果液冷数量为0则不检测 ************************************/
    liquid_num = thaisenGetLiquidNum();
    if(liquid_num == 0x00){
        memset(&s_ykcm_liquid_f_info, 0x00, sizeof(s_ykcm_liquid_f_info));
        return;
    }
#if 0
    /************************************ 在此等待服务器响应 ************************************/
    if(s_ykcm_liquid_f_info.info.wait_response == NET_ENUM_TRUE){
        if(s_ykcm_liquid_f_info.wait_response_time < (0xFF - 0x01)){
            s_ykcm_liquid_f_info.wait_response_time++;
        }
        /** 已接收到服务器响应 */
        if(ykc_monitor_net_event_receive(NET_YKC_MONITOR_USER_EVENT_HANDLE_SERVER, NET_YKC_MONITOR_EVENT_TYPE_RESPONSE, 0x00,
                (NET_YKC_MONITOR_EVENT_OPTION_OR |NET_YKC_MONITOR_EVENT_OPTION_CLEAR), NET_YKC_MONITOR_USER_SRES_EVENT_LFAULT_RES, NULL) > 0){
            s_ykcm_liquid_f_info.info.wait_response = NET_ENUM_FALSE;
            s_ykcm_liquid_f_info.wait_response_time = 0x00;
        }
        /** 函数调用时基100ms，大概5s，重发 */
        if(s_ykcm_liquid_f_info.wait_response_time >= 50){
            s_ykcm_liquid_f_info.wait_response_time = 0x00;
            /** 发送事件，上报液冷故障信息 */
            ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                    0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_LIQUID_FAULT);
        }
    }else{
        s_ykcm_liquid_f_info.wait_response_time = 0x00;
    }
#else
    s_ykcm_liquid_f_info.info.wait_response = NET_ENUM_FALSE;
    s_ykcm_liquid_f_info.wait_response_time = 0x00;
#endif

    /************************************ 上锁超过一定时间不解锁时，强制解锁 ************************************/
    if(s_ykcm_liquid_f_info.info.is_lock == NET_ENUM_TRUE){
        if(s_ykcm_liquid_f_info.wait_unlock_time < (0xFF - 0x01)){
            s_ykcm_liquid_f_info.wait_unlock_time++;
        }
        /** 函数调用时基100ms，大概10s */
        if(s_ykcm_liquid_f_info.wait_unlock_time < 100){
            return;
        }
        /** 强制解锁 */
        s_ykcm_liquid_f_info.info.is_lock = NET_ENUM_FALSE;
        s_ykcm_liquid_f_info.wait_unlock_time = 0x00;
    }else{
        s_ykcm_liquid_f_info.wait_unlock_time = 0x00;
    }

    /************************************ 填写液冷故障信息 ************************************/
    for(uint8_t i = 0x00; (i < NET_YKC_MONITOR_LIQUID_F_INFO_MAX) && (i < liquid_num); i++){
        liquid_info = thaisenGetLiquidPara(i);
        s_ykcm_liquid_f_info.group[i].info.sequence = 0x00;
        s_ykcm_liquid_f_info.group[i].info.type = thaisenLiquid_get_LiquidDev();
        s_ykcm_liquid_f_info.group[i].f_value = liquid_info->state_flag.fault_code;
        /** 故障已变化 */
        if(s_ykcm_liquid_f_info.group[i].f_value != s_ykcm_liquid_f_info.group[i].f_value_last){
            s_ykcm_liquid_f_info.info.is_lock = NET_ENUM_TRUE;
        }
        /** 设备在线状态已发生变化 */
        else if(s_ykcm_liquid_f_info.group[i].info.is_offline != liquid_info->offlineflag){
            s_ykcm_liquid_f_info.info.is_lock = NET_ENUM_TRUE;
        }
        s_ykcm_liquid_f_info.group[i].info.is_offline = liquid_info->offlineflag;
        s_ykcm_liquid_f_info.group[i].f_value_last = s_ykcm_liquid_f_info.group[i].f_value;
    }
    /************************************ 液冷故障信息有变，发送事件上报液冷故障信息 ************************************/
    if(s_ykcm_liquid_f_info.info.is_lock == NET_ENUM_TRUE){
        s_ykcm_liquid_f_info.info.wait_response = NET_ENUM_TRUE;
        /** 发送事件，上报液冷故障信息 */
        ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
                0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_LIQUID_FAULT);
        /** 清除重发计时 */
        s_ykcm_liquid_f_info.wait_response_time = 0x00;
    }
}

/*************************************************
 * 函数名      ykc_monitor_message_padding_liquid_fault_info
 * 功能         组包：填充液冷故障信息
 * 参数         buf      缓存
 *       ilen    输入缓存长度
 *       olen    填写数据总长度
 * 返回         >=0：成功       <0：失败
 * **********************************************/
int8_t ykc_monitor_message_padding_liquid_fault_info(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    extern uint8_t thaisenGetLiquidNum(void);

    uint16_t total = sizeof(Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t) + 0x01;
    uint8_t liquid_num = thaisenGetLiquidNum();

    if(liquid_num > NET_YKC_MONITOR_LIQUID_F_INFO_MAX){
        liquid_num = NET_YKC_MONITOR_LIQUID_F_INFO_MAX;
    }
    total += (liquid_num *sizeof(struct liquid_f_segment));

    if(buf == NULL){
        return -0x01;
    }
    if(ilen < total){
        return -0x02;
    }

    Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t *message = (Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t*)buf;
    struct running_data_info *running_data = (struct running_data_info*)((uint8_t*)&(message->body.msg_version) + 0x01);
    struct liquid_f_segment *segment = (struct liquid_f_segment*)((uint8_t*)&(running_data->segment_num) + 0x01);

    memset(message, 0x00, sizeof(ilen));

    rt_enter_critical();

    running_data->segment_num = liquid_num;
    for(uint8_t i = 0x00; i< liquid_num; i++){
        switch(s_ykcm_liquid_f_info.group[i].info.type){
        case thaisenLiquidDev_YTND:
            segment[i].liquid_type = NETYKCM_LIQUID_TYPE_YTND;
            break;
        case thaisenLiquidDev_HL:
            segment[i].liquid_type = NETYKCM_LIQUID_TYPE_HL;
            break;
        case thaisenLiquidDev_ImmersionJGD:
            segment[i].liquid_type = NETYKCM_LIQUID_TYPE_JGD;
            break;
        case thaisenLiquidDev_TPS:
            segment[i].liquid_type = NETYKCM_LIQUID_TYPE_TBS;
            break;
        default:
            segment[i].liquid_type = NETYKCM_LIQUID_TYPE_SIZE;
            break;
        }
        segment[i].f_value = s_ykcm_liquid_f_info.group[i].f_value;
        segment[i].option.is_offline = s_ykcm_liquid_f_info.group[i].info.is_offline;
        segment[i].option.reserve = 0x00;
    }

    rt_exit_critical();

    message->body.info_type = NETYKCM_DEV_RUNNING_DATA_LIQUID_FAULT;
    message->body.option = 0x01;        /** 数据上报 */
    message->body.msg_version = 0x00;   /** 报文版本 */
    message->body.gunno = (0x00 + 0x01);
    memcpy(message->body.pile_number, g_ykc_monitor_preq_login.body.pile_number, NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT);
    /** 解锁 */
    s_ykcm_liquid_f_info.info.is_lock = NET_ENUM_FALSE;

    if(olen){
        *olen = total;
    }

    return 0x00;
}

#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
/******************************** 以下是外部调用触发 *******************************/
/******************************** 以下是外部调用触发 *******************************/

/*************************************************
 * 函数名      ykc_monitor_storage_thread_monitor_err_info
 * 功能          保存线程监控错误信息
 * 参数          name  错误线程名
 * 返回          >=0：成功          <0：失败
 * **********************************************/
int8_t ykc_monitor_storage_thread_monitor_err_info(char *name)
{
    ykc_monitor_storage_struct *config = (ykc_monitor_storage_struct*)(s_ykc_monitor_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT));
    if(config == NULL){
        return -0x01;
    }
    memset(config->reset_lable, 0x00, sizeof(config->reset_lable));
    if(name){
        if(strlen(name) > sizeof(config->reset_lable)){
            memcpy(config->reset_lable, name, sizeof(config->reset_lable));
        }else{
            memcpy(config->reset_lable, name, strlen(name));
        }
        config->flag.is_thread_error = NET_ENUM_TRUE;
    }else{
        config->flag.is_thread_error = NET_ENUM_FALSE;
    }

    return s_ykc_monitor_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_MONITOR_PLAT);
}


#endif /* NET_YKC_MONITOR_AS_MONITOR */

#endif /* NET_PACK_USING_YKC_MONITOR */
