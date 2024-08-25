/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */
#ifndef NET_NET_SGCC_INC_SGCC_MESSAGE_PADDING_H_
#define NET_NET_SGCC_INC_SGCC_MESSAGE_PADDING_H_

#include "net_pack_config.h"

uint8_t sgcc_get_applycharge_result(uint8_t gunno);
uint16_t sgcc_get_applycharge_fault_reason(uint8_t gunno);
uint16_t sgcc_get_applycharge_fail_reason(uint8_t gunno);

uint8_t sgcc_get_remotecharge_result(uint8_t gunno);
uint16_t sgcc_get_remotecharge_fault_reason(uint8_t gunno);
uint16_t sgcc_get_remotecharge_fail_reason(uint8_t gunno);

uint8_t sgcc_get_remotestop_result(uint8_t gunno);
uint16_t sgcc_get_remotestop_fault_reason(uint8_t gunno);
uint16_t sgcc_get_remotestop_fail_reason(uint8_t gunno);

int8_t sgcc_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t sgcc_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t sgcc_response_padding_apply_charge_result(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t sgcc_response_padding_elock_ctrl_result(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);

int8_t sgcc_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t sgcc_message_pro_remote_stop_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t sgcc_message_pro_apply_charge_response(uint8_t gunno, void *data, uint8_t len);
int8_t sgcc_message_pro_billing_model_request_response(void *data, uint8_t len);
int8_t sgcc_message_pro_config_update_request(void *data, uint16_t len);
int8_t sgcc_message_pro_config_query_request(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t sgcc_message_pro_ctrl_elock_request(uint8_t gunno, void *data, uint8_t len);
int8_t sgcc_message_pro_dev_maintain_request(void *data, uint8_t len);
int8_t sgcc_message_pro_query_maintain_info_request(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t sgcc_message_pro_time_sync_request(void *data, uint8_t len);
int8_t sgcc_message_pro_reservation_request(uint8_t gunno, void *data, uint8_t len);
int8_t sgcc_message_pro_orderly_charge_request(void *data, uint8_t len);
int8_t sgcc_message_pro_query_dev_record_request(uint8_t gunno, void *data, uint8_t len);

void sgcc_start_charge_response_asynchronously(uint8_t gunno, uint8_t result, uint16_t reason, uint16_t fault);
void sgcc_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result, uint16_t reason, uint16_t fault);
void sgcc_chargepile_state_changed(uint8_t gunno);
void sgcc_chargepile_request_padding_state_data(uint8_t gunno, uint8_t is_init);

int8_t sgcc_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);

void sgcc_chargepile_request_padding_out_ammeter_val(uint8_t gunno, uint8_t init);
void sgcc_chargepile_request_padding_bms_data(uint8_t gunno, uint8_t init);
void sgcc_chargepile_request_padding_monitor_property(uint8_t gunno);
int8_t sgcc_chargepile_request_padding_card_authority(uint8_t gunno);
int8_t sgcc_chargepile_request_padding_vin_authority(uint8_t gunno);

uint8_t sgcc_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat);

uint16_t sgcc_chargepile_fault_converted(uint16_t bit);
int8_t sgcc_chargepile_fault_report(uint8_t gunno, uint16_t code, uint8_t is_resume, uint8_t rank);
uint8_t sgcc_query_transaction_verify_state(uint8_t gunno);

void sgcc_message_info_init(uint8_t gunno);
int sgcc_realtime_process_init(void);

#endif /* NET_NET_SGCC_INC_SGCC_MESSAGE_PADDING_H_ */
