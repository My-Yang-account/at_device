/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_MESSAGE_RECEIVE_H_
#define NET_PACK_NET_YKC_INC_YKC_MESSAGE_RECEIVE_H_

#include "ykc_message_struct_define.h"

#ifdef NET_PACK_USING_YKC

#define NET_YKC_CARD_VIN_WHITELIST_MAX                               (17 *25)  /* 卡、VIN码白名单最大数量 */
#define NET_YKC_QRCODE_BUF_MAX                                       256       /* 二维码缓存长度 */

#define NET_YKC_WHITELIST_TYPE_CARD                          0x00      /* 白名单类型：卡白名单 */
#define NET_YKC_WHITELIST_TYPE_VIN                           0x01      /* 白名单类型：VIN码白名单 */

#define NET_YKC_CARD_WHITELIST_QUERY                         (1 <<0)
#define NET_YKC_CARD_WHITELIST_DELETE                        (1 <<1)

/** server response event */
#define NET_YKC_SRES_EVENT_LOGIN                               0    /* 服务器响应事件：登录响应 */
#define NET_YKC_SRES_EVENT_HEARTBEAT                           1    /* 服务器响应事件：心跳响应 */
#define NET_YKC_SRES_EVENT_BILLING_MODEL_VERIFY                2    /* 服务器响应事件：计费模型验证请求响应 */
#define NET_YKC_SRES_EVENT_BILLING_MODEL_REQUEST               3    /* 服务器响应事件：计费模型请求响应 */
#define NET_YKC_SRES_EVENT_BILL_VERIFY                         4    /* 服务器响应事件：订单确认 */
#define NET_YKC_SRES_EVENT_APPLY_CHARGE_ACTIVE                 5    /* 服务器响应事件：充电桩主动申请启动充电响应 */
#define NET_YKC_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE           6    /* 服务器响应事件： 停止完成上报订单响应 */

#define NET_YKC_SERVER_SRES_NUM                                7    /* 服务器响应事件总数 */

/** server request event */
#define NET_YKC_SREQ_EVENT_QUERY_REALTIME_DATA                 0    /* 服务器请求事件：读取实时数据*/
#define NET_YKC_SREQ_EVENT_REMOTE_START_CHARGE                 1    /* 服务器请求事件：运营平台远程控制启机*/
#define NET_YKC_SREQ_EVENT_REMOTE_STOP_CHARGE                  2    /* 服务器请求事件：运营平台远程停机*/
#define NET_YKC_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE             3    /* 服务器请求事件：远程账户余额更新*/
#define NET_YKC_SREQ_EVENT_SYNC_OFFLINE_CARD                   4    /* 服务器请求事件：离线卡数据同步*/
#define NET_YKC_SREQ_EVENT_CLEAR_OFFLINE_CARD                  5    /* 服务器请求事件：离线卡数据清除*/
#define NET_YKC_SREQ_EVENT_QUERY_OFFLINE_CARD                  6    /* 服务器请求事件：离线卡数据查询*/
#define NET_YKC_SREQ_EVENT_SET_WORK_PARA                       7    /* 服务器请求事件：充电桩工作参数设置*/
#define NET_YKC_SREQ_EVENT_TIME_SYNC                           8    /* 服务器请求事件：对时设置*/
#define NET_YKC_SREQ_EVENT_BILLING_MODEL_SET                   9    /* 服务器请求事件：计费模型设置*/
#define NET_YKC_SREQ_EVENT_GROUND_LOCK_LIFTING                 10   /* 服务器请求事件：遥控地锁升锁与降锁命令*/
#define NET_YKC_SREQ_EVENT_REMOTE_REBOOT                       11   /* 服务器请求事件：远程重启*/
#define NET_YKC_SREQ_EVENT_REMOTE_UPDATE                       12   /* 服务器请求事件：远程更新*/
#define NET_YKC_SREQ_EVENT_REMOTE_START_MERGE_CHARGE           13   /* 服务器请求事件：运营平台远程控制并充启机*/
#define NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC                    14   /* 服务器请求事件：二维码配置(国充)*/
#define NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15                 15   /* 服务器请求事件：二维码配置(云快充1.5)*/

#define NET_YKC_SERVER_SREQ_NUM                                16   /* 服务器请求事件总数 */

/**=======================================[服务器请求报文]=======================================*/
/**=======================================[服务器请求报文]=======================================*/
/** 读取实时数据 */
extern Net_YkcPro_SReq_Query_RealTimeData_t g_ykc_sreq_query_realtime_data[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程控制启机 */
extern Net_YkcPro_SReq_Remote_StartCharge_t g_ykc_sreq_remote_start_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 运营平台远程停机 */
extern Net_YkcPro_SReq_Remote_StopCharge_t g_ykc_sreq_remote_stop_charge[NET_SYSTEM_GUN_NUMBER];   // OK
/** 远程账户余额更新 */
extern Net_YkcPro_SReq_AccountBallance_Update_t g_ykc_sreq_account_ballance_update[NET_SYSTEM_GUN_NUMBER];   // OK
/** 离线卡数据同步 */
extern Net_YkcPro_SReq_Sync_OfflineCard_t g_ykc_sreq_sync_offline_card;   // OK
/** 离线卡数据清除 */
extern Net_YkcPro_SReq_Clear_OfflineCard_t g_ykc_sreq_clear_offline_card;   // OK
/** 离线卡数据查询 */
extern Net_YkcPro_SReq_Query_OfflineCard_t g_ykc_sreq_query_offline_card;   // OK
/** 充电桩工作参数设置 */
extern Net_YkcPro_SReq_Set_WorkPara_t g_ykc_sreq_set_work_para;   // OK
/** 对时设置 */
extern Net_YkcPro_SReq_TimeSync_t g_ykc_sreq_time_sync;   // OK
/** 计费模型设置 */
extern Net_YkcPro_SRep_BillingModel_Set_t g_ykc_sreq_billing_model_set;   // OK
/** 遥控地锁升锁与降锁命令 */
extern Net_YkcPro_SReq_GroundLock_Lifting_t g_ykc_sreq_ground_lock_lifting[NET_SYSTEM_GUN_NUMBER];
/** 远程重启 */
extern Net_YkcPro_SReq_RemoteReboot_t g_ykc_sreq_remote_reboot;
/** 远程更新 */
extern Net_YkcPro_SReq_RemoteUpdate_t g_ykc_sreq_remote_update;
/** 运营平台远程控制并充启机 */
extern Net_YkcPro_SReq_Remote_StartMergeCharge_t g_ykc_sreq_remote_start_merge_charge[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(国充) */
extern Net_YkcPro_SReq_Qrcode_Config_GC_t g_ykc_sreq_qrcode_config_gc[NET_SYSTEM_GUN_NUMBER];
/** 运营平台下发二维码配置(云快充1.5) */
extern Net_YkcPro_SReq_Qrcode_Config_Ykc15_t g_ykc_sreq_qrcode_config_ykc15;

/**=======================================[服务器响应报文]=======================================*/
/**=======================================[服务器响应报文]=======================================*/
/** 登录签到响应 */
extern Net_YkcPro_SRes_LogIn_t g_ykc_sres_login;   // OK
///** 心跳响应 */
//extern Net_YkcPro_SRes_HeartBeat_t g_ykc_sres_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证请求响应 */
extern Net_YkcPro_SRes_BillingModel_Verify_t g_ykc_sres_billing_model_verify;   // OK
/** 计费模型请求响应 */
extern Net_YkcPro_SRes_BillingModel_Request_t g_ykc_sres_billing_model_request;   // OK
/** 交易记录响应 */
extern Net_YkcPro_SRes_TransactionRecords_t g_ykc_sres_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 充电桩主动申请启动充电响应 */
extern Net_YkcPro_SRes_ApplyCharge_Active_t g_ykc_sres_apply_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电桩主动申请并充充电响应 */
extern Net_YkcPro_SRes_ApplyMergeCharge_Active_t g_ykc_sres_apply_merge_charge_active[NET_SYSTEM_GUN_NUMBER];   // OK

#pragma pack(1)

typedef struct{
    struct{
        uint8_t is_used : 1;
    }flag;
    uint8_t set_type;
    uint8_t general_type;
    uint16_t length;
    uint8_t qrcode[NET_YKC_QRCODE_BUF_MAX];
}ykc_qrcode_buf_t;

typedef struct{
    struct{
        uint8_t is_used : 1;
    }flag;
    uint8_t type;
    uint16_t length;
    uint8_t whitlelist[NET_YKC_CARD_VIN_WHITELIST_MAX];
}ykc_card_vin_buf_t;

#pragma pack()

void* ykc_get_qrcode_info(void);
void* ykc_get_card_vin_whitelists_info(void);

int32_t ykc_message_recv_init(void);

#endif /* NET_PACK_USING_YKC */

#endif /* NET_PACK_NET_YKC_INC_YKC_MESSAGE_RECEIVE_H_ */
