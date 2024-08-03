/*

 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     leven       the first version
 */

#include "net_operation.h"

#include "sgcc_device_register.h"
#include "interface.h"
#include "protocol.h"

#define DBG_TAG "register"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_SGCC

static sgcc_storage_struct *s_sgcc_storage_struct = NULL;

void sgcc_register_storage_struct(void *storage_struct)
{
    if(storage_struct != NULL){
        s_sgcc_storage_struct = (sgcc_storage_struct*)storage_struct;
    }
}

int callback_service_EVS_CERT_GET(evs_device_meta *meta)
{
    LOG_D("cert get");

    if(s_sgcc_storage_struct == NULL){
        return -1;
    }

    int len = strlen(s_sgcc_storage_struct->product_key);
    if(meta == NULL){
        return -1;
    }
    if (len < 5){
        return -1;
    }
    memset(meta->product_key, 0x0, sizeof(meta->product_key));
    strncpy(meta->product_key, s_sgcc_storage_struct->product_key, len);
    meta->product_key[len] = '\0';

    len = strlen(s_sgcc_storage_struct->device_name);
    if (len < 2){
        return -1;
    }
    memset(meta->device_name, 0x0, sizeof(meta->device_name));
    strncpy(meta->device_name, s_sgcc_storage_struct->device_name, len);
    meta->device_name[len] = '\0';

    len = strlen(s_sgcc_storage_struct->device_secret);
    if (len < 5){
        return -1;
    }
    memset(meta->device_secret, 0x0, sizeof(meta->device_secret));
    strncpy(meta->device_secret, s_sgcc_storage_struct->device_secret, len);
    meta->device_secret[len] = '\0';

    LOG_I("product key get: %s", meta->product_key);
    LOG_I("device name get: %s", meta->device_name);
    LOG_I("device secret get: %s", meta->device_secret);

    return 0;
}

int callback_service_EVS_CERT_SET(const evs_device_meta meta)
{
    LOG_D("cert set");

    if(s_sgcc_storage_struct == NULL){
        return -1;
    }
    struct net_handle* handle = net_get_net_handle();

    int len = strlen(meta.product_key);
    int valid_len = sizeof(s_sgcc_storage_struct->product_key);

    valid_len = valid_len > len ? len : valid_len;
    memset(s_sgcc_storage_struct->product_key, 0x0, sizeof(s_sgcc_storage_struct->product_key));
    strncpy(s_sgcc_storage_struct->product_key, meta.product_key, valid_len);
    s_sgcc_storage_struct->product_key[valid_len] = '\0';

    len = strlen(meta.device_name);
    valid_len = valid_len > len ? len : valid_len;
    memset(s_sgcc_storage_struct->device_name, 0x0, sizeof(s_sgcc_storage_struct->device_name));
    strncpy(s_sgcc_storage_struct->device_name, meta.device_name, valid_len);
    s_sgcc_storage_struct->device_name[valid_len] = '\0';

    len = strlen(meta.device_secret);
    valid_len = valid_len > len ? len : valid_len;
    memset(s_sgcc_storage_struct->device_secret, 0x0, sizeof(s_sgcc_storage_struct->device_secret));
    strncpy(s_sgcc_storage_struct->device_secret, meta.device_secret, valid_len);
    s_sgcc_storage_struct->device_secret[valid_len] = '\0';

    LOG_I("product key set: %s", s_sgcc_storage_struct->product_key);
    LOG_I("device name set: %s", s_sgcc_storage_struct->device_name);
    LOG_I("device secret set: %s", s_sgcc_storage_struct->device_secret);

    if(handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
        s_sgcc_storage_struct->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
        LOG_E("callback_service_EVS_CERT_SET storage fail");
        return -0x01;
    }

    return 0;
}

int callback_service_EVS_DEVICE_REG_CODE_GET(char *device_reg_code)
{
    if(s_sgcc_storage_struct == NULL){
        return -1;
    }
    int len = strlen(s_sgcc_storage_struct->device_reg_code);
    memset(device_reg_code, 0x0, IOTX_DEVICE_REG_CODE_LEN);
    strncpy(device_reg_code, s_sgcc_storage_struct->device_reg_code, IOTX_DEVICE_REG_CODE_LEN);
    device_reg_code[len] = '\0';

    LOG_I("reg code get: %s", device_reg_code);

    return strlen(s_sgcc_storage_struct->device_reg_code);
}

int callback_service_EVS_DEVICE_UID_GET(char *device_uid)
{
    if(s_sgcc_storage_struct == NULL){
        return -1;
    }
    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_GW |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, option));
    int len = strlen(pile_number);

    memset(device_uid, 0x0, IOTX_DEVICE_UID_LEN);
    strncpy(device_uid, pile_number, IOTX_DEVICE_UID_LEN);
    device_uid[len] = '\0';

    LOG_I("uid get: %s", device_uid);

    return strlen(pile_number);
}

void sgcc_device_register_init(void)
{
    EVS_RegisterCallback(EVS_CERT_GET, callback_service_EVS_CERT_GET);
    EVS_RegisterCallback(EVS_CERT_SET, callback_service_EVS_CERT_SET);
    EVS_RegisterCallback(EVS_DEVICE_REG_CODE_GET, callback_service_EVS_DEVICE_REG_CODE_GET);
    EVS_RegisterCallback(EVS_DEVICE_UID_GET, callback_service_EVS_DEVICE_UID_GET);
}

int sgcc_device_uid_set(char *device_uid)
{
    if(s_sgcc_storage_struct == NULL){
        return -1;
    }
    uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_GW |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    int len = strlen(device_uid);

    LOG_I("uid set: %s[%d]", device_uid, len);

    if(handle->set_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, (uint8_t*)device_uid, len, option) >= 0x00){
        LOG_E("sgcc_device_uid_set storage fail");
        return -0x01;
    }

    return strlen(device_uid);
}

#endif /* NET_PACK_USING_SGCC */
