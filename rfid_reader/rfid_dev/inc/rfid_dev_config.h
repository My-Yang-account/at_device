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

#endif /* RFID_READER_RFID_DEV_INC_RFID_DEV_CONFIG_H_ */
