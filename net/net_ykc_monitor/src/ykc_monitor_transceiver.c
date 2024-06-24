/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#include "ykc_monitor_transceiver.h"
#include "ykc_monitor_message_send.h"
#include "ykc_monitor_message_struct_define.h"
#include "ykc_monitor_ota.h"

#include "net_operation.h"

#include "net_socket_interface.h"

#ifdef NET_PACK_USING_YKC_MONITOR

#define DBG_TAG "mykc_sr"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define YKC_MONITOR_MESSAGE_FIX_LEN_DEFAULT        0x04

typedef struct
{
    uint8_t service_id;
    void *service_cb;
}ykc_monitor_service_callback_map;

static uint8_t s_ykc_monitor_service_number = 0;
static ykc_monitor_service_callback_map s_ykc_monitor_service_callback_map[YKC_MONITOR_SERVICE_CALLBACK_ITEM_MAX];

static uint8_t s_ykc_monitor_message_recv_buff[YKC_MONITOR_RECV_BUFF_SIZE];
static struct rt_thread s_ykc_monitor_message_recv_thread;
static uint8_t s_ykc_monitor_message_recv_thread_stack[YKC_MONITOR_RECV_THREAD_STACK_SIZE];

static uint16_t ykc_monitor_get_check_code(uint16_t crc, uint8_t *data, uint32_t len)
{
    static uint16_t crc_talbe[] = {
        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
    };

    uint32_t i = 0;
    uint8_t ch;

    for (i = 0; i < len; i++){
        ch = *data++;
        crc = crc_talbe[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = crc_talbe[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    }
    return crc;
}

static uint8_t ykc_monitor_readline_data(int fd)
{
    uint16_t length = 0x00, rbyte = 0x01, rlen = 0x00, body_len = 0x00;
    while(1)
    {
        if((length = net_socket_recv(fd, (s_ykc_monitor_message_recv_buff + rlen), rbyte)) != rbyte){
            if(rlen == 0x00){
                return 0x00;
            }

            uint8_t rentry = 0x00;
            while(rentry < 50){
                if(net_socket_data_comein(fd, 0x01)){
                    break;
                }
                rt_thread_mdelay(10);
                rentry++;
            }
            if(rentry >= 50){
                rlen = 0x00;
                LOG_W("ykc monitor data receive timeout, quit");
                return 0x00;
            }
            if(rbyte > length){
                rbyte -= length;
                rlen += length;
                continue;
            }else{
                rlen = 0x00;
                LOG_E("ykc monitor data calculate error|%d, %d", rbyte, length);
                return 0x00;
            }
        }

        rlen += rbyte;
        if(rlen == 0x01){
            if(s_ykc_monitor_message_recv_buff[rlen - 0x01] != NET_YKC_MONITOR_MESSAGE_START_CODE){
                rlen = 0x00;
                continue;
            }
        }
        if(rlen == 0x02){
            body_len = s_ykc_monitor_message_recv_buff[rlen - 0x01];
            if((body_len + 0x04) > YKC_MONITOR_RECV_BUFF_SIZE){
                rlen = 0x00;
                LOG_W("ykc monitor net message receive buff not enough|%d, %d\n", (body_len + 0x04), YKC_MONITOR_RECV_BUFF_SIZE);
                return 0x00;
            }
            rbyte = body_len + 0x02;
        }

        if(rlen >= (body_len + 0x04)){
            return rlen;
        }
    }
}

uint8_t ggk[] = {0x68, 0x62, 0x0D, 0xD6, 0x00, 0x94, 0x32, 0x01, 0x06, 0x00, 0x20, 0x83, 0x16, 0x02, 0x1E, 0x00,
        0x31, 0x31, 0x34, 0x2E, 0x35, 0x35, 0x2E, 0x31, 0x31, 0x34, 0x2E, 0x31, 0x37, 0x34, 0x00, 0x00,
        0x15, 0x00, 0x73, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x73, 0x72, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x2F, 0x31, 0x37, 0x31, 0x36, 0x38, 0x30, 0x35, 0x39, 0x32, 0x34, 0x37, 0x39, 0x31,
        0x2F, 0x34, 0x44, 0x41, 0x38, 0x38, 0x30, 0x46, 0x37, 0x2E, 0x62, 0x69, 0x6E, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x02, 0x3C, 0xE2, 0x07};

uint8_t hdj, hk, lld=  0;

uint8_t power_data[15];
static void ykc_monitor_message_recv_thread_entry(void *parameter)
{
    uint8_t ykc_monitor_id[5];
    uint16_t length = 0x00;
    ykc_monitor_socket_info_t *socket = ykc_monitor_get_socket_info();
    memset(ykc_monitor_id, 0x00, sizeof(ykc_monitor_id));
    while(1)
    {
        if(net_get_ota_info()->state < NET_OTA_STATE_LOGIN_WAIT){
            if((socket->state >= YKC_MONITOR_SOCKET_STATE_LOGIN_WAIT) && (socket->fd >= 0x00)){
                if(socket->state == YKC_MONITOR_SOCKET_STATE_LOGIN_SUCCESS){
//                    if(hdj == 0){
//                        if(++hk > 100){
//                            hdj = 1;
//                            lld = 1;
//                            memset(power_data, 0x00, sizeof(power_data));
//                            power_data[0] = 0x68;
//                            power_data[1] = 0x11;
//                            power_data[2] = 0x00;
//                            power_data[3] = 0x41;
//                            power_data[5] = 0xB0;
//
//                            power_data[6] = 0x32;
//                            power_data[7] = 0x01;
//                            power_data[8] = 0x06;
//                            power_data[9] = 0x00;
//                            power_data[10] = 0x30;
//                            power_data[11] = 0x23;
//                            power_data[12] = 0x41;
//
//                            power_data[13] = 0x03;
//
//
//                            memcpy(&power_data[14], "12345678", strlen("12345678"));
//                            memcpy(&power_data[22], "23456789", strlen("23456789"));\
//                            memcpy(&power_data[30], "345678a", strlen("345678a"));
//
////                            power_data[15] = 0xB7;
////                            power_data[19] = 0x03;
////
////                            power_data[23] = 0xB7;
////                            power_data[27] = 0x03;
////
////                            power_data[31] = 0xB7;
////                            power_data[35] = 0x03;
////
////                            power_data[39] = 0xB7;
////                            power_data[43] = 0x03;
//                        }
//                    }
                }
                if((length = ykc_monitor_readline_data(socket->fd))/* || lld*/){
//                    if(lld){
//                        lld = 0;
////                        length=  sizeof(ggk);
////                        memcpy(s_ykc_message_recv_buff, ggk, sizeof(ggk));
//                        length=  sizeof(power_data);
//                        memcpy(s_ykc_monitor_message_recv_buff, power_data, sizeof(power_data));
//                        rt_kprintf("zhfFH(%d)\n",((Net_YkcMonitorPro_Head_t*)s_ykc_monitor_message_recv_buff)->type);
//                    }

                    sprintf((char*)ykc_monitor_id, "%s", "MYKC");
                    NETDATA_DEBUG((const char*)ykc_monitor_id, s_ykc_monitor_message_recv_buff, length, NETDATA_DEBUG_DIR_RECV);

                    void *callback = NULL;
                    callback = ykc_monitor_get_service_callback(((Net_YkcMonitorPro_Head_t*)s_ykc_monitor_message_recv_buff)->type);
                    if(callback){
                        uint16_t recv_check = 0x00, cal_check = 0x00;
                        cal_check = ykc_monitor_get_check_code(0xFFFF, (uint8_t*)&(((Net_YkcMonitorPro_Head_t*)s_ykc_monitor_message_recv_buff)->sequence), \
                                (length - YKC_MONITOR_MESSAGE_FIX_LEN_DEFAULT));
                        recv_check = *(s_ykc_monitor_message_recv_buff + length - 0x01);
                        recv_check <<=0x08;
                        recv_check |= *(s_ykc_monitor_message_recv_buff + length - 0x02);
                        if(recv_check == cal_check){
                            ((void (*)(uint8_t*, uint16_t))callback)(s_ykc_monitor_message_recv_buff, length);
                        }else{
                            LOG_E("ykc monitor service id|%x check error|%x, %x", ((Net_YkcMonitorPro_Head_t*)s_ykc_monitor_message_recv_buff)->type, recv_check, cal_check);
                        }
                    }else{
                        LOG_E("ykc monitor service id|%x cb is not register", ((Net_YkcMonitorPro_Head_t*)s_ykc_monitor_message_recv_buff)->type);
                    }
                }
            }
        }else{
            rt_thread_mdelay(5000);
            continue;
        }

        rt_thread_mdelay(100);
    }
}

int32_t ykc_monitor_message_send_port(uint8_t cmd, int fd, void *data, uint16_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if(len < sizeof(Net_YkcMonitorPro_Head_t) + 0x01){
        return -0x02;
    }
    if(fd < 0x00){
        return -0x03;
    }

    uint8_t ykc_monitor_id[5];
    uint16_t check_code = 0x00;
    Net_YkcMonitorPro_Head_t *head = (Net_YkcMonitorPro_Head_t*)data;
    head->start_code = NET_YKC_MONITOR_MESSAGE_START_CODE;
    head->type = cmd;
    head->length = len - 0x04;
    check_code = ykc_monitor_get_check_code(0xFFFF, (uint8_t*)&(((Net_YkcMonitorPro_Head_t*)data)->sequence), \
            (len - 0x04));

    *((uint8_t*)data + len - 0x01) = (uint8_t)(check_code >>0x08);
    *((uint8_t*)data + len - 0x02) = check_code;

    memset(ykc_monitor_id, 0x00, sizeof(ykc_monitor_id));
    sprintf((char*)ykc_monitor_id, "%s", "MYKC");
    NETDATA_DEBUG((const char*)ykc_monitor_id, data, len, NETDATA_DEBUG_DIR_SEND);

    return ykc_monitor_socket_send(fd, data, len);
}

int ykc_monitor_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port)
{
    return net_socket_open(fd, host, host_len, port);
}

int ykc_monitor_socket_close(int fd)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_close(fd);
}

int ykc_monitor_socket_send(int fd, void *data, uint16_t len)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_send(fd, data, len);
}

int ykc_monitor_socket_recv(int fd, void *buff, uint16_t len)
{
    return net_socket_recv(fd, buff, len);
}

int ykc_monitor_socket_data_comein(int fd, uint32_t timeout)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_data_comein(fd, timeout);
}

int ykc_monitor_socket_wait_data_write(int fd, uint32_t timeout)
{
    return net_socket_wait_data_write(fd, timeout);
}

void ykc_monitor_service_callback_register(uint8_t id, void *cb)
{
    uint8_t service = 0x00;

    if(s_ykc_monitor_service_number >= YKC_MONITOR_SERVICE_CALLBACK_ITEM_MAX){
        return;
    }
    for(service = 0; service < YKC_MONITOR_SERVICE_CALLBACK_ITEM_MAX; service++){
        if((id < s_ykc_monitor_service_callback_map[service].service_id) ||
                (s_ykc_monitor_service_callback_map[service].service_id == 0x00)){
            if(s_ykc_monitor_service_callback_map[service].service_id == 0x00){
                s_ykc_monitor_service_callback_map[service].service_id = id;
                s_ykc_monitor_service_callback_map[service].service_cb = cb;
            }else{
                for(uint8_t index = s_ykc_monitor_service_number; ((index > service) && (index >= 1)); index--){
                    s_ykc_monitor_service_callback_map[index].service_id = s_ykc_monitor_service_callback_map[index - 0x01].service_id;
                    s_ykc_monitor_service_callback_map[index].service_cb = s_ykc_monitor_service_callback_map[index - 0x01].service_cb;
                }
                s_ykc_monitor_service_callback_map[service].service_id = id;
                s_ykc_monitor_service_callback_map[service].service_cb = cb;
            }
            s_ykc_monitor_service_number++;
            break;
        }
    }
}

void *ykc_monitor_get_service_callback(uint8_t id)
{
    int8_t index = 0x00,
           start_index_temp = 0x00,
           start_index = 0x00,
           end_index = (s_ykc_monitor_service_number - 0x01);

    for(; (end_index - start_index) > 0x03; ){
        start_index_temp = start_index;
        start_index = (id >= s_ykc_monitor_service_callback_map[(end_index + start_index_temp) /0x02].service_id ?   \
                (end_index + start_index_temp) /0x02 : start_index_temp);

        end_index = (id > s_ykc_monitor_service_callback_map[(end_index + start_index_temp) /0x02].service_id ?      \
                end_index : (end_index + start_index_temp) /0x02);
    }
    for(index = start_index; index <= end_index; index++){
        if(id == s_ykc_monitor_service_callback_map[index].service_id){
            return s_ykc_monitor_service_callback_map[index].service_cb;
        }
    }
    return NULL;
}

int32_t ykc_monitor_transceiver_init(void)
{
    if(rt_thread_init(&s_ykc_monitor_message_recv_thread, "ykc_mrecv", ykc_monitor_message_recv_thread_entry, NULL,
                            s_ykc_monitor_message_recv_thread_stack, YKC_MONITOR_RECV_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ykc monitor message recv thread init fail");
        return -0x01;
    }
    if(rt_thread_startup(&s_ykc_monitor_message_recv_thread) != RT_EOK){
        LOG_E("ykc monitor message recv thread startup fail");
        return -0x01;
    }
    return 0x00;
}

#endif /* NET_PACK_USING_YKC_MONITOR */

