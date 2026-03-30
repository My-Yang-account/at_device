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
#include "version.h"
#include "mw_norflash.h"
#include "mw_storage.h"

#include "chargepile_config.h"

#include "net_netdev.h"


/***********************************************
 * 函数名     app_net_occured_socket_close_passive
 * 功能         出现了socket 被动关闭
 **********************************************/
void app_net_occured_socket_close_passive(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_CLOSE_PASSIVE);
}

/***********************************************
 * 函数名     app_net_occured_socket_pdp_invalid
 * 功能         出现了socket PDP 场景失效
 **********************************************/
void app_net_occured_socket_pdp_invalid(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_SOCKET_PDP);
}

/***********************************************
 * 函数名     app_net_occured_close_communicate_module
 * 功能         出现了关闭通信模块
 **********************************************/
void app_net_occured_close_communicate_module(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_CLOSE_MODULE);
}

/***********************************************
 * 函数名     app_net_occured_at_physics_error
 * 功能         出现了通信模块物理层故障
 **********************************************/
void app_net_occured_at_physics_error(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_AT_PHYSICS);
}

/***********************************************
 * 函数名     app_net_occured_cpin_lk_mac_error
 * 功能         出现了通信模块数据链路故障(MAC CPIN)
 **********************************************/
void app_net_occured_cpin_lk_mac_error(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_CPIN_LK_MAC);
}

/***********************************************
 * 函数名     app_net_occured_cimi_lk_mac_error
 * 功能         出现了通信模块数据链路故障(LCC CIMI)
 **********************************************/
void app_net_occured_cimi_lk_mac_error(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_CIMI_LK_MAC);
}

/***********************************************
 * 函数名     app_net_occured_signal_strength_error
 * 功能         出现了通信模块查询信号强度失败
 **********************************************/
void app_net_occured_signal_strength_error(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_SIGNAL_STRENGTH_LK_MAC);
}

/***********************************************
 * 函数名     app_net_occured_gsm_registered_error
 * 功能         出现了通信模块GSM网络注册失败
 **********************************************/
void app_net_occured_gsm_registered_error(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_GSM_REGISTERED);
}

/***********************************************
 * 函数名     app_net_occured_gprs_registered_error
 * 功能         出现了通信模块GPRS网络注册失败
 **********************************************/
void app_net_occured_gprs_registered_error(int fd)
{
    net_dis_reason_store(fd, NET_DIS_REASON_GPRS_REGISTERED);
}

/***********************************************
 * 函数名     app_ndata_update
 * 功能         更新外部数据
 **********************************************/
static void app_ndata_update(void)
{
    extern void app_set_system_reset_event(uint8_t event, uint8_t state);
    uint8_t state = net_netdev_query_devstate(), err = 0x00;

    net_netdev_dev_control(NET_NETDEV_CTRL_CMD_QUERY_SYS_ERR, NULL, 0x00, &err, sizeof(err));
    if(err){
        app_set_system_reset_event(APP_SYS_RESET_NDEV_ERROR, 0x01);
    }else{
        app_set_system_reset_event(APP_SYS_RESET_NDEV_ERROR, 0x00);
    }

    switch(state){
    case NET_NETDEV_STATE_PHY:
        net_operation_set_net_fault(NET_FAULT_PHYSICAL_LAYER);
        break;
    case NET_NETDEV_STATE_DATA_LINK_MAC:
        net_operation_clear_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_set_net_fault(NET_FAULT_SIM_CARD);
        break;
    case NET_NETDEV_STATE_DATA_LINK_LCC:
        net_operation_clear_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_clear_net_fault(NET_FAULT_SIM_CARD);
        net_operation_set_net_fault(NET_FAULT_DATA_LINK_LAYER);
        break;
    case NET_NETDEV_STATE_NET_REGISTERED:
        net_operation_clear_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_clear_net_fault(NET_FAULT_SIM_CARD);
        net_operation_set_net_fault(NET_FAULT_DATA_LINK_LAYER);
        break;
    case NET_NETDEV_STATE_MODULE_INIT:
        net_operation_clear_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_clear_net_fault(NET_FAULT_SIM_CARD);
        net_operation_clear_net_fault(NET_FAULT_DATA_LINK_LAYER);
        net_operation_set_net_fault(NET_FAULT_MODULE_INIT);
        break;
    case NET_NETDEV_STATE_NORMAL:
        net_operation_clear_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_clear_net_fault(NET_FAULT_SIM_CARD);
        net_operation_clear_net_fault(NET_FAULT_DATA_LINK_LAYER);
        net_operation_clear_net_fault(NET_FAULT_MODULE_INIT);
        break;
    default:
        net_operation_set_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_set_net_fault(NET_FAULT_SIM_CARD);
        net_operation_set_net_fault(NET_FAULT_DATA_LINK_LAYER);
        net_operation_set_net_fault(NET_FAULT_MODULE_INIT);
        break;
    }

    /** 场景失效 */
    if(get_at_socket_deactivated()){
        net_operation_set_esock_state(NET_ESOCKET_STATE_CLOSE);
    }else{
        net_operation_set_esock_state(NET_ESOCKET_STATE_OPEN);
    }
}

/*********************************************************
 * 函数名     ethernet_init_hook
 * 功能         以太网初始化钩子函数
 ********************************************************/
void ethernet_init_hook(uint8_t complete, uint8_t success)
{
    if(success){
//        net_set_netdev_type(NET_NETDEV_TYPE_ETHERNET, 0x00);
    }else{
//        net_clear_netdev_type(NET_NETDEV_TYPE_ETHERNET);
    }

    if(complete){
        net_set_netdev_init_status(NET_NETDEV_TYPE_ETHERNET, 0x01);
    }else{
        net_set_netdev_init_status(NET_NETDEV_TYPE_ETHERNET, 0x00);
    }
}

/***********************************************
 * 函数名     app_ndevice_operate
 * 功能         操作网络设备
 **********************************************/
static int32_t app_ndevice_operate(void *para, uint32_t option)
{
    if(option &NET_DEV_OPERATE_OPTION_RESET){
        net_operation_set_net_fault(NET_FAULT_PHYSICAL_LAYER);
        net_operation_set_net_fault(NET_FAULT_SIM_CARD);
        net_operation_set_net_fault(NET_FAULT_DATA_LINK_LAYER);
        net_operation_set_net_fault(NET_FAULT_MODULE_INIT);

        return net_netdev_dev_control(NET_NETDEV_CTRL_CMD_RESET, NULL, 0x00, NULL, 0x00);
    }else if(option &NET_DEV_OPERATE_OPTION_CLOSE_SOCKET){
        if(para == NULL){
            return -0x01;
        }
        extern int sal_modify_socket_close(int fd);

        return sal_modify_socket_close(*((int32_t*)para));
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

/***********************************************
 * 函数名     app_nsystem_data_storage
 * 功能         系统数据存储
 **********************************************/
static int32_t app_nsystem_data_storage(uint32_t option)
{
    if(option &NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST){
        return sys_card_number_whitelists_storage();
    }else if(option &NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST){
        return sys_card_uid_whitelists_storage();
    }else if(option &NET_SYSTEM_DATA_OPTION_VIN_WHITELIST){
        return sys_vin_whitelists_storage();
    }else{
        return sys_storage_config_item();
    }
    return -0x01;
}

/***********************************************
 * 函数名     app_nsystem_control
 * 功能         系统控制
 **********************************************/
static int32_t app_nsystem_control(uint8_t gunno, uint16_t item, uint8_t *para, uint32_t option)
{
    switch(item){
    case NET_SYSTEM_CTRL_ITEM_ELOCK:
        switch(option){
        case NET_SYSTEM_CTRL_OPTION_ACTION:
            if(gunno == 0x00){
                if(thaisenElectLock() != thaisen_elect_lock_ok){
                    return -0x01;
                }
            }else{
                if(thaisenElectLockB() != thaisen_elect_lock_ok){
                    return -0x01;
                }
            }
            break;
        case NET_SYSTEM_CTRL_OPTION_RELEASE:
            if(gunno == 0x00){
                if(thaisenElectUnlock() != thaisen_elect_lock_ok){
                    return -0x01;
                }
            }else{
                if(thaisenElectUnlockB() != thaisen_elect_lock_ok){
                    return -0x01;
                }
            }
            break;
        case NET_SYSTEM_CTRL_OPTION_STATE:
            if(gunno == 0x00){
                if(thaisenGetElectLockStaA() == thaisen_elect_lock_ok){
                    return NET_SYSTEM_CTRL_STATE_ACTION;
                }else{
                    return NET_SYSTEM_CTRL_STATE_RELEASE;
                }
            }else{
                if(thaisenGetElectLockStaB() == thaisen_elect_lock_ok){
                    return NET_SYSTEM_CTRL_STATE_ACTION;
                }else{
                    return NET_SYSTEM_CTRL_STATE_RELEASE;
                }
            }
            break;
        default:
            break;
        }
    default:

        break;
    }
    return 0x00;
}

/***********************************************
 * 函数名     app_nget_system_data
 * 功能         获取系统数据
 **********************************************/
static uint8_t* app_nget_system_data(uint8_t name, void *vector, uint32_t vlen, uint32_t option)
{
    uint8_t is_content = (option &0x01), platform = 0x00;
    for(uint8_t bit = 0x01; bit < 0x08; bit++){
        if(option &(0x01 <<bit)){
            platform = bit;
        }
    }

    switch(name){
    case NET_SYSTEM_DATA_NAME_PILE_NUMBER:
        return sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_NETWORKED_WAY:
        if(net_query_netdev_type() == NET_NETDEV_TYPE_4G){
            return (uint8_t *)(NET_NETWORKED_WAY_4G);
        }else{
            return (uint8_t *)(NET_NETWORKED_WAY_ETH);
        }
        break;
#if 0
    case NET_SYSTEM_DATA_NAME_MCC:

        break;
    case NET_SYSTEM_DATA_NAME_MNC:

        break;
    case NET_SYSTEM_DATA_NAME_LAC:

        break;
    case NET_SYSTEM_DATA_NAME_CI:

        break;
    case NET_SYSTEM_DATA_NAME_RAM:

        break;
    case NET_SYSTEM_DATA_NAME_ROM:

        break;
    case NET_SYSTEM_DATA_NAME_MAC:

        break;
#endif /* 0 */
    case NET_SYSTEM_DATA_NAME_IMEI:
        if(vector){
            net_netdev_dev_control(NET_NETDEV_CTRL_CMD_QUERY_IMEI, NULL, 0x00, vector, vlen);
        }
        break;
    case NET_SYSTEM_DATA_NAME_ICCID:
        if(vector){
            net_netdev_dev_control(NET_NETDEV_CTRL_CMD_QUERY_SIM, NULL, 0x00, vector, vlen);
        }
        break;
#if 0
    case NET_SYSTEM_DATA_NAME_IMSI:

        break;
    case NET_SYSTEM_DATA_NAME_OPERATOR:

        break;
#endif /* 0 */
    case NET_SYSTEM_DATA_NAME_QRCODE:
        return sys_read_config_item_content(CONFIG_ITEM_QRCODE_PRE, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_DOMAIN:
        return sys_read_config_item_content(CONFIG_ITEM_IP_DOMAIN, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_PORT:
        return sys_read_config_item_content(CONFIG_ITEM_PORT, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_VOLTAGE_MAX:
        return sys_read_config_item_content(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_VOLTAGE_MIN:
        return sys_read_config_item_content(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0x00);
        break;
    case NET_SYSTEM_DATA_NAME_CURRENT_MAX:
        return sys_read_config_item_content(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0x00);
        break;
#if 0
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
#endif /* 0 */
    case NET_SYSTEM_DATA_NAME_HARDWARE_VERSION:
    {
        extern uint8_t *__thaisen_get_test_hard_version(void);
#ifdef APP_USING_DOUBLEGUN
        if(strstr((const char*)__thaisen_get_test_hard_version(), "7103D")){
            return "-V10";
        }else{
            return "-V01";
        }
#else
        if(strstr((const char*)__thaisen_get_test_hard_version(), "7101G")){
            return "-V10";
        }else{
            return "-V01";
        }
#endif /* APP_USING_DOUBLEGUN */
    }
        break;
    case NET_SYSTEM_DATA_NAME_HARDWARE_INFO:
    {
        extern uint8_t *__thaisen_get_test_hard_version(void);
        return __thaisen_get_test_hard_version();
    }
        break;
    case NET_SYSTEM_DATA_NAME_SIGNAL_STRENGTH:
        if(vector){
            net_netdev_dev_control(NET_NETDEV_CTRL_CMD_QUERY_STRENGTH, NULL, 0x00, vector, vlen);
        }
        break;
    case NET_SYSTEM_DATA_NAME_PLATFORM_DATA:
        if((option &NET_SYSTEM_DATA_OPTION_TARGET_PLAT)){
            return sys_read_config_item_content(CONFIG_ITEM_TARGET_PLATFORM, 0x00);
        }else if((option &NET_SYSTEM_DATA_OPTION_MONITOR_PLAT)){
            return sys_read_config_item_content(CONFIG_ITEM_MONITOR_PLATFORM, 0x00);
        }else if((option &NET_SYSTEM_DATA_OPTION_TARGET_PLAT_ADDITIONAL) != 0x00){
#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
            return sys_read_config_item_content(CONFIG_ITEM_TARGET_PLATFORM_ADDITIONAL, 0x00);
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */
        }
        return NULL;
        break;
    case NET_SYSTEM_DATA_NAME_AMMETER_ADDRESS:
    {
        uint8_t gunno = *((uint8_t*)vector);
        if(gunno == 0x00){
            return sys_read_config_item_content(CONFIG_ITEM_METER_NOA, 0x00);
        }else{
            return sys_read_config_item_content(CONFIG_ITEM_METER_NOB, 0x00);
        }
    }

        break;
    case NET_SYSTEM_DATA_NAME_SOFTWARE_MODEL:
        return (uint8_t*)(SOFTWARE_MODULE);
        break;
    case NET_SYSTEM_DATA_NAME_DEV_TYPE:
    {
        uint8_t dev_type = thaisenGetChargGunRunType();
        if(dev_type == thaisenDeviceType_singleGun){
            return (uint8_t *)(NET_DEV_TYPE_SINGLEGUN_TERMINAL);
        }else if(dev_type == thaisenDeviceType_average){
            return (uint8_t *)(NET_DEV_TYPE_DYNAMIC_DOUBLEGUN);
        }else if(dev_type == thaisenDeviceType_Rectifier_cabinet){
            return (uint8_t *)(NET_DEV_TYPE_MAIN_CABINET);
        }else{
            return (uint8_t *)(NET_DEV_TYPE_AVERAGE_DOUBLEGUN);
        }
    }
        break;
    case NET_SYSTEM_DATA_NAME_DEVICE_ID:
    {
        extern  uint8_t *__thaisen_get_test_number(void);
        return __thaisen_get_test_number();
    }
        break;
    default:
        break;
    }

    return NULL;
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
        return sys_sync_config_item_content(CONFIG_ITEM_PILE_NUMBER, data, len);
        break;
#if 0
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
#endif /* 0 */
    case NET_SYSTEM_DATA_NAME_DOMAIN:
        if(sys_sync_config_item_content(CONFIG_ITEM_IP_DOMAIN, data, len) < 0x00){
            return -0x01;
        }
        break;
    case NET_SYSTEM_DATA_NAME_PORT:
        if(sys_sync_config_item_content(CONFIG_ITEM_PORT, data, len) < 0x00){
            return -0x01;
        }
        break;
#if 0
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
#endif /* 0 */
    case NET_SYSTEM_DATA_NAME_QRCODE:
        if(sys_sync_config_item_content(CONFIG_ITEM_QRCODE_PRE, data, len) < 0x00){
            return -0x01;
        }
        return sys_storage_config_item();
    case NET_SYSTEM_DATA_NAME_HELP_PHONE:
        if(sys_sync_config_item_content(CONFIG_ITEM_HELP_PHONE, data, len) < 0x00){
            return -0x01;
        }
        return sys_storage_config_item();
        break;
    case NET_SYSTEM_DATA_NAME_PLATFORM_DATA:
        if((option &NET_SYSTEM_DATA_OPTION_TARGET_PLAT) || (option &NET_SYSTEM_DATA_OPTION_MONITOR_PLAT)){
            return sys_storage_config_item();
        }
#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
        else if((option &NET_SYSTEM_DATA_OPTION_TARGET_PLAT_ADDITIONAL)){
            return sys_storage_config_tp_additional_region();
        }
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */
        return -0x01;
        break;
    case NET_SYSTEM_DATA_NAME_TEMP_PROTECT_SWITCH:
    {
        uint8_t function = *data;
        if(function > 0x01){
            function = 0x01;
        }
        return sys_sync_config_item_content(CONFIG_ITEM_INEN_TEMPPRO, &function, sizeof(function));
    }
        break;
    case NET_SYSTEM_DATA_NAME_TEMP_PROTECT_STOP_VAL:
    {
        uint16_t value = *((uint16_t*)data);
        if((value > PROTECT_OVERTEMP_STOP_VALUE_MAX) || (value < PROTECT_OVERTEMP_STOP_VALUE_MIN)){
            value = PROTECT_OVERTEMP_STOP_VALUE_DEFAULT;
        }
        return sys_sync_config_item_content(CONFIG_ITEM_OVERTEMP_STOP, &value, sizeof(value));
    }
        break;
    case NET_SYSTEM_DATA_NAME_SCREEN_PW:
        return sys_sync_config_item_content(CONFIG_ITEM_SCREEN_PASSWORD, data, len);
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
    if(option &NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST){
        return sys_card_number_whitelists_add(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST){
        return sys_card_uid_whitelists_add(data, len);
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
    if(option &NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST){
        return sys_card_number_whitelists_query(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST){
        return sys_card_uid_whitelists_query(data, len);
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
    if(option &NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST){
        if(option &NET_SYSTEM_DATA_OPTION_CARD_UID_JOINT){
            uint8_t *card_uid = NULL;
            int32_t index = sys_card_number_whitelists_delete(data, len);

            if(index < 0x00){
                return index;
            }
            card_uid = sys_card_uid_get(index);
            if(card_uid == NULL){
                return -0x01;
            }
            return sys_card_uid_whitelists_delete(card_uid, len);
        }
        return sys_card_number_whitelists_delete(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST){
        if(option &NET_SYSTEM_DATA_OPTION_CARD_NUMBER_JOINT){
            uint8_t *card_number = NULL;
            int32_t index = sys_card_uid_whitelists_delete(data, len);

            if(index < 0x00){
                return index;
            }
            card_number = sys_card_number_get(index);
            if(card_number == NULL){
                return -0x01;
            }
            return sys_card_number_whitelists_delete(card_number, len);
        }
        return sys_card_uid_whitelists_delete(data, len);
    }else if(option &NET_SYSTEM_DATA_OPTION_VIN_WHITELIST){
        return sys_vin_whitelists_delete(data, len);
    }
    return -0x01;
}

/***********************************************
 * 函数名     app_nquery_system_record
 * 功能         查询指定时间段内的系统记录
 **********************************************/
static int32_t app_nquery_system_record(net_record_info_t *info)
{
    if(info->gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }

    uint8_t region = 0x00;
    int32_t index = 0x00;

    switch(info->option){
    case NET_SYSTEM_RECORD_OPTION_CHARGE:
        if(info->buf == NULL){
            return -0x01;
        }
        region = RECORD_REGION_CHARGE_RECORDA;
        if(info->gunno){
            region = RECORD_REGION_CHARGE_RECORDB;
        }
        index = mw_storage_record_query_findex_with_time_period(info->sindex, info->stime, info->etime, region);
        if(index < 0x00){
            return -0x04;
        }
        return mw_storage_record_get_designate_index_record((uint8_t*)(info->buf), info->len, region, index);
        break;
    case NET_SYSTEM_RECORD_OPTION_FAULT:
        if(info->buf == NULL){
            return -0x01;
        }
        region = RECORD_REGION_FAULT_RECORDA;
        if(info->gunno){
            region = RECORD_REGION_FAULT_RECORDB;
        }
        index = mw_storage_record_query_findex_with_time_period(info->sindex, info->stime, info->etime, region);
        if(index < 0x00){
            return -0x04;
        }
        return mw_storage_record_get_designate_index_record((uint8_t*)(info->buf), info->len, region, index);
        break;
    case NET_SYSTEM_RECORD_OPTION_CNUM:
        region = RECORD_REGION_CHARGE_RECORDA;
        if(info->gunno){
            region = RECORD_REGION_CHARGE_RECORDB;
        }
        return mw_storage_record_get_record_total_num(region);
        break;
    case NET_SYSTEM_RECORD_OPTION_FNUM:
        region = RECORD_REGION_FAULT_RECORDA;
        if(info->gunno){
            region = RECORD_REGION_FAULT_RECORDB;
        }
        return mw_storage_record_get_record_total_num(region);
        break;
    default:
        return -0x03;
        break;
    }
    return -0x04;
}

/***********************************************
 * 函数名     app_ncrc16_modbus
 * 功能         CRC16 校验
 **********************************************/
static uint16_t app_ncrc16_modbus(uint16_t init, const uint8_t *data, uint32_t len)
{
    extern uint16_t get_crc16_modbus(uint16_t crc, const uint8_t *data, uint32_t len);
    return get_crc16_modbus(init, data, len);
}

/***********************************************
 * 函数名     app_ncrc32_updtae
 * 功能         CRC32 校验
 **********************************************/
static uint32_t app_ncrc32_updtae(uint32_t init, const uint8_t *data, uint32_t len)
{
    extern uint32_t crc32_ieee(uint32_t crc, const uint8_t *data, uint32_t len);
    return crc32_ieee(init, data, len);
}

/***********************************************
 * 函数名     app_nthread_init_hook
 * 功能         网络模块线程初始化回调
 **********************************************/
static int32_t app_nthread_init_hook(void *thread, void *para, uint32_t plen, uint32_t option)
{
    extern int32_t app_thread_monitor_add(void *thread, void *para, uint32_t plen, uint32_t option);

    if(option &NET_THREAD_RUNNING_OPTION_ENTRY_MAX){
        app_thread_monitor_add(thread, para, plen, APP_THREAD_MONITOR_OPT_ENTRY);
    }else if(option &NET_THREAD_RUNNING_OPTION_NAME){
        app_thread_monitor_add(thread, para, plen, APP_THREAD_MONITOR_OPT_NAME);
    }

    return 0x00;
}

/***********************************************
 * 函数名     app_nthread_running
 * 功能         网络模块线程运行回调
 **********************************************/
static int32_t app_nthread_running(void *thread, void *para, uint32_t plen, uint32_t option)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);
    uint32_t _option = 0x00;

    if(option &NET_THREAD_RUNNING_OPTION_URGENT){
        _option |= APP_THREAD_MONITOR_OPT_URGENT;
    }
    app_thread_monitor_process(thread, NULL, 0x00, _option);

    return 0x00;
}


int32_t app_nfunc_config_init(void)
{
    struct net_handle* handle = net_get_net_handle();
    extern int32_t net_netdev_init(void);
    extern void ethch395_set_init_hook(void *hook);

    if(handle == NULL){
        return -0x01;
    }

#ifdef NET_DESIGNATE_REGION
    extern void net_operation_info_init(void);
    net_operation_info_init();
#endif /* NET_DESIGNATE_REGION */

    uint8_t nettype = CP_NETTYPE_4G, offline_billing = 0x00, plug_and_play = 0x00;

    nettype = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_NET_TYPE, 0x00));
    offline_billing = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, 0x00));
    plug_and_play = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_PLUGCHARGE, 0x00));

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
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_SYSTEM_CONTROL,        app_nsystem_control, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_QUERY_SYSTEM_RECORD,   app_nquery_system_record, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_NDEV_OPERATE,          app_ndevice_operate, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_CRC16_8005,            app_ncrc16_modbus, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_CRC32_UPDATE,          app_ncrc32_updtae, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_THREAD_INIT,           app_nthread_init_hook, handle);
    handle->para_config(0x00, NET_PARA_CONFIG_INDEX_THREAD_RUNNING,        app_nthread_running, handle);

    if((offline_billing == 0x01) || ((plug_and_play == 0x01) && (nettype == CP_NETTYPE_OFFLINE))){
        return -0x01;
    }

    ethch395_set_init_hook(ethernet_init_hook);
    if(nettype == CP_NETTYPE_ETH){
        ethch395_set_node_init_handle(app_nthread_init_hook);
        ethch395_set_node_running_handle(app_nthread_running);
        net_set_netdev_type(NET_NETDEV_TYPE_ETHERNET, 0x00);
        net_netdev_init();
    }

    handle->start_func(handle);

    return 0x00;
}


