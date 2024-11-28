/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_FAULT_ANALYSE_H_
#define NET_PACK_NET_YKC_INC_YKC_FAULT_ANALYSE_H_

#include "net_operation.h"

#ifdef NET_PACK_USING_YKC

void ykc_fault_event_detect_callback(uint8_t gunno, uint32_t code, uint8_t is_resume);
void ykc_fault_detect_report(uint8_t gunno);

#endif /* NET_PACK_USING_YKC */
#endif /* NET_PACK_NET_YKC_INC_YKC_FAULT_ANALYSE_H_ */
