/**
 ******************************************************************************
 * @file app_buzzer.c
 * @author leven
 * @brief 
 ******************************************************************************
 */
//
//#include <rtthread.h>
//
//#include "app_buzzer.h"
//#include "app_ota.h"
//
//#define DBG_TAG "app.buzzer"
//#define DBG_LVL DBG_LOG
//#include <rtdbg.h>
//
//#define BUZZON_STATE_MAX_COUNT   8
//
//static enum buzzon_state s_buzzon_state_array[BUZZON_STATE_MAX_COUNT];
//
//static enum buzzon_state s_buzzon_state = BUZZON_STATE_NULL;
//
//struct rt_messagequeue g_buzzon_mq;
//
//void buzzer_ipc_init(void)
//{
//    rt_mq_init(&g_buzzon_mq, "buzzon_mq", s_buzzon_state_array, sizeof(enum buzzon_state), sizeof(s_buzzon_state_array), RT_IPC_FLAG_PRIO);
//}
//
//void buzzer_thread_entry(void *parameter)
//{
//    (void)parameter;
//
//    while (1)
//    {
//        switch (get_ota_state()) {
//        case OTA_STATE_INTERNET_UP:
//        case OTA_STATE_AUTH_SUCCESS:
//        case OTA_STATE_UPDATEING:
//        case OTA_STATE_UPDATE_SECCESS:
//        case OTA_STATE_UPDATE_FAILED:
//            return;
//        default:
//            break;
//        }
//
//
//        rt_thread_mdelay(100);
//    }
//}
//
//enum buzzon_state get_buzzon_state(void)
//{
//    return s_buzzon_state;
//}
//
//void set_buzzon_state(enum buzzon_state state)
//{
//    s_buzzon_state = state;
//}
//
//void clear_buzzon_state(void)
//{
//    s_buzzon_state = BUZZON_STATE_NULL;
//}
