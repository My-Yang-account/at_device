/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#include "ykc_monitor_ota.h"
#include "ykc_monitor_message_padding.h"
#include "ykc_monitor_message_receive.h"
#include "ykc_monitor_message_send.h"
#include "ykc_monitor_transceiver.h"
#include "net_operation.h"
#include "ota_support.h"

#define DBG_TAG "mykc_ota"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YKC_MONITOR
#ifdef NET_INCLUDE_OTA

/** update control */
#define NET_YKC_MONITOR_UPDATE_CONTROL_EXECUTE_IMMEDIATELY    0x01        /* 升级控制：立即执行 */
#define NET_YKC_MONITOR_UPDATE_CONTROL_EXECUTE_IDLE_STATE     0x02        /* 升级控制：空闲执行 */

#define NET_YKC_MONITOR_UPDATE_RESULT_SUCCESS                 0x00        /* 升级结果：成功 */
#define NET_YKC_MONITOR_UPDATE_RESULT_SN_INCORRECT            0x01        /* 升级结果：编号错误 */
#define NET_YKC_MONITOR_UPDATE_RESULT_MODEL_INCORRECT         0x02        /* 升级结果：程序与桩型号不符 */
#define NET_YKC_MONITOR_UPDATE_RESULT_TIMEOUT                 0x03        /* 升级结果：下载更新文件超时 */

//#define NET_YKC_MONITOR_OTA_FILE_VERSION_FLAG                 "-V01"
#define NET_YKC_MONITOR_OTA_FILE_VERSION_FLAG_LEN             0x04        /* strlen("-V01") */
#define NET_YKC_MONITOR_OTA_FILE_FLAG                         "THAISEN"   /* "THAISEN" */
#define NET_YKC_MONITOR_OTA_FILE_FLAG_LEN                     0x07
#define NET_YKC_MONITOR_FLASH_SECTOR_SIZE                     4096
#define NET_YKC_MONITOR_OTA_LOGIN_RENTRY                      10
#define NET_YKC_MONITOR_OTA_THREAD_STACK_SIZE                 2048
#define NET_YKC_MONITOR_OTA_RESPONSE_TIMEOUT                  8000
#define NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN               (NET_YKC_MONITOR_OTA_FILE_VERSION_FLAG_LEN + NET_YKC_MONITOR_OTA_FILE_FLAG_LEN)

struct ykc_monitor_server_info{
    char host[32];
    char user[16];
    char password[16];
    char path[128];
};

struct ykc_monitor_ota_storage_info{
    uint8_t  app_is_update;
    uint8_t  boot_is_update;
    uint32_t pack_len;
    uint32_t check_sum;
    uint32_t update_addr;
};

struct ykc_monitor_ota_flag{
    uint8_t is_parse : 1;
    uint8_t file_correct : 1;
    uint8_t ota_login : 1;
    uint8_t was_requested : 1;
};

static net_ota_info_t *s_ykc_monitor_ota_info = NULL;
static struct net_handle* s_handle = NULL;
static struct ykc_monitor_ota_flag s_ykc_monitor_ota_flag;
static struct ykc_monitor_server_info s_ykc_monitor_server_info;
static struct ykc_monitor_ota_storage_info s_ykc_monitor_ota_storage_info;
static struct rt_thread s_ykc_monitor_ota_thread;
static uint8_t s_ykc_monitor_ota_thread_stack[NET_YKC_MONITOR_OTA_THREAD_STACK_SIZE];

static uint32_t s_ykc_monitor_ota_timeout = 0x00;
static uint32_t s_ykc_monitor_crc, s_ykc_monitor_actual_crc;
static uint32_t s_ykc_monitor_spiflash_addr;

static uint8_t s_ykc_monitor_ota_file_flag[NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN];
static uint32_t s_ykc_monitor_start_tick = 0;
static int32_t ykc_monitor_parse_ota_data(const uint8_t *data, uint32_t len);

void ykc_monitor_set_ota_was_requested_flag(void)
{
    s_ykc_monitor_ota_flag.was_requested = 0x01;
}

uint8_t ykc_monitor_get_ota_was_requested_flag(void)
{
    return s_ykc_monitor_ota_flag.was_requested;
}

static void ykc_monitor_ota_thread_entry(void *parameter)
{
    (void)parameter;

    uint8_t port_str[8] = {0}, crc_start_addr, len;
    uint32_t pack_len = 0, crc = 0;

    while (1)
    {
        if(s_ykc_monitor_ota_flag.was_requested){
            switch(s_ykc_monitor_ota_info->state){
            case NET_OTA_STATE_NULL:
                if(net_operation_get_event(0x00, NET_OPERATION_EVENT_START_UPDATE)){
                    net_operation_clear_event(0x00, NET_OPERATION_EVENT_START_UPDATE);
                    s_ykc_monitor_ota_info->state = NET_OTA_STATE_READY;

                    LOG_D("ykc monitor receive ota request");
                    s_ykc_monitor_ota_timeout = time(NULL);
                }
                break;
            case NET_OTA_STATE_READY:
                if(g_ykc_monitor_sreq_remote_update.body.control_cmd == NET_YKC_MONITOR_UPDATE_CONTROL_EXECUTE_IDLE_STATE){
//                    while(s_ykc_monitor_ota_info->flag.permit_ota == 0x00){
                    if(s_ykc_monitor_ota_info->flag.permit_ota == 0x00){
                        if(0x01/*(time(NULL) - s_ykc_monitor_ota_timeout) > g_ykc_monitor_sreq_remote_update.body.timeout_value *60*/){
                            s_ykc_monitor_ota_flag.is_parse = 0x00;
                            s_ykc_monitor_ota_flag.file_correct = 0x00;
                            s_ykc_monitor_ota_info->flag.start_ota = 0x00;
                            s_ykc_monitor_ota_flag.ota_login = 0x00;
                            s_ykc_monitor_ota_flag.was_requested = 0x00;

                            s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
                            LOG_W("ykc monitor chargepile wait idle state timeout|%d", g_ykc_monitor_sreq_remote_update.body.timeout_value);
                            ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_TIMEOUT);
                            break;
                        }
                    }
                }else{
                    if(s_ykc_monitor_ota_info->flag.permit_ota == 0x00){
                        s_ykc_monitor_ota_flag.is_parse = 0x00;
                        s_ykc_monitor_ota_flag.file_correct = 0x00;
                        s_ykc_monitor_ota_info->flag.start_ota = 0x00;
                        s_ykc_monitor_ota_flag.ota_login = 0x00;
                        s_ykc_monitor_ota_flag.was_requested = 0x00;

                        s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
                        LOG_W("ykc monitor chargepile current state is not allow update");
                        ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_TIMEOUT);
                        break;
                    }
                }
                s_ykc_monitor_ota_storage_info.app_is_update = 0xAA;
                s_ykc_monitor_ota_storage_info.boot_is_update = 0x00;
                s_ykc_monitor_ota_storage_info.update_addr = NET_OTA_DATA_ADDR;

                s_ykc_monitor_ota_info->flag.start_ota = 0x01;
                s_ykc_monitor_ota_info->progress = 0x00;
                s_handle->flash_erase(NET_OTA_DATA_ADDR, NET_OTA_DATA_REGION_SIZE);
                s_ykc_monitor_ota_info->state = NET_OTA_STATE_OPEN_LINK;

                LOG_D("ykc monitor OTA info: user_name|%s  password|%s  path|%s", g_ykc_monitor_sreq_remote_update.body.user_name,
                        g_ykc_monitor_sreq_remote_update.body.password, g_ykc_monitor_sreq_remote_update.body.path);
                break;
            case NET_OTA_STATE_OPEN_LINK:
                if(s_ykc_monitor_ota_info->flag.start_ota){
                    /* 提取服务器信息 */
                    memcpy(s_ykc_monitor_server_info.host, g_ykc_monitor_sreq_remote_update.body.server_addr, strlen((const char *)&(g_ykc_monitor_sreq_remote_update.body.server_addr)));
                    len = strlen(s_ykc_monitor_server_info.host);
                    s_ykc_monitor_server_info.host[len] = ':';
                    sprintf((char *)&(port_str), "%u", g_ykc_monitor_sreq_remote_update.body.server_port);
                    strcat(s_ykc_monitor_server_info.host, (const char *)&(port_str));
                    memcpy(s_ykc_monitor_server_info.user, g_ykc_monitor_sreq_remote_update.body.user_name, strlen((const char *)&(g_ykc_monitor_sreq_remote_update.body.user_name)));
                    memcpy(s_ykc_monitor_server_info.password, g_ykc_monitor_sreq_remote_update.body.password, strlen((const char *)&(g_ykc_monitor_sreq_remote_update.body.password)));
                    memcpy(s_ykc_monitor_server_info.path, g_ykc_monitor_sreq_remote_update.body.path, strlen((const char *)&(g_ykc_monitor_sreq_remote_update.body.path)));

                    /* 提取CRC信息 */
                    crc_start_addr = strlen((const char *)&(g_ykc_monitor_sreq_remote_update.body.path)) - 4 - 8;
                    crc = strtoul((const char *)&(g_ykc_monitor_sreq_remote_update.body.path[crc_start_addr]), NULL, 16);

                    LOG_D("ykc monitor OTA info|%d  |%d  |0x%x  |0x%x[%s]\n", s_ykc_monitor_ota_storage_info.app_is_update,  s_ykc_monitor_ota_storage_info.boot_is_update,
                            crc, s_ykc_monitor_ota_storage_info.update_addr, s_ykc_monitor_server_info.host);

                    s_ykc_monitor_crc = 0x00;
                    s_ykc_monitor_spiflash_addr = NET_OTA_DATA_ADDR;
                    s_ykc_monitor_start_tick = rt_tick_get();

                    /* 登录FTP服务器 */
                    ftp_callback_init(ykc_monitor_parse_ota_data);
                    uint8_t rentry = 0;
                    int32_t result = 0;
                    uint16_t ver_data = 0x00;
                    while(rentry < NET_YKC_MONITOR_OTA_LOGIN_RENTRY){
                        result = ftp_linkkie_new(s_ykc_monitor_server_info.host, s_ykc_monitor_server_info.user, s_ykc_monitor_server_info.password, s_ykc_monitor_server_info.path);
                        if (0 <= result) {
                            break;
                        }
                        LOG_W("ykc monitor ota ftp link kit new failed (host is %s, result|%d, rentry|%d)", s_ykc_monitor_server_info.host, result, rentry);
                        ftp_free();
                        rentry++;
                        rt_thread_mdelay(10000);
                    }
                    if(rentry >= NET_YKC_MONITOR_OTA_LOGIN_RENTRY){
                        ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_TIMEOUT);
                        s_ykc_monitor_ota_info->state = NET_OTA_STATE_FAIL;
                        LOG_E("ykc monitor ota open socket fail");
                        ftp_free();
                        break;
                    }

                    /* 从服务器获取升级文件的长度 */
                    pack_len = ftp_file_size();
                    if ((pack_len == 0x00) || (pack_len >= NET_OTA_DATA_REGION_SIZE)) {
                        s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
                        LOG_W("ykc monitor get ftp file size error (%d), quit ota", pack_len);
                        ftp_free();
                        break;
                    }

                    s_ykc_monitor_ota_storage_info.pack_len = pack_len;
                    s_ykc_monitor_ota_storage_info.check_sum = crc;
                    memset(s_ykc_monitor_ota_file_flag, '\0', NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN);

                    LOG_D("ykc monitor ftp_file_size(%d)[%x, %x, %d], OTA FTP link success\n", pack_len, s_ykc_monitor_spiflash_addr,
                            NET_OTA_DATA_ADDR, s_ykc_monitor_ota_storage_info.pack_len);
                    s_ykc_monitor_ota_info->state = NET_OTA_STATE_LOGIN_WAIT;
                }else{
                    LOG_W("ykc monitor ota state error(%d)", s_ykc_monitor_ota_info->state);
                    s_ykc_monitor_ota_flag.is_parse = 0x00;
                    s_ykc_monitor_ota_flag.file_correct = 0x00;
                    s_ykc_monitor_ota_info->flag.start_ota = 0x00;
                    s_ykc_monitor_ota_flag.ota_login = 0x00;
                    s_ykc_monitor_ota_flag.was_requested = 0x00;
                    s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
                }
                break;
            case NET_OTA_STATE_LOGIN_WAIT:
                s_ykc_monitor_ota_info->state = NET_OTA_STATE_UPDATING;
                break;
            case NET_OTA_STATE_UPDATING:
                if(s_ykc_monitor_ota_info->flag.start_ota){
                    if (s_ykc_monitor_spiflash_addr >= NET_OTA_DATA_ADDR + s_ykc_monitor_ota_storage_info.pack_len) {
                        if(s_ykc_monitor_ota_flag.file_correct != true){
                            LOG_W("ykc monitor ota file incorrect|%s", s_ykc_monitor_ota_file_flag);
                            ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_MODEL_INCORRECT);
                            s_ykc_monitor_ota_info->state = NET_OTA_STATE_FAIL;
                        }else if (s_ykc_monitor_crc == s_ykc_monitor_ota_storage_info.check_sum) {
                            LOG_I("ykc monitor ota check success");
                            s_ykc_monitor_ota_storage_info.pack_len = pack_len - NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN;
                            s_ykc_monitor_ota_storage_info.check_sum = s_ykc_monitor_actual_crc;
                            s_ykc_monitor_ota_info->state = NET_OTA_STATE_SUCCESS;
                        } else {
                            LOG_W("ykc monitor ota check failed, crc is 0x%X", s_ykc_monitor_crc);
                            ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_TIMEOUT);
                            s_ykc_monitor_ota_info->state = NET_OTA_STATE_FAIL;
                        }
                    } else {
                        ftp_read(); /* 读取FTP数据 */
                    }
                }else{
                    LOG_W("ykc monitor ota state error(%d)", s_ykc_monitor_ota_info->state);
                    s_ykc_monitor_ota_flag.is_parse = 0x00;
                    s_ykc_monitor_ota_flag.file_correct = 0x00;
                    s_ykc_monitor_ota_info->flag.start_ota = 0x00;
                    s_ykc_monitor_ota_flag.ota_login = 0x00;
                    s_ykc_monitor_ota_flag.was_requested = 0x00;
                    s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
                }
                break;
            case NET_OTA_STATE_SUCCESS:
                if(s_ykc_monitor_ota_info->flag.start_ota){
                    s_handle->flash_erase(NET_OTA_INFO_ADDR, NET_OTA_INFO_REGION_SIZE);
                    s_handle->flash_write_directly(NET_OTA_INFO_ADDR, (uint8_t*)(&s_ykc_monitor_ota_storage_info), sizeof(s_ykc_monitor_ota_storage_info));

                    LOG_D("ykc monitor ota storage info");
                    LOG_D("app_is_update(%x) boot_is_update(%x) pack_len(%d) check_sum(%x) update_addr(%x)",
                            s_ykc_monitor_ota_storage_info.app_is_update, s_ykc_monitor_ota_storage_info.boot_is_update,
                            s_ykc_monitor_ota_storage_info.pack_len, s_ykc_monitor_ota_storage_info.check_sum,
                            s_ykc_monitor_ota_storage_info.update_addr);

                    ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_SUCCESS);
                    rt_thread_mdelay(5000);
                    __set_FAULTMASK(1);  /* 关闭所有中断 */
                    NVIC_SystemReset();  /* 重启 */
                }else{
                    LOG_W("ykc monitor ota state error(%d)", s_ykc_monitor_ota_info->state);
                    s_ykc_monitor_ota_flag.is_parse = 0x00;
                    s_ykc_monitor_ota_flag.file_correct = 0x00;
                    s_ykc_monitor_ota_info->flag.start_ota = 0x00;
                    s_ykc_monitor_ota_flag.ota_login = 0x00;
                    s_ykc_monitor_ota_flag.was_requested = 0x00;
                    s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
                }
                break;
            case NET_OTA_STATE_FAIL:
                if(s_ykc_monitor_ota_info->flag.start_ota){
                    rt_thread_mdelay(5000);
                    __set_FAULTMASK(1);  /* 关闭所有中断 */
                    NVIC_SystemReset();  /* 重启 */
                }else{
                    LOG_W("ykc monitor ota state error(%d)", s_ykc_monitor_ota_info->state);
                    s_ykc_monitor_ota_flag.is_parse = 0x00;
                    s_ykc_monitor_ota_flag.file_correct = 0x00;
                    s_ykc_monitor_ota_info->flag.start_ota = 0x00;
                    s_ykc_monitor_ota_flag.ota_login = 0x00;
                    s_ykc_monitor_ota_flag.was_requested = 0x00;
                    s_ykc_monitor_ota_info->state = NET_OTA_STATE_NULL;
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

static int32_t ykc_monitor_parse_ota_data(const uint8_t *data, uint32_t len)
{
    uint16_t actual_data_len = len;
    float progress_flaot = 0;

    /** 对升级包进行文件类型判定：正确文件类型是文件最后 11字节 为 THAISEN-V01 **/
    if (s_ykc_monitor_spiflash_addr + len >= NET_OTA_DATA_ADDR + s_ykc_monitor_ota_storage_info.pack_len) {
        uint8_t remain_ota_flag_len = NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN -  strlen((char*)s_ykc_monitor_ota_file_flag);
        memcpy((s_ykc_monitor_ota_file_flag + strlen((char*)s_ykc_monitor_ota_file_flag)), &data[len - remain_ota_flag_len], remain_ota_flag_len);
        if(memcmp(s_ykc_monitor_ota_file_flag, NET_YKC_MONITOR_OTA_FILE_FLAG, NET_YKC_MONITOR_OTA_FILE_FLAG_LEN) == 0){
            s_ykc_monitor_ota_flag.file_correct = true;   /* 文件类型正确 */
        }else{
            s_ykc_monitor_ota_flag.file_correct = false;  /* 文件类型错误 */
        }
        if(s_ykc_monitor_ota_flag.file_correct == true){
            char *ver = ((char*)s_handle->get_system_data(NET_SYSTEM_DATA_NAME_HARDWARE_VERSION, NULL, 0x00, NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR));
            if(memcmp((s_ykc_monitor_ota_file_flag + NET_YKC_MONITOR_OTA_FILE_FLAG_LEN), ver, NET_YKC_MONITOR_OTA_FILE_VERSION_FLAG_LEN) != 0){
                s_ykc_monitor_ota_flag.file_correct = false;   /* 文件版本错误 */
                LOG_W("ykc monitor ota file version error, is not allow update");
            }
        }
        actual_data_len -= remain_ota_flag_len;
    }else if((s_ykc_monitor_spiflash_addr + len) > (NET_OTA_DATA_ADDR + s_ykc_monitor_ota_storage_info.pack_len - NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN)){
        uint8_t rec_ota_flag_len = (s_ykc_monitor_spiflash_addr + len + NET_YKC_MONITOR_OTA_FILE_FLAG_TOTAL_LEN) - (NET_OTA_DATA_ADDR + s_ykc_monitor_ota_storage_info.pack_len);
        memcpy((s_ykc_monitor_ota_file_flag + strlen((char*)s_ykc_monitor_ota_file_flag)), &data[len - rec_ota_flag_len], rec_ota_flag_len);
        actual_data_len -= rec_ota_flag_len;
    }

    /** 进行升级超时判定：防止由于数据有丢失而使得 OTA 进入死循环 **/
    if(len == 0x00){
        if(rt_tick_get() < s_ykc_monitor_start_tick){
            s_ykc_monitor_start_tick = rt_tick_get();
        }
        if(rt_tick_get() - s_ykc_monitor_start_tick > NET_YKC_MONITOR_OTA_RESPONSE_TIMEOUT){
            LOG_W("ykc monitor ota response timeout|%d", NET_YKC_MONITOR_OTA_RESPONSE_TIMEOUT);
            ykc_monitor_chargepile_update_result_report(NET_YKC_MONITOR_UPDATE_RESULT_TIMEOUT);
            rt_thread_mdelay(5000);
            __set_FAULTMASK(1);  /* 关闭所有中断 */
            NVIC_SystemReset();  /* 重启 */
        }
    }else{
        s_ykc_monitor_start_tick = rt_tick_get();
    }

    /** 文件最后 11 字节不属于升级数据，需要处理掉 **/
    if(actual_data_len){
        s_handle->flash_write_directly(s_ykc_monitor_spiflash_addr, (uint8_t*)data, actual_data_len);
        s_ykc_monitor_actual_crc = s_handle->crc32_updtae(s_ykc_monitor_actual_crc, data, actual_data_len);
    }

    s_ykc_monitor_spiflash_addr += len;
    s_ykc_monitor_crc = s_handle->crc32_updtae(s_ykc_monitor_crc, data, len);

    /* 计算下载进度 */
    progress_flaot = (float)(((float)(s_ykc_monitor_spiflash_addr - s_ykc_monitor_ota_storage_info.update_addr)) / (float)(s_ykc_monitor_ota_storage_info.pack_len));
    s_ykc_monitor_ota_info->progress = progress_flaot *100 *100;
    if (s_ykc_monitor_ota_info->progress >= 10000) {
        s_ykc_monitor_ota_info->progress = 9999; /* 将进度条保持在（99.99%） */
    }

    LOG_W("ykc monitor data len is %d |%d |%d", len, s_ykc_monitor_spiflash_addr - NET_OTA_DATA_ADDR, s_ykc_monitor_ota_storage_info.pack_len);

    if (s_ykc_monitor_spiflash_addr >= NET_OTA_DATA_ADDR + s_ykc_monitor_ota_storage_info.pack_len) {
        return 0x00;
    } else {
        return 0x01;
    }
    return 0x00;
}

int32_t ykc_monitor_ota_init(void)
{
    if(rt_thread_init(&s_ykc_monitor_ota_thread, "ykc_ota", ykc_monitor_ota_thread_entry, NULL,
            s_ykc_monitor_ota_thread_stack, NET_YKC_MONITOR_OTA_THREAD_STACK_SIZE, NET_OTA_THREAD_PRIORITY, 10) != RT_EOK){
        LOG_E("ykc monitor ota thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ykc_monitor_ota_thread) != RT_EOK){
        LOG_E("ykc monitor ota thread startup fail, please check");
        return -0x01;
    }
    s_ykc_monitor_ota_info = net_get_ota_info();
    s_handle = net_get_net_handle();

    return 0x00;
}

#endif /* NET_INCLUDE_OTA */
#endif /* NET_PACK_USING_YKC_MONITOR */
