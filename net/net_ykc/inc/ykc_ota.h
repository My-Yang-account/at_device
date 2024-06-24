/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_OTA_H_
#define NET_PACK_NET_YKC_INC_YKC_OTA_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YKC
#ifdef NET_INCLUDE_OTA

void ykc_set_ota_was_requested_flag(void);
uint8_t ykc_get_ota_was_requested_flag(void);

int32_t ykc_ota_init(void);

#endif /* NET_INCLUDE_OTA */
#endif /* NET_PACK_USING_YKC */

#endif /* NET_PACK_NET_YKC_INC_YKC_OTA_H_ */
