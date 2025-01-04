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

#define ETH_DESIGNATE_REGION                                         /* 变量定义到指定区 */

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

#ifdef ETH_DESIGNATE_REGION
#define ETH_DEF_TCMRAM __attribute__((section(".TCM_RAM")))      /* 将变量定义在TCMRAM区，注：对于GD32F470ZGT6 TCMRAM 不能存放代码，不能被任何 DMA 访问，可以将一些变量定义在该地址空间；定义的变量初始值是未知的 */
#define ETH_DEF_SRAM0  __attribute__((section(".SRAM0_RAM")))    /* 将变量定义在SRAM0区，注：对于GD32F470ZGT6 SRAM0 可以存放代码，也可以存放变量，也可以作为线程的栈地址空间，可以被 DMA 访问；定义的变量初始值是未知的 */
#define ETH_DEF_SRAM1  __attribute__((section(".SRAM1_RAM")))    /* 将变量定义在SRAM1区，注：对于GD32F470ZGT6 SRAM1 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#define ETH_DEF_SRAM2  __attribute__((section(".SRAM2_RAM")))    /* 将变量定义在SRAM2区，注：对于GD32F470ZGT6 SRAM2 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#else
#define ETH_DEF_TCMRAM
#define ETH_DEF_SRAM0
#define ETH_DEF_SRAM1
#define ETH_DEF_SRAM2
#endif /* ETH_DESIGNATE_REGION */

#endif /* NET_INCLUDE_ETHERNET_PACK */

#endif /* NET_INTERFACE_ETH_395_INC_ETH_395_CONFIG_H_ */
