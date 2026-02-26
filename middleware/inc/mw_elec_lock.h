/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-07     我的杨yang       the first version
 */
#ifndef MIDDLEWARE_INC_MW_ELEC_LOCK_H_
#define MIDDLEWARE_INC_MW_ELEC_LOCK_H_

#include "thaisen7102Public.h"

enum elec_lock_ops_t
{
    ELEC_LOCK_OPERATE_SUCCESS = thaisen_elect_lock_ok,    /* 锁止或解锁电磁锁成功 */
    ELEC_LOCK_OPERATE_FAIL = thaisen_elect_lock_fail,     /* 锁止或解锁电磁锁失败 */
};

enum elec_lock_state_t
{
    ELEC_LOCK_STATE_UNLOCK,   /* 电磁锁状态：解锁 */
    ELEC_LOCK_STATE_LOCK,     /* 电磁锁状态：锁止 */
};


enum elec_lock_state_t mw_get_elec_lock_state(void);
enum elec_lock_ops_t mw_operate_elec_lock(unsigned char gunno, enum elec_lock_state_t ops_state);


#endif /* MIDDLEWARE_INC_MW_ELEC_LOCK_H_ */
