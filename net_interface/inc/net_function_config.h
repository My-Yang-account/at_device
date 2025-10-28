/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_INC_NET_FUNCTION_CONFIG_H_
#define NET_INTERFACE_INC_NET_FUNCTION_CONFIG_H_

#include "stdio.h"

/***********************************************
 * 函数名     app_net_occured_socket_close_passive
 * 功能         出现了socket 被动关闭
 **********************************************/
void app_net_occured_socket_close_passive(int fd);

/***********************************************
 * 函数名     app_net_occured_socket_pdp_invalid
 * 功能         出现了socket PDP 场景失效
 **********************************************/
void app_net_occured_socket_pdp_invalid(int fd);

/***********************************************
 * 函数名     app_net_occured_close_communicate_module
 * 功能         出现了关闭通信模块
 **********************************************/
void app_net_occured_close_communicate_module(int fd);

/***********************************************
 * 函数名     app_net_occured_at_physics_error
 * 功能         出现了通信模块物理层故障
 **********************************************/
void app_net_occured_at_physics_error(int fd);

/***********************************************
 * 函数名     app_net_occured_cpin_lk_mac_error
 * 功能         出现了通信模块数据链路故障(MAC CPIN)
 **********************************************/
void app_net_occured_cpin_lk_mac_error(int fd);

/***********************************************
 * 函数名     app_net_occured_cimi_lk_mac_error
 * 功能         出现了通信模块数据链路故障(LCC CIMI)
 **********************************************/
void app_net_occured_cimi_lk_mac_error(int fd);

/***********************************************
 * 函数名     app_net_occured_signal_strength_error
 * 功能         出现了通信模块查询信号强度失败
 **********************************************/
void app_net_occured_signal_strength_error(int fd);

/***********************************************
 * 函数名     app_net_occured_gsm_registered_error
 * 功能         出现了通信模块GSM网络注册失败
 **********************************************/
void app_net_occured_gsm_registered_error(int fd);

/***********************************************
 * 函数名     app_net_occured_gprs_registered_error
 * 功能         出现了通信模块GPRS网络注册失败
 **********************************************/
void app_net_occured_gprs_registered_error(int fd);

#endif /* NET_INTERFACE_INC_NET_FUNCTION_CONFIG_H_ */
