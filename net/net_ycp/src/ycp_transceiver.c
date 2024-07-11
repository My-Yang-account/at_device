/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#include "ycp_transceiver.h"
#include "ycp_message_send.h"
#include "ycp_message_struct_define.h"
#include "ycp_ota.h"

#include "net_operation.h"

#include "net_socket_interface.h"

#ifdef NET_PACK_USING_YCP

#define DBG_TAG "ycp_sr"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define YCP_MESSAGE_FIX_LEN_DEFAULT        0x06

typedef struct
{
    uint8_t service_id;
    void *service_cb;
}ycp_service_callback_map;

static uint8_t s_ycp_service_number = 0;
static ycp_service_callback_map s_ycp_service_callback_map[YCP_SERVICE_CALLBACK_ITEM_MAX];

static uint8_t s_ycp_message_recv_buff[YCP_RECV_BUFF_SIZE];
static struct rt_thread s_ycp_message_recv_thread;
static uint8_t s_ycp_message_recv_thread_stack[YCP_RECV_THREAD_STACK_SIZE];

static uint16_t ycp_get_check_code(uint16_t crc, uint8_t *data, uint32_t len)
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


static uint16_t ycp_readline_data(int fd)
{
    uint16_t length = 0x00, rbyte = 0x01, rlen = 0x00, body_len = 0x00;
    while(1)
    {
        if((length = net_socket_recv(fd, (s_ycp_message_recv_buff + rlen), rbyte)) != rbyte){
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
                LOG_W("ycp data receive timeout, quit");
                return 0x00;
            }
            if(rbyte > length){
                rbyte -= length;
                rlen += length;
                continue;
            }else{
                rlen = 0x00;
                LOG_E("ycp data calculate error|%d, %d", rbyte, length);
                return 0x00;
            }
        }

        rlen += rbyte;
        if(rlen == 0x01){
            if(s_ycp_message_recv_buff[rlen - 0x01] != NET_YCP_MESSAGE_START_CODE){
                rlen = 0x00;
                continue;
            }
        }
        if(rlen == 0x04){
            body_len = (uint16_t)((s_ycp_message_recv_buff[rlen - 0x01] <<0x08) |s_ycp_message_recv_buff[rlen - 0x02]);
            if((body_len + 0x04) > YCP_RECV_BUFF_SIZE){
                rlen = 0x00;
                LOG_W("ycp net message receive buff not enough|%d, %d\n", (body_len + 0x04), YCP_RECV_BUFF_SIZE);
                return 0x00;
            }
            rbyte = body_len;
        }

        if(rlen >= (body_len + 0x04)){
            return rlen;
        }
    }
}

static void ycp_message_recv_thread_entry(void *parameter)
{
    uint8_t ycp_id[5];
    uint16_t length = 0x00;
    ycp_socket_info_t *socket = ycp_get_socket_info();
    memset(ycp_id, 0x00, sizeof(ycp_id));
    while(1)
    {
        if(net_get_ota_info()->state < NET_OTA_STATE_LOGIN_WAIT){
            if((socket->state >= YCP_SOCKET_STATE_LOGIN_WAIT) && (socket->fd >= 0x00)){
                if(length = ycp_readline_data(socket->fd)){

                    sprintf((char*)ycp_id, "%s", "YCP");
                    NETDATA_DEBUG((const char*)ycp_id, s_ycp_message_recv_buff, length, NETDATA_DEBUG_DIR_RECV);

                    void *callback = NULL;
                    callback = ycp_get_service_callback(((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd);
                    if(callback){
                        uint16_t recv_check = 0x00, cal_check = 0x00;
                        cal_check = ycp_get_check_code(0xFFFF, (uint8_t*)(&(((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->sequence)), \
                                (length - YCP_MESSAGE_FIX_LEN_DEFAULT));
                        recv_check = *(s_ycp_message_recv_buff + length - 0x01);
                        recv_check <<=0x08;
                        recv_check |= *(s_ycp_message_recv_buff + length - 0x02);
                        if(recv_check == cal_check){
                            LOG_D("find callback cmd(%d)", ((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd);
                            ((void (*)(uint8_t*, uint16_t))callback)(s_ycp_message_recv_buff, length);
                        }else{
                            LOG_E("ycp service id|%x check error|%x, %x", ((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd, recv_check, cal_check);
                        }
                    }else{
                        LOG_E("ycp service id|%x cb is not register", ((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd);
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

int32_t ycp_message_send_port(uint8_t cmd, int fd, void *data, uint16_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if(len < sizeof(Net_YcpPro_Head_t) + 0x01){
        return -0x02;
    }
    if(fd < 0x00){
        return -0x03;
    }

    uint8_t ycp_id[5];
    uint16_t check_code = 0x00;
    Net_YcpPro_Head_t *head = (Net_YcpPro_Head_t*)data;
    head->start_code = NET_YCP_MESSAGE_START_CODE;
    head->cmd = cmd;
    head->length = len - 0x04;
    check_code = ycp_get_check_code(0xFFFF, (uint8_t*)&(((Net_YcpPro_Head_t*)data)->sequence), \
            (len - 0x06));

    *((uint8_t*)data + len - 0x01) = (uint8_t)(check_code >>0x08);
    *((uint8_t*)data + len - 0x02) = check_code;

    memset(ycp_id, 0x00, sizeof(ycp_id));
    sprintf((char*)ycp_id, "%s", "YCP");
    NETDATA_DEBUG((const char*)ycp_id, data, len, NETDATA_DEBUG_DIR_SEND);

    return ycp_socket_send(fd, data, len);
}

int ycp_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port)
{
    return net_socket_open(fd, host, host_len, port);
}

int ycp_socket_close(int fd)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_close(fd);
}

int ycp_socket_send(int fd, void *data, uint16_t len)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_send(fd, data, len);
}

int ycp_socket_recv(int fd, void *buff, uint16_t len)
{
    return net_socket_recv(fd, buff, len);
}

int ycp_socket_data_comein(int fd, uint32_t timeout)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_data_comein(fd, timeout);
}

int ycp_socket_wait_data_write(int fd, uint32_t timeout)
{
    return net_socket_wait_data_write(fd, timeout);
}

void ycp_service_callback_register(uint8_t id, void *cb)
{
    uint8_t service = 0x00;

    if(s_ycp_service_number >= YCP_SERVICE_CALLBACK_ITEM_MAX){
        return;
    }
    for(service = 0; service < YCP_SERVICE_CALLBACK_ITEM_MAX; service++){
        if((id < s_ycp_service_callback_map[service].service_id) ||
                (s_ycp_service_callback_map[service].service_id == 0x00)){
            if(s_ycp_service_callback_map[service].service_id == 0x00){
                s_ycp_service_callback_map[service].service_id = id;
                s_ycp_service_callback_map[service].service_cb = cb;
            }else{
                for(uint8_t index = s_ycp_service_number; ((index > service) && (index >= 1)); index--){
                    s_ycp_service_callback_map[index].service_id = s_ycp_service_callback_map[index - 0x01].service_id;
                    s_ycp_service_callback_map[index].service_cb = s_ycp_service_callback_map[index - 0x01].service_cb;
                }
                s_ycp_service_callback_map[service].service_id = id;
                s_ycp_service_callback_map[service].service_cb = cb;
            }
            s_ycp_service_number++;
            break;
        }
    }
}

void *ycp_get_service_callback(uint8_t id)
{
    int8_t index = 0x00,
           start_index_temp = 0x00,
           start_index = 0x00,
           end_index = (s_ycp_service_number - 0x01);

    for(; (end_index - start_index) > 0x03; ){
        start_index_temp = start_index;
        start_index = (id >= s_ycp_service_callback_map[(end_index + start_index_temp) /0x02].service_id ?   \
                (end_index + start_index_temp) /0x02 : start_index_temp);

        end_index = (id > s_ycp_service_callback_map[(end_index + start_index_temp) /0x02].service_id ?      \
                end_index : (end_index + start_index_temp) /0x02);
    }
    for(index = start_index; index <= end_index; index++){
        if(id == s_ycp_service_callback_map[index].service_id){
            return s_ycp_service_callback_map[index].service_cb;
        }
    }
    return NULL;
}

int32_t ycp_transceiver_init(void)
{
    if(rt_thread_init(&s_ycp_message_recv_thread, "ycp_recv", ycp_message_recv_thread_entry, NULL,
                            s_ycp_message_recv_thread_stack, YCP_RECV_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ycp message recv thread init fail");
        return -0x01;
    }
    if(rt_thread_startup(&s_ycp_message_recv_thread) != RT_EOK){
        LOG_E("ycp message recv thread startup fail");
        return -0x01;
    }
    return 0x00;
}

#endif /* NET_PACK_USING_YCP */
