/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-14     我的杨yang       the first version
 */
#ifndef APPLICATIONS_INC_APP_BILLING_RULE_H_
#define APPLICATIONS_INC_APP_BILLING_RULE_H_

#include "stdio.h"

#define APP_BILLING_RULE_PERIOD_MAX               0x30           /* 时段总数 */
#define APP_BILLING_RULE_RATE_TYPE_MAX            0x04           /* 费率类型总数 */

#define APP_RATE_TYPE_SHARP                       0x00           /* 费率类型：尖 */
#define APP_RATE_TYPE_PEAK                        0x01           /* 费率类型：峰 */
#define APP_RATE_TYPE_FLAT                        0x02           /* 费率类型：平 */
#define APP_RATE_TYPE_VALLEY                      0x03           /* 费率类型：谷 */

#define APP_BILLING_MODEL_SN_LEN                  0x10           /* 计费模型编号长度 */

/** [计费规则] */
struct billing_rule{
    struct{
        uint32_t elect : 20;                               /** 时段电费价格(精度：0.0001) */
        uint32_t service : 20;                             /** 时段服务费价格(精度：0.0001) */
        uint32_t delay : 20;                               /** 时段延迟费价格(精度：0.0001) */
        uint32_t reserve : 4;
    }period_price[APP_BILLING_RULE_PERIOD_MAX];            /** 时段价格 */
    uint8_t rate_number[APP_BILLING_RULE_PERIOD_MAX];      /** 时段费率号 */
    uint32_t rate_price[APP_BILLING_RULE_RATE_TYPE_MAX];   /** 尖、峰、平、谷费率价格(精度：0.0001) */
    uint8_t model_sn[APP_BILLING_MODEL_SN_LEN + 1];        /** 计费模型编号 */
};

uint8_t app_billingrule_is_valid(uint8_t gunno);
struct billing_rule app_billingrule_get_rule(uint8_t gunno);
void app_billingrule_update_billingrule_info(uint8_t gunno);
uint32_t app_billingrule_get_rate_type_price(uint8_t gunno, uint8_t type);
void app_billingrule_set_model_sn(uint8_t gunno, uint8_t *sn, uint8_t len);

void app_billingrule_set_period_elect_price(uint8_t gunno, uint8_t period, uint32_t price);
void app_billingrule_set_period_service_price(uint8_t gunno, uint8_t period, uint32_t price);
void app_billingrule_set_period_delay_price(uint8_t gunno, uint8_t period, uint32_t price);
void app_billingrule_set_rate_price(uint8_t gunno, uint8_t type, uint32_t price);
void app_billingrule_set_period_rate_number(uint8_t gunno, uint8_t period, uint8_t number);

uint32_t app_billingrule_get_period_elect_price(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_service_price(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_delay_price(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_rate_price(uint8_t gunno, uint8_t type);
uint8_t app_billingrule_get_period_rate_number(uint8_t gunno, uint8_t period);

uint32_t app_billingrule_get_period_elect_price(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_service_price(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_delay_price(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_rate_price(uint8_t gunno, uint8_t type);
uint8_t app_billingrule_get_period_rate_number(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_start_elcet(uint8_t gunno);
uint32_t app_billingrule_get_stop_elcet(uint8_t gunno);
uint32_t app_billingrule_get_elcet_total(uint8_t gunno);
uint32_t app_billingrule_get_fees_total(uint8_t gunno);
uint32_t app_billingrule_get_service_fees_total(uint8_t gunno);
uint32_t app_billingrule_get_rate_type_elect(uint8_t gunno, uint8_t type);
uint32_t app_billingrule_get_rate_type_fess(uint8_t gunno, uint8_t type);
uint32_t app_billingrule_get_period_elect(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_elect_fees(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_service_fees(uint8_t gunno, uint8_t period);
uint32_t app_billingrule_get_period_delay_fees(uint8_t gunno, uint8_t period);
uint8_t app_get_current_period(uint8_t gunno);
uint8_t app_calculate_current_period(uint32_t current_time);
void app_billing_info_init(uint32_t init_elect, uint8_t gunno);
void app_billing_info_calculate(uint32_t current_time, uint32_t current_elect, uint8_t gunno);

#endif /* APPLICATIONS_INC_APP_BILLING_RULE_H_ */
