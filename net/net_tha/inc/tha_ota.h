/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-06     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_OTA_H_
#define NET_PACK_NET_THA_INC_THA_OTA_H_

#include "net_pack_config.h"
#include "net_operation.h"

#ifdef NET_PACK_USING_THA
#ifdef NET_INCLUDE_OTA

void tha_set_ota_was_requested_flag(void);
uint8_t tha_get_ota_was_requested_flag(void);
void tha_ota_data_analyse(const uint8_t *data, uint16_t len);
int32_t tha_ota_init(void);

#endif /* NET_INCLUDE_OTA */
#endif /* NET_PACK_USING_THA */

#endif /* NET_PACK_NET_THA_INC_THA_OTA_H_ */
