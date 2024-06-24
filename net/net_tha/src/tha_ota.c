/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-06     我的杨yang       the first version
 */
#include "tha_ota.h"
#include "tha_message_padding.h"
#include "tha_message_receive.h"
#include "tha_message_send.h"
#include "tha_transceiver.h"

#define DBG_TAG "tha_ota"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_THA
#ifdef NET_INCLUDE_OTA

#define NET_THA_OTA_FILE_VERSION_FLAG                 "-V01"
#define NET_THA_OTA_FILE_VERSION_FLAG_LEN             0x04
#define NET_THA_OTA_FILE_FLAG                         "THAISEN"
#define NET_THA_OTA_FILE_FLAG_LEN                     0x07
#define NET_THA_FLASH_SECTOR_SIZE                     4096
#define NET_THA_OTA_LOGIN_RENTRY                      10
#define NET_THA_STORAGE_PACK_SN_NUM                   5
#define NET_THA_OTA_THREAD_STACK_SIZE                 2048
#define NET_THA_OTA_RESPONSE_TIMEOUT                  3000
#define NET_THA_OTA_FILE_FLAG_TOTAL_LEN               (NET_THA_OTA_FILE_VERSION_FLAG_LEN + NET_THA_OTA_FILE_FLAG_LEN)

struct tha_ota_storage_info {
    uint8_t  app_is_update;
    uint8_t  boot_is_update;
    uint32_t pack_len;
    uint32_t check_sum;
    uint32_t update_addr;
};

struct tha_ota_flag{
    uint8_t is_parse : 1;
    uint8_t file_correct : 1;
    uint8_t write_fail : 1;
    uint8_t ota_login : 1;
    uint8_t was_requested : 1;
};

struct ota_assistion_info{
    uint8_t pack_request_sn_count : 3;
    uint8_t pack_recv_number : 3;
    uint8_t ota_gunno : 2;
};

static int16_t s_tha_request_sn[NET_THA_STORAGE_PACK_SN_NUM];
static net_ota_info_t *s_tha_ota_info = NULL;
static struct net_handle* s_handle = NULL;
static struct tha_ota_flag s_tha_ota_flag;
static struct ota_assistion_info s_tha_assistion_info;
static struct tha_ota_storage_info s_tha_ota_storage_info;
static struct rt_thread s_tha_ota_thread;
static uint8_t s_tha_ota_thread_stack[NET_THA_OTA_THREAD_STACK_SIZE];
static uint32_t s_tha_write_addr = NET_OTA_DATA_ADDR, s_tha_last_pack_len = 0x00;
static uint8_t s_tha_ota_data_wbuff[NET_THA_FLASH_SECTOR_SIZE], s_tha_ota_data_rbuff[NET_THA_FLASH_SECTOR_SIZE];
static uint8_t tha_ota_message_sequence = 0x00;

static uint32_t s_tha_crc, s_tha_actual_crc;
static uint32_t s_tha_spiflash_addr, s_tha_start_addr, s_tha_end_addr;
static uint8_t s_tha_ota_file_flag[NET_THA_OTA_FILE_FLAG_TOTAL_LEN];

void tha_set_ota_was_requested_flag(void)
{
    s_tha_ota_flag.was_requested = 0x01;
}

uint8_t tha_get_ota_was_requested_flag(void)
{
    return s_tha_ota_flag.was_requested;
}

static void tha_ipv4_to_str(char *str, uint8_t *addr, uint8_t len)
{
    uint8_t i = 0;
    if(len < 0x04){
        return;
    }
    sprintf(&str[0], "%u", addr[0]);
    i = strlen(str);
    str[i++] = '.';
    sprintf(&str[i], "%u", addr[1]);
    i = strlen(str);
    str[i++] = '.';
    sprintf(&str[i], "%u", addr[2]);
    i = strlen(str);
    str[i++] = '.';
    sprintf(&str[i], "%u", addr[3]);
}

void tha_ota_thread_entry(void *parameter)
{
    (void)parameter;
    uint32_t th = 0, time = 0, retry = 0, failed = 0, addrress = 0;

    while (1)
    {
        switch(s_tha_ota_info->state){
        case NET_OTA_STATE_NULL:
            for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                if(net_operation_get_event(gunno, NET_OPERATION_EVENT_START_UPDATE)){
                    net_operation_clear_event(gunno, NET_OPERATION_EVENT_START_UPDATE);
                    s_tha_assistion_info.ota_gunno = gunno;
                    s_tha_ota_info->state = NET_OTA_STATE_READY;
                    break;
                }
            }
            break;
        case NET_OTA_STATE_READY:
            while(s_tha_ota_info->flag.permit_ota == 0x00){
                rt_thread_mdelay(50);
            }
            s_tha_ota_storage_info.app_is_update = 0xAA;
            s_tha_ota_storage_info.boot_is_update = 0x00;
            s_tha_ota_storage_info.pack_len = g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.update_pack_len;
            s_tha_ota_storage_info.check_sum = g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.update_pack_check;
            s_tha_ota_storage_info.update_addr = NET_OTA_DATA_ADDR;

            s_tha_ota_info->flag.start_ota = 0x01;
            s_tha_ota_info->progress = 0x00;
            s_handle->flash_erase(NET_OTA_DATA_ADDR, NET_OTA_DATA_REGION_SIZE);
            s_tha_ota_info->state = NET_OTA_STATE_OPEN_LINK;

            LOG_D("thaisen OTA info: pack_len|%d  segmented_len|%d  check|%x", g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.update_pack_len,
                    g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.segment_len,
                    g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.update_pack_check);
            break;
        case NET_OTA_STATE_OPEN_LINK:
            if(s_tha_ota_info->flag.start_ota){
                int32_t result = 0x00, rentry = 0x0A;
                char host[16];
                uint16_t port;
                memset(host, '\0', sizeof(host));

                tha_ipv4_to_str(host, g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.updated_addr, 0x04);
                port = (uint16_t)(g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.updated_addr[4]) |    \
                        (uint16_t)(g_tha_sreq_start_update[s_tha_assistion_info.ota_gunno].body.updated_addr[5] << 8);

                while(rentry){
                    result = tha_socket_open(&(s_tha_ota_info->fd), host, strlen(host), port);
                    if(result >= 0x00){
                        s_tha_ota_info->state = NET_OTA_STATE_LOGIN_WAIT;
                        break;
                    }
                    tha_socket_close(s_tha_ota_info->fd);
                    rt_thread_mdelay(1000);
                    rentry--;
                }
                if(rentry == 0x00){
                    tha_chargepile_update_result_report(0x02, s_tha_assistion_info.ota_gunno);
                    s_tha_ota_info->flag.start_ota = 0x00;
                    s_tha_ota_flag.ota_login = 0x00;
                    s_tha_ota_flag.was_requested = 0x00;
                    s_tha_ota_info->state = NET_OTA_STATE_NULL;
                    LOG_E("thaisen ota open socket fail");
                }
            }else{
                LOG_W("tha ota state error(%d)", s_tha_ota_info->state);
                s_tha_ota_flag.is_parse = 0x00;
                s_tha_ota_flag.file_correct = 0x00;
                s_tha_ota_info->flag.start_ota = 0x00;
                s_tha_ota_flag.ota_login = 0x00;
                s_tha_ota_flag.was_requested = 0x00;
                s_tha_ota_info->state = NET_OTA_STATE_NULL;
            }
            break;
        case NET_OTA_STATE_LOGIN_WAIT:
            if(s_tha_ota_info->flag.start_ota){
                uint8_t rentry = 0x00, wait_res = 0x00;
                for(rentry = 0x00; rentry < NET_THA_OTA_LOGIN_RENTRY; rentry++){
                    wait_res = 0x00;
                    tha_message_send_port(NET_THA_MESSAGE_CMD_LOGIN, s_tha_assistion_info.ota_gunno, s_tha_ota_info->fd, &g_tha_preq_login[s_tha_assistion_info.ota_gunno],
                            sizeof(g_tha_preq_login[s_tha_assistion_info.ota_gunno]));
                    while(wait_res < 100){
                        if(s_tha_ota_flag.ota_login > 0x00){
                            break;
                        }
                        rt_thread_mdelay(30);
                        wait_res++;
                    }
                    if(wait_res >= 100){
                        rt_thread_mdelay(30 *1000);
                        LOG_D("thaisen ota login fail(timeout)");
                    }else{
                        break;
                    }
                }
                if(rentry == NET_THA_OTA_LOGIN_RENTRY){
                    tha_chargepile_update_result_report(0x02, s_tha_assistion_info.ota_gunno);
                    s_tha_ota_info->flag.start_ota = 0x00;
                    s_tha_ota_flag.ota_login = 0x00;
                    s_tha_ota_flag.was_requested = 0x00;
                    s_tha_ota_info->state = NET_OTA_STATE_NULL;
                }else{
                    memset(s_tha_ota_file_flag, '\0', NET_THA_OTA_FILE_FLAG_TOTAL_LEN);
                    s_tha_ota_flag.is_parse = 0x01;
                    s_tha_start_addr = 0x01;
                    s_tha_end_addr = NET_OTA_SEGMENT_LEN;
                    s_tha_spiflash_addr = NET_OTA_DATA_ADDR;
                    s_tha_ota_info->state = NET_OTA_STATE_UPDATING;
                }
            }else{
                LOG_W("tha ota state error(%d)", s_tha_ota_info->state);
                s_tha_ota_flag.is_parse = 0x00;
                s_tha_ota_flag.file_correct = 0x00;
                s_tha_ota_info->flag.start_ota = 0x00;
                s_tha_ota_flag.ota_login = 0x00;
                s_tha_ota_flag.was_requested = 0x00;
                s_tha_ota_info->state = NET_OTA_STATE_NULL;
            }
            break;
        case NET_OTA_STATE_UPDATING:
            if(s_tha_ota_info->flag.start_ota){
                if(s_tha_ota_flag.write_fail){
                    s_tha_ota_info->state = NET_OTA_STATE_FAIL;
                    break; /* 退出 */
                }
                if (s_tha_ota_flag.is_parse) {
                    s_tha_ota_flag.is_parse = 0x00;
                    if (s_tha_end_addr <= s_tha_ota_storage_info.pack_len) {
                        /* 桩向平台请求升级数据，桩处理数据时发生异常的重试退出处理 */
                        if (addrress != s_tha_end_addr) {
                            failed = 0;
                        } else {
                            failed++;
                        }
                        if (failed > 10) {
                            s_tha_ota_info->state = NET_OTA_STATE_FAIL;
                            break; /* 退出 */
                        }
                        addrress = s_tha_end_addr;

                        g_tha_preq_update_pack_load.head.sequence = (++tha_ota_message_sequence);
                        g_tha_preq_update_pack_load.head.status = NET_THA_STATUS_CODE_NORMAL;
                        g_tha_preq_update_pack_load.body.start_byte = s_tha_start_addr;
                        g_tha_preq_update_pack_load.body.end_byte = s_tha_end_addr;

                        s_tha_request_sn[s_tha_assistion_info.pack_request_sn_count] = g_tha_preq_update_pack_load.head.sequence;
                        s_tha_assistion_info.pack_request_sn_count = (s_tha_assistion_info.pack_request_sn_count >= (NET_THA_STORAGE_PACK_SN_NUM - 1)) ?  \
                                0 : (s_tha_assistion_info.pack_request_sn_count + 1);

                        LOG_I("thaisen %uth post ... start: %u, end: %u total: %u", th++, s_tha_start_addr, s_tha_end_addr,
                                s_tha_ota_storage_info.pack_len);
                        retry = 0;
                        time = rt_tick_get();
                        tha_message_send_port(NET_THA_MESSAGE_CMD_UPDATE_PACK_LOAD, s_tha_assistion_info.ota_gunno, s_tha_ota_info->fd, &g_tha_preq_update_pack_load,
                                sizeof(g_tha_preq_update_pack_load));
                    } else {
                        if(s_tha_ota_flag.file_correct != 0x01){
                            LOG_W("thaisen ota file incorrect|%s\n", s_tha_ota_file_flag);
                            s_tha_ota_info->state = NET_OTA_STATE_FAIL;
                        }else if (s_tha_crc == s_tha_ota_storage_info.check_sum) {
                            LOG_I("thaisen ota check success");
                            s_tha_ota_storage_info.pack_len -= NET_THA_OTA_FILE_FLAG_TOTAL_LEN;
                            s_tha_ota_storage_info.check_sum = s_tha_actual_crc;
                            s_tha_ota_info->state = NET_OTA_STATE_SUCCESS;
                        } else {
                            LOG_W("thaisen ota check failed, crc is 0x%X", s_tha_crc);
                            s_tha_ota_info->state = NET_OTA_STATE_FAIL;
                        }
                    }
                } else {
                    /* 桩向平台请求升级数据，平台不响应（3000 MS 超时）的重试退出处理 */
                    if (rt_tick_get() - time >= NET_THA_OTA_RESPONSE_TIMEOUT) {
                        retry++;
                        time = rt_tick_get();

                        g_tha_preq_update_pack_load.head.sequence = tha_ota_message_sequence++;
                        s_tha_request_sn[s_tha_assistion_info.pack_request_sn_count] = g_tha_preq_update_pack_load.head.sequence;
                        s_tha_assistion_info.pack_request_sn_count = (s_tha_assistion_info.pack_request_sn_count >= (NET_THA_STORAGE_PACK_SN_NUM - 1)) ?  \
                                0 : (s_tha_assistion_info.pack_request_sn_count + 1);

                        LOG_I("thaisen %uth post ... start: %u, end: %u total: %u", th, s_tha_start_addr, s_tha_end_addr,
                                s_tha_ota_storage_info.pack_len);

                        tha_message_send_port(NET_THA_MESSAGE_CMD_UPDATE_PACK_LOAD, s_tha_assistion_info.ota_gunno, s_tha_ota_info->fd, &g_tha_preq_update_pack_load,
                                sizeof(g_tha_preq_update_pack_load));
                    }
                    if (retry > 5) {
                        s_tha_ota_info->state = NET_OTA_STATE_FAIL;
                        break; /* 退出 */
                    }
                }
            }else{
                LOG_W("tha ota state error(%d)", s_tha_ota_info->state);
                s_tha_ota_flag.is_parse = 0x00;
                s_tha_ota_flag.file_correct = 0x00;
                s_tha_ota_info->flag.start_ota = 0x00;
                s_tha_ota_flag.ota_login = 0x00;
                s_tha_ota_flag.was_requested = 0x00;
                s_tha_ota_info->state = NET_OTA_STATE_NULL;
            }
            break;
        case NET_OTA_STATE_SUCCESS:
            if(s_tha_ota_info->flag.start_ota){
                /* 通知线程上送升级包下载结果 */
                tha_chargepile_update_result_report(0x00, s_tha_assistion_info.ota_gunno);
                LOG_D("APP: 0x%X, BOOT: 0x%X, PLEN: %d, CRC: 0x%X, ADDR: 0x%X",
                        s_tha_ota_storage_info.app_is_update, s_tha_ota_storage_info.boot_is_update, s_tha_ota_storage_info.pack_len,
                        s_tha_ota_storage_info.check_sum, s_tha_ota_storage_info.update_addr);

                s_handle->flash_erase(NET_OTA_INFO_ADDR, NET_OTA_INFO_REGION_SIZE); /* 擦除 */
                s_handle->flash_write_directly(NET_OTA_INFO_ADDR, (uint8_t*)&(s_tha_ota_storage_info), sizeof(s_tha_ota_storage_info));

                rt_thread_mdelay(5000);
                __set_FAULTMASK(1);  /* 关闭所有中断 */
                NVIC_SystemReset();  /* 重启 */
            }else{
                LOG_W("tha ota state error(%d)", s_tha_ota_info->state);
                s_tha_ota_flag.is_parse = 0x00;
                s_tha_ota_flag.file_correct = 0x00;
                s_tha_ota_info->flag.start_ota = 0x00;
                s_tha_ota_flag.ota_login = 0x00;
                s_tha_ota_flag.was_requested = 0x00;
                s_tha_ota_info->state = NET_OTA_STATE_NULL;
            }
            break;
        case NET_OTA_STATE_FAIL:
            if(s_tha_ota_info->flag.start_ota){
                /* 通知线程上送升级包下载结果 */
                tha_chargepile_update_result_report(0x02, s_tha_assistion_info.ota_gunno);
                rt_thread_mdelay(5000);
                __set_FAULTMASK(1);  /* 关闭所有中断 */
                NVIC_SystemReset();  /* 重启 */
            }else{
                LOG_W("tha ota state error(%d)", s_tha_ota_info->state);
                s_tha_ota_flag.is_parse = 0x00;
                s_tha_ota_flag.file_correct = 0x00;
                s_tha_ota_info->flag.start_ota = 0x00;
                s_tha_ota_flag.ota_login = 0x00;
                s_tha_ota_flag.was_requested = 0x00;
                s_tha_ota_info->state = NET_OTA_STATE_NULL;
            }
            break;
        default:
            break;
        }
        rt_thread_mdelay(50);
    }
}


static void tha_parse_ota_data(uint32_t sum, const uint8_t *data, uint32_t len)
{
    uint32_t actual_data_len = len;
    float progress_flaot = 0;

    /** 对升级包进行文件类型判定：正确文件类型是文件最后 11字节 为 THAISEN-V01 **/
    if ((s_tha_spiflash_addr + len) >= (s_tha_ota_storage_info.pack_len + s_tha_ota_storage_info.update_addr)) {
        uint8_t remain_ota_flag_len = NET_THA_OTA_FILE_FLAG_TOTAL_LEN -  strlen((char*)s_tha_ota_file_flag);
        memcpy((s_tha_ota_file_flag + strlen((char*)s_tha_ota_file_flag)), &data[len - remain_ota_flag_len], remain_ota_flag_len);
        if(memcmp(s_tha_ota_file_flag, NET_THA_OTA_FILE_FLAG, NET_THA_OTA_FILE_FLAG_LEN) == 0){
            s_tha_ota_flag.file_correct = 0x01;   /* 文件类型正确 */
        }else{
            s_tha_ota_flag.file_correct = 0x00;  /* 文件类型错误 */
        }
        if(s_tha_ota_flag.file_correct == true){
            if(memcmp((s_tha_ota_file_flag + NET_THA_OTA_FILE_FLAG_LEN), NET_THA_OTA_FILE_VERSION_FLAG, NET_THA_OTA_FILE_VERSION_FLAG_LEN) != 0){
                s_tha_ota_flag.file_correct = false;   /* 文件版本错误 */
                LOG_W("tha ota file version error, is not allow update");
            }
        }
        actual_data_len -= remain_ota_flag_len;
    }else if((s_tha_spiflash_addr + len) > (s_tha_ota_storage_info.pack_len + s_tha_ota_storage_info.update_addr - NET_THA_OTA_FILE_FLAG_TOTAL_LEN)){
        uint8_t rec_ota_flag_len = (s_tha_spiflash_addr + len + NET_THA_OTA_FILE_FLAG_TOTAL_LEN) - (NET_OTA_DATA_ADDR + s_tha_ota_storage_info.pack_len);
        memcpy((s_tha_ota_file_flag + strlen((char*)s_tha_ota_file_flag)), &data[len - rec_ota_flag_len], rec_ota_flag_len);
        actual_data_len -= rec_ota_flag_len;
    }

    /** 文件最后 11 字节不属于升级数据，需要处理掉 **/
    if(actual_data_len){
        s_tha_actual_crc = s_handle->crc32_updtae(s_tha_actual_crc, data, actual_data_len);
    }

    s_tha_spiflash_addr += len;
    s_tha_crc = s_handle->crc32_updtae(s_tha_crc, data, len);

    if(s_tha_spiflash_addr - NET_OTA_DATA_ADDR >= s_tha_ota_storage_info.pack_len){
        s_tha_last_pack_len = len;
    }
    memcpy(((uint8_t*)s_tha_ota_data_wbuff + s_tha_assistion_info.pack_recv_number *NET_OTA_SEGMENT_LEN), data, len);
    s_tha_assistion_info.pack_recv_number++;

    if(s_tha_assistion_info.pack_recv_number == 0x04 || s_tha_last_pack_len != 0x00){
        uint8_t rentry = 0;
        uint32_t read_crc = 0, write_crc = 0;
        if(s_tha_last_pack_len != 0){   /* 最后一包 */
            write_crc = s_handle->crc32_updtae(0, (uint8_t*)s_tha_ota_data_wbuff, (NET_OTA_SEGMENT_LEN *  \
                    (s_tha_assistion_info.pack_recv_number - 1) + s_tha_last_pack_len));
        }else{
            write_crc = s_handle->crc32_updtae(0, (uint8_t*)s_tha_ota_data_wbuff, NET_THA_FLASH_SECTOR_SIZE);
        }

        while(rentry <= 5){
            if(s_tha_last_pack_len != 0){   /* 最后一包 */
                LOG_W("thaisen this is last pack");
                s_handle->flash_write_directly(s_tha_write_addr, s_tha_ota_data_wbuff, (NET_OTA_SEGMENT_LEN *   \
                        (s_tha_assistion_info.pack_recv_number - 1) + s_tha_last_pack_len));

                memset(s_tha_ota_data_rbuff, 0x00, NET_THA_FLASH_SECTOR_SIZE);
                s_handle->flash_read(s_tha_write_addr, s_tha_ota_data_rbuff, (NET_OTA_SEGMENT_LEN *   \
                        (s_tha_assistion_info.pack_recv_number - 1) + s_tha_last_pack_len));
                read_crc = s_handle->crc32_updtae(0, s_tha_ota_data_rbuff, (NET_OTA_SEGMENT_LEN *     \
                        (s_tha_assistion_info.pack_recv_number - 1) + s_tha_last_pack_len));
            }else{
                s_handle->flash_write_directly(s_tha_write_addr, s_tha_ota_data_wbuff, NET_THA_FLASH_SECTOR_SIZE);

                memset(s_tha_ota_data_rbuff, 0x00, NET_THA_FLASH_SECTOR_SIZE);
                s_handle->flash_read(s_tha_write_addr, s_tha_ota_data_rbuff, NET_THA_FLASH_SECTOR_SIZE);
                read_crc = s_handle->crc32_updtae(0, s_tha_ota_data_rbuff, NET_THA_FLASH_SECTOR_SIZE);
            }

            if(read_crc == write_crc){
                break;
            }else{
                LOG_E("thaisen ota data check fail!");
                s_handle->flash_erase(s_tha_write_addr, NET_THA_FLASH_SECTOR_SIZE);
            }
            rentry++;
        }
        if(rentry > 5){
            s_tha_ota_flag.write_fail = 0x01;
            LOG_E("thaisen ota data write fail!!!!!!");
        }
        s_tha_write_addr = s_tha_spiflash_addr;
        s_tha_assistion_info.pack_recv_number = 0x00;
    }

    /* 计算下载进度 */
    progress_flaot = (float)(((float)(s_tha_spiflash_addr - s_tha_ota_storage_info.update_addr)) / (float)(s_tha_ota_storage_info.pack_len));
    s_tha_ota_info->progress = progress_flaot *100 *100;
    if (10000 <= s_tha_ota_info->progress) {
        s_tha_ota_info->progress = 9999; /* 将进度条保持在（99.99%） */
    }
    LOG_D("thaisen current load progress|%d%, %x, %x", s_tha_ota_info->progress, s_tha_spiflash_addr, s_tha_ota_storage_info.pack_len);

    if (0 == s_tha_ota_storage_info.pack_len - s_tha_end_addr) {
        s_tha_end_addr += 0x01;
    } else if (s_tha_ota_storage_info.pack_len - s_tha_end_addr < NET_OTA_SEGMENT_LEN) {
        s_tha_start_addr = s_tha_end_addr + 0x01;
        s_tha_end_addr = s_tha_start_addr + (s_tha_ota_storage_info.pack_len - s_tha_end_addr) - 0x01;
    } else {
        s_tha_start_addr = s_tha_end_addr + 0x01;
        s_tha_end_addr = s_tha_start_addr + NET_OTA_SEGMENT_LEN - 0x01;
    }

    s_tha_ota_flag.is_parse = 0x01;

    LOG_D("thaisen verify | sum: 0x%08X | crc: 0x%08X | len: 0x%02X", sum, s_tha_crc, len);
}

void tha_ota_data_analyse(const uint8_t *data, uint16_t len)
{
    if(data == NULL){
        return;
    }
    if(data[0] == NET_THA_MESSAGE_START_CODE_RES){
        if((data[1] == NET_THA_MESSAGE_CMD_LOGIN) && (s_tha_ota_flag.ota_login == 0x00)){
            Net_ThaPro_SRes_LogIn_t *ota_data = (Net_ThaPro_SRes_LogIn_t*)data;
            if(ota_data->head.status == NET_THA_STATUS_CODE_NORMAL){
                s_tha_ota_flag.ota_login = 0x01;
                g_tha_preq_update_pack_load.head.check_sum = ota_data->body.random_sign;
                LOG_D("thaisen ota server login success");
            }else{
                s_tha_ota_flag.ota_login = 0x00;
                LOG_D("thaisen ota server login fail(%d)", ota_data->head.status);
            }
        }else if((data[1] == NET_THA_MESSAGE_CMD_UPDATE_PACK_LOAD) &&(s_tha_ota_flag.ota_login == 0x01)){
            uint8_t count = 0x00;
            Net_ThaPro_SRes_UpdatePack_Load_t *ota_data = (Net_ThaPro_SRes_UpdatePack_Load_t*)data;
            for(count = 0x00; count < NET_THA_STORAGE_PACK_SN_NUM; count++){
                if(s_tha_request_sn[count] == ota_data->head.sequence){
                    memset(s_tha_request_sn, -0x01, NET_THA_STORAGE_PACK_SN_NUM);
                    break;
                }
            }
            if(count < NET_THA_STORAGE_PACK_SN_NUM){
                tha_parse_ota_data(ota_data->body.check_code, (const uint8_t*)&(ota_data->body.data), ota_data->body.pack_len);
            }
        }
    }
}

int32_t tha_ota_init(void)
{
    if(rt_thread_init(&s_tha_ota_thread, "tha_ota", tha_ota_thread_entry, NULL,
            s_tha_ota_thread_stack, NET_THA_OTA_THREAD_STACK_SIZE, NET_OTA_THREAD_PRIORITY, 10) != RT_EOK){
        LOG_E("thaisen ota thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_tha_ota_thread) != RT_EOK){
        LOG_E("thaisen ota thread startup fail, please check");
        return -0x01;
    }
    s_tha_ota_info = net_get_ota_info();
    s_handle = net_get_net_handle();
    memset(s_tha_request_sn, -0x01, NET_THA_STORAGE_PACK_SN_NUM);

    return 0x00;
}

#endif /* NET_INCLUDE_OTA */
#endif /* NET_PACK_USING_THA */
