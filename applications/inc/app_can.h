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
#endif /* CP_USING_LV_MODULE_BMS */

#if (defined(USING_TCU_CAN) && (!defined(CP_USING_LV_MODULE_BMS)))
void app_tcan_send_thread_entry(void *parameter);
void app_tcan_recv_thread_entry(void *parameter);
#endif /* (defined(USING_TCU_CAN) && (!defined(CP_USING_LV_MODULE_BMS))) */

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
 * 函数名           app_bms_lv_get_target_volt
 * 供能               获取充电目标电压
 * 参数              gunno    枪号
 * 返回              充电目标电压(0.01V)
 ************************************************/
uint16_t app_bms_lv_get_target_volt(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_target_curr
 * 供能               获取充电目标电流
 * 参数              gunno    枪号
 * 返回              充电目标电压(0.01A)
 ************************************************/
uint16_t app_bms_lv_get_target_curr(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_get_cmd
 * 供能               获取充电指令
 * 参数              gunno    枪号
 * 返回              充电指令@bms_lv_cmd
 ************************************************/
uint8_t app_bms_lv_get_cmd(uint8_t gunno);

/*************************************************
 * 函数名           app_bms_lv_is_offline
 * 供能               判断BMS是否已离线
 * 参数              gunno    枪号
 * 返回              1：是    0：否
 ************************************************/
uint8_t app_bms_lv_is_offline(uint8_t gunno);
#endif /* CP_USING_LV_MODULE_BMS */

#endif /* APPLICATIONS_INC_APP_CAN_H_ */
