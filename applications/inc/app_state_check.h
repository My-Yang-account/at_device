/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-10     31638       the first version
 */
#ifndef APPLICATIONS_INC_APP_STATE_CHECK_H_
#define APPLICATIONS_INC_APP_STATE_CHECK_H_

#include "stdio.h"

/***********************************
 * 函数名      app_is_out_ov
 * 功能          检查是否已输出过压
 * 参数          gunno    枪号
 * 返回          1：是    0：否
 **********************************/
uint8_t app_is_out_ov(uint8_t gunno);

/***********************************
 * 函数名      app_is_out_uv
 * 功能          检查是否已输出欠压
 * 参数          gunno    枪号
 * 返回          1：是    0：否
 **********************************/
uint8_t app_is_out_uv(uint8_t gunno);

/***********************************
 * 函数名      app_is_out_oc
 * 功能          检查是否已输出过流
 * 参数          gunno    枪号
 * 返回          1：是    0：否
 **********************************/
uint8_t app_is_out_oc(uint8_t gunno);

/*****************************
 * 函数名      app_state_check_init
 * 功能          状态检测部分初始化
 * 参数
 * 说明          >=0：成功     <0：失败
 ****************************/
int32_t app_state_check_init(void);

#endif /* APPLICATIONS_INC_APP_STATE_CHECK_H_ */
