/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#include "net_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "sal_low_lvl.h"
#include "at_socket.h"

#define DBG_TAG "sock_port"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void _port_uint2str(uint16_t input, char *output)
{
    uint8_t i = 0, j = 0;
    char temp[6] = {0};

    do {
        temp[i++] = input % 10 + '0';
    } while ((input /= 10) > 0);

    do {
        output[--i] = temp[j++];
    } while (i > 0);
}

int app_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port)
{
    uint8_t block_connect = 0x01;
    int result = 0x00, fd = 0x00, flag = 0x00;
    char port_str[6];
    struct addrinfo hints;
    struct addrinfo *addr_list;

    memset(&hints, 0x00, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;         /* only IPv4 */
    hints.ai_socktype = SOCK_STREAM;   /* socket 类型 */
    hints.ai_protocol = IPPROTO_TCP;   /* 协议 */
    hints.ai_flags = 0;                /* 标志 */

    _port_uint2str(port, port_str);

    result = getaddrinfo(host, port_str, &hints, &addr_list);

    if(result != RT_EOK){
        freeaddrinfo(addr_list);
        LOG_E("get address info fail, result|%d", result);
        return -0x01;
    }

    fd = socket(addr_list->ai_family, addr_list->ai_socktype, addr_list->ai_protocol);
    if(fd < 0x00){
        freeaddrinfo(addr_list);
        LOG_E("socket create fail, result|%d", fd);
        return -0x02;
    }
    flag = fcntl(fd, F_GETFL);
    if(flag < 0x00){
        block_connect = 0x00;
    }else{
        flag &= (~O_NONBLOCK);
        result = fcntl(fd, F_SETFL, flag);
        if(result < 0x00){
            block_connect = 0x00;
        }
    }

    if(block_connect){
        result = connect(fd, addr_list->ai_addr, addr_list->ai_addrlen);
        if(errno == EINPROGRESS){
            fd_set writefds;
            struct timeval timeselect;

            FD_ZERO(&writefds);
            FD_SET(fd, &writefds);

            timeselect.tv_sec = 10 *1000;
            timeselect.tv_usec = 0x00;
            result = select(fd + 1, NULL, &writefds, NULL, &timeselect);
            if (result == 0x00){
                freeaddrinfo(addr_list);
                LOG_E("fail to establish link with host[%s:%d] fd|%d timeot", host, port, fd);
                return -0x04;
            }else if (result < 0x00){
                freeaddrinfo(addr_list);
                LOG_E("fail to establish link with host[%s:%d] fd|%d", host, port, fd);
                return -0x04;
            }else{
                if (FD_ISSET(fd, &writefds)){
                    result = connect(fd, addr_list->ai_addr, addr_list->ai_addrlen);
                    if ((result != 0x00 && errno == EISCONN) || result == 0x00){

                    }else{
                        freeaddrinfo(addr_list);
                        LOG_E("fail to establish link with host[%s:%d] fd|%d error", host, port, fd);
                        return -0x06;
                    }
                }else{
                    freeaddrinfo(addr_list);
                    LOG_E("fail to establish link with host[%s:%d] fd|%d timeot", host, port, fd);
                    return -0x05;
                }
            }
        }else if(result != 0x00){
            freeaddrinfo(addr_list);
            LOG_E("fail to establish link with host[%s:%d] fd|%d", host, port, fd);
            return -0x03;
        }
    }else{
        result = connect(fd, addr_list->ai_addr, addr_list->ai_addrlen);
        if(result != 0x00){
            freeaddrinfo(addr_list);
            LOG_E("fail to establish link with host[%s:%d] fd|%d", host, port, fd);
            return -0x03;
        }
    }
    LOG_D("success to establish link with host[%s:%d] fd|%d", host, port, fd);
    if(socket_fd){
        *socket_fd = fd;
        struct sal_socket *sal_sock = sal_get_socket(dfs_net_getsocket(fd));
        struct at_socket *at_sock = at_get_socket((int)sal_sock->user_data);
        if(at_sock){
            at_sock->recv_timeout = 0x0A;
        }
    }

    freeaddrinfo(addr_list);
    return 0x00;

#if 0
    struct sal_socket *sal_sock = NULL;
    struct at_socket *at_sock = RT_NULL;
    struct addrinfo ai, *ret_ai;
    char port_str[6];
    int fd, ret = 0;

    memset(port_str, '\0', 6);
    memset(&ai, 0x00, sizeof(struct addrinfo));

    ai.ai_family = AF_UNSPEC;
    ai.ai_socktype = SOCK_STREAM;
    ai.ai_protocol = IPPROTO_TCP;

    _port_uint2str(port, port_str);

    if((ret = getaddrinfo(host, port_str, &ai, &ret_ai)) == 0){
        if((fd = socket(ret_ai->ai_family, ret_ai->ai_socktype, ret_ai->ai_protocol)) >= 0){
            if(connect(fd, ret_ai->ai_addr, ret_ai->ai_addrlen) == 0){
                if(socket_fd){
                    *socket_fd = fd;
                    sal_sock = sal_get_socket(dfs_net_getsocket(fd));
                    at_sock = at_get_socket((int)sal_sock->user_data);
                    if(at_sock){
                        at_sock->recv_timeout = 0xC8;
                    }
                    LOG_D("http connect to host[%s:%d] success, socketfd(%d)", host, port, fd);
                }
            }else{
                ret = -1;
                LOG_D("http connect to host[%s:%d] fail", host, port);
            }
        }else{
            ret = -2;
            LOG_D("http init socket fail");
        }
    }else{
        LOG_D("http get address fail host[%s:%d], %d", host, port, ret);
        ret = -3;
    }
    return ret;
#endif
}
//#if 0
int app_socket_send_port(int socket_fd, void *data, uint16_t len)
{
    if(data == NULL){
        return -0x01;
    }
    int32_t result = 0x00;
    uint8_t overreturn = 0x00;
    uint8_t sendbytes = 0x00;
    uint32_t start_tick = rt_tick_get(), remain_tick = 0x00, timeout = 5000;
    fd_set writefds;
    struct timeval timeselect;

    while(1){
        if(start_tick > rt_tick_get()){
            if(overreturn == 0x00){
                overreturn = 0x01;
            }
        }
        if((start_tick + overreturn *0xFFFFFFFF - rt_tick_get()) > timeout){
            return -0x05;
        }else{
            remain_tick = timeout - (start_tick + overreturn *0xFFFFFFFF - rt_tick_get());
            timeselect.tv_sec = remain_tick /1000;
            timeselect.tv_usec = remain_tick %1000;
        }
        FD_ZERO(&writefds);
        FD_SET(socket_fd, &writefds);
        result = select(socket_fd + 1, NULL, &writefds, NULL, &timeselect);
        if (result == 0x00){
            LOG_W("select wait allow write timeout");
            continue;
        }
        else if (result < 0x00){
            LOG_W("select wait allow write fail errno:%d", errno);
            return -0x02;
        }else{
            if (FD_ISSET(socket_fd, &writefds)){
                result = send(socket_fd, ((uint8_t*)data + sendbytes), (len - sendbytes), 0);
                if (result == 0x00){
                    LOG_W("socket have closed when send data, send byte|%d, %d", len, sendbytes);
                    return -0x03;
                }else if (result < 0x00){
                    LOG_W("socket send data error errno: (%d, %d)", errno, result);
                    if (errno == EINTR){
                        continue;
                    }
                    return -0x04;
                }else{
                    sendbytes += result;
                    if (sendbytes == len){
                        return len;
                    }
                }
            }
        }
    }
}
//#endif /* 0 */

#if 0
int net_socket_send(int socket_fd, void *data, uint16_t len)
{
    if(data == NULL){
        return -0x01;
    }

    int32_t result = 0x00;
    uint8_t overreturn = 0x00;
    uint8_t sendbytes = 0x00;
    uint32_t start_tick = rt_tick_get(), remain_tick = 0x00, timeout = 5000;
    fd_set writefds;
    struct timeval timeselect;

//    timeselect.tv_sec = timeout /1000;
//    timeselect.tv_usec = timeout %1000;
//    FD_ZERO(&writefds);
//    FD_SET(socket_fd, &writefds);
//    result = select(socket_fd + 1, NULL, &writefds, NULL, &timeselect);
//    if (result == 0x00){
//        LOG_W("select wait allow write timeout");
//        return -0x02;
//    }
//    else if (result < 0x00){
//        LOG_W("select wait allow write fail errno:%d", errno);
//        return -0x02;
//    }else{
//        if (FD_ISSET(socket_fd, &writefds)){
//            result = send(socket_fd, (uint8_t*)data, len, 0);
//            if (result == 0x00){
//                LOG_W("socket have closed when send data, send byte|%d, %d", len, sendbytes);
//                return -0x03;
//            }else if (result < 0x00){
//                LOG_W("socket send data error errno: (%d, %d)", errno, result);
//                return -0x04;
//            }
//        }
//    }
//
//    return 0x00;

    while(1){
        if(start_tick > rt_tick_get()){
            if(overreturn == 0x00){
                overreturn = 0x01;
            }
        }
        if((start_tick + overreturn *0xFFFFFFFF - rt_tick_get()) > timeout){
            return -0x05;
        }else{
            remain_tick = timeout - (start_tick + overreturn *0xFFFFFFFF - rt_tick_get());
            timeselect.tv_sec = remain_tick /1000;
            timeselect.tv_usec = remain_tick %1000;
        }
        FD_ZERO(&writefds);
        FD_SET(socket_fd, &writefds);
        result = select(socket_fd + 1, NULL, &writefds, NULL, &timeselect);
        if (result == 0x00){
            LOG_W("select wait allow write timeout(%d)", socket_fd);
            continue;
        }
        else if (result < 0x00){
            LOG_W("select wait allow write fail errno:%d|%d", errno, socket_fd);
            return -0x02;
        }else{
            if (FD_ISSET(socket_fd, &writefds)){
                result = send(socket_fd, ((uint8_t*)data + sendbytes), (len - sendbytes), 0);
                if (result == 0x00){
                    LOG_W("socket have closed when send data, send byte|%d, %d, %d", len, sendbytes, socket_fd);
                    return -0x03;
                }else if (result < 0x00){
                    LOG_W("socket send data error errno: (%d, %d, %d)", errno, result, socket_fd);
                    if (errno == EINTR){
                        continue;
                    }
                    return -0x04;
                }else{
                    sendbytes += result;
                    if (sendbytes == len){
                        return len;
                    }
                }
            }
        }
    }
}
#endif /* 0 */

int app_socket_recv_port(int socket_fd, void *buff, uint16_t len)
{
    if(buff == NULL){
        return -0x01;
    }
    int32_t result = 0x00;

    result = recv(socket_fd, buff, len, 0);
    if (result == 0x00){
        LOG_W("socket have closed when recv data, recv byte|%d fd|%d", len, socket_fd);
        return -0x02;
    }else if (result < 0x00){
//        LOG_W("socket recv data error errno: (%d, %d)", errno, result);
        return -0x03;
    }else{
        return result;
    }
}

int app_socket_close_port(int socket_fd)
{
    return closesocket(socket_fd);
}

int app_socket_get_state_port(int socket_fd)
{
    int fd;
    struct sal_socket *sal_sock = NULL;
    struct at_socket *at_sock = NULL;

    fd = dfs_net_getsocket(socket_fd);
    sal_sock = sal_get_socket(fd);
    if(sal_sock == NULL){
        return -0x01;
    }
    at_sock = at_get_socket((int)(sal_sock->user_data));
    if(at_sock == NULL){
        return -0x02;
    }
    return (int32_t)(at_sock->state);
}

int app_socket_data_comein_port(int socket_fd, uint32_t timeout)
{
    int32_t result = 0x00;
    fd_set readfds;
    struct timeval timeselect;

    FD_ZERO(&readfds);
    FD_SET(socket_fd, &readfds);
    timeselect.tv_sec = timeout /1000;
    timeselect.tv_usec = timeout %1000;

    result = select(socket_fd + 1, &readfds, NULL, NULL, &timeselect);
    rt_kprintf("net_socket_data_comein(%d, %d, %d, %d, %d)\n", socket_fd, result, timeout, timeselect.tv_sec,
            timeselect.tv_usec);
    if(result > 0x00){
        if (FD_ISSET(socket_fd, &readfds)){
            return 0x01;
        }
    }
    return 0x00;
}

