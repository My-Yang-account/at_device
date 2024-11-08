/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#ifndef RFID_READER_RFID_DEV_INC_RFID_DEV_HARDWARE_H_
#define RFID_READER_RFID_DEV_INC_RFID_DEV_HARDWARE_H_

#include "rfid_dev_config.h"

#define RFID_DEV_BAUDRATE_4800                       4800            /* 射频识别设备波特率 4800 */
#define RFID_DEV_BAUDRATE_9600                       9600            /* 射频识别设备波特率 9600 */
#define RFID_DEV_BAUDRATE_19200                      19200           /* 射频识别设备波特率 19200 */
#define RFID_DEV_BAUDRATE_38400                      38400           /* 射频识别设备波特率 38400 */
#define RFID_DEV_BAUDRATE_57600                      57600           /* 射频识别设备波特率 57600 */
#define RFID_DEV_BAUDRATE_76800                      76800           /* 射频识别设备波特率 76800 */
#define RFID_DEV_BAUDRATE_115200                     115200          /* 射频识别设备波特率 115200 */
#define RFID_DEV_BAUDRATE_460800                     460800          /* 射频识别设备波特率 460800 */
#define RFID_DEV_BAUDRATE_921600                     921600          /* 射频识别设备波特率 921600 */
#define RFID_DEV_BAUDRATE_100000                     100000          /* 射频识别设备波特率 100000 */
#define RFID_DEV_BAUDRATE_1000000                    1000000         /* 射频识别设备波特率 1000000 */
#define RFID_DEV_BAUDRATE_3000000                    3000000         /* 射频识别设备波特率 3000000 */

#define RFID_DEV_CTRL_BAUDRATE                       0x00            /** 射频识别设备参数控制：修改波特率 */
#define RFID_DEV_CTRL_HARDRESET                      0x01            /** 射频识别设备参数控制：硬复位 */

/**************************************************************
 * 函数名        rfid_dev_init
 * 功能            射频识别设备初始化
 * 参数
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_hardware_init(void);

/**************************************************************
 * 函数名        take_rfid_dev_data_sem
 * 功能            获取设备串口数据信号量
 * 参数            timeout   等待信号量时长(ms)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int take_rfid_dev_data_sem(unsigned int timeout);

/**************************************************************
 * 函数名        reset_rfid_dev_data_sem
 * 功能            复位设备串口数据信号量
 * 参数
 * 返回            0
 *************************************************************/
int reset_rfid_dev_data_sem(void);

/**************************************************************
 * 函数名        rfid_dev_ctrl
 * 功能            控制设备、修改设备参数
 * 参数            cmd     控制指令
 *       para    控制参数
 *       plen    控制参数长度(B)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_hardware_ctrl(unsigned char cmd, void *para, unsigned char plen);

/**************************************************************
 * 函数名        rfid_dev_send
 * 功能            向设备串口发送数据
 * 参数            data   数据
 *       len    数据长度(B)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_send(void *data, unsigned int len);

/**************************************************************
 * 函数名        rfid_dev_recv
 * 功能            从设备串口读取数据
 * 参数            buf     数据存放缓存
 *       len     缓存长度(B)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_recv(void *buf, unsigned int len);

#endif /* RFID_READER_RFID_DEV_INC_RFID_DEV_HARDWARE_H_ */
