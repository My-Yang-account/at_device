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

#define DBG_TAG "net_operation"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define NET_ENABLE_DEBUG

#define NET_SYSYTEM_TOTAL_POWER_MIN         1000   /* 系统总功率最小值 */
#pragma pack(1)

struct setup_para{
    uint32_t total_power;          /* 系统总功率，单位W */
};

#pragma pack()

static struct setup_para s_setup_para;
static net_plat_socket_info_t s_plat_socket_info;
static uint8_t s_net_ndev_reset;
static net_ota_info_t s_net_ota_info;
static uint32_t s_net_operation_event_set[NET_SYSTEM_GUN_NUMBER];

static void net_start_function(void* handle);
static int32_t net_para_config_function(uint8_t platform, uint8_t index, void* para, void* handle);

#ifdef NET_ENABLE_DEBUG
void NETDATA_DEBUG(const char *id, void *data, int len, uint8_t dir)
{
    if(len >= 300){
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

static struct net_handle s_net_handle =
{
    .net_fault = 0xFF,
    .start_func = net_start_function,
    .para_config = net_para_config_function,
};

static int32_t net_operation_init(void);

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

    if(s_net_ndev_reset &NET_PLATFORM_MASK_ALL){
        s_net_ndev_reset &= (~NET_PLATFORM_MASK_ALL);
        (void)s_net_handle.ndev_operate(NET_DEV_OPERATE_OPTION_RESET);
    }
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
 * 函数名     net_operation_set_total_power
 * 功能         设置系统总功率
 * ***************************************/
void net_operation_set_total_power(uint32_t power)
{
    s_setup_para.total_power = power;
}

/******************************************
 * 函数名     net_operation_get_total_power
 * 功能         获取系统总功率
 * ***************************************/
uint32_t net_operation_get_total_power(void)
{
    return s_setup_para.total_power;
}

/******************************************
 * 函数名     net_operation_get_target_socket_state
 * 功能         获取目标平台 socket 信息
 * ***************************************/
net_plat_socket_info_t net_operation_get_target_socket_state(void)
{
    memset(&s_plat_socket_info, 0x00, sizeof(s_plat_socket_info));
#ifdef NET_YKC_AS_TARGET
    s_plat_socket_info.state = ykc_get_socket_info()->state;
    s_plat_socket_info.open_count = ykc_get_socket_info()->operate_fail.open_socket;
    s_plat_socket_info.login_count = ykc_get_socket_info()->operate_fail.login;

    return s_plat_socket_info;
#endif /* NET_YKC_AS_TARGET */

#ifdef NET_YKC_MONITOR_AS_TARGET
    s_plat_socket_info.state = ykc_monitor_get_socket_info()->state;
    s_plat_socket_info.open_count = ykc_monitor_get_socket_info()->operate_fail.open_socket;
    s_plat_socket_info.login_count = ykc_monitor_get_socket_info()->operate_fail.login;

    return s_plat_socket_info;
#endif /* NET_YKC_MONITOR_AS_TARGET */

#ifdef NET_YCP_AS_TARGET
    s_plat_socket_info.state = ycp_get_socket_info()->state;
    s_plat_socket_info.open_count = ycp_get_socket_info()->operate_fail.open_socket;
    s_plat_socket_info.login_count = ycp_get_socket_info()->operate_fail.login;

    return s_plat_socket_info;
#endif /* NET_YCP_AS_TARGET */

    return s_plat_socket_info;
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
    NET_MY_ASSERT(s_net_handle.system_data_storage, NET_PARA_CONFIG_INDEX_NDEV_OPERATE);
    NET_MY_ASSERT(s_net_handle.crc32_updtae, NET_PARA_CONFIG_INDEX_CRC32_UPDATE);

    NET_MY_ASSERT(s_net_handle.flash_erase, NET_PARA_CONFIG_INDEX_FLASH_ERASE);
    NET_MY_ASSERT(s_net_handle.flash_read, NET_PARA_CONFIG_INDEX_FLASH_READ);
    NET_MY_ASSERT(s_net_handle.flash_write, NET_PARA_CONFIG_INDEX_FLASH_WRITE);
    NET_MY_ASSERT(s_net_handle.flash_write_directly, NET_PARA_CONFIG_INDEX_FLASH_WRITE_DIRECTLY);

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
        s_net_handle.get_system_data = (uint8_t* (*)(uint8_t, uint32_t*, uint32_t))para;
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
    case NET_PARA_CONFIG_INDEX_CRC32_UPDATE :
        s_net_handle.crc32_updtae = (uint32_t (*)(uint32_t, const uint8_t*, uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_GET_BASE_DATA :
        s_net_handle.get_base_data = (void* (*)(uint8_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_SYSTEM_DATA_STORAGE :
        s_net_handle.system_data_storage = (int32_t (*)(uint32_t))para;
        break;
    case NET_PARA_CONFIG_INDEX_NDEV_OPERATE :
        s_net_handle.ndev_operate = (int32_t (*)(uint8_t))para;
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
#ifdef NET_PACK_USING_THA
    tha_ota_init();
    tha_message_recv_init();
    tha_transceiver_init();
    net_tha_message_send_init();
#endif /* NET_PACK_USING_THA */

#ifdef NET_PACK_USING_YKC
    extern int32_t ykc_ota_init(void);
    extern int32_t ykc_message_recv_init(void);
    extern int32_t ykc_transceiver_init(void);
    extern int32_t ykc_message_send_init(void);
    extern int32_t ykc_realtime_process_init(void);
    ykc_ota_init();
    ykc_message_recv_init();
    ykc_transceiver_init();
    ykc_message_send_init();
    ykc_realtime_process_init();
#endif /* NET_PACK_USING_YKC */

#ifdef NET_PACK_USING_YKC_MONITOR
    extern int32_t ykc_monitor_ota_init(void);
    extern int32_t ykc_monitor_message_recv_init(void);
    extern int32_t ykc_monitor_transceiver_init(void);
    extern int32_t ykc_monitor_message_send_init(void);
    extern int32_t ykc_monitor_realtime_process_init(void);
    ykc_monitor_ota_init();
    ykc_monitor_message_recv_init();
    ykc_monitor_transceiver_init();
    ykc_monitor_message_send_init();
    ykc_monitor_realtime_process_init();
#endif /* NET_PACK_USING_YKC_MONITOR */

#ifdef NET_PACK_USING_YCP
    extern int32_t ycp_ota_init(void);
    extern int32_t ycp_message_recv_init(void);
    extern int32_t ycp_transceiver_init(void);
    extern int32_t ycp_message_send_init(void);
    ycp_ota_init();
    ycp_message_recv_init();
    ycp_transceiver_init();
    ycp_message_send_init();
#endif /* NET_PACK_USING_YCP */

#ifdef NET_PACK_USING_XJ

#endif /* NET_PACK_USING_XJ */

#ifdef NET_PACK_USING_SL

#endif /* NET_PACK_USING_SL */

    return 0x00;
}

