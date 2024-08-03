/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */

#include "net_operation.h"

#include "sgcc_message_send.h"
#include "sgcc_message_receive.h"
#include "interface.h"

#define DBG_TAG "gw_callback"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

/**=======================================[服务器请求报文]=======================================*/
///设备配置信息
evs_data_dev_config evs_data_dev_configs[EVS_MAX_PORT_NUM];
///设备功能配置信息
evs_service_dev_fun_config evs_service_dev_fun_configs[EVS_MAX_PORT_NUM];
///设备部件配置信息
evs_service_config_parts evs_service_config_partss[EVS_MAX_PORT_NUM];
///设备部件配置查询
evs_service_parts_config_get evs_service_parts_config_gets[EVS_MAX_PORT_NUM];
///日志查询
evs_service_query_log evs_service_query_logs[EVS_MAX_PORT_NUM];
///维护
evs_service_dev_maintain evs_service_dev_maintains[EVS_MAX_PORT_NUM];
///电子锁
evs_service_lockCtrl evs_service_lockCtrls[EVS_MAX_PORT_NUM];
///计费模型
evs_service_issue_feeModel evs_service_issue_feeModels[EVS_MAX_PORT_NUM];
///开始充电
evs_service_startCharge evs_service_startCharges[EVS_MAX_PORT_NUM];
///鉴权
evs_service_authCharge evs_service_authCharges[EVS_MAX_PORT_NUM];
///停止充电
evs_service_stopCharge evs_service_stopCharges[EVS_MAX_PORT_NUM];
///交易记录获取
evs_service_trade_get evs_service_trade_gets[EVS_MAX_PORT_NUM];
///电表底值获取
evs_service_meter_get evs_service_meter_gets[EVS_MAX_PORT_NUM];
///交易记录确认
evs_service_confirmTrade evs_service_confirmTrades[EVS_MAX_PORT_NUM];
///VIN码白名单更新
evs_service_vinList_update evs_service_vinList_updates[EVS_MAX_PORT_NUM];
///预约
evs_service_rsvCharge evs_service_rsvCharges[EVS_MAX_PORT_NUM];
///地锁
evs_service_groundLock_ctrl evs_service_groundLock_ctrls[EVS_MAX_PORT_NUM];
///智能门锁
evs_service_gateLock_ctrl evs_service_gateLock_ctrls[EVS_MAX_PORT_NUM];
///有序充电
evs_service_orderCharge evs_service_orderCharges[EVS_MAX_PORT_NUM];
///设备功能配置查询
evs_service_get_dev_fun_config evs_service_get_dev_fun_configs[EVS_MAX_PORT_NUM];
///计费模型查询
evs_service_feeModel_query evs_service_feeModel_querys[EVS_MAX_PORT_NUM];
///蓝牙密钥更新
evs_service_blesecret_update evs_service_blesecret_updates[EVS_MAX_PORT_NUM];
///蓝牙信息
evs_service_ble_plug_charge_info_conf evs_service_ble_plug_charge_info_confs[EVS_MAX_PORT_NUM];
///蓝牙清空
evs_service_blelist_clean evs_service_blelist_cleans[EVS_MAX_PORT_NUM];

/**=======================================[服务器响应报文]=======================================*/
///设备维护指令结果
evs_event_feedback_dev_maintain evs_event_feedback_dev_maintains[EVS_MAX_PORT_NUM];
///电子锁控制结果
evs_event_feedback_lockCtrl evs_event_feedback_lockCtrls[EVS_MAX_PORT_NUM];
///日志查询
evs_service_feedback_query_log evs_service_feedback_query_logs[EVS_MAX_PORT_NUM];
///设备维护状态
evs_service_feedback_maintain_query evs_service_feedback_maintain_querys[EVS_MAX_PORT_NUM];
///计费模型更新
evs_service_feedback_feeModel evs_service_feedback_feeModels[EVS_MAX_PORT_NUM];
///启动充电
evs_service_feedback_startCharge evs_service_feedback_startCharges[EVS_MAX_PORT_NUM];
///鉴权充电
evs_service_feedback_authCharge evs_service_feedback_authCharges[EVS_MAX_PORT_NUM];
///停止充电
evs_service_feedback_stopCharge evs_service_feedback_stopCharges[EVS_MAX_PORT_NUM];
///设备部件配置参数
evs_service_feedback_config_parts evs_service_feedback_config_partss[EVS_MAX_PORT_NUM];
///设备部件配置获取
evs_service_feedback_config_parts_get evs_service_feedback_config_parts_gets[EVS_MAX_PORT_NUM];
///交易记录获取
evs_service_feedback_trade_get evs_service_feedback_trade_gets[EVS_MAX_PORT_NUM];
///电表底值
evs_service_feedback_meter_get evs_service_feedback_meter_gets[EVS_MAX_PORT_NUM];
///VIN码列表更新
evs_service_feedback_vinList_update evs_service_feedback_vinList_updates[EVS_MAX_PORT_NUM];
///预约结果
evs_service_feedback_rsvCharge evs_service_feedback_rsvCharges[EVS_MAX_PORT_NUM];
///地锁
evs_service_feedback_groundLock_ctrl evs_service_feedback_groundLock_ctrls[EVS_MAX_PORT_NUM];
///智能门锁
evs_service_feedback_gateLock_ctrl evs_service_feedback_gateLock_ctrls[EVS_MAX_PORT_NUM];
///有序充电
evs_service_feedback_orderCharge evs_service_feedback_orderCharges[EVS_MAX_PORT_NUM];
///设备功能配置查询
evs_service_feedback_dev_fun_config evs_service_feedback_dev_fun_configs[EVS_MAX_PORT_NUM];
///设备计费模型查询
evs_service_feedback_feeModel_qurey evs_service_feedback_feeModel_qureys[EVS_MAX_PORT_NUM];
///蓝牙密钥更新
evs_service_feedback_blesecret_update evs_service_feedback_blesecret_updates[EVS_MAX_PORT_NUM];
///蓝牙列表清除
evs_service_feedback_blelist_clean evs_service_feedback_blelist_cleans[EVS_MAX_PORT_NUM];
///蓝牙重置
evs_service_feedback_ble_reset evs_service_feedback_ble_resets[EVS_MAX_PORT_NUM];

int callback_service_EVS_DEV_MAINTAIN_SRV(evs_service_dev_maintain *request)
{
    return 0;
}

int callback_service_EVS_CTRL_LOCK_SRV(evs_service_lockCtrl *request)
{
    return 0;
}

int callback_service_EVS_FEE_MODEL_UPDATE_SRV(evs_service_issue_feeModel *request, evs_service_feedback_feeModel *feedback)
{
    LOG_D("gw fee model update-> id: %s", request->feeModelId);

    return 0;
}

int callback_service_EVS_START_CHARGE_SRV(evs_service_startCharge *request, evs_service_feedback_startCharge *feedback)
{
    LOG_D("gw remote start charge-> no: %d", request->gunNo);

    return 0;
}

int callback_service_EVS_AUTH_RESULT_SRV(evs_service_authCharge *request, evs_service_feedback_authCharge *feedback)
{
    return 0;
}

int callback_service_EVS_STOP_CHARGE_SRV(evs_service_stopCharge *request, evs_service_feedback_stopCharge *feedback)
{
    LOG_D("gw remote stop charge-> no: %d", request->gunNo);

    return 0;
}

int callback_service_EVS_ORDER_CHECK_SRV(evs_service_confirmTrade *request, void *feedback)
{
    return 0;
}

int callback_service_EVS_RSV_CHARGE_SRV(evs_service_rsvCharge *request, evs_service_feedback_rsvCharge *feedback)
{
    return 0;
}

int callback_service_EVS_GROUND_LOCK_SRV(evs_service_groundLock_ctrl *request, evs_service_feedback_groundLock_ctrl *feedback)
{
    return 0;
}

int callback_service_EVS_GATE_LOCK_SRV(evs_service_gateLock_ctrl *request, evs_service_feedback_gateLock_ctrl *feedback)
{
    return 0;
}

int callback_service_EVS_CONF_UPDATE_SRV(evs_data_dev_config *request, int *feedback)
{
    return 0;
}

int callback_service_EVS_CONF_GET_SRV(evs_data_dev_config *feedback)
{
    return 0;
}

int callback_service_EVS_MAINTAIN_RESULT_SRV(evs_service_feedback_maintain_query *feedback)
{
    return 0;
}

int callback_service_EVS_FUN_CONF_UPDATE_SRV(evs_service_dev_fun_config *request, evs_service_feedback_dev_fun_config *feedback)
{
    return 0;
}

int callback_service_EVS_FUN_CONF_GET_SRV(evs_service_get_dev_fun_config *request, evs_service_dev_fun_config *feedback)
{
    return 0;
}

int callback_service_EVS_FEE_MODEL_QUERY_SRV(evs_service_feedback_feeModel_qurey *feedback)
{
    return 0;
}

int callback_service_EVS_BLE_SECRET_UPDATE_SRV(evs_service_blesecret_update *request, evs_service_feedback_blesecret_update *feedback)
{
    return 0;
}

int callback_service_EVS_BLE_PLUG_CHG_INFO_GET_SRV(evs_event_ble_plug_charge_info *feedback)
{
    return 0;
}

int callback_service_EVS_BLE_PLUG_CHG_INFO_CONF_SRV(evs_service_ble_plug_charge_info_conf *request)
{
    return 0;
}

int callback_service_EVS_BLE_AUTH_LIST_CLEAN_SRV(evs_service_blelist_clean *request, evs_service_feedback_blelist_clean *feedback)
{
    return 0;
}

int callback_service_EVS_BLE_RESET_SRV(evs_service_feedback_ble_reset *feedback)
{
    return 0;
}

int callback_service_EVS_PILE_PARTS_CONF_SRV(evs_service_config_parts *request, evs_service_feedback_config_parts *feedback)
{
    return 0;
}

int callback_service_EVS_PILE_PARTS_CONF_GET_SRV(evs_service_parts_config_get *request, evs_service_feedback_config_parts_get *feedback)
{
    return 0;
}

int callback_service_EVS_VIN_LIST_UPDATE_SRV(evs_service_vinList_update *request, evs_service_feedback_vinList_update *feedback)
{
    return 0;
}

int callback_service_EVS_ORDER_ASK_SRV(evs_service_trade_get *request, evs_service_feedback_trade_get *feedback)
{
    return 0;
}

int callback_service_EVS_Meter_ASK_SRV(evs_service_meter_get *request, evs_service_feedback_meter_get *feedback)
{
    return 0;
}

int callback_service_EVS_QUE_DATA_SRV(evs_service_query_log *request, evs_service_feedback_query_log *feedback)
{
    return 0;
}

int callback_service_EVS_ORDERLY_CHARGE_SRV(evs_service_orderCharge *request, evs_service_feedback_orderCharge *feedback)
{
    return 0;
}

int callback_service_EVS_OTA_UPDATE(const char *request)
{
    return 0;
}

int callback_service_EVS_TIME_SYNC(const unsigned int request)
{
    return 0;
}

int callback_service_EVS_CONNECT_SUCC(void)
{
    return 0;
}

int callback_service_EVS_DISCONNECTED(void)
{
    return 0;
}

int callback_service_EVS_REPORT_REPLY(const int msgid, const int code, const char *reply, const int reply_len)
{
    return 0;
}

int callback_service_EVS_TRIGGER_EVENT_REPLY(const int msgid, const int code, const char *eventid, const int eventid_len, const char *message, const int message_len)
{
    return 0;
}

int callback_service_EVS_STATE_EVERYTHING(int ev, const char *msg)
{
    return 0;
}

int sgcc_message_recvive_init(void)
{
    EVS_RegisterCallback(EVS_DEV_MAINTAIN_SRV, callback_service_EVS_DEV_MAINTAIN_SRV);
    EVS_RegisterCallback(EVS_CTRL_LOCK_SRV, callback_service_EVS_CTRL_LOCK_SRV);
    EVS_RegisterCallback(EVS_FEE_MODEL_UPDATE_SRV, callback_service_EVS_FEE_MODEL_UPDATE_SRV);
    EVS_RegisterCallback(EVS_START_CHARGE_SRV, callback_service_EVS_START_CHARGE_SRV);
    EVS_RegisterCallback(EVS_AUTH_RESULT_SRV, callback_service_EVS_AUTH_RESULT_SRV);
    EVS_RegisterCallback(EVS_STOP_CHARGE_SRV, callback_service_EVS_STOP_CHARGE_SRV);
    EVS_RegisterCallback(EVS_ORDER_CHECK_SRV, callback_service_EVS_ORDER_CHECK_SRV);
    EVS_RegisterCallback(EVS_RSV_CHARGE_SRV, callback_service_EVS_RSV_CHARGE_SRV);
    EVS_RegisterCallback(EVS_GROUND_LOCK_SRV, callback_service_EVS_GROUND_LOCK_SRV);
    EVS_RegisterCallback(EVS_GATE_LOCK_SRV, callback_service_EVS_GATE_LOCK_SRV);
    EVS_RegisterCallback(EVS_CONF_UPDATE_SRV, callback_service_EVS_CONF_UPDATE_SRV);
    EVS_RegisterCallback(EVS_CONF_GET_SRV, callback_service_EVS_CONF_GET_SRV);
    EVS_RegisterCallback(EVS_MAINTAIN_RESULT_SRV, callback_service_EVS_MAINTAIN_RESULT_SRV);
    EVS_RegisterCallback(EVS_FUN_CONF_UPDATE_SRV, callback_service_EVS_FUN_CONF_UPDATE_SRV);
    EVS_RegisterCallback(EVS_FUN_CONF_GET_SRV, callback_service_EVS_FUN_CONF_GET_SRV);
    EVS_RegisterCallback(EVS_FEE_MODEL_QUERY_SRV, callback_service_EVS_FEE_MODEL_QUERY_SRV);
    EVS_RegisterCallback(EVS_BLE_SECRET_UPDATE_SRV, callback_service_EVS_BLE_SECRET_UPDATE_SRV);
    EVS_RegisterCallback(EVS_BLE_PLUG_CHG_INFO_GET_SRV, callback_service_EVS_BLE_PLUG_CHG_INFO_GET_SRV);
    EVS_RegisterCallback(EVS_BLE_PLUG_CHG_INFO_CONF_SRV, callback_service_EVS_BLE_PLUG_CHG_INFO_CONF_SRV);
    EVS_RegisterCallback(EVS_BLE_AUTH_LIST_CLEAN_SRV, callback_service_EVS_BLE_AUTH_LIST_CLEAN_SRV);
    EVS_RegisterCallback(EVS_BLE_RESET_SRV, callback_service_EVS_BLE_RESET_SRV);
    EVS_RegisterCallback(EVS_PILE_PARTS_CONF_SRV, callback_service_EVS_PILE_PARTS_CONF_SRV);
    EVS_RegisterCallback(EVS_PILE_PARTS_CONF_GET_SRV, callback_service_EVS_PILE_PARTS_CONF_GET_SRV);
    EVS_RegisterCallback(EVS_VIN_LIST_UPDATE_SRV, callback_service_EVS_VIN_LIST_UPDATE_SRV);
    EVS_RegisterCallback(EVS_ORDER_ASK_SRV, callback_service_EVS_ORDER_ASK_SRV);
    EVS_RegisterCallback(EVS_Meter_ASK_SRV, callback_service_EVS_Meter_ASK_SRV);
    EVS_RegisterCallback(EVS_QUE_DATA_SRV, callback_service_EVS_QUE_DATA_SRV);
    EVS_RegisterCallback(EVS_ORDERLY_CHARGE_SRV, callback_service_EVS_ORDERLY_CHARGE_SRV);
    EVS_RegisterCallback(EVS_OTA_UPDATE, callback_service_EVS_OTA_UPDATE);
    EVS_RegisterCallback(EVS_TIME_SYNC, callback_service_EVS_TIME_SYNC);
    EVS_RegisterCallback(EVS_CONNECT_SUCC, callback_service_EVS_CONNECT_SUCC);
    EVS_RegisterCallback(EVS_DISCONNECTED, callback_service_EVS_DISCONNECTED);
    EVS_RegisterCallback(EVS_REPORT_REPLY, callback_service_EVS_REPORT_REPLY);
    EVS_RegisterCallback(EVS_TRIGGER_EVENT_REPLY, callback_service_EVS_TRIGGER_EVENT_REPLY);
    EVS_RegisterCallback(EVS_STATE_EVERYTHING, callback_service_EVS_STATE_EVERYTHING);

    return 0x00;
}

#endif /* NET_PACK_USING_SGCC */
