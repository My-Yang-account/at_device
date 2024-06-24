/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-28     我的杨yang       the first version
 */
#ifndef APPLICATIONS_INC_APP_FLASH_H_
#define APPLICATIONS_INC_APP_FLASH_H_

#include "stdio.h"
#include <rtthread.h>

extern struct rt_mutex g_flash_mutex;

#define TAKE_FLASH_ACCESS_MUTEX           \
        rt_mutex_take(&g_flash_mutex, RT_WAITING_FOREVER);

#define RELEASE_FLASH_ACCESS_MUTEX        \
        rt_mutex_release(&g_flash_mutex);

int32_t app_flash_ipc_init(void);

#endif /* APPLICATIONS_INC_APP_FLASH_H_ */
