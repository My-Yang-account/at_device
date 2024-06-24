/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-05     我的杨yang       the first version
 */
#ifndef MIDDLEWARE_INC_MW_CC1_H_
#define MIDDLEWARE_INC_MW_CC1_H_

#include "stdio.h"
#include "thaisen7102Public.h"


enum cc1_state_t{
    CC1_12V = thaisenCC1_12v,      /* 按下S 但未插入插座 */
    CC1_6V = thaisenCC1_6v,        /* 枪空闲未连接或枪已插入但按下S */
    CC1_4V = thaisenCC1_4v,        /* 枪已完全连接 */
    CC1_0V = thaisenCC1_0v,        /* cc1 错误 */
};

enum cc1_state_t mw_get_cc1(uint8_t gunno);
uint8_t mw_get_cc1_value(enum cc1_state_t cc1_enum);

#endif /* MIDDLEWARE_INC_MW_CC1_H_ */
