/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-26     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_ETH_CH395_INC_ETH_CH395_DNS_H_
#define NET_INTERFACE_ETH_CH395_INC_ETH_CH395_DNS_H_

#include "stdio.h"

#define ETHCH395_DNS_BUF_SIZE    512

#define TYPE_A        1                                                              /* Host address */
#define TYPE_NS       2                                                              /* Name server */
#define TYPE_MD       3                                                              /* Mail destination (obsolete) */
#define TYPE_MF       4                                                              /* Mail forwarder (obsolete) */
#define TYPE_CNAME    5                                                              /* Canonical name */
#define TYPE_SOA      6                                                              /* Start of Authority */
#define TYPE_MB       7                                                              /* Mailbox name (experimental) */
#define TYPE_MG       8                                                              /* Mail group member (experimental) */
#define TYPE_MR       9                                                              /* Mail rename name (experimental) */
#define TYPE_NULL     10                                                             /* Null (experimental) */
#define TYPE_WKS      11                                                             /* Well-known sockets */
#define TYPE_PTR      12                                                             /* Pointer record */
#define TYPE_HINFO    13                                                             /* Host information */
#define TYPE_MINFO    14                                                             /* Mailbox information (experimental)*/
#define TYPE_MX       15                                                             /* Mail exchanger */
#define TYPE_TXT      16                                                             /* Text strings */
#define TYPE_ANY      255                                                            /* Matches any type */

#define CLASS_IN      1

struct dhdr
{
  uint16_t id;                                                                       /* 标识 */
  uint8_t qr;                                                                        /* 查询或应答标志*/
  uint8_t opcode;
  uint8_t aa;                                                                        /* 授权回答 */
  uint8_t tc;                                                                        /* 可截断的 */
  uint8_t rd;                                                                        /* 期望递归*/
  uint8_t ra;                                                                        /* 可以递归 */
  uint8_t rcode;                                                                     /* 应答码 */
  uint16_t qdcount;                                                                  /* 问题数 */
  uint16_t ancount;                                                                  /* 应答数 */
  uint16_t nscount;                                                                  /* 授权数 */
  uint16_t arcount;                                                                  /* 额外记录数 */
};

/**********************************************************************************
* 函数名  : ethch395_config_dns_ip
* 功能      : 配置DNS服务器IP
* 参数      : ip       IP
*      iplen    IP 长度
* 返回
**********************************************************************************/
void ethch395_config_dns_ip(uint8_t *ip, uint8_t iplen);

/**********************************************************************************
* 函数名  : ethch395_domain_parse
* 功能      : 域名解析
* 参数           url        域名
*        urllen     域名长度(B)
*        parseip    用于存放解析后的IP值
*        iplen      缓存长度(B)
* 返回       >= 0 : 成功，< 0 : 失败
* 注：         解析结果是点分十进制式IP各十进制的值(例：若DNS返回结果："121.43.69.62"，则本函数返回的IP值为4
*        个十进制值：121(D), 43(D), 69(D), 62(D))
**********************************************************************************/
int32_t ethch395_domain_parse(const char *url, uint8_t urllen, uint8_t *parseip, uint8_t iplen);

#endif /* NET_INTERFACE_ETH_CH395_INC_ETH_CH395_DNS_H_ */
