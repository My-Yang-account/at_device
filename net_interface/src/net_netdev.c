/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-08     我的杨yang       the first version
 */
#include "net_netdev.h"
#include "net_socket_interface.h"

#include "4g_socket.h"
#include "eth_ch395_transceiver.h"

static enum netdev_type s_netdev_type = NET_NETDEV_TYPE_4G;
static uint8_t s_netdev_init = 0x00;

/**************************************************
 *  函数名   net_set_netdev_init_status
 *  参数       devid      网络设备类型ID
 *     status     初始化状态(>0：完成，0：未完成)
 *  功能       设置网络设备初始化状态
 *  返回
 *************************************************/
void net_set_netdev_init_status(uint8_t devid, uint8_t status)
{
    if(status){
        s_netdev_init |= devid;
    }else{
        s_netdev_init &= (~devid);
    }
}
/**************************************************
 *  函数名   net_query_netdev_init_status
 *  参数       devid      网络设备类型ID
 *  功能       查询网络设备初始化状态
 *  返回        1：已初始化，0：未初始化
 *************************************************/
uint8_t net_query_netdev_init_status(uint8_t devid)
{
    if(s_netdev_init &devid){
        return 0x01;
    }

    return 0x00;
}

/**************************************************
 *  函数名   net_set_netdev_type
 *  参数       type          网络设备类型
 *     is_append     这是增加网络设备
 *  功能       设置网络设备类型
 *  返回
 *************************************************/
void net_set_netdev_type(uint8_t type, uint8_t is_append)
{
    if(is_append){
        s_netdev_type |= type;
    }else{
        s_netdev_type = type;
    }
}
/**************************************************
 *  函数名   net_clear_netdev_type
 *  参数       type     网络设备类型
 *  功能       清除网络设备类型
 *  返回
 *************************************************/
void net_clear_netdev_type(uint8_t type)
{
    s_netdev_type &= (~type);
}
/**************************************************
 *  函数名   net_query_netdev_type
 *  参数
 *  功能       获取网络设备类型
 *  返回      网络设备类型
 *************************************************/
enum netdev_type net_query_netdev_type(void)
{
    return s_netdev_type;
}

/**************************************************
 *  函数名   app_socket_open_port
 *  参数       socket_fd   socket下标
 *     host        主机名
 *     host_len    主机名长度
 *     port        主机端口
 *  功能       向指定主机建立TCP连接
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int app_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        return netdev_4g_socket_open_port(socket_fd, host, host_len, port);
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        return netdev_ethch395_socket_open_port(socket_fd, host, host_len, port);
    }

    return -0x01;
}
/**************************************************
 *  函数名   app_socket_send_port
 *  参数       socket_fd     socket 下标
 *     buf     数据
 *     len     数据长度
 *  功能       向网络发送数据
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int app_socket_send_port(int socket_fd, void *data, uint16_t len)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        return netdev_4g_socket_send_port(socket_fd, data, len);
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        return netdev_ethch395_socket_send_port(socket_fd, data, len);
    }

    return -0x01;
}
/**************************************************
 *  函数名   app_socket_recv_port
 *  参数       socket_fd     socket 下标
 *     buff    数据缓存
 *     len     数据缓存长度
 *  功能       接收socket数据
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int app_socket_recv_port(int socket_fd, void *buff, uint16_t len)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        return netdev_4g_socket_recv_port(socket_fd, buff, len);
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        return netdev_ethch395_socket_recv_port(socket_fd, buff, len);
    }

    return -0x01;
}
/**************************************************
 *  函数名   app_socket_close_port
 *  参数       socket_fd     socket 下标
 *  功能       关闭 socket
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int app_socket_close_port(int socket_fd)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        return netdev_4g_socket_close_port(socket_fd);
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        return netdev_ethch395_socket_close_port(socket_fd);
    }

    return -0x01;
}
/**************************************************
 *  函数名   app_socket_get_state_port
 *  参数       socket_fd     socket 下标
 *  功能       查询 socket 状态
 *  返回      > 0 : 连接，< =0 ：断开
 *************************************************/
int app_socket_get_state_port(int socket_fd)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        return netdev_4g_socket_get_state_port(socket_fd);
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        return netdev_ethch395_socket_query_state_port(socket_fd);
    }

    return 0x00;
}
/**************************************************
 *  函数名   app_socket_data_comein_port
 *  参数       socket_fd     socket 下标
 *     timeout       等待超时时间
 *  功能       查询 socket 是否有数据
 *  返回      > =0 : 有，< 0 ：无
 *************************************************/
int app_socket_data_comein_port(int socket_fd, uint32_t timeout)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        return netdev_4g_socket_data_comein_port(socket_fd, timeout);
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        return netdev_ethch395_socket_data_comein_port(socket_fd, timeout);
    }

    return -0x01;
}

/**************************************************
 *  函数名   app_socket_control_port
 *  参数       socket_fd     socket 下标
 *       cmd       指令
 *       para      指令参数
 *  功能       socket 控制指令
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int app_socket_control_port(int socket_fd, uint8_t cmd, void *para)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        switch(cmd){
        case NET_SOCKET_CONTROL_RECV_TIMEOUT:
            return netdev_4g_socket_control(socket_fd, NETDEV_4G_SOCKET_CONTROL_RECV_TIMEOUT, para);
            break;
        default:
            break;
        }
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        switch(cmd){
        case NET_SOCKET_CONTROL_RECV_TIMEOUT:
            return netdev_ethch395_socket_control(socket_fd, NETDEV_ETHCH395_SOCKET_CONTROL_RECV_TIMEOUT, para);
            break;
        default:
            break;
        }
    }
    return -0x01;
}

/**************************************************
 *  函数名   net_netdev_init
 *  参数
 *  功能       网络设备初始化
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int32_t net_netdev_init(void)
{
    /** 4G 初始化 */

    /** 以太网初始化 */
    return net_ethch395_transceiver_init();
}
