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

enum netdev_state{
    NET_NETDEV_STATE_PHY,                                           /** 网络设备状态：物理层初始化 */
    NET_NETDEV_STATE_DATA_LINK_MAC,                                 /** 网络设备状态：链路MAC层初始化 */
    NET_NETDEV_STATE_DATA_LINK_LCC,                                 /** 网络设备状态：链路LCC层初始化 */
    NET_NETDEV_STATE_NET_REGISTERED,                                /** 网络设备状态：网络注册 */
    NET_NETDEV_STATE_MODULE_INIT,                                   /** 网络设备状态：通信模块信息初始化 */
    NET_NETDEV_STATE_NORMAL,                                        /** 网络设备状态：正常 */
    NET_NETDEV_STATE_SIZE,
};

enum netdev_ctrl{
    NET_NETDEV_CTRL_CMD_RESET,
    NET_NETDEV_CTRL_CMD_QUERY_SIM,
    NET_NETDEV_CTRL_CMD_QUERY_STRENGTH,
    NET_NETDEV_CTRL_CMD_QUERY_IMEI,
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
int app_socket_control_port(int socket_fd, uint8_t cmd, void *para, uint8_t para_len, void *ret, uint8_t ret_len);

uint8_t net_netdev_query_devstate(void);
int32_t net_netdev_dev_control(uint8_t cmd, void *para, uint16_t para_len, void *ret, uint16_t ret_len);

int32_t net_netdev_init(void);

#endif /* NET_INTERFACE_INC_NET_NETDEV_H_ */
