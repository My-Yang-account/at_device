/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-26     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_YKC_INC_YKC_MESSAGE_STRUCT_DEFINE_H_
#define NET_PACK_NET_YKC_INC_YKC_MESSAGE_STRUCT_DEFINE_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YKC

//#define NET_YKC_MESSAGE_USING_DUPU                                         /* 这是度普平台的报文 */
//#define NET_YKC_MESSAGE_USING_TLD                                          /* 这是特来电平台的报文 */

#define NET_YKC_STORAGE_INIT_FLAG                              0x12345777  /* 平台数据存储标志 */

#define NET_YKC_MESSAGE_START_CODE                             0x68        /* 报文起始码 */
#define NET_YKC_MESSAGE_ENCRYPT_ENABLE                         0x01        /* 报文加密 */
#define NET_YKC_MESSAGE_ENCRYPT_DISABLE                        0x00        /* 报文不加密 */
#define NET_YKC_PROTOCOL_VERSION                               0x10        /* 协议版本号（v1.6） */

#define NET_YKC_PROTOCOL_CHECK_REGION_SIZE                     0x02        /* 校验码域长度：单位字节 */

#define NET_YKC_NET_LINK_TYPE_SIM                              0x01        /* 网络链接类型：SIM 卡 */
#define NET_YKC_NET_LINK_TYPE_LAN                              0x02        /* 网络链接类型：LAN */
#define NET_YKC_NET_LINK_TYPE_WAN                              0x03        /* 网络链接类型：WAN */
#define NET_YKC_NET_LINK_TYPE_OTHER                            0x04        /* 网络链接类型：其它 */

#define NET_YKC_OPERATOR_MOBILE                                0x00        /* 运营商名称：中国移动 */
#define NET_YKC_OPERATOR_TELECOM                               0x02        /* 运营商名称：中国电信 */
#define NET_YKC_OPERATOR_UNICOM                                0x03        /* 运营商名称：中国联通 */
#define NET_YKC_OPERATOR_OTHER                                 0x04        /* 运营商名称：其它 */

#define NET_YKC_PILE_TYPE_DC                                   0x00        /* 桩类型：直流 */
#define NET_YKC_PILE_TYPE_AC                                   0x01        /* 桩类型：交流 */

#define NET_YKC_CHARGEPILE_LENGTH_DEFAULT                      0x07        /* 默认桩号长度 */
#define NET_YKC_CARD_NUMBER_LENGTH_MAX                         0x08        /* 卡号长度最大值 */
#define NET_YKC_CAR_VIN_NUMBER_LENGTH_MAX                      0x11        /* 车VIN码长度最大值 */
#define NET_YKC_RATE_PERIOD_COUNT_MAX                          0x30        /* 费率时段数 */
#define NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT                   0x10        /* 默认流水号长度 */
#define NET_YKC_USER_NAME_LENGTH_DEFAULT                       0x10        /* 默认用户名长度 */
#define NET_YKC_PASSWORD_LENGTH_DEFAULT                        0x10        /* 默认密码长度 */
#define NET_YKC_SERVER_ADDR_LENGTH_DEFAULT                     0x10        /* 默认服务器地址长度 */
#define NET_YKC_FILE_PATH_LENGTH_DEFAULT                       0x20        /* 默认文件路径长度 */
#define NET_YKC_GUNLINE_NUMBER_LENGTH_DEFAULT                  0x08        /* 默认枪线编号长度 */
#define NET_YKC_MERGE_CHARGE_SN_LENGTH_DEFAULT                 0x06        /* 默认并充序号长度 */
#define NET_YKC_SOFT_VERSION_LENGTH_DEFAULT                    0x08        /* 默认软件版本长度 */
#define NET_YKC_SIM_BCD_LENGTH_DEFAULT                         0x0A        /* 默认sim卡卡号BCD码长度 */
#define NET_YKC_CARD_NUMBER_COUNT_MAX                          0x18        /* 卡号最大个数 */

enum ykc_device_state{
    NETYKC_DEVICE_STATE_OFFLINE = 0x00,                      /* 设备状态：离线 */
    NETYKC_DEVICE_STATE_FAULTING = 0x01,                     /* 设备状态：故障 */
    NETYKC_DEVICE_STATE_IDLE = 0x02,                         /* 设备状态：空闲 */
    NETYKC_DEVICE_STATE_CHARGING = 0x03,                     /* 设备状态：充电 */
};

enum ykc_start_way{
    NETYKC_START_WAY_APP = 0x01,                             /* 启动方式：APP */
    NETYKC_START_WAY_ONLINE = 0x02,                          /* 启动方式：在线卡 */
    NETYKC_START_WAY_OFFLINE = 0x04,                         /* 启动方式：离线卡 */
    NETYKC_START_WAY_VIN = 0x05,                             /* 启动方式：VIN */
};

/* CC as CHARGE COMPLETE */
enum ykc_charge_complete{
    NETYKC_CC_REASON40_APP = 0x40,                           /* 充电完成原因：APP 远程停止 */
    NETYKC_CC_REASON41_CHARGE_FULL = 0x41,                   /* 充电完成原因：达到 100% */
    NETYKC_CC_REASON42_TARGET_ELECT = 0x42,                  /* 充电完成原因：充电电量满足设定条件 */
    NETYKC_CC_REASON43_TARGET_MONEY = 0x43,                  /* 充电完成原因：充电金额满足设定条件 */
    NETYKC_CC_REASON44_TARGET_TIME = 0x44,                   /* 充电完成原因：充电时间满足设定条件 */
    NETYKC_CC_REASON45_MANUAL_STOP = 0x45,                   /* 充电完成原因：手动停止充电 */
    NETYKC_CC_REASON46_RESERVE = 0x46,                       /* 充电完成原因：其他方式（预留） */
};

/* SF as START FAIL */
enum ykc_start_fail{
    NETYKC_SF_REASON4A_SYSCONTROL_FAULT = 0x4A,              /* 充电启动失败，充电桩控制系统故障(需要重启或自动恢复) */
    NETYKC_SF_REASON4B_GUIDE_DISCONNECT = 0x4B,              /* 充电启动失败，控制导引断开 */
    NETYKC_SF_REASON4C_CIRCUIT_BREAKER = 0x4C,               /* 充电启动失败，断路器跳位 */
    NETYKC_SF_REASON4D_AMMETER_COMMUNICATION = 0x4D,         /* 充电启动失败，电表通信中断 */
    NETYKC_SF_REASON4E_NO_BALLANCE = 0x4E,                   /* 充电启动失败，余额不足 */
    NETYKC_SF_REASON4F_CHARGE_MODULE = 0x4F,                 /* 充电启动失败，充电模块故障 */
    NETYKC_SF_REASON50_EMERGENCY_STOP = 0x50,                /* 充电启动失败，急停开入 */
    NETYKC_SF_REASON51_LPS_FAULT = 0x51,                     /* 充电启动失败，防雷器异常 */
    NETYKC_SF_REASON52_BMS_UNREADY = 0x52,                   /* 充电启动失败，BMS 未就绪 */
    NETYKC_SF_REASON53_ABNORMAL_TEMP = 0x53,                 /* 充电启动失败，温度异常 */
    NETYKC_SF_REASON54_BATTERY_REVERSE = 0x54,               /* 充电启动失败，电池反接故障 */
    NETYKC_SF_REASON55_ELECT_LOCK = 0x55,                    /* 充电启动失败，电子锁异常 */
    NETYKC_SF_REASON56_CLOSE_FAIL = 0x56,                    /* 充电启动失败，合闸失败 */
    NETYKC_SF_REASON57_INSULATION_ABNORMAL = 0x57,           /* 充电启动失败，绝缘异常 */
    NETYKC_SF_REASON58_RESERVE0 = 0x58,                      /* 预留 */
    NETYKC_SF_REASON59_RECV_BHM_TIMEOUT = 0x59,              /* 充电启动失败，接收 BMS 握手报文 BHM 超时 */
    NETYKC_SF_REASON5A_RECV_BRM_TIMEOUT = 0x5A,              /* 充电启动失败，接收 BMS 和车辆的辨识报文超时 BRM */
    NETYKC_SF_REASON5B_RECV_BCP_TIMEOUT = 0x5B,              /* 充电启动失败，接收电池充电参数报文超时 BCP */
    NETYKC_SF_REASON5C_RECV_BRO_AA_TIMEOUT = 0x5C,           /* 充电启动失败，接收 BMS 完成充电准备报文超时 BRO AA */
    NETYKC_SF_REASON5D_RECV_BCS_TIMEOUT = 0x5D,              /* 充电启动失败，接收电池充电总状态报文超时 BCS  */
    NETYKC_SF_REASON5E_RECV_BCL_TIMEOUT = 0x5E,              /* 充电启动失败，接收电池充电要求报文超时 BCL */
    NETYKC_SF_REASON5F_RECV_BSM_TIMEOUT = 0x5F,              /* 充电启动失败，接收电池状态信息报文超时 BSM */
    NETYKC_SF_REASON60_BHM_STAGE_VOLT_OVERRANGE = 0x60,      /* 充电启动失败，GB2015 电池在 BHM 阶段有电压不允许充电 */
    NETYKC_SF_REASON61_BRO_AA_STAGE_VOLT_OVERRANGE = 0x61,   /* 充电启动失败，GB2015 辨识阶段在 BRO_AA 时候电池实际电压与 BCP 报文电池电压差距大于 5% */
    NETYKC_SF_REASON62_BRO_AA_TO_BRO_00 = 0x62,              /* 充电完成原因：充电启动失败，B2015 充电机在预充电阶段从 BRO_AA 变成BRO_00 状态 */
    NETYKC_SF_REASON63_RECV_HOST_CONFIG_MESSAGE_TIMEOUT = 0x63, /* 充电启动失败，接收主机配置报文超时 */
    NETYKC_SF_REASON64_CHARGER_UNREADY = 0x64,               /* 充电启动失败：充电启动失败，充电机未准备就绪,我们没有回 CRO AA，对应老国标 */
    NETYKC_SF_REASON65_DC_RELAY = 0x65,                      /* 充电启动失败，DC继电器故障 */
    NETYKC_SF_REASON66_AUXPOWER = 0x66,                      /* 充电启动失败，辅助电源故障 */
    NETYKC_SF_REASON67_READY_VOLTAGE = 0x67,                 /* 充电异常中止，准备电压 */
    NETYKC_SF_REASON68_INSULT_VOLTAGE = 0x68,                /* 充电异常中止，绝缘电压 */
    NETYKC_SF_REASON69_RESERVE1 = 0x69,                      /* （其他原因）预留 */
};

/* AS as ABNORMAL STOP */
enum ykc_abnormal_stop{
    NETYKC_AS_REASON6A_SYSTEM_ATRESIA = 0x6A,                /* 充电异常中止，系统闭锁 */
    NETYKC_AS_REASON6B_GUIDANCE_DISCONNECT = 0x6B,           /* 充电异常中止，导引断开 */
    NETYKC_AS_REASON6C_CIRCUIT_BREAKER_ACTION = 0x6C,        /* 充电异常中止，断路器跳位 */
    NETYKC_AS_REASON6D_AMMETER_COMMUNICATE = 0x6D,           /* 充电异常中止，电表通信中断 */
    NETYKC_AS_REASON6E_NO_BALLANCE = 0x6E,                   /* 充电异常中止，余额不足 */
    NETYKC_AS_REASON6F_AC_PROTECT_ACTION = 0x6F,             /* 充电异常中止，交流保护动作 */
    NETYKC_AS_REASON70_DC_PROTECT_ACTION = 0x70,             /* 充电异常中止，直流保护动作 */
    NETYKC_AS_REASON71_CHARGE_MODULE = 0x71,                 /* 充电异常中止，充电模块故障 */
    NETYKC_AS_REASON72_EMERGENCY_STOP = 0x72,                /* 充电异常中止，急停开入 */
    NETYKC_AS_REASON73_LIGHTNING_PROTECTORS = 0x73,          /* 充电异常中止，防雷器异常 */
    NETYKC_AS_REASON74_TEMPERATURE_ABNORMAL = 0x74,          /* 充电异常中止，温度异常 */
    NETYKC_AS_REASON75_OUTPUT_ABNORMAL = 0x75,               /* 充电异常中止，输出异常 */
    NETYKC_AS_REASON76_CURRENT_ABNORMAL = 0x76,              /* 充电异常中止，充电无流 */
    NETYKC_AS_REASON77_ELOCK_ABNORMAL = 0x77,                /* 充电异常中止，电子锁异常 */
    NETYKC_AS_REASON78_RESERVE0 = 0x78,                      /* 预留*/
    NETYKC_AS_REASON79_CHARGE_TVOLTAGE_ABNORMAL = 0x79,      /* 充电异常中止，总充电电压异常 */
    NETYKC_AS_REASON7A_CHARGE_TCURRENT_ABNORMAL = 0x7A,      /* 充电异常中止，总充电电流异常 */
    NETYKC_AS_REASON7B_CHARGE_SVOLTAGE_ABNORMAL = 0x7B,      /* 充电异常中止，单体充电电压异常 */
    NETYKC_AS_REASON7C_BATTERY_GROUP_OVERTEMP = 0x7C,        /* 充电异常中止，电池组过温 */
    NETYKC_AS_REASON7D_CHARGE_SVOLTAGE_MAX_ABNORMAL = 0x7D,  /* 充电异常中止，最高单体充电电压异常 */
    NETYKC_AS_REASON7E_BGROUP_MTEMP_OVERTEMP = 0x7E,         /* 充电异常中止，最高电池组过温 */
    NETYKC_AS_REASON7F_BMV_CHARGE_SVOLTAGE_ABNORMAL = 0x7F,  /* 充电异常中止，BMV 单体充电电压异常 */
    NETYKC_AS_REASON80_BMT_BGROUP_OVERTEMP = 0x80,           /* 充电异常中止，BMT 电池组过温 */
    NETYKC_AS_REASON81_BSTATE_ABNORMAL = 0x81,               /* 充电异常中止，电池状态异常停止充电 */
    NETYKC_AS_REASON82_CAR_COMMAND_STOP = 0x82,              /* 充电异常中止，车辆发报文禁止充电 */
    NETYKC_AS_REASON83_POWER_OFF = 0x83,                     /* 充电异常中止，充电桩断电 */
    NETYKC_AS_REASON84_RECV_BCS_TIMEOUT = 0x84,              /* 充电异常中止，接收电池充电总状态报文超时 */
    NETYKC_AS_REASON85_RECV_BCL_TIMEOUT = 0x85,              /* 充电异常中止，接收电池充电要求报文超时 */
    NETYKC_AS_REASON86_RECV_BSM_TIMEOUT = 0x86,              /* 充电异常中止，接收电池状态信息报文超时 */
    NETYKC_AS_REASON87_RECV_BST_TIMEOUT = 0x87,              /* 充电异常中止，接收 BMS 中止充电报文超时 */
    NETYKC_AS_REASON88_RECV_BSD_TIMEOUT = 0x88,              /* 充电异常中止，接收 BMS 充电统计报文超时 */
    NETYKC_AS_REASON89_RECV_CCS_TIMEOUT = 0x89,              /* 充电异常中止，接收对侧 CCS 报文超时 */
    NETYKC_AS_REASON8A_CARDREADER = 0x8A,                    /* 充电异常中止，读卡器故障 */
    NETYKC_AS_REASON8B_DC_RELAY = 0x8B,                      /* 充电异常中止，DC继电器故障 */
    NETYKC_AS_REASON8C_AC_RELAY = 0x8C,                      /* 充电异常中止，AC继电器故障 */
    NETYKC_AS_REASON8D_PARALLEL_RELAY = 0x8D,                /* 充电异常中止，并联继电器故障 */
    NETYKC_AS_REASON8E_STORAGE_CHIP = 0x8E,                  /* 充电异常中止，存储芯片 */
    NETYKC_AS_REASON8F_BSM_WARNNING = 0x8F,                  /* 充电异常中止，BSM 告警 */
    NETYKC_AS_REASON90_UNKNOW = 0x90,                        /* 充电异常中止，未知原因停止 */
    NETYKC_AS_REASON91_GATE = 0x91,                          /* 充电异常中止，门禁故障 */
};

enum ykc_cmd{
    NETYKC_PREQCMD_SINGIN = 0x01,                            /* 指令：桩登录请求 */
    NETYKC_SRESCMD_SINGIN = 0x02,                            /* 指令：服务器响应登录 */

    NETYKC_PREQCMD_HEARTBEAT = 0x03,                         /* 指令：桩心跳请求 */
    NETYKC_SRESCMD_HEARTBEAT = 0x04,                         /* 指令：服务器响应心跳 */

    NETYKC_PREQCMD_BILLING_MODEL_VERIFY = 0x05,              /* 指令：桩请求计费模型验证 */
    NETYKC_SRESCMD_BILLING_MODEL_VERIFY = 0x06,              /* 指令：服务器响应计费模型验证 */

    NETYKC_PREQCMD_BILLING_MODEL_REQUEST = 0x09,             /* 指令：桩请求计费模型 */
    NETYKC_SRESCMD_BILLING_MODEL_REQUEST = 0x0A,             /* 指令：服务器响应计费模型请求  */

    NETYKC_SREQCMD_READ_REALTIME_DATA = 0x12,                /* 指令：服务器请求读取实时监测数据 */
    NETYKC_PREQCMD_PRESCMD_REPORT_REALTIME_DATA = 0x13,      /* 指令：桩请求上传(响应)实时监测数据  */

    NETYKC_PREQCMD_CHARGE_SHAKE_HAND = 0x15,                 /* 指令：桩上报充电握手 */

    NETYKC_PREQCMD_PARA_CONFIG = 0x17,                       /* 指令：桩上报参数配置 */

    NETYKC_PREQCMD_CHARGE_END = 0x19,                        /* 指令：桩上报充电结束 */

    NETYKC_PREQCMD_ERROR_MESSAGE = 0x1B,                     /* 指令：桩上报错误报文 */

    NETYKC_PREQCMD_BMS_STOP = 0x1D,                          /* 指令：桩上报充电阶段 BMS 中止 */

#ifdef NET_YKC_MESSAGE_USING_DUPU
    NETYKC_PREQCMD_STORED_ENERGY_INFO = 0x1F,                /* 指令：桩上报储能信息(度普)*/
#endif /* NET_YKC_MESSAGE_USING_DUPU */

    NETYKC_PREQCMD_CHARGER_STOP = 0x21,                      /* 指令：桩上报充电阶段充电机中止 */

    NETYKC_PREQCMD_CHARGER_OUTPUT_BMS_REQUIRE = 0x23,        /* 指令：桩上报充电过程 BMS 需求与充电机输出 */

    NETYKC_PREQCMD_BMS_INFO = 0x25,                          /* 指令：桩上报充电过程 BMS 信息 */

    NETYKC_PREQCMD_APPLY_START_CHARGE = 0x31,                /* 指令：充电桩主动申请启动充电 */
    NETYKC_SRESCMD_APPLY_START_CHARGE = 0x32,                /* 指令：运营平台确认启动充电 */

    NETYKC_SREQCMD_SERVER_START_CHARGE = 0x34,               /* 指令：运营平台远程控制启机 */
    NETYKC_PRESCMD_SERVER_START_CHARGE = 0x33,               /* 指令：远程启动充电命令回复 */

    NETYKC_SREQCMD_SERVER_STOP_CHARGE = 0x36,                /* 指令：运营平台远程停机 */
    NETYKC_PRESCMD_SERVER_STOP_CHARGE = 0x35,                /* 指令：远程停机命令回复 */

    NETYKC_PREQCMD_TRANSACTION_RECORD = 0x3B,                /* 指令：桩上报交易记录 */
    NETYKC_SRESCMD_TRANSACTION_RECORD_VERIFY = 0x40,         /* 指令：服务器交易记录确认 */

    NETYKC_SREQCMD_ACCOUNT_BALLANCE_UPDATE = 0x42,           /* 指令：服务器远程账户余额更新 */
    NETYKC_PRESCMD_ACCOUNT_BALLANCE_UPDATE = 0x41,           /* 指令：桩响应余额更新 */

    NETYKC_SREQCMD_SYNC_OFFLINECARD = 0x44,                  /* 指令：服务器同步离线卡数据 */
    NETYKC_PRESCMD_SYNC_OFFLINECARD = 0x43,                  /* 指令：桩响应离线卡数据同步 */

    NETYKC_SREQCMD_CLEAR_OFFLINECARD = 0x46,                 /* 指令：服务器清除离线卡数据 */
    NETYKC_PRESCMD_CLEAR_OFFLINECARD = 0x45,                 /* 指令：桩响应离线卡数据清除 */

    NETYKC_SREQCMD_QUERY_OFFLINECARD = 0x48,                 /* 指令：服务器查询离线卡数据 */
    NETYKC_PRESCMD_QUERY_OFFLINECARD = 0x47,                 /* 指令：桩响应离线卡数据查询 */

    NETYKC_SREQCMD_SET_WORK_PARA = 0x52,                     /* 指令：服务器设置充电桩工作参数 */
    NETYKC_PRESCMD_SET_WORK_PARA = 0x51,                     /* 指令：桩响应设置充电桩工作参数 */

    NETYKC_SREQCMD_TIME_SYNC = 0x56,                         /* 指令：服务器对时设置 */
    NETYKC_PRESCMD_TIME_SYNC = 0x55,                         /* 指令：桩响应对时设置 */

    NETYKC_SREQCMD_SET_BILLING_MODEL = 0x58,                 /* 指令：服务器设置计费模型 */
    NETYKC_PRESCMD_SET_BILLING_MODEL = 0x57,                 /* 指令：桩响应设置计费模型 */

    NETYKC_SREQCMD_QRCODE_CONFIG_GC = 0x5A,                  /* 指令：服务器设置二维码(国充) */
    NETYKC_PRESCMD_QRCODE_CONFIG_GC = 0x59,                  /* 指令：桩响应设置二维码(国充) */

    NETYKC_PREQCMD_GROUNDLOCK_INFO = 0x61,                   /* 指令：桩上报地锁数据 */

    NETYKC_SREQCMD_GROUNDLOCK_LIFTING = 0x62,                /* 指令：服务器遥控地锁升锁与降锁 */
    NETYKC_PRESCMD_GROUNDLOCK_LIFTING = 0x63,                /* 指令：桩响应遥控地锁升锁与降锁 */

    NETYKC_SREQCMD_REMOTE_REBOOT = 0x92,                     /* 指令：服务器远程重启 */
    NETYKC_PRESCMD_REMOTE_REBOOT = 0x91,                     /* 指令：桩响应远程重启 */

    NETYKC_SREQCMD_REMOTE_UPDATE = 0x94,                     /* 指令：服务器远程更新 */
    NETYKC_PRESCMD_REMOTE_UPDATE = 0x93,                     /* 指令：桩响应远程更新 */

    NETYKC_SREQCMD_QRCODE_CONFIG_TLD = 0x9C,                 /* 指令：服务器设置二维码(特来电) */
    NETYKC_PRESCMD_QRCODE_CONFIG_TLD = 0x9B,                 /* 指令：桩响应设置二维码(特来电) */

    NETYKC_PREQCMD_APPLY_START_MERGECHARGE = 0xA1,           /* 指令：充电桩主动申请启动并充充电 */
    NETYKC_SRESCMD_APPLY_START_MERGECHARGE = 0xA2,           /* 指令：运营平台确认启动并充充电 */

    NETYKC_SREQCMD_SERVER_START_MERGECHARGE = 0xA4,          /* 指令：运营平台远程控制并充启机 */
    NETYKC_PRESCMD_SERVER_START_MERGECHARGE = 0xA3,          /* 指令：远程并充启动充电命令回复 */

    NETYKC_SREQCMD_QRCODE_CONFIG_YKC15 = 0xF0,               /* 指令：服务器设置二维码(云快充1.5) */
    NETYKC_PRESCMD_QRCODE_CONFIG_YKC15 = 0xF1,               /* 指令：桩响应设置二维码(云快充1.5) */
};


#pragma pack(1)

typedef union {
    uint8_t tm[7];
    struct {
        uint16_t msec;
        uint8_t  min   :6;
        uint8_t  res1  :1;
        uint8_t  iv    :1; /* <0> = 有效；<1> = 无效 */
        uint8_t  hour  :5;
        uint8_t  res2  :2;
        uint8_t  su    :1; /* <0> = 标准时间；<1> = 夏时令 */
        uint8_t  mday  :5; /* Day of Month */
        uint8_t  wday  :3; /* Day of Week */
        uint8_t  month :4;
        uint8_t  res3  :4;
        uint8_t  year  :7;
        uint8_t  res4  :1;
    } cp56time2a_tm;
}cp56time2a_t;

#pragma pack() /* #pragma pack(1) */

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * YkcPro   as YkcProtocol                    云快充协议
 * SRes     as Server Response                服务器响应
 * SReq     as Server Request                 服务器请求
 * PRes     as Pile Response                  充电桩响应
 * PReq     as Pile Request                   充电桩请求
 * GResReq  as General Response Request       通用请求、响应
 * PSReq    as pile server request            桩端、服务器请求
 * PReqSRes as pile request server response   充电桩请求、服务器响应
 * SReqPRes as server request pile response   服务器请求、充电桩响应
 * SPRes    as server pile request            服务器、充电桩请求
 * SPReq    as server pile response           服务器、充电桩响应
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(1)

/** 云快充平台数据存储体 */
typedef struct{
    uint32_t storage_init_flag;              /* 存储初始化标志 */
    uint8_t verify_result;                   /* 数据校验结果 */

    struct{
        uint32_t lock :1;                    /* 功能开关：锁桩 */
        uint32_t reserve0 :31;               /* 功能开关：预留 */
        uint32_t reserve1 :32;               /* 功能开关：预留 */
    }fswitch;                                /* 功能开关 1：开启  0：关闭 */
}ykc_storage_struct;

/** 协议头部 */
typedef struct{
    uint8_t start_code;                           /* 起始码 */
    uint8_t length;                               /* 数据长度 */
    uint16_t sequence;                            /* 序列号 */
    uint8_t encrypt;                              /* 加密标志 */
    uint8_t type;                                 /* 帧类型 */
}Net_YkcPro_Head_t;

/** 0x01 登录签到帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t pile_type;                       /* 桩类型 */
        uint8_t gun_count;                       /* 充电枪数量 */
        uint8_t protocol_ver;                    /* 通信协议版本 */
        uint8_t software_ver[NET_YKC_SOFT_VERSION_LENGTH_DEFAULT];  /* 软件版本 */
        uint8_t net_link_type;                   /* 网络连接类型 */
        uint8_t sim_number[NET_YKC_SIM_BCD_LENGTH_DEFAULT];         /* SIM 卡卡号 */
        uint8_t operators;                       /* 运营商 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_LogIn_t;

/** 0x02 登录签到响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                          /* 登录结果 */
    }body;
}Net_YkcPro_SRes_LogIn_t;

/** 0x03 心跳帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t state;                           /* 枪状态 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_HeartBeat_t;

/** 0x04 心跳响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t response;                        /* 心跳响应 */
    }body;
}Net_YkcPro_SRes_HeartBeat_t;

/** 0x05 计费模型验证请求帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t model_number;                   /* 计费模型编号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_BillingModel_Verify_t;

/** 0x06 计费模型验证请求响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t model_number;                   /* 计费模型编号 */
        uint8_t result;                          /* 验证结果 */
    }body;
}Net_YkcPro_SRes_BillingModel_Verify_t;

/** 0x09 计费模型请求帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_BillingModel_Request_t;

/** 0x0A 计费模型请求响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t model_number;                   /* 计费模型编号 */
        uint32_t tip_elect_rate;                 /* 尖电费费率 */
        uint32_t tip_service_rate;               /* 尖服务费费率 */
        uint32_t peak_elect_rate;                /* 峰电费费率 */
        uint32_t peak_service_rate;              /* 峰服务费费率 */
        uint32_t flat_elect_rate;                /* 平电费费率 */
        uint32_t flat_service_rate;              /* 平服务费费率 */
        uint32_t valley_elect_rate;              /* 谷电费费率 */
        uint32_t valley_service_rate;            /* 谷服务费费率 */
        uint8_t loss_proportion;                 /* 计损比例 */
        uint8_t rate_number[NET_YKC_RATE_PERIOD_COUNT_MAX];     /* 费率号 */
    }body;
}Net_YkcPro_SRes_BillingModel_Request_t;

/** 0x012 读取实时数据帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
    }body;
}Net_YkcPro_SReq_Query_RealTimeData_t;

/** 0x013 读取实时数据响应(实时数据请求)帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t state;                           /* 状态 */
        uint8_t homing;                          /* 枪是否归位 */
        uint8_t plug_gun;                        /* 是否插枪 */
        uint16_t output_voltage;                 /* 输出电压 */
        uint16_t output_current;                 /* 输出电流 */
        uint8_t gun_temperature;                 /* 枪头温度 */
        uint8_t gunline_number[NET_YKC_GUNLINE_NUMBER_LENGTH_DEFAULT]; /* 充电枪线号 */
        uint8_t soc;                             /* SOC */
        uint8_t battery_group_temp_max;          /* 电池组最高温度 */
        uint16_t charge_time;                    /* 累计充电时长 */
        uint16_t remain_time;                    /* 剩余充电时间 */
        uint32_t charge_elect;                   /* 充电电量 */
        uint32_t loss_elect;                     /* 计损电量 */
        uint32_t consume_amount;                 /* 消费 */
        uint16_t hardware_fault;                 /* 硬件故障 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Query_PReq_Report_RealTimeData_t;

/** 0x15 充电握手帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t bms_protocol_ver[3];             /* BMS协议版本 */
        uint8_t bms_bat_type;                    /* BMS电池类型 */
        uint16_t bms_bat_rated_capacity;         /* BMS电池额定容量 */
        uint16_t bms_bat_rated_volt;             /* BMS电池额定电压 */
        uint8_t bms_bat_maker_name[4];           /* BMS电池制造商名 */
        uint8_t bms_bat_sn[4];                   /* BMS电池组序列号 */
        uint8_t bms_bat_date_year;               /* BMS电池组生产日期：年 */
        uint8_t bms_bat_date_month;              /* BMS电池组生产日期：月 */
        uint8_t bms_bat_date_day;                /* BMS电池组生产日期：日 */
        uint8_t bms_bat_charge_num[3];           /* BMS电池组充电次数 */
        uint8_t bms_bat_title_identification;    /* BMS电池组产权标识 */
        uint8_t reserve;                         /* 保留 */
        uint8_t vin[NET_YKC_CAR_VIN_NUMBER_LENGTH_MAX]; /* VIN码 */
        uint8_t bms_software_ver[8];             /* BMS软件版本 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ShakeHand_t;

/** 0x17 参数配置帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint16_t bms_single_bat_allow_volt_max;  /* BMS 单体动力蓄电池最高允许充电电压  */
        int16_t bms_allow_curr_max;              /* BMS 最高允许充电电流  */
        uint16_t bms_bat_nominal_energy_all;     /* BMS 动力蓄电池标称总能量  */
        uint16_t bms_allow_volt_max;             /* BMS 最高允许充电总电压  */
        int8_t bms_allow_temp_max;               /* BMS 最高允许温度  */
        uint16_t soc;                            /* SOC  */
        uint16_t bms_bat_current_volt;           /* BMS 整车动力蓄电池当前电池电压  */
        uint16_t pile_output_volt_max;           /* 电桩最高输出电压  */
        uint16_t pile_output_volt_min;           /* 电桩最低输出电压  */
        int16_t pile_output_curr_max;            /* 电桩最大输出电流  */
        int16_t pile_output_curr_min;            /* 电桩最小输出电流  */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ParameterConfig_t;

/** 0x19 充电结束帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t bms_end_soc;                     /* 结束SOC */
        uint16_t bms_single_bat_volt_min;        /* BMS 动力蓄电池单体最低电压 */
        uint16_t bms_single_bat_volt_max;        /* BMS 动力蓄电池单体最高电压 */
        uint8_t bms_bat_temp_min;                /* BMS 动力蓄电池最低温度 */
        uint8_t bms_bat_temp_max;                /* BMS 动力蓄电池最高温度 */
        uint16_t total_charge_time;              /* 电桩累计充电时间 */
        uint16_t charge_elect;                   /* 电桩输出能量 */
        uint8_t charger_number[4];               /* 电桩充电机编号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ChargeFinish_t;

/** 0x1B 错误帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        struct{
            uint8_t spn2560_00_identify         : 2;   /* 接收 SPN2560=0x00 的充电机辨识报文超时 */
            uint8_t spn2560_aa_identify         : 2;   /* 接收 SPN2560=0xAA 的充电机辨识报文超时 */
            uint8_t reserve_1                   : 4;   /* 保留 */
            uint8_t charger_sync_and_output_max : 2;   /* 接收充电机的时间同步和充电机最大输出能力报文超时 */
            uint8_t charger_charge_ready_ok     : 2;   /* 接收充电机完成充电准备报文超时 */
            uint8_t reserve_2                   : 4;   /* 保留 */
            uint8_t charger_charge_state        : 2;   /* 接收充电机充电状态报文超时 */
            uint8_t charger_end_charge          : 2;   /* 接收充电机中止充电报文超时 */
            uint8_t reserve_3                   : 4;   /* 保留 */
            uint8_t charger_charge_statistics   : 2;   /* 接收充电机充电统计报文超时 */
            uint8_t bms_others                  : 6;   /* BMS 其他 */
            uint8_t bms_and_car                 : 2;   /* 接收 BMS 和车辆的辨识报文超时 */
            uint8_t reserve_4                   : 6;   /* 保留 */
            uint8_t bat_parameter               : 2;   /* 接收电池充电参数报文超时 */
            uint8_t bms_charge_ready_ok         : 2;   /* 接收 BMS 完成充电准备报文超时 */
            uint8_t reserve_5                   : 4;   /* 保留 */
            uint8_t bat_state_all               : 2;   /* 接收电池充电总状态报文超时 */
            uint8_t bat_charge_conmand          : 2;   /* 接收电池充电要求报文超时 */
            uint8_t bms_end_charge              : 2;   /* 接收 BMS 中止充电报文超时 */
            uint8_t reserve_6                   : 2;   /* 保留 */
            uint8_t bms_charge_count            : 2;   /* 接收 BMS 充电统计报文超时 */
            uint8_t charger_others              : 6;   /* 充电机其他 */
        }timeout;
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ErrorMessage_t;

/** 0x1D 充电过程中 BMS 终止帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        struct{
            uint8_t reach_soc                     : 2;   /* 所需求的 SOC 目标值 */
            uint8_t reach_volt_all                : 2;   /* 达到总电压的设定值 */
            uint8_t reach_single_volt             : 2;   /* 达到单体电压设定值 */
            uint8_t charger_end                   : 2;   /* 充电机主动中止 */

            uint8_t fault_insulation              : 2;   /* 绝缘故障 */
            uint8_t fault_out_linker_overtemp     : 2;   /* 输出连接器过温故障 */
            uint8_t fault_bms_out_linker_overtemp : 2;   /* BMS 元件、输出连接器过温 */
            uint8_t fault_charge_linker           : 2;   /* 充电连接器故障 */
            uint8_t fault_bat_group_overtemp      : 2;   /* 电池组温度过高故障 */
            uint8_t fault_high_volt_relay         : 2;   /* 高压继电器故障 */
            uint8_t fault_test_2_point_volt       : 2;   /* 检测点 2 电压检测故障 */
            uint8_t fault_other                   : 2;   /* 其他故障 */

            uint8_t overcurr                      : 2;   /* 电流过大 */
            uint8_t volt_abnormal                 : 2;   /* 电压异常*/
            uint8_t reserve                       : 4;   /* 预留位 */
        }reason;
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_BmsEnd_t;

/** 0x21 充电过程中充电机终止帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        struct{
            uint8_t reach_set_adition               : 2; /* 达到充电机设定的条件中止 */
            uint8_t manual_end                      : 2; /* 人工中止 */
            uint8_t exception_end                   : 2; /* 异常中止 */
            uint8_t bms_end                         : 2; /* BMS 主动中止 */

            uint8_t fault_charger_overtemp          : 2; /* 充电机过温故障 */
            uint8_t fault_charge_linker             : 2; /* 充电连接器故障 */
            uint8_t fault_charger_internal_overtemp : 2; /* 充电机内部过温故障 */
            uint8_t fault_elect_can_not_report      : 2; /* 所需电量不能传送 */
            uint8_t fault_charger_scram             : 2; /* 充电机急停故障 */
            uint8_t fault_other                     : 2; /* 其他故障 */
            uint8_t fault_reserve                   : 4; /* 预留位 */

            uint8_t curr_no_match                   : 2; /* 电流不匹配 */
            uint8_t volt_exception                  : 2; /* 电压异常 */
            uint8_t reserve                         : 4; /* 预留位 */
        }reason;
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ChargerEnd_t;

/** 0x23 充电过程 BMS 需求与充电机输出帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint16_t bms_volt_command;                       /* BMS 电压需求 */
        int16_t bms_curr_command;                        /* BMS 电流需求 */
        uint8_t  bms_charge_mode;                        /* BMS 充电模式 */
        uint16_t bms_volt_measure_value;                 /* BMS 充电电压测量值  */
        int16_t bms_curr_measure_value;                  /* BMS 充电电流测量值  */
        struct{
            uint16_t bms_max_single_bat_volt : 12;       /* BMS 最高单体动力蓄电池电压及组号 */
            uint16_t max_single_bat_volt_gn : 4;         /*  */
        }max_volt_and_gn;
        uint8_t  bms_current_soc;                        /* MS 当前荷电状态 SOC*/
        uint16_t bms_remain_charge_time;                 /* BMS 估算剩余充电时间 */
        uint16_t pile_output_volt;                       /* 电桩电压输出值 */
        uint16_t pile_output_curr;                       /* 电桩电流输出值 */
        uint16_t charge_time;                            /* 累计充电时间 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_BmsCommand_ChargerOut_t;

/** 0x25 充电过程 BMS 信息帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t bms_max_single_volt_bat_number;          /* BMS 最高单体动力蓄电池电压所在编号 */
        uint8_t bms_bat_temp_max;                        /* BMS 最高动力蓄电池温度 */
        uint8_t bat_temp_max_measure_number;             /* 最高温度检测点编号 */
        uint8_t bms_bat_temp_min;                        /* 最低动力蓄电池温度 */
        uint8_t bat_temp_min_measure_number;             /* 最低动力蓄电池温度检测点编号 */
        struct{
            uint8_t bms_single_volt       : 2;           /* BMS 单体动力蓄电池电压过高/过低 */
            uint8_t bms_bat_soc           : 2;           /* BMS 整车动力蓄电池荷电状态 SOC 过高/过低 */
            uint8_t bms_bat_curr          : 2;           /* BMS 动力蓄电池充电过电流 */
            uint8_t bms_bat_temp          : 2;           /* BMS 动力蓄电池温度过高 */
            uint8_t bms_bat_isolate       : 2;           /* BMS 动力蓄电池绝缘状态 */
            uint8_t bms_bat_output_linker : 2;           /* BMS 动力蓄电池组输出连接器连接状态 */
            uint8_t charge_forbid         : 2;           /* 充电禁止 */
            uint8_t reserve               : 2;           /* 预留位  */
        }state;
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_BmsInfo_t;

/** 0x31 充电桩主动申请启动充电帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t start_type;                              /* 启动方式 */
        uint8_t whether_password;                        /* 是否需要密码 */
        uint8_t account_or_phycard_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint8_t password[NET_YKC_PASSWORD_LENGTH_DEFAULT];   /* 输入密码 */
        uint8_t vin[NET_YKC_CAR_VIN_NUMBER_LENGTH_MAX];      /* VIN码 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ApplyCharge_Active_t;

/** 0x32 充电桩主动申请启动充电响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 逻辑卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint8_t authentication_success;                  /* 鉴权成功标志 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
}Net_YkcPro_SRes_ApplyCharge_Active_t;

/** 0x34 运营平台远程控制启机帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];    /* 逻辑卡号 */
        uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_Remote_StartCharge_t;

/** 0x33 运营平台远程控制启机响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Remote_StartCharge_t;

/** 0x36 运营平台远程停机帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_Remote_StopCharge_t;

/** 0x35 运营平台远程停机响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Remote_StopCharge_t;

/** 0x3B 交易记录帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        cp56time2a_t start_time;                         /* 开始时间 */
        cp56time2a_t stop_time;                          /* 结束时间 */
        uint32_t tip_unit_price;                         /* 尖单价 */
        uint32_t tip_elect;                              /* 尖电量 */
        uint32_t tip_loss_elect;                         /* 尖计损电量 */
        uint32_t tip_amount;                             /* 尖金额 */
        uint32_t peak_unit_price;                        /* 峰单价 */
        uint32_t peak_elect;                             /* 峰电量 */
        uint32_t peak_loss_elect;                        /* 峰计损电量 */
        uint32_t peak_amount;                            /* 峰金额 */
        uint32_t flat_unit_price;                        /* 平单价 */
        uint32_t flat_elect;                             /* 平电量 */
        uint32_t flat_loss_elect;                        /* 平计损电量 */
        uint32_t flat_amount;                            /* 平金额 */
        uint32_t valley_unit_price;                      /* 谷单价 */
        uint32_t valley_elect;                           /* 谷电量 */
        uint32_t valley_loss_elect;                      /* 谷计损电量 */
        uint32_t valley_amount;                          /* 谷金额 */
        uint8_t ammeter_start_val[5];                    /* 电表总起值 */
        uint8_t ammeter_end_val[5];                      /* 电表总止值 */
        uint32_t total_elect;                            /* 总电量 */
        uint32_t total_loss_elect;                       /* 计损总电量 */
        uint32_t consume_amount;                         /* 消费金额 */
        uint8_t vin[NET_YKC_CAR_VIN_NUMBER_LENGTH_MAX];  /* VIN 码 */
        uint8_t transaction_identity;                    /* 交易标识 */
        cp56time2a_t transaction_date;                   /* 交易日期 */
        uint8_t stop_reason;                             /* 停止原因 */
        uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_TransactionRecords_t;

/** 0x40 交易记录响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t result;                                  /* 确认结果 */
    }body;
}Net_YkcPro_SRes_TransactionRecords_t;

/** 0x42 远程账户余额更新帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
        uint32_t account_amount;                         /* 修改后账户金额 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_AccountBallance_Update_t;

/** 0x41 远程账户余额更新响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_AccountBallance_Update_t;

/** 0x44 离线卡数据同步帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t count;                                   /* 下发卡个数 */
        uint16_t result;
        /** 以下为卡的物理卡号数据 */
    }body;
}Net_YkcPro_SReq_Sync_OfflineCard_t;

/** 0x43 离线卡数据同步响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Sync_OfflineCard_t;

/** 0x46 离线卡数据清除帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t count;                                   /* 清除离线卡的个数 */
        uint16_t result;
        /** 以下为卡的物理卡号数据 */
    }body;
}Net_YkcPro_SReq_Clear_OfflineCard_t;

/** 0x45 离线卡数据清除响应帧 */
struct clear_card_block{
    uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
    uint8_t result;                                  /* 结果 */
    uint8_t fail_reason;                             /* 失败原因 */
};

typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        /** 以下为离线卡清除结果信息 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Clear_OfflineCard_t;

/** 0x48 离线卡数据查询帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t count;                                   /* 查询的离线卡个数 */
        uint16_t result;
        /** 卡数据 */
    }body;
}Net_YkcPro_SReq_Query_OfflineCard_t;

/** 0x47 离线卡数据查询响应帧 */
struct query_card_block{
    uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
    uint8_t result;                                  /* 结果 */
};

typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        /* 以下为离线卡查询结果信息 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Query_OfflineCard_t;

/** 0x52 充电桩工作参数设置帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t forbidden;                               /* 锁定桩(禁用) */
        uint8_t power_max_percent;                       /* 充电桩最大允许输出功率百分比 */
    }body;
}Net_YkcPro_SReq_Set_WorkPara_t;

/** 0x51 充电桩工作参数设置响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Set_WorkPara_t;

/** 0x56 对时设置帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        cp56time2a_t current_time;                       /* 当前时间 */
    }body;
}Net_YkcPro_SReq_TimeSync_t;

/** 0x55 对时设置响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        cp56time2a_t current_time;                       /* 当前时间 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_TimeSync_t;

/** 0x58 计费模型设置帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t model_number;                   /* 计费模型编号 */
        uint32_t tip_elect_rate;                 /* 尖电费费率 */
        uint32_t tip_service_rate;               /* 尖服务费费率 */
        uint32_t peak_elect_rate;                /* 峰电费费率 */
        uint32_t peak_service_rate;              /* 峰服务费费率 */
        uint32_t flat_elect_rate;                /* 平电费费率 */
        uint32_t flat_service_rate;              /* 平服务费费率 */
        uint32_t valley_elect_rate;              /* 谷电费费率 */
        uint32_t valley_service_rate;            /* 谷服务费费率 */
        uint8_t loss_proportion;                 /* 计损比例 */
        uint8_t rate_number[NET_YKC_RATE_PERIOD_COUNT_MAX];     /* 计损比例 */
    }body;
}Net_YkcPro_SRep_BillingModel_Set_t;

/** 0x57 计费模型设置响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_BillingModel_Set_t;

/** 0x61 地锁数据上送帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t groundlock_state;                        /* 地锁状态 */
        uint8_t packingspace_state;                      /* 车位状态 */
        uint8_t groundlock_power;                        /* 地锁能量百分比 */
        uint8_t warnning_state;                          /* 报警状态 */
        uint32_t reserve;                                /* 保留 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_GroundLock_Info_t;

/** 0x62 遥控地锁升锁与降锁命令帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t control_cmd;                             /* 升/降地锁 */
        uint32_t reserve;                                /* 保留 */
    }body;
}Net_YkcPro_SReq_GroundLock_Lifting_t;

/** 0x63 遥控地锁升锁与降锁命令响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint32_t reserve;                                /* 保留 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_GroundLock_Lifting_t;

/** 0x92 远程重启帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t control_cmd;                             /* 控制指令 */
    }body;
}Net_YkcPro_SReq_RemoteReboot_t;

/** 0x91 远程重启响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_RemoteReboot_t;

/** 0x94 远程更新帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t pile_model;                              /* 桩型号 */
        uint16_t pile_power;                             /* 桩功率 */
        uint8_t server_addr[NET_YKC_SERVER_ADDR_LENGTH_DEFAULT];    /* 服务器地址 */
        uint16_t server_port;                            /* 服务器端口 */
        uint8_t user_name[NET_YKC_USER_NAME_LENGTH_DEFAULT];        /* 用户名 */
        uint8_t password[NET_YKC_PASSWORD_LENGTH_DEFAULT];          /* 密码 */
        uint8_t path[NET_YKC_FILE_PATH_LENGTH_DEFAULT];             /* 升级文件路径 */
        uint8_t control_cmd;                             /* 升级控制 */
        uint8_t timeout_value;                           /* 升级超时时间 */
        uint16_t result;                                 /* 处理结果 */
    }body;
}Net_YkcPro_SReq_RemoteUpdate_t;

/** 0x93 远程更新响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_RemoteUpdate_t;

/** 0xA1 充电桩主动申请并充充电帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t start_type;                              /* 启动方式 */
        uint8_t whether_password;                        /* 是否需要密码 */
        uint8_t account_or_phycard_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint8_t password[NET_YKC_PASSWORD_LENGTH_DEFAULT];   /* 输入密码 */
        uint8_t vin[NET_YKC_CAR_VIN_NUMBER_LENGTH_MAX];      /* VIN码 */
        uint8_t main_auxiliary_gun_flag;                     /* 主辅枪标记 */
        uint8_t merge_charge_sn[NET_YKC_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_ApplyMergeCharge_Active_t;

/** 0xA2 充电桩主动申请并充充电响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 逻辑卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint8_t authentication_success;                  /* 鉴权成功标志 */
        uint8_t fail_reason;                             /* 失败原因 */
        uint8_t merge_charge_sn[NET_YKC_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
    }body;
}Net_YkcPro_SRes_ApplyMergeCharge_Active_t;

/** 0xA4 运营平台远程控制并充启机帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];    /* 逻辑卡号 */
        uint8_t physics_card_number[NET_YKC_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint8_t merge_charge_sn[NET_YKC_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_Remote_StartMergeCharge_t;

/** 0xA3 运营平台远程控制并充启机响应帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
        uint8_t main_auxiliary_gun_flag;                 /* 主辅枪标记 */
        uint8_t merge_charge_sn[NET_YKC_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Remote_StartMergeCharge_t;

/** 0x5A 云服务器二维码配置请求帧(国充) */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        /** 二维码数据 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_Qrcode_Config_GC_t;

/** 0x59 充电桩二维码配置应答帧(国充) */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Qrcode_Config_GC_t;

/** 0xF0 云服务器二维码配置请求帧(云快充1.5) */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t format;                          /* 格式 */
        uint8_t length;                          /* 长度 */
        /** 二维码数据 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_Qrcode_Config_Ykc15_t;

/** 0xF1 充电桩二维码配置应答帧(云快充1.5) */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Qrcode_Config_Ykc15_t;

/** 0x9C 云服务器二维码配置请求帧(特来电) */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号*/
        uint16_t length;                         /* 长度 */
        /** 二维码数据 */
        uint16_t result;
    }body;
}Net_YkcPro_SReq_Qrcode_Config_Tld_t;

/** 0x9B 充电桩二维码配置应答帧(特来电) */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号*/
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PRes_Qrcode_Config_Tld_t;



#ifdef NET_YKC_MESSAGE_USING_DUPU
/** 0x1F 充电桩上报储能信息请求帧 */
typedef struct{
    Net_YkcPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t soc;                            /* SOC */
        uint16_t power;                          /* 功率 */
        uint16_t voltage;                        /* 电压 */
        uint16_t current;                        /* 电流 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcPro_PReq_StoredEnergy_Info_t;
#endif /* NET_YKC_MESSAGE_USING_DUPU */

#pragma pack()

#endif /* NET_PACK_USING_YKC */

#endif /* NET_PACK_NET_YKC_INC_YKC_MESSAGE_STRUCT_DEFINE_H_ */
