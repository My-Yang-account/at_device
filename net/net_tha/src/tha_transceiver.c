/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-03     我的杨yang       the first version
 */
#include "tha_transceiver.h"
#include "tha_message_send.h"
#include "tha_message_struct_define.h"
#include "tha_ota.h"

#include "net_socket_interface.h"
#include "net_operation.h"

#ifdef NET_PACK_USING_THA

#define DBG_TAG "tha_sr"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

typedef struct
{
    uint8_t service_id;
    void *service_cb;
}tha_service_callback_map;

static uint8_t s_tha_service_number = 0;
static tha_service_callback_map s_tha_service_callback_map[THA_SERVICE_CALLBACK_ITEM_MAX];

static uint8_t s_tha_message_recv_buff[THA_RECV_BUFF_SIZE];
static struct rt_thread s_tha_message_recv_thread;
static uint8_t s_tha_message_recv_thread_stack[THA_RECV_THREAD_STACK_SIZE];

uint8_t tha_get_check_code(uint8_t random, uint8_t *data, uint16_t len)
{
    if(data == NULL){
        return 0;
    }
    uint8_t result = random;
    for(uint8_t count = 0x00; count < len; count++){
        result ^= data[count];
    }
    return result;
}

static uint16_t tha_readline_data(int fd)
{
    uint16_t length = 0x00, rbyte = 0x01, rlen = 0x00, body_len = 0x00;
    while(1)
    {
//        rt_kprintf("000000 fd(%d), length(%d), rbyte(%d), rlen(%d), body_len(%d)\n", fd, length, rbyte, rlen, body_len);
        if((length = net_socket_recv(fd, (s_tha_message_recv_buff + rlen), rbyte)) != rbyte){
            if(rlen == 0x00){
                return 0x00;
            }

            uint8_t rentry = 0x00;
            while(rentry < 50){
                if(net_socket_data_comein(fd, 0x01) > 0x00){
                    break;
                }
                rt_thread_mdelay(10);
                rentry++;
            }
            if(rentry >= 50){
                rlen = 0x00;
                return 0x00;
            }

            if(rbyte > length){
                rbyte -= length;
                rlen += length;
                continue;
            }else{
                rlen = 0x00;
                LOG_E("thaisen data calculate error|%d, %d", rbyte, length);
                return 0x00;
            }
        }
        rlen += rbyte;
//        rt_kprintf("11111111 fd(%d), length(%d), rbyte(%d), rlen(%d), body_len(%d)\n", fd, length, rbyte, rlen, body_len);
        if(rlen == 0x01){
            if((s_tha_message_recv_buff[rlen - 0x01] != 0x68) && (s_tha_message_recv_buff[rlen - 0x01] != 0xAA)){
                rlen = 0x00;
                continue;
            }
            rbyte = 0x05;
        }
        if(rlen == 0x06){
            body_len = (uint16_t)((s_tha_message_recv_buff[rlen - 0x01] <<8) |s_tha_message_recv_buff[rlen - 0x02]);
//            rt_kprintf("body_len(%x, %d)\n", body_len, body_len);
            if((body_len + 0x08) > THA_RECV_BUFF_SIZE){
                rlen = 0x00;
                LOG_W("thaisen net message receive buff not enough|%d, %d\n", (body_len + 0x08), THA_RECV_BUFF_SIZE);
                return 0x00;
            }
            rbyte = body_len + 0x02;
        }

        if(rlen >= (body_len + 0x08)){
            return rlen;
        }
    }
}

static void tha_message_recv_thread_entry(void *parameter)
{
    uint8_t gunno = 0x00, tha_id[5];
    uint16_t length = 0x00;
    tha_socket_info_t *socket = tha_get_socket_info();
    memset(tha_id, 0x00, sizeof(tha_id));
    while(1)
    {
        if(net_get_ota_info()->state < NET_OTA_STATE_OPEN_LINK){
            for(gunno = 0; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                if((socket->state[gunno] >= THA_SOCKET_STATE_LOGIN_WAIT) && (socket->fd[gunno] >= 0x00)){
                    if((length = tha_readline_data(socket->fd[gunno]))){

                        sprintf((char*)tha_id, "%s%c", "THA", ('A' + gunno));
                        NETDATA_DEBUG((const char*)tha_id, s_tha_message_recv_buff, length, NETDATA_DEBUG_DIR_RECV);

                        void *callback = NULL;
                        callback = tha_get_service_callback(((Net_ThaPro_Head_t*)s_tha_message_recv_buff)->cmd);
                        if(callback){
                            /* 服务回调 */
                            ((void (*)(uint8_t, uint8_t*, uint16_t))callback)(gunno, s_tha_message_recv_buff, length);
                        }else{
                            LOG_E("thaisen service id|%x cb is not register", ((Net_ThaPro_Head_t*)s_tha_message_recv_buff)->cmd);
                        }
                    }
                }
            }
        }else{
            if(net_get_ota_info()->flag.start_ota){
                if((length = tha_readline_data(net_get_ota_info()->fd))){
                    tha_ota_data_analyse((const uint8_t*)s_tha_message_recv_buff, length);
                }
            }
        }

        rt_thread_mdelay(100);
    }
}

int32_t tha_message_send_port(uint8_t cmd, uint8_t gunno, int fd, void *data, uint16_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if(len < sizeof(Net_ThaPro_Head_t)){
        return -0x02;
    }
    if(fd < 0x00){
        return -0x03;
    }

    uint8_t tha_id[5];
    Net_ThaPro_Head_t *head = (Net_ThaPro_Head_t*)data;
    head->cmd = cmd;
    head->length = len - sizeof(Net_ThaPro_Head_t);
    head->flag = 0x00;
    head->check_sum = tha_get_check_code(head->check_sum, (uint8_t*)data, (sizeof(Net_ThaPro_Head_t) - 1));
    head->check_sum = tha_get_check_code(head->check_sum, (uint8_t*)((uint8_t*)data + sizeof(Net_ThaPro_Head_t)), \
            (len - sizeof(Net_ThaPro_Head_t)));

    memset(tha_id, 0x00, sizeof(tha_id));
    sprintf((char*)tha_id, "%s%c", "THA", ('A' + gunno));
    NETDATA_DEBUG((const char*)tha_id, data, len, NETDATA_DEBUG_DIR_SEND);
    return net_socket_send(fd, data, len);
}

int tha_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port)
{
    return net_socket_open(fd, host, host_len, port);
}

int tha_socket_close(int fd)
{
    if(fd < 0x00){
        return -0x01;
    }
    return net_socket_close(fd);
}


void tha_service_callback_register(uint8_t id, void *cb)
{
    uint8_t service = 0x00;

    if(s_tha_service_number >= THA_SERVICE_CALLBACK_ITEM_MAX){
        return;
    }
    for(service = 0; service < THA_SERVICE_CALLBACK_ITEM_MAX; service++){
        if((id < s_tha_service_callback_map[service].service_id) ||
                (s_tha_service_callback_map[service].service_id == 0x00)){
            if(s_tha_service_callback_map[service].service_id == 0x00){
                s_tha_service_callback_map[service].service_id = id;
                s_tha_service_callback_map[service].service_cb = cb;
            }else{
                for(uint8_t index = s_tha_service_number; ((index > service) && (index >= 1)); index--){
                    s_tha_service_callback_map[index].service_id = s_tha_service_callback_map[index - 0x01].service_id;
                    s_tha_service_callback_map[index].service_cb = s_tha_service_callback_map[index - 0x01].service_cb;
                }
                s_tha_service_callback_map[service].service_id = id;
                s_tha_service_callback_map[service].service_cb = cb;
            }
            s_tha_service_number++;
            break;
        }
    }
}

void *tha_get_service_callback(uint8_t id)
{
    int8_t index = 0x00,
           start_index_temp = 0x00,
           start_index = 0x00,
           end_index = (s_tha_service_number - 0x01);

    for(; (end_index - start_index) > 0x03; ){
        start_index_temp = start_index;
        start_index = (id >= s_tha_service_callback_map[(end_index + start_index_temp) /0x02].service_id ?   \
                (end_index + start_index_temp) /0x02 : start_index_temp);

        end_index = (id > s_tha_service_callback_map[(end_index + start_index_temp) /0x02].service_id ?      \
                end_index : (end_index + start_index_temp) /0x02);
    }
    for(index = start_index; index <= end_index; index++){
        if(id == s_tha_service_callback_map[index].service_id){
            return s_tha_service_callback_map[index].service_cb;
        }
    }
    return NULL;
}

int32_t tha_transceiver_init(void)
{
    if(rt_thread_init(&s_tha_message_recv_thread, "tha_recv", tha_message_recv_thread_entry, NULL,
                            s_tha_message_recv_thread_stack, THA_RECV_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("thaisen message recv thread init fail");
        return -0x01;
    }
    if(rt_thread_startup(&s_tha_message_recv_thread) != RT_EOK){
        LOG_E("thaisen message recv thread startup fail");
        return -0x01;
    }
    return 0x00;
}

#endif /* NET_PACK_USING_THA */

