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
#include "app_ofsm.h"

#define DBG_TAG "gw_ota"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

/** update control */
#define NET_SGCC_UPDATE_CONTROL_EXECUTE_IMMEDIATELY    0x01        /* 升级控制：立即执行 */
#define NET_SGCC_UPDATE_CONTROL_EXECUTE_IDLE_STATE     0x02        /* 升级控制：空闲执行 */

#define NET_SGCC_UPDATE_RESULT_SUCCESS                 0x00        /* 升级结果：成功 */
#define NET_SGCC_UPDATE_RESULT_SN_INCORRECT            0x01        /* 升级结果：编号错误 */
#define NET_SGCC_UPDATE_RESULT_MODEL_INCORRECT         0x02        /* 升级结果：程序与桩型号不符 */
#define NET_SGCC_UPDATE_RESULT_TIMEOUT                 0x03        /* 升级结果：下载更新文件超时 */

//#define NET_SGCC_OTA_FILE_VERSION_FLAG                 "-V01"
#define NET_SGCC_OTA_FILE_VERSION_FLAG_LEN             0x04        /* strlen("-V01") */
#define NET_SGCC_OTA_FILE_FLAG                         "THAISEN"   /* "THAISEN" */
#define NET_SGCC_OTA_FILE_FLAG_LEN                     0x07
#define NET_SGCC_FLASH_SECTOR_SIZE                     4096
#define NET_SGCC_OTA_LOGIN_RENTRY                      10
#define NET_SGCC_OTA_THREAD_STACK_SIZE                 1024 *10
#define NET_SGCC_OTA_RESPONSE_TIMEOUT                  8000
#define NET_SGCC_OTA_BUFF_SIZE                         512
#define NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN               (NET_SGCC_OTA_FILE_VERSION_FLAG_LEN + NET_SGCC_OTA_FILE_FLAG_LEN)

struct sgcc_ota_storage_info{
    uint8_t  app_is_update;
    uint8_t  boot_is_update;
    uint32_t pack_len;
    uint32_t check_sum;
    uint32_t update_addr;
};

struct sgcc_ota_flag{
    uint8_t is_parse : 1;
    uint8_t file_correct : 1;
    uint8_t was_requested : 1;
    uint8_t ota_login : 1;
};

NET_DEF_SRAM2 static net_ota_info_t *s_sgcc_ota_info = NULL;
NET_DEF_SRAM2 static struct net_handle* s_handle = NULL;
NET_DEF_SRAM2 static struct sgcc_ota_flag s_sgcc_ota_flag;
NET_DEF_SRAM2 static struct sgcc_ota_storage_info s_sgcc_ota_storage_info;

NET_DEF_SRAM2 static uint32_t s_sgcc_crc, s_sgcc_actual_crc;
NET_DEF_SRAM2 static uint32_t s_sgcc_spiflash_addr;

NET_DEF_SRAM2 static uint8_t s_sgcc_ota_file_flag[NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN];
NET_DEF_SRAM2 static uint8_t s_sgcc_ota_buff[NET_SGCC_OTA_BUFF_SIZE];
NET_DEF_SRAM2 static char firmware_version[32] = "APP1.0.1-EVSDK1.1.7";

static uint8_t sgcc_parse_ota_data(const uint8_t *data, uint32_t len);

void sgcc_set_ota_was_requested_flag(void)
{
    s_sgcc_ota_flag.was_requested = 0x01;
}

uint8_t sgcc_get_ota_was_requested_flag(void)
{
    return s_sgcc_ota_flag.was_requested;
}

uint8_t *sgcc_get_ota_buff(void)
{
    return s_sgcc_ota_buff;
}

uint32_t sgcc_get_ota_blen(void)
{
    return NET_SGCC_OTA_BUFF_SIZE;
}

void sgcc_firmware_file_size(uint32_t file_size)
{
    s_sgcc_ota_storage_info.pack_len = file_size;
    s_sgcc_ota_info->flag.start_ota = NET_ENUM_TRUE;

    LOG_D("sgcc firmware file size(%d)", s_sgcc_ota_storage_info.pack_len);
}

int sgcc_firmware_start(void)
{
    s_sgcc_spiflash_addr = NET_OTA_DATA_ADDR;
    memset(s_sgcc_ota_file_flag, '\0', NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN);

    s_sgcc_ota_storage_info.app_is_update = 0xAA;
    s_sgcc_ota_storage_info.boot_is_update = 0x00;
    s_sgcc_ota_storage_info.update_addr = NET_OTA_DATA_ADDR;

    s_sgcc_ota_info->progress = 0x00;
    s_handle->flash_erase(NET_OTA_DATA_ADDR, NET_OTA_DATA_REGION_SIZE);

    s_sgcc_ota_info->state = NET_OTA_STATE_UPDATING;

    return 0;
}

int sgcc_firmware_write(char *buffer, uint32_t length)
{
    sgcc_parse_ota_data((const uint8_t*)buffer, length);
    return 0;
}

int sgcc_firmware_stop(void)
{
    LOG_D("sgcc_firmware_stop");
    if(s_sgcc_spiflash_addr >= NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len){
        if(s_sgcc_ota_flag.file_correct != true){
            LOG_W("sgcc ota file incorrect|%s", s_sgcc_ota_file_flag);
            s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
        }else{
            if(s_sgcc_ota_storage_info.pack_len > NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN){
                s_sgcc_ota_storage_info.pack_len -= NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN;
            }else{
                s_sgcc_ota_storage_info.pack_len = 0x00;
            }
            s_sgcc_ota_storage_info.check_sum = s_sgcc_actual_crc;
            s_sgcc_ota_info->state = NET_OTA_STATE_SUCCESS;
            LOG_I("sgcc ota check success(%x, %d)\n", s_sgcc_ota_storage_info.check_sum, s_sgcc_ota_storage_info.check_sum);
        }
    }else{
        LOG_W("sgcc ota fail, is remain length(%x, %x)", s_sgcc_spiflash_addr, (NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len));
        s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
    }

    if(s_sgcc_ota_info->state == NET_OTA_STATE_SUCCESS){
        s_handle->flash_erase(NET_OTA_INFO_ADDR, NET_OTA_INFO_REGION_SIZE);
        s_handle->flash_write_directly(NET_OTA_INFO_ADDR, (uint8_t*)(&s_sgcc_ota_storage_info), sizeof(s_sgcc_ota_storage_info));
    }else{
        s_sgcc_ota_storage_info.app_is_update = 0x00;
        s_sgcc_ota_storage_info.pack_len = 0x00;
        s_sgcc_ota_storage_info.check_sum = 0x00;
    }

    rt_thread_mdelay(5000);
    __set_FAULTMASK(1);  /* 关闭所有中断 */
    NVIC_SystemReset();  /* 重启 */

    return 0;
}

static uint8_t sgcc_parse_ota_data(const uint8_t *data, uint32_t len)
{
    uint16_t actual_data_len = len;
    float progress_flaot = 0;

    /** 对升级包进行文件类型判定：正确文件类型是文件最后 11字节 为 THAISEN-V01 **/
    if (s_sgcc_spiflash_addr + len >= NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len) {
        uint8_t remain_ota_flag_len = NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN -  strlen((char*)s_sgcc_ota_file_flag);
        memcpy((s_sgcc_ota_file_flag + strlen((char*)s_sgcc_ota_file_flag)), &data[len - remain_ota_flag_len], remain_ota_flag_len);
        if(memcmp(s_sgcc_ota_file_flag, NET_SGCC_OTA_FILE_FLAG, NET_SGCC_OTA_FILE_FLAG_LEN) == 0x00){
            s_sgcc_ota_flag.file_correct = true;   /* 文件类型正确 */
        }else{
            s_sgcc_ota_flag.file_correct = false;  /* 文件类型错误 */
        }
        if(s_sgcc_ota_flag.file_correct == true){
            char *ver = ((char*)s_handle->get_system_data(NET_SYSTEM_DATA_NAME_HARDWARE_VERSION, NULL, 0x00, NET_SYSTEM_DATA_OPTION_PLAT_SGCC));
            if(memcmp((s_sgcc_ota_file_flag + NET_SGCC_OTA_FILE_FLAG_LEN), ver, NET_SGCC_OTA_FILE_VERSION_FLAG_LEN) != 0){
                s_sgcc_ota_flag.file_correct = false;   /* 文件版本错误 */
                LOG_W("sgcc ota file version error, is not allow update[%s, %s]", (s_sgcc_ota_file_flag + NET_SGCC_OTA_FILE_FLAG_LEN), ver);
            }
        }
        actual_data_len -= remain_ota_flag_len;
    }else if((s_sgcc_spiflash_addr + len) > (NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len - NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN)){
        uint8_t rec_ota_flag_len = (s_sgcc_spiflash_addr + len + NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN) - (NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len);
        memcpy((s_sgcc_ota_file_flag + strlen((char*)s_sgcc_ota_file_flag)), &data[len - rec_ota_flag_len], rec_ota_flag_len);
        actual_data_len -= rec_ota_flag_len;
    }

    /** 文件最后 11 字节不属于升级数据，需要处理掉 **/
    if(actual_data_len){
        s_handle->flash_write_directly(s_sgcc_spiflash_addr, (uint8_t*)data, actual_data_len);
        s_sgcc_actual_crc = s_handle->crc32_updtae(s_sgcc_actual_crc, data, actual_data_len);
    }

    s_sgcc_spiflash_addr += len;
    s_sgcc_crc = s_handle->crc32_updtae(s_sgcc_crc, data, len);

    /* 计算下载进度 */
    progress_flaot = (float)(((float)(s_sgcc_spiflash_addr - s_sgcc_ota_storage_info.update_addr)) / (float)(s_sgcc_ota_storage_info.pack_len));
    s_sgcc_ota_info->progress = progress_flaot *100 *100;
    if (s_sgcc_ota_info->progress >= 10000) {
        s_sgcc_ota_info->progress = 9999; /* 将进度条保持在（99.99%） */
    }

    LOG_W("sgcc data len is %d |%d |%d", len, s_sgcc_spiflash_addr - NET_OTA_DATA_ADDR, s_sgcc_ota_storage_info.pack_len);

    if (s_sgcc_spiflash_addr >= NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len) {
        return 0x00;
    } else {
        return 0x01;
    }
    return 0x00;
}

int sgcc_ota_init(void)
{
#ifdef NET_DESIGNATE_REGION
    s_sgcc_ota_info = NULL;
    s_handle = NULL;

    s_sgcc_crc = 0x00;
    s_sgcc_actual_crc = 0x00;
    s_sgcc_spiflash_addr = 0x00;

    memset(&s_sgcc_ota_flag, 0x00, sizeof(s_sgcc_ota_flag));
    memset(&s_sgcc_ota_storage_info, 0x00, sizeof(s_sgcc_ota_storage_info));
    memset(s_sgcc_ota_file_flag, 0x00, sizeof(s_sgcc_ota_file_flag));
    memset(s_sgcc_ota_buff, 0x00, sizeof(s_sgcc_ota_buff));
    memset(firmware_version, 0x00, sizeof(firmware_version));
    memcpy(firmware_version, "APP1.0.1-EVSDK1.1.7", strlen("APP1.0.1-EVSDK1.1.7"));
#endif /* NET_DESIGNATE_REGION */

    s_sgcc_ota_info = net_get_ota_info();
    s_handle = net_get_net_handle();

    return 0x00;
}

int sgcc_firmware_version(char *version)
{
    int len = strlen(firmware_version);
    System_BaseData *base = (System_BaseData*)(net_get_net_handle()->get_base_data(0x00));

    memset(version, 0x0, sizeof(firmware_version));

    sprintf(version, "V%d.%d.%d", base->soft_ver_main, base->soft_ver_sub, base->soft_ver_revise);

    version[len] = '\0';

    LOG_D("gw firmware version: %s", version);

    return (len = strlen(version));
}

#endif /* NET_PACK_USING_SGCC */
