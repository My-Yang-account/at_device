/**
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include <rtthread.h>

#include "app_hci.h"
#include "bsp_serial.h"
#include "app.h"
#include "serialScreen.h"

#define DBG_TAG "app.hci"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

APP_DEF_SRAM1 static uint8_t s_hci_event[APP_SYSTEM_GUNNO_SIZE] = {0};
APP_DEF_SRAM1 static struct LCD_DATA_FIFO_TYPE *p_hci_data_info;

#ifdef APP_DESIGNATE_REGION
/*************************************
 * 函数名       app_hci_info_init
 * 功能           屏幕信息、变量初始化
 * 参数
 * 返回
 ************************************/
void app_hci_info_init(void)
{
    for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        s_hci_event[gunno] = 0x00;
    }
    p_hci_data_info = NULL;
}
#endif /* APP_DESIGNATE_REGION */

uint8_t app_get_hci_event(uint8_t gunno, uint16_t event, uint8_t is_clear)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0;
    }

    if(s_hci_event[gunno] &(1 <<event)){
        if(is_clear){
            s_hci_event[gunno] &= (~(1 <<event));
        }
        return 1;
    }
    if(is_clear){
        s_hci_event[gunno] &= (~(1 <<event));
    }
    return 0;
}

void app_set_hci_event(uint8_t gunno, uint16_t event)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    s_hci_event[gunno] |= (1 <<event);
}

void app_hci_res_thread_entry(void *parameter)
{
    (void)parameter;

    uint8_t ch = 0x00;
    uint16_t queue_size = 0x00;
    struct LCD_DATA_FIFO_TYPE *ptr = NULL;

    while(NULL == p_hci_data_info)
    {
        rt_thread_mdelay(10);
    }

    ptr = p_hci_data_info;
    queue_size = sizeof(ptr->buffer);

    while (1)
    {
        while(rt_device_read(g_hci_serial, 0, &ch, 1) != 1){
            rt_sem_take(&g_hci_rx_sem, RT_WAITING_FOREVER);
        }

        rt_enter_critical();

        (*ptr).buffer[(*ptr).put_index] = ch;

        (*ptr).put_index += 1;

        if((*ptr).put_index >= queue_size){
            (*ptr).put_index = 0;
        }

        if((*ptr).put_index == (*ptr).get_index){
            (*ptr).get_index += 1;
            LOG_E("hci queue is full, please get data in time|%d", queue_size);
        }

        rt_exit_critical();
    }
}

void app_hci_req_thread_entry(void *parameter)
{
    (void)parameter;


    /* 调取屏幕初始化函数 */
    p_hci_data_info = serialScreen_ObjectAi_Init();

    if(p_hci_data_info == NULL){
        LOG_E("hci info init fail, please check,");
        return;
    }

    rt_thread_mdelay(5000);   /** 等待屏幕初始化完成 */

    while (1)
    {
        /* 调取屏幕处理函数 */
        serialScreen_ObjectAi_main();
        rt_thread_mdelay(10);
        
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
