/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_INC_YCP_TRANSCEIVER_H_
#define NET_NET_YCP_INC_YCP_TRANSCEIVER_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YCP

#define YCP_SERVICE_CALLBACK_ITEM_MAX                           18                          /* 服务回调项数量 */
#define YCP_RECV_BUFF_SIZE                                      16 + NET_OTA_SEGMENT_LEN    /* 接收缓存大小 */
#define YCP_RECV_THREAD_STACK_SIZE                              2048                        /* 报文接收线程栈大小 */

int ycp_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port);
int ycp_socket_close(int fd);
int ycp_socket_send(int fd, void *data, uint16_t len);
int ycp_socket_recv(int fd, void *buff, uint16_t len);
int ycp_socket_data_comein(int fd, uint32_t timeout);
int ycp_socket_wait_data_write(int fd, uint32_t timeout);

uint8_t ycp_socket_is_lock(void);

void ycp_service_callback_register(uint8_t id, void *cb);
void *ycp_get_service_callback(uint8_t id);

int32_t ycp_transceiver_init(void);
int32_t ycp_message_send_port(uint8_t cmd, int fd, void *data, uint16_t len);

#endif /* NET_PACK_USING_YCP */
#endif /* NET_NET_YCP_INC_YCP_TRANSCEIVER_H_ */
