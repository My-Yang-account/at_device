/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_INC_YCP_OTA_H_
#define NET_NET_YCP_INC_YCP_OTA_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YCP
#ifdef NET_INCLUDE_OTA

void ycp_set_ota_was_requested_flag(void);
uint8_t ycp_get_ota_was_requested_flag(void);

int32_t ycp_ota_init(void);

#endif /* NET_INCLUDE_OTA */
#endif /* NET_PACK_USING_YCP */


#endif /* NET_NET_YCP_INC_YCP_OTA_H_ */
