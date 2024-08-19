/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     leven       the first version
 */
#ifndef NET_NET_SGCC_INC_SGCC_FAULT_ANALYSE_H_
#define NET_NET_SGCC_INC_SGCC_FAULT_ANALYSE_H_

#include "net_operation.h"

#ifdef NET_PACK_USING_SGCC

void sgcc_fault_event_detect_callback(uint8_t gunno, uint8_t code, uint8_t is_resume);
uint32_t sgcc_get_current_fault_set(uint8_t gunno);
void sgcc_fault_detect_report(uint8_t gunno);

#endif /* NET_PACK_USING_SGCC */

#endif /* NET_NET_SGCC_INC_SGCC_FAULT_ANALYSE_H_ */
