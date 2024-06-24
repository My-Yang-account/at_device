/**
  ******************************************************************************
  * @file
  * @author
  * @brief 前置
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#include <rtthread.h>

#include "prepose.h"
#include "chargepile_config.h"

#include "bsp_serial.h"

#include "app.h"
#include "app_osupport.h"
#include "app_hci.h"
#include "app_card.h"
#include "app_flash.h"

#include "mw_fault_check.h"
#include "mw_led.h"
#include "mw_storage.h"
#include "mw_iwdg.h"

#define DBG_TAG "prepose"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

void prepose_init(void)
{
    /* 对指示灯进行初始化 */
    mw_led_init();

    app_flash_ipc_init();

    rt_thread_mdelay(1000);
    /* 对HCI串口进行初始化 */
    while (1)
    {
        if(0 > bsp_hci_serial_init())
        {
            LOG_E("hci serial initialize failed, please check hci serial!");
            rt_thread_mdelay(1000);
        }
        else
        {
            LOG_D("hci serial initialize success");
            break;
        }
    }

    mw_iwdg_refresh();

    /* 对读卡器串口进行初始化 */
    while (1)
    {
        if(0 > bsp_reader_serial_init())
        {
            LOG_E("reader serial initialize failed, please check reader serial!");
            rt_thread_mdelay(1000);
        }
        else
        {
            LOG_D("reader serial initialize success");
            break;
        }
    }

    buzzer_ipc_init();

    app_led_init();

    while (1) {
        if(0 > chargepile_config_init()) {
            rt_thread_mdelay(1000);
            mw_iwdg_refresh();

        } else {
            LOG_D("configure initialize success");
            break;
        }
    }

    while (1) {
        if(0 > chargepile_check_config()) {
            rt_thread_mdelay(3000);
            break;
        } else {
            break;
        }
    }

    rt_thread_mdelay(1000);

    while (1) {
        if(0 > mw_storage_init()) {
            rt_thread_mdelay(100);
            mw_iwdg_refresh();
//            LOG_D("record initialize fail");
        } else {
            LOG_D("record initialize success");
            break;
        }
    }

    app_operation_init();

    /**************************************************************************/
    /* 对终端串口进行初始化 */
//    while (1) {
//        if(0 > bsp_terminal_serial_init()) {
//            LOG_E("terminal serial initialize failed, please check terminal serial!");
//            break;
//        } else {
//            LOG_D("terminal serial initialize success");
//            break;
//        }
//    }
//
//    /* 对终端的线程进行初始化 */
//    app_terminal_init();

    mw_iwdg_refresh();
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
