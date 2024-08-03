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

void sgcc_chargepile_state_changed(uint8_t gunno);
void sgcc_chargepile_request_padding_state_data(uint8_t gunno, uint8_t is_init);

int8_t sgcc_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);

uint8_t sgcc_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat);

uint8_t sgcc_query_transaction_verify_state(uint8_t gunno);

void sgcc_message_info_init(void);
int sgcc_realtime_process_init(void);

#endif /* NET_NET_SGCC_INC_SGCC_MESSAGE_PADDING_H_ */
