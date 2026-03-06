/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-06     31638       the first version
 */
#ifndef APPLICATIONS_INC_APP_CAN_H_
#define APPLICATIONS_INC_APP_CAN_H_

#include "app_ofsm.h"
#include "app.h"

#ifdef CP_USING_LV_MODULE_BMS
typedef enum{
    APP_BMSLV_CMD_FULL_OF_STOP,
    APP_BMSLV_CMD_SLOW_CHARGING,
    APP_BMSLV_CMD_FAST_CHARGING,
    APP_BMSLV_CMD_CHARGING_FAULT,
    APP_BMSLV_CMD_SIZE,
}bms_lv_cmd;

typedef enum{
    APP_CHARGER_LV_MODE_IDLE,                        /** 充电机工作模式：空闲 */
    APP_CHARGER_LV_MODE_CHARGING,                    /** 充电机工作模式：充电 */
    APP_CHARGER_LV_MODE_FAULTING,                    /** 充电机工作模式：故障 */
    APP_CHARGER_LV_MODE_STOP,                        /** 充电机工作模式：停止 */
    APP_CHARGER_LV_MODE_SIZE,                        /** 充电机工作模式 */
}charger_lv_mode;

typedef enum{
    APP_CHARGER_LV_CMD_IDLE,                         /** 充电机请求：无请求-未插枪 */
    APP_CHARGER_LV_CMD_END_SELFCHECK,                /** 充电机请求：自检结束请求充电 */
    APP_CHARGER_LV_CMD_SELFCHECK,                    /** 充电机请求：自检结束过程中 */
    APP_CHARGER_LV_CMD_FAULTING,                     /** 充电机请求：充电桩故障 */
    APP_CHARGER_LV_CMD_BMS_FAULT,                    /** 充电机请求：BMS故障 */
    APP_CHARGER_LV_CMD_NULL0,                        /** 充电机请求：无效0 */
    APP_CHARGER_LV_CMD_NULL1,                        /** 充电机请求：无效1 */
    APP_CHARGER_LV_CMD_NULL2,                        /** 充电机请求：无效2 */
    APP_CHARGER_LV_CMD_SIZE,
}charger_lv_cmd;

typedef enum{
    APP_CHARGER_LV_F_RANK_NONE,                      /** 故障等级：无故障 */
    APP_CHARGER_LV_F_RANK_LEVEL1,                    /** 故障等级：level1 */
    APP_CHARGER_LV_F_RANK_LEVEL2,                    /** 故障等级：level2 */
    APP_CHARGER_LV_F_RANK_LEVEL3,                    /** 故障等级：level3 */
    APP_CHARGER_LV_F_RANK_SIZE,                      /** 故障等级 */
}charger_lv_f_rank;

typedef enum{
    APP_CHARGER_LV_ELOCK_UNLOCK,                     /** 电子锁状态：解锁 */
    APP_CHARGER_LV_ELOCK_LOCK,                       /** 电子锁状态：上锁 */
    APP_CHARGER_LV_ELOCK_SIZE,                       /** 电子锁状态 */
}charger_lv_elock_state;

typedef enum{
    APP_CHARGER_LV_CLINKER_DISCONNECT,               /** 充电连接器状态：断开 */
    APP_CHARGER_LV_CLINKER_CONNECTED,                /** 充电连接器状态：连接 */
    APP_CHARGER_LV_CLINKER_FAULT,                    /** 充电连接器状态：故障 */
    APP_CHARGER_LV_CLINKER_SIZE,                     /** 充电连接器状态 */
}charger_lv_clinker_state;
#endif /* CP_USING_LV_MODULE_BMS */

#if (defined(USING_TCU_CAN) && (!defined(CP_USING_CYCLE_MATRIX)))
void app_tcan_send_thread_entry(void *parameter);
void app_tcan_recv_thread_entry(void *parameter);
#endif /* (defined(USING_TCU_CAN) && (!defined(CP_USING_CYCLE_MATRIX))) */

/*******************************************
 * 函数名                app_is_using_maintenance_mode
 * 功能                    判断是否使用保养模式
 * 参数
 * 返回                    1：是         0：否
 ******************************************/
uint8_t app_is_using_maintenance_mode(void);

/*******************************************
 * 函数名                app_charge_mode_is_changed
 * 功能                    判断充电模式是否已改变
 * 参数
 * 返回                    1：是         0：否
 ******************************************/
uint8_t app_charge_mode_is_changed(void);

#ifdef CP_USING_LV_MODULE_BMS
void app_bms_lv_can_thread_entry(void *parameter);

/*************************************************
 * 函数名           app_bms_lv_get_svolt_max
 * 功能               获取电池最高单体电压
 * 参数              gunno    枪号
 * 返回              电池最高单体电压(0.01V)
 ************************************************/
uint16_t app_bms_lv_get_svolt_max(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_temp_max
 * 功能               获取电池最高温度
 * 参数              gunno    枪号
 * 返回              电池最高温度(0.1度)
 ************************************************/
int8_t app_bms_lv_get_temp_max(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_target_volt
 * 功能               获取充电目标电压
 * 参数              gunno    枪号
 * 返回              充电目标电压(0.01V)
 ************************************************/
uint16_t app_bms_lv_get_target_volt(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_target_curr
 * 功能               获取充电目标电流
 * 参数              gunno    枪号
 * 返回              充电目标电压(0.01A)
 ************************************************/
uint16_t app_bms_lv_get_target_curr(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_cmd
 * 功能               获取充电指令
 * 参数              gunno    枪号
 * 返回              充电指令@bms_lv_cmd
 ************************************************/
uint8_t app_bms_lv_get_cmd(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_is_offline
 * 功能               判断BMS是否已离线
 * 参数              gunno    枪号
 * 返回              1：是    0：否
 ************************************************/
uint8_t app_bms_lv_is_offline(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_start_charge
 * 功能               开始充电
 * 参数              gunno    枪号
 * 返回
 ************************************************/
void app_bms_lv_start_charge(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_stop_charge
 * 功能               停止充电
 * 参数              gunno    枪号
 * 返回
 ************************************************/
void app_bms_lv_stop_charge(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_start_state
 * 功能               获取启动状态
 * 参数              gunno    枪号
 * 返回              1：已启动      0：未启动
 ************************************************/
uint8_t app_bms_lv_get_start_state(uint8_t gunno);

#endif /* CP_USING_LV_MODULE_BMS */

#endif /* APPLICATIONS_INC_APP_CAN_H_ */
