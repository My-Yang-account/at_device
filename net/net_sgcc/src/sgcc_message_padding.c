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
//#include "sgcc_message_receive.h"
#include "sgcc_device_register.h"

#include "app_ofsm.h"
#include "app_billing_rule.h"
#include "net_operation.h"

#define DBG_TAG "gw_rl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

#define SGCC_DISPOSABLE_EVENT_CONNECT              0x00          /* 漏报事件：桩状态 */

#define SGCC_REALTIME_DATA_INTERVAL_INIT           0x05          /* 刚连上网时实时数据上报间隔 */
#define SGCC_REALTIME_DATA_INTERVAL_CHARGING       0x0F          /* 充电中实时数据上报间隔  */
#define SGCC_REALTIME_DATA_INTERVAL_IDLE           0x05 *60      /* 空闲实时数据上报间隔  */
#define SGCC_STATE_INTERVAL_NONCHARGING_DEF        180 *1000     /* 非充电中实时数据上报间隔  */

#define SGCC_REALTIME_PROCESS_THREAD_STACK_SIZE    1536          /* 实时处理线程栈大小 */

#pragma pack(1)

struct sgcc_disposable_info{
    struct{
        uint8_t connect : 4;
        uint8_t reserve : 4;
    }state;                                       /* 桩状态 */
    uint32_t timestamp;                           /* 时间 */
    uint8_t disposable_event;
};

struct sgcc_flag_info{
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

static struct sgcc_flag_info s_sgcc_flag_info[NET_SYSTEM_GUN_NUMBER];
static struct sgcc_disposable_info s_sgcc_disposable_info[NET_SYSTEM_GUN_NUMBER];
static uint32_t s_sgcc_state_noncharging_count[NET_SYSTEM_GUN_NUMBER];

static struct rt_thread s_sgcc_realtime_process_thread;
static uint8_t s_sgcc_realtime_process_thread_stack[4096];

static struct net_handle* s_sgcc_handle = NULL;
static System_BaseData *s_sgcc_base = NULL;

static uint8_t sgcc_chargepile_transaction_identity_converted(uint8_t identity, uint8_t online_order);
static uint16_t sgcc_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting);

/*************************************************
 * 函数名      sgcc_set_clear_disposable_event
 * 功能          设置漏报报文事件
 * **********************************************/
static void sgcc_set_clear_disposable_event(uint8_t gunno, uint8_t event, uint8_t is_clear)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(is_clear){
        s_sgcc_disposable_info[gunno].disposable_event &= (~(1 <<event));
    }else{
        s_sgcc_disposable_info[gunno].disposable_event |= (1 <<event);
    }
}

/*************************************************
 * 函数名      sgcc_get_disposable_event
 * 功能          获取漏报报文事件
 * **********************************************/
static uint8_t sgcc_get_disposable_event(uint8_t gunno, uint8_t event, uint8_t *buf)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(buf){
        *buf = s_sgcc_disposable_info[gunno].disposable_event;
    }
    if(s_sgcc_disposable_info[gunno].disposable_event &(1 <<event)){
        return 0x01;
    }
    return 0x00;
}

/*************************************************
 * 函数名      sgcc_storage_data_check
 * 功能          校验存储的平台数据
 * **********************************************/
static void sgcc_storage_data_check(void)
{
    uint8_t verify_success = 0x01;
    sgcc_storage_struct *config = (sgcc_storage_struct*)(s_sgcc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));

//    memset(config, 0x00, sizeof(sgcc_storage_struct));
    memset(config->product_key, 0x00, sizeof(config->product_key));
    memcpy(config->product_key, "a1q5OZTEiYV", strlen("a1q5OZTEiYV"));

    memset(config->device_name, 0x00, sizeof(config->device_name));
    memcpy(config->device_name, "961720420097522411938385", strlen("961720420097522411938385"));

    memset(config->device_secret, 0x00, sizeof(config->device_secret));
    memcpy(config->device_secret, "7d42a44b1cda88a96db18cb981021c55", strlen("7d42a44b1cda88a96db18cb981021c55"));
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

    if(verify_success){

    }else{

    }

    sgcc_register_storage_struct(config);
}


/*************************************************
 * 函数名      sgcc_message_info_init
 * 功能          国网报文信息初始化
 * **********************************************/
void sgcc_message_info_init(void)  ///////// 这是网络部分外部调用的第一个函数(可以在里面进行相关初始化)
{
    static uint8_t sgcc_is_init = NET_ENUM_FALSE;
    if(sgcc_is_init){
        return;
    }
    s_sgcc_base = (System_BaseData*)(s_sgcc_handle->get_base_data(0x00));

#if 0
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YKC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = NULL;
    pile_number = (uint8_t*)(s_ykc_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, option));
    s_ykc_base = (System_BaseData*)(s_ykc_handle->get_base_data(0x00));

    ykc_is_init = NET_ENUM_TRUE;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ykc_realtime_data_interval[gunno] = YKC_REALTIME_DATA_INTERVAL_INIT;
        s_ykc_realtime_data_count[gunno] = rt_tick_get();
    }
    memset(&s_ykc_flag_info, 0x00, sizeof(s_ykc_flag_info));
#endif
    /** 初始化登录签到 */
    unsigned char meterAddr1[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
    unsigned char meterAddr2[6] = {0x23, 0x34, 0x45, 0x56, 0x67, 0x78};

    memset(evs_event_firmware_infos.inMeter, 0, sizeof(evs_event_firmware_infos.inMeter));
    memset(evs_event_firmware_infos.outMeter, 0, sizeof(evs_event_firmware_infos.outMeter));
    memcpy(evs_event_firmware_infos.inMeter[0], meterAddr1, 6);
    memcpy(evs_event_firmware_infos.inMeter[1], meterAddr2, 6);
    memcpy(evs_event_firmware_infos.outMeter[0], meterAddr1, 6);
    memset(evs_event_firmware_infos.btMac, 0, 33);
    memcpy(evs_event_firmware_infos.btMac, "aa:bb:cc:dd", strlen("aa:bb:cc:dd"));
    evs_event_firmware_infos.CT = 30;
    memset(evs_event_firmware_infos.devSn, 0, 17);
    memcpy(evs_event_firmware_infos.devSn, "2222aaaabbccdd", strlen("2222aaaabbccdd"));
    evs_event_firmware_infos.devType = 10;
    evs_event_firmware_infos.gridType = 12;
    evs_event_firmware_infos.height = 100;
    evs_event_firmware_infos.isGateLock = 10;
    evs_event_firmware_infos.isGroundLock = 10;
    evs_event_firmware_infos.latitude = 0;
    evs_event_firmware_infos.longitude = 0;
    evs_event_firmware_infos.meaType = 10;
    memset(evs_event_firmware_infos.feeModelId, 0, 17);
    memcpy(evs_event_firmware_infos.feeModelId, "22222aaaabbccdd", strlen("22222aaaabbccdd"));
    evs_event_firmware_infos.otMaxVol = 500;
    evs_event_firmware_infos.otMinVol = 220;
    evs_event_firmware_infos.otRate = 70;
    evs_event_firmware_infos.otCur = 380;
    evs_event_firmware_infos.portNum = 2;
    memset(evs_event_firmware_infos.simMac, 0, 33);
    memset(evs_event_firmware_infos.simNo, 0, 20);
    memset(evs_event_firmware_infos.stakeModel, 0, 20);
    memcpy(evs_event_firmware_infos.simMac, "aa:bb:cc:d", strlen("aa:bb:cc:d"));
    memcpy(evs_event_firmware_infos.simNo, "111111111111111111", strlen("111111111111111111"));
    memcpy(evs_event_firmware_infos.stakeModel, "222111111111111111", strlen("222111111111111111"));
    evs_event_firmware_infos.vendorCode = 1287;
    evs_event_firmware_infos.mutliChargingMode = 10;




#if 0
    g_ykc_preq_login.head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
    g_ykc_preq_login.head.sequence = 0x00;

    ykc_ascii_to_bcd(pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    g_ykc_preq_login.body.pile_type = NET_YKC_PILE_TYPE_DC;
    g_ykc_preq_login.body.gun_count = NET_SYSTEM_GUN_NUMBER;
    g_ykc_preq_login.body.protocol_ver = NET_YKC_PROTOCOL_VERSION;

    memset(g_ykc_preq_login.body.software_ver, '\0', sizeof(g_ykc_preq_login.body.software_ver));
    g_ykc_preq_login.body.software_ver[0] = s_ykc_base->soft_ver_main + '0';
    g_ykc_preq_login.body.software_ver[1] = '.';
    g_ykc_preq_login.body.software_ver[2] = s_ykc_base->soft_ver_sub + '0';
    g_ykc_preq_login.body.software_ver[3] = '.';
    sprintf((char *)&g_ykc_preq_login.body.software_ver[3 + 1], "%2d", s_ykc_base->soft_ver_revise);

    g_ykc_preq_login.body.net_link_type = NET_YKC_NET_LINK_TYPE_SIM;
#endif
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        /** 初始实时数据(非充电中)请求 */
        evs_property_dc_nonWorks[gunno].gunNo = gunno + 0x01;
        evs_property_dc_nonWorks[gunno].workStatus = SGCC_WORKSTATE_IDLE;
        evs_property_dc_nonWorks[gunno].gunStatus = SGCC_OPSCTL_SILENT;
        /** 初始实时数据(充电中)请求 */
        evs_property_dc_works[gunno].gunNo = gunno + 0x01;
        evs_property_dc_works[gunno].workStatus = SGCC_WORKSTATE_IDLE;
        evs_property_dc_works[gunno].gunStatus = SGCC_OPSCTL_SILENT;
        /** 初始化桩状态变化上报请求 */
        evs_event_pile_stutus_changes[gunno].gunNo = gunno + 0x01;
        /** 初始化上报交易记录请求 */
        evs_event_tradeInfos[gunno].gunNo = gunno + 0x01;
//        g_ykc_preq_heartbeat[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_heartbeat[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_heartbeat[gunno].body.gunno = gunno + 0x01;
//        g_ykc_preq_heartbeat[gunno].body.state = 0x00;     /* 心跳状态默认正常 */
//
//        /** 初始化读取实时数据响应(实时数据请求) */
//        g_ykc_preq_report_realtime_data[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_report_realtime_data[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_report_realtime_data[gunno].body.gunno = gunno + 0x01;
//        g_ykc_preq_report_realtime_data[gunno].body.hardware_fault = 0x00;
//
//        /** 初始化充电握手请求 */
//        g_ykc_preq_shake_hand[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_shake_hand[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_shake_hand[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化参数配置请求 */
//        g_ykc_preq_parameter_config[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_parameter_config[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_parameter_config[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电结束请求 */
//        g_ykc_preq_charge_finish[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_charge_finish[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_charge_finish[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化错误报文请求 */
//        g_ykc_preq_error_message[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_error_message[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_error_message[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电过程中 BMS 终止请求 */
//        g_ykc_preq_bms_end[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_bms_end[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_bms_end[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电过程中充电机终止请求 */
//        g_ykc_preq_charger_end[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_charger_end[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_charger_end[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电过程 BMS 需求与充电机输出请求 */
//        g_ykc_preq_bmscommand_chargerout[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_bmscommand_chargerout[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_bmscommand_chargerout[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电过程 BMS 信息请求 */
//        g_ykc_preq_bms_info[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_bms_info[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_bms_info[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电桩主动申请启动充电请求 */
//        g_ykc_preq_apply_charge_active[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_apply_charge_active[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_apply_charge_active[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化交易记录请求 */
//        g_ykc_preq_transaction_records[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_transaction_records[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_transaction_records[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化地锁数据上送请求 */
//        g_ykc_preq_ground_lock_info[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_ground_lock_info[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_ground_lock_info[gunno].body.gunno = gunno + 0x01;
//
//        /** 初始化充电桩主动申请并充充电请求 */
//        g_ykc_preq_apply_merge_charge_active[gunno].head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
//        memcpy(g_ykc_preq_apply_merge_charge_active[gunno].body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
//        g_ykc_preq_apply_merge_charge_active[gunno].body.gunno = gunno + 0x01;

    }
#if 0
    /** 初始化计费模型验证请求 */
    g_ykc_preq_billing_model_verify.head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
    memcpy(g_ykc_preq_billing_model_verify.body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
    g_ykc_preq_billing_model_verify.body.model_number = 0x00;

    /** 初始化计费模型请求 */
    g_ykc_preq_billing_model_request.head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
    memcpy(g_ykc_preq_billing_model_request.body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);

    /** 初始化升级结果响应 */
    g_ykc_pres_remote_update.head.encrypt = NET_YKC_MESSAGE_ENCRYPT_DISABLE;
    memcpy(g_ykc_pres_remote_update.body.pile_number, g_ykc_preq_login.body.pile_number, NET_YKC_CHARGEPILE_LENGTH_DEFAULT);
#endif
    sgcc_storage_data_check();

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

    s_sgcc_base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(is_init){
        uint8_t valid_len = sizeof(s_sgcc_base->transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_property_dc_works[gunno].preTradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_dc_works[gunno].preTradeNo, s_sgcc_base->transaction_number, valid_len);

        valid_len = sizeof(s_sgcc_base->device_transaction_number);
        valid_len = valid_len > EVS_MAX_TRADE_LEN ? EVS_MAX_TRADE_LEN : valid_len;
        memset(evs_property_dc_works[gunno].tradeNo, 0x00, EVS_MAX_TRADE_LEN);
        memcpy(evs_property_dc_works[gunno].tradeNo, s_sgcc_base->device_transaction_number, valid_len);

        evs_property_dc_works[gunno].chgType = sgcc_chargepile_transaction_identity_converted(s_sgcc_base->start_type, \
                !(s_sgcc_base->flag.is_local_charging));
#if 0
        ykc_monitor_chargepile_request_padding_bmscommand_chargerout(gunno, NET_ENUM_TRUE);
        ykc_monitor_chargepile_request_padding_bmsinfo_duringcharge(gunno, NET_ENUM_TRUE);
#endif
        return;
    }

    if(s_sgcc_base->state.current == APP_OFSM_STATE_CHARGING){
        struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(s_sgcc_base->bms_data);

        evs_property_dc_works[gunno].eLockStatus = SGCC_OPSCTL_SILENT;                      // 4 充电枪电子锁状态
        evs_property_dc_works[gunno].DCK1Status = SGCC_OPSCTL_SILENT;                       // 5 直流输出接触器K1状态
        evs_property_dc_works[gunno].DCK2Status = SGCC_OPSCTL_SILENT;                       // 6 直流输出接触器K2状态
        evs_property_dc_works[gunno].DCPlusFuseStatus = SGCC_OPSCTL_SILENT;                 // 7 DC+熔断器状态
        evs_property_dc_works[gunno].DCMinusFuseStatus = SGCC_OPSCTL_SILENT;                // 8 DC-熔断器状态
        evs_property_dc_works[gunno].conTemp1 = (s_sgcc_base->gunline_temperature[0x00] + 500);                        // 9 充电接口DC+温度
        evs_property_dc_works[gunno].conTemp2 = (s_sgcc_base->gunline_temperature[0x01] + 500);                        // 10 充电接口DC-温度
        evs_property_dc_works[gunno].dcVol = s_sgcc_base->voltage_a /10;                             // 11 输出电压
        evs_property_dc_works[gunno].dcCur = s_sgcc_base->current_a;                             // 12 输出电流
        evs_property_dc_works[gunno].realPower = s_sgcc_base->power_a /100;                         // 16 充电设备输出功率
        evs_property_dc_works[gunno].chgTime = s_sgcc_base->charge_time /60;                           // 17 累计充电时间
        evs_property_dc_works[gunno].remainT = bms->BCS.SurplChgTime;                         // 18 估算充满剩余充电时间
        evs_property_dc_works[gunno].socVal = bms->BCS.SOC;                           // 19 SOC
        evs_property_dc_works[gunno].needVol = bms->BCL.BMSneedVolt;                         // 20 充电需求电压
        evs_property_dc_works[gunno].needCur = bms->BCL.BMSneedCurlt;                         // 21 充电需求电流
        evs_property_dc_works[gunno].chargeMode = bms->BCL.ChagModel;                       // 22 充电模式
        evs_property_dc_works[gunno].bmsVol = bms->BCS.ChargVolt;                          // 23 BMS充电电压测量值
        evs_property_dc_works[gunno].bmsCur = (4000 - bms->BCS.ChargCurlt);                          // 24 BMS充电电流测量值
        evs_property_dc_works[gunno].SingleMHV = bms->BCS.CellHigVolt;                       // 25 最高单体动力蓄电池电压
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_works[gunno].SingleMLV = bms->BCS.CellHigVolt;                       // 26 最高单体动力蓄电池电压
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_works[gunno].MHTemp = (bms->BSM.HigTemp + 50) *10;                          // 27 最高动力蓄电池温度
        evs_property_dc_works[gunno].MLTemp = (bms->BSM.LowTemp + 50) *10;                          // 28 最低动力蓄电池温度
        evs_property_dc_works[gunno].SingleMHVNo = bms->BCS.HigVoltCellNum;                       // 29 最高单体动力蓄电池电压所在编号
        evs_property_dc_works[gunno].MHTempNo = bms->BSM.HigTempNum;                          // 30 最高动力蓄电池温度检测点编号
        evs_property_dc_works[gunno].MLTempNo = bms->BSM.LowTempNum;                          // 31 最低动力蓄电池温度检测点编号
        evs_property_dc_works[gunno].guidanceVol = 0x00;
        evs_property_dc_works[gunno].acInputContactorState = SGCC_OPSCTL_SILENT;            // 33 交流输入接触器状态
        evs_property_dc_works[gunno].acInputContactorCtrlState = SGCC_OPSCTL_SILENT;        // 34 交流输入接触器控制状态
        evs_property_dc_works[gunno].k1CtrlState = SGCC_OPSCTL_SILENT;                      // 35 直流接触器K1控制状态
        evs_property_dc_works[gunno].k2CtrlState = SGCC_OPSCTL_SILENT;                      // 36 直流接触器K2控制状态
        evs_property_dc_works[gunno].apsSwtichCtrlState = SGCC_OPSCTL_SILENT;               // 37 交流输入接触器控制状态
        evs_property_dc_works[gunno].carbinFanCtrlState = SGCC_OPSCTL_SILENT;               // 38 风机开关控制状态
        evs_property_dc_works[gunno].elockCtrlState = SGCC_OPSCTL_SILENT;                   // 39 电子锁控制状态
        evs_property_dc_works[gunno].meterStartVal = s_sgcc_base->start_elect *10;                        // 40 表底值起始值
        evs_property_dc_works[gunno].meterRealVal = s_sgcc_base->current_elect *10;                         // 41 表底值当前值
        evs_property_dc_works[gunno].totalElect = s_sgcc_base->elect_a *10;                        // 42 总电量
        evs_property_dc_works[gunno].totalCost = s_sgcc_base->fees_total;                         // 43 总金额
        evs_property_dc_works[gunno].totalPowerCost = s_sgcc_base->elect_fees_total;                    // 44 总电费
        evs_property_dc_works[gunno].totalServCost = s_sgcc_base->service_fees_total;                     // 45 总服务费
        evs_property_dc_works[gunno].timeNum = s_sgcc_base->period_num;                          // 46 时段数

        for(uint8_t count = 0x00; count < APP_BILLING_RULE_PERIOD_MAX; count++){
            evs_property_dc_works[gunno].partElect[count] = app_billingrule_get_period_elect(gunno, count);
            evs_property_dc_works[gunno].chargeFee[count] = app_billingrule_get_period_elect_fees(gunno, count);
            evs_property_dc_works[gunno].serviceFee[count] = app_billingrule_get_period_service_fees(gunno, count);
        }
        evs_property_dc_works[gunno].startPoint = s_sgcc_base->start_period;                       // 50 起始点标识
        evs_property_dc_works[gunno].crossPoints = s_sgcc_base->period_num;                      // 51 跨越点数
//        evs_property_dc_works[gunno].pointsElect[EVS_MAX_MODEL_DEVSEG]; // 52 跨越点电量
    }else{
        evs_property_dc_nonWorks[gunno].eLockStatus = SGCC_OPSCTL_SILENT;          // 4 充电枪电子锁状态
        evs_property_dc_nonWorks[gunno].DCK1Status = SGCC_OPSCTL_SILENT;           // 5 直流输出接触器K1状态
        evs_property_dc_nonWorks[gunno].DCK2Status = SGCC_OPSCTL_SILENT;           // 6 直流输出接触器K2状态
        evs_property_dc_nonWorks[gunno].DCPlusFuseStatus = SGCC_OPSCTL_SILENT;     // 7 DC+熔断器状态
        evs_property_dc_nonWorks[gunno].DCMinusFuseStatus = SGCC_OPSCTL_SILENT;    // 8 DC-熔断器状态
        evs_property_dc_nonWorks[gunno].conTemp1 = (s_sgcc_base->gunline_temperature[0x00] + 500);            // 9 充电接口DC+温度
        evs_property_dc_nonWorks[gunno].conTemp2 = (s_sgcc_base->gunline_temperature[0x01] + 500);            // 10 充电接口DC-温度
        evs_property_dc_nonWorks[gunno].dcVol = 0x00;                 // 11 输出电压
        evs_property_dc_nonWorks[gunno].dcCur = 0x00;                 // 12 输出电流
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_nonWorks[gunno].chargeCnt = 0x00;             // 13 充电枪总充电次数
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_nonWorks[gunno].chargeTime = 0x00;            // 14 充电枪总充电时长
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_nonWorks[gunno].k1Cnt = 0x00;                 // 15 K1总动作次数
        evs_property_dc_nonWorks[gunno].k2Cnt = 0x00;                 // 16 K2总动作次数
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_nonWorks[gunno].acContactorState = SGCC_OPSCTL_SILENT;     // 17 交流输入接触器状态
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_nonWorks[gunno].guidanceVal = 0x00;          // 18 控制导引电压
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_property_dc_nonWorks[gunno].auxiliaryPowerStatus = SGCC_OPSCTL_SILENT; // 19 辅助电源开关控制状态
        evs_property_dc_nonWorks[gunno].elockCtrlState = SGCC_OPSCTL_SILENT;       // 20 电子锁控制状态
        evs_property_dc_nonWorks[gunno].fanSwitchCtrlStatus = SGCC_OPSCTL_SILENT;  // 21 风机开关控制状态
        evs_property_dc_nonWorks[gunno].sumMeter = s_sgcc_base->current_elect *10;                 // 22 电表底值
    }
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

    s_sgcc_base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));
    sgcc_set_transaction_verify_state(gunno, 0x00);

    if((sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD) == NET_SGCC_SEND_STATE_COMPLETE) || !is_repeat){
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

        evs_event_tradeInfos[gunno].timeDivType = SGCC_OPSCTL_ACTION;                      // 5 计量计费类型
        evs_event_tradeInfos[gunno].startType = sgcc_chargepile_transaction_identity_converted(_transaction->start_type, \
                _transaction->order_state.online_order);                        // 6 启动方式
        evs_event_tradeInfos[gunno].chargeStartTime = _transaction->start_time;                   // 7 开始充电时间
        evs_event_tradeInfos[gunno].chargeEndTime = _transaction->end_time;                     // 8 结束充电时间
        evs_event_tradeInfos[gunno].startSoc = _transaction->start_soc;                         // 9 启动时SOC
        evs_event_tradeInfos[gunno].endSoc = _transaction->stop_soc;                           // 10 停止时SOC
        evs_event_tradeInfos[gunno].reason = sgcc_chargepile_stop_reason_converted(_transaction->stop_reason, _transaction->order_state.is_start_fail);                            // 11 停止充电原因

        valid_len = sizeof(_transaction->rule.model_sn);
        valid_len = valid_len > EVS_MAX_MODEL_ID_LEN ? EVS_MAX_MODEL_ID_LEN : valid_len;
        memset(evs_event_tradeInfos[gunno].feeModelId, 0x00, sizeof(evs_event_tradeInfos[gunno].feeModelId));
        memcpy(evs_event_tradeInfos[gunno].feeModelId, _transaction->rule.model_sn, valid_len);

        evs_event_tradeInfos[gunno].sumStart = _transaction->ammeter_start *10;                             // 13 电表总起示值
        evs_event_tradeInfos[gunno].sumEnd = _transaction->ammeter_stop *10;                               // 14 电表总止示值
        evs_event_tradeInfos[gunno].totalElect = _transaction->total_elect *10;                        // 15 总电量
        evs_event_tradeInfos[gunno].totalPowerCost = _transaction->charge_fee;                    // 16 总电费
        evs_event_tradeInfos[gunno].totalServCost = _transaction->service_fee;                     // 17 总服务费
        evs_event_tradeInfos[gunno].totalCost = _transaction->total_fee;                         // 18 总消费金额
        evs_event_tradeInfos[gunno].timeNum = _transaction->period_count;                          // 19 时段数

        for(uint8_t count = 0x00; count < APP_BILLING_RULE_PERIOD_MAX; count++){
            evs_event_tradeInfos[gunno].partElect[count] = _transaction->period_elect[count] *10;
            evs_event_tradeInfos[gunno].chargeFee[count] = _transaction->period_elect_fees[count];
            evs_event_tradeInfos[gunno].serviceFee[count] = _transaction->period_service_fees[count];
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        evs_event_tradeInfos[gunno].startPoint = _transaction->start_period_number;                       // 23 起始点标识
        evs_event_tradeInfos[gunno].crossPoints = _transaction->period_count;                      // 24 跨越点数
//        evs_event_tradeInfos[gunno].pointsElect[EVS_MAX_MODEL_DEVSEG]; // 25 跨越点电量
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////

        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
        return 0x01;
    }

    return 0x00;
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

    s_sgcc_base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(s_sgcc_base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        s_sgcc_disposable_info[gunno].state.connect = SGCC_OPSCTL_ACTION;
    }else{
        s_sgcc_disposable_info[gunno].state.connect = SGCC_OPSCTL_SILENT;
    }

    s_sgcc_disposable_info[gunno].timestamp = s_sgcc_base->current_time;
    if((s_sgcc_disposable_info[gunno].state.connect == evs_event_pile_stutus_changes[gunno].connCheckStatus)){
        sgcc_set_clear_disposable_event(gunno, SGCC_DISPOSABLE_EVENT_CONNECT, NET_ENUM_TRUE);
        return;
    }
    if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED) == NET_SGCC_SEND_STATE_ONGOING){
        sgcc_set_clear_disposable_event(gunno, SGCC_DISPOSABLE_EVENT_CONNECT, NET_ENUM_FALSE);
        return;
    }

    evs_event_pile_stutus_changes[gunno].connCheckStatus = s_sgcc_disposable_info[gunno].state.connect;
    evs_event_pile_stutus_changes[gunno].yxOccurTime = s_sgcc_disposable_info[gunno].timestamp;
    sgcc_set_clear_disposable_event(gunno, SGCC_DISPOSABLE_EVENT_CONNECT, NET_ENUM_TRUE);

    sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED);
}

/*************************************************
 * 函数名      sgcc_chargepile_create_local_transaction_number
 * 功能          创建本地交易号
 * **********************************************/
int8_t sgcc_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
#if 0
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

#endif
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
        return SGCC_START_TYPE_APP_ONE_CLICK;
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
 * 函数名      sgcc_chargepile_stop_reason_converted
 * 功能          停充原因转换
 * **********************************************/
static uint16_t sgcc_chargepile_stop_reason_converted(uint8_t reason, uint8_t stop_in_starting)
{
#if 0
    uint16_t _reason = NETYKC_AS_REASON90_UNKNOW;

    switch(reason){
    /* 急停 */
    case APP_SYSTEM_STOP_WAY_SCRAM:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON50_EMERGENCY_STOP;
        }else{
            _reason = NETYKC_AS_REASON72_EMERGENCY_STOP;
        }
        break;
    /* 读卡器、门禁 */
    case APP_SYSTEM_STOP_WAY_CARDREADER:
    case APP_SYSTEM_STOP_WAY_DOOR:
        _reason = NETYKC_AS_REASON8A_CARDREADER;
        break;
    /* 电表 */
    case APP_SYSTEM_STOP_WAY_AMMETER:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON4D_AMMETER_COMMUNICATION;
        }else{
            _reason = NETYKC_AS_REASON6D_AMMETER_COMMUNICATE;
        }
        break;
    /* 充电模块 */
    case APP_SYSTEM_STOP_WAY_CHARGEMODULE:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON4F_CHARGE_MODULE;
        }else{
            _reason = NETYKC_AS_REASON71_CHARGE_MODULE;
        }
        break;
    /* 过温 */
    case APP_SYSTEM_STOP_WAY_OVERTEMP:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON53_ABNORMAL_TEMP;
        }else{
            _reason = NETYKC_AS_REASON74_TEMPERATURE_ABNORMAL;
        }
        break;
    /* 过、欠压 */
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        _reason = NETYKC_AS_REASON79_CHARGE_TVOLTAGE_ABNORMAL;
        break;
    /* 过流 */
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        _reason = NETYKC_AS_REASON7A_CHARGE_TCURRENT_ABNORMAL;
        break;
    /* DC 继电器 */
    case APP_SYSTEM_STOP_WAY_RELAY:
        _reason = NETYKC_AS_REASON8B_DC_RELAY;
        break;
    /* 并联 继电器 */
    case APP_SYSTEM_STOP_WAY_PARALLEL_RELAY:
        _reason = NETYKC_AS_REASON8D_PARALLEL_RELAY;
        break;
    /* AC 继电器 */
    case APP_SYSTEM_STOP_WAY_AC_RELAY:
        _reason = NETYKC_AS_REASON8C_AC_RELAY;
        break;
    /* 充满 */
    case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
        _reason = NETYKC_CC_REASON41_CHARGE_FULL;
        break;
    /* 拔枪 */
    case APP_SYSTEM_STOP_WAY_PULL_GUN:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON4B_GUIDE_DISCONNECT;
        }else{
            _reason = NETYKC_AS_REASON6B_GUIDANCE_DISCONNECT;
        }
        break;
    /* 电子锁 */
    case APP_SYSTEM_STOP_WAY_ELECTRY_LOCK:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON55_ELECT_LOCK;
        }else{
            _reason = NETYKC_AS_REASON77_ELOCK_ABNORMAL;
        }
        break;
    /* 通讯 */
    case APP_SYSTEM_STOP_WAY_COMMINICATION:
        if(stop_in_starting){
            _reason = NETYKC_SF_REASON5A_RECV_BRM_TIMEOUT;
        }else{
            _reason = NETYKC_AS_REASON84_RECV_BCS_TIMEOUT;
        }
        break;
    /* 辅源 */
    case APP_SYSTEM_STOP_WAY_AUXPOWER:
        _reason = NETYKC_SF_REASON66_AUXPOWER;
        break;
    /* 断电 */
    case APP_SYSTEM_STOP_WAY_POWER_OFF:
        _reason = NETYKC_AS_REASON83_POWER_OFF;
        break;
    /* 存储芯片 */
    case APP_SYSTEM_STOP_WAY_FLASH:
    case APP_SYSTEM_STOP_WAY_EEPROM:
        _reason = NETYKC_AS_REASON8E_STORAGE_CHIP;
        break;
    /* 短路 */
    case APP_SYSTEM_STOP_WAY_SHORTS:
        _reason = NETYKC_AS_REASON6C_CIRCUIT_BREAKER_ACTION;
        break;
    /* 枪电压 */
    case APP_SYSTEM_STOP_WAY_GUNVOLT:
        _reason = NETYKC_SF_REASON60_BHM_STAGE_VOLT_OVERRANGE;
        break;
    /* 绝缘 */
    case APP_SYSTEM_STOP_WAY_INSULT:
        _reason = NETYKC_SF_REASON57_INSULATION_ABNORMAL;
        break;
    /* 电池电压 */
    case APP_SYSTEM_STOP_WAY_BATTERY_VOLT:
        _reason = NETYKC_SF_REASON61_BRO_AA_STAGE_VOLT_OVERRANGE;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = NETYKC_AS_REASON82_CAR_COMMAND_STOP;
        break;
    /* 准备电压 */
    case APP_SYSTEM_STOP_WAY_READY_VOLT:
        _reason = NETYKC_SF_REASON67_READY_VOLTAGE;
        break;
    /* 绝缘电压 */
    case APP_SYSTEM_STOP_WAY_INSULT_VOLT:
        _reason = NETYKC_SF_REASON68_INSULT_VOLTAGE;
        break;
    /* BSM */
    case APP_SYSTEM_STOP_WAY_BSM:
        _reason = NETYKC_AS_REASON8F_BSM_WARNNING;
        break;
    /* APP */
    case APP_SYSTEM_STOP_WAY_APP_STOP:
        _reason = NETYKC_CC_REASON40_APP;
        break;
    /* 刷卡 */
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = NETYKC_CC_REASON45_MANUAL_STOP;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        _reason = NETYKC_AS_REASON6E_NO_BALLANCE;
        break;
    /* 屏幕 */
    case APP_SYSTEM_STOP_WAY_SCREEN_STOP:
        _reason = NETYKC_CC_REASON45_MANUAL_STOP;
        break;
    /* 到达设定电量 */
    case APP_SYSTEM_STOP_WAY_REACH_ELECT:
        _reason = NETYKC_CC_REASON42_TARGET_ELECT;
        break;
    /* 到达设定时间 */
    case APP_SYSTEM_STOP_WAY_REACH_TIME:
        _reason = NETYKC_CC_REASON44_TARGET_TIME;
        break;
    /* 到达设定余额 */
    case APP_SYSTEM_STOP_WAY_REACH_MONEY:
        _reason = NETYKC_CC_REASON43_TARGET_MONEY;
        break;
    /* 充电电流异常 */
    case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
        _reason = NETYKC_AS_REASON76_CURRENT_ABNORMAL;
        break;
    /* 达到SOC 限定值 */
    case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
        _reason = NETYKC_CC_REASON46_RESERVE;
        break;
    default:
        break;
    }
    return _reason;
#endif
    return 0x00;
}

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

    s_sgcc_base = (System_BaseData*)(s_sgcc_handle->get_base_data(gunno));

    if(sgcc_get_socket_info()->state == SGCC_SOCKET_STATE_LOGIN_SUCCESS){
        sgcc_request_message_repeat(gunno);

        if(s_sgcc_base->state.current == APP_OFSM_STATE_CHARGING){
//            if(s_sgcc_realtime_data_count[gunno] > rt_tick_get()){
//                s_sgcc_realtime_data_count[gunno] = rt_tick_get();
//            }
//            if((rt_tick_get() - s_sgcc_realtime_data_count[gunno]) > SGCC_REALTIME_DATA_INTERVAL_CHARGING *1000){
//                s_ykc_realtime_data_count[gunno] = rt_tick_get();
//                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_REALTIME_DATA);
//                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
//                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_BMS_INFO);
//            }
        }else{
            if(s_sgcc_state_noncharging_count[gunno] > rt_tick_get()){
                s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_sgcc_state_noncharging_count[gunno]) > SGCC_STATE_INTERVAL_NONCHARGING_DEF){
                s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
            }
        }
    }else{
        s_sgcc_state_noncharging_count[gunno] = rt_tick_get();
        sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING);
    }

    if((s_sgcc_base->state.current == APP_OFSM_STATE_STARTING) || (s_sgcc_base->state.current == APP_OFSM_STATE_CHARGING)){
        sgcc_clear_message_wait_response_state(gunno, NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD);
    }
}

/*
 * 用于检测只上报一次的报文是否有漏报
 * */
static void sgcc_disposable_message_check(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t _event = 0x00;
    sgcc_get_disposable_event(gunno, 0x00, &_event);
    if(_event){
        if(_event &(1 <<SGCC_DISPOSABLE_EVENT_CONNECT)){
            if(sgcc_get_message_send_state(gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED) == NET_SGCC_SEND_STATE_COMPLETE){
                evs_event_pile_stutus_changes[gunno].connCheckStatus = s_sgcc_disposable_info[gunno].state.connect;
                evs_event_pile_stutus_changes[gunno].yxOccurTime = s_sgcc_disposable_info[gunno].timestamp;

                sgcc_set_clear_disposable_event(gunno, SGCC_DISPOSABLE_EVENT_CONNECT, NET_ENUM_TRUE);
                sgcc_net_event_send(NET_SGCC_EVENT_HANDLE_CHARGEPILE, NET_SGCC_EVENT_TYPE_REQUEST, gunno, NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED);
            }
        }
    }
}

static void sgcc_realtime_process_thread_entry(void *parameter)
{
    uint8_t gunno = 0x00;

    while(1)
    {
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }
        if((s_sgcc_handle == NULL) || (s_sgcc_base == NULL)){
            rt_thread_mdelay(100);
            continue;
        }

        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
//            sgcc_fault_detect_report(gunno);
            sgcc_data_realtime_process(gunno);
            sgcc_disposable_message_check(gunno);
        }

        rt_thread_mdelay(100);
    }
}

int sgcc_realtime_process_init(void)
{
    if(rt_thread_init(&s_sgcc_realtime_process_thread, "sgcc_rl_pro", sgcc_realtime_process_thread_entry, NULL,
            s_sgcc_realtime_process_thread_stack, SGCC_REALTIME_PROCESS_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("sgcc realtime process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_realtime_process_thread) != RT_EOK){
        LOG_E("sgcc realtime process thread startup fail, please check");
        return -0x01;
    }

    s_sgcc_handle = net_get_net_handle();

    return 0x00;
}

#endif /* NET_PACK_USING_SGCC */
