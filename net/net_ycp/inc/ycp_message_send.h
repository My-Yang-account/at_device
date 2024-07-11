/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_INC_YCP_MESSAGE_SEND_H_
#define NET_NET_YCP_INC_YCP_MESSAGE_SEND_H_

#include "ycp_message_struct_define.h"

#ifdef NET_PACK_USING_YCP

#define NET_YCP_MESSAGE_SEND_THREAD_STACK_SIZE               3072       /* 报文发送线程栈 */
#define NET_YCP_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE         2048       /* 服务器报文处理线程栈 */
#define NET_YCP_HOST_LENGTH_MAX                              128        /* 主机名最大长度 */
#define NET_YCP_GENERA_RESPONSE_BUFF_LENGTH                  256        /* 同用响应缓存大小 */
#define NET_YCP_BILL_SERIAL_NUMBER_COUNT_MAX                 0x03       /* 账单流水号记录最大个数 */

/** net state */
#define NET_YCP_NET_STATE_OPEN_SOCKET                        0x00       /* 打开socket */
#define NET_YCP_NET_STATE_LOGIN                              0x01       /* 登录 */
#define NET_YCP_NET_STATE_MONITORING                         0x03       /* 监控 */

/** send state */
#define NET_YCP_SEND_STATE_COMPLETE                          0x00       /* 报文发送完成 */
#define NET_YCP_SEND_STATE_ONGOING                           0x01       /* 报文正在发送中 */
#define NET_YCP_WAIT_RESPONSE_TIMEOUT                        5000       /* 报文等待响应超时时间 */

/** 事件类型(请求或响应) */
#define NET_YCP_EVENT_TYPE_REQUEST                           0x00       /* 请求事件 */
#define NET_YCP_EVENT_TYPE_RESPONSE                          0x01       /* 响应事件 */
#define NET_YCP_EVENT_TYPE_SIZE                              0x02       /* 事件类型总数 */
/** 事件句柄(服务器的或充电桩的) */
#define NET_YCP_EVENT_HANDLE_CHARGEPILE                      0x00       /* 充电桩事件句柄 */
#define NET_YCP_EVENT_HANDLE_SERVER                          0x01       /* 服务器事件句柄 */
#define NET_YCP_EVENT_HANDLE_SIZE                            0x02       /* 事件句柄总数 */
/** event option */
#define NET_YCP_EVENT_OPTION_AND                             (1 <<0)    /* 事件与 */
#define NET_YCP_EVENT_OPTION_OR                              (1 <<1)    /* 事件或 */
#define NET_YCP_EVENT_OPTION_CLEAR                           (1 <<2)    /* 清除事件 */

/** fautl code */
#define NET_YCP_FAULT_CODE_EMERGENCY                        1000     /* 故障码：急停按下 */
#define NET_YCP_FAULT_CODE_GROUND                           1001     /* 故障码：接地 */
#define NET_YCP_FAULT_CODE_RELAY_ADH                        1002     /* 故障码：继电器黏连 */
#define NET_YCP_FAULT_CODE_GATE                             1003     /* 故障码：门禁 */
#define NET_YCP_FAULT_CODE_CARD_READER                      1004     /* 故障码：读卡器 */
#define NET_YCP_FAULT_CODE_FUMES                            1005     /* 故障码：烟雾告警 */
#define NET_YCP_FAULT_CODE_OVERCURR                         1006     /* 故障码：过流 */
#define NET_YCP_FAULT_CODE_AMMETER                          1007     /* 故障码：电表 */
#define NET_YCP_FAULT_CODE_CP                               1008     /* 故障码：CP */
#define NET_YCP_FAULT_CODE_COMMUNICATION_MODULE             1009     /* 故障码：通信模块 */
#define NET_YCP_FAULT_CODE_AC_RELAY                         1010     /* 故障码：交流接触器异常 */
#define NET_YCP_FAULT_CODE_ENVIRO_TEMP_HIGH                 1011     /* 故障码：环境高温告警 */
#define NET_YCP_FAULT_CODE_ENVIRO_TEMP_LOW                  1012     /* 故障码：环境低温告警 */
#define NET_YCP_FAULT_CODE_FUSE                             1013     /* 故障码：熔断器 */
#define NET_YCP_FAULT_CODE_CIRCUIT_BREAKER                  1014     /* 故障码：断路器 */
#define NET_YCP_FAULT_CODE_LIGHTNING_PROTECTOR              1015     /* 故障码：防雷器 */
#define NET_YCP_FAULT_CODE_BATTERY_REVERSE                  1016     /* 故障码：电池反接 */
#define NET_YCP_FAULT_CODE_INPUT_OVERVOLT                   1017     /* 故障码：输入过压 */
#define NET_YCP_FAULT_CODE_INPUT_UNDERVOLT                  1018     /* 故障码：输入欠压 */
#define NET_YCP_FAULT_CODE_OUTPUT_OVERVOLT                  1019     /* 故障码：输出过压 */
#define NET_YCP_FAULT_CODE_POWER_GRID_VOLT_HIGH             1020     /* 故障码：电网电压高 */
#define NET_YCP_FAULT_CODE_POWER_GRID_VOLT_LOW              1021     /* 故障码：电网电压低 */
#define NET_YCP_FAULT_CODE_POWER_GRID_FREQ_HIGH             1022     /* 故障码：电网频率高 */
#define NET_YCP_FAULT_CODE_POWER_GRID_FREQ_LOW              1023     /* 故障码：电网频率低 */
#define NET_YCP_FAULT_CODE_GUN_OVERTEMP                     1024     /* 故障码：充电枪头过温 */
#define NET_YCP_FAULT_CODE_OUTPUT_OVERCURR                  1025     /* 故障码：输出过流故障 */
#define NET_YCP_FAULT_CODE_MAIN_SWITCH                      1026     /* 故障码：主开关及熔断器故障 */
#define NET_YCP_FAULT_CODE_MISSING_PHASE_A                  1027     /* 故障码：AC 输入-A 相缺相 */
#define NET_YCP_FAULT_CODE_MISSING_PHASE_B                  1028     /* 故障码：AC 输入-B 相缺相 */
#define NET_YCP_FAULT_CODE_MISSING_PHASE_C                  1029     /* 故障码：AC 输入-C 相缺相 */
#define NET_YCP_FAULT_CODE_AIR_INLEY_TEMP_HIGH              1030     /* 故障码：进风口过温 */
#define NET_YCP_FAULT_CODE_AIR_INLEY_TEMP_LOW               1031     /* 故障码：进风口低温 */
#define NET_YCP_FAULT_CODE_AIR_OUTLEY_TEMP_HIGH             1032     /* 故障码：出风口过温 */
#define NET_YCP_FAULT_CODE_AIR_OUTLEY_TEMP_LOW              1033     /* 故障码：出风口低温 */

#define NET_YCP_FAULT_CODE_NO_GUN                           1034     /* 故障码：未插枪 */
#define NET_YCP_FAULT_CODE_CHARGING                         1035     /* 故障码：充电中启动 */
#define NET_YCP_FAULT_CODE_NO_CHARGE                        1036     /* 故障码：未充电停止 */
#define NET_YCP_FAULT_CODE_BILLING                          1037     /* 故障码：无效计费规则 */


/** chargepile request event */
#define NET_YCP_PREQ_EVENT_SIGNIN                            0    /* 充电桩请求事件：登录 */
#define NET_YCP_PREQ_EVENT_TIME_SYNC                         1    /* 充电桩请求事件：对时 */
#define NET_YCP_PREQ_EVENT_HEARTBEAT                         2    /* 充电桩请求事件：心跳 */
#define NET_YCP_PREQ_EVENT_BILLING_MODEL_VERIFY              3    /* 充电桩请求事件：计费模型验证 */
#define NET_YCP_PREQ_EVENT_REPORT_STATE_DATA                 4    /* 充电桩请求事件：上报单枪状态数据 */
#define NET_YCP_PREQ_EVENT_APPLY_START_CHARGE                5    /* 充电桩请求事件：主动申请启动充电 */
#define NET_YCP_PREQ_EVENT_TRANSACTION_RECORD                6    /* 充电桩请求事件：交易记录 */
#define NET_YCP_PREQ_EVENT_REPORT_DEVICE_FAULT               7    /* 充电桩请求事件：上报设备故障 */
#define NET_YCP_PREQ_EVENT_CHARGE_SHAKE_HAND                 8    /* 充电桩请求事件：充电握手 */
#define NET_YCP_PREQ_EVENT_PARA_CONFIG                       9    /* 充电桩请求事件：参数配置 */
#define NET_YCP_PREQ_EVENT_CHARGE_END                        10   /* 充电桩请求事件：充电结束 */
#define NET_YCP_PREQ_EVENT_ERROR_MESSAGE                     11   /* 充电桩请求事件：错误报文 */
#define NET_YCP_PREQ_EVENT_BMS_STOP                          12   /* 充电桩请求事件：充电阶段 BMS 中止 */
#define NET_YCP_PREQ_EVENT_CHARGER_STOP                      13   /* 充电桩请求事件：充电阶段充电机中止 */
#define NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE        14   /* 充电桩请求事件：充电过程 BMS 需求与充电机输出 */
#define NET_YCP_PREQ_EVENT_BMS_INFO                          15   /* 充电桩请求事件：充电过程 BMS 信息 */

#define NET_YCP_CHARGEPILE_PREQ_NUM                          16   /* 充电桩请求事件总数 */

/** chargepile response event */
#define NET_YCP_PRES_EVENT_SET_PARA                          0    /* 充电桩响应事件：设置充电桩参数 */
#define NET_YCP_PRES_EVENT_QRCODE_CONFIG                     1    /* 充电桩响应事件：二维码配置命令回复 */
#define NET_YCP_PRES_EVENT_SET_SERVICE_PHONE                 2    /* 充电桩响应事件：设置客服电话命令回复 */
#define NET_YCP_PRES_EVENT_SET_BILLING_MODEL                 3    /* 充电桩响应事件：设置计费模型 */
#define NET_YCP_PRES_EVENT_QUERY_STATE_DATA                  4    /* 充电桩响应事件：查询单枪状态数据 */
#define NET_YCP_PRES_EVENT_QUERY_STATE_DATA_ALL              5    /* 充电桩响应事件：查询所有枪状态数据 */
#define NET_YCP_PRES_EVENT_SERVER_START_CHARGE               6    /* 充电桩响应事件：远程启动充电命令回复 */
#define NET_YCP_PRES_EVENT_SERVER_STOP_CHARGE                7    /* 充电桩响应事件：远程停机命令回复 */
#define NET_YCP_PRES_EVENT_REMOTE_REBOOT                     8    /* 充电桩响应事件：远程重启 */
#define NET_YCP_PRES_EVENT_REMOTE_UPDATE                     9    /* 充电桩响应事件：远程更新 */
#define NET_YCP_PRES_EVENT_MODIFY_SERVER_ADDR                10   /* 充电桩响应事件：修改服务器地址 */
#define NET_YCP_PRES_EVENT_QUERY_DEVICE_FAULT                11   /* 充电桩响应事件：查询设备故障 */

#define NET_YCP_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY       12   /* 充电桩响应事件：远程启机异步响应 */
#define NET_YCP_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY        13   /* 充电桩响应事件：远程停机异步响应 */
#define NET_YCP_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY  14   /* 充电桩响应事件：设置功率百分比异步响应 */

#define NET_YCP_CHARGEPILE_PRES_NUM                          15   /* 充电桩响应事件总数 */

/** 登录签到 */
extern Net_YcpPro_PReq_LogIn_t g_ycp_preq_login;
 /** 对时 */
extern Net_YcpPro_PReq_TimeSync_t g_ycp_preq_time_sync;  // OK
/** 上报心跳 */
extern Net_YcpPro_PReq_HeartBeat_t g_ycp_preq_heartbeat;   // OK
/** 计费模型验证 */
extern Net_YcpPro_PReq_BillingModel_Verify_t g_ycp_preq_billing_model_verify;   // OK
/** 上报单枪状态数据 */
extern Net_YcpPro_PRes_Query_PReq_Report_PileState_t g_ycp_preq_report_state_data[NET_SYSTEM_GUN_NUMBER];  // OK
/** 充电桩主动申请启动充电 */
extern Net_YcpPro_PReq_ApplyCharge_Active_t g_ycp_preq_apply_charge_active[NET_SYSTEM_GUN_NUMBER];  // OK
/** 交易记录 */
extern Net_YcpPro_PReq_TransactionRecords_t g_ycp_preq_transaction_records[NET_SYSTEM_GUN_NUMBER];
/** 上报设备故障 */
extern Net_YcpPro_PReq_Report_DeviceFault_t g_ycp_preq_report_device_fault;  // OK
/** 充电握手 */
extern Net_YcpPro_PReq_ShakeHand_t g_ycp_preq_shake_hand[NET_SYSTEM_GUN_NUMBER];   // OK
/** 参数配置 */
extern Net_YcpPro_PReq_ParameterConfig_t g_ycp_preq_parameter_config[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电结束 */
extern Net_YcpPro_PReq_ChargeFinish_t g_ycp_preq_charge_finish[NET_SYSTEM_GUN_NUMBER];   // OK
/** 错误报文 */
extern Net_YcpPro_PReq_ErrorMessage_t g_ycp_preq_error_message[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中 BMS 终止 */
extern Net_YcpPro_PReq_BmsEnd_t g_ycp_preq_bms_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程中充电机终止 */
extern Net_YcpPro_PReq_ChargerEnd_t g_ycp_preq_charger_end[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 需求与充电机输出 */
extern Net_YcpPro_PReq_BmsCommand_ChargerOut_t g_ycp_preq_bmscommand_chargerout[NET_SYSTEM_GUN_NUMBER];   // OK
/** 充电过程 BMS 信息 */
extern Net_YcpPro_PReq_BmsInfo_t g_ycp_preq_bms_info[NET_SYSTEM_GUN_NUMBER];   // OK
/** 升级结果上送 */
extern Net_YcpPro_PRes_RemoteUpdate_t g_ycp_pres_remote_update;

/** 网络状态 */
enum{
    YCP_SOCKET_STATE_PHY,
    YCP_SOCKET_STATE_DATA_LINK,
    YCP_SOCKET_STATE_OPEN,
    YCP_SOCKET_STATE_LOGIN_WAIT,
    YCP_SOCKET_STATE_LOGIN_SUCCESS,
};

typedef struct{
     uint16_t length;
     uint8_t general_transmit_buff[NET_YCP_GENERA_RESPONSE_BUFF_LENGTH];
}ycp_response_message_buf_t;

typedef struct{
    int fd;
    uint8_t state;
    uint8_t heartbeat;
    struct{
        uint8_t open_socket : 4;
        uint8_t login : 4;
    }operate_fail;
}ycp_socket_info_t;

ycp_socket_info_t* ycp_get_socket_info(void);

uint8_t ycp_transaction_is_verify(uint8_t gunno);
void ycp_set_transaction_verify_state(uint8_t gunno, uint8_t state);

uint8_t ycp_get_message_send_state(uint8_t gunno, uint32_t message_bit);
void ycp_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit);

int32_t ycp_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event);
int32_t ycp_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf);

uint8_t ycp_exist_message_wait_response(uint8_t gunno, uint32_t *state);
void ycp_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit);
uint8_t ycp_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit);

void ycp_ascii_to_bcd(uint8_t *ascii, uint8_t *bcd, uint8_t len);
void ycp_timestamp_to_timebcd(uint32_t timestamp, uint8_t *bcd, uint8_t len);
uint32_t ycp_timebcd_to_timestamp(uint8_t *bcd, uint8_t len);

int32_t ycp_message_send_init(void);

#endif /* NET_PACK_USING_YCP */

#endif /* NET_NET_YCP_INC_YCP_MESSAGE_SEND_H_ */
