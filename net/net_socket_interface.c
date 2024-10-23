/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#include "net_socket_interface.h"
#include "net_netdev.h"

int net_socket_open(int *fd, char* host, uint16_t host_len, uint16_t port)
{
    return app_socket_open_port(fd, host, host_len, port);
}

int net_socket_send(int fd, void *data, uint16_t len)
{
    return app_socket_send_port(fd, data, len);
}

int net_socket_recv(int fd, void *buff, uint16_t len)
{
    return app_socket_recv_port(fd, buff, len);
}

int net_socket_close(int fd)
{
    return app_socket_close_port(fd);
}

int net_socket_get_state(int fd)
{
    return app_socket_get_state_port(fd);
}

int net_socket_data_comein(int fd, uint32_t timeout)
{
    return app_socket_data_comein_port(fd, timeout);
}

int net_socket_control(int fd, uint8_t cmd, void* para, uint8_t para_len, void *ret, uint8_t ret_len)
{
    return app_socket_control_port(fd, cmd, para, para_len, ret, ret_len);
}

