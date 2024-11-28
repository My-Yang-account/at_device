/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#ifndef NET_NET_YKC_MONITOR_INC_YKC_MONITOR_FAULT_ANALYSE_H_
#define NET_NET_YKC_MONITOR_INC_YKC_MONITOR_FAULT_ANALYSE_H_

#include "net_operation.h"

#ifdef NET_PACK_USING_YKC_MONITOR

void ykc_monitor_fault_event_detect_callback(uint8_t gunno, uint32_t code, uint8_t is_resume);
void ykc_monitor_fault_detect_report(uint8_t gunno);

#endif /* NET_PACK_USING_YKC_MONITOR */

#endif /* NET_NET_YKC_MONITOR_INC_YKC_MONITOR_FAULT_ANALYSE_H_ */
