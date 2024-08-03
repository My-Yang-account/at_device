/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     leven       the first version
 */

#include "sgcc_ota.h"
#include "sgcc_message_padding.h"
#include "sgcc_message_receive.h"
#include "sgcc_message_send.h"
#include "net_operation.h"
#include "ota_support.h"

#define DBG_TAG "gw_ota"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//#ifdef NET_PACK_USING_SGCC

struct ota_storage_info{
    uint8_t  app_is_update;
    uint8_t  boot_is_update;
    uint32_t pack_len;
    uint32_t check_sum;
    uint32_t update_addr;
};

static struct ota_storage_info s_ota_storage_info;
static uint32_t check_sum = 0;
static unsigned int s_data_size = 0, s_info_addr = 0, s_data_addr = 0, s_download_size = 0;
static char firmware_version[32] = "APP1.0.1-EVSDK1.1.10";

static uint32_t crc32_ieee(uint32_t crc, const uint8_t *data, size_t len)
{
    /* crc table generated from polynomial 0xedb88320 */
    static const uint32_t table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    crc = ~crc;

    for (size_t i = 0; i < len; i++) {
        uint8_t byte = data[i];

        crc = (crc >> 4) ^ table[(crc ^ byte) & 0x0f];
        crc = (crc >> 4) ^ table[(crc ^ (byte >> 4)) & 0x0f];
    }

    return (~crc);
}

void sgcc_firmware_start(uint64_t file_size)
{
    s_data_size = file_size;
    s_info_addr = THAISEN_FLASH_UPDATE_PACKAGE_INFO_ADDRESS;
    s_data_addr = THAISEN_FLASH_UPDATE_PACKAGE_DATA_ADDESSS;

    LOG_D("gw firmware start");
}

int sgcc_firmware_write(char *buffer, uint32_t length)
{
    thaisenW25qxxWrite((unsigned char *)buffer, s_data_addr, length);

    s_data_addr = s_data_addr + length;
    s_download_size = s_download_size + length;

    check_sum = crc32_ieee(check_sum, (const uint8_t *)buffer, length);

    LOG_D("gw firmware write, len: %d", length);

    return 0;
}

int sgcc_firmware_stop(int process)
{
    if (s_download_size < s_data_size || 0 != process)
    {
        LOG_D("gw firmware ota failed");

        return -1;
    }

    s_ota_storage_info.app_is_update = 0xAA;
    s_ota_storage_info.boot_is_update = 0;
    s_ota_storage_info.pack_len = s_data_size;
    s_ota_storage_info.check_sum = check_sum;
    s_ota_storage_info.update_addr = s_data_addr;

    thaisenW25qxxWrite((unsigned char *)&(s_ota_storage_info), s_info_addr, sizeof(s_ota_storage_info));

    LOG_D("gw firmware stop");

    rt_thread_mdelay(1000);

    __set_FAULTMASK(1);
    NVIC_SystemReset();

    return 0;
}

int sgcc_firmware_version(char *version)
{
    int len = strlen(firmware_version);

    memset(version, 0x0, sizeof(firmware_version));
    strncpy(version, firmware_version, sizeof(firmware_version));
    version[len] = '\0';

    LOG_D("gw firmware version: %s", version);

    return (len = strlen(version));
}

//#endif /* NET_PACK_USING_SGCC */
