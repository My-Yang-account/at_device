/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_MESSAGE_PADDING_H_
#define NET_PACK_NET_THA_INC_THA_MESSAGE_PADDING_H_

#include "net_operation.h"

#ifdef NET_PACK_USING_THA

uint8_t tha_is_start_charge_success(uint8_t gunno);
uint8_t tha_is_stop_charge_success(uint8_t gunno);
uint8_t tha_is_set_power_success(uint8_t gunno);

int8_t tha_response_padding_general_message(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_time_sync(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_system_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_charge_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_system_info(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_billing_rule(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_charge_system_set(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_device_fault_info(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_battery_charge_info(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t tha_response_padding_query_order_state(uint8_t *buf, uint16_t ilen, uint16_t *olen);

int8_t tha_message_pro_update_billing_rule_request(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_set_reservation_request(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_cancel_reservation_request(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_stop_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_remote_update_request(void *data, uint8_t len);
int8_t tha_message_pro_unlock_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_authority_response(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_vin_authority_response(uint8_t gunno, void *data, uint8_t len);
int8_t tha_message_pro_set_power_request(uint8_t gunno, void *data, uint8_t len);

void tha_message_info_init(uint8_t gunno);

int8_t tha_chargepile_request_padding_card_authority(uint8_t gunno);
void tha_chargepile_request_padding_charge_data(uint8_t gunno, uint8_t is_init);
void tha_chargepile_request_padding_battery_state(uint8_t gunno, uint8_t is_init);
void tha_chargepile_request_padding_history_bill(uint8_t gunno, void *transaction);
void tha_chargepile_request_padding_transaction_bill(uint8_t gunno, void *transaction);
int8_t tha_chargepile_request_padding_vin_authority(uint8_t gunno);
void tha_chargepile_request_padding_net_info(uint8_t gunno);
void tha_chargepile_request_padding_battery_info(uint8_t gunno);
void tha_chargepile_request_padding_gun_realtime_temp(uint8_t gunno);

void tha_start_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void tha_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void tha_set_power_response_asynchronously(uint8_t gunno, uint8_t result);

void tha_chargepile_state_changed(uint8_t gunno);
void tha_chargepile_event_occurred(uint8_t gunno);
void tha_chargepile_update_result_report(uint8_t result, uint8_t gunno);
void tha_chargepile_fault_report(uint8_t gunno, char str[3], uint8_t rank, uint32_t timestamp, uint8_t is_resume);
void tha_chargepile_state_detect(uint8_t gunno);
int8_t tha_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);
uint8_t tha_chargepile_fault_converted(uint8_t bit);
void tha_chargepile_time_sync_revise(uint8_t gunno);
uint8_t tha_query_transaction_verify_state(uint8_t gunno);

void tha_data_realtime_process(uint8_t gunno);

#endif /* NET_PACK_USING_THA */
#endif /* NET_PACK_NET_THA_INC_THA_MESSAGE_PADDING_H_ */
