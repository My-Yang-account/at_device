/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_ETH_395_INC_ETH_395_TRANSCEIVER_H_
#define NET_INTERFACE_ETH_395_INC_ETH_395_TRANSCEIVER_H_

#include "eth_ch395_config.h"

#define ETHCH395_NODE_RUNNING_OPTION_ENTRY_MAX     (1 <<0x01)      /** 节点运行选项字：最大容忍次数 */
#define ETHCH395_NODE_RUNNING_OPTION_URGENT        (1 <<0x02)      /** 节点运行选项字：紧急(无需判断，直接处理) */
#define ETHCH395_NODE_RUNNING_OPTION_NAME          (1 <<0x03)      /** 节点运行选项字：线程名字 */

enum{
    NETDEV_ETHCH395_SOCKET_CONTROL_RECV_TIMEOUT,                   /** 以太网 ch395 socket 控制指令：修改数据接收等待时间 */
    NETDEV_ETHCH395_SOCKET_CONTROL_DOMAIN_PARSE,                   /** 以太网 ch395 socket 控制指令：域名解析 */
};

enum{
    NETDEV_ETHCH395_STATE_PHY,                                     /** 以太网 ch395状态：物理层 */
    NETDEV_ETHCH395_STATE_LINK_MAC,                                /** 以太网 ch395状态：数据链路MAC层 */
    NETDEV_ETHCH395_STATE_LINK_LCC,                                /** 以太网 ch395状态：数据链路LCC层 */
    NETDEV_ETHCH395_STATE_NET_REGISTERED,                          /** 以太网 ch395状态： 网络层*/
    NETDEV_ETHCH395_STATE_MODULE_INIT,                             /** 以太网 ch395状态： 模块初始化*/
    NETDEV_ETHCH395_STATE_NORMAL,                                  /** 以太网 ch395状态：正常 */
    NETDEV_ETHCH395_STATE_SIZE,                                    /** 以太网 ch395状态： */
};

uint8_t ethch395_query_state(void);
void ethch395_set_init_hook(void *hook);

int32_t ethch395_set_node_init_handle(void *handle);
int32_t ethch395_set_node_running_handle(void *handle);

int netdev_ethch395_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port);
int netdev_ethch395_socket_send_port(int socket_fd, void *data, uint32_t len);
int netdev_ethch395_socket_recv_port(int socket_fd, void *buff, uint32_t len);
int netdev_ethch395_socket_close_port(int socket_fd);
int netdev_ethch395_socket_query_state_port(int socket_fd);
int netdev_ethch395_socket_data_comein_port(int socket_fd, uint32_t timeout);
int netdev_ethch395_socket_control(int socket_fd, uint8_t cmd, void *para, uint8_t para_len, void *ret, uint8_t ret_len);

void ethch395_cmd_data_clear(void);
int32_t ethch395_cmd_data_recv(uint8_t *buf, uint8_t len);

int32_t ethch395_device_reset(void);
int32_t net_ethch395_transceiver_init(void);

#endif /* NET_INTERFACE_ETH_395_INC_ETH_395_TRANSCEIVER_H_ */
