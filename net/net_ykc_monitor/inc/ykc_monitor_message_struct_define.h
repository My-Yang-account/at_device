/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#ifndef NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_STRUCT_DEFINE_H_
#define NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_STRUCT_DEFINE_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_YKC_MONITOR

#define NET_YKC_MONITOR_FAULT_USING_EXTEND                                         /* 使用扩展故障字段 */
#define NET_YKC_MONITOR_USING_EXTEND_PROTOCOL                                      /* 使用监控扩展协议 */

#define NET_YKC_MONITOR_STORAGE_INIT_FLAG                              0x12345678  /* 平台数据存储标志 */

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
#define NET_YKC_MONITOR_FEES_TYPE_YKC15                                0x00        /* 费率类型：云快充1.5 */
#define NET_YKC_MONITOR_FEES_TYPE_YKC20                                0x01        /* 费率类型：云快充2.0 */
#define NET_YKC_MONITOR_FEES_TYPE_PERIOD_15MIN                         0x02        /* 费率类型：15分钟一个时段 */
#define NET_YKC_MONITOR_FEES_TYPE_START_STOP_TIME                      0x03        /* 费率类型：按开始时间、结束时间上报 */
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#define NET_YKC_MONITOR_MESSAGE_START_CODE                             0x68        /* 报文起始码 */
#define NET_YKC_MONITOR_MESSAGE_ENCRYPT_ENABLE                         0x01        /* 报文加密 */
#define NET_YKC_MONITOR_MESSAGE_ENCRYPT_DISABLE                        0x00        /* 报文不加密 */
#define NET_YKC_MONITOR_PROTOCOL_VERSION                               0x10        /* 协议版本号（v1.6） */

#define NET_YKC_MONITOR_PROTOCOL_CHECK_REGION_SIZE                     0x02        /* 校验码域长度：单位字节 */

#define NET_YKC_MONITOR_NET_LINK_TYPE_SIM                              0x00        /* 网络链接类型：SIM 卡 */
#define NET_YKC_MONITOR_NET_LINK_TYPE_LAN                              0x01        /* 网络链接类型：LAN */
#define NET_YKC_MONITOR_NET_LINK_TYPE_WAN                              0x02        /* 网络链接类型：WAN */
#define NET_YKC_MONITOR_NET_LINK_TYPE_OTHER                            0x03        /* 网络链接类型：其它 */

#define NET_YKC_MONITOR_OPERATOR_MOBILE                                0x00        /* 运营商名称：中国移动 */
#define NET_YKC_MONITOR_OPERATOR_TELECOM                               0x02        /* 运营商名称：中国电信 */
#define NET_YKC_MONITOR_OPERATOR_UNICOM                                0x03        /* 运营商名称：中国联通 */
#define NET_YKC_MONITOR_OPERATOR_OTHER                                 0x04        /* 运营商名称：其它 */

#define NET_YKC_MONITOR_PILE_TYPE_DC                                   0x00        /* 桩类型：直流 */
#define NET_YKC_MONITOR_PILE_TYPE_AC                                   0x01        /* 桩类型：交流 */

#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
#define NET_YKC_MONITOR_EXTEND_CHARGEPILE_LENGTH                       0x30        /* 默认扩展桩号长度 */
#define NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT                      0x40        /* 默认桩号长度 */

#define NET_YKC_MONITOR_EMODEL_SN_LENGTH_DEFAULT                       0x11        /* 默认电费模型编号长度 */
#define NET_YKC_MONITOR_SMODEL_SN_LENGTH_DEFAULT                       0x11        /* 默认服务费模型编号长度 */
#else
#define NET_YKC_MONITOR_EXTEND_CHARGEPILE_LENGTH                       0x30        /* 默认扩展桩号长度 */
#define NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT                      0x07        /* 默认桩号长度 */
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#define NET_YKC_MONITOR_DOMAIN_LENGTH_DEFAULT                          0x80        /* 默认域名长度 */

#define NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX                         0x08        /* 卡号长度最大值 */
#define NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX                      0x11        /* 车VIN码长度最大值 */
#define NET_YKC_MONITOR_RATE_PERIOD_COUNT_MAX                          0x30        /* 费率时段数 */
#define NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT                   0x10        /* 默认流水号长度 */
#define NET_YKC_MONITOR_USER_NAME_LENGTH_DEFAULT                       0x10        /* 默认用户名长度 */
#define NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT                        0x10        /* 默认密码长度 */
#define NET_YKC_MONITOR_SERVER_ADDR_LENGTH_DEFAULT                     0x10        /* 默认服务器地址长度 */
#define NET_YKC_MONITOR_FILE_PATH_LENGTH_DEFAULT                       0x20        /* 默认文件路径长度 */
#define NET_YKC_MONITOR_GUNLINE_NUMBER_LENGTH_DEFAULT                  0x08        /* 默认枪线编号长度 */
#define NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT                 0x06        /* 默认并充序号长度 */
#define NET_YKC_MONITOR_SOFT_VERSION_LENGTH_DEFAULT                    0x08        /* 默认软件版本长度 */
#define NET_YKC_MONITOR_SIM_BCD_LENGTH_DEFAULT                         0x0A        /* 默认sim卡卡号BCD码长度 */
#define NET_YKC_MONITOR_CARD_NUMBER_COUNT_MAX                          0x18        /* 卡号最大个数 */

#ifdef NET_YKC_MONITOR_AS_MONITOR
#define NET_YKC_MONITOR_PROCESS_INFO_TYPE_STARTING                     0x00        /* 过程信息类型：启动中 */
#define NET_YKC_MONITOR_PROCESS_INFO_TYPE_CHARGING                     0x01        /* 过程信息类型：充电中 */
#define NET_YKC_MONITOR_PROCESS_INFO_TYPE_FINISH                       0x02        /* 过程信息类型：充电结束 */

#define NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX                           0x0E        /* 单次上报设置电压、流对的最大个数 */
#define NET_YKC_MONITOR_STARTING_INFO_MAX                              0x0A        /* 单次上报启动中信息的最大个数 */
#define NET_YKC_MONITOR_CHARGING_INFO_MAX                              0x0E        /* 单次上报充电中中信息的最大个数 */
#define NET_YKC_MONITOR_FINISH_INFO_MAX                                0x01        /* 单次上报充电结束信息的最大个数 */
#define NET_YKC_MONITOR_GUIDANCE_CHANGED_INFO_MAX                      0x05        /* 导引状态变化信息的最大个数 */
#define NET_YKC_MONITOR_DEVICE_CTRL_CHANGED_INFO_MAX                   0x0A        /* 器件控制状态变化信息的最大个数 */
#define NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX                     0x05        /* 断网原因信息的最大个数 */

#define NET_YKC_MONITOR_SCREEN_PW_LENGTH_DEFAULT                       0x0F        /* 默认屏幕密码长度 */

#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
#define NET_YKC_MONITOR_FAULT_SET_NUM                                  0x02        /* 扩展故障集数量 */
#define NET_YKC_MONITOR_FAULT_SET_1                                    0x00        /* 扩展故障集1 */
#define NET_YKC_MONITOR_FAULT_SET_2                                    0x01        /* 扩展故障集2 */
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */

/*********************************************************************************
 * 设备配置信息报文
 ********************************************************************************/
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
#define NET_YKC_MONITOR_VIN_COUNT_MAX                                  0x06        /* VIN码个数 */
#define NET_YKC_MONITOR_AMMETER_ADDR_COUNT_MAX                         0x02        /* 电表地址个数 */
/************************************* 7104 *********************************************/
#define NET_YKC_MONITOR_INPUT_PORT_MAX                                 0x0D        /* 输入端口号最大值 */
#define NET_YKC_MONITOR_INPUT_PORT_MIN                                 0x01        /* 输入端口号最小值 */
#define NET_YKC_MONITOR_OUTPUT_PORT_MAX                                0x0E        /* 输出端口号最大值 */
#define NET_YKC_MONITOR_OUTPUT_PORT_MIN                                0x01        /* 输出端口号最小值 */
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#endif /* NET_YKC_MONITOR_AS_MONITOR */


#ifdef NET_YKC_MONITOR_AS_MONITOR
/*********************************************************************************
 * BMS报文信息类型码
 ********************************************************************************/
enum ykcm_bms_msg{
    NETYKCM_BMS_MSG_YT_CFC,                                          /* BMS报文：宇通CFC */
    NETYKCM_BMS_MSG_YT_BFC,                                          /* BMS报文：宇通BFC */
    NETYKCM_BMS_MSG_CRM_START,                                       /* BMS报文：开始的CRM */
    NETYKCM_BMS_MSG_CRM_END,                                         /* BMS报文：结束的CRM */
    NETYKCM_BMS_MSG_BRM,                                             /* BMS报文：BRM */
    NETYKCM_BMS_MSG_BST,                                             /* BMS报文：BST */
    NETYKCM_BMS_MSG_BSM,                                             /* BMS报文：BSM */
    NETYKCM_BMS_MSG_BEM,                                             /* BMS报文：BEM */
    NETYKCM_BMS_MSG_BSD,                                             /* BMS报文：BSD */
    NETYKCM_BMS_MSG_OTHER,                                           /* BMS报文：其它数据 */
    NETYKCM_BMS_MSG_SIZE,                                            /* BMS报文 */
};
/*********************************************************************************
 * 设备运行实时信息指令码
 ********************************************************************************/
enum ykcm_dev_running{
    NETYKCM_DEV_RUNNING_CONTROL_INFO,                                /* 设备运行实时信息指令码：控制信息 */
    NETYKCM_DEV_RUNNING_STATUS_INFO,                                 /* 设备运行实时信息指令码：状态信息 */
    NETYKCM_DEV_RUNNING_DATA_INFO_GUIDANCE,                          /* 设备运行实时信息指令码：数据信息-导引 */
    NETYKCM_DEV_RUNNING_DATA_DISCONNECT_REASON,                      /* 设备运行实时信息指令码：数据信息-断网原因 */
    NETYKCM_DEV_RUNNING_DATA_BMS_MESSAGE,                            /* 设备运行实时信息指令码：数据信息-BMS报文 */
    NETYKCM_DEV_RUNNING_SIZE,                                        /* 设备运行实时信息指令码 */
};
/*********************************************************************************/
/*********************************************************************************
 * 设备配置信息报文
 ********************************************************************************/
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
enum ykcm_config_info_type{
    NETYKCM_CONFIG_INFO_TYPE_SYSTEM,                                /* 配置信息类型：系统信息 */
    NETYKCM_CONFIG_INFO_TYPE_PILE,                                  /* 配置信息类型：桩信息 */
    NETYKCM_CONFIG_INFO_TYPE_SERVER,                                /* 配置信息类型：服务器信息 */
    NETYKCM_CONFIG_INFO_TYPE_AMMETER,                               /* 配置信息类型：电表信息 */
    NETYKCM_CONFIG_INFO_TYPE_MODULE,                                /* 配置信息类型：模块信息 */
    NETYKCM_CONFIG_INFO_TYPE_VIN,                                   /* 配置信息类型：VIN码信息 */
    NETYKCM_CONFIG_INFO_TYPE_PROTECT_INFO,                          /* 配置信息类型：保护信息 */
    NETYKCM_CONFIG_INFO_TYPE_FUNCTION_CONFIG,                       /* 配置信息类型：功能配置 */
    NETYKCM_CONFIG_INFO_TYPE_OFFLINE_BILLING,                       /* 配置信息类型：离线计费 */
    NETYKCM_CONFIG_INFO_TYPE_INPUT_7103_7101,                       /* 配置信息类型：输入信息(7103/7101) */
    NETYKCM_CONFIG_INFO_TYPE_PUBLIC_INPUT_7104,                     /* 配置信息类型：通用输入信息(7104) */
    NETYKCM_CONFIG_INFO_TYPE_GUN_INPUT_7104,                        /* 配置信息类型：枪输入信息(7104) */
    NETYKCM_CONFIG_INFO_TYPE_PUBLIC_OUTPUT_7104,                    /* 配置信息类型：通用输出信息(7104) */
    NETYKCM_CONFIG_INFO_TYPE_GUN_OUTPUT_7104,                       /* 配置信息类型：枪输出信息(7104) */
    NETYKCM_CONFIG_INFO_MODE_SELECT_NORMAL,                         /* 配置信息类型：模式选择：正常模式 */
    NETYKCM_CONFIG_INFO_MODE_SELECT_V2G,                            /* 配置信息类型：模式选择：V2G模式 */
    NETYKCM_DEBUG_INFO_MONITOR,                                     /* 调试信息类型：监控信息 */
    NETYKCM_DEBUG_INFO_LIQUID,                                      /* 调试信息类型：液冷调试 */
    NETYKCM_DEBUG_INFO_IO,                                          /* 调试信息类型：输入输出 */
    NETYKCM_DEBUG_INFO_MODULE,                                      /* 调试信息类型：模块信息 */
    NETYKCM_DEBUG_INFO_MATRIX_RELAY,                                /* 调试信息类型：矩阵继电器调试 */
    NETYKCM_CONFIG_INFO_OTHER_CONFIG,                               /* 配置信息类型：其它配置 */
    NETYKCM_CONFIG_INFO_DYNAMIC_CMD_INFO,                           /* 配置信息类型：动态类型指令信息 */
    NETYKCM_CONFIG_INFO_FIXED_CMD_INFO,                             /* 配置信息类型：固定类型指令信息 */
    NETYKCM_CONFIG_INFO_TYPE_SIZE,                                  /* 配置信息类型： */
};
enum ykcm_config_info_option{
    NETYKCM_CONFIG_INFO_OPTION_QUERY,                                /* 配置信息操作类型：查询 */
    NETYKCM_CONFIG_INFO_OPTION_SET,                                  /* 配置信息操作类型：设置 */
    NETYKCM_CONFIG_INFO_OPTION_SIZE,                                 /* 配置信息操作类型： */
};

enum ykcm_config_result{
    NETYKCM_CONFIG_RES_SUCCESS = 0x00,                               /* 配置执行结果：成功 */
    NETYKCM_CONFIG_RES_FAIL_STORAGE = 0x01,                          /* 配置执行结果：保存失败 */
    NETYKCM_CONFIG_RES_ITEM_FAIL_BASE = 0x02,                        /* 配置执行结果：配置条目失败基偏移 */
    NETYKCM_CONFIG_RES_SYS_ASSERT_BASE = 0xFF,                       /* 配置执行结果：系统总数据断言失败基偏移 */
    NETYKCM_CONFIG_RES_SYS_ITEM_ASSERT_BASE = 0x110,                 /* 配置执行结果：系统配置项断言失败基偏移 */
    NETYKCM_CONFIG_RES_EXTERN_INVOKE_ASSERT_BASE = 0x120,            /* 配置执行结果：调用外部函数的断言失败基偏移 */
};
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
#endif /* NET_YKC_MONITOR_AS_MONITOR */

enum ykc_monitor_device_state{
    NETYKC_MONITOR_DEVICE_STATE_OFFLINE = 0x00,                      /* 设备状态：离线 */
    NETYKC_MONITOR_DEVICE_STATE_FAULTING = 0x01,                     /* 设备状态：故障 */
    NETYKC_MONITOR_DEVICE_STATE_IDLE = 0x02,                         /* 设备状态：空闲 */
    NETYKC_MONITOR_DEVICE_STATE_CHARGING = 0x03,                     /* 设备状态：充电 */
};

enum ykc_monitor_start_way{
    NETYKC_MONITOR_START_WAY_APP = 0x01,                             /* 启动方式：APP */
    NETYKC_MONITOR_START_WAY_ONLINE = 0x02,                          /* 启动方式：在线卡 */
    NETYKC_MONITOR_START_WAY_OFFLINE = 0x04,                         /* 启动方式：离线卡 */
    NETYKC_MONITOR_START_WAY_VIN = 0x05,                             /* 启动方式：VIN */
};

/* CC as CHARGE COMPLETE */
enum ykc_monitor_charge_complete{
    NETYKC_MONITOR_CC_REASON40_APP = 0x40,                           /* 充电完成原因：APP 远程停止 */
    NETYKC_MONITOR_CC_REASON41_CHARGE_FULL = 0x41,                   /* 充电完成原因：达到 100% */
    NETYKC_MONITOR_CC_REASON42_TARGET_ELECT = 0x42,                  /* 充电完成原因：充电电量满足设定条件 */
    NETYKC_MONITOR_CC_REASON43_TARGET_MONEY = 0x43,                  /* 充电完成原因：充电金额满足设定条件 */
    NETYKC_MONITOR_CC_REASON44_TARGET_TIME = 0x44,                   /* 充电完成原因：充电时间满足设定条件 */
    NETYKC_MONITOR_CC_REASON45_MANUAL_STOP = 0x45,                   /* 充电完成原因：手动停止充电 */
    NETYKC_MONITOR_CC_REASON46_RESERVE = 0x46,                       /* 充电完成原因：其他方式（达到SOC 限定值） */
};

/* SF as START FAIL */
enum ykc_monitor_start_fail{
    NETYKC_MONITOR_SF_REASON4A_SYSCONTROL_FAULT = 0x4A,              /* 充电启动失败，充电桩控制系统故障(需要重启或自动恢复) */
    NETYKC_MONITOR_SF_REASON4B_GUIDE_DISCONNECT = 0x4B,              /* 充电启动失败，控制导引断开 */
    NETYKC_MONITOR_SF_REASON4C_CIRCUIT_BREAKER = 0x4C,               /* 充电启动失败，断路器跳位 */
    NETYKC_MONITOR_SF_REASON4D_AMMETER_COMMUNICATION = 0x4D,         /* 充电启动失败，电表通信中断 */
    NETYKC_MONITOR_SF_REASON4E_NO_BALLANCE = 0x4E,                   /* 充电启动失败，余额不足 */
    NETYKC_MONITOR_SF_REASON4F_CHARGE_MODULE = 0x4F,                 /* 充电启动失败，充电模块故障 */
    NETYKC_MONITOR_SF_REASON50_EMERGENCY_STOP = 0x50,                /* 充电启动失败，急停开入 */
    NETYKC_MONITOR_SF_REASON51_LPS_FAULT = 0x51,                     /* 充电启动失败，防雷器异常 */
    NETYKC_MONITOR_SF_REASON52_BMS_UNREADY = 0x52,                   /* 充电启动失败，BMS 未就绪 */
    NETYKC_MONITOR_SF_REASON53_ABNORMAL_TEMP = 0x53,                 /* 充电启动失败，温度异常 */
    NETYKC_MONITOR_SF_REASON54_BATTERY_REVERSE = 0x54,               /* 充电启动失败，电池反接故障 */
    NETYKC_MONITOR_SF_REASON55_ELECT_LOCK = 0x55,                    /* 充电启动失败，电子锁异常 */
    NETYKC_MONITOR_SF_REASON56_CLOSE_FAIL = 0x56,                    /* 充电启动失败，合闸失败 */
    NETYKC_MONITOR_SF_REASON57_INSULATION_ABNORMAL = 0x57,           /* 充电启动失败，绝缘异常 */
    NETYKC_MONITOR_SF_REASON58_RESERVE0 = 0x58,                      /* 预留 */
    NETYKC_MONITOR_SF_REASON59_RECV_BHM_TIMEOUT = 0x59,              /* 充电启动失败，接收 BMS 握手报文 BHM 超时 */
    NETYKC_MONITOR_SF_REASON5A_RECV_BRM_TIMEOUT = 0x5A,              /* 充电启动失败，接收 BMS 和车辆的辨识报文超时 BRM */
    NETYKC_MONITOR_SF_REASON5B_RECV_BCP_TIMEOUT = 0x5B,              /* 充电启动失败，接收电池充电参数报文超时 BCP */
    NETYKC_MONITOR_SF_REASON5C_RECV_BRO_AA_TIMEOUT = 0x5C,           /* 充电启动失败，接收 BMS 完成充电准备报文超时 BRO AA */
    NETYKC_MONITOR_SF_REASON5D_RECV_BCS_TIMEOUT = 0x5D,              /* 充电启动失败，接收电池充电总状态报文超时 BCS  */
    NETYKC_MONITOR_SF_REASON5E_RECV_BCL_TIMEOUT = 0x5E,              /* 充电启动失败，接收电池充电要求报文超时 BCL */
    NETYKC_MONITOR_SF_REASON5F_RECV_BSM_TIMEOUT = 0x5F,              /* 充电启动失败，接收电池状态信息报文超时 BSM */
    NETYKC_MONITOR_SF_REASON60_BHM_STAGE_VOLT_OVERRANGE = 0x60,      /* 充电启动失败，GB2015 电池在 BHM 阶段有电压不允许充电 */
    NETYKC_MONITOR_SF_REASON61_BRO_AA_STAGE_VOLT_OVERRANGE = 0x61,   /* 充电启动失败，GB2015 辨识阶段在 BRO_AA 时候电池实际电压与 BCP 报文电池电压差距大于 5% */
    NETYKC_MONITOR_SF_REASON62_BRO_AA_TO_BRO_00 = 0x62,              /* 充电完成原因：充电启动失败，B2015 充电机在预充电阶段从 BRO_AA 变成BRO_00 状态 */
    NETYKC_MONITOR_SF_REASON63_RECV_HOST_CONFIG_MESSAGE_TIMEOUT = 0x63, /* 充电启动失败，接收主机配置报文超时 */
    NETYKC_MONITOR_SF_REASON64_CHARGER_UNREADY = 0x64,               /* 充电完成原因：充电启动失败，充电机未准备就绪,我们没有回 CRO AA，对应老国标 */
    NETYKC_MONITOR_SF_REASON65_DC_RELAY = 0x65,                      /* 充电启动失败，DC继电器故障 */
    NETYKC_MONITOR_SF_REASON66_AUXPOWER = 0x66,                      /* 充电启动失败，辅助电源故障 */
    NETYKC_MONITOR_SF_REASON67_READY_VOLTAGE = 0x67,                 /* 充电异常中止，准备电压 */
    NETYKC_MONITOR_SF_REASON68_INSULT_VOLTAGE = 0x68,                /* 充电异常中止，绝缘电压 */
    NETYKC_MONITOR_SF_REASON69_RESERVE1 = 0x69,                      /* （其他原因）预留 */
};

/* AS as ABNORMAL STOP */
enum ykc_monitor_abnormal_stop{
    NETYKC_MONITOR_AS_REASON6A_SYSTEM_ATRESIA = 0x6A,                /* 充电异常中止，系统闭锁 */
    NETYKC_MONITOR_AS_REASON6B_GUIDANCE_DISCONNECT = 0x6B,           /* 充电异常中止，导引断开 */
    NETYKC_MONITOR_AS_REASON6C_CIRCUIT_BREAKER_ACTION = 0x6C,        /* 充电异常中止，断路器跳位 */
    NETYKC_MONITOR_AS_REASON6D_AMMETER_COMMUNICATE = 0x6D,           /* 充电异常中止，电表通信中断 */
    NETYKC_MONITOR_AS_REASON6E_NO_BALLANCE = 0x6E,                   /* 充电异常中止，余额不足 */
    NETYKC_MONITOR_AS_REASON6F_AC_PROTECT_ACTION = 0x6F,             /* 充电异常中止，交流保护动作 */
    NETYKC_MONITOR_AS_REASON70_DC_PROTECT_ACTION = 0x70,             /* 充电异常中止，直流保护动作 */
    NETYKC_MONITOR_AS_REASON71_CHARGE_MODULE = 0x71,                 /* 充电异常中止，充电模块故障 */
    NETYKC_MONITOR_AS_REASON72_EMERGENCY_STOP = 0x72,                /* 充电异常中止，急停开入 */
    NETYKC_MONITOR_AS_REASON73_LIGHTNING_PROTECTORS = 0x73,          /* 充电异常中止，防雷器异常 */
    NETYKC_MONITOR_AS_REASON74_TEMPERATURE_ABNORMAL = 0x74,          /* 充电异常中止，温度异常 */
    NETYKC_MONITOR_AS_REASON75_OUTPUT_ABNORMAL = 0x75,               /* 充电异常中止，输出异常 */
    NETYKC_MONITOR_AS_REASON76_CURRENT_ABNORMAL = 0x76,              /* 充电异常中止，充电无流 */
    NETYKC_MONITOR_AS_REASON77_ELOCK_ABNORMAL = 0x77,                /* 充电异常中止，电子锁异常 */
    NETYKC_MONITOR_AS_REASON78_RESERVE0 = 0x78,                      /* 预留*/
    NETYKC_MONITOR_AS_REASON79_CHARGE_TVOLTAGE_ABNORMAL = 0x79,      /* 充电异常中止，总充电电压异常 */
    NETYKC_MONITOR_AS_REASON7A_CHARGE_TCURRENT_ABNORMAL = 0x7A,      /* 充电异常中止，总充电电流异常 */
    NETYKC_MONITOR_AS_REASON7B_CHARGE_SVOLTAGE_ABNORMAL = 0x7B,      /* 充电异常中止，单体充电电压异常 */
    NETYKC_MONITOR_AS_REASON7C_BATTERY_GROUP_OVERTEMP = 0x7C,        /* 充电异常中止，电池组过温 */
    NETYKC_MONITOR_AS_REASON7D_CHARGE_SVOLTAGE_MAX_ABNORMAL = 0x7D,  /* 充电异常中止，最高单体充电电压异常 */
    NETYKC_MONITOR_AS_REASON7E_BGROUP_MTEMP_OVERTEMP = 0x7E,         /* 充电异常中止，最高电池组过温 */
    NETYKC_MONITOR_AS_REASON7F_BMV_CHARGE_SVOLTAGE_ABNORMAL = 0x7F,  /* 充电异常中止，BMV 单体充电电压异常 */
    NETYKC_MONITOR_AS_REASON80_BMT_BGROUP_OVERTEMP = 0x80,           /* 充电异常中止，BMT 电池组过温 */
    NETYKC_MONITOR_AS_REASON81_BSTATE_ABNORMAL = 0x81,               /* 充电异常中止，电池状态异常停止充电 */
    NETYKC_MONITOR_AS_REASON82_CAR_COMMAND_STOP = 0x82,              /* 充电异常中止，车辆发报文禁止充电 */
    NETYKC_MONITOR_AS_REASON83_POWER_OFF = 0x83,                     /* 充电异常中止，充电桩断电 */
    NETYKC_MONITOR_AS_REASON84_RECV_BCS_TIMEOUT = 0x84,              /* 充电异常中止，接收电池充电总状态报文超时 */
    NETYKC_MONITOR_AS_REASON85_RECV_BCL_TIMEOUT = 0x85,              /* 充电异常中止，接收电池充电要求报文超时 */
    NETYKC_MONITOR_AS_REASON86_RECV_BSM_TIMEOUT = 0x86,              /* 充电异常中止，接收电池状态信息报文超时 */
    NETYKC_MONITOR_AS_REASON87_RECV_BST_TIMEOUT = 0x87,              /* 充电异常中止，接收 BMS 中止充电报文超时 */
    NETYKC_MONITOR_AS_REASON88_RECV_BSD_TIMEOUT = 0x88,              /* 充电异常中止，接收 BMS 充电统计报文超时 */
    NETYKC_MONITOR_AS_REASON89_RECV_CCS_TIMEOUT = 0x89,              /* 充电异常中止，接收对侧 CCS 报文超时 */
    NETYKC_MONITOR_AS_REASON8A_CARDREADER = 0x8A,                    /* 充电异常中止，读卡器故障 */
    NETYKC_MONITOR_AS_REASON8B_DC_RELAY = 0x8B,                      /* 充电异常中止，DC继电器故障 */
    NETYKC_MONITOR_AS_REASON8C_AC_RELAY = 0x8C,                      /* 充电异常中止，AC继电器故障 */
    NETYKC_MONITOR_AS_REASON8D_PARALLEL_RELAY = 0x8D,                /* 充电异常中止，并联继电器故障 */
    NETYKC_MONITOR_AS_REASON8E_STORAGE_CHIP = 0x8E,                  /* 充电异常中止，存储芯片 */
    NETYKC_MONITOR_AS_REASON8F_BSM_WARNNING = 0x8F,                  /* 充电异常中止，BSM 告警 */
    NETYKC_MONITOR_AS_REASON90_UNKNOW = 0x90,                        /* 充电异常中止，未知原因停止 */
    NETYKC_MONITOR_AS_REASON91_GATE = 0x91,                          /* 充电异常中止，门禁故障 */
    NETYKC_MONITOR_AS_REASON92_BATTERY_VOLTAGE = 0x92,               /* 充电异常中止，电池电压 */
    NETYKC_MONITOR_AS_REASON93_VIN_AUTHEN_FAIL = 0x93,               /* 充电异常中止，VIN 码鉴权失败 */
    NETYKC_MONITOR_AS_REASON94_LIGHTPROTECT = 0x94,                  /* 充电异常中止，防雷器 */
    NETYKC_MONITOR_AS_REASON95_GUNSITE = 0x95,                       /* 充电异常中止，枪座 */
    NETYKC_MONITOR_AS_REASON96_CIRCUIT_BREAKER = 0x96,               /* 充电异常中止，断路器 */
    NETYKC_MONITOR_AS_REASON97_FLOODING = 0x97,                      /* 充电异常中止，水浸 */
    NETYKC_MONITOR_AS_REASON98_SMOKE = 0x98,                         /* 充电异常中止，烟感 */
    NETYKC_MONITOR_AS_REASON99_POUR = 0x99,                          /* 充电异常中止，倾倒 */
    NETYKC_MONITOR_AS_REASON9A_LIQUIDCOOLING = 0x9A,                 /* 充电异常中止，液冷 */
    NETYKC_MONITOR_AS_REASON9B_FUSE = 0x9B,                          /* 充电异常中止，熔断器 */
    NETYKC_MONITOR_AS_REASON9C_MAIN_CABINET_FORBID = 0x9C,           /* 充电异常中止，主机柜禁止充电 */
    NETYKC_MONITOR_AS_REASON9D_YT_BFC = 0x9D,                        /* 充电异常中止，宇通BFC */
    NETYKC_MONITOR_AS_REASON9D_MAIN_CABINET_FAULT = 0x9E,            /* 充电异常中止，主机柜故障 */
    NETYKC_MONITOR_AS_REASON9F_MATRIX_RELAY_KPN1_1 = 0x9F,           /* 充电异常中止，矩阵正负接触器KPN1-1故障 */
    NETYKC_MONITOR_AS_REASONA0_MATRIX_RELAY_KPN1_2 = 0xA0,           /* 充电异常中止，矩阵正负接触器KPN1-2故障 */
    NETYKC_MONITOR_AS_REASONA1_MATRIX_RELAY_KPN1_3 = 0xA1,           /* 充电异常中止，矩阵正负接触器KPN1-3故障 */
    NETYKC_MONITOR_AS_REASONA2_MATRIX_RELAY_KPN2_1 = 0xA2,           /* 充电异常中止，矩阵正负接触器KPN2-1故障 */
    NETYKC_MONITOR_AS_REASONA3_MATRIX_RELAY_KPN2_2 = 0xA3,           /* 充电异常中止，矩阵正负接触器KPN2-2故障 */
    NETYKC_MONITOR_AS_REASONA4_MATRIX_RELAY_KPN3_1 = 0xA4,           /* 充电异常中止，矩阵正负接触器KPN3-1故障 */
    NETYKC_MONITOR_AS_REASONA5_SLAVE_DEVICE_OFFLINE = 0xA5,          /* 充电异常中止，从设备离线故障 */
    NETYKC_MONITOR_AS_REASONA6_BASE_COMMUNICATION = 0xA6,            /* 充电异常中止，通信(总原因，无详细原因填这个) */
    NETYKC_MONITOR_AS_REASONA7_CAR_STOP = 0xA7,                      /* 充电异常中止，车端停(总原因，无详细原因填这个) */
    NETYKC_MONITOR_AS_REASONA8_BST_TARGET_SOC = 0xA8,                /* 充电异常中止，车端停详细原因：SOC达到目标值 */
    NETYKC_MONITOR_AS_REASONA9_BST_TARGET_TVOLT = 0xA9,              /* 充电异常中止，车端停详细原因：总电压达到目标值 */
    NETYKC_MONITOR_AS_REASONAA_BST_TARGET_SVOLT = 0xAA,              /* 充电异常中止，车端停详细原因：单体电压达到目标值 */
    NETYKC_MONITOR_AS_REASONAB_BST_CHARGER_END = 0xAB,               /* 充电异常中止，车端停详细原因：充电机主动停止 */
    NETYKC_MONITOR_AS_REASONAC_BST_INSULT = 0xAC,                    /* 充电异常中止，车端停详细原因：绝缘故障 */
    NETYKC_MONITOR_AS_REASONAD_BST_OUT_LINKER = 0xAD,                /* 充电异常中止，车端停详细原因：输出连接器故障 */
    NETYKC_MONITOR_AS_REASONAE_BST_ELEMENT = 0xAE,                   /* 充电异常中止，车端停详细原因：BMS元件故障 */
    NETYKC_MONITOR_AS_REASONAF_BST_CHARGE_LINKER = 0xAF,             /* 充电异常中止，车端停详细原因：充电连接故障 */
    NETYKC_MONITOR_AS_REASONB0_BST_HV_RELAY = 0xB0,                  /* 充电异常中止，车端停详细原因：高压继电器故障 */
    NETYKC_MONITOR_AS_REASONB1_BST_POINT_2 = 0xB1,                   /* 充电异常中止，车端停详细原因：检测点2电压检测故障 */
    NETYKC_MONITOR_AS_REASONB2_BSM_SVOLT = 0xB2,                     /* 充电异常中止，BSM详细原因：单体电压异常 */
    NETYKC_MONITOR_AS_REASONB3_BSM_SOC_STATE = 0xB3,                 /* 充电异常中止，BSM详细原因：SOC状态异常 */
    NETYKC_MONITOR_AS_REASONB4_BSM_OVERCURR = 0xB4,                  /* 充电异常中止，BSM详细原因：电池充电过流 */
    NETYKC_MONITOR_AS_REASONB5_BSM_BATGRP_OT = 0xB5,                 /* 充电异常中止，BSM详细原因：电池温度过高 */
    NETYKC_MONITOR_AS_REASONB6_BSM_BAT_INSULT = 0xB6,                /* 充电异常中止，BSM详细原因：电池绝缘状态异常 */
    NETYKC_MONITOR_AS_REASONB7_BSM_OUT_LINKER = 0xB7,                /* 充电异常中止，BSM详细原因：输出连接器状态异常 */
    NETYKC_MONITOR_AS_REASONB8_TARGET_OFFLINE_TIME = 0xB8,           /* 充电异常中止，达到离线可充电时长 */
    NETYKC_MONITOR_AS_REASONB9_START_TIMEOUT = 0xB9,                 /* 充电异常中止，启动超时 */
    NETYKC_MONITOR_AS_REASONBA_SCREEN = 0xBA,                        /* 充电异常中止：屏幕 */
    NETYKC_MONITOR_AS_REASONBB_BLUE = 0xBB,                          /* 充电异常中止：蓝牙 */
    NETYKC_MONITOR_AS_REASONBC_PASSWORD = 0xBC,                      /* 充电异常中止：密码 */
    NETYKC_MONITOR_AS_REASONBD_FINGER_PRINT = 0xBD,                  /* 充电异常中止：指纹 */
    NETYKC_MONITOR_AS_REASONBE_RECV_BRO_00_TIMEOUT = 0xBE,           /* 充电异常中止，接收 BMS 充电准备报文超时 BRO_00 */
    NETYKC_MONITOR_AS_REASONBF_MAIN_CABINET_SCRAM = 0xBF,            /* 充电异常中止，主机柜急停 */
    NETYKC_MONITOR_AS_REASONC0_MAIN_CABINET_DOOR = 0xC0,             /* 充电异常中止，主机柜门禁 */
    NETYKC_MONITOR_AS_REASONC1_MAIN_CABINET_PDU = 0xC1,              /* 充电异常中止，主机柜开关板 */
    NETYKC_MONITOR_AS_REASONC2_MAIN_CABINET_MODULEFAULT = 0xC2,      /* 充电异常中止，主机柜模块 */
    NETYKC_MONITOR_AS_REASONC3_MAIN_CABINET_CONFIG = 0xC3,           /* 充电异常中止，主机柜配置 */
    NETYKC_MONITOR_AS_REASONC4_MAIN_CABINET_ACRELAY = 0xC4,          /* 充电异常中止，主机柜交流接触器 */
    NETYKC_MONITOR_AS_REASONC5_MAIN_CABINET_SMOKE = 0xC5,            /* 充电异常中止，主机柜烟感故障 */
    NETYKC_MONITOR_AS_REASONC6_MAIN_CABINET_POUR = 0xC6,             /* 充电异常中止，主机柜倾倒故障 */
    NETYKC_MONITOR_AS_REASONC7_MAIN_CABINET_FLOODING = 0xC7,         /* 充电异常中止，主机柜水浸故障 */
    NETYKC_MONITOR_AS_REASONC8_MAIN_CABINET_OTHER = 0xC8,            /* 充电异常中止，主机柜其它故障 */
    NETYKC_MONITOR_AS_REASONC9_MAIN_CABINET_LIGHTPROTECT = 0xC9,     /* 充电异常中止，主机柜防雷故障 */
    NETYKC_MONITOR_AS_REASONCA_FAN_FAULT = 0xCA,                     /* 充电异常中止，风扇故障 */
    NETYKC_MONITOR_AS_REASONCB_IS_LOCKED = 0xCB,                     /* 充电异常中止，设备已锁定 */
};

enum ykc_monitor_cmd{
    NETYKC_MONITOR_PREQCMD_SINGIN = 0x01,                            /* 指令：桩登录请求 */
    NETYKC_MONITOR_SRESCMD_SINGIN = 0x02,                            /* 指令：服务器响应登录 */

    NETYKC_MONITOR_PREQCMD_HEARTBEAT = 0x03,                         /* 指令：桩心跳请求 */
    NETYKC_MONITOR_SRESCMD_HEARTBEAT = 0x04,                         /* 指令：服务器响应心跳 */

    NETYKC_MONITOR_PREQCMD_BILLING_MODEL_VERIFY = 0x05,              /* 指令：桩请求计费模型验证 */
    NETYKC_MONITOR_SRESCMD_BILLING_MODEL_VERIFY = 0x06,              /* 指令：服务器响应计费模型验证 */

    NETYKC_MONITOR_PREQCMD_BILLING_MODEL_REQUEST = 0x09,             /* 指令：桩请求计费模型 */
    NETYKC_MONITOR_SRESCMD_BILLING_MODEL_REQUEST = 0x0A,             /* 指令：服务器响应计费模型请求  */

    NETYKC_MONITOR_SREQCMD_READ_REALTIME_DATA = 0x12,                /* 指令：服务器请求读取实时监测数据 */
    NETYKC_MONITOR_PREQCMD_PRESCMD_REPORT_REALTIME_DATA = 0x13,      /* 指令：桩请求上传(响应)实时监测数据  */

    NETYKC_MONITOR_PREQCMD_CHARGE_SHAKE_HAND = 0x15,                 /* 指令：桩上报充电握手 */

    NETYKC_MONITOR_PREQCMD_PARA_CONFIG = 0x17,                       /* 指令：桩上报参数配置 */

    NETYKC_MONITOR_PREQCMD_CHARGE_END = 0x19,                        /* 指令：桩上报充电结束 */

    NETYKC_MONITOR_PREQCMD_ERROR_MESSAGE = 0x1B,                     /* 指令：桩上报错误报文 */

    NETYKC_MONITOR_PREQCMD_BMS_STOP = 0x1D,                          /* 指令：桩上报充电阶段 BMS 中止 */

    NETYKC_MONITOR_PREQCMD_CHARGER_STOP = 0x21,                      /* 指令：桩上报充电阶段充电机中止 */

    NETYKC_MONITOR_PREQCMD_CHARGER_OUTPUT_BMS_REQUIRE = 0x23,        /* 指令：桩上报充电过程 BMS 需求与充电机输出 */

    NETYKC_MONITOR_PREQCMD_BMS_INFO = 0x25,                          /* 指令：桩上报充电过程 BMS 信息 */

    NETYKC_MONITOR_PREQCMD_APPLY_START_CHARGE = 0x31,                /* 指令：充电桩主动申请启动充电 */
    NETYKC_MONITOR_SRESCMD_APPLY_START_CHARGE = 0x32,                /* 指令：运营平台确认启动充电 */

    NETYKC_MONITOR_SREQCMD_SERVER_START_CHARGE = 0x34,               /* 指令：运营平台远程控制启机 */
    NETYKC_MONITOR_PRESCMD_SERVER_START_CHARGE = 0x33,               /* 指令：远程启动充电命令回复 */

    NETYKC_MONITOR_SREQCMD_SERVER_STOP_CHARGE = 0x36,                /* 指令：运营平台远程停机 */
    NETYKC_MONITOR_PRESCMD_SERVER_STOP_CHARGE = 0x35,                /* 指令：远程停机命令回复 */

    NETYKC_MONITOR_PREQCMD_TRANSACTION_RECORD = 0x3B,                /* 指令：桩上报交易记录 */
    NETYKC_MONITOR_SRESCMD_TRANSACTION_RECORD_VERIFY = 0x40,         /* 指令：服务器交易记录确认 */

    NETYKC_MONITOR_SREQCMD_ACCOUNT_BALLANCE_UPDATE = 0x42,           /* 指令：服务器远程账户余额更新 */
    NETYKC_MONITOR_PRESCMD_ACCOUNT_BALLANCE_UPDATE = 0x41,           /* 指令：桩响应余额更新 */

    NETYKC_MONITOR_SREQCMD_SYNC_OFFLINECARD = 0x44,                  /* 指令：服务器同步离线卡数据 */
    NETYKC_MONITOR_PRESCMD_SYNC_OFFLINECARD = 0x43,                  /* 指令：桩响应离线卡数据同步 */

    NETYKC_MONITOR_SREQCMD_CLEAR_OFFLINECARD = 0x46,                 /* 指令：服务器清除离线卡数据 */
    NETYKC_MONITOR_PRESCMD_CLEAR_OFFLINECARD = 0x45,                 /* 指令：桩响应离线卡数据清除 */

    NETYKC_MONITOR_SREQCMD_QUERY_OFFLINECARD = 0x48,                 /* 指令：服务器查询离线卡数据 */
    NETYKC_MONITOR_PRESCMD_QUERY_OFFLINECARD = 0x47,                 /* 指令：桩响应离线卡数据查询 */

    NETYKC_MONITOR_SREQCMD_SET_WORK_PARA = 0x52,                     /* 指令：服务器设置充电桩工作参数 */
    NETYKC_MONITOR_PRESCMD_SET_WORK_PARA = 0x51,                     /* 指令：桩响应设置充电桩工作参数 */

    NETYKC_MONITOR_SREQCMD_TIME_SYNC = 0x56,                         /* 指令：服务器对时设置 */
    NETYKC_MONITOR_PRESCMD_TIME_SYNC = 0x55,                         /* 指令：桩响应对时设置 */

    NETYKC_MONITOR_SREQCMD_SET_BILLING_MODEL = 0x58,                 /* 指令：服务器设置计费模型 */
    NETYKC_MONITOR_PRESCMD_SET_BILLING_MODEL = 0x57,                 /* 指令：桩响应设置计费模型 */

    NETYKC_MONITOR_SREQCMD_QRCODE_CONFIG_GC = 0x5A,                  /* 指令：服务器设置二维码(国充) */
    NETYKC_MONITOR_PRESCMD_QRCODE_CONFIG_GC = 0x59,                  /* 指令：桩响应设置二维码(国充) */

    NETYKC_MONITOR_PREQCMD_GROUNDLOCK_INFO = 0x61,                   /* 指令：桩上报地锁数据 */

    NETYKC_MONITOR_SREQCMD_GROUNDLOCK_LIFTING = 0x62,                /* 指令：服务器遥控地锁升锁与降锁 */
    NETYKC_MONITOR_PRESCMD_GROUNDLOCK_LIFTING = 0x63,                /* 指令：桩响应遥控地锁升锁与降锁 */

    NETYKC_MONITOR_SREQCMD_REMOTE_REBOOT = 0x92,                     /* 指令：服务器远程重启 */
    NETYKC_MONITOR_PRESCMD_REMOTE_REBOOT = 0x91,                     /* 指令：桩响应远程重启 */

    NETYKC_MONITOR_SREQCMD_REMOTE_UPDATE = 0x94,                     /* 指令：服务器远程更新 */
    NETYKC_MONITOR_PRESCMD_REMOTE_UPDATE = 0x93,                     /* 指令：桩响应远程更新 */

    NETYKC_MONITOR_SREQCMD_QRCODE_CONFIG_TLD = 0x9C,                 /* 指令：服务器设置二维码(特来电) */
    NETYKC_MONITOR_PRESCMD_QRCODE_CONFIG_TLD = 0x9B,                 /* 指令：桩响应设置二维码(特来电) */

    NETYKC_MONITOR_PREQCMD_APPLY_START_MERGECHARGE = 0xA1,           /* 指令：充电桩主动申请启动并充充电 */
    NETYKC_MONITOR_SRESCMD_APPLY_START_MERGECHARGE = 0xA2,           /* 指令：运营平台确认启动并充充电 */

    NETYKC_MONITOR_SREQCMD_SERVER_START_MERGECHARGE = 0xA4,          /* 指令：运营平台远程控制并充启机 */
    NETYKC_MONITOR_PRESCMD_SERVER_START_MERGECHARGE = 0xA3,          /* 指令：远程并充启动充电命令回复 */

    NETYKC_MONITOR_SREQCMD_QRCODE_CONFIG_YKC15 = 0xF0,               /* 指令：服务器设置二维码(云快充1.5) */
    NETYKC_MONITOR_PRESCMD_QRCODE_CONFIG_YKC15 = 0xF1,               /* 指令：桩响应设置二维码(云快充1.5) */

#ifdef NET_YKC_MONITOR_AS_MONITOR
    NETYKC_MONITOR_SREQCMD_QUERY_MODULE_INFO = 0xB0,                 /* 指令：运营平台查询充电模块信息 */
    NETYKC_MONITOR_PRES_PREQCMD_QUERY_MODULE_INFO = 0xB1,            /* 指令：(充电桩上报模块信息)远程查询充电模块信息命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_MODULE_SETUPINFO = 0xB2,            /* 指令：运营平台查询模块配置信息 */
    NETYKC_MONITOR_PRES_PREQCMD_QUERY_MODULE_SETUPINFO = 0xB3,       /* 指令：(充电桩上报模块配置信息)远程查询查询模块配置信息命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_FAULT_RECORD = 0xB4,                /* 指令：运营平台查询故障记录 */
    NETYKC_MONITOR_PRESCMD_QUERY_FAULT_RECORD = 0xB5,                /* 指令：远程查询故障记录命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_CHARGE_RECORD = 0xB6,               /* 指令：运营平台查询充电记录 */
    NETYKC_MONITOR_PRESCMD_QUERY_CHARGE_RECORD = 0xB7,               /* 指令：远程查询充电记录命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_INPUTINFO_SETUP = 0xB8,             /* 指令：运营平台查询输入信息配置 */
    NETYKC_MONITOR_PRESCMD_QUERY_INPUTINFO_SETUP = 0xB9,             /* 指令：远程查询输入信息配置命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_PROTECTINFO_SETUP = 0xBA,           /* 指令：运营平台查询保护信息配置 */
    NETYKC_MONITOR_PRESCMD_QUERY_PROTECTINFO_SETUP = 0xBB,           /* 指令：远程查询保护信息配置命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_FUNCTION_SETUP = 0xBC,              /* 指令：运营平台查询功能配置 */
    NETYKC_MONITOR_PRESCMD_QUERY_FUNCTION_SETUP = 0xBD,              /* 指令：远程查询功能配置命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_DVE_INFO = 0xBE,                    /* 指令：运营平台查询设备信息 */
    NETYKC_MONITOR_PRES_PREQCMD_QUERY_REPORT_DVE_INFO = 0xBF,        /* 指令：远程查询设备信息命令回复/设备信息上报 */
    NETYKC_MONITOR_SRESCMD_REPORT_DVE_INFO = 0xC0,                   /* 指令：远程设备信息上报命令回复 */

    NETYKC_MONITOR_PREQCMD_TARGET_PLAT_LOG = 0xC1,                   /* 指令：上报目标平台日志 */
    NETYKC_MONITOR_SRESCMD_TARGET_PLAT_LOG = 0xC2,                   /* 指令：上报目标平台日志响应 */

    NETYKC_MONITOR_SREQCMD_QUERY_BILLING_RULE = 0xC3,                /* 指令：运营平台查询计费规则 */
    NETYKC_MONITOR_PRESCMD_QUERY_BILLING_RULE = 0xC4,                /* 指令：远程查询计费规则命令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_TSOCKET_INFO = 0xC5,                /* 指令：运营平台查询目标socket信息 */
    NETYKC_MONITOR_PRESCMD_QUERY_TSOCKET_INFO = 0xC6,                /* 指令：远程查询目标socket信息命令回复 */

    NETYKC_MONITOR_SREQCMD_INFOPARA_MODIFY = 0xC7,                   /* 指令：运营平台下发修改桩信息、参数 */
    NETYKC_MONITOR_PRESCMD_INFOPARA_CONFIRM = 0xC8,                  /* 指令：桩向运营平台确认修改的桩信息、参数 */
    NETYKC_MONITOR_SRESCMD_INFOPARA_CONFIRM_RESULT = 0xC9,           /* 指令：运营平台回复修改的桩信息、参数确认结果 */

    NETYKC_MONITOR_SREQCMD_FUNCTION_SWITCH = 0xCA,                   /* 指令：运营平台下发功能开关指令 */
    NETYKC_MONITOR_PRESCMD_FUNCTION_SWITCH = 0xCB,                   /* 指令：功能开关指令回复 */

    NETYKC_MONITOR_SREQCMD_QUERY_SET_VOLTCURR = 0xCC,                /* 指令：运营平台查询给模块设置的电压、电流 */
    NETYKC_MONITOR_PRES_PREQCMD_QUERY_SET_VOLTCURR = 0xCD,           /* 指令：上报(响应)给模块设置的电压、电流 */

    NETYKC_MONITOR_PREQCMD_REPORT_STARTING_INFO = 0xD7,              /* 指令：上报过程中信息 */
    NETYKC_MONITOR_SREQCMD_MODIFY_PILE_INFO = 0xD8,                  /* 指令：修改桩信息 */
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    NETYKC_MONITOR_PRESCMD_QUERY_SET_CONFIG_INFO = 0xDC,             /* 指令：桩响应运营平台下发的查询、修改桩配置信息指令 */
    NETYKC_MONITOR_SREQCMD_QUERY_SET_CONFIG_INFO = 0xDB,             /* 指令：运营平台下发查询、修改桩配置信息 */

    NETYKC_MONITOR_PREQ_SRESCMD_MFAULT_INFO = 0xDF,                  /* 指令：上报、响应模块故障信息 */

    NETYKC_MONITOR_PREQ_SRESCMD_REQUEST_SERVER_INFO = 0xE1,          /* 指令：向服务器请求信息或服务器响应请求 */

    NETYKC_MONITOR_PREQ_SREQCMD_RUNNING_REALTIME_INFO = 0xF9,        /* 指令：服务器查询、设备上报设备运行实时信息帧 */

#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#endif /* NET_YKC_MONITOR_AS_MONITOR */
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
}cp56time2a_monitor_t;

#pragma pack() /* #pragma pack(1) */

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * YkcMonitorPro   as Ykc Monitor Protocol    云快充协议
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

/** 监控平台数据存储体 */
typedef struct{
    uint32_t storage_init_flag;              /* 存储初始化标志 */
    uint8_t verify_result;                   /* 数据校验结果 */

    struct{
        uint32_t tplat_log :1;               /* 功能开关：上报目标平台日志 */
        uint32_t lock :1;                    /* 功能开关：锁桩 */
        uint32_t reserve0 :30;               /* 功能开关：预留 */
        uint32_t reserve1 :32;               /* 功能开关：预留 */
    }fswitch;                                /* 功能开关 0：开启  1：关闭 */
    uint32_t reset_count;                    /* 重启次数 */
    uint32_t reset_reason;                   /* 重启原因 */
    uint8_t reset_lable[8];                  /* 重启标签 */
    struct{
        uint16_t is_thread_error :1;         /* 线程监控检测出线程错误 1：是，0：否 */
        uint16_t freserve : 15;
    }flag;                                   /* 标志位 */
}ykc_monitor_storage_struct;

/** 协议头部 */
typedef struct{
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
    uint8_t start_code;                           /* 起始码 */
    uint16_t length;                              /* 数据长度 */
    uint16_t sequence;                            /* 序列号 */
    uint8_t encrypt;                              /* 加密标志 */
    uint16_t type;                                /* 帧类型 */
#else
    uint8_t start_code;                           /* 起始码 */
    uint8_t length;                               /* 数据长度 */
    uint16_t sequence;                            /* 序列号 */
    uint8_t encrypt;                              /* 加密标志 */
    uint8_t type;                                 /* 帧类型 */
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
}Net_YkcMonitorPro_Head_t;

/** 0x01 登录签到帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t pile_type;                       /* 桩类型 */
        uint8_t gun_count;                       /* 充电枪数量 */
        uint8_t protocol_ver;                    /* 通信协议版本 */
        uint8_t software_ver[NET_YKC_MONITOR_SOFT_VERSION_LENGTH_DEFAULT];  /* 软件版本 */
        uint8_t net_link_type;                   /* 网络连接类型 */
        uint8_t sim_number[NET_YKC_MONITOR_SIM_BCD_LENGTH_DEFAULT];         /* SIM 卡卡号 */
        uint8_t operators;                       /* 运营商 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_LogIn_t;

/** 0x02 登录签到响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                          /* 登录结果 */
    }body;
}Net_YkcMonitorPro_SRes_LogIn_t;

/** 0x03 心跳帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t state;                           /* 枪状态 */
        uint32_t meter_value;                    /* 电表值(0.01度) */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_HeartBeat_t;

/** 0x04 心跳响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t response;                        /* 心跳响应 */
    }body;
}Net_YkcMonitorPro_SRes_HeartBeat_t;

/** 0x05 计费模型验证请求帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t model_number;                   /* 计费模型编号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_BillingModel_Verify_t;

/** 0x06 计费模型验证请求响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t model_number;                   /* 计费模型编号 */
        uint8_t result;                          /* 验证结果 */
    }body;
}Net_YkcMonitorPro_SRes_BillingModel_Verify_t;

/** 0x09 计费模型请求帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_BillingModel_Request_t;

/** 0x0A 计费模型请求响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
        uint8_t rate_number[NET_YKC_MONITOR_RATE_PERIOD_COUNT_MAX];     /* 费率号 */
    }body;
}Net_YkcMonitorPro_SRes_BillingModel_Request_t;

/** 0x012 读取实时数据帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
    }body;
}Net_YkcMonitorPro_SReq_Query_RealTimeData_t;

/** 0x013 读取实时数据响应(实时数据请求)帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t state;                           /* 状态 */
        uint8_t homing;                          /* 枪是否归位 */
        uint8_t plug_gun;                        /* 是否插枪 */
        uint16_t output_voltage;                 /* 输出电压 */
        uint16_t output_current;                 /* 输出电流 */
        uint8_t gun_temperature;                 /* 枪头温度 */
        uint8_t gunline_number[NET_YKC_MONITOR_GUNLINE_NUMBER_LENGTH_DEFAULT]; /* 充电枪线号 */
        uint8_t soc;                             /* SOC */
        uint8_t battery_group_temp_max;          /* 电池组最高温度 */
        uint16_t charge_time;                    /* 累计充电时长 */
        uint16_t remain_time;                    /* 剩余充电时间 */
        uint32_t charge_elect;                   /* 充电电量 */
        uint32_t loss_elect;                     /* 计损电量 */
        uint32_t consume_amount;                 /* 消费 */
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        uint32_t fault_set[NET_YKC_MONITOR_FAULT_SET_NUM]; /* 故障集 */
#else
        uint16_t hardware_fault;                 /* 硬件故障 */
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        uint8_t is_parallel_deputy;              /* 是并充副枪 */
        uint8_t belong_main_gun;                 /* 归属主枪(是并充副枪时有效) */
        uint8_t platform_id;                     /* 报文来源的平台编号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Query_PReq_Report_RealTimeData_t;

/** 0x15 充电握手帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
        uint8_t vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX]; /* VIN码 */
        uint8_t bms_software_ver[8];             /* BMS软件版本 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_ShakeHand_t;

/** 0x17 参数配置帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_ParameterConfig_t;

/** 0x19 充电结束帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_ChargeFinish_t;

/** 0x1B 错误帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_ErrorMessage_t;

/** 0x1D 充电过程中 BMS 终止帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_BmsEnd_t;

/** 0x21 充电过程中充电机终止帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_ChargerEnd_t;

/** 0x23 充电过程 BMS 需求与充电机输出帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_BmsCommand_ChargerOut_t;

/** 0x25 充电过程 BMS 信息帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
}Net_YkcMonitorPro_PReq_BmsInfo_t;

/** 0x31 充电桩主动申请启动充电帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t start_type;                              /* 启动方式 */
        uint8_t whether_password;                        /* 是否需要密码 */
        uint8_t account_or_phycard_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint8_t password[NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT];   /* 输入密码 */
        uint8_t vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX];      /* VIN码 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_ApplyCharge_Active_t;

/** 0x32 充电桩主动申请启动充电响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 逻辑卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint8_t authentication_success;                  /* 鉴权成功标志 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
}Net_YkcMonitorPro_SRes_ApplyCharge_Active_t;

/** 0x34 运营平台远程控制启机帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];    /* 逻辑卡号 */
        uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_Remote_StartCharge_t;

/** 0x33 运营平台远程控制启机响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Remote_StartCharge_t;

/** 0x36 运营平台远程停机帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_Remote_StopCharge_t;

/** 0x35 运营平台远程停机响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Remote_StopCharge_t;

/** 0x3B 交易记录帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        cp56time2a_monitor_t start_time;                 /* 开始时间 */
        cp56time2a_monitor_t stop_time;                  /* 结束时间 */
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
        uint8_t vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX];  /* VIN 码 */
        uint8_t transaction_identity;                    /* 交易标识 */
        cp56time2a_monitor_t transaction_date;           /* 交易日期 */
        uint8_t stop_reason;                             /* 停止原因 */
        uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_TransactionRecords_t;

/** 0x40 交易记录响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t result;                                  /* 确认结果 */
    }body;
}Net_YkcMonitorPro_SRes_TransactionRecords_t;

/** 0x42 远程账户余额更新帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
        uint32_t account_amount;                         /* 修改后账户金额 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_AccountBallance_Update_t;

/** 0x41 远程账户余额更新响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_AccountBallance_Update_t;

/** 0x44 离线卡数据同步帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t count;                                   /* 下发卡个数 */
        uint16_t result;
        /** 以下为卡的物理卡号数据 */
    }body;
}Net_YkcMonitorPro_SReq_Sync_OfflineCard_t;

/** 0x43 离线卡数据同步响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Sync_OfflineCard_t;

/** 0x46 离线卡数据清除帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t count;                                   /* 清除离线卡的个数 */
        uint16_t result;
        /** 以下为卡的物理卡号数据 */
    }body;
}Net_YkcMonitorPro_SReq_Clear_OfflineCard_t;

/** 0x45 离线卡数据清除响应帧 */
struct clear_monitor_card_block{
    uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
    uint8_t result;                                  /* 结果 */
    uint8_t fail_reason;                             /* 失败原因 */
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        /* 以下为离线卡清除结果信息 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Clear_OfflineCard_t;

/** 0x48 离线卡数据查询帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t count;                                   /* 查询的离线卡个数 */
        uint16_t result;
        /** 卡数据 */
    }body;
}Net_YkcMonitorPro_SReq_Query_OfflineCard_t;

/** 0x47 离线卡数据查询响应帧 */
struct query_monitor_card_block{
    uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
    uint8_t result;                                  /* 结果 */
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        /* 以下为离线卡查询结果信息 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Query_OfflineCard_t;

/** 0x52 充电桩工作参数设置帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t forbidden;                               /* 锁定桩(禁用) */
        uint8_t power_max_percent;                       /* 充电桩最大允许输出功率百分比 */
    }body;
}Net_YkcMonitorPro_SReq_Set_WorkPara_t;

/** 0x51 充电桩工作参数设置响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Set_WorkPara_t;

/** 0x56 对时设置帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        cp56time2a_monitor_t current_time;                       /* 当前时间 */
    }body;
}Net_YkcMonitorPro_SReq_TimeSync_t;

/** 0x55 对时设置响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        cp56time2a_monitor_t current_time;                       /* 当前时间 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_TimeSync_t;

/** 0x58 计费模型设置帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
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
        uint8_t rate_number[NET_YKC_MONITOR_RATE_PERIOD_COUNT_MAX];     /* 计损比例 */
    }body;
}Net_YkcMonitorPro_SRep_BillingModel_Set_t;

/** 0x57 计费模型设置响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_BillingModel_Set_t;

/** 0x61 地锁数据上送帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t groundlock_state;                        /* 地锁状态 */
        uint8_t packingspace_state;                      /* 车位状态 */
        uint8_t groundlock_power;                        /* 地锁能量百分比 */
        uint8_t warnning_state;                          /* 报警状态 */
        uint32_t reserve;                                /* 保留 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_GroundLock_Info_t;

/** 0x62 遥控地锁升锁与降锁命令帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t control_cmd;                             /* 升/降地锁 */
        uint32_t reserve;                                /* 保留 */
    }body;
}Net_YkcMonitorPro_SReq_GroundLock_Lifting_t;

/** 0x63 遥控地锁升锁与降锁命令响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint32_t reserve;                                /* 保留 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_GroundLock_Lifting_t;

/** 0x92 远程重启帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t control_cmd;                             /* 控制指令 */
    }body;
}Net_YkcMonitorPro_SReq_RemoteReboot_t;

/** 0x91 远程重启响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_RemoteReboot_t;

/** 0x94 远程更新帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t pile_model;                              /* 桩型号 */
        uint16_t pile_power;                             /* 桩功率 */
        uint8_t server_addr[NET_YKC_MONITOR_SERVER_ADDR_LENGTH_DEFAULT];    /* 服务器地址 */
        uint16_t server_port;                            /* 服务器端口 */
        uint8_t user_name[NET_YKC_MONITOR_USER_NAME_LENGTH_DEFAULT];        /* 用户名 */
        uint8_t password[NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT];          /* 密码 */
        uint8_t path[NET_YKC_MONITOR_FILE_PATH_LENGTH_DEFAULT];             /* 升级文件路径 */
        uint8_t control_cmd;                             /* 升级控制 */
        uint8_t timeout_value;                           /* 升级超时时间 */
        uint16_t result;                                 /* 处理结果 */
    }body;
}Net_YkcMonitorPro_SReq_RemoteUpdate_t;

/** 0x93 远程更新响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                                  /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_RemoteUpdate_t;

/** 0xA1 充电桩主动申请并充充电帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t start_type;                              /* 启动方式 */
        uint8_t whether_password;                        /* 是否需要密码 */
        uint8_t account_or_phycard_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint8_t password[NET_YKC_MONITOR_PASSWORD_LENGTH_DEFAULT];   /* 输入密码 */
        uint8_t vin[NET_YKC_MONITOR_CAR_VIN_NUMBER_LENGTH_MAX];      /* VIN码 */
        uint8_t main_auxiliary_gun_flag;                     /* 主辅枪标记 */
        uint8_t merge_charge_sn[NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PReq_ApplyMergeCharge_Active_t;

/** 0xA2 充电桩主动申请并充充电响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 逻辑卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint8_t authentication_success;                  /* 鉴权成功标志 */
        uint8_t fail_reason;                             /* 失败原因 */
        uint8_t merge_charge_sn[NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
    }body;
}Net_YkcMonitorPro_SRes_ApplyMergeCharge_Active_t;

/** 0xA4 运营平台远程控制并充启机帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t logic_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];    /* 逻辑卡号 */
        uint8_t physics_card_number[NET_YKC_MONITOR_CARD_NUMBER_LENGTH_MAX];  /* 账户或物理卡号 */
        uint32_t account_ballance;                       /* 账户余额 */
        uint8_t merge_charge_sn[NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_Remote_StartMergeCharge_t;

/** 0xA3 运营平台远程控制并充启机响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t serial_number[NET_YKC_MONITOR_SERIAL_NUMBER_LENGTH_DEFAULT];   /* 流水号号*/
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                                   /* 枪号 */
        uint8_t result;                                  /* 结果 */
        uint8_t fail_reason;                             /* 失败原因 */
        uint8_t main_auxiliary_gun_flag;                 /* 主辅枪标记 */
        uint8_t merge_charge_sn[NET_YKC_MONITOR_MERGE_CHARGE_SN_LENGTH_DEFAULT];    /* 并充序号 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Remote_StartMergeCharge_t;

/** 0x5A 云服务器二维码配置请求帧(国充) */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        /** 二维码数据 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_Qrcode_Config_GC_t;

/** 0x59 充电桩二维码配置应答帧(国充) */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Qrcode_Config_GC_t;

/** 0xF0 云服务器二维码配置请求帧(云快充1.5) */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t format;                          /* 格式 */
        uint8_t length;                          /* 长度 */
        /** 二维码数据 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_Qrcode_Config_Ykc15_t;

/** 0xF1 充电桩二维码配置应答帧(云快充1.5) */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Qrcode_Config_Ykc15_t;

/** 0x9C 云服务器二维码配置请求帧(特来电) */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号*/
        uint16_t length;                         /* 长度 */
        /** 二维码数据 */
        uint16_t result;
    }body;
}Net_YkcMonitorPro_SReq_Qrcode_Config_Tld_t;

/** 0x9B 充电桩二维码配置应答帧(特来电) */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号*/
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Qrcode_Config_Tld_t;

/******************************** 以下是监控报文 *******************************/
/******************************** 以下是监控报文 *******************************/
#ifdef NET_YKC_MONITOR_AS_MONITOR
/** 服务器通用请求报文 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
    }body;
}Net_YkcMonitorPro_SReq_General_t;

/** 0xB1 上报充电模块信息、查询充电模块信息应答帧 */
struct single_module_info{
    uint16_t address;
    uint16_t voltage;
    uint16_t current;
    uint32_t state;
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t num;                             /* 模块数量 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Preq_ModuleInfo_t;

/** 0xB3 模块配置信息应答帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t model;                           /* 模块型号 */
        uint8_t group;                           /* 模块组数 */
        uint8_t module_num[8];                   /* 组内模块数 */
        uint16_t mrated_volt;                    /* 模块额定电压 */
        uint16_t mrated_curr;                    /* 模块额定电流 */
        uint16_t poutvolt_max;                   /* 桩最大输出电压 */
        uint16_t poutcurr_max;                   /* 桩最大输出电流 */
        uint16_t poutvolt_min;                   /* 桩最小输出电压 */
        uint16_t poutcurr_min;                   /* 桩最小输出电流 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_Preq_ModuleSetupInfo_t;

/** 0xB9 输入信息配置响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t emergency_enable;                /* 急停启用 */
        uint8_t emergency_reversal;              /* 急停取反 */
        uint8_t gate_enable;                     /* 门禁启用 */
        uint8_t gate_reversal;                   /* 门禁取反 */
        uint8_t acrelay_enable;                  /* 交流接触器启用 */
        uint8_t acrelay_reversal;                /* 交流接触器取反 */
        uint8_t dcrelay_enable;                  /* 直流接触器启用 */
        uint8_t dcrelay_reversal;                /* 直流接触器取反 */
        uint8_t fan_enable;                      /* 风扇启用 */
        uint8_t fan_reversal;                    /* 风扇取反 */
        uint8_t elock_enable;                    /* 电子锁启用 */
        uint8_t elock_reversal;                  /* 电子锁取反 */
        uint8_t temppro_enable;                  /* 温度保护启用 */
        uint8_t temppro_reversal;                /* 温度保护取反 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_InInfo_Setup_t;

/** 0xBB 保护信息查询响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint16_t inover_volt;                    /* 输入过压 */
        uint16_t inundver_volt;                  /* 输入欠压 */
        uint16_t ouover_volt;                    /* 输出过压 */
        uint16_t ouundver_volt;                  /* 输出欠压 */
        uint16_t ouover_curr;                    /* 输出过流 */
        uint16_t stop_soc;                       /* 停充SOC */
        uint16_t ot_warnning;                    /* 过温告警 */
        uint16_t ot_stop;                        /* 过温停充 */
        uint16_t ot_resume;                      /* 过温恢复 */
        uint16_t ot_limit;                       /* 过温限流 */
        uint16_t gun_volt;                       /* 枪头电压 */
        uint16_t power_percent;                  /* 功率百分比 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_ProtectInfo_Setup_t;

/** 0xBD 功能配置查询响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t insulation;                      /* 绝缘检测 */
        uint8_t vin_charge;                      /* VIN充电 */
        uint8_t plug_and_play;                   /* 即插即充 */
        uint8_t card_reader;                     /* 读卡器 */
        uint8_t local_charge;                    /* 本地启动 */
        uint8_t cc4_uplimit;                     /* CC1 4V 上限 */
        uint8_t cc4_downlimit;                   /* CC1 4V 下限 */
        uint8_t cc6_uplimit;                     /* CC1 6V 上限 */
        uint8_t cc6_downlimit;                   /* CC1 6V 下限 */
        uint8_t cc12_uplimit;                    /* CC1 12V 上限 */
        uint8_t cc12_downlimit;                  /* CC1 12V 下限 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PRes_FunctionSetup_t;

/** 0xBF 设备信息响应(上报)帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
        uint8_t pile_number_whole[48];           /* 真正的桩号*/
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
        uint8_t interconnecting_way;             /* 联网方式 */
        uint8_t hardware[16];                    /* 硬件版本 */
        uint8_t soft_model[16];                  /* 软件型号 */
        uint8_t dev_type;                        /* 设备类型 */
        uint8_t target_plat_protocol;            /* 目标平台协议 */
        uint16_t customer;                       /* 客户代码 */
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
        uint32_t reset_count;                    /* 重启次数 */
        uint32_t reset_reason;                   /* 重启原因 */
        uint8_t reset_lable[8];                  /* 重启标签 */
        struct{
            uint16_t verify_result :1;           /* 存储数据校验结果：1：成功，0：失败 */
            uint16_t freserve : 15;
        }flag;                                   /* 标志位 */
        uint8_t sim_no[NET_YKC_MONITOR_SIM_BCD_LENGTH_DEFAULT *0x02];   /* SIM卡卡号(20字节，可包含字母) */
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_PRes_DevInfo_t;

/** 0xC0 服务器设备信息响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
        uint8_t pile_number_whole[48];           /* 真正的桩号*/
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sres_DevInfo_t;

/** 0xC1 目标平台日志上报帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
        uint8_t pile_number_whole[48];           /* 真正的桩号*/
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
        uint8_t data_verify;                     /* 日志数据校验是否正确(1:正确，0：错误) */
        uint32_t id;                             /* 报文ID */
        /** 以下为数据部分 */
    }body;
}Net_YkcMonitorPro_Preq_TargetPlat_Log_t;

/** 0xC2 服务器目标平台日志上报响应帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
#ifndef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
        uint8_t pile_number_whole[48];           /* 真正的桩号*/
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */
        uint32_t id;                             /* 报文ID */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sres_TargetPlat_Log_t;

/** 0xC4 计费策略响应(上报)帧 */
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
/** 费率类型：云快充1.6(此类型时段数量不起作用)[48个时段，半小时一个时段] */
struct fees_type_ykc16{
    uint32_t tip_elect_rate;                 /* 尖电费费率 */
    uint32_t tip_service_rate;               /* 尖服务费费率 */
    uint32_t peak_elect_rate;                /* 峰电费费率 */
    uint32_t peak_service_rate;              /* 峰服务费费率 */
    uint32_t flat_elect_rate;                /* 平电费费率 */
    uint32_t flat_service_rate;              /* 平服务费费率 */
    uint32_t valley_elect_rate;              /* 谷电费费率 */
    uint32_t valley_service_rate;            /* 谷服务费费率 */
    uint8_t loss_proportion;                 /* 计损比例 */
    uint8_t rate_number[NET_YKC_MONITOR_RATE_PERIOD_COUNT_MAX];     /* 计损比例 */
};
/** 费率类型：云快充2.0(此类型时段数量不起作用)[48个时段(有可能不是48，根据实际来)，半小时一个时段，每个时段可设置不同的费率，费率最多48个(有可能不是48，根据实际来)] */
struct fees_type_ykc20{
    /** uint8_t fees_num; //费率数量 */
    /** 接下来是 fees_num 个费率，每个费率占4字节，精确到小数点后5位*/
    /** uint32_t fees_1   //费率1 */
    /** uint32_t fees_2   //费率2 */
    /** uint32_t fees_3   //费率3 */
    /** ...... */
    /** 接下来是时段费率号(1-48(有可能不是48，根据实际来)分别代表各费率) */
    /** fees_number[48] */

    /** 举例：4个费率 */
    /*
     * struct{
     *   uint8_t fees;      //费率数量
     *   uint32_t fees_1;   //费率1
     *   uint32_t fees_2;   //费率2
     *   uint32_t fees_3;   //费率3
     *   uint32_t fees_4;   //费率4
     *   uint8_t fees_number[4];  //费率号
     * };
     */
};
/** 费率类型： 15分钟一个时段 */
struct fees_type_period15min{
    uint32_t elect_fees : 20;                    /* 电费 */
    uint32_t service_fees : 20;                  /* 服务费 */
    uint32_t delay_fees : 20;                    /* 延迟费 */
    uint32_t reserve : 4;                        /* 预留 */
};
/** 费率类型：以开始时间(时间戳)和结束时间上报) */
struct fees_type_start_end_time{
    uint32_t start_time;                         /* 开始时间(时间戳) */
    uint32_t end_time;                           /* 结束时间(时间戳) */
    struct{
        uint32_t elect_fees : 20;                /* 电费 */
        uint32_t service_fees : 20;              /* 服务费 */
        uint32_t delay_fees : 20;                /* 延迟费 */
        uint32_t reserve : 4;                    /* 预留 */
    }fees;
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号(0xFF表示所有枪) */
        uint8_t emodel_sn[NET_YKC_MONITOR_EMODEL_SN_LENGTH_DEFAULT];     /* 电费模型编号 */
        uint8_t smodel_sn[NET_YKC_MONITOR_EMODEL_SN_LENGTH_DEFAULT];     /* 服务费费模型编号 */
        uint8_t group_num;                                               /* 时段数量 */
        uint8_t fees_type;                       /* 费率类型(0：标准云快充1.6  1：云快充2.0  2：15分钟一个时段    3：以开始时间(时间戳)和结束时间上报) */
        /* 以下是费率信息(根据费率类型来解析) */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_Pres_BillingRule_t;
#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

/** 0xC6 目标平台socket信息响应(上报)帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t domain[128];                     /* 域名 */
        uint16_t port;                           /* 端口 */
        uint8_t socket_state;                    /* socket 状态 */
        uint8_t program_state;                   /* 程序运行状态 */
        uint8_t open_count;                      /* 连续重复open socket次数*/
        uint8_t login_count;                     /* 连续重复登录次数 */
        uint8_t heartbeat_count;                 /* 心跳超时次数 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_Pres_TsocketInfo_t;

/** 0xC7、0xC8 IP、端口、桩号修改(信息确认)帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];     /* 桩号*/
        uint8_t new_pile_number[NET_YKC_MONITOR_EXTEND_CHARGEPILE_LENGTH];  /* 新桩号*/
        uint8_t domain[NET_YKC_MONITOR_DOMAIN_LENGTH_DEFAULT];              /* IP/域名 */
        uint16_t port;                           /* 端口 */
    }body;
    uint16_t ongoing;                            /* 正在处理数据 */
//    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sreq_Pres_InfoPara_ModifyConfirm_t;

/** 0xC9 IP、服务器IP、端口、桩号信息确认结果帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint16_t type;                           /* 确认报文类型码 */
        uint8_t result;                          /* 确认结果 */
        uint8_t err_reason;                      /* 错误原因 */
    }body;
    uint16_t ongoing;                            /* 正在处理数据 */
//    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sres_InfoPara_ConfirmResult_t;

/** 0xCA 服务器下发功能控制开关帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t tplat_log;                       /* 目标平台数据(1：开启， 0：关闭， 0xFF：无效) */
        uint8_t lock_module;                     /* 锁模块(1：上锁，0：解锁， 0xFF：无效) */
        uint8_t clear_record;                    /* 清除记录(1：清除， 其他：无效) */
        uint8_t reserve2;                        /* 预留 */
        uint8_t reserve3;                        /* 预留 */
        uint8_t reserve4;                        /* 预留 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sreq_FunctionSwitch_t;

/** 0xCB 桩回复服务器下发功能控制开关帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t result;                          /* 结果 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Pres_FunctionSwitch_t;

/** 0xCD 给模块设置的电压、电流响应(上报)帧 */
struct voltcurr_pair{
    uint32_t is_open : 1;                        /* 模块组开机(1：开机，0：关机) */
    uint32_t voltage : 14;                       /* 设置的电压(0.1) */
    uint32_t current : 17;                       /* 设置的电流(0.01) */
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint32_t timestamp;                                              /* 采样时间 */
        uint8_t group_num;                                               /* 模块组数 */
#if 0
        struct voltcurr_pair pair[NET_YKC_MONITOR_SETVOLTCURR_PAIR_MAX]; /* 每秒采样一次，采样14次上报一次(此帧每20秒上报一次) */
#endif
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_Pres_SetVoltCurr_t;

struct value{
    uint16_t symbol : 1;                         /* 数据的符号(0：正值，1：负值) */
    uint16_t data : 15;                          /* 数据(绝对值，0.1精度) */
};
/** 0xD7 过程中信息上报帧 */
/** 启动中信息 */
struct starting_info{
    uint8_t state;                               /* 充电状态 */
    uint16_t recved_message;                     /* BMS报文接收情况(按位来，1是已接到) */
    uint16_t sended_message;                     /* BMS报文发送情况(按位来，1是已接到) */
    struct value sampling_voltage;               /* 采样电压(精度：0.1) */
    struct value max_alllow_voltage;             /* 最大允许电压(精度：0.1) */
    struct value battery_voltage;                /* 电池电压(精度：0.1) */
    struct value module_voltage;                 /* 模块电压(精度：0.1) */
    struct value positive_insul_volt;            /* 正极绝缘电压(精度：0.1) */
    struct value negative_insul_volt;            /* 负极绝缘电压(精度：0.1) */
    uint16_t positive_insul_resistance;          /* 正极绝缘电阻(单位K欧) */
    uint16_t negative_insul_resistance;          /* 负极绝缘电阻(单位K欧) */
    uint16_t pile_measure_voltage;               /* 桩测量电压(精度：0.1) */
};
/** 充电中信息 */
struct charging_info{
    uint8_t state;                               /* 充电状态 */
    uint16_t recved_message;                     /* BMS报文接收情况(按位来，1是已接到) */
    uint16_t sended_message;                     /* BMS报文发送情况(按位来，1是已接到) */
    struct value require_voltage;                /* 需求电压(精度：0.1) */
    struct value require_current;                /* 需求电流(精度：0.1) */
    struct value module_voltage;                 /* 模块电压(精度：0.1) */
    struct value module_current;                 /* 模块电流(精度：0.1) */
    struct value bms_measure_voltage;            /* BMS测量电压(精度：0.1) */
    struct value bms_measure_current;            /* BMS测量电流(精度：0.1) */
    struct value pile_measure_current;           /* 桩测量电流(精度：0.1) */
    uint16_t pile_measure_voltage;               /* 桩测量电压(精度：0.1) */
};
/** 充电结束信息 */
struct finish_info{
    uint8_t state;                               /* 充电状态 */
    uint8_t recved_message;                      /* BMS报文接收情况(按位来，1是已接到) */
    uint8_t sended_message;                      /* BMS报文发送情况(按位来，1是已接到) */
    struct{
        uint8_t msingle_bat_sn;                  /* 最高单体动力蓄电池电压所在编号 */
        uint8_t max_bat_temp;                    /* 最高动力蓄电池温度 */
        uint8_t max_bat_temp_sn;                 /* 最高温度检测点编号 */
        uint8_t min_bat_temp;                    /* 最低动力蓄电池温度 */
        uint8_t min_bat_temp_sn;                 /* 最低动力蓄电池温度检测点编号 */
        struct{
            uint16_t bms_single_volt       : 2;  /* BMS 单体动力蓄电池电压过高/过低 */
            uint16_t bms_bat_soc           : 2;  /* BMS 整车动力蓄电池荷电状态 SOC 过高/过低 */
            uint16_t bms_bat_curr          : 2;  /* BMS 动力蓄电池充电过电流 */
            uint16_t bms_bat_temp          : 2;  /* BMS 动力蓄电池温度过高 */
            uint16_t bms_bat_isolate       : 2;  /* BMS 动力蓄电池绝缘状态 */
            uint16_t bms_bat_output_linker : 2;  /* BMS 动力蓄电池组输出连接器连接状态 */
            uint16_t charge_forbid         : 2;  /* 充电禁止 */
            uint16_t reserve               : 2;  /* 预留位  */
        }state;
    }bsm;
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint32_t timestamp;                      /* 采样时间 */
        struct{
            uint8_t info_type : 4;               /* 信息类型(0：启动中信息，1：充电中信息，2：充电结束信息) */
            uint8_t msg_version : 4;             /* 报文版本(为兼容之前的格式，版本从1开始) */
        }info;
        uint8_t group_num;                       /* 有效采样组数 */
        /* 信息数据 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_ProcessInfo_t;

/** 0xD8 服务器修改设备信息帧 */
/** 屏幕密码 */
struct screen_pw{
    uint16_t dlen;                                /* 数据长度 */
    uint8_t password[NET_YKC_MONITOR_SCREEN_PW_LENGTH_DEFAULT];  /* 密码 */
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t info_type;                       /* 信息类型 */
        /* 信息数据 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sreq_Modify_DeviceInfo_t;


/*********************************************************************************
 * 设备配置信息报文(ASCII 无效值为'0/'， bin 无效值为0xFF)
 ********************************************************************************/
#ifdef NET_YKC_MONITOR_USING_EXTEND_PROTOCOL
/************************************ 公共报文 *************************************/
/** 系统信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上但小于 NETYKCM_CONFIG_RES_SYS_ASSERT_BASE 表示某一配置项配置失败,按配置项次序升序排列(例：2：本机功能配置失败；3：分配方式配置失败；4：A枪终端地址配置失败；5：B枪终端地址配置失败) */
struct ykcm_sys_info{
    uint8_t dev_function;                        /* 本机功能(0：单枪超充，1：均充双枪，2：双枪终端，3：单枪快充，4：动态切换) */
    uint8_t allocate_way;                        /* 分配方式(0：均充, 1：先到先得, 2：功率优先) */
    uint16_t terminal_addr[2];                   /* 终端地址(两把枪：A枪在前) */
};
/** 桩信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_pile_info{
    /** 桩号使用其它报文 */
    uint8_t qrcode_prefix[128];                  /* 二维码前缀(前2字节分别为设置格式和生成格式，第3字节开始是实际数据) */
    uint8_t qrcode_suffix[128];                  /* 二维码后缀 */
    uint8_t help_number[32];                     /* 帮助电话 */
    uint8_t screen_password[15];                 /* 屏幕密码 */
    /**  新增：2025/09/05*/
    uint8_t card_key[13];                        /* 卡密钥：ASCII(前12字节有效) */
    uint8_t cardnumber_block;                    /* 卡号所在块号(0-69) */
    uint8_t qrcode_rule;                         /* 二维码规则(0：云快充，1：星星充电，2：新电途， 3：小桔， 4：云端下发，其它：非法) */
    uint8_t register_code[64];                   /* 注册码：ASCII(前63字节有效) */
    uint8_t manufacturer_sn[9];                  /* 厂商编码：ASCII(前8字节有效) */
    uint8_t random_str[32];                      /* 随机串：ASCII(前31字节有效) */

};
/** 服务器信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_server_info{
    uint8_t domain[256];                         /* 域名 */
    uint16_t port;                               /* 端口 */
    uint8_t net_mode;                            /* 网络模式(0：4G，1：以太网，2：离线) */
};
/** 电表信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_ammeter_info{
    uint8_t ammeter_addr[NET_YKC_MONITOR_AMMETER_ADDR_COUNT_MAX][13]; /* 电表地址(两把枪：A枪在前; 地址12位，最后一位填空字符) */
    uint8_t ammeter_model;                       /* 电表协议(0：瑞银，1：雅达，2：科达瑞，3：英利达，4：安科瑞，5：科为，6：预留) */
    uint8_t baudrate;                            /* 波特率(0：9600，1：2400，2：4800，3：38400，4：115200) */
    uint8_t check_way;                           /* 校验位(0:偶校验, 1:奇校验, 2:无校验) */
};

/** 模块信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_module_info{
    uint8_t module_protocol;                     /* 模块协议(0：英飞源，1：国网，2：永联，3：优优，4：易能，5：预留) */
    uint8_t module_group;                        /* 模块组数 */
    uint8_t module_num_single[8];                /* 每组模块数(目前定死8组) */
    uint16_t module_rated_voltage;               /* 模块额定电压 */
    uint16_t module_rated_current;               /* 模块额定电流 */
    uint16_t pile_outvoltage_max;                /* 桩最大输出电压 */
    uint16_t pile_outvoltage_min;                /* 桩最小输出电压 */
    uint16_t pile_outcurrent_max;                /* 桩最大输出电流 */
    uint16_t pile_outcurrent_min;                /* 桩最小输出电流 */
    /** 新增 2025/11/04 */
    uint8_t lowpower_module;                     /* 低功耗模块(0：无，1：易能) */
};
/** VIN码信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_vin_info{
    uint8_t vin_whitelist[NET_YKC_MONITOR_VIN_COUNT_MAX][18]; /* 目前最多6个VIN码，VIN码17位(最后一位填空字符) */
};
/** 保护信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_protect_info{
    uint16_t overtemp_alarm;                     /* 过温告警值(范围：1-300) */
    uint16_t overtemp_stop;                      /* 过温停充值(范围：1-300) */
    uint16_t overtemp_recovery;                  /* 过温恢复值(范围：1-120) */
    uint16_t overtemp_limitcur;                  /* 过温限流值(范围：1-300) */
    uint16_t gunvolt_limit;                      /* 枪头电压限值(100倍) */
    uint16_t soc_stop;                           /* 停充 SOC(范围：1-100) */
    uint16_t power_percent;                      /* 功率百分比(10倍) */
    uint16_t eloss_proportion;                   /* 电损比(10倍) */
    uint16_t cc1_12_max;                         /* CC1 12V 上限(100倍) */
    uint16_t cc1_12_min;                         /* CC1 12V 下限(100倍) */
    uint16_t cc1_6_max;                          /* CC1 6V 上限(100倍) */
    uint16_t cc1_6_min;                          /* CC1 6V 下限(100倍) */
    uint16_t cc1_4_max;                          /* CC1 4V 上限(100倍) */
    uint16_t cc1_4_min;                          /* CC1 4V 下限(100倍) */
    /**  新增：2025/10/28*/
    uint32_t out_overvolt;                       /* 输出过压值(0.01V) */
    uint32_t out_undervolt;                      /* 输出欠压值(0.01V) */
    uint32_t in_overvolt;                        /* 输入过压值(0.01V) */
    uint16_t in_undervolt;                       /* 输入欠压值(0.01V) */
    uint32_t out_overcurr;                       /* 输出过流值(0.01A) */
};
/** 功能配置 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_function_config{
    uint16_t insult_detect : 1;                  /* 绝缘检测(1：启用，0：禁用) */
    uint16_t card_reader : 1;                    /* 读卡器(1：启用，0：禁用) */
    uint16_t parallel_charge : 1;                /* 并充(1：启用，0：禁用) */
    uint16_t vin_charge : 1;                     /* VIN码(1：启用，0：禁用) */
    uint16_t parallel_relay : 1;                 /* 并联继电器(1：启用，0：禁用) */
    uint16_t module_silence : 1;                 /* 模块静音(1：启用，0：禁用) */
    uint16_t plug_charge : 1;                    /* 即插即充(1：启用，0：禁用) */
    uint16_t local_start : 1;                    /* 本地启动(1：启用，0：禁用) */
    uint16_t local_stop : 1;                     /* 本地停止(1：启用，0：禁用) */
    uint16_t auxpower_24V : 1;                   /* 24V辅源(1：启用，0：禁用) */
    uint16_t offline_billing : 1;                /* 离线计费(1：启用，0：禁用) */
    uint16_t password_start : 1;                 /* 密码启动(1：启用，0：禁用) */
    /** 新增：2025/06/08 */
    uint16_t mode_select : 1;                    /* 模式选择(1：启用，0：禁用) */
    uint16_t offline_card : 1;                   /* 离线卡(1：启用，0：禁用) */
};
/** 离线计费 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(2：在充电，3：时间段格式错误，4：时间段不连续，5：时间段重复) */
struct ykcm_fees_time_info{
    uint8_t start_hour;                          /* 时段开始小时(范围：0-23) */
    uint8_t start_min;                           /* 时段开始分钟(范围：0-59) */
    uint8_t end_hour;                            /* 时段结束小时(范围：0-23) */
    uint8_t end_min;                             /* 时段结束分钟(范围：0-59) */
    uint8_t rated_number;                        /* 时段费率号(0：尖尖，1：尖，2：峰，3：平，4：谷) */
};

/** 模式选择：正常模式 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_mode_select_normal{
    uint8_t mode;                                /* 当前模式： 0：自动充满，1：限制金额，2：限制电量，3：限制时间，4：预约 */
    uint32_t mode_parameter;                     /* 模式参数 ：
                                                                                                                                       对于模式0：无用，默认填0
                                                                                                                                       对于模式1：单位：0.01元
                                                                                                                                       对于模式2：单位：0.001度
                                                                                                                                       对于模式3：单位：1min
                                                                                                                                       对于模式4：单位：1s(当天启动时间秒数：例 预约 13：56 充电，则为：13 *60 *60 + 56 *60)*/
};

/** 模式选择：V2G模式 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_mode_select_v2g{
    uint8_t mode;                                /* 当前模式 */
};

struct ykcm_offline_billing{
    uint32_t service_price;                      /* 服务费价格(单位：元，10000倍) */
    uint32_t sharp_sharp_price;                  /* 尖尖电费价格(单位：元，10000倍) */
    struct ykcm_fees_time_info sstime1;          /* 尖尖时段1 */
    struct ykcm_fees_time_info sstime2;          /* 尖尖时段2 */
    uint32_t sharp_price;                        /* 尖电费价格(单位：元，10000倍) */
    struct ykcm_fees_time_info stime1;           /* 尖时段1 */
    struct ykcm_fees_time_info stime2;           /* 尖时段2 */
    uint32_t peak_price;                         /* 峰电费价格(单位：元，10000倍) */
    struct ykcm_fees_time_info ptime1;           /* 峰时段1 */
    struct ykcm_fees_time_info ptime2;           /* 峰时段2 */
    uint32_t flat_price;                         /* 平电费价格(单位：元，10000倍) */
    struct ykcm_fees_time_info ftime1;           /* 平时段1 */
    struct ykcm_fees_time_info ftime2;           /* 平时段2 */
    uint32_t valley_price;                       /* 谷电费价格(单位：元，10000倍) */
    struct ykcm_fees_time_info vtime1;           /* 谷时段1 */
    struct ykcm_fees_time_info vtime2;           /* 谷时段2 */
};

/** 其它配置 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
/** 液冷 */
struct ykcm_liquid{
    uint8_t used : 4;                            /* 液冷使用与否：1：使用   0：不使用 */
    uint8_t fdetect : 4;                         /* 是否检测液冷故障：1：是   0：否 */
    uint8_t address;                             /* 液冷地址 */
};

struct ykcm_other_config{
    struct ykcm_liquid liquid[4];
    uint16_t fan_work_time;                      /* 停充后风扇工作时间(S) */
};

/** 固定类型指令信息配置 */
/** 信息设置响应结果：0：成功  1：保存失败   2.报文版本不对， 3.及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_fixed_cmd_info{
    uint8_t msg_version;                         /* 报文版本(初始版本为0) */
    struct{
        uint8_t batvolt_detect : 1;              /* 电池电压检测(1：启用，0：禁用) */
        uint8_t bcltimeout_detect : 1;           /* BCL超时检测(1：启用，0：禁用) */
        uint8_t fast_protocol : 1;               /* FAST协议(1：启用，0：禁用) */
        uint8_t cfc_protocol : 1;                /* CFC协议(1：启用，0：禁用) */
        uint8_t bay_area_protocol : 1;           /* 湾区协议(1：启用，0：禁用) */
        uint8_t protocol_gb_t : 1;               /* 国标协议(27930)(1：启用，0：禁用) */
        uint8_t bms_several_frame : 1;           /* BMS多帧(1：启用，0：禁用) */
        uint8_t reserve : 1;                     /* 预留 */
    }info;
};

/** 动态类型指令信息配置 */
/** 信息设置响应结果：0：成功  1：保存失败   2.报文版本不对， 3.及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
/** 平台下发修改或桩上报格式 */
struct cmd_modify_segment{
    uint8_t parameter_len;                       /* 参数长度 */
    uint8_t cmd[17];                             /* 指令 */
    uint8_t parameter[33];                       /* 参数 */
};
struct ykcm_dynamic_cmd_modify{
    uint8_t msg_version;                         /* 报文版本(初始版本为0) */
    uint8_t cmd_num;                             /* 指令个数 */
    /** 以下是指令组数据 */
    /** @struct cmd_modify_segment */
};
/** 动态类型指令-读，段 */
struct cmd_read_segment{
    uint8_t cmd[17];                                              /** 指令 */
};
/** 平台下发读取格式 */
struct ykcm_dynamic_cmd_read{
    uint8_t msg_version;                         /* 报文版本(初始版本为0) */
    uint8_t cmd_num;                             /* 指令个数 */
    /** 以下是指令码数据，每个指令码占17字节 */
};

/************************************* 7103/7101 *********************************************/
/** 输入信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(类似系统信息的响应) */
struct ykcm_input_pair_7103_7101{
    uint8_t enable : 4;                          /* 1：启用，0：禁用 */
    uint8_t reversal : 4;                        /* 1：取反，0：不取反 */
};
struct ykcm_input_info_7103_7101{
    struct ykcm_input_pair_7103_7101 scram;      /* 急停 */
    struct ykcm_input_pair_7103_7101 door;       /* 门禁 */
    struct ykcm_input_pair_7103_7101 acrelay;    /* 交流接触器 */
    struct ykcm_input_pair_7103_7101 dcrelay;    /* 直流接触器 */
    struct ykcm_input_pair_7103_7101 fan;        /* 风扇 */
    struct ykcm_input_pair_7103_7101 elock;      /* 电子锁 */
    struct ykcm_input_pair_7103_7101 tempprotect;/* 温度保护 */

    /** 新增：2025/03/29 */
    struct ykcm_input_pair_7103_7101 pour;       /* 倾倒 */
    struct ykcm_input_pair_7103_7101 protect_light;/* 防雷 */
    struct ykcm_input_pair_7103_7101 flood;      /* 水浸 */
    struct ykcm_input_pair_7103_7101 smoke;      /* 烟感 */
    struct ykcm_input_pair_7103_7101 gunsite;    /* 枪座 */
    struct ykcm_input_pair_7103_7101 fuse;       /* 熔断器 */
    struct ykcm_input_pair_7103_7101 liquid;     /* 液冷 */
    struct ykcm_input_pair_7103_7101 circuit_breaker; /* 断路器 */
};
/************************************* 7104 *********************************************/
/** 输入信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(2：端口重复) */
struct ykcm_input_pair_7104{
    uint8_t port_number;                         /* 输入口号(NET_YKC_MONITOR_INPUT_PORT_MIN - NET_YKC_MONITOR_INPUT_PORT_MAX) */
    struct{
        uint8_t enable : 4;                      /* 1：启用，0：禁用 */
        uint8_t reversal : 4;                    /* 1：取反，0：不取反 */
    }state;
};
/** 通用输入信息 */
struct ykcm_public_input_info_7104{
    struct ykcm_input_pair_7104 protectlight;    /* 防雷器 */
    struct ykcm_input_pair_7104 parallel_relay1; /* 母联1 */
    struct ykcm_input_pair_7104 parallel_relay2; /* 母联2 */
    struct ykcm_input_pair_7104 parallel_relay3; /* 母联3 */
    struct ykcm_input_pair_7104 scram;           /* 急停 */
    struct ykcm_input_pair_7104 breaker;         /* 断路器 */
    struct ykcm_input_pair_7104 acrelay;         /* 交流接触器 */
    struct ykcm_input_pair_7104 fan;             /* 风扇 */
    struct ykcm_input_pair_7104 flooding;        /* 水浸 */
    struct ykcm_input_pair_7104 door;            /* 门禁 */
    struct ykcm_input_pair_7104 smoke;           /* 烟感 */
    struct ykcm_input_pair_7104 fall;            /* 倾倒 */
};
/** 枪输入信息 */
struct ykcm_gun_input_info_7104{
    struct ykcm_input_pair_7104 dcrelay;         /* 直流继电器 */
    struct ykcm_input_pair_7104 elock;           /* 电子锁 */
    struct ykcm_input_pair_7104 gunsite;         /* 枪座 */
    struct ykcm_input_pair_7104 liquid;          /* 液冷 */
    struct ykcm_input_pair_7104 fuse;            /* 熔断器 */
    struct ykcm_input_pair_7104 temp_detect;     /* 温度检测 */
};

/** 输出信息 */
/** 信息设置响应结果：0：成功  1：保存失败   2及以上表示某一配置项配置失败,按配置项次序升序排列(2：端口重复) */
struct ykcm_output_config_7104{
    uint8_t port_number;                         /* 输入口号(NET_YKC_MONITOR_OUTPUT_PORT_MIN - NET_YKC_MONITOR_OUTPUT_PORT_MAX) */
    uint8_t enable;                              /* 1:启用, 0:禁用 */
};
/** 通用输出信息 */
struct ykcm_public_output_info_7104{
    struct ykcm_output_config_7104 fan;             /* 风扇 */
    struct ykcm_output_config_7104 parallel_relay1; /* 母联1 */
    struct ykcm_output_config_7104 parallel_relay2; /* 母联2 */
    struct ykcm_output_config_7104 parallel_relay3; /* 母联3 */
    struct ykcm_output_config_7104 acrelay;         /* 交流接触器 */
};
/** 枪输出信息 */
struct ykcm_gun_output_info_7104{
    struct ykcm_output_config_7104 auxpower_24V;    /* 24V辅源 */
    struct ykcm_output_config_7104 auxpower_12V;    /* 12V辅源 */
    struct ykcm_output_config_7104 dcrelay;         /* 直流继电器 */
    struct ykcm_output_config_7104 relief;          /* 泄放 */
    struct ykcm_output_config_7104 elock;           /* 电子锁 */
    struct ykcm_output_config_7104 liquid;          /* 液冷 */
};
/** 响应结果 */
struct ykcm_response_result{
    uint8_t result;                                 /* 结果：0：成功，1：失败 */
    uint32_t fail_reason;                           /* 失败原因：配置失败的最前一个配置项次序(从1开始) */
};

/** 0xDB 服务器查询、修改设备配置信息帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号(从1开始，0xFF表示所有枪) */
        uint8_t info_type;                       /* 查询或修改信息的类型(enum ykcm_config_info_type) */
        uint8_t option;                          /* 查询：0或修改：1或响应结果2(enum ykcm_config_info_option) */
        /* 如果是修改则会带有修改的数据 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Sreq_QuerySet_ConfigInfo_t;

/** 0xDC 桩响应服务器查询、修改设备配置信息帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号(从1开始，0xFF表示所有枪), 和查询或修改的一样 */
        uint8_t info_type;                       /* 查询或修改信息的类型, 和查询或修改的一样 */
        uint8_t option;                          /* 查询：0或修改：1或响应结果2, 和查询或修改的一样 */
        /* 配置数据 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Pres_QuerySet_ConfigInfo_t;

/** 模块故障预处理信息 */
struct ykcm_mfault_pre_process_info{
    uint32_t timestamp;                      /* 发生时间 */
    uint8_t faddr;                           /* 检测到有故障的第一个模块的地址(只要一个模块有故障就上报) */
    uint8_t group_num;                       /* 有效模块组数 */
    uint8_t module_protocol;                 /* 模块协议- 0：英飞源   1：国网    2：永联   3：优优   4：易能    5：科士达 */
    uint8_t is_resume;                       /* 1:是故障恢复  0：是故障发生 */
};

/** 模块具体故障信息 */
struct ykcm_mfault_info{
    uint8_t addr;                                /* 模块地址 */
    uint32_t main_fault;                         /* 模块主故障集 */
    uint32_t sub_fault;                          /* 模块子故障集 */
};
/** 0xDF 模块故障信息帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t info_type;                       /* 信息类型(0：信息上报，1：信息上报响应) */
        /* 预处理@struct ykcm_mfault_pre_process_info */
        /* 信息数据@struct ykcm_mfault_info */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_Sres_ModuleFaultInfo_t;

/** 0xE1 向平台请求信息帧 */
typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t info_type;                       /* 信息类型(0：对时请求[无请求/响应参数]) */
        /* 请求/响应参数 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_Preq_RequestServerInfo_t;



/******************************************** 0xF9 服务器查询、设备上报设备运行实时信息帧 0xF9 *********************************************/
/** 器件枚举 */
typedef enum{
    /** 枪口类型器件(解析时按枪分) */
    NETYKCM_DEVICE_ENUM_POS_DCRELAY,             /* 器件枚举：正极直流继电器 */
    NETYKCM_DEVICE_ENUM_NEG_DCRELAY,             /* 器件枚举：负极直流继电器 */

    NETYKCM_DEVICE_ENUM_AUXPOWER_12V,            /* 器件枚举：12V辅源 */
    NETYKCM_DEVICE_ENUM_AUXPOWER_24V,            /* 器件枚举：24V辅源 */
    NETYKCM_DEVICE_ENUM_ELOCK,                   /* 器件枚举：电子锁 */
    NETYKCM_DEVICE_ENUM_FAN,                     /* 器件枚举：风扇 */
    NETYKCM_DEVICE_ENUM_LIQUID,                  /* 器件枚举：液冷 */

    /** 整机类型器件(是整桩的，解析时要分配到所有枪，只有在枪号值最小的枪上报的才是有效的) */
    NETYKCM_DEVICE_ENUM_POS_PARALLEL_RELAY_0,    /* 器件枚举：母联继电器0正极 */
    NETYKCM_DEVICE_ENUM_NEG_PARALLEL_RELAY_0,    /* 器件枚举：母联继电器0负极 */
    NETYKCM_DEVICE_ENUM_POS_PARALLEL_RELAY_1,    /* 器件枚举：母联继电器1正极 */
    NETYKCM_DEVICE_ENUM_NEG_PARALLEL_RELAY_1,    /* 器件枚举：母联继电器1负极 */
    NETYKCM_DEVICE_ENUM_POS_PARALLEL_RELAY_2,    /* 器件枚举：母联继电器2正极 */
    NETYKCM_DEVICE_ENUM_NEG_PARALLEL_RELAY_2,    /* 器件枚举：母联继电器2负极 */

    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_1_1_POS,    /* 正极矩阵继电器KP1-1控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_1_1_NEG,    /* 负极矩阵继电器KP1-1控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_1_2_POS,    /* 正极矩阵继电器KP1-2控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_1_2_NEG,    /* 负极矩阵继电器KP1-2控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_1_3_POS,    /* 正极矩阵继电器KP1-3控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_1_3_NEG,    /* 负极矩阵继电器KP1-3控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_2_1_POS,    /* 正极矩阵继电器KP2-1控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_2_1_NEG,    /* 负极矩阵继电器KP2-1控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_2_2_POS,    /* 正极矩阵继电器KP2-2控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_2_2_NEG,    /* 负极矩阵继电器KP2-2控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_3_1_POS,    /* 正极矩阵继电器KP3-1控制 */
    NETYKCM_DEVICE_ENUM_MATRIX_RELAY_3_1_NEG,    /* 负极矩阵继电器KP3-1控制 */

    NETYKCM_DEVICE_ENUM_ACRELAY,                 /* 器件枚举：交流接触器 */

    NETYKCM_DEVICE_ENUM_SIZE,                    /* 器件枚举 */
}ykcm_device_enum;

/** 直流继电器操作结果 */
typedef enum{
    NETYKCM_DEV_DCRELAY_RESULT_SUCCESS_0,        /** 直流继电器器件操作结果：第一次操作成功 */
    NETYKCM_DEV_DCRELAY_RESULT_SUCCESS_1,        /** 直流继电器器件操作结果：第二次操作成功 */
    NETYKCM_DEV_DCRELAY_RESULT_SUCCESS_2,        /** 直流继电器器件操作结果：第三次操作成功 */
    NETYKCM_DEV_DCRELAY_RESULT_PRESS_SCRAM,      /** 直流继电器器件操作结果：操作成功-急停按下 */
    NETYKCM_DEV_DCRELAY_RESULT_OPT_DIRECTLT,     /** 直流继电器器件操作结果：直接操作，不检反馈 */
    NETYKCM_DEV_DCRELAY_RESULT_FAIL,             /** 直流继电器器件操作结果：操作失败 */
    NETYKCM_DEV_DCRELAY_RESULT_SIZE,             /** 直流继电器器件操作结果：无 */
}ykcm_dev_dcrelay_opt_result;

/** 母联继电器操作结果 */
typedef enum{
    NETYKCM_DEV_PARARELAY_RESULT_SUCCESS_0,               /** 母联继电器器件操作结果：第一次操作成功 */
    NETYKCM_DEV_PARARELAY_RESULT_SUCCESS_1,               /** 母联继电器器件操作结果：第二次操作成功 */
    NETYKCM_DEV_PARARELAY_RESULT_SUCCESS_2,               /** 母联继电器器件操作结果：第三次操作成功 */
    NETYKCM_DEV_PARARELAY_RESULT_PRESS_SCRAM,             /** 母联继电器器件操作结果：操作成功-急停按下 */
    NETYKCM_DEV_PARARELAY_RESULT_OPT_DIRECTLT,            /** 母联继电器器件操作结果：直接操作，不检反馈 */
    NETYKCM_DEV_PARARELAY_RESULT_FAIL,                    /** 母联继电器器件操作结果：操作失败 */
    NETYKCM_DEV_PARARELAY_RESULT_SIZE,                    /** 母联继电器器件操作结果：无 */
}ykcm_dev_pararelay_opt_result;

/** 交流接触器操作结果 */
typedef enum{
    NETYKCM_DEV_ACRELAY_RESULT_SUCCESS_0,                 /** 交流接触器器件操作结果：第一次操作成功(正常模式) */
    NETYKCM_DEV_ACRELAY_RESULT_SUCCESS_1,                 /** 交流接触器器件操作结果：第二次操作成功(正常模式)  */
    NETYKCM_DEV_ACRELAY_RESULT_SUCCESS_2,                 /** 交流接触器器件操作结果：第三次操作成功(正常模式)  */
    NETYKCM_DEV_ACRELAY_RESULT_OPT_DIRECTLT,              /** 交流接触器器件操作结果：直接操作，不检反馈(正常模式)  */
    NETYKCM_DEV_ACRELAY_RESULT_SUCCESS_MAGNRTIC,          /** 交流接触器器件操作结果：操作成功(磁保持模式) */
    NETYKCM_DEV_ACRELAY_RESULT_FAIL_FB_N,                 /** 交流接触器器件操作结果：操作失败-反馈不对(正常模式)  */
    NETYKCM_DEV_ACRELAY_RESULT_FAIL_FB_M,                 /** 交流接触器器件操作结果：操作失败-反馈不对(磁保持模式)  */
    NETYKCM_DEV_ACRELAY_RESULT_FAIL_NORMAL,               /** 交流接触器器件操作结果：操作失败-不是正常模式 */
    NETYKCM_DEV_ACRELAY_RESULT_FAIL_MAGNRTIC,             /** 交流接触器器件操作结果：操作失败-不是磁保持模式 */
    NETYKCM_DEV_ACRELAY_RESULT_SIZE,                      /** 交流接触器器件操作结果：无 */
}ykcm_dev_acrelay_opt_result;

/** 电子锁操作结果 */
typedef enum{
    NETYKCM_DEV_ELOCK_RESULT_SUCCESS_0,                   /** 电子锁器件操作结果：第一次操作成功 */
    NETYKCM_DEV_ELOCK_RESULT_SUCCESS_1,                   /** 电子锁器件操作结果：第二次操作成功 */
    NETYKCM_DEV_ELOCK_RESULT_SUCCESS_2,                   /** 电子锁器件操作结果：第二次操作成功 */
    NETYKCM_DEV_ELOCK_RESULT_OPT_DIRECTLT,                /** 电子锁器件操作结果：直接操作，不检反馈 */
    NETYKCM_DEV_ELOCK_RESULT_FAIL,                        /** 电子锁器件操作结果：操作失败 */
    NETYKCM_DEV_ELOCK_RESULT_SIZE,                        /** 电子锁器件操作结果：无 */
}ykcm_device_eLock_opt_result;

/** 风扇操作结果 */
typedef enum{
    NETYKCM_DEV_FAN_RESULT_SUCCESS,                      /** 风扇器件操作结果：操作成功 */
    NETYKCM_DEV_FAN_RESULT_SIZE,                         /** 风扇器件操作结果：无 */
}ykcm_dev_fan_opt_result;

/** 辅源操作结果 */
typedef enum{
    NETYKCM_DEV_AUXPOWER_RESULT_SUCCESS,                 /** 辅源器件操作结果：操作成功 */
    NETYKCM_DEV_AUXPOWER_RESULT_SIZE,                    /** 辅源器件操作结果：无 */
}ykcm_device_auxpower_opt_result;

/** 液冷操作结果 */
typedef enum{
    NETYKCM_DEV_LIQUID_RESULT_SUCCESS,                   /** 液冷器件操作结果：操作成功 */
    NETYKCM_DEV_LIQUID_RESULT_FAIL_OFFLINE,              /** 液冷器件操作结果：操作失败-离线 */
    NETYKCM_DEV_LIQUID_RESULT_SIZE,                      /** 液冷器件操作结果：无 */
}ykcm_device_liquid_opt_result;


/** 控制器件小段 */
struct control_segment{
    uint32_t timestamp;                          /* 变化时的时间(时间戳) */
    uint8_t device;                              /* 器件名@ykcm_device_enum */
    struct{
        uint8_t ctrl : 1;                        /* 控制操作(1：动作，0：释放) */
        uint8_t is_debug : 1;                    /* 是否是调试控制(1：是，0：否) */
        uint8_t result : 4;                      /* 控制操作结果 */
        uint8_t type : 2;                        /* 器件类型：0：枪口类型(解析时按枪分)
                                                                                                                                                          1：整机类型(是整桩的，解析时要分配到所有枪，只有在枪号值最小的枪上报的才是有效的) */
    }info;
};

/** 控制信息 */
struct running_control_info{
    uint8_t segment_num;                         /* 有效段数 */
#if 0
    /** 控制段1 */
    struct{
        /** 以下是控制段信息 */
        struct control_segment segment1
    }segment;
    /** 控制段2 */
    struct{
        /** 以下是控制段信息 */
        struct control_segment segment2
    }segment;
    .......
#endif
};


/** 设备运行实时信息：状态信息 */
/** 信息段 */
struct running_status_segment{
    uint32_t timestamp;                          /* 变化时的时间(时间戳) */
    struct{
        /** 枪口类型器件(解析时按枪分) */
        uint32_t dcrelay_positive_status : 1;          /* 正直流继电器状态(1：动作，0：释放) */
        uint32_t dcrelay_negtive_status : 1;           /* 负直流继电器状态(1：动作，0：释放) */

        uint32_t elock_status : 1;                     /* 电子锁状态(1：动作，0：释放) */
        uint32_t liquid_status : 1;                    /* 液冷状态(1：开，0：关) */
        uint32_t fan_status : 1;                       /* 风扇状态(1：开，0：关) */

        uint32_t auxpower_12v_status : 1;              /* 12V辅源状态(1：闭合，0：断开) */
        uint32_t auxpower_24v_status : 1;              /* 24V辅源状态(1：闭合，0：断开) */

        /** 整机类型器件(是整桩的，解析时要分配到所有枪，只有在枪号值最小的枪上报的才是有效的) */
        uint32_t parallelrelay_0_positive_status : 1;  /* 正母联0继电器状态(1：动作，0：释放) */
        uint32_t parallelrelay_0_negtive_status : 1;   /* 负母联0继电器状态(1：动作，0：释放) */

        uint32_t parallelrelay_1_positive_status : 1;  /* 正母联1继电器状态(1：动作，0：释放) */
        uint32_t parallelrelay_1_negtive_status : 1;   /* 负母联1继电器状态(1：动作，0：释放) */

        uint32_t parallelrelay_2_positive_status : 1;  /* 正母联2继电器状态(1：动作，0：释放) */
        uint32_t parallelrelay_2_negtive_status : 1;   /* 负母联2继电器状态(1：动作，0：释放) */

        uint32_t matrixrelay_1_1_positive_status : 1;  /* 正矩阵继电器KP1-1状态(1：动作，0：释放) */
        uint32_t matrixrelay_1_1_negtive_status : 1;   /* 负矩阵继电器KP1-1状态(1：动作，0：释放) */

        uint32_t matrixrelay_1_2_positive_status : 1;  /* 正矩阵继电器KP1-2状态(1：动作，0：释放) */
        uint32_t matrixrelay_1_2_negtive_status : 1;   /* 负矩阵继电器KP1-2状态(1：动作，0：释放) */

        uint32_t matrixrelay_1_3_positive_status : 1;  /* 正矩阵继电器KP1-3状态(1：动作，0：释放) */
        uint32_t matrixrelay_1_3_negtive_status : 1;   /* 负矩阵继电器KP1-3状态(1：动作，0：释放) */

        uint32_t matrixrelay_2_1_positive_status : 1;  /* 正矩阵继电器KP2-1状态(1：动作，0：释放) */
        uint32_t matrixrelay_2_1_negtive_status : 1;   /* 负矩阵继电器KP2-1状态(1：动作，0：释放) */

        uint32_t matrixrelay_2_2_positive_status : 1;  /* 正矩阵继电器KP2-2状态(1：动作，0：释放) */
        uint32_t matrixrelay_2_2_negtive_status : 1;   /* 负矩阵继电器KP2-2状态(1：动作，0：释放) */

        uint32_t matrixrelay_3_1_positive_status : 1;  /* 正矩阵继电器KP3-1状态(1：动作，0：释放) */
        uint32_t matrixrelay_3_1_negtive_status : 1;   /* 负矩阵继电器KP3-1状态(1：动作，0：释放) */

        uint32_t acrelay_status : 1;                   /* 交流接触器状态(1：动作，0：释放) */
        uint32_t reserve : 6;                          /* 预留 */
    }info;
};
/** 状态信息 */
struct running_status_info{
    uint8_t segment_num;                           /* 有效段数 */
    struct{
        struct running_status_segment data;        /* 状态数据 */
    }segment;
};

/******************************* 运行数据段 *******************************/
/***************** 导引数据段 *****************/
struct guidance_segment{
    uint32_t timestamp;                          /* 变化时的时间(时间戳) */
    int16_t voltage;                             /* 导引当前电压值(0.01V) */
    int16_t voltage_last;                        /* 导引前一次电压值(0.01V) */
    uint16_t diff_positive_adc;                  /* 当前差分正ADC */
    uint16_t diff_negtive_adc;                   /* 当前差分负ADC */
    uint16_t diff_positive_adc_last;             /* 前一次差分正ADC */
    uint16_t diff_negtive_adc_last;              /* 前一次差分负ADC */
    uint8_t flag;                                /* 标志 */
};

/***************** 断网原因数据段 *****************/
/** socket 断网信息小组 */
struct socket_dis_group{
    struct{
        int8_t fd;
        uint32_t timestamp;                                    /** 时间戳 */
    }info[NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX];
};

/** 通信模块断网信息小组 */
struct module_dis_group{
    uint32_t timestamp[NET_YKC_MONITOR_DISCONNECT_REASON_INFO_MAX];    /** 时间戳 */
};

/** 断网原因信息信息段 */
struct disconnect_reason_segment{
    struct socket_dis_group close_passive;                     /** socket 被动关闭 */
    struct socket_dis_group close_active;                      /** socket 主动关闭 */
    struct socket_dis_group heartbeat_timeout;                 /** socket 心跳超时 */
    struct module_dis_group socket_pdp;                        /** socket PDP场景失效 */
    struct module_dis_group close_module;                      /** 通信模块关闭 */
    struct module_dis_group at_physics;                        /** AT指令(以太网物理层：查是否在线、查版本、修改波特率) */
    struct module_dis_group cpin_lk_mac;                       /** 查找SIM卡(以太网数据链路层：初始化芯片、寻线、开DHCP) */
    struct module_dis_group cimi_lk_mac;                       /** 查找CIMI号(以太网数据链路层：初始化芯片、寻线、开DHCP) */
    struct module_dis_group signal_strength_lk_mac;            /** 查询信号强度(以太网数据链路层：初始化芯片、寻线、开DHCP) */
    struct module_dis_group gsm_registered;                    /** 注册GSM网络(以太网网络层：判断DHCP是否启动、获取IP信息、查询MAC地址) */
    struct module_dis_group gprs_registered;                   /** 注册GPRS网络(以太网网络层：判断DHCP是否启动、获取IP信息、查询MAC地址) */
};

/***************** BMS报文数据段 *****************/
/** BMS 报文信息头 */
struct bms_msg_info_head{
    uint8_t msg_type;                                          /** 报文类型 */
    uint16_t msg_len;                                          /** 报文内容长度(B) */
};
/****** 超级(大)电流协议报文 ******/
enum currprotocol{
    NETYKCM_SUPER_CURR_PROTOCOL_NONE,                          /** 超级电流协议：无 */
    NETYKCM_SUPER_CURR_PROTOCOL_YUTONG,                        /** 超级电流协议：宇通CFC */
    NETYKCM_SUPER_CURR_PROTOCOL_FAST,                          /** 超级电流协议：FAST */
    NETYKCM_SUPER_CURR_PROTOCOL_SIZE,                          /** 超级电流协议 */
};

/** 宇通CFC报文 */
struct yt_cfc{
    uint8_t curr_offset;                                   /** 电流偏移(8bits，100A/bit；范围 4-20， 双枪充电默认为6，受电弓充电默认 12) */
    uint8_t gun_num : 4;                                   /** 表示充电机检测到的有效插枪数量 */
    uint8_t ack : 2;                                       /** 应答信号(00：与 CRM 一起发送，作为通知 BMS 充电机的协议为电流偏移量自动识别 */
                                                           /**        01：已经收到 BFC 的反馈，但 BMS 回复的电流偏移量或检测到的充电枪数量等信息不符 */
                                                           /**        10：无效 */
                                                           /**        11：已经收到 BFC 的反馈，且 BMS 回复的电流偏移量和检测到的充电枪数量等信息均一致) */
    uint8_t reserve0 : 2;                                  /** 保留(填充不做要求) */
    uint8_t reserve1;                                      /** 保留(填充0xFF) */
    uint8_t reserve2;                                      /** 保留(填充0xFF) */
    uint8_t reserve3;                                      /** 保留(填充0xFF) */
    uint8_t reserve4;                                      /** 保留(填充0xFF) */
    uint8_t reserve5;                                      /** 保留(填充0xFF) */
    uint8_t reserve6;                                      /** 保留(填充0xFF) */
};
/** 宇通BFC报文 */
struct yt_bfc{
    uint8_t curr_offset;                                   /** 电流偏移(8bits，100A/bit；范围 4-20， 双枪充电默认为6，受电弓充电默认 12) */
    uint8_t gun_num : 4;                                   /** 表示充电机检测到的有效插枪数量 */
    uint8_t ack : 2;                                       /** 应答信号(00：与 CRM 一起发送，作为通知 BMS 充电机的协议为电流偏移量自动识别 */
                                                           /**        01：已经收到 BFC 的反馈，但 BMS 回复的电流偏移量或检测到的充电枪数量等信息不符 */
                                                           /**        10：无效 */
                                                           /**        11：已经收到 BFC 的反馈，且 BMS 回复的电流偏移量和检测到的充电枪数量等信息均一致) */
    uint8_t reserve0 : 2;                                  /** 保留(填充不做要求) */
    uint8_t reserve1;                                      /** 保留(填充0xFF) */
    uint8_t reserve2;                                      /** 保留(填充0xFF) */
    uint8_t reserve3;                                      /** 保留(填充0xFF) */
    uint8_t reserve4;                                      /** 保留(填充0xFF) */
    uint8_t reserve5;                                      /** 保留(填充0xFF) */
    uint8_t reserve6;                                      /** 保留(填充0xFF) */
};

/** 开始CRM报文 */
struct start_crm{
    uint8_t  discern;                                          /** 辨识结果 00:未识别 AA:识别 */
    uint32_t chage_number;                                     /** 充电机编号 */
    uint8_t  chage_place[3];                                   /** 充电机所在区域编号 ASCII码 3byte */
};

/** 结束CRM报文 */
struct end_crm{
    uint8_t  discern;                                          /** 辨识结果 00:未识别 AA:识别 */
    uint32_t chage_number;                                     /** 充电机编号 */
    uint8_t  chage_place[3];                                   /** 充电机所在区域编号 ASCII码 3byte */
};

/** BRM报文 */
struct brm{
    uint8_t  bms_version[3];                                   /** BMS版本号 3byte */
    uint8_t  bat_type;                                         /** 电池类型 01:铅酸 02:镍氢 03:磷酸铁锂 04:锰酸锂 05:钴酸锂 06:三元材料 07:聚合物锂 08:钛酸锂 FF:其他 */
    uint16_t bat_rate_capacity;                                /** 动力电池额定容量       0.1AH/bit 0-1000AH */
    uint16_t bat_rate_volt;                                    /** 动力电池额定总电压     0.1V/bit 0-750V */
    uint8_t  bat_firm[4];                                      /** 电池生产厂商 ASCII码 4byte */
    uint8_t  serial_number[4];                                 /** 电池组序号 4byte */
    uint8_t  bat_buld_year;                                    /** 电池生产日期 1年/bit 偏移1985 1985-2235 */
    uint8_t  bat_buld_month;                                   /** 1月/bit */
    uint8_t  bat_buld_day;                                     /** 1日/bit */
    uint8_t  chage_timer[3];                                   /** 电池充电次数 3byte */
    uint8_t  bat_property;                                     /** 电池组产权标识 0:租赁 1:自有 */
    uint8_t  reserved ;                                        /** 预留 */
    uint8_t  car_vin[17];                                      /** 车辆识别信息 17byte */
    uint8_t  bms_ver_number[8];
};
/****** BMS标准报文 ******/
/** BST报文 */
struct bst{
    uint8_t target_soc : 2;                                    /** SOC达到目标值           00:未达到  01:达到 11:不可信状态 */
    uint8_t target_total_volt : 2;                             /** 总电压达到目标值        00:未达到  01:达到 11:不可信状态 */
    uint8_t target_single_volt : 2;                            /** 单体电压达到目标值      00:未达到  01:达到 11:不可信状态 */
    uint8_t charger_end : 2;                                   /** 充电机主动停止 */

    uint8_t insultion_fault : 2;                               /** 绝缘故障           00:正常  01:故障 10:不可信状态 */
    uint8_t outlinker_fault : 2;                               /** 输出连接器故障     00:正常  01:故障 10:不可信状态 */
    uint8_t bms_element_fault : 2;                             /** BMS元件故障        00:正常  01:故障 10:不可信状态 */
    uint8_t charge_linker_fault : 2;                           /** 充电连接故障 */

    uint8_t bat_group_fault : 2;                               /** 电池组温度故障     00:正常  01:故障 10:不可信状态 */
    uint8_t hv_relay_fault : 2;                                /** 高压继电器故障     00:正常  01:故障 10:不可信状态 */
    uint8_t detect_point_2 : 2;                                /** 检测点2电压检测故障00:正常  01:故障 10:不可信状态 */
    uint8_t other_fault : 2;                                   /** 其他故障 */

    uint8_t over_curr : 2;                                     /** 充电电流过流       00:正常  01:超过需求值 01:不可信状态 */
    uint8_t volt_abnormal : 2;                                 /** 充电电压异常       00:正常  01:电压异常   01:不可信状态 */
    uint8_t reserve : 4;                                       /** 预留 */
};

/** BSM报文 */
struct bsm{
    uint8_t max_singlevolt_sn;                                 /** 最高单体电压单体所在编号 */
    int8_t highest_temp;                                       /** 动力电池最高温度   1°/bit  -50-200 偏移-50 */
    uint8_t highest_temp_sn;                                   /** 最高温度检测点编号 */
    int8_t lowest_temp;                                        /** 动力电池最低温度   1°/bit  -50-200 偏移-50 */
    uint8_t lowest_temp_sn;                                    /** 最低温度检测点编号 */

    uint8_t singlevolt_over : 2;                               /** 单体过压           00:正常  01:过高 01:过低 */
    uint8_t soc_state : 2;                                     /** SOC状态            00:正常  01:过高 01:过低 */
    uint8_t bat_overcurrr : 2;                                 /** 电池充电过流       00:正常  01:过高 01:过低 */
    uint8_t bat_overtemp : 2;                                  /** 电池温度过高       00:正常  01:过高 01:过低 */

    uint8_t insultion_state : 2;                               /** 电池绝缘状态       00:正常  01:不正常 01:不可信状态 */
    uint8_t outlinker_state : 2;                               /** 输出连接器状态     00:正常  01:不正常 01:不可信状态 */
    uint8_t is_allow_charge : 2;                               /** 允许充电           00:禁止  01:允许 */
    uint8_t reserve : 2;                                       /** 预留 */
};

/** BEM报文 */
struct bem{
    uint8_t crm_00_timeout : 2;                                /** 接收CRM_A 00超时   00:正常   01:超时  01:不可信状态 */
    uint8_t crm_aa_timeout : 2;                                /** 接收CRM_A AA超时   00:正常   01:超时  01:不可信状态 */
    uint8_t : 4;

    uint8_t cts_cml_timeout : 2;                               /** 接收CTS_A.CML_A超时  00:正常   01:超时  01:不可信状态 */
    uint8_t cro_aa_timeout : 2;                                /** 接收CRO_A超时      00:正常   01:超时  01:不可信状态 */
    uint8_t : 4;

    uint8_t ccs_timeout : 2;                                   /** 接收CCS_A超时      00:正常   01:超时  01:不可信状态 */
    uint8_t cst_timeout : 2;                                   /** 接收CST_A超时      00:正常   01:超时  01:不可信状态 */
    uint8_t : 4;

    uint8_t csd_timeout : 2;                                   /** 接收CSD_A超时      00:正常   01:超时  01:不可信状态 */
    uint8_t : 6;
};

/** BSD报文 */
struct bsd{
    uint8_t end_soc;                                           /** 终止电荷状态       1%/bit    0-100% */
    uint16_t singlevolt_lowest;                                /** 最低单体电压       0.01V/bit  0-24V */
    uint16_t singlevolt_highest;                               /** 最高单体电压       0.01V/bit  0-24V */
    uint8_t temp_lowest;                                       /** 动力电池最低温度   0.1°/bit  -50-200 偏移-50 */
    uint8_t temp_highest;                                      /** 动力电池最高温度   0.1°/bit  -50-200 偏移-50 */
};

/** 其它数据 */
struct other_data{
    uint8_t protocol_type;                                     /** 超级(大)电流协议类型@enum currprotocol */
    uint16_t current_offset;                                   /** 电流偏移(0.1A) */
};

/** 可能后续会增加其它数据段 */

/** 设备运行实时信息：数据信息 */
struct running_data_info{
    uint8_t segment_num;                         /* 有效段数 */
#if 0
    /** 数据段1 */
    struct{
        /** 以下是数据段信息 */
    }segment;
    /** 数据段2 */
    struct{
        /** 以下是数据段信息 */
    }segment;
    .......
#endif
};

typedef struct{
    Net_YkcMonitorPro_Head_t head;
    struct{
        uint8_t pile_number[NET_YKC_MONITOR_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t gunno;                           /* 枪号 */
        uint8_t info_type;                       /* 信息类型@enum ykcm_dev_running */
        uint8_t option;                          /* 查询：0或上报：1 */
        uint8_t msg_version;                     /* 报文版本(初始版本为0) */
        /* 信息数据 */
    }body;
    uint16_t check_sum;                          /* 校验码 */
}Net_YkcMonitorPro_PreqReport_SreqQuery_RealtimeInfo_t;

#endif /* NET_YKC_MONITOR_USING_EXTEND_PROTOCOL */

#endif /* NET_YKC_MONITOR_AS_MONITOR */

#pragma pack()

#endif /* NET_PACK_USING_YKC_MONITOR */

#endif /* NET_NET_YKC_MONITOR_INC_YKC_MONITOR_MESSAGE_STRUCT_DEFINE_H_ */
