/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_MESSAGE_SEND_H_
#define NET_PACK_NET_YKC_INC_YKC_MESSAGE_SEND_H_

#include "ykc_message_struct_define.h"

#ifdef NET_PACK_USING_YKC

#define NET_YKC_MESSAGE_SEND_THREAD_STACK_SIZE               3072       /* 报文发送线程栈 */
#define NET_YKC_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE         2048       /* 服务器报文处理线程栈 */
#define NET_YKC_HOST_LENGTH_MAX                              128        /* 主机名最大长度 */
#define NET_YKC_GENERA_RESPONSE_BUFF_LENGTH                  256        /* 同用响应缓存大小 */
#define NET_YKC_BILL_SERIAL_NUMBER_COUNT_MAX                 0x03       /* 账单流水号记录最大个数 */

/** net state */
#define NET_YKC_NET_STATE_OPEN_SOCKET                        0x00       /* 打开socket */
#define NET_YKC_NET_STATE_LOGIN                              0x01       /* 登录 */
#define NET_YKC_NET_STATE_MONITORING                         0x03       /* 监控 */

/** send state */
#define NET_YKC_SEND_STATE_COMPLETE                          0x00       /* 报文发送完成 */
#define NET_YKC_SEND_STATE_ONGOING                           0x01       /* 报文正在发送中 */
#define NET_YKC_WAIT_RESPONSE_TIMEOUT                        5000       /* 报文等待响应超时时间 */

/** 事件类型(请求或响应) */
#define NET_YKC_EVENT_TYPE_REQUEST                           0x00       /* 请求事件 */
#define NET_YKC_EVENT_TYPE_RESPONSE                          0x01       /* 响应事件 */
#define NET_YKC_EVENT_TYPE_SIZE                              0x02       /* 事件类型总数 */
/** 事件句柄(服务器的或充电桩的) */
#define NET_YKC_EVENT_HANDLE_CHARGEPILE                      0x00       /* 充电桩事件句柄 */
#define NET_YKC_EVENT_HANDLE_SERVER                          0x01       /* 服务器事件句柄 */
#define NET_YKC_EVENT_HANDLE_SIZE                            0x02       /* 事件句柄总数 */
/** event option */
#define NET_YKC_EVENT_OPTION_AND                             (1 <<0)    /* 事件与 */
#define NET_YKC_EVENT_OPTION_OR                              (1 <<1)    /* 事件或 */
#define NET_YKC_EVENT_OPTION_CLEAR                           (1 <<2)    /* 清除事件 */

/** start charge fail reason */
#define NET_YKC_START_FAIL_REASON_NO                        0     /* 启动失败原因：无 */
#define NET_YKC_START_FAIL_REASON_PILE_NUMBER_NOT_MATCH     1     /* 启动失败原因：设备编号不匹配 */
#define NET_YKC_START_FAIL_REASON_IS_CHARGING               2     /* 启动失败原因：枪已在充电 */
#define NET_YKC_START_FAIL_REASON_IS_FAULTING               3     /* 启动失败原因：设备故障 */
#define NET_YKC_START_FAIL_REASON_OFFLINE                   4     /* 启动失败原因：设备离线 */
#define NET_YKC_START_FAIL_REASON_NO_GUN                    5
#define NET_YKC_START_FAIL_REASON_BILLING                   6     /* 启动失败原因：无效计费规则 */
#define NET_YKC_START_FAIL_REASON_GUN_STATE                 7     /* 启动失败原因：枪状态不适合 */

/** stop charge fail reason */
#define NET_YKC_STOP_FAIL_REASON_NO                         0     /* 停机失败原因：无 */
#define NET_YKC_STOP_FAIL_REASON_PILE_NUMBER_NOT_MATCH      1     /* 停机失败原因：设备编号不匹配 */
#define NET_YKC_STOP_FAIL_REASON_NOT_CHARGING               2     /* 停机失败原因：枪未在充电 */
#define NET_YKC_STOP_FAIL_REASON_OTHER                      3     /* 停机失败原因：其它 */

/** chargepile request event */
#define NET_YKC_PREQ_EVENT_SIGNIN                            0    /* 充电桩请求事件：登录 */
#define NET_YKC_PREQ_EVENT_HEARTBEAT                         1    /* 充电桩请求事件：心跳 */
#define NET_YKC_PREQ_EVENT_BILLING_MODEL_VERIFY              2    /* 充电桩请求事件：计费模型验证 */
#define NET_YKC_PREQ_EVENT_BILLING_MODEL_REQUEST             3    /* 充电桩请求事件：请求计费模型 */
#define NET_YKC_PREQ_EVENT_REPORT_REALTIME_DATA              4    /* 充电桩请求事件：上传实时监测数据 */
#define NET_YKC_PREQ_EVENT_CHARGE_SHAKE_HAND                 5    /* 充电桩请求事件：充电握手 */
#define NET_YKC_PREQ_EVENT_PARA_CONFIG                       6    /* 充电桩请求事件：参数配置 */
#define NET_YKC_PREQ_EVENT_CHARGE_END                        7    /* 充电桩请求事件：充电结束 */
#define NET_YKC_PREQ_EVENT_ERROR_MESSAGE                     8    /* 充电桩请求事件：错误报文 */
#define NET_YKC_PREQ_EVENT_BMS_STOP                          9    /* 充电桩请求事件：充电阶段 BMS 中止 */
#define NET_YKC_PREQ_EVENT_CHARGER_STOP                      10   /* 充电桩请求事件：充电阶段充电机中止 */
#define NET_YKC_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE        11   /* 充电桩请求事件：充电过程 BMS 需求与充电机输出 */
#define NET_YKC_PREQ_EVENT_BMS_INFO                          12   /* 充电桩请求事件：充电过程 BMS 信息 */
#define NET_YKC_PREQ_EVENT_APPLY_START_CHARGE                13   /* 充电桩请求事件：主动申请启动充电 */
#define NET_YKC_PREQ_EVENT_TRANSACTION_RECORD                14   /* 充电桩请求事件：交易记录 */
#define NET_YKC_PREQ_EVENT_GROUNDLOCK_INFO                   15   /* 充电桩请求事件：地锁数据 */
#define NET_YKC_PREQ_EVENT_APPLY_START_MERGECHARGE           16   /* 充电桩请求事件：主动申请启动并充充电 */
#define NET_YKC_PREQ_EVENT_REALTIME_FAULT_REPORT             17   /* 充电桩请求事件：实时故障上报 */

#ifdef NET_YKC_MESSAGE_USING_DUPU
#define NET_YKC_PREQ_EVENT_STORED_ENERGY_INFO_REPORT         18   /* 充电桩请求事件：储能信息上报 */
#define NET_YKC_CHARGEPILE_PREQ_NUM                          19   /* 充电桩请求事件总数 */
#else
#define NET_YKC_CHARGEPILE_PREQ_NUM                          18   /* 充电桩请求事件总数 */
#endif /* NET_YKC_MESSAGE_USING_DUPU */

/** chargepile response event */
#define NET_YKC_PRES_EVENT_READ_REALTIME_DATA                0    /* 充电桩响应事件：读取实时监测数据 */
#define NET_YKC_PRES_EVENT_SERVER_START_CHARGE               1    /* 充电桩响应事件：远程启动充电命令回复 */
#define NET_YKC_PRES_EVENT_SERVER_STOP_CHARGE                2    /* 充电桩响应事件：远程停机命令回复 */
#define NET_YKC_PRES_EVENT_ACCOUNT_BALLANCE_UPDATE           3    /* 充电桩响应事件：余额更新 */
#define NET_YKC_PRES_EVENT_SYNC_OFFLINECARD                  4    /* 充电桩响应事件：离线卡数据同步 */
#define NET_YKC_PRES_EVENT_CLEAR_OFFLINECARD                 5    /* 充电桩响应事件：离线卡数据清除 */
#define NET_YKC_PRES_EVENT_QUERY_OFFLINECARD                 6    /* 充电桩响应事件：离线卡数据查询 */
#define NET_YKC_PRES_EVENT_SET_WORK_PARA                     7    /* 充电桩响应事件：设置充电桩工作参数 */
#define NET_YKC_PRES_EVENT_TIME_SYNC                         8    /* 充电桩响应事件：对时设置 */
#define NET_YKC_PRES_EVENT_SET_BILLING_MODEL                 9    /* 充电桩响应事件：设置计费模型 */
#define NET_YKC_PRES_EVENT_GROUNDLOCK_LIFTING                10   /* 充电桩响应事件：遥控地锁升锁与降锁 */
#define NET_YKC_PRES_EVENT_REMOTE_REBOOT                     11   /* 充电桩响应事件：远程重启 */
#define NET_YKC_PRES_EVENT_REMOTE_UPDATE                     12   /* 充电桩响应事件：远程更新 */
#define NET_YKC_PRES_EVENT_SERVER_START_MERGECHARGE          13   /* 充电桩响应事件：远程并充启动充电命令回复 */
#define NET_YKC_PRES_EVENT_QRCODE_CONFIG_GC                  14   /* 充电桩响应事件：二维码配置命令回复(国充) */
#define NET_YKC_PRES_EVENT_QRCODE_CONFIG_YKC15               15   /* 充电桩响应事件：二维码配置命令回复(云快充1.5) */
#define NET_YKC_PRES_EVENT_QRCODE_CONFIG_TLD                 16   /* 充电桩响应事件：二维码配置命令回复(特来电) */

#define NET_YKC_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY       17   /* 充电桩响应事件：远程启机异步响应 */
#define NET_YKC_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY        18   /* 充电桩响应事件：远程停机异步响应 */
#define NET_YKC_PRES_EVENT_START_MERGECHARGE_ASYNCHRONOUSLY  19   /* 充电桩响应事件：远程并充启机异步响应 */
#define NET_YKC_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY  20   /* 充电桩响应事件：设置功率百分比异步响应 */

#define NET_YKC_CHARGEPILE_PRES_NUM                          21   /* 充电桩响应事件总数 */

/** 登录签到 */
extern Net_YkcPro_PReq_LogIn_t g_ykc_preq_login;
/** 上报心跳 */
extern Net_YkcPro_PReq_HeartBeat_t g_ykc_preq_heartbeat[NET_SYSTEM_GUN_NUMBER];   // OK
/** 计费模型验证 */
extern Net_YkcPro_PReq_BillingModel_Verify_t g_ykc_preq_billing_model_verify;   // OK
/** 计费模型请求 */
extern Net_YkcPro_PReq_BillingModel_Request_t g_ykc_preq_billing_model_request;  // OK
/** 上传实时监测数据 */
extern Net_YkcPro_PRes_Query_PReq_Report_RealTimeData_t g_ykc_preq_report_realtime_data[NET_SYSTEM_GUN_NUMBER];  // OK
/** 充电握手 */
extern Net_YkcPro_PReq_ShakeHand_t g_ykc_preq_shake_hand[NET_SYSTEM_GUN_NUMBER];   // OK
/** 参数配置 */
extern Net_YkcPro_PReq_ParameterConfig_t g_ykc_preq_parameter_config[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电结束 */
extern Net_YkcPro_PReq_ChargeFinish_t g_ykc_preq_charge_finish[NET_SYSTEM_GUN_NUMBER];   // OK
/** 错误报文 */
extern Net_YkcPro_PReq_ErrorMessage_t g_ykc_preq_error_message[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中 BMS 终止 */
extern Net_YkcPro_PReq_BmsEnd_t g_ykc_preq_bms_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中充电机终止 */
extern Net_YkcPro_PReq_ChargerEnd_t g_ykc_preq_charger_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 需求与充电机输出 */
extern Net_YkcPro_PReq_BmsCommand_ChargerOut_t g_ykc_preq_bmscommand_chargerout[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 信息 */
extern Net_YkcPro_PReq_BmsInfo_t g_ykc_preq_bms_info[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电桩主动申请启动充电 */
extern Net_YkcPro_PReq_ApplyCharge_Active_t g_ykc_preq_apply_charge_active[NET_SYSTEM_GUN_NUMBER];  // OK
/** 交易记录 */
extern Net_YkcPro_PReq_TransactionRecords_t g_ykc_preq_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 地锁数据上送 */
extern Net_YkcPro_PReq_GroundLock_Info_t g_ykc_preq_ground_lock_info[NET_SYSTEM_GUN_NUMBER];  // OK
/** 充电桩主动申请并充充电 */
extern Net_YkcPro_PReq_ApplyMergeCharge_Active_t g_ykc_preq_apply_merge_charge_active[NET_SYSTEM_GUN_NUMBER];  // OK

/** 升级结果上送 */
extern Net_YkcPro_PRes_RemoteUpdate_t g_ykc_pres_remote_update;
#ifdef NET_YKC_MESSAGE_USING_DUPU
/** 上送储能信息 */
extern Net_YkcPro_PReq_StoredEnergy_Info_t g_ykc_preq_stored_energy_info;
#endif /* NET_YKC_MESSAGE_USING_DUPU */

/** 网络状态 */
enum{
    YKC_SOCKET_STATE_PHY,
    YKC_SOCKET_STATE_SIM,
    YKC_SOCKET_STATE_DATA_LINK,
    YKC_SOCKET_STATE_MODULE_INIT,
    YKC_SOCKET_STATE_OPEN,
    YKC_SOCKET_STATE_LOGIN_WAIT,
    YKC_SOCKET_STATE_LOGIN_SUCCESS,
};

#pragma pack(1)

typedef struct{
     uint16_t length;
     uint8_t general_transmit_buff[NET_YKC_GENERA_RESPONSE_BUFF_LENGTH];
}ykc_response_message_buf_t;

typedef struct{
    int fd;
    uint8_t socket_state;
    uint8_t program_state;
    uint8_t heartbeat[NET_SYSTEM_GUN_NUMBER];
    uint8_t domain_is_prase;   /** 域名已解析 */
    struct{
        uint8_t open_socket : 4;
        uint8_t login : 4;
    }operate_fail;
}ykc_socket_info_t;

#pragma pack()

uint8_t ykc_is_recved_billing_rule(void);
ykc_socket_info_t* ykc_get_socket_info(void);

uint8_t ykc_transaction_is_verify(uint8_t gunno);
void ykc_set_transaction_verify_state(uint8_t gunno, uint8_t state);

uint8_t ykc_get_message_send_state(uint8_t gunno, uint32_t message_bit);
void ykc_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit);

int32_t ykc_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event);
int32_t ykc_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf);

uint8_t ykc_exist_message_wait_response(uint8_t gunno, uint32_t *state);
void ykc_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit);
uint8_t ykc_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit);

void ykc_ascii_to_bcd(uint8_t *ascii, uint8_t alen, uint8_t *bcd, uint8_t blen);

int32_t ykc_message_send_init(void);

#endif /* NET_PACK_USING_YKC */
#endif /* NET_PACK_NET_YKC_INC_YKC_MESSAGE_SEND_H_ */
