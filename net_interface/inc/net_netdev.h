/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_INC_NET_NETDEV_H_
#define NET_INTERFACE_INC_NET_NETDEV_H_

#include "stdio.h"

enum netdev_type{
    NET_NETDEV_TYPE_4G = 0x01,
    NET_NETDEV_TYPE_ETHERNET = 0x02,
    NET_NETDEV_TYPE_WIFI = 0x04,
    NET_NETDEV_TYPE_BLUE = 0x08,
};

void net_set_netdev_init_status(uint8_t devid, uint8_t status);
uint8_t net_query_netdev_init_status(uint8_t devid);

void net_set_netdev_type(uint8_t type, uint8_t is_append);
void net_clear_netdev_type(uint8_t type);
enum netdev_type net_query_netdev_type(void);

int app_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port);
int app_socket_send_port(int socket_fd, void *data, uint16_t len);
int app_socket_recv_port(int socket_fd, void *buff, uint16_t len);
int app_socket_close_port(int socket_fd);
int app_socket_get_state_port(int socket_fd);
int app_socket_data_comein_port(int socket_fd, uint32_t timeout);
int app_socket_control_port(int socket_fd, uint8_t cmd, void *para);

int32_t net_netdev_init(void);

#endif /* NET_INTERFACE_INC_NET_NETDEV_H_ */
