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

/** thread monitor info */
#define ETHCH395_NODE_RUNNING_OPTION_ENTRY_MAX     (1 <<0x01)      /** 节点运行选项字：最大容忍次数 */
#define ETHCH395_NODE_RUNNING_OPTION_URGENT        (1 <<0x02)      /** 节点运行选项字：紧急(无需判断，直接处理) */
#define ETHCH395_NODE_RUNNING_OPTION_NAME          (1 <<0x03)      /** 节点运行选项字：线程名字 */

/** ethch395 socket control cmd */
enum{
    NETDEV_ETHCH395_SOCKET_CONTROL_RECV_TIMEOUT,                   /** 以太网 ch395 socket 控制指令：修改数据接收等待时间 */
    NETDEV_ETHCH395_SOCKET_CONTROL_DOMAIN_PARSE,                   /** 以太网 ch395 socket 控制指令：域名解析 */
};

/** ethch395 device state */
enum{
    NETDEV_ETHCH395_STATE_PHY,                                     /** 以太网 ch395状态：物理层 */
    NETDEV_ETHCH395_STATE_LINK_MAC,                                /** 以太网 ch395状态：数据链路MAC层 */
    NETDEV_ETHCH395_STATE_LINK_LCC,                                /** 以太网 ch395状态：数据链路LCC层 */
    NETDEV_ETHCH395_STATE_NET_REGISTERED,                          /** 以太网 ch395状态： 网络层*/
    NETDEV_ETHCH395_STATE_MODULE_INIT,                             /** 以太网 ch395状态： 模块初始化*/
    NETDEV_ETHCH395_STATE_NORMAL,                                  /** 以太网 ch395状态：正常 */
    NETDEV_ETHCH395_STATE_SIZE,                                    /** 以太网 ch395状态： */
};

/**************************************************
 *  函数名   ethch395_is_occured_sys_err
 *  参数
 *  功能       查询是否产生了系统故障
 *  返回        0：否    1：是
 *************************************************/
uint8_t ethch395_is_occured_sys_err(void);

/**************************************************
 *  函数名   ethch395_get_thread_handle
 *  参数       hook        函数入口
 *  功能       配置 ethch395 初始化钩子函数
 *  返回
 *************************************************/
uint8_t ethch395_query_state(void);

/**************************************************
 *  函数名   ethch395_set_init_hook
 *  参数       hook        函数入口
 *  功能       配置 ethch395 初始化钩子函数
 *  返回
 *************************************************/
void ethch395_set_init_hook(void *hook);

/*********************************************************************
 * 函数名        ethch395_set_node_init_handle
 * 功能            配置节点(线程)初始化回调句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int32_t ethch395_set_node_init_handle(void *handle);

/*********************************************************************
 * 函数名        ethch395_set_node_running_handle
 * 功能            配置节点运行句柄
 * 参数            handle    句柄
 * 返回           >=0：成功   <0：失败
 ********************************************************************/
int32_t ethch395_set_node_running_handle(void *handle);

/** socket operate api */

/**************************************************
 *  函数名   netdev_ethch395_socket_open_port
 *  参数       socket_fd   socket下标
 *     host        主机名
 *     host_len    主机名长度
 *     port        主机端口
 *  功能       向指定主机建立TCP连接
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port);

/**************************************************
 *  函数名   netdev_ethch395_socket_send_port
 *  参数       socket_fd   socket下标
 *     data        数据
 *     len         数据长度
 *  功能       向指定主发送数据
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_send_port(int socket_fd, void *data, uint32_t len);

/**************************************************
 *  函数名   netdev_ethch395_socket_recv_port
 *  参数       socket_fd   socket下标
 *     buff        数据缓存
 *     len         数据缓存长度
 *  功能       从指定socket接收数据数据
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_recv_port(int socket_fd, void *buff, uint32_t len);

/**************************************************
 *  函数名   netdev_ethch395_socket_close_port
 *  参数       socket_fd   socket下标
 *  功能       关闭指定socket
 *  返回      >= 0 : 成功，< 0 ：失败
 *************************************************/
int netdev_ethch395_socket_close_port(int socket_fd);

/**************************************************
 *  函数名  netdev_ethch395_socket_query_state_port
 *  参数       socket_fd   socket下标
 *  功能       查询指定 socket 状态
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_query_state_port(int socket_fd);

/**************************************************
 *  函数名  netdev_ethch395_socket_data_comein_port
 *  参数       socket_fd   socket下标
 *     timeout     查询超时时间
 *  功能       查询指定 socket 是否有接收到的数据
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_data_comein_port(int socket_fd, uint32_t timeout);

/**************************************************
 *  函数名   netdev_ethch395_socket_control
 *  参数       socket_fd     socket 下标
 *       cmd       指令
 *       para      指令参数
 *  功能       socket 控制指令
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int netdev_ethch395_socket_control(int socket_fd, uint8_t cmd, void *para, uint8_t para_len, void *ret, uint8_t ret_len);



/**************************************************
 *  函数名   ethch395_cmd_data_clear
 *  参数
 *  功能       清除指令响应数据
 *  返回
 *************************************************/
void ethch395_cmd_data_clear(void);

/**************************************************
 *  函数名   ethch395_cmd_data_recv
 *  参数       buf      存放响应数据的缓存
 *        len      缓存长度
 *  功能       接收指令响应数据
 *  返回       >=0：成功接收到数据或响应超时       <0：接收失败
 *************************************************/
int32_t ethch395_cmd_data_recv(uint8_t *buf, uint8_t len);

/**************************************************
 *  函数名   ethch395_device_reset
 *  参数
 *  功能       复位ethch395
 *  返回
 *************************************************/
int32_t ethch395_device_reset(void);

/**************************************************
 *  函数名   net_ethch395_transceiver_init
 *  参数
 *  功能       初始化ethch395 设备IO及相关处理线程
 *  返回      >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t net_ethch395_transceiver_init(void);

#endif /* NET_INTERFACE_ETH_395_INC_ETH_395_TRANSCEIVER_H_ */
