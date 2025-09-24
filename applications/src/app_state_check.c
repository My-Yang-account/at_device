/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-10     31638       the first version
 */
#include "app_state_check.h"
#include "app_ofsm.h"
#include "rtthread.h"
#include "chargepile_config.h"
#include "thaisen7102Public.h"
#include "thaisenChargModuleLib.h"
#include "mw_meter.h"

#define APP_STATE_CHECK_PERIOD           100   /** 状态检测周期(ms) */

#define APP_OUT_OV_STEP_NORMAL           0     /** 输出过压检测步骤：正常 */
#define APP_OUT_OV_STEP_FILTER           1     /** 输出过压检测步骤：过渡阶段(滤波) */
#define APP_OUT_OV_STEP_ABNORMAL         2     /** 输出过压检测步骤：异常 */
#define APP_OUT_OV_FILTER_VALUE          1000  /** 输出过压检测过渡差值(0.01V) */
#define APP_OUT_OV_VERIFY_TIME           (4900 /APP_STATE_CHECK_PERIOD)     /** 输出过压发生确认时间(ms) */

#define APP_OUT_UV_STEP_NORMAL           0     /** 输出欠压检测步骤：正常 */
#define APP_OUT_UV_STEP_FILTER           1     /** 输出欠压检测步骤：过渡阶段(滤波) */
#define APP_OUT_UV_STEP_ABNORMAL         2     /** 输出欠压检测步骤：异常 */
#define APP_OUT_UV_FILTER_VALUE          1000  /** 输出欠压检测过渡差值(0.01V) */
#define APP_OUT_UV_VERIFY_TIME           (6900 /APP_STATE_CHECK_PERIOD)     /** 输出欠压发生确认时间(ms) */

#define APP_OUT_OC_STEP_NORMAL           0     /** 输出过流检测步骤：正常 */
#define APP_OUT_OC_STEP_FILTER           1     /** 输出过流检测步骤：过渡阶段(滤波) */
#define APP_OUT_OC_STEP_ABNORMAL         2     /** 输出过流检测步骤：异常 */
#define APP_OUT_OC_FILTER_VALUE          200   /** 输出过流检测过渡差值(0.01A) */
#define APP_OUT_OC_VERIFY_TIME           (4900 /APP_STATE_CHECK_PERIOD)     /** 输出过流发生确认时间(ms) */

typedef struct{
    uint8_t out_ov_step;                       /** 输出过压检测步骤(ov:over voltage) */
    uint8_t out_uv_step;                       /** 输出欠压检测步骤(ov:over voltage) */
    uint8_t out_oc_step;                       /** 输出过流检测步骤(ov:over voltage) */

    uint16_t out_ov_count;                     /** 输出过压计数(ov:over voltage) */
    uint16_t out_uv_count;                     /** 输出欠压计数(uv:under voltage) */
    uint16_t out_oc_count;                     /** 输出过流计数(oc:over current) */
}state_check_t;

APP_DEF_SRAM1 static struct rt_thread scheck_thread;
APP_DEF_SRAM0 static rt_uint8_t scheck_thread_stack[1024];
APP_DEF_SRAM1 static state_check_t s_state_check[APP_SYSTEM_GUNNO_SIZE];

/********************************************** 系统实际值 **********************************************/
/******************************************
 * 函数名       app_get_out_volt_value
 * 功能           获取输出电压值
 * 参数          gunno    枪号
 *
 * 返回         输出过压配置值(0.01V)
 *****************************************/
static uint32_t app_get_out_volt_value(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return (thaisenModuleGetHignestVolt(gunno) *10);
    }
    return 0x00;
}

/******************************************
 * 函数名       app_get_out_curr_value
 * 功能           获取输出电流值
 * 参数          gunno    枪号
 *
 * 返回         输出过流配置值(0.01A)
 *****************************************/
static uint32_t app_get_out_curr_value(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return (thaisen_get_module_curr(gunno) *10);
    }
    return 0x00;
}

/********************************************** 系统配置值获取 **********************************************/
/******************************************
 * 函数名       app_get_config_out_ov_value
 * 功能           获取输出过压配置值
 * 参数          gunno    枪号
 *
 * 返回         输出过压配置值(0.01V)
 *****************************************/
static uint32_t app_get_config_out_ov_value(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERVOL, 0x00));
    }
    return 0xFFFFFFFF;
}

/******************************************
 * 函数名       app_get_config_out_uv_value
 * 功能           获取输出欠压配置值
 * 参数          gunno    枪号
 *
 * 返回         输出欠压配置值(0.01V)
 *****************************************/
static uint32_t app_get_config_out_uv_value(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_UNDERVOL, 0x00));
    }
    return 0xFFFFFFFF;
}

/******************************************
 * 函数名       app_get_config_out_oc_value
 * 功能           获取输出过流配置值
 * 参数          gunno    枪号
 *
 * 返回         输出过流配置值(0.01A)
 *****************************************/
static uint32_t app_get_config_out_oc_value(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        return *(uint32_t*)(sys_read_config_item_content(CONFIG_ITEM_OUTPUT_OVERCUR, 0x00));
    }
    return 0xFFFFFFFF;
}

/********************************************** 故障设置/清除 **********************************************/
/******************************************
 * 函数名       app_out_ov_fault_operate
 * 功能           输出过压故障操作
 * 参数          gunno    枪号
 *        state    状态(1：故障发生，0：故障恢复)
 * 返回
 *****************************************/
static void app_out_ov_fault_operate(uint8_t gunno, uint8_t state)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        rt_enter_critical();
        if(state == 0x00){
            if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_OV, gunno) == 0x00){
                thaisenClearSysFaultLib(thaisenFaultOverVolt, gunno);
                thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_OV, 0x01, gunno);
            }
        }else{
            if(thaisenGetSysFaultCheckEnBit(thaisenFaultOverVolt)){
                if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_OV, gunno) == 0x00){
                    thaisenSetSysFaultLib(thaisenFaultOverVolt, gunno);
                    thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_OV, 0x00, gunno);
                }
            }else{
                if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_OV, gunno) == 0x00){
                    thaisenClearSysFaultLib(thaisenFaultOverVolt, gunno);
                    thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_OV, 0x01, gunno);
                }
            }
        }
        rt_exit_critical();
    }
}

/******************************************
 * 函数名       app_out_uv_fault_operate
 * 功能           输出欠压故障操作
 * 参数          gunno    枪号
 *        state    状态(1：故障发生，0：故障恢复)
 * 返回
 *****************************************/
static void app_out_uv_fault_operate(uint8_t gunno, uint8_t state)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        rt_enter_critical();
        if(state == 0x00){
            if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_UV, gunno) == 0x00){
                thaisenClearSysFaultLib(thaisenFaultUnderVolt, gunno);
                thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_UV, 0x01, gunno);
            }
        }else{
            if(thaisenGetSysFaultCheckEnBit(thaisenFaultUnderVolt)){
                if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_UV, gunno) == 0x00){
                    thaisenSetSysFaultLib(thaisenFaultUnderVolt, gunno);
                    thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_UV, 0x00, gunno);
                }
            }else{
                if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_UV, gunno) == 0x00){
                    thaisenClearSysFaultLib(thaisenFaultUnderVolt, gunno);
                    thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_UV, 0x01, gunno);
                }
            }
        }
        rt_exit_critical();
    }
}

/******************************************
 * 函数名       app_out_oc_fault_operate
 * 功能           输出过流故障操作
 * 参数          gunno    枪号
 *        state    状态(1：故障发生，0：故障恢复)
 * 返回
 *****************************************/
static void app_out_oc_fault_operate(uint8_t gunno, uint8_t state)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        rt_enter_critical();
        if(state == 0x00){
            if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_OC, gunno) == 0x00){
                thaisenClearSysFaultLib(thaisenFaultOverCurrent, gunno);
                thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_OC, 0x01, gunno);
            }
        }else{
            if(thaisenGetSysFaultCheckEnBit(thaisenFaultOverCurrent)){
                if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_OC, gunno) == 0x00){
                    thaisenSetSysFaultLib(thaisenFaultOverCurrent, gunno);
                    thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_OC, 0x00, gunno);
                }
            }else{
                if(thaisenModuleGetFOccurState(THAISEN_MODULE_FAULT_OUT_OC, gunno) == 0x00){
                    thaisenClearSysFaultLib(thaisenFaultOverCurrent, gunno);
                    thaisenModuleSetFEnState(THAISEN_MODULE_FAULT_OUT_OC, 0x01, gunno);
                }
            }
        }
        rt_exit_critical();
    }
}

/******************************************************************* 状态检测 *******************************************************************/
/******************************************
 * 函数名       app_out_ov_check
 * 功能           输出过压检测
 * 参数          gunno    枪号
 * 返回
 *****************************************/
static void app_out_ov_check(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    uint32_t config_ov_value = app_get_config_out_ov_value(gunno);
    uint32_t out_volt_value = app_get_out_volt_value(gunno);
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    if(!((ofsm->state >= APP_OFSM_STATE_STARTING) && ((ofsm->state <= APP_OFSM_STATE_CHARGING)))){
        s_state_check[gunno].out_ov_step = APP_OUT_OV_STEP_NORMAL;
        s_state_check[gunno].out_ov_count = 0x00;
        /** 清除故障 */
        app_out_ov_fault_operate(gunno, 0x00);
        return;
    }

    switch(s_state_check[gunno].out_ov_step){
    case APP_OUT_OV_STEP_NORMAL:
        if(out_volt_value > config_ov_value){
            if(++s_state_check[gunno].out_ov_count >= APP_OUT_OV_VERIFY_TIME){
                s_state_check[gunno].out_ov_step = APP_OUT_OV_STEP_ABNORMAL;
                s_state_check[gunno].out_ov_count = 0x00;
            }
        }else{
            s_state_check[gunno].out_ov_count = 0x00;
        }
        /** 清除故障 */
        app_out_ov_fault_operate(gunno, 0x00);
        break;
    case APP_OUT_OV_STEP_FILTER:
        if(config_ov_value > APP_OUT_OV_FILTER_VALUE){
            if(out_volt_value <= (config_ov_value - APP_OUT_OV_FILTER_VALUE)){
                if(++s_state_check[gunno].out_ov_count >= APP_OUT_OV_VERIFY_TIME){
                    s_state_check[gunno].out_ov_step = APP_OUT_OV_STEP_NORMAL;
                    s_state_check[gunno].out_ov_count = 0x00;
                }
            }else{
                s_state_check[gunno].out_ov_count = 0x00;
            }
        }
        /** 设置故障 */
        app_out_ov_fault_operate(gunno, 0x01);
        break;
    case APP_OUT_OV_STEP_ABNORMAL:
        s_state_check[gunno].out_ov_step = APP_OUT_OV_STEP_ABNORMAL;
        if(out_volt_value < config_ov_value){
            s_state_check[gunno].out_ov_step = APP_OUT_OV_STEP_FILTER;
        }
        s_state_check[gunno].out_ov_count = 0x00;
        /** 设置故障 */
        app_out_ov_fault_operate(gunno, 0x01);
        break;
    default:
        s_state_check[gunno].out_ov_step = APP_OUT_OV_STEP_NORMAL;
        break;
    }
}

/******************************************
 * 函数名       app_out_uv_check
 * 功能           输出欠压压检测
 * 参数          gunno    枪号
 * 返回
 *****************************************/
static void app_out_uv_check(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    uint32_t config_uv_value = app_get_config_out_uv_value(gunno);
    uint32_t out_volt_value = app_get_out_volt_value(gunno);
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    if((ofsm->state != APP_OFSM_STATE_CHARGING) || (thaisen_get_charging_pause_activate(gunno) == thaisenChargingPause)){
        s_state_check[gunno].out_uv_step = APP_OUT_UV_STEP_NORMAL;
        s_state_check[gunno].out_uv_count = 0x00;
        /** 清除故障 */
        app_out_uv_fault_operate(gunno, 0x00);
        return;
    }

    switch(s_state_check[gunno].out_uv_step){
    case APP_OUT_UV_STEP_NORMAL:
        if(out_volt_value < config_uv_value){
            if(++s_state_check[gunno].out_uv_count >= APP_OUT_UV_VERIFY_TIME){
                s_state_check[gunno].out_uv_step = APP_OUT_UV_STEP_ABNORMAL;
                s_state_check[gunno].out_uv_count = 0x00;
            }
        }else{
            s_state_check[gunno].out_uv_count = 0x00;
        }
        /** 清除故障 */
        app_out_uv_fault_operate(gunno, 0x00);
        break;
    case APP_OUT_UV_STEP_FILTER:
        if(out_volt_value >= (config_uv_value + APP_OUT_UV_FILTER_VALUE)){
            if(++s_state_check[gunno].out_uv_count >= APP_OUT_UV_VERIFY_TIME){
                s_state_check[gunno].out_uv_step = APP_OUT_UV_STEP_NORMAL;
                s_state_check[gunno].out_uv_count = 0x00;
            }
        }else{
            s_state_check[gunno].out_uv_count = 0x00;
        }
        /** 设置故障 */
        app_out_uv_fault_operate(gunno, 0x01);
        break;
    case APP_OUT_UV_STEP_ABNORMAL:
        s_state_check[gunno].out_uv_step = APP_OUT_UV_STEP_ABNORMAL;
        if(out_volt_value >= config_uv_value){
            s_state_check[gunno].out_uv_step = APP_OUT_UV_STEP_FILTER;
        }
        s_state_check[gunno].out_uv_count = 0x00;
        /** 设置故障 */
        app_out_uv_fault_operate(gunno, 0x01);
        break;
    default:
        s_state_check[gunno].out_uv_step = APP_OUT_UV_STEP_NORMAL;
        break;
    }
}

/******************************************
 * 函数名       app_out_oc_check
 * 功能           输出过流检测
 * 参数          gunno    枪号
 * 返回
 *****************************************/
static void app_out_oc_check(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    uint32_t config_oc_value = app_get_config_out_oc_value(gunno);
    uint32_t out_curr_value = app_get_out_curr_value(gunno);
    struct ofsm_info *ofsm = get_ofsm_info(gunno);

    if(ofsm->state != APP_OFSM_STATE_CHARGING){
        s_state_check[gunno].out_oc_step = APP_OUT_OC_STEP_NORMAL;
        s_state_check[gunno].out_oc_count = 0x00;
        /** 清除故障 */
        app_out_oc_fault_operate(gunno, 0x00);
        return;
    }

    switch(s_state_check[gunno].out_oc_step){
    case APP_OUT_OC_STEP_NORMAL:
        if(out_curr_value > config_oc_value){
            if(++s_state_check[gunno].out_oc_count >= APP_OUT_OC_VERIFY_TIME){
                s_state_check[gunno].out_oc_step = APP_OUT_OC_STEP_ABNORMAL;
                s_state_check[gunno].out_oc_count = 0x00;
            }
        }else{
            s_state_check[gunno].out_oc_count = 0x00;
        }
        /** 清除故障 */
        app_out_oc_fault_operate(gunno, 0x00);
        break;
    case APP_OUT_OC_STEP_FILTER:
        if(config_oc_value > APP_OUT_OC_FILTER_VALUE){
            if(out_curr_value <= (config_oc_value - APP_OUT_OC_FILTER_VALUE)){
                if(++s_state_check[gunno].out_oc_count >= APP_OUT_OC_VERIFY_TIME){
                    s_state_check[gunno].out_oc_step = APP_OUT_OC_STEP_NORMAL;
                    s_state_check[gunno].out_oc_count = 0x00;
                }
            }else{
                s_state_check[gunno].out_oc_count = 0x00;
            }
        }
        /** 设置故障 */
        app_out_oc_fault_operate(gunno, 0x01);
        break;
    case APP_OUT_OC_STEP_ABNORMAL:
        s_state_check[gunno].out_oc_step = APP_OUT_OC_STEP_ABNORMAL;
        if(out_curr_value < config_oc_value){
            s_state_check[gunno].out_oc_step = APP_OUT_OC_STEP_FILTER;
        }
        s_state_check[gunno].out_oc_count = 0x00;
        /** 设置故障 */
        app_out_oc_fault_operate(gunno, 0x01);
        break;
    default:
        s_state_check[gunno].out_oc_step = APP_OUT_OC_STEP_NORMAL;
        break;
    }
}


static void state_check_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);
    uint8_t gunno = 0x00;

    while(1){
        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);
        for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            app_out_ov_check(gunno);
            app_out_uv_check(gunno);
            app_out_oc_check(gunno);
        }
        rt_thread_mdelay(APP_STATE_CHECK_PERIOD);
    }
}

/***********************************
 * 函数名      app_is_out_ov
 * 功能          检查是否已输出过压
 * 参数          gunno    枪号
 * 返回          1：是    0：否
 **********************************/
uint8_t app_is_out_ov(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(s_state_check[gunno].out_ov_step != APP_OUT_OV_STEP_NORMAL){
        return 0x01;
    }
    return 0x00;
}

/***********************************
 * 函数名      app_is_out_uv
 * 功能          检查是否已输出欠压
 * 参数          gunno    枪号
 * 返回          1：是    0：否
 **********************************/
uint8_t app_is_out_uv(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(s_state_check[gunno].out_uv_step != APP_OUT_UV_STEP_NORMAL){
        return 0x01;
    }
    return 0x00;
}

/***********************************
 * 函数名      app_is_out_oc
 * 功能          检查是否已输出过流
 * 参数          gunno    枪号
 * 返回          1：是    0：否
 **********************************/
uint8_t app_is_out_oc(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(s_state_check[gunno].out_oc_step != APP_OUT_OC_STEP_NORMAL){
        return 0x01;
    }
    return 0x00;
}

/*****************************
 * 函数名      app_state_check_init
 * 功能          状态检测部分初始化
 * 参数
 * 说明          >=0：成功     <0：失败
 ****************************/
int32_t app_state_check_init(void)
{
    extern int32_t app_thread_monitor_add(void *thread, void *para, uint32_t plen, uint32_t option);
    uint8_t entry = 0x05, name[8];

    memset(&s_state_check, 0x00, sizeof(s_state_check));
    /** 创建线程 */
    if(rt_thread_init(&scheck_thread, "scheck", state_check_thread_entry, NULL, &scheck_thread_stack, sizeof(scheck_thread_stack), 13, 10) != RT_EOK){
        return -0x01;
    }
    /** 启动线程 */
    rt_thread_startup(&scheck_thread);
    /** 添加线程监控节点 */
    app_thread_monitor_add(&scheck_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

    memset(name, 0x00, sizeof(name));
    memcpy(name, "scheck", strlen("scheck"));
    app_thread_monitor_add(&scheck_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);

    return 0x00;
}
