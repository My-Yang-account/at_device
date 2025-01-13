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
#include "chargepile_config.h"

#define DBG_TAG "app"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**************************************************************************/
#ifdef USING_THREAD_MONITOR
#define APP_THREAD_MONITOR_DEBUG
#define APP_THREAD_ENTRY_PERIOD         15000   /** 线程不正常超过一定时长(ms)，容忍次数加1(这个时间需要参考看门狗复位时间) */

#ifdef CP_CONFIG_USING_DUPU
#define APP_APPLICATION_THREAD_MAX      20      /** 应用线程数量 */
#else
#define APP_APPLICATION_THREAD_MAX      18      /** 应用线程数量 */
#endif /* CP_CONFIG_USING_DUPU */

#pragma pack(1)

/** 线程监控 */
typedef struct{
    void *thread;                            /** 线程句柄 */
    uint8_t index;                           /** 线程下标(用于确定当前需要处理的线程) */
    uint8_t rentry;                          /** 容忍失误次数 */
    char name[8];                            /** 线程名字 */
}thread_moniotr_node;                        /** 监控节点 */

typedef struct{
    uint32_t tick;
    uint8_t is_error;
    uint8_t thread_num;                      /** 被监控的线程数量 */
    uint8_t current_index;                   /** 当前线程下标 */
    thread_moniotr_node node[APP_APPLICATION_THREAD_MAX];
}thread_moniotr;

#pragma pack()
#endif /* USING_THREAD_MONITOR */

/**************************************************************************/
APP_DEF_SRAM1 static uint8_t s_thread_gunno[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static struct rt_thread ofsm_thread[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM0 static rt_uint8_t ofsm_thread_stack[APP_SYSTEM_GUNNO_SIZE][6144];

APP_DEF_SRAM1 static struct rt_thread osupport_thread;
APP_DEF_SRAM0 static rt_uint8_t osupport_thread_stack[4096];

APP_DEF_SRAM1 static struct rt_thread led_thread;
APP_DEF_SRAM2 static rt_uint8_t led_thread_stack[512];

APP_DEF_SRAM1 static struct rt_thread hci_req_thread;
APP_DEF_SRAM0 static rt_uint8_t hci_req_thread_stack[4096];

APP_DEF_SRAM1 static struct rt_thread hci_res_thread;
APP_DEF_SRAM0 static rt_uint8_t hci_res_thread_stack[512];

#ifdef CP_CONFIG_USING_DUPU
APP_DEF_SRAM1 static struct rt_thread terminal_thread;
APP_DEF_SRAM0 static rt_uint8_t terminal_thread_stack[2048];

APP_DEF_SRAM1 static struct rt_thread terminal_req_thread;
APP_DEF_SRAM0 static rt_uint8_t terminal_req_thread_stack[1024];
#endif /* CP_CONFIG_USING_DUPU */

#ifdef USING_THREAD_MONITOR
APP_DEF_SRAM2 static thread_moniotr s_thread_moniotr;
#endif /* USING_THREAD_MONITOR */
/**************************************************************************/

#ifdef APP_DESIGNATE_REGION
/*************************************
 * 函数名       app_application_info_init
 * 功能           应用信息、变量初始化
 * 参数
 * 返回
 ************************************/
void app_application_info_init(void)
{
#ifdef USING_THREAD_MONITOR
    memset(&s_thread_moniotr, 0x00, sizeof(s_thread_moniotr));
#endif /* USING_THREAD_MONITOR */
}
#endif /* APP_DESIGNATE_REGION */

/******************************************
 * 函数名     app_thread_monitor_add
 * 功能         应用线程监控添加
 * 参数         thread   线程句柄
 *      entry    最大容忍次数
 * 返回         >=0：成功      < 0：失败
 * ***************************************/
int32_t app_thread_monitor_add(void *thread, void *para, uint32_t plen, uint32_t option)
{
#ifdef USING_THREAD_MONITOR
    if(thread == NULL){
        return -0x01;
    }
    uint8_t count = 0x00;

    rt_enter_critical();

    for(count = 0x00; count < s_thread_moniotr.thread_num; count++){
        if(s_thread_moniotr.node[count].thread == thread){
            if((option &APP_THREAD_MONITOR_OPT_ENTRY) && para){
                uint8_t entry = *(uint8_t*)para;
                s_thread_moniotr.node[count].rentry = entry;
            }else if((option &APP_THREAD_MONITOR_OPT_NAME) && para){
                memset(s_thread_moniotr.node[count].name, 0x00, sizeof(s_thread_moniotr.node[count].name));
                if(plen > sizeof(s_thread_moniotr.node[count].name)){
                    memcpy(s_thread_moniotr.node[count].name, para, sizeof(s_thread_moniotr.node[count].name));
                }else{
                    memcpy(s_thread_moniotr.node[count].name, para, plen);
                }
            }
            break;
        }
    }
    if(count >= s_thread_moniotr.thread_num){
        if(s_thread_moniotr.thread_num >= APP_APPLICATION_THREAD_MAX){
            return -0x01;
        }

        if((option &APP_THREAD_MONITOR_OPT_ENTRY) && para){
            uint8_t entry = *(uint8_t*)para;
            s_thread_moniotr.node[s_thread_moniotr.thread_num].rentry = entry;
        }else if((option &APP_THREAD_MONITOR_OPT_NAME) && para){
            memset(s_thread_moniotr.node[s_thread_moniotr.thread_num].name, 0x00, sizeof(s_thread_moniotr.node[s_thread_moniotr.thread_num].name));
            if(plen > sizeof(s_thread_moniotr.node[s_thread_moniotr.thread_num].name)){
                memcpy(s_thread_moniotr.node[s_thread_moniotr.thread_num].name, para, sizeof(s_thread_moniotr.node[s_thread_moniotr.thread_num].name));
            }else{
                memcpy(s_thread_moniotr.node[s_thread_moniotr.thread_num].name, para, plen);
            }
        }
        s_thread_moniotr.node[s_thread_moniotr.thread_num].thread = thread;
        s_thread_moniotr.node[s_thread_moniotr.thread_num].index = s_thread_moniotr.thread_num;

        if(++s_thread_moniotr.thread_num >= APP_APPLICATION_THREAD_MAX){
            s_thread_moniotr.thread_num = APP_APPLICATION_THREAD_MAX;
        }
    }
    rt_exit_critical();

    return 0x00;
#else
    return -0x01;
#endif /* USING_THREAD_MONITOR */
}

/******************************************
 * 函数名     app_thread_monitor_remove
 * 功能         应用线程监控移除
 * 参数         thread   线程句柄
 * 返回         >=0：成功      < 0：失败
 * ***************************************/
int32_t app_thread_monitor_remove(void *thread)
{
#ifdef USING_THREAD_MONITOR
    if((s_thread_moniotr.thread_num == 0x00) || (thread == NULL)){
        return -0x01;
    }

    uint8_t pos = 0x00, count = 0x00;

    rt_enter_critical();

    for(pos = 0x00; pos < s_thread_moniotr.thread_num; pos++){
        if(s_thread_moniotr.node[pos].thread == thread){
#ifdef APP_THREAD_MONITOR_DEBUG
            LOG_D("thread node remove(%X, %s)\n", thread, s_thread_moniotr.node[pos].name);
#endif /* APP_THREAD_MONITOR_DEBUG */
            break;
        }
    }
    if(pos < s_thread_moniotr.thread_num){
        memset(&s_thread_moniotr.node[pos], 0x00, sizeof(thread_moniotr_node));
        for(count = pos; count < (s_thread_moniotr.thread_num - 0x01); count++){
            memcpy(&s_thread_moniotr.node[count], &s_thread_moniotr.node[count + 0x01], sizeof(thread_moniotr_node));
        }
        s_thread_moniotr.thread_num--;
        rt_exit_critical();
        return 0x00;
    }

    rt_exit_critical();
#endif /* USING_THREAD_MONITOR */
    return -0x01;
}

/******************************************
 * 函数名     app_thread_monitor_process
 * 功能         应用线程监控总处理
 * 参数         thread   线程句柄
 *      para     可选参数
 *      plen     参数长度(B)
 *      option   选项字
 * 返回         1：正常       0：异常      -1：错误
 * ***************************************/
int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option)
{
#ifdef USING_THREAD_MONITOR
    extern void thaisen_clear_screen_reboot(void);
    extern void thaisen_set_screen_reboot(void);
    extern void mw_iwdg_refresh(void);

    static uint8_t entry = 0x00;

    rt_enter_critical();
    /** 变量安全处理 */
    if(s_thread_moniotr.thread_num > APP_APPLICATION_THREAD_MAX){
        s_thread_moniotr.thread_num = APP_APPLICATION_THREAD_MAX;
    }
    /** 如果是紧急事件或没有线程被监控，直接按正常进行处理 */
    if((option &APP_THREAD_MONITOR_OPT_URGENT) || (s_thread_moniotr.thread_num == 0x00)){
        s_thread_moniotr.tick = rt_tick_get();
        s_thread_moniotr.is_error = 0x00;
        entry = 0x00;
        /** 喂狗 */
        mw_iwdg_refresh();
        /** 清除重启事件 */
        thaisen_clear_screen_reboot();
    }else{
        uint8_t count = 0x00;
        for(count = 0x00; count < s_thread_moniotr.thread_num; count++){
            if((thread == s_thread_moniotr.node[count].thread) &&
                    (s_thread_moniotr.current_index == s_thread_moniotr.node[count].index)){
                /** 喂狗 */
                mw_iwdg_refresh();
                /** 清除重启事件 */
                thaisen_clear_screen_reboot();

                s_thread_moniotr.tick = rt_tick_get();
                s_thread_moniotr.is_error = 0x00;
                entry = 0x00;
                if(++s_thread_moniotr.current_index >= s_thread_moniotr.thread_num){
                    s_thread_moniotr.current_index = 0x00;
                }
                break;
            }
        }
        if(count >= s_thread_moniotr.thread_num){
            if(s_thread_moniotr.tick > rt_tick_get()){
                s_thread_moniotr.tick = rt_tick_get();
            }
            if((rt_tick_get() - s_thread_moniotr.tick) > APP_THREAD_ENTRY_PERIOD){
                s_thread_moniotr.tick = rt_tick_get();
                /** 喂狗 */
                mw_iwdg_refresh();
                if(++entry > s_thread_moniotr.node[s_thread_moniotr.current_index].rentry){
                    s_thread_moniotr.is_error = 0x01;
                    entry = s_thread_moniotr.node[s_thread_moniotr.current_index].rentry;
                    /** 设置重启事件 */
                    thaisen_set_screen_reboot();
                    rt_exit_critical();
                    return -0x01;
                }else{
                    /** 清除重启事件 */
                    thaisen_clear_screen_reboot();
                }
                rt_exit_critical();
                return 0x00;
            }
            /** 这是一个错误的监控节点，寻找下一个节点 */
            if(s_thread_moniotr.node[s_thread_moniotr.current_index].thread == NULL){
                /** 喂狗 */
                mw_iwdg_refresh();
                /** 清除重启事件 */
                thaisen_clear_screen_reboot();

                s_thread_moniotr.tick = rt_tick_get();
                s_thread_moniotr.is_error = 0x00;
                entry = 0x00;
                s_thread_moniotr.node[s_thread_moniotr.current_index].rentry = 0x00;
                if(++s_thread_moniotr.current_index >= s_thread_moniotr.thread_num){
                    s_thread_moniotr.current_index = 0x00;
                }
            }
        }

        if(s_thread_moniotr.is_error){
            thaisen_set_screen_reboot();
#ifdef APP_THREAD_MONITOR_DEBUG
            LOG_D("thread monitor occur error(%s)\n", s_thread_moniotr.node[s_thread_moniotr.current_index].name);
#endif /* APP_THREAD_MONITOR_DEBUG */
        }else{
            thaisen_clear_screen_reboot();
        }
    }

    rt_exit_critical();
#endif /* USING_THREAD_MONITOR */
    return 0x01;
}

/******************************************
 * 函数名     app_thread_monitor_occur_error
 * 功能         查询线程监控是否检测到了错误
 * 参数
 * 返回         1：是       0：否
 * ***************************************/
uint8_t app_thread_monitor_occur_error(void)
{
#ifdef USING_THREAD_MONITOR
    if(s_thread_moniotr.is_error){
        return 0x01;
    }
#endif /* USING_THREAD_MONITOR */
    return 0x00;
}

/******************************************
 * 函数名     app_thread_monitor_get_err_thread_name
 * 功能         线程监控获取错误线程名
 * 参数
 * 返回         线程监控检测到了错误： 错误线程名；  没有检测到错误：NULL
 * ***************************************/
char *app_thread_monitor_get_err_thread_name(void)
{
#ifdef USING_THREAD_MONITOR
    if(s_thread_moniotr.is_error){
        return s_thread_moniotr.node[s_thread_moniotr.current_index].name;
    }
#endif /* USING_THREAD_MONITOR */
    return NULL;
}

void app_led_init(void)
{
    rt_err_t result = RT_EOK;
    uint8_t entry = 0x08, name[8];

    result = rt_thread_init(&led_thread, "task_led",
            app_led_thread_entry, RT_NULL, &led_thread_stack, sizeof(led_thread_stack), 18, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&led_thread);
    }

    app_thread_monitor_add(&led_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

    memset(name, 0x00, sizeof(name));
    memcpy(name, "led", strlen("led"));
    app_thread_monitor_add(&led_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);

}

void app_hci_init(void)
{
    rt_err_t result = RT_EOK;
    uint8_t entry = 0x03, name[8];

    result = rt_thread_init(&hci_req_thread, "task_hci_req",
            app_hci_req_thread_entry, RT_NULL, &hci_req_thread_stack, sizeof(hci_req_thread_stack), 13, 10);  // 18
    if (RT_EOK == result) {
        rt_thread_startup(&hci_req_thread);
    }

    app_thread_monitor_add(&hci_req_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

    memset(name, 0x00, sizeof(name));
    memcpy(name, "hci_req", strlen("hci_req"));
    app_thread_monitor_add(&hci_req_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);

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
    uint8_t entry = 0x03;

    ofsm_fun_list_init();

    for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        sprintf((task_name + strlen("tofsm_0") - 1), "%u", gunno);
        s_thread_gunno[gunno] = gunno;
        result = rt_thread_init(&ofsm_thread[gunno], task_name,
                ofsm_thread_entry, &s_thread_gunno[gunno], &ofsm_thread_stack[gunno], sizeof(ofsm_thread_stack[gunno]), 15, 10);
        if (RT_EOK == result) {

            rt_err_t err = rt_thread_startup(&ofsm_thread[gunno]);
            rt_kprintf("app_operation_init(%d, %d)\n", gunno, err);

            app_thread_monitor_add(&ofsm_thread[gunno], &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);
            app_thread_monitor_add(&ofsm_thread[gunno], task_name, strlen((const char*)task_name), APP_THREAD_MONITOR_OPT_NAME);
        }
    }
}

void app_terminal_init(void)
{
#ifdef CP_CONFIG_USING_DUPU
    rt_err_t result = RT_EOK;
    uint8_t entry = 0x03, name[8];

    result = rt_thread_init(&terminal_thread, "terminal_res",
            terminal_thread_entry, RT_NULL, terminal_thread_stack, sizeof(terminal_thread_stack), 19, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&terminal_thread);

        app_thread_monitor_add(&terminal_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

        memset(name, 0x00, sizeof(name));
        memcpy(name, "ter_res", strlen("ter_res"));
        app_thread_monitor_add(&terminal_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);
    }
    result = rt_thread_init(&terminal_req_thread, "terminal_req",
            terminal_req_thread_entry, RT_NULL, terminal_req_thread_stack, sizeof(terminal_req_thread_stack), 16, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&terminal_req_thread);

        app_thread_monitor_add(&terminal_req_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

        memset(name, 0x00, sizeof(name));
        memcpy(name, "ter_req", strlen("ter_req"));
        app_thread_monitor_add(&terminal_req_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);
    }
#endif /* CP_CONFIG_USING_DUPU */
}

void app_init(void)
{
    /**************************************************************************/

    rt_err_t result = RT_EOK;
    uint8_t entry = 0x03, name[8];

    result = rt_thread_init(&osupport_thread, "task_osupport",
            app_osupport_thread_entry, RT_NULL, &osupport_thread_stack, sizeof(osupport_thread_stack), 16, 10);
    if (RT_EOK == result) {
        rt_thread_startup(&osupport_thread);

        app_thread_monitor_add(&osupport_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

        memset(name, 0x00, sizeof(name));
        memcpy(name, "fdet", strlen("fdet"));
        app_thread_monitor_add(&osupport_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);
    }

    app_card_init();
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
