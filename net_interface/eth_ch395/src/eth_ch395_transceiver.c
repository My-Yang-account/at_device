/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-05     我的杨yang       the first version
 */
#include "eth_ch395_transceiver.h"
#include "eth_ch395_netdev.h"
#include "eth_ch395_cmd.h"
#include "eth_ch395_dns.h"

#include "net_netdev.h"
#include <ctype.h>
#include <stdlib.h>

#ifdef NET_INCLUDE_ETHERNET_PACK

#define DBG_TAG "ethtran"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define ETHCH395_WAIT_RECV_TIME_DEF                                 10000             /* 等待数据接收完成最大时长 */
#define ETHCH395_WAIT_CONNECTED_TIMEOUT                             20000             /* 等待连接最大时长(ms) */
#define ETHCH395_WAIT_CLOSED_TIMEOUT                                5000              /* 等待关闭最大时长(ms) */
#define ETHCH395_WAIT_SBUF_FREE_TIMEOUT                             5000              /* 等待发送缓存为空最大时长(ms) */
#define ETHCH395_WAIT_CMD_RESDATA_TIMEOUT                           1000              /* 等待指令响应数据最大时长(ms) */

#define ETHCH395_RECV_DATA_SIZE_MAX                                 1500              /* 单次接收数据最大字节(B) */
#define ETHCH395_RECV_RENTRY_MAX                                    5                 /* 未接收到完整数据时最大尝试次数 */
#define ETHCH395_QUERY_DLEN_RENTRY_MAX                              3                 /* 查询接收缓冲区数据长度最大尝试次数 */
#define ETHCH395_SEND_RENTRY_MAX                                    5                 /* 发送指令失败或响应失败时最大尝试次数 */

#define ETHCH395_EVENT_PRO_THREAD_STACK_SIZE                        1024              /* 以太网事件处理线程栈大小 */
#define ETHCH395_RECV_THREAD_STACK_SIZE                             2048              /* 以太网数据接收线程栈大小 */

#ifdef ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44
#define ETHCH395_SOCKET_INT_MASK                                    0xFF0             /* 全局中断中socket中断部分掩码 */
#else
#define ETHCH395_SOCKET_INT_MASK                                    0xF0              /* 全局中断中socket中断部分掩码 */
#endif /* ETHCH395_CHIP_VER_MORE_ADVANCED_THAN_0X44 */

#define ethch395_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#pragma pack(1)

/** socket 链表点 */
typedef struct slist{
    struct slist *next;
}ethch395_slist;

/** 接收数据包信息 */
typedef struct{
    ethch395_slist node;                           /** 链表点 */
    uint32_t bfsz_totle;                           /** 数据大小 */
    uint32_t bfsz_index;                           /** 数据当前获取下标 */
    uint8_t *buff;                                 /** 数据 */
}ethch395_recv_pkt;

struct ethch395_socket_info{
    struct{
        uint8_t send_ok : 1;                       /** socket 信息：发送成功 */
        uint8_t sbuf_free : 1;                     /** socket 信息：发送缓冲区为空 */
        uint8_t recv : 1;                          /** socket 信息：接收缓冲区非空 */
        uint8_t timeout : 1;                       /** socket 信息：超时 */
        uint8_t connected : 1;                     /** socket 信息：已连接 */
    }flag;
    int32_t recv_timeout;                          /** socket 接收数据超时时间 */
    uint32_t recv_size;                            /** socket 接收缓存数据大小 */
    ethch395_slist list;                           /** 单向数据链表 */
};

struct ethch395_assistant{
    struct{
        uint8_t error : 1;                             /** 发生了错误 */
        uint8_t init_complete : 1;                     /** ethch395 初始化已完成 */
    }flag;
};

struct ethch395_access_lock{
    uint8_t lock;
    void *owner;
};
#pragma pack()

static struct ethch395_access_lock s_ethch395_access_lock;
static void (*ethch395_init_hook)(uint8_t, uint8_t);           /** ethch395 初始化 钩子函数 参数1：是否已初始化， 参数2：初始化成功与否*/
static struct ethch395_assistant s_ethch395_assistant_info;
union ethch395_globe_int *s_ethch395_globe_int = NULL;
static struct ethch395_socket_info s_ethch395_socket_info[ETHCH395_SOCKET_NUM_MAX];
static struct rt_thread s_ethch395_event_pro_thread;
static uint8_t s_ethch395_event_pro_thread_stack[ETHCH395_EVENT_PRO_THREAD_STACK_SIZE];
static struct rt_thread s_ethch395_recv_thread;
static uint8_t s_ethch395_recv_thread_stack[ETHCH395_RECV_THREAD_STACK_SIZE];
static struct rt_event s_ethch395_event;
static uint8_t s_ethch395_state = NETDEV_ETHCH395_STATE_PHY;

static void ethch395_device_init(void);

/**************************************************
 *  函数名   ethch395_get_thread_handle
 *  参数       hook        函数入口
 *  功能       配置 ethch395 初始化钩子函数
 *  返回
 *************************************************/
uint8_t ethch395_query_state(void)
{
    return s_ethch395_state;
}

/**************************************************
 *  函数名   ethch395_get_thread_handle
 *  参数       hook        函数入口
 *  功能       配置 ethch395 初始化钩子函数
 *  返回
 *************************************************/
void *ethch395_get_thread_handle(void)
{
    struct rt_thread *handle = rt_thread_self();
    return (void *)handle;
}

/**************************************************
 *  函数名   ethch395_set_init_hook
 *  参数       hook        函数入口
 *  功能       配置 ethch395 初始化钩子函数
 *  返回
 *************************************************/
void ethch395_set_init_hook(void *hook)
{
    if(hook){
        ethch395_init_hook = (void (*)(uint8_t, uint8_t))hook;
    }
}

/*********************************************************************************************
 * **************************************** 数据链表 *****************************************
 ********************************************************************************************/
/**************************************************
 *  函数名   ethch395_slist_init
 *  参数       list        链表表头
 *  功能       初始化一个socket 数据链表
 *  返回
 *************************************************/
static void ethch395_slist_init(ethch395_slist *list)
{
    list->next = NULL;
}
/**************************************************
 *  函数名   ethch395_slist_isempty
 *  参数       list        链表表头
 *  功能       查询 socket 数据链表是否为空
 *  返回      > 0 : 否，< =0 ：是
 *************************************************/
static uint8_t ethch395_slist_isempty(ethch395_slist *list)
{
    return (list->next == NULL);
}
/**************************************************
 *  函数名   ethch395_slist_append
 *  参数       list        链表表头
 *     node        节点
 *  功能       向指定链表中添加节点
 *  返回
 *************************************************/
static void ethch395_slist_append(ethch395_slist *list, ethch395_slist *node)
{
    ethch395_slist *n;

    n = list;
    while(n->next){
        n = n->next;
    }

    n->next = node;
    node->next = NULL;
}
/*********************************************************************************
 *  函数名   ethch395_slist_remove
 *  参数       list        链表表头
 *     node        节点
 *  功能       从链表中移除指定节点
 *  返回     链表表头
 ********************************************************************************/
static ethch395_slist *ethch395_slist_remove(ethch395_slist *list, ethch395_slist *node)
{
    ethch395_slist *n = list;
    while(n->next && n->next != node){
        n = n->next;
    }

    if(n->next != (ethch395_slist *)0){
        n->next = n->next->next;
    }

    return list;
}
/***************************************************************************************
 *  函数名   ethch395_recvpkt_node_delete
 *  参数       rlist       链表表头
 *     node        节点
 *  功能       释放指定链表中自定节点的内存
 *  返回      >= 0 : 成功，< 0 ：失败
 **************************************************************************************/
static uint8_t ethch395_recvpkt_node_delete(ethch395_slist *rlist, ethch395_slist *node)
{
    ethch395_recv_pkt *pack = NULL;

    if(ethch395_slist_isempty(rlist)){
        return 0;
    }

    ethch395_slist_remove(rlist, node);

    pack = ethch395_container_of(node, ethch395_recv_pkt, node);
    if(pack->buff){
        rt_free(pack->buff);
    }
    if(pack){
        rt_free(pack);
        pack = NULL;
    }

    return 0;
}
/**********************************************************************************
 *  函数名   ethch395_recvpkt_get
 *  参数       rlist       数据链表头
 *     mem         缓存
 *     len         缓存长度
 *  功能      从指定链表中获取数据
 *  返回      > 0 : 有数据，< =0 ：错误
 *********************************************************************************/
static int32_t ethch395_recvpkt_get(ethch395_slist *rlist, char *mem, uint32_t len)
{
    ethch395_slist *node = NULL;
    ethch395_recv_pkt *pack = NULL;
    int32_t content_pos = 0, page_pos = 0;

    if(ethch395_slist_isempty(rlist)){
        return -0x02;
    }

    for(node = rlist->next; node; node = node->next){
        pack = ethch395_container_of(node, ethch395_recv_pkt, node);

        page_pos = pack->bfsz_totle - pack->bfsz_index;

        if(page_pos >= len - content_pos){
            memcpy((char *)mem + content_pos, pack->buff + pack->bfsz_index, len - content_pos);

            pack->bfsz_index += len - content_pos;

            if(pack->bfsz_index == pack->bfsz_totle){
                ethch395_recvpkt_node_delete(rlist, node);
            }
            content_pos = len;
            break;
        }else{
            memcpy((char *) mem + content_pos, pack->buff + pack->bfsz_index, page_pos);
            content_pos += page_pos;
            pack->bfsz_index += page_pos;
            ethch395_recvpkt_node_delete(rlist, node);
        }
    }

    return content_pos;
}

/*********************************************************************************************
 * **************************************** 数据链表 *****************************************
 ********************************************************************************************/

uint8_t ethch395_wait_operate_lock(int32_t timeout, void *handle)
{
    uint32_t tick = rt_tick_get();

    if((s_ethch395_access_lock.owner == handle) && (s_ethch395_access_lock.owner != NULL)){
        LOG_D("operate_lock(%d, %d)\n", s_ethch395_access_lock.owner, handle);
        s_ethch395_access_lock.lock = ETHCH395_ENUM_TRUE;
        return ETHCH395_ENUM_TRUE;
    }

    while(s_ethch395_access_lock.lock){
        if(timeout < 0x00){
            rt_thread_mdelay(10);
            continue;
        }
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) >= timeout){
            return ETHCH395_ENUM_FALSE;
        }
        rt_thread_mdelay(10);
    }

    rt_enter_critical();
    if(s_ethch395_access_lock.lock == ETHCH395_ENUM_FALSE){
        LOG_D("000 operate_lock(%d, %d)\n", s_ethch395_access_lock.owner, handle);
        s_ethch395_access_lock.lock = ETHCH395_ENUM_TRUE;
        s_ethch395_access_lock.owner = handle;

        rt_exit_critical();
        return ETHCH395_ENUM_TRUE;
    }

    rt_exit_critical();
    return ETHCH395_ENUM_FALSE;
}

uint8_t ethch395_unlock_operate_lock(void *handle)
{
    if((s_ethch395_access_lock.owner == handle) && (s_ethch395_access_lock.owner != NULL)){
        LOG_D("ethch395_unlock_operate_lock(%d, %d)", s_ethch395_access_lock.owner, handle);
        s_ethch395_access_lock.lock = ETHCH395_ENUM_FALSE;
        s_ethch395_access_lock.owner = NULL;

        return ETHCH395_ENUM_TRUE;
    }
    return ETHCH395_ENUM_FALSE;
}

/**************************************************
 *  函数名   host_is_pure_digital
 *  参数       host        主机名
 *     host_len    主机名长度
 *     ipbuf       解析IP缓存
 *     plen        解析IP缓存长度
 *  功能       判断主机名是否是纯数字IP(点分十进制式IP)
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
static int32_t host_is_pure_digital(char* host, uint16_t host_len, uint8_t *ipbuf, uint8_t plen)
{
    if((host == NULL) || (host_len == 0x00)){
        return ETHCH395_ENUM_FALSE;
    }
    if(host_len > 15){ /* 点分十进制式IP最大长度为 15 */
        return ETHCH395_ENUM_FALSE;
    }
    if((ipbuf == NULL) || (plen < 0x04)){    /* 点分十进制式IP为4个十进制数组合 */
        return ETHCH395_ENUM_FALSE;
    }

    int digit = 0x00;
    uint8_t i = 0x00, j = 0x00;
    uint8_t *ptr = (uint8_t*)host, clen = 0x00, count = 0x00, strbuf[0x04];

    for(i = 0x00; i < host_len; i++){
        memset(strbuf, 0x00, sizeof(strbuf));
        if(host[i] == '.'){     /* 点分十进制式IP分隔符 */
            if(((uint8_t*)host + i) > (ptr + 0x03)){    /* 点分十进制式IP，单个十进制数最大长度为3 */
                return ETHCH395_ENUM_FALSE;
            }
            clen = (((uint8_t*)host + i) - ptr);
            if(clen == 0x00){
                return ETHCH395_ENUM_FALSE;
            }
            for(j = 0x00; j < clen; j++){
                if(!isdigit(ptr[j])){
                    return ETHCH395_ENUM_FALSE;
                }
                strbuf[j] = ptr[j];
            }
            digit = atoi((const char*)strbuf);
            if((digit > 0xFF) || (digit < 0x00)){
                return ETHCH395_ENUM_FALSE;
            }
            ipbuf[count] = digit;
            ptr = ((uint8_t*)host + i + 0x01);
            count++;
            if(count >= 0x04){  /* 点分十进制式IP为4个十进制数组合 */
                return ETHCH395_ENUM_FALSE;
            }
        }
    }
    if(count <= 0x02){
        return ETHCH395_ENUM_FALSE;
    }

    clen = (((uint8_t*)host + host_len) - ptr);
    if(clen == 0x00){
        return ETHCH395_ENUM_FALSE;
    }
    for(j = 0x00; j < clen; j++){
        if(!isdigit(ptr[j])){
            return ETHCH395_ENUM_FALSE;
        }
        strbuf[j] = ptr[j];
    }
    digit = atoi((const char*)strbuf);
    if((digit > 0xFF) || (digit < 0x00)){
        return ETHCH395_ENUM_FALSE;
    }
    ipbuf[0x03] = digit;

    return ETHCH395_ENUM_TRUE;
}

/**************************************************
 *  函数名   netdev_ethch395_socket_open_port
 *  参数       socket_fd   socket下标
 *     host        主机名
 *     host_len    主机名长度
 *     port        主机端口
 *  功能       向指定主机建立TCP连接
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_open_port(int *socket_fd, char* host, uint16_t host_len, uint16_t port)
{
    if((host == NULL) || (host_len == 0x00)){
        LOG_E("ethch395 open socket input host invalid");
        return -0x01;
    }
    if((port == 0xFFFF) || (port == 0x00)){
        LOG_E("ethch395 open socket input port invalid");
        return -0x01;
    }

    int res = 0x00, _fd = -0x01;
    uint8_t _ip[0x04];           /** 点分十进制式IP */
    uint32_t wait_tick = 0x00;
    ethch395_slist *node = NULL;
    void *handle = ethch395_get_thread_handle();

    memset(_ip, 0x00, sizeof(_ip));

    if(host_is_pure_digital(host, host_len, _ip, sizeof(_ip)) == ETHCH395_ENUM_FALSE){
        if(ethch395_domain_parse(host, host_len, _ip, sizeof(_ip)) < 0x00){
            LOG_E("ethch395 DNS domain parse fail(%s)n", host);
        }
    }

    while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

    /** 申请一个 socket */
    res = ethch395_socket(0);
    _fd = res;
    if(res < 0x00){
        LOG_E("ethch395 get socket info fail(%d)", res);
        ethch395_unlock_operate_lock(handle);
        return -0x01;
    }

    s_ethch395_socket_info[_fd].flag.connected = ETHCH395_ENUM_FALSE;      /** 清除已连接标志 */
    memset(&s_ethch395_socket_info[_fd], 0x00, sizeof(s_ethch395_socket_info[_fd]));
    for(node = s_ethch395_socket_info[_fd].list.next; node; node = node->next){
        ethch395_recvpkt_node_delete(&s_ethch395_socket_info[_fd].list, node);
    }
    ethch395_slist_init(&s_ethch395_socket_info[_fd].list);                /** 初始化socket 数据链表 */

    /** 保存目的端口 */
    res = ethch395_config_socket_dest_port(_fd, port);
    if(res < 0x00){
        LOG_E("ethch395 config socket dest port fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }
    /** 保存目的IP */
    res = ethch395_config_socket_dest_ip(_fd, _ip, sizeof(_ip));
    if(res < 0x00){
        LOG_E("ethch395 config socket dest ip fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }

    /** 设置 socket 类型 */
    res = ethch395_cmd_set_socket_protocol(_fd, ETHCH395_WORK_MODE_SIZE);
    if(res < 0x00){
        LOG_E("ethch395 set socket protocol type fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 目的IP */
    res = ethch395_cmd_set_remote_ip(_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dest ip fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 目的端口 */
    res = ethch395_cmd_set_remote_port(_fd);
    if(res < 0x00){
        LOG_E("ethch395 set dest port fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }
    /** 给网络设备配置 socket 源端口 */
    res = ethch395_cmd_set_source_port(_fd);
    if(res < 0x00){
        LOG_E("ethch395 set sour port fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }

    /** 设置 socket 发送缓存 */
    res = ethch395_cmd_set_send_buf(_fd);
    if(res < 0x00){
        LOG_E("ethch395 set send buff fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }
    /** 设置 socket 接收缓存 */
    res = ethch395_cmd_set_recv_buf(_fd);
    if(res < 0x00){
        LOG_E("ethch395 set send buff fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }

    /** 打开 socket */
    res = ethch395_cmd_open_socket(_fd);
    if(res < 0x00){
        LOG_E("ethch395 open socket fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }
    /** 建立TCP连接 */
    res = ethch395_cmd_connect_socket(_fd);
    if(res < 0x00){
        LOG_E("ethch395 connect socket fail(%d, %d)", res, _fd);
        ethch395_unlock_operate_lock(handle);
        ethch395_socket_free(_fd);
        return -0x01;
    }

    /** 设置 socket keeplive */
    ethch395_cmd_set_socket_keeplive(_fd, 0x00);

    ethch395_unlock_operate_lock(handle);

    /** 等待连接结果 */
    wait_tick = rt_tick_get();
    while(s_ethch395_socket_info[_fd].flag.connected == ETHCH395_ENUM_FALSE){
        if(wait_tick > rt_tick_get()){
            wait_tick = rt_tick_get();
        }
        if((rt_tick_get() - wait_tick) > ETHCH395_WAIT_CONNECTED_TIMEOUT){
            LOG_E("ethch395 wait socket connect timeout(%d)", _fd);
            ethch395_socket_free(_fd);
            return -0x01;
        }
        rt_thread_mdelay(50);
    }
    if(socket_fd){
        *socket_fd = _fd;
    }

    /** 复位 socket 相关中断标志 */
    s_ethch395_socket_info[_fd].flag.recv = ETHCH395_ENUM_FALSE;
    s_ethch395_socket_info[_fd].flag.sbuf_free = ETHCH395_ENUM_TRUE;
    s_ethch395_socket_info[_fd].flag.send_ok = ETHCH395_ENUM_FALSE;
    s_ethch395_socket_info[_fd].flag.timeout = ETHCH395_ENUM_FALSE;

    LOG_D("ethch395 connect server success");
    return 0x00;
}

/**************************************************
 *  函数名   netdev_ethch395_socket_send_port
 *  参数       socket_fd   socket下标
 *     data        数据
 *     len         数据长度
 *  功能       向指定主发送数据
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_send_port(int socket_fd, void *data, uint32_t len)
{
    if((socket_fd < 0x00) || (socket_fd >= ETHCH395_SOCKET_NUM_MAX)){
        return -0x01;
    }
    if(s_ethch395_socket_info[socket_fd].flag.connected != ETHCH395_ENUM_TRUE){
        return -0x01;
    }

    uint8_t *idata = (uint8_t*)data;
    int32_t ssize = 0x00, rsize = 0x00, msize = ethch395_get_socket_sbuf_szie(socket_fd);
    uint32_t ethch395_send_time_tick = 0x00;
    void *handle = ethch395_get_thread_handle();

    if(msize == 0x00){
        return -0x01;
    }
    rsize = len;

    s_ethch395_socket_info[socket_fd].flag.send_ok = ETHCH395_ENUM_FALSE;

    while(1){
        if(rsize > msize){
            ssize += msize;
        }else{
            ssize += rsize;
        }

        ethch395_send_time_tick = rt_tick_get();
        if(s_ethch395_socket_info[socket_fd].flag.sbuf_free == ETHCH395_ENUM_FALSE){
            while(1){
                if((ethch395_send_time_tick > rt_tick_get())){
                    ethch395_send_time_tick = rt_tick_get();
                }
                if((rt_tick_get() - ethch395_send_time_tick) > ETHCH395_WAIT_SBUF_FREE_TIMEOUT){
                    break;
                }
                if(s_ethch395_socket_info[socket_fd].flag.sbuf_free == ETHCH395_ENUM_TRUE){
                    break;
                }
                rt_thread_mdelay(10);
            }
        }

        s_ethch395_socket_info[socket_fd].flag.sbuf_free = ETHCH395_ENUM_FALSE;
        s_ethch395_socket_info[socket_fd].flag.send_ok = ETHCH395_ENUM_FALSE;

        while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

        if(ethch395_cmd_padding_data_sbuf(socket_fd, idata, ssize) < 0x00){
            ethch395_unlock_operate_lock(handle);
            LOG_E("ethch395 data send fail(%d)", socket_fd);
            return -0x01;
        }
        ethch395_unlock_operate_lock(handle);

        idata += ssize;
        rsize -= ssize;
        if(rsize <= 0x00){
            break;
        }
    }
    /** 按需是否需要等待发送成功 */

    return (rsize <= 0x00 ? msize : (msize - rsize));
}

/**************************************************
 *  函数名   netdev_ethch395_socket_recv_port
 *  参数       socket_fd   socket下标
 *     buff        数据缓存
 *     len         数据缓存长度
 *  功能       从指定socket接收数据数据
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_recv_port(int socket_fd, void *buff, uint32_t len)
{
    if((socket_fd < 0x00) || (socket_fd >= ETHCH395_SOCKET_NUM_MAX)){
        return -0x01;
    }
    /** 针对于OTA FTP 传输完成连接被关掉的情况 */
#if 0
    if(s_ethch395_socket_info[socket_fd].flag.connected != ETHCH395_ENUM_TRUE){
        return -0x01;
    }
#endif

    int32_t res = 0x00;

    res = ethch395_recvpkt_get(&s_ethch395_socket_info[socket_fd].list, buff, len);
    if(res <= 0x00){
        if(rt_event_recv(&s_ethch395_event, (0x01 <<(socket_fd + 0x01)), RT_EVENT_FLAG_OR |RT_EVENT_FLAG_CLEAR, s_ethch395_socket_info[socket_fd].recv_timeout, NULL) >= 0x00){
            return ethch395_recvpkt_get(&s_ethch395_socket_info[socket_fd].list, buff, len);
        }
    }

    return res;
}

/**************************************************
 *  函数名   netdev_ethch395_socket_close_port
 *  参数       socket_fd   socket下标
 *  功能       关闭指定socket
 *  返回      >= 0 : 成功，< 0 ：失败
 *************************************************/
int netdev_ethch395_socket_close_port(int socket_fd)
{
    if((socket_fd < 0x00) || (socket_fd >= ETHCH395_SOCKET_NUM_MAX)){
        return -0x01;
    }
    if(s_ethch395_socket_info[socket_fd].flag.connected != ETHCH395_ENUM_TRUE){
        return -0x01;
    }

    uint32_t wait_tick = 0x00;
    ethch395_slist *node = NULL;
    void *handle = ethch395_get_thread_handle();

    while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

    if(ethch395_cmd_close_socket(socket_fd) < 0x00){
        LOG_E("ethch395 close socket fail(%d)", socket_fd);
        ethch395_unlock_operate_lock(handle);
        return -0x01;
    }
    if(ethch395_cmd_disconnect_tcp(socket_fd) < 0x00){
        LOG_E("ethch395 disconnect tcp fail(%d)", socket_fd);
        ethch395_unlock_operate_lock(handle);
        return -0x01;
    }

    ethch395_unlock_operate_lock(handle);

    /** 针对于OTA FTP 传输完成连接被关掉的情况 */
#if 0
    for(node = s_ethch395_socket_info[socket_fd].list.next; node; node = node->next){
        ethch395_recvpkt_node_delete(&s_ethch395_socket_info[socket_fd].list, node);
    }
#endif

    /** 等待关闭结果 */
    wait_tick = rt_tick_get();
    while(s_ethch395_socket_info[socket_fd].flag.connected == ETHCH395_ENUM_TRUE){
        if(wait_tick > rt_tick_get()){
            wait_tick = rt_tick_get();
        }
        if((rt_tick_get() - wait_tick) > ETHCH395_WAIT_CLOSED_TIMEOUT){
            LOG_E("ethch395 wait socket close timeout(%d)", socket_fd);
            ethch395_socket_free(socket_fd);
    /** 针对于OTA FTP 传输完成连接被关掉的情况 */
#if 0
            memset(&s_ethch395_socket_info[socket_fd], 0x00, sizeof(s_ethch395_socket_info[socket_fd]));
#endif
            return -0x01;
        }
        rt_thread_mdelay(50);
    }

    ethch395_socket_free(socket_fd);
    /** 针对于OTA FTP 传输完成连接被关掉的情况 */
#if 0
    memset(&s_ethch395_socket_info[socket_fd], 0x00, sizeof(s_ethch395_socket_info[socket_fd]));
#endif

    LOG_D("netdev ethch395 socket close success(%d)", socket_fd);

    return 0x00;
}

/**************************************************
 *  函数名  netdev_ethch395_socket_query_state_port
 *  参数       socket_fd   socket下标
 *  功能       查询指定 socket 状态
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_query_state_port(int socket_fd)
{
    if((socket_fd < 0x00) || (socket_fd >= ETHCH395_SOCKET_NUM_MAX)){
        return -0x01;
    }
    if(s_ethch395_socket_info[socket_fd].flag.connected != ETHCH395_ENUM_TRUE){
        return -0x01;
    }

    uint8_t status = 0x00;
    int32_t res = 0x00;
    void *handle = ethch395_get_thread_handle();

    while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

    if((status = ethch395_cmd_query_socket_status(socket_fd)) < 0x00){
        LOG_E("ethch395 disconnect tcp fail(%d)", socket_fd);
        ethch395_unlock_operate_lock(handle);
        return -0x01;
    }

    ethch395_unlock_operate_lock(handle);

    status = (uint8_t)res;
    if(status){
        return ETHCH395_ENUM_TRUE;
    }

    return ETHCH395_ENUM_FALSE;
}

/**************************************************
 *  函数名  netdev_ethch395_socket_data_comein_port
 *  参数       socket_fd   socket下标
 *     timeout     查询超时时间
 *  功能       查询指定 socket 是否有接收到的数据
 *  返回      > 0 : 是，< =0 ：否
 *************************************************/
int netdev_ethch395_socket_data_comein_port(int socket_fd, uint32_t timeout)
{
    if((socket_fd < 0x00) || (socket_fd >= ETHCH395_SOCKET_NUM_MAX)){
        return -0x01;
    }
    /** 针对于OTA FTP 传输完成连接被关掉的情况 */
#if 0
    if(s_ethch395_socket_info[socket_fd].flag.connected != ETHCH395_ENUM_TRUE){
        return -0x01;
    }
#endif

    if(ethch395_slist_isempty(&s_ethch395_socket_info[socket_fd].list) == ETHCH395_ENUM_FALSE){
        return ETHCH395_ENUM_TRUE;
    }
    if(rt_event_recv(&s_ethch395_event, (0x01 <<(socket_fd + 0x01)), RT_EVENT_FLAG_OR |RT_EVENT_FLAG_CLEAR, timeout, NULL) != RT_EOK){
        return ETHCH395_ENUM_FALSE;
    }
    return ETHCH395_ENUM_TRUE;
}

/**************************************************
 *  函数名   netdev_ethch395_socket_control
 *  参数       socket_fd     socket 下标
 *       cmd       指令
 *       para      指令参数
 *  功能       socket 控制指令
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int netdev_ethch395_socket_control(int socket_fd, uint8_t cmd, void *para, uint8_t para_len, void *ret, uint8_t ret_len)
{
    if((socket_fd < 0x00) || (socket_fd >= ETHCH395_SOCKET_NUM_MAX)){
        return -0x01;
    }

    switch(cmd){
    case NETDEV_ETHCH395_SOCKET_CONTROL_RECV_TIMEOUT:
        s_ethch395_socket_info[socket_fd].recv_timeout = *((int32_t*)para);
        return 0x00;
        break;
    case NETDEV_ETHCH395_SOCKET_CONTROL_DOMAIN_PARSE:
    {
        if((para == NULL) || (para_len == 0x00)){
            return -0x01;
        }
        if((ret == NULL) || (ret_len < 0x10)){  /** 点分十进制式IP，最长15字节，预留一字节 */
            return -0x01;
        }

        uint8_t ip_str[0x04];
        memset(ret, 0x00, ret_len);

        if(host_is_pure_digital((char*)para, (uint16_t)para_len, ip_str, sizeof(ip_str)) == ETHCH395_ENUM_FALSE){
            if(ethch395_domain_parse((char*)para, (uint16_t)para_len, ip_str, sizeof(ip_str)) < 0x00){
                LOG_E("ethch395 socket control DNS domain parse fail(%s)n", (char*)para);
                return -0x01;
            }
        }
        sprintf((char*)ret, "%d.%d.%d.%d", ip_str[0x00], ip_str[0x01], ip_str[0x02], ip_str[0x03]);
        return 0x00;
    }
        break;
    default:
        break;
    }

    return -0x01;
}


/**************************************************
 *  函数名   ethch395_cmd_data_clear
 *  参数
 *  功能       清除指令响应数据
 *  返回
 *************************************************/
void ethch395_cmd_data_clear(void)
{
    uint8_t ch = 0x01;
    while(ethch395_netdev_recv(&ch, 0x01) == 0x01);

    reset_ethch395_data_sem();
}

int32_t ethch395_cmd_data_recv(uint8_t *buf, uint8_t len)
{
    if((buf == NULL) || (len == 0x00)){
        return -0x01;
    }

    if(take_ethch395_data_sem(ETHCH395_WAIT_CMD_RESDATA_TIMEOUT) < 0x00){
        LOG_D("000 ethch395 wait serial data timeout");
        return -0x02;
    }

    uint8_t ch = 0x01, rlen = 0;
    while(1)
    {
        if(ethch395_netdev_recv(&ch, 0x01) != 0x01){
            if(take_ethch395_data_sem(ETHCH395_WAIT_CMD_RESDATA_TIMEOUT) < 0x00){
                LOG_D("111 ethch395 wait serial data timeout");
                return 0x00;
            }else{
                continue;
            }
        }

        rt_kprintf("recv(%02X, %d, %d)\n", ch, rlen, len);
        buf[rlen] = ch;
        rlen++;

        if(rlen >= len){
            return 0x00;
        }
    }
}

/******************************************************
 *  函数名   ethch395_recv_thread_entry
 *  参数       parameter
 *  功能       接收 ethch395 接收缓冲区中的数据
 *  返回
 *****************************************************/
static void ethch395_recv_thread_entry(void* parameter)
{
    uint8_t socket = 0x00, count = 0x00;
    void *handle = ethch395_get_thread_handle();

    rt_thread_mdelay(1000);
    ethch395_device_init();

    while(1)
    {
        if(s_ethch395_assistant_info.flag.init_complete == ETHCH395_ENUM_FALSE){
            ethch395_device_init();
        }
        if(s_ethch395_assistant_info.flag.error == ETHCH395_ENUM_TRUE){
            ethch395_unlock_operate_lock(handle);
            rt_thread_mdelay(5000);
            continue;
        }

        count = 0x00;

        for(socket = 0x00; socket < ETHCH395_SOCKET_NUM_MAX; socket++){
            if(s_ethch395_socket_info[socket].flag.recv == ETHCH395_ENUM_TRUE){

                while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

                if(ethch395_cmd_read_rbuf_data(socket, s_ethch395_socket_info[socket].recv_size) < 0x00){
                    LOG_E("ethch395 read socket rbuf data fail(%d, %d)", socket, s_ethch395_socket_info[socket].recv_size);
                    s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_FALSE;
                    rt_thread_mdelay(10);
                    continue;
                }

                if(take_ethch395_data_sem(ETHCH395_WAIT_CMD_RESDATA_TIMEOUT) < 0x00){
                    LOG_E("ethch395 wait socket data sem fail(%d)", socket);
                    s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_FALSE;
                    rt_thread_mdelay(10);
                    continue;
                }

                uint8_t *buf = NULL, rentry = 0x00;
                ethch395_recv_pkt *pack = NULL;
                uint32_t len = 0x00, recved_len = 0x00, remain_len = s_ethch395_socket_info[socket].recv_size;

                buf = (uint8_t*)rt_malloc(s_ethch395_socket_info[socket].recv_size);
                if(buf == NULL){
                    rt_thread_mdelay(10);
                    continue;
                }
                pack = (ethch395_recv_pkt*)rt_malloc(sizeof(ethch395_recv_pkt));
                if(pack == NULL){
                    rt_free(buf);
                    rt_thread_mdelay(10);
                    continue;
                }
                pack->bfsz_totle = s_ethch395_socket_info[socket].recv_size;
                pack->bfsz_index = 0;
                pack->buff = buf;

                while(1)
                {
                    if((len = ethch395_netdev_recv((buf + recved_len), remain_len)) != remain_len){
                        recved_len += len;
                        remain_len -= len;

                        if(take_ethch395_data_sem(ETHCH395_WAIT_CMD_RESDATA_TIMEOUT) < 0x00){  /** 数据没接收完最多等待1000ms(等待时间) */
                            if(++rentry > ETHCH395_RECV_RENTRY_MAX){
                                s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_FALSE;
                                pack->bfsz_totle = recved_len;
                                if(recved_len == 0x00){
                                    rt_free(pack->buff);
                                    rt_free(pack);
                                }else{
                                    ethch395_slist_append(&s_ethch395_socket_info[socket].list, &pack->node);
                                }
                                break;
                            }
                            if(ethch395_cmd_read_rbuf_data(socket, remain_len) < 0x00){
                                LOG_E("ethch395 read socket rbuf data again fail(%d, %d)", socket, remain_len);
                                continue;
                            }
                            if(take_ethch395_data_sem(ETHCH395_WAIT_CMD_RESDATA_TIMEOUT) < 0x00){
                                LOG_E("ethch395 wait socket data sem again fail(%d)", socket);
                                continue;
                            }

                            LOG_E("ethch395 recv again(%d)\n", socket);
                            continue;
                        }else{
                            continue;
                        }
                    }
                    recved_len += len;
                    remain_len -= len;

                    if(recved_len >= s_ethch395_socket_info[socket].recv_size){
                        s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_FALSE;
                        ethch395_slist_append(&s_ethch395_socket_info[socket].list, &pack->node);
                        break;
                    }
                }
                if(pack){
                    rt_event_send(&s_ethch395_event, (0x01 <<(socket + 0x01)));
                }
            }else{
                count++;
            }
        }
        if(count == ETHCH395_SOCKET_NUM_MAX){
            ethch395_unlock_operate_lock(handle);
        }

        rt_thread_mdelay(10);
    }
}

/**************************************************
 *  函数名   ethch395_event_pro_thread_entry
 *  参数       parameter
 *  功能       处理 ethch395 的中断信息
 *  返回
 *************************************************/
static void ethch395_event_pro_thread_entry(void* parameter)
{
    union ethch395_socket_int *s_ethch395_socket_int = NULL;
    memset(s_ethch395_socket_info, 0x00, sizeof(s_ethch395_socket_info));
    void *handle = ethch395_get_thread_handle();

    while(1)
    {
        if(s_ethch395_assistant_info.flag.error == ETHCH395_ENUM_TRUE){
            ethch395_unlock_operate_lock(handle);
            rt_thread_mdelay(5000);
            continue;
        }

        if(is_ethch395_irq_coming()){   /* 中断管脚产生了中断 */

            while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

            if(ethch395_cmd_query_globe_int_status() < 0x00){
                ethch395_unlock_operate_lock(ethch395_get_thread_handle());
                LOG_E("ethch395 get globe int status fail");
                rt_thread_mdelay(20);
                continue;
            }
        }else{
            rt_thread_mdelay(20);
            continue;
        }

        /** 不可达中断 */
        if(s_ethch395_globe_int->bit.not_reachable == ETHCH395_ENUM_TRUE){
            s_ethch395_globe_int->bit.not_reachable = ETHCH395_ENUM_FALSE;
        }
        /** IP 冲突 */
        if(s_ethch395_globe_int->bit.ip_conflict == ETHCH395_ENUM_TRUE){
            s_ethch395_globe_int->bit.ip_conflict = ETHCH395_ENUM_FALSE;
        }
        /** PHY 状态改变 */
        if(s_ethch395_globe_int->bit.phy_changed == ETHCH395_ENUM_TRUE){
//                s_ethch395_globe_int->bit.phy_changed = ETHCH395_ENUM_FALSE;
        }
        /** DHCP */
        if(s_ethch395_globe_int->bit.dhcp == ETHCH395_ENUM_TRUE){
//                s_ethch395_globe_int->bit.dhcp = ETHCH395_ENUM_FALSE;
        }

        /** socket 中断 */
        if(s_ethch395_globe_int->value &ETHCH395_SOCKET_INT_MASK){
            uint8_t rentry = 0x00, int_base = 0x04;     /** socket 中断从第 4位开始 */
            for(uint8_t socket = 0x00; socket < ETHCH395_SOCKET_NUM_MAX; socket++){
                if(s_ethch395_globe_int->value &(0x01 <<(int_base + socket))){
                    s_ethch395_globe_int->value &= (~(0x01 <<(int_base + socket)));

                    for(rentry = 0x00; rentry < ETHCH395_SEND_RENTRY_MAX; rentry++){
                        if(ethch395_cmd_query_socket_int(socket) < 0x00){
                            LOG_E("ethch395 query socket init fail(%d)", socket);
                            rt_thread_mdelay(20);
                            continue;
                        }
                        break;
                    }
                    if(rentry >= ETHCH395_SEND_RENTRY_MAX){
                        LOG_E("ethch395 query socket init fail(%d)", socket);
                        continue;
                    }

                    s_ethch395_socket_int = ethch395_get_socket_int_info(socket);
                    if(s_ethch395_socket_info[socket].flag.connected == ETHCH395_ENUM_FALSE){
                        if(s_ethch395_socket_int->bit.tcp_connect){
                            LOG_D("socket(%d) connect interrupt", socket);
                            s_ethch395_socket_info[socket].flag.connected = ETHCH395_ENUM_TRUE;
                        }
                    }else{
                        if(s_ethch395_socket_int->bit.tcp_disconnect){
                            LOG_D("socket(%d) disconnect interrupt", socket);
                            s_ethch395_socket_info[socket].flag.connected = ETHCH395_ENUM_FALSE;
                        }
                    }

                    if(s_ethch395_socket_int->bit.sbuf_free){
                        LOG_D("socket(%d) send free interrupt", socket);
                        s_ethch395_socket_info[socket].flag.sbuf_free = ETHCH395_ENUM_TRUE;
                    }

                    if(s_ethch395_socket_int->bit.send_ok){
                        LOG_D("socket(%d) send ok interrupt", socket);
                        s_ethch395_socket_info[socket].flag.send_ok = ETHCH395_ENUM_TRUE;
                    }

                    if(s_ethch395_socket_int->bit.timeout){
                        LOG_D("socket(%d) send timeout interrupt", socket);
                        s_ethch395_socket_info[socket].flag.timeout = ETHCH395_ENUM_TRUE;
                    }

                    if(s_ethch395_socket_int->bit.recv || s_ethch395_socket_int->bit.tcp_disconnect){
                        int8_t rentry = ETHCH395_QUERY_DLEN_RENTRY_MAX;
                        uint16_t size = 0x00;
                        while((size == 0x00) && (rentry >= 0x00)){
                            ethch395_cmd_query_rbuf_data_len(socket, (uint8_t*)&size, sizeof(size));
                            rentry--;
                        }

                        if(rentry < 0x00){
                            LOG_E("ethch395 query socket rbuf data len fail(%d)", socket);
                            s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_FALSE;
                            continue;
                        }

                        s_ethch395_socket_info[socket].recv_size = size;
                        s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_TRUE;
                    }
                }
            }
        }
        ethch395_unlock_operate_lock(handle);

        rt_thread_mdelay(20);
    }
}

/**************************************************
 *  函数名   ethch395_device_init
 *  参数
 *  功能       ethch395 设备初始化线程
 *  返回
 *************************************************/
static void ethch395_device_init(void)
{
    uint8_t resp[32], rentry = 0x00;
    int32_t res = 0x00;
    uint32_t baudrate = ETHCH395_NETDEV_BAUDRATE_9600, wait_tick;
    memset(resp, 0x00, sizeof(resp));
    ethch395_slist *node = NULL;
    void *handle = ethch395_get_thread_handle();

    while(1){
        s_ethch395_assistant_info.flag.error = ETHCH395_ENUM_FALSE;
        s_ethch395_assistant_info.flag.init_complete = ETHCH395_ENUM_FALSE;
        s_ethch395_state = NETDEV_ETHCH395_STATE_PHY;    /** 芯片状态：初始化物理层 */

        for(uint8_t socket = 0x00; socket < ETHCH395_SOCKET_NUM_MAX; socket++){
            s_ethch395_socket_info[socket].flag.recv = ETHCH395_ENUM_FALSE;
            for(node = s_ethch395_socket_info[socket].list.next; node; node = node->next){
                ethch395_recvpkt_node_delete(&s_ethch395_socket_info[socket].list, node);
            }
        }

        res = 0x00;
        rentry = 0x00;

        while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

        baudrate = ETHCH395_NETDEV_BAUDRATE_9600;
        ethch395_netdev_ctrl(ETHCH395_NETDEV_CTRL_BAUDRATE, &baudrate, sizeof(baudrate));
        rt_thread_mdelay(100);

        /** 查询  ethch395 通信状态 */
        if(ethch395_cmd_test_communication_status() < 0x00){
            LOG_E("ethch395 communication status abnormal");
            res = -0x01;
            goto _is_end;
        }
        /** 查询  ethch395 版本 */
        if(ethch395_cmd_query_ic_version(resp, sizeof(resp)) < 0x00){
            LOG_E("ethch395 query chip version fail");
            res = -0x01;
            goto _is_end;
        }
        LOG_D("ethch395 version:%s", resp);

        /** 命令复位  ethch395 */
        ethch395_cmd_hard_reset();

        /** 设置 ethch395 TCP MSS */
        if(ethch395_cmd_set_tcp_mss(ETHCH395_TCP_MSS_DEF) < 0x00){
            LOG_E("ethch395 set tcp mss fail");
            res = -0x01;
            goto _is_end;
        }

        /** 修改 ethch395 通信波特率 */
        if(ethch395_cmd_set_baudrate(ETHCH395_CMD_BAUDRATE_115200) < 0x00){
            LOG_E("ethch395 modify baudrate fail(%d)", ETHCH395_CMD_BAUDRATE_115200);
            res = -0x01;
            goto _is_end;
        }

        baudrate = ETHCH395_NETDEV_BAUDRATE_115200;
        ethch395_netdev_ctrl(ETHCH395_NETDEV_CTRL_BAUDRATE, &baudrate, sizeof(baudrate));
        rt_thread_mdelay(100);
        /**  查询 ethch395 版本(只是为了验证波特率修改是否成功) */
        memset(resp, 0x00, sizeof(resp));
        if(ethch395_cmd_query_ic_version(resp, sizeof(resp)) < 0x00){
            LOG_E("ethch395 query chip version fail when detect new baudrate");
            res = -0x01;
            goto _is_end;
        }

        s_ethch395_state = NETDEV_ETHCH395_STATE_LINK_MAC;    /** 芯片状态：初始化链路MAC层 */

        rt_thread_mdelay(10);

    #if 0
        ethch395_cmd_set_func_para(0);

        /** 设置  ethch395 PHY 模式为自动协商 */
        ethch395_cmd_set_phy(ETHCH395_PHY_TYPE_AUTO);
    #endif

        s_ethch395_state = NETDEV_ETHCH395_STATE_LINK_LCC;    /** 芯片状态：初始化链路LCC层 */
        /** 命令初始化  ethch395 */
        if(ethch395_cmd_init() < 0x00){
            LOG_E("ethch395 init fail");
            res = -0x01;
            goto _is_end;
        }

        ethch395_unlock_operate_lock(handle);

        /** 等待 PHY 中断( ethch395 连接上以太网后就会产生此中断) */
        wait_tick = rt_tick_get();
        while(s_ethch395_globe_int->bit.phy_changed == ETHCH395_ENUM_FALSE){  /** 等待PHY中断产生 */
            if(wait_tick > rt_tick_get()){
                wait_tick = rt_tick_get();
            }
            if((rt_tick_get() - wait_tick) > 30000){
                LOG_E("ethch395 wait PHY interrupt timeout");
                res = -0x01;
                goto _is_end;
            }
            rt_thread_mdelay(10);
        }

        while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

        /** 启动 DHCP，IP动态分配，但socket 原端口还需配置 */
        if(ethch395_cmd_set_dhcp_status(0x01) < 0x00){
            LOG_E("ethch395 set dhcp status fail(%d)", 0x01);
            res = -0x01;
            goto _is_end;
        }

        s_ethch395_state = NETDEV_ETHCH395_STATE_NET_REGISTERED;    /** 芯片状态：等待网络注册 */
        ethch395_unlock_operate_lock(handle);

        /** 等待 DHCP 中断，主要用于获取DNS 服务器信息，如果不需要则不等待即可 */
        wait_tick = rt_tick_get();
        while(s_ethch395_globe_int->bit.dhcp == ETHCH395_ENUM_FALSE){  /** 等待DHCP中断产生 */
            if(wait_tick > rt_tick_get()){
                wait_tick = rt_tick_get();
            }
            if((rt_tick_get() - wait_tick) > 30000){
                LOG_E("ethch395 wait dhcp interrupt timeout");
                res = -0x01;
                goto _is_end;
            }
            rt_thread_mdelay(10);
        }

        while(ethch395_wait_operate_lock(-0x01, handle) == ETHCH395_ENUM_FALSE);

        /** 查询 DHCP 是否启动成功 */
        while(rentry < 100){
            if(ethch395_cmd_query_dhcp_status() != 0x00){    /** 发送命令 CMD_GET_DHCP_STATUS 获取 DHCP 状态，如果状态码为 0 则表示成功 */
                LOG_E("ethch395 query dhcp status fail(%d)", rentry);
                rentry++;
                rt_thread_mdelay(100);
                continue;
            }
            break;
        }
        if(rentry >= 100){
            LOG_E("ethch395 query dhcp status timeout", rentry);
            res = -0x01;                                 /** DHCP 有时会出现返回失败，但是后续交互正常的情况，故如果返回失败暂时不认为是失败 */
            goto _is_end;
        }

        /** 获取DHCP相关信息 */
        if(ethch395_cmd_query_ip_info(resp, sizeof(resp)) < 0x00){
            LOG_E("ethch395 query ip info fail");
            res = -0x01;
            goto _is_end;
        }

        ethch395_config_dns_ip(ethch395_get_dev_gatewayip(), 0x04);

        s_ethch395_state = NETDEV_ETHCH395_STATE_MODULE_INIT;    /** 芯片状态：芯片相关信息初始化 */
        /** 查询设备MAC地址 */
        if(ethch395_cmd_query_dev_mac(resp, sizeof(resp)) < 0x00){
            LOG_E("ethch395 query device MAC fail");
            res = -0x01;
            goto _is_end;
        }

        s_ethch395_state = NETDEV_ETHCH395_STATE_NORMAL;         /** 芯片状态：正常 */
        s_ethch395_assistant_info.flag.init_complete = ETHCH395_ENUM_TRUE;
        ethch395_unlock_operate_lock(handle);

        LOG_D("ethch395 device init success");
        break;

_is_end:

        ethch395_unlock_operate_lock(handle);
        s_ethch395_assistant_info.flag.error = ETHCH395_ENUM_TRUE;
        s_ethch395_assistant_info.flag.init_complete = ETHCH395_ENUM_FALSE;

        ethch395_device_reset();
        rt_thread_mdelay(10000);
        LOG_D("ethch395 device init repeat");
    }
}

/**************************************************
 *  函数名   ethch395_device_reset
 *  参数
 *  功能       复位ethch395
 *  返回
 *************************************************/
int32_t ethch395_device_reset(void)
{
    LOG_D("ethch395_device_reset");
    s_ethch395_state = NETDEV_ETHCH395_STATE_PHY;    /** 芯片状态：初始化物理层 */
    s_ethch395_assistant_info.flag.error = ETHCH395_ENUM_FALSE;
    s_ethch395_assistant_info.flag.init_complete = ETHCH395_ENUM_FALSE;

    return ethch395_netdev_ctrl(ETHCH395_NETDEV_CTRL_HARDRESET, NULL, 0x00);
}

/**************************************************
 *  函数名   net_ethch395_transceiver_init
 *  参数
 *  功能       初始化ethch395 设备IO及相关处理线程
 *  返回      >= 0 : 成功，< 0 ：失败
 *************************************************/
int32_t net_ethch395_transceiver_init(void)
{
    int32_t res = 0x00;

    if(ethch395_init_hook){
        ethch395_init_hook(ETHCH395_ENUM_FALSE, ETHCH395_ENUM_FALSE);
    }

    s_ethch395_assistant_info.flag.error = ETHCH395_ENUM_FALSE;
    s_ethch395_assistant_info.flag.init_complete = ETHCH395_ENUM_FALSE;

    s_ethch395_globe_int = ethch395_get_globe_int_info();
    for(uint8_t socket = 0x00; socket < ETHCH395_SOCKET_NUM_MAX; socket++){
        ethch395_slist_init(&s_ethch395_socket_info[socket].list);
    }

    /** 网络设备初始化 */
    if(ethch395_netdev_init() < 0x00){
        LOG_E("ethch395 netdev init fail");
        res = -0x01;
        goto _init_end;
    }

    if(rt_thread_init(&s_ethch395_event_pro_thread, "395pro", ethch395_event_pro_thread_entry, NULL,
            s_ethch395_event_pro_thread_stack, ETHCH395_EVENT_PRO_THREAD_STACK_SIZE, 14, 5) != RT_EOK){
        LOG_E("ethch395 event process thread init fail");
        res = -0x01;
        goto _init_end;
    }
    if(rt_thread_startup(&s_ethch395_event_pro_thread) != RT_EOK){
        LOG_E("ethch395 event process thread startup fail");
        res = -0x01;
        goto _init_end;
    }

    if(rt_thread_init(&s_ethch395_recv_thread, "395recv", ethch395_recv_thread_entry, NULL,
            s_ethch395_recv_thread_stack, ETHCH395_RECV_THREAD_STACK_SIZE, 14, 5) != RT_EOK){
        LOG_E("ethch395 recv thread init fail");
        res = -0x01;
        goto _init_end;
    }

    if(rt_event_init(&s_ethch395_event, "395event", RT_IPC_FLAG_PRIO) != RT_EOK){
        LOG_E("ethch395 event set init fail");
        res = -0x01;
        goto _init_end;
    }
    if(rt_thread_startup(&s_ethch395_recv_thread) != RT_EOK){
        LOG_E("ethch395 recv thread startup fail");
        res = -0x01;
        goto _init_end;
    }

_init_end:

    if(ethch395_init_hook){
        if(res >= 0x00){
            ethch395_init_hook(ETHCH395_ENUM_FALSE, ETHCH395_ENUM_FALSE);
        }else{
            ethch395_init_hook(ETHCH395_ENUM_TRUE, ETHCH395_ENUM_FALSE);
        }
    }
    if(res < 0x00){
        s_ethch395_assistant_info.flag.error = ETHCH395_ENUM_TRUE;
        s_ethch395_assistant_info.flag.init_complete = ETHCH395_ENUM_TRUE;
    }

    return res;
}

#endif /* NET_INCLUDE_ETHERNET_PACK */

