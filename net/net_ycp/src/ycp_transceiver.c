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

//uint8_t ggk[] = {0xAA, 0x00, 0x06, 0x00, 0x00, 0x14, 0x01, 0x02, 0xC0, 0x71, 0x20, 0x83, 0x16, 0x02, 0x1E, 0x00,
//        0x31, 0x31, 0x34, 0x2E, 0x35, 0x35, 0x2E, 0x31, 0x31, 0x34, 0x2E, 0x31, 0x37, 0x34, 0x00, 0x00,
//        0x15, 0x00, 0x73, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0x00, 0x00, 0x73, 0x72, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0x00, 0x00, 0x2F, 0x31, 0x37, 0x31, 0x36, 0x38, 0x30, 0x35, 0x39, 0x32, 0x34, 0x37, 0x39, 0x31,
//        0x2F, 0x34, 0x44, 0x41, 0x38, 0x38, 0x30, 0x46, 0x37, 0x2E, 0x62, 0x69, 0x6E, 0x00, 0x00, 0x00,
//        0x00, 0x00, 0x02, 0x3C, 0xE2, 0x07};

//uint8_t hdj1, hk1, lld1=  0;
//
//uint8_t power_data1[160];
static void ycp_message_recv_thread_entry(void *parameter)
{
    uint8_t ycp_id[5];
    uint16_t length = 0x00;
    ycp_socket_info_t *socket = ycp_get_socket_info();
    memset(ycp_id, 0x00, sizeof(ycp_id));
    sprintf((char*)ycp_id, "%s", "YCP");

    while(1)
    {
        if(net_get_ota_info()->state < NET_OTA_STATE_LOGIN_WAIT){
            if((socket->state >= YCP_SOCKET_STATE_LOGIN_WAIT) && (socket->fd >= 0x00)){
#if 0
                if(socket->state == YCP_SOCKET_STATE_LOGIN_SUCCESS){
                    if(hdj1 == 0){
                        if(++hk1 > 160){
                            hdj1 = 1;
                            lld1 = 1;
                            memset(power_data1, 0x00, sizeof(power_data1));
                            power_data1[0] = 0xAA;
                            power_data1[1] = 0x00;
                            power_data1[2] = 0x06;
                            power_data1[3] = 0x00;
                            power_data1[4] = 0x05;

                            power_data1[5] = 0x0C;
                            power_data1[6] = 0x01;
                            power_data1[7] = strlen("121.43.69.62");

                            memcpy(&power_data1[8], "121.43.69.62", strlen("121.43.69.62"));
//                            power_data1[70] = (8767 &0x00ff);
//                            power_data1[71] = (8767 >>8);
//                            power_data1[72] = 0x00;
#if 0
                            memcpy(&power_data1[7], "123.123.121.112:8908", strlen("123.123.121.112:8908"));
                            memcpy(&power_data1[71], "file_path", strlen("file_path"));
                            memcpy(&power_data1[103], "user_name", strlen("user_name"));
                            memcpy(&power_data1[119], "password", strlen("password"));

                            power_data1[135] = 0x01;
                            power_data1[136] = 0x1E;
#endif
//                            power_data1[70] = (8767 &0xFF);
//                            power_data1[71] = (8767 >>8);
//                            power_data1[72] = 0x00;
#if 0
                            power_data1[6] = 0x00;

                            power_data1[7] = 0xB8;
                            power_data1[8] = 0xC0;
                            power_data1[9] = 0x01;
                            power_data1[10] = 0x01;
                            power_data1[11] = 0x01;
                            power_data1[12] = 0x01;
                            power_data1[13] = 0x01;
                            power_data1[14] = 0x01;
                            power_data1[15] = 0x01;
                            power_data1[16] = 0x01;
                            power_data1[17] = 0x01;
                            power_data1[18] = 0x01;
                            power_data1[19] = 0x01;
                            power_data1[20] = 0x01;
                            power_data1[21] = 0x01;
                            power_data1[22] = 0x01;
//                            power_data1[23] = 0x01;
//                            power_data1[24] = 0x01;
//                            power_data1[25] = 0x01;
//                            power_data1[26] = 0x01;
//                            power_data1[27] = 0x01;

                            power_data1[39] = 0x01;
                            power_data1[40] = 0x64;
                            power_data1[41] = 0x00;
                            power_data1[42] = 0x00;
                            power_data1[43] = 0x00;
                            power_data1[44] = 0x01;
                            power_data1[45] = 0x00;

#endif
//                            power_data[11] = 0x23;
//                            power_data[12] = 0x41;
//
//                            power_data[13] = 0x03;


//                            memcpy(&power_data[14], "12345678", strlen("12345678"));
//                            memcpy(&power_data[14], "23456789", strlen("23456789"));\
//                            memcpy(&power_data[30], "3456789a", strlen("3456789a"));

//                            power_data[15] = 0xB7;
//                            power_data[19] = 0x03;
//
//                            power_data[23] = 0xB7;
//                            power_data[27] = 0x03;
//
//                            power_data[31] = 0xB7;
//                            power_data[35] = 0x03;
//
//                            power_data[39] = 0xB7;
//                            power_data[43] = 0x03;
                        }
                    }
                }
#endif
                if((length = ycp_readline_data(socket->fd))/* || lld1*/){

//                    if(lld1){
//                        lld1 = 0;
////                        length=  sizeof(ggk);
////                        memcpy(s_ykc_message_recv_buff, ggk, sizeof(ggk));
//                        length=  10 + power_data1[7];
//                        memcpy(s_ycp_message_recv_buff, power_data1, length);
//                    }
                    rt_kprintf("zhfFH(%d, %d)\n",((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd, hk1);

                    NETDATA_DEBUG((const char*)ycp_id, s_ycp_message_recv_buff, length, NETDATA_DEBUG_DIR_RECV);

                    void *callback = NULL;
                    callback = ycp_get_service_callback(((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd);
                    if(callback){
                        uint16_t recv_check = 0x00, cal_check = 0x00;
                        cal_check = net_get_net_handle()->crc16_8005(0xFFFF, (uint8_t*)(&(((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->sequence)), \
                                (length - YCP_MESSAGE_FIX_LEN_DEFAULT));
                        recv_check = *(s_ycp_message_recv_buff + length - 0x01);
                        recv_check <<=0x08;
                        recv_check |= *(s_ycp_message_recv_buff + length - 0x02);
                        if(recv_check == cal_check){
                            LOG_D("find callback cmd(%d)", ((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd);
                            ((void (*)(uint8_t*, uint16_t))callback)(s_ycp_message_recv_buff, length);
                        }else{
                            LOG_E("ycp service id|%02X check error|%x, %x", ((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd, recv_check, cal_check);
                        }
                    }else{
                        LOG_E("ycp service id|%02X cb is not register", ((Net_YcpPro_Head_t*)s_ycp_message_recv_buff)->cmd);
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
    check_code = net_get_net_handle()->crc16_8005(0xFFFF, (uint8_t*)&(((Net_YcpPro_Head_t*)data)->sequence), \
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
