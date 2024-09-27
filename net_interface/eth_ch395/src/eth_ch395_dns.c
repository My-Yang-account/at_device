/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-26     我的杨yang       the first version
 */
#include "eth_ch395_dns.h"

#include "eth_ch395_transceiver.h"
#include "eth_ch395_config.h"
#include "eth_ch395_cmd.h"
#include "eth_ch395_netdev.h"

#define DBG_TAG "395dns"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_INCLUDE_ETHERNET_PACK

#define ETHCH395_DNS_PORT                           53                   /** DNS 服务器端口 */

static uint16_t s_ethch395_dnsmsg_id = 0x1100;                           /** 标识 */
static int32_t socket_fd = -0x01;
static uint8_t ethch395_dns_ip[0x04];

/*********************************************************************************
* 函数名  : ethch395_get16
* 功能      : 将缓冲区UINT8数据转为UINT16格式数据
* 参数      : UINT8 类型数据
* 返回      : 转化后的UINT16类型数据
**********************************************************************************/
static uint16_t ethch395_get16(uint8_t *s)
{
  uint16_t i;
  i = *s++ << 8;
  i = i + *s;

  return i;
}

/*********************************************************************************
* 函数名  : ethch395_put16
* 功能      : UINT16 格式数据按UINT8格式存到缓冲区
* 参数      : s -缓冲区首地址
       :i -UINT16数据
* 返回      : 偏移指针
**********************************************************************************/
static uint8_t *ethch395_put16(uint8_t *s, uint16_t i)
{
  *s++ = i >> 8;
  *s++ = i;
  return s;
}

/**********************************************************************************
* 函数名  : ethch395_parse_name
* 功能      : 分析完整的域名
* 参数      : msg            -指向报文的指针
         compressed     -指向报文中主域名的指针
         buf            -缓冲区指针，用于存放转化后域名
* 返回      : 压缩报文的长度
**********************************************************************************/
static int32_t ethch395_parse_name(uint8_t *msg, uint8_t *compressed, char *buf)
{
  uint16_t slen;                                                                            /* 当前片段长度*/
  uint8_t * cp;

  int clen = 0;                                                                           /* 压缩域名长度 */
  int indirect = 0;
  int nseg = 0;                                                                           /* 域名被分割的片段总数 */
  cp = compressed;
  for (;;)
  {
    slen = *cp++;                                                                          /* 首字节的计数值*/
    if (!indirect) clen++;
    if ((slen & 0xc0) == 0xc0)                                                             /*计数字节高两比特为1，用于压缩格式*/
    {
      if (!indirect)
        clen++;
        indirect = 1;
        cp = &msg[((slen & 0x3f)<<8) + *cp];                                              /*按计数字节数值指针偏移到指定位置*/
        slen = *cp++;
    }
    if (slen == 0)                                                                        /*计数为0，结束*/
        break;
    if (!indirect) clen += slen;
    while (slen-- != 0) *buf++ = (char)*cp++;
    *buf++ = '.';
    nseg++;
  }
   if (nseg == 0)
   {
     *buf++ = '.';
   }
   *buf++ = '\0';
   return clen;                                                                            /*压缩报文长度 */
}

/*********************************************************************************
* 函数名  : ethch395_dns_question
* 功能      : 分析响应报文中的问题记录部分
* 参数      : msg          -指向响应报文的指针
         cp           -指向问题记录的指针
* 返回      : 指向下一记录的指针
**********************************************************************************/
static uint8_t *ethch395_dns_question(uint8_t *msg, uint8_t *cp)
{
  int len;
  char name[ETHCH395_DNS_BUF_SIZE];
  len = ethch395_parse_name(msg, cp, name);
  cp += len;
  cp += 2;                                                                                  /* 类型 */
  cp += 2;                                                                                  /* 类 */
  return cp;
}

/*********************************************************************************
* 函数名  : ethch395_dns_answer
* 功能      : 分析响应报文中的回答记录部分
* 参数      : msg            -指向响应报文的指针
         cp             -指向回答记录的指针
         psip
* 返回      : 指向下一记录的指针
**********************************************************************************/
static uint8_t *ethch395_dns_answer(uint8_t *msg, uint8_t *cp, uint8_t *psip)
{
  int len, type;
  char name[ETHCH395_DNS_BUF_SIZE];
  len = ethch395_parse_name(msg, cp, name);
  cp += len;
  type = ethch395_get16(cp);
  cp += 2;                                                                                   /* 类型 */
  cp += 2;                                                                                   /* 类 */
  cp += 4;                                                                                   /* 生存时间 */
  cp += 2;                                                                                   /* 资源数据长度 */
  switch (type)
  {
  case TYPE_A:
    psip[0] = *cp++;
    psip[1] = *cp++;
    psip[2] = *cp++;
    psip[3] = *cp++;
    break;
  case TYPE_CNAME:
  case TYPE_MB:
  case TYPE_MG:
  case TYPE_MR:
  case TYPE_NS:
  case TYPE_PTR:
    len = ethch395_parse_name(msg, cp, name);
    cp += len;
    break;
  case TYPE_HINFO:

  case TYPE_MX:

  case TYPE_SOA:

  case TYPE_TXT:
    break;
  default:
    break;
  }
  return cp;
}

/*******************************************************************************
* 函数名  : ethch395_udp_send_data
* 功能     : udp向指定的ip和端口发送数据
* 参数     : buf     : 发送数据缓冲区
         len     : 发送数据长度
         ip      : 目标ip
         port    : 目标端口
         socket_fd : socket索引值
* 返回         : None
*******************************************************************************/
static uint16_t ethch395_udp_send_data(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port, uint8_t socket_fd)
{
    uint16_t res;

    if(len > ETHCH395_DNS_BUF_SIZE){
        res = ETHCH395_DNS_BUF_SIZE;
    }else{
        res = len;
    }
#if 0
    /** 给网络设备配置 socket 目的IP */
    res = ethch395_cmd_set_remote_ip(socket_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dns dest ip fail(%d, %d)", res, socket_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 目的端口 */
    res = ethch395_cmd_set_remote_port(socket_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dns dest port fail(%d, %d)", res, socket_fd);
        return -0x01;
    }
#endif
    ethch395_cmd_padding_data_sbuf(socket_fd, buf, len);

    return res;
}

/*********************************************************************************
* 函数名  : ethch395_parse_message
* 功能    : 分析响应报文中的资源记录部分
* 参数      : msg            -指向DNS报文头部的指针
         cp             -指向响应报文的指针
         pSip
* 返回         : 成功返回1，否则返回0
**********************************************************************************/
static uint8_t ethch395_parse_message(struct dhdr *pdhdr, uint8_t *pbuf, uint8_t *pSip)
{
  uint16_t tmp;
  uint16_t i;
  uint8_t * msg;
  uint8_t * cp;

  msg = pbuf;
  memset(pdhdr, 0, sizeof(pdhdr));
  pdhdr->id = ethch395_get16(&msg[0]);
  tmp = ethch395_get16(&msg[2]);
  if (tmp & 0x8000) pdhdr->qr = 1;
  pdhdr->opcode = (tmp >> 11) & 0xf;
  if (tmp & 0x0400) pdhdr->aa = 1;
  if (tmp & 0x0200) pdhdr->tc = 1;
  if (tmp & 0x0100) pdhdr->rd = 1;
  if (tmp & 0x0080) pdhdr->ra = 1;
  pdhdr->rcode = tmp & 0xf;
  pdhdr->qdcount = ethch395_get16(&msg[4]);
  pdhdr->ancount = ethch395_get16(&msg[6]);
  pdhdr->nscount = ethch395_get16(&msg[8]);
  pdhdr->arcount = ethch395_get16(&msg[10]);
  /* 分析可变数据长度部分*/
  cp = &msg[12];

  for (i = 0; i < pdhdr->qdcount; i++)                                                      /* 查询问题 */
  {
    cp = ethch395_dns_question(msg, cp);
  }
  for (i = 0; i < pdhdr->ancount; i++)                                                      /* 回答 */
  {
    cp = ethch395_dns_answer(msg, cp, pSip);
        //printf("acquire IP=%d.%d.%d.%d\r\n",pSip[0],pSip[1],pSip[2],pSip[3]);
  }
  for (i = 0; i < pdhdr->nscount; i++)                                                      /*授权 */
  {
    /*待解析*/  ;
  }
  for (i = 0; i < pdhdr->arcount; i++)                                                      /* 附加信息 */
  {
    /*待解析*/  ;
  }
  if(pdhdr->rcode == 0) return 1;                                                           /*rcode = 0:成功*/
  else return 0;
}

/**********************************************************************************
* 函数名  : ethch395_make_dnsquery_message
* 功能    : 制作DNS查询报文
* 参数          : op   - 递归
*          name - 指向待查域名指针
*          buf  - DNS缓冲区.
*          len  - 缓冲区最大长度.
* 返回         : 指向DNS报文指针
**********************************************************************************/
static uint16_t ethch395_make_dnsquery_message(uint16_t op, char *name, uint8_t *buf, uint16_t len)
{
  uint8_t *cp;
  uint16_t p;
  uint16_t dlen;

  char *cp1;
  char tmpname[ETHCH395_DNS_BUF_SIZE];
  char *dname;
  cp = buf;
  s_ethch395_dnsmsg_id++;
  cp = ethch395_put16(cp, s_ethch395_dnsmsg_id);                                          /*标识*/
  p = (op << 11) | 0x0100;
  cp = ethch395_put16(cp, p);                                                             /* 0x0100：Recursion desired */
  cp = ethch395_put16(cp, 1);                                                             /*问题数：1*/
  cp = ethch395_put16(cp, 0);                                                             /*资源记录数：0*/
  cp = ethch395_put16(cp, 0);                                                             /*资源记录数：0*/
  cp = ethch395_put16(cp, 0);                                                             /*额外资源记录数：0*/
  strcpy(tmpname, name);
  dname = tmpname;
  dlen = strlen(dname);
  for (;;)
  {                                                                                        /*按照DNS请求报文域名格式，把URI写入到buf里面去*/
    cp1 = strchr(dname, '.');
    if (cp1 != NULL) len = cp1 - dname;
    else len = dlen;
    *cp++ = len;
    if (len == 0) break;
    strncpy((char *)cp, dname, len);
    cp += len;
    if (cp1 == NULL)
    {
      *cp++ = 0;
      break;
    }
    dname += len+1;                                                                        /*dname首地址后移*/
    dlen -= len+1;                                                                         /*dname长度减小*/
  }
  cp = ethch395_put16(cp, 0x0001);                                                         /* type ：1------ip地址*/
  cp = ethch395_put16(cp, 0x0001);                                                         /* class ：1-------互联网地址*/
  return ((uint16_t)(cp - buf));
}

/**********************************************************************************
* 函数名  : ethch395_config_dns_ip
* 功能      : 配置DNS服务器IP
* 参数      : ip       IP
*      iplen    IP 长度
* 返回
**********************************************************************************/
void ethch395_config_dns_ip(uint8_t *ip, uint8_t iplen)
{
    if((ip == NULL) || (iplen < 0x04)){   /** 点分十进制式 */
        return;
    }

    memcpy(ethch395_dns_ip, ip, sizeof(ethch395_dns_ip));
}

/**********************************************************************************
* 函数名  : ethch395_dnsudp_socket_init
* 功能      : 初始化DNS UDP socket
* 参数
* 返回       >= 0 : 成功，< 0 : 失败
**********************************************************************************/
static int32_t ethch395_dnsudp_socket_init(void)
{
    if((socket_fd = ethch395_socket(0x00)) < 0x00){
        return -0x01;
    }

    int32_t res = 0x00;

    /** 保存目的端口 */
    res = ethch395_config_socket_dest_port(socket_fd, ETHCH395_DNS_PORT);
    if(res < 0x00){
        LOG_E("ethch395 config dns socket dest port fail(%d, %d)", res, socket_fd);
        return -0x01;
    }
    /** 保存目的IP */
    res = ethch395_config_socket_dest_ip(socket_fd, ethch395_dns_ip, sizeof(ethch395_dns_ip));
    if(res < 0x00){
        LOG_E("ethch395 config dns socket dest ip fail(%d, %d)", res, socket_fd);
        return -0x01;
    }


    /** 设置 socket 类型 */
    res = ethch395_cmd_set_socket_protocol(socket_fd, ETHCH395_WORK_MODE_UDP);
    if(res < 0x00){
        LOG_E("ethch395 set dns socket protocol type fail(%d, %d)", res, socket_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 目的IP */
    res = ethch395_cmd_set_remote_ip(socket_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dns dest ip fail(%d, %d)", res, socket_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 目的端口 */
    res = ethch395_cmd_set_remote_port(socket_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dns dest port fail(%d, %d)", res, socket_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 源端口 */
    res = ethch395_cmd_set_source_port(socket_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dns sour port fail(%d, %d)", res, socket_fd);
        return -0x01;
    }

    /** 打开 socket */
    res = ethch395_cmd_open_socket(socket_fd);
    if(res < 0x00){
        LOG_E("ethch395 open dns socket fail(%d, %d)", res, socket_fd);
        return -0x01;
    }

    return 0x00;
}

int32_t ethch395_domain_parse(const char *url, uint8_t urllen, uint8_t *parseip, uint8_t iplen)
{
    if((url == NULL) || (urllen == 0x00)){
        return -0x01;
    }
    if((parseip == NULL) || (iplen < 0x04)){   /** 点分十进制式 */
        return -0x02;
    }

    if(ethch395_dnsudp_socket_init() < 0x00){
        return -0x02;
    }
    rt_thread_mdelay(100);

    uint8_t *buf = rt_malloc(ETHCH395_DNS_BUF_SIZE);
    uint16_t message_len = 0x00;
    uint32_t wait_tick = 0x00;
    struct dhdr dhp;

    if(buf == NULL){
        if(ethch395_cmd_close_socket(socket_fd) < 0x00){
            LOG_E("ethch395 close dns socket fail(%d)", socket_fd);
            ethch395_socket_free(socket_fd);
            return -0x01;
        }
        ethch395_socket_free(socket_fd);
        return -0x03;
    }

    message_len = ethch395_make_dnsquery_message(0x00, (char*)url, buf, ETHCH395_DNS_BUF_SIZE);
    wait_tick = rt_tick_get();
    while(1){
        ethch395_udp_send_data(buf, message_len, ethch395_dns_ip, ETHCH395_DNS_PORT, socket_fd);

        if(netdev_ethch395_socket_data_comein_port(socket_fd, 3000) > 0x00){
            break;
        }
        if(wait_tick > rt_tick_get()){
            wait_tick = rt_tick_get();
        }
        if((rt_tick_get() - wait_tick) > 15 *1000){
            LOG_E("ethch395 DNS response timeout(%d)", 15 *1000);
            if(ethch395_cmd_close_socket(socket_fd) < 0x00){
                LOG_E("ethch395 close dns socket fail(%d)", socket_fd);
                ethch395_socket_free(socket_fd);
                return -0x01;
            }
            ethch395_socket_free(socket_fd);
            return -0x06;
        }
    }

    if(netdev_ethch395_socket_recv_port(socket_fd, buf, ETHCH395_DNS_BUF_SIZE) <= 0x00){
        rt_free(buf);
        if(ethch395_cmd_close_socket(socket_fd) < 0x00){
            LOG_E("ethch395 close dns socket fail(%d)", socket_fd);
            ethch395_socket_free(socket_fd);
            return -0x01;
        }
        ethch395_socket_free(socket_fd);
        return -0x05;
    }
    ethch395_parse_message(&dhp, buf, parseip);
    LOG_D("ethch395 DNS parse[%s]->[%d, %d, %d, %d]\\n", url, parseip[0], parseip[1], parseip[2], parseip[3]);

    if(ethch395_cmd_close_socket(socket_fd) < 0x00){
        LOG_E("ethch395 close dns socket fail(%d)", socket_fd);
        ethch395_socket_free(socket_fd);
        return -0x01;
    }
    ethch395_socket_free(socket_fd);

    rt_free(buf);
    return 0x00;
}

#endif /* NET_INCLUDE_ETHERNET_PACK */

