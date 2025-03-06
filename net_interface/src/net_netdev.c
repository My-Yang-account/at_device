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
int app_socket_control_port(int socket_fd, uint8_t cmd, void *para, uint8_t para_len, void *ret, uint8_t ret_len)
{
    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        switch(cmd){
        case NET_SOCKET_CONTROL_RECV_TIMEOUT:
            return netdev_4g_socket_control(socket_fd, NETDEV_4G_SOCKET_CONTROL_RECV_TIMEOUT, para, para_len, ret, ret_len);
            break;
        case NET_SOCKET_CONTROL_DOMAIN_PARSE:
            return netdev_4g_socket_control(socket_fd, NETDEV_4G_SOCKET_CONTROL_DOMAIN_PARSE, para, para_len, ret, ret_len);
            break;
        default:
            break;
        }
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        switch(cmd){
        case NET_SOCKET_CONTROL_RECV_TIMEOUT:
            return netdev_ethch395_socket_control(socket_fd, NETDEV_ETHCH395_SOCKET_CONTROL_RECV_TIMEOUT, para, para_len, ret, ret_len);
            break;
        case NET_SOCKET_CONTROL_DOMAIN_PARSE:
            return netdev_ethch395_socket_control(socket_fd, NETDEV_ETHCH395_SOCKET_CONTROL_DOMAIN_PARSE, para, para_len, ret, ret_len);
            break;
        default:
            break;
        }
    }
    return -0x01;
}

/**************************************************
 *  函数名   net_netdev_query_devstate
 *  参数
 *  功能       查询网络设备状态
 *  返回      网络设备状态
 *************************************************/
uint8_t net_netdev_query_devstate(void)
{
    uint8_t state = NET_NETDEV_STATE_SIZE;

    if(s_netdev_type &NET_NETDEV_TYPE_4G){
        extern int get_at_device_appinfo_at(void);
        extern int get_at_device_appinfo_check_card(void);
        extern int get_at_device_appinfo_check_gprs_registered(void);
        extern int get_at_device_appinfo_is_complete(void);

        if(!get_at_device_appinfo_at()){
            state = NET_NETDEV_STATE_PHY;
        }else if(!get_at_device_appinfo_check_card()){
            state = NET_NETDEV_STATE_DATA_LINK_MAC;
        }else if(!get_at_device_appinfo_check_gprs_registered()){
            state = NET_NETDEV_STATE_NET_REGISTERED;
        }else if(!get_at_device_appinfo_is_complete()){
            state = NET_NETDEV_STATE_MODULE_INIT;
        }else{
            state = NET_NETDEV_STATE_NORMAL;
        }
    }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
        uint8_t sta = ethch395_query_state();
        switch(sta){
        case NETDEV_ETHCH395_STATE_PHY:
            state = NET_NETDEV_STATE_PHY;
            break;
        case NETDEV_ETHCH395_STATE_LINK_MAC:
            state = NET_NETDEV_STATE_DATA_LINK_MAC;
            break;
        case NETDEV_ETHCH395_STATE_LINK_LCC:
            state = NET_NETDEV_STATE_DATA_LINK_MAC;
//            state = NET_NETDEV_STATE_DATA_LINK_LCC;
            break;
        case NETDEV_ETHCH395_STATE_NET_REGISTERED:
            state = NET_NETDEV_STATE_NET_REGISTERED;
            break;
        case NETDEV_ETHCH395_STATE_MODULE_INIT:
            state = NET_NETDEV_STATE_MODULE_INIT;
            break;
        case NETDEV_ETHCH395_STATE_NORMAL:
            state = NET_NETDEV_STATE_NORMAL;
            break;
        default:
            break;
        }
    }

    return state;
}

/**************************************************
 *  函数名   net_netdev_dev_control
 *  参数       cmd         控制命令
 *        para        控制命令参数
 *        para_len    控制命令参数长度
 *        ret         控制命令返回参数
 *        ret_len     控制命令返回参数长度
 *  功能       网络设备控制
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int32_t net_netdev_dev_control(uint8_t cmd, void *para, uint16_t para_len, void *ret, uint16_t ret_len)
{
    switch(cmd){
    case NET_NETDEV_CTRL_CMD_RESET:
        if(s_netdev_type &NET_NETDEV_TYPE_4G){
            extern void ec20_at_device_reset(void);
            ec20_at_device_reset();
        }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
            return ethch395_device_reset();
        }
        return 0x0;
        break;
    case NET_NETDEV_CTRL_CMD_QUERY_SIM:
        if(s_netdev_type &NET_NETDEV_TYPE_4G){
            if(ret){
                extern char *get_at_device_appinfo_iccid(void);
                char *iccid = get_at_device_appinfo_iccid();
                uint8_t valid_len = strlen(iccid);
                valid_len = valid_len > 20 ? 20 : valid_len;    /** imei 最长 20 位 */
                memset(ret, 0x00, ret_len);
                memcpy(ret, iccid, valid_len);
            }
        }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
            if(ret){
                memset(ret, 0x00, ret_len);
            }
        }
        return 0x0;
        break;
    case NET_NETDEV_CTRL_CMD_QUERY_STRENGTH:
        if(s_netdev_type &NET_NETDEV_TYPE_4G){
            if(ret && ret_len > 0x00){
                extern int get_at_device_appinfo_signal_strength(void);
                *(uint8_t *)ret = (uint8_t)(get_at_device_appinfo_signal_strength());
            }
        }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
            if(ret){
                *(uint8_t *)ret = 20;
            }
        }
        return 0x0;
    case NET_NETDEV_CTRL_CMD_QUERY_IMEI:
        if(s_netdev_type &NET_NETDEV_TYPE_4G){
            if(ret){
                extern char *get_at_device_appinfo_imei(void);
                char *imei = get_at_device_appinfo_imei();
                uint8_t valid_len = strlen(imei);
                valid_len = valid_len > 16 ? 16 : valid_len;    /** imei 最长 16-1 位 */
                memset(ret, 0x00, ret_len);
                memcpy(ret, imei, valid_len);
            }
        }else if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
            if(ret){
                uint8_t *mac = ethch395_get_dev_mac(), value = 0x00;
                memset(ret, 0x00, ret_len);

                for(uint8_t i = 0x00, j = 0x00; (i < 0x06) && (j < ret_len); i++, j += 0x02){  /** MAC地址为6字节 */
                    value = (((mac[i]) &0xF0) >>0x04);
                    if(value > 0x09){
                        value += ('A' - (0x09 + 0x01));
                    }else{
                        value += '0';
                    }
                    *((uint8_t*)ret + j) = value;

                    if((j + 0x01) < ret_len){
                        value = ((mac[i]) &0x0F);
                        if(value > 0x09){
                            value += ('A' - (0x09 + 0x01));
                        }else{
                            value += '0';
                        }
                        *((uint8_t*)ret + j + 0x01) = value;
                    }
                }
            }
        }
        return 0x0;
        break;
    case NET_NETDEV_CTRL_CMD_QUERY_SYS_ERR:
        if(ret){
            *(uint8_t*)ret = 0x00;             /** 无故障 */
            if(s_netdev_type &NET_NETDEV_TYPE_ETHERNET){
                if(ethch395_is_occured_sys_err()){
                    *(uint8_t*)ret = 0x01;     /** 有故障 */
                }
            }
            return 0x00;
        }
        break;
    default:
        break;
    }

    return -0x01;
}

/**************************************************
 *  函数名   net_netdev_init
 *  功能       网络设备初始化
 *  参数
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int32_t net_netdev_init(void)
{
    /** 4G 初始化 */

    /** 以太网初始化 */
    return net_ethch395_transceiver_init();
}
