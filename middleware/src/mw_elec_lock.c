/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-07     我的杨yang       the first version
 */

#include "mw_elec_lock.h"


enum elec_lock_state_t mw_get_elec_lock_state(void)
{
    return (enum elec_lock_state_t)thaisenGetElectLockFeedbackSta();
}

enum elec_lock_ops_t mw_operate_elec_lock(unsigned char gunno, enum elec_lock_state_t ops_state)
{
    if(gunno == 0x00){
        if(ops_state == ELEC_LOCK_STATE_UNLOCK)
            return (enum elec_lock_ops_t)thaisenElectUnlock();
        return (enum elec_lock_ops_t)thaisenElectLock();
    }else{
        if(ops_state == ELEC_LOCK_STATE_UNLOCK)
            return (enum elec_lock_ops_t)thaisenElectUnlockB();
        return (enum elec_lock_ops_t)thaisenElectLockB();
    }
}

