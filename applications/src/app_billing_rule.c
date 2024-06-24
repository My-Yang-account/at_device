/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-14     我的杨yang       the first version
 */
#include "app_billing_rule.h"
#include "app_ofsm.h"

/** [计费信息] */
struct billing_info{
    struct{
        uint32_t elect : 20;                                   /** 时段电费费用(精度：0.0001) */
        uint32_t service : 20;                                 /** 时段服务费费用(精度：0.0001) */
        uint32_t delay : 20;                                   /** 时段延迟费费用(精度：0.0001) */
        uint32_t reserve : 4;
    }period_fees[APP_BILLING_RULE_PERIOD_MAX];                 /** 时段费用 */
    uint8_t period_number;                                     /** 当前时段号 */
    uint32_t period_elect[APP_BILLING_RULE_PERIOD_MAX];        /** 时段电量(精度：0.001) */
    uint32_t rate_type_elect[APP_BILLING_RULE_RATE_TYPE_MAX];  /** 尖、峰、平、谷费率内消耗的电量(精度：0.001) */
    uint32_t rate_type_fess[APP_BILLING_RULE_RATE_TYPE_MAX];   /** 尖、峰、平、谷费率内消耗的费用(精度：0.0001) */
    uint32_t elect_total;                                      /** 充电总电量(精度：0.001) */
    uint32_t fees_total;                                       /** 充电总费用(精度：0.0001) */
    uint32_t service_fees_total;                               /** 充电总服务费(精度：0.0001) */
    uint32_t start_elect;                                      /** 充电起始电量(精度：0.001) */
    uint32_t stop_elect;                                       /** 充电结束电量(精度：0.001) */
};

static struct billing_info s_billing_info[APP_SYSTEM_GUNNO_SIZE];
static struct billing_rule s_billing_rule[APP_SYSTEM_GUNNO_SIZE + 1];

/*******************************************************
 * 函数名               app_billingrule_is_valid
 * 功能                  判断计费规则是否有效
 * 参数                  gunno     枪号
 * 返回
 ******************************************************/
uint8_t app_billingrule_is_valid(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    for(uint8_t type = 0x00; type < APP_BILLING_RULE_PERIOD_MAX; type++){
        if(!((s_billing_rule[gunno].period_price[type].elect == 0x00) && (s_billing_rule[gunno].period_price[type].service == 0x00))){
            return 0x01;
        }
    }
    return 0x00;
}

/*******************************************************
 * 函数名               app_billingrule_get_rule
 * 功能                  获取指定枪的计费规则
 * 参数                  gunno     枪号
 * 返回
 ******************************************************/
struct billing_rule app_billingrule_get_rule(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return s_billing_rule[APP_SYSTEM_GUNNOA];
    }
    return s_billing_rule[gunno];
}

/*******************************************
 * 函数名            app_billingrule_update_billingrule_info
 * 功能                更新对应枪的计费策略
 ******************************************/
void app_billingrule_update_billingrule_info(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    memcpy(&s_billing_rule[gunno], &s_billing_rule[APP_SYSTEM_GUNNO_SIZE], sizeof(s_billing_rule[gunno]));
}

/*******************************************
 * 函数名            app_billingrule_get_rate_type_price
 * 功能                获取费率价格
 ******************************************/
uint32_t app_billingrule_get_rate_type_price(uint8_t gunno, uint8_t type)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(type > APP_BILLING_RULE_RATE_TYPE_MAX){
        return 0x00;
    }
    return s_billing_rule[gunno].rate_price[type];
}

/****************************************************[计费规则信息]**********************************************************/
/****************************************************[计费规则信息]**********************************************************/
/*****************************************************************************
 * 函数名               app_billingrule_set_period_elect_price
 * 功能                  设置时段电费价格
 * 参数                  gunno     枪号
 *           period    时段
 *           price     价格
 * 返回
 ****************************************************************************/
void app_billingrule_set_period_elect_price(uint8_t gunno, uint8_t period, uint32_t price)
{
    if(gunno > APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return;
    }
    s_billing_rule[gunno].period_price[period].elect = price;
}
/*****************************************************************************
 * 函数名               app_billingrule_set_period_service_price
 * 功能                  设置时段服务费价格
 * 参数                  gunno     枪号
 *           period    时段
 *           price     价格
 * 返回
 ****************************************************************************/
void app_billingrule_set_period_service_price(uint8_t gunno, uint8_t period, uint32_t price)
{
    if(gunno > APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return;
    }
    s_billing_rule[gunno].period_price[period].service = price;
}
/*****************************************************************************
 * 函数名               app_billingrule_set_period_delay_price
 * 功能                  设置时段延时费价格
 * 参数                  gunno     枪号
 *           period    时段
 *           price     价格
 * 返回
 ****************************************************************************/
void app_billingrule_set_period_delay_price(uint8_t gunno, uint8_t period, uint32_t price)
{
    if(gunno > APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return;
    }
    s_billing_rule[gunno].period_price[period].delay = price;
}
/*****************************************************************************
 * 函数名               app_billingrule_set_rate_price
 * 功能                  设置费率价格
 * 参数                  gunno     枪号
 *           type      费率类型
 *           price     价格
 * 返回
 ****************************************************************************/
void app_billingrule_set_rate_price(uint8_t gunno, uint8_t type, uint32_t price)
{
    if(gunno > APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    if(type >= APP_BILLING_RULE_RATE_TYPE_MAX){
        return;
    }
    s_billing_rule[gunno].rate_price[type] = price;
}
/*****************************************************************************
 * 函数名               app_billingrule_set_period_rate_number
 * 功能                  设置时段费率号
 * 参数                  gunno     枪号
 *           period    时段
 *           number    费率号
 * 返回
 ****************************************************************************/
void app_billingrule_set_period_rate_number(uint8_t gunno, uint8_t period, uint8_t number)
{
    if(gunno > APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    if(number >= APP_BILLING_RULE_RATE_TYPE_MAX){
        return;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return;
    }
    s_billing_rule[gunno].rate_number[period] = number;
}

/****************************************************[计费信息]**********************************************************/
/****************************************************[计费信息]**********************************************************/
/*****************************************************************
 * 函数名               app_billingrule_get_period_elect_price
 * 功能                  获取时段电费价格
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  时段电费
 ****************************************************************/
uint32_t app_billingrule_get_period_elect_price(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }
    return s_billing_rule[gunno].period_price[period].elect;
}
/*****************************************************************
 * 函数名               app_billingrule_get_period_elect_price
 * 功能                  获取时段服务费价格
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  时段服务费
 ****************************************************************/
uint32_t app_billingrule_get_period_service_price(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }
    return s_billing_rule[gunno].period_price[period].service;
}
/*****************************************************************
 * 函数名               app_billingrule_get_period_elect_price
 * 功能                  获取时段延时费价格
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  时段延时费
 ****************************************************************/
uint32_t app_billingrule_get_period_delay_price(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }
    return s_billing_rule[gunno].period_price[period].delay;
}
/*******************************************************
 * 函数名               app_billingrule_get_rate_price
 * 功能                  获取费率价格
 * 参数                  gunno     枪号
 *           type      费率类型
 * 返回                  费率价格
 ******************************************************/
uint32_t app_billingrule_get_rate_price(uint8_t gunno, uint8_t type)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(type >= APP_BILLING_RULE_RATE_TYPE_MAX){
        return 0x00;
    }
    return s_billing_rule[gunno].rate_price[type];
}
/*******************************************************
 * 函数名               app_billingrule_get_period_rate_number
 * 功能                  获取时段费率号
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  时段费率号
 ******************************************************/
uint8_t app_billingrule_get_period_rate_number(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_RATE_TYPE_VALLEY;
    }
    if(period >= APP_BILLING_RULE_PERIOD_MAX){
        return APP_RATE_TYPE_VALLEY;
    }
    return s_billing_rule[gunno].rate_number[period];
}

/*******************************************************
 * 函数名               app_billingrule_get_start_elcet
 * 功能                  获取起始电量
 * 参数                  gunno     枪号
 * 返回                  起始电量
 ******************************************************/
uint32_t app_billingrule_get_start_elcet(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    return s_billing_info[gunno].start_elect;
}
/*******************************************************
 * 函数名               app_billingrule_get_stop_elcet
 * 功能                  获取终止电量
 * 参数                  gunno     枪号
 * 返回                  终止电量
 ******************************************************/
uint32_t app_billingrule_get_stop_elcet(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    return s_billing_info[gunno].stop_elect;
}
/*******************************************************
 * 函数名               app_billingrule_get_elcet_total
 * 功能                  获取充电总电量
 * 参数                  gunno     枪号
 * 返回                  充电总电量
 ******************************************************/
uint32_t app_billingrule_get_elcet_total(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    return s_billing_info[gunno].elect_total;
}
/*******************************************************
 * 函数名              app_billingrule_get_fees_total
 * 功能                  获取充电总费用
 * 参数                  gunno     枪号
 * 返回                  充电总费用
 ******************************************************/
uint32_t app_billingrule_get_fees_total(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    return s_billing_info[gunno].fees_total /100;
}
/*******************************************************
 * 函数名               app_billingrule_get_service_fees_total
 * 功能                  获取充电总服务费
 * 参数                  gunno     枪号
 * 返回                  充电总费用
 ******************************************************/
uint32_t app_billingrule_get_service_fees_total(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    return s_billing_info[gunno].service_fees_total /100;
}
/*******************************************************
 * 函数名               app_billingrule_get_rate_type_elect
 * 功能                  获取指定费率类型总电量
 * 参数                  gunno     枪号
 *           type      费率类型
 * 返回                  指定费率类型总电量
 ******************************************************/
uint32_t app_billingrule_get_rate_type_elect(uint8_t gunno, uint8_t type)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(type > APP_BILLING_RULE_RATE_TYPE_MAX){
        return 0x00;
    }

    return s_billing_info[gunno].rate_type_elect[type];
}
/*******************************************************
 * 函数名               app_billingrule_get_rate_type_fess
 * 功能                  获取指定费率类型总费用
 * 参数                  gunno     枪号
 *           type      费率类型
 * 返回                  指定费率类型总费用
 ******************************************************/
uint32_t app_billingrule_get_rate_type_fess(uint8_t gunno, uint8_t type)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(type > APP_BILLING_RULE_RATE_TYPE_MAX){
        return 0x00;
    }

    return s_billing_info[gunno].rate_type_fess[type] /100;
}
/*******************************************************
 * 函数名               app_billingrule_get_period_elect
 * 功能                  获取指定时段总电量
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  指定时段总电量
 ******************************************************/
uint32_t app_billingrule_get_period_elect(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period > APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }

    return s_billing_info[gunno].period_elect[period];
}
/*******************************************************
 * 函数名               app_billingrule_get_period_elect_fees
 * 功能                  获取指定时段总电费
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  指定时段总电费
 ******************************************************/
uint32_t app_billingrule_get_period_elect_fees(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period > APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }

    return s_billing_info[gunno].period_fees[period].elect /100;
}
/*******************************************************
 * 函数名               app_billingrule_get_period_service_fees
 * 功能                  获取指定时段总服务费
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  指定时段总服务费
 ******************************************************/
uint32_t app_billingrule_get_period_service_fees(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period > APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }

    return s_billing_info[gunno].period_fees[period].service /100;
}
/*******************************************************
 * 函数名               app_billingrule_get_period_delay_fees
 * 功能                  获取指定时段总延迟费
 * 参数                  gunno     枪号
 *           period    时段
 * 返回                  指定时段总延迟费
 ******************************************************/
uint32_t app_billingrule_get_period_delay_fees(uint8_t gunno, uint8_t period)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(period > APP_BILLING_RULE_PERIOD_MAX){
        return 0x00;
    }

    return s_billing_info[gunno].period_fees[period].delay /100;
}

/*******************************************************
 * 函数名               app_get_current_period
 * 功能                  获取当前时段
 * 参数                  current_time     当前时间
 * 返回                  当前时段
 ******************************************************/
uint8_t app_get_current_period(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_BILLING_RULE_PERIOD_MAX;
    }
    if(s_billing_info[gunno].period_number >= APP_BILLING_RULE_PERIOD_MAX){
        return APP_BILLING_RULE_PERIOD_MAX;
    }

    return s_billing_info[gunno].period_number;
}

/*******************************************************
 * 函数名               app_calculate_current_period
 * 功能                  计算当前时段
 * 参数                  current_time     当前时间
 * 返回                  当前时段
 ******************************************************/
uint8_t app_calculate_current_period(uint32_t current_time)
{
    uint8_t period;
    struct tm *_tm;

    _tm = localtime((time_t*)&current_time);

    current_time = _tm->tm_hour *60 *60 + _tm->tm_min *60 + _tm->tm_sec;
    period = current_time / (24 * 60 / APP_BILLING_RULE_PERIOD_MAX * 60);

    return period;
}
/*******************************************************
 * 函数名               app_billing_info_init
 * 功能                  初始化费率信息
 * 参数                  init_elect     初始电量(精度：0.001)
 *         gunno          枪号
 * 返回
 ******************************************************/
void app_billing_info_init(uint32_t init_elect, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    s_billing_info[gunno].start_elect = init_elect;
    s_billing_info[gunno].stop_elect = s_billing_info[gunno].start_elect;
    s_billing_info[gunno].elect_total = 0x00;
    s_billing_info[gunno].fees_total = 0x00;
    s_billing_info[gunno].service_fees_total = 0x00;
    for(uint8_t period = 0; period < APP_BILLING_RULE_PERIOD_MAX; period++){
        s_billing_info[gunno].period_elect[period] = 0x00;
        s_billing_info[gunno].period_fees[period].elect = 0x00;
        s_billing_info[gunno].period_fees[period].service = 0x00;
        s_billing_info[gunno].period_fees[period].delay = 0x00;
    }
}
/*******************************************************
 * 函数名              app_billing_info_calculate
 * 功能                  计算费率信息
 * 参数                  current_time     当前时间(北京时间)
 *         current_elect    当前电量(精度：0.001)
 *         gunno            枪号
 * 返回
 ******************************************************/
void app_billing_info_calculate(uint32_t current_time, uint32_t current_elect, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }

    uint8_t period = 0x00, rate_number = 0x00;
    uint32_t elect_inc = 0x00;

    if((current_elect <= s_billing_info[gunno].stop_elect) || (current_elect <= s_billing_info[gunno].start_elect)){
        return;
    }

    period = app_calculate_current_period(current_time);
    rate_number = s_billing_rule[gunno].rate_number[period];
    elect_inc = current_elect - s_billing_info[gunno].stop_elect;

    s_billing_info[gunno].period_number = period;
    s_billing_info[gunno].stop_elect = current_elect;

    if(s_billing_info[gunno].stop_elect > s_billing_info[gunno].start_elect){
        s_billing_info[gunno].elect_total = s_billing_info[gunno].stop_elect - s_billing_info[gunno].start_elect;
    }
    s_billing_info[gunno].period_elect[period] += elect_inc;
    s_billing_info[gunno].period_fees[period].elect += (elect_inc *s_billing_rule[gunno].period_price[period].elect /10);
    s_billing_info[gunno].period_fees[period].service += (elect_inc *s_billing_rule[gunno].period_price[period].service /10);
    s_billing_info[gunno].period_fees[period].delay += (elect_inc *s_billing_rule[gunno].period_price[period].delay /10);
    s_billing_info[gunno].service_fees_total += (elect_inc *s_billing_rule[gunno].period_price[period].service /10);
    s_billing_info[gunno].fees_total += (elect_inc *(s_billing_rule[gunno].period_price[period].elect +
            s_billing_rule[gunno].period_price[period].service + elect_inc *s_billing_rule[gunno].period_price[period].delay) /10);
    s_billing_info[gunno].rate_type_elect[rate_number] += elect_inc;
    s_billing_info[gunno].rate_type_fess[rate_number] += (elect_inc *s_billing_rule[gunno].rate_price[rate_number] /10);

    rt_kprintf("gunno(%d)  period(%d) rate_number(%d) elect_inc(%d) rate_price(%d, %d, %d)\n", gunno, period, rate_number, elect_inc,
            s_billing_rule[gunno].rate_price[rate_number], s_billing_info[gunno].rate_type_fess[rate_number],
            s_billing_info[gunno].rate_type_elect[rate_number]);
}

