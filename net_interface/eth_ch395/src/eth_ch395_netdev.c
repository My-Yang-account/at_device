/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#include "eth_ch395_netdev.h"
#include "board.h"

#define DBG_TAG "ethdev"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define ETHCH395_RST_PIN      GET_PIN(A, 3)
#define ETHCH395_CFG_PIN      GET_PIN(A, 2)

static uint32_t s_ethch395_irq_notice = 0x00;
static struct rt_semaphore s_ethch395_netdev_sem;
static rt_device_t s_ethch395_netdev;

/**--------------------------------------------------
                  [串口中断回调函数]
 -------------------------------------------------*/
static rt_err_t ethch395_netdev_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&s_ethch395_netdev_sem);
    return 0;
}

/**--------------------------------------------------
                  [管脚中断回调函数]
 -------------------------------------------------*/
static void ethch395_irq_callback(void *args)
{
    s_ethch395_irq_notice++;
}

/**--------------------------------------------------
                  [串口数据信号量]
 -------------------------------------------------*/
int32_t take_ethch395_data_sem(uint32_t timeout)
{
    return rt_sem_take(&s_ethch395_netdev_sem, timeout);
}

/**--------------------------------------------------
                  [复位串口数据信号量]
 -------------------------------------------------*/
int32_t reset_ethch395_data_sem(void)
{
    while(rt_sem_take(&s_ethch395_netdev_sem, 0) == RT_EOK);
    return 0x00;
//    return rt_sem_control(&s_ethch395_netdev_sem, RT_IPC_CMD_RESET, NULL);   /* 使用这个函数时会将线程错误码置为 -1，再次获取该信号量时会直接退出 */
}

/**--------------------------------------------------
                  [获取管脚中断状态]
 -------------------------------------------------*/
uint8_t is_ethch395_irq_coming(void)
{
    if(rt_pin_read(ETHCH395_CFG_PIN) == 0x00){
        return 0x01;
    }

    return 0x00;
}

/**--------------------------------------------------
                  [网络设备发送函数]
 -------------------------------------------------*/
int32_t ethch395_netdev_send(void *data, uint32_t len)
{
    if(data == NULL || len == 0){
        return 0x00;
    }

    rt_kprintf("send:%02X\n", *((uint8_t*)data + 0));

    return rt_device_write(s_ethch395_netdev, 0, data, len);
}

/**--------------------------------------------------
                  [网络设备接收函数]
 -------------------------------------------------*/
int32_t ethch395_netdev_recv(void *buf, uint32_t len)
{
    if(buf == NULL || len == 0){
        return 0x00;
    }

    return rt_device_read(s_ethch395_netdev, 0, buf, len);
}

/**--------------------------------------------------
                  [网络设备控制函数]
 -------------------------------------------------*/
int32_t ethch395_netdev_ctrl(uint8_t cmd, void *para, uint8_t plen)
{
    switch(cmd){
    case ETHCH395_NETDEV_CTRL_BAUDRATE:
    {
        uint32_t baudrate = *((uint32_t*)(para));
        struct serial_configure serial_para = NET_ETHERNET_SERIAL_CONFIG_DEFAULT;

        serial_para.baud_rate = baudrate;
        if(rt_device_close(s_ethch395_netdev) != RT_EOK){
            LOG_E("ethch395 net device close, please check!");
            return -0x01;
        }

        if(rt_device_open(s_ethch395_netdev, RT_DEVICE_FLAG_DMA_RX |RT_DEVICE_FLAG_RDWR) != RT_EOK){
            LOG_E("ethch395 net device open fail, please check!");
            return -0x01;
        }

        if(rt_device_control(s_ethch395_netdev, RT_DEVICE_CTRL_CONFIG, &serial_para) != RT_EOK){
            LOG_E("ethch395 net device modify baudrate fail, please check(%d)!", baudrate);
            return -0x01;
        }
        break;
    }
    case ETHCH395_NETDEV_CTRL_HARDRESET:
    {
        rt_pin_write(ETHCH395_RST_PIN,PIN_LOW);                                                   /* 硬件复位 */
        rt_thread_mdelay(10);
        rt_pin_write(ETHCH395_RST_PIN,PIN_HIGH);
        rt_thread_mdelay(500);
    }
        break;
    default:
        break;
    }

    return 0x00;
}

/**--------------------------------------------------
                  [网络设备初始化函数]
 -------------------------------------------------*/
int32_t ethch395_netdev_init(void)
{
    /**** 寻找串口设备 ****/
    s_ethch395_netdev = rt_device_find(NET_ETHERNET_NETDEV_NAME);
    if(s_ethch395_netdev == NULL){
        LOG_E("ethch395 net device find fail, please check!");
        return -1;
    }

    /**** 配置串口参数 ****/
    struct serial_configure serial_para = NET_ETHERNET_SERIAL_CONFIG_DEFAULT;

    if(rt_device_control(s_ethch395_netdev, RT_DEVICE_CTRL_CONFIG, &serial_para) != RT_EOK){
        LOG_E("ethch395 net device para config fail, please check!");
        return -2;
    }

    /**** 以中断接收方式打开串口设备 ****/
    if(rt_device_open(s_ethch395_netdev, RT_DEVICE_FLAG_DMA_RX |RT_DEVICE_FLAG_RDWR) != RT_EOK){
        LOG_E("ethch395 net device open fail, please check!");
        return -3;
    }
    /**** 初始化数据接收信号量 ****/
    rt_sem_init(&s_ethch395_netdev_sem, "ethch395_sem", 0, RT_IPC_FLAG_FIFO);
    /**** 设置串口接收回调函数 ****/
    rt_device_set_rx_indicate(s_ethch395_netdev, ethch395_netdev_input);

    rt_pin_mode(ETHCH395_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(ETHCH395_CFG_PIN, PIN_MODE_INPUT_PULLUP);
#if 0
    rt_pin_attach_irq(ETHCH395_CFG_PIN, PIN_IRQ_MODE_FALLING, ethch395_irq_callback, RT_NULL);/* 绑定中断，下升沿模式 */
    rt_pin_irq_enable(ETHCH395_CFG_PIN, PIN_IRQ_ENABLE);                                      /* 使能中断 */
#endif

    rt_pin_write(ETHCH395_RST_PIN,PIN_LOW);                                                   /* 硬件复位 */
    rt_thread_mdelay(10);
    rt_pin_write(ETHCH395_RST_PIN,PIN_HIGH);
    rt_thread_mdelay(500);

    return 0x00;
}
