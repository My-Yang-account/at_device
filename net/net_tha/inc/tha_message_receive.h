/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_MESSAGE_RECEIVE_H_
#define NET_PACK_NET_THA_INC_THA_MESSAGE_RECEIVE_H_

#include "tha_message_struct_define.h"

#ifdef NET_PACK_USING_THA

/** server response event */
#define NET_THA_SRES_EVENT_LOGIN                               0    /* 服务器响应事件：登录响应 */
#define NET_THA_SRES_EVENT_AUTHORITY                           1    /* 服务器响应事件：权限认证响应 */
#define NET_THA_SRES_EVENT_REPORT_CHARGE_DATA                  3    /* 服务器响应事件：上报充电数据响应 */
#define NET_THA_SRES_EVENT_HISTORY_ORDER                       4    /* 服务器响应事件：上报历史订单响应 */
#define NET_THA_SRES_EVENT_HEARTBEAT                           5    /* 服务器响应事件：上报心跳响应 */
#define NET_THA_SRES_EVENT_STOP_CHARGE_COMPLETE                6    /* 服务器响应事件： 停止完成上报订单响应 */
#define NET_THA_SRES_EVENT_REPORT_BATTERY_STATE                7    /* 服务器响应事件：上报电池状态应 */
#define NET_THA_SRES_EVENT_EVENT_STATE                         8    /* 服务器响应事件：上报事件状态响应 */
#define NET_THA_SRES_EVENT_ERROR_INFO                          9    /* 服务器响应事件：上报错误信息响应 */
#define NET_THA_SRES_EVENT_UPDATE_RESULT                       10   /* 服务器响应事件：上报升级结果响应 */
#define NET_THA_SRES_EVENT_QUERY_PILENUMBER_OF_ICCID           11   /* 服务器响应事件：查询ICCID对应的桩编号响应 */
#define NET_THA_SRES_EVENT_REPORT_PROCESS_PARA                 12   /* 服务器响应事件：上报处理用参数响应 */
#define NET_THA_SRES_EVENT_VIN_AUTHORIZE                       13   /* 服务器响应事件：vin码鉴权响应 */
#define NET_THA_SRES_EVENT_REPORT_NET_INFO                     14   /* 服务器响应事件：上报网络信息响应 */
#define NET_THA_SRES_EVENT_REPORT_BATTERY_INFO                 15   /* 服务器响应事件：上报电池信息响应 */
#define NET_THA_SRES_EVENT_REPORT_DEVICE_FAULT_INFO            16   /* 服务器响应事件：上报设备故障信息响应 */
#define NET_THA_SRES_EVENT_REPORT_CAR_WARNNING_INFO            17   /* 服务器响应事件：上报车辆告警信息响应 */
#define NET_THA_SRES_EVENT_REPORT_REAL_TEMPERATION             18   /* 服务器响应事件：上报充电中充电枪的实时温度响应 */
#define NET_THA_SRES_EVENT_QUERY_AVAILABLE_UPDATE_PACK_VER     19   /* 设备查询最新可用升级包软件版本号响应 */
#define NET_THA_SRES_EVENT_DEVICE_REQUEST_UPDATE               20   /* 设备发起远程升级请求响应 */
#define NET_THA_SRES_EVENT_QUERY_PRIVATE_RESERVATION           21   /* 私桩(桩端)查询定时充电预约响应 */
/** server request event */
#define NET_THA_SREQ_EVENT_TIME_SYNC                           0    /* 服务器请求事件：时间同步请求*/
#define NET_THA_SREQ_EVENT_SYSTEM_REBOOT                       1    /* 服务器请求事件：系统复位请求*/
#define NET_THA_SREQ_EVENT_QUERY_CHARGE_DATA                   2    /* 服务器请求事件：读取充电数据请求*/
#define NET_THA_SREQ_EVENT_QUERY_SYSTEM_INFO                   3    /* 服务器请求事件：查询系统信息请求*/
#define NET_THA_SREQ_EVENT_UPDATE_SYSTEM_INFO                  4    /* 服务器请求事件：更新系统信息请求*/
#define NET_THA_SREQ_EVENT_QUERY_BILLING_RULE                  5    /* 服务器请求事件：查询计费规则请求*/
#define NET_THA_SREQ_EVENT_UPDATE_BILLING_RULE                 6    /* 服务器请求事件：更新计费规则请求*/
#define NET_THA_SREQ_EVENT_QUERY_SYSTEM_SET_INFO               7    /* 服务器请求事件：查询系统设置信息请求*/
#define NET_THA_SREQ_EVENT_UPDATE_SYSTEM_SET_INFO              8    /* 服务器请求事件：更新系统设置信息请求*/
#define NET_THA_SREQ_EVENT_QUERY_DEVICE_FAULT_INFO             9    /* 服务器请求事件：查询设备故障信息请求*/
#define NET_THA_SREQ_EVENT_QUERY_BATTERY_CHARGE_INFO           10   /* 服务器请求事件：查询电池充电信息请求*/
#define NET_THA_SREQ_EVENT_QUERY_ORDER_STATE                   11   /* 服务器请求事件：查询订单状态请求*/
#define NET_THA_SREQ_EVENT_SET_RESERVATION                     12   /* 服务器请求事件：设置定时充电请求*/
#define NET_THA_SREQ_EVENT_CANCEL_RESERVATION                  13   /* 服务器请求事件：取消定时充电请求*/
#define NET_THA_SREQ_EVENT_STOP_CHARGE                         14   /* 服务器请求事件：停止充电请求*/
#define NET_THA_SREQ_EVENT_START_UPDATE                        15   /* 服务器请求事件：开始升级请求*/
#define NET_THA_SREQ_EVENT_SET_RUNNING_PARA                    16   /* 服务器请求事件：设置桩运行参数请求*/
#define NET_THA_SREQ_EVENT_SET_CHARGE_POWER                    17   /* 服务器请求事件：设置充电功率请求*/
#define NET_THA_SREQ_EVENT_UNLOCK_CHARGE                       18   /* 服务器请求事件：解锁充电请求*/
#define NET_THA_SREQ_EVENT_ISSUED_PROCESS_PARA                 19   /* 服务器请求事件：下发处理用参数请求*/
#define NET_THA_SREQ_EVENT_SET_PRIVATE_RESERVATION             20   /* 服务器请求事件：私桩预约充电求*/
#define NET_THA_SREQ_EVENT_CANCEL_PRIVATE_RESERVATION          21   /* 服务器请求事件：取消私桩预约充电请求*/
#define NET_THA_SREQ_EVENT_ISSUED_PWM                          22   /* 服务器请求事件：私桩PWM电流值下发请求*/


/** 登录签到响应 */
extern Net_ThaPro_SRes_LogIn_t g_tha_sres_login[NET_SYSTEM_GUN_NUMBER];
/** 权限认证响应 */
extern Net_ThaPro_SRes_Authority_t g_tha_sres_authority[NET_SYSTEM_GUN_NUMBER];
/** 请求下载升级包响应 */
extern Net_ThaPro_SRes_UpdatePack_Load_t g_tha_sres_update_pack_load;   /* 请求下载升级包同时最多一把枪 */
/** 查询ICCID对应的桩编号响应 */
extern Net_ThaPro_SRes_Query_PileNumberOfIccid_t g_tha_sres_query_pile_number_of_iccid;
/** VIN码鉴权响应 */
extern Net_ThaPro_SRes_VinAuthorize_t g_tha_sres_vin_authorize[NET_SYSTEM_GUN_NUMBER];
/** 时间同步 */
extern Net_ThaPro_SReqPRes_TimeSync_t g_tha_sreq_time_sync;
/** 系统复位 */
extern Net_ThaPro_SReqPRes_SystemReboot_t g_tha_sreq_system_reboot;
/** 用户绑定 */
extern Net_ThaPro_SReq_User_Binding_t g_tha_sreq_user_binding;
/** 更新系统信息 */
extern Net_ThaPro_SReq_Updated_SystemInfo_t g_tha_sreq_updated_system_info;
/** 更新计费规则 */
extern Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t g_tha_sreq_updated_billing_rule[NET_SYSTEM_GUN_NUMBER];
/** 查询订单状态 */
extern Net_ThaPro_SReq_Query_OrderState_t g_tha_sreq_query_order_state;
/** 停止充电 */
extern Net_ThaPro_SReq_StopCharge_t g_tha_sreq_stop_charge[NET_SYSTEM_GUN_NUMBER];
/** 启动远程升级 */
extern Net_ThaPro_SReq_StartUpdate_t g_tha_sreq_start_update[NET_SYSTEM_GUN_NUMBER];
/** 设置桩运行参数 */
extern Net_ThaPro_SReq_Set_Pile_RunningPara_t g_tha_sreq_set_pile_running_para[NET_SYSTEM_GUN_NUMBER];
/** 设置充电枪充电功率 */
extern Net_ThaPro_SReq_Set_Pile_ChargePower_t g_tha_sreq_set_pile_charge_power[NET_SYSTEM_GUN_NUMBER];
/** 解锁充电 */
extern Net_ThaPro_SReq_UnlockCharge_t g_tha_sreq_unlock_charge[NET_SYSTEM_GUN_NUMBER];
/** 平台下发处理用参数 */
extern Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t g_tha_sreq_issued_process_para;
/** 设备查询最新可用升级包软件版本号响应帧 */
extern Net_ThaPro_SRes_Query_Available_UpdatePackVer_t g_tha_sres_query_available_update_pack_ver;

#ifdef NET_THA_PRO_USING_AC
/** 设置定时充电 */
extern Net_ThaPro_SReq_Set_Reservation_t g_tha_sreq_set_reservation[NET_SYSTEM_GUN_NUMBER];
/** 取消定时充电 */
extern Net_ThaPro_SReq_Cancel_Reservation_t g_tha_sreq_cancel_reservation[NET_SYSTEM_GUN_NUMBER];
/** 私桩定时充电预约 */
extern Net_ThaPro_SReq_Set_PrivateReservation_t g_tha_sreq_set_private_reservation;
/** 私桩取消特定ID的定时充电预约 */
extern Net_ThaPro_SReq_Cancel_PrivateReservation_t g_tha_sreq_cancel_private_reservatione;
/** 私桩(桩端)查询定时充电预约响应 */
extern Net_ThaPro_SRes_Query_PrivateReservation_t g_tha_sres_query_private_reservation;
/** 私桩PWM电流值下发 */
extern Net_ThaPro_SReq_IssudePwm_t g_tha_sreq_issude_pwm;
#endif /* NET_THA_PRO_USING_AC */

void tha_clear_recv_message_item(uint8_t cmd, uint8_t gunno);
int16_t tha_get_recv_message_item_status(uint8_t cmd, uint8_t gunno, uint8_t *status);
int16_t tha_get_recv_message_item_serial_number(uint8_t cmd, uint8_t gunno, uint8_t *serial_number);

//uint8_t tha_get_message_response_event(uint32_t event);
//void tha_clear_message_response_event(uint32_t event);

int32_t tha_message_recv_init(void);

#endif /* NET_PACK_USING_THA */
#endif /* NET_PACK_NET_THA_INC_THA_MESSAGE_RECEIVE_H_ */
