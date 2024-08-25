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
enum sgcc_general_stop{
    NETSGCC_GS_REASON1000_CHARGE_FULL = 1000,           /* 常规停止原因：充满停 */
    NETSGCC_GS_REASON1001_SCREEN = 1001,                /* 常规停止原因：触控屏手动停止  */
    NETSGCC_GS_REASON1002_SERVER = 1002,                /* 常规停止原因：后台停止充电 */
    NETSGCC_GS_REASON1003_TARGET_TIME = 1003,           /* 常规停止原因：达到设置充电时长停止 */
    NETSGCC_GS_REASON1004_TARGET_ELECT = 1004,          /* 常规停止原因：达到设置充电电量停止 */
    NETSGCC_GS_REASON1005_TARGET_MONEY = 1005,          /* 常规停止原因：达到设置充电金额停止 */
    NETSGCC_GS_REASON1006_OFFLINE_TIME = 1006,          /* 常规停止原因：达到离线停机条件 */
    NETSGCC_GS_REASON1007_TARGET_SOC = 1007,            /* 常规停止原因：达到 SOC 终止条件停止 */
    NETSGCC_GS_REASON1008_PULL_GUN = 1008,              /* 常规停止原因：枪未正确连接 */
    NETSGCC_GS_REASON1009_SCAN_CODE = 1009,             /* 常规停止原因：扫码停止 */
    NETSGCC_GS_REASON1010_CAR_S2_DISCONNECT = 1010,     /* 常规停止原因：车端 S2 主动断开 */
    NETSGCC_GS_REASON1011_BMS_STOP = 1011,              /* 常规停止原因：BMS 停止充电 */

    NETSGCC_GS_REASON1012_SWIP_CARD = 1012,             /* 常规停止原因：刷卡停 */
};

/** ACA as AC abnormal (交流充电设备异常代码) */
enum sgcc_ac_abnormal{
    NETSGCC_ACA_REASON3000_SELFCHECK_TIMEOUT = 3000,    /* 交流充电设备异常代码：设备自检超时故障 */
    NETSGCC_ACA_REASON3001_OFFLINE = 3001,              /* 交流充电设备异常代码：离线故障 */
    NETSGCC_ACA_REASON3002_OPEN_DOOR = 3002,            /* 交流充电设备异常停代码：柜门被打开故障 （非检修状态） */
    NETSGCC_ACA_REASON3003_CRASH_STOP = 3003,           /* 交流充电设备异常代码：急停按键被按下故障 */
    NETSGCC_ACA_REASON3004_GUN_NOT_HOME = 3004,         /* 交流充电设备异常代码：充电枪未归位告警 */
    NETSGCC_ACA_REASON3005_CARD_READER = 3005,          /* 交流充电设备异常代码：读卡器异常故障 */
    NETSGCC_ACA_REASON3006_AMMETER_COMM = 3006,         /* 交流充电设备异常代码：电表通讯故障故障 */
    NETSGCC_ACA_REASON3007_AMMETER_DATA = 3007,         /* 交流充电设备异常代码：电表数据异常故障 */
    NETSGCC_ACA_REASON3008_OUTLINK_ADH = 3008,          /* 交流充电设备异常代码：输出接触器粘连故障 */
    NETSGCC_ACA_REASON3009_DEV_OVERTEMP = 3009,         /* 交流充电设备异常代码：充电设备过温告警 */
    NETSGCC_ACA_REASON3010_CHARGE_PORT_OVERTEMP = 3010, /* 交流充电设备异常代码：充电接口过温告警 */
    NETSGCC_ACA_REASON3011_ELOCK = 3011,                /* 交流充电设备异常代码：充电接口电子锁故障 */
    NETSGCC_ACA_REASON3012_FLOODING = 3012,             /* 交流充电设备异常代码：水浸故障 */
    NETSGCC_ACA_REASON3013_INTERNAL_COMM = 3013,        /* 交流充电设备异常代码：充电设备内部通讯故障 */
    NETSGCC_ACA_REASON3014_CHARGE_LINK = 3014,          /* 交流充电设备异常代码：充电连接故障 */
    NETSGCC_ACA_REASON3015_PORT_ABNORMAL = 3015,        /* 交流充电设备异常代码：枪口异常故障 */
    NETSGCC_ACA_REASON3016_CARLOCK = 3016,              /* 交流充电设备异常代码：车位锁故障 */
    NETSGCC_ACA_REASON3017_CARLOCK_POWEROFF = 3017,     /* 交流充电设备异常代码：车位锁电池耗尽故障 */
    NETSGCC_ACA_REASON3018_CARDLOCK_UNLOCK_FAIL = 3018, /* 交流充电设备异常代码：车位锁落锁失败故障 */
    NETSGCC_ACA_REASON3019_POWER_ALLOCATE_FAIL = 3019,  /* 交流充电设备异常代码：执行远程功率分配策略失败告警 */
    NETSGCC_ACA_REASON3020_AC_RELAY = 3020,             /* 交流充电设备异常代码：交流接触器故障 */
    NETSGCC_ACA_REASON3021_PULL_GUN_COUNT = 3021,       /* 交流充电设备异常代码：枪头插拔次数告警 */
    NETSGCC_ACA_REASON3022_START_TIMEOUT = 3022,        /* 交流充电设备异常代码：启动充电超时故障 */
    NETSGCC_ACA_REASON3023_START_COMPLETE_RES_FAIL = 3023, /* 交流充电设备异常代码：启动完成应答失败故障 */
    NETSGCC_ACA_REASON3024_GUID_BOARD_COMM = 3024,      /* 交流充电设备异常代码：导引板通讯故障 */
    NETSGCC_ACA_REASON3025_LED_BOARD_COMM = 3025,       /* 交流充电设备异常代码：灯板通讯故障 */
    NETSGCC_ACA_REASON3026_OUTPUT_SHORTS = 3026,        /* 交流充电设备异常代码：输出短路故障 */
    NETSGCC_ACA_REASON3027_LIGHTNING_PROTECTORS = 3027, /* 交流充电设备异常代码：避雷器故障 */
    NETSGCC_ACA_REASON3028_SMOKING = 3028,              /* 交流充电设备异常代码：烟雾故障 */
    NETSGCC_ACA_REASON3029_TRANSACTION_RECORD_FULL = 3029, /* 交流充电设备异常代码：交易记录已满告警 */
};

/** ACPA as AC power abnormal (交流充电电源异常代码)*/
enum sgcc_acpower_abnormal{
    NETSGCC_ACPA_REASON4000_INPUT_POWER = 4000,         /* 交流充电车辆异常代码：输入电源故障（过压、过流、欠压，跳闸）  */
    NETSGCC_ACPA_REASON4001_AC_CIRCUIT_BREAKER = 4001,  /* 交流充电车辆异常代码：交流断路器故障  */
    NETSGCC_ACPA_REASON4002_DETECT_POINT_VOLT = 4002,   /* 交流充电车辆异常代码：检测点电压检测故障  */
    NETSGCC_ACPA_REASON4003_INPUT_MISS_PHASE = 4003,    /* 交流充电电源异常代码：输入缺相告警  */
    NETSGCC_ACPA_REASON4004_LEAKAGE_PROTECT = 4004,     /* 交流充电电源异常代码：漏电保护故障  */
    NETSGCC_ACPA_REASON4005_GROUND = 4005,              /* 交流充电电源异常代码：地线故障  */
    NETSGCC_ACPA_REASON4006_AC_LIGHTING_PROTECTORS = 4006, /* 交流充电电源异常代码：交流防雷故障  */
    NETSGCC_ACPA_REASON4007_THREE_PHASE_NNBALLANCE = 4007, /* 交流充电电源异常代码：三相不平衡告警  */
};

/** ACCA as AC car abnormal (交流充电车辆异常代码)*/
enum sgcc_accar_abnormal{
    NETSGCC_ACCA_REASON5000_OCCUPY_TIMEOUT = 5000,      /* 交流充电车辆异常代码：车辆占位超时告警  */
};

/** DCA as DC abnormal (直流充电设备异常代码) */
enum sgcc_dc_abnormal{
    NETSGCC_DCA_REASON3030_SELFCHECK_TIMEOUT = 3030,    /* 直流充电设备异常代码：设备自检超时故障 */
    NETSGCC_DCA_REASON3031_OFFLINE = 3031,              /* 直流充电设备异常代码：桩离线故障 */
    NETSGCC_DCA_REASON3032_OPNE_DOOR = 3032,            /* 直流充电设备异常代码：柜门被打开故障 （非检修状态） */
    NETSGCC_DCA_REASON3033_CRASH_STOP = 3033,           /* 直流充电设备异常代码：急停按键被按下故障 */
    NETSGCC_DCA_REASON3034_SYSTEM_FAN = 3034,           /* 直流充电设备异常代码：系统风扇故障 */
    NETSGCC_DCA_REASON3035_MODULE_FAN = 3035,           /* 直流充电设备异常代码：模块风扇故障 */
    NETSGCC_DCA_REASON3036_GUN_NOT_HOME = 3036,         /* 直流充电设备异常代码：充电枪未归位告警 */
    NETSGCC_DCA_REASON3037_CARD_READER = 3037,          /* 直流充电设备异常代码：读卡器异常故障 */
    NETSGCC_DCA_REASON3038_MODULE_COMM = 3038,          /* 直流充电设备异常代码：模块通讯故障*/
    NETSGCC_DCA_REASON3039_PMODULE_ADDR_CONFLICT = 3039,/* 直流充电设备异常代码：电源模块地址冲突故障 */
    NETSGCC_DCA_REASON3040_PMODULE_FAULT = 3040,        /* 直流充电设备异常代码：电源模块故障 */
    NETSGCC_DCA_REASON3041_PMODULE_OVERTEMP = 3041,     /* 直流充电设备异常代码：电源模块过温告警 */
    NETSGCC_DCA_REASON3042_NO_USABLE_MODULE = 3042,     /* 直流充电设备异常代码：无空闲模块可用（限智能分配功率） */
    NETSGCC_DCA_REASON3043_AMMETER_COMM = 3043,         /* 直流充电设备异常代码：电表通讯故障 */
    NETSGCC_DCA_REASON3044_AMMETER_DATA = 3044,         /* 直流充电设备异常代码：电表数据异常故障 */
    NETSGCC_DCA_REASON3045_OUTLINK_ADH = 3045,          /* 直流充电设备异常代码：输出接触器粘连故障 */
    NETSGCC_DCA_REASON3046_DC_RELAY = 3046,             /* 直流充电设备异常代码：直流接触器故障 */
    NETSGCC_DCA_REASON3047_DC_FUSE = 3047,              /* 直流充电设备异常代码：直流熔断器故障 */
    NETSGCC_DCA_REASON3048_MIDDLE_RELAY = 3048,         /* 直流充电设备异常代码：中间继电器故障 */
    NETSGCC_DCA_REASON3049_AUXPOWER = 3049,             /* 直流充电设备异常代码：辅助电源故障 */
    NETSGCC_DCA_REASON3050_INSULATION = 3050,           /* 直流充电设备异常代码：绝缘监测故障 */
    NETSGCC_DCA_REASON3051_VENT_CIRCUITS = 3051,        /* 直流充电设备异常代码：泄放回路故障 */
    NETSGCC_DCA_REASON3052_OVERTEMP = 3052,             /* 直流充电设备异常代码：过温告警 */
    NETSGCC_DCA_REASON3053_CHARGE_PORT_OVERTEMP = 3053, /* 直流充电设备异常代码：充电接口过温告警 */
    NETSGCC_DCA_REASON3054_ELOCK = 3054,                /* 直流充电设备异常代码：充电接口电子锁故障 */
    NETSGCC_DCA_REASON3055_FLOODING = 3055,             /* 直流充电设备异常代码：水浸故障*/
    NETSGCC_DCA_REASON3056_INTERNAL_COMM = 3056,        /* 直流充电设备异常代码：内部通讯故障 */
    NETSGCC_DCA_REASON3057_CHARGE_LINK = 3057,          /* 直流充电设备异常代码：充电连接故障 */
    NETSGCC_DCA_REASON3058_GUN_PORT = 3058,             /* 直流充电设备异常代码：枪口异常故障 */
    NETSGCC_DCA_REASON3059_CARLOCK = 3059,              /* 直流充电设备异常代码：车位锁故障 */
    NETSGCC_DCA_REASON3060_CARLOCK_POWER_OFF = 3060,    /* 直流充电设备异常代码：车位锁电池耗尽故障 */
    NETSGCC_DCA_REASON3060_CARLOCK_UNLOCK_FAIL = 3061,  /* 直流充电设备异常代码：车位锁落锁失败故障 */
    NETSGCC_DCA_REASON3062_TRANSACTION_NUMBER_NOMATCH = 3062, /* 直流充电设备异常代码：指令要求终止的订单号不存在或者和目标充电口当前订单不一致 */
    NETSGCC_DCA_REASON3063_POWER_ALLOCATE_FAIL = 3063,  /* 直流充电设备异常代码：执行远程功率 分配策略失败告警 */
    NETSGCC_DCA_REASON3064_DEV_PAUSE_USE = 3064,        /* 直流充电设备异常代码：充电设备暂停使用 */
    NETSGCC_DCA_REASON3065_AC_RELAY = 3065,             /* 直流充电设备异常代码：交流接触器故障 */
    NETSGCC_DCA_REASON3066_PULL_GUN_COUNT = 3066,       /* 直流充电设备异常代码：枪头插拔次数告警 */
    NETSGCC_DCA_REASON3067_SELFCHECK_POWER_ALLOCATE_TIMEOUT = 3067,  /* 直流充电设备异常代码：自检功率分配超时告警 */
    NETSGCC_DCA_REASON3068_PARALLEL_RELAY_ADH = 3068,   /* 直流充电设备异常代码：母联粘连故障 */
    NETSGCC_DCA_REASON3069_READY_CHARGE_TIMEOUT = 3069, /* 直流充电设备异常代码：预充完成超时故障 */
    NETSGCC_DCA_REASON3070_START_TIMEOUT = 3070,        /* 直流充电设备异常代码：启动充电超时 */
    NETSGCC_DCA_REASON3071_START_COMPLETE_RES_FAIL = 3071, /* 直流充电设备异常代码：启动完成应答失败故障 */
    NETSGCC_DCA_REASON3072_MODULE_OPEN_TIMEOUT = 3072,  /* 直流充电设备异常代码：模块开机超时故障 */
    NETSGCC_DCA_REASON3073_POWER_CTRL_MODULE = 3073,    /* 直流充电设备异常代码：功率控制模块故障 */
    NETSGCC_DCA_REASON3074_SWITCH_MODULE = 3074,        /* 直流充电设备异常代码：开关模块故障 */
    NETSGCC_DCA_REASON3075_CCU_COMM = 3075,             /* 直流充电设备异常代码：计费控制单元通讯故障 */
    NETSGCC_DCA_REASON3076_ENVIRON_MONITOR_BOARD_COMM = 3076, /* 直流充电设备异常代码：环境监控板通讯故障 */
    NETSGCC_DCA_REASON3077_AIR_COMM = 3077,             /* 直流充电设备异常代码：空调通讯故障 */
    NETSGCC_DCA_REASON3078_PASSIVE_OUTPUT_COMM = 3078,  /* 直流充电设备异常代码：无源开出盒通讯故障 */
    NETSGCC_DCA_REASON3078_PASSIVE_INPUT_COMM = 3079,   /* 直流充电设备异常代码：无源开入盒通讯故障 */
    NETSGCC_DCA_REASON3080_INSULT_MODULE_COMM = 3080,   /* 直流充电设备异常代码：绝缘采样盒通讯故障 */
    NETSGCC_DCA_REASON3081_DC_MODULE_COMM = 3081,       /* 直流充电设备异常代码：直流采样盒通讯故障 */
    NETSGCC_DCA_REASON3082_GUID_BOARD_COMM = 3082,      /* 直流充电设备异常代码：导引板通讯故障 */
    NETSGCC_DCA_REASON3082_LED_BOARD_COMM = 3083,       /* 直流充电设备异常代码：灯板通讯故障 */
    NETSGCC_DCA_REASON3084_LIGHTING_PROTECTORS = 3084,  /* 直流充电设备异常代码：避雷器故障 */
    NETSGCC_DCA_REASON3085_SMOKING = 3085,              /* 直流充电设备异常代码：烟雾故障 */
    NETSGCC_DCA_REASON3086_TRANSACTION_RECORD_FULL = 3086,  /* 直流充电设备异常代码：交易记录已满告警  */

    NETSGCC_DCA_REASON3087_POWER_OFF = 3087,            /* 直流充电设备异常代码：断电  */
    NETSGCC_DCA_REASON3088_STORAGE_CHIP = 3088,         /* 直流充电设备异常代码：存储芯片  */
    NETSGCC_DCA_REASON3089_INSULT_VOLT = 3089,          /* 直流充电设备异常代码：绝缘电压  */
    NETSGCC_DCA_REASON3090_NOBALLANCE = 3090,           /* 直流充电设备异常代码：余额不足  */
    NETSGCC_DCA_REASON3091_ABNORMAL_CURRENT = 3091,     /* 直流充电设备异常代码：异常电流  */
    NETSGCC_DCA_REASON3092_UNKNOW = 3092,               /* 直流充电设备异常代码：未知  */
};

/** DCPA as DC power abnormal (直流充电电源异常代码)*/
enum sgcc_dcpower_abnormal{
    NETSGCC_DCPA_REASON4008_INPUT_POWER = 4008,         /* 直流充电电源异常代码：输入电源故障（过压、过流、欠压，跳闸）  */
    NETSGCC_DCPA_REASON4009_OUTPUT_OVERVOLT = 4009,     /* 直流充电电源异常代码：输出电压过压故障  */
    NETSGCC_DCPA_REASON4010_OUTPUT_OVERCURR = 4010,     /* 直流充电电源异常代码：输出电压过流故障  */
    NETSGCC_DCPA_REASON4011_OUTPUT_UNDERVOLT = 4011,    /* 直流充电电源异常代码：输出电压欠压故障  */
    NETSGCC_DCPA_REASON4012_OUTPUT_SHORTS = 4012,       /* 直流充电电源异常代码：输出短路故障  */
    NETSGCC_DCPA_REASON4013_AC_CIRCUIT_BREAKER = 4013,  /* 直流充电电源异常代码：交流断路器故障  */
    NETSGCC_DCPA_REASON4014_RELAY_OUTSIDE_OVER10V = 4014, /* 直流充电电源异常代码：接触器外侧电压大于 10v  */
    NETSGCC_DCPA_REASON4015_DETECT_POINT_VOLT = 4015,   /* 直流充电电源异常代码：检测点电压检测故障  */
    NETSGCC_DCPA_REASON4016_GC_CAPACITY_OVER_RATED_POWER = 4016, /* 直流充电电源异常代码：桩群电容量超过额定限制故障  */
    NETSGCC_DCPA_REASON4017_INPUT_MISS_PHASE = 4017,    /* 直流充电电源异常代码：输入缺相告警  */
    NETSGCC_DCPA_REASON4018_LEAKAGE_PROTECT = 4018,     /* 直流充电电源异常代码：漏电保护故障  */
    NETSGCC_DCPA_REASON4019_GROUND = 4019,              /* 直流充电电源异常代码：地线故障  */
    NETSGCC_DCPA_REASON4020_AC_LIGHTING_PROTECTORS = 4020, /* 直流充电电源异常代码：交流防雷故障  */
    NETSGCC_DCPA_REASON4021_CAR_PILE_VOLT = 4021,       /* 直流充电电源异常代码：车/桩电压异常故障  */
    NETSGCC_DCPA_REASON4022_MODULE_PROTECT = 4022,      /* 直流充电电源异常代码：模块保护故障  */
    NETSGCC_DCPA_REASON4023_THREE_PHASE_UNBALLANCE = 4023, /* 直流充电电源异常代码：三相不平衡告警  */
};

/** DCCA as DC car abnormal (直流充电车辆异常代码)*/
enum sgcc_dccar_abnormal{
    NETSGCC_DCCA_REASON5001_BMS_COMM = 5001,            /* 直流充电车辆异常代码：BMS通讯异常  */
    NETSGCC_DCCA_REASON5002_BCP_TIMEOUT = 5002,         /* 直流充电车辆异常代码：BCP充电参数配置报文超时  */
    NETSGCC_DCCA_REASON5003_BRO_TIMEOUT = 5003,         /* 直流充电车辆异常代码：BRO充电准备就绪报文超时  */
    NETSGCC_DCCA_REASON5004_BCS_TIMEOUT = 5004,         /* 直流充电车辆异常代码：BCS电池充电状态报文超时  */
    NETSGCC_DCCA_REASON5005_BCL_TIMEOUT = 5005,         /* 直流充电车辆异常代码：BCL电池充电需求报文超时  */
    NETSGCC_DCCA_REASON5006_BST_TIMEOUT = 5006,         /* 直流充电车辆异常代码：BST中止充电报文超时  */
    NETSGCC_DCCA_REASON5007_BSD_TIMEOUT = 5007,         /* 直流充电车辆异常代码：BSD充电统计数据报文超时  */
    NETSGCC_DCCA_REASON5008_BSM_TIMEOUT = 5008,         /* 直流充电车辆异常代码：BSM动力蓄电池状态报文超时  */
    NETSGCC_DCCA_REASON5009_BRO_MAJOR_FAULT = 5009,     /* 直流充电车辆异常代码：BRO重大故障停止充电  */
    NETSGCC_DCCA_REASON5010_BHM_OUTPUT_NOMATCH = 5010,  /* 直流充电车辆异常代码：BHM桩的输出能力不匹配  */
    NETSGCC_DCCA_REASON5011_BRM_TIMEOUT = 5011,         /* 直流充电车辆异常代码：BRM车辆辨识报文超时   */
    NETSGCC_DCCA_REASON5012_BEM_TIMEOUT = 5012,         /* 直流充电车辆异常代码：BEM充电错误报文超时  */
    NETSGCC_DCCA_REASON5013_BMS_REQUIRE_VOLT_ABNORMAL = 5013, /* 直流充电车辆异常代码：BMS需求电压过低/过高  */
    NETSGCC_DCCA_REASON5014_BMS_INSULATION = 5014,      /* 直流充电车辆异常代码：BMS绝缘故障   */
    NETSGCC_DCCA_REASON5015_BMS_COMPONENT_OVERTEMP = 5015, /* 直流充电车辆异常代码：BMS元件过温  */
    NETSGCC_DCCA_REASON5016_BMS_OVERVOLT = 5016,        /* 直流充电车辆异常代码：BMS电压过高  */
    NETSGCC_DCCA_REASON5017_BMS_READY_VOLT_NOMATCH = 5017, /* 直流充电车辆异常代码：BMS预充电压不匹配  */
    NETSGCC_DCCA_REASON5018_BMS_OTHER = 5018,           /* 直流充电车辆异常代码：BMS其他故障  */
    NETSGCC_DCCA_REASON5019_SBATTERY_VOLT_HIGH = 5019,  /* 直流充电车辆异常代码：单体动力蓄电池电压过高  */
    NETSGCC_DCCA_REASON5020_SBATTERY_VOLT_LOW = 5020,   /* 直流充电车辆异常代码：单体动力蓄电池电压过低  */
    NETSGCC_DCCA_REASON5021_SOC_HIGH = 5021,            /* 直流充电车辆异常代码：整车动力蓄电池荷电状态 SOC过高  */
    NETSGCC_DCCA_REASON5022_SOC_LOW = 5022,             /* 直流充电车辆异常代码：整车动力蓄电池荷电状态 SOC过低  */
    NETSGCC_DCCA_REASON5023_BATTERY_OVERCURR = 5023,    /* 直流充电车辆异常代码：动力蓄电池充电过流  */
    NETSGCC_DCCA_REASON5024_BATTERY_OVERTEMP = 5024,    /* 直流充电车辆异常代码：动力蓄电池温度过高  */
    NETSGCC_DCCA_REASON5025_BATTERY_INSULATION = 5025,  /* 直流充电车辆异常代码：动力蓄电池绝缘故障  */
    NETSGCC_DCCA_REASON5026_BATTERY_LINKER = 5026,      /* 直流充电车辆异常代码：动力蓄电池连接器故障  */
    NETSGCC_DCCA_REASON5027_BATTERY_REVERSE = 5027,     /* 直流充电车辆异常代码：电池反接  */
    NETSGCC_DCCA_REASON5028_BATTERY_UNDERVOLT = 5028,   /* 直流充电车辆异常代码：电池欠压  */
    NETSGCC_DCCA_REASON5029_BATTERY_VOLT_ABNORMAL = 5029, /* 直流充电车辆异常代码：电池电压异常  */
    NETSGCC_DCCA_REASON5030_CRO_TIMEOUT = 5030,         /* 直流充电车辆异常代码：CRO充电机输出就绪超时  */
    NETSGCC_DCCA_REASON5031_CCS_TIMEOUT = 5031,         /* 直流充电车辆异常代码：CCS充电机状态报文超时 */
    NETSGCC_DCCA_REASON5032_CST_TIMEOUT = 5032,         /* 直流充电车辆异常代码：CST充电机终止充电报文超时  */
    NETSGCC_DCCA_REASON5033_CSD_TIMEOUT = 5033,         /* 直流充电车辆异常代码：CSD充电统计数据报文超时  */
    NETSGCC_DCCA_REASON5034_CAR_CURRENT_NOMATCH = 5034, /* 直流充电车辆异常代码：车辆电流不匹配  */
    NETSGCC_DCCA_REASON5035_CAR_ELECT_CANNOT_TRANSMIT = 5035, /* 直流充电车辆异常代码：车辆电量无法传送  */
    NETSGCC_DCCA_REASON5036_OCCUPY_TIMEOUT = 5036,      /* 直流充电车辆异常代码：车辆占位超时告警  */
    NETSGCC_DCCA_REASON5037_CAR_JUDGE_STANDAR_TIMEOUT = 5037, /* 直流充电车辆异常代码：新老国标探测超时，此为车辆故障  */
    NETSGCC_DCCA_REASON5038_BMS_ABNORMAL_STOP = 5038,   /* 直流充电车辆异常代码：BMS异常停止  */
};

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
