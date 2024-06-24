/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#ifndef NET_NET_YKC_MONITOR_INC_YKC_MONITOR_OTA_H_
#define NET_NET_YKC_MONITOR_INC_YKC_MONITOR_OTA_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YKC_MONITOR
#ifdef NET_INCLUDE_OTA

void ykc_monitor_set_ota_was_requested_flag(void);
uint8_t ykc_monitor_get_ota_was_requested_flag(void);

int32_t ykc_monitor_ota_init(void);

#endif /* NET_INCLUDE_OTA */
#endif /* NET_PACK_USING_YKC_MONITOR */

#endif /* NET_NET_YKC_MONITOR_INC_YKC_MONITOR_OTA_H_ */
