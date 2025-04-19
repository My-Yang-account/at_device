/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-06     我的杨yang       the first version
 */
#ifndef NET_NET_YCP_MESSAGE_STRUCT_DEFINE_H_
#define NET_NET_YCP_MESSAGE_STRUCT_DEFINE_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YCP

#define NET_YCP_MESSAGE_START_CODE                             0xAA        /* 报文起始码 */
#define NET_YCP_MESSAGE_ENCRYPT_ENABLE                         0x01        /* 报文加密 */
#define NET_YCP_MESSAGE_ENCRYPT_DISABLE                        0x00        /* 报文不加密 */
#define NET_YCP_PROTOCOL_VERSION_MAIN                          0x01        /* 协议主版本号 */
#define NET_YCP_PROTOCOL_VERSION_SUB                           0x00        /* 协议子版本号 */
#define NET_YCP_PROTOCOL_VERSION_REV                           0x00        /* 协议修订版本号  */

#define NET_YCP_PROTOCOL_CHECK_REGION_SIZE                     0x02        /* 校验码域长度：单位字节 */

#define NET_YCP_NET_LINK_TYPE_SIM                              0x01        /* 网络链接类型：SIM 卡 */
#define NET_YCP_NET_LINK_TYPE_LAN                              0x02        /* 网络链接类型：LAN */
#define NET_YCP_NET_LINK_TYPE_WAN                              0x03        /* 网络链接类型：WAN */
#define NET_YCP_NET_LINK_TYPE_OTHER                            0x04        /* 网络链接类型：其它 */

#define NET_YCP_OPERATOR_MOBILE                                0x01        /* 运营商名称：中国移动 */
#define NET_YCP_OPERATOR_TELECOM                               0x02        /* 运营商名称：中国电信 */
#define NET_YCP_OPERATOR_UNICOM                                0x03        /* 运营商名称：中国联通 */
#define NET_YCP_OPERATOR_OTHER                                 0x04        /* 运营商名称：其它 */

#define NET_YCP_PILE_TYPE_DC                                   0x00        /* 桩类型：直流 */
#define NET_YCP_PILE_TYPE_AC                                   0x01        /* 桩类型：交流 */

#define NET_YCP_CHARGEPILE_LENGTH_DEFAULT                      0x07        /* 默认桩号长度 */
#define NET_YCP_SOFTWARE_LENGTH_DEFAULT                        0x08        /* 默认软件版本长度 */
#define NET_YCP_SIM_BCD_LENGTH_DEFAULT                         0x0A        /* 默认sim卡卡号BCD码长度 */
#define NET_YCP_COMMUNICATE_MODULE_SN_LENGTH_DEFAULT           0x08        /* 默认通信模块绑定设备编号长度 */
#define NET_YCP_NETWORK_KEYS_LENGTH_DEFAULT                    0x10        /* 默认入网密钥长度 */
#define NET_YCP_TIME_BCD_LENGTH_DEFAULT                        0x07        /* 默认时间BCD长度 */
#define NET_YCP_RATE_PERIOD_COUNT_MAX                          0x30        /* 费率时段数 */
#define NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT                   0x10        /* 默认流水号长度 */
#define NET_YCP_GUNLINE_NUMBER_LENGTH_DEFAULT                  0x08        /* 默认枪线编号长度 */
#define NET_YCP_PHYCARD_NUMBER_LENGTH_MAX                      0x10        /* 物理卡号长度最大值 */
#define NET_YCP_LOGICCARD_NUMBER_LENGTH_MAX                    0x10        /* 逻辑卡号长度最大值 */
#define NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX                      0x11        /* 车VIN码长度最大值 */
#define NET_YCP_INPUT_PASSWORD_LENGTH_DEFAULT                  0x10        /* 默认输入密码长度 */
#define NET_YCP_USER_NAME_LENGTH_DEFAULT                       0x10        /* 默认用户名长度 */
#define NET_YCP_PASSWORD_LENGTH_DEFAULT                        0x10        /* 默认密码长度 */
#define NET_YCP_SERVER_ADDR_LENGTH_DEFAULT                     0x40        /* 默认服务器地址长度 */
#define NET_YCP_FILE_PATH_LENGTH_DEFAULT                       0x20        /* 默认文件路径长度 */
#define NET_YCP_QRCODE_LENGTH_DEFAULT                          0x96        /* 默认二维码长度 */

#define NET_YCP_STORAGE_INIT_FLAG                              0x12345678  /* 平台数据存储标志 */
#define NET_YCP_HEARTBEAT_INTERVAL_DEFAULT                     0x3C        /* 默认心跳上报间隔 */
#define NET_YCP_VOICE_VOLUME_DEFAULT                           0x32        /* 默认语音音量 */
#define NET_YCP_DEVICE_PASSWORD_DEFAULT                        0x00B2      /* 默认设备密码 */
#define NET_YCP_TEMP_PROTECT_DEFAULT                           0xFF        /* 默认温度保护值 */
#define NET_YCP_CHARGEDATA_TYPE_DEFAULT                        0x02        /* 默认充电数据上报类型 */
#define NET_YCP_BMSDATA_SWITCH_DEFAULT                         0x00        /* 默认BMS 数据上报开关 */
#define NET_YCP_POWER_PERCENT_DEFAULT                          0x64        /* 默认输出功率百分比 */
#define NET_YCP_MODEL_NUMBER_DEFAULT                           0x00        /* 默认计费模型编号 */

#define NET_YCP_PARASET_RESULT_SUCCESS                         0x00        /* 参数设置结果：成功 */
#define NET_YCP_PARASET_RESULT_FAIL_HEARTBEAT                  0x01        /* 参数设置结果：心跳参数异常 */
#define NET_YCP_PARASET_RESULT_FAIL_VOLUME                     0x02        /* 参数设置结果：音量参数异常 */
#define NET_YCP_PARASET_RESULT_FAIL_PASSWORD                   0x03        /* 参数设置结果：密码参数异常 */
#define NET_YCP_PARASET_RESULT_FAIL_TEMPPRO                    0x04        /* 参数设置结果：温度保护纸参数异常 */
#define NET_YCP_PARASET_RESULT_FAIL_CHARGEDATA_TYPE            0x05        /* 参数设置结果：充电数据上报类型参数异常 */
#define NET_YCP_PARASET_RESULT_FAIL_BMS_SWITCH                 0x06        /* 参数设置结果：BMS 数据上报开关参数异常 */
#define NET_YCP_PARASET_RESULT_FAIL_POWER                      0x07        /* 参数设置结果：输出功率参数异常 */

#define NET_YCP_CHARGE_STRATEGY_TIME                           0x01        /* 充电策略：按时间 */
#define NET_YCP_CHARGE_STRATEGY_ELECT                          0x02        /* 充电策略：按电量 */
#define NET_YCP_CHARGE_STRATEGY_MONEY                          0x03        /* 充电策略：按金额 */
#define NET_YCP_CHARGE_STRATEGY_FULL                           0x04        /* 充电策略：充满 */

enum ycp_start_way{
    NETYCP_START_WAY_APP = 0x01,                             /* 启动方式：APP */
    NETYCP_START_WAY_SWIP_CARD = 0x02,                       /* 启动方式：刷卡 */
    NETYCP_START_WAY_PASSWORD = 0x03,                        /* 启动方式：密码 */
    NETYCP_START_WAY_VIN = 0x04,                             /* 启动方式：VIN */
    NETYCP_START_WAY_BLUE = 0x05,                            /* 启动方式：蓝牙 */
};

/* CC as CHARGE COMPLETE */
enum ycp_charge_complete{
    NETYCP_CC_REASON01_APP = 0x01,                           /* 充电完成原因：APP 远程停止 */
    NETYCP_CC_REASON02_CHARGE_FULL = 0x02,                   /* 充电完成原因：达到 100% */
    NETYCP_CC_REASON03_TARGET_ELECT = 0x03,                  /* 充电完成原因：充电电量满足设定条件 */
    NETYCP_CC_REASON04_TARGET_MONEY = 0x04,                  /* 充电完成原因：充电金额满足设定条件 */
    NETYCP_CC_REASON05_TARGET_TIME = 0x05,                   /* 充电完成原因：充电时间满足设定条件 */
    NETYCP_CC_REASON06_SWIP_CARD = 0x06,                     /* 充电完成原因：刷卡停止充电 */
    NETYCP_CC_REASON07_RESERVE = 0x07,                       /* 充电完成原因：其他方式（预留） */
};

/* SF as START FAIL */
enum ycp_start_fail{
    NETYCP_SF_REASON10_SYSCONTROL_FAULT = 0x10,              /* 充电启动失败，充电桩控制系统故障(需要重启或自动恢复) */
    NETYCP_SF_REASON11_GUIDE_DISCONNECT = 0x11,              /* 充电启动失败，控制导引断开 */
    NETYCP_SF_REASON12_CIRCUIT_BREAKER = 0x12,               /* 充电启动失败，断路器跳位 */
    NETYCP_SF_REASON13_AMMETER_COMMUNICATION = 0x13,         /* 充电启动失败，电表通信中断 */
    NETYCP_SF_REASON14_NO_BALLANCE = 0x14,                   /* 充电启动失败，余额不足 */
    NETYCP_SF_REASON15_CHARGE_MODULE = 0x15,                 /* 充电启动失败，充电模块故障 */
    NETYCP_SF_REASON16_EMERGENCY_STOP = 0x16,                /* 充电启动失败，急停开入 */
    NETYCP_SF_REASON17_LPS_FAULT = 0x17,                     /* 充电启动失败，防雷器异常 */
    NETYCP_SF_REASON18_BMS_UNREADY = 0x18,                   /* 充电启动失败，BMS 未就绪 */
    NETYCP_SF_REASON19_ABNORMAL_TEMP = 0x19,                 /* 充电启动失败，温度异常 */
    NETYCP_SF_REASON1A_BATTERY_REVERSE = 0x1A,               /* 充电启动失败，电池反接故障 */
    NETYCP_SF_REASON1B_ELECT_LOCK = 0x1B,                    /* 充电启动失败，电子锁异常 */
    NETYCP_SF_REASON1C_CLOSE_FAIL = 0x1C,                    /* 充电启动失败，合闸失败 */
    NETYCP_SF_REASON1D_INSULATION_ABNORMAL = 0x1D,           /* 充电启动失败，绝缘异常 */
    NETYCP_SF_REASON1E_RECV_BHM_TIMEOUT = 0x1E,              /* 充电启动失败，接收 BMS 握手报文 BHM 超时 */
    NETYCP_SF_REASON1F_RECV_BRM_TIMEOUT = 0x1F,              /* 充电启动失败，接收 BMS 和车辆的辨识报文超时 BRM */
    NETYCP_SF_REASON20_RECV_BCP_TIMEOUT = 0x20,              /* 充电启动失败，接收电池充电参数报文超时 BCP */
    NETYCP_SF_REASON21_RECV_BRO_AA_TIMEOUT = 0x21,           /* 充电启动失败，接收 BMS 完成充电准备报文超时 BRO AA */
    NETYCP_SF_REASON22_RECV_BCS_TIMEOUT = 0x22,              /* 充电启动失败，接收电池充电总状态报文超时 BCS  */
    NETYCP_SF_REASON23_RECV_BCL_TIMEOUT = 0x23,              /* 充电启动失败，接收电池充电要求报文超时 BCL */
    NETYCP_SF_REASON24_RECV_BSM_TIMEOUT = 0x24,              /* 充电启动失败，接收电池状态信息报文超时 BSM */
    NETYCP_SF_REASON25_BHM_STAGE_VOLT_OVERRANGE = 0x25,      /* 充电启动失败，GB2015 电池在 BHM 阶段有电压不允许充电 */
    NETYCP_SF_REASON26_BRO_AA_STAGE_VOLT_OVERRANGE = 0x26,   /* 充电启动失败，GB2015 辨识阶段在 BRO_AA 时候电池实际电压与 BCP 报文电池电压差距大于 5% */
    NETYCP_SF_REASON27_BRO_AA_TO_BRO_00 = 0x27,              /* 充电完成原因：充电启动失败，B2015 充电机在预充电阶段从 BRO_AA 变成BRO_00 状态 */
    NETYCP_SF_REASON28_RECV_HOST_CONFIG_MESSAGE_TIMEOUT = 0x28, /* 充电启动失败，接收主机配置报文超时 */
    NETYCP_SF_REASON29_CHARGER_UNREADY = 0x29,               /* 充电启动失败：充电启动失败，充电机未准备就绪,我们没有回 CRO AA，对应老国标 */
    NETYCP_SF_REASON2A_DC_RELAY = 0x2A,                      /* 充电启动失败，DC继电器故障 */
    NETYCP_SF_REASON2B_AUXPOWER = 0x2B,                      /* 充电启动失败，辅助电源故障 */
    NETYCP_SF_REASON2C_READY_VOLTAGE = 0x2C,                 /* 充电异常中止，准备电压 */
    NETYCP_SF_REASON2D_INSULT_VOLTAGE = 0x2D,                /* 充电异常中止，绝缘电压 */
    NETYCP_SF_REASON2E_RESERVE1 = 0x2E,                      /* （其他原因）预留 */
};

/* AS as ABNORMAL STOP */
enum ycp_abnormal_stop{
    NETYCP_AS_REASON50_SYSTEM_ATRESIA = 0x50,                /* 充电异常中止，系统闭锁 */
    NETYCP_AS_REASON51_GUIDANCE_DISCONNECT = 0x51,           /* 充电异常中止，导引断开 */
    NETYCP_AS_REASON52_CIRCUIT_BREAKER_ACTION = 0x52,        /* 充电异常中止，断路器跳位 */
    NETYCP_AS_REASON53_AMMETER_COMMUNICATE = 0x53,           /* 充电异常中止，电表通信中断 */
    NETYCP_AS_REASON54_AC_PROTECT_ACTION = 0x54,             /* 充电异常中止，交流保护动作 */
    NETYCP_AS_REASON55_DC_PROTECT_ACTION = 0x55,             /* 充电异常中止，直流保护动作 */
    NETYCP_AS_REASON56_CHARGE_MODULE = 0x56,                 /* 充电异常中止，充电模块故障 */
    NETYCP_AS_REASON57_EMERGENCY_STOP = 0x57,                /* 充电异常中止，急停开入 */
    NETYCP_AS_REASON58_LIGHTNING_PROTECTORS = 0x58,          /* 充电异常中止，防雷器异常 */
    NETYCP_AS_REASON59_TEMPERATURE_ABNORMAL = 0x59,          /* 充电异常中止，温度异常 */
    NETYCP_AS_REASON5A_OUTPUT_ABNORMAL = 0x5A,               /* 充电异常中止，输出异常 */
    NETYCP_AS_REASON5B_CURRENT_ABNORMAL = 0x5B,              /* 充电异常中止，充电无流 */
    NETYCP_AS_REASON5C_ELOCK_ABNORMAL = 0x5C,                /* 充电异常中止，电子锁异常 */
    NETYCP_AS_REASON5D_CHARGE_TVOLTAGE_ABNORMAL = 0x5D,      /* 充电异常中止，总充电电压异常 */
    NETYCP_AS_REASON5E_CHARGE_TCURRENT_ABNORMAL = 0x5E,      /* 充电异常中止，总充电电流异常 */
    NETYCP_AS_REASON5F_CHARGE_SVOLTAGE_ABNORMAL = 0x5F,      /* 充电异常中止，单体充电电压异常 */
    NETYCP_AS_REASON60_BATTERY_GROUP_OVERTEMP = 0x60,        /* 充电异常中止，电池组过温 */
    NETYCP_AS_REASON61_CHARGE_SVOLTAGE_MAX_ABNORMAL = 0x61,  /* 充电异常中止，最高单体充电电压异常 */
    NETYCP_AS_REASON62_BGROUP_MTEMP_OVERTEMP = 0x62,         /* 充电异常中止，最高电池组过温 */
    NETYCP_AS_REASON63_BMV_CHARGE_SVOLTAGE_ABNORMAL = 0x63,  /* 充电异常中止，BMV 单体充电电压异常 */
    NETYCP_AS_REASON64_BMT_BGROUP_OVERTEMP = 0x64,           /* 充电异常中止，BMT 电池组过温 */
    NETYCP_AS_REASON65_BSTATE_ABNORMAL = 0x65,               /* 充电异常中止，电池状态异常停止充电 */
    NETYCP_AS_REASON66_CAR_COMMAND_STOP = 0x66,              /* 充电异常中止，车辆发报文禁止充电 */
    NETYCP_AS_REASON67_POWER_OFF = 0x67,                     /* 充电异常中止，充电桩断电 */
    NETYCP_AS_REASON68_RECV_BCS_TIMEOUT = 0x68,              /* 充电异常中止，接收电池充电总状态报文超时 */
    NETYCP_AS_REASON69_RECV_BCL_TIMEOUT = 0x69,              /* 充电异常中止，接收电池充电要求报文超时 */
    NETYCP_AS_REASON6A_RECV_BSM_TIMEOUT = 0x6A,              /* 充电异常中止，接收电池状态信息报文超时 */
    NETYCP_AS_REASON6B_RECV_BST_TIMEOUT = 0x6B,              /* 充电异常中止，接收 BMS 中止充电报文超时 */
    NETYCP_AS_REASON6C_RECV_BSD_TIMEOUT = 0x6C,              /* 充电异常中止，接收 BMS 充电统计报文超时 */
    NETYCP_AS_REASON6D_RECV_CCS_TIMEOUT = 0x6D,              /* 充电异常中止，接收对侧 CCS 报文超时 */
    NETYCP_AS_REASON6E_RECV_CCS_TIMEOUT = 0x6E,              /* 充电异常中止，CP接地 */
    NETYCP_AS_REASON6F_CARDREADER = 0x6F,                    /* 充电异常中止，读卡器故障 */
    NETYCP_AS_REASON70_DC_RELAY = 0x70,                      /* 充电异常中止，DC继电器故障 */
    NETYCP_AS_REASON71_AC_RELAY = 0x71,                      /* 充电异常中止，AC继电器故障 */
    NETYCP_AS_REASON72_PARALLEL_RELAY = 0x72,                /* 充电异常中止，并联继电器故障 */
    NETYCP_AS_REASON73_STORAGE_CHIP = 0x73,                  /* 充电异常中止，存储芯片 */
    NETYCP_AS_REASON74_BSM_WARNNING = 0x74,                  /* 充电异常中止，BSM 告警 */
    NETYCP_AS_REASON75_GATE = 0x75,                          /* 充电异常中止，门禁故障 */
    NETYCP_AS_REASON76_BATTERY_VOLTAGE = 0x76,               /* 充电异常中止，电池电压 */
    NETYCP_AS_REASON77_VIN_AUTHEN_FAIL = 0x77,               /* 充电异常中止，VIN 码鉴权失败 */
    NETYCP_AS_REASON78_LIGHTPROTECT = 0x78,                  /* 充电异常中止，防雷器 */
    NETYCP_AS_REASON79_GUNSITE = 0x79,                       /* 充电异常中止，枪座 */
    NETYCP_AS_REASON7A_CIRCUIT_BREAKER = 0x7A,               /* 充电异常中止，断路器 */
    NETYCP_AS_REASON7B_FLOODING = 0x7B,                      /* 充电异常中止，水浸 */
    NETYCP_AS_REASON7C_SMOKE = 0x7C,                         /* 充电异常中止，烟感 */
    NETYCP_AS_REASON7D_POUR = 0x7D,                          /* 充电异常中止，倾倒 */
    NETYCP_AS_REASON7E_LIQUIDCOOLING = 0x7E,                 /* 充电异常中止，液冷 */
    NETYCP_AS_REASON7F_FUSE = 0x7F,                          /* 充电异常中止，熔断器 */
    NETYCP_AS_REASON9C_MAIN_CABINET_FORBID = 0x80,           /* 充电异常中止，主机柜禁止充电 */
    NETYCP_AS_REASON9D_YT_BFC = 0x81,                        /* 充电异常中止，宇通BFC */
    NETYCP_AS_REASON9D_MAIN_CABINET_FAULT = 0x82,            /* 充电异常中止，主机柜故障 */
    NETYCP_AS_REASONFF_UNKNOW = 0xFF,                        /* 充电异常中止，未知原因停止 */
};

enum ycp_cmd{
    NETYCP_PREQCMD_SINGIN = 0x01,                            /* 指令：桩登录请求 */
    NETYCP_SRESCMD_SINGIN = 0x02,                            /* 指令：服务器响应登录 */

    NETYCP_PREQCMD_TIME_SYNC = 0x03,                         /* 指令：桩对时强求 */
    NETYCP_SRESCMD_TIME_SYNC = 0x04,                         /* 指令：服务器响应对时请求 */

    NETYCP_PREQCMD_HEARTBEAT = 0x05,                         /* 指令：桩心跳请求 */
    NETYCP_SRESCMD_HEARTBEAT = 0x06,                         /* 指令：服务器响应心跳 */

    NETYCP_PREQCMD_BILLING_MODEL_VERIFY = 0x07,              /* 指令：桩请求计费模型验证 */
    NETYCP_SRESCMD_BILLING_MODEL_VERIFY = 0x08,              /* 指令：服务器响应计费模型验证 */

    NETYCP_SREQCMD_SET_PARA = 0x0A,                          /* 指令：服务器设置充电桩参数 */
    NETYCP_PRESCMD_SET_PARA = 0x09,                          /* 指令：桩响应设置充电桩参数 */

    NETYCP_SREQCMD_QRCODE_CONFIG = 0x0C,                     /* 指令：服务器设置二维码 */
    NETYCP_PRESCMD_QRCODE_CONFIG = 0x0B,                     /* 指令：桩响应设置二维码 */

    NETYCP_SREQCMD_SET_SERVER_PHONE = 0x0E,                  /* 指令：客服电话设置 */
    NETYCP_PRESCMD_SET_SERVER_PHONE = 0x0D,                  /* 指令：客服电话设置应答 */

    NETYCP_PRESCMD_BILLING_MODEL_SET = 0x11,                 /* 指令：计费模型应答 */
    NETYCP_SREQCMD_BILLING_MODEL_SET = 0x12,                 /* 指令：计费模型下发  */

    NETYCP_SREQCMD_QUERY_STATE_DATA = 0x14,                  /* 指令：获取单端口状态 */
    NETYCP_PREQCMD_PRESCMD_REPORT_STATE_DATA = 0x13,         /* 指令：单端口状态应答(端口状态上报)  */

    NETYCP_SREQCMD_QUERY_STATE_DATA_ALL = 0x16,              /* 指令：获取所有端口状态 */
    NETYCP_PREQCMD_PRESCMD_REPORT_STATE_DATA_ALL = 0x15,     /* 指令：所有端口状态应答(所有端口状态上报)  */

    NETYCP_SREQCMD_SERVER_START_CHARGE = 0x18,               /* 指令：远程开启充电 */
    NETYCP_PRESCMD_SERVER_START_CHARGE = 0x17,               /* 指令：远程开启充电应答 */

    NETYCP_PREQCMD_APPLY_START_CHARGE = 0x19,                /* 指令：充电桩主动请求充电 */
    NETYCP_SRESCMD_APPLY_START_CHARGE = 0x1A,                /* 指令：充电桩主动请求充电回复 */

    NETYCP_SREQCMD_SERVER_STOP_CHARGE = 0x1C,                /* 指令：远程结束充电 */
    NETYCP_PRESCMD_SERVER_STOP_CHARGE = 0x1B,                /* 指令：远程结束充电回复 */

    NETYCP_PREQCMD_TRANSACTION_RECORD = 0x1D,                /* 指令：充电结束报告上传 */
    NETYCP_SRESCMD_TRANSACTION_RECORD_VERIFY = 0x1E,         /* 指令：充电结束报告应答 */

    NETYCP_SREQCMD_REMOTE_UPDATE = 0x22,                     /* 指令：远程升级 */
    NETYCP_PRESCMD_REMOTE_UPDATE = 0x21,                     /* 指令：远程升级应答 */

    NETYCP_SREQCMD_REMOTE_REBOOT = 0x24,                     /* 指令：远程重启 */
    NETYCP_PRESCMD_REMOTE_REBOOT = 0x23,                     /* 指令：远程重启应答 */

    NETYCP_SREQCMD_MODIFY_SERVER_ADDR = 0x26,                /* 指令：远程修改联网地址 */
    NETYCP_PRESCMD_MODIFY_SERVER_ADDR = 0x25,                /* 指令：远程修改联网地址应答 */

    NETYCP_SREQCMD_QUERY_DEVICE_FAULT = 0x28,                /* 指令：获取当前设备故障信息 */
    NETYCP_PREQCMD_PRESCMD_REPORT_DEVICE_FAULT = 0x27,       /* 指令：设备故障上报(故障查询响应) */

    NETYCP_PREQCMD_CHARGE_SHAKE_HAND = 0x41,                 /* 指令：桩上报充电握手 */

    NETYCP_PREQCMD_PARA_CONFIG = 0x42,                       /* 指令：桩上报参数配置 */

    NETYCP_PREQCMD_CHARGE_END = 0x43,                        /* 指令：桩上报充电结束 */

    NETYCP_PREQCMD_ERROR_MESSAGE = 0x44,                     /* 指令：桩上报错误报文 */

    NETYCP_PREQCMD_BMS_STOP = 0x45,                          /* 指令：桩上报充电阶段 BMS 中止 */

    NETYCP_PREQCMD_CHARGER_STOP = 0x46,                      /* 指令：桩上报充电阶段充电机中止 */

    NETYCP_PREQCMD_CHARGER_OUTPUT_BMS_REQUIRE = 0x47,        /* 指令：桩上报充电过程 BMS 需求与充电机输出 */

    NETYCP_PREQCMD_BMS_INFO = 0x48,                          /* 指令：桩上报充电过程 BMS 信息 */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * YcpPro   as Ycp Protocol                   越城公用协议
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

/** 越城平台数据存储体 */
typedef struct{
    uint32_t storage_init_flag;              /* 存储初始化标志 */
    uint8_t verify_result;                   /* 数据校验结果 */

    uint16_t heartbeat_interval;             /* 心跳上报间隔 */
    uint8_t voice_volume;                    /* 语音音量 */
    uint16_t device_password;                /* 设备密码 */
    uint8_t temp_protect;                    /* 温度保护值 */
    uint8_t chargedata_type;                 /* 充电数据上报类型 */
    uint8_t bmsdata_switch;                  /* BMS 数据上报开关 */
    uint8_t power_percent;                   /* 输出功率 */

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
    uint8_t rate_number[NET_YCP_RATE_PERIOD_COUNT_MAX];     /* 费率号 */
}ycp_storage_struct;

/** 协议头部 */
typedef struct{
    uint8_t start_code;                           /* 起始码 */
    uint8_t flag;                                 /* 加密标志 */
    uint16_t length;                              /* 数据长度 */
    uint8_t sequence;                             /* 序列号 */
    uint8_t cmd;                                  /* 帧命令 */
}Net_YcpPro_Head_t;

/** 0x01 充电桩登录请求 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t pile_type;                       /* 桩类型 */
        uint8_t gun_count;                       /* 充电枪数量 */
        uint8_t protocol_ver[3];                 /* 通信协议版本 */
        uint8_t software_ver[NET_YCP_SOFTWARE_LENGTH_DEFAULT];  /* 软件版本 */
        uint8_t net_link_type;                   /* 网络连接类型 */
        uint8_t sim_number[NET_YCP_SIM_BCD_LENGTH_DEFAULT];     /* SIM 卡卡号 */
        uint8_t operators;                       /* 运营商 */
        uint8_t communicate_module_sn[NET_YCP_COMMUNICATE_MODULE_SN_LENGTH_DEFAULT]; /* 通信模块绑定设备编号 */
        uint8_t network_keys[NET_YCP_NETWORK_KEYS_LENGTH_DEFAULT]; /* 入网密钥 */
        uint8_t transmit_type;                   /* 传输类型 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_LogIn_t;

/** 0x02 登录认证应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 登录结果 */
        uint8_t device_sn_length;                /* 设备编号长度 */
        /* 以下为设备编号 */
    }body;
}Net_YcpPro_SRes_LogIn_t;

/** 0x03 对时请求 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t current_time[NET_YCP_TIME_BCD_LENGTH_DEFAULT];  /* 当前时间 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_TimeSync_t;

/** 0x04 对时请求应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        /** 报文已修改，去除了登录结果字段 */
//        uint8_t login_result;                        /* 登录结果 */
        uint8_t current_time[NET_YCP_TIME_BCD_LENGTH_DEFAULT];  /* 当前时间 */
    }body;
}Net_YcpPro_SRes_TimeSync_t;

/** 0x05 心跳上传 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t signal_value;                    /* 信号值 */
        uint16_t temp;                           /* 温度 */
        uint16_t input_voltage;                  /* 输入电压 */
        uint16_t input_current;                  /* 输入电流 */
        uint16_t output_voltage;                 /* 输出电压 */
        uint16_t output_current;                 /* 输出电流 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_HeartBeat_t;

/** 0x06 心跳应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
}Net_YcpPro_SRes_HeartBeat_t;

/** 0x07 计费模型验证请求 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint16_t model_sn;                       /* 计费模型编号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_BillingModel_Verify_t;

/** 0x08 计费模型验证应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
        uint16_t model_number;                       /* 计费模型编号 */
        uint32_t tip_elect_rate;                     /* 尖电费费率 */
        uint32_t tip_service_rate;                   /* 尖服务费费率 */
        uint32_t peak_elect_rate;                    /* 峰电费费率 */
        uint32_t peak_service_rate;                  /* 峰服务费费率 */
        uint32_t flat_elect_rate;                    /* 平电费费率 */
        uint32_t flat_service_rate;                  /* 平服务费费率 */
        uint32_t valley_elect_rate;                  /* 谷电费费率 */
        uint32_t valley_service_rate;                /* 谷服务费费率 */
        uint8_t loss_proportion;                     /* 计损比例 */
        uint8_t rate_number[NET_YCP_RATE_PERIOD_COUNT_MAX];     /* 费率号 */
    }body;
}Net_YcpPro_SRes_BillingModel_Verify_t;

/** 0x09 参数设置回复 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
        uint8_t reason;                          /* 原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_ParaSet_t;

/** 0x0A 参数设置 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint16_t heartbeat_interval;             /* 心跳上报间隔 */
        uint8_t voice_volume;                    /* 语音音量 */
        uint16_t device_password;                /* 设备密码 */
        uint8_t temp_protect;                    /* 温度保护值 */
        uint8_t chargedata_type;                 /* 充电数据上报类型 */
        uint8_t bmsdata_switch;                  /* BMS 数据上报开关 */
        uint8_t power_percent;                   /* 输出功率 */
    }body;
}Net_YcpPro_SReq_ParaSet_t;

/** 0x0B 二维码设置应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_Qrcode_Config_t;

/** 0x0C 二维码设置 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t qrcode_type;                     /* 二维码类型 */
        uint8_t qrcode_len;                      /* 二维码长度 */
        /* 以下是二维码内容 */
        uint16_t result;                         /* 结果 */
    }body;
}Net_YcpPro_SReq_Qrcode_Config_t;

/** 0x0D 客服电话设置应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_ServicePhone_t;

/** 0x0E 客服电话设置 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t service_phone_len;               /* 客服电话长度 */
        /* 以下是客服电话内容 */
        uint16_t result;                         /* 结果 */
    }body;
}Net_YcpPro_SReq_ServicePhone_t;

/** 0x11 计费模型应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_BillingModel_Set_t;

/** 0x12 计费模型下发 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
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
        uint8_t rate_number[NET_YCP_RATE_PERIOD_COUNT_MAX];     /* 费率号 */
    }body;
}Net_YcpPro_SReq_BillingModel_Set_t;

/** 0x13 桩状态(上报)应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t state;                           /* 状态 */
        uint8_t homing;                          /* 枪是否归位 */
        uint8_t plug_gun;                        /* 是否插枪 */
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号*/
        uint16_t output_voltage;                 /* 输出电压 */
        uint16_t output_current;                 /* 输出电流 */
        uint8_t gunline_number[NET_YCP_GUNLINE_NUMBER_LENGTH_DEFAULT]; /* 充电枪线号 */
        uint8_t gun_temperature;                 /* 枪头温度 */
        uint16_t soc;                            /* SOC */
        uint8_t battery_group_temp_max;          /* 电池组最高温度 */
        uint16_t charge_time;                    /* 累计充电时长 */
        uint16_t remain_time;                    /* 剩余充电时间 */
        uint32_t charge_elect;                   /* 充电电量 */
        uint32_t loss_elect;                     /* 计损电量 */
        uint32_t consume_amount;                 /* 已充金额 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_Query_PReq_Report_PileState_t;

/** 0x14 查询桩状态 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t data_type;                       /* 数据类型 */
    }body;
}Net_YcpPro_SReq_Query_PileState_t;

/** 0x15 所有端口状态(上报)应答 */
struct gun_state{
    uint8_t gunno;                               /* 枪号 */
    uint8_t state;                               /* 状态 */
    uint8_t homing;                              /* 枪是否归位 */
    uint8_t plug_gun;                            /* 是否插枪 */
};

struct gun_data{
    uint8_t gunno;                               /* 枪号 */
    uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号*/
    uint16_t output_voltage;                     /* 输出电压 */
    uint16_t output_current;                     /* 输出电流 */
    uint8_t gunline_number[NET_YCP_GUNLINE_NUMBER_LENGTH_DEFAULT]; /* 充电枪线号 */
    uint8_t gun_temperature;                     /* 枪头温度 */
    uint16_t soc;                                /* SOC */
    uint8_t battery_group_temp_max;              /* 电池组最高温度 */
    uint16_t charge_time;                        /* 累计充电时长 */
    uint16_t remain_time;                        /* 剩余充电时间 */
    uint32_t charge_elect;                       /* 充电电量 */
    uint32_t loss_elect;                         /* 计损电量 */
    uint32_t consume_amount;                     /* 已充金额 */
};

typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t data_type;                       /* 数据类型 */
        uint8_t gun_num;                         /* 枪数量 */
        /** 以下是枪状态 */
        /** 以下是可能的枪数据 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t;

/** 0x16 获取所有端口状态 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t data_type;                       /* 数据类型 */
    }body;
}Net_YcpPro_SReq_Query_PileState_All_t;

/** 0x17 远程开启充电应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号*/
        uint8_t result;                          /* 启动结果 */
        uint8_t reason;                          /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_Remote_StartCharge_t;

/** 0x18 远程开启充电 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号*/
        uint8_t strategy;                        /* 策略 */
        uint32_t strategy_para;                  /* 策略参数 */
    }body;
}Net_YcpPro_SReq_Remote_StartCharge_t;

/** 0x1A 服务器回复充电桩主动请求充电请求 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                                   /* 枪号 */
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号*/
        uint8_t logic_card_number[NET_YCP_LOGICCARD_NUMBER_LENGTH_MAX];  /* 逻辑卡号 */
        uint8_t strategy;                        /* 策略 */
        uint32_t strategy_para;                  /* 策略参数 */
        uint8_t result;                          /* 启动结果 */
        uint8_t reason;                          /* 失败原因 */
    }body;
}Net_YcpPro_SRes_ApplyCharge_Active_t;

/** 0x19 充电桩主动请求充电 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t start_type;                      /* 启动方式 */
        uint8_t phycard_number[NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01];  /* 物理卡号 */
        uint8_t password[NET_YCP_INPUT_PASSWORD_LENGTH_DEFAULT];   /* 输入密码 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_ApplyCharge_Active_t;

/** 0x1C 远程结束充电 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
    }body;
}Net_YcpPro_SReq_Remote_StopCharge_t;

/** 0x1B 远程结束充电回复 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t result;                          /* 结果 */
        uint8_t reason;                          /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_Remote_StopCharge_t;

/** 0x1D 充电结束报告上传 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                                   /* 枪号 */
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t start_time[NET_YCP_TIME_BCD_LENGTH_DEFAULT];  /* 开始时间 */
        uint8_t end_time[NET_YCP_TIME_BCD_LENGTH_DEFAULT];    /* 结束时间 */
        uint32_t tip_unit_price;                         /* 尖单价 */
        uint32_t tip_elect;                              /* 尖电量 */
        uint32_t tip_loss_elect;                         /* 尖计损电量 */
        uint32_t peak_unit_price;                        /* 峰单价 */
        uint32_t peak_elect;                             /* 峰电量 */
        uint32_t peak_loss_elect;                        /* 峰计损电量 */
        uint32_t flat_unit_price;                        /* 平单价 */
        uint32_t flat_elect;                             /* 平电量 */
        uint32_t flat_loss_elect;                        /* 平计损电量 */
        uint32_t valley_unit_price;                      /* 谷单价 */
        uint32_t valley_elect;                           /* 谷电量 */
        uint32_t valley_loss_elect;                      /* 谷计损电量 */
        uint32_t consume_amount;                         /* 消费金额 */
        uint8_t vin[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX];  /* VIN 码 */
        uint8_t start_way;                               /* 启动方式 */
        uint8_t stop_reason;                             /* 停止原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_TransactionRecords_t;

/** 0x1E 充电结束报告应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t result;                                  /* 确认结果 */
    }body;
}Net_YcpPro_SRes_TransactionRecords_t;

/** 0x22 远程升级 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t update_way;                              /* 升级方式 */
        uint8_t server_addr[NET_YCP_SERVER_ADDR_LENGTH_DEFAULT];    /* 服务器地址 */
        uint8_t path[NET_YCP_FILE_PATH_LENGTH_DEFAULT];             /* 升级文件路径 */
        uint8_t user_name[NET_YCP_USER_NAME_LENGTH_DEFAULT];        /* 用户名 */
        uint8_t password[NET_YCP_PASSWORD_LENGTH_DEFAULT];          /* 密码 */
        uint8_t control_cmd;                             /* 升级控制 */
        uint8_t timeout_value;                           /* 升级超时时间 */
    }body;
}Net_YcpPro_SReq_RemoteUpdate_t;

/** 0x21 远程升级应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
        uint8_t reason;                          /* 原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_RemoteUpdate_t;

/** 0x24 远程重启 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t control_cmd;                     /* 控制指令 */
    }body;
}Net_YcpPro_SReq_RemoteReboot_t;

/** 0x23 远程重启应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_RemoteReboot_t;

/** 0x26 远程修改联网地址 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t server_addr[NET_YCP_SERVER_ADDR_LENGTH_DEFAULT];    /* 服务器地址 */
        uint16_t port;                           /* 端口(新增字段) */
        uint8_t control_cmd;                     /* 控制指令 */
    }body;
}Net_YcpPro_SReq_Modify_ServerAddr_t;

/** 0x25 远程修改联网地址应答 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PRes_Modify_ServerAddr_t;

/** 0x28 获取当前设备故障信息 */
typedef struct{
    Net_YcpPro_Head_t head;
#if 0
    struct{
    }body;
#endif /* 0 */
}Net_YcpPro_SReq_Query_DeviceFault_t;

/** 0x27 设备故障上报 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t fault_num;                       /* 枪口故障数 */
        /** 以下是故障信息 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_Report_DeviceFault_t;

/** 0x41 充电握手阶段报文 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t bms_protocol_ver[3];             /* BMS协议版本 */
        uint8_t bms_bat_type;                    /* BMS电池类型 */
        uint16_t bms_bat_rated_capacity;         /* BMS电池额定容量 */
        uint16_t bms_bat_rated_volt;             /* BMS电池额定电压 */
        uint8_t bms_bat_maker_name[8];           /* BMS电池制造商名 */
        uint8_t bms_bat_sn[4];                   /* BMS电池组序列号 */
        uint8_t bms_bat_date[4];                 /* BMS电池组生产日期 */
        uint8_t bms_bat_charge_num[3];           /* BMS电池组充电次数 */
        uint8_t bms_bat_title_identification;    /* BMS电池组产权标识 */
        uint8_t vin[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX]; /* VIN码 */
        uint8_t bms_software_ver[8];             /* BMS软件版本 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_ShakeHand_t;

/** 0x42 参数设备内容上报 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
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
}Net_YcpPro_PReq_ParameterConfig_t;

/** 0x43 充电结束报文 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                           /* 枪号 */
        uint16_t bms_end_soc;                    /* 结束SOC */
        uint16_t bms_single_bat_volt_min;        /* BMS 动力蓄电池单体最低电压 */
        uint16_t bms_single_bat_volt_max;        /* BMS 动力蓄电池单体最高电压 */
        uint8_t bms_bat_temp_min;                /* BMS 动力蓄电池最低温度 */
        uint8_t bms_bat_temp_max;                /* BMS 动力蓄电池最高温度 */
        uint16_t total_charge_time;              /* 电桩累计充电时间 */
        uint16_t charge_elect;                   /* 电桩输出能量 */
        uint8_t charger_number[4];               /* 电桩充电机编号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_ChargeFinish_t;

/** 0x44 充电错误报文 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                           /* 枪号 */
        struct{
            uint8_t spn2560_00_identify;         /* 接收 SPN2560=0x00 的充电机辨识报文超时 */
            uint8_t spn2560_aa_identify;         /* 接收 SPN2560=0xAA 的充电机辨识报文超时 */
            uint8_t charger_sync_and_output_max; /* 接收充电机的时间同步和充电机最大输出能力报文超时 */
            uint8_t charger_charge_ready_ok;     /* 接收充电机完成充电准备报文超时 */
            uint8_t charger_charge_state;        /* 接收充电机充电状态报文超时 */
            uint8_t charger_end_charge;          /* 接收充电机中止充电报文超时 */
            uint8_t charger_charge_statistics;   /* 接收充电机充电统计报文超时 */
            uint8_t bms_and_car;                 /* 接收 BMS 和车辆的辨识报文超时 */
            uint8_t bat_parameter;               /* 接收电池充电参数报文超时 */
            uint8_t bms_charge_ready_ok;         /* 接收 BMS 完成充电准备报文超时 */
            uint8_t bat_state_all;               /* 接收电池充电总状态报文超时 */
            uint8_t bat_charge_conmand;          /* 接收电池充电要求报文超时 */
            uint8_t bms_end_charge;              /* 接收 BMS 中止充电报文超时 */
            uint8_t bms_charge_count;            /* 接收 BMS 充电统计报文超时 */
        }timeout;
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_ErrorMessage_t;

/** 0x45 充电阶段 BMS 终止充电 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t stop_reason;                             /* 终止原因 */
        uint8_t stop_fault;                              /* 终止故障原因 */
        uint8_t stop_error;                              /* 终止错误原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_BmsEnd_t;

/** 0x46 充电阶段充电机终止充电 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t stop_reason;                             /* 终止原因 */
        uint8_t stop_fault;                              /* 终止故障原因 */
        uint8_t stop_error;                              /* 终止错误原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_ChargerEnd_t;

/** 0x47 充电阶段 BMS 和充电机供需信息 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                                   /* 枪号 */
        uint16_t bms_volt_command;                       /* BMS 电压需求 */
        uint16_t bms_curr_command;                       /* BMS 电流需求 */
        uint8_t  bms_charge_mode;                        /* BMS 充电模式 */
        uint16_t bms_volt_measure_value;                 /* BMS 充电电压测量值  */
        uint16_t bms_curr_measure_value;                 /* BMS 充电电流测量值  */
        uint8_t max_single_bat_volt_gn;                  /* BMS 最高单体动力蓄电池组号 */
        uint8_t bms_max_single_bat_volt;                 /* BMS 最高单体动力蓄电池电压 */
        uint16_t  bms_current_soc;                       /* MS 当前荷电状态 SOC*/
        uint16_t bms_remain_charge_time;                 /* BMS 估算剩余充电时间 */
        uint16_t pile_output_volt;                       /* 电桩电压输出值 */
        uint16_t pile_output_curr;                       /* 电桩电流输出值 */
        uint16_t charge_time;                            /* 累计充电时间 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_BmsCommand_ChargerOut_t;

/** 0x48 充电过程 BMS 信息 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t bms_max_single_volt_bat_number;          /* BMS 最高单体动力蓄电池电压所在编号 */
        uint16_t bms_bat_temp_max;                       /* BMS 最高动力蓄电池温度 */
        uint8_t bat_temp_max_measure_number;             /* 最高温度检测点编号 */
        uint16_t bms_bat_temp_min;                       /* 最低动力蓄电池温度 */
        uint8_t bat_temp_min_measure_number;             /* 最低动力蓄电池温度检测点编号 */
        uint8_t bms_single_volt;                         /* BMS 单体动力蓄电池电压过高/过低 */
        uint8_t bms_bat_soc;                             /* BMS 整车动力蓄电池荷电状态 SOC 过高/过低 */
        uint8_t bms_bat_curr;                            /* BMS 动力蓄电池充电过电流 */
        uint8_t bms_bat_temp;                            /* BMS 动力蓄电池温度过高 */
        uint8_t bms_bat_isolate;                         /* BMS 动力蓄电池绝缘状态 */
        uint8_t bms_bat_output_linker;                   /* BMS 动力蓄电池组输出连接器连接状态 */
        uint8_t charge_forbid;                           /* 充电禁止 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YcpPro_PReq_BmsInfo_t;

#pragma pack()

#endif /* NET_PACK_USING_YCP */

#endif /* NET_NET_YCP_MESSAGE_STRUCT_DEFINE_H_ */
