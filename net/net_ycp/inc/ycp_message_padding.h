/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_INC_YCP_MESSAGE_PADDING_H_
#define NET_NET_YCP_INC_YCP_MESSAGE_PADDING_H_

#include "ycp_message_struct_define.h"

#ifdef NET_PACK_USING_YCP

uint8_t ycp_is_start_charge_success(uint8_t gunno);
uint8_t ycp_is_stop_charge_success(uint8_t gunno);
uint8_t ycp_is_set_power_success(void);

int8_t ycp_response_padding_query_state_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_query_state_data_all(uint8_t *buf, uint16_t ilen, uint16_t *olen);

int8_t ycp_message_padding_device_fault(uint8_t *buf, uint16_t ilen, uint16_t *olen);

int8_t ycp_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_set_work_para(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_remote_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_set_billing_model(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_remote_update(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_qrcode_config(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_set_service_phone(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ycp_response_padding_modify_server_addr(uint8_t *buf, uint16_t ilen, uint16_t *olen);
void ycp_request_padding_heartbeat(void);

int8_t ycp_message_pro_billing_model_set_response(void *data, uint8_t len, uint8_t is_init);
int8_t ycp_message_pro_apply_charge_active_response(uint8_t gunno, void *data, uint8_t len);
int16_t ycp_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len);
int16_t ycp_message_pro_remote_stop_charge_request(uint8_t gunno);
int8_t ycp_message_pro_time_sync_response(void *data, uint8_t len);
int8_t ycp_message_pro_set_para_request(void *data, uint8_t len);
int8_t ycp_message_pro_remote_reset_request(void *data, uint8_t len);

void ycp_message_info_init(uint8_t gunno);

void ycp_chargepile_request_padding_state_data(uint8_t gunno, uint8_t is_init);
void ycp_chargepile_request_padding_bms_shakehand(uint8_t gunno);
void ycp_chargepile_request_padding_bms_paraconfig(uint8_t gunno);
void ycp_chargepile_request_padding_bms_chargeend(uint8_t gunno);
void ycp_chargepile_request_padding_bms_error(uint8_t gunno);
void ycp_chargepile_request_padding_bmsend_duringcharge(uint8_t gunno);
void ycp_chargepile_request_padding_chargerend_duringcharge(uint8_t gunno);
void ycp_chargepile_request_padding_bmscommand_chargerout(uint8_t gunno, uint8_t is_init);
void ycp_chargepile_request_padding_bmsinfo_duringcharge(uint8_t gunno, uint8_t is_init);
int8_t ycp_chargepile_request_padding_card_authority(uint8_t gunno);
int8_t ycp_chargepile_request_padding_vin_authority(uint8_t gunno);
uint8_t ycp_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat);

void ycp_start_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void ycp_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void ycp_set_power_percent_response_asynchronously(uint8_t result);

void ycp_chargepile_state_changed(uint8_t gunno);
void ycp_chargepile_update_result_report(uint8_t result, uint8_t reason);
int8_t  ycp_chargepile_fault_report(uint8_t gunno, uint16_t code, uint8_t is_resume, uint8_t rank);
int8_t ycp_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);
void ycp_chargepile_time_sync_revise(uint8_t gunno);
uint16_t ycp_chargepile_fault_converted(uint16_t bit);
uint8_t ycp_query_transaction_verify_state(uint8_t gunno);

#endif /* NET_PACK_USING_YCP */

#endif /* NET_NET_YCP_INC_YCP_MESSAGE_PADDING_H_ */
