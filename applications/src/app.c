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

#include <rtthread.h>

#include "app.h"
#include "app_ofsm.h"
#include "app_osupport.h"
#include "app_card.h"
#include "app_led.h"
#include "app_hci.h"
#include "app_terminal.h"

#include "mw_fault_check.h"
#include "mw_charge_control.h"

#define DBG_TAG "app"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**************************************************************************/
static struct rt_thread card_thread;
static rt_uint8_t card_thread_stack[2048];

static uint8_t s_thread_gunno[APP_SYSTEM_GUNNO_SIZE];
static struct rt_thread ofsm_thread[APP_SYSTEM_GUNNO_SIZE];
static rt_uint8_t ofsm_thread_stack[APP_SYSTEM_GUNNO_SIZE][6144];

static struct rt_thread osupport_thread;
static rt_uint8_t osupport_thread_stack[4096];

static struct rt_thread led_thread;
static rt_uint8_t led_thread_stack[512];

static struct rt_thread hci_req_thread;
static rt_uint8_t hci_req_thread_stack[4096];

static struct rt_thread hci_res_thread;
static rt_uint8_t hci_res_thread_stack[512];

static struct rt_thread terminal_thread;
static rt_uint8_t terminal_thread_stack[2048];

/**************************************************************************/

void app_led_init(void)
{
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&led_thread, "task_led",
            app_led_thread_entry, RT_NULL, &led_thread_stack, sizeof(led_thread_stack), 18, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&led_thread);
    }
}

void app_hci_init(void)
{
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&hci_req_thread, "task_hci_req",
            app_hci_req_thread_entry, RT_NULL, &hci_req_thread_stack, sizeof(hci_req_thread_stack), 13, 10);  // 18
    if (RT_EOK == result) {
        rt_thread_startup(&hci_req_thread);
    }

    result = rt_thread_init(&hci_res_thread, "task_hci_res",
            app_hci_res_thread_entry, RT_NULL, &hci_res_thread_stack, sizeof(hci_res_thread_stack), 13, 10);  // 18
    if (RT_EOK == result) {
        rt_thread_startup(&hci_res_thread);
    }
}


void app_operation_init(void)
{
    rt_err_t result = RT_EOK;

    char task_name[10];
    memset(task_name, '\0', sizeof(task_name));
    memcpy(task_name, "tofsm_0", strlen("tofsm_0"));

    ofsm_fun_list_init();

    for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        sprintf((task_name + strlen("tofsm_0") - 1), "%u", gunno);
        s_thread_gunno[gunno] = gunno;
        result = rt_thread_init(&ofsm_thread[gunno], task_name,
                ofsm_thread_entry, &s_thread_gunno[gunno], &ofsm_thread_stack[gunno], sizeof(ofsm_thread_stack[gunno]), 15, 10);
        if (RT_EOK == result) {

            rt_err_t err = rt_thread_startup(&ofsm_thread[gunno]);
            rt_kprintf("app_operation_init(%d, %d)\n", gunno, err);
        }
    }
}

void app_terminal_init(void)
{
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&terminal_thread, "task_terminal",
            terminal_thread_entry, RT_NULL, terminal_thread_stack, sizeof(terminal_thread_stack), 19, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&terminal_thread);
    }
}

void app_init(void)
{
    /**************************************************************************/

    rt_err_t result = RT_EOK;

    result = rt_thread_init(&osupport_thread, "task_osupport",
            app_osupport_thread_entry, RT_NULL, &osupport_thread_stack, sizeof(osupport_thread_stack), 16, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&osupport_thread);
    }

    result = rt_thread_init(&card_thread, "task_card",
            card_thread_entry, RT_NULL, &card_thread_stack, sizeof(card_thread_stack), 14, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&card_thread);
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
