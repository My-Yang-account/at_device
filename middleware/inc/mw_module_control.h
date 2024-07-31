/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-31     我的杨yang       the first version
 */
#ifndef MIDDLEWARE_INC_MW_MODULE_CONTROL_H_
#define MIDDLEWARE_INC_MW_MODULE_CONTROL_H_

#include "thaisenChargModuleLib.h"

enum{
    APP_MODULE_ALLOW_CHARGE = thaisenAllowCharge_allow,
    APP_MODULE_FORBID_CHARGE = thaisenAllowCharge_forbid,
    APP_MODULE_CHARGE_SIZE = thaisenAllowCharge_size,
};

void mw_module_set_permit_charge_state(uint8_t state);
uint8_t mw_module_get_permit_charge_state(void);

uint8_t mw_module_is_starting(uint8_t gunno);

#endif /* MIDDLEWARE_INC_MW_MODULE_CONTROL_H_ */
