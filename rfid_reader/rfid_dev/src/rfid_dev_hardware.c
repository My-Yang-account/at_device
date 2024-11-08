/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#include "rfid_dev_hardware.h"
#include "board.h"

#define DBG_TAG "rfidhard"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//#define RFIDR_HARDWARE_DEBUG

static struct rt_semaphore s_rfid_dev_sem;
static rt_device_t s_rfid_dev;

/**************************************************************
 * 函数名        rfid_dev_input
 * 功能            设备串口数据中断回调函数
 * 参数            dev    串口设备句柄
 *       size   数据大小(B)
 * 返回            0
 *************************************************************/
static rt_err_t rfid_dev_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&s_rfid_dev_sem);
    return 0;
}

/**************************************************************
 * 函数名        take_rfid_dev_data_sem
 * 功能            获取设备串口数据信号量
 * 参数            timeout   等待信号量时长(ms)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int take_rfid_dev_data_sem(unsigned int timeout)
{
    return rt_sem_take(&s_rfid_dev_sem, timeout);
}

/**************************************************************
 * 函数名        reset_rfid_dev_data_sem
 * 功能            复位设备串口数据信号量
 * 参数
 * 返回            0
 *************************************************************/
int reset_rfid_dev_data_sem(void)
{
    while(rt_sem_take(&s_rfid_dev_sem, 0x00) == RT_EOK);
    return 0x00;
//    return rt_sem_control(&s_rfid_dev_sem, RT_IPC_CMD_RESET, NULL);   /* 使用这个函数时会将线程错误码置为 -1，再次获取该信号量时会直接退出 */
}

/**************************************************************
 * 函数名        rfid_dev_send
 * 功能            向设备串口发送数据
 * 参数            data   数据
 *       len    数据长度(B)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_send(void *data, unsigned int len)
{
    if(data == NULL || len == 0x00){
        return 0x00;
    }

#ifdef RFIDR_HARDWARE_DEBUG
    LOG_D("rfid_dev_send(%d)\n", len);
    for(unsigned char i = 0; i < len; i++){
        rt_kprintf("%02X ", *((unsigned char*)data + i));
    }
    rt_kprintf("\n");
#endif /* RFIDR_HARDWARE_DEBUG */

    return rt_device_write(s_rfid_dev, 0x00, data, len);
}

/**************************************************************
 * 函数名        rfid_dev_recv
 * 功能            从设备串口读取数据
 * 参数            buf     数据存放缓存
 *       len     缓存长度(B)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_recv(void *buf, unsigned int len)
{
    if(buf == NULL || len == 0x00){
        return 0x00;
    }

    return rt_device_read(s_rfid_dev, 0x00, buf, len);
}

/**************************************************************
 * 函数名        rfid_dev_hardware_ctrl
 * 功能            控制设备、修改设备参数
 * 参数            cmd     控制指令
 *       para    控制参数
 *       plen    控制参数长度(B)
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_hardware_ctrl(unsigned char cmd, void *para, unsigned char plen)
{
    switch(cmd){
    case RFID_DEV_CTRL_BAUDRATE:
    {
        unsigned int baudrate = *((uint32_t*)(para));
        struct serial_configure serial_para = RFID_DEV_SERIAL_CONFIG_DEFAULT;

        serial_para.baud_rate = baudrate;
        if(rt_device_control(s_rfid_dev, RT_DEVICE_CTRL_CONFIG, &serial_para) != RT_EOK){
            LOG_E("RFID device modify baudrate fail, please check(%d)!", baudrate);
            return -0x02;
        }
        break;
    }
    case RFID_DEV_CTRL_HARDRESET:
        break;
    default:
        break;
    }

    return 0x00;
}

/**************************************************************
 * 函数名        rfid_dev_hardware_init
 * 功能            射频识别设备初始化
 * 参数
 * 返回            >=0：成功   <0：失败
 *************************************************************/
int rfid_dev_hardware_init(void)
{
    /**** 寻找串口设备 ****/
    s_rfid_dev = rt_device_find(RFID_DEV_NAME);
    if(s_rfid_dev == NULL){
        LOG_E("rfid device find fail, please check!");
        return -1;
    }

    /**** 配置串口参数 ****/
    struct serial_configure serial_para = RFID_DEV_SERIAL_CONFIG_DEFAULT;
    serial_para.baud_rate = BAUD_RATE_115200;
    serial_para.data_bits = DATA_BITS_8;
    serial_para.stop_bits = STOP_BITS_1;
    serial_para.bufsz     = 256;
    serial_para.parity    = PARITY_NONE;

    if(rt_device_control(s_rfid_dev, RT_DEVICE_CTRL_CONFIG, &serial_para) != RT_EOK){
        LOG_E("rfid device para config fail, please check!");
        return -0x02;
    }

    /**** 以中断接收方式打开串口设备 ****/
    if(rt_device_open(s_rfid_dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK){
        LOG_E("rfid device open fail, please check!");
        return -0x03;
    }
    /**** 初始化数据接收信号量 ****/
    rt_sem_init(&s_rfid_dev_sem, "rfid_sem", 0x00, RT_IPC_FLAG_FIFO);
    /**** 设置串口接收回调函数 ****/
    rt_device_set_rx_indicate(s_rfid_dev, rfid_dev_input);

    return 0x00;
}
