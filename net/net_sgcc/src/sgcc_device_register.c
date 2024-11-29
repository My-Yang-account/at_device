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
    LOG_D("sgcc cert get");

    if(meta == NULL){
        return -0x01;
    }
    if(s_sgcc_storage_struct == NULL){
        return -0x01;
    }

    memset(meta->device_secret, 0x0, sizeof(meta->device_secret));
    memset(meta->device_name, 0x0, sizeof(meta->device_name));
    memset(meta->product_key, 0x0, sizeof(meta->product_key));

    if((strlen(s_sgcc_storage_struct->device_secret) <= 0x05) ||   \
            (strlen(s_sgcc_storage_struct->device_secret) > IOTX_DEVICE_SECRET_LEN)){
        LOG_W("sgcc device secret len invalid(%d)[%s]\n", strlen(s_sgcc_storage_struct->device_secret), s_sgcc_storage_struct->device_secret);
        return -0x01;

    }
    if((strlen(s_sgcc_storage_struct->device_name) <= 0x05) ||   \
            (strlen(s_sgcc_storage_struct->device_name) > IOTX_DEVICE_NAME_LEN)){
        LOG_W("sgcc device name len invalid(%d)[%s]\n", strlen(s_sgcc_storage_struct->device_name), s_sgcc_storage_struct->device_name);
        return -0x01;

    }
    if((strlen(s_sgcc_storage_struct->product_key) <= 0x05) ||   \
            (strlen(s_sgcc_storage_struct->product_key) > IOTX_PRODUCT_KEY_LEN)){
        LOG_W("sgcc product key len invalid(%d)[%s]\n", strlen(s_sgcc_storage_struct->product_key), s_sgcc_storage_struct->product_key);
        return -0x01;
    }

    memcpy(meta->device_secret, s_sgcc_storage_struct->device_secret, strlen(s_sgcc_storage_struct->device_secret));
    memcpy(meta->device_name, s_sgcc_storage_struct->device_name, strlen(s_sgcc_storage_struct->device_name));
    memcpy(meta->product_key, s_sgcc_storage_struct->product_key, strlen(s_sgcc_storage_struct->product_key));

    LOG_I("sgcc product key query : %s", meta->product_key);
    LOG_I("sgcc device name query : %s", meta->device_name);
    LOG_I("sgcc device secret query : %s", meta->device_secret);

    return 0x00;
}

int callback_service_EVS_CERT_SET(const evs_device_meta meta)
{
    LOG_D("sgcc cert set");

    if(s_sgcc_storage_struct == NULL){
        return -0x01;
    }
    struct net_handle* handle = net_get_net_handle();

    int len = 0x00, valid_len = 0x00;

    len = strlen(meta.product_key);
    valid_len = sizeof(s_sgcc_storage_struct->product_key);
    valid_len = valid_len > len ? len : valid_len;
    memset(s_sgcc_storage_struct->product_key, 0x0, sizeof(s_sgcc_storage_struct->product_key));
    memcpy(s_sgcc_storage_struct->product_key, meta.product_key, valid_len);

    len = strlen(meta.device_name);
    valid_len = sizeof(s_sgcc_storage_struct->device_name);
    valid_len = valid_len > len ? len : valid_len;
    memset(s_sgcc_storage_struct->device_name, 0x0, sizeof(s_sgcc_storage_struct->device_name));
    memcpy(s_sgcc_storage_struct->device_name, meta.device_name, valid_len);

    len = strlen(meta.device_secret);
    valid_len = sizeof(s_sgcc_storage_struct->device_secret);
    valid_len = valid_len > len ? len : valid_len;
    memset(s_sgcc_storage_struct->device_secret, 0x0, sizeof(s_sgcc_storage_struct->device_secret));
    memcpy(s_sgcc_storage_struct->device_secret, meta.device_secret, valid_len);

    LOG_I("sgcc product key set: %s", s_sgcc_storage_struct->product_key);
    LOG_I("sgcc device name set: %s", s_sgcc_storage_struct->device_name);
    LOG_I("sgcc device secret set: %s", s_sgcc_storage_struct->device_secret);

    if(handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
        s_sgcc_storage_struct->storage_init_flag = NET_SGCC_STORAGE_INIT_FLAG - 0x01;
        LOG_E("callback_service_EVS_CERT_SET storage fail");
        return -0x01;
    }

    return 0x00;
}

int callback_service_EVS_DEVICE_REG_CODE_GET(char *device_reg_code)
{
    if(s_sgcc_storage_struct == NULL){
        return -0x01;
    }
    if(device_reg_code == NULL){
        return -0x01;
    }

    int len = strlen(s_sgcc_storage_struct->device_reg_code);
    if((len < 0x05) || (len > IOTX_DEVICE_REG_CODE_LEN)){
        LOG_W("sgcc reg code len invalid(%d)[%s]\n", len, s_sgcc_storage_struct->device_reg_code);
        return len;
    }

    memset(device_reg_code, 0x0, IOTX_DEVICE_REG_CODE_LEN);
    memcpy(device_reg_code, s_sgcc_storage_struct->device_reg_code, len);

    return len;
}

int callback_service_EVS_DEVICE_UID_GET(char *device_uid)
{
    if(s_sgcc_storage_struct == NULL){
        return -1;
    }
    if(device_uid == NULL){
        return -0x01;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_SGCC |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    struct net_handle* handle = net_get_net_handle();
    char *pile_number = (char*)(handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    int len = strlen(pile_number);

    if(len > IOTX_DEVICE_UID_LEN){
        LOG_W("sgcc device uid len invalid(%d)[%s]\n", len, pile_number);
        return len;
    }

    memset(device_uid, 0x0, IOTX_DEVICE_UID_LEN);
    memcpy(device_uid, pile_number, len);

    LOG_I("sgcc uid query : %s", device_uid);

    return len;
}

void sgcc_device_register_init(void)
{
    EVS_RegisterCallback(EVS_CERT_GET, callback_service_EVS_CERT_GET);
    EVS_RegisterCallback(EVS_CERT_SET, callback_service_EVS_CERT_SET);
    EVS_RegisterCallback(EVS_DEVICE_REG_CODE_GET, callback_service_EVS_DEVICE_REG_CODE_GET);
    EVS_RegisterCallback(EVS_DEVICE_UID_GET, callback_service_EVS_DEVICE_UID_GET);
}

#endif /* NET_PACK_USING_SGCC */
