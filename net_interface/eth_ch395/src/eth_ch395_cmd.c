/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#include "eth_ch395_cmd.h"
#include "eth_ch395_netdev.h"
#include "eth_ch395_transceiver.h"

#ifdef NET_INCLUDE_ETHERNET_PACK

#define ETHCH395_WAIT_CMD_STATUS_TIME                             1000            /* ch395 查询指令状态最大时长 */
#define ETHCH395_WAIT_CMD_OPERATE_TIME                            10000           /* ch395 等待指令操作最大时长 */

#define ETHCH395_DEVICE_MAC_LEN                                   6               /* ch395 MAC 地址最大长度 */
#define ETHCH395_DEVICE_IP_LEN                                    4               /* ch395 IP地址最大长度 */
#define ETHCH395_DEVICE_GATEWAYIP_LEN                             4               /* ch395 网关IP最大长度 */
#define ETHCH395_DEVICE_SUBNET_MASK_LEN                           4               /* ch395 子网掩码最大长度 */

#define ETHCH395_SOCKET_DEST_IP_LEN                               4               /* ch395 目的IP最大长度 */

#define ETHCH395_DNS1_IP_LEN                                      4               /* ch395 DNS1 IP最大长度 */
#define ETHCH395_DNS2_IP_LEN                                      4               /* ch395 DNS2 IP最大长度 */

struct ethch395_socket_info{
    struct{
        uint8_t used : 1;                                                         /* socket 已使用 */
    }flag;
    uint8_t sbuf_block_num;                                                       /* socket 发送缓冲区块数 */
    uint8_t rbuf_block_num;                                                       /* socket 接收缓冲区块数 */
    uint8_t work_mode;                                                            /* socket 工作模式 */
    uint16_t sour_port;                                                           /* socket 源地址端口 */
    uint16_t dest_port;                                                           /* socket 目的地址端口 */
    uint8_t dest_ip[ETHCH395_SOCKET_DEST_IP_LEN];                                 /* socket 目的地址IP */
    union ethch395_socket_int interrupt;                                          /* socket 中断 */
};

struct ethch395_info{
    struct ethch395_socket_info socket[ETHCH395_SOCKET_NUM_MAX];                  /* ch395 socket 信息 */
    uint8_t dev_mac[ETHCH395_DEVICE_MAC_LEN];                                     /* ch395 MAC 地址 */
    uint8_t dev_ip[ETHCH395_DEVICE_IP_LEN];                                       /* ch395 IP地址 */
    uint8_t dev_gatewayip[ETHCH395_DEVICE_GATEWAYIP_LEN];                         /* ch395 网关IP */
    uint8_t dev_subnet_mask[ETHCH395_DEVICE_SUBNET_MASK_LEN];                     /* ch395 子网掩码 */
    uint8_t dns1[ETHCH395_DNS1_IP_LEN];                                           /* ch395 DNS1 IP */
    uint8_t dns2[ETHCH395_DNS2_IP_LEN];                                           /* ch395 DNS2 IP */
    union ethch395_globe_int interrupt;                                           /* ch395 中断 */
};


static struct ethch395_info s_ethch395_info = {
    .socket[0x00].rbuf_block_num = 0x06,
    .socket[0x00].sbuf_block_num = 0x03,

    .socket[0x01].rbuf_block_num = 0x06,
    .socket[0x01].sbuf_block_num = 0x03,

    .socket[0x02].rbuf_block_num = 0x06,
    .socket[0x02].sbuf_block_num = 0x03,

    .socket[0x03].rbuf_block_num = 0x06,
    .socket[0x03].sbuf_block_num = 0x03,

    .socket[0x04].rbuf_block_num = 0x06,
    .socket[0x04].sbuf_block_num = 0x03,

    .socket[0x05].rbuf_block_num = 0x02,
    .socket[0x05].sbuf_block_num = 0x01,

    .socket[0x00].sour_port = 30001,
    .socket[0x01].sour_port = 2000,

    .socket[0x02].sour_port = 30500,
    .socket[0x03].sour_port = 30300,

    .socket[0x04].sour_port = 30040,
    .socket[0x05].sour_port = 35000,

    .socket[0x06].sour_port = 30010,
    .socket[0x07].sour_port = 30200,
};

static uint8_t ethch395_operate_lock = 0x00;

static void ethch395_wait_operate_lock(void)
{
    uint32_t tick = rt_tick_get();

    while(ethch395_operate_lock){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > ETHCH395_WAIT_CMD_OPERATE_TIME){
            break;
        }
        rt_thread_mdelay(10);
    }
}

static void ethch395_lock_operate_lock(void)
{
    ethch395_operate_lock = 0x01;
}

static void ethch395_unlock_operate_lock(void)
{
    ethch395_operate_lock = 0x00;
}


/**************************************************
 *  函数名   ethch395_send_cmd
 *  参数       cmd       指令
 *  功能       发送一个指令
 *  返回
 *************************************************/
static void ethch395_send_cmd(uint8_t cmd)
{
    uint8_t byte = 0x00;

    ethch395_cmd_data_clear();

    byte = 0x57;  /* 指令同步码1 */
    ethch395_netdev_send(&byte, 0x01);

    byte = 0xAB;  /* 指令同步码2 */
    ethch395_netdev_send(&byte, 0x01);

    byte = cmd;   /* 指令 */
    ethch395_netdev_send(&byte, 0x01);
}

/**************************************************
 *  函数名   ethch395_send_data
 *  参数       byte       数据
 *  功能       发送数据(对于一个带参数的指令)
 *  返回
 *************************************************/
static void ethch395_send_cmd_data(uint8_t data)
{
    uint8_t byte = data;
    ethch395_netdev_send(&byte, sizeof(byte));
}

/**************************************************
 *  函数名   ethch395_cmd_query_ic_version
 *  参数       buf       数据缓存
 *     len        数据缓存长度
 *  功能       查询模块版本
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_query_ic_version(uint8_t *buf, uint8_t len)
{
    int32_t res = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_IC_VER);
    res = ethch395_cmd_data_recv(buf, len);

    ethch395_unlock_operate_lock();
    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_set_baudrate
 *  参数       prar1       系数1
 *     prar2       系数2
 *     prar3       系数3
 *  功能      设置串口波特率
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_baudrate(uint32_t baudrate)
{
    uint8_t para0 = 0x00, para1 = 0x00, para2 = 0x00;

    switch(baudrate){
    case ETHCH395_CMD_BAUDRATE_4800:
        para0 = ETHCH395_CMD_BAUDRATE_4800_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_4800_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_4800_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_9600:
        para0 = ETHCH395_CMD_BAUDRATE_9600_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_9600_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_9600_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_19200:
        para0 = ETHCH395_CMD_BAUDRATE_19200_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_19200_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_19200_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_38400:
        para0 = ETHCH395_CMD_BAUDRATE_38400_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_38400_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_38400_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_57600:
        para0 = ETHCH395_CMD_BAUDRATE_57600_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_57600_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_57600_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_76800:
        para0 = ETHCH395_CMD_BAUDRATE_76800_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_76800_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_76800_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_115200:
        para0 = ETHCH395_CMD_BAUDRATE_115200_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_115200_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_115200_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_460800:
        para0 = ETHCH395_CMD_BAUDRATE_460800_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_460800_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_460800_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_921600:
        para0 = ETHCH395_CMD_BAUDRATE_921600_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_921600_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_921600_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_100000:
        para0 = ETHCH395_CMD_BAUDRATE_100000_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_100000_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_100000_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_1000000:
        para0 = ETHCH395_CMD_BAUDRATE_1000000_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_1000000_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_1000000_PARA2;
        break;
    case ETHCH395_CMD_BAUDRATE_3000000:
        para0 = ETHCH395_CMD_BAUDRATE_3000000_PARA0;
        para1 = ETHCH395_CMD_BAUDRATE_3000000_PARA1;
        para2 = ETHCH395_CMD_BAUDRATE_3000000_PARA2;
        break;
    default:
        return -0x01;
        break;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_BAUDRATE);

    ethch395_send_cmd_data(para0);
    ethch395_send_cmd_data(para1);
    ethch395_send_cmd_data(para2);

    rt_thread_mdelay(100);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_hard_reset
 *  参数
 *  功能       发送硬复位指令
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_hard_reset(void)
{
    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_RESET_ALL);

    rt_thread_mdelay(100);
    ethch395_unlock_operate_lock();

    return 0x00;
}

#ifdef ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44
/**************************************************
 *  函数名   ethch395_cmd_query_globe_int_status
 *  参数       buf       数据缓存
 *     len        数据缓存长度
 *  功能       查询全局中断状态
 *  返回       中断状态码
 *************************************************/
int32_t ethch395_cmd_query_globe_int_status(void)
{
    uint16_t status = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_GLOB_INT_STATUS_ALL);
    if(ethch395_cmd_data_recv((uint8_t*)(&status), sizeof(status)) < 0x00){
        ethch395_unlock_operate_lock();
        return -0x01;
    }

    s_ethch395_info.interrupt.value = (uint16_t)status;     /** 只有两字节是有效的 */
    ethch395_unlock_operate_lock();

    return 0x00;
}
#else
/**************************************************
 *  函数名   ethch395_cmd_query_globe_int_status
 *  参数       buf       数据缓存
 *     len        数据缓存长度
 *  功能       查询全局中断状态
 *  返回       中断状态码
 *************************************************/
int32_t ethch395_cmd_query_globe_int_status(void)
{
    uint8_t status = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_GLOB_INT_STATUS);
    if(ethch395_cmd_data_recv(&status, sizeof(status)) < 0x00){
        ethch395_unlock_operate_lock();
        return -0x01;
    }

    s_ethch395_info.interrupt.value = (uint8_t)status;     /** 只有两字节是有效的 */
    ethch395_unlock_operate_lock();

    return 0x00;
}
#endif /* ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44 */

/**************************************************
 *  函数名   ethch395_cmd_query_socket_int
 *  参数       fd         socket 下标
 *  功能       查询socket中断
 *  返回       socket中断状态码
 *************************************************/
int32_t ethch395_cmd_query_socket_int(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    uint8_t status = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_INT_STATUS_SN);
    ethch395_send_cmd_data(fd);
    if(ethch395_cmd_data_recv((uint8_t*)(&status), sizeof(status)) < 0x00){
        ethch395_unlock_operate_lock();
        return -0x01;
    }

    s_ethch395_info.socket[fd].interrupt.value = status;
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_query_socket_status
 *  参数       fd         socket 下标
 *  功能       查询socket状态
 *  返回       socket状态码
 *************************************************/
int32_t ethch395_cmd_query_socket_status(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    uint16_t status = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_SOCKET_STATUS_SN);
    if(ethch395_recv((uint8_t*)(&status), sizeof(status)) < 0x00){
        ethch395_unlock_operate_lock();
        return -0x01;
    }

    ethch395_unlock_operate_lock();

    return status;
}

/**************************************************
 *  函数名   ethch395_cmd_test_communication_status
 *  参数
 *  功能       测试通信接口状态
 *  返回        >= 0 : 成功，< 0 : 失败
* 注：
* 该命令需要输入 1 个字节数据，可以是任意数据，如果 CH395 正常工作，那么 CH395 的输出数据是输入数据的按位取反。
*例如，输入数据是 57H，则输出数据是 A8H
 *************************************************/
int32_t ethch395_cmd_test_communication_status(void)
{
    uint8_t sbyte = 0x57, rbyte = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_CHECK_EXIST);
    ethch395_send_cmd_data(sbyte);

    if(ethch395_cmd_data_recv(&rbyte, sizeof(rbyte)) < 0x00){
        ethch395_unlock_operate_lock();
        return -0x01;
    }

    if(rbyte != (uint8_t)(~sbyte)){
        ethch395_unlock_operate_lock();
        return -0x02;
    }

    rt_thread_mdelay(10);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_phy
 *  参数       mode       模式
 *  功能      设置                      ch395 PHY连接方式
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_phy(uint8_t mode)
{
    if((mode < ETHCH395_PHY_TYPE_DISCONNECT) || (mode > ETHCH395_PHY_TYPE_AUTO)){
        return -0x01;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_PHY);
    ethch395_send_cmd_data(mode);

    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_query_cmd_status
 *  参数
 *  功能      查询指令执行状态
 *  返回       >= 0 : 状态码，< 0 : 查询失败失败
 *************************************************/
int32_t ethch395_cmd_query_cmd_status(uint8_t nesting)
{
    uint8_t rbyte = 0x00;
    int32_t res = 0x00;

    if(nesting == ETHCH395_ENUM_FALSE){
        ethch395_wait_operate_lock();
        ethch395_lock_operate_lock();
    }

    ethch395_send_cmd(ETHCH395_CMD_GET_CMD_STATUS);
    res = ethch395_cmd_data_recv(&rbyte, sizeof(rbyte));
    if(nesting == ETHCH395_ENUM_FALSE){
        ethch395_unlock_operate_lock();
    }

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_set_func_para
 *  参数       para     参数(仅支持0和1)
 *  功能      设置TCP功能参数
 *  返回       >= 0 : 状态码，< 0 : 查询失败失败
 *************************************************/
int32_t ethch395_cmd_set_func_para(uint8_t para)
{
    int32_t res = 0x00;
#if 0
    uint8_t rbyte = 0x04;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_FUN_PARA);
    ethch395_send_cmd_data(rbyte);
    rbyte = 0;
    ethch395_send_cmd_data(rbyte);
    ethch395_send_cmd_data(rbyte);
    ethch395_send_cmd_data(rbyte);

    ethch395_unlock_operate_lock();
    rt_thread_mdelay(100);
#endif
    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_init
 *  参数
 *  功能      初始化ch395芯片
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_init(void)
{
    int32_t res = 0x00;
    uint32_t stime = rt_tick_get();

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_INIT_CH395);

    while(1){
        if(stime > rt_tick_get()){
            stime = rt_tick_get();
        }
        if((rt_tick_get() - stime) > ETHCH395_WAIT_CMD_STATUS_TIME){
            res = -0x01;
            break;
        }

        rt_thread_mdelay(10);
        res = ethch395_cmd_query_cmd_status(ETHCH395_ENUM_TRUE);

        if(res == ETHCH395_CMD_STATUS_BUSY){
            continue;
        }else{
            break;
        }
    }

    rt_thread_mdelay(300);
    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_set_dhcp_status
 *  参数
 *  功能      设置DHCP状态
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_dhcp_status(uint8_t status)
{
    int32_t res = 0x00;
    uint32_t stime = rt_tick_get();

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_DHCP_ENABLE);
    if(status){
        ethch395_send_cmd_data(ETHCH395_ENUM_TRUE);
    }else{
        ethch395_send_cmd_data(ETHCH395_ENUM_FALSE);
    }

    while(1){
        if(stime > rt_tick_get()){
            stime = rt_tick_get();
        }
        if((rt_tick_get() - stime) > ETHCH395_WAIT_CMD_STATUS_TIME){
            res = -0x01;
            break;
        }

        rt_thread_mdelay(10);
        res = ethch395_cmd_query_cmd_status(ETHCH395_ENUM_TRUE);

        if(res == ETHCH395_CMD_STATUS_BUSY){
            continue;
        }else{
            break;
        }
    }

    rt_thread_mdelay(300);
    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_query_dhcp_status
 *  参数
 *  功能      查询DHCP状态
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_query_dhcp_status(void)
{
    uint32_t byte = 0x02;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_DHCP_STATUS);
    if(ethch395_cmd_data_recv(&byte, sizeof(byte)) < 0x00){
        ethch395_unlock_operate_lock();
        return -0x01;
    }

    ethch395_unlock_operate_lock();

    return byte;
}

/**************************************************
 *  函数名   ethch395_cmd_query_ip_info
 *  参数       buf       数据缓存
 *     len        数据缓存长度
 *  功能       查询IP信息
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_query_ip_info(uint8_t *buf, uint8_t len)
{
    if((buf == NULL) || (len < (4 *5))){   /** 4 字节 IP 地址、4 字节网关 IP、4 字节子网掩码、4 字节 DNS1(主 DNS)、4 字节DNS2(次 DNS) */
        return -0x01;
    }

    uint8_t base = 0x00;
    int32_t res = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_IP_INF);
    res = ethch395_cmd_data_recv(buf, len);

    for(uint8_t count = 0x00; count < 0x04; count++){
        s_ethch395_info.dev_ip[count] = buf[base + count];
    }

    base += 0x04;
    for(uint8_t count = 0x00; count < 0x04; count++){
        s_ethch395_info.dev_gatewayip[count] = buf[base + count];
    }

    base += 0x04;
    for(uint8_t count = 0x00; count < 0x04; count++){
        s_ethch395_info.dev_subnet_mask[count] = buf[base + count];
    }

    base += 0x04;
    for(uint8_t count = 0x00; count < 0x04; count++){
        s_ethch395_info.dns1[count] = buf[base + count];
    }

    base += 0x04;
    for(uint8_t count = 0x00; count < 0x04; count++){
        s_ethch395_info.dns2[count] = buf[base + count];
    }

    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_set_socket_protocol
 *  参数       fd       socket 下标
 *        is_mode  指定协议类型
 *  功能       设置socket 协议类型
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_socket_protocol(uint8_t fd, uint8_t is_mode)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x02;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    uint8_t mode = is_mode;
    if(mode >= ETHCH395_WORK_MODE_SIZE){
        mode = s_ethch395_info.socket[fd].work_mode;
    }
    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_PROTO_TYPE_SN);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(mode);

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_dev_ip
 *  参数
 *  功能       配置设备的IP地址
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_dev_ip(void)
{
    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_IP_ADDR);
    for(uint8_t count = 0x00; count < ETHCH395_DEVICE_IP_LEN; count++){
        ethch395_send_cmd_data(s_ethch395_info.dev_ip[count]);
    }

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_dev_gateway_ip
 *  参数
 *  功能       配置设备的网关IP地址
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_dev_gateway_ip(void)
{
    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_GWIP_ADDR);
    for(uint8_t count = 0x00; count < ETHCH395_DEVICE_GATEWAYIP_LEN; count++){
        ethch395_send_cmd_data(s_ethch395_info.dev_gatewayip[count]);
    }

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_dev_subnet_mask
 *  参数
 *  功能       配置设备的子网掩码
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_dev_subnet_mask(void)
{
    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_MASK_ADDR);
    for(uint8_t count = 0x00; count < ETHCH395_DEVICE_SUBNET_MASK_LEN; count++){
        ethch395_send_cmd_data(s_ethch395_info.dev_subnet_mask[count]);
    }

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_remote_ip
 *  参数       fd       socket 下标
 *  功能       设置目标IP
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_remote_ip(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x02;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_IP_ADDR_SN);
    ethch395_send_cmd_data(fd);
    for(uint8_t count = 0x00; count < ETHCH395_SOCKET_DEST_IP_LEN; count++){
        ethch395_send_cmd_data(s_ethch395_info.socket[fd].dest_ip[count]);
    }

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_remote_port
 *  参数       fd       socket 下标
 *  功能       设置目标port
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_remote_port(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x02;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    uint8_t byte = (uint8_t)(s_ethch395_info.socket[fd].dest_port);

    ethch395_send_cmd(ETHCH395_CMD_SET_DES_PORT_SN);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(byte);

    byte = (uint8_t)(s_ethch395_info.socket[fd].dest_port >>0x08);
    ethch395_send_cmd_data(byte);

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_source_port
 *  参数       fd       socket 下标
 *  功能       设置源port
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_source_port(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x02;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    uint16_t port = s_ethch395_info.socket[fd].sour_port;
    uint8_t byte = (uint8_t)(port);

    ethch395_send_cmd(ETHCH395_CMD_SET_SOUR_PORT_SN);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(byte);

    byte = (uint8_t)(port >>0x08);
    ethch395_send_cmd_data(byte);

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_open_socket
 *  参数       fd       socket 下标
 *  功能      设置DHCP状态
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_open_socket(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    int32_t res = 0x00;
    uint32_t stime = rt_tick_get();

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_OPEN_SOCKET_SN);
    ethch395_send_cmd_data(fd);

    while(1){
        if(stime > rt_tick_get()){
            stime = rt_tick_get();
        }
        if((rt_tick_get() - stime) > ETHCH395_WAIT_CMD_STATUS_TIME){
            res = -0x01;
            break;
        }

        rt_thread_mdelay(10);
        res = ethch395_cmd_query_cmd_status(ETHCH395_ENUM_TRUE);

        if(res == ETHCH395_CMD_STATUS_BUSY){
            continue;
        }else{
            break;
        }
    }

    rt_thread_mdelay(300);
    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_connect_socket
 *  参数       fd       socket 下标
 *  功能      启动 socket 连接
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_connect_socket(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    int32_t res = 0x00;
    uint32_t stime = rt_tick_get();

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_TCP_CONNECT_SN);
    ethch395_send_cmd_data(fd);

    while(1){
        if(stime > rt_tick_get()){
            stime = rt_tick_get();
        }
        if((rt_tick_get() - stime) > ETHCH395_WAIT_CMD_STATUS_TIME){
            res = -0x01;
            break;
        }

        rt_thread_mdelay(10);
        res = ethch395_cmd_query_cmd_status(ETHCH395_ENUM_TRUE);

        if(res == ETHCH395_CMD_STATUS_BUSY){
            continue;
        }else{
            break;
        }
    }

    rt_thread_mdelay(300);
    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_padding_data_sbuf
 *  参数       fd       socket 下标
 *     data     数据首地址
 *     dlen     数据长度
 *  功能      向发送缓冲区填数据
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_padding_data_sbuf(uint8_t fd, void *data, uint16_t dlen)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }
    if((data == NULL) || (dlen == 0x00)){
        return -0x01;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_WRITE_SEND_BUF_SN);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(dlen);
    ethch395_send_cmd_data((uint8_t)(dlen >>0x08));

    for(uint16_t len = 0x00; len < dlen; len++){
        ethch395_netdev_send((data + len), 0x01);   /** 看需要是否要一个字节一个字节地发送 */
    }

    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_query_rbuf_data_len
 *  参数       fd        socket 下标
 *     buf       数据缓存
 *     len        数据缓存长度
 *  功能       查询接收缓冲区内的数据长度
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_query_rbuf_data_len(uint8_t fd, uint8_t *buf, uint8_t len)
{
    if((buf == NULL) || (len < 0x02)){     /** CH395接收到该命令后输出 2 个字节的长度(低字节在前) */
        return -0x01;
    }
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x02;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    int32_t res = 0x00;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_GET_RECV_LEN_SN);
    ethch395_send_cmd_data(fd);
    res = ethch395_cmd_data_recv(buf, len);

    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_read_rbuf_data
 *  参数       fd        socket 下标
 *     len       要接收的数据长度
 *  功能       读取接收缓冲区内的数据(只是发送命令，接收由数据接收线程完成)
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_read_rbuf_data(uint8_t fd, uint32_t len)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x02;
    }
#if 0
    if(len > (s_ethch395_info.socket[fd].rbuf_block_num *ETHCH395_SINGLE_BUFF_BLOCK_SIZE)){
        len = (s_ethch395_info.socket[fd].rbuf_block_num *ETHCH395_SINGLE_BUFF_BLOCK_SIZE);
    }
#endif
    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_READ_RECV_BUF_SN);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(len);
    ethch395_send_cmd_data((uint8_t)(len >>0x08));

    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_close_socket
 *  参数       fd       socket 下标
 *  功能      关闭socket
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_close_socket(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    int32_t res = 0x00;
    uint32_t stime = rt_tick_get();

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_CLOSE_SOCKET_SN);
    ethch395_send_cmd_data(fd);

    while(1){
        if(stime > rt_tick_get()){
            stime = rt_tick_get();
        }
        if((rt_tick_get() - stime) > ETHCH395_WAIT_CMD_STATUS_TIME){
            res = -0x01;
            break;
        }

        rt_thread_mdelay(10);
        res = ethch395_cmd_query_cmd_status(ETHCH395_ENUM_TRUE);

        if(res == ETHCH395_CMD_STATUS_BUSY){
            continue;
        }else{
            break;
        }
    }

    rt_thread_mdelay(300);
    ethch395_unlock_operate_lock();

    return res;
}

/**************************************************
 *  函数名   ethch395_cmd_disconnect_tcp
 *  参数       fd       socket 下标
 *  功能      断开TCP连接(在 TCP 模式下，关闭 Socket时 CH395 会自动断开 TCP 连接)
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_disconnect_tcp(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    int32_t res = 0x00;
    uint32_t stime = rt_tick_get();

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_TCP_DISNCONNECT_SN);
    ethch395_send_cmd_data(fd);

    while(1){
        if(stime > rt_tick_get()){
            stime = rt_tick_get();
        }
        if((rt_tick_get() - stime) > ETHCH395_WAIT_CMD_STATUS_TIME){
            res = -0x01;
            break;
        }

        rt_thread_mdelay(10);
        res = ethch395_cmd_query_cmd_status(ETHCH395_ENUM_TRUE);

        if(res == ETHCH395_CMD_STATUS_BUSY){
            continue;
        }else{
            break;
        }
    }

    rt_thread_mdelay(300);
    ethch395_unlock_operate_lock();

    return res;
}



/**************************************************
 *  函数名   ethch395_cmd_set_tcp_mss
 *  参数       mss
 *  功能       设置 tcp mss
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_tcp_mss(uint16_t mss)
{
    if((mss < ETHCH395_TCP_MSS_MIN) || (mss > ETHCH395_TCP_MSS_MAX)){
        mss = ETHCH395_TCP_MSS_DEF;
    }

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_TCP_MSS);
    ethch395_send_cmd_data((uint8_t)mss);
    ethch395_send_cmd_data((uint8_t)(mss >>0x08));

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_send_buf
 *  参数       fd
 *  功能       设置发送缓冲区
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_send_buf(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    uint8_t sblock = 0x00, block_num = 0x00;
    for(uint8_t count = 0x00; count < fd; count++){
        sblock += (s_ethch395_info.socket[count].rbuf_block_num + s_ethch395_info.socket[count].sbuf_block_num);
    }
    sblock += s_ethch395_info.socket[fd].rbuf_block_num;
    block_num = s_ethch395_info.socket[fd].sbuf_block_num;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_SEND_BUF);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(sblock);
    ethch395_send_cmd_data(block_num);

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_cmd_set_recv_buf
 *  参数       fd
 *  功能       设置接收缓冲区
 *  返回       >= 0 : 成功，< 0 : 失败
 *************************************************/
int32_t ethch395_cmd_set_recv_buf(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return -0x01;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return -0x03;
    }

    uint8_t sblock = 0x00, block_num = 0x00;
    for(uint8_t count = 0x00; count < fd; count++){
        sblock += (s_ethch395_info.socket[count].rbuf_block_num + s_ethch395_info.socket[count].sbuf_block_num);
    }
    block_num = s_ethch395_info.socket[fd].rbuf_block_num;

    ethch395_wait_operate_lock();
    ethch395_lock_operate_lock();

    ethch395_send_cmd(ETHCH395_CMD_SET_RECV_BUF);
    ethch395_send_cmd_data(fd);
    ethch395_send_cmd_data(sblock);
    ethch395_send_cmd_data(block_num);

    rt_thread_mdelay(50);
    ethch395_unlock_operate_lock();

    return 0x00;
}



/**************************************************
 *  函数名   ethch395_socket
 *  参数       protocol       协议类型
 *  功能       查找一个未使用的 socket，并返回其下标
 *  返回       >= 0 : 有效socket下标，< 0 ：socket都已被使用
 *************************************************/
int32_t ethch395_socket(uint8_t protocol)
{
    uint8_t s = 0x00;
    for(s = 0x00; s < ETHCH395_SOCKET_NUM_MAX; s++){
        if(s_ethch395_info.socket[s].flag.used == ETHCH395_ENUM_FALSE){
            s_ethch395_info.socket[s].flag.used = ETHCH395_ENUM_TRUE;
            s_ethch395_info.socket[s].work_mode = ETHCH395_WORK_MODE_TCP;
            return s;
        }
    }

    return -0x01;
}

/**************************************************
 *  函数名   ethch395_socket_free
 *  参数       s                  socket下标
 *  功能       释放一个已使用的 socket
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_socket_free(int s)
{
    if((s >= 0x00) && (s < ETHCH395_SOCKET_NUM_MAX)){
        /** 目前 */
        uint16_t port = s_ethch395_info.socket[s].sour_port;
        uint8_t sbuf_block_num = s_ethch395_info.socket[s].sbuf_block_num,
                rbuf_block_num = s_ethch395_info.socket[s].rbuf_block_num,
                work_mode = s_ethch395_info.socket[s].work_mode;

        memset(&s_ethch395_info.socket[s], 0x00, sizeof(s_ethch395_info.socket[s]));
        s_ethch395_info.socket[s].sour_port = port;
        s_ethch395_info.socket[s].sbuf_block_num = sbuf_block_num;
        s_ethch395_info.socket[s].rbuf_block_num = rbuf_block_num;
        s_ethch395_info.socket[s].work_mode = work_mode;

        return 0x00;
    }

    return -0x01;
}

/**************************************************
 *  函数名   ethch395_config_dev_ip
 *  参数       s                  socket下标
 *        ip                 socket 目的IP
 *        len                socket 目的IP长度
 *  功能       设置 设备的IP地址
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_config_dev_ip(int s, uint8_t *ip, uint8_t len)
{
    if((ip == NULL) || (len < ETHCH395_DEVICE_IP_LEN)){
        return -0x01;
    }

    memcpy(s_ethch395_info.dev_ip, ip, ETHCH395_DEVICE_IP_LEN);

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_config_dev_gateway_ip
 *  参数       s                  socket下标
 *        ip                 socket 目的IP
 *        len                socket 目的IP长度
 *  功能       设置 设备网关IP
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_config_dev_gateway_ip(int s, uint8_t *ip, uint8_t len)
{
    if((ip == NULL) || (len < ETHCH395_DEVICE_GATEWAYIP_LEN)){
        return -0x01;
    }

    memcpy(s_ethch395_info.dev_gatewayip, ip, ETHCH395_DEVICE_GATEWAYIP_LEN);

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_config_dev_subnet_mask
 *  参数       s                  socket下标
 *        ip                 socket 目的IP
 *        len                socket 目的IP长度
 *  功能       设置 设备子网掩码
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_config_dev_subnet_mask(int s, uint8_t *mask, uint8_t len)
{
    if((mask == NULL) || (len < ETHCH395_DEVICE_SUBNET_MASK_LEN)){
        return -0x01;
    }

    memcpy(s_ethch395_info.dev_subnet_mask, mask, ETHCH395_DEVICE_SUBNET_MASK_LEN);

    return 0x00;
}

/**************************************************
 *  函数名   ethch395_config_socket_dest_ip
 *  参数       s                  socket下标
 *        ip                 socket 目的IP
 *        len                socket 目的IP长度
 *  功能       设置 socket 的目的IP
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_config_socket_dest_ip(int s, uint8_t *ip, uint8_t len)
{
    if((ip == NULL) || (len < ETHCH395_SOCKET_DEST_IP_LEN)){
        return -0x01;
    }
    if((s >= 0x00) && (s < ETHCH395_SOCKET_NUM_MAX)){
        if(s_ethch395_info.socket[s].flag.used != ETHCH395_ENUM_TRUE){
            return -0x02;
        }
        memcpy(s_ethch395_info.socket[s].dest_ip, ip, ETHCH395_SOCKET_DEST_IP_LEN);
        return 0x00;
    }

    return -0x03;
}

/**************************************************
 *  函数名   ethch395_config_socket_dest_port
 *  参数       s                  socket下标
 *        port               socket 目的端口
 *  功能       设置 socket 的目的端口
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_config_socket_dest_port(int s, uint16_t port)
{
    if((s >= 0x00) && (s < ETHCH395_SOCKET_NUM_MAX)){
        if(s_ethch395_info.socket[s].flag.used != ETHCH395_ENUM_TRUE){
            return -0x01;
        }
        s_ethch395_info.socket[s].dest_port = port;
        return 0x00;
    }

    return -0x02;
}

/**************************************************
 *  函数名   ethch395_config_socket_sour_port
 *  参数       s                  socket 原端口
 *        port
 *  功能       设置 socket 的 原端口
 *  返回       >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t ethch395_config_socket_sour_port(int s, uint16_t port)
{
    if((s >= 0x00) && (s < ETHCH395_SOCKET_NUM_MAX)){
        if(s_ethch395_info.socket[s].flag.used != ETHCH395_ENUM_TRUE){
            return -0x01;
        }
        s_ethch395_info.socket[s].sour_port = port;
        return 0x00;
    }

    return -0x01;
}


/**************************************************
 *  函数名   ethch395_get_globe_int_info
 *  参数
 *  功能       获取全局中断信息
 *  返回       全局中断信息指针
 *************************************************/
union ethch395_globe_int *ethch395_get_globe_int_info(void)
{
    return &s_ethch395_info.interrupt;
}

/**************************************************
 *  函数名   ethch395_get_socket_int_info
 *  参数       fd         socket 下标
 *  功能       获取 socket 中断信息
 *  返回      socket 中断信息指针
 *************************************************/
union ethch395_socket_int *ethch395_get_socket_int_info(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return 0x00;
    }
    if(s_ethch395_info.socket[fd].flag.used != ETHCH395_ENUM_TRUE){
        return 0x00;
    }

    return &s_ethch395_info.socket[fd].interrupt;
}

/**************************************************
 *  函数名   ethch395_get_socket_sbuf_szie
 *  参数       fd         socket 下标
 *  功能       获取 socket 发送缓存大小
 *  返回      发送缓存大小
 *************************************************/
uint32_t ethch395_get_socket_sbuf_szie(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return 0x00;
    }

    return (s_ethch395_info.socket[fd].sbuf_block_num *ETHCH395_SINGLE_BUFF_BLOCK_SIZE);
}

/**************************************************
 *  函数名   ethch395_get_socket_rbuf_szie
 *  参数       fd         socket 下标
 *  功能       获取 socket 接收缓存大小
 *  返回      接收缓存大小
 *************************************************/
uint32_t ethch395_get_socket_rbuf_szie(uint8_t fd)
{
    if(fd >= ETHCH395_SOCKET_NUM_MAX){
        return 0x00;
    }

    return (s_ethch395_info.socket[fd].rbuf_block_num *ETHCH395_SINGLE_BUFF_BLOCK_SIZE);
}

/**************************************************
 *  函数名   ethch395_get_dev_mac
 *  参数
 *  功能       获取 设备MAC地址
 *  返回      设备MAC地址指针
 *************************************************/
uint8_t *ethch395_get_dev_mac(void)
{
    return s_ethch395_info.dev_mac;
}

/**************************************************
 *  函数名   ethch395_get_dev_ip
 *  参数
 *  功能       获取 设备IP地址
 *  返回      设备IP地址指针
 *************************************************/
uint8_t *ethch395_get_dev_ip(void)
{
    return s_ethch395_info.dev_ip;
}

/**************************************************
 *  函数名   ethch395_get_dev_gatewayip
 *  参数
 *  功能       获取 设备网关IP地址
 *  返回      设备网关IP地址指针
 *************************************************/
uint8_t *ethch395_get_dev_gatewayip(void)
{
    return s_ethch395_info.dev_gatewayip;
}

/**************************************************
 *  函数名   ethch395_get_dev_subnet_mask
 *  参数
 *  功能       获取 设备子网掩码
 *  返回      设备子网掩码指针
 *************************************************/
uint8_t *ethch395_get_dev_subnet_mask(void)
{
    return s_ethch395_info.dev_subnet_mask;
}

/**************************************************
 *  函数名   ethch395_get_dns1_ip
 *  参数
 *  功能       获取DNS1 IP地址
 *  返回      DNS1 IP地址指针
 *************************************************/
uint8_t *ethch395_get_dns1_ip(void)
{
    return s_ethch395_info.dns1;
}

/**************************************************
 *  函数名   ethch395_get_dns2_ip
 *  参数
 *  功能       获取 DNS2 IP地址
 *  返回       DNS2 IP地址指针
 *************************************************/
uint8_t *ethch395_get_dns2_ip(void)
{
    return s_ethch395_info.dns2;
}

#endif /* NET_INCLUDE_ETHERNET_PACK */

