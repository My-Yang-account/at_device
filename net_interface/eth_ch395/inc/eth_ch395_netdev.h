/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_ETH_395_INC_ETH_395_NETDEV_H_

#define NET_INTERFACE_ETH_395_INC_ETH_395_NETDEV_H_

#include "eth_ch395_config.h"

#ifdef NET_INCLUDE_ETHERNET_PACK

/** 以太网设备通信波特率 */
#define ETHCH395_NETDEV_BAUDRATE_4800                       4800            /* ethch395 网络设备波特率 4800 */
#define ETHCH395_NETDEV_BAUDRATE_9600                       9600            /* ethch395 网络设备波特率 9600 */
#define ETHCH395_NETDEV_BAUDRATE_19200                      19200           /* ethch395 网络设备波特率 19200 */
#define ETHCH395_NETDEV_BAUDRATE_38400                      38400           /* ethch395 网络设备波特率 38400 */
#define ETHCH395_NETDEV_BAUDRATE_57600                      57600           /* ethch395 网络设备波特率 57600 */
#define ETHCH395_NETDEV_BAUDRATE_76800                      76800           /* ethch395 网络设备波特率 76800 */
#define ETHCH395_NETDEV_BAUDRATE_115200                     115200          /* ethch395 网络设备波特率 115200 */
#define ETHCH395_NETDEV_BAUDRATE_460800                     460800          /* ethch395 网络设备波特率 460800 */
#define ETHCH395_NETDEV_BAUDRATE_921600                     921600          /* ethch395 网络设备波特率 921600 */
#define ETHCH395_NETDEV_BAUDRATE_100000                     100000          /* ethch395 网络设备波特率 100000 */
#define ETHCH395_NETDEV_BAUDRATE_1000000                    1000000         /* ethch395 网络设备波特率 1000000 */
#define ETHCH395_NETDEV_BAUDRATE_3000000                    3000000         /* ethch395 网络设备波特率 3000000 */

/** 以太网设备控制指令码 */
#define ETHCH395_NETDEV_CTRL_BAUDRATE                       0x00            /** ethch395 网络设备参数控制：修改波特率 */
#define ETHCH395_NETDEV_CTRL_HARDRESET                      0x01            /** ethch395 网络设备参数控制：硬复位 */

/********************************************************
 * 函数名             ethch395_netdev_init
 * 功能                 初始化以太网芯片设备
 * 参数
 * 返回                 >=0：成功       <0：失败
 *******************************************************/
int32_t ethch395_netdev_init(void);

/********************************************************
 * 函数名             take_ethch395_data_sem
 * 功能                 设备接收到数据后会释放该信号量，用来判断设备是否接收到了数据
 * 参数                 timeout    等待信号量时间(ms)
 * 返回                 0：获取到信号量      其它：未获取到信号量
 *******************************************************/
int32_t take_ethch395_data_sem(uint32_t timeout);

/********************************************************
 * 函数名             reset_ethch395_data_sem
 * 功能                 用于向芯片发送指令前清除数据信号量(由于mcu与以太网芯片是单串口通信
 *           为一发一收模式，为防止干扰，发送指令前需先清除信号量)
 * 参数                 timeout    等待信号量时间(ms)
 * 返回                 0：获取到信号量      其它：未获取到信号量
 *******************************************************/
int32_t reset_ethch395_data_sem(void);

/********************************************************
 * 函数名             is_ethch395_irq_coming
 * 功能                 查询是否有数据中断
 * 参数
 * 返回                 1：有      0：没有
 * 注：                  查询数据中断方式不使用IO管脚边沿中断，而是根据芯片数据中断
 *           管脚的显示状态来定(有数据时数据中断管脚一直是高电平)
 *******************************************************/
uint8_t is_ethch395_irq_coming(void);

/********************************************************
 * 函数名             ethch395_netdev_ctrl
 * 功能                 以太网设备控制
 * 参数                 cmd     指令码(见：以太网设备控制指令码)
 *           para    指令参数
 *           plen    参数长度(B)
 * 返回                 >=0：成功      <0：失败
 *******************************************************/
int32_t ethch395_netdev_ctrl(uint8_t cmd, void *para, uint8_t plen);

/********************************************************
 * 函数名             ethch395_netdev_send
 * 功能                 向设备发送数据
 * 参数                 data    数据指针
 *          len      数据长度(B)
 * 返回                 >=0：成功      <0：失败
 *******************************************************/
int32_t ethch395_netdev_send(void *data, uint32_t len);

/********************************************************
 * 函数名             ethch395_netdev_recv
 * 功能                 从设备缓存接收数据
 * 参数                 buf      用于存放接收到的数据的缓存
 *           len      缓存长度(B)
 * 返回                 接收到的字节数
 *******************************************************/
int32_t ethch395_netdev_recv(void *buf, uint32_t len);

#endif /* NET_INCLUDE_ETHERNET_PACK */

#endif /* NET_INTERFACE_ETH_395_INC_ETH_395_NETDEV_H_ */
