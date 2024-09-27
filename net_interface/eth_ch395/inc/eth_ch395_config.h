/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_ETH_395_INC_ETH_395_CONFIG_H_
#define NET_INTERFACE_ETH_395_INC_ETH_395_CONFIG_H_

#include <rtthread.h>
#include <rtdevice.h>

#define NET_INCLUDE_ETHERNET_PACK

#ifdef NET_INCLUDE_ETHERNET_PACK

#define NET_ETHERNET_NETDEV_NAME                          "uart7"    /* 以太网网络设备名 */

/** 串口参数配置 */
#define NET_ETHERNET_SERIAL_CONFIG_DEFAULT    \
{                                    \
    BAUD_RATE_9600,                  \
    DATA_BITS_8,                     \
    STOP_BITS_1,                     \
    PARITY_NONE,                     \
    BIT_ORDER_LSB,                   \
    NRZ_NORMAL,                      \
    4096,              \
    0                                \
}

#endif /* NET_INCLUDE_ETHERNET_PACK */

#endif /* NET_INTERFACE_ETH_395_INC_ETH_395_CONFIG_H_ */
