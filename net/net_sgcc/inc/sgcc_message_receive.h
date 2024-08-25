/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */
#ifndef NET_NET_SGCC_INC_SGCC_MESSAGE_RECEIVE_H_
#define NET_NET_SGCC_INC_SGCC_MESSAGE_RECEIVE_H_

#include "protocol.h"

#define NET_SGCC_RECV_MESSAGE_ITEM_MAX      0x03       /** 处理结果项最大值 */
#define NET_SGCC_FIRMWARE_VERSION_LEN       0x08       /* 固件版本长度 */
/** server response event */
#define NET_SGCC_SRES_EVENT_BILL_VERIFY                         0    /* 服务器响应事件：订单确认 */
#define NET_SGCC_SRES_EVENT_APPLY_CHARGE_ACTIVE                 1    /* 服务器响应事件：充电桩主动申请启动充电响应 */

//#define NET_YKC_SRES_EVENT_LOGIN                               0    /* 服务器响应事件：登录响应 */
//#define NET_YKC_SRES_EVENT_HEARTBEAT                           1    /* 服务器响应事件：心跳响应 */
//#define NET_YKC_SRES_EVENT_BILLING_MODEL_VERIFY                2    /* 服务器响应事件：计费模型验证请求响应 */
//#define NET_YKC_SRES_EVENT_BILLING_MODEL_REQUEST               3    /* 服务器响应事件：计费模型请求响应 */
//#define NET_YKC_SRES_EVENT_APPLY_CHARGE_ACTIVE                 5    /* 服务器响应事件：充电桩主动申请启动充电响应 */
//#define NET_YKC_SRES_EVENT_APPLY_MERGE_CHARGE_ACTIVE           6    /* 服务器响应事件： 停止完成上报订单响应 */

#define NET_SGCC_SERVER_SRES_NUM                                7    /* 服务器响应事件总数 */

/** server request event */
#define NET_SGCC_SREQ_EVENT_BILLING_MODEL_SET                  0    /* 服务器请求事件：计费模型设置*/
#define NET_SGCC_SREQ_EVENT_START_CHARGE                       1    /* 服务器请求事件：开始充电 */
#define NET_SGCC_SREQ_EVENT_STOP_CHARGE                        2    /* 服务器请求事件：停止充电 */
#define NET_SGCC_SREQ_EVENT_FIRMWARE_UPDATE                    3    /* 服务器请求事件：固件更新 */
#define NET_SGCC_SREQ_EVENT_SET_DEV_MAINTAIN                   4    /* 服务器请求事件：设备维护 */
#define NET_SGCC_SREQ_EVENT_QUERY_DEV_MAINTAIN                 5    /* 服务器请求事件：查询设备维护信息 */
#define NET_SGCC_SREQ_EVENT_TIME_SYNC                          6    /* 服务器请求事件：时间同步 */
#define NET_SGCC_SREQ_EVENT_QUERY_DEV_RECORD                   7    /* 服务器请求事件：查询设备记录 */
//#define NET_SGCC_SREQ_EVENT_ELOCK_CONTROL                      4    /* 服务器请求事件：电子锁控制 */

//#define NET_YKC_SREQ_EVENT_REMOTE_START_CHARGE                 1    /* 服务器请求事件：运营平台远程控制启机*/
//#define NET_YKC_SREQ_EVENT_REMOTE_STOP_CHARGE                  2    /* 服务器请求事件：运营平台远程停机*/
//#define NET_YKC_SREQ_EVENT_ACCOUNT_BALLANCE_UPDATE             3    /* 服务器请求事件：远程账户余额更新*/
//#define NET_YKC_SREQ_EVENT_SYNC_OFFLINE_CARD                   4    /* 服务器请求事件：离线卡数据同步*/
//#define NET_YKC_SREQ_EVENT_CLEAR_OFFLINE_CARD                  5    /* 服务器请求事件：离线卡数据清除*/
//#define NET_YKC_SREQ_EVENT_QUERY_OFFLINE_CARD                  6    /* 服务器请求事件：离线卡数据查询*/
//#define NET_YKC_SREQ_EVENT_SET_WORK_PARA                       7    /* 服务器请求事件：充电桩工作参数设置*/
//#define NET_YKC_SREQ_EVENT_TIME_SYNC                           8    /* 服务器请求事件：对时设置*/
//#define NET_YKC_SREQ_EVENT_GROUND_LOCK_LIFTING                 10   /* 服务器请求事件：遥控地锁升锁与降锁命令*/
//#define NET_YKC_SREQ_EVENT_REMOTE_REBOOT                       11   /* 服务器请求事件：远程重启*/
//#define NET_YKC_SREQ_EVENT_REMOTE_UPDATE                       12   /* 服务器请求事件：远程更新*/
//#define NET_YKC_SREQ_EVENT_REMOTE_START_MERGE_CHARGE           13   /* 服务器请求事件：运营平台远程控制并充启机*/
//#define NET_YKC_SREQ_EVENT_QRCODE_CONFIG_GC                    14   /* 服务器请求事件：二维码配置(国充)*/
//#define NET_YKC_SREQ_EVENT_QRCODE_CONFIG_YKC15                 15   /* 服务器请求事件：二维码配置(云快充1.5)*/

#define NET_SGCC_SERVER_SREQ_NUM                                16   /* 服务器请求事件总数 */

/**=======================================[服务器请求报文]=======================================*/
extern evs_data_dev_config evs_data_dev_configs;
//extern evs_service_dev_fun_config evs_service_dev_fun_configs[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_config_parts evs_service_config_partss[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_parts_config_get evs_service_parts_config_gets[NET_SYSTEM_GUN_NUMBER];
extern evs_service_query_log evs_service_query_logs[NET_SYSTEM_GUN_NUMBER];
extern evs_service_dev_maintain evs_service_dev_maintains[NET_SYSTEM_GUN_NUMBER];
extern evs_service_lockCtrl evs_service_lockCtrls[NET_SYSTEM_GUN_NUMBER];
extern evs_service_issue_feeModel evs_service_issue_feeModels[NET_SYSTEM_GUN_NUMBER];
extern evs_service_startCharge evs_service_startCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_service_authCharge evs_service_authCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_service_stopCharge evs_service_stopCharges[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_trade_get evs_service_trade_gets[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_meter_get evs_service_meter_gets[NET_SYSTEM_GUN_NUMBER];
extern evs_service_confirmTrade evs_service_confirmTrades[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_vinList_update evs_service_vinList_updates[NET_SYSTEM_GUN_NUMBER];
extern evs_service_rsvCharge evs_service_rsvCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_service_groundLock_ctrl evs_service_groundLock_ctrls[NET_SYSTEM_GUN_NUMBER];
extern evs_service_gateLock_ctrl evs_service_gateLock_ctrls[NET_SYSTEM_GUN_NUMBER];
extern evs_service_orderCharge evs_service_orderCharges[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_get_dev_fun_config evs_service_get_dev_fun_configs[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feeModel_query evs_service_feeModel_querys[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_blesecret_update evs_service_blesecret_updates[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_ble_plug_charge_info_conf evs_service_ble_plug_charge_info_confs[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_blelist_clean evs_service_blelist_cleans[NET_SYSTEM_GUN_NUMBER];

/**=======================================[服务器响应报文]=======================================*/
//extern evs_event_feedback_dev_maintain evs_event_feedback_dev_maintains[NET_SYSTEM_GUN_NUMBER];
//extern evs_event_feedback_lockCtrl evs_event_feedback_lockCtrls[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_query_log evs_service_feedback_query_logs[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_maintain_query evs_service_feedback_maintain_querys[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_feeModel evs_service_feedback_feeModels[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_startCharge evs_service_feedback_startCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_authCharge evs_service_feedback_authCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_stopCharge evs_service_feedback_stopCharges[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_config_parts evs_service_feedback_config_partss[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_config_parts_get evs_service_feedback_config_parts_gets[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_trade_get evs_service_feedback_trade_gets[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_meter_get evs_service_feedback_meter_gets[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_vinList_update evs_service_feedback_vinList_updates[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_rsvCharge evs_service_feedback_rsvCharges[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_groundLock_ctrl evs_service_feedback_groundLock_ctrls[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_gateLock_ctrl evs_service_feedback_gateLock_ctrls[NET_SYSTEM_GUN_NUMBER];
extern evs_service_feedback_orderCharge evs_service_feedback_orderCharges[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_dev_fun_config evs_service_feedback_dev_fun_configs[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_feeModel_qurey evs_service_feedback_feeModel_qureys[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_blesecret_update evs_service_feedback_blesecret_updates[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_blelist_clean evs_service_feedback_blelist_cleans[NET_SYSTEM_GUN_NUMBER];
//extern evs_service_feedback_ble_reset evs_service_feedback_ble_resets[NET_SYSTEM_GUN_NUMBER];

#pragma pack(1)

typedef struct{
    uint8_t event;
    uint8_t gunno;
    uint8_t result;
}sgcc_recv_item;

#pragma pack()

uint8_t *sgcc_get_firmware_pack_version(void);
void sgcc_clear_recv_message_item(uint8_t event, uint8_t gunno, uint8_t res);
int16_t sgcc_get_recv_message_item_result(uint8_t event, uint8_t gunno, uint8_t *res);

int sgcc_message_recvive_init(void);

#endif /* NET_NET_SGCC_INC_SGCC_MESSAGE_RECEIVE_H_ */
