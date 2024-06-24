/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-17     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_TRANSCEIVER_H_
#define NET_PACK_NET_YKC_INC_YKC_TRANSCEIVER_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YKC

#define YKC_SERVICE_CALLBACK_ITEM_MAX                           22                          /* 服务回调项数量 */
#define YKC_RECV_BUFF_SIZE                                      16 + NET_OTA_SEGMENT_LEN    /* 接收缓存大小 */
#define YKC_RECV_THREAD_STACK_SIZE                              2048                        /* 报文接收线程栈大小 */

int ykc_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port);
int ykc_socket_close(int fd);
int ykc_socket_send(int fd, void *data, uint16_t len);
int ykc_socket_recv(int fd, void *buff, uint16_t len);
int ykc_socket_data_comein(int fd, uint32_t timeout);
int ykc_socket_wait_data_write(int fd, uint32_t timeout);

void ykc_service_callback_register(uint8_t id, void *cb);
void *ykc_get_service_callback(uint8_t id);

int32_t ykc_transceiver_init(void);
int32_t ykc_message_send_port(uint8_t cmd, int fd, void *data, uint16_t len);

#endif /* NET_PACK_USING_YKC */
#endif /* NET_PACK_NET_YKC_INC_YKC_TRANSCEIVER_H_ */
