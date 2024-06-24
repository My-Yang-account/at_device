/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-15     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_FAULT_ANALYSE_H_
#define NET_PACK_NET_THA_INC_THA_FAULT_ANALYSE_H_

#include "net_operation.h"

#ifdef NET_PACK_USING_THA

void tha_fault_event_detect_callback(uint8_t gunno, uint8_t code, uint32_t timestamp, uint8_t is_resume);
uint32_t tha_get_current_fault_set(uint8_t gunno);
void tha_fault_detect_report(uint8_t gunno);

#endif /* NET_PACK_USING_THA */
#endif /* NET_PACK_NET_THA_INC_THA_FAULT_ANALYSE_H_ */
