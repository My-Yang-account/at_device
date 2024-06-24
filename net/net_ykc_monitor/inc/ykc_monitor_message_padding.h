/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#ifndef NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_PADDING_H_
#define NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_PADDING_H_

#include "ykc_monitor_message_struct_define.h"

#ifdef NET_PACK_USING_YKC_MONITOR

cp56time2a_monitor_t ykc_monitor_get_cp56time2a_from_timestamp(uint32_t timestamp);
uint32_t ykc_monitor_get_timestamp_from_cp56time2a(cp56time2a_monitor_t _cp56time2a);

uint8_t ykc_monitor_is_start_charge_success(uint8_t gunno);
uint8_t ykc_monitor_is_stop_charge_success(uint8_t gunno);
uint8_t ykc_monitor_is_start_mergecharge_success(uint8_t gunno);
uint8_t ykc_monitor_is_set_power_success(void);

int8_t ykc_monitor_response_padding_query_realtime_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_account_ballance_update(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_sync_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_clear_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_query_offline_card(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_set_work_para(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_time_sync(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_remote_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_set_billing_model(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_ground_lock_lifting(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_remote_update(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_remote_start_merge_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_response_padding_qrcode_config(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);

int8_t ykc_monitor_message_pro_billing_model_set_response(void *data, uint8_t len);
int8_t ykc_monitor_message_pro_apply_charge_active_response(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_monitor_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_monitor_message_pro_remote_stop_charge_request(uint8_t gunno);
int8_t ykc_monitor_message_pro_account_ballance_update_request(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_monitor_message_pro_time_sync_request(void *data, uint8_t len);
int8_t ykc_monitor_message_pro_set_work_para_request(void *data, uint8_t len);
int8_t ykc_monitor_message_pro_apply_merge_charge_active_response(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_monitor_message_pro_remote_reset_request(void *data, uint8_t len);
int8_t ykc_monitor_message_pro_remote_start_merge_charge_request(uint8_t gunno, void *data, uint8_t len);
int8_t ykc_monitor_message_pro_qrcode_config_request(uint8_t gunno);

void ykc_monitor_message_info_init(void);

void ykc_monitor_chargepile_request_padding_realtime_data(uint8_t gunno, uint8_t is_init);
void ykc_monitor_chargepile_request_padding_bms_shakehand(uint8_t gunno);
void ykc_monitor_chargepile_request_padding_bms_paraconfig(uint8_t gunno);
void ykc_monitor_chargepile_request_padding_bms_chargeend(uint8_t gunno);
void ykc_monitor_chargepile_request_padding_bms_error(uint8_t gunno);
void ykc_monitor_chargepile_request_padding_bmsend_duringcharge(uint8_t gunno);
void ykc_monitor_chargepile_request_padding_chargerend_duringcharge(uint8_t gunno);
void ykc_monitor_chargepile_request_padding_bmscommand_chargerout(uint8_t gunno, uint8_t is_init);
void ykc_monitor_chargepile_request_padding_bmsinfo_duringcharge(uint8_t gunno, uint8_t is_init);
int8_t ykc_monitor_chargepile_request_padding_card_authority(uint8_t gunno);
int8_t ykc_monitor_chargepile_request_padding_vin_authority(uint8_t gunno);
uint8_t ykc_monitor_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat);
int8_t ykc_monitor_chargepile_request_padding_mergecharge_card_authority(uint8_t gunno);
int8_t ykc_monitor_chargepile_request_padding_mergecharge_vin_authority(uint8_t gunno);

void ykc_monitor_start_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void ykc_monitor_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result);
void ykc_monitor_start_mergecharge_response_asynchronously(uint8_t gunno, uint8_t result);
void ykc_monitor_set_power_percent_response_asynchronously(uint8_t result);

void ykc_monitor_chargepile_state_changed(uint8_t gunno);
void ykc_monitor_chargepile_update_result_report(uint8_t result);
void ykc_monitor_chargepile_fault_report(uint8_t gunno, uint16_t code);
void ykc_monitor_chargepile_state_detect(uint8_t gunno);
int8_t ykc_monitor_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);
void ykc_monitor_chargepile_time_sync_revise(uint8_t gunno);
uint8_t ykc_monitor_chargepile_fault_converted(uint8_t bit);
uint8_t ykc_monitor_query_transaction_verify_state(uint8_t gunno);
void ykc_monitor_data_realtime_process(uint8_t gunno);
void ykc_monitor_disposable_message_check(uint8_t gunno);


/******************************** 以下是监控报文 *******************************/
/******************************** 以下是监控报文 *******************************/
int8_t ykc_monitor_message_padding_module_info(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_message_padding_module_setup_info(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_message_padding_fault_record(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_message_padding_charge_record(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_message_padding_input_info_setup(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_message_padding_protect_info_setup(uint8_t *buf, uint16_t ilen, uint16_t *olen);
int8_t ykc_monitor_message_padding_function_setup(uint8_t *buf, uint16_t ilen, uint16_t *olen);

#endif /* NET_PACK_USING_YKC_MONITOR */

#endif /* NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_PADDING_H_ */
