/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-03     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_TRANSCEIVER_H_
#define NET_PACK_NET_THA_INC_THA_TRANSCEIVER_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_THA

#define THA_SERVICE_CALLBACK_ITEM_MAX                           40                          /* 服务回调项数量 */
#define THA_RECV_BUFF_SIZE                                      16 + NET_OTA_SEGMENT_LEN    /* 接收缓存大小 */
#define THA_RECV_THREAD_STACK_SIZE                              1024                        /* 报文接收线程栈大小 */

int tha_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port);
int tha_socket_close(int fd);

void tha_service_callback_register(uint8_t id, void *cb);
void *tha_get_service_callback(uint8_t id);

int32_t tha_transceiver_init(void);
int32_t tha_message_send_port(uint8_t cmd, uint8_t gunno, int fd, void *data, uint16_t len);
uint8_t tha_get_check_code(uint8_t random, uint8_t *data, uint16_t len);

#endif /* NET_PACK_USING_THA */
#endif /* NET_PACK_NET_THA_INC_THA_TRANSCEIVER_H_ */

