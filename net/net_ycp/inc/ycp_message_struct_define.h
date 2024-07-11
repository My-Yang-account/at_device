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
#define NET_YCP_FAULT_NUM_MAX_DEFAULT                          0x0A        /* 默认上报故障数目最大值 */
#define NET_YCP_QRCODE_LENGTH_DEFAULT                          0x96        /* 默认二维码长度 */

#define NET_YCP_STORAGE_INIT_FLAG                              0x12345678  /* 平台数据存储标志 */
#define NET_YCP_HEARTBEAT_INTERVAL_DEFAULT                     0x3C        /* 默认心跳上报间隔 */
#define NET_YCP_VOICE_VOLUME_DEFAULT                           0x32        /* 默认语音音量 */
#define NET_YCP_DEVICE_PASSWORD_DEFAULT                        0x00B2      /* 默认设备密码 */
#define NET_YCP_TEMP_PROTECT_DEFAULT                           0xFF        /* 默认温度保护值 */
#define NET_YCP_CHARGEDATA_TYPE_DEFAULT                        0x02        /* 默认充电数据上报类型 */
#define NET_YCP_BMSDATA_SWITCH_DEFAULT                         0x01        /* 默认BMS 数据上报开关 */
#define NET_YCP_POWER_PERCENT_DEFAULT                          0x64        /* 默认输出功率百分比 */
#define NET_YCP_MODEL_NUMBER_DEFAULT                           0x00        /* 默认计费模型编号 */

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
        uint8_t state;                           /* 枪状态 */
        uint8_t temp;                            /* 温度 */
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
        uint16_t fault;                          /* 失败故障码 */
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
        uint16_t strategy_para;                  /* 策略参数 */
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
        uint16_t strategy_para;                  /* 策略参数 */
        uint8_t result;                          /* 启动结果 */
        uint8_t reason;                          /* 失败原因 */
        uint16_t fault;                          /* 失败故障码 */
    }body;
}Net_YcpPro_SRes_ApplyCharge_Active_t;

/** 0x19 充电桩主动请求充电 */
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t gunno;                           /* 枪号 */
        uint8_t start_type;                      /* 启动方式 */
        uint8_t phycard_number[NET_YCP_PHYCARD_NUMBER_LENGTH_MAX];  /* 物理卡号 */
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
        uint8_t vin[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX];   /* VIN 码 */
        uint8_t pay_way;                                 /* 支付方式 */
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
        uint8_t user_name[NET_YCP_USER_NAME_LENGTH_DEFAULT];        /* 用户名 */
        uint8_t password[NET_YCP_PASSWORD_LENGTH_DEFAULT];          /* 密码 */
        uint8_t path[NET_YCP_FILE_PATH_LENGTH_DEFAULT];             /* 升级文件路径 */
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
struct faut_info{
    uint8_t fault_type;                          /* 故障类型 */
    uint8_t fault_code;                          /* 故障码 */
};
typedef struct{
    Net_YcpPro_Head_t head;
    struct{
        uint8_t fault_num;                       /* 故障数 */
        struct faut_info fault[NET_YCP_FAULT_NUM_MAX_DEFAULT]; /* 故障信息 */
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
        uint8_t bms_bat_temp_max;                        /* BMS 最高动力蓄电池温度 */
        uint8_t bat_temp_max_measure_number;             /* 最高温度检测点编号 */
        uint8_t bms_bat_temp_min;                        /* 最低动力蓄电池温度 */
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
