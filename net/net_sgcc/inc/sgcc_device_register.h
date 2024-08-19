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

/** order verify result */
#define NET_SGCC_ORDER_VERIFY_RESULT_SUCCESS                    10          /* 订单确认结果：成功 */
#define NET_SGCC_ORDER_VERIFY_RESULT_DATA_ERROR                 11          /* 订单确认结果：数据错误 */
#define NET_SGCC_ORDER_VERIFY_RESULT_CHECK_ERROR                12          /* 订单确认结果：校验交易数据错误 */
#define NET_SGCC_ORDER_VERIFY_RESULT_MSN_NO_MATCH               13          /* 订单确认结果：模型编号不存在 */
#define NET_SGCC_ORDER_VERIFY_RESULT_SERVICE_ABNORMAL           14          /* 订单确认结果：服务异常 */
#define NET_SGCC_ORDER_VERIFY_RESULT_SYSTEM_ABNORMAL            15          /* 订单确认结果：系统异常 */

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
    SGCC_NETTYPE_NONE = 10,                            /* 网络类型：无 */
    SGCC_NETTYPE_2G = 11,                              /* 网络类型：2G */
    SGCC_NETTYPE_3G = 12,                              /* 网络类型：3G */
    SGCC_NETTYPE_4G = 13,                              /* 网络类型：4G */
    SGCC_NETTYPE_5G = 14,                              /* 网络类型：5G */
    SGCC_NETTYPE_NB_IOT = 15,                          /* 网络类型：NB-IOT */
    SGCC_NETTYPE_WIFI = 16,                            /* 网络类型：WIFI */
    SGCC_NETTYPE_WIRE = 17,                            /* 网络类型：有线网络 */
}sgcc_nettype;

typedef enum{
    SGCC_OPERATOR_NONE = 10,                           /* 网络运营商：无 */
    SGCC_OPERATOR_UNICOM = 11,                         /* 网络运营商：联通 */
    SGCC_OPERATOR_MOBILE = 12,                         /* 网络运营商：移动 */
    SGCC_OPERATOR_TELECOM = 13,                        /* 网络运营商：电信 */
    SGCC_OPERATOR_OTHER = 14,                          /* 网络运营商：其它 */
}sgcc_operator;

typedef enum{
    SGCC_OPSCTL_ACTION = 10,                            /* 控制状态：动作 */
    SGCC_OPSCTL_SILENT = 11,                            /* 控制状态：未动作 */
    SGCC_OPSCTL_NONE = 12,                              /* 控制状态：无 */
}sgcc_opsctl;

typedef enum{
    SGCC_ELOCK_CTRL_SUCCESS = 10,                       /* 电子锁控制结果：成功 */
    SGCC_ELOCK_CTRL_FAIL_FORBID_UNLOCK = 11,            /* 电子锁控制结果：设置失败，当前状态不允许解锁 */
    SGCC_ELOCK_CTRL_FAIL_FORBID_LOCK = 12,              /* 电子锁控制结果：设置失败，当前状态不允许锁定 */
    SGCC_ELOCK_CTRL_NO_ELOCK = 12,                      /* 电子锁控制结果：无电子锁 */
}sgcc_elock_ctrl;

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

typedef enum{
    SGCC_CHARGE_MODE_FULL = 10,                         /* 充电模式：不做限制的充电(充满) */
    SGCC_CHARGE_MODE_MONEY = 11,                        /* 充电模式：限制金额 */
    SGCC_CHARGE_MODE_ELECT = 12,                        /* 充电模式：限制电量 */
    SGCC_CHARGE_MODE_SOC = 13,                          /* 充电模式：限制 SOC*/
    SGCC_CHARGE_MODE_TIME = 14,                         /* 充电模式：限制充电时长 */
    SGCC_CHARGE_MODE_POWER = 15,                        /* 充电模式：限制功率 */
}sgcc_charge_mode;

typedef enum{
    SGCC_DEV_STATE_REBOOT = 11,                         /* 设备状态：重启 */
    SGCC_DEV_STATE_OVERHAUL = 12,                       /* 设备状态：检修 */
    SGCC_DEV_STATE_FREEZE = 13,                         /* 设备状态：冻结 */
    SGCC_DEV_STATE_COMMISSIONING = 14,                  /* 设备状态：投运 */
    SGCC_DEV_STATE_OUTAGE = 15,                         /* 设备状态：停运 */
    SGCC_DEV_STATE_RETURNS = 16,                        /* 设备状态：退启 */
    SGCC_DEV_STATE_RESTORE_SRTTING = 17,                /* 设备状态：恢复出厂设置 */
}sgcc_dev_state;

/** GS as general stop (常规停止)*/
//enum{
//
//};

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

    uint32_t rt_property_interval;                      /* 充电设备实时监测属性上报频率(单位：秒) */
    uint32_t charging_property_interval;                /* 充电枪充电中实时监测属性上报频率(单位：秒) */
    uint32_t noncharging_property_interval;             /* 充电枪非充电中实时监测属性上报频率(单位：秒) */
    uint32_t fault_warning_interval;                    /* 故障告警全信息上传频率(单位：秒) */
    uint32_t ac_meter_interval;                         /* 充电设备交流电表底值监测属性上报频率(单位：分钟) */
    uint32_t dc_meter_interval;                         /* 直流输出电表底值监测属性上报频率(单位：分钟) */
    uint32_t offline_charge_time;                       /* 离线后可充电时长(单位：分钟) */
    uint32_t groundlock_interval;                       /* 地锁监测上送频率(单位：分钟) */
    uint32_t doorlock_interval;                         /* 网门锁监测上送频率(单位：分钟) */
    uint32_t encode_con;                                /* 报文加密 */
    uint8_t dev_state;                                  /* 设备状态 */
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
