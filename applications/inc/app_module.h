/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2026-02-06     31638       the first version
 */
#ifndef APPLICATIONS_INC_APP_MODULE_H_
#define APPLICATIONS_INC_APP_MODULE_H_

#include "stdio.h"

/** 调试信息 */
#define MCTRL_DEBUG_COLOR_OPEN(code)       printf("\033["#code"m")
#define MCTRL_DEBUG_COLOR_CLOSE            printf("\033[0m\n")

#define MCTRL_DEBUG(...)                           \
    do{                                             \
        printf(__VA_ARGS__);                        \
    }while(0)                                       \

#define MCTRL_DEBUG_INFO(...)                      \
        do{                                         \
        MCTRL_DEBUG_COLOR_OPEN(32);                \
        printf(__VA_ARGS__);                        \
        MCTRL_DEBUG_COLOR_CLOSE;                   \
    }while(0)                                       \

#define MCTRL_DEBUG_WARNNING(...)                  \
    do{                                             \
        MCTRL_DEBUG_COLOR_OPEN(33);                \
        printf(__VA_ARGS__);                        \
        MCTRL_DEBUG_COLOR_CLOSE;                   \
    }while(0)                                       \

#define MCTRL_DEBUG_ERROR(...)                     \
    do{                                             \
        MCTRL_DEBUG_COLOR_OPEN(31);                \
        printf(__VA_ARGS__);                        \
        MCTRL_DEBUG_COLOR_CLOSE;                   \
    }while(0)                                       \


/*********************************************************************************************
 * 函数名      app_module_relay_check_enable
 * 功能          模块矩阵继电器故障检测使能
 * 参数          relay_port      继电器口@enum udrv_relay_port
 *         state           状态(1:使能     0:不使能)
 * 返回
 ********************************************************************************************/
void app_module_relay_check_enable(unsigned char relay_port, unsigned char state);

/*********************************************************************************************
 * 函数名      app_module_relay_fb_reversal
 * 功能          模块矩阵继电器反馈取反
 * 参数          relay_port      继电器口@enum udrv_relay_port
 *         state           状态(1:取反     0:不取反)
 * 返回
 ********************************************************************************************/
void app_module_relay_fb_reversal(unsigned char relay_port, unsigned char state);

/*****************************************
 * 函数名             app_module_set_module_current_min
 * 功能                设置模块最小输出电流(0.01A)
 * 参数                current     电流值
 * 返回
 ****************************************/
void app_module_set_module_current_min(unsigned short current);

/*****************************************
 * 函数名             app_module_set_module_current_max
 * 功能                设置模块最大输出电流(0.01A)
 * 参数                current     电流值
 * 返回
 ****************************************/
void app_module_set_module_current_max(unsigned int current);

/*****************************************
 * 函数名             app_module_set_power_allocate_way
 * 功能                设置功率分配方式
 * 参数                way     功率分配方式
 * 返回
 ****************************************/
void app_module_set_power_allocate_way(unsigned char way);

/*****************************************
 * 函数名             app_module_set_single_module_power
 * 功能                设置单个模块功率(1W)
 * 参数                power     单个模块功率值
 * 返回
 ****************************************/
void app_module_set_single_module_power(unsigned int power);

/*****************************************
 * 函数名             app_module_get_setup_voltage
 * 功能                按组获取给模块设置的电压(0.1V)
 * 参数                group     组号
 * 返回                给模块设置的电压(0.1V)
 ****************************************/
unsigned int app_module_get_setup_voltage(unsigned char group);

/*****************************************
 * 函数名             app_module_get_setup_current
 * 功能                按组获取给模块设置的电流(0.01A)
 * 参数                group     组号
 * 返回                给模块设置的电流(0.01A)
 ****************************************/
unsigned int app_module_get_setup_current(unsigned char group);

/*****************************************
 * 函数名             app_module_get_setup_current
 * 功能                按组获取给模块设置的电流(0.01A)
 * 参数                group     组号
 * 返回                给模块设置的电流(0.01A)
 ****************************************/
unsigned char app_module_is_open(unsigned char group);

/*****************************************
 * 函数名             app_module_belong_gun
 * 功能                获取模块组归属枪
 * 参数               group      模块组组号(从0开始)
 * 返回                归属枪号(从1开始)
 ****************************************/
unsigned char app_module_belong_gun(unsigned char group);

/*****************************************
 * 函数名             app_module_schedule_judge
 * 功能                模块调度启用判断
 * 参数                current     电流值
 * 返回
 ****************************************/
void app_module_schedule_judge(void);

/*****************************************************
 * 函数名              app_module_input_power_control
 * 功能                 模块输入电源控制
 * 参数
 * 返回
 ****************************************************/
void app_module_input_power_control(void);

/*****************************************************
 * 函数名              app_mctrl_fan_control
 * 功能                 风机控制
 * 参数
 * 返回
 ****************************************************/
void app_module_fan_control(void);

/*********************************************************************************************
 * 函数名         app_module_debug_start
 * 功能             模块调试强制启动
 * 参数             gunno      枪号
 *       voltage    设置电压(0.1V)
 *       current    设置电流(0.1A)
 * 返回            >=0：成功     <0：失败
 ********************************************************************************************/
int app_module_debug_start(unsigned char gunno, unsigned short voltage, unsigned short current);

/*********************************************************************************************
 * 函数名         app_module_debug_stop
 * 功能             模块调试强制停止
 * 参数             gunno      枪号
 * 返回            >=0：成功     <0：失败
 ********************************************************************************************/
int app_module_debug_stop(unsigned char gunno);

/*********************************************************************************************
 * 函数名         app_module_is_debug_started
 * 功能             判断模块是否已进行调试强制启动
 * 参数             gunno      枪号
 * 返回            1：是        0：否
 ********************************************************************************************/
unsigned char app_module_is_debug_started(unsigned char gunno);

/*****************************************
 * 函数名             app_module_get_module_group_voltage
 * 功能                按组获取模块输出电压(0.1V, 最高电压)
 * 参数                group     组号(从0开始)
 * 返回                组模块输出电压(0.1V, 最高电压)
 ****************************************/
unsigned int app_module_get_module_group_voltage(unsigned char group);

/*****************************************
 * 函数名             app_module_get_module_group_current
 * 功能                按组获取模块输出电流(0.01A)
 * 参数                group     组号(从0开始)
 * 返回                组模块输出电流(0.01A)
 ****************************************/
unsigned int app_module_get_module_group_current(unsigned char group);

/*****************************************
 * 函数名             app_module_loop
 * 功能                模块控制部分实时运行
 * 参数
 * 返回
 ****************************************/
void app_module_loop(void);

/*****************************************
 * 函数名             app_module_ctrl_init
 * 功能                模块控制部分初始化
 * 参数
 * 返回                 >=0：成功     <0：失败
 ****************************************/
int app_module_ctrl_init(void);

#endif /* APPLICATIONS_INC_APP_MODULE_H_ */
