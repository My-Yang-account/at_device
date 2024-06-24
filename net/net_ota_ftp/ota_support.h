/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_OTA_SUPPORT_H_
#define NET_PACK_NET_YKC_INC_YKC_OTA_SUPPORT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int32_t ftp_read(void);
void ftp_callback_init(int32_t (*callback)(const uint8_t *data, uint32_t len));
int32_t ftp_linkkie_new(const char *host, const char *user, const char *password, const char *path);
uint32_t ftp_file_size(void);
void ftp_free(void);

#ifdef __cplusplus
}
#endif

#endif /* NET_PACK_NET_YKC_INC_YKC_OTA_SUPPORT_H_ */
