/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_INC_YCP_MESSAGE_RECEIVE_H_
#define NET_NET_YCP_INC_YCP_MESSAGE_RECEIVE_H_

#include "ycp_message_struct_define.h"

#ifdef NET_PACK_USING_YCP

/** server response event */
#define NET_YCP_SRES_EVENT_LOGIN                               0    /* 服务器响应事件：登录响应 */
#define NET_YCP_SRES_EVENT_TIME_SYNC                           1    /* 服务器请求事件：对时响应*/
#define NET_YCP_SRES_EVENT_HEARTBEAT                           2    /* 服务器响应事件：心跳响应 */
#define NET_YCP_SRES_EVENT_BILLING_MODEL_VERIFY                3    /* 服务器响应事件：计费模型验证请求响应 */
#define NET_YCP_SRES_EVENT_APPLY_CHARGE_ACTIVE                 4    /* 服务器响应事件：服务器回复充电桩主动请求充电请求 */
#define NET_YCP_SRES_EVENT_BILL_VERIFY                         5    /* 服务器响应事件：充电结束报告应答 */

#define NET_YCP_SERVER_SRES_NUM                                6    /* 服务器响应事件总数 */

/** server request event */
#define NET_YCP_SREQ_EVENT_SET_PARA                            0    /* 服务器请求事件：充电桩参数设置*/
#define NET_YCP_SREQ_EVENT_QRCODE_CONFIG                       1    /* 服务器请求事件：二维码配置*/
#define NET_YCP_SREQ_EVENT_SET_SERVICE_PHONE                   2    /* 服务器请求事件：设置客服电话*/
#define NET_YCP_SREQ_EVENT_BILLING_MODEL_SET                   3    /* 服务器请求事件：计费模型设置*/
#define NET_YCP_SREQ_EVENT_QUERY_DEVICE_STATE                  4    /* 服务器请求事件：查询单个枪状态*/
#define NET_YCP_SREQ_EVENT_QUERY_DEVICE_STATE_ALL              5    /* 服务器请求事件：查询所有枪状态*/
#define NET_YCP_SREQ_EVENT_REMOTE_START_CHARGE                 6    /* 服务器请求事件：远程开启充电*/
#define NET_YCP_SREQ_EVENT_REMOTE_STOP_CHARGE                  7    /* 服务器请求事件：远程结束充电*/
#define NET_YCP_SREQ_EVENT_REMOTE_UPDATE                       8    /* 服务器请求事件：远程升级*/
#define NET_YCP_SREQ_EVENT_REMOTE_REBOOT                       9    /* 服务器请求事件：远程重启*/
#define NET_YCP_SREQ_EVENT_MODIFY_SERVICE_ADDR                 10   /* 服务器请求事件：远程修改联网地址*/
#define NET_YCP_SREQ_EVENT_QUERY_DEVICE_FAULT                  11   /* 服务器请求事件：获取当前设备故障信息*/

#define NET_YCP_SERVER_SREQ_NUM                                12   /* 服务器请求事件总数 */

/**=======================================[服务器请求报文]=======================================*/
/**=======================================[服务器请求报文]=======================================*/
/** 充电桩参数设置 */
extern Net_YcpPro_SReq_ParaSet_t g_ycp_sreq_set_para;   // OK
/** 运营平台下发二维码配置 */
extern Net_YcpPro_SReq_Qrcode_Config_t g_ycp_sreq_qrcode_config[NET_SYSTEM_GUN_NUMBER];
/** 客服电话设置 */
extern Net_YcpPro_SReq_ServicePhone_t g_ycp_sreq_set_service_phone;
/** 计费模型下发 */
extern Net_YcpPro_SReq_BillingModel_Set_t g_ycp_sreq_billing_model_set;
/** 查询单个枪状态 */
extern Net_YcpPro_SReq_Query_PileState_t g_ycp_sreq_query_device_state[NET_SYSTEM_GUN_NUMBER];
/** 查询所有枪状态 */
extern Net_YcpPro_SReq_Query_PileState_All_t g_ycp_sreq_query_device_state_all;
/** 远程开启充电 */
extern Net_YcpPro_SReq_Remote_StartCharge_t g_ycp_sreq_remote_start_charge[NET_SYSTEM_GUN_NUMBER];
/** 远程结束充电 */
extern Net_YcpPro_SReq_Remote_StopCharge_t g_ycp_sreq_remote_stop_charge[NET_SYSTEM_GUN_NUMBER];
/** 远程升级 */
extern Net_YcpPro_SReq_RemoteUpdate_t g_ycp_sreq_remote_update;
/** 远程重启 */
extern Net_YcpPro_SReq_RemoteReboot_t g_ycp_sreq_remote_reboot;
/** 远程修改联网地址 */
extern Net_YcpPro_SReq_Modify_ServerAddr_t g_ycp_sreq_modify_server_addr;
/** 获取当前设备故障信息 */
extern Net_YcpPro_SReq_Query_DeviceFault_t g_ycp_sreq_query_device_fault;

/**=======================================[服务器响应报文]=======================================*/
/**=======================================[服务器响应报文]=======================================*/
/** 登录签到响应 */
extern Net_YcpPro_SRes_LogIn_t g_ycp_sres_login;   // OK
/** 对时设置 */
extern Net_YcpPro_SRes_TimeSync_t g_ycp_sres_time_sync;   // OK
///** 心跳响应 */
//extern Net_YcpPro_SRes_HeartBeat_t g_ycp_sres_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证请求响应 */
extern Net_YcpPro_SRes_BillingModel_Verify_t g_ycp_sres_billing_model_verify;   // OK
/** 服务器回复充电桩主动请求充电请求 */
extern Net_YcpPro_SRes_ApplyCharge_Active_t g_ycp_sres_apply_charge_active[NET_SYSTEM_GUN_NUMBER];
/** 充电结束报告应答 */
extern Net_YcpPro_SRes_TransactionRecords_t g_ycp_sres_transaction_records[NET_SYSTEM_GUN_NUMBER];

int32_t ycp_message_recv_init(void);

#endif /* NET_PACK_USING_YCP */

#endif /* NET_NET_YCP_INC_YCP_MESSAGE_RECEIVE_H_ */
