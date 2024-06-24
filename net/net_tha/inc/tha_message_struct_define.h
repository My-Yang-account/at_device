/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_THA_INC_THA_MESSAGE_STRUCT_DEFINE_H_
#define NET_PACK_NET_THA_INC_THA_MESSAGE_STRUCT_DEFINE_H_

#include "net_pack_config.h"

#ifdef NET_PACK_USING_THA

#define NET_THA_MESSAGE_START_CODE_REQ                         0x68        /* 请求起始码 */
#define NET_THA_MESSAGE_START_CODE_RES                         0xAA        /* 响应起始码 */
#define NET_THA_MESSAGE_FLAG                                   0x00        /* 报文标志 */

#define NET_THA_PILE_TYPE_SINGLE_GUN                           0x01        /* 桩类型：单枪 */
#define NET_THA_PILE_TYPE_DOUBLE_GUN                           0x02        /* 桩类型：双枪 */

/** 客户标识定义 */
#ifdef NET_THA_AS_MONITOR
#define NET_THA_USER_PROVIDER                                  0x80
#define NET_THA_USER_MAGIC_NUMBER                              0xCDA58B7A
#else
#define NET_THA_USER_PROVIDER                                  0x7A
#define NET_THA_USER_MAGIC_NUMBER                              0xCDA58B7E
#endif /* NET_THA_AS_MONITOR */
#define NET_THA_PROTOCOL_VERSION                               0x0201      /* 通信协议版本号 */
#define NET_THA_SOFTWARE_MODEL                                 "TH0805AC"  /* 嵌入式软件型号 */

#define NET_THA_UPDATE_SEGMENT_LENGTH_MAX                      1024
#define NET_THA_CHARGEPILE_TYPE_AC                             0x00
#define NET_THA_CHARGEPILE_TYPE_DC                             0x01

#define NET_THA_CHARGEPILE_LENGTH_DEFAULT                      0x14        /* 默认桩号长度 */
#define NET_THA_CARD_NUMBER_LENGTH_MAX                         0x14        /* 卡号长度最大值 */
#define NET_THA_CAR_VIN_NUMBER_LENGTH_MAX                      0x11        /* 车VIN码长度最大值 */

/* 使用扩展部分 */
#define USING_EXPAND_SECTION

enum tha_status{
    NET_THA_STATUS_CODE_NORMAL                            = 0x00,      /* 正常 */
    NET_THA_STATUS_CODE_FORMAT_ERROR                      = 0x11,      /* 请求格式错误 */
    NET_THA_STATUS_CODE_NO_AUTHORITY                      = 0x12,      /* 无权限 */
    NET_THA_STATUS_CODE_REFUSE_SERVICE                    = 0x13,      /* 拒绝提供服务 */
    NET_THA_STATUS_CODE_CMD_ISNOT_EXIT                    = 0x14,      /* 指令不存在 */
    NET_THA_STATUS_CODE_DEVICE_NOT_SUPPORT                = 0x15,      /* 设备不支持 */
    NET_THA_STATUS_CODE_INTERNAL_ERROR_OF_SERVER          = 0x50,      /* 服务器内部出错 */
    NET_THA_STATUS_CODE_EXECUTE_FAIL                      = 0x51,      /* 指令执行失败 */
};

enum tha_cmd{
    NET_THA_MESSAGE_CMD_LOGIN                             = 0x10,      /* 登录签到 */
    NET_THA_MESSAGE_CMD_AUTHORIZE                         = 0x11,      /* 权限认证 */
    NET_THA_MESSAGE_CMD_CHARGE_DATA                       = 0x12,      /* 上报充电数据 */
    NET_THA_MESSAGE_CMD_HISTORY_ORDER                     = 0x14,      /* 上报历史订单 */
    NET_THA_MESSAGE_CMD_HEARTBEAT                         = 0x15,      /* 上报心跳 */
    NET_THA_MESSAGE_CMD_UPDATE_PACK_LOAD                  = 0x16,      /* 请求升级包下载 */
    NET_THA_MESSAGE_CMD_REPORT_BATTERY_STATE              = 0x17,      /* 上报电池状态状态 */
    NET_THA_MESSAGE_CMD_STOP_CHARGE_COMPLETE              = 0x18,      /* 停止充电动作完成 */
    NET_THA_MESSAGE_CMD_EVENT_STATE                       = 0x19,      /* 上报事件 */
    NET_THA_MESSAGE_CMD_ERROR_INFO                        = 0x1A,      /* 上报错误信息 */
    NET_THA_MESSAGE_CMD_QUERY_PILE_NUMBER_OF_ICCID        = 0x1C,      /* 上报错误信息 */
    NET_THA_MESSAGE_CMD_VIN_AUTHORIZE                     = 0x20,      /* vin 码鉴权 */
    NET_THA_MESSAGE_CMD_UPDATEPACK_LOADRESULT             = 0x21,      /* 上报升级包下载结果 */
    NET_THA_MESSAGE_CMD_NET_INFO                          = 0x22,      /* 上报设备网络信息 */
    NET_THA_MESSAGE_CMD_REPORT_BATTERY_INFO               = 0x25,      /* 上报电池信息 */
    NET_THA_MESSAGE_CMD_DEVICE_FAULT_INFO                 = 0x26,      /* 上报设备故障信息 */
    NET_THA_MESSAGE_CMD_CAR_WARNNING_INFO                 = 0x27,      /* 上报车辆告警信息 */
    NET_THA_MESSAGE_CMD_GUN_REALTEMP                      = 0x2C,      /* 上报充电中充电枪的实时温度 */
    NET_THA_MESSAGE_CMD_TIME_SYNC                         = 0x30,      /* 时间同步 */
    NET_THA_MESSAGE_CMD_SYSTEM_REBOOT                     = 0x31,      /* 系统重启 */
    NET_THA_MESSAGE_CMD_QUERY_CHARGE_DATA                 = 0x33,      /* 查询充电数据 */
    NET_THA_MESSAGE_CMD_QUERY_SYSTEM_INFO                 = 0x36,      /* 查询系统信息 */
    NET_THA_MESSAGE_CMD_QUERY_BILLING_RULE                = 0x38,      /* 查询计费规则 */
    NET_THA_MESSAGE_CMD_UPDATED_BILLING_RULE              = 0x39,      /* 更新计费规则 */
    NET_THA_MESSAGE_CMD_SET_RESERVATION                   = 0x3A,      /* 设置定时充电 */
    NET_THA_MESSAGE_CMD_CANCEL_RESERVATION                = 0x3B,      /* 取消定时充电 */
    NET_THA_MESSAGE_CMD_STOP_CHARGE                       = 0x3D,      /* 停止充电 */
    NET_THA_MESSAGE_CMD_SERVER_START_UPDATE               = 0x3E,      /* 开始升级 */
    NET_THA_MESSAGE_CMD_QUERY_CHARGE_SYSTEM_SET           = 0x40,      /* 查询充电系统设置 */
    NET_THA_MESSAGE_CMD_QUERY_DEVICE_FAULT_INFO           = 0x42,      /* 查询设备故障信息 */
    NET_THA_MESSAGE_CMD_QUERY_BATTERY_CHARGE_INFO         = 0x43,      /* 查询电池充电信息 */
    NET_THA_MESSAGE_CMD_QUERY_ORDER_STATE                 = 0x44,      /* 查询订单状态 */
    NET_THA_MESSAGE_CMD_SET_PILE_RUNNING_PARA             = 0x48,      /* 设置桩运行参数 */
    NET_THA_MESSAGE_CMD_SET_PILE_CHARGE_POWER             = 0x49,      /* 设置桩充电功率 */
    NET_THA_MESSAGE_CMD_UNLOCK_CHARGE                     = 0x51,      /* 解锁充电 */
    NET_THA_MESSAGE_CMD_ISSUED_PROCESS_PARA               = 0x52,      /* 平台下发处理用参数 */
    NET_THA_MESSAGE_CMD_SET_PRIVATE_RESERVATION           = 0x56,      /* 私桩定时充电预约 */
    NET_THA_MESSAGE_CMD_CANCEL_PRIVATE_RESERVATION        = 0x57,      /* 私桩取消特定ID的定时充电预约 */
    NET_THA_MESSAGE_CMD_QUERY_AVAILABLE_PACK_VER          = 0x58,      /* 设备查询最新可用升级包软件版本号 */
    NET_THA_MESSAGE_CMD_DEVICE_START_UPDATE               = 0x59,      /* 设备发起远程升级请求 */
    NET_THA_MESSAGE_CMD_QUERY_PRIVATE_RESERVATION         = 0x6A,      /* 私桩(桩端)查询定时充电预约 */
    NET_THA_MESSAGE_CMD_ISSUDE_PWM                        = 0x6B,      /* 私桩PWM电流值下发 */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * ThaPro   as Thaisen Protocol               钛享协议
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

/** 协议头部 */
typedef struct{
    uint8_t start_code;                           /* 起始码 */
    uint8_t cmd;                                  /* 指令码 */
    uint8_t status;                               /* 状态码 */
    uint8_t sequence;                             /* 序列号 */
    uint16_t length;                              /* body 长度 */
    uint8_t flag;                                 /* 保留标志 */
    uint8_t check_sum;                            /* 校验码 */
}Net_ThaPro_Head_t;

/** 通用请求、响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t gunno;                            /* 枪号 */
    }body;
}Net_ThaPro_GeneralResReq_t;

/** 登录签到帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t pile_number[NET_THA_CHARGEPILE_LENGTH_DEFAULT];  /* 桩号*/
        uint8_t pile_type;                       /* 桩类型 */
        uint8_t client_mark;                     /* 客户端标识 */
        uint32_t magic_number;                   /* magic number，配合Provider使用 */
        struct{
            uint16_t protocol_ver;               /* 通信协议版本；数字，高字节为次版本号、低字节为主版本号 */
            uint8_t software_ver_main;           /* 设备嵌入式软件版本；数字，字节次序由高到低依次为修订版本号、次版本号、主版本号 */
            uint8_t software_ver_sub;            /* 设备嵌入式软件版本；数字，字节次序由高到低依次为修订版本号、次版本号、主版本号 */
            uint8_t software_ver_revise;         /* 设备嵌入式软件版本；数字，字节次序由高到低依次为修订版本号、次版本号、主版本号 */
            uint8_t software_model[10];          /* 字符串，嵌入式软件型号（各设备厂商自行定义）2.01中桩信息中增加通信协议版本号、嵌入式软件版本号和设备类型，其他字节备用 */
            uint8_t tbd[5];                      /* TBD */
        }pile_info;                              /* 桩信息 */
        struct{
            uint8_t tbd[20];                     /* TBD */
        }isp_info;                               /* 运营商信息  */
        uint8_t random_sign;                     /* 随机签名参数 */
    }body;
}Net_ThaPro_PReq_LogIn_t;

/** 登录签到响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t random_sign;                     /* 随机签名参数 */
        uint32_t timestamp;                      /* 时间戳 */
    }body;
}Net_ThaPro_SRes_LogIn_t;

/** 权限认证帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t card_type;                       /* 卡类型 */
        uint8_t card_number_length;              /* 卡号长度 */
        uint8_t card_number[NET_THA_CARD_NUMBER_LENGTH_MAX];  /* 卡号 */
        uint32_t card_ballance;                  /* 卡余额 */
        uint8_t unpay_num;                       /* 未结算次数 */
        uint8_t gunno;                           /* 枪端口号 */
    }body;
}Net_ThaPro_PReq_Authority_t;

/** 权限认证响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                        /* 用户ID */
        uint32_t trade_no;                       /* 流水号 */
        uint32_t account_ballance;               /* 账户余额 */
        uint8_t gunno;                           /* 枪端口号 */
    }body;
}Net_ThaPro_SRes_Authority_t;

/** 充电数据帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                       /* 流水号 */
        uint16_t elect;                          /* 充电电量  */
        uint16_t voltage;                        /* 充电电压 */
        uint16_t current;                        /* 充电电流 */
        uint32_t charge_time;                    /* 充电时长*/
        uint32_t expand_voltage;                 /* 扩充充电电压 *//////////////////////////////////////////// 扩展
        uint8_t gunno;                           /* 枪端口号 */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];/* 车辆VIN码 */
    }body;
}Net_ThaPro_PReq_Report_ChargeData_t;

/** 历史订单帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                       /* 流水号 */
        uint16_t elect;                          /* 充电电量 */
        uint32_t start_timestamp;                /* 充电起始时间 */
        uint32_t stop_timestamp;                 /* 充电结束时间 */
        uint32_t charge_time;                    /* 充电时长 */
        uint32_t charge_fee;                     /* 充电费用 */
        uint8_t is_pay_by_card;                  /* 卡结算标志 */
        uint8_t user_mark_type;                  /* 用户标识类型  */
        uint8_t user_mark_type_length;           /* 用户标识类型长度 */
        union{
            uint32_t user_id;                    /* 用户ID */
            uint8_t card_number[NET_THA_CARD_NUMBER_LENGTH_MAX]; /* 卡号 */
            uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX]; /* VIN 码 */
        }user_mark;                              /* 用户标识 */
        uint8_t gunno;                           /* 枪号 */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];/* VIN 码 */
    }body;
}Net_ThaPro_PReq_Report_HistoryOrder_t;

/** 心跳帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t device_state;                    /* 设备状态 */
        uint8_t gunno;                           /* 枪端口号 */
    }body;
}Net_ThaPro_PReq_Report_HeartBeat_t;

/** 请求下载升级包帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t start_byte;                     /* 起始字节 */
        uint32_t end_byte;                      /* 结束字节 */
    }body;
}Net_ThaPro_PReq_UpdatePack_Load_t;

/** 请求下载升级包响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t check_code;                     /* 校验码 */
        uint32_t pack_len;                       /* 文件块长度  */
        uint8_t data;                            /* 文件块内容  */
    }body;
}Net_ThaPro_SRes_UpdatePack_Load_t;

/** 上报电池充电状态帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                       /* 流水号   */
        uint8_t charge_mode;                     /* 充电模式  */
        uint16_t remain_time;                    /* 剩余充电时间  */
        uint8_t soc;                             /* 电池荷电状态  */
        uint8_t battery_temp;                    /* 动力电池温度  */
        uint16_t single_voltage_max;             /* 单体最高电压  */
        uint8_t gunno;                           /* 枪端口号   */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX]; /* 车辆VIN码 */
    }body;
}Net_ThaPro_PReq_Battery_ChargeState_t;

/** 停止充电动作完成，上报信息帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                       /* 流水号  */
        uint16_t elect;                          /* 充电电量   */
        uint32_t start_timestamp;                /* 充电起始时间  */
        uint32_t stop_timestamp;                 /* 充电结束时间   */
        uint32_t charge_time;                    /* 充电时长  */
        uint32_t charge_fee;                     /* 充电费用  */
        uint8_t is_pay_by_card;                  /* 卡结算标志  */
        struct{
            uint8_t reach_target_soc : 2;        /* 达到所需求的SOC目标值 */
            uint8_t reach_set_value_volt : 2;    /* 达到总电压的设定值  */
            uint8_t reach_set_value_single_volt : 2; /* 达到单体电压的设定值  */
            uint8_t charger_stop : 2;            /* 充电机主动中止  */
        }bms_stop_reason;                        /* BMS中止充电原因  */
        struct{
            uint16_t insultion : 2;              /* 绝缘故障  */
            uint16_t olinker_overtemp : 2;       /* 输出连接器过温故障   */
            uint16_t bmsunit_olinker_overtemp : 2; /* BMS元件、输出连接器过温  */
            uint16_t charge_linker : 2;          /* 充电连接器故障  */
            uint16_t battery_group_overtemp : 2; /* 电池组温度过高故障  */
            uint16_t hv_relay : 2;               /* 高压继电器故障  */
            uint16_t volt_detect_point_2 : 2;    /* 检测点2电压检测故障  */
            uint16_t other : 2;                  /* 其他故障  */
        }bms_fault_reason;                       /* BMS充电故障原因  */
        uint8_t finish_reason;                   /* 充电结束原因  */
        uint8_t user_mark_type;                  /* 用户标识类型  */
        uint8_t user_mark_type_length;           /* 用户标识类型长度  */
        union{
            uint32_t user_id;                    /* 用户ID  */
            uint8_t card_number[NET_THA_CARD_NUMBER_LENGTH_MAX]; /* 卡号  */
            uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];   /* VIN 码  */
        }user_mark;                              /* 用户标识  */
        uint8_t gunno;                           /* 枪端口号  */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX]; /* VIN 码   */
    }body;
}Net_ThaPro_PReq_StopCharge_Complete_t;

/** 事件状态上报帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t device_event;                    /* 充电设备事件  */
        uint8_t gunno;                           /* 枪端口号  */
    }body;
}Net_ThaPro_PReq_Report_EventState_t;

/** 错误信息上报帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t device_type;                     /* 设备类型  */
        uint8_t error_mark;                      /* 错误标识  */
        uint8_t error_type;                      /* 错误类型  */
        uint8_t error_code[3];                   /* 错误代码  */
        uint32_t timestamp;                      /* 时间戳  */
        uint8_t gunno;                           /* 枪端口号  */
    }body;
}Net_ThaPro_PReq_Report_ErrorInfo_t;

/** 上传车辆行驶里程帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t mileage;                        /* 车辆行驶里程   */
        uint8_t gunno;                           /* 枪端口号  */
    }body;
}Net_ThaPro_PReq_CarMileage_t;

/** 查询ICCID对应的桩编号帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t iccid[20];                            /* ICCID */
    }body;
}Net_ThaPro_PReq_Query_PileNumberOfIccid_t;

/** 查询ICCID对应的桩编号响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t pile_number[20];                      /* 桩编号 */
    }body;
}Net_ThaPro_SRes_Query_PileNumberOfIccid_t;

/** 设备上报桩编号和ICCID的绑定关系（工厂模式）帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t pile_number[20];                       /* 桩编号 */
        uint8_t iccid[20];                             /* ICCID */
        uint8_t pile_type_charge;                      /* 桩类型(充电方面) */
        uint8_t pile_type_usage;                       /* 桩类型(用途方面) */
        uint8_t install_way;                           /* 安装方式 */
        uint8_t gun_situation;                         /* 带枪情况 */
        uint8_t age_of_use;                            /* 使用年限 */
        uint16_t rated_of_current;                     /* 额定电流 */
        uint16_t rated_of_power;                       /* 额定功率 */
        uint8_t device[20];                            /* 设备类型 */
    }body;
}Net_ThaPro_PReq_Report_BindTie_IccidAndPileNumber_t;

/** VIN码鉴权帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t vin_state;                             /* VIN码状态 */
        uint8_t vin_length;                            /* VIN码长度 */
        uint8_t vin[20];                               /* VIN码 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PReq_VinAuthorize_t;

/** VIN码鉴权响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                              /* 用户ID */
        uint32_t trade_no;                             /* 流水号 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_SRes_VinAuthorize_t;

/** 升级包下载结果帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t result;                                /* 结果 */
    }body;
}Net_ThaPro_PReq_UpdatePack_LoadResult_t;

/** 设备网络信息上传帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t networked_way;                         /* 联网方式 */
        uint16_t mcc;                                  /* 国家码 */
        uint16_t mnc;                                  /*  */
        uint16_t lac;                                  /*  */
        uint16_t ci;                                   /*  */
        uint8_t imei[16];                              /* 移动设备身份码 */
        uint8_t ram[6];                                /* 充电设备RAM大小 */
        uint8_t rom[6];                                /* 充电设备ROM大小 */
        uint8_t mac[6];                                /*  */
        uint8_t iccid[20];                             /* SIM卡卡号 */
        uint8_t imsi[15];                              /*  */
    }body;
}Net_ThaPro_PReq_Report_NetInfo_t;

/** 上报电池信息、 查询电池充电信息帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t battery_type;                          /* 电池类型 */
        uint8_t allow_temp_max;                        /* 最高允许温度 */
        uint16_t allow_chargevolt_max_ofbms;           /* BMS最高允许充电电压 */
        uint16_t allow_chargevolt_max_ofbsingle;       /* 单体最高允许充电电压 */
        uint16_t allow_chargecurr_max;                 /* 最高允许充电电流 */
        uint16_t battery_ratedvolt_total;              /* 整车动力蓄电池额定总电压 */
        uint16_t battery_voltage_current;              /* 整车动力蓄电池当前电压 */
        uint16_t battery_rated_capacity;               /* 整车动力蓄电池额定容量 */
        uint16_t battery_nominal_power;                /* 整车动力蓄电池标称能量 */
        uint8_t gunno;                                 /* 枪号 */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];        /* vin码 */
    }body;
}Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_t;

/** 上报设备故障信息、查询设备故障信息帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint16_t ac_device_fault_state;                 /* 交流设备故障状态 */
        uint32_t dc_device_fault_state;                 /* 直流设备故障状态 */
        uint16_t dc_chargemodule_fault_state;           /* 直流充电模块故障状态 */
    }body;
}Net_ThaPro_PReq_Report_PRes_Query_Device_FaultInfo_t;

/** 上报车辆告警信息帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                              /* 流水号 */
        uint8_t error_type;                             /* 错误类型 */
        uint32_t battery_temp;                          /* 电池温度 */
        uint8_t gunno;                                  /* 枪号 */
    }body;
}Net_ThaPro_PReq_Report_Car_WarnningInfo_t;

/** 上报电表读数帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t elect;                                /* 电量 */
        uint32_t timstamp;                             /* 时间戳 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PReq_Report_AmmeterData_t;

/** 上报充电枪额定温度帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        int16_t rated_temp;                            /* 充电枪额定温度 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PReq_Report_Gun_RatedTemp_t;

/** 上报充电中充电枪的实时温度帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        int16_t real_temp;                             /* 充电枪实时温度 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PReq_Report_Gun_RealTemp_t;

/** 上报温度传感器实时状态帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t sensors_1;                             /* 传感器1状态 */
        uint8_t sensors_2;                             /* 传感器2状态 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PReq_Report_TempSensors_State_t;

/** 时间同步帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t timpstamp;                            /* 时间戳 */
    }body;
}Net_ThaPro_SReqPRes_TimeSync_t;

/** 系统复位（重启）指令帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t timpstamp;                            /* 时间戳 */
    }body;
}Net_ThaPro_SReqPRes_SystemReboot_t;

/** 查询充电数据响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t start_time;                           /* 开始时间 */
        uint32_t elect_total;                          /* 累计电量 */
        uint32_t trade_no;                             /* 流水号 */
        uint16_t elect;                                /* 充电电量 */
        uint32_t volatge;                              /* 充电电压 */
        uint16_t current;                              /* 充电电流 */
        uint32_t charge_time;                          /* 充电时长 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PRes_Query_ChargeData_t;

/** 用户绑定帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t card_type;                             /* 卡类型 */
        uint8_t card_length;                           /* 卡号长度 */
        uint8_t card_number[20];                       /* 卡号 */
    }body;
}Net_ThaPro_SReq_User_Binding_t;

/** 查询系统信息响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t software_ver[3];                       /* 软件版本1 */
        uint8_t pile_name_length;                      /* 充电桩名称长度 */
        uint8_t pile_name[20];                         /* 充电桩名称 */
        uint8_t operator_name_length;                  /* 运营商名称长度 */
        uint8_t operator_name[20];                     /* 运营商名称 */
        uint8_t server_domain_length;                  /* 服务器域名长度 */
        uint16_t port;                                 /* 端口号 */
    }body;
}Net_ThaPro_PRes_Query_SystemInfo_t;

/** 更新系统信息帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t pile_name_length;                      /* 充电桩名称长度 */
        uint8_t pile_name[20];                         /* 充电桩名称 */
        uint8_t operator_name_length;                  /* 运营商名称长度 */
        uint8_t operator_name[20];                     /* 运营商名称 */
        uint8_t server_domain_length;                  /* 服务器域名长度 */
        uint8_t* server_domain;                        /* 服务器域名 */
        uint16_t port;                                 /* 端口号 */
    }body;
}Net_ThaPro_SReq_Updated_SystemInfo_t;

/** 查询计费规则响、更新计费规则帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint16_t rate_1;                               /* 一号费率 */
        uint16_t rate_2;                               /* 二号费率 */
        uint16_t rate_3;                               /* 三号费率 */
        uint16_t rate_4;                               /* 四号费率 */
        uint16_t service_rate;                         /* 服务费 */
        uint8_t period_rate_number[24];                /* 时段费率号(一小时一个时段) */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_SReq_Update_PRes_Query_BillingRule_t;

/** 查询充电系统设置、更新充电系统信息帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint16_t voltage_max;                          /* 最大电压 */
        uint16_t voltage_min;                          /* 最小电压 */
        uint16_t current_max;                          /* 最大电流 */
        uint8_t power_module_num;                      /* 功率模块数量 */
        uint16_t power_of_single_module;               /* 单模块功率 */
        uint16_t power_limit_of_ac_charge;             /* 交流充电功率限制 */
#ifdef USING_EXPAND_SECTION
        uint32_t expand_voltage_max;                   /* 扩充最大电压 */
        uint32_t expand_current_min;                   /* 扩充最小电压 */
#endif /* USING_EXPAND_SECTION */
    }body;
}Net_ThaPro_SReq_Updated_PRes_Query_ChargeSystem_Config_t;

/** 查询订单状态帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint32_t trade_no;                            /* 流水号 */
    }body;
}Net_ThaPro_SReq_Query_OrderState_t;

/** 查询订单状态响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                            /* 流水号 */
        uint8_t state;                                /* 状态 */
    }body;
}Net_ThaPro_PRes_Query_OrderState_t;

/** 设置定时充电帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint32_t start_time;                          /* 开始时间 */
        uint32_t stop_time;                           /* 结束时间 */
        uint8_t charge_mode;                          /* 充电模式 */
        uint8_t pwm_value;                            /* pwm值 */
        uint8_t gunno;                                /* 枪号 */
    }body;
}Net_ThaPro_SReq_Set_Reservation_t;

/** 取消定时充电帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint8_t gunno;                                /* 枪号 */
    }body;
}Net_ThaPro_SReq_Cancel_Reservation_t;

/** 停止充电帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint8_t gunno;                                /* 枪号 */
    }body;
}Net_ThaPro_SReq_StopCharge_t;

/** 启动远程升级帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t client_identification;                /* 客户端标识 */
        uint8_t software_ver[3];                      /* 软件版本 */
        uint32_t update_pack_len;                     /* 升级包长度 */
        uint32_t update_pack_check;                   /* 升级包检验码 */
        uint32_t segment_len;                         /* 分段下载字节长度 */
        uint8_t updated_way;                          /* 升级方式 */
        uint8_t updated_addr_len;                     /* 升级地址长度 */
        uint8_t updated_addr[6];                      /* 升级地址(默认TCP) */
    }body;
}Net_ThaPro_SReq_StartUpdate_t;

/** 设置桩运行参数帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t heartbeat_interval;                   /* 心跳上报间隔 */
        uint8_t charge_data_interval;                 /* 充电数据上报间隔 */
        uint8_t reserve[50];                          /* 保留 */
        uint8_t gunno;                                /* 枪号 */
    }body;
}Net_ThaPro_SReq_Set_Pile_RunningPara_t;

/** 设置充电枪充电功率帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint16_t charge_power;                        /* 充电功率 */
        uint8_t gunno;                                /* 枪号 */
    }body;
}Net_ThaPro_SReq_Set_Pile_ChargePower_t;

/** 解锁充电帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint32_t trade_no;                            /* 流水号 */
        uint8_t gunno;                                /* 枪号 */
    }body;
}Net_ThaPro_SReq_UnlockCharge_t;

/** 上报电池信息（充电电流大于655A时用）、查询电池信息（充电电流大于655A时用）帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t battery_type;                          /* 电池类型 */
        uint16_t allow_temp_max;                       /* 最高允许温度 */
        uint32_t allow_chargevolt_max_ofbms;           /* BMS最高允许充电电压 */
        uint32_t allow_chargevolt_max_ofbsingle;       /* 单体最高允许充电电压 */
        uint32_t allow_chargecurr_max;                 /* 最高允许充电电流 */
        uint32_t battery_ratedvolt_total;              /* 整车动力蓄电池额定总电压 */
        uint32_t battery_voltage_current;              /* 整车动力蓄电池当前电压 */
        uint32_t battery_rated_capacity;               /* 整车动力蓄电池额定容量 */
        uint32_t battery_nominal_power;                /* 整车动力蓄电池标称能量 */
        uint8_t gunno;                                 /* 枪号 */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];      /* vin码 */
    }body;
}Net_ThaPro_PReq_Report_PRes_Query_BatteryInfo_Expand_t;

/** 上报充电数据（充电电流大于655A时用）帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                             /* 流水号 */
        uint16_t elect;                                /* 电量 */
        uint32_t voltage;                              /* 电压 */
        uint32_t current;                              /* 电流 */
        uint32_t charge_time;                          /* 充电时长 */
        uint8_t gunno;                                 /* 枪号 */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];      /* vin码 */
    }body;
}Net_ThaPro_PReq_ChargeData_Expand_t;

/** 上报电池充电状态（充电电流大于655A时用）帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t trade_no;                             /* 流水号 */
        uint8_t charge_mode;                           /* 充电模式 */
        uint16_t remain_time;                          /* 剩余充电时间 */
        uint8_t soc;                                   /* SOC */
        uint8_t battery_temp;                          /* 电池温度 */
        uint16_t single_voltage_max;                   /* 单体最高电压 */
        uint32_t voltage_of_require;                   /* 需求充电电压 */
        uint32_t current_of_require;                   /* 需求充电电流 */
        uint8_t gunno;                                 /* 枪号 */
        uint8_t vin[NET_THA_CAR_VIN_NUMBER_LENGTH_MAX];      /* vin码 */
    }body;
}Net_ThaPro_PReq_BatteryState_Expand_t;

/** 查询充电数据响应（充电电流大于655A时用）帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t start_time;                           /* 开始时间 */
        uint32_t elect_total;                          /* 累计电量 */
        uint32_t trade_no;                             /* 流水号 */
        uint16_t elect;                                /* 充电电量 */
        uint32_t volatge;                              /* 充电电压 */
        uint32_t current;                              /* 充电电流 */
        uint32_t charge_time;                          /* 充电时长 */
        uint8_t gunno;                                 /* 枪号 */
    }body;
}Net_ThaPro_PRes_Query_ChargeData_Expand_t;

/** 设备上报处理用参数、平台下发处理用参数帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t leakage_current;                      /* 漏电流 */
        uint8_t pwm_rank;                             /* pwm等级 */
        uint8_t under_voltage;                        /* 欠压 */
        uint8_t over_current;                         /* 过流 */
        uint8_t under_frequency;                      /* 欠频 */
        uint8_t over_frequency;                       /* 过频 */
        uint8_t ground;                               /* 接地 */
        uint8_t temperature;                          /* 温度 */
        uint8_t over_voltage;                         /* 过压 */
        uint8_t mode_plug_and_play;                   /* 即插即充模式 */
        uint8_t mode_blue_authorize;                  /* 蓝牙授权模式 */
        uint8_t mode_logo_lighting;                   /* logo常亮模式 */
    }body;
}Net_ThaPro_PReq_Report_SReq_Issued_ProcessPara_t;

/** 私桩定时充电预约帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint32_t start_time;                          /* 开始时间 */
        uint32_t stop_time;                           /* 结束时间 */
        uint8_t serial_number[27];                    /* 序列号 */
        uint8_t effectiveness;                        /* 定时预约充电有效性 */
        uint32_t trade_no;                            /* 流水号 */
    }body;
}Net_ThaPro_SReq_Set_PrivateReservation_t;

/** 私桩取消特定ID的定时充电预约帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint32_t user_id;                             /* 用户id */
        uint8_t serial_number[27];                    /* 序列号 */
    }body;
}Net_ThaPro_SReq_Cancel_PrivateReservation_t;

/** 设备查询最新可用升级包软件版本号帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t client_identification;                /* 客户端标识 */
        uint8_t device_model[10];                     /* 设备型号 */
    }body;
}Net_ThaPro_PReq_Query_Available_UpdatePackVer_t;

/** 设备查询最新可用升级包软件版本号响应帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t client_identification;                /* 客户端标识 */
        uint8_t device_model[10];                     /* 设备型号 */
        uint8_t update_pack_ver[3];                   /* 升级包版本 */
        uint16_t update_pack_id;                      /* 升级包id */
    }body;
}Net_ThaPro_SRes_Query_Available_UpdatePackVer_t;

/** 设备发起远程升级请求帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t pile_number[20];                      /* 桩号 */
        uint8_t client_identification;                /* 客户端标识 */
        uint8_t device_model[10];                     /* 设备型号 */
        uint8_t software_ver[3];                      /* 软件版本 */
        uint16_t update_pack_id;                      /* 升级包id */
    }body;
}Net_ThaPro_PReq_Start_Update_t;

/** 私桩(桩端)查询定时充电预约响应帧 */
typedef struct{
    uint32_t user_id;                                 /* 用户id */
    uint8_t serial_number[27];                        /* 序列号 */
    uint32_t start_time;                              /* 开始时间 */
    uint32_t stop_time;                               /* 结束时间 */
    uint8_t effectiveness;                            /* 定时预约充电有效性 */
    uint32_t trade_no;                                /* 流水号 */
}Net_ThaPro_PrivateReservation_Body_t;

typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t reservation_num;                      /* 预约次数 */
        Net_ThaPro_PrivateReservation_Body_t* info_body; /* 预约信息体 */
    }body;
}Net_ThaPro_SRes_Query_PrivateReservation_t;

/** 私桩PWM电流值下发帧 */
typedef struct{
    Net_ThaPro_Head_t head;
    struct{
        uint8_t pwm_value;                            /* pwm 值 */
    }body;
}Net_ThaPro_SReq_IssudePwm_t;

#endif /* NET_PACK_USING_THA */

#endif /* NET_PACK_NET_THA_INC_THA_MESSAGE_STRUCT_DEFINE_H_ */
