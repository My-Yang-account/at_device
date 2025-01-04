/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */

#include "sgcc_message_padding.h"
#include "sgcc_message_send.h"
#include "sgcc_message_receive.h"
#include "sgcc_device_register.h"

#include "app_ofsm.h"
#include "app_billing_rule.h"
#include "net_operation.h"
#include "thaisen7102Public.h"

#define DBG_TAG "sgcc_rl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

#define SGCC_CHARGE_ELECT_MAX                      500000    /* 最大充电电量值(精度：0.001) */
#define SGCC_SPEND_AMOUNT_MAX                      5000000   /* 最大消费金额值(精度：0.0001) */

#define SGCC_STATE_PERIOD_CHARGING_FIRST_DEF       (60 *1000)      /* 启动充电时前1min 充电中实时数据上报间隔为5s  */
#define SGCC_STATE_INTERVAL_STARTING_DEF           (5 *1000)       /* 充电中实时数据上报间隔(启动前2min)  */

#define SGCC_STATE_INTERVAL_CHARGING_DEF           (90)            /* 充电中实时数据上报间隔(单位s)  */
#define SGCC_STATE_INTERVAL_NONCHARGING_DEF        (180)           /* 非充电中实时数据上报间隔(单位s)  */
#define SGCC_OAMMETER_VALUE_INTERVAL_DEF           (3600)          /* 电表底值上报间隔(单位s)  */
#define SGCC_BMS_DATA_INTERVAL_DEF                 (30)            /* BMS数据上报间隔(单位s)  */
#define SGCC_MONITOR_PROPERTY_INTERVAL_DEF         (600)           /* 实时监测属性上报间隔(单位s)  */
#define SGCC_FAULT_WARNNING_INTERVAL_DEF           (360)           /* 故障告警全信息上报间隔(单位s)  */
#define SGCC_AC_AMMETER_VALUE_INTERVAL_DEF         (3600)          /* 交流电表底值监测属性上报间隔(单位s)  */
#define SGCC_OFFLINE_CHARGE_TIME_DEF               (300)           /* 离线后可充电时长(单位s)  */
#define SGCC_GROUND_LOCK_INFO_INTERVAL_DEF         (3600)          /* 地锁监测上报间隔(单位s)  */
#define SGCC_DOOR_LOCK_INFO_INTERVAL_DEF           (3600)          /* 门锁监测上送频率上报间隔(单位s)  */

#define SGCC_ORDERLY_CHARGE_TIME_POINT_NUM         0x05            /* 有序充电时间点数 */

#define SGCC_BMS_DATA_REPORT_NUM_MAX               0x06            /* BMS数据上报次数 */
#define SGCC_REALTIME_PROCESS_THREAD_STACK_SIZE    1536            /* 实时处理线程栈大小 */

#pragma pack(1)

struct sgcc_state_info{
    struct{
        uint32_t connect : 4;
        uint32_t electlock : 4;
        uint32_t dck1 : 4;
        uint32_t dck2 : 4;
        uint32_t dc_plusFuse : 4;
        uint32_t dc_minusFuse : 4;
        uint32_t state : 5;
    }state;                                       /* 桩状态 */
    uint32_t timestamp;                           /* 时间 */
};

struct sgcc_flag_info{
    uint8_t is_start_charge : 1;                  /*  已启动充电 */
    uint8_t is_stop_charge : 1;                   /*  已停止充电 */
    uint8_t is_start_mergecharge : 1;             /*  已启动并充充电 */
    uint8_t is_set_power : 1;                     /*  已设置功率百分比 */
    uint8_t is_orderly_charge : 1;                /*  已设置有序充电 */

    uint8_t start_success : 1;                    /*  启机成功 */
    uint8_t stop_success : 1;                     /*  停机成功 */
    uint8_t mergestart_success : 1;               /*  并充启机成功 */
    uint8_t set_power_success : 1;                /*  设置功率百分比成功 */

    uint8_t omit_oammeter_val : 1;                /* 遗漏：上报电表底值 */
    uint8_t omit_bms_data : 1;                    /* 遗漏：上报BMS 数据 */
    uint8_t omit_monitor_property : 1;            /* 遗漏：上报实时监测属性数据 */
    uint8_t init_complete : 1;                    /* 初始化完成 */
};

struct sgcc_order_charge{
    uint8_t current_index;                                     /* 当前时间点下标 */
    uint32_t time_sec[SGCC_ORDERLY_CHARGE_TIME_POINT_NUM];     /* 有序充电时间点(s) */
    uint16_t period_power[SGCC_ORDERLY_CHARGE_TIME_POINT_NUM]; /* 有序充电时间段功率(0.1KW) */
};

#pragma pack()

NET_DEF_SRAM2 static uint8_t s_agcc_applycharge_result[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_agcc_applycharge_fault_reason[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_agcc_applycharge_fail_reason[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static uint8_t s_agcc_remotecharge_result[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_agcc_remotecharge_fault_reason[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_agcc_remotecharge_fail_reason[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static uint8_t s_agcc_remotestop_result[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_agcc_remotestop_fault_reason[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_agcc_remotestop_fail_reason[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static struct sgcc_order_charge s_sgcc_order_charge[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct sgcc_flag_info s_sgcc_flag_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct sgcc_state_info s_sgcc_state_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_sgcc_state_noncharging_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_sgcc_state_charging_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_sgcc_state_charging_interval[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_sgcc_state_charging_period_first[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_sgcc_oammeter_val_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_sgcc_faultwarn_count[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static uint32_t s_sgcc_bms_data_count[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint8_t s_sgcc_bms_data_report_num[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static uint32_t s_sgcc_monitor_property_count[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static uint16_t s_sgcc_charge_sn[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint8_t s_sgcc_operation_sn[NET_SYSTEM_GUN_NUMBER];

NET_DEF_SRAM2 static struct rt_thread s_sgcc_realtime_process_thread;
NET_DEF_SRAM0 static uint8_t s_sgcc_realtime_process_thread_stack[4096];

NET_DEF_SRAM2 static struct net_handle* s_sgcc_handle = NULL;

static uint8_t sgcc_chargepile_transaction_identity_converted(uint8_t identity, uint8_t online_order);
static uint16_t sgcc_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting);

/*******************************************************
 * 函数名               sgcc_enter_critical
 * 功能                  进入临界区
 * 参数
 * 返回
 ******************************************************/
static void sgcc_enter_critical(void)
{
    rt_enter_critical();
}

/*******************************************************
 * 函数名               sgcc_exit_critical
 * 功能                  退出临界区
 * 参数
 * 返回
 ******************************************************/
static void sgcc_exit_critical(void)
{
    rt_exit_critical();
}

/*************************************************
 * 函数名      sgcc_get_applycharge_result
 * 功能          获取申请充电启动结果
 * **********************************************/
uint8_t sgcc_get_applycharge_result(uint8_t gunno)
{
    return s_agcc_applycharge_result[gunno];
}

/*************************************************
 * 函数名      sgcc_get_applycharge_fault_reason
 * 功能          获取申请充电启动故障原因
 * **********************************************/
uint16_t sgcc_get_applycharge_fault_reason(uint8_t gunno)
{
    return s_agcc_applycharge_fault_reason[gunno];
}

/*************************************************
 * 函数名      sgcc_get_applycharge_fail_reason
 * 功能          获取申请充电启动失败原因
 * **********************************************/
uint16_t sgcc_get_applycharge_fail_reason(uint8_t gunno)
{
    return s_agcc_applycharge_fail_reason[gunno];
}

/*************************************************
 * 函数名      sgcc_get_remotecharge_result
 * 功能          获取远程启动结果
 * **********************************************/
uint8_t sgcc_get_remotecharge_result(uint8_t gunno)
{
    return s_agcc_remotecharge_result[gunno];
}

/*************************************************
 * 函数名      sgcc_get_remotecharge_fault_reason
 * 功能          获取远程启动故障原因
 * **********************************************/
uint16_t sgcc_get_remotecharge_fault_reason(uint8_t gunno)
{
    return s_agcc_remotecharge_fault_reason[gunno];
}

/*************************************************
 * 函数名      sgcc_get_remotecharge_fail_reason
 * 功能          获取远程启动失败原因
 * **********************************************/
uint16_t sgcc_get_remotecharge_fail_reason(uint8_t gunno)
{
    return s_agcc_remotecharge_fail_reason[gunno];
}

/*************************************************
 * 函数名      sgcc_get_remotestop_result
 * 功能          获取远程停止结果
 * **********************************************/
uint8_t sgcc_get_remotestop_result(uint8_t gunno)
{
    return s_agcc_remotestop_result[gunno];
}

/*************************************************
 * 函数名      sgcc_get_remotestop_fault_reason
 * 功能          获取远程停止故障原因
 * **********************************************/
uint16_t sgcc_get_remotestop_fault_reason(uint8_t gunno)
{
    return s_agcc_remotestop_fault_reason[gunno];
}

/*************************************************
 * 函数名      sgcc_get_remotestop_fail_reason
 * 功能          获取远程停止失败原因
 * **********************************************/
uint16_t sgcc_get_remotestop_fail_reason(uint8_t gunno)
{
    return s_agcc_remotestop_fail_reason[gunno];
}

/*************************************************
 * 函数名      sgcc_calculate_period_with_hm
 * 功能          根据传入的小时、分钟计算时段
 * **********************************************/
static uint8_t sgcc_calculate_period_with_hm(uint8_t hour, uint8_t min)
{
#define SGCC_PERIOD_MINITE    15           /* 一个时段15min */

    if((hour > 23) || (min >= 60)){
        return 0x00;
    }

    uint32_t second = hour *60 *60 + min *60;
    uint8_t period = second /(SGCC_PERIOD_MINITE *60);

#undef SGCC_PERIOD_MINITE
    return period;
}

/*************************************************
 * 函数名      sgcc_storage_data_check
 * 功能          校验存储的平台数据
 * **********************************************/
static void sgcc_storage_data_check(void)
{
    uint8_t verify_success = 0x01;
    System_BaseData *base = NULL;
    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));

    if(config == NULL){
        verify_success = 0x00;
    }else if((config->verify_result == 0x00) || (config->storage_init_flag != NET_SGCC_STORAGE_INIT_FLAG)){
        verify_success = 0x00;
    }else if(0/*((config->tip_elect_rate == 0x00) && (config->tip_service_rate == 0x00)) &&
            ((config->peak_elect_rate == 0x00) && (config->peak_service_rate == 0x00)) &&
            ((config->flat_elect_rate == 0x00) && (config->flat_service_rate == 0x00)) &&
            ((config->valley_elect_rate == 0x00) && (config->valley_service_rate == 0x00))*/){
        /** 对特定数据进行判断 */
        verify_success = 0x00;
    }

    config->dev_state = SGCC_DEV_STATE_COMMISSIONING;

    if(verify_success){
        evs_data_dev_configs.equipParamFreq = config->rt_property_interval;
        if(evs_data_dev_configs.equipParamFreq == 0x00){
            evs_data_dev_configs.equipParamFreq = SGCC_MONITOR_PROPERTY_INTERVAL_DEF;
        }

        evs_data_dev_configs.gunElecFreq = config->charging_property_interval;
        if(evs_data_dev_configs.gunElecFreq == 0x00){
            evs_data_dev_configs.gunElecFreq = SGCC_STATE_INTERVAL_CHARGING_DEF;
        }

        evs_data_dev_configs.nonElecFreq = config->noncharging_property_interval;
        if(evs_data_dev_configs.nonElecFreq == 0x00){
            evs_data_dev_configs.nonElecFreq = SGCC_STATE_INTERVAL_NONCHARGING_DEF;
        }

        evs_data_dev_configs.faultWarnings = config->fault_warning_interval;
        if(evs_data_dev_configs.faultWarnings == 0x00){
            evs_data_dev_configs.faultWarnings = SGCC_FAULT_WARNNING_INTERVAL_DEF;
        }

        evs_data_dev_configs.acMeterFreq = config->ac_meter_interval;
        if(evs_data_dev_configs.acMeterFreq == 0x00){
            evs_data_dev_configs.acMeterFreq = SGCC_AC_AMMETER_VALUE_INTERVAL_DEF;
        }

        evs_data_dev_configs.dcMeterFreq = config->dc_meter_interval;
        if(evs_data_dev_configs.dcMeterFreq == 0x00){
            evs_data_dev_configs.dcMeterFreq = SGCC_OAMMETER_VALUE_INTERVAL_DEF;
        }

        evs_data_dev_configs.offlinChaLen = config->offline_charge_time;
        if(evs_data_dev_configs.offlinChaLen == 0x00){
            evs_data_dev_configs.offlinChaLen = SGCC_OFFLINE_CHARGE_TIME_DEF;
        }

        evs_data_dev_configs.grndLock = config->groundlock_interval;
        if(evs_data_dev_configs.grndLock == 0x00){
            evs_data_dev_configs.grndLock = SGCC_GROUND_LOCK_INFO_INTERVAL_DEF;
        }

        evs_data_dev_configs.doorLock = config->doorlock_interval;
        if(evs_data_dev_configs.doorLock == 0x00){
            evs_data_dev_configs.doorLock = SGCC_DOOR_LOCK_INFO_INTERVAL_DEF;
        }

        evs_data_dev_configs.encodeCon = config->encode_con;

        if(config->dev_state == SGCC_DEV_STATE_REBOOT){
            config->dev_state = SGCC_DEV_STATE_COMMISSIONING;
        }

        /** 初始化计费模型请求 */
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            memset(evs_event_ask_feeModels[gunno].eleModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
            memcpy(evs_event_ask_feeModels[gunno].eleModelId, config->billing_rule.eleModelId, EVS_MAX_MODEL_ID_LEN);

            memset(evs_event_ask_feeModels[gunno].serModeId, 0x00, EVS_MAX_MODEL_ID_LEN);
            memcpy(evs_event_ask_feeModels[gunno].serModeId, config->billing_rule.serModelId, EVS_MAX_MODEL_ID_LEN);
        }

        sgcc_message_pro_billing_model_request_response(&config->billing_rule, sizeof(config->billing_rule), 0x01);
    }else{
        evs_data_dev_configs.equipParamFreq = SGCC_MONITOR_PROPERTY_INTERVAL_DEF;
        evs_data_dev_configs.gunElecFreq = SGCC_STATE_INTERVAL_CHARGING_DEF;
        evs_data_dev_configs.nonElecFreq = SGCC_STATE_INTERVAL_NONCHARGING_DEF;
        evs_data_dev_configs.faultWarnings = SGCC_FAULT_WARNNING_INTERVAL_DEF;
        evs_data_dev_configs.acMeterFreq = SGCC_AC_AMMETER_VALUE_INTERVAL_DEF;
        evs_data_dev_configs.dcMeterFreq = SGCC_OAMMETER_VALUE_INTERVAL_DEF;
        evs_data_dev_configs.offlinChaLen = SGCC_OFFLINE_CHARGE_TIME_DEF;
        evs_data_dev_configs.grndLock = SGCC_GROUND_LOCK_INFO_INTERVAL_DEF;
        evs_data_dev_configs.doorLock = SGCC_DOOR_LOCK_INFO_INTERVAL_DEF;
        evs_data_dev_configs.encodeCon = config->encode_con;
        config->dev_state = SGCC_DEV_STATE_COMMISSIONING;

        /** 初始化计费模型请求 */
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            memset(evs_event_ask_feeModels[gunno].eleModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
            memset(evs_event_ask_feeModels[gunno].serModeId, 0x00, EVS_MAX_MODEL_ID_LEN);
        }
    }

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
        base->device_state = config->dev_state;
    }

    sgcc_register_storage_struct(config);

    rt_kprintf("evs_data_dev_configs.rt_property_interval(%d)\n", evs_data_dev_configs.equipParamFreq);
    rt_kprintf("evs_data_dev_configs.charging_property_interval(%d)\n", evs_data_dev_configs.gunElecFreq);
    rt_kprintf("evs_data_dev_configs.noncharging_property_interval(%d)\n", evs_data_dev_configs.nonElecFreq);
    rt_kprintf("evs_data_dev_configs.fault_warning_interval(%d)\n", evs_data_dev_configs.faultWarnings);
    rt_kprintf("evs_data_dev_configs.ac_meter_interval(%d)\n", evs_data_dev_configs.acMeterFreq);
    rt_kprintf("evs_data_dev_configs.dc_meter_interval(%d)\n", evs_data_dev_configs.dcMeterFreq);
    rt_kprintf("evs_data_dev_configs.offline_charge_time(%d)\n", evs_data_dev_configs.offlinChaLen);
    rt_kprintf("evs_data_dev_configs.groundlock_interval(%d)\n", evs_data_dev_configs.grndLock);
    rt_kprintf("evs_data_dev_configs.doorlock_interval(%d)\n", evs_data_dev_configs.doorLock);
}

/*************************************************
 * 函数名      sgcc_response_padding_remote_start_charge
 * 功能          组包：远程启机响应
 * **********************************************/
int8_t sgcc_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    /** 来到这里一定是在线启动方式 */
    uint8_t data_len = sizeof(evs_event_startResult);

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
    evs_event_startResult *response = NULL;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    response = ((evs_event_startResult*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(evs_service_startCharges[gunno].preTradeNo);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(response->preTradeNo, evs_service_startCharges[gunno].preTradeNo, valid_len);

    valid_len = sizeof(evs_service_startCharges[gunno].tradeNo);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(response->tradeNo, evs_service_startCharges[gunno].tradeNo, valid_len);

    valid_len = sizeof(base->car_vin);
    valid_len = valid_len > EVS_MAX_CAR_VIN_LEN ? EVS_MAX_CAR_VIN_LEN : valid_len;
    memcpy(response->vinCode, base->car_vin, valid_len);

    response->gunNo = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      sgcc_response_padding_remote_stop_charge
 * 功能          组包：远程停机响应
 * **********************************************/
int8_t sgcc_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    /** 来到这里一定是在线启动方式且已启动成功 */
    uint8_t data_len = sizeof(evs_event_stopCharge);

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
    evs_event_stopCharge *response = NULL;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    response = ((evs_event_stopCharge*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(evs_service_stopCharges[gunno].preTradeNo);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    /** 平台停止 */
    if(base->reason_code == APP_SYSTEM_STOP_WAY_APP_STOP){
        memcpy(response->preTradeNo, evs_service_stopCharges[gunno].preTradeNo, valid_len);
    }else{
        /** 平台启动 */
        if(base->start_type == APP_CHARGE_START_WAY_APP){
            memcpy(response->preTradeNo, evs_service_startCharges[gunno].preTradeNo, valid_len);
        }
        /** 鉴权启动 */
        else{
            memcpy(response->preTradeNo, evs_service_authCharges[gunno].preTradeNo, valid_len);
        }
    }

    valid_len = sizeof(evs_service_stopCharges[gunno].tradeNo);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    /** 平台停止 */
    if(base->reason_code == APP_SYSTEM_STOP_WAY_APP_STOP){
        memcpy(response->tradeNo, evs_service_stopCharges[gunno].tradeNo, valid_len);
    }else{
        /** 平台启动 */
        if(base->start_type == APP_CHARGE_START_WAY_APP){
            memcpy(response->tradeNo, evs_service_startCharges[gunno].tradeNo, valid_len);
        }
        /** 鉴权启动 */
        else{
            memcpy(response->tradeNo, evs_service_authCharges[gunno].tradeNo, valid_len);
        }
    }

    response->gunNo = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      sgcc_response_padding_apply_charge_result
 * 功能          组包：申请充电启动结果响应
 * **********************************************/
int8_t sgcc_response_padding_apply_charge_result(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(evs_event_startResult);

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
    evs_event_startResult *response = NULL;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    response = ((evs_event_startResult*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(evs_service_authCharges[gunno].preTradeNo);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(response->preTradeNo, evs_service_authCharges[gunno].preTradeNo, valid_len);

    valid_len = sizeof(evs_service_authCharges[gunno].tradeNo);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(response->tradeNo, evs_service_authCharges[gunno].tradeNo, valid_len);

    valid_len = sizeof(base->car_vin);
    valid_len = valid_len > EVS_MAX_CAR_VIN_LEN ? EVS_MAX_CAR_VIN_LEN : valid_len;
    memcpy(response->vinCode, base->car_vin, valid_len);

    response->gunNo = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      sgcc_response_padding_elock_ctrl_result
 * 功能          组包：电子锁控制结果响应
 * **********************************************/
int8_t sgcc_response_padding_elock_ctrl_result(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(evs_event_startResult);

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
    evs_event_startResult *response = NULL;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    response = ((evs_event_startResult*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(base->transaction_number);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(response->preTradeNo, base->transaction_number, valid_len);

    valid_len = sizeof(base->device_transaction_number);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(response->tradeNo, base->device_transaction_number, valid_len);

    valid_len = sizeof(base->car_vin);
    valid_len = valid_len > EVS_MAX_CAR_VIN_LEN ? EVS_MAX_CAR_VIN_LEN : valid_len;
    memcpy(response->vinCode, base->car_vin, valid_len);

    response->gunNo = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}




#ifdef NET_SGCC_PRO_USING_DC
/*************************************************
 * 函数名      sgcc_message_pro_remote_start_charge_request
 * 功能          处理服务器下发的远程启机
 * **********************************************/
int8_t sgcc_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len, uint16_t *reason)
{
    uint8_t data_len = sizeof(evs_service_startCharge);

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
        if(reason){
            *reason = NETSGCC_DCA_REASON7007_BILLING_RULE_INVALID;
        }
        return NET_SGCC_START_RESULT_FAULTING;
    }

    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_startCharge *request = (evs_service_startCharge*)data;

    if(evs_event_pile_stutus_changes[gunno].yxOccurTime != request->insertGunTime){
        if(reason){
            *reason = NETSGCC_DCA_REASON7008_INSERT_GUN_TIME;
        }
        LOG_D("gunno(%d) sgcc insert gun time not match(%d, %d)", gunno, evs_event_pile_stutus_changes[gunno].yxOccurTime, request->insertGunTime);
        return NET_SGCC_START_RESULT_FAULTING;
    }

    /** 并充时不能让平台启动副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if(reason){
            *reason = 0x00;
        }
        return NET_SGCC_START_RESULT_CHARGING;
    }

    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
        if(reason){
            *reason = NETSGCC_DCA_REASON3057_CHARGE_LINK;
        }
        return NET_SGCC_START_RESULT_DISCONNECT;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
        memset(base->card_number, 0x00, sizeof(base->card_number));
        memset(base->card_uid, 0x00, sizeof(base->card_uid));
        memset(base->user_number, 0x00, sizeof(base->user_number));

        valid_len = sizeof(base->transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
        memcpy(base->transaction_number, request->preTradeNo, valid_len);

        valid_len = sizeof(base->device_transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(base->device_transaction_number, 0x00, sizeof(base->device_transaction_number));
        memcpy(base->device_transaction_number, request->tradeNo, valid_len);

        base->account_ballance_before = 0x00;
        base->account_ballance_after = 0x00;
        base->offline_chargetime = evs_data_dev_configs.offlinChaLen;

        switch(request->chargeMode){
        case SGCC_CHARGE_MODE_FULL:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = 0x00;
            break;
        case SGCC_CHARGE_MODE_MONEY:
            base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
            base->charge_strategy_para = request->limitData *100;
            base->account_ballance_before = request->limitData;
            base->account_ballance_after = request->limitData;
            break;
        case SGCC_CHARGE_MODE_ELECT:
            base->charge_strategy = APP_CHARGE_STRATEGY_ELECT;
            base->charge_strategy_para = request->limitData *100;
            break;
        case SGCC_CHARGE_MODE_SOC:
            base->charge_strategy = APP_CHARGE_STRATEGY_SOC;
            base->charge_strategy_para = request->limitData;
            break;
        case SGCC_CHARGE_MODE_TIME:
            base->charge_strategy = APP_CHARGE_STRATEGY_TIME;
            base->charge_strategy_para = request->limitData *60;
            break;
#if 0
        case SGCC_CHARGE_MODE_POWER:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = 0x00;
            break;
#endif
        default:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = 0x00;
            break;
        }

        s_sgcc_flag_info[gunno].is_start_charge = NET_ENUM_TRUE;

        if(reason){
            *reason = 0x00;
        }
        return NET_SGCC_START_RESULT_SUCCESS;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        if(reason){
            *reason = 0x00;
        }
        return NET_SGCC_START_RESULT_CHARGING;
        break;
    default:
        return NET_SGCC_START_RESULT_FAULTING;   /* 目前故障状态不允许充电 */
        break;
    }

    if(reason){
        *reason = 0x00;
    }
    return NET_SGCC_START_RESULT_SUCCESS;
}
#else
/*************************************************
 * 函数名      sgcc_message_pro_remote_start_charge_request
 * 功能          处理服务器下发的远程启机
 * **********************************************/
int8_t sgcc_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_startCharge);

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
        if(reason){
            *reason = NETSGCC_ACA_REASON6000_BILLING_RULE_INVALID;
        }
        return NET_SGCC_START_RESULT_FAULTING;
    }

    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_startCharge *request = (evs_service_startCharge*)data;

    if(evs_event_pile_stutus_changes[gunno].yxOccurTime != request->insertGunTime){
        if(reason){
            *reason = NETSGCC_ACA_REASON6001_INSERT_GUN_TIME;
        }
        LOG_D("gunno(%d) sgcc insert gun time not match(%d, %d)", gunno, evs_event_pile_stutus_changes[gunno].yxOccurTime, request->insertGunTime);
        return NET_SGCC_START_RESULT_FAULTING;
    }

    /** 并充时不能让平台启动副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if(reason){
            *reason = 0x00;
        }
        return NET_SGCC_START_RESULT_CHARGING;
    }

    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
        if(reason){
            *reason = NETSGCC_ACA_REASON3014_CHARGE_LINK;
        }
        return NET_SGCC_START_RESULT_DISCONNECT;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_FINISHING:
        memset(base->card_number, 0x00, sizeof(base->card_number));
        memset(base->card_uid, 0x00, sizeof(base->card_uid));
        memset(base->user_number, 0x00, sizeof(base->user_number));

        valid_len = sizeof(base->transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
        memcpy(base->transaction_number, request->preTradeNo, valid_len);

        valid_len = sizeof(base->device_transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(base->device_transaction_number, 0x00, sizeof(base->device_transaction_number));
        memcpy(base->device_transaction_number, request->tradeNo, valid_len);

        base->account_balance = 0x00;
        base->offline_chargetime = evs_data_dev_configs.offlinChaLen;

        switch(request->chargeMode){
        case SGCC_CHARGE_MODE_FULL:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = 0x00;
            break;
        case SGCC_CHARGE_MODE_MONEY:
            base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
            base->charge_strategy_para = request->limitData *100;
            break;
        case SGCC_CHARGE_MODE_ELECT:
            base->charge_strategy = APP_CHARGE_STRATEGY_ELECT;
            base->charge_strategy_para = request->limitData *100;
            break;
        case SGCC_CHARGE_MODE_SOC:
            base->charge_strategy = APP_CHARGE_STRATEGY_SOC;
            base->charge_strategy_para = request->limitData;
            break;
        case SGCC_CHARGE_MODE_TIME:
            base->charge_strategy = APP_CHARGE_STRATEGY_TIME;
            base->charge_strategy_para = request->limitData *60;
            break;
#if 0
        case SGCC_CHARGE_MODE_POWER:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = 0x00;
            break;
#endif
        default:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = 0x00;
            break;
        }

        s_sgcc_flag_info[gunno].is_start_charge = NET_ENUM_TRUE;

        if(reason){
            *reason = 0x00;
        }
        return NET_SGCC_START_RESULT_SUCCESS;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        if(reason){
            *reason = 0x00;
        }
        return NET_SGCC_START_RESULT_CHARGING;
        break;
    default:
        return NET_SGCC_START_RESULT_FAULTING;   /* 目前故障状态不允许充电 */
        break;
    }

    if(reason){
        *reason = 0x00;
    }
    return NET_SGCC_START_RESULT_SUCCESS;
}
#endif /* NET_SGCC_PRO_USING_DC */

#ifdef NET_SGCC_PRO_USING_DC
/*************************************************
 * 函数名      sgcc_message_pro_remote_stop_charge_request
 * 功能          处理服务器下发的远程停机
 * **********************************************/
int8_t sgcc_message_pro_remote_stop_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_stopCharge);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    uint8_t stop_authorize_success = NET_ENUM_TRUE;

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_stopCharge *request = (evs_service_stopCharge*)data;

    /** 并充时不能让平台停止副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if(gunno != base->main_gunno){
            return 0x01;
        }
    }
    if((base->state.current != APP_OFSM_STATE_CHARGING) &&
            (base->state.current != APP_OFSM_STATE_STARTING)){
        stop_authorize_success = NET_ENUM_FALSE;
    }
    if(memcmp(base->device_transaction_number, request->tradeNo, EVS_MAX_TRADE_LEN)){
        stop_authorize_success = NET_ENUM_FALSE;
    }
    if(memcmp(base->transaction_number, request->preTradeNo, EVS_MAX_TRADE_LEN)){
        stop_authorize_success = NET_ENUM_FALSE;
    }

    if(stop_authorize_success == NET_ENUM_TRUE){
        s_sgcc_flag_info[gunno].is_stop_charge = NET_ENUM_TRUE;

        if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
            net_operation_set_event(base->main_gunno, NET_OPERATION_EVENT_STOP_CHARGE);
        }else{
            net_operation_set_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
        }

        return 0x00;
    }

    return 0x02;
}
#else
/*************************************************
 * 函数名      sgcc_message_pro_remote_stop_charge_request
 * 功能          处理服务器下发的远程停机
 * **********************************************/
int8_t sgcc_message_pro_remote_stop_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_stopCharge);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    uint8_t stop_authorize_success = NET_ENUM_TRUE;

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_stopCharge *request = (evs_service_stopCharge*)data;

    /** 并充时不能让平台停止副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if(gunno != base->main_gunno){
            return 0x01;
        }
    }
    if((base->state.current != APP_OFSM_STATE_CHARGING) &&
            (base->state.current != APP_OFSM_STATE_STARTING)){
        stop_authorize_success = NET_ENUM_FALSE;
    }
    if(memcmp(base->device_transaction_number, request->tradeNo, EVS_MAX_TRADE_LEN)){
        stop_authorize_success = NET_ENUM_FALSE;
    }
    if(memcmp(base->transaction_number, request->preTradeNo, EVS_MAX_TRADE_LEN)){
        stop_authorize_success = NET_ENUM_FALSE;
    }

    if(stop_authorize_success == NET_ENUM_TRUE){
        s_sgcc_flag_info[gunno].is_stop_charge = NET_ENUM_TRUE;
        return 0x00;
    }

    return 0x02;
}
#endif /* NET_SGCC_PRO_USING_DC */

/*************************************************
 * 函数名      sgcc_message_pro_apply_charge_response
 * 功能          处理服务器响应的申请鉴权充电响应
 * **********************************************/
int8_t sgcc_message_pro_apply_charge_response(uint8_t gunno, void *data, uint16_t len)
{
    uint16_t data_len = sizeof(evs_service_authCharge);

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
        return 0x01;
    }

    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_authCharge *response = (evs_service_authCharge*)data;

    if(evs_event_startCharges[gunno].startType == SGCC_OPSCTL_SILENT){
        if(memcmp(evs_event_startCharges[gunno].authCode, base->car_vin, sizeof(base->car_vin))){
            return 0x06;
        }
    }else{
#if 0
        if(memcmp(evs_event_startCharges[gunno].authCode, base->car_vin, sizeof(base->car_vin))){
            return 0x06;
        }
#endif
        return 0x06;
    }
    if(evs_event_pile_stutus_changes[gunno].yxOccurTime != response->insertGunTime){
        LOG_D("gunno(%d) sgcc insert gun time not match(%d, %d)", gunno, evs_event_pile_stutus_changes[gunno].yxOccurTime, response->insertGunTime);
        return 0x07;
    }

    /** 并充时不能让平台启动副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        return 0x02;
    }

//    memset(base->card_number, 0x00, sizeof(base->card_number));
//    memset(base->card_uid, 0x00, sizeof(base->card_uid));
    memset(base->user_number, 0x00, sizeof(base->user_number));

    valid_len = sizeof(base->transaction_number);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
    memcpy(base->transaction_number, response->preTradeNo, valid_len);

    valid_len = sizeof(base->device_transaction_number);
    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memset(base->device_transaction_number, 0x00, sizeof(base->device_transaction_number));
    memcpy(base->device_transaction_number, response->tradeNo, valid_len);

    base->account_ballance_before = 0x00;
    base->account_ballance_after = 0x00;
    base->offline_chargetime = evs_data_dev_configs.offlinChaLen;

    switch(response->chargeMode){
    case SGCC_CHARGE_MODE_FULL:
        base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
        base->charge_strategy_para = 0x00;
        break;
    case SGCC_CHARGE_MODE_MONEY:
        base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
        base->charge_strategy_para = response->limitData *100;
        base->account_ballance_before = response->limitData;
        base->account_ballance_after = response->limitData;
        break;
    case SGCC_CHARGE_MODE_ELECT:
        base->charge_strategy = APP_CHARGE_STRATEGY_ELECT;
        base->charge_strategy_para = response->limitData *100;
        break;
    case SGCC_CHARGE_MODE_SOC:
        base->charge_strategy = APP_CHARGE_STRATEGY_SOC;
        base->charge_strategy_para = response->limitData;
        break;
    case SGCC_CHARGE_MODE_TIME:
        base->charge_strategy = APP_CHARGE_STRATEGY_TIME;
        base->charge_strategy_para = response->limitData *60;
        break;
#if 0
    case SGCC_CHARGE_MODE_POWER:
        base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
        base->charge_strategy_para = 0x00;
        break;
#endif
    default:
        base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
        base->charge_strategy_para = 0x00;
        break;
    }

    s_sgcc_flag_info[gunno].is_start_charge = NET_ENUM_TRUE;

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_billing_model_request_response
 * 功能          处理服务器响应(下发)的计费模型
 * **********************************************/
int8_t sgcc_message_pro_billing_model_request_response(void *data, uint8_t len, uint8_t is_init)
{
    uint8_t data_len = sizeof(evs_service_issue_feeModel);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    System_BaseData *base = NULL;
    evs_service_issue_feeModel *request = (evs_service_issue_feeModel*)data;
    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));

    if(is_init == 0x00){
        if(config == NULL){
            return -0x03;
        }
        config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
        memcpy(&config->billing_rule, data, sizeof(evs_service_issue_feeModel));

        if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
            return -0x03;
        }
    }

    for(uint8_t _gunno = 0x00; _gunno < NET_SYSTEM_GUN_NUMBER; _gunno++){
        uint8_t gunno = _gunno;
        uint8_t hour = 0x00, min = 0x00, start_period = 0x00, end_period = 0x00, rate_type = 0x00, time_val[0x03];
        base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
        if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING) ||
                (base->state.current == APP_OFSM_STATE_STOPING)){
            net_operation_set_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
            gunno = NET_SYSTEM_GUN_NUMBER;
        }

        LOG_D("sgcc gunno(%d) billingrule info[%s][%s]", gunno, request->eleModelId, request->serModelId);
        LOG_D("ter(%d) tsr(%d) per(%d) psr(%d) fer(%d) fsr(%d) ver(%d) vsr(%d)", request->chargeFee[APP_RATE_TYPE_SHARP],
                request->serviceFee[APP_RATE_TYPE_SHARP], request->chargeFee[APP_RATE_TYPE_PEAK], request->serviceFee[APP_RATE_TYPE_PEAK],
                request->chargeFee[APP_RATE_TYPE_FLAT], request->serviceFee[APP_RATE_TYPE_FLAT], request->chargeFee[APP_RATE_TYPE_VALLEY],
                request->serviceFee[APP_RATE_TYPE_VALLEY]);

        app_billingrule_set_elect_model_sn(gunno, (uint8_t*)(request->eleModelId), sizeof(request->eleModelId));
        app_billingrule_set_service_model_sn(gunno, (uint8_t*)(request->serModelId), sizeof(request->serModelId));

        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_SHARP, (request->chargeFee[APP_RATE_TYPE_SHARP] + request->serviceFee[APP_RATE_TYPE_SHARP]));
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_PEAK, (request->chargeFee[APP_RATE_TYPE_PEAK] + request->serviceFee[APP_RATE_TYPE_PEAK]));
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_FLAT, (request->chargeFee[APP_RATE_TYPE_FLAT] + request->serviceFee[APP_RATE_TYPE_FLAT]));
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_VALLEY, (request->chargeFee[APP_RATE_TYPE_VALLEY] + request->serviceFee[APP_RATE_TYPE_VALLEY]));

        for(uint8_t segment = 0x00; segment < request->TimeNum; segment++){
            memset(time_val, 0x00, sizeof(time_val));
            time_val[0x00] = request->TimeSeg[segment][0x00];
            time_val[0x01] = request->TimeSeg[segment][0x01];
            hour = atoi((char*)time_val);

            memset(time_val, 0x00, sizeof(time_val));
            time_val[0x00] = request->TimeSeg[segment][0x02];
            time_val[0x01] = request->TimeSeg[segment][0x03];
            min = atoi((char*)time_val);

            start_period = sgcc_calculate_period_with_hm(hour, min);

            LOG_D("start_period[%d, %d, %d]\n", hour, min, start_period);

            if((segment + 0x01) >= request->TimeNum){
                end_period = APP_BILLING_RULE_PERIOD_MAX;
            }else{
                memset(time_val, 0x00, sizeof(time_val));
                time_val[0x00] = request->TimeSeg[segment + 0x01][0x00];
                time_val[0x01] = request->TimeSeg[segment + 0x01][0x01];
                hour = atoi((char*)time_val);

                memset(time_val, 0x00, sizeof(time_val));
                time_val[0x00] = request->TimeSeg[segment + 0x01][0x02];
                time_val[0x01] = request->TimeSeg[segment + 0x01][0x03];
                min = atoi((char*)time_val);

                end_period = sgcc_calculate_period_with_hm(hour, min);
            }

            LOG_D("end_period[%d, %d, %d]\n", hour, min, end_period);

            rate_type = APP_RATE_TYPE_VALLEY;
            if((request->SegFlag[segment] >= 10) && (request->SegFlag[segment] <= 13)){
                rate_type = request->SegFlag[segment] - 10;
            }

            for(uint8_t period = start_period; period < end_period; period++){
                LOG_D("[%d, %d, %d, %d, %d, %d]\n", gunno, segment, period, start_period, end_period, rate_type);
                app_billingrule_set_period_rate_number(gunno, period, rate_type);
                switch(rate_type){
                case APP_RATE_TYPE_SHARP :
                    app_billingrule_set_period_elect_price(gunno, period, request->chargeFee[APP_RATE_TYPE_SHARP]);
                    app_billingrule_set_period_service_price(gunno, period, request->serviceFee[APP_RATE_TYPE_SHARP]);
                    app_billingrule_set_period_delay_price(gunno, period, 0x00);
                    break;
                case APP_RATE_TYPE_PEAK :
                    app_billingrule_set_period_elect_price(gunno, period, request->chargeFee[APP_RATE_TYPE_PEAK]);
                    app_billingrule_set_period_service_price(gunno, period, request->serviceFee[APP_RATE_TYPE_PEAK]);
                    app_billingrule_set_period_delay_price(gunno, period, 0x00);
                    break;
                case APP_RATE_TYPE_FLAT :
                    app_billingrule_set_period_elect_price(gunno, period, request->chargeFee[APP_RATE_TYPE_FLAT]);
                    app_billingrule_set_period_service_price(gunno, period, request->serviceFee[APP_RATE_TYPE_FLAT]);
                    app_billingrule_set_period_delay_price(gunno, period, 0x00);
                    break;
                case APP_RATE_TYPE_VALLEY :
                    app_billingrule_set_period_elect_price(gunno, period, request->chargeFee[APP_RATE_TYPE_VALLEY]);
                    app_billingrule_set_period_service_price(gunno, period, request->serviceFee[APP_RATE_TYPE_VALLEY]);
                    app_billingrule_set_period_delay_price(gunno, period, 0x00);
                    break;
                default:
                    break;
                }
            }
        }
    }
    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_config_update_request
 * 功能          处理服务器下发的配置更新请求
 * **********************************************/
int8_t sgcc_message_pro_config_update_request(void *data, uint16_t len)
{
    uint16_t data_len = sizeof(evs_data_dev_config);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    evs_data_dev_config *request = (evs_data_dev_config*)data;

    if(config == NULL){
        return -0x03;
    }

    config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;

    config->rt_property_interval = request->equipParamFreq;
    config->charging_property_interval = request->gunElecFreq;
    config->noncharging_property_interval = request->nonElecFreq;
    config->fault_warning_interval = request->faultWarnings;
    config->ac_meter_interval = request->acMeterFreq *60;
    config->dc_meter_interval = request->dcMeterFreq *60;
    config->offline_charge_time = request->offlinChaLen *60;
    config->groundlock_interval = request->grndLock *60;
    config->doorlock_interval = request->doorLock *60;
    config->encode_con = request->encodeCon;

    LOG_D("config->rt_property_interval(%d)", config->rt_property_interval);
    LOG_D("config->charging_property_interval(%d)", config->charging_property_interval);
    LOG_D("config->noncharging_property_interval(%d)", config->noncharging_property_interval);
    LOG_D("config->fault_warning_interval(%d)", config->fault_warning_interval);
    LOG_D("config->ac_meter_interval(%d)", config->ac_meter_interval);
    LOG_D("config->dc_meter_interval(%d)", config->dc_meter_interval);
    LOG_D("config->offline_charge_time(%d)", config->offline_charge_time);
    LOG_D("config->groundlock_interval(%d)", config->groundlock_interval);
    LOG_D("config->doorlock_interval(%d)", config->doorlock_interval);
    LOG_D("config->encode_con(%d)", config->encode_con);
    LOG_D("config->qrCode(%s)", request->qrCode[0x00]);

    if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_QRCODE, (uint8_t*)(request->qrCode[0x00]), strlen((char*)request->qrCode[0x00]), NET_SYSTEM_DATA_OPTION_PLAT_SGCC) < 0x00){
        config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
        return -0x04;
    }

    if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
        config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
        return -0x05;
    }

    evs_data_dev_configs.equipParamFreq = config->rt_property_interval;
    evs_data_dev_configs.gunElecFreq = config->charging_property_interval;
    evs_data_dev_configs.nonElecFreq = config->noncharging_property_interval;
    evs_data_dev_configs.faultWarnings = config->fault_warning_interval;
    evs_data_dev_configs.acMeterFreq = config->ac_meter_interval;
    evs_data_dev_configs.dcMeterFreq = config->dc_meter_interval;
    evs_data_dev_configs.offlinChaLen = config->offline_charge_time;
    evs_data_dev_configs.grndLock = config->groundlock_interval;
    evs_data_dev_configs.doorLock = config->doorlock_interval;
    evs_data_dev_configs.encodeCon = config->encode_con;

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_config_query_request
 * 功能          处理服务器下发的配置查询请求
 * **********************************************/
int8_t sgcc_message_pro_config_query_request(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = sizeof(evs_data_dev_config);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    uint16_t valid_len = 0x00;
    uint8_t *qrcode = s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_QRCODE, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT);
    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    evs_data_dev_config *vector = (evs_data_dev_config*)buf;

    if(config == NULL){
        return -0x03;
    }
    vector->equipParamFreq = config->rt_property_interval;
    vector->gunElecFreq = config->charging_property_interval;
    vector->nonElecFreq = config->noncharging_property_interval;
    vector->faultWarnings = config->fault_warning_interval;
    vector->acMeterFreq = config->ac_meter_interval /60;
    vector->dcMeterFreq = config->dc_meter_interval /60;
    vector->offlinChaLen = config->offline_charge_time /60;
    vector->grndLock = config->groundlock_interval /60;
    vector->doorLock = config->doorlock_interval /60;
    vector->encodeCon = config->encode_con;
    memset(vector->qrCode, 0x00, sizeof(vector->qrCode));

    LOG_D("vector->rt_property_interval(%d)", vector->equipParamFreq);
    LOG_D("vector->charging_property_interval(%d)", vector->gunElecFreq);
    LOG_D("vector->noncharging_property_interval(%d)", vector->nonElecFreq);
    LOG_D("vector->fault_warning_interval(%d)", vector->faultWarnings);
    LOG_D("vector->ac_meter_interval(%d)", vector->acMeterFreq);
    LOG_D("vector->dc_meter_interval(%d)", vector->dcMeterFreq);
    LOG_D("vector->offline_charge_time(%d)", vector->offlinChaLen);
    LOG_D("vector->groundlock_interval(%d)", vector->grndLock);
    LOG_D("vector->doorlock_interval(%d)", vector->doorLock);

    if(qrcode == NULL){
        return -0x04;
    }

    valid_len = strlen((char*)qrcode);
    valid_len = valid_len > EVS_MAX_QRCODE_LEN ? EVS_MAX_QRCODE_LEN : valid_len;
    memcpy(vector->qrCode[0x00], qrcode, valid_len);

    LOG_D("vector->qrCode(%s)", vector->qrCode[0x00]);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_ctrl_elock_request
 * 功能          处理服务器下发的电子锁控制
 * **********************************************/
int8_t sgcc_message_pro_ctrl_elock_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_lockCtrl);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_lockCtrl *request = (evs_service_lockCtrl*)data;

    switch(base->state.current){
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
    case APP_OFSM_STATE_STOPING:
        return 0x01;
        break;
    default:
        if(request->lockParam == SGCC_OPSCTL_ACTION){
            s_sgcc_handle->system_control(gunno, NET_SYSTEM_CTRL_ITEM_ELOCK, NULL, NET_SYSTEM_CTRL_OPTION_ACTION);
            if(s_sgcc_handle->system_control(gunno, NET_SYSTEM_CTRL_ITEM_ELOCK, NULL, NET_SYSTEM_CTRL_OPTION_STATE) != NET_SYSTEM_CTRL_STATE_ACTION){
                return 0x02;
            }
        }else{
            s_sgcc_handle->system_control(gunno, NET_SYSTEM_CTRL_ITEM_ELOCK, NULL, NET_SYSTEM_CTRL_OPTION_RELEASE);
            if(s_sgcc_handle->system_control(gunno, NET_SYSTEM_CTRL_ITEM_ELOCK, NULL, NET_SYSTEM_CTRL_OPTION_STATE) != NET_SYSTEM_CTRL_STATE_RELEASE){
                return 0x03;
            }
        }
        break;
    }

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_dev_maintain_request
 * 功能          处理服务器下发的设备维护请求
 * **********************************************/
int8_t sgcc_message_pro_dev_maintain_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_dev_maintain);

    if(data == NULL){
        return 12;
    }

    if(data_len > len){
        return 12;
    }

    uint8_t i = 0x00;
    evs_service_dev_maintain *request = (evs_service_dev_maintain*)data;
    System_BaseData *base = NULL;

    for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
        base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
        if((base->state.current == APP_OFSM_STATE_STARTING) || (base->state.current == APP_OFSM_STATE_CHARGING)){
            break;
        }
    }
    if(i < NET_SYSTEM_GUN_NUMBER){
        LOG_W("sgcc gunno(%d) is charging, forbid to maintain chargepile", i);
        return 11;
    }

    switch(request->ctrlType){
    case SGCC_DEV_STATE_REBOOT:
    {
        for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
        if(i == NET_SYSTEM_GUN_NUMBER){
            sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
            config->dev_state = SGCC_DEV_STATE_REBOOT;

            if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
                config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
                return 12;
            }

            net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
            return 10;
        }
    }
        break;
    case SGCC_DEV_STATE_OVERHAUL:
    {
        for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
        if(i == NET_SYSTEM_GUN_NUMBER){
            sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
            config->dev_state = SGCC_DEV_STATE_OVERHAUL;

            if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
                config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
                return 12;
            }
            for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
                base->device_state = APP_DEVICE_STATE_OVERHAUL;
            }
            return 10;
        }
    }
        break;
    case SGCC_DEV_STATE_FREEZE:
    {
        for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
        if(i == NET_SYSTEM_GUN_NUMBER){
            sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
            config->dev_state = SGCC_DEV_STATE_FREEZE;

            if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
                config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
                return 12;
            }
            for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
                base->device_state = APP_DEVICE_STATE_FREEZE;
            }
            return 10;
        }
    }
        break;
    case SGCC_DEV_STATE_COMMISSIONING:
    {
        sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
        config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
        config->dev_state = SGCC_DEV_STATE_COMMISSIONING;

        if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
            return 12;
        }
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
            base->device_state = APP_DEVICE_STATE_COMMISSIONING;
        }
        return 10;
    }
        break;
    case SGCC_DEV_STATE_OUTAGE:
    {
        for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
        if(i == NET_SYSTEM_GUN_NUMBER){
            sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
            config->dev_state = SGCC_DEV_STATE_OUTAGE;

            if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
                config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
                return 12;
            }
            for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
                base->device_state = APP_DEVICE_STATE_OUTAGE;
            }
            return 10;
        }
    }
        break;
    case SGCC_DEV_STATE_RETURNS:
    {
        for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
        if(i == NET_SYSTEM_GUN_NUMBER){
            sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
            config->dev_state = SGCC_DEV_STATE_RETURNS;

            if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
                config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
                return 12;
            }
            for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
                base->device_state = APP_DEVICE_STATE_RETURNS;
            }
            return 10;
        }
    }
        break;
    case SGCC_DEV_STATE_RESTORE_SRTTING:
    {
        for(i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            base = (System_BaseData*)(s_sgcc_handle->get_base_data(i));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
        if(i == NET_SYSTEM_GUN_NUMBER){
            sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
            config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG;
            config->dev_state = SGCC_DEV_STATE_RESTORE_SRTTING;

            memset(config->device_secret, 0x0, sizeof(config->device_secret));
            memset(config->device_name, 0x0, sizeof(config->device_name));
            memset(config->product_key, 0x0, sizeof(config->product_key));

            if(s_sgcc_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
                config->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
                return 12;
            }

            net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
            return 10;
        }
    }
        break;
    default:
        return 12;
        break;
    }

    return 12;
}

/*************************************************
 * 函数名      sgcc_message_pro_query_maintain_info_request
 * 功能          处理服务器下发的查询设备维护信息请求
 * **********************************************/
int8_t sgcc_message_pro_query_maintain_info_request(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(evs_service_feedback_maintain_query);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x03;
    }

    evs_service_feedback_maintain_query *request = (evs_service_feedback_maintain_query*)buf;
    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    request->ctrlType = config->dev_state;
    request->result = 10;

    if(olen){
        *olen = data_len;
    }

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_time_sync_request
 * 功能          处理服务器下发的时间同步请求
 * **********************************************/
int8_t sgcc_message_pro_time_sync_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(uint32_t);

    if(data == NULL){
        return -0x01;
    }

    if(data_len > len){
        return -0x03;
    }

    struct tm *_tm = NULL;
    uint32_t timestamp;

    LOG_D("Sys now timestamp is:%d, set time strcmp is:%d", time(NULL), *(uint32_t*)data);
    s_sgcc_handle->time_sync(*(uint32_t*)data);

    net_operation_set_event(0x00, NET_OPERATION_EVENT_TIME_SYNC);

    timestamp = time(NULL);

    sgcc_enter_critical();
    _tm = localtime((const time_t*)&(timestamp));
    LOG_D("set time ok, now time is %d-%02d-%02d %02d:%02d:%02d\n", _tm->tm_year + 1900, _tm->tm_mon + 1, _tm->tm_mday, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
    sgcc_exit_critical();

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_reservation_request
 * 功能          处理服务器下发的预约请求
 * **********************************************/
int8_t sgcc_message_pro_reservation_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_rsvCharge);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_rsvCharge *request = (evs_service_rsvCharge*)data;

    if(request->appomathod == 11){
        if(base->state.current == APP_OFSM_STATE_RESERVATION){
            net_operation_set_event(gunno, NET_OPERATION_EVENT_CANCEL_RESERVATION);
            net_operation_clear_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
            return 0x00;
        }
    }else{
        switch(base->state.current){
        case APP_OFSM_STATE_READYING:
            base->reservation_time_sec = request->appoDelay *60;
            base->reservation_strategy_para = 0x00;
            base->reservation_strategy = (uint8_t)(APP_RESERVATE_STRATEGY_PULLGUN_CANCEL | \
                    APP_RESERVATE_STRATEGY_TIMEOUT_CANCEL |APP_RESERVATE_STRATEGY_VIN_AUTH |  \
                    APP_RESERVATE_STRATEGY_FAULT_CANCEL);
            base->offline_chargetime = evs_data_dev_configs.offlinChaLen;
            net_operation_set_event(gunno, NET_OPERATION_EVENT_SET_RESERVATION);
            net_operation_set_event(gunno, NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT);
            return 0x00;
            break;
        default:
            return -0x03;
            break;
        }
    }
    return -0x04;
}

/*************************************************
 * 函数名      sgcc_message_pro_orderly_charge_request
 * 功能          处理服务器下发的有序充电请求
 * **********************************************/
int8_t sgcc_message_pro_orderly_charge_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_orderCharge);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    uint8_t gunno = 0x00, hour = 0x00, min = 0x00, time_val[0x03];
    uint32_t temp_value = 0x00;
    evs_service_orderCharge *request = (evs_service_orderCharge*)data;
    System_BaseData *base = NULL;

    for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
        if((base->state.current == APP_OFSM_STATE_STARTING) || (base->state.current == APP_OFSM_STATE_CHARGING)){
            if(memcmp(request->preTradeNo, base->transaction_number, strlen(request->preTradeNo)) == 0x00){
               LOG_D("sgcc gunno(%d) set orderly charge", gunno);
                break;
            }
        }
    }
    if(gunno == NET_SYSTEM_GUN_NUMBER){
        LOG_W("no match charging transaction number when set orderly charge");
        return -0x03;
    }

    memset(&s_sgcc_order_charge[gunno], 0x00, sizeof(s_sgcc_order_charge[gunno]));
    for(uint8_t count = 0x00; count < SGCC_ORDERLY_CHARGE_TIME_POINT_NUM; count++){
        memset(time_val, 0x00, sizeof(time_val));
        time_val[0x00] = request->validTime[count][0x00];
        time_val[0x01] = request->validTime[count][0x01];
        hour = atoi((char*)time_val);

        memset(time_val, 0x00, sizeof(time_val));
        time_val[0x00] = request->validTime[count][0x02];
        time_val[0x01] = request->validTime[count][0x03];
        min = atoi((char*)time_val);

        s_sgcc_order_charge[gunno].time_sec[count] = (hour *60 *60 + min *60);
        s_sgcc_order_charge[gunno].period_power[count] = request->kw[count];

        if(s_sgcc_order_charge[gunno].period_power[count] *100 > base->system_power_max){
            LOG_E("power out of range when set orderly charge(%d, %d, %d, %d)", gunno, count,
                    s_sgcc_order_charge[gunno].period_power[count] *100, base->system_power_max);
            return -0x04;
        }

        LOG_D("orderly charge point(%d, %d:%d)[%ds, %dw]", gunno, hour, min, \
                s_sgcc_order_charge[gunno].time_sec[count], s_sgcc_order_charge[gunno].period_power[count] *100);
    }

    s_sgcc_flag_info[gunno].is_orderly_charge = NET_ENUM_TRUE;
    s_sgcc_order_charge[gunno].current_index = (SGCC_ORDERLY_CHARGE_TIME_POINT_NUM + 0x01);

    /** 由小到大排序 */
    for(uint8_t i = 0x00; i < SGCC_ORDERLY_CHARGE_TIME_POINT_NUM; i++){
        for(uint8_t j = (i + 0x01); j < SGCC_ORDERLY_CHARGE_TIME_POINT_NUM; j++){
            if(s_sgcc_order_charge[gunno].time_sec[i] > s_sgcc_order_charge[gunno].time_sec[j]){
                temp_value = s_sgcc_order_charge[gunno].time_sec[i];
                s_sgcc_order_charge[gunno].time_sec[i] = s_sgcc_order_charge[gunno].time_sec[j];
                s_sgcc_order_charge[gunno].time_sec[j] = temp_value;

                temp_value = s_sgcc_order_charge[gunno].period_power[i];
                s_sgcc_order_charge[gunno].period_power[i] = s_sgcc_order_charge[gunno].period_power[j];
                s_sgcc_order_charge[gunno].period_power[j] = temp_value;
            }
        }
    }

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_message_pro_query_dev_record_request
 * 功能          处理服务器下发的查询设备日志请求
 * **********************************************/
int8_t sgcc_message_pro_query_dev_record_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(evs_service_query_log);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    evs_service_query_log *request = (evs_service_query_log*)data;
    thaisen_transaction_t *_transaction = NULL;
    net_record_info_t info;
    uint8_t valid_len = 0x00;
    int32_t record_num = 0x00;

    switch(base->state.current){
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
    case APP_OFSM_STATE_STOPING:
        return -0x03;
        break;
    default:
        break;
    }

    switch(request->askType){
    case 10:
    case 11:
        info.option = NET_SYSTEM_RECORD_OPTION_CNUM;
        break;
#if 0
    case 13:
        info.option = NET_SYSTEM_RECORD_OPTION_FNUM;
        break;
#endif
    default:
        return -0x04;
        break;
    }

    info.gunno = gunno;
    info.sindex = 0x00;
    info.stime = request->startDate;
    info.etime = request->stopDate;

    record_num = s_sgcc_handle->query_system_record(&info);
    if((record_num <= 0x00) && (record_num > NET_SYSTEM_RECORD_STORAGE_NUM_MAX)){
        return -0x04;
    }

    switch(request->askType){
    case 10:
    case 11:
        info.option = NET_SYSTEM_RECORD_OPTION_CHARGE;
        break;
#if 0
    case 13:
        info.option = NET_SYSTEM_RECORD_OPTION_FAULT;
        break;
#endif
    default:
        return -0x04;
        break;
    }

    if(request->askType == 10){
        _transaction = (thaisen_transaction_t*)rt_malloc(sizeof(thaisen_transaction_t));
        if(_transaction == NULL){
            return -0x03;
        }

        info.buf = _transaction;
        info.len = sizeof(thaisen_transaction_t);

        for(; info.sindex < record_num; info.sindex++){
            if(s_sgcc_handle->query_system_record(&info) < 0x00){
                rt_free(_transaction);
                return -0x05;
            }
            break;
        }
        if(info.sindex == record_num){
            return -0x06;
        }
    }

    evs_event_logQuery_Results[gunno].gunNo = request->gunNo;
    evs_event_logQuery_Results[gunno].startDate = request->startDate;
    evs_event_logQuery_Results[gunno].stopDate = request->stopDate;
    evs_event_logQuery_Results[gunno].askType = request->askType;
    evs_event_logQuery_Results[gunno].result = 11;
    memcpy(evs_event_logQuery_Results[gunno].logQueryNo, request->logQueryNo, EVS_MAX_LOGQUERY_LEN);
    evs_event_logQuery_Results[gunno].retType = 10;
    evs_event_logQuery_Results[gunno].logQueryEvtSum = 0x01;
    evs_event_logQuery_Results[gunno].logQueryEvtNo = 0x01;

    if(request->askType == 10){
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.gunNo = request->gunNo;
        valid_len = sizeof(_transaction->serial_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.preTradeNo, 0x00, sizeof(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.preTradeNo));
        memcpy(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.preTradeNo, _transaction->serial_number, valid_len);

        valid_len = sizeof(_transaction->device_serial_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.tradeNo, 0x00, sizeof(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.preTradeNo));
        memcpy(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.tradeNo, _transaction->device_serial_number, valid_len);

        memset(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.vinCode, 0x00, sizeof(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.vinCode));
        memcpy(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.vinCode, _transaction->car_vin, sizeof(_transaction->car_vin));

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.timeDivType = SGCC_OPSCTL_ACTION;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.chargeStartTime = _transaction->start_time;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.chargeEndTime = _transaction->end_time;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.startSoc = _transaction->start_soc;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.endSoc = _transaction->stop_soc;

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.reason = sgcc_chargepile_stop_reason_converted(_transaction->stop_reason, _transaction->order_state.is_start_fail);                            // 11 停止充电原因

        valid_len = sizeof(_transaction->rule.elect_model_sn);
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.eleModelId, 0x00, sizeof(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.eleModelId));
        memcpy(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.eleModelId, _transaction->rule.elect_model_sn, valid_len);

        valid_len = sizeof(_transaction->rule.service_model_sn);
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.serModelId, 0x00, sizeof(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.serModelId));
        memcpy(evs_event_logQuery_Results[gunno].dataArea.tradeInfo.serModelId, _transaction->rule.service_model_sn, valid_len);

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.sumStart = _transaction->ammeter_start;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.sumEnd = _transaction->ammeter_stop;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.totalElect = _transaction->total_elect;

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.sharpElect = _transaction->rate_type_elect[APP_RATE_TYPE_SHARP];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.peakElect = _transaction->rate_type_elect[APP_RATE_TYPE_PEAK];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.flatElect = _transaction->rate_type_elect[APP_RATE_TYPE_FLAT];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.valleyElect = _transaction->rate_type_elect[APP_RATE_TYPE_VALLEY];

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.totalPowerCost = _transaction->charge_fee;
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.totalServCost = _transaction->service_fee;

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.sharpPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_SHARP];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.peakPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_PEAK];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.flatPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_FLAT];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.valleyPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_VALLEY];

        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.sharpServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_SHARP];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.peakServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_PEAK];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.flatServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_FLAT];
        evs_event_logQuery_Results[gunno].dataArea.tradeInfo.valleyServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_VALLEY];

    }else{
//        struct tm _tm;
//        uint8_t used_len = 0x00;
//
//        sgcc_enter_critical();
//        _tm = *(localtime((const time_t*)&(_transaction->end_time)));
//        sgcc_exit_critical();
//
//        evs_event_logQuery_Results[gunno].dataArea.meterData.gunNo = request->gunNo;
//
//        memset(evs_event_logQuery_Results[gunno].dataArea.meterData.mailAddr, 0x00, EVS_MAX_METER_ADDR_LEN);
//        memset(evs_event_logQuery_Results[gunno].dataArea.meterData.meterNo, 0x00, EVS_MAX_METER_ADDR_LEN);
//        memset(evs_event_logQuery_Results[gunno].dataArea.meterData.assetId, 0x00, EVS_MAX_METER_ASSET_LEN);
//
//        valid_len = sizeof(_transaction->serial_number);
//        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
//        memset(evs_event_logQuery_Results[gunno].dataArea.meterData.lastTrade, 0x00, sizeof(evs_event_logQuery_Results[gunno].dataArea.meterData.lastTrade));
//        memcpy(evs_event_logQuery_Results[gunno].dataArea.meterData.lastTrade, _transaction->serial_number, valid_len);
//
//        memset(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime, 0x00, EVS_MAX_TIMESTAMP_LEN);
//        sprintf((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime + used_len), "%d", (_tm->tm_year + 1900));
//        used_len = strlen((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime));
//        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
//            return -0x07;
//        }
//        sprintf((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime + used_len), "%02d", (_tm->tm_mon + 0x01));
//        used_len = strlen((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime));
//        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
//            return -0x07;
//        }
//        sprintf((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime + used_len), "%02d", _tm->tm_mday);
//        used_len = strlen((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime));
//        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
//            return -0x07;
//        }
//        sprintf((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime + used_len), "%02d", _tm->tm_hour);
//        used_len = strlen((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime));
//        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
//            return -0x07;
//        }
//        sprintf((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime + used_len), "%02d", _tm->tm_min);
//        used_len = strlen((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime));
//        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
//            return -0x07;
//        }
//        sprintf((char*)(evs_event_logQuery_Results[gunno].dataArea.meterData.acqTime + used_len), "%02d", _tm->tm_sec);
//
//        evs_event_logQuery_Results[gunno].dataArea.meterData.sumMeter = _transaction->ammeter_stop;
//        evs_event_logQuery_Results[gunno].dataArea.meterData.elec = _transaction->total_elect;

        memcpy(&evs_event_logQuery_Results[gunno].dataArea.meterData, &evs_property_meters[gunno], sizeof(evs_property_meters[gunno]));
    }

    rt_free(_transaction);
    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_DEV_RECORD);

    return 0x00;
}


/*************************************************
 * 函数名      sgcc_message_info_init
 * 功能          国网报文信息初始化
 * **********************************************/
void sgcc_message_info_init(uint8_t gunno)  ///////// 这是网络部分外部调用的第一个函数(可以在里面进行相关初始化)
{
    static uint8_t sgcc_is_init = NET_ENUM_FALSE;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(sgcc_is_init){
        s_sgcc_flag_info[gunno].init_complete = NET_ENUM_TRUE;
        return;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_SGCC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *data = NULL, count = 0x00;
    sgcc_storage_struct *config = NULL;

    s_sgcc_handle = net_get_net_handle();
    data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));

    memset(&s_sgcc_flag_info, 0x00, sizeof(s_sgcc_flag_info));
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    /** 初始化登录签到 */
    memset(evs_event_firmware_infos.eleModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
    memset(evs_event_firmware_infos.serModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
    memset(evs_event_firmware_infos.stakeModel, 0x00, EVS_MAX_PILE_TYPE_LEN);
#ifdef NET_SGCC_PRO_USING_DC
    memcpy(evs_event_firmware_infos.stakeModel, "TH7103DC", strlen("TH7103DC"));
#else
    memcpy(evs_event_firmware_infos.stakeModel, "THAISEN_ACPile", strlen("THAISEN_ACPile"));
#endif /* NET_SGCC_PRO_USING_DC */

    evs_event_firmware_infos.vendorCode = 1287;
#if 0
    valid_len = strlen(data);
    valid_len = valid_len > EVS_MAX_DEV_SN_LEN ? EVS_MAX_DEV_SN_LEN : valid_len;
    memset(evs_event_firmware_infos.devSn, 0x00, EVS_MAX_DEV_SN_LEN);
    memcpy(evs_event_firmware_infos.devSn, data, valid_len);
#else
    memset(evs_event_firmware_infos.devSn, 0x00, EVS_MAX_DEV_SN_LEN);
    memcpy(evs_event_firmware_infos.devSn, "1000241125010001", strlen("1000241125010001"));
#endif

#ifdef NET_SGCC_PRO_USING_DC
    evs_event_firmware_infos.devType = 12;
#else
    evs_event_firmware_infos.devType = 11;
#endif /* NET_SGCC_PRO_USING_DC */

    evs_event_firmware_infos.portNum = NET_SYSTEM_GUN_NUMBER;
    memset(evs_event_firmware_infos.simMac, 0x00, EVS_MAX_MAC_ADDR_LEN);

    evs_event_firmware_infos.latitude = 0x00;
    evs_event_firmware_infos.longitude = 0x00;
    evs_event_firmware_infos.height = 100;
    evs_event_firmware_infos.gridType = 10;

    memset(evs_event_firmware_infos.btMac, 0x00, EVS_MAX_MAC_ADDR_LEN);
#ifdef NET_SGCC_PRO_USING_DC
    evs_event_firmware_infos.meaType = 10;
#else
    evs_event_firmware_infos.meaType = 13;
#endif /* NET_SGCC_PRO_USING_DC */
    evs_event_firmware_infos.otRate = base->system_power_max /100;

    data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_VOLTAGE_MIN, NULL, 0x00, option));
    evs_event_firmware_infos.otMinVol = 2000;
    if(data){
        evs_event_firmware_infos.otMinVol = (*(uint16_t*)data) *10;
    }

    data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_VOLTAGE_MAX, NULL, 0x00, option));
    evs_event_firmware_infos.otMaxVol = 12000;
    if(data){
        evs_event_firmware_infos.otMaxVol = (*(uint16_t*)data) *10;
    }

    data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_CURRENT_MAX, NULL, 0x00, option));
    evs_event_firmware_infos.otCur = 5000;
    if(data){
        evs_event_firmware_infos.otCur = (*(uint16_t*)data) *10;
    }

    memset(evs_event_firmware_infos.inMeter, 0x00, EVS_MAX_INPUT_METER_NUM *EVS_MAX_METER_ADDR_LEN);
    memset(evs_event_firmware_infos.outMeter, 0x00, EVS_MAX_PORT_NUM *EVS_MAX_METER_ADDR_LEN);
    data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_AMMETER_ADDRESS, &count, sizeof(count), option));
    if(data){
        sgcc_ascii_to_bcd(data, strlen((char*)data), evs_event_firmware_infos.outMeter, EVS_MAX_METER_ADDR_LEN, NET_ENUM_TRUE);
    }

    evs_event_firmware_infos.CT = 30;

    evs_event_firmware_infos.isGateLock = 10;
    evs_event_firmware_infos.isGroundLock = 10;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        /** 初始实时数据(非充电中)请求 */
#ifdef NET_SGCC_PRO_USING_DC
        evs_property_dc_nonWorks[gunno].gunNo = gunno + 0x01;
        evs_property_dc_nonWorks[gunno].workStatus = SGCC_WORKSTATE_IDLE;
        evs_property_dc_nonWorks[gunno].gunStatus = SGCC_OPSCTL_SILENT;
#else
        evs_property_ac_nonWorks[gunno].gunNo = gunno + 0x01;
        evs_property_ac_nonWorks[gunno].workStatus = SGCC_WORKSTATE_IDLE;
        evs_property_ac_nonWorks[gunno].conStatus = SGCC_OPSCTL_SILENT;
#endif /* #ifdef NET_SGCC_PRO_USING_DC */

        /** 初始实时数据(充电中)请求 */
#ifdef NET_SGCC_PRO_USING_DC
        evs_property_dc_works[gunno].gunNo = gunno + 0x01;
        evs_property_dc_works[gunno].workStatus = SGCC_WORKSTATE_IDLE;
        evs_property_dc_works[gunno].gunStatus = SGCC_OPSCTL_SILENT;
#else
        evs_property_ac_works[gunno].gunNo = gunno + 0x01;
        evs_property_ac_works[gunno].workStatus = SGCC_WORKSTATE_IDLE;
        evs_property_ac_works[gunno].conStatus = SGCC_OPSCTL_SILENT;
#endif /* #ifdef NET_SGCC_PRO_USING_DC */
        /** 初始化桩状态变化上报请求 */
        evs_event_pile_stutus_changes[gunno].gunNo = gunno + 0x01;
        /** 初始化上报交易记录请求 */
        evs_event_tradeInfos[gunno].gunNo = gunno + 0x01;
        /** 初始化计费模型请求 */
        evs_event_ask_feeModels[gunno].gunNo = gunno + 0x01;
        /** 初始化电表底值请求 */
        evs_property_meters[gunno].gunNo = gunno + 0x01;
        memset(evs_property_meters[gunno].mailAddr, 0x00, EVS_MAX_METER_ADDR_LEN);
        memset(evs_property_meters[gunno].meterNo, 0x00, EVS_MAX_METER_ADDR_LEN);
        memset(evs_property_meters[gunno].assetId, 0x00, EVS_MAX_METER_ASSET_LEN);
        memset(evs_property_meters[gunno].lastTrade, 0x00, EVS_MAX_TRADE_LEN);
#ifdef NET_SGCC_PRO_USING_DC
        /** 初始化BMS数据请求 */
        evs_property_BMSs[gunno].gunNo = gunno + 0x01;
        /** 初始化实时监测数据 */
        evs_property_dcPiles.netType = SGCC_NETTYPE_4G;
#else
        evs_property_acPiles.netType = SGCC_NETTYPE_4G;
#endif /* NET_SGCC_PRO_USING_DC */
        /** 初始化故障告警信息 */
        evs_event_alarms[gunno].gunNo = gunno + 0x01;
        /** 初始化启动鉴权信息 */
        evs_event_startCharges[gunno].gunNo = gunno + 0x01;
        /** 初始化车辆信息 */
        evs_event_car_infos[gunno].gunNo = gunno + 0x01;
        evs_event_car_infos[gunno].batterySOC = 0x00;
        evs_event_car_infos[gunno].batteryCap = 0x00;
        memset(evs_event_car_infos[gunno].vinCode, 0x00, EVS_MAX_CAR_VIN_LEN);
        evs_event_car_infos[gunno].state = 11;
        /** 初始化枪状态变化信息 */
        evs_event_pile_stutus_changes[gunno].connCheckStatus = SGCC_OPSCTL_SILENT;
    }

    evs_event_ver_infos.devRegMethod = 10;
    memset(evs_event_ver_infos.pileHardwareVer, 0x00, EVS_MAX_HARDWAREVER_LEN);
#if 0
    data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_HARDWARE_INFO, NULL, 0x00, option));
    valid_len = strlen(data);
    valid_len = valid_len > EVS_MAX_HARDWAREVER_LEN ? EVS_MAX_HARDWAREVER_LEN : valid_len;
    memcpy(evs_event_ver_infos.pileHardwareVer, data, valid_len);
#else
    memcpy(evs_event_ver_infos.pileHardwareVer, "V7103", strlen("V7103"));
#endif

    memset(evs_event_ver_infos.pileSoftwareVer, 0x00, EVS_MAX_SOFTWAREVER_LEN);
#if 0
    sprintf("V%02d.%02d.%02d", base->soft_ver_main, base->soft_ver_sub, base->soft_ver_revise);
#else
    memcpy(evs_event_ver_infos.pileSoftwareVer, "V1.3.2", strlen("V1.3.2"));
#endif

    memset(evs_event_ver_infos.sdkVer, 0x00, EVS_MAX_SDKVER_LEN);
    memcpy(evs_event_ver_infos.sdkVer, "SDK_V1.1.10", strlen("SDK_V1.1.10"));

    sgcc_storage_data_check();

    if(config){
        memcpy(evs_event_firmware_infos.eleModelId, config->billing_rule.eleModelId, EVS_MAX_MODEL_ID_LEN);
        memcpy(evs_event_firmware_infos.serModelId, config->billing_rule.serModelId, EVS_MAX_MODEL_ID_LEN);
    }

    s_sgcc_flag_info[gunno].init_complete = NET_ENUM_TRUE;
    sgcc_is_init = NET_ENUM_TRUE;
}

/*************************************************
 * 函数名      sgcc_chargepile_request_padding_state_nonchargig
 * 功能          充电桩请求报文填报：实时数据(非充电中)
 * **********************************************/
void sgcc_chargepile_request_padding_state_data(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(is_init){
        uint8_t valid_len = sizeof(base->transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
#ifdef NET_SGCC_PRO_USING_DC
        memset(evs_property_dc_works[gunno].preTradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_dc_works[gunno].preTradeNo, base->transaction_number, valid_len);

        valid_len = sizeof(base->device_transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_property_dc_works[gunno].tradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_dc_works[gunno].tradeNo, base->device_transaction_number, valid_len);

        evs_property_dc_works[gunno].chgType = sgcc_chargepile_transaction_identity_converted(base->start_type, \
                !(base->flag.is_local_charging));

        evs_property_dc_works[gunno].dcVol = 0x00;
        evs_property_dc_works[gunno].dcCur = 0x00;
        evs_property_dc_works[gunno].realPower = 0x00;
        evs_property_dc_works[gunno].chgTime = 0x00;
        evs_property_dc_works[gunno].remainT = 0x00;
        evs_property_dc_works[gunno].socVal = 0x00;
        evs_property_dc_works[gunno].needVol = 0x00;
        evs_property_dc_works[gunno].needCur = 0x00;
        evs_property_dc_works[gunno].chargeMode = (SGCC_OPSCTL_ACTION + 0x01);
        evs_property_dc_works[gunno].bmsVol = 0x00;
        evs_property_dc_works[gunno].bmsCur = 4000;
        evs_property_dc_works[gunno].SingleMHV = 0x00;
        evs_property_dc_works[gunno].MHTemp = 500;
        evs_property_dc_works[gunno].MLTemp = 500;
        evs_property_dc_works[gunno].totalElect = 0x00;
        evs_property_dc_works[gunno].totalCost = 0x00;
        evs_property_dc_works[gunno].totalPowerCost = 0x00;
        evs_property_dc_works[gunno].totalServCost = 0x00;
        evs_property_dc_works[gunno].sharpElect = 0x00;
        evs_property_dc_works[gunno].peakElect = 0x00;
        evs_property_dc_works[gunno].flatElect = 0x00;
        evs_property_dc_works[gunno].valleyElect = 0x00;

        sgcc_chargepile_request_padding_bms_data(gunno, NET_ENUM_TRUE);
#else
        memset(evs_property_ac_works[gunno].preTradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_ac_works[gunno].preTradeNo, base->transaction_number, valid_len);

        valid_len = sizeof(base->device_transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_property_ac_works[gunno].tradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_ac_works[gunno].tradeNo, base->device_transaction_number, valid_len);

        evs_property_ac_works[gunno].acVolA = 0x00;
        evs_property_ac_works[gunno].acCurA = 0x00;
        evs_property_ac_works[gunno].acVolB = 0x00;
        evs_property_ac_works[gunno].acCurB = 0x00;
        evs_property_ac_works[gunno].acVolC = 0x00;
        evs_property_ac_works[gunno].acCurC = 0x00;
        evs_property_ac_works[gunno].realPower = 0x00;
        evs_property_ac_works[gunno].chgTime = 0x00;
        evs_property_ac_works[gunno].totalElect = 0x00;
        evs_property_ac_works[gunno].sharpElect = 0x00;
        evs_property_ac_works[gunno].peakElect = 0x00;
        evs_property_ac_works[gunno].flatElect = 0x00;
        evs_property_ac_works[gunno].valleyElect = 0x00;
        evs_property_ac_works[gunno].totalPowerCost = 0x00;
        evs_property_ac_works[gunno].totalServCost = 0x00;
        evs_property_ac_works[gunno].PwmDutyRadio = 53;
#endif /* NET_SGCC_PRO_USING_DC */
        return;
    }

    if(base->state.current == APP_OFSM_STATE_CHARGING){
        if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
#ifdef NET_SGCC_PRO_USING_DC
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            evs_property_dc_works[gunno].dcVol = base->voltage_a /10;
            evs_property_dc_works[gunno].dcCur = base->current_a;
            evs_property_dc_works[gunno].realPower = base->power_a /100;
            evs_property_dc_works[gunno].chgTime = base->charge_time /60;
            evs_property_dc_works[gunno].remainT = bms->BCS.SurplChgTime;
            evs_property_dc_works[gunno].socVal = bms->BCS.SOC;
            evs_property_dc_works[gunno].needVol = bms->BCL.BMSneedVolt;
            evs_property_dc_works[gunno].needCur = bms->BCL.BMSneedCurlt;
            evs_property_dc_works[gunno].chargeMode = (bms->BCL.ChagModel + SGCC_OPSCTL_ACTION);
            evs_property_dc_works[gunno].bmsVol = bms->BCS.ChargVolt;
            evs_property_dc_works[gunno].bmsCur = (4000 - bms->BCS.ChargCurlt);
            evs_property_dc_works[gunno].SingleMHV = bms->BCS.CellHigVolt;
            evs_property_dc_works[gunno].MHTemp = (bms->BSM.HigTemp + 50) *10;
            evs_property_dc_works[gunno].MLTemp = (bms->BSM.LowTemp + 50) *10;
            evs_property_dc_works[gunno].totalElect = base->elect_a;
            evs_property_dc_works[gunno].totalCost = base->fees_total;
            evs_property_dc_works[gunno].totalPowerCost = base->elect_fees_total;
            evs_property_dc_works[gunno].totalServCost = base->service_fees_total;
            evs_property_dc_works[gunno].sharpElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_SHARP);
            evs_property_dc_works[gunno].peakElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_PEAK);
            evs_property_dc_works[gunno].flatElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_FLAT);
            evs_property_dc_works[gunno].valleyElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_VALLEY);
#else
            evs_property_ac_works[gunno].acVolA = base->voltage_a /10;
            evs_property_ac_works[gunno].acCurA = base->current_a /10;
            evs_property_ac_works[gunno].acVolB = base->voltage_b /10;
            evs_property_ac_works[gunno].acCurB = base->current_b /10;
            evs_property_ac_works[gunno].acVolC = base->voltage_c /10;
            evs_property_ac_works[gunno].acCurC = base->current_b /10;
            evs_property_ac_works[gunno].realPower = (base->power_a + base->power_b + base->power_c) /100;
            evs_property_ac_works[gunno].chgTime = base->charge_time /60;
            evs_property_ac_works[gunno].totalElect = base->elect_a;
            evs_property_ac_works[gunno].sharpElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_SHARP);
            evs_property_ac_works[gunno].peakElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_PEAK);
            evs_property_ac_works[gunno].flatElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_FLAT);
            evs_property_ac_works[gunno].valleyElect = app_billingrule_get_rate_type_elect(gunno, APP_RATE_TYPE_VALLEY);
            evs_property_ac_works[gunno].totalPowerCost = base->elect_fees_total;
            evs_property_ac_works[gunno].totalServCost = base->service_fees_total;
            evs_property_ac_works[gunno].PwmDutyRadio = 53;
#endif /* NET_SGCC_PRO_USING_DC */
        }
    }
}

/*************************************************
 * 函数名      sgcc_chargepile_request_padding_out_ammeter_val
 * 功能          充电桩请求报文填报：输出电表底值
 * **********************************************/
void sgcc_chargepile_request_padding_out_ammeter_val(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t transaction_sn_len = 0x00, valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE) == NET_SGCC_SEND_STATE_COMPLETE){
        uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_SGCC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT), count = 0x00;
        struct tm _tm;
        uint8_t used_len = 0x00, *data = NULL;

        sgcc_enter_critical();
        _tm = *(localtime((const time_t*)&(base->current_time)));
        sgcc_exit_critical();

        data = (uint8_t*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_AMMETER_ADDRESS, &count, sizeof(count), option));
        memset(evs_property_meters[gunno].meterNo, 0x00, EVS_MAX_METER_ADDR_LEN);
        if(data){
            sgcc_ascii_to_bcd(data, strlen((char*)data), evs_property_meters[gunno].meterNo, EVS_MAX_METER_ADDR_LEN, NET_ENUM_TRUE);
        }

        memset(evs_property_meters[gunno].acqTime, 0x00, EVS_MAX_TIMESTAMP_LEN);
        sprintf((char*)(evs_property_meters[gunno].acqTime + used_len), "%d", (_tm.tm_year + 1900));
        used_len = strlen((char*)(evs_property_meters[gunno].acqTime));
        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
            return;
        }
        sprintf((char*)(evs_property_meters[gunno].acqTime + used_len), "%02d", (_tm.tm_mon + 0x01));
        used_len = strlen((char*)(evs_property_meters[gunno].acqTime));
        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
            return;
        }
        sprintf((char*)(evs_property_meters[gunno].acqTime + used_len), "%02d", _tm.tm_mday);
        used_len = strlen((char*)(evs_property_meters[gunno].acqTime));
        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
            return;
        }
        sprintf((char*)(evs_property_meters[gunno].acqTime + used_len), "%02d", _tm.tm_hour);
        used_len = strlen((char*)(evs_property_meters[gunno].acqTime));
        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
            return;
        }
        sprintf((char*)(evs_property_meters[gunno].acqTime + used_len), "%02d", _tm.tm_min);
        used_len = strlen((char*)(evs_property_meters[gunno].acqTime));
        if((used_len + 0x02) > EVS_MAX_TIMESTAMP_LEN){
            return;
        }
        sprintf((char*)(evs_property_meters[gunno].acqTime + used_len), "%02d", _tm.tm_sec);

        if(base->ammeter_elect > evs_property_meters[gunno].sumMeter){
            evs_property_meters[gunno].sumMeter = base->ammeter_elect;
        }
        if(base->elect_a > 0x00){
            evs_property_meters[gunno].elec = base->elect_a;
        }

        transaction_sn_len = strlen((char*)(base->device_transaction_number));

        if(transaction_sn_len > 0x00){
            valid_len = sizeof(base->device_transaction_number);
            valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
            memset(evs_property_meters[gunno].lastTrade, 0x00, EVS_MAX_TRADE_LEN);
            memcpy(evs_property_meters[gunno].lastTrade, base->device_transaction_number, valid_len);
        }

        s_sgcc_flag_info[gunno].omit_oammeter_val = NET_ENUM_FALSE;
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE);
    }else{
        s_sgcc_flag_info[gunno].omit_oammeter_val = NET_ENUM_TRUE;
    }
}

/*************************************************
 * 函数名      sgcc_chargepile_request_padding_bms_data
 * 功能          充电桩请求报文填报：BMS数据
 * **********************************************/
void sgcc_chargepile_request_padding_bms_data(uint8_t gunno, uint8_t init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
#ifdef NET_SGCC_PRO_USING_DC
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(init){
        uint8_t valid_len = sizeof(base->transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_property_BMSs[gunno].preTradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_BMSs[gunno].preTradeNo, base->transaction_number, valid_len);

        valid_len = sizeof(base->device_transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_property_BMSs[gunno].tradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_BMSs[gunno].tradeNo, base->device_transaction_number, valid_len);

        return;
    }

    if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA) == NET_SGCC_SEND_STATE_COMPLETE){
        struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

        if(base->state.current == APP_OFSM_STATE_STARTING){
            if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
            evs_property_BMSs[gunno].socVal = bms->BCP.SOC /10;
        }else{
            if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
            evs_property_BMSs[gunno].socVal = bms->BCS.SOC;
        }

        if(bms->BRM.BMSVer[0x00] == 0x00){
            evs_property_BMSs[gunno].BMSVer = SGCC_OPSCTL_ACTION;
        }else{
            evs_property_BMSs[gunno].BMSVer = SGCC_OPSCTL_SILENT;
        }
        evs_property_BMSs[gunno].BMSMaxVol = bms->BCP.BatAlowHigVolt;

        if((bms->BRM.BatType <= 0x08) && (bms->BRM.BatType > 0x00)){
            evs_property_BMSs[gunno].batType = (SGCC_OPSCTL_ACTION + bms->BRM.BatType);
        }else{
            evs_property_BMSs[gunno].batType = 99;
        }

        evs_property_BMSs[gunno].batRatedCap = bms->BRM.BatRateCap;
        evs_property_BMSs[gunno].batRatedTotalVol = bms->BRM.BatRateVolt;
        evs_property_BMSs[gunno].singlBatMaxAllowVol = bms->BCP.CellAlowHigVolt;
        evs_property_BMSs[gunno].maxAllowCur = (4000 - bms->BCP.AlowCurlt);
        evs_property_BMSs[gunno].battotalEnergy = bms->BCP.BatRateKW;
        evs_property_BMSs[gunno].maxVol = bms->BCP.BatAlowHigVolt;
        evs_property_BMSs[gunno].maxTemp = (bms->BCP.BatAlowHigTemp + 50);
        evs_property_BMSs[gunno].batCurVol = bms->BCP.BatVolt;

        s_sgcc_bms_data_report_num[gunno]++;
        s_sgcc_flag_info[gunno].omit_bms_data = NET_ENUM_FALSE;
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA);
    }else{
        s_sgcc_flag_info[gunno].omit_bms_data = NET_ENUM_TRUE;
    }
#endif /* NET_SGCC_PRO_USING_DC */
}

/*************************************************
 * 函数名      sgcc_chargepile_request_padding_monitor_property
 * 功能          充电桩请求报文填报：实时监测属性
 * **********************************************/
void sgcc_chargepile_request_padding_monitor_property(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY) == NET_SGCC_SEND_STATE_COMPLETE){
        uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_SGCC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
        uint8_t *data = NULL, valid_len = 0x00, signal = 0x00;
#ifdef NET_SGCC_PRO_USING_DC
        valid_len = strlen((char*)(app_billingrule_get_elect_model_sn(gunno)));
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_property_dcPiles.eleModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
        memcpy(evs_property_dcPiles.eleModelId, app_billingrule_get_elect_model_sn(gunno), valid_len);

        valid_len = strlen((char*)(app_billingrule_get_service_model_sn(gunno)));
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_property_dcPiles.serModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
        memcpy(evs_property_dcPiles.serModelId, app_billingrule_get_service_model_sn(gunno), valid_len);

        (void)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_SIGNAL_STRENGTH, &signal, sizeof(signal), NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
        evs_property_dcPiles.sigVal = signal;
        data = s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_OPERATOR, NULL, 0x00, option);

        switch (*data) {
        case NET_OPERATOR_NAME_CHINA_MOBILE:
            evs_property_dcPiles.netId = SGCC_OPERATOR_MOBILE;
            break;
        case NET_OPERATOR_NAME_CHINA_TELECOM:
            evs_property_dcPiles.netId = SGCC_OPERATOR_TELECOM;
            break;
        case NET_OPERATOR_NAME_CHINA_UNICOM:
            evs_property_dcPiles.netId = SGCC_OPERATOR_UNICOM;
            break;
        default:
            evs_property_dcPiles.netId = SGCC_OPERATOR_OTHER;
            break;
        }

        evs_property_dcPiles.acVolA = base->voltage_a /10;
        evs_property_dcPiles.acCurA = base->current_a /10;
        evs_property_dcPiles.acVolB = base->voltage_b /10;
        evs_property_dcPiles.acCurB = base->current_b /10;
        evs_property_dcPiles.acVolC = base->voltage_c /10;
        evs_property_dcPiles.acCurC = base->current_b /10;

        evs_property_dcPiles.caseTemp = (250 + 500);
        evs_property_dcPiles.inletTemp = (250 + 500);
        evs_property_dcPiles.outletTemp = (250 + 500);
#else
        valid_len = strlen((char*)(app_billingrule_get_elect_model_sn(gunno)));
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_property_acPiles.eleModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
        memcpy(evs_property_acPiles.eleModelId, app_billingrule_get_elect_model_sn(gunno), valid_len);

        valid_len = strlen((char*)(app_billingrule_get_service_model_sn(gunno)));
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_property_acPiles.serModelId, 0x00, EVS_MAX_MODEL_ID_LEN);
        memcpy(evs_property_acPiles.serModelId, app_billingrule_get_service_model_sn(gunno), valid_len);

        (void)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_SIGNAL_STRENGTH, &signal, sizeof(signal), NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
        evs_property_acPiles.sigVal = signal;
        data = s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_OPERATOR, NULL, 0x00, option);

        switch (*data) {
        case NET_OPERATOR_NAME_CHINA_MOBILE:
            evs_property_acPiles.netId = SGCC_OPERATOR_MOBILE;
            break;
        case NET_OPERATOR_NAME_CHINA_TELECOM:
            evs_property_acPiles.netId = SGCC_OPERATOR_TELECOM;
            break;
        case NET_OPERATOR_NAME_CHINA_UNICOM:
            evs_property_acPiles.netId = SGCC_OPERATOR_UNICOM;
            break;
        default:
            evs_property_acPiles.netId = SGCC_OPERATOR_OTHER;
            break;
        }

        evs_property_acPiles.acVolA = base->voltage_a /10;
        evs_property_acPiles.acCurA = base->current_a /10;
        evs_property_acPiles.acVolB = base->voltage_b /10;
        evs_property_acPiles.acCurB = base->current_b /10;
        evs_property_acPiles.acVolC = base->voltage_c /10;
        evs_property_acPiles.acCurC = base->current_b /10;

        evs_property_acPiles.caseTemp = (250 + 500);
#endif /* NET_SGCC_PRO_USING_DC */

        s_sgcc_flag_info[gunno].omit_monitor_property = NET_ENUM_FALSE;
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY);
    }else{
        s_sgcc_flag_info[gunno].omit_monitor_property = NET_ENUM_TRUE;
    }
}


/*************************************************
 * 函数名      sgcc_chargepile_request_padding_card_authority
 * 功能          充电桩请求报文填报：刷卡权限认证
 * **********************************************/
int8_t sgcc_chargepile_request_padding_card_authority(uint8_t gunno)
{
//#ifndef NET_SGCC_AS_MONITOR
#if 0
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return -0x01;
    }
    if(sgcc_get_socket_info()->state != SGCC_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ykc_handle->get_base_data(gunno));

    if(!base->flag.card_info_is_uid){
        return -0x01;
    }
    g_ykc_preq_apply_charge_active[gunno].body.start_type = 0x01;
    g_ykc_preq_apply_charge_active[gunno].body.whether_password = NET_ENUM_FALSE;

    valid_len = sizeof(base->card_uid);
    valid_len = valid_len > NET_YKC_CARD_NUMBER_LENGTH_MAX ? NET_YKC_CARD_NUMBER_LENGTH_MAX : valid_len;
    memset(g_ykc_preq_apply_charge_active[gunno].body.account_or_phycard_number, 0x00, NET_YKC_CARD_NUMBER_LENGTH_MAX);
    if(valid_len > base->card_uid_len){
        memcpy((g_ykc_preq_apply_charge_active[gunno].body.account_or_phycard_number + (valid_len - base->card_uid_len)),
                base->card_uid, base->card_uid_len);
    }else{
        memcpy(g_ykc_preq_apply_charge_active[gunno].body.account_or_phycard_number, base->card_uid, valid_len);
    }
    memset(g_ykc_preq_apply_charge_active[gunno].body.password, 0x00, NET_YKC_PASSWORD_LENGTH_DEFAULT);
    memset(g_ykc_preq_apply_charge_active[gunno].body.vin, 0x00, NET_YKC_CAR_VIN_NUMBER_LENGTH_MAX);

    ykc_net_event_send(NET_YKC_EVENT_HANDLE_CHARGEPILE, NET_YKC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_SGCC_AS_MONITOR */
}

/*************************************************
 * 函数名      sgcc_chargepile_request_padding_vin_authority
 * 功能          充电桩请求报文填报：VIN 码权限认证
 * **********************************************/
int8_t sgcc_chargepile_request_padding_vin_authority(uint8_t gunno)
{
#ifndef NET_SGCC_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == NET_ENUM_FALSE){
        return -0x01;
    }
    if(sgcc_get_socket_info()->state != SGCC_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    uint8_t valid_len = sizeof(base->device_transaction_number);
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    memset(evs_event_startCharges[gunno].preTradeNo, 0x00, EVS_MAX_TRADE_LEN);
    memset(evs_event_startCharges[gunno].tradeNo, 0x00, EVS_MAX_TRADE_LEN);

    valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
    memcpy(evs_event_startCharges[gunno].tradeNo, base->device_transaction_number, valid_len);

    evs_event_startCharges[gunno].startType = SGCC_OPSCTL_SILENT;
    memset(evs_event_startCharges[gunno].authCode, 0x00, EVS_MAX_CAR_VIN_LEN);
    memcpy(evs_event_startCharges[gunno].authCode, base->car_vin, sizeof(base->car_vin));
    evs_event_startCharges[gunno].batterySOC = bms->BCP.SOC /10;
    evs_event_startCharges[gunno].batteryCap = bms->BRM.BatRateCap;
    evs_event_startCharges[gunno].chargeTimes = 0x00;
    evs_event_startCharges[gunno].batteryVol = bms->BCP.BatVolt;

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_SGCC_AS_MONITOR */
}

/*************************************************
 * 函数名      sgcc_chargepile_request_padding_transaction_record
 * 功能          充电桩请求报文填报：交易记录信息
 * **********************************************/
uint8_t sgcc_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    uint8_t valid_len = 0x00;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)transaction;

    sgcc_set_transaction_verify_state(gunno, 0x00);

    if((sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD) == NET_SGCC_SEND_STATE_COMPLETE)){
        valid_len = sizeof(_transaction->serial_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_event_tradeInfos[gunno].preTradeNo, 0x00, sizeof(evs_event_tradeInfos[gunno].preTradeNo));
        memcpy(evs_event_tradeInfos[gunno].preTradeNo, _transaction->serial_number, valid_len);

        valid_len = sizeof(_transaction->device_serial_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_event_tradeInfos[gunno].tradeNo, 0x00, sizeof(evs_event_tradeInfos[gunno].preTradeNo));
        memcpy(evs_event_tradeInfos[gunno].tradeNo, _transaction->device_serial_number, valid_len);

        memset(evs_event_tradeInfos[gunno].vinCode, 0x00, sizeof(evs_event_tradeInfos[gunno].vinCode));
        memcpy(evs_event_tradeInfos[gunno].vinCode, _transaction->car_vin, sizeof(_transaction->car_vin));

        evs_event_tradeInfos[gunno].timeDivType = SGCC_OPSCTL_ACTION;

        if(_transaction->end_time > _transaction->charge_time){
            _transaction->start_time = _transaction->end_time - _transaction->charge_time;
        }else{
            _transaction->start_time = _transaction->end_time;
            _transaction->charge_time = 0x00;
        }
        evs_event_tradeInfos[gunno].chargeStartTime = _transaction->start_time;
        evs_event_tradeInfos[gunno].chargeEndTime = _transaction->end_time;
        evs_event_tradeInfos[gunno].startSoc = _transaction->start_soc;
        evs_event_tradeInfos[gunno].endSoc = _transaction->stop_soc;

        evs_event_tradeInfos[gunno].reason = sgcc_chargepile_stop_reason_converted(_transaction->stop_reason, _transaction->order_state.is_start_fail);

        valid_len = sizeof(_transaction->rule.elect_model_sn);
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_event_tradeInfos[gunno].eleModelId, 0x00, sizeof(evs_event_tradeInfos[gunno].eleModelId));
        memcpy(evs_event_tradeInfos[gunno].eleModelId, _transaction->rule.elect_model_sn, valid_len);

        valid_len = sizeof(_transaction->rule.service_model_sn);
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_event_tradeInfos[gunno].serModelId, 0x00, sizeof(evs_event_tradeInfos[gunno].serModelId));
        memcpy(evs_event_tradeInfos[gunno].serModelId, _transaction->rule.service_model_sn, valid_len);

        evs_event_tradeInfos[gunno].sumStart = _transaction->ammeter_start;
        evs_event_tradeInfos[gunno].sumEnd = _transaction->ammeter_stop;
        evs_event_tradeInfos[gunno].totalElect = _transaction->total_elect;
        if(evs_event_tradeInfos[gunno].totalElect > SGCC_CHARGE_ELECT_MAX){
            evs_event_tradeInfos[gunno].totalElect = SGCC_CHARGE_ELECT_MAX;
        }

        evs_event_tradeInfos[gunno].sharpElect = _transaction->rate_type_elect[APP_RATE_TYPE_SHARP];
        if(evs_event_tradeInfos[gunno].sharpElect > SGCC_CHARGE_ELECT_MAX){
            evs_event_tradeInfos[gunno].sharpElect = SGCC_CHARGE_ELECT_MAX;
        }
        evs_event_tradeInfos[gunno].peakElect = _transaction->rate_type_elect[APP_RATE_TYPE_PEAK];
        if(evs_event_tradeInfos[gunno].peakElect > SGCC_CHARGE_ELECT_MAX){
            evs_event_tradeInfos[gunno].peakElect = SGCC_CHARGE_ELECT_MAX;
        }
        evs_event_tradeInfos[gunno].flatElect = _transaction->rate_type_elect[APP_RATE_TYPE_FLAT];
        if(evs_event_tradeInfos[gunno].flatElect > SGCC_CHARGE_ELECT_MAX){
            evs_event_tradeInfos[gunno].flatElect = SGCC_CHARGE_ELECT_MAX;
        }
        evs_event_tradeInfos[gunno].valleyElect = _transaction->rate_type_elect[APP_RATE_TYPE_VALLEY];
        if(evs_event_tradeInfos[gunno].valleyElect > SGCC_CHARGE_ELECT_MAX){
            evs_event_tradeInfos[gunno].valleyElect = SGCC_CHARGE_ELECT_MAX;
        }

        evs_event_tradeInfos[gunno].totalPowerCost = _transaction->charge_fee;
        if(evs_event_tradeInfos[gunno].totalPowerCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].totalPowerCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].totalServCost = _transaction->service_fee;
        if(evs_event_tradeInfos[gunno].totalServCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].totalServCost = SGCC_SPEND_AMOUNT_MAX;
        }

        evs_event_tradeInfos[gunno].sharpPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_SHARP];
        if(evs_event_tradeInfos[gunno].sharpPowerCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].sharpPowerCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].peakPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_PEAK];
        if(evs_event_tradeInfos[gunno].peakPowerCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].peakPowerCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].flatPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_FLAT];
        if(evs_event_tradeInfos[gunno].flatPowerCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].flatPowerCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].valleyPowerCost = _transaction->rate_type_elect_amount[APP_RATE_TYPE_VALLEY];
        if(evs_event_tradeInfos[gunno].valleyPowerCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].valleyPowerCost = SGCC_SPEND_AMOUNT_MAX;
        }

        evs_event_tradeInfos[gunno].sharpServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_SHARP];
        if(evs_event_tradeInfos[gunno].sharpServCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].sharpServCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].peakServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_PEAK];
        if(evs_event_tradeInfos[gunno].peakServCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].peakServCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].flatServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_FLAT];
        if(evs_event_tradeInfos[gunno].flatServCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].flatServCost = SGCC_SPEND_AMOUNT_MAX;
        }
        evs_event_tradeInfos[gunno].valleyServCost = _transaction->rate_type_service_amount[APP_RATE_TYPE_VALLEY];
        if(evs_event_tradeInfos[gunno].valleyServCost > SGCC_SPEND_AMOUNT_MAX){
            evs_event_tradeInfos[gunno].valleyServCost = SGCC_SPEND_AMOUNT_MAX;
        }

        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
        return 0x01;
    }

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_start_charge_response_asynchronously
 * 功能          充电桩报文响应事件：启动充电异步响应
 * **********************************************/
void sgcc_start_charge_response_asynchronously(uint8_t gunno, uint8_t result, uint16_t reason, uint16_t fault)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    /** 在线启动才需要上报启动结果 */
    if(base->flag.is_local_charging == NET_ENUM_TRUE){
        return;
    }
    if(result){
        s_sgcc_flag_info[gunno].start_success = NET_ENUM_TRUE;
        s_agcc_remotecharge_result[gunno] = 10;
        s_agcc_applycharge_result[gunno] = 10;
    }else{
        s_sgcc_flag_info[gunno].start_success = NET_ENUM_FALSE;
        s_agcc_remotecharge_result[gunno] = 11;
        s_agcc_applycharge_result[gunno] = 11;
    }

    if(base->start_type == APP_CHARGE_START_WAY_APP){
        if(s_agcc_remotecharge_result[gunno] == 10){
            s_agcc_remotecharge_fail_reason[gunno] = 0x00;
        }else{
            s_agcc_remotecharge_fail_reason[gunno] = sgcc_chargepile_stop_reason_converted(reason, NET_ENUM_TRUE);
        }

        s_agcc_remotecharge_fault_reason[gunno] = s_agcc_remotecharge_fail_reason[gunno];
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY);
    }else{
        if(s_agcc_applycharge_result[gunno] == 10){
            s_agcc_applycharge_fail_reason[gunno] = 0x00;
        }else{
            s_agcc_applycharge_fail_reason[gunno] = sgcc_chargepile_stop_reason_converted(reason, NET_ENUM_TRUE);
        }

        s_agcc_applycharge_fault_reason[gunno] = s_agcc_applycharge_fail_reason[gunno];
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_APPLY_CHARGE_ASYNCHRONOUSLY);
    }

    s_sgcc_flag_info[gunno].is_start_charge = NET_ENUM_FALSE;
}

/*************************************************
 * 函数名      sgcc_stop_charge_response_asynchronously
 * 功能          充电桩报文响应事件：停止充电异步响应
 * **********************************************/
void sgcc_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result, uint16_t reason, uint16_t fault)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        gunno = base->main_gunno;
    }

    /** 在线启动才需要上报停止结果 */
    if(base->flag.is_local_charging == NET_ENUM_TRUE){
        return;
    }
    /** 启动成功才上报停止结果 */
    if(base->flag.start_result == NET_ENUM_FALSE){
        return;
    }
    if(result){
        s_sgcc_flag_info[gunno].stop_success = NET_ENUM_TRUE;
        s_agcc_remotestop_result[gunno] = 10;
    }else{
        s_sgcc_flag_info[gunno].stop_success = NET_ENUM_FALSE;
        s_agcc_remotestop_result[gunno] = 11;
    }

    s_agcc_remotestop_fail_reason[gunno] = sgcc_chargepile_stop_reason_converted(reason, NET_ENUM_TRUE);
    s_agcc_remotestop_fault_reason[gunno] = 11;
    s_sgcc_flag_info[gunno].is_stop_charge = NET_ENUM_FALSE;

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_RESPONSE, gunno, NET_SGCC_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      sgcc_chargepile_state_changed
 * 功能          桩状态变化上报
 * **********************************************/
void sgcc_chargepile_state_changed(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        if(s_sgcc_state_info[gunno].state.connect == SGCC_OPSCTL_SILENT){
            s_sgcc_state_info[gunno].timestamp = base->current_time;
        }
        s_sgcc_state_info[gunno].state.connect = SGCC_OPSCTL_ACTION;
    }else{
        if(s_sgcc_state_info[gunno].state.connect == SGCC_OPSCTL_ACTION){
            s_sgcc_state_info[gunno].timestamp = base->current_time;
        }
        s_sgcc_state_info[gunno].state.connect = SGCC_OPSCTL_SILENT;
    }

    switch(base->state.current){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
        s_sgcc_state_info[gunno].state.state = SGCC_WORKSTATE_IDLE;

        s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
        s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;

        break;
    case APP_OFSM_STATE_READYING:
        s_sgcc_state_info[gunno].state.state = SGCC_WORKSTATE_INSERT;

        s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
        s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;

        break;
    case APP_OFSM_STATE_STARTING:
    {
        if(s_sgcc_state_info[gunno].state.state != SGCC_WORKSTATE_STARTING){
            if((base->flag.is_local_charging == NET_ENUM_FALSE) && (base->start_type != APP_CHARGE_START_WAY_VIN)){
                if(++s_sgcc_charge_sn[gunno] >= 10000){
                    s_sgcc_charge_sn[gunno] = 0x01;
                }
                s_sgcc_operation_sn[gunno] = 0x00;
            }
        }

        s_sgcc_state_info[gunno].state.state = SGCC_WORKSTATE_STARTING;
        switch(base->charctrl_state.current){
        case APP_CHARGE_CTRL_STATE_SHAKE_HAND:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        case APP_CHARGE_CTRL_STATE_INSULATION:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        case APP_CHARGE_CTRL_STATE_CONFIGURE:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        case APP_CHARGE_CTRL_STATE_CHARGING:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_SILENT;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        case APP_CHARGE_CTRL_STATE_FINISH:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        case APP_CHARGE_CTRL_STATE_FAULTING:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        case APP_CHARGE_CTRL_STATE_WAIT_PULL_GUN:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        default:
            s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
            s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
            s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;
            break;
        }
    }
        break;
    case APP_OFSM_STATE_CHARGING:
        if(s_sgcc_state_info[gunno].state.state != SGCC_WORKSTATE_CHARGINGING){
            if((base->flag.is_local_charging == NET_ENUM_FALSE) && (base->start_type == APP_CHARGE_START_WAY_VIN)){
                if(++s_sgcc_charge_sn[gunno] >= 10000){
                    s_sgcc_charge_sn[gunno] = 0x01;
                }
                s_sgcc_operation_sn[gunno] = 0x00;
            }
        }
        s_sgcc_state_info[gunno].state.state = SGCC_WORKSTATE_CHARGINGING;

        s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_SILENT;
        s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_SILENT;
        s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_SILENT;
        s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
        s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;

        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        s_sgcc_state_info[gunno].state.state = SGCC_WORKSTATE_FINISH;

        s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
        s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;

        break;
    case APP_OFSM_STATE_FAULTING:
        s_sgcc_state_info[gunno].state.state = SGCC_WORKSTATE_FAULTING;

        s_sgcc_state_info[gunno].state.electlock = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck1 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dck2 = SGCC_OPSCTL_ACTION;
        s_sgcc_state_info[gunno].state.dc_plusFuse = SGCC_OPSCTL_NONE;
        s_sgcc_state_info[gunno].state.dc_minusFuse = SGCC_OPSCTL_NONE;

        break;
    default:
        break;
    }
}

/*************************************************
 * 函数名      sgcc_chargepile_fault_report
 * 功能          故障告警上报
 * **********************************************/
int8_t sgcc_chargepile_fault_report(uint8_t gunno, uint16_t code, uint8_t is_resume, uint8_t rank)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
        if(sgcc_get_message_send_state(i, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING) == NET_SGCC_SEND_STATE_ONGOING){
            return -0x01;
        }
    }

    uint8_t pos = 0x00, faultwarn_num = 0x00;
    uint16_t *faultwarn_set = NULL;

    if(is_resume){
        /** 故障 */
        if(rank == 0x01){
            faultwarn_set = evs_event_alarms[gunno].faultValue;
            faultwarn_num = evs_event_alarms[gunno].faultSum;
        }else{
            faultwarn_set = evs_event_alarms[gunno].warnValue;
            faultwarn_num = evs_event_alarms[gunno].warnSum;
        }

        for(pos = 0x00; pos < faultwarn_num; pos++){
            if(code == faultwarn_set[pos]){
                for(uint8_t i = pos; i < faultwarn_num; i++){
                    if((i + 0x01) < faultwarn_num){
                        faultwarn_set[i] = faultwarn_set[i + 0x01];
                    }
                }

                if(faultwarn_num > 0x00){
                    faultwarn_set[faultwarn_num - 0x01] = 0x00;
                }else{
                    faultwarn_set[0x00] = 0x00;
                }

                /** 故障 */
                if(rank == 0x01){
                    if(evs_event_alarms[gunno].faultSum > 0x00){
                        evs_event_alarms[gunno].faultSum--;
                    }
                }else{
                    if(evs_event_alarms[gunno].warnSum > 0x00){
                        evs_event_alarms[gunno].warnSum--;
                    }
                }
                break;
            }
        }
    }else{
        /** 故障 */
        if(rank == 0x01){
            faultwarn_set = evs_event_alarms[gunno].faultValue;
            faultwarn_num = evs_event_alarms[gunno].faultSum;
        }else{
            faultwarn_set = evs_event_alarms[gunno].warnValue;
            faultwarn_num = evs_event_alarms[gunno].warnSum;
        }

        for(uint8_t pos = 0x00; pos < faultwarn_num; pos++){
            if(code == faultwarn_set[pos]){
                return 0x00;
            }
        }
        if(faultwarn_num >= EVS_MAX_ALARM_LEN){
            return -0x01;
        }
        faultwarn_set[faultwarn_num] = code;

        /** 故障 */
        if(rank == 0x01){
            if(evs_event_alarms[gunno].faultSum < EVS_MAX_ALARM_LEN){
                evs_event_alarms[gunno].faultSum++;
            }
        }else{
            if(evs_event_alarms[gunno].warnSum < EVS_MAX_ALARM_LEN){
                evs_event_alarms[gunno].warnSum++;
            }
        }
    }
    s_sgcc_faultwarn_count[gunno] = rt_tick_get();

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING);

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_chargepile_create_local_transaction_number
 * 功能          创建本地交易号
 * **********************************************/
int8_t sgcc_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if((vector == NULL) || (len == 0x00)){
        return -0x02;
    }
    if((len + 0x01) < EVS_MAX_TRADE_LEN){
        return -0x03;
    }

#define SGCC_DEVICE_NAME_VALID_LEN      24     /** 序列号中设备资产码所需长度 */

    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    uint8_t sn_len = 0x00;
    struct tm _tm;
    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    sgcc_enter_critical();
    _tm = *(localtime((const time_t*)&(base->current_time)));
    sgcc_exit_critical();

    memset(vector, 0x00, len);
    memcpy((vector + sn_len), config->device_name, SGCC_DEVICE_NAME_VALID_LEN);
    sn_len += SGCC_DEVICE_NAME_VALID_LEN;
    if((sn_len + 0x02) >= len){
        return -0x01;
    }

    sprintf((vector + sn_len), "%02d", (gunno + 0x01));
    sn_len += 0x02;
    if((sn_len + 0x02) >= len){
        return -0x01;
    }

    sprintf((vector + sn_len), "%02d", (_tm.tm_year - 100));
    sn_len += 0x02;
    if((sn_len + 0x02) >= len){
        return -0x01;
    }

    sprintf((vector + sn_len), "%02d", (_tm.tm_mon + 0x01));
    sn_len += 0x02;
    if((sn_len + 0x02) >= len){
        return -0x01;
    }

    sprintf((vector + sn_len), "%02d", _tm.tm_mday);
    sn_len += 0x02;
    if((sn_len + 0x04) >= len){
        return -0x01;
    }

    s_sgcc_operation_sn[gunno]++;
    if(s_sgcc_operation_sn[gunno] >= 100){
        s_sgcc_operation_sn[gunno] = 0x01;
    }

    sprintf((vector + sn_len), "%04d", s_sgcc_charge_sn[gunno]);
    sn_len += 0x04;
    if((sn_len + 0x02) >= len){
        return -0x01;
    }

    sprintf((vector + sn_len), "%02d", s_sgcc_operation_sn[gunno]);
    sn_len += 0x02;

#undef SGCC_DEVICE_NAME_VALID_LEN

    LOG_D("sgcc create local transaction number(%d)[%s]\n", gunno, (char*)vector);

    return 0x00;
}

/*************************************************
 * 函数名      sgcc_chargepile_transaction_identity_converted
 * 功能          交易标识转换
 * **********************************************/
static uint8_t sgcc_chargepile_transaction_identity_converted(uint8_t identity, uint8_t online_order)
{
    switch(identity){
    case APP_CHARGE_START_WAY_APP:
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
//        return SGCC_START_TYPE_APP_ONE_CLICK;
        return SGCC_START_TYPE_PLATFORM;
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        break;
    case APP_CHARGE_START_WAY_ONLINE_CARD:
        break;
    case APP_CHARGE_START_WAY_OFFLINE_CARD:
        break;
    case APP_CHARGE_START_WAY_SCREEN:
        break;
    case APP_CHARGE_START_WAY_TIMING:
        break;
    case APP_CHARGE_START_WAY_PLUG_AND_CHARGE:
        if(online_order){
            return SGCC_START_TYPE_PLUG_AND_CHARGE;
        }
        return SGCC_START_TYPE_OFFLINE_VIN;
        break;
    case APP_CHARGE_START_WAY_BLUE:
        if(online_order){
            return SGCC_START_TYPE_BLUE_INTELLIGENT;
        }
        return SGCC_START_TYPE_BLUE;
        break;
    case APP_CHARGE_START_WAY_VIN:
        if(online_order){
            return SGCC_START_TYPE_PLUG_AND_CHARGE;
        }
        return SGCC_START_TYPE_OFFLINE_VIN;
        break;
    default:
        break;
    }
    return SGCC_START_TYPE_QRCODE;
}

/*************************************************
 * 函数名      sgcc_query_highest_priority_fault
 * 功能          获取最高优先级故障
 * **********************************************/
uint16_t sgcc_query_highest_priority_fault(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    System_BaseData *base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    return base->system_fault;
}

/*************************************************
 * 函数名      sgcc_chargepile_fault_converted
 * 功能          故障转换
 * **********************************************/
#ifdef NET_SGCC_PRO_USING_DC
uint16_t sgcc_chargepile_fault_converted(uint16_t bit, uint8_t *rank)
{
    switch(bit){
    case APP_SYS_FAULT_SCRAM :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3033_CRASH_STOP;
    case APP_SYS_FAULT_CARD_READER :
        if(rank){
            *rank = 0x00;
        }
        return NETSGCC_DCA_REASON3037_CARD_READER;
    case APP_SYS_FAULT_DOOR :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3032_OPNE_DOOR;
    case APP_SYS_FAULT_AMMETER :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3043_AMMETER_COMM;
    case APP_SYS_FAULT_CHARGE_MODULE :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3038_MODULE_COMM;
    case APP_SYS_FAULT_OVER_TEMP :
        if(rank){
            *rank = 0x00;
        }
        return NETSGCC_DCA_REASON3053_CHARGE_PORT_OVERTEMP;
    case APP_SYS_FAULT_OVER_VOLT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCPA_REASON4009_OUTPUT_OVERVOLT;
    case APP_SYS_FAULT_UNDER_VOLT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCPA_REASON4011_OUTPUT_UNDERVOLT;
    case APP_SYS_FAULT_OVER_CURR :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCPA_REASON4010_OUTPUT_OVERCURR;
    case APP_SYS_FAULT_RELAY :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3046_DC_RELAY;
    case APP_SYS_FAULT_PARALLEL_RELAY :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3068_PARALLEL_RELAY_ADH;
    case APP_SYS_FAULT_AC_RELAY :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3065_AC_RELAY;
    case APP_SYS_FAULT_ELOCK :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3054_ELOCK;
    case APP_SYS_FAULT_AUXPOWER :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3049_AUXPOWER;
    case APP_SYS_FAULT_FLASH :
        return 0x00;
    case APP_SYS_FAULT_EEPROM :
        return 0x00;
    case APP_SYSTEM_STOP_WAY_GUNVOLT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCPA_REASON4014_RELAY_OUTSIDE_OVER10V;
    case APP_SYSTEM_STOP_WAY_INSULT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3050_INSULATION;
    case APP_SYSTEM_STOP_WAY_COMMINICATION :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCCA_REASON5001_BMS_COMM;
    case APP_SYSTEM_STOP_WAY_BATTERY_VOLT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCCA_REASON5029_BATTERY_VOLT_ABNORMAL;
    case APP_SYSTEM_STOP_WAY_PULL_GUN :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3057_CHARGE_LINK;
    case APP_SYSTEM_STOP_WAY_READY_VOLT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCCA_REASON5017_BMS_READY_VOLT_NOMATCH;
    case APP_SYSTEM_STOP_WAY_INSULT_VOLT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON7002_INSULT_VOLT;
    case APP_SYS_FAULT_LIGHT_PRPTECT :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3084_LIGHTING_PROTECTORS;
    case APP_SYS_FAULT_GUN_SITE :
        return 0x00;
    case APP_SYS_FAULT_CIRCUIT_BREAKER :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCPA_REASON4013_AC_CIRCUIT_BREAKER;
    case APP_SYS_FAULT_FLOODING :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3055_FLOODING;
    case APP_SYS_FAULT_SMOKE :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3085_SMOKING;
    case APP_SYS_FAULT_POUR :
        return 0x00;
    case APP_SYS_FAULT_LIQUID_COOLING :
        return 0x00;
    case APP_SYS_FAULT_FUSE :
        if(rank){
            *rank = 0x01;
        }
        return NETSGCC_DCA_REASON3047_DC_FUSE;
    default:
        return 0x00;
    }
    return 0x00;
}
#else
uint16_t sgcc_chargepile_fault_converted(uint16_t bit, uint8_t *rank)
{
    return 0x00;
}
#endif /* NET_SGCC_PRO_USING_DC */

/*************************************************
 * 函数名      sgcc_chargepile_stop_reason_converted
 * 功能          停充原因转换
 * **********************************************/
#ifdef NET_SGCC_PRO_USING_DC
static uint16_t sgcc_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting)
{
    uint16_t _reason = NETSGCC_DCA_REASON7009_UNKNOW;

    switch(reason){
    /* 急停 */
    case APP_SYSTEM_STOP_WAY_SCRAM:
        _reason = NETSGCC_DCA_REASON3033_CRASH_STOP;
        break;
    /* 读卡器 */
    case APP_SYSTEM_STOP_WAY_CARDREADER:
        _reason = NETSGCC_DCA_REASON3037_CARD_READER;
        break;
    /* 门禁 */
    case APP_SYSTEM_STOP_WAY_DOOR:
        _reason = NETSGCC_DCA_REASON3032_OPNE_DOOR;
        break;
    /* 电表 */
    case APP_SYSTEM_STOP_WAY_AMMETER:
        _reason = NETSGCC_DCA_REASON3043_AMMETER_COMM;
        break;
    /* 充电模块 */
    case APP_SYSTEM_STOP_WAY_CHARGEMODULE:
        _reason = NETSGCC_DCA_REASON3038_MODULE_COMM;
        break;
    /* 过温 */
    case APP_SYSTEM_STOP_WAY_OVERTEMP:
        _reason = NETSGCC_DCA_REASON3053_CHARGE_PORT_OVERTEMP;
        break;
    /* 过、欠压 */
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        _reason = NETSGCC_DCPA_REASON4008_INPUT_POWER;
        break;
    /* 过流 */
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        _reason = NETSGCC_DCPA_REASON4010_OUTPUT_OVERCURR;
        break;
    /* DC 继电器 */
    case APP_SYSTEM_STOP_WAY_RELAY:
        _reason = NETSGCC_DCA_REASON3046_DC_RELAY;
        break;
    /* 并联 继电器 */
    case APP_SYSTEM_STOP_WAY_PARALLEL_RELAY:
        _reason = NETSGCC_DCA_REASON3068_PARALLEL_RELAY_ADH;
        break;
    /* AC 继电器 */
    case APP_SYSTEM_STOP_WAY_AC_RELAY:
        _reason = NETSGCC_DCA_REASON3065_AC_RELAY;
        break;
    /* 充满 */
    case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
        _reason = NETSGCC_GS_REASON1000_CHARGE_FULL;
        break;
    /* 拔枪 */
    case APP_SYSTEM_STOP_WAY_PULL_GUN:
        _reason = NETSGCC_DCA_REASON3057_CHARGE_LINK;
        break;
    /* 电子锁 */
    case APP_SYSTEM_STOP_WAY_ELECTRY_LOCK:
        _reason = NETSGCC_DCA_REASON3054_ELOCK;
        break;
    /* 通讯 */
    case APP_SYSTEM_STOP_WAY_COMMINICATION:
        _reason = NETSGCC_DCCA_REASON5001_BMS_COMM;
        break;
    /* 辅源 */
    case APP_SYSTEM_STOP_WAY_AUXPOWER:
        _reason = NETSGCC_DCA_REASON3049_AUXPOWER;
        break;
    /* 断电 */
    case APP_SYSTEM_STOP_WAY_POWER_OFF:
        _reason = NETSGCC_DCA_REASON7000_POWER_OFF;
        break;
    /* 存储芯片 */
    case APP_SYSTEM_STOP_WAY_FLASH:
    case APP_SYSTEM_STOP_WAY_EEPROM:
        _reason = NETSGCC_DCA_REASON7001_STORAGE_CHIP;
        break;
    /* 短路 */
    case APP_SYSTEM_STOP_WAY_SHORTS:
        _reason = NETSGCC_DCPA_REASON4012_OUTPUT_SHORTS;
        break;
    /* 枪电压 */
    case APP_SYSTEM_STOP_WAY_GUNVOLT:
        _reason = NETSGCC_DCPA_REASON4014_RELAY_OUTSIDE_OVER10V;
        break;
    /* 绝缘 */
    case APP_SYSTEM_STOP_WAY_INSULT:
        _reason = NETSGCC_DCA_REASON3050_INSULATION;
        break;
    /* 电池电压 */
    case APP_SYSTEM_STOP_WAY_BATTERY_VOLT:
        _reason = NETSGCC_DCCA_REASON5029_BATTERY_VOLT_ABNORMAL;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = NETSGCC_GS_REASON1011_BMS_STOP;
        break;
    /* 准备电压 */
    case APP_SYSTEM_STOP_WAY_READY_VOLT:
        _reason = NETSGCC_DCCA_REASON5017_BMS_READY_VOLT_NOMATCH;
        break;
    /* 绝缘电压 */
    case APP_SYSTEM_STOP_WAY_INSULT_VOLT:
        _reason = NETSGCC_DCA_REASON7002_INSULT_VOLT;
        break;
    /* BSM */
    case APP_SYSTEM_STOP_WAY_BSM:
        _reason = NETSGCC_DCCA_REASON5038_BMS_ABNORMAL_STOP;
        break;
    /* APP */
    case APP_SYSTEM_STOP_WAY_APP_STOP:
        _reason = NETSGCC_GS_REASON1002_SERVER;
        break;
    /* 刷卡 */
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = NETSGCC_GS_REASON1012_SWIP_CARD;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        _reason = NETSGCC_DCA_REASON7003_NOBALLANCE;
        break;
    /* 屏幕 */
    case APP_SYSTEM_STOP_WAY_SCREEN_STOP:
        _reason = NETSGCC_GS_REASON1001_SCREEN;
        break;
    /* 到达设定电量 */
    case APP_SYSTEM_STOP_WAY_REACH_ELECT:
        _reason = NETSGCC_GS_REASON1004_TARGET_ELECT;
        break;
    /* 到达设定时间 */
    case APP_SYSTEM_STOP_WAY_REACH_TIME:
        _reason = NETSGCC_GS_REASON1003_TARGET_TIME;
        break;
    /* 到达设定余额 */
    case APP_SYSTEM_STOP_WAY_REACH_MONEY:
        _reason = NETSGCC_GS_REASON1005_TARGET_MONEY;
        break;
    /* 充电电流异常 */
    case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
        _reason = NETSGCC_DCA_REASON7004_ABNORMAL_CURRENT;
        break;
    /* 达到SOC 限定值 */
    case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
        _reason = NETSGCC_GS_REASON1007_TARGET_SOC;
        break;
    /* 主机柜禁止充电 */
    case APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID:
        _reason = NETSGCC_DCA_REASON7005_CABINET_FORBID;
        break;
    /* 达到离线可充电时长 */
//    case APP_SYSTEM_STOP_WAY_OFFLINE_CHARGE_TIME:
//        _reason = NETSGCC_GS_REASON1006_OFFLINE_TIME;
//        break;
    default:
        break;
    }
    return _reason;
}
#else
static uint16_t sgcc_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting)
{
    uint16_t _reason = NETSGCC_ACA_REASON6002_UNKNOW;

    return _reason;
}
#endif /* NET_SGCC_PRO_USING_DC */

/*************************************************
 * 函数名      sgcc_query_transaction_verify_state
 * 功能          查询订单确认状态
 * **********************************************/
uint8_t sgcc_query_transaction_verify_state(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    if(sgcc_transaction_is_verify(gunno)){
        sgcc_set_transaction_verify_state(gunno, NET_ENUM_FALSE);
        return 0x01;
    }
    return 0x00;
}

static void sgcc_request_message_repeat(uint8_t gunno)
{
    uint8_t event = 0;
    if(sgcc_exist_message_wait_response(gunno, NULL)){
        for(event = 0; event < NET_SGCC_CHARGEPILE_PREQ_NUM; event++){
            if(sgcc_get_message_wait_response_timeout_state(gunno, NET_SGCC_WAIT_RESPONSE_TIMEOUT, event)){
                rt_kprintf("sgcc_request_message_repeat(%d, %d)\n", gunno, event);
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, event);
            }
        }
    }
}

static void sgcc_data_realtime_process(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData* base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(sgcc_get_socket_info()->state == SGCC_SOCKET_STATE_LOGIN_SUCCESS){
        sgcc_request_message_repeat(gunno);

        if(s_sgcc_faultwarn_count[gunno] > rt_tick_get()){
            s_sgcc_faultwarn_count[gunno] = rt_tick_get();
        }
        if((rt_tick_get() - s_sgcc_faultwarn_count[gunno]) > evs_data_dev_configs.faultWarnings *1000){
            if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING) == NET_SGCC_SEND_STATE_COMPLETE){
                s_sgcc_faultwarn_count[gunno] = rt_tick_get();
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING);
            }
        }

        if((base->state.current != APP_OFSM_STATE_CHARGING) && (base->state.current != APP_OFSM_STATE_STARTING)){
            if(s_sgcc_state_noncharging_count[gunno] > rt_tick_get()){
                s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
            }

            if((rt_tick_get() - s_sgcc_state_noncharging_count[gunno]) > evs_data_dev_configs.nonElecFreq *1000){
                s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
            }
            s_sgcc_state_charging_interval[gunno] = SGCC_STATE_INTERVAL_STARTING_DEF;
            s_sgcc_state_charging_count[gunno] = rt_tick_get();
            s_sgcc_state_charging_period_first[gunno] = rt_tick_get();

            s_sgcc_bms_data_report_num[gunno] = 0x00;
            s_sgcc_bms_data_count[gunno] = rt_tick_get();
        }else{
            /** 实时监测数据 */
            if(s_sgcc_state_charging_count[gunno] > rt_tick_get()){
                s_sgcc_state_charging_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_sgcc_state_charging_count[gunno]) > s_sgcc_state_charging_interval[gunno]){
                s_sgcc_state_charging_count[gunno] = rt_tick_get();
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING);
            }
            /** 启动前120s  */
            if(s_sgcc_state_charging_interval[gunno] == SGCC_STATE_INTERVAL_STARTING_DEF){
                if(s_sgcc_state_charging_period_first[gunno] > rt_tick_get()){
                    s_sgcc_state_charging_period_first[gunno] = rt_tick_get();
                }
                if((rt_tick_get() - s_sgcc_state_charging_period_first[gunno]) > SGCC_STATE_PERIOD_CHARGING_FIRST_DEF){
                    s_sgcc_state_charging_period_first[gunno] = rt_tick_get();
                    s_sgcc_state_charging_interval[gunno] = evs_data_dev_configs.gunElecFreq *1000;
                }
            }

            /** BMS数据 */
            if(s_sgcc_bms_data_report_num[gunno] < SGCC_BMS_DATA_REPORT_NUM_MAX){
                if(s_sgcc_bms_data_count[gunno] > rt_tick_get()){
                    s_sgcc_bms_data_count[gunno] = rt_tick_get();
                }
                if((rt_tick_get() - s_sgcc_bms_data_count[gunno]) > (SGCC_BMS_DATA_INTERVAL_DEF *1000)){
                    s_sgcc_bms_data_count[gunno] = rt_tick_get();
                    sgcc_chargepile_request_padding_bms_data(gunno, NET_ENUM_FALSE);
                }
            }
        }
    }else{
        s_sgcc_faultwarn_count[gunno] = rt_tick_get();

        s_sgcc_state_charging_interval[gunno] = SGCC_STATE_INTERVAL_STARTING_DEF;
        s_sgcc_state_charging_count[gunno] = rt_tick_get();
        s_sgcc_state_charging_period_first[gunno] = rt_tick_get();

        s_sgcc_bms_data_report_num[gunno] = 0x00;
        s_sgcc_bms_data_count[gunno] = rt_tick_get();

        s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
    }

    if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
#ifdef NET_SGCC_PRO_USING_DC
        evs_property_dc_nonWorks[gunno].conTemp1 = (base->gunline_temperature[0x00] + 500);
        evs_property_dc_nonWorks[gunno].conTemp2 = (base->gunline_temperature[0x01] + 500);
#else
        evs_property_ac_nonWorks[gunno].gunNo = gunno + 0x01;
        evs_property_ac_nonWorks[gunno].gunTemp = (base->gunline_temperature[0x00] + 500);
#endif /* #ifdef NET_SGCC_PRO_USING_DC */
    }

    if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
#ifdef NET_SGCC_PRO_USING_DC
        evs_property_dc_works[gunno].conTemp1 = (base->gunline_temperature[0x00] + 500);
        evs_property_dc_works[gunno].conTemp2 = (base->gunline_temperature[0x01] + 500);
#else
        evs_property_ac_works[gunno].gunTemp = (base->gunline_temperature[0x00] + 500);
#endif /* NET_SGCC_PRO_USING_DC */
    }

    if((rt_tick_get() - s_sgcc_oammeter_val_count[gunno]) > evs_data_dev_configs.dcMeterFreq *1000){
        s_sgcc_oammeter_val_count[gunno] = rt_tick_get();
        sgcc_chargepile_request_padding_out_ammeter_val(gunno);
    }

    if((rt_tick_get() - s_sgcc_monitor_property_count[0x00]) > evs_data_dev_configs.equipParamFreq *1000){
        s_sgcc_monitor_property_count[0x00] = rt_tick_get();
        sgcc_chargepile_request_padding_monitor_property(0x00);
    }

    switch(base->state.current){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_STOPING:
        sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);
        break;
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        break;
    case APP_OFSM_STATE_STARTING:
        if(base->start_type != APP_CHARGE_START_WAY_VIN){
            sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);
        }
        sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
        break;
    case APP_OFSM_STATE_CHARGING:
        sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE);
        sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
        break;
    default:
        break;
    }
}

/*
 * 用于检测到状态有变化时上报
 * */
static void sgcc_state_changed_check(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData* base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(evs_event_pile_stutus_changes[gunno].connCheckStatus != s_sgcc_state_info[gunno].state.connect){
        if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED) == NET_SGCC_SEND_STATE_COMPLETE){
            if((evs_event_pile_stutus_changes[gunno].connCheckStatus == 11) && (s_sgcc_state_info[gunno].state.connect == 10)){
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_CAR_INFO);
            }
            evs_event_pile_stutus_changes[gunno].connCheckStatus = s_sgcc_state_info[gunno].state.connect;
            evs_event_pile_stutus_changes[gunno].yxOccurTime = s_sgcc_state_info[gunno].timestamp;
            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED);
        }
    }

    /**********************************************[[非充电中的数据上报]**********************************************/
    /**********************************************[[非充电中的数据上报]**********************************************/
    if((base->state.current != APP_OFSM_STATE_CHARGING) && (base->state.current != APP_OFSM_STATE_STARTING)){
#ifdef NET_SGCC_PRO_USING_DC
        if((evs_property_dc_nonWorks[gunno].gunStatus != s_sgcc_state_info[gunno].state.connect) ||
                (evs_property_dc_nonWorks[gunno].workStatus != s_sgcc_state_info[gunno].state.state) ||
                (evs_property_dc_nonWorks[gunno].eLockStatus != s_sgcc_state_info[gunno].state.electlock) ||
                (evs_property_dc_nonWorks[gunno].DCK1Status != s_sgcc_state_info[gunno].state.dck1) ||
                (evs_property_dc_nonWorks[gunno].DCK2Status != s_sgcc_state_info[gunno].state.dck2) ||
                (evs_property_dc_nonWorks[gunno].DCPlusFuseStatus != s_sgcc_state_info[gunno].state.dc_plusFuse) ||
                (evs_property_dc_nonWorks[gunno].DCMinusFuseStatus != s_sgcc_state_info[gunno].state.dc_minusFuse)){
            if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
                evs_property_dc_nonWorks[gunno].gunStatus = s_sgcc_state_info[gunno].state.connect;
                evs_property_dc_nonWorks[gunno].workStatus = s_sgcc_state_info[gunno].state.state;
                evs_property_dc_nonWorks[gunno].eLockStatus = s_sgcc_state_info[gunno].state.electlock;
                evs_property_dc_nonWorks[gunno].DCK1Status = s_sgcc_state_info[gunno].state.dck1;
                evs_property_dc_nonWorks[gunno].DCK2Status = s_sgcc_state_info[gunno].state.dck2;
                evs_property_dc_nonWorks[gunno].DCPlusFuseStatus = s_sgcc_state_info[gunno].state.dc_plusFuse;
                evs_property_dc_nonWorks[gunno].DCMinusFuseStatus = s_sgcc_state_info[gunno].state.dc_minusFuse;
                s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
            }
        }
#else
        if((evs_property_ac_nonWorks[gunno].conStatus != s_sgcc_state_info[gunno].state.connect) ||
                (evs_property_ac_nonWorks[gunno].workStatus != s_sgcc_state_info[gunno].state.state) ||
                (evs_property_ac_nonWorks[gunno].eLockStatus != s_sgcc_state_info[gunno].state.electlock) ||
                (evs_property_ac_nonWorks[gunno].outRelayStatus != s_sgcc_state_info[gunno].state.dck1)){
            if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
                evs_property_ac_nonWorks[gunno].conStatus = s_sgcc_state_info[gunno].state.connect;
                evs_property_ac_nonWorks[gunno].workStatus = s_sgcc_state_info[gunno].state.state;
                evs_property_ac_nonWorks[gunno].eLockStatus = s_sgcc_state_info[gunno].state.electlock;
                evs_property_ac_nonWorks[gunno].outRelayStatus = s_sgcc_state_info[gunno].state.dck1;
                s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
            }
        }
#endif /* NET_SGCC_PRO_USING_DC */
    }else if(base->state.current == APP_OFSM_STATE_STARTING){
#ifdef NET_SGCC_PRO_USING_DC
        if(evs_property_dc_nonWorks[gunno].workStatus != SGCC_WORKSTATE_STARTING){
            evs_property_dc_nonWorks[gunno].gunStatus = s_sgcc_state_info[gunno].state.connect;
            evs_property_dc_nonWorks[gunno].workStatus = SGCC_WORKSTATE_STARTING;
            evs_property_dc_nonWorks[gunno].eLockStatus = s_sgcc_state_info[gunno].state.electlock;
            evs_property_dc_nonWorks[gunno].DCK1Status = s_sgcc_state_info[gunno].state.dck1;
            evs_property_dc_nonWorks[gunno].DCK2Status = s_sgcc_state_info[gunno].state.dck2;
            evs_property_dc_nonWorks[gunno].DCPlusFuseStatus = s_sgcc_state_info[gunno].state.dc_plusFuse;
            evs_property_dc_nonWorks[gunno].DCMinusFuseStatus = s_sgcc_state_info[gunno].state.dc_minusFuse;
            s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
        }
#else
        if(evs_property_ac_nonWorks[gunno].workStatus != SGCC_WORKSTATE_STARTING){
            evs_property_ac_nonWorks[gunno].conStatus = s_sgcc_state_info[gunno].state.connect;
            evs_property_ac_nonWorks[gunno].workStatus = SGCC_WORKSTATE_STARTING;
            evs_property_ac_nonWorks[gunno].eLockStatus = s_sgcc_state_info[gunno].state.electlock;
            evs_property_ac_nonWorks[gunno].outRelayStatus = s_sgcc_state_info[gunno].state.dck1;
            s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
            sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
        }
#endif /* NET_SGCC_PRO_USING_DC */
    }

    /**********************************************[[充电中的数据上报]**********************************************/
    /**********************************************[[充电中的数据上报]**********************************************/
    if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING)){
#ifdef NET_SGCC_PRO_USING_DC
        if((evs_property_dc_works[gunno].gunStatus != s_sgcc_state_info[gunno].state.connect) ||
                (evs_property_dc_works[gunno].workStatus != SGCC_WORKSTATE_CHARGINGING) ||
                (evs_property_dc_works[gunno].eLockStatus != s_sgcc_state_info[gunno].state.electlock) ||
                (evs_property_dc_works[gunno].DCK1Status != s_sgcc_state_info[gunno].state.dck1) ||
                (evs_property_dc_works[gunno].DCK2Status != s_sgcc_state_info[gunno].state.dck2) ||
                (evs_property_dc_works[gunno].DCPlusFuseStatus != s_sgcc_state_info[gunno].state.dc_plusFuse) ||
                (evs_property_dc_works[gunno].DCMinusFuseStatus != s_sgcc_state_info[gunno].state.dc_minusFuse)){
            if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
                evs_property_dc_works[gunno].gunStatus = s_sgcc_state_info[gunno].state.connect;
                evs_property_dc_works[gunno].workStatus = SGCC_WORKSTATE_CHARGINGING;
                evs_property_dc_works[gunno].eLockStatus = s_sgcc_state_info[gunno].state.electlock;
                evs_property_dc_works[gunno].DCK1Status = s_sgcc_state_info[gunno].state.dck1;
                evs_property_dc_works[gunno].DCK2Status = s_sgcc_state_info[gunno].state.dck2;
                evs_property_dc_works[gunno].DCPlusFuseStatus = s_sgcc_state_info[gunno].state.dc_plusFuse;
                evs_property_dc_works[gunno].DCMinusFuseStatus = s_sgcc_state_info[gunno].state.dc_minusFuse;

                s_sgcc_state_charging_count[gunno] = rt_tick_get();

                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING);
            }
        }
#else
        if((evs_property_ac_works[gunno].conStatus != s_sgcc_state_info[gunno].state.connect) ||
                (evs_property_ac_works[gunno].workStatus != SGCC_WORKSTATE_CHARGINGING) ||
                (evs_property_ac_works[gunno].eLockStatus != s_sgcc_state_info[gunno].state.electlock) ||
                (evs_property_ac_works[gunno].outRelayStatus != s_sgcc_state_info[gunno].state.dck1)){
            if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING) == NET_SGCC_SEND_STATE_COMPLETE){
                evs_property_ac_works[gunno].conStatus = s_sgcc_state_info[gunno].state.connect;
                evs_property_ac_works[gunno].workStatus = SGCC_WORKSTATE_CHARGINGING;
                evs_property_ac_works[gunno].eLockStatus = s_sgcc_state_info[gunno].state.electlock;
                evs_property_ac_works[gunno].outRelayStatus = s_sgcc_state_info[gunno].state.dck1;

                s_sgcc_state_charging_count[gunno] = rt_tick_get();

                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING);
            }
        }
#endif /* NET_SGCC_PRO_USING_DC */
    }

    /**********************************************[[漏报数据重报]**********************************************/
    /**********************************************[[漏报数据重报]**********************************************/
    if(s_sgcc_flag_info[gunno].omit_oammeter_val == NET_ENUM_TRUE){
        s_sgcc_flag_info[gunno].omit_oammeter_val = NET_ENUM_FALSE;

        s_sgcc_oammeter_val_count[gunno] = rt_tick_get();
        sgcc_chargepile_request_padding_out_ammeter_val(gunno);
    }
    if(s_sgcc_flag_info[gunno].omit_bms_data == NET_ENUM_TRUE){
        s_sgcc_flag_info[gunno].omit_bms_data = NET_ENUM_FALSE;

        s_sgcc_bms_data_count[gunno] = rt_tick_get();
        sgcc_chargepile_request_padding_bms_data(gunno, NET_ENUM_FALSE);
    }
    if(s_sgcc_flag_info[gunno].omit_monitor_property == NET_ENUM_TRUE){
        s_sgcc_flag_info[gunno].omit_monitor_property = NET_ENUM_FALSE;

        s_sgcc_monitor_property_count[0x00] = rt_tick_get();
        sgcc_chargepile_request_padding_monitor_property(0x00);
    }
}

static void sgcc_realtime_process_thread_entry(void *parameter)
{
    uint8_t gunno = 0x00;
    static System_BaseData *base = NULL;

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }
        if(s_sgcc_handle == NULL){
            rt_thread_mdelay(100);
            continue;
        }

        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_sgcc_flag_info[gunno].init_complete == NET_ENUM_FALSE){
                break;
            }
            sgcc_fault_detect_report(gunno);
            sgcc_data_realtime_process(gunno);
            sgcc_state_changed_check(gunno);
        }

        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            if(s_sgcc_flag_info[gunno].init_complete == NET_ENUM_FALSE){
                break;
            }
            if(s_sgcc_flag_info[gunno].is_orderly_charge == NET_ENUM_TRUE){
                base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
                if((base->state.current != APP_OFSM_STATE_STARTING) && (base->state.current != APP_OFSM_STATE_CHARGING)){
                    s_sgcc_flag_info[gunno].is_orderly_charge = NET_ENUM_FALSE;
                    s_sgcc_order_charge[gunno].current_index = (SGCC_ORDERLY_CHARGE_TIME_POINT_NUM + 0x01);
                }else{
                    uint8_t count = 0x00;
                    uint32_t time_sec = 0x00;
                    struct tm _tm;

                    sgcc_enter_critical();
                    _tm = *(localtime((const time_t*)&(base->current_time)));
                    sgcc_exit_critical();

                    time_sec = (_tm.tm_hour *60 *60 + _tm.tm_min *60 + _tm.tm_sec);
                    for(count = 0x00; count < SGCC_ORDERLY_CHARGE_TIME_POINT_NUM; count++){
                        if((count + 0x02) <= SGCC_ORDERLY_CHARGE_TIME_POINT_NUM){
                            if((time_sec >= s_sgcc_order_charge[gunno].time_sec[count]) &&  \
                                    (time_sec < s_sgcc_order_charge[gunno].time_sec[count + 0x01])){

                                if(s_sgcc_order_charge[gunno].current_index != count){
                                    s_sgcc_order_charge[gunno].current_index = count;

                                    net_operation_set_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER);
                                    net_operation_set_total_power(s_sgcc_order_charge[gunno].period_power[count] *100, gunno);
                                    s_sgcc_flag_info[gunno].is_set_power = NET_ENUM_TRUE;
                                    base->power_strategy = APP_POWER_STRATEGY_ORDER;
                                    base->power_strategy_para = 0x00;
                                    break;
                                }else{
                                    break;
                                }
                            }else if(time_sec == s_sgcc_order_charge[gunno].time_sec[count]){
                                break;
                            }
                        }else{
                            if(s_sgcc_order_charge[gunno].current_index != (SGCC_ORDERLY_CHARGE_TIME_POINT_NUM - 0x01)){
                                s_sgcc_order_charge[gunno].current_index = (SGCC_ORDERLY_CHARGE_TIME_POINT_NUM - 0x01);

                                net_operation_set_event(gunno, NET_OPERATION_EVENT_SET_CHARGE_POWER);
                                net_operation_set_total_power(s_sgcc_order_charge[gunno].period_power[(SGCC_ORDERLY_CHARGE_TIME_POINT_NUM - 0x01)] *100, gunno);
                                s_sgcc_flag_info[gunno].is_set_power = NET_ENUM_TRUE;
                                base->power_strategy = APP_POWER_STRATEGY_ORDER;
                                base->power_strategy_para = 0x00;
                                break;
                            }
                        }
                    }
                }
            }
        }
        rt_thread_mdelay(100);
    }
}

int sgcc_realtime_process_init(void)
{
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_sgcc_charge_sn[gunno] = 0x01;
        s_sgcc_state_info[gunno].state.connect = SGCC_OPSCTL_SILENT;
#ifdef NET_DESIGNATE_REGION
        s_sgcc_state_noncharging_count[gunno] = 0x00;
        s_sgcc_state_charging_count[gunno] = 0x00;
        s_sgcc_state_charging_interval[gunno] = 0x00;
        s_sgcc_state_charging_period_first[gunno] = 0x00;
        s_sgcc_oammeter_val_count[gunno] = 0x00;
        s_sgcc_faultwarn_count[gunno] = 0x00;
        s_sgcc_bms_data_count[gunno] = 0x00;
        s_sgcc_bms_data_report_num[gunno] = 0x00;
        s_sgcc_monitor_property_count[gunno] = 0x00;
        s_sgcc_state_noncharging_count[gunno] = 0x00;
        s_sgcc_charge_sn[gunno] = 0x00;
        s_sgcc_operation_sn[gunno] = 0x00;

        s_agcc_applycharge_result[gunno] = 0x00;
        s_agcc_applycharge_fault_reason[gunno] = 0x00;
        s_agcc_applycharge_fail_reason[gunno] = 0x00;

        s_agcc_remotecharge_result[gunno] = 0x00;
        s_agcc_remotecharge_fault_reason[gunno] = 0x00;
        s_agcc_remotecharge_fail_reason[gunno] = 0x00;

        s_agcc_remotestop_result[gunno] = 0x00;
        s_agcc_remotestop_fault_reason[gunno] = 0x00;
        s_agcc_remotestop_fail_reason[gunno] = 0x00;

        memset(&s_sgcc_order_charge[gunno], 0x00, sizeof(s_sgcc_order_charge[gunno]));
        memset(&s_sgcc_flag_info[gunno], 0x00, sizeof(s_sgcc_flag_info[gunno]));
        memset(&s_sgcc_state_info[gunno], 0x00, sizeof(s_sgcc_state_info[gunno]));
#endif /* NET_DESIGNATE_REGION */
    }
#ifdef NET_DESIGNATE_REGION
    s_sgcc_handle = NULL;
#endif /* NET_DESIGNATE_REGION */

    if(rt_thread_init(&s_sgcc_realtime_process_thread, "sgcc_rl_pro", sgcc_realtime_process_thread_entry, NULL,
            s_sgcc_realtime_process_thread_stack, SGCC_REALTIME_PROCESS_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("sgcc realtime process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_realtime_process_thread) != RT_EOK){
        LOG_E("sgcc realtime process thread startup fail, please check");
        return -0x01;
    }

    return 0x00;
}

#endif /* NET_PACK_USING_SGCC */
