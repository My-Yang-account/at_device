/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-26     leven       the first version
 */

#include "net_operation.h"

#include "sgcc_smart_gun.h"

#define DBG_TAG "smart"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

#define SMART_GUN_TIME_SIZE 96 //智能充电的时间段数量

typedef struct
{
    char preTradeNo[128];                            // 1   订单流水号
    unsigned char num;                               // 2   策略配置时间段数量
    unsigned char validTime[SMART_GUN_TIME_SIZE][5]; // 3   策略生效时间//字符串数组。时间格式采用HHMM，24小时制。例如 ：[time1,time2,time3…]
    unsigned int  valiMin[SMART_GUN_TIME_SIZE];      // 4   策略生效时间//整形数组。时间格式采用分。例如 ：[time1,time2,time3…]
    unsigned short kw[SMART_GUN_TIME_SIZE];          // 5   策略配置功率//整型数组。功率精确到0.1KW[kw1,kw2,kw3…]
} smart_gun_param_t;

static smart_gun_param_t s_smart_gun_param;

///将策略生效时间从字符串转为整数
void time_str_to_min_int(void)
{
    int hour = 0, min = 0;

    for (int var = 0; var < SMART_GUN_TIME_SIZE; ++var)
    {
        hour = (s_smart_gun_param.validTime[var][0] - '0') * 10 + (s_smart_gun_param.validTime[var][1] - '0');
        min  = (s_smart_gun_param.validTime[var][2] - '0') * 10 + (s_smart_gun_param.validTime[var][3] - '0');

        s_smart_gun_param.valiMin[var] = hour * 60 + min;
    }
}

unsigned short get_current_smart_gun_kw(unsigned int t)
{
    unsigned short kw = -1;

    //...

    return kw;
}

#endif /* NET_PACK_USING_SGCC */
