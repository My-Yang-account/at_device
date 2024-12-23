/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#ifndef NET_NET_YKC_MONITOR_INC_YKC_MONITOR_TRANSCEIVER_H_
#define NET_NET_YKC_MONITOR_INC_YKC_MONITOR_TRANSCEIVER_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YKC_MONITOR

#ifdef NET_YKC_MONITOR_AS_MONITOR
#define YKC_USER_MONITOR_SERVICE_CALLBACK_NUM    15
#else
#define YKC_USER_MONITOR_SERVICE_CALLBACK_NUM    0
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#define YKC_MONITOR_SERVICE_CALLBACK_ITEM_MAX                           24 + YKC_USER_MONITOR_SERVICE_CALLBACK_NUM    /* 服务回调项数量 */

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
#define YKC_MONITOR_RECV_BUFF_SIZE                                      18 + NET_OTA_SEGMENT_LEN    /* 接收缓存大小 */
#else
#define YKC_MONITOR_RECV_BUFF_SIZE                                      16 + NET_OTA_SEGMENT_LEN    /* 接收缓存大小 */
#endif

#define YKC_MONITOR_RECV_THREAD_STACK_SIZE                              2048                        /* 报文接收线程栈大小 */

uint8_t ykc_monitor_socket_is_lock(void);

int ykc_monitor_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port);
int ykc_monitor_socket_close(int fd);
int ykc_monitor_socket_send(int fd, void *data, uint16_t len);
int ykc_monitor_socket_recv(int fd, void *buff, uint16_t len);
int ykc_monitor_socket_data_comein(int fd, uint32_t timeout);
int ykc_monitor_socket_wait_data_write(int fd, uint32_t timeout);
int ykc_monitor_socket_modify_recv_timeout(int fd, int32_t timeout);
int ykc_monitor_socket_domain_parse(int fd, char *domain, uint8_t dlen, void *ret, uint8_t ret_len);

void ykc_monitor_service_callback_register(uint8_t id, void *cb);
void *ykc_monitor_get_service_callback(uint8_t id);

int32_t ykc_monitor_transceiver_init(void);
int32_t ykc_monitor_message_send_port(uint16_t cmd, int fd, void *data, uint32_t len, char* lable);

#endif /* NET_PACK_USING_YKC_MONITOR */
#endif /* NET_NET_YKC_MONITOR_INC_YKC_MONITOR_TRANSCEIVER_H_ */
