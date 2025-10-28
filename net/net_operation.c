/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#include "net_operation.h"
#include "app_ofsm.h"

#include "ykc_message_send.h"
#include "ycp_message_send.h"
#include "ykc_monitor_message_send.h"
#include "sgcc_message_send.h"

#define DBG_TAG "net_operation"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define NET_ENABLE_DEBUG

#define NET_SYSYTEM_TOTAL_POWER_MIN         1000   /* 系统总功率最小值 */
#pragma pack(1)

struct setup_para{
    uint32_t total_power[NET_SYSTEM_GUN_NUMBER];          /* 系统总功率，单位W */
    uint8_t fees_set_gunno;                               /* 费率设置的枪号 */
};

#pragma pack()

NET_DEF_SRAM2 static struct setup_para s_setup_para;
NET_DEF_SRAM2 static net_plat_socket_info_t s_plat_socket_info;
NET_DEF_SRAM2 static uint8_t s_net_ndev_reset;
NET_DEF_SRAM2 static net_ota_info_t s_net_ota_info;
NET_DEF_SRAM2 static uint32_t s_net_operation_event_set[NET_SYSTEM_GUN_NUMBER];

static void net_start_function(void* handle);
static int32_t net_para_config_function(uint8_t platform, uint8_t index, void* para, void* handle);

#ifdef NET_ENABLE_DEBUG
void NETDATA_DEBUG(const char *id, void *data, int len, uint8_t dir)
{
    if(len >= 1024){
        return;
    }

    int total;
    uint8_t line_num, count, *ptr = (uint8_t*)data, dir_ch = '>', sline_dcount = 0x10;

    total = len;
    line_num = (total /sline_dcount);
    if(total %sline_dcount){
        line_num += 0x01;
    }

    if(dir == NETDATA_DEBUG_DIR_RECV){
        dir_ch = '<';
    }

//    printf("\033[34m");

    for(uint8_t line = 0x00; line < line_num; line++){
        count = 0x10;
        if(total < 0x10){
            count = total;
        }

        rt_kprintf("[%s-D%03d] %c ", id, len, dir_ch);

        for(uint8_t i = 0x00; i < count; i++){
            if(ptr[line *sline_dcount + i] < 0x10){
                rt_kprintf("0%X ", ptr[line *sline_dcount + i]);
            }else{
                rt_kprintf("%X ", ptr[line *sline_dcount + i]);
            }
            if((i + 0x01) == sline_dcount /0x02){
                rt_kprintf(" ");
            }
        }

        rt_kprintf(" | ");
        for(uint8_t i = 0x00; i < count; i++){
            if(ptr[line *sline_dcount + i] >= 0x20){
                rt_kprintf("%c", ptr[line *sline_dcount + i]);
            }
        }

        rt_kprintf("\n");

        total -= count;
    }
//    printf("\033[0m\r\n");
}
#else
void NETDATA_DEBUG(const char *id, void *data, int len, uint8_t dir)
{

}
#endif /* NET_ENABLE_DEBUG */

NET_DEF_SRAM2 static struct net_handle s_net_handle =
{
    .net_fault = 0xFF,
    .start_func = net_start_function,
    .para_config = net_para_config_function,
};

static int32_t net_operation_init(void);

#ifdef NET_DESIGNATE_REGION
/*************************************************
 * 函数名      net_operation_info_init
 * 功能          网络总操作信息、变量初始化
 * **********************************************/
void net_operation_info_init(void)
{
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_net_operation_event_set[gunno] = 0x00;
    }
    s_net_handle.net_fault = 0xFF;
    s_net_handle.start_func = net_start_function;
    s_net_handle.para_config = net_para_config_function;

    s_net_ndev_reset = 0x00;

    memset(&s_setup_para, 0x00, sizeof(s_setup_para));
    memset(&s_plat_socket_info, 0x00, sizeof(s_plat_socket_info));
    memset(&s_net_ota_info, 0x00, sizeof(s_net_ota_info));

}
#endif /* NET_DESIGNATE_REGION */

/******************************************
 * 函数名     net_thread_init_hook
 * 功能         网络模块线程初始化回调
 * 参数         thread   线程句柄
 *      para     可选参数
 *      plen     参数长度(B)
 *      option   选项字
 * 返回
 * ***************************************/
int32_t net_thread_init_hook(void *thread, void *para, uint32_t plen, uint32_t option)
{
    if(s_net_handle.thread_init_hook){
        return s_net_handle.thread_init_hook(thread, para, plen, option);
    }
    return -0x01;
}

/******************************************
 * 函数名     net_thread_running
 * 功能         网络模块线程运行回调
 * 参数         thread   线程句柄
 *      para     可选参数
 *      plen     参数长度(B)
 *      option   选项字
 * 返回
 * ***************************************/
int32_t net_thread_running(void *thread, void *para, uint32_t plen, uint32_t option)
{
    if(s_net_handle.thread_running){
        return s_net_handle.thread_running(thread, para, plen, option);
    }
    return -0x01;
}

/******************************************
 * 函数名     net_set_clear_ndev_reset_state
 * 功能         设置清除网络设备复位状态
 * ***************************************/
void net_set_clear_ndev_reset_state(uint8_t plat_mask, uint8_t is_clear)
{
    if(is_clear){
        s_net_ndev_reset &= (~plat_mask);
    }else{
        s_net_ndev_reset |= plat_mask;
    }

#if 0
    if((s_net_ndev_reset &NET_PLATFORM_MASK_ALL) == NET_PLATFORM_MASK_ALL){
        s_net_ndev_reset &= (~NET_PLATFORM_MASK_ALL);
        (void)s_net_handle.ndev_operate(NULL, NET_DEV_OPERATE_OPTION_RESET);
    }
#else
    if(is_clear == 0x00){
        (void)s_net_handle.ndev_operate(NULL, NET_DEV_OPERATE_OPTION_RESET);
    }
#endif
}

/******************************************
 * 函数名     net_get_ota_info
 * 功能         获取OTA信息
 * ***************************************/
net_ota_info_t *net_get_ota_info(void)
{
    return &s_net_ota_info;
}

/******************************************
 * 函数名     net_operation_set_event
 * 功能         设置网络事件(用于外部调用)
 * ***************************************/
void net_operation_set_event(uint8_t gunno, uint8_t event)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    s_net_operation_event_set[gunno] |= (1 <<event);
}
/******************************************
 * 函数名     net_operation_get_event
 * 功能         获取网络事件(用于外部调用)
 * ***************************************/
uint8_t net_operation_get_event(uint8_t gunno, uint8_t event)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    if(s_net_operation_event_set[gunno] &(1 <<event)){
        return 0x01;
    }
    return 0x00;
}
/******************************************
 * 函数名     net_operation_clear_event
 * 功能         清除网络事件(用于外部调用)
 * ***************************************/
void net_operation_clear_event(uint8_t gunno, uint8_t event)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_net_operation_event_set[gunno] &= (~(1 <<event));
}

/******************************************
 * 函数名     net_operation_clear_net_fault
 * 功能         清除网络故障(用于外部调用)
 * ***************************************/
void net_operation_clear_net_fault(uint8_t event)
{
    s_net_handle.net_fault &= (~event);
}

/******************************************
 * 函数名     net_operation_set_net_fault
 * 功能         设置网络故障(用于外部调用)
 * ***************************************/
void net_operation_set_net_fault(uint8_t event)
{
    s_net_handle.net_fault |= event;
}

/******************************************
 * 函数名     net_operation_set_esock_state
 * 功能         设置外部socket 状态(用于外部调用)
 * ***************************************/
void net_operation_set_esock_state(uint8_t state)
{
    s_net_handle.esocket_state = state;
}

/******************************************
 * 函数名     net_operation_get_esock_state
 * 功能         获取外部socket 状态(用于外部调用)
 * ***************************************/
uint8_t net_operation_get_esock_state(void)
{
    return s_net_handle.esocket_state;
}

/******************************************
 * 函数名     net_operation_set_total_power
 * 功能         设置系统总功率
 * ***************************************/
void net_operation_set_total_power(uint32_t power, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_setup_para.total_power[gunno] = power;
}

/******************************************
 * 函数名     net_operation_get_total_power
 * 功能         获取系统总功率
 * ***************************************/
uint32_t net_operation_get_total_power(uint8_t gunno)
{
    return s_setup_para.total_power[gunno];
}

/******************************************
 * 函数名     net_operation_get_target_socket_info
 * 功能         获取目标平台 socket 信息
 * 参数         sync_data   是否同步数据(1:是，0：否)
 * 返回          目标平台 socket 信息
 * ***************************************/
net_plat_socket_info_t *net_operation_get_target_socket_info(uint8_t sync_data)
{
/** 注：如果是双socket 或有双枪信息，则A枪信息放在高4位，B枪信息放在低4位 */
    if(sync_data){
#ifdef NET_YKC_AS_TARGET
        ykc_socket_info_t *ykc_socket = ykc_get_socket_info();
        s_plat_socket_info.socket_state = ykc_socket->socket_state;
        s_plat_socket_info.open_count = ykc_socket->operate_fail.open_socket;
        s_plat_socket_info.login_count = ykc_socket->operate_fail.login;
        s_plat_socket_info.program_state = ykc_socket->program_state;
        s_plat_socket_info.heartbeat_count = 0x00;
        for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            s_plat_socket_info.heartbeat_count |= (uint8_t)(ykc_socket->heartbeat[i] &0x0F);
            if((i + 0x01) < NET_SYSTEM_GUN_NUMBER){
                s_plat_socket_info.heartbeat_count <<=0x04;
            }
        }

        return &s_plat_socket_info;
#endif /* NET_YKC_AS_TARGET */

#ifdef NET_YKC_MONITOR_AS_TARGET
        ykc_monitor_socket_info_t *ykc_monitor_socket = ykc_monitor_get_socket_info();
        s_plat_socket_info.socket_state = ykc_monitor_socket->socket_state;
        s_plat_socket_info.open_count = ykc_monitor_socket->operate_fail.open_socket;
        s_plat_socket_info.login_count = ykc_monitor_socket->operate_fail.login;
        s_plat_socket_info.program_state = ykc_monitor_socket->program_state;
        s_plat_socket_info.heartbeat_count = 0x00;
        for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
            s_plat_socket_info.heartbeat_count |= (uint8_t)(ykc_monitor_socket->heartbeat[i] &0x0F);
            if((i + 0x01) < NET_SYSTEM_GUN_NUMBER){
                s_plat_socket_info.heartbeat_count <<=0x04;
            }
        }

        return &s_plat_socket_info;
#endif /* NET_YKC_MONITOR_AS_TARGET */

#ifdef NET_YCP_AS_TARGET
        ycp_socket_info_t *ycp_socket = ycp_get_socket_info();
        s_plat_socket_info.socket_state = ycp_socket->socket_state;
        s_plat_socket_info.open_count = ycp_socket->operate_fail.open_socket;
        s_plat_socket_info.login_count = ycp_socket->operate_fail.login;
        s_plat_socket_info.heartbeat_count = ycp_socket->heartbeat;
        s_plat_socket_info.program_state = ycp_socket->program_state;

        return &s_plat_socket_info;
#endif /* NET_YCP_AS_TARGET */

#ifdef NET_SGCC_AS_TARGET
        sgcc_socket_info_t *sgcc_socket = sgcc_get_socket_info();
        s_plat_socket_info.socket_state = sgcc_socket->socket_state;
        s_plat_socket_info.open_count = sgcc_socket->operate_fail.open;
        s_plat_socket_info.login_count = sgcc_socket->operate_fail.login;
        s_plat_socket_info.heartbeat_count = sgcc_socket->sync_repeat;
        s_plat_socket_info.program_state = sgcc_socket->program_state;

        return &s_plat_socket_info;
#endif /* NET_SGCC_AS_TARGET */
    }

    return &s_plat_socket_info;
}

/******************************************
 * 函数名     net_operation_set_target_socket_domain
 * 功能         设置目标平台 socket 域名
 * ***************************************/
void net_operation_set_target_socket_domain(char* domain, uint8_t domain_len)
{
    uint8_t valid_len = sizeof(s_plat_socket_info.domain);
    valid_len = valid_len > domain_len ? domain_len : valid_len;

    memset(s_plat_socket_info.domain, 0x00, sizeof(s_plat_socket_info.domain));
    memcpy(s_plat_socket_info.domain, domain, valid_len);
}

/******************************************
 * 函数名     net_operation_set_target_socket_port
 * 功能         设置目标平台 socket 端口
 * ***************************************/
void net_operation_set_target_socket_port(uint16_t port)
{
    s_plat_socket_info.port = port;
    s_plat_socket_info.domain_is_prase = 0x01;    /** 先设置了 domain ，然后才设置port */
}

/******************************************
 * 函数名     net_operation_insert_tplat_log
 * 功能         保存目标平台日志
 * ***************************************/
void net_operation_insert_tplat_log(void *data, uint16_t len, uint8_t verify_result, const char* label)
{
#ifdef NET_INCLUDE_MONITOR_PLATFORM
    ykc_monitor_platlog_data_insert(data, len, verify_result, label);
#endif /* NET_INCLUDE_MONITOR_PLATFORM */
}

/******************************************
 * 函数名     net_operation_tplat_info_trigger
 * 功能         触发上报目标平台socket信息
 * 参数         sync_data   是否同步数据(1:是，0：否)
 * 返回
 * ***************************************/
void net_operation_tplat_info_trigger(uint8_t sync_data)
{
#ifdef NET_INCLUDE_MONITOR_PLATFORM
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    (void)net_operation_get_target_socket_info(sync_data);
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
            0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_TSOCKET_INFO);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_INCLUDE_MONITOR_PLATFORM */
}

/******************************************
 * 函数名     net_operation_set_fees_gunno
 * 功能         设置已配置了费率的枪号
 * 参数         gunno       枪号
 *       is_appand   是否是追加
 * 返回
 * ***************************************/
void net_operation_set_fees_gunno(uint8_t gunno, uint8_t is_appand)
{
#ifdef NET_INCLUDE_MONITOR_PLATFORM
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(is_appand){
        s_setup_para.fees_set_gunno |= (uint8_t)(0x01 <<gunno);
    }else{
        /** 当 gunno >= 8 *sizeof(s_setup_para.fees_set_gunno) 时表示清空 s_setup_para.fees_set_gunno */
        s_setup_para.fees_set_gunno = (uint8_t)(0x01 <<gunno);
    }
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_INCLUDE_MONITOR_PLATFORM */
}

/******************************************
 * 函数名     net_operation_is_gunno_updated_fees
 * 功能         判断枪是否已经设置了费率
 * 参数         gunno   枪号
 * 返回         1：已设置     0：未设置
 * ***************************************/
uint8_t net_operation_is_gunno_updated_fees(uint8_t gunno)
{
#ifdef NET_INCLUDE_MONITOR_PLATFORM
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(s_setup_para.fees_set_gunno &(0x01 <<gunno)){
        return 0x01;
    }

    return 0x00;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_INCLUDE_MONITOR_PLATFORM */
}

/******************************************
 * 函数名     net_operation_updated_billing_trigger
 * 功能         触发上报目标平台计费信息信息
 * 参数
 * 返回
 * ***************************************/
void net_operation_updated_billing_trigger(void)
{
#ifdef NET_INCLUDE_MONITOR_PLATFORM
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    ykc_monitor_net_event_send(NET_YKC_MONITOR_EXTERNAL_EHANDLE_CHARGEPILE, NET_YKC_MONITOR_EVENT_TYPE_REQUEST,  \
            0x00, NET_YKC_MONITOR_EXTERNAL_PREQ_EVENT_BILLING_INFO);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_INCLUDE_MONITOR_PLATFORM */
}

/******************************************
 * 函数名     net_dis_reason_store
 * 功能         保存断网原因信息
 * 参数         fd          文件描述符
 *       en           原因枚举
 * 返回
 * ***************************************/
void net_dis_reason_store(int fd, uint8_t en)
{
#ifdef NET_INCLUDE_MONITOR_PLATFORM
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    extern void ykc_monitor_disconnect_reason_callback(int8_t fd, uint8_t reason_en);
    ykc_monitor_disconnect_reason_callback(fd, en);
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_INCLUDE_MONITOR_PLATFORM */
}

static void net_start_function(void* handle)
{
    if(handle != &s_net_handle){
        LOG_E("input net handle error when call net_start_function");
        return;
    }

    NET_MY_ASSERT(s_net_handle.data_updata, NET_PARA_CONFIG_INDEX_DATA_UPDATA);
    NET_MY_ASSERT(s_net_handle.time_sync, NET_PARA_CONFIG_INDEX_TIME_SYNC);
    NET_MY_ASSERT(s_net_handle.get_base_data, NET_PARA_CONFIG_INDEX_GET_BASE_DATA);
    NET_MY_ASSERT(s_net_handle.get_system_data, NET_PARA_CONFIG_INDEX_GET_SYSTEM_DATA);
    NET_MY_ASSERT(s_net_handle.set_system_data, NET_PARA_CONFIG_INDEX_SET_SYSTEM_DATA);
    NET_MY_ASSERT(s_net_handle.card_vin_whitelists_set, NET_PARA_CONFIG_INDEX_SET_CARD_VIN);
    NET_MY_ASSERT(s_net_handle.card_vin_whitelists_query, NET_PARA_CONFIG_INDEX_QUERY_CARD_VIN);
    NET_MY_ASSERT(s_net_handle.card_vin_whitelists_delete, NET_PARA_CONFIG_INDEX_DELETE_CARD_VIN);
    NET_MY_ASSERT(s_net_handle.system_data_storage, NET_PARA_CONFIG_INDEX_SYSTEM_DATA_STORAGE);
    NET_MY_ASSERT(s_net_handle.system_control, NET_PARA_CONFIG_INDEX_SYSTEM_CONTROL);
    NET_MY_ASSERT(s_net_handle.query_system_record, NET_PARA_CONFIG_INDEX_QUERY_SYSTEM_RECORD);
    NET_MY_ASSERT(s_net_handle.ndev_operate, NET_PARA_CONFIG_INDEX_NDEV_OPERATE);
    NET_MY_ASSERT(s_net_handle.crc16_8005, NET_PARA_CONFIG_INDEX_CRC16_8005);
    NET_MY_ASSERT(s_net_handle.crc32_updtae, NET_PARA_CONFIG_INDEX_CRC32_UPDATE);

    NET_MY_ASSERT(s_net_handle.flash_erase, NET_PARA_CONFIG_INDEX_FLASH_ERASE);
    NET_MY_ASSERT(s_net_handle.flash_read, NET_PARA_CONFIG_INDEX_FLASH_READ);
    NET_MY_ASSERT(s_net_handle.flash_write, NET_PARA_CONFIG_INDEX_FLASH_WRITE);
    NET_MY_ASSERT(s_net_handle.flash_write_directly, NET_PARA_CONFIG_INDEX_FLASH_WRITE_DIRECTLY);

    NET_MY_ASSERT(s_net_handle.thread_init_hook, NET_PARA_CONFIG_INDEX_THREAD_INIT);
    NET_MY_ASSERT(s_net_handle.thread_running, NET_PARA_CONFIG_INDEX_THREAD_RUNNING);

    net_operation_init();
}

static int32_t net_para_config_function(uint8_t platform, uint8_t index, void* para, void* handle)
{
    if(handle != &s_net_handle){
        LOG_E("input net handle error when call net_para_config_function");
        return -0x01;
    }
    if(index >= NET_PARA_CONFIG_INDEX_SIZE){
        LOG_E("input net config_index error when call net_para_config_function");
        return -0x02;
    }
    if(para == NULL){
        LOG_E("input net para null when call net_para_config_function");
        return -0x03;
    }

    switch(index){
    case NET_PARA_CONFIG_INDEX_FLASH_ERASE :
        s_net_handle.flash_erase = (int32_t (*)(uint32_t, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_FLASH_READ :
        s_net_handle.flash_read = (int32_t (*)(uint32_t, uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_FLASH_WRITE :
        s_net_handle.flash_write = (int32_t (*)(uint32_t, uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_FLASH_WRITE_DIRECTLY :
        s_net_handle.flash_write_directly = (int32_t (*)(uint32_t, uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_DATA_UPDATA :
        s_net_handle.data_updata = (void (*)(void))para;
        break;
    case NET_PARA_CONFIG_INDEX_TIME_SYNC :
        s_net_handle.time_sync = (void (*)(uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_GET_SYSTEM_DATA :
        s_net_handle.get_system_data = (uint8_t* (*)(uint8_t, void *, uint32_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_SET_SYSTEM_DATA :
        s_net_handle.set_system_data = (int32_t (*)(uint8_t, uint8_t*, uint16_t, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_SET_CARD_VIN :
        s_net_handle.card_vin_whitelists_set = (int32_t (*)(uint8_t*, uint8_t, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_QUERY_CARD_VIN :
        s_net_handle.card_vin_whitelists_query = (int32_t (*)(uint8_t*, uint8_t, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_DELETE_CARD_VIN :
        s_net_handle.card_vin_whitelists_delete = (int32_t (*)(uint8_t*, uint8_t, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_CRC16_8005 :
        s_net_handle.crc16_8005 = (uint16_t (*)(uint16_t, const uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_CRC32_UPDATE :
        s_net_handle.crc32_updtae = (uint32_t (*)(uint32_t, const uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_GET_BASE_DATA :
        s_net_handle.get_base_data = (void* (*)(uint8_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_SYSTEM_DATA_STORAGE :
        s_net_handle.system_data_storage = (int32_t (*)(uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_SYSTEM_CONTROL :
        s_net_handle.system_control = (int32_t (*)(uint8_t, uint16_t, uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_QUERY_SYSTEM_RECORD :
        s_net_handle.query_system_record = (int32_t (*)(net_record_info_t*))para;
        break;
    case NET_PARA_CONFIG_INDEX_NDEV_OPERATE :
        s_net_handle.ndev_operate = (int32_t (*)(void*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_THREAD_INIT :
        s_net_handle.thread_init_hook = (int32_t (*)(void*, void*, uint32_t, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_THREAD_RUNNING :
        s_net_handle.thread_running = (int32_t (*)(void*, void*, uint32_t, uint32_t))para;
        break;
    default:
        return -0x06;
        break;
    }
    return 0x00;
}

struct net_handle* net_get_net_handle(void)
{
    return &s_net_handle;
}

static int32_t net_operation_init(void)
{
/** 钛享平台 */
#ifdef NET_PACK_USING_THA
    tha_ota_init();
    tha_transceiver_init();    /** 初始化有先后顺序 */
    tha_message_recv_init();   /** 初始化有先后顺序 */
    net_tha_message_send_init();
#endif /* NET_PACK_USING_THA */

/** 云快充平台 */
#ifdef NET_PACK_USING_YKC
    extern int32_t ykc_ota_init(void);
    extern int32_t ykc_message_recv_init(void);
    extern int32_t ykc_transceiver_init(void);
    extern int32_t ykc_message_send_init(void);
    extern int32_t ykc_realtime_process_init(void);

#ifdef NET_DESIGNATE_REGION
    extern void ykc_fault_info_init(void);
    extern void ykc_mreceive_info_init(void);

    ykc_fault_info_init();
    ykc_mreceive_info_init();
#endif /* NET_DESIGNATE_REGION */

    ykc_ota_init();
    ykc_transceiver_init();    /** 初始化有先后顺序 */
    ykc_message_recv_init();   /** 初始化有先后顺序 */
    ykc_message_send_init();
    ykc_realtime_process_init();
#endif /* NET_PACK_USING_YKC */

/** 监控平台 */
#ifdef NET_PACK_USING_YKC_MONITOR
    extern int32_t ykc_monitor_ota_init(void);
    extern int32_t ykc_monitor_message_recv_init(void);
    extern int32_t ykc_monitor_transceiver_init(void);
    extern int32_t ykc_monitor_message_send_init(void);
    extern int32_t ykc_monitor_realtime_process_init(void);

#ifdef NET_DESIGNATE_REGION
    extern void ykc_monitor_fault_info_init(void);
    extern void ykc_monitor_mreceive_info_init(void);

    ykc_monitor_fault_info_init();
    ykc_monitor_mreceive_info_init();
#endif /* NET_DESIGNATE_REGION */

    ykc_monitor_ota_init();
    ykc_monitor_transceiver_init();     /** 初始化有先后顺序 */
    ykc_monitor_message_recv_init();    /** 初始化有先后顺序 */
    ykc_monitor_message_send_init();
    ykc_monitor_realtime_process_init();
#endif /* NET_PACK_USING_YKC_MONITOR */

/** 越城公用平台 */
#ifdef NET_PACK_USING_YCP
    extern int32_t ycp_ota_init(void);
    extern int32_t ycp_message_recv_init(void);
    extern int32_t ycp_transceiver_init(void);
    extern int32_t ycp_message_send_init(void);
    extern int32_t ycp_realtime_process_init(void);

#ifdef NET_DESIGNATE_REGION
    extern void ycp_fault_info_init(void);
    extern void ycp_mreceive_info_init(void);

    ycp_fault_info_init();
    ycp_mreceive_info_init();
#endif /* NET_DESIGNATE_REGION */

    ycp_ota_init();
    ycp_transceiver_init();     /** 初始化有先后顺序 */
    ycp_message_recv_init();    /** 初始化有先后顺序 */
    ycp_message_send_init();
    ycp_realtime_process_init();
#endif /* NET_PACK_USING_YCP */

/** 国网平台 */
#ifdef NET_PACK_USING_SGCC
    extern int sgcc_ota_init(void);
    extern int sgcc_message_recvive_init(void);
    extern int sgcc_message_send_init(void);
    extern int sgcc_realtime_process_init(void);
    extern void sgcc_device_register_init(void);

#ifdef NET_DESIGNATE_REGION
    extern void sgcc_fault_info_init(void);
    extern void sgcc_mreceive_info_init(void);

    sgcc_fault_info_init();
    sgcc_mreceive_info_init();
#endif /* NET_DESIGNATE_REGION */

    sgcc_ota_init();
    sgcc_message_send_init();
    sgcc_message_recvive_init();
    sgcc_realtime_process_init();
    sgcc_device_register_init();
#endif /* NET_PACK_USING_SGCC */

#ifdef NET_PACK_USING_XJ

#endif /* NET_PACK_USING_XJ */

#ifdef NET_PACK_USING_SL

#endif /* NET_PACK_USING_SL */

    memset(&s_plat_socket_info, 0x00, sizeof(s_plat_socket_info));

    return 0x00;
}

