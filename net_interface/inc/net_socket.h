/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_INC_NET_SOCKET_H_
#define NET_INTERFACE_INC_NET_SOCKET_H_

#include "stdio.h"

int app_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port);
int app_socket_send_port(int socket_fd, void *data, uint16_t len);
int app_socket_recv_port(int socket_fd, void *buff, uint16_t len);
int app_socket_close_port(int socket_fd);
int app_socket_get_state_port(int socket_fd);
int app_socket_data_comein_port(int socket_fd, uint32_t timeout);

#endif /* NET_INTERFACE_INC_NET_SOCKET_H_ */
