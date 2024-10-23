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

#define ETHCH395_NETDEV_CTRL_BAUDRATE                       0x00            /** ethch395 网络设备参数控制：修改波特率 */
#define ETHCH395_NETDEV_CTRL_HARDRESET                      0x01            /** ethch395 网络设备参数控制：硬复位 */

int32_t ethch395_netdev_init(void);

int32_t take_ethch395_data_sem(uint32_t timeout);
int32_t reset_ethch395_data_sem(void);
uint8_t is_ethch395_irq_coming(void);
int32_t ethch395_netdev_ctrl(uint8_t cmd, void *para, uint8_t plen);

int32_t ethch395_netdev_send(void *data, uint32_t len);
int32_t ethch395_netdev_recv(void *buf, uint32_t len);

#endif /* NET_INCLUDE_ETHERNET_PACK */

#endif /* NET_INTERFACE_ETH_395_INC_ETH_395_NETDEV_H_ */
