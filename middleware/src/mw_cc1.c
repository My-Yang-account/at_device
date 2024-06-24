/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-05     我的杨yang       the first version
 */
#include "mw_cc1.h"
#include "app_ofsm.h"

static uint8_t s_cc1_state_filter[APP_SYSTEM_GUNNO_SIZE];
static enum cc1_state_t s_cc1_state_last[APP_SYSTEM_GUNNO_SIZE];

enum cc1_state_t mw_get_cc1(uint8_t gunno)
{
    enum cc1_state_t current_state;
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            current_state = thaisen_get_CC1_status();
            if(current_state != s_cc1_state_last[APP_SYSTEM_GUNNOA]){
                if(++s_cc1_state_filter[APP_SYSTEM_GUNNOA] > 3){
                    s_cc1_state_filter[APP_SYSTEM_GUNNOA] = 0;
                    s_cc1_state_last[APP_SYSTEM_GUNNOA] = current_state;
                }
            }
            return (enum cc1_state_t)(s_cc1_state_last[APP_SYSTEM_GUNNOA]);
        }else{
#ifdef APP_USING_DOUBLEGUN
            current_state = thaisen_get_CC1_statusB();
            if(current_state != s_cc1_state_last[APP_SYSTEM_GUNNOB]){
                if(++s_cc1_state_filter[APP_SYSTEM_GUNNOB] > 3){
                    s_cc1_state_filter[APP_SYSTEM_GUNNOB] = 0;
                    s_cc1_state_last[APP_SYSTEM_GUNNOB] = current_state;
                }
            }
            return (enum cc1_state_t)(s_cc1_state_last[APP_SYSTEM_GUNNOB]);
#endif /* APP_USING_DOUBLEGUN */
        }
    }
    return CC1_0V;
}

uint8_t mw_get_cc1_value(enum cc1_state_t cc1_enum)
{
    uint8_t cc1_value = 0;
    switch(cc1_enum)
    {
    case CC1_12V:
        cc1_value = 12;
        break;
    case CC1_6V:
        cc1_value = 6;
        break;
    case CC1_4V:
        cc1_value = 4;
        break;
    default:
        break;
    }
    return cc1_value;
}
