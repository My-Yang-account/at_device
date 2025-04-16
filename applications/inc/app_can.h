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

#include "stdio.h"

void app_tcan_send_thread_entry(void *parameter);
void app_tcan_recv_thread_entry(void *parameter);

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

#endif /* APPLICATIONS_INC_APP_CAN_H_ */
