/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     leven       the first version
 */
#ifndef NET_NET_SGCC_INC_SGCC_DEVICE_REGISTER_H_
#define NET_NET_SGCC_INC_SGCC_DEVICE_REGISTER_H_

#include "protocol.h"

#ifdef NET_PACK_USING_SGCC


#define NET_SGCC_STORAGE_INIT_FLAG                              0x12345678  /* 平台数据存储标志 */

typedef enum{
    SGCC_WORKSTATE_IDLE = 10,                           /* 工作状态：空闲 */
    SGCC_WORKSTATE_INSERT = 11,                         /* 工作状态：插枪 */
    SGCC_WORKSTATE_STARTING = 12,                       /* 工作状态：启动中 */
    SGCC_WORKSTATE_CHARGINGING = 13,                    /* 工作状态：充电中 */
    SGCC_WORKSTATE_FINISH = 14,                         /* 工作状态：充电完成后未拔枪 */
    SGCC_WORKSTATE_APPOINTMENT = 15,                    /* 工作状态：预约状态 */
    SGCC_WORKSTATE_FAULTING = 16,                       /* 工作状态：系统故障(不能充电,故障状态下即使插上充电枪仍然反馈故障状态) */
    SGCC_WORKSTATE_FULL = 17,                           /* 工作状态：充满 */
    SGCC_WORKSTATE_PAUSE = 18,                          /* 工作状态：暂停 */
}sgcc_workstate;

typedef enum{
    SGCC_OPSCTL_ACTION = 10,                            /* 控制状态：动作 */
    SGCC_OPSCTL_SILENT = 11,                            /* 控制状态：未动作 */
    SGCC_OPSCTL_NONE = 12,                              /* 控制状态：无 */
}sgcc_opsctl;

typedef enum{
    SGCC_START_TYPE_APP_ONE_CLICK = 10,                 /* 启动类型：app 一键启动 */
    SGCC_START_TYPE_PLUG_AND_CHARGE = 11,               /* 启动类型：即插即充 */
    SGCC_START_TYPE_BLUE_INTELLIGENT = 12,              /* 启动类型：蓝牙离线(智能枪)启动 */
    SGCC_START_TYPE_QRCODE = 13,                        /* 启动类型：二维码启动 */
    SGCC_START_TYPE_SCAN_QRCODE = 14,                   /* 启动类型：桩侧扫码启动（桩自带识别器） */
    SGCC_START_TYPE_PLATFORM = 15,                      /* 启动类型：平台启动 */
    SGCC_START_TYPE_BLUE = 16,                          /* 启动类型：蓝牙启动 */
    SGCC_START_TYPE_OFFLINE_VIN = 17,                   /* 启动类型：离线VIN */
}sgcc_start_type;

#pragma pack(1)

/** 国网平台数据存储体 */
typedef struct{
    uint32_t storage_init_flag;                         /* 存储初始化标志 */
    uint8_t verify_result;                              /* 数据校验结果 */

    char product_key[IOTX_PRODUCT_KEY_LEN + 1];         /* 设备品类标识字符串 */
    char product_secret[IOTX_PRODUCT_SECRET_LEN + 1];   /* 设备品类密钥 */
    char device_name[IOTX_DEVICE_NAME_LEN + 1];         /* 某台设备的标识字符串:未注册前为设备出厂编号（16位长度），注册后为设备在物联管理平台的资产码（24位长度） */
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1];     /* 某台设备的设备密钥 */
    char device_reg_code[IOTX_DEVICE_REG_CODE_LEN + 1]; /* 某台设备的设备注册码 */
}sgcc_storage_struct;

#pragma pack()

void sgcc_register_storage_struct(void *storage_struct);

void sgcc_device_register_init(void);

/// 设置设备的UID（通常来说UID就是桩号）
/// @param device_uid UID
/// @return 设置的UID长度
int sgcc_device_uid_set(char *device_uid);

#endif /* NET_PACK_USING_SGCC */
#endif /* NET_NET_SGCC_INC_SGCC_DEVICE_REGISTER_H_ */
