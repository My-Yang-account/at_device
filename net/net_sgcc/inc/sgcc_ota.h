/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */
#ifndef NET_NET_SGCC_INC_SGCC_OTA_H_
#define NET_NET_SGCC_INC_SGCC_OTA_H_

#include <stdint.h>

int sgcc_firmware_start(void);
int sgcc_firmware_write(char *buffer, uint32_t length);
int sgcc_firmware_stop(void);
int sgcc_firmware_version(char *version);

uint8_t *sgcc_get_ota_buff(void);
uint32_t sgcc_get_ota_blen(void);

#endif /* NET_NET_SGCC_INC_SGCC_OTA_H_ */
