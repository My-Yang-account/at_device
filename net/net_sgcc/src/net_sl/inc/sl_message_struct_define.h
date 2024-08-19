/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-03     我的杨yang       the first version
 */
#ifndef NET_NET_SL_INC_SL_MESSAGE_STRUCT_DEFINE_H_
#define NET_NET_SL_INC_SL_MESSAGE_STRUCT_DEFINE_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_SL

#define NET_SL_DEVICE_SN_LENGTH_MAX                          0x20        /* 设备SN最大长度 */
#define NET_SL_STANDAR_TIME_LENGTH_MAX                       0x08        /* 表示标准时间的最大长度 */
#define NET_SL_SERIAL_NUMBER_LENGTH_MAX                      0x20        /* 流水号最大长度 */
#define NET_SL_SOFTWARE_VERSION_LENGTH_MAX                   0x14        /* 软件版本最大长度 */
#define NET_SL_HARDWARE_VERSION_LENGTH_MAX                   0x08        /* 硬件版本最大长度 */
#define NET_SL_CARD_NUMBER_LENGTH_MAX                        0x20        /* 充电卡号最大长度 */
#define NET_SL_USER_NUMBER_LENGTH_MAX                        0x20        /* 用户码最大长度 */
#define NET_SL_DEVICE_NAME_LENGTH_MAX                        0x14        /* 设备名称最大长度 */
#define NET_SL_PROTOCOL_VERSION_LENGTH_MAX                   0x08        /* 协议版本最大长度 */
#define NET_SL_CARD_VIN_LENGTH_MAX                           0x11        /* VIN码最大长度 */
#define NET_SL_LICENSE_PLATE_NUMBER_LENGTH_MAX               0x08        /* 车牌号最大长度 */
#define NET_SL_SINGLE_TIME_CHARGE_PERIOD_MAX                 0x30        /* 单次充电时段最大数 */
#define NET_SL_MODULE_SN_LENGTH_MAX                          0x10        /* 模块SN最大长度 */
#define NET_SL_SINGLE_BATTERY_NUMBER_MAX                     0xFF        /* 单体电池最大数量 */
#define NET_SL_BATTERY_TEMP_NUMBER_MAX                       0x80        /* 电池温度最大数量 */
#define NET_SL_LOG_REPROT_PATH_LENGTH_MAX                    0x40        /* 本地日志文件路径最大长度 */
#define NET_SL_LOG_REPORT_SERVER_ADDR_MAX                    0x80        /* 服务器日志传输地址 最大长度 */
#define NET_SL_SERVER_LOG_CATALOG_LENGTH_MAX                 0x40        /* 服务器日志传输目录最大长度 */
#define NET_SL_HISTORY_CHARGE_RECORD_SN_LENGTH_MAX           0x20        /* 历史充电记录编号最大长度 */
#define NET_SL_PERIOD_COUNT_MAX                              0x30        /* 时段数最大值 */
#define NET_SL_UPDATE_FILE_NAME_LENGTH_MAX                   0x20        /* 升级文件名最大长度 */
#define NET_SL_DEVICE_FEATURES_LENGTH_MAX                    0x20        /* 设备特征码最大长度 */
#define NET_SL_FTP_UPDATE_SERVER_IP_LENGTH_MAX               0x10        /* FTP 服务器 IP 地址最大长度 */
#define NET_SL_FTP_UPDATE_USER_NAME_LENGTH_MAX               0x20        /* FTP 服务器用户名最大长度 */
#define NET_SL_FTP_UPDATE_PASSWORD_LENGTH_MAX                0x20        /* FTP 服务器密码最大长度 */
#define NET_SL_FTP_UPDATE_PATH_LENGTH_MAX                    0x20        /* 升级文件路径最大长度 */
#define NET_SL_GATEWAY_SN_LENGTH_MAX                         0x20        /* 网关编码最大长度 */
#define NET_SL_DEVICE_NAME_GATEWAY_LENGTH_MAX                0x10        /* 设备名称(网关签到)最大长度 */
#define NET_SL_DEVICE_MODEL_LENGTH_MAX                       0x10        /* 设备型号最大长度 */
#define NET_SL_SIM_ICCID_LENGTH_MAX                          0x14        /* SIM卡ICCID号最大长度 */
#define NET_SL_DEVICE_NODE_SN_LENGTH_MAX                     0x20        /* 设备节点SN最大长度 */
#define NET_SL_DEVICE_NODE_NUM_MAX                           0x08        /* 设备节点数最大值 */
#define NET_SL_SINGLE_INT_PARA_NUM_MAX                       0x0A        /* 单次可设置整型参数个数最大值 */
#define NET_SL_SINGLE_STRING_PARA_LENGTH_MAX                 0xFF        /* 单次设置字符型参数长度最大值 */

#define SLPRO_STARTCODE                             0xF5AA             /* 起始码 */
#define SLPRO_COMMUNICATION_VERSION                 0x03               /* 通信协议版本：V3.0.0 */
#define SLPRO_TYPE                                  0x3F               /* 默认-0x3F(限阳光乐充平台使用) */
#define SLVERSION_SN                                0x01               /* 默认-0x01(限阳光乐充平台使用) */

#define COMMUNICATE_WAY_ETHERNET                    0x01               /* 通讯方式：以太网 */
#define COMMUNICATE_WAY_WIFI                        0x02               /* 通讯方式：WiFi */
#define COMMUNICATE_WAY_GPRS                        0x03               /* 通讯方式：GPRS */
#define COMMUNICATE_WAY_SUB_1G                      0x04               /* 通讯方式：Sub_1G */
#define COMMUNICATE_WAY_ZIGBEE                      0x05               /* 通讯方式：ZigBee */
#define COMMUNICATE_WAY_BLUETOOTH                   0x06               /* 通讯方式：Bluetooth */
#define COMMUNICATE_WAY_CAN                         0x07               /* 通讯方式：CAN */
#define COMMUNICATE_WAY_RS485                       0x08               /* 通讯方式：WAY_RS485 */
#define COMMUNICATE_WAY_4G                          0x09               /* 通讯方式：4G */

/* net event */
/**=======================================【充电桩请求事件】================================================*/
/**=======================================【充电桩请求事件】================================================*/
#define SLNET_PREQEVENT_REPORT_HEARTBEAT                               (1 <<0)           /* 充电桩上传心跳包信息 */
#define SLNET_PREQEVENT_REPORT_STATE_INFO                              (1 <<1)           /* 充电桩状态信息包上报 */
#define SLNET_PREQEVENT_REPORT_SIGNIN_INFO                             (1 <<2)           /* 充电桩签到信息上报 */
#define SLNET_PREQEVENT_REPORT_CHARGEINFO_AFTER_CHARGEEND              (1 <<3)           /* 充电桩上报最新一次充电信息(停充后订单上报) */
#define SLNET_PREQEVENT_REPORT_USERCARD_INFO                           (1 <<4)           /* 充电桩上传用户卡信息 */
#define SLNET_PREQEVENT_REPORT_BMS_INFO                                (1 <<5)           /* 充电桩上报 BMS 信息 */
#define SLNET_PREQEVENT_REPORT_BMS_SINGLEBATTERY_VOLTAGE               (1 <<6)           /* 充电桩上报 BMS 每节电池电压信息 */
#define SLNET_PREQEVENT_REPORT_BMS_BATTERY_TEMPERATURE                 (1 <<7)           /* 充电桩上报 BMS 动力蓄电池温度信息 */
#define SLNET_PREQEVENT_REPORT_UPDATE_STATE                            (1 <<8)           /* 充电桩升级状态上报 */
#define SLNET_PREQEVENT_GATEWAY_SIGN_IN                                (1 <<9)           /* 网关签到 */
#define SLNET_PREQEVENT_GATEWAY_REPORT_HEARTBEAT                       (1 <<10)          /* 网关上传心跳包信息 */
#define SLNET_PREQEVENT_GATEWAY_REPORT_DEBUG_LOG                       (1 <<11)          /* 网关上传 log 调试信息 */
#define SLNET_PREQEVENT_GATEWAY_REPORT_MESHNODE_STRUCT                 (1 <<12)          /* 网关（根节点）上传 MESH 节点结构 */
#define SLNET_PREQEVENT_REPORT_EVENT_INFO                              (1 <<13)          /* 充电桩上传事件信息 */
/**=======================================【充电桩响应事件】================================================*/
/**=======================================【充电桩响应事件】================================================*/
#define SLNET_PRESEVENT_SET_INT_WORKPARA                               (1 <<0)           /* 充电桩整形参数设置/查询应答 */
#define SLNET_PRESEVENT_SET_STRING_WORKPARA                            (1 <<1)           /* 充电桩整形参数设置/查询应答 */
#define SLNET_PRESEVENT_ISSUE_CONTROL_COMMAND                          (1 <<2)           /* 充电桩对后台控制命令应答 */
#define SLNET_PRESEVENT_START_CHARGE_COMMAND                           (1 <<3)           /* 充电桩对后台下发的充电桩开启充电控制应答 */
#define SLNET_PRESEVENT_QUERY_CHARGEPILE_VERSION                       (1 <<4)           /* 充电桩版本信息应答 */
#define SLNET_PRESEVENT_REPORT_DEBUG_LOG                               (1 <<5)           /* 充电桩回应上传 log 调试消息 */
#define SLNET_PRESEVENT_QUERY_HISTORY_CHARGE_RECORD                    (1 <<6)           /* 充电桩上报历史的充电记录 */
#define SLNET_PRESEVENT_SOFTWARE_UPDATE                                (1 <<7)           /* 充电桩回复服务器升级 */
#define SLNET_PRESEVENT_ISSUE_BILLING_RULE                             (1 <<8)           /* 充电桩响应后台电费计价策略信息 */
#define SLNET_PRESEVENT_REPORT_MODULE_PARA_INFO                        (1 <<9)           /* 充电桩上传模块参数信息 */
#define SLNET_PRESEVENT_QUERY_MODULE_STATE                             (1 <<10)          /* 充电桩上传模块状态信息 */
#define SLNET_PRESEVENT_ISSUE_CHARGEPILE_REPORT_EVENT_CMD              (1 <<11)          /* 充电桩响应服务器下发参数命令 */
/**=======================================【服务器请求事件】================================================*/
/**=======================================【服务器请求事件】================================================*/
#define SLNET_SREQEVENT_SET_INT_WORKPARA                               (1 <<0)           /* 后台服务器向充电桩下发充电桩整形工作参数设置/查询 */
#define SLNET_SREQEVENT_SET_STRING_WORKPARA                            (1 <<1)           /* 后台服务器设置/查询充电桩字符型工作参数 */
#define SLNET_SREQEVENT_ISSUE_CONTROL_COMMAND                          (1 <<2)           /* 后台服务器下发充电桩电控制命令 */
#define SLNET_SREQEVENT_START_CHARGE_COMMAND                           (1 <<3)           /* 后台服务器下发充电桩开启充电控制命令 */
#define SLNET_SREQEVENT_QUERY_CHARGEPILE_VERSION                       (1 <<4)           /* 服务器查询电桩版本命令 */
#define SLNET_SREQEVENT_REPORT_DEBUG_LOG                               (1 <<5)           /* 服务器下发充电桩上传 log 调试信息 */
#define SLNET_SREQEVENT_QUERY_HISTORY_CHARGE_RECORD                    (1 <<6)           /* 服务器查询充电桩历史充电记录 */
#define SLNET_SREQEVENT_SOFTWARE_UPDATE                                (1 <<7)           /* 服务器下发升级指令 */
#define SLNET_SREQEVENT_ISSUE_BILLING_RULE                             (1 <<8)           /* 后台下发电费计价策略信息 */
#define SLNET_SREQEVENT_REPORT_MODULE_PARA_INFO                        (1 <<9)           /* 服务器下发模块参数命令 */
#define SLNET_SREQEVENT_QUERY_MODULE_STATE                             (1 <<10)          /* 服务器下发查询模块状态命令 */
#define SLNET_SREQEVENT_ISSUE_CHARGEPILE_REPORT_EVENT_CMD              (1 <<11)          /* 服务器下发充电桩上传事件命令 */

#define SLNET_SREQEVENT_ISSUE_STOP_CHARGE_COMMAND                      (1 <<12)          /* 服务器下发停止充电命令 */
/**=======================================【服务器响应事件】================================================*/
/**=======================================【服务器响应事件】================================================*/
#define SLNET_SRESEVENT_REPORT_HEARTBEAT                               (1 <<0)           /* 服务器应答心跳包信息 */
#define SLNET_SRESEVENT_REPORT_STATE_INFO                              (1 <<1)           /* 服务器应答充电桩状态信息包 */
#define SLNET_SRESEVENT_REPORT_SIGNIN_INFO                             (1 <<2)           /* 服务器应答充电桩签到命令 */
#define SLNET_SRESEVENT_REPORT_CHARGEINFO_AFTER_CHARGEEND              (1 <<3)           /* 服务器响应充电桩上报最新一次充电信息(停充后订单上报) */
#define SLNET_SRESEVENT_REPORT_USERCARD_INFO                           (1 <<4)           /* 服务器应答用户卡查询信息 */
#define SLNET_SRESEVENT_REPORT_BMS_INFO                                (1 <<5)           /* 服务器应答充电桩上报BMS信息 */
#define SLNET_SRESEVENT_REPORT_BMS_SINGLEBATTERY_VOLTAGE               (1 <<6)           /* 服务器应答充电桩 BMS 每节电池电压信息 */
#define SLNET_SRESEVENT_REPORT_BMS_BATTERY_TEMPERATURE                 (1 <<7)           /* 服务器应答充电桩 BMS 动力蓄电池温度信息 */
#define SLNET_SRESEVENT_REPORT_UPDATE_STATE                            (1 <<8)           /* 服务器回应升级状态上报 */
#define SLNET_SRESEVENT_GATEWAY_SIGN_IN                                (1 <<9)           /* 服务器应答网关签到 */
#define SLNET_SRESEVENT_GATEWAY_REPORT_HEARTBEAT                       (1 <<10)          /* 服务器应答网关心跳包信息 */
#define SLNET_SRESEVENT_GATEWAY_REPORT_DEBUG_LOG                       (1 <<11)          /* 服务器回应上传 log 调试消息 */
#define SLNET_SRESEVENT_GATEWAY_REPORT_MESHNODE_STRUCT                 (1 <<12)          /* 服务器回应上传 MESH 节点结构 */
#define SLNET_SRESEVENT_REPORT_EVENT_INFO                              (1 <<13)          /* 服务器响应充电桩上传事件 */
#define SLNET_SRESEVENT_REPORT_GATEWAY_SIGNIN_INFO                     (1 <<14)          /* 服务器应答网关签到命令 */

enum sl_cmd{
    NETSL_SREQCMD_SET_INT_WORKPARA = 0x01,                                /* 后台服务器向充电桩下发充电桩整形工作参数设置/查询命令指令 */
    NETSL_PRESCMD_SET_INT_WORKPARA = 0x02,                                /* 充电桩整形参数设置/查询应答指令 */

    NETSL_SREQCMD_SET_STRING_WORKPARA = 0x03,                             /* 后台服务器设置/查询充电桩字符型工作参数指令 */
    NETSL_PRESCMD_SET_STRING_WORKPARA = 0x04,                             /* 充电桩字符型参数设置/查询应答指令 */

    NETSL_SREQCMD_ISSUE_CONTROL_COMMAND = 0x05,                           /* 后台服务器下发控制命令指令 */
    NETSL_PRESCMD_ISSUE_CONTROL_COMMAND = 0x06,                           /* 客户端对后台控制命令应答指令 */

    NETSL_SREQCMD_START_CHARGE_COMMAND = 0x07,                            /* 后台服务器下发充电桩开启充电控制命令指令 */
    NETSL_PRESCMD_START_CHARGE_COMMAND = 0x08,                            /* 充电桩对后台下发的充电桩开启充电控制应答指令 */

    NETSL_SREQCMD_QUERY_CHARGEPILE_VERSION = 0x09,                        /* 服务器查询电桩版本命令指令 */
    NETSL_PRESCMD_QUERY_CHARGEPILE_VERSION = 0x0A,                        /* 充电桩版本信息应答指令 */

    NETSL_SRESCMD_REPORT_HEARTBEAT = 0x65,                                /* 服务器应答心跳包信息指令 */
    NETSL_PREQCMD_REPORT_HEARTBEAT = 0x66,                                /* 充电桩上传心跳包信息指令 */

    NETSL_SRESCMD_REPORT_STATE_INFO = 0x67,                               /* 服务器应答充电桩状态信息包 指令 */
    NETSL_PREQCMD_REPORT_STATE_INFO = 0x68,                               /* 充电桩状态信息包上报指令 */

    NETSL_SRESCMD_REPORT_SIGNIN_INFO = 0x69,                              /* 服务器应答充电桩签到命令指令 */
    NETSL_PREQCMD_REPORT_SIGNIN_INFO = 0x6A,                              /* 充电桩签到信息上报指令 */

    NETSL_SRESCMD_REPORT_CHARGEINFO_AFTER_CHARGEEND = 0xC9,               /* 服务器响应充电桩上报最新一次充电信息(停充后订单上报)指令 */
    NETSL_PREQCMD_REPORT_CHARGEINFO_AFTER_CHARGEEND = 0xCA,               /* 充电桩上报最新一次充电信息(停充后订单上报)指令 */

    NETSL_SRESCMD_REPORT_USERCARD_INFO = 0xCB,                            /* 服务器应答用户卡查询信息指令 */
    NETSL_PREQCMD_REPORT_USERCARD_INFO = 0xCC,                            /* 充电桩上传用户卡信息指令 */

    NETSL_SRESCMD_REPORT_BMS_INFO = 0x12D,                                /* 服务器应答充电桩上报BMS信息指令 */
    NETSL_PREQCMD_REPORT_BMS_INFO = 0x12E,                                /* 充电桩上报 BMS 信息指令 */

    NETSL_SRESCMD_REPORT_BMS_SINGLEBATTERY_VOLTAGE = 0x12F,               /* 服务器应答充电桩 BMS 每节电池电压信息指令 */
    NETSL_PREQCMD_REPORT_BMS_SINGLEBATTERY_VOLTAGE = 0x130,               /* 充电桩上报 BMS 每节电池电压信息指令 */

    NETSL_SRESCMD_REPORT_BMS_BATTERY_TEMPERATURE = 0x131,                 /* 服务器应答充电桩 BMS 动力蓄电池温度信息指令 */
    NETSL_PREQCMD_REPORT_BMS_BATTERY_TEMPERATURE = 0x132,                 /* 充电桩上报 BMS 动力蓄电池温度信息指令 */

    NETSL_SREQCMD_REPORT_DEBUG_LOG = 0x133,                               /* 服务器下发充电桩上传 log 调试信息指令 */
    NETSL_PRESCMD_REPORT_DEBUG_LOG = 0x134,                               /* 充电桩回应上传 log 调试消息指令 */

    NETSL_SREQCMD_QUERY_HISTORY_CHARGE_RECORD = 0x191,                    /* 服务器查询充电桩历史充电记录指令 */
    NETSL_PRESCMD_QUERY_HISTORY_CHARGE_RECORD = 0x192,                    /* 充电桩上报历史的充电记录指令 */

    NETSL_SREQCMD_SOFTWARE_UPDATE = 0x3E9,                                /* 服务器下发升级指令指令 */
    NETSL_PRESCMD_SOFTWARE_UPDATE = 0x3EA,                                /* 客户端回复服务器升级指令 */

    NETSL_SRESCMD_REPORT_UPDATE_STATE = 0x3EB,                            /* 服务器回应升级状态上报指令 */
    NETSL_PREQCMD_REPORT_UPDATE_STATE = 0x3EC,                            /* 客户端升级状态上报指令 */

    NETSL_SREQCMD_ISSUE_BILLING_RULE = 0x515,                             /* 后台下发电费计价策略信息指令 */
    NETSL_PRESCMD_ISSUE_BILLING_RULE = 0x516,                             /* 充电桩响应后台电费计价策略信息指令 */

    NETSL_SRESCMD_GATEWAY_SIGN_IN = 0x5DD,                                /* 服务器应答网关签到指令 */
    NETSL_PREQCMD_GATEWAY_SIGN_IN = 0x5DE,                                /* 网关签到指令 */

    NETSL_SRESCMD_GATEWAY_REPORT_HEARTBEAT = 0x5DF,                       /* 服务器应答网关心跳包信息指令 */
    NETSL_PREQCMD_GATEWAY_REPORT_HEARTBEAT = 0x5E0,                       /* 网关上传心跳包信息指令 */

    NETSL_SRESCMD_GATEWAY_REPORT_DEBUG_LOG = 0x5E1,                       /* 服务器回应上传 log 调试消息指令 */
    NETSL_PREQCMD_GATEWAY_REPORT_DEBUG_LOG = 0x5E2,                       /* 网关上传 log 调试信息指令 */

    NETSL_SRESCMD_GATEWAY_REPORT_MESHNODE_STRUCT = 0x5E3,                 /* 服务器回应上传 MESH 节点结构指令 */
    NETSL_PREQCMD_GATEWAY_REPORT_MESHNODE_STRUCT = 0x5E4,                 /* 网关（根节点）上传 MESH 节点结构指令 */

    NETSL_SREQCMD_REPORT_MODULE_PARA_INFO = 0x641,                        /* 服务器下发模块参数命令指令 */
    NETSL_PRESCMD_REPORT_MODULE_PARA_INFO = 0x642,                        /* 充电桩上传模块参数信息指令 */

    NETSL_SREQCMD_QUERY_MODULE_STATE = 0x643,                             /* 服务器下发查询模块状态命令指令 */
    NETSL_PRESCMD_QUERY_MODULE_STATE = 0x644,                             /* 充电桩上传模块状态信息指令 */

    NETSL_SRESCMD_REPORT_EVENT_INFO = 0x6A5,                              /* 服务器响应充电桩上传事件指令 */
    NETSL_PREQCMD_REPORT_EVENT_INFO = 0x6A6,                              /* 充电桩上传事件信息指令 */

    NETSL_SREQCMD_ISSUE_CHARGEPILE_REPORT_EVENT_CMD = 0x6A7,              /* 服务器下发充电桩上传事件命令指令 */
    NETSL_PRESCMD_ISSUE_CHARGEPILE_REPORT_EVENT_CMD = 0x6A8,              /* 充电桩响应服务器下发参数命令指令 */
};

#pragma pack(1)

/*
 *   SLPro     as sunlight protocol   阳光乐通协议
 */

/*( 直流参数部分的头字段 */
typedef struct{
    uint8_t pile_number[32];
    uint8_t trade_number[32];
    uint8_t pile_state;
    uint8_t connect_state;
}Net_XjPro_DcSection_Head_t;

/** 协议头 */
typedef struct{
    uint16_t start;          /* 起始码 */
    uint16_t length;         /* 长度 */
    uint8_t version;         /* 版本号 */
    uint8_t serial_number;   /* 序列号 */
    uint16_t cmd;            /* 指令码 */
}Net_SLPro_Head_t;

/** 01 后台服务器向充电桩下发充电桩整形工作参数设置/查询命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_type;                                   /* 设备端类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t type;                                          /* 类型：0-查询 1-设置 */
        uint32_t base_addr;                                    /* 设置/查询参数 启始地址 */
        uint8_t number;                                        /* 设置/查询个数 */
        uint16_t para_byte_number;                             /* 设置参数字节数 */
    }body;
}Net_SLPro_SReq_QuerySet_IntWorkPara_t;

/** 02 充电桩整形参数设置/查询应答 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_type;                                   /* 设备端类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t type;                                          /* 类型：0-查询 1-设置 */
        uint32_t base_addr;                                    /* 设置/查询参数 启始地址 */
        uint8_t number;                                        /* 设置/查询个数 */
        uint8_t result;                                        /* 结果 */
        uint8_t data[4 *NET_SL_SINGLE_INT_PARA_NUM_MAX];       /* 数据 */
    }body;
    uint8_t check_sum;                                         /* 检验码 */
}Net_SLPro_PRes_QuerySet_IntWorkPara_t;

/** 03 后台服务器设置/查询充电桩字符型工作参数 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_type;                                   /* 设备端类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t type;                                          /* 类型：0-查询 1-设置 */
        uint32_t base_addr;                                    /* 设置/查询参数 启始地址 */
        uint16_t para_byte_number;                             /* 设置参数字节数 */
    }body;
}Net_SLPro_SReq_QuerySet_StringWorkPara_t;

/** 04 充电桩字符型参数设置/查询应答 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_type;                                   /* 设备端类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t type;                                          /* 类型：0-查询 1-设置 */
        uint32_t base_addr;                                    /* 设置/查询参数 启始地址 */
        uint8_t result;                                        /* 结果 */
        uint8_t data[NET_SL_SINGLE_STRING_PARA_LENGTH_MAX];    /* 数据 */
    }body;
    uint8_t check_sum;                                         /* 检验码 */
}Net_SLPro_PRes_QuerySet_StringWorkPara_t;

/** 5 后台服务器下发控制命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_type;                                   /* 设备端类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gunno;                                         /* 枪号 */
        uint32_t command_start_flag;                           /* 命令起始标志 */
        uint8_t command_number;                                /* 命令个数 */
        uint16_t command_para_len;                             /* 命令参数长度 */
    }body;
}Net_SLPro_SReq_Issue_ControlCommand_t;

/** 6 客户端对后台控制命令应答 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_type;                                   /* 设备端类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gunno;                                         /* 枪号 */
        uint32_t command_start_flag;                           /* 命令起始标志 */
        uint8_t command_number;                                /* 命令个数 */
        uint8_t result;                                        /* 结果 */
    }body;
    uint8_t check_sum;                                         /* 检验码 */
}Net_SLPro_PRes_Issue_ControlCommand_t;

/** 7 后台服务器下发充电桩开启充电控制命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gunno;                                         /* 枪号 */
        uint32_t charge_type;                                  /* 充电类型(0:即时充电  2:预约充电) */
        uint32_t account_ballance;                             /* 账号余额:金额单位为 0.01 元  */
        uint32_t charge_strategy;                              /* 充电策略 */
        uint32_t charge_strategy_para;                         /* 充电策略参数 */
        uint8_t reservation_start_time[NET_SL_STANDAR_TIME_LENGTH_MAX];     /* 预约开始时间 */
        uint8_t reserve2;                                      /* 保留 */
        uint8_t trade_no[NET_SL_SERIAL_NUMBER_LENGTH_MAX];     /* 流水号 */
        uint8_t charging_after_disconnect;                     /* 断网充电标志 */
        uint32_t allow_charge_elect_in_offline;                /* 离网可充电量 */
    }body;
}Net_SLPro_SReq_Issue_StartCharge_Command_t;

/** 8 充电桩对后台下发的充电桩开启充电控制应答  */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gunno;                                         /* 枪号 */
        uint32_t result;                                       /* 结果 */
    }body;
    uint8_t check_sum;                                         /* 检验码 */
}Net_SLPro_PRes_Issue_StartCharge_Command_t;

/** 9 服务器应查询电桩版本命令  */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                                     /* 保留 */
        uint16_t reserve1;                                     /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
    }body;
}Net_SLPro_SReq_Query_ChargePile_Version_t;

/** 10 充电桩版本信息应答  */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint32_t reserve0;                                     /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t mc_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX];   /* 主控板软件(mc as master control ) */
        uint8_t mc_hard_version[NET_SL_HARDWARE_VERSION_LENGTH_MAX];   /* 主控板硬件 */
        uint8_t led_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX];  /* 灯控软件版本 */
        uint8_t led_hard_version[NET_SL_HARDWARE_VERSION_LENGTH_MAX];  /* 灯控硬件版本 */
        uint8_t nm_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX];   /* 网联模块软件版本(nm as net module) */
        uint8_t nm_hard_version[NET_SL_HARDWARE_VERSION_LENGTH_MAX];   /* 网联模块硬件版本 */
        uint8_t sub_board_number;                              /* 分控板个数 */
        uint8_t sc_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX];  /* 分控板软件版本(sc as sub control) */
        uint8_t sc_hard_version[NET_SL_HARDWARE_VERSION_LENGTH_MAX];  /* 分控板硬件版本 */
        uint8_t module_number;                                 /* 模块个数 */
        uint8_t pfc_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX];  /* PFC 软件版本 */
        uint8_t pfc_hard_version[NET_SL_HARDWARE_VERSION_LENGTH_MAX];  /* PFC 硬件版本 */
        uint8_t llc_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX];  /* LLC 软件版本 */
        uint8_t llc_hard_version[NET_SL_HARDWARE_VERSION_LENGTH_MAX];  /* LLC 硬件版本 */
    }body;
    uint8_t check_sum;                                         /* 检验码 */
}Net_SLPro_PRes_Query_ChargePile_Version_t;

/** 102 充电桩上传心跳包信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint16_t heartbeat_sn;                  /* 心跳序号 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_Heartbeat_t;

/** 101 服务器应答心跳包信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint16_t heartbeat_response_sn;         /* 心跳响应序列号 */
    }body;
}Net_SLPro_SRes_Report_Heartbeat_t;

/** 104 充电桩状态信息包上报 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t protocol_type;                  /* 协议类型 */
        uint16_t reserve0;                      /* 保留 */
        uint8_t elect_accuracy;                 /* 电量精度标志 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gun_amount;                     /* 枪数量 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t pile_type;                      /* 桩类型 */
        uint8_t pile_state;                     /* 桩状态 */
        uint8_t current_soc;                    /* 当前SOC */
        uint8_t ac_fault_state[16];             /* 故障状态（保留填充 0） */
        uint8_t warnning_state[4];              /* 告警状态（保留填充 0） */
        uint8_t connect_state;                  /* 连接状态 */
        uint32_t current_charge_fees;           /* 本次充电费用 */
        uint16_t cp_voltage;                    /* CP 电压 */
        uint16_t diode_voltage;                 /* 二极管检测电压 */
        uint32_t reserve1;                      /* 保留 */
        uint16_t dc_charge_voltage;             /* 直流充电电压 */
        uint16_t dc_charge_current;             /* 直流充电电流 */
        uint16_t bms_require_voltage;           /* BMS需求电压 */
        uint16_t bms_require_current;           /* BMS需求电流 */
        uint8_t bms_charge_mode;                /* BMS充电模式 */
        uint16_t ac_charge_voltage_a_phase;     /* A相充电电压 */
        uint16_t ac_charge_voltage_b_phase;     /* B相充电电压 */
        uint16_t ac_charge_voltage_c_phase;     /* C相充电电压 */
        uint16_t ac_charge_current_a_phase;     /* A相充电电流 */
        uint16_t ac_charge_current_b_phase;     /* B相充电电流 */
        uint16_t ac_charge_current_c_phase;     /* C相充电电流 */
        uint16_t remain_charge_time;            /* 剩余充电时间 */
        uint32_t charge_time;                   /* 已充电时长 */
        uint32_t current_charge_elect;          /* 本次已充电量 */
        uint32_t ammeter_value_before_charge;   /* 充电前电表读数 */
        uint32_t ammeter_value_current;         /* 当前电表读数 */
        uint8_t start_type;                     /* 启动方式 */
        uint8_t charge_strategy;                /* 充电策略 */
        uint32_t charge_strategy_para;          /* 充电策略参数 */
        uint8_t reserve2;                       /* 保留 */
        uint8_t user_number[NET_SL_USER_NUMBER_LENGTH_MAX]; /* 充电/预约用户 */
        uint8_t reservation_timeout_time;       /* 预约超时时间 */
        uint8_t reservation_start_time[NET_SL_STANDAR_TIME_LENGTH_MAX]; /* 预约开始时间 */
        uint32_t card_ballance_before_charge;   /* 充电枪卡余额 */
        uint8_t temperaure1;                    /* 温度1 */
        uint8_t temperaure2;                    /* 温度2 */
        uint8_t temperaure3;                    /* 温度3 */
        uint8_t temperaure4;                    /* 温度4 */
        uint32_t charge_power;                  /* 充电功率 */
        uint32_t mc_drive_in_detect;            /* 主控开入检测状态 */
        uint32_t mc_drive_out_detect;           /* 主控开出检测状态 */
        uint32_t reserve3;                      /* 保留 */
        uint32_t fault_state[4];                /* 故障状态 */
        uint32_t warnning[4];                   /* 告警 */
        uint32_t service_fees;                  /* 服务费 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_StateInfo_t;

/** 103 服务器应答充电桩状态信息包 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t card_number[NET_SL_CARD_NUMBER_LENGTH_MAX];    /* 充电卡号 */
        uint32_t card_ballance;                 /* 充电卡余额 */
        uint8_t is_enough_card_ballance;        /* 充电卡余额是否足够 */
    }body;
}Net_SLPro_SRes_Report_StateInfo_t;

/** 106 充电桩签到信息上报 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t elect_loss_rate;               /* 电损率 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];  /* SN 编码 */
        uint8_t allow_pile_setup;               /* 离线/在线允许充电桩设置 */
        uint8_t pile_soft_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX]; /* 充电桩软件版本 */
        uint8_t pile_hard_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX]; /* 充电桩硬件版本 */
        uint16_t pile_model;                    /* 充电桩型号代码 */
        uint8_t device_name[NET_SL_DEVICE_NAME_LENGTH_MAX]; /* 充电桩设备名称 */
        uint8_t data_report_mode;               /* 数据上传模式 */
        uint16_t sign_in_interval_time;         /* 签到间隔时间 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t gun_amount;                     /* 充电枪个数 */
        uint8_t heartbeat_report_period;        /* 心跳上报周期 */
        uint8_t heartbeat_timeout_count;        /* 心跳检测超时次数 */
        uint32_t charge_record_number;          /* 充电记录数量 */
        uint8_t current_system_time[NET_SL_STANDAR_TIME_LENGTH_MAX]; /* 充电桩当前系统时间 */
        uint8_t protocol_version[NET_SL_PROTOCOL_VERSION_LENGTH_MAX]; /* 协议版本 */
        uint8_t platform_mode;                  /* 平台模式 */
        uint8_t reserve2[7];                    /* 保留 */
        uint8_t reserve3[8];                    /* 保留 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_SignIn_Info_t;

/** 105 服务器应答充电桩签到命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t sign_in_state;                  /* 签到状态 */
    }body;
}Net_SLPro_SRes_Report_SignIn_Info_t;

/** 202 充电桩上报最新一次充电信息(停充后订单上报) */
struct segment{
    uint8_t start_period;                       /* 开始时段 */
    uint8_t end_period;                         /* 开始时段 */
    uint8_t elect_fees_rank;                    /* 电费等级 */
    uint32_t period_elect;                      /* 时段电量 */
    uint32_t period_time;                       /* 时段时长 */
    uint32_t period_elect_fees_price;           /* 时段电费价格 */
    uint32_t period_service_fees_price;         /* 时段服务费价格 */
    uint32_t period_occupy_fees;                /* 时段占位费（保留） */
};
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t protocol_type;                  /* 协议类型 */
        uint8_t version_sn;                     /* 版本编号 */
        uint8_t reserve0;                       /* 保留 */
        uint8_t elect_accuracy;                 /* 电量精度标志 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t chargegun_type;                 /* 充电枪类型 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t card_user_number[NET_SL_CARD_NUMBER_LENGTH_MAX];    /* 充电卡/用户号 */
        uint8_t start_time[NET_SL_STANDAR_TIME_LENGTH_MAX];   /* 充电开始时间 */
        uint8_t stop_time[NET_SL_STANDAR_TIME_LENGTH_MAX];    /* 充电结束时间 */
        uint32_t charge_time;                   /* 充电时长 */
        uint8_t start_soc;                      /* 起始SOC */
        uint8_t end_soc;                        /* 终止SOC */
        uint8_t stop_reason0[4];                /* 充电结束原因 */
        uint32_t current_charge_elect;          /* 本次充电电量 */
        uint32_t ammeter_data_start;            /* 充电前电表读数 */
        uint32_t ammeter_data_stop;             /* 充电后电表读数 */
        uint32_t current_charge_fees;           /* 本次充电费用 */
        uint32_t reserve1;                      /* 保留 */
        uint32_t card_ballance_start;           /* 充电前卡余额 */
        uint32_t card_ballance_stop;            /* 充电后卡余额 */
        uint32_t service_fee_amount;            /* 服务费金额 */
        uint8_t is_payed;                       /* 是否已支付 */
        uint8_t charge_strategy;                /* 充电策略 */
        uint32_t charge_strategy_para;          /* 充电策略参数 */
        uint8_t car_vin[NET_SL_CARD_VIN_LENGTH_MAX];  /* 车辆 VIN */
        uint8_t license_plate_number[NET_SL_LICENSE_PLATE_NUMBER_LENGTH_MAX];  /* 车牌号 */
        uint8_t start_type;                     /* 启动方式 */
        uint8_t stop_reason1[4];                /* 充电结束原因 */
        uint8_t group_number;                   /* 分组数 */
        struct segment segment_info[NET_SL_SINGLE_TIME_CHARGE_PERIOD_MAX];
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_ChargeInfo_After_ChargeEnd_t;

/** 201 服务器响应充电桩上报最新一次充电信息(停充后订单上报) */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t card_number[NET_SL_CARD_NUMBER_LENGTH_MAX];    /* 充电卡号 */
    }body;
}Net_SLPro_SRes_Report_ChargeInfo_After_ChargeEnd_t;

/** 204 充电桩上传用户卡信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t card_number[NET_SL_CARD_NUMBER_LENGTH_MAX];    /* 充电卡号 */
        uint8_t card_type;                      /* 卡类型 */
        uint32_t card_ballance;                 /* 卡余额 */
        uint8_t is_update_ballance;             /* 是否更新卡余额 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_UserCard_Info_t;

/** 203 服务器应答用户卡查询信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint32_t response_code;                 /* 响应码 */
        uint32_t account_ballance;              /* 账户余额 */
        uint32_t unpay_money;                   /* 用户未结算金额 */
        uint8_t charge_date_last[NET_SL_STANDAR_TIME_LENGTH_MAX]; /* 保留 */
        uint16_t charge_time;                   /* 充电时长 */
        uint32_t charge_elect;                  /* 充电电量 */
        uint8_t ballance_indicate;              /* 余额指示 */
    }body;
}Net_SLPro_SRes_Report_UserCard_Info_t;

/** 302 充电桩上报 BMS 信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t message_sequence_count;        /* 报文次序计数 */
        uint16_t reserve0;                      /* 保留 */
        uint16_t gunno;                         /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t pile_state;                     /* 桩状态 */
        uint8_t connect_state;                  /* 车连接状态 */
        uint8_t bms_protocol_version[3];        /* BMS通讯协议版本号 */
        uint8_t battery_type;                   /* 电池类型 */
        uint32_t battery_rated_capacity;        /* 整车动力蓄电池系统额定容量/Ah */
        uint32_t battery_rated_voltage;         /* 整车动力蓄电池系统额定总电压/V */
        uint8_t battery_manufacturers[4];       /* 电池生产厂商 */
        uint8_t battery_group_sn[4];            /* 电池组序号 */
        uint16_t battery_production_date_year;  /* 电池组生厂日期：年 */
        uint8_t battery_production_date_month;  /* 电池组生厂日期：月 */
        uint8_t battery_production_date_date;   /* 电池组生厂日期：日 */
        uint8_t battery_charge_count[4];        /* 电池组充电次数 */
        uint8_t battery_title_identification;   /* 电池组产权标识 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t car_vin[NET_SL_CARD_VIN_LENGTH_MAX];   /* VIN码 */
        uint8_t bms_software_version[8];        /* BMS软件版本 */
        uint32_t single_allowvoltage_max;       /* 单体动力蓄电池最高允许充电电压 */
        uint32_t allow_charge_current_max;      /* 最高允许充电电流 */
        uint32_t nominal_total_energy;          /* 动力蓄电池标称总能量 */
        uint32_t allow_charge_voltage_total_max;/* 最高允许充电总电压 */
        uint8_t allow_temp_max;                 /* 最高允许温度 */
        uint16_t start_soc;                     /* 整车动力蓄电池荷电状态 BCP */
        uint32_t current_battery_voltage;       /* 整车动力蓄电池当前电池电压 */
        uint8_t whether_bms_ready;              /* BMS是否充电准备好 */
        uint32_t bms_require_voltage;           /* BMS 需求电压 */
        uint32_t bms_require_current;           /* BMS 需求电流 */
        uint8_t battery_charge_mode;            /* 电池充电模式（恒压、恒流） */
        uint32_t measure_voltage;               /* 充电电压测量值 */
        uint32_t measure_current;               /* 充电电流测量值 */
        uint32_t single_battery_voltage_max;    /* 最高单体动力蓄电池电压 */
        uint8_t single_voltage_max_group;       /* 最高单体动力蓄电池组号 */
        uint16_t soc;                           /* 当前荷电状态soc% */
        uint32_t remain_time;                   /* 估算剩余充电时间 */
        uint8_t single_voltage_max_sn;          /* 最高单体动力蓄电池电压所在编号 */
        uint8_t single_temp_max;                /* 最高动力蓄电池温度 */
        uint8_t single_temp_max_sn;             /* 最高温度检测点编号 */
        uint8_t single_temp_min;                /* 最低动力蓄电池温度 */
        uint8_t single_temp_min_sn;             /* 最低温度检测点编号 */
        struct{
            uint8_t single_voltage_abnormal;    /* 单体动力蓄电池电压异常(过高或过低) */
            uint8_t soc_abnormal;               /* 整车动力蓄电池荷电状态soc异常(过高或过低) */
            uint8_t battery_charge_overcurr;    /* 动力蓄电池充电过电流 */
            uint8_t battery_overtemp;           /* 动力蓄电池温度过高 */
            uint8_t battery_insulation;         /* 动力蓄电池绝缘状态 */
            uint8_t battery_output_linker;      /* 动力蓄电池组输出连接器连接状态 */
            uint8_t allow_charge;               /* 允许充电 */
        }state_bsm;
        struct{
            uint8_t reach_target_soc;           /* BMS达到所需求的SOC目标值 */
            uint8_t reach_target_voltage;       /* BMS达到总电压的设定值 */
            uint8_t reach_target_single_voltage;/* 达到单体电压的设定值 */
            uint8_t pile_end_charge;            /* 充电桩主动终止 */
            uint8_t insulation_fault;           /* 绝缘故障 */
            uint8_t output_linker_overtemp;     /* 输出连接器过温故障 */
            uint8_t bms_output_linker_overtemp; /* BMS元件，输出连接器过温 */
            uint8_t charge_linker_fault;        /* 充电连接器故障 */
            uint8_t battery_group_overtemp;     /* 电池组温度过高故障 */
            uint8_t hv_relay_fault;             /* 高压继电器故障 */
            uint8_t detect_point_2_voltage;     /* 检测点2电压检测故障 */
            uint8_t other_fault;                /* 其他故障 */
            uint8_t overcurr;                   /* 电流过大 */
            uint8_t voltage_abnormal;           /* 电压异常 */
        }state_bst;
        struct{
            uint16_t end_soc;                   /* 终止荷电状态soc */
            uint32_t single_voltage_min;        /* 动力蓄电池单体最低电压 */
            uint32_t single_voltage_max;        /* 动力蓄电池单体最高电压 */
            uint8_t battery_temp_min;           /* 动力蓄电池最低温度 */
            uint8_t battery_temp_max;           /* 动力蓄电池最高温度 */
        }state_bsd;
        struct{
            uint8_t spn2560_00_message_timeout; /* 接收SPN2560=0x00的充电桩辨识报文超时 */
            uint8_t spn2560_aa_message_timeout; /* 接收SPN2560=0xaa的充电桩辨识报文超时 */
            uint8_t pile_sync_output_max_message_timeout; /* 接收充电桩的时间同步和最大输出能力报文超时 */
            uint8_t pile_ready_message_timeout; /* 接收充电桩完成充电准备报文超时 */
            uint8_t pile_state_message_timeout; /* 接收充电桩充电状态报文超时 */
            uint8_t pile_end_message_timeout;   /* 接收充电桩终止充电报文超时 */
            uint8_t pile_statistics_message_timeout; /* 接收充电桩充电统计报文超时 */
            uint8_t other;   /* 其它 */
        }state_bem;
        uint16_t module_set_voltage;            /* 模块设置电压 */
        uint16_t module_set_current;            /* 模块设置电流 */
        uint8_t module_state;                   /* 模块状态 */
        uint8_t module_sn[NET_SL_MODULE_SN_LENGTH_MAX]; /* 模块设置电压 */
        uint8_t module_fault_group;             /* 模块故障组*/
        uint8_t llc_mos_temp;                   /* LLC MOS 管温度*/
        uint8_t anti_backflow_diode_temp;       /* 防倒灌二极管温度 */
        uint8_t llc_transformer_temp;           /* LLC 变压器温度 */
        uint8_t pfc_rectifier_bridge_temp;      /* PFC 整流桥温度 */
        uint32_t reserve3;                      /* 预留 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_BMSInfo_t;

/** 301 服务器应答充电桩上报BMS信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t message_sequence_count;        /* 报文次序计数 */
        uint16_t gunno;                         /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
    }body;
}Net_SLPro_SRes_Report_BMSInfo_t;

/** 304 充电桩上报 BMS 每节电池电压信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t message_sequence_count;        /* 报文次序计数 */
        uint16_t gunno;                         /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t work_state;                     /* 工作状态 */
        uint16_t single_battery_voltage[NET_SL_SINGLE_BATTERY_NUMBER_MAX]; /* 单体电池电压 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_BMSSingleBattery_Voltage_t;

/** 303 服务器应答充电桩 BMS 每节电池电压信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t message_sequence_count;        /* 报文次序计数 */
        uint16_t gunno;                         /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
    }body;
}Net_SLPro_SRes_Report_BMSSingleBattery_Voltage_t;

/** 306 充电桩上报 BMS 动力蓄电池温度信息*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t message_sequence_count;        /* 报文次序计数 */
        uint16_t gunno;                         /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t work_state;                     /* 工作状态 */
        uint16_t battery_temp[NET_SL_BATTERY_TEMP_NUMBER_MAX]; /* 电池温度 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_BMSBattery_Temperature_t;

/** 305 服务器应答充电桩 BMS 动力蓄电池温度信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t message_sequence_count;        /* 报文次序计数 */
        uint16_t gunno;                         /* 枪号 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
    }body;
}Net_SLPro_SRes_Report_BMSBattery_Temperature_t;

/** 308 充电桩回应上传 log 调试消息*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t result;                         /* 结果 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_Report_DebugLog_t;

/** 307 服务器下发充电桩上传 log 调试信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t report_path[NET_SL_LOG_REPROT_PATH_LENGTH_MAX]; /* 本地日志文件路径 */
        uint8_t server_addr[NET_SL_LOG_REPORT_SERVER_ADDR_MAX]; /* 服务器日志传输地址 */
        uint8_t log_catalog[NET_SL_SERVER_LOG_CATALOG_LENGTH_MAX]; /* 服务器日志传输目录 */
    }body;
}Net_SLPro_SReq_Report_DebugLog_t;

/** 402 充电桩上报历史的充电记录 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];     /* SN 编码 */
        uint8_t chargegun_type;                 /* 充电枪类型 */
        uint8_t gunno;                          /* 枪号 */
        uint8_t order_id[NET_SL_CARD_NUMBER_LENGTH_MAX];    /* 订单 ID */
        uint8_t start_time[NET_SL_STANDAR_TIME_LENGTH_MAX];   /* 充电开始时间 */
        uint8_t stop_time[NET_SL_STANDAR_TIME_LENGTH_MAX];    /* 充电结束时间 */
        uint32_t charge_time;                   /* 充电时长 */
        uint8_t start_soc;                      /* 起始SOC */
        uint8_t end_soc;                        /* 终止SOC */
        uint8_t stop_reason0[4];                /* 充电结束原因 */
        uint32_t current_charge_elect;          /* 本次充电电量 */
        uint32_t ammeter_data_start;            /* 充电前电表读数 */
        uint32_t ammeter_data_stop;             /* 充电后电表读数 */
        uint32_t current_charge_fees;           /* 本次充电费用 */
        uint32_t stop_isnot_by_card;            /* 是否不刷卡结束 */
        uint32_t card_ballance_start;           /* 充电前卡余额 */
        uint32_t card_ballance_stop;            /* 充电后卡余额 */
        uint32_t service_fee_amount;            /* 服务费金额 */
        uint8_t is_charge_discharge_order;      /* 是否充放电订单 */
        uint8_t charge_strategy;                /* 充电策略 */
        uint32_t charge_strategy_para;          /* 充电策略参数 */
        uint8_t car_vin[NET_SL_CARD_VIN_LENGTH_MAX];  /* 车辆 VIN */
        uint8_t license_plate_number[NET_SL_LICENSE_PLATE_NUMBER_LENGTH_MAX];  /* 车牌号 */
        uint16_t period_elect[NET_SL_PERIOD_COUNT_MAX];  /* 时段电量 */
        uint8_t start_type;                     /* 启动方式 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_Query_History_ChargeRecord_t;

/** 401 服务器查询充电桩历史充电记录 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];     /* SN 编码 */
        uint8_t query_sn[NET_SL_HISTORY_CHARGE_RECORD_SN_LENGTH_MAX];    /* 查询编码 */
    }body;
}Net_SLPro_SReq_Query_History_ChargeRecord_t;

/** 1002 客户端回复服务器升级指令*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t parameter;                      /* 参数 */
        uint8_t result;                         /* 响应状态 */
        uint16_t download_percent;              /* 文件下载进度 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_SoftwareUpdate_t;

/** 1001 服务器下发升级指令 */
struct para_03{
    uint8_t parameter;                      /* 参数 */
    uint16_t total_pack_num;                /* 总包数 */
    uint16_t current_pack_number;           /* 当前包数 */
    uint16_t data_len;                      /* 数据长度 */
    /* 具体数据 */
};
struct ftp_update_para{
    uint8_t server_ip[NET_SL_FTP_UPDATE_SERVER_IP_LENGTH_MAX];  /* FTP 服务器 IP 地址 */
    uint16_t server_port;                   /* FTP 服务器 PORT */
    uint8_t user_name[NET_SL_FTP_UPDATE_USER_NAME_LENGTH_MAX];  /* FTP 服务器用户名 */
    uint8_t password[NET_SL_FTP_UPDATE_PASSWORD_LENGTH_MAX];    /* FTP 服务器密码 */
    uint8_t path[NET_SL_FTP_UPDATE_PATH_LENGTH_MAX];            /* 升级文件路径 */
};
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t parameter;                      /* 参数 */
        uint8_t file_name[NET_SL_UPDATE_FILE_NAME_LENGTH_MAX]; /* 升级文件名 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t device_number;                  /* 设备数量 */
        uint8_t device_feature_code[NET_SL_DEVICE_FEATURES_LENGTH_MAX];  /* 设备特征码 */
        struct ftp_update_para para;            /* 升级参数 */
        uint8_t result;                         /* 结果 */
    }body;
}Net_SLPro_SReq_SoftwareUpdate_t;

/** 1004 客户端升级状态上报*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t device_type;                    /* 设备类型 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_logic_addr_count;        /* 设备逻辑地址总数 */
        uint8_t device_update_logic_addr;       /* 升级设备逻辑地址 */
        uint8_t update_state;                   /* 升级状态 */
        uint8_t execute_result;                 /* 执行结果*/
        uint32_t execute_result_para;           /* 执行结果参数 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_UpdateState_t;

/** 1003 服务器回应升级状态上报 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t result;                         /* 结果 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
    }body;
}Net_SLPro_SRes_Report_Report_UpdateState_t;

/** 1302 充电桩响应后台电费计价策略信息*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t result;                         /* 结果 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_Issue_BillingRule_t;

/** 1301 后台下发电费计价策略信息 */
struct period_segment{
    uint8_t start_period;                       /* 开始时段 */
    uint8_t end_period;                         /* 开始时段 */
    uint8_t elect_fees_rank;                    /* 电费等级 */
    uint32_t period_elect_fees_price;           /* 时段电费价格 */
    uint32_t period_service_fees_price;         /* 时段服务费价格 */
    uint32_t period_occupy_fees;                /* 时段占位费（保留） */
};
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t group_num;                      /* 分组数 */
    }body;
}Net_SLPro_SReq_Issue_BillingRule_t;

/** 1502 网关签到 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t gateway_sn[NET_SL_GATEWAY_SN_LENGTH_MAX];  /* 网关编码 编码 */
        uint16_t sign_in_interval_time;         /* 签到间隔时间 */
        uint16_t heartbeat_report_period;       /* 心跳上报周期 */
        uint8_t heartbeat_timeout_count;        /* 心跳检测超时次数 */
        uint8_t communication_type;             /* 通讯方式 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t current_state;                  /* 当前状态 */
        uint8_t device_model[NET_SL_DEVICE_MODEL_LENGTH_MAX];    /* 设备型号 */
        uint8_t device_name[NET_SL_DEVICE_NAME_GATEWAY_LENGTH_MAX];   /* 设备名称 */
        uint8_t hardware_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX]; /* 硬件版本 */
        uint8_t software_version[NET_SL_SOFTWARE_VERSION_LENGTH_MAX]; /* 软件版本 */
        uint8_t sim_iccid[NET_SL_SIM_ICCID_LENGTH_MAX];               /* SIM 卡ICCID */
        uint16_t gprs_strength;                  /* GPRS 信号强度 */
        uint8_t gateway_time[NET_SL_STANDAR_TIME_LENGTH_MAX]; /* 当前网关时间 */
    }body;
    uint8_t check_sum;                           /* 检验码 */
}Net_SLPro_PReq_GateWay_SignIn_t;

/** 1501 服务器应答网关签到命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t sign_in_state;                  /* 签到状态 */
        uint8_t heartbeat_timeout_count;        /* 心跳包检测超时次数 */
        uint16_t sign_in_interval_time;         /* 签到间隔时间 */
        uint16_t heartbeat_report_period;       /* 心跳上报周期 */
        uint8_t gateway_time[NET_SL_STANDAR_TIME_LENGTH_MAX]; /* 当前网关时间 */
    }body;
}Net_SLPro_SRes_GateWay_SignIn_t;

/** 1504 网关上传心跳包信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t signal_quality;                /* 信号质量 */
        uint16_t heartbeat_sn;                  /* 心跳序号 */
        uint32_t warnning_info;                 /* 告警信息 */
        uint32_t fault_info;                    /* 故障信息 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_GateWay_Report_Heartbeat_t;

/** 1503 服务器应答网关心跳包信息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint16_t response_sn;                   /* 心跳响应序列号 */
    }body;
}Net_SLPro_SRes_GateWay_Report_Heartbeat_t;

/** 1506 网关上传 log 调试信息*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t report_way;                     /* 上传方案 */
        uint16_t total_pack_num;                /* 总包数 */
        uint16_t current_progress;              /* 当前进度 */
        uint16_t data_len;                      /* 数据长度 */
        /* 数据 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_GateWay_Report_DebugLog_t;

/** 1505 服务器回应上传 log 调试消息 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint16_t reserve0;                      /* 保留 */
        uint16_t reserve1;                      /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX];        /* SN 编码 */
        uint8_t report_path[NET_SL_LOG_REPROT_PATH_LENGTH_MAX]; /* 本地日志文件路径 */
        uint8_t server_addr[NET_SL_LOG_REPORT_SERVER_ADDR_MAX]; /* 服务器日志传输地址 */
        uint8_t log_catalog[NET_SL_SERVER_LOG_CATALOG_LENGTH_MAX]; /* 服务器日志传输目录 */
    }body;
}Net_SLPro_SReq_GateWay_Report_DebugLog_t;

/** 1508 网关（根节点）上传 MESH 节点结构 */
struct device_node{
    uint8_t node_sn[NET_SL_DEVICE_NODE_SN_LENGTH_MAX]; /* 节点SN */
    uint8_t node_type;                          /* 节点类型 */
};
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t node_num;                       /* 节点数 */
        struct device_node node[NET_SL_DEVICE_NODE_NUM_MAX];
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_GateWay_Report_MESHNode_Struct_t;

/** 1507 服务器回应上传 MESH 节点结构 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t result;                         /* 结果 */
    }body;
}Net_SLPro_SRes_GateWay_Report_MESHNode_Struct_t;

/** 1602 充电桩上传模块参数信息*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t cmd_addr;                       /* 命令地址 */
        uint8_t para_page;                      /* 参数页 */
        uint16_t result;                        /* 结果 */
        uint8_t reserve2[8];                    /* 保留 */
        uint32_t data[20];                      /* 数据 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_Report_ModulePara_Info_t;

/** 1601 服务器下发模块参数命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t cmd_addr;                       /* 命令地址 */
        uint8_t type;                           /* 查询/设置 */
        uint8_t para_page;                      /* 参数页 */
        uint8_t reserve2[8];                    /* 保留 */
        /* 数据 */
    }body;
}Net_SLPro_SReq_Report_ModulePara_Info_t;

/** 1604 充电桩上传模块状态信息*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t cmd_addr;                       /* 命令地址 */
        uint8_t report_way;                     /* 上传周期 */
        uint8_t state_page;                     /* 状态页 */
        uint8_t reserve2[5];                    /* 保留 */
        uint8_t reserve3[3];                    /* 保留 */
        uint16_t state[40];                     /* 数据 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PRes_Query_ModuleState_t;

/** 1603 服务器下发查询模块状态命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t cmd_addr;                       /* 命令地址 */
        uint8_t report_way;                     /* 上传周期 */
        uint8_t state_page;                     /* 状态页 */
        uint8_t reserve2[8];                    /* 保留 */
    }body;
}Net_SLPro_SReq_Query_ModuleState_t;

/** 1702 充电桩上传事件信息*/
struct event_format{
    uint32_t event_time;                        /* 时间 */
    uint32_t event_code;                        /* 事件编码 */
    uint32_t additional_para;                   /* 附加参数 */
};
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t event_type;                     /* 事件类型 */
        uint16_t reserve2;                      /* 保留 */
        struct event_format event[20];          /* 事件 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_PReq_Report_EventInfo_t;

/** 1701 服务器响应充电桩上传事件 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t event_type;                     /* 事件类型 */
        uint8_t reserve2[8];                    /* 保留 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_SRes_Report_EventInfo_t;

/** 1704 充电桩响应服务器下发参数命令*/
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t event_type;                     /* 事件类型 */
        uint16_t reprot_period;                 /* 事件上传周期 */
        uint16_t event_up_call;                 /* 事件上召 */
        uint8_t reserve2[8];                    /* 保留 */
    }body;
}Net_SLPro_PRes_Issue_ChargePile_ReportEvent_Cmd_t;

/** 1703 服务器下发充电桩上传事件命令 */
typedef struct{
    Net_SLPro_Head_t head;
    struct{
        uint8_t reserve0[3];                    /* 保留 */
        uint8_t reserve1;                       /* 保留 */
        uint8_t device_sn[NET_SL_DEVICE_SN_LENGTH_MAX]; /* SN 编码 */
        uint8_t device_type;                    /* 设备类型 */
        uint8_t event_type;                     /* 事件类型 */
        uint16_t reprot_period;                 /* 事件上传周期 */
        uint16_t event_up_call;                 /* 事件上召 */
        uint8_t reserve2[8];                    /* 保留 */
    }body;
    uint8_t check_sum;                          /* 检验码 */
}Net_SLPro_SReq_Issue_ChargePile_ReportEvent_Cmd_t;

#pragma pack()

#endif /* NET_PACK_USING_SL */
#endif /* NET_NET_SL_INC_SL_MESSAGE_STRUCT_DEFINE_H_ */
