/**
  ******************************************************************************
  * @file
  * @author
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
#include <string.h>

#include "bsp_serial.h"
#include "app.h"

#define DBG_TAG "bsp.serial"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef USING_DOUBLE_GUN

#define TERMINAL_UART_NAME   "uart3"        /* 终端串口设备名称 */
static struct serial_configure s_terminal_config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
#define HCI_UART_NAME        "uart4"        /* 屏幕串口设备名称 */
static struct serial_configure s_hci_config = RT_SERIAL_CONFIG_DEFAULT;       /* 初始化配置参数 */
#define READER_UART_NAME      "uart2"        /* 读卡器串口设备名称 */
static struct serial_configure s_reader_config = RT_SERIAL_CONFIG_DEFAULT;       /* 初始化配置参数 */

#else

#define TERMINAL_UART_NAME   "uart3"        /* 终端串口设备名称 */
static struct serial_configure s_terminal_config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */
#define HCI_UART_NAME        "uart2"        /* 屏幕串口设备名称 */
static struct serial_configure s_hci_config = RT_SERIAL_CONFIG_DEFAULT;       /* 初始化配置参数 */
#define READER_UART_NAME      "uart6"        /* 读卡器串口设备名称 */
static struct serial_configure s_reader_config = RT_SERIAL_CONFIG_DEFAULT;       /* 初始化配置参数 */

#endif /* USING_DOUBLE_GUN */

rt_device_t g_terminal_serial = NULL;
struct rt_semaphore g_terminal_rx_sem;
rt_device_t g_hci_serial = NULL;
struct rt_semaphore g_hci_rx_sem;
rt_device_t g_reader_serial = NULL;
struct rt_semaphore g_reader_rx_sem;

static rt_err_t terminal_uart_input(rt_device_t dev, rt_size_t size)
{
    if (size > 0) {
        rt_sem_release(&g_terminal_rx_sem);
    }

    return RT_EOK;
}

static rt_err_t hci_uart_input(rt_device_t dev, rt_size_t size)
{
    if (size > 0) {
        rt_sem_release(&g_hci_rx_sem);
    }

    return RT_EOK;
}

static rt_err_t reader_uart_input(rt_device_t dev, rt_size_t size)
{
    if (size > 0) {
        rt_sem_release(&g_reader_rx_sem);
    }

    return RT_EOK;
}

int32_t bsp_terminal_serial_init(void)
{
    /* step1：查找串口设备 */
    g_terminal_serial = rt_device_find(TERMINAL_UART_NAME);
    if (NULL == g_terminal_serial) {
        LOG_D("find terminal device failed");
        return -1;
    }

    /* step2：修改串口配置参数 */
    s_terminal_config.baud_rate = BAUD_RATE_115200;
    s_terminal_config.data_bits = DATA_BITS_8;
    s_terminal_config.stop_bits = STOP_BITS_1;
    s_terminal_config.bufsz     = 256;
    s_terminal_config.parity    = PARITY_NONE;

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    if (RT_EOK != rt_device_control(g_terminal_serial, RT_DEVICE_CTRL_CONFIG, &s_terminal_config)) {
        LOG_D("control terminal device failed");
        return -1;
    }

    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    if (RT_EOK != rt_device_open(g_terminal_serial, RT_DEVICE_FLAG_INT_RX)) {
        LOG_D("open terminal device failed");
        return -1;
    }

    /* 初始化信号量 */
    if (RT_EOK != rt_sem_init(&g_terminal_rx_sem, "t_sem", 0, RT_IPC_FLAG_PRIO)) {
        LOG_D("Initialization semaphore terminal device failed");
        return -1;
    }

    /* 设置接收回调函数 */
    if (RT_EOK != rt_device_set_rx_indicate(g_terminal_serial, terminal_uart_input)) {
        LOG_D("set terminal device callback failed");
        return -1;
    }

    return 0;
}

int32_t bsp_hci_serial_init(void)
{
    /* step1：查找串口设备 */
    g_hci_serial = rt_device_find(HCI_UART_NAME);
    if (NULL == g_hci_serial)
    {
        LOG_D("find hci device failed");
        return -1;
    }

    /* step2：修改串口配置参数 */
    s_hci_config.baud_rate = BAUD_RATE_115200;
    s_hci_config.data_bits = DATA_BITS_8;
    s_hci_config.stop_bits = STOP_BITS_1;
    s_hci_config.bufsz     = 512;
    s_hci_config.parity    = PARITY_NONE;

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    if (RT_EOK != rt_device_control(g_hci_serial, RT_DEVICE_CTRL_CONFIG, &s_hci_config)) {
        LOG_D("control hci device failed");
        return -1;
    }

    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    if (RT_EOK != rt_device_open(g_hci_serial, RT_DEVICE_FLAG_INT_RX)) {
        LOG_D("open hci device failed");
        return -1;
    }

    /* 初始化信号量 */
    if (RT_EOK != rt_sem_init(&g_hci_rx_sem, "hci_sem", 0, RT_IPC_FLAG_PRIO)) {
        LOG_D("Initialization semaphore hci device failed");
        return -1;
    }

    /* 设置接收回调函数 */
    if (RT_EOK != rt_device_set_rx_indicate(g_hci_serial, hci_uart_input)) {
        LOG_D("set hci device callback failed");
        return -1;
    }

    return 0;
}

int32_t bsp_reader_serial_init(void)
{
    /* step1：查找串口设备 */
    g_reader_serial = rt_device_find(READER_UART_NAME);
    if (NULL == g_reader_serial) {
        LOG_D("find reader device failed|%s", READER_UART_NAME);
        return -1;
    }

    /* step2：修改串口配置参数 */
    s_reader_config.baud_rate = BAUD_RATE_115200;
    s_reader_config.data_bits = DATA_BITS_8;
    s_reader_config.stop_bits = STOP_BITS_1;
    s_reader_config.bufsz     = 256;
    s_reader_config.parity    = PARITY_NONE;

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    if (RT_EOK != rt_device_control(g_reader_serial, RT_DEVICE_CTRL_CONFIG, &s_reader_config)) {
        LOG_D("control reader device failed");
        return -1;
    }

    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    if (RT_EOK != rt_device_open(g_reader_serial, RT_DEVICE_FLAG_INT_RX)) {
        LOG_D("open reader device failed");
        return -1;
    }

    /* 初始化信号量 */
    if (RT_EOK != rt_sem_init(&g_reader_rx_sem, "reader_sem", 0, RT_IPC_FLAG_PRIO)) {
        LOG_D("Initialization semaphore reader device failed");
        return -1;
    }

    /* 设置接收回调函数 */
    if (RT_EOK != rt_device_set_rx_indicate(g_reader_serial, reader_uart_input)) {
        LOG_D("set reader device callback failed");
        return -1;
    }

    return 0;
}


void terminal_send_data(char *data)
{
    rt_device_write(g_terminal_serial, 0, data, strlen(data));
}

void hci_send_data(uint8_t *data, uint8_t len)
{
    rt_device_write(g_hci_serial, 0, data, len);
}

void reader_send_data(uint8_t *data, uint8_t len)
{
    rt_device_write(g_reader_serial, 0, data, len);
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
