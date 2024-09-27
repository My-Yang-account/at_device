/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-08     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_4G_INC_4G_SOCKET_H_
#define NET_INTERFACE_4G_INC_4G_SOCKET_H_

#include "stdio.h"

enum{
    NETDEV_4G_SOCKET_CONTROL_RECV_TIMEOUT,                   /** 4G socket 控制指令：修改数据接收等待时间 */
};

int netdev_4g_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port);
int netdev_4g_socket_send_port(int socket_fd, void *data, uint16_t len);
int netdev_4g_socket_recv_port(int socket_fd, void *buff, uint16_t len);
int netdev_4g_socket_close_port(int socket_fd);
int netdev_4g_socket_get_state_port(int socket_fd);
int netdev_4g_socket_data_comein_port(int socket_fd, uint32_t timeout);
int netdev_4g_socket_control(int socket_fd, uint8_t cmd, void *para);

#endif /* NET_INTERFACE_4G_INC_4G_SOCKET_H_ */
