/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-28     我的杨yang       the first version
 */
#include "app_flash.h"


struct rt_mutex g_flash_mutex;

int32_t app_flash_ipc_init(void)
{
    return rt_mutex_init(&g_flash_mutex, "flash_mutex", RT_IPC_FLAG_PRIO);
}
