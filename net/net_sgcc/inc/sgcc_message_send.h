/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */
#ifndef NET_NET_SGCC_INC_SGCC_MESSAGE_SEND_H_
#define NET_NET_SGCC_INC_SGCC_MESSAGE_SEND_H_

#include "protocol.h"
#include "net_pack_config.h"

#ifdef NET_PACK_USING_SGCC

#define NET_SGCC_MESSAGE_SEND_THREAD_STACK_SIZE               (12 *1024)       /* 报文发送线程栈 */
#define NET_SGCC_SERVER_MESSAGE_PRO_THREAD_STACK_SIZE         2048       /* 服务器报文处理线程栈 */
#define NET_SGCC_CONNECT_THREAD_STACK_SIZE                    (12 *1024)       /* 连接线程栈 */
#define NET_SGCC_HOST_LENGTH_MAX                              128        /* 主机名最大长度 */
#define NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH                  256        /* 同用响应缓存大小 */
#define NET_SGCC_BILL_SERIAL_NUMBER_COUNT_MAX                 0x03       /* 账单流水号记录最大个数 */

/** net state */
#define NET_SGCC_NET_STATE_ELEMENT_GROUP                      0x00       /* 获取三元组并创建套件 */
#define NET_SGCC_NET_STATE_OPEN                               0x01       /* 打开socket */
#define NET_SGCC_NET_STATE_LOGIN                              0x02       /* 登录 */
#define NET_SGCC_NET_STATE_MONITORING                         0x03       /* 监控 */

/** send state */
#define NET_SGCC_SEND_STATE_COMPLETE                          0x00       /* 报文发送完成 */
#define NET_SGCC_SEND_STATE_ONGOING                           0x01       /* 报文正在发送中 */
#define NET_SGCC_WAIT_RESPONSE_TIMEOUT                        5000       /* 报文等待响应超时时间 */

/** 事件类型(请求或响应) */
#define NET_SGCC_EVENT_TYPE_REQUEST                           0x00       /* 请求事件 */
#define NET_SGCC_EVENT_TYPE_RESPONSE                          0x01       /* 响应事件 */
#define NET_SGCC_EVENT_TYPE_SIZE                              0x02       /* 事件类型总数 */
/** 事件句柄(服务器的或充电桩的) */
#define NET_SGCC_EVENT_HANDLE_CHARGEPILE                      0x00       /* 充电桩事件句柄 */
#define NET_SGCC_EVENT_HANDLE_SERVER                          0x01       /* 服务器事件句柄 */
#define NET_SGCC_EVENT_HANDLE_SIZE                            0x02       /* 事件句柄总数 */
/** event option */
#define NET_SGCC_EVENT_OPTION_AND                             (1 <<0)    /* 事件与 */
#define NET_SGCC_EVENT_OPTION_OR                              (1 <<1)    /* 事件或 */
#define NET_SGCC_EVENT_OPTION_CLEAR                           (1 <<2)    /* 清除事件 */

/** chargepile request event */
#define NET_SGCC_PREQ_EVENT_REPORT_SDK_VERSION                0    /* 充电桩请求事件：上报SDK版本信息 */
#define NET_SGCC_PREQ_EVENT_REPORT_FIRMWARE_INFO              1    /* 充电桩请求事件：上报固件信息 */
#define NET_SGCC_PREQ_EVENT_REQUEST_BILLING_MODE              2    /* 充电桩请求事件：请求计费模型 */
#define NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_NONCHARGING     3    /* 充电桩请求事件：上报枪状态数据(非充电中) */
#define NET_SGCC_PREQ_EVENT_REPORT_STATE_DATA_CHARGING        4    /* 充电桩请求事件：上报枪状态数据(充电中) */
#define NET_SGCC_PREQ_EVENT_GUNSTATE_CHANGED                  5    /* 充电桩请求事件：充电枪状态发生变化 */
#define NET_SGCC_PREQ_EVENT_TRANSACTION_RECORD                6    /* 充电桩请求事件：上报交易记录 */
#define NET_SGCC_PREQ_EVENT_REPORT_AMMETER_VALUE              7    /* 充电桩请求事件：上报电表底值 */
#define NET_SGCC_PREQ_EVENT_REPORT_BMS_DATA                   8    /* 充电桩请求事件：上报BMS数据 */
#define NET_SGCC_PREQ_EVENT_REPORT_MONITOR_PROPERTY           9    /* 充电桩请求事件：上送实时监测属性 */
#define NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING             10   /* 充电桩请求事件：上送故障告警信息 */
#define NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE                11   /* 充电桩请求事件：主动申请启动充电 */
#define NET_SGCC_PREQ_EVENT_REPORT_DEV_RECORD                 12   /* 充电桩请求事件：上报设备记录 */
#define NET_SGCC_PREQ_EVENT_REPORT_CAR_INFO                   13   /* 充电桩请求事件：上报车辆信息 */
#define NET_SGCC_PREQ_EVENT_CONFIG_UPDATE                     14   /* 充电桩请求事件：配置更新请求 */


//#define NET_SGCC_PREQ_EVENT_APPLY_START_CHARGE                5    /* 充电桩请求事件：主动申请启动充电 */
//#define NET_SGCC_PREQ_EVENT_REPORT_DEVICE_FAULT               7    /* 充电桩请求事件：上报设备故障 */
//#define NET_SGCC_PREQ_EVENT_CHARGE_SHAKE_HAND                 8    /* 充电桩请求事件：充电握手 */
//#define NET_SGCC_PREQ_EVENT_PARA_CONFIG                       9    /* 充电桩请求事件：参数配置 */
//#define NET_SGCC_PREQ_EVENT_CHARGE_END                        10   /* 充电桩请求事件：充电结束 */
//#define NET_SGCC_PREQ_EVENT_ERROR_MESSAGE                     11   /* 充电桩请求事件：错误报文 */
//#define NET_SGCC_PREQ_EVENT_BMS_STOP                          12   /* 充电桩请求事件：充电阶段 BMS 中止 */
//#define NET_SGCC_PREQ_EVENT_CHARGER_STOP                      13   /* 充电桩请求事件：充电阶段充电机中止 */
//#define NET_SGCC_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE        14   /* 充电桩请求事件：充电过程 BMS 需求与充电机输出 */
//#define NET_SGCC_PREQ_EVENT_BMS_INFO                          15   /* 充电桩请求事件：充电过程 BMS 信息 */

#define NET_SGCC_CHARGEPILE_PREQ_NUM                          14   /* 充电桩请求事件总数 */

/** chargepile response event */
#define NET_SGCC_PRES_EVENT_SERVER_START_CHARGE               0    /* 充电桩响应事件：远程启动充电命令回复 */
#define NET_SGCC_PRES_EVENT_SERVER_STOP_CHARGE                1    /* 充电桩响应事件：远程停机命令回复 */
#define NET_SGCC_PRES_EVENT_APPLY_CHARGE_RESULT               2    /* 充电桩响应事件：申请充电启动结果回复 */
//#define NET_SGCC_PRES_EVENT_ELOCK_CTRL_RESULT                 3    /* 充电桩响应事件：电子锁控制结果回复 */

//#define NET_SGCC_PRES_EVENT_SET_PARA                          0    /* 充电桩响应事件：设置充电桩参数 */
//#define NET_SGCC_PRES_EVENT_QRCODE_CONFIG                     1    /* 充电桩响应事件：二维码配置命令回复 */
//#define NET_SGCC_PRES_EVENT_SET_SERVICE_PHONE                 2    /* 充电桩响应事件：设置客服电话命令回复 */
//#define NET_SGCC_PRES_EVENT_SET_BILLING_MODEL                 3    /* 充电桩响应事件：设置计费模型 */
//#define NET_SGCC_PRES_EVENT_QUERY_STATE_DATA                  4    /* 充电桩响应事件：查询单枪状态数据 */
//#define NET_SGCC_PRES_EVENT_QUERY_STATE_DATA_ALL              5    /* 充电桩响应事件：查询所有枪状态数据 */
//#define NET_SGCC_PRES_EVENT_SERVER_START_CHARGE               6    /* 充电桩响应事件：远程启动充电命令回复 */
//#define NET_SGCC_PRES_EVENT_SERVER_STOP_CHARGE                7    /* 充电桩响应事件：远程停机命令回复 */
//#define NET_SGCC_PRES_EVENT_REMOTE_REBOOT                     8    /* 充电桩响应事件：远程重启 */
//#define NET_SGCC_PRES_EVENT_REMOTE_UPDATE                     9    /* 充电桩响应事件：远程更新 */
//#define NET_SGCC_PRES_EVENT_MODIFY_SERVER_ADDR                10   /* 充电桩响应事件：修改服务器地址 */
//#define NET_SGCC_PRES_EVENT_QUERY_DEVICE_FAULT                11   /* 充电桩响应事件：查询设备故障 */

#define NET_SGCC_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY       12   /* 充电桩响应事件：远程启机异步响应 */
#define NET_SGCC_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY        13   /* 充电桩响应事件：远程停机异步响应 */
#define NET_SGCC_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY  14   /* 充电桩响应事件：设置功率百分比异步响应 */
#define NET_SGCC_PRES_EVENT_APPLY_CHARGE_ASYNCHRONOUSLY       15   /* 充电桩响应事件：申请充电启动结果异步响应 */

#define NET_SGCC_CHARGEPILE_PRES_NUM                          16   /* 充电桩响应事件总数 */

/**=======================================[充电桩公共请求报文]=======================================*/
extern evs_event_fireware_info evs_event_firmware_infos;
extern evs_event_ver_info evs_event_ver_infos;
//extern evs_event_devmdu_info evs_event_devmdu_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_card_info evs_event_card_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_card_auth evs_event_card_auths[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_card_auth_result evs_event_card_auth_results[NET_SYSTEM_GUN_NUMBER];
extern evs_event_ask_feeModel evs_event_ask_feeModels[NET_SYSTEM_GUN_NUMBER];
extern evs_event_startResult evs_event_startResults[NET_SYSTEM_GUN_NUMBER];
extern evs_event_startCharge evs_event_startCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_event_stopCharge evs_event_stopCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_event_tradeInfo evs_event_tradeInfos[NET_SYSTEM_GUN_NUMBER];
extern evs_event_alarm evs_event_alarms[NET_SYSTEM_GUN_NUMBER];
extern evs_event_groundLock_change evs_event_groundLock_changes[NET_SYSTEM_GUN_NUMBER];
extern evs_event_gateLock_change evs_event_gateLock_changes[NET_SYSTEM_GUN_NUMBER];
extern evs_event_pile_stutus_change evs_event_pile_stutus_changes[NET_SYSTEM_GUN_NUMBER];
extern evs_event_car_info evs_event_car_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_vinList_result evs_event_vinList_results[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_time_sync_result evs_event_time_sync_results[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_charge_connect_change evs_event_charge_connect_changes[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_card_check_error evs_event_card_check_errors[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_ble_plug_charge_info evs_event_ble_plug_charge_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_ble_conn_change evs_event_ble_conn_changes[NET_SYSTEM_GUN_NUMBER];
extern evs_event_logQuery_Result evs_event_logQuery_Results[NET_SYSTEM_GUN_NUMBER];

/**=======================================[直流充电桩请求报文]=======================================*/
//extern evs_event_ccu_info evs_event_ccu_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_pcu_info evs_event_pcu_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_cu_info evs_event_cu_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_switch_info evs_event_switch_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_edas_info evs_event_edas_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_cu_work_info evs_event_cu_work_infos[NET_SYSTEM_GUN_NUMBER];
//extern evs_smart_gun_auth_param evs_smart_gun_auth_params[NET_SYSTEM_GUN_NUMBER];

/**=======================================[充电桩公共请求报文]=======================================*/
extern evs_property_meter evs_property_meters[NET_SYSTEM_GUN_NUMBER];

/**=======================================[交流充电桩请求报文]=======================================*/
extern evs_property_acPile evs_property_acPiles;
extern evs_property_ac_work evs_property_ac_works[NET_SYSTEM_GUN_NUMBER];
extern evs_property_ac_nonWork evs_property_ac_nonWorks[NET_SYSTEM_GUN_NUMBER];

/**=======================================[直流充电桩请求报文]=======================================*/
extern evs_property_dcPile evs_property_dcPiles;
extern evs_property_BMS evs_property_BMSs[NET_SYSTEM_GUN_NUMBER];
extern evs_property_dc_work evs_property_dc_works[NET_SYSTEM_GUN_NUMBER];
extern evs_property_dc_nonWork evs_property_dc_nonWorks[NET_SYSTEM_GUN_NUMBER];
extern evs_property_dc_input_meter evs_property_dc_input_meters[NET_SYSTEM_GUN_NUMBER];

/** 网络状态 */
enum{
    SGCC_SOCKET_STATE_PHY,
    SGCC_SOCKET_STATE_SIM,
    SGCC_SOCKET_STATE_DATA_LINK,
    SGCC_SOCKET_STATE_MODULE_INIT,
    SGCC_SOCKET_STATE_OPEN,
    SGCC_SOCKET_STATE_LOGIN_WAIT,
    SGCC_SOCKET_STATE_LOGIN_SUCCESS,
};

#pragma pack(1)

typedef struct{
     uint16_t length;
     uint8_t general_transmit_buff[NET_SGCC_GENERA_RESPONSE_BUFF_LENGTH];
}sgcc_response_message_buf_t;

typedef struct{
    int fd;
    uint8_t socket_state;
    uint8_t program_state;
    uint8_t sync_repeat;
    uint8_t domain_is_prase;   /** 域名已解析 */
    struct{
        uint16_t auth : 3;     /** 获取认证信息(三元组) */
        uint16_t open : 3;     /** 打开socket */
        uint16_t login  : 3;   /** 登录 */
        uint16_t sync : 1;     /** 类似心跳 */
        uint16_t reserve : 6;
    }operate_fail;
}sgcc_socket_info_t;

#pragma pack()

uint8_t sgcc_is_recved_billing_rule(void);
void sgcc_set_recv_billing_state(uint8_t state);
uint8_t sgcc_is_recved_timesync(void);

sgcc_socket_info_t* sgcc_get_socket_info(void);

uint8_t sgcc_transaction_is_verify(uint8_t gunno);
void sgcc_set_transaction_verify_state(uint8_t gunno, uint8_t state);

uint8_t sgcc_get_message_send_state(uint8_t gunno, uint32_t message_bit);
void sgcc_set_message_send_state(uint8_t gunno, uint8_t state, uint32_t message_bit);

int32_t sgcc_net_event_send(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint32_t event);
int32_t sgcc_net_event_receive(uint8_t event_handle, uint8_t event_type, uint8_t gunno, uint8_t option,
        uint32_t event, uint32_t* event_buf);

uint8_t sgcc_exist_message_wait_response(uint8_t gunno, uint32_t *state);
void sgcc_clear_message_wait_response_state(uint8_t gunno, uint32_t message_bit);
uint8_t sgcc_get_message_wait_response_timeout_state(uint8_t gunno, uint32_t timeout, uint32_t message_bit);
void sgcc_ascii_to_bcd(uint8_t *ascii, uint8_t alen, uint8_t *bcd, uint8_t blen, uint8_t is_order);

int sgcc_message_send_init(void);

#endif /* NET_PACK_USING_SGCC */
#endif /* NET_NET_SGCC_INC_SGCC_MESSAGE_SEND_H_ */
