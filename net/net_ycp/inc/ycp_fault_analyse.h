/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_INC_YCP_FAULT_ANALYSE_H_
#define NET_NET_YCP_INC_YCP_FAULT_ANALYSE_H_

#include "net_operation.h"

#ifdef NET_PACK_USING_YCP

void ycp_fault_event_detect_callback(uint8_t gunno, uint32_t code, uint8_t is_resume);
void ycp_fault_detect_report(uint8_t gunno);

#endif /* NET_PACK_USING_YCP */

#endif /* NET_NET_YCP_INC_YCP_FAULT_ANALYSE_H_ */
