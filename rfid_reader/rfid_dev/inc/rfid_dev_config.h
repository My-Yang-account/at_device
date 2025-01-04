/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#ifndef RFID_READER_RFID_DEV_INC_RFID_DEV_CONFIG_H_
#define RFID_READER_RFID_DEV_INC_RFID_DEV_CONFIG_H_

#include "stdio.h"
#include "string.h"
#include "app_ofsm.h"

#define RFID_DEV_INCLUDE_THA                              /* 射频识别设备：钛昕 */
#define RFID_DEV_INCLUDE_MT                              /* 射频识别设备：铭特 */

#define RFID_DESIGNATE_REGION                             /* 变量定义到指定区 */

#ifdef APP_USING_DOUBLEGUN
#define RFID_DEV_NAME                          "uart2"    /* 射频识别设备名 */
#else
#define RFID_DEV_NAME                          "uart6"    /* 射频识别设备名 */
#endif /* APP_USING_DOUBLEGUN */

/** 串口参数配置 */
#define RFID_DEV_SERIAL_CONFIG_DEFAULT    \
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

#ifdef RFID_DESIGNATE_REGION
#define RFID_DEF_TCMRAM __attribute__((section(".TCM_RAM")))      /* 将变量定义在TCMRAM区，注：对于GD32F470ZGT6 TCMRAM 不能存放代码，不能被任何 DMA 访问，可以将一些变量定义在该地址空间；定义的变量初始值是未知的 */
#define RFID_DEF_SRAM0  __attribute__((section(".SRAM0_RAM")))    /* 将变量定义在SRAM0区，注：对于GD32F470ZGT6 SRAM0 可以存放代码，也可以存放变量，也可以作为线程的栈地址空间，可以被 DMA 访问；定义的变量初始值是未知的 */
#define RFID_DEF_SRAM1  __attribute__((section(".SRAM1_RAM")))    /* 将变量定义在SRAM1区，注：对于GD32F470ZGT6 SRAM1 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#define RFID_DEF_SRAM2  __attribute__((section(".SRAM2_RAM")))    /* 将变量定义在SRAM2区，注：对于GD32F470ZGT6 SRAM2 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#else
#define RFID_DEF_TCMRAM
#define RFID_DEF_SRAM0
#define RFID_DEF_SRAM1
#define RFID_DEF_SRAM2
#endif /* RFID_DESIGNATE_REGION */

#endif /* RFID_READER_RFID_DEV_INC_RFID_DEV_CONFIG_H_ */
