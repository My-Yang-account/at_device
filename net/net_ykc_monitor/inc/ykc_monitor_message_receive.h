/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#ifndef NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_RECEIVE_H_
#define NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_RECEIVE_H_

#include "ykc_monitor_message_struct_define.h"

#ifdef NET_PACK_USING_YKC_MONITOR

#define NET_YKC_MONITOR_CARD_VIN_WHITELIST_MAX                       (17 *25)  /* 卡、VIN码白名单最大数量 */
#define NET_YKC_MONITOR_QRCODE_BUF_MAX                               256       /* 二维码缓存长度 */
#define NET_YKC_MONITOR_DEV_INFO_BUF_MAX                             32        /* 设备西信息缓存长度 */

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
#define NET_YKC_MONITOR_CONFIG_INFO_BUF_MAX                          600       /* 设备配置信息缓存长度 */
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#define NET_YKC_MONITOR_WHITELIST_TYPE_CARD                          0x00      /* 白名单类型：卡白名单 */
#define NET_YKC_MONITOR_WHITELIST_TYPE_VIN                           0x01      /* 白名单类型：VIN码白名单 */

#define NET_YKC_MONITOR_CARD_WHITELIST_QUERY                         (1 <<0)
#define NET_YKC_MONITOR_CARD_WHITELIST_DELETE                        (1 <<1)

/** server response event */
#define NET_YKC_MONITOR_SRES_EVENT_LOGIN                               0    /* 服务器响应事件：登录响应 */
#define NET_YKC_MONITOR_SRES_EVENT_HEARTBEAT                           1    /* 服务器响应事件：心跳响应 */
#define NET_YKC_MONITOR_SRES_EVENT_BILLING_MODEL_VERIFY                2    /* 服务器响应事件：计费模型验证请求响应 */
#define NET_YKC_MONITOR_SRES_EVENT_BILLING_MODEL_REQUEST               3    /* 服务器响应事件：计费模型请求响应 */
#define NET_YKC_MONITOR_SRES_EVENT_BILL_VERIFY                         4    /* 服务器响应事件：订单确认 */
#define NET_YKC_MONITOR_SRES_EVENT_APPLY_CHARGE_ACTIVE                 5    /* 服务器响应事件：充电桩主动申请启动充电响应 */
#define NET_YKC_MONITOR_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE           6    /* 服务器响应事件： 停止完成上报订单响应 */

#define NET_YKC_SERVER_SRES_NUM                                        7    /* 服务器响应事件总数 */

/** server request event */
#define NET_YKC_MONITOR_SREQ_EVENT_QUERY_REALTIME_DATA                 0    /* 服务器请求事件：读取实时数据*/
#define NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_CHARGE                 1    /* 服务器请求事件：运营平台远程控制启机*/
#define NET_YKC_MONITOR_SREQ_EVENT_REMOTE_STOP_CHARGE                  2    /* 服务器请求事件：运营平台远程停机*/
#define NET_YKC_MONITOR_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE             3    /* 服务器请求事件：远程账户余额更新*/
#define NET_YKC_MONITOR_SREQ_EVENT_SYNC_OFFLINE_CARD                   4    /* 服务器请求事件：离线卡数据同步*/
#define NET_YKC_MONITOR_SREQ_EVENT_CLEAR_OFFLINE_CARD                  5    /* 服务器请求事件：离线卡数据清除*/
#define NET_YKC_MONITOR_SREQ_EVENT_QUERY_OFFLINE_CARD                  6    /* 服务器请求事件：离线卡数据查询*/
#define NET_YKC_MONITOR_SREQ_EVENT_SET_WORK_PARA                       7    /* 服务器请求事件：充电桩工作参数设置*/
#define NET_YKC_MONITOR_SREQ_EVENT_TIME_SYNC                           8    /* 服务器请求事件：对时设置*/
#define NET_YKC_MONITOR_SREQ_EVENT_BILLING_MODEL_SET                   9    /* 服务器请求事件：计费模型设置*/
#define NET_YKC_MONITOR_SREQ_EVENT_GROUND_LOCK_LIFTING                 10   /* 服务器请求事件：遥控地锁升锁与降锁命令*/
#define NET_YKC_MONITOR_SREQ_EVENT_REMOTE_REBOOT                       11   /* 服务器请求事件：远程重启*/
#define NET_YKC_MONITOR_SREQ_EVENT_REMOTE_UPDATE                       12   /* 服务器请求事件：远程更新*/
#define NET_YKC_MONITOR_SREQ_EVENT_REMOTE_START_MERGE_CHARGE           13   /* 服务器请求事件：运营平台远程控制并充启机*/
#define NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_GC                    14   /* 服务器请求事件：二维码配置(国充)*/
#define NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_YKC15                 15   /* 服务器请求事件：二维码配置(云快充1.5)*/
#define NET_YKC_MONITOR_SREQ_EVENT_QRCODE_CONFIG_TLD                   16   /* 服务器请求事件：二维码配置(特来电)*/

#define NET_YKC_MONITOR_SERVER_SREQ_NUM                                17   /* 服务器请求事件总数 */

/******************************** 以下是监控报文事件 *******************************/
/******************************** 以下是监控报文事件 *******************************/
#ifdef NET_YKC_MONITOR_AS_MONITOR
/** server user request event */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_MODULE_INFO            0    /* 服务器监控请求事件：查询模块信息 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_MODULE_SETUPINFO       1    /* 服务器监控请求事件：查询模块配置信息 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_FAULT_RECORD           2    /* 服务器监控请求事件：查询故障记录 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_CHARGE_RECORD          3    /* 服务器监控请求事件：查询充电记录 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_ININFO_SETUP           4    /* 服务器监控请求事件：查询输入信息配置 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_PROTECTINFO_SETUP      5    /* 服务器监控请求事件：询保护信息配置  */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_FUNCTION_SETUP         6    /* 服务器监控请求事件：查询功能配置 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_TSOCKET_INFO           7    /* 服务器监控请求事件：查询目标socket */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_DEV_INFO               8    /* 服务器监控请求事件：查询设备信息 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_FUNCTION_SWITCH              9    /* 服务器监控请求事件：功能开关控制 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_INFOPARA_MODIFY              10   /* 服务器监控请求事件：修改桩信息、参数 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_MODIFY_DEV_INFO              11   /* 服务器监控请求事件：修改设备信息 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_BILLING_RULE           12   /* 服务器监控请求事件：查询计费信息 */
#define NET_YKC_MONITOR_USER_SREQ_EVENT_QUERY_SET_CONFIG_INFO        13   /* 服务器监控请求事件：查询、设置配置信息 */

/** server user response event */
#define NET_YKC_MONITOR_USER_SRES_EVENT_REPORT_TPLAT_LOG             0    /* 服务器监控响应事件：上报目标平台日志 */
#define NET_YKC_MONITOR_USER_SRES_EVENT_REPORT_DEV_INFO              1    /* 服务器监控响应事件：上报设备信息 */
#define NET_YKC_MONITOR_USER_SRES_EVENT_INFOPARA_CONFIRM_RES         2    /* 服务器监控响应事件：修改的桩信息、参数确认结果 */
#define NET_YKC_MONITOR_USER_SRES_EVENT_MFAULT_RES                   3    /* 服务器监控响应事件：上报模块故障信息响应 */
#define NET_YKC_MONITOR_USER_SRES_EVENT_LFAULT_RES                   4    /* 服务器监控响应事件：上报液冷故障信息响应 */

#endif /* NET_YKC_MONITOR_AS_MONITOR */

/**=======================================[服务器请求报文]=======================================*/
/**=======================================[服务器请求报文]=======================================*/
/** 读取实时数据 */
extern Net_YkcMonitorPro_SReq_Query_RealTimeData_t g_ykc_monitor_sreq_query_realtime_data[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程控制启机 */
extern Net_YkcMonitorPro_SReq_Remote_StartCharge_t g_ykc_monitor_sreq_remote_start_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程停机 */
extern Net_YkcMonitorPro_SReq_Remote_StopCharge_t g_ykc_monitor_sreq_remote_stop_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 远程账户余额更新 */
extern Net_YkcMonitorPro_SReq_AccountBallance_Update_t g_ykc_monitor_sreq_account_ballance_update[NET_SYSTEM_GUN_NUMBER];   // OK
/** 离线卡数据同步 */
extern Net_YkcMonitorPro_SReq_Sync_OfflineCard_t g_ykc_monitor_sreq_sync_offline_card;   // OK
/** 离线卡数据清除 */
extern Net_YkcMonitorPro_SReq_Clear_OfflineCard_t g_ykc_monitor_sreq_clear_offline_card;   // OK
/** 离线卡数据查询 */
extern Net_YkcMonitorPro_SReq_Query_OfflineCard_t g_ykc_monitor_sreq_query_offline_card;   // OK
/** 充电桩工作参数设置 */
extern Net_YkcMonitorPro_SReq_Set_WorkPara_t g_ykc_monitor_sreq_set_work_para;   // OK
/** 对时设置 */
extern Net_YkcMonitorPro_SReq_TimeSync_t g_ykc_monitor_sreq_time_sync;   // OK
/** 计费模型设置 */
extern Net_YkcMonitorPro_SRep_BillingModel_Set_t g_ykc_monitor_sreq_billing_model_set;   // OK
/** 遥控地锁升锁与降锁命令 */
extern Net_YkcMonitorPro_SReq_GroundLock_Lifting_t g_ykc_monitor_sreq_ground_lock_lifting[NET_SYSTEM_GUN_NUMBER];
/** 远程重启 */
extern Net_YkcMonitorPro_SReq_RemoteReboot_t g_ykc_monitor_sreq_remote_reboot;
/** 远程更新 */
extern Net_YkcMonitorPro_SReq_RemoteUpdate_t g_ykc_monitor_sreq_remote_update;
/** 运营平台远程控制并充启机 */
extern Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t g_ykc_monitor_sreq_remote_start_merge_charge[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(国充) */
extern Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t g_ykc_monitor_sreq_qrcode_config_gc[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(云快充1.5) */
extern Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t g_ykc_monitor_sreq_qrcode_config_ykc15;
/** 运营平台下发二维码配置(特来电) */
extern Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t g_ykc_monitor_sreq_qrcode_config_tld[NET_SYSTEM_GUN_NUMBER];
/**=======================================[服务器响应报文]=======================================*/
/**=======================================[服务器响应报文]=======================================*/
/** 登录签到响应 */
extern Net_YkcMonitorPro_SRes_LogIn_t g_ykc_monitor_sres_login;   // OK
///** 心跳响应 */
//extern Net_YkcMonitorPro_SRes_HeartBeat_t g_ykc_monitor_sres_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证请求响应 */
extern Net_YkcMonitorPro_SRes_BillingModel_Verify_t g_ykc_monitor_sres_billing_model_verify;   // OK
/** 计费模型请求响应 */
extern Net_YkcMonitorPro_SRes_BillingModel_Request_t g_ykc_monitor_sres_billing_model_request;   // OK
/** 交易记录响应 */
extern Net_YkcMonitorPro_SRes_TransactionRecords_t g_ykc_monitor_sres_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 充电桩主动申请启动充电响应 */
extern Net_YkcMonitorPro_SRes_ApplyCharge_Active_t g_ykc_monitor_sres_apply_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电桩主动申请并充充电响应 */
extern Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t g_ykc_monitor_sres_apply_merge_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK

#ifdef NET_YKC_MONITOR_AS_MONITOR
/** 充电桩目标平台日志上报响应 */
extern Net_YkcMonitorPro_Sres_TargetPlat_Log_t g_ykc_monitor_sres_target_plat_log;
/** 充电桩设备信息上报响应 */
extern Net_YkcMonitorPro_Sres_DevInfo_t g_ykc_monitor_sres_dev_info;
/** 服务器下发功能开关控制请求 */
extern Net_YkcMonitorPro_Sreq_FunctionSwitch_t g_ykc_monitor_sreq_function_switch;
/** IP、端口、桩号修改(信息确认)请求 */
extern Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t g_ykc_monitor_sreq_pres_info_para_modify_confirm;
/** IP、服务器IP、端口、桩号信息确认结果响应 */
extern Net_YkcMonitorPro_Sres_InfoPara_ConfirmResult_t g_ykc_monitor_sres_info_para_confirm_result;
/** 服务器下发修改设备信息请求 */
extern Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t g_ykc_monitor_sreq_modify_device_info;
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#pragma pack(1)

typedef struct{
    struct{
        uint8_t is_used : 1;
    }flag;
    uint8_t set_type;
    uint8_t general_type;
    uint16_t length;
    uint8_t qrcode[NET_YKC_MONITOR_QRCODE_BUF_MAX];
}ykc_monitor_qrcode_buf_t;

typedef struct{
    struct{
        uint8_t is_used : 1;
    }flag;
    uint8_t type;
    uint16_t length;
    uint8_t whitlelist[NET_YKC_MONITOR_CARD_VIN_WHITELIST_MAX];
}ykc_monitor_card_vin_buf_t;

typedef struct{
    struct{
        uint8_t is_used : 1;
    }flag;
    uint8_t info_type;
    uint8_t length;
    uint8_t info[NET_YKC_MONITOR_DEV_INFO_BUF_MAX];
}ykc_monitor_dev_info_buf_t;

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
typedef struct{
    struct{
        uint8_t is_used : 1;
    }flag;
    uint16_t length;
    uint8_t info[NET_YKC_MONITOR_CONFIG_INFO_BUF_MAX];
}ykc_monitor_config_info_buf_t;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#pragma pack()

#ifdef NET_YKC_MONITOR_AS_MONITOR
void ykc_monitor_clear_recv_message_item(uint8_t cmd, uint8_t gunno);
uint16_t ykc_monitor_get_recv_message_item_serial_number(uint8_t cmd, uint8_t gunno);
#endif /* NET_YKC_MONITOR_AS_MONITOR */

void* ykc_monitor_get_qrcode_info(void);
void* ykc_monitor_get_card_vin_whitelists_info(void);
void* ykc_monitor_get_device_info(void);

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
void* ykc_monitor_get_config_info(void);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

int32_t ykc_message_recv_init(void);

#endif /* NET_PACK_USING_YKC_MONITOR */

#endif /* NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_RECEIVE_H_ */
