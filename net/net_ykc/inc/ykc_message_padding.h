/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_MESSAGE_PADDING_H_
#define NET_PACK_NET_YKC_INC_YKC_MESSAGE_PADDING_H_

#include "ykc_message_struct_define.h"

#ifdef NET_PACK_USING_YKC

cp56time2a_t ykc_get_cp56time2a_from_timestamp(uint32_t timestamp);
uint32_t ykc_get_timestamp_from_cp56time2a(cp56time2a_t _cp56time2a);

uint8_t ykc_is_start_charge_success(uint8_t gunno);
uint8_t ykc_is_stop_charge_success(uint8_t gunno);
uint8_t ykc_is_start_mergecharge_success(uint8_t gunno);
uint8_t ykc_is_set_power_success(void);

int8_t ykc_response_padding_query_realtime_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_account_ballance_update(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_sync_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_clear_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_query_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_set_work_para(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_time_sync(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_remote_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_set_billing_model(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_ground_lock_lifting(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_remote_update(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_remote_start_merge_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_qrcode_config_gc(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_response_padding_qrcode_config_ykc15(uint8_t *buf, uint16_t ilen, uint16_t *olen);

int8_t ykc_message_pro_billing_model_set_response(void *data, uint8_t len);
int8_t ykc_message_pro_apply_charge_active_response(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_message_pro_remote_stop_charge_request(uint8_t gunno);
int8_t ykc_message_pro_account_ballance_update_request(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_message_pro_time_sync_request(void *data, uint8_t len);
int8_t ykc_message_pro_set_work_para_request(void *data, uint8_t len);
int8_t ykc_message_pro_apply_merge_charge_active_response(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_message_pro_remote_reset_request(void *data, uint8_t len);
int8_t ykc_message_pro_remote_start_merge_charge_request(uint8_t gunno, void *data, uint8_t len);

void ykc_message_info_init(uint8_t gunno);

void ykc_chargepile_request_padding_realtime_data(uint8_t gunno, uint8_t is_init);
void ykc_chargepile_request_padding_bms_shakehand(uint8_t gunno);
void ykc_chargepile_request_padding_bms_paraconfig(uint8_t gunno);
void ykc_chargepile_request_padding_bms_chargeend(uint8_t gunno);
void ykc_chargepile_request_padding_bms_error(uint8_t gunno);
void ykc_chargepile_request_padding_bmsend_duringcharge(uint8_t gunno);
void ykc_chargepile_request_padding_chargerend_duringcharge(uint8_t gunno);
void ykc_chargepile_request_padding_bmscommand_chargerout(uint8_t gunno, uint8_t is_init);
void ykc_chargepile_request_padding_bmsinfo_duringcharge(uint8_t gunno, uint8_t is_init);
int8_t ykc_chargepile_request_padding_card_authority(uint8_t gunno);
int8_t ykc_chargepile_request_padding_vin_authority(uint8_t gunno);
uint8_t ykc_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat);
int8_t ykc_chargepile_request_padding_mergecharge_card_authority(uint8_t gunno);
int8_t ykc_chargepile_request_padding_mergecharge_vin_authority(uint8_t gunno);

void ykc_start_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void ykc_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void ykc_start_mergecharge_response_asynchronously(uint8_t gunno, uint8_t result);
void ykc_set_power_percent_response_asynchronously(uint8_t result);

void ykc_chargepile_state_changed(uint8_t gunno);
void ykc_chargepile_update_result_report(uint8_t result);
void ykc_chargepile_fault_report(uint8_t gunno, uint16_t code);
int8_t ykc_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);
void ykc_chargepile_time_sync_revise(uint8_t gunno);
uint16_t ykc_chargepile_fault_converted(uint16_t bit);
uint8_t ykc_query_transaction_verify_state(uint8_t gunno);

#endif /* NET_PACK_USING_YKC */
#endif /* NET_PACK_NET_YKC_INC_YKC_MESSAGE_PADDING_H_ */
