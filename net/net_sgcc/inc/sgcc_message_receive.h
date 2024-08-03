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

/**=======================================[服务器请求报文]=======================================*/
extern evs_data_dev_config evs_data_dev_configs[EVS_MAX_PORT_NUM];
extern evs_service_dev_fun_config evs_service_dev_fun_configs[EVS_MAX_PORT_NUM];
extern evs_service_config_parts evs_service_config_partss[EVS_MAX_PORT_NUM];
extern evs_service_parts_config_get evs_service_parts_config_gets[EVS_MAX_PORT_NUM];
extern evs_service_query_log evs_service_query_logs[EVS_MAX_PORT_NUM];
extern evs_service_dev_maintain evs_service_dev_maintains[EVS_MAX_PORT_NUM];
extern evs_service_lockCtrl evs_service_lockCtrls[EVS_MAX_PORT_NUM];
extern evs_service_issue_feeModel evs_service_issue_feeModels[EVS_MAX_PORT_NUM];
extern evs_service_startCharge evs_service_startCharges[EVS_MAX_PORT_NUM];
extern evs_service_authCharge evs_service_authCharges[EVS_MAX_PORT_NUM];
extern evs_service_stopCharge evs_service_stopCharges[EVS_MAX_PORT_NUM];
extern evs_service_trade_get evs_service_trade_gets[EVS_MAX_PORT_NUM];
extern evs_service_meter_get evs_service_meter_gets[EVS_MAX_PORT_NUM];
extern evs_service_confirmTrade evs_service_confirmTrades[EVS_MAX_PORT_NUM];
extern evs_service_vinList_update evs_service_vinList_updates[EVS_MAX_PORT_NUM];
extern evs_service_rsvCharge evs_service_rsvCharges[EVS_MAX_PORT_NUM];
extern evs_service_groundLock_ctrl evs_service_groundLock_ctrls[EVS_MAX_PORT_NUM];
extern evs_service_gateLock_ctrl evs_service_gateLock_ctrls[EVS_MAX_PORT_NUM];
extern evs_service_orderCharge evs_service_orderCharges[EVS_MAX_PORT_NUM];
extern evs_service_get_dev_fun_config evs_service_get_dev_fun_configs[EVS_MAX_PORT_NUM];
extern evs_service_feeModel_query evs_service_feeModel_querys[EVS_MAX_PORT_NUM];
extern evs_service_blesecret_update evs_service_blesecret_updates[EVS_MAX_PORT_NUM];
extern evs_service_ble_plug_charge_info_conf evs_service_ble_plug_charge_info_confs[EVS_MAX_PORT_NUM];
extern evs_service_blelist_clean evs_service_blelist_cleans[EVS_MAX_PORT_NUM];

/**=======================================[服务器响应报文]=======================================*/
extern evs_event_feedback_dev_maintain evs_event_feedback_dev_maintains[EVS_MAX_PORT_NUM];
extern evs_event_feedback_lockCtrl evs_event_feedback_lockCtrls[EVS_MAX_PORT_NUM];
extern evs_service_feedback_query_log evs_service_feedback_query_logs[EVS_MAX_PORT_NUM];
extern evs_service_feedback_maintain_query evs_service_feedback_maintain_querys[EVS_MAX_PORT_NUM];
extern evs_service_feedback_feeModel evs_service_feedback_feeModels[EVS_MAX_PORT_NUM];
extern evs_service_feedback_startCharge evs_service_feedback_startCharges[EVS_MAX_PORT_NUM];
extern evs_service_feedback_authCharge evs_service_feedback_authCharges[EVS_MAX_PORT_NUM];
extern evs_service_feedback_stopCharge evs_service_feedback_stopCharges[EVS_MAX_PORT_NUM];
extern evs_service_feedback_config_parts evs_service_feedback_config_partss[EVS_MAX_PORT_NUM];
extern evs_service_feedback_config_parts_get evs_service_feedback_config_parts_gets[EVS_MAX_PORT_NUM];
extern evs_service_feedback_trade_get evs_service_feedback_trade_gets[EVS_MAX_PORT_NUM];
extern evs_service_feedback_meter_get evs_service_feedback_meter_gets[EVS_MAX_PORT_NUM];
extern evs_service_feedback_vinList_update evs_service_feedback_vinList_updates[EVS_MAX_PORT_NUM];
extern evs_service_feedback_rsvCharge evs_service_feedback_rsvCharges[EVS_MAX_PORT_NUM];
extern evs_service_feedback_groundLock_ctrl evs_service_feedback_groundLock_ctrls[EVS_MAX_PORT_NUM];
extern evs_service_feedback_gateLock_ctrl evs_service_feedback_gateLock_ctrls[EVS_MAX_PORT_NUM];
extern evs_service_feedback_orderCharge evs_service_feedback_orderCharges[EVS_MAX_PORT_NUM];
extern evs_service_feedback_dev_fun_config evs_service_feedback_dev_fun_configs[EVS_MAX_PORT_NUM];
extern evs_service_feedback_feeModel_qurey evs_service_feedback_feeModel_qureys[EVS_MAX_PORT_NUM];
extern evs_service_feedback_blesecret_update evs_service_feedback_blesecret_updates[EVS_MAX_PORT_NUM];
extern evs_service_feedback_blelist_clean evs_service_feedback_blelist_cleans[EVS_MAX_PORT_NUM];
extern evs_service_feedback_ble_reset evs_service_feedback_ble_resets[EVS_MAX_PORT_NUM];

int sgcc_message_recvive_init(void);

#endif /* NET_NET_SGCC_INC_SGCC_MESSAGE_RECEIVE_H_ */
