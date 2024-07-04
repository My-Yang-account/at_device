/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#include "net_function_config.h"
#include "net_operation.h"
#include "app_ofsm.h"
#include "mw_norflash.h"

#include "chargepile_config.h"


/***********************************************
 * 函数名     app_ndata_update
 * 功能         更新外部数据
 **********************************************/
static void app_ndata_update(void)
{
    extern int get_at_device_appinfo_at(void);
    extern int get_at_device_appinfo_is_complete(void);
    if(get_at_device_appinfo_at()){
        net_operation_clear_net_fault(NET_FAULT_PHYSICAL_LAYER);
    }else{
        net_operation_set_net_fault(NET_FAULT_PHYSICAL_LAYER);
    }

    if(get_at_device_appinfo_is_complete()){
        net_operation_clear_net_fault(NET_FAULT_DATA_LINK_LAYER);
    }else{
        net_operation_set_net_fault(NET_FAULT_DATA_LINK_LAYER);
    }
}

/***********************************************
 * 函数名     app_ndevice_operate
 * 功能         操作网络设备
 **********************************************/
static int32_t app_ndevice_operate(uint8_t option)
{
    if(option &NET_DEV_OPERATE_OPTION_RESET){
        extern void ec20_at_device_reset(void);

        net_operation_set_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_set_net_fault(NET_FAULT_DATA_LINK_LAYER);
        ec20_at_device_reset();
        return 0x00;
    }
    return -0x01;
}

/***********************************************
 * 函数名     app_ntime_sync
 * 功能         时间同步
 **********************************************/
static void app_ntime_sync(uint32_t timestamp)
{
    extern void mw_set_datetime_from_timestamp(uint32_t timestamp);
    mw_set_datetime_from_timestamp(timestamp);
}

/***********************************************
 * 函数名     app_nget_base_data
 * 功能         获取外部基本数据
 **********************************************/
static void* app_nget_base_data(uint8_t gunno)
{
    return &(get_ofsm_info(gunno)->base);
}

static int32_t app_nsystem_data_storage(uint32_t option)
{
    if(option &NET_SYSTEM_DATA_OPTION_CARD_WHITELIST){
        return sys_card_whitelists_storage();
    }else if(option &NET_SYSTEM_DATA_OPTION_VIN_WHITELIST){
        return sys_vin_whitelists_storage();
    }
    return -0x01;
}



//
///** system data name */
//#define NET_SYSTEM_DATA_NAME_PILE_NUMBER               0x00    /* 系统数据名：桩号 */
//#define NET_SYSTEM_DATA_NAME_NETWORKED_WAY             0x01    /* 系统数据名：联网方式 0 为2G， 1为3G，2为4G，3为5G,4为wifi, 5为eth */
//#define NET_SYSTEM_DATA_NAME_MCC                       0x02    /* 系统数据名：国家码，中国代码460即0x0C1C */
//#define NET_SYSTEM_DATA_NAME_MNC                       0x03    /* 系统数据名：0移动，1联通(电信对应sid) 2非sim卡方式 */
//#define NET_SYSTEM_DATA_NAME_LAC                       0x04    /* 系统数据名：lac(电信对应nid) 非sim卡方式时全0 */
//#define NET_SYSTEM_DATA_NAME_CI                        0x05    /* 系统数据名：cellid(电信对应bid) 非sim卡方式时全0 */
//#define NET_SYSTEM_DATA_NAME_IMEI                      0x06    /* 系统数据名：移动设备身份码（电子串号），不足部分后缀补0，非sim卡方式时全0 */
//#define NET_SYSTEM_DATA_NAME_RAM                       0x07    /* 系统数据名：单位为K字节，充电设备RAM大小 */
//#define NET_SYSTEM_DATA_NAME_ROM                       0x08    /* 系统数据名：单位为 K字节，充电设备ROM大小 */
//#define NET_SYSTEM_DATA_NAME_MAC                       0x09    /* 系统数据名：sim卡方式时全0 */
//#define NET_SYSTEM_DATA_NAME_ICCID                     0x0A    /* 系统数据名：SIM卡卡号 */
//#define NET_SYSTEM_DATA_NAME_IMSI                      0x0B    /* 系统数据名： */
//#define NET_SYSTEM_DATA_NAME_OPERATOR                  0x0C    /* 系统数据名： 运营商名称*/
//#define NET_SYSTEM_DATA_NAME_DOMAIN                    0x0D    /* 系统数据名： 域名*/
//#define NET_SYSTEM_DATA_NAME_PORT                      0x0E    /* 系统数据名： 端口号*/
//#define NET_SYSTEM_DATA_NAME_VOLTAGE_MAX               0x0F    /* 系统数据名： 最大充电电压*/
//#define NET_SYSTEM_DATA_NAME_VOLTAGE_MIN               0x10    /* 系统数据名： 最小充电电压*/
//#define NET_SYSTEM_DATA_NAME_CURRENT_MAX               0x11    /* 系统数据名： 最大充电电流*/
//#define NET_SYSTEM_DATA_NAME_MODULE_NUM                0x12    /* 系统数据名： 充电模块总数*/
//#define NET_SYSTEM_DATA_NAME_SINGLE_MODULE_POWER       0x13    /* 系统数据名： 单个模块的功率*/
//#define NET_SYSTEM_DATA_NAME_CHARGE_POWER_PERCENT      0x14    /* 系统数据名： 功率百分比*/
//#define NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MAX        0x15    /* 系统数据名： 扩充最大电压*/
//#define NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MIN        0x16    /* 系统数据名： 扩充最小电压*/
//
///** operator name */
//#define NET_OPERATOR_NAME_CHINA_MOBILE                 0x00    /* 运营商名称：中国移动*/
//#define NET_OPERATOR_NAME_CHINA_TELECOM                0x01    /* 运营商名称：中国电信*/
//#define NET_OPERATOR_NAME_CHINA_UNICOM                 0x02    /* 运营商名称：中国联通*/
//
///** system data option */
//#define NET_SYSTEM_DATA_OPTION_DATA_CONTENT            (0x01 <<0x00)    /* 系统数据选项：数据内容(此位不置位则返回内容长度) */
//#define NET_SYSTEM_DATA_OPTION_PLAT_THA                (0x00 <<0x01)    /* 系统数据选项：钛享平台数据 */
//#define NET_SYSTEM_DATA_OPTION_PLAT_YKC                (0x01 <<0x01)    /* 系统数据选项：云快充平台数据 */
//#define NET_SYSTEM_DATA_OPTION_PLAT_XJ                 (0x02 <<0x01)    /* 系统数据选项：小桔平台数据 */
//#define NET_SYSTEM_DATA_OPTION_SELECT_ALL              (0x03 <<0x01)    /* 系统数据选项：选择所有 */
//#define NET_SYSTEM_DATA_OPTION_CARD_WHITELIST          (0x04 <<0x01)    /* 系统数据选项：卡白名单数据 */
//#define NET_SYSTEM_DATA_OPTION_VIN_WHITELIST           (0x05 <<0x01)    /* 系统数据选项：VIN码白名单数据 */
//#define NET_SYSTEM_DATA_OPTION_PLAT_RE3                (0x06 <<0x01)    /* 系统数据选项：保留平台数据 */
//#define NET_SYSTEM_DATA_OPTION_PLAT_RE4                (0x07 <<0x01)    /* 系统数据选项：保留平台数据 */

/***********************************************
 * 函数名     app_nget_system_data
 * 功能         获取系统数据
 **********************************************/
static uint8_t* app_nget_system_data(uint8_t name, uint32_t *dlen, uint32_t option)
{
    uint8_t is_content = (option &0x01), platform = 0x00;
    for(uint8_t bit = 0x01; bit < 0x08; bit++){
        if(option &(0x01 <<bit)){
            platform = bit;
        }
    }

    switch(name){
    case NET_SYSTEM_DATA_NAME_PILE_NUMBER:
        if(dlen){
            *dlen = strlen((char*)(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00)));
        }
        return sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_NETWORKED_WAY:

        break;
    case NET_SYSTEM_DATA_NAME_MCC:

        break;
    case NET_SYSTEM_DATA_NAME_MNC:

        break;
    case NET_SYSTEM_DATA_NAME_LAC:

        break;
    case NET_SYSTEM_DATA_NAME_CI:

        break;
    case NET_SYSTEM_DATA_NAME_IMEI:

        break;
    case NET_SYSTEM_DATA_NAME_RAM:

        break;
    case NET_SYSTEM_DATA_NAME_ROM:

        break;
    case NET_SYSTEM_DATA_NAME_MAC:

        break;
    case NET_SYSTEM_DATA_NAME_ICCID:
    {
        extern char *get_at_device_appinfo_iccid(void);
        return get_at_device_appinfo_iccid();
    }
        break;
    case NET_SYSTEM_DATA_NAME_IMSI:

        break;
    case NET_SYSTEM_DATA_NAME_OPERATOR:

        break;
    case NET_SYSTEM_DATA_NAME_DOMAIN:
        return sys_read_config_item_content(CONFIG_ITEM_IP_DOMAIN, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_PORT:
        return sys_read_config_item_content(CONFIG_ITEM_PORT, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_VOLTAGE_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_VOLTAGE_MIN:

        break;
    case NET_SYSTEM_DATA_NAME_CURRENT_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_MODULE_NUM:

        break;
    case NET_SYSTEM_DATA_NAME_SINGLE_MODULE_POWER:

        break;
    case NET_SYSTEM_DATA_NAME_SYSTEM_POWER_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MIN:

        break;
    case NET_SYSTEM_DATA_NAME_HARDWARE_VERSION:
    {
        extern uint8_t *__thaisen_get_test_hard_version(void);
        uint16_t ver_data = 0x00;

        sscanf(((char*)__thaisen_get_test_hard_version()), "ver:000%u", &ver_data);
        rt_kprintf("__thaisen_get_test_hard_version(%s) ver_data(%d)\n", __thaisen_get_test_hard_version(), ver_data);
        if(ver_data >= 7103){
            return "-V10";
        }else{
            return "-V01";
        }
    }
        break;
    default:
        break;
    }
}

/***********************************************
 * 函数名     app_nset_system_data
 * 功能         设置系统数据
 **********************************************/
static int32_t app_nset_system_data(uint8_t name, uint8_t *data, uint16_t len, uint32_t option)
{
    uint8_t is_content = (option &0x01), platform = 0x00;
    for(uint8_t bit = 0x01; bit < 0x08; bit++){
        if(option &(0x01 <<bit)){
            platform = bit;
        }
    }

    switch(name){
    case NET_SYSTEM_DATA_NAME_PILE_NUMBER:
//        if(dlen){
//            *dlen = strlen((char*)(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00)));
//        }
//        return sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_NETWORKED_WAY:

        break;
    case NET_SYSTEM_DATA_NAME_MCC:

        break;
    case NET_SYSTEM_DATA_NAME_MNC:

        break;
    case NET_SYSTEM_DATA_NAME_LAC:

        break;
    case NET_SYSTEM_DATA_NAME_CI:

        break;
    case NET_SYSTEM_DATA_NAME_IMEI:

        break;
    case NET_SYSTEM_DATA_NAME_RAM:

        break;
    case NET_SYSTEM_DATA_NAME_ROM:

        break;
    case NET_SYSTEM_DATA_NAME_MAC:

        break;
    case NET_SYSTEM_DATA_NAME_ICCID:

        break;
    case NET_SYSTEM_DATA_NAME_IMSI:

        break;
    case NET_SYSTEM_DATA_NAME_OPERATOR:

        break;
    case NET_SYSTEM_DATA_NAME_DOMAIN:
//        return sys_read_config_item_content(CONFIG_ITEM_IP_DOMAIN, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_PORT:
//        return sys_read_config_item_content(CONFIG_ITEM_PORT, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_VOLTAGE_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_VOLTAGE_MIN:

        break;
    case NET_SYSTEM_DATA_NAME_CURRENT_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_MODULE_NUM:

        break;
    case NET_SYSTEM_DATA_NAME_SINGLE_MODULE_POWER:

        break;
    case NET_SYSTEM_DATA_NAME_SYSTEM_POWER_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MAX:

        break;
    case NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MIN:

        break;
    case NET_SYSTEM_DATA_NAME_QRCODE:
        rt_kprintf("NET_SYSTEM_DATA_NAME_QRCODE qrcode config(%s)\n", data);
        if(sys_sync_config_item_content(CONFIG_ITEM_QRCODE_PRE, data, len) < 0x00){
            return -0x01;
        }
        return sys_storage_config_item();
    case NET_SYSTEM_DATA_NAME_HELP_PHONE:
        rt_kprintf("NET_SYSTEM_DATA_NAME_HELP_PHONE help phone config(%s)\n", data);
        if(sys_sync_config_item_content(CONFIG_ITEM_HELP_PHONE, data, len) < 0x00){
            return -0x01;
        }
        return sys_storage_config_item();
        break;
    default:
        break;
    }

    return -0x01;
}

/***********************************************
 * 函数名     app_nflash_erase
 * 功能         flash 擦除
 **********************************************/
static int32_t app_nflash_erase(uint32_t addr, uint32_t size)
{
    return mw_norflash_erase(addr, size);
}
/***********************************************
 * 函数名     app_nflash_read
 * 功能         flash 读
 **********************************************/
static int32_t app_nflash_read(uint32_t addr, uint8_t* data, uint32_t size)
{
    return mw_norflash_read(addr, data, size);
}
/***********************************************
 * 函数名     app_nflash_write
 * 功能         flash 带擦除写
 **********************************************/
static int32_t app_nflash_write(uint32_t addr, uint8_t* data, uint32_t size)
{
    return mw_norflash_write(addr, (const uint8_t*)data, size);
}
/***********************************************
 * 函数名     app_nflash_write_directly
 * 功能         flash 直接写
 **********************************************/
static int32_t app_nflash_write_directly(uint32_t addr, uint8_t* data, uint32_t size)
{
    return mw_norflash_write_directly(addr, (const uint8_t*)data, size);
}
/***********************************************
 * 函数名     app_ncard_vin_whitelists_set
 * 功能         VIN 卡白名单设置
 **********************************************/
static int32_t app_ncard_vin_whitelists_set(uint8_t* data, uint8_t len, uint32_t option)
{
    if(option &NET_SYSTEM_DATA_OPTION_CARD_WHITELIST){
        return sys_card_whitelists_add(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_VIN_WHITELIST){
        return sys_vin_whitelists_add(data, len);
    }
    return -0x01;
}

/***********************************************
 * 函数名     app_ncard_vin_whitelists_query
 * 功能         VIN 卡白名单查询
 **********************************************/
static int32_t app_ncard_vin_whitelists_query(uint8_t* data, uint8_t len, uint32_t option)
{
    if(option &NET_SYSTEM_DATA_OPTION_CARD_WHITELIST){
        return sys_card_whitelists_query(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_VIN_WHITELIST){
        return sys_vin_whitelists_query(data, len);
    }
    return -0x01;
}

/***********************************************
 * 函数名     app_ncard_vin_whitelists_delete
 * 功能         VIN 卡白名单删除
 **********************************************/
static int32_t app_ncard_vin_whitelists_delete(uint8_t* data, uint8_t len, uint32_t option)
{
    if(option &NET_SYSTEM_DATA_OPTION_CARD_WHITELIST){
        return sys_card_whitelists_delete(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_VIN_WHITELIST){
        return sys_vin_whitelists_delete(data, len);
    }
    return -0x01;
}

/***********************************************
 * 函数名     app_ncard_vin_whitelists_query
 * 功能         CRC32 校验
 **********************************************/
static uint32_t app_ncrc32_updtae(uint32_t init, const uint8_t *data, uint32_t len)
{
    extern uint32_t crc32_ieee(uint32_t crc, const uint8_t *data, uint32_t len);
    return crc32_ieee(init, data, len);
}

int32_t app_nfunc_config_init(void)
{
    struct net_handle* handle = net_get_net_handle();

    if(handle == NULL){
        return -0x01;
    }

    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_DATA_UPDATA,           app_ndata_update, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_TIME_SYNC,             app_ntime_sync, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_GET_BASE_DATA,         app_nget_base_data, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_GET_SYSTEM_DATA,       app_nget_system_data, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_SET_SYSTEM_DATA,       app_nset_system_data, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_FLASH_ERASE,           app_nflash_erase, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_FLASH_READ,            app_nflash_read, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_FLASH_WRITE,           app_nflash_write, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_FLASH_WRITE_DIRECTLY,  app_nflash_write_directly, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_SET_CARD_VIN,          app_ncard_vin_whitelists_set, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_QUERY_CARD_VIN,        app_ncard_vin_whitelists_query, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_DELETE_CARD_VIN,       app_ncard_vin_whitelists_delete, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_SYSTEM_DATA_STORAGE,   app_nsystem_data_storage, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_NDEV_OPERATE,          app_ndevice_operate, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_CRC32_UPDATE,          app_ncrc32_updtae, handle);

    handle->start_func(handle);
    return 0x00;
}

