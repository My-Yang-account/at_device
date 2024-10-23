/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_ETH_CH395_INC_ETH_395_CMD_H_

#define NET_INTERFACE_ETH_CH395_INC_ETH_395_CMD_H_

#include "eth_ch395_config.h"

#ifdef NET_INCLUDE_ETHERNET_PACK

#define ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44                                   /* 芯片版本大于0x44 */

#define ETHCH395_SINGLE_BUFF_BLOCK_SIZE                             512             /* ch395 单个缓冲区块大小(单位：字节) */
#ifdef ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44
#define ETHCH395_SOCKET_NUM_MAX                                     8               /* ch395 最大socket 数 */
#else
#define ETHCH395_SOCKET_NUM_MAX                                     4               /* ch395 最大socket 数 */
#endif /* ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44 */

/** ethch395 tcp mss */
#define ETHCH395_TCP_MSS_MIN                                        60              /* ch395 tcp mss 最小值 */
#define ETHCH395_TCP_MSS_MAX                                        1460            /* ch395 tcp mss 最大值 */
#define ETHCH395_TCP_MSS_DEF                                        512             /* ch395 tcp mss 默认值 */

/** ethch395 cmd */
#define ETHCH395_CMD_GET_IC_VER                                     0x01            /* 指令：获取芯片及固件版本 */
#define ETHCH395_CMD_SET_BAUDRATE                                   0x02            /* 指令：设置串口通讯波特率 */
#define ETHCH395_CMD_ENTER_SLEEP                                    0x03            /* 指令：进入低功耗睡眠挂起状态 */
#define ETHCH395_CMD_RESET_ALL                                      0x05            /* 指令：执行硬件复位 */
#define ETHCH395_CMD_GET_GLOB_INT_STATUS_ALL                        0x19            /* 指令：获取全局中断状态 */
#define ETHCH395_CMD_CHECK_EXIST                                    0x06            /* 指令：测试通讯接口和工作状态 */
#define ETHCH395_CMD_SET_PHY                                        0x20            /* 指令：设置 PHY 连接方式 */
#define ETHCH395_CMD_SET_MAC_ADDR                                   0x21            /* 指令：设置 MAC 地址 */
#define ETHCH395_CMD_SET_IP_ADDR                                    0x22            /* 指令：设置 IP 地址 */
#define ETHCH395_CMD_SET_GWIP_ADDR                                  0x23            /* 指令：设置网关 IP 地址 */
#define ETHCH395_CMD_SET_MASK_ADDR                                  0x24            /* 指令：设置子网掩码 */
#define ETHCH395_CMD_SET_MAC_FILT                                   0x25            /* 指令：设置 MAC 过滤模式 */
#define ETHCH395_CMD_GET_PHY_STATUS                                 0x26            /* 指令：获取 PHY 的状态 */
#define ETHCH395_CMD_INIT_CH395                                     0x27            /* 指令：初始化 CH395 芯片 */
#define ETHCH395_CMD_GET_UNREACH_IPPORT                             0x28            /* 指令：获取不可达 IP，端口和协议 */
#define ETHCH395_CMD_GET_GLOB_INT_STATUS                            0x29            /* 指令：获取全局中断状态 */
#define ETHCH395_CMD_SET_RETRAN_COUNT                               0x2A            /* 指令：设置重试次数，最大 20 次 */
#define ETHCH395_CMD_SET_RETRAN_PERIOD                              0x2B            /* 指令：设置重试周期，最大 1000MS */
#define ETHCH395_CMD_GET_CMD_STATUS                                 0x2C            /* 指令：获取命令执行状态 */
#define ETHCH395_CMD_GET_REMOT_IPP_SN                               0x2D            /* 指令：获取远端(目的)的 IP 和端口 */
#define ETHCH395_CMD_CLEAR_RECV_BUF_SN                              0x2E            /* 指令：清空 Socket 的接收缓冲区 */
#define ETHCH395_CMD_GET_SOCKET_STATUS_SN                           0x2F            /* 指令：获取 Socket 状态 */
#define ETHCH395_CMD_GET_INT_STATUS_SN                              0x30            /* 指令：获取 Socket 的中断状态 */
#define ETHCH395_CMD_SET_IP_ADDR_SN                                 0x31            /* 指令：设置 Socket 的目的 IP 地址 */
#define ETHCH395_CMD_SET_DES_PORT_SN                                0x32            /* 指令：设置 Socket 的目的端口 */
#define ETHCH395_CMD_SET_SOUR_PORT_SN                               0x33            /* 指令：设置 Socket 的源端口 */
#define ETHCH395_CMD_SET_PROTO_TYPE_SN                              0x34            /* 指令：设置 Socket 的工作模式 */
#define ETHCH395_CMD_OPEN_SOCKET_SN                                 0x35            /* 指令：打开 Socket */
#define ETHCH395_CMD_TCP_LISTEN_SN                                  0x36            /* 指令：启动 Socket 监听 */
#define ETHCH395_CMD_TCP_CONNECT_SN                                 0x37            /* 指令：启动 Socket 连接 */
#define ETHCH395_CMD_TCP_DISNCONNECT_SN                             0x38            /* 指令：断开 Socket 的 TCP 连接 */
#define ETHCH395_CMD_WRITE_SEND_BUF_SN                              0x39            /* 指令：向 Socket 发送缓冲区写数据 */
#define ETHCH395_CMD_GET_RECV_LEN_SN                                0x3B            /* 指令：获取 Socket 接收数据长度 */
#define ETHCH395_CMD_READ_RECV_BUF_SN                               0x3C            /* 指令：从 Socket 接收缓冲区接收数据 */
#define ETHCH395_CMD_CLOSE_SOCKET_SN                                0x3D            /* 指令：关闭 Socket */
#define ETHCH395_CMD_SET_IPRAW_PRO_SN                               0x3E            /* 指令：设置 Socket 的 IP 包的协议字段 */
#define ETHCH395_CMD_PING_ENABLE                                    0x3F            /* 指令：PING 使能 */
#define ETHCH395_CMD_GET_MAC_ADDR                                   0x40            /* 指令：获取 MAC 地址 */
#define ETHCH395_CMD_DHCP_ENABLE                                    0x41            /* 指令：启动（停止）DHCP */
#define ETHCH395_CMD_GET_DHCP_STATUS                                0x42            /* 指令：获取 DHCP 状态 */
#define ETHCH395_CMD_GET_IP_INF                                     0x43            /* 指令：获取 IP，MASK，DNS 等信息 */
#define ETHCH395_CMD_SET_TCP_MSS                                    0x50            /* 指令：设置 TCP MSS */
#define ETHCH395_CMD_SET_TTL                                        0x51            /* 指令：设置 TTL 值，最大 128  */
#define ETHCH395_CMD_SET_RECV_BUF                                   0x52            /* 指令：设置 Socket 接收缓冲区 */
#define ETHCH395_CMD_SET_SEND_BUF                                   0x53            /* 指令：设置 Socket 发送缓冲区 */
#define ETHCH395_CMD_SET_FUN_PARA                                   0x55            /* 指令：设置功能参数 */
#define ETHCH395_CMD_SET_KEEP_LIVE_IDLE                             0x56            /* 指令：设置 KEEPLIVE 空闲时间 */
#define ETHCH395_CMD_SET_KEEP_LIVE_INTVL                            0x57            /* 指令：设置 KEEPLIVE 超时时间 */
#define ETHCH395_CMD_SET_KEEP_LIVE_CNT                              0x58            /* 指令：设置 KEEPLIVE 超时重试次数 */
#define ETHCH395_CMD_SET_KEEP_LIVE_SN                               0x59            /* 指令：设置设置 Socket KEEPLIVE */
#define ETHCH395_CMD_EEPROM_ERASE                                   0xE9            /* 指令：擦除 EEPROM */
#define ETHCH395_CMD_EEPROM_WRITE                                   0xEA            /* 指令：写 EEPROM */
#define ETHCH395_CMD_EEPROM_READ                                    0xEB            /* 指令：读 EEPROM */
#define ETHCH395_CMD_READ_GPIO_REG                                  0xEC            /* 指令：读 GPIO 寄存器 */
#define ETHCH395_CMD_WRITE_GPIO_REG                                 0xED            /* 指令：写 GPIO 寄存器 */

/** ethch395 phy type */
#define ETHCH395_PHY_TYPE_DISCONNECT                                0x01            /* PHY连接方式：断开 PHY 连接 */
#define ETHCH395_PHY_TYPE_FULL_DUPLEX_10M                           0x02            /* PHY连接方式：10M 全双工 */
#define ETHCH395_PHY_TYPE_HALF_DUPLEX_10M                           0x04            /* PHY连接方式：10M 半双工 */
#define ETHCH395_PHY_TYPE_FULL_DUPLEX_100M                          0x08            /* PHY连接方式：100M 全双工 */
#define ETHCH395_PHY_TYPE_HALF_DUPLEX_100M                          0x10            /* PHY连接方式：100M 半双工 */
#define ETHCH395_PHY_TYPE_AUTO                                      0x20            /* PHY连接方式：自动协商 */

/** ethch395 cmd status */
#define ETHCH395_CMD_STATUS_SUCCESS                                 0x00            /* 命令执行状态：成功 */
#define ETHCH395_CMD_STATUS_BUSY                                    0x10            /* 命令执行状态：忙，表示命令正在执行 */
#define ETHCH395_CMD_STATUS_MEMORY_ERR                              0x11            /* 命令执行状态：内存管理错误 */
#define ETHCH395_CMD_STATUS_BUFF_ERR                                0x12            /* 命令执行状态：缓冲区错误 */
#define ETHCH395_CMD_STATUS_TIMEOUT                                 0x13            /* 命令执行状态：超时 */
#define ETHCH395_CMD_STATUS_ROUTING_ERR                             0x14            /* 命令执行状态：路由错误 */
#define ETHCH395_CMD_STATUS_LINK_STOP                               0x15            /* 命令执行状态：连接中止 */
#define ETHCH395_CMD_STATUS_LINK_RESET                              0x16            /* 命令执行状态：连接复位 */
#define ETHCH395_CMD_STATUS_LINK_CLOSE                              0x17            /* 命令执行状态：连接关闭 */
#define ETHCH395_CMD_STATUS_UNABLE_CONNECT                          0x18            /* 命令执行状态：无连接 */
#define ETHCH395_CMD_STATUS_VALUE_ERR                               0x19            /* 命令执行状态：值错误 */
#define ETHCH395_CMD_STATUS_PARA_ERR                                0x1A            /* 命令执行状态：参数错误 */
#define ETHCH395_CMD_STATUS_USED                                    0x1B            /* 命令执行状态：已被使用 */
#define ETHCH395_CMD_STATUS_MAC_ERR                                 0x1C            /* 命令执行状态：MAC 错误 */
#define ETHCH395_CMD_STATUS_CONNECTED                               0x1D            /* 命令执行状态：已连接 */
#define ETHCH395_CMD_STATUS_OPENED                                  0x20            /* 命令执行状态：已打开 */

/** ethch395 socket state */
#define ETHCH395_SOCKET_STATUS_CLOSE                                0x00            /* Socket 的状态码：关闭 */
#define ETHCH395_SOCKET_STATUS_OPEN                                 0x05            /* Socket 的状态码：打开 */

#define ETHCH395_TCP_STATUS_CLOSE                                   0x00            /* TCP 的状态码：关闭  */
#define ETHCH395_TCP_STATUS_LISTEN                                  0x01            /* TCP 的状态码：监听  */
#define ETHCH395_TCP_STATUS_SYN_SENT                                0x02            /* TCP 的状态码：SYN 发送  */
#define ETHCH395_TCP_STATUS_SYN_RCVD                                0x03            /* TCP 的状态码：SYN 接收  */
#define ETHCH395_TCP_STATUS_ESTABLISHED                             0x04            /* TCP 的状态码：TCP 连接建立  */
#define ETHCH395_TCP_STATUS_FIN_WAIT_1                              0x05            /* TCP 的状态码：主动关闭方首次发送 FIN  */
#define ETHCH395_TCP_STATUS_FIN_WAIT_2                              0x06            /* TCP 的状态码：主动关闭方收到 FIN 的 ACK  */
#define ETHCH395_TCP_STATUS_CLOSE_WAIT                              0x07            /* TCP 的状态码：被动关闭方收到 FIN  */
#define ETHCH395_TCP_STATUS_CLOSING                                 0x08            /* TCP 的状态码：正在关闭  */
#define ETHCH395_TCP_STATUS_LAST_ACK                                0x09            /* TCP 的状态码：被动关闭方发送 FIN  */
#define ETHCH395_TCP_STATUS_TIME_WAIT                               0x0A            /* TCP 的状态码：2MLS 等待状态  */

/** ethch395 socket interrupt */
#define ETHCH395_CMD_STATUS_OPENED                                  0x20            /* 命令执行状态：已打开 */
#define ETHCH395_CMD_STATUS_OPENED                                  0x20            /* 命令执行状态：已打开 */

/** ethch395 baudrate */
#define ETHCH395_CMD_BAUDRATE_4800                                  0x00            /* ethch395 波特率 4800 */
#define ETHCH395_CMD_BAUDRATE_4800_PARA0                            0xC0            /* ethch395 波特率 4800，参数0 */
#define ETHCH395_CMD_BAUDRATE_4800_PARA1                            0x12            /* ethch395 波特率 4800，参数1 */
#define ETHCH395_CMD_BAUDRATE_4800_PARA2                            0x00            /* ethch395 波特率 4800，参数2 */

#define ETHCH395_CMD_BAUDRATE_9600                                  0x01            /* ethch395 波特率 9600 */
#define ETHCH395_CMD_BAUDRATE_9600_PARA0                            0x80            /* ethch395 波特率 9600，参数0 */
#define ETHCH395_CMD_BAUDRATE_9600_PARA1                            0x25            /* ethch395 波特率 9600，参数1 */
#define ETHCH395_CMD_BAUDRATE_9600_PARA2                            0x00            /* ethch395 波特率 9600，参数2 */

#define ETHCH395_CMD_BAUDRATE_19200                                 0x02            /* ethch395 波特率 19200 */
#define ETHCH395_CMD_BAUDRATE_19200_PARA0                           0x00            /* ethch395 波特率 19200，参数0 */
#define ETHCH395_CMD_BAUDRATE_19200_PARA1                           0x4B            /* ethch395 波特率 19200，参数1 */
#define ETHCH395_CMD_BAUDRATE_19200_PARA2                           0x00            /* ethch395 波特率 19200，参数2 */

#define ETHCH395_CMD_BAUDRATE_38400                                 0x03            /* ethch395 波特率 38400 */
#define ETHCH395_CMD_BAUDRATE_38400_PARA0                           0x00            /* ethch395 波特率 38400，参数0 */
#define ETHCH395_CMD_BAUDRATE_38400_PARA1                           0x96            /* ethch395 波特率 38400，参数1 */
#define ETHCH395_CMD_BAUDRATE_38400_PARA2                           0x00            /* ethch395 波特率 38400，参数2 */

#define ETHCH395_CMD_BAUDRATE_57600                                 0x04            /* ethch395 波特率 57600 */
#define ETHCH395_CMD_BAUDRATE_57600_PARA0                           0x00            /* ethch395 波特率 57600，参数0 */
#define ETHCH395_CMD_BAUDRATE_57600_PARA1                           0xE1            /* ethch395 波特率 57600，参数1 */
#define ETHCH395_CMD_BAUDRATE_57600_PARA2                           0x00            /* ethch395 波特率 57600，参数2 */

#define ETHCH395_CMD_BAUDRATE_76800                                 0x05            /* ethch395 波特率 76800 */
#define ETHCH395_CMD_BAUDRATE_76800_PARA0                           0x00            /* ethch395 波特率 76800，参数0 */
#define ETHCH395_CMD_BAUDRATE_76800_PARA1                           0x2C            /* ethch395 波特率 76800，参数1 */
#define ETHCH395_CMD_BAUDRATE_76800_PARA2                           0x01            /* ethch395 波特率 76800，参数2 */

#define ETHCH395_CMD_BAUDRATE_115200                                0x06            /* ethch395 波特率 115200 */
#define ETHCH395_CMD_BAUDRATE_115200_PARA0                          0x00            /* ethch395 波特率 115200，参数0 */
#define ETHCH395_CMD_BAUDRATE_115200_PARA1                          0xC2            /* ethch395 波特率 115200，参数1 */
#define ETHCH395_CMD_BAUDRATE_115200_PARA2                          0x01            /* ethch395 波特率 115200，参数2 */

#define ETHCH395_CMD_BAUDRATE_460800                                0x07            /* ethch395 波特率 460800 */
#define ETHCH395_CMD_BAUDRATE_460800_PARA0                          0x00            /* ethch395 波特率 460800，参数0 */
#define ETHCH395_CMD_BAUDRATE_460800_PARA1                          0x08            /* ethch395 波特率 460800，参数1 */
#define ETHCH395_CMD_BAUDRATE_460800_PARA2                          0x07            /* ethch395 波特率 460800，参数2 */

#define ETHCH395_CMD_BAUDRATE_921600                                0x08            /* ethch395 波特率 921600 */
#define ETHCH395_CMD_BAUDRATE_921600_PARA0                          0x00            /* ethch395 波特率 921600，参数0 */
#define ETHCH395_CMD_BAUDRATE_921600_PARA1                          0x10            /* ethch395 波特率 921600，参数1 */
#define ETHCH395_CMD_BAUDRATE_921600_PARA2                          0x0E            /* ethch395 波特率 921600，参数2 */

#define ETHCH395_CMD_BAUDRATE_100000                                0x09            /* ethch395 波特率 100000 */
#define ETHCH395_CMD_BAUDRATE_100000_PARA0                          0xA0            /* ethch395 波特率 100000，参数0 */
#define ETHCH395_CMD_BAUDRATE_100000_PARA1                          0x86            /* ethch395 波特率 100000，参数1 */
#define ETHCH395_CMD_BAUDRATE_100000_PARA2                          0x01            /* ethch395 波特率 100000，参数2 */

#define ETHCH395_CMD_BAUDRATE_1000000                               0x0A            /* ethch395 波特率 1000000 */
#define ETHCH395_CMD_BAUDRATE_1000000_PARA0                         0x40            /* ethch395 波特率 1000000，参数0 */
#define ETHCH395_CMD_BAUDRATE_1000000_PARA1                         0x42            /* ethch395 波特率 1000000，参数1 */
#define ETHCH395_CMD_BAUDRATE_1000000_PARA2                         0x0F            /* ethch395 波特率 1000000，参数2 */

#define ETHCH395_CMD_BAUDRATE_3000000                               0x0B            /* ethch395 波特率 3000000 */
#define ETHCH395_CMD_BAUDRATE_3000000_PARA0                         0xC0            /* ethch395 波特率 3000000，参数0 */
#define ETHCH395_CMD_BAUDRATE_3000000_PARA1                         0xC6            /* ethch395 波特率 3000000，参数1 */
#define ETHCH395_CMD_BAUDRATE_3000000_PARA2                         0x2D            /* ethch395 波特率 3000000，参数2 */

/** ethch395 socket number */
#define ETHCH395_SOCKET0_NUMBER                                     0x00            /* socket0 中断号 */
#define ETHCH395_SOCKET1_NUMBER                                     0x01            /* socket1 中断号 */
#define ETHCH395_SOCKET2_NUMBER                                     0x02            /* socket2 中断号 */
#define ETHCH395_SOCKET3_NUMBER                                     0x03            /* socket3 中断号 */
#ifdef ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44
#define ETHCH395_SOCKET4_NUMBER                                     0x04            /* socket4 中断号 */
#define ETHCH395_SOCKET5_NUMBER                                     0x05            /* socket5 中断号 */
#define ETHCH395_SOCKET6_NUMBER                                     0x06            /* socket6 中断号 */
#define ETHCH395_SOCKET7_NUMBER                                     0x07            /* socket7 中断号 */
#endif /* #ifdef ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44 */

enum ethch395_enum{
    ETHCH395_ENUM_FALSE,
    ETHCH395_ENUM_TRUE,
};

enum ethch395_work_mode{
    ETHCH395_WORK_MODE_IP_MESSAGE,
    ETHCH395_WORK_MODE_MAC_MESSAGE,
    ETHCH395_WORK_MODE_UDP,
    ETHCH395_WORK_MODE_TCP,
    ETHCH395_WORK_MODE_SIZE,
};

union ethch395_globe_int{
    struct{
        uint16_t not_reachable : 1;                   /* 中断：不可达中断 */
        uint16_t ip_conflict : 1;                     /* 中断：IP 冲突 */
        uint16_t phy_changed : 1;                     /* 中断：PHY 状态改变 */
        uint16_t dhcp : 1;                            /* 中断：dhcp */
        uint16_t socket0 : 1;                         /* 中断：socket0 */
        uint16_t socket1 : 1;                         /* 中断：socket1 */
        uint16_t socket2 : 1;                         /* 中断：socket2 */
        uint16_t socket3 : 1;                         /* 中断：socket3 */
        uint16_t socket4 : 1;                         /* 中断：socket4 */
        uint16_t socket5 : 1;                         /* 中断：socket5 */
        uint16_t socket6 : 1;                         /* 中断：socket6 */
        uint16_t socket7 : 1;                         /* 中断：socket7 */
        uint16_t reserve : 4;                         /* 预留 */
    }bit;
    uint16_t value;
};

union ethch395_socket_int{
    struct{
        uint8_t sbuf_free : 1;                       /* 中断：发送缓冲区空闲 */
        uint8_t send_ok : 1;                         /* 中断：发送成功 */
        uint8_t recv : 1;                            /* 中断：接收缓冲区非空 */
        uint8_t tcp_connect : 1;                     /* 中断：TCP 连接 */
        uint8_t tcp_disconnect : 1;                  /* 中断：TCP 断开 */
        uint8_t reserve0 : 1;                        /* 中断：保留 */
        uint8_t timeout : 1;                         /* 中断：超时 */
        uint8_t reserve1 : 1;                        /* 中断：保留 */
    }bit;
    uint8_t value;
};

int32_t ethch395_cmd_query_ic_version(uint8_t *buf, uint8_t len);
int32_t ethch395_cmd_set_baudrate(uint32_t baudrate);
int32_t ethch395_cmd_hard_reset(void);
int32_t ethch395_cmd_query_globe_int_status(void);
int32_t ethch395_cmd_query_socket_int(uint8_t fd);
int32_t ethch395_cmd_query_socket_status(uint8_t fd);
int32_t ethch395_cmd_test_communication_status(void);
int32_t ethch395_cmd_set_phy(uint8_t mode);
int32_t ethch395_cmd_query_cmd_status(void);
int32_t ethch395_cmd_set_func_para(uint8_t para);
int32_t ethch395_cmd_init(void);
int32_t ethch395_cmd_set_dhcp_status(uint8_t status);
int32_t ethch395_cmd_query_dhcp_status(void);
int32_t ethch395_cmd_query_ip_info(uint8_t *buf, uint8_t len);
int32_t ethch395_cmd_query_dev_mac(uint8_t *buf, uint8_t len);
int32_t ethch395_cmd_set_socket_protocol(uint8_t fd, uint8_t is_mode);
int32_t ethch395_cmd_set_dev_ip(void);
int32_t ethch395_cmd_set_dev_gateway_ip(void);
int32_t ethch395_cmd_set_dev_subnet_mask(void);
int32_t ethch395_cmd_set_remote_ip(uint8_t fd);
int32_t ethch395_cmd_set_remote_port(uint8_t fd);
int32_t ethch395_cmd_set_source_port(uint8_t fd);
int32_t ethch395_cmd_open_socket(uint8_t fd);
int32_t ethch395_cmd_connect_socket(uint8_t fd);
int32_t ethch395_cmd_padding_data_sbuf(uint8_t fd, void *data, uint16_t dlen);
int32_t ethch395_cmd_query_rbuf_data_len(uint8_t fd, uint8_t *buf, uint8_t len);
int32_t ethch395_cmd_read_rbuf_data(uint8_t fd, uint32_t len);
int32_t ethch395_cmd_close_socket(uint8_t fd);
int32_t ethch395_cmd_disconnect_tcp(uint8_t fd);

int32_t ethch395_cmd_set_tcp_mss(uint16_t mss);
int32_t ethch395_cmd_set_send_buf(uint8_t fd);
int32_t ethch395_cmd_set_recv_buf(uint8_t fd);

int32_t ethch395_cmd_set_socket_keeplive(uint8_t fd, uint8_t state);

int32_t ethch395_socket(uint8_t protocol);
int32_t ethch395_socket_free(int s);

int32_t ethch395_config_dev_ip(int s, uint8_t *ip, uint8_t len);
int32_t ethch395_config_dev_gateway_ip(int s, uint8_t *ip, uint8_t len);
int32_t ethch395_config_dev_subnet_mask(int s, uint8_t *mask, uint8_t len);
int32_t ethch395_config_socket_dest_ip(int s, uint8_t *ip, uint8_t len);
int32_t ethch395_config_socket_dest_port(int s, uint16_t port);
int32_t ethch395_config_socket_sour_port(int s, uint16_t port);

union ethch395_globe_int *ethch395_get_globe_int_info(void);
union ethch395_socket_int *ethch395_get_socket_int_info(uint8_t fd);
uint32_t ethch395_get_socket_sbuf_szie(uint8_t fd);
uint32_t ethch395_get_socket_rbuf_szie(uint8_t fd);

uint8_t *ethch395_get_dev_mac(void);
uint8_t *ethch395_get_dev_ip(void);
uint8_t *ethch395_get_dev_gatewayip(void);
uint8_t *ethch395_get_dev_subnet_mask(void);
uint8_t *ethch395_get_dns1_ip(void);
uint8_t *ethch395_get_dns2_ip(void);

#endif /* NET_INCLUDE_ETHERNET_PACK */

#endif /* NET_INTERFACE_ETH_CH395_INC_ETH_395_CMD_H_ */
