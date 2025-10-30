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

#define APP_STATE_CHECK_PERIOD           100  /** 状态检测周期(ms) */
#define APP_CTRL_CHECK_PERIOD            100  /** 控制检测周期(ms) */

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

#define APP_AC_RELAY_CONTROL_TIME        ((5 *60 *1000) /APP_STATE_CHECK_PERIOD)     /** 交流接触器控制时间(ms) */
#define APP_WAIT_MODULE_CLOSE_TIME       ((10 *1000) /APP_CTRL_CHECK_PERIOD)           /** 等待模块全部关机时间(ms) */
#define APP_AC_RELAY_ACTION_TIME         ((2 *1000) /APP_CTRL_CHECK_PERIOD)            /** 确认交流接触器动作时间(ms) */
#define APP_AC_RELAY_RELEASE_TIME        ((2 *1000) /APP_CTRL_CHECK_PERIOD)            /** 确认交流接触器释放时间(ms) */

#pragma pack(1)
typedef struct{
    uint8_t out_ov_step;                       /** 输出过压检测步骤(ov:over voltage) */
    uint8_t out_uv_step;                       /** 输出欠压检测步骤(ov:over voltage) */
    uint8_t out_oc_step;                       /** 输出过流检测步骤(ov:over voltage) */

    uint16_t out_ov_count;                     /** 输出过压计数(ov:over voltage) */
    uint16_t out_uv_count;                     /** 输出欠压计数(uv:under voltage) */
    uint16_t out_oc_count;                     /** 输出过流计数(oc:over current) */
}state_check_t;

typedef struct{
    struct{
        uint16_t dcrealy_status : 1;           /** 器件状态：直流继电器 */
        uint16_t acrealy_status : 1;           /** 器件状态：交流接触器 */
        uint16_t pararealy_0_pos_status : 1;   /** 器件状态： 母联继电器0-正极*/
        uint16_t pararealy_0_neg_status : 1;   /** 器件状态： 母联继电器0-负极 */
        uint16_t pararealy_1_pos_status : 1;   /** 器件状态： 母联继电器1-正极 */
        uint16_t pararealy_1_neg_status : 1;   /** 器件状态： 母联继电器1-负极 */
        uint16_t pararealy_2_pos_status : 1;   /** 器件状态： 母联继电器2-正极 */
        uint16_t pararealy_2_neg_status : 1;   /** 器件状态： 母联继电器2-负极 */
        uint16_t auxpower_12v_status : 1;      /** 器件状态：12V辅源 */
        uint16_t auxpower_24v_status : 1;      /** 器件状态：24V辅源 */
        uint16_t elock_status : 1;             /** 器件状态：电子锁 */
        uint16_t fan_status : 1;               /** 器件状态：风扇 */
        uint16_t liquid_status : 1;            /** 器件状态：液冷 */
        uint16_t reserve : 3;
    }opt;
}state_device_t;
#pragma pack()

APP_DEF_SRAM1 static struct rt_thread ctrl_check_thread;
APP_DEF_SRAM0 static rt_uint8_t ctrl_check_thread_stack[1024];
APP_DEF_SRAM1 static struct rt_thread scheck_thread;
APP_DEF_SRAM0 static rt_uint8_t scheck_thread_stack[1024];

APP_DEF_SRAM1 static uint8_t s_module_inpower_enable = 0x00;
APP_DEF_SRAM1 static state_check_t s_state_check[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static state_device_t s_state_device[APP_SYSTEM_GUNNO_SIZE];

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
            if(thaisenGetSysFaultCheckEnBit(thaisenFaultOverVolt, gunno)){
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
            if(thaisenGetSysFaultCheckEnBit(thaisenFaultUnderVolt, gunno)){
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
            if(thaisenGetSysFaultCheckEnBit(thaisenFaultOverCurrent, gunno)){
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

/************************************************ 模块输入电源连接状态检测 ************************************************/

/********************************************
 * 函数名          app_module_inpower_judge
 * 功能              模块输入电源连接判断
 * 参数
 * 返回
 *******************************************/
static void app_module_inpower_judge(void)
{
    uint8_t inpower_connected = 0x00;
    /*****************************************************************************************************************
           * 分以下三种情况：
     * 1.开启交流接触器反馈检测：按按反馈判断是否闭合
     * 2.关闭交流接触器反馈检测且反馈取反：认为是有交流接触器但是这个接触器没有反馈；控制接触器闭合就是闭合，控制接触器断开就是断开
     * 3.关闭交流接触器反馈检测且反馈不取反：认为是没有交流接触器，电源是直连的
     ****************************************************************************************************************/

    /** 交流接触器开启反馈检测，说明有交流接触器(模块的输入电源控制接触器) */
    if(thaisenGetSysFaultCheckEnBit(thaisenRelayAc, 0x00)){
        /** 交流接触器已闭合(模块的输入电源已连接) */
        if(thaisen_relay_AC_FB() == thaisenRelayClose){
            inpower_connected = 0x01;
            thaisenModule_SetInPowerType(THAISEN_MODULE_INPOWER_TYPE_CONTROL_FB);
        }
    }
    /** 无交流接触器情况 */
    else{
        /** 这是取反了 */
        if(*(sys_read_config_item_content(CONFIG_ITEM_INNEG_ACRELAY, 0x00))){
#if 0
            /** 这是低功耗模块 */
            if(*(sys_read_config_item_content(CONFIG_ITEM_LP_MODULE, 0x00)) == CONFIG_LP_CONSUMPTION_MODULE_YN)
            {
                if(s_module_inpower_enable){
                    inpower_connected = 0x01;
                }
            }
            /** 这是正常的模块 */
            else
#endif
            {
                /** 交流接触器已闭合(接触器控制IO口已被控制) */
                if(thaisen_relay_AC_SetFB() == thaisenRelayClose){
                    inpower_connected = 0x01;
                }
            }
            thaisenModule_SetInPowerType(THAISEN_MODULE_INPOWER_TYPE_ONLY_CONTROL);
        }
        /** 这是没有取反的，电源是直连的 */
        else{
            inpower_connected = 0x01;
            thaisenModule_SetInPowerType(THAISEN_MODULE_INPOWER_TYPE_DIRECTLY);
        }
    }
    if(inpower_connected){
        thaisenModule_SetInPowerConnectState(0x01);
    }else{
        thaisenModule_SetInPowerConnectState(0x00);
    }
}

/************************************************ 交流接触器闭合断开控制 ************************************************/
/********************************************
 * 函数名          app_is_all_module_closed
 * 功能              判断所有模块是否已关机
 * 参数
 * 返回             1：是     0：否
 *******************************************/
static uint8_t app_is_all_module_closed(void)
{
    uint8_t length = 0x00, group_num = 0x00;
    thaisenModuleFaultInfoStruct *fault = NULL;
//    thaisenModuleVoltCurrStruct *voltcurr = NULL;

    /** 获取模块组数 */
    group_num = *(sys_read_config_item_content(CONFIG_ITEM_MODULE_GROUP_NUM, 0x00));
    for(uint8_t group = 0; group < group_num; group++){
        length = 0x00;
//            voltcurr = thaisenGetModuleVoltCurrInfo(&length, group);
        fault = thaisenGetModuleFaultInfo(&length, group);
        for(uint8_t count = 0; count < length; count++){
//                if((voltcurr[count].voltage > 600) || (voltcurr[count].current > 10)){
            if(fault[count].state.state.bit.BootState){
                return 0x00;
            }
        }
    }
    return 0x01;
}


/********************************************
 * 函数名          app_acrelay_control
 * 功能              交流接触器控制
 * 参数
 * 返回
 *******************************************/
static void app_acrelay_control(void)
{
#define CTRL_STEP_ACTION_JUDGE           0             /** 控制步骤：继电器动作条件判断 */
#define CTRL_STEP_ACTION                 1             /** 控制步骤：继电器动作 */
#define CTRL_STEP_RELEASE_JUDGE          2             /** 控制步骤：继电器释放条件判断 */
#define CTRL_STEP_RELEASE                3             /** 控制步骤：继电器释放 */
#define CTRL_STEP_NULL                   4             /** 控制步骤：空 */

    static uint8_t ctrl_step = CTRL_STEP_ACTION_JUDGE, is_charging = 0x00, is_plugin = 0x00, is_switching = 0x00;
    static uint8_t guidance_current[APP_SYSTEM_GUNNO_SIZE], guidance_last[APP_SYSTEM_GUNNO_SIZE];
    /** 超时检测判断 */
    static uint16_t judge_timing = 0x00, action_timeout = 0x00, release_timeout = 0x00;
    /** 反馈检测判断 */
    static uint8_t fb_detect_count = 0x00, fb_filter = 0x00;
    struct ofsm_info *ofsm = NULL;

    is_charging = 0x00;
    is_plugin = 0x00;

    /** 判断是否有枪在充电或插枪 */
    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
        ofsm = get_ofsm_info(i);
        if((ofsm->base.state.current >= APP_OFSM_STATE_STARTING) && (ofsm->base.state.current <= APP_OFSM_STATE_STOPING)){
            is_charging = 0x01;
        }
        if(ofsm->base.flag.connect_state == APP_CONNECT_STATE_CONNECT){
            is_plugin = 0x01;
            guidance_current[i] = APP_CONNECT_STATE_CONNECT;
        }else{
            guidance_current[i] = APP_CONNECT_STATE_DISCONNECT;
        }
    }

    switch(ctrl_step){
    /******************************** 继电器动作条件判断 ********************************/
    case CTRL_STEP_ACTION_JUDGE:
        /** 已插枪或在充电 */
        if(is_plugin || is_charging){
            /** 判断所有模块的电压、电流都小于一定值，不能带载切换 */
            if(app_is_all_module_closed()){
                /** 所有模块已关机，进入继电器动作状态 */
                judge_timing = 0x00;
                ctrl_step = CTRL_STEP_ACTION;
            }else{
#if 0
                if(action_timeout < (0xFFFF - 0x01)){
                    action_timeout++;
                }
                /** 动作控制时间最多10s */
                if(action_timeout >= APP_WAIT_MODULE_CLOSE_TIME){
                    action_timeout = 0x00;
                    action_timeout = 0x00;
                    ctrl_step = CTRL_STEP_ACTION_JUDGE;
                }
#endif
            }
        }else{
            action_timeout = 0x00;
        }
        break;
    /******************************** 继电器动作 ********************************/
    case CTRL_STEP_ACTION:
    {
        fb_detect_count = 0x00;
        fb_filter = 0x00;
        app_acrelay_action_magnetic();
        if(thaisenGetSysFaultCheckEnBit(thaisenRelayAc, 0x00)){
            while(1){
                if(fb_detect_count < (0xFF - 0x01)){
                    fb_detect_count++;
                }
                /** 反馈正确 */
                if(thaisen_relay_AC_FB() == thaisenGetACRelayCloseStaus()){
                    fb_filter++;
                }else{
                    fb_filter = 0x00;
                }
                /** 已连续多次反馈正确，故判定继电器已动作 */
                if(fb_filter >= 0x03){
                    judge_timing = 0x00;
                    ctrl_step = CTRL_STEP_RELEASE_JUDGE;
                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        thaisenClearSysFaultLib(thaisenRelayAc, i);
                    }
                    break;
                }
                if(fb_detect_count >= APP_AC_RELAY_ACTION_TIME){
                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        thaisenSetSysFaultLib(thaisenRelayAc, i);
                    }
                    /** 动作失败需要重新拔枪 */
                    ctrl_step = CTRL_STEP_NULL;
                    s_module_inpower_enable = 0x00;
                    break;
                }
                rt_thread_mdelay(10);
            }
        }else{
            judge_timing = 0x00;
            ctrl_step = CTRL_STEP_RELEASE_JUDGE;
        }
    }
        break;
    /******************************** 继电器释放条件判断 ********************************/
    case CTRL_STEP_RELEASE_JUDGE:
        /** 未在充电 */
        if(is_charging == 0x00){
            if(judge_timing < (0xFFFF - 0x01)){
                judge_timing++;
            }
            /** 空闲5分钟后断开 */
            if(judge_timing >= APP_AC_RELAY_CONTROL_TIME){
                /** 判断所有模块的电压、电流都小于一定值，不能带载切换 */
                if(app_is_all_module_closed()){
                    /** 所有模块已关机，进入继电器释放状态 */
                    judge_timing = 0x00;
                    ctrl_step = CTRL_STEP_RELEASE;
                }else{
#if 0
                    if(release_timing < (0xFFFF - 0x01)){
                        release_timing++;
                    }
                    /** 释放控制时间最多5s */
                    if(release_timing >= APP_WAIT_MODULE_CLOSE_TIME){
                        judge_timing = 0x00;
                        release_timing = 0x00;
                        ctrl_step = CTRL_STEP_ACTION_JUDGE;
                    }
#endif
                }
            }
        }else{
            judge_timing = 0x00;
        }
        for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
            /** 有枪开始插枪 */
            if((guidance_current[i] != guidance_last[i]) && (guidance_current[i] == APP_CONNECT_STATE_CONNECT)){
                is_switching = 0x01;
                ctrl_step = CTRL_STEP_NULL;
                break;
            }
        }
        break;
    /******************************** 继电器释放 ********************************/
    case CTRL_STEP_RELEASE:
    {
        fb_detect_count = 0x00;
        fb_filter = 0x00;
        app_acrelay_release_magnetic();

        if(thaisenGetSysFaultCheckEnBit(thaisenRelayAc, 0x00)){
            while(1){
                if(fb_detect_count < (0xFF - 0x01)){
                    fb_detect_count++;
                }
                if(thaisen_relay_AC_FB() != thaisenGetACRelayCloseStaus()){
                    fb_filter++;
                }else{
                    fb_filter = 0x00;
                }
                if(fb_filter >= 0x03){
                    judge_timing = 0x00;
                    ctrl_step = CTRL_STEP_NULL;
                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        thaisenClearSysFaultLib(thaisenRelayAc, i);
                    }
                    break;
                }
                if(fb_detect_count >= APP_AC_RELAY_RELEASE_TIME){
                    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                        thaisenSetSysFaultLib(thaisenRelayAc, i);
                    }
                    /** 动作失败需要重新拔枪 */
                    ctrl_step = CTRL_STEP_NULL;
                    s_module_inpower_enable = 0x01;
                    break;
                }
                rt_thread_mdelay(10);
            }
        }else{
            ctrl_step = CTRL_STEP_NULL;
        }
    }
        break;
    /******************************** 空 ********************************/
    case CTRL_STEP_NULL:
    {
        uint8_t need_switch = 0x00;   /** 需要切换状态 */
        /** 有枪开始充电 */
        if(is_charging){
            need_switch = 0x01;
        }else{
            for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
                /** 有枪开始插枪 */
                if((guidance_current[i] != guidance_last[i]) && (guidance_current[i] == APP_CONNECT_STATE_CONNECT)){
                    need_switch = 0x01;
                    is_switching = 0x01;
                    break;
                }
            }
        }

        if(need_switch){
            /** 判断所有模块的电压、电流都小于一定值，不能带载切换 */
            if(app_is_all_module_closed()){
                /** 所有模块已关机，进入继电器动作状态 */
                is_switching = 0x00;
                ctrl_step = CTRL_STEP_ACTION;
            }else{
#if 0
                if(action_timeout < (0xFFFF - 0x01)){
                    action_timeout++;
                }
                /** 动作控制时间最多5s */
                if(action_timeout >= APP_WAIT_MODULE_CLOSE_TIME){
                    action_timeout = 0x00;
                    is_switching = 0x00;
                    ctrl_step = CTRL_STEP_ACTION_JUDGE;
                }
#endif
            }
        }else{
            action_timeout = 0x00;
        }
    }
        break;
    default:
        break;
    }

    if(is_switching == 0x00){
        for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
            guidance_last[i] = guidance_current[i];
        }
    }
}

static void state_check_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);
    uint8_t gunno = 0x00;

    while(1){
        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

        app_module_inpower_judge();
        for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            app_out_ov_check(gunno);
            app_out_uv_check(gunno);
            app_out_oc_check(gunno);
        }
        rt_thread_mdelay(APP_STATE_CHECK_PERIOD);
    }
}

static void control_check_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

    while(1){
#if 0
        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

        app_module_inpower_judge();
        /** 这是低功耗模块 */
        if(*(sys_read_config_item_content(CONFIG_ITEM_LP_MODULE, 0x00)) == CONFIG_LP_CONSUMPTION_MODULE_YN){
            thaisenSetACRelayType(THADRV_ACRELAY_TYPE_MAGNETIC);
            app_acrelay_control();
        }else{
            thaisenSetACRelayType(THADRV_ACRELAY_TYPE_NORMAL);
        }
#endif
        rt_thread_mdelay(APP_CTRL_CHECK_PERIOD);
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

/***********************************
 * 函数名      app_acrelay_action_magnetic
 * 功能          磁保持类交流接触器动作
 * 参数
 * 返回
 **********************************/
void app_acrelay_action_magnetic(void)
{
#if 0
    if(thaisenGetACRelayType() == THADRV_ACRELAY_TYPE_MAGNETIC){
        /** 易能模块磁保持继电器控制：闭合：一个250ms以上的正脉冲     断开：一个250ms以上的负脉冲 */
        /** 先拉低断开继电器 */
        thaisen_relay_AC_NegtivePlus_Magnetic(0);
        rt_thread_mdelay(300);
        /** 再操作闭合继电器 */
        thaisen_relay_AC_PositivePlus_Magnetic(1);
        rt_thread_mdelay(2000);
        thaisen_relay_AC_PositivePlus_Magnetic(0);
        s_module_inpower_enable = 0x01;
    }
#endif
}

/***********************************
 * 函数名      app_acrelay_release_magnetic
 * 功能          磁保持类交流接触器释放
 * 参数
 * 返回
 **********************************/
void app_acrelay_release_magnetic(void)
{
#if 0
    if(thaisenGetACRelayType() == THADRV_ACRELAY_TYPE_MAGNETIC){
        /** 易能模块磁保持继电器控制：闭合：一个250ms以上的正脉冲     断开：一个250ms以上的负脉冲 */
        /** 先拉低闭合继电器 */
        thaisen_relay_AC_PositivePlus_Magnetic(0);
        rt_thread_mdelay(300);
        /** 再操作断开继电器 */
        thaisen_relay_AC_NegtivePlus_Magnetic(1);
        rt_thread_mdelay(2000);
        thaisen_relay_AC_NegtivePlus_Magnetic(0);
        s_module_inpower_enable = 0x00;
    }
#endif
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
    uint8_t entry = 0x05, name[10];

    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
        s_state_device[i].opt.dcrealy_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.auxpower_24v_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.auxpower_12v_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.elock_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.fan_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.liquid_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.acrealy_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.pararealy_0_neg_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.pararealy_0_pos_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.pararealy_1_neg_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.pararealy_1_pos_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.pararealy_2_neg_status = THAISEN_DEVICE_OPT_RELEASE;
        s_state_device[i].opt.pararealy_2_pos_status = THAISEN_DEVICE_OPT_RELEASE;
    }

    memset(s_state_check, 0x00, sizeof(s_state_check));
    /** 创建线程 */
    if(rt_thread_init(&scheck_thread, "scheck", state_check_thread_entry, NULL, &scheck_thread_stack, sizeof(scheck_thread_stack), 13, 10) != RT_EOK){
        return -0x01;
    }
//    /** 创建线程 */
//    if(rt_thread_init(&ctrl_check_thread, "ctrlcheck", control_check_thread_entry, NULL, &ctrl_check_thread_stack, sizeof(ctrl_check_thread_stack), 16, 10) != RT_EOK){
//        return -0x01;
//    }
    /** 启动线程 */
    rt_thread_startup(&scheck_thread);
//    /** 启动线程 */
//    rt_thread_startup(&ctrl_check_thread);
    /** 添加线程监控节点 */
    app_thread_monitor_add(&scheck_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);
//    /** 添加线程监控节点 */
//    app_thread_monitor_add(&ctrl_check_thread, &entry, sizeof(entry), APP_THREAD_MONITOR_OPT_ENTRY);

    memset(name, 0x00, sizeof(name));
    memcpy(name, "scheck", strlen("scheck"));
    app_thread_monitor_add(&scheck_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);

//    memset(name, 0x00, sizeof(name));
//    memcpy(name, "ctrlcheck", strlen("ctrlcheck"));
//    app_thread_monitor_add(&ctrl_check_thread, name, strlen((char*)name), APP_THREAD_MONITOR_OPT_NAME);

    return 0x00;
}


/********************************************************** 导引状态变化 **********************************************************/
/********************************************************** 导引状态变化 **********************************************************/
extern void ykc_monitor_guidance_changed_callback(uint8_t gunno, uint8_t flag, uint32_t timestamp, int voltage, int voltage_last, uint16_t diff_positive_adc, \
        uint16_t diff_negtive_adc, uint16_t diff_positive_adc_last, uint16_t diff_negtive_adc_last);
/*************************************************************************************
 * 函数名        app_state_guidance_changed
 * 功能            导引状态变化回调
 * 参数            info    导引变化信息
 *       flag    变化标志@thaisenGuidanceChanged_t
 *       port    枪口号
 * 返回
 ************************************************************************************/
void app_state_guidance_changed(thaisenGuidanceInfo_t info, uint8_t flag, uint8_t port)
{
    uint32_t timestamp = time(NULL);

    ykc_monitor_guidance_changed_callback(port, flag, timestamp, info.voltage, info.voltage_last, info.channel_adc, 0x00, info.channel_adc_last, 0x00);
}

/********************************************************** 器件状态变化 **********************************************************/
/********************************************************** 器件状态变化 **********************************************************/
extern void ykc_monitor_dev_control_changed_callback(uint8_t gunno, uint8_t device, uint8_t type, uint8_t is_debug, uint8_t control, \
        uint8_t result, uint32_t timestamp);
/*************************************************************************************
 * 函数名        app_state_device_status_changed
 * 功能            器件状态变化回调
 * 参数            device    器件枚举@thaisenDeviceEnum
 *       parameter 变化信息参数@thaisenDeviceParameter
 *       plen      参数长度
 *       port      枪口号
 * 返回
 ************************************************************************************/
void app_state_device_status_changed(uint8_t device, void *parameter, uint8_t plen, uint8_t port)
{
    if((parameter == NULL) || (port >= APP_SYSTEM_GUNNO_SIZE)){
        return;
    }
    uint8_t is_changed = 0x00, is_whole_device = 0x00;
    uint32_t timestamp = time(NULL);
    thaisenDeviceParameter *p = (thaisenDeviceParameter*)parameter;

    switch(device){
    case THAISEN_DEVICE_ENUM_DCRELAY:
        if(s_state_device[port].opt.dcrealy_status != p->opt){
            rt_kprintf("device dcrelay status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.dcrealy_status = p->opt;
            is_changed = 0x01;
        }
        break;
    case THAISEN_DEVICE_ENUM_ACRELAY:
        if(s_state_device[port].opt.acrealy_status != p->opt){
            rt_kprintf("device acrelay status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.acrealy_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_0:
        if(s_state_device[port].opt.pararealy_0_pos_status != p->opt){
            rt_kprintf("device pararelay 0 pos status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.pararealy_0_pos_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_0:
        if(s_state_device[port].opt.pararealy_0_neg_status != p->opt){
            rt_kprintf("device pararelay 0 neg status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.pararealy_0_neg_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_1:
        if(s_state_device[port].opt.pararealy_1_pos_status != p->opt){
            rt_kprintf("device pararelay 1 pos status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.pararealy_1_pos_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_1:
        if(s_state_device[port].opt.pararealy_1_neg_status != p->opt){
            rt_kprintf("device pararelay 1 neg status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.pararealy_1_neg_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_2:
        if(s_state_device[port].opt.pararealy_2_pos_status != p->opt){
            rt_kprintf("device pararelay 2 pos status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.pararealy_2_pos_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_2:
        if(s_state_device[port].opt.pararealy_2_neg_status != p->opt){
            rt_kprintf("device pararelay 2 neg status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.pararealy_2_neg_status = p->opt;
            is_changed = 0x01;
            is_whole_device = 0x01;
            port = 0x00;
        }
        break;
    case THAISEN_DEVICE_ENUM_AUXPOWER_12V:
        if(s_state_device[port].opt.auxpower_12v_status != p->opt){
            rt_kprintf("device auxpower_12v status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.auxpower_12v_status = p->opt;
            is_changed = 0x01;
        }
        break;
    case THAISEN_DEVICE_ENUM_AUXPOWER_24V:
        if(s_state_device[port].opt.auxpower_24v_status != p->opt){
            rt_kprintf("device auxpower_24v status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.auxpower_24v_status = p->opt;
            is_changed = 0x01;
        }
        break;
    case THAISEN_DEVICE_ENUM_ELOCK:
        if(s_state_device[port].opt.elock_status != p->opt){
            rt_kprintf("device elock status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.elock_status = p->opt;
            is_changed = 0x01;
        }
        break;
    case THAISEN_DEVICE_ENUM_FAN:
        if(s_state_device[port].opt.fan_status != p->opt){
            rt_kprintf("device fan status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.fan_status = p->opt;
            is_changed = 0x01;
        }
        break;
    case THAISEN_DEVICE_ENUM_LIQUID:
        if(s_state_device[port].opt.liquid_status != p->opt){
            rt_kprintf("device liquid status changed(%d, %d, %d)\n", port, p->opt, p->result);
            s_state_device[port].opt.liquid_status = p->opt;
            is_changed = 0x01;
        }
        break;
    default:
        break;
    }
    if(is_changed){
        uint8_t ctrl = 0x00, is_debug = 0x00;

        if((p->opt == THAISEN_DEVICE_OPT_DEBUG_CONTROL) || (p->opt == THAISEN_DEVICE_OPT_DEBUG_RELEASE)){
            is_debug = 0x01;
        }
        if((p->opt == THAISEN_DEVICE_OPT_CONTROL) || (p->opt == THAISEN_DEVICE_OPT_DEBUG_CONTROL)){
            ctrl = 0x01;
        }
        ykc_monitor_dev_control_changed_callback(port, device, is_whole_device, is_debug, ctrl, p->result, timestamp);
    }
}
