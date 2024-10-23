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

#if 0
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






#else
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

static net_ota_info_t *s_sgcc_ota_info = NULL;
static struct net_handle* s_handle = NULL;
static struct sgcc_ota_flag s_sgcc_ota_flag;
static struct sgcc_ota_storage_info s_sgcc_ota_storage_info;
static struct rt_thread s_sgcc_ota_thread;
static uint8_t s_sgcc_ota_thread_stack[NET_SGCC_OTA_THREAD_STACK_SIZE];

static uint32_t s_sgcc_crc, s_sgcc_actual_crc;
static uint32_t s_sgcc_spiflash_addr;

static uint8_t s_sgcc_ota_file_flag[NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN];
static uint8_t s_sgcc_ota_buff[NET_SGCC_OTA_BUFF_SIZE];
static char firmware_version[32] = "APP1.0.1-EVSDK1.1.10";
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
    sgcc_parse_ota_data(buffer, length);
    return 0;
}

int sgcc_firmware_stop(void)
{
    rt_kprintf("sgcc_firmware_stop\n");
//    if(s_sgcc_spiflash_addr >= NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len){
//        if(s_sgcc_ota_flag.file_correct != true){
//            LOG_W("sgcc ota file incorrect|%s", s_sgcc_ota_file_flag);
//            s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
//        }else if (s_sgcc_crc == s_sgcc_ota_storage_info.check_sum) {
//            if(s_sgcc_ota_storage_info.pack_len > NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN){
//                s_sgcc_ota_storage_info.pack_len -= NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN;
//            }else{
//                s_sgcc_ota_storage_info.pack_len = 0x00;
//            }
//            s_sgcc_ota_storage_info.check_sum = s_sgcc_actual_crc;
//            s_sgcc_ota_info->state = NET_OTA_STATE_SUCCESS;
//            LOG_I("sgcc ota check success(%x, %d)\n", s_sgcc_ota_storage_info.check_sum, s_sgcc_ota_storage_info.check_sum);
//        }else{
//            LOG_W("sgcc ota check failed, crc is 0x%X", s_sgcc_crc);
//            s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
//        }
//    }else{
//        LOG_W("sgcc ota fail, is remain length(%x, %x)", s_sgcc_spiflash_addr, (NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len));
//        s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
//    }
//
//    if(s_sgcc_ota_info->state == NET_OTA_STATE_SUCCESS){
//        s_handle->flash_erase(NET_OTA_INFO_ADDR, NET_OTA_INFO_REGION_SIZE);
//        s_handle->flash_write_directly(NET_OTA_INFO_ADDR, (uint8_t*)(&s_sgcc_ota_storage_info), sizeof(s_sgcc_ota_storage_info));
//    }else{
//        s_sgcc_ota_storage_info.app_is_update = 0x00;
//        s_sgcc_ota_storage_info.pack_len = 0x00;
//        s_sgcc_ota_storage_info.check_sum = 0x00;
//    }
//
//    rt_thread_mdelay(5000);
//    __set_FAULTMASK(1);  /* 关闭所有中断 */
//    NVIC_SystemReset();  /* 重启 */

    return 0;
}
//#if 0
static void sgcc_ota_thread_entry(void *parameter)
{
    (void)parameter;

    uint8_t port_str[8] = {0}, crc_start_addr, len;
    uint32_t pack_len = 0, crc = 0;

    while (1)
    {
        if(s_sgcc_ota_flag.was_requested){
            switch(s_sgcc_ota_info->state){
            case NET_OTA_STATE_NULL:
                if(net_operation_get_event(0x00, NET_OPERATION_EVENT_START_UPDATE)){
                    net_operation_clear_event(0x00, NET_OPERATION_EVENT_START_UPDATE);
                    s_sgcc_ota_info->state = NET_OTA_STATE_READY;
                }
                break;
            case NET_OTA_STATE_READY:
                if(s_sgcc_ota_info->flag.permit_ota == 0x00){
                    s_sgcc_ota_flag.is_parse = 0x00;
                    s_sgcc_ota_flag.file_correct = 0x00;
                    s_sgcc_ota_info->flag.start_ota = 0x00;
                    s_sgcc_ota_flag.ota_login = 0x00;
                    s_sgcc_ota_flag.was_requested = 0x00;

                    s_sgcc_ota_info->state = NET_OTA_STATE_NULL;
                    LOG_W("sgcc chargepile current state is not allow update");
//                    sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_TIMEOUT);
                    break;
                }

                s_sgcc_spiflash_addr = NET_OTA_DATA_ADDR;
                memset(s_sgcc_ota_file_flag, '\0', NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN);

                s_sgcc_ota_storage_info.app_is_update = 0xAA;
                s_sgcc_ota_storage_info.boot_is_update = 0x00;
                s_sgcc_ota_storage_info.update_addr = NET_OTA_DATA_ADDR;

                s_sgcc_ota_info->progress = 0x00;
                s_handle->flash_erase(NET_OTA_DATA_ADDR, NET_OTA_DATA_REGION_SIZE);
                s_sgcc_ota_info->state = NET_OTA_STATE_OPEN_LINK;
                break;
            case NET_OTA_STATE_OPEN_LINK:
                s_sgcc_ota_info->state = NET_OTA_STATE_LOGIN_WAIT;
                break;
            case NET_OTA_STATE_LOGIN_WAIT:
                s_sgcc_ota_info->state = NET_OTA_STATE_UPDATING;
                break;
            case NET_OTA_STATE_UPDATING:
            {
                LOG_D("sgcc ota start");
                int res = evs_linkkit_fota(s_sgcc_ota_buff, NET_SGCC_OTA_BUFF_SIZE);
                if(res < 0x00){
                    LOG_W("sgcc ota fail result(%x)", res);
//                    sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_TIMEOUT);
                    s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
                    break;
                }

                if(s_sgcc_ota_info->flag.start_ota){
                    if (s_sgcc_spiflash_addr >= NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len) {
                        if(s_sgcc_ota_flag.file_correct != true){
                            LOG_W("sgcc ota file incorrect|%s", s_sgcc_ota_file_flag);
//                            sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_MODEL_INCORRECT);
                            s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
                        }else if (s_sgcc_crc == s_sgcc_ota_storage_info.check_sum) {
                            LOG_I("sgcc ota check success");
                            s_sgcc_ota_storage_info.pack_len = pack_len - NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN;
                            s_sgcc_ota_storage_info.check_sum = s_sgcc_actual_crc;
                            s_sgcc_ota_info->state = NET_OTA_STATE_SUCCESS;
                        } else {
                            LOG_W("sgcc ota check failed, crc is 0x%X", s_sgcc_crc);
//                            sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_TIMEOUT);
                            s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
                        }
                    }else{
                        LOG_W("sgcc ota fail, is remain length(%x, %x)", s_sgcc_spiflash_addr, (NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len));
//                        sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_TIMEOUT);
                        s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
                        break;
                    }
                }else{
                    LOG_W("sgcc ota is not start");
//                    sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_TIMEOUT);
                    s_sgcc_ota_info->state = NET_OTA_STATE_FAIL;
                    break;
                }
            }
                break;
            case NET_OTA_STATE_SUCCESS:
                if(s_sgcc_ota_info->flag.start_ota){
                    s_handle->flash_erase(NET_OTA_INFO_ADDR, NET_OTA_INFO_REGION_SIZE);
                    s_handle->flash_write_directly(NET_OTA_INFO_ADDR, (uint8_t*)(&s_sgcc_ota_storage_info), sizeof(s_sgcc_ota_storage_info));

//                    sgcc_chargepile_update_result_report(NET_SGCC_UPDATE_RESULT_SUCCESS);
                    rt_thread_mdelay(5000);
                    __set_FAULTMASK(1);  /* 关闭所有中断 */
                    NVIC_SystemReset();  /* 重启 */
                }else{
                    LOG_W("sgcc ota state error(%d)", s_sgcc_ota_info->state);
                    s_sgcc_ota_flag.is_parse = 0x00;
                    s_sgcc_ota_flag.file_correct = 0x00;
                    s_sgcc_ota_info->flag.start_ota = 0x00;
                    s_sgcc_ota_flag.ota_login = 0x00;
                    s_sgcc_ota_flag.was_requested = 0x00;
                    s_sgcc_ota_info->state = NET_OTA_STATE_NULL;
                }
                break;
            case NET_OTA_STATE_FAIL:
                if(s_sgcc_ota_info->flag.start_ota){
                    rt_thread_mdelay(5000);
                    __set_FAULTMASK(1);  /* 关闭所有中断 */
                    NVIC_SystemReset();  /* 重启 */
                }else{
                    LOG_W("sgcc ota state error(%d)", s_sgcc_ota_info->state);
                    s_sgcc_ota_flag.is_parse = 0x00;
                    s_sgcc_ota_flag.file_correct = 0x00;
                    s_sgcc_ota_info->flag.start_ota = 0x00;
                    s_sgcc_ota_flag.ota_login = 0x00;
                    s_sgcc_ota_flag.was_requested = 0x00;
                    s_sgcc_ota_info->state = NET_OTA_STATE_NULL;
                }
                break;
            default:
                break;
            }
            rt_thread_mdelay(1);
        }else{
            rt_thread_mdelay(100);
        }
    }
}
//#endif
static uint8_t sgcc_parse_ota_data(const uint8_t *data, uint32_t len)
{
    uint16_t actual_data_len = len;
    float progress_flaot = 0;

    /** 对升级包进行文件类型判定：正确文件类型是文件最后 11字节 为 THAISEN-V01 **/
    if (s_sgcc_spiflash_addr + len >= NET_OTA_DATA_ADDR + s_sgcc_ota_storage_info.pack_len) {
        uint8_t remain_ota_flag_len = NET_SGCC_OTA_FILE_FLAG_TOTAL_LEN -  strlen((char*)s_sgcc_ota_file_flag);
        memcpy((s_sgcc_ota_file_flag + strlen((char*)s_sgcc_ota_file_flag)), &data[len - remain_ota_flag_len], remain_ota_flag_len);
        if(memcmp(s_sgcc_ota_file_flag, NET_SGCC_OTA_FILE_FLAG, NET_SGCC_OTA_FILE_FLAG_LEN) == 0){
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
//        s_handle->flash_write_directly(s_sgcc_spiflash_addr, (uint8_t*)data, actual_data_len);
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
//#if 0
    if(rt_thread_init(&s_sgcc_ota_thread, "sgcc_ota", sgcc_ota_thread_entry, NULL,
            s_sgcc_ota_thread_stack, NET_SGCC_OTA_THREAD_STACK_SIZE, NET_OTA_THREAD_PRIORITY, 10) != RT_EOK){
        LOG_E("sgcc ota thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_sgcc_ota_thread) != RT_EOK){
        LOG_E("sgcc ota thread startup fail, please check");
        return -0x01;
    }
//#endif
    s_sgcc_ota_info = net_get_ota_info();
    s_handle = net_get_net_handle();

    return 0x00;
}







int sgcc_firmware_version(char *version)
{
    int len = strlen(firmware_version);

    memset(version, 0x0, sizeof(firmware_version));
    strncpy(version, "V1.3.2", strlen("V1.3.2"));

    version[len] = '\0';

    LOG_D("gw firmware version: %s", version);

    return (len = strlen(version));
}


#endif
//#endif /* NET_PACK_USING_SGCC */
