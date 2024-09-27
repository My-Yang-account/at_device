/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_SOCKET_INTERFACE_H_
#define NET_PACK_NET_SOCKET_INTERFACE_H_

#include "net_pack_config.h"

enum {
    NET_SOCKET_CONTROL_RECV_TIMEOUT,                    /** socket 控制指令：修改数据接收等待时间 */
};

int net_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port);
int net_socket_send(int fd, void *data, uint16_t len);
int net_socket_recv(int fd, void *buff, uint16_t len);
int net_socket_close(int fd);
int net_socket_get_state(int fd);
int net_socket_data_comein(int fd, uint32_t timeout);
int net_socket_control(int fd, uint8_t cmd, void* para);

#endif /* NET_PACK_NET_SOCKET_INTERFACE_H_ */
