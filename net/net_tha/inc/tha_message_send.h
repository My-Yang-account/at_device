/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_MESSAGE_SEND_H_
#define NET_PACK_NET_THA_INC_THA_MESSAGE_SEND_H_

#include "tha_message_struct_define.h"

#ifdef NET_PACK_USING_THA

#define NET_THA_MESSAGE_SEND_THREAD_STACK_SIZE               2048       /* 报文发送线程栈 */
#define NET_THA_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE         1024       /* 服务器报文处理线程栈 */
#define NET_THA_HOST_LENGTH_MAX                              128        /* 主机名最大长度 */
#define NET_THA_GENERA_RESPONSE_BUFF_LENGTH                  256        /* 同用响应缓存大小 */
#define NET_THA_LOGIN_OPERATION_INTERVAL                     30 *1000   /* 登录操作间隔(单位ms) */
#define NET_THA_TRANSACTION_REPORT_COUNT_MAX                 0x03       /* 在未得到响应前可连续上报的订单数 */

/** net state */
#define NET_THA_NET_STATE_OPEN_SOCKET                        0x00       /* 打开socket */
#define NET_THA_NET_STATE_LOGIN                              0x01       /* 登录 */
#define NET_THA_NET_STATE_MONITORING                         0x03       /* 监控 */

/** send state */
#define NET_THA_SEND_STATE_COMPLETE                          0x00       /* 报文发送完成 */
#define NET_THA_SEND_STATE_ONGOING                           0x01       /* 报文正在发送中 */
#define NET_THA_WAIT_RESPONSE_TIMEOUT                        5000       /* 报文等待响应超时时间 */

/** 事件类型(请求或响应) */
#define NET_THA_EVENT_TYPE_REQUEST                           0x00       /* 请求事件 */
#define NET_THA_EVENT_TYPE_RESPONSE                          0x01       /* 响应事件 */
#define NET_THA_EVENT_TYPE_SIZE                              0x02       /* 事件类型总数 */
/** 事件句柄(服务器的或充电桩的) */
#define NET_THA_EVENT_HANDLE_CHARGEPILE                      0x00       /* 充电桩事件句柄 */
#define NET_THA_EVENT_HANDLE_SERVER                          0x01       /* 服务器事件句柄 */
#define NET_THA_EVENT_HANDLE_SIZE                            0x02       /* 事件句柄总数 */
/** event option */
#define NET_THA_EVENT_OPTION_AND                             (1 <<0)    /* 事件与 */
#define NET_THA_EVENT_OPTION_OR                              (1 <<1)    /* 事件或 */
#define NET_THA_EVENT_OPTION_CLEAR                           (1 <<2)    /* 清除事件 */

/** chargepile request event */
#define NET_THA_PREQ_EVENT_AUTHORIZE                         0    /* 充电桩请求事件：权限认证 */
#define NET_THA_PREQ_EVENT_REPORT_CHARGE_DATA                1    /* 充电桩请求事件：上报充电数据 */
#define NET_THA_PREQ_EVENT_REPORT_HISTORY_ORDER              2    /* 充电桩请求事件：上报历史订单 */
#define NET_THA_PREQ_EVENT_HEARTBEAT                         3    /* 充电桩请求事件：上报心跳 */
#define NET_THA_PREQ_EVENT_REPORT_BATTERY_STATE              4    /* 充电桩请求事件：上报电池状态 */
#define NET_THA_PREQ_EVENT_STOP_CHARGE_COMPLETE              5    /* 充电桩请求事件：停止充电动作完成，上报信息 */
#define NET_THA_PREQ_EVENT_EVENT_STATE                       6    /* 充电桩请求事件：上报事件状态 */
#define NET_THA_PREQ_EVENT_ERROR_INFO                        7    /* 充电桩请求事件：上报错误信息 */
#define NET_THA_PREQ_EVENT_VIN_AUTHORIZE                     8    /* 充电桩请求事件：VIN码鉴权 */
#define NET_THA_PREQ_EVENT_UPDATE_RESULT                     9    /* 充电桩请求事件：升级包下载结果 */
#define NET_THA_PREQ_EVENT_DEVICE_NET_INFO                   10   /* 充电桩请求事件：上传设备网络信息 */
#define NET_THA_PREQ_EVENT_REPORT_BATTERY_INFO               11   /* 充电桩请求事件：上报电池信息 */
#define NET_THA_PREQ_EVENT_DEVICE_FAULT_INFO                 12   /* 充电桩请求事件：上报设备故障信息 */
#define NET_THA_PREQ_EVENT_CAR_WARNNING_INFO                 13   /* 充电桩请求事件：上报车辆告警信息 */
#define NET_THA_PREQ_EVENT_GUN_REALTEMP                      14   /* 充电桩请求事件：上报充电中充电枪的实时温度 */
#define NET_THA_PREQ_EVENT_QUERY_AVAILABLE_PACK_VER          15   /* 充电桩请求事件：设备查询最新可用升级包软件版本号 */
#define NET_THA_PREQ_EVENT_DEVICE_REQUEST_UPDATE             16   /* 充电桩请求事件：设备发起远程升级请求 */

#define NET_THA_CHARGEPILE_PREQ_NUM                          17   /* 充电桩请求事件总数 */

/** chargepile response event */
#define NET_THA_PRES_EVENT_TIME_SYNC                         0    /* 充电桩响应事件：时间同步响应 */
#define NET_THA_PRES_EVENT_REBOOT                            1    /* 充电桩响应事件：系统重启响应 */
#define NET_THA_PRES_EVENT_READ_CHARGE_DATA                  2    /* 充电桩响应事件：读取充电数据响应 */
#define NET_THA_PRES_EVENT_USER_BINDING                      3    /* 充电桩响应事件：用户绑定响应 */
#define NET_THA_PRES_EVENT_QUERY_SYSTEM_INFO                 4    /* 充电桩响应事件：查询系统信息响应 */
#define NET_THA_PRES_EVENT_UPDATE_SYSTEM_INFO                5    /* 充电桩响应事件：更新系统信息响应 */
#define NET_THA_PRES_EVENT_QUERY_BILLING_RULE                6    /* 充电桩响应事件：查询计费规则响应 */
#define NET_THA_PRES_EVENT_UPDATE_BILLING_RULE               7    /* 充电桩响应事件：更新计费规则响应 */
#define NET_THA_PRES_EVENT_QUERY_CHARGE_SYSTEM_SET           8    /* 充电桩响应事件：查询充电系统设置响应 */
#define NET_THA_PRES_EVENT_UPDATE_CHARGE_SYSTEM_SET          9    /* 充电桩响应事件：更新充电系统设置响应 */
#define NET_THA_PRES_EVENT_QUERY_DEVICE_FAULT_INFO           10   /* 充电桩响应事件：查询设备故障信息响应 */
#define NET_THA_PRES_EVENT_QUERY_BATTERY_CHARGE_INFO         11   /* 充电桩响应事件：查询电池充电信息响应 */
#define NET_THA_PRES_EVENT_QUERY_ORDER_STATE                 12   /* 充电桩响应事件：查询订单状态响应 */
#define NET_THA_PRES_EVENT_SET_RESERVATION                   13   /* 充电桩响应事件：设置定时响应 */
#define NET_THA_PRES_EVENT_CANCEL_RESERVATION                14   /* 充电桩响应事件：取消定时响应 */
#define NET_THA_PRES_EVENT_STOP_CHARGE                       15   /* 充电桩响应事件：停止充电响应 */
#define NET_THA_PRES_EVENT_START_UPDATE                      16   /* 充电桩响应事件：启动远程升级响应 */
#define NET_THA_PRES_EVENT_SET_PILE_RUNNING_PARA             17   /* 充电桩响应事件：设置桩运行参数响应 */
#define NET_THA_PRES_EVENT_SET_CHARGE_POWER                  18   /* 充电桩响应事件：设置充电枪充电功率响应 */
#define NET_THA_PRES_EVENT_UNLOCK_CHARGE                     19   /* 充电桩响应事件：解锁充电响应 */
#define NET_THA_PRES_EVENT_ISSUED_PROCESS_PARA               20   /* 充电桩响应事件：平台下发处理用参数响应 */
#define NET_THA_PRES_EVENT_SET_PRIVATE_RESERVATION           21   /* 充电桩响应事件：私桩定时充电预约响应 */
#define NET_THA_PRES_EVENT_CANCEL_PRIVATE_RESERVATION        22   /* 充电桩响应事件：取消私桩定时充电预约响应 */
#define NET_THA_PRES_EVENT_ISSUED_PWM                        23   /* 充电桩响应事件：私桩PWM电流值下发响应 */

#define NET_THA_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY       24   /* 充电桩响应事件：远程启机异步响应 */
#define NET_THA_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY        25   /* 充电桩响应事件：远程停机异步响应 */
#define NET_THA_PRES_EVENT_SET_POWER_ASYNCHRONOUSLY          26   /* 充电桩响应事件：设置功率百分比异步响应 */

#define NET_THA_CHARGEPILE_PRES_NUM                          27   /* 充电桩响应事件总数 */

/** 登录签到 */
extern Net_ThaPro_PReq_LogIn_t g_tha_preq_login[NET_SYSTEM_GUN_NUMBER];
/** 权限认证 */
extern Net_ThaPro_PReq_Authority_t g_tha_preq_authority[NET_SYSTEM_GUN_NUMBER];   // OK
/** 上报充电数据 */
extern Net_ThaPro_PReq_Report_ChargeData_t g_tha_preq_charge_data[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报历史订单 */
extern Net_ThaPro_PReq_Report_HistoryOrder_t g_tha_preq_history_order[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报心跳 */
extern Net_ThaPro_PReq_Report_HeartBeat_t g_tha_preq_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 请求下载升级包 */
extern Net_ThaPro_PReq_UpdatePack_Load_t g_tha_preq_update_pack_load;
/** 上报电池充电状态 */
extern Net_ThaPro_PReq_Battery_ChargeState_t g_tha_preq_report_battery_charge_state[NET_SYSTEM_GUN_NUMBER];  // OK
/** 停止充电动作完成，上报信息 */
extern Net_ThaPro_PReq_StopCharge_Complete_t g_tha_preq_stop_charge_complete[NET_SYSTEM_GUN_NUMBER];  // OK
/** 事件状态上报 */
extern Net_ThaPro_PReq_Report_EventState_t g_tha_preq_event_state[NET_SYSTEM_GUN_NUMBER];  // OK
/** 错误信息上报 */
extern Net_ThaPro_PReq_Report_ErrorInfo_t g_tha_preq_error_info[NET_SYSTEM_GUN_NUMBER];   // OK
/** 查询ICCID对应的桩编号 */
extern Net_ThaPro_SRes_Query_PileNumberOfIccid_t g_tha_preq_query_pile_number_of_iccid;
/** 设备上报处理用参数 */
extern Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t g_tha_preq_report_process_para;
/** 设备上报桩编号和ICCID的绑定关系（工厂模式） */
extern Net_ThaPro_PReq_Report_BindTie_IccidAndPileNumber_t g_tha_preq_report_bindtie_of_iccid_and_number;
/** VIN码鉴权 */
extern Net_ThaPro_PReq_VinAuthorize_t g_tha_preq_vin_authorize[NET_SYSTEM_GUN_NUMBER];   // OK
/** 升级包下载结果 */
extern Net_ThaPro_PReq_UpdatePack_LoadResult_t g_pile_request_updatepack_loadresult;     // OK
/** 设备网络信息上传 */
extern Net_ThaPro_PReq_Report_NetInfo_t g_pile_request_net_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报电池信息 */
extern Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_t g_tha_preq_report_battery_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报设备故障信息 */
extern Net_ThaPro_PReq_Report_PRes_Query_Device_FaultInfo_t g_tha_preq_report_device_fault_info;  // OK
/** 上报车辆告警信息 */
extern Net_ThaPro_PReq_Report_Car_WarnningInfo_t  g_tha_preq_car_warnning_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 上报充电中充电枪的实时温度 */
extern Net_ThaPro_PReq_Report_Gun_RealTemp_t  g_tha_preq_report_gun_realtemp[NET_SYSTEM_GUN_NUMBER];  // OK
/** 系统复位响应 */
extern Net_ThaPro_SReqPRes_SystemReboot_t g_pres_system_reboot;
/** 查询充电数据响应 */
extern Net_ThaPro_PRes_Query_ChargeData_t g_pres_query_charge_data[NET_SYSTEM_GUN_NUMBER];
/** 查询系统信息响应 */
extern Net_ThaPro_PRes_Query_SystemInfo_t g_pres_query_system_info;
/** 查询计费规则响应 */
extern Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t g_pres_query_billing_rule[NET_SYSTEM_GUN_NUMBER];
/** 查询订单状态响应 */
extern Net_ThaPro_PRes_Query_OrderState_t g_pres_query_order_state;
/** 设备查询最新可用升级包软件版本号 */
extern Net_ThaPro_PReq_Query_Available_UpdatePackVer_t g_pres_query_available_update_pack_ver;  // OK
/** 设备发起远程升级请求 */
extern Net_ThaPro_PReq_Start_Update_t g_preq_start_update;  // OK

/** 上报电池信息（充电电流大于655A时用）、查询电池信息（充电电流大于655A时用） */
extern Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_Expand_t g_tha_preq_pres_battery_info_expand[NET_SYSTEM_GUN_NUMBER];
/** 上报充电数据（充电电流大于655A时用） */
extern Net_ThaPro_PReq_ChargeData_Expand_t g_tha_preq_charge_data_expand[NET_SYSTEM_GUN_NUMBER];
/** 上报电池充电状态（充电电流大于655A时用） */
extern Net_ThaPro_PReq_BatteryState_Expand_t g_tha_preq_battery_state_expand[NET_SYSTEM_GUN_NUMBER];

/** 网络状态 */
enum{
    THA_SOCKET_STATE_PHY,
    THA_SOCKET_STATE_DATA_LINK,
    THA_SOCKET_STATE_OPEN,
    THA_SOCKET_STATE_LOGIN_WAIT,
    THA_SOCKET_STATE_LOGIN_SUCCESS,
};

#pragma pack(1)
typedef struct{
     uint16_t length;
     uint8_t general_transmit_buff[NET_THA_GENERA_RESPONSE_BUFF_LENGTH];
}tha_response_message_buf_t;

typedef struct{
     uint8_t sn;
     uint8_t gunno;
}tha_transaction_info_t;

typedef struct{
    int fd[NET_SYSTEM_GUN_NUMBER];
    uint8_t state[NET_SYSTEM_GUN_NUMBER];
    struct{
        uint16_t open_socket : 3;
        uint16_t login : 3;
        uint16_t heartbeat : 3;
    }operate_fail[NET_SYSTEM_GUN_NUMBER];
}tha_socket_info_t;
#pragma pack()

tha_socket_info_t* tha_get_socket_info(void);
void tha_response_buff_release_mutex(void);
tha_response_message_buf_t* tha_get_response_buff(int32_t timeout);

uint8_t tha_transaction_is_verify(uint8_t gunno);
void tha_set_transaction_verify_state(uint8_t gunno, uint8_t state);
tha_transaction_info_t* tha_get_transaction_info(uint8_t gunno);

uint8_t tha_get_message_send_state(uint8_t gunno, uint32_t message_bit);
void tha_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit);

int32_t tha_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event);
int32_t tha_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf);

uint8_t tha_exist_message_wait_response(uint8_t gunno, uint32_t *state);
void tha_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit);
uint8_t tha_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit);

int32_t net_tha_message_send_init(void);

#endif /* NET_PACK_USING_THA */
#endif /* NET_PACK_NET_THA_INC_THA_MESSAGE_SEND_H_ */
