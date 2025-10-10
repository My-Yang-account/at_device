/**
  ******************************************************************************
  * @file
  * @brief 平台状态机
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __APP_OFSM_H
#define __APP_OFSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "net_sal.h"
#include "mw_fault_check.h"
#include "app_billing_rule.h"

#include "chargepile_config.h"

#define APP_DESIGNATE_REGION           /* 变量定义到指定区 */

#ifdef APP_DESIGNATE_REGION
#define APP_DEF_TCMRAM CFG_DEF_TCMRAM
#define APP_DEF_SRAM0 CFG_DEF_SRAM0
#define APP_DEF_SRAM1 CFG_DEF_SRAM1
#define APP_DEF_SRAM2 CFG_DEF_SRAM2
#else
#define APP_DEF_TCMRAM
#define APP_DEF_SRAM0
#define APP_DEF_SRAM1
#define APP_DEF_SRAM2
#endif /* APP_DESIGNATE_REGION */

#ifdef CP_USING_OFFLINE_BILLING
#define APP_USING_OFFLINE_BILLING                      /* 包含离线计费 */
#endif /* CP_USING_OFFLINE_BILLING */

#ifdef CP_USING_NO_BMS
#define APP_USING_NO_BMS                               /* 使用无BMS版本(强制启动模块充电(需要在离线计费模式下)) */
#endif /* CP_USING_NO_BMS */

#ifdef CP_USING_LV_MODULE
#define APP_USING_LV_MODULE                            /* 使用低压模块版本 */
#endif /* CP_USING_LV_MODULE */

#ifdef CP_USING_METER_ELECT_DETECT_STRATEGY
#define APP_USING_METER_ELECT_DETECT_STRATEGY          /* 使用电表电量检测策略 */
#endif /* CP_USING_METER_ELECT_DETECT_STRATEGY */

#ifdef CP_USING_BAT_VOLT_DETECT_STRATEGY
#define APP_USING_BAT_VOLT_DETECT_STRATEGY             /* 使用电池电压检测策略 */
#endif /* CP_USING_BAT_VOLT_DETECT_STRATEGY */

#ifdef CP_USING_CHARGE_CURR_DETECT_STRATEGY
#define APP_USING_CHARGE_CURR_DETECT_STRATEGY          /* 使用充电电流检测策略 */
#endif /* CP_USING_CHARGE_CURR_DETECT_STRATEGY */

#ifdef CP_USING_FB_DETECT
#define APP_USING_FB_DETECT                            /* 使用反馈实时检测 */
#endif /* CP_USING_FB_DETECT */

#ifdef CP_INCLUDE_BATVOLT_DETECT_QRCODE
#define APP_INCLUDE_BATVOLT_DETECT_QRCODE              /* 包含电池电压报告检测二维码 */
#endif /* CP_INCLUDE_BATVOLT_DETECT_QRCODE */

#define APP_USING_DOUBLEGUN                            /* 使用双枪 */

#define APP_MAINTENTANCE_MODE_CURR_MAX       200       /* 保养模式最大电流20A(0.1) */

#define APP_CURRENT_OFFSET_DEFAULT           4000      /* 电流偏移默认值(0.1) */
#define APP_MCURRENT_SINGLEGUN_DEFAULT       25000     /* 单枪最大电流默认值(0.01) */
#define APP_MCURRENT_SINGLEGUN_PARACHARGE    45000     /* 并充时单枪最大电流值(0.01) */

#define APP_PARACHARGE_IDENTIFY_CAN_ID       0x1FFFFFFF /* 并充自动识别CAN ID */
#define APP_PARACHARGE_IDENTIFY_CAN_DATA     0x5A       /* 并充自动识别CAN 数据 */

#define APP_CHARGE_ELECT_MAX                 1000000   /* 最大充电电量值(精度：0.001) */
#define APP_SPEND_AMOUNT_MAX                 30000000  /* 最大消费金额值(精度：0.0001) */
#define APP_CALCULATE_ELECT_DIFF_MAX         510       /* 最大计算电量差值(精度：0.001) */
#define APP_EMS_STOP_POWER_MIN               5         /* ems停充功率最小值(精度：0.1KW) */

#define APP_CARD_NUMBER_COMPARE_LEN_MIN                   12    /* 卡号最小对比长度 */
#define APP_CARD_NUMBER_COMPARE_LEN_MIN_OFFLINE_BILLING   6     /* 卡号最小对比长度(离线计费模式下) */

#define START_CHARGE_TIMEOUT  (100 *1)
#define TINY_CURRENT_ABNOAMAL_VALUE          200       /* 小电流异常值 */
#define TINY_CURRENT_ABNOAMAL_TIMEOUT        1800000   /* 小电流异常持续时间 30 *60 *1000  */

#define APP_CURR_CCSBCL_ABNORMAL_TIMEOUT     15000     /* CCS 和 BCL 电流异常停充超时时间 */
#define APP_CURR_CCSBCS_ABNORMAL_TIMEOUT     30000     /* CCS 和 BCS 电流异常停充超时时间 */
#define APP_CURR_COMPARE_CCSBCL_MAX          500       /* CCS 和 BCL 电流比较最大差值 */
#define APP_CURR_COMPARE_CCSBCS_MAX          1500      /* CCS 和 BCS 电流比较最大差值 */

#define APP_ORDER_FIXES_WAIT_TIME            (30000)   /* 订单矫正等待最长时间(ms) */

#define APP_STORAGE_TRANSATION_INTERVAL      (30 *60 *1000)   /* 充电中保存交易记录间隔 */
#define APP_CARD_NUMBER_COMPARE_LEN          16        /* 卡号对比长度 */
#define OVERTEMP_DECREASE_CURR_PERCENT       5 /10     /* 过温降流百分比 */

#ifdef APP_INCLUDE_YKC17_PROTOCOL
#define APP_AMMETER_ENCRY_WAIT_START_REPLY_MS       1500         /* 等待开始充电指令响应时长 */
#define APP_AMMETER_ENCRY_WAIT_STOP_REPLY_MS        1500         /* 等待结束充电指令响应时长 */
#define APP_AMMETER_ENCRY_WAIT_READING_REPLY_MS     2000         /* 等待抄表指令响应时长 */

#define APP_AMMETER_ENCRY_STEP_NULL          0x00                /* 获取电表加密数据步骤：开始 */
#define APP_AMMETER_ENCRY_STEP_START         0x01                /* 获取电表加密数据步骤：发开始充电指令 */
#define APP_AMMETER_ENCRY_STEP_STOP          0x02                /* 获取电表加密数据步骤：发结束充电指令 */
#define APP_AMMETER_ENCRY_STEP_METER_READING 0x03                /* 获取电表加密数据步骤：发抄表指令 */
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

#if (defined(APP_USING_NO_BMS) && defined(APP_USING_OFFLINE_BILLING))
#define APP_NO_BMS_PRECHARGE_SAMPLING_STEADY_TIME                5000            /* 预充时等待采样稳定时间(ms) */
#define APP_NO_BMS_BOOT_TIMEOUT                                  50000           /* 启动超时时间(ms) */
#define APP_NO_BMS_RISE_CURRENT_PERIOD                           1000            /* 电流爬升时间间隔(ms) */
#define APP_NO_BMS_STAGE_1_CHARGE_TIME_MAX_64V125AH              3600            /* 64V125AH型电池第一阶段最大充电时长(s) */
#define APP_NO_BMS_STAGE_1_CHARGE_TIME_MAX_51_2V125AH            3600            /* 51_2V125AH型电池第一阶段最大充电时长(s) */
#define APP_NO_BMS_FULL_CONTINUOUS_TIME_MAX                      20000           /* 满充最大持续时长(ms) */
#define APP_NO_BMS_STAGE_CROSSING_CONTINUOUS_TIME_MAX            5000            /* 阶段跨越最大持续时长(ms) */
#define APP_NO_BMS_CLOSE_MODULE_WAIT_TIME_MAX                    2000            /* 关闭模块最大等待时长(ms) */

#define APP_NO_BMS_PRECHARGE_SAMPLING_VOLTAGE_MIN                100             /* 预充时采样电压最小值(0.1V) */
#define APP_NO_BMS_PRECHARGE_VOLTAGE_THRESHOLD                   50              /* 预充时电池电压和模块电压比较阈值(0.1V) */
#define APP_NO_BMS_STAGE_1_REQUEST_VOLTAGE_64V125AH              724             /* 64V125AH型电池第一阶段请求电压(0.1V) */
#define APP_NO_BMS_STAGE_2_REQUEST_VOLTAGE_64V125AH              720             /* 64V125AH型电池第二阶段请求电压(0.1V) */
#define APP_NO_BMS_STAGE_1_REQUEST_VOLTAGE_51_2V125AH            563             /* 51_2V125AH型电池第一阶段请求电压(0.1V) */
#define APP_NO_BMS_STAGE_2_REQUEST_VOLTAGE_51_2V125AH            576             /* 51_2V125AH型电池第二阶段请求电压(0.1V) */
#define APP_NO_BMS_STAGE_1_TO_STAGE_2_VOLTAGE_64V125AH           704             /* 64V125AH型电池从阶段1到阶段2的电压阈值(0.1V) */
#define APP_NO_BMS_STAGE_1_TO_STAGE_2_VOLTAGE_51_2V125AH         563             /* 51_2V125AH型电池从阶段1到阶段2的电压阈值(0.1V) */
#define APP_NO_BMS_MODULE_CLOSE_VOLTAGE_MAX                      600             /* 模块认为是已关闭时的电压阈值最大值(0.1V) */

#define APP_NO_BMS_PRECHARGE_CURRENT                             200             /* 预充时设定电流(0.01A) */
#define APP_NO_BMS_RISE_CURRENT_STEP                             1000            /* 电流爬升步进(0.01A) */
#define APP_NO_BMS_STAGE_1_REQUEST_CURRENT_64V125AH              8000            /* 64V125AH型电池第一阶段请求电流(0.01A) */
#define APP_NO_BMS_STAGE_2_REQUEST_CURRENT_64V125AH              6000            /* 64V125AH型电池第二阶段请求电流(0.01A) */
#define APP_NO_BMS_STAGE_1_REQUEST_CURRENT_51_2V125AH            8000            /* 51_2V125AH型电池第一阶段请求电流(0.01A) */
#define APP_NO_BMS_STAGE_2_REQUEST_CURRENT_51_2V125AH            6000            /* 51_2V125AH型电池第二阶段请求电流(0.01A) */
#define APP_NO_BMS_FULL_CURRENT_64V125AH                         20              /* 64V125AH型电池满充电流值(0.1A) */
#define APP_NO_BMS_FULL_CURRENT_51_2V125AH                       20              /* 51_2V125AH型电池满充电流值(0.1A) */
#endif /* (defined(APP_USING_NO_BMS) && defined(APP_USING_OFFLINE_BILLING)) */

#define APP_RESERVATE_STRATEGY_PULLGUN_CANCEL  (0x01 <<0x00)     /* 预约策略：拔枪取消 */
#define APP_RESERVATE_STRATEGY_FAULT_CANCEL    (0x01 <<0x01)     /* 预约策略：故障取消取消 */
#define APP_RESERVATE_STRATEGY_TIMEOUT_CANCEL  (0x01 <<0x02)     /* 预约策略：预约超时取消(最大超时时间由预约策略参数规定[单位：s]) */
#define APP_RESERVATE_STRATEGY_VIN_AUTH        (0x01 <<0x03)     /* 预约策略：预约时间到后需要VIN码鉴权启动 */
#define APP_RESERVATE_STRATEGY_START_DIRECTLY  (0x01 <<0x04)     /* 预约策略：预约时间到后直接启动 */

#define APP_SYSTEM_RUN_TIME_PERIOD                      100      /* 系统业务运行周期(ms) */

#ifdef APP_USING_METER_ELECT_DETECT_STRATEGY
#define APP_MELECT_DETECT_CURR_RANGE_0                  0        /* 电流范围0：0A <= I >= 5A */
#define APP_MELECT_DETECT_CURR_RANGE_1                  1        /* 电流范围1：5A < I >= 20A */
#define APP_MELECT_DETECT_CURR_RANGE_2                  2        /* 电流范围2：0A < 20A */
#define APP_MDETECT_CURR_RANGE_0_MAX                    50       /* 电流范围0电流最大值 */
#define APP_MDETECT_CURR_RANGE_1_MAX                    200      /* 电流范围1电流最大值 */
#define APP_MELECT_DETECT_RANGE_THRESHOLD               20       /* 电流范围切换阈值(0.1A， 电流上升时要超过阈值，下降时直接切换) */
#define APP_METER_ELECT_ERR_COUNT_MAX                   2        /* 电表电量检测最大错误次数 */
#define APP_SIMULATE_ELECT_CALCULATE_PERIOD             ((60 *1000) /APP_SYSTEM_RUN_TIME_PERIOD)  /* 电量模拟计算周期(ms) */
#define APP_SIMULATE_ELECT_COMPARE_VALUE_1              10       /* 电量模拟值对比值(单位：0.001度， 对应范围1) */
#define APP_SIMULATE_ELECT_COMPARE_VALUE_2              50       /* 电量模拟值对比值(单位：0.001度， 对应范围2) */
#endif /* APP_USING_METER_ELECT_DETECT_STRATEGY */

#ifdef APP_USING_BAT_VOLT_DETECT_STRATEGY
#define APP_BATTERY_VOLTAGE_DETECT_PERIOD               (15000 /APP_SYSTEM_RUN_TIME_PERIOD)    /* 电池电压检测周期(ms) */
#define APP_BATTERY_VOLTAGE_FLOAT_VALUE                 500      /* 电池电压检测浮动值(单位：0.1V) */
#define APP_BATTERY_VOLTAGE_ERR_COUNT_MAX               5        /* 电池电压检测错误次数 */
#endif /* APP_USING_BAT_VOLT_DETECT_STRATEGY */

#ifdef APP_USING_CHARGE_CURR_DETECT_STRATEGY
#define APP_DETECT_TOTAL_PERIOD                         ((5 *60 *1000) /APP_SYSTEM_RUN_TIME_PERIOD)   /* 检测总周期(ms) */
#define APP_CURRENT_DETECT_PERIOD                       (5000 /APP_SYSTEM_RUN_TIME_PERIOD)     /* 电流检测周期(ms) */
#define APP_CURRENT_STEADY_DIFF                         30       /* 电流稳定比较差值(单位：0.1V) */
#define APP_CURRENT_COMPARE_DIFF                        100      /* 电流异常比较差值(单位：0.1A) */
#define APP_CURRENT_STEADY_COUNT                        12       /* 电流稳定次数 */
#endif /* APP_USING_CHARGE_CURR_DETECT_STRATEGY */

#ifdef APP_USING_FB_DETECT
#define APP_ELOCK_RELAY_CHECK_TIME                      (10000 /APP_SYSTEM_RUN_TIME_PERIOD)    /* 电子锁、继电器检测故障时间(ms) */
#endif /* APP_USING_FB_DETECT */

enum buzzon_state {
    APP_BUZZON_STATE_NULL = 0,
    APP_BUZZON_STATE_OK,
    APP_BUZZON_STATE_FAILED,  /** 平台鉴权卡片失败 */
    APP_BUZZON_STATE_WARRING, /** 密钥验证或获取卡号失败，本地鉴权失败 */
    APP_BUZZON_STATE_AUTHING, /** 平台正在鉴权卡片 */
    APP_BUZZON_STATE_SUCCESS, /** 平台鉴权卡片成功 */
};

enum booting_step_t{
    APP_BOOTING_STEP_IDLE,
    APP_BOOTING_STEP_HAND,
    APP_BOOTING_STEP_INSULT,
    APP_BOOTING_STEP_CONFIG,
};

enum system_gunno_enum{
    APP_SYSTEM_GUNNOA,                                 /* A枪枪号 */
#ifdef APP_USING_DOUBLEGUN
    APP_SYSTEM_GUNNOB,                                 /* B枪枪号 */
#endif /* APP_USING_DOUBLEGUN */
    APP_SYSTEM_GUNNO_SIZE,
};

enum run_mode_enum{
    APP_RUN_MODE_4G_ETH,                               /* 桩运行模式：4G、以太网联网 */
    APP_RUN_MODE_OFFLINE_BILLING,                      /* 桩运行模式：4离线计费 */
    APP_RUN_MODE_OFFLINE,                              /* 桩运行模式：离线模式 */
    APP_RUN_MODE_PLUG_AND_PLAY,                        /* 桩运行模式：即插即充模式 */
    APP_RUN_MODE_SIZE,                                 /* 桩运行模式： */
};

enum charge_strategy_enum{
    APP_CHARGE_STRATEGY_TIME,                          /* 充电策略：按时间(单位是s) */
    APP_CHARGE_STRATEGY_ELECT,                         /* 充电策略：按电量(单位0.001度) */
    APP_CHARGE_STRATEGY_MONEY,                         /* 充电策略：按金额(单位0.0001元) */
    APP_CHARGE_STRATEGY_RESERVATION,                   /* 充电策略：预约 */
    APP_CHARGE_STRATEGY_FULL,                          /* 充电策略：充满 */
    APP_CHARGE_STRATEGY_SOC,                           /* 充电策略：按SOC */
};

/** 调功率策略 */
enum power_strategy{
    APP_POWER_STRATEGY_ORDER = 0x01,                   /* 调功率策略：有序充电 */
    APP_POWER_STRATEGY_SET_LIMIT = 0x02,               /* 调功率策略：设置功率限值 */
    APP_POWER_STRATEGY_SIZE = 0x03,                    /* 调功率策略 */
};

enum system_start_way{
    APP_CHARGE_START_WAY_APP,                          /* 启动方式：APP */
    APP_CHARGE_START_WAY_ONLINE_CARD,                  /* 启动方式：在线卡 */
    APP_CHARGE_START_WAY_OFFLINE_CARD,                 /* 启动方式：离线卡 */
    APP_CHARGE_START_WAY_VIN,                          /* 启动方式：VIN码 */
    APP_CHARGE_START_WAY_SCREEN,                       /* 启动方式：屏幕 */
    APP_CHARGE_START_WAY_BLUE,                         /* 启动方式：蓝牙 */
    APP_CHARGE_START_WAY_PLUG_AND_CHARGE,              /* 启动方式：即插即充 */
    APP_CHARGE_START_WAY_RESERVATION,                  /* 启动方式：定时预约 */
    APP_CHARGE_START_WAY_PASSWORD,                     /* 启动方式：密码 */
};

enum net_state{
    APP_NET_STATE_NULL = 0,         //物理层故障（网卡故障）
    APP_NET_STATE_CARD = 1,         //检测SIM卡异常
    APP_NET_STATE_UP = 2,           //链路层故障（GPRS网络注册异常）
    APP_NET_STATE_INTERNET_UP = 3,  //连接平台中
    APP_NET_STATE_AUTH_SECCESS = 4, //连接平台成功

    APP_NET_STATE_ETH_NULL = 6,     //以太网：模块检测
    APP_NET_STATE_ETH_LINE = 7,     //以太网：网线检测
    APP_NET_STATE_ETH_QUERY_NET = 8,//以太网：网络检测

    APP_NET_STATE_SIZE,
};

enum ota_state{
    APP_OTA_STATE_NULL,
    APP_OTA_STATE_UP,
    APP_OTA_STATE_LINK_UP,
    APP_OTA_STATE_INTERNET_UP,
    APP_OTA_STATE_AUTHING,
    APP_OTA_STATE_AUTH_SUCCESS,
    APP_OTA_STATE_UPDATEING,
    APP_OTA_STATE_UPDATE_SECCESS,
    APP_OTA_STATE_UPDATE_FAILED,
    APP_OTA_STATE_SIZE,
};

enum ofsm_state {
    APP_OFSM_STATE_WAIT_NET,    /** 状态机状态：等待网络 */
    APP_OFSM_STATE_IDLEING,     /** 状态机状态： 空闲中 */
    APP_OFSM_STATE_READYING,    /** 状态机状态： 准备中 */
    APP_OFSM_STATE_RESERVATION, /** 状态机状态： 预约中 */
    APP_OFSM_STATE_STARTING,    /** 状态机状态： 开始中 */
    APP_OFSM_STATE_CHARGING,    /** 状态机状态： 充电中 */
    APP_OFSM_STATE_STOPING,     /** 状态机状态： 停止中 */
    APP_OFSM_STATE_FINISHING,   /** 状态机状态： 充电完成未拔枪 */
    APP_OFSM_STATE_FAULTING,    /** 状态机状态： 故障 */

    APP_OFSM_STATE_SIZE,
};

/** charge control state */
enum char_ctrl_state{
    APP_CHARGE_CTRL_STATE_IDLE,            /** 充电控制状态： 空闲 */
    APP_CHARGE_CTRL_STATE_SHAKE_HAND,      /** 充电控制状态：握手 */
    APP_CHARGE_CTRL_STATE_INSULATION,      /** 充电控制状态：绝缘 */
    APP_CHARGE_CTRL_STATE_CONFIGURE,       /** 充电控制状态：配置 */
    APP_CHARGE_CTRL_STATE_CHARGING,        /** 充电控制状态：充电 */
    APP_CHARGE_CTRL_STATE_FINISH,          /** 充电控制状态：结束 */
    APP_CHARGE_CTRL_STATE_FAULTING,        /** 充电控制状态：故障 */
    APP_CHARGE_CTRL_STATE_WAIT_PULL_GUN,   /** 充电控制状态：等待拔枪 */
    APP_CHARGE_CTRL_STATE_SIZE,            /** 充电控制状态：空 */
};

enum device_state {
    APP_DEVICE_STATE_COMMISSIONING,        /** 设备状态： 投运 */
    APP_DEVICE_STATE_OVERHAUL,             /** 设备状态：检修 */
    APP_DEVICE_STATE_FREEZE,               /** 设备状态： 冻结 */
    APP_DEVICE_STATE_OUTAGE,               /** 设备状态： 停运 */
    APP_DEVICE_STATE_RETURNS,              /** 设备状态： 退运 */

    APP_DEVICE_STATE_SIZE,
};

enum{
    APP_THA_ENUM_FALSE,
    APP_THA_ENUM_TRUE,
};

enum{
    APP_CONNECT_STATE_DISCONNECT,       /* 枪连接状态：未连接 */
    APP_CONNECT_STATE_HALFWAY,          /* 枪连接状态：半连接 */
    APP_CONNECT_STATE_CONNECT,          /* 枪连接状态：已连接 */
};

enum charge_way{
    APP_CHARGE_WAY_NONE,                /* 充电方式：无 */
    APP_CHARGE_WAY_SINGLEGUN,           /* 充电方式：单枪 */
    APP_CHARGE_WAY_PARACHARGE_LOCAL,    /* 充电方式：并充(本地选择:最终只上报一把枪的交易) */
    APP_CHARGE_WAY_PARACHARGE_CLOUD,    /* 充电方式：并充(云端选择:最终需上报两把枪的交易) */
};

#ifdef APP_USING_FB_DETECT
enum{
    APP_CHARGE_CTRL_IDLE,
    APP_CHARGE_CTRL_AUA_POWER,
    APP_CHARGE_CTRL_CHM,
    APP_CHARGE_CTRL_INSULT,
    APP_CHARGE_CTRL_FINISH,
    APP_CHARGE_CTRL_CRM,
    APP_CHARGE_CTRL_CTSCML,
    APP_CHARGE_CTRL_CRO,
    APP_CHARGE_CTRL_CROAA,
    APP_CHARGE_CTRL_CCS,
    APP_CHARGE_CTRL_CST,
    APP_CHARGE_CTRL_CSD,
    APP_CHARGE_CTRL_STOP,
    APP_CHARGE_CTRL_WAIT_GUN,
    APP_CHARGE_CTRL_FAULT,
    APP_CHARGE_CTRL_CHARGING_FAULT,
    APP_CHARGE_CTRL_COMMON_FAULT,
    APP_CHARGE_CTRL_COMMON_END_FAULT,
    APP_CHARGE_CTRL_SIZE,
};
#endif /* APP_USING_FB_DETECT */

#if (defined(APP_USING_NO_BMS) && defined(APP_USING_OFFLINE_BILLING))
/** 电池类型 */
enum bat_type{
    APP_BATTERY_TYPE_64V125AH,          /* 电池类型：64V125Ah */
    APP_BATTERY_TYPE_51_2V125AH,        /* 电池类型：51.2V125Ah */
    APP_BATTERY_TYPE_SIZE,              /* 电池类型:  */
};

/** 电池充电阶段 */
enum bat_charge_stage{
    APP_BATTERY_CHARGE_STAGE_1,         /* 充电阶段：1 */
    APP_BATTERY_CHARGE_STAGE_2,         /* 充电阶段：2 */
    APP_BATTERY_CHARGE_STAGE_SIZE,      /* 充电阶段： */
};
#endif /* defined(APP_USING_NO_BMS) && defined(APP_USING_OFFLINE_BILLING) */

#pragma pack(1)

typedef struct{
    uint8_t state;                      /* OTA 状态 */
    uint8_t start_flag;                 /* OTA 开始标志 */
    uint16_t result;                    /* OTA 结果 */
    uint32_t progress;                  /* OTA 进度 */
}ota_info;

/****************************************** 信息变化提醒 *************************************************/
/****************************************** 信息变化提醒 *************************************************/
struct _info_charged{
    uint8_t msetup : 1;                   /* 模块信息配置有变化 */
    uint8_t isetup : 1;                   /* 输入信息配置有变化 */
    uint8_t psetup : 1;                   /* 保护信息配置有变化 */
    uint8_t fsetup : 1;                   /* 功能配置有变化 */
};

/****************************************** 充电账单信息 *************************************************/
/****************************************** 充电账单信息 *************************************************/
struct _order_info{
    uint8_t verify_fail : 1;              /* 上报确认失败 */
    uint8_t is_start_fail : 1;            /* 启动失败 */
    uint8_t is_charging : 1;              /* 正在充电 */
    uint8_t online_order : 1;             /* 在线订单 */
    uint8_t bms_recommunicate : 1;        /* BMS通信重连 */
    uint8_t waiting_charge : 1;           /* 订单已创建，正在等待充电(离线计费模式下的预约启动) */
    uint8_t reserve : 2;
};

typedef struct
{
#ifndef APP_INCLUDE_SGCC_PROTOCOL
    uint8_t chargepile_id[20];            /* 充电桩ID */
#else
    uint8_t reserve2[20];
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    uint8_t gunno;                        /* 枪口号 */

    uint8_t logic_card_number[20];        /* 逻辑卡号 ascii */
    uint8_t physics_card_number[8];       /* 物理卡号 ascii */
    uint8_t serial_number[40];            /* 流水号 */
    uint8_t user_number[32];             /* 用户号 */
#ifdef APP_INCLUDE_SGCC_PROTOCOL
    uint8_t chargepile_id[64];            /* 充电桩ID */
    uint32_t account_ballance_before;     /* 充电前卡(账户)余额 */
    uint32_t account_ballance_after;      /* 充电后卡(账户)余额 */
    uint8_t reserve0[24];
#else
    uint32_t account_ballance_before;     /* 充电前卡(账户)余额 */
    uint32_t account_ballance_after;      /* 充电后卡(账户)余额 */
    uint8_t reserve0[88];
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

    uint32_t start_time;                  /* 充电开始时间 */
    uint32_t end_time;                    /* 充电结束时间 */

    uint32_t charge_time;                 /* 充电时间, 单位s */

    uint8_t start_type;                   /* 启动方式 */
    uint8_t boot_result;                  /* 启动结果 1 成功 0是 失败 */

    uint8_t car_vin[17];                  /* VIN码 ascii */
    uint16_t start_soc;                   /* 起始SOC */
    uint16_t stop_soc;                    /* 结束SOC */

    uint32_t ammeter_start;               /* 电表电量起始值 */
    uint32_t ammeter_stop;                /* 电表电量结束值 */

    uint32_t stop_reason;                 /* 停止原因 */
    struct _order_info order_info;        /* 订单信息 */

    uint32_t total_elect;                 /* 充电总电量 */
    uint32_t total_loss_elect;            /* 总计损电量 */
    uint32_t charge_fee;                  /* 充电费用 */
    uint32_t service_fee;                 /* 服务费用 */
    uint32_t total_fee;                   /* 总费用 */

#if (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR) || \
    defined (APP_INCLUDE_YCP_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
    uint32_t rate_type_unit[APP_BILLING_RULE_RATE_TYPE_MAX];        /* 费率类型单价 */
    uint32_t rate_type_elect[APP_BILLING_RULE_RATE_TYPE_MAX];       /* 费率类型电量 */
    uint32_t rate_type_amount[APP_BILLING_RULE_RATE_TYPE_MAX];      /* 费率类型金额  */
    uint32_t rate_type_loss_elect[APP_BILLING_RULE_RATE_TYPE_MAX];  /* 费率类型计损电量 */
#endif /* (defined (APP_INCLUDE_YKC_PROTOCOL) || defined (APP_INCLUDE_YKC_PROTOCOL_MONITOR)) */

#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL))
    uint32_t period_elect[APP_BILLING_RULE_PERIOD_MAX];            /* 时段电量 */
    uint32_t period_elect_fees[APP_BILLING_RULE_PERIOD_MAX];       /* 时段电费 */
    uint32_t period_service_fees[APP_BILLING_RULE_PERIOD_MAX];     /* 时段服务费 */
    uint32_t period_occupy_fees[APP_BILLING_RULE_PERIOD_MAX];      /* 时段占位费 */
    uint8_t elect_model_sn[APP_BILLING_MODEL_SN_LEN + 1];          /* 电费计费模型编号 */
    uint8_t service_model_sn[APP_BILLING_MODEL_SN_LEN + 1];        /* 服务费计费模型编号 */
    uint8_t reserve3[112];
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_SGCC_PROTOCOL)) */

#ifdef APP_INCLUDE_SGCC_PROTOCOL
    uint8_t device_serial_number[40 + 1];                         /* 设备流水号 */
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

#ifdef APP_INCLUDE_XJ_PROTOCOL
    uint32_t delay_fee;                   /* 延迟费用 */
#endif /* APP_INCLUDE_XJ_PROTOCOL */

    uint8_t start_period_number;          /* 起始时段 */
    uint8_t period_count;                 /* 跨时段个数 */
    uint8_t charge_strategy;              /* 充电策略参数 */
    uint32_t charge_strategy_para;        /* 充电策略参数 */
    struct{
        uint8_t target_soc : 2;           /* 达到所需求的SOC目标值 */
        uint8_t target_tvolt : 2;         /* 达到总电压的设定值 */
        uint8_t target_svolt : 2;         /* 达到单体电压的设定值 */
        uint8_t chargerend : 2;           /* 充电机主动中止 */
    }bms_stop_reason;
    struct{
        uint16_t insultion : 2;           /* 绝缘故障 */
        uint16_t olink_ot : 2;            /* 输出连接器过温故障 */
        uint16_t bms_comp_olink_ot : 2;   /* BMS元件、输出连接器过温 */
        uint16_t clink_fault : 2;         /* 充电连接器故障 */
        uint16_t battery_ot : 2;          /* 电池组温度过高故障 */
        uint16_t hv_relay : 2;            /* 高压继电器故障 */
        uint16_t detect_point2_volt : 2;  /* 检测点2电压检测故障 */
        uint16_t other : 2;               /* 其他故障 */
    }bms_fault_reason;

    uint8_t charge_way;                   /* 充电方式 */
#if (defined (APP_INCLUDE_SGCC_PROTOCOL))
    uint32_t rate_type_elect_amount[APP_BILLING_RULE_RATE_TYPE_MAX];      /* 费率类型电费金额  */
    uint32_t rate_type_service_amount[APP_BILLING_RULE_RATE_TYPE_MAX];    /* 费率类型服务费金额  */
#endif /* (defined (APP_INCLUDE_SGCC_PROTOCOL) */

#ifdef APP_INCLUDE_YKC17_PROTOCOL
    /* 电表加密 */
    uint16_t meter_ver;                   /* 通讯协议版本号(HEX) */
    uint8_t meter_encry_type;             /* 加密方式 */
    uint8_t meter_trade_number[16];       /* 流水号(BCD) */
    uint8_t meter_dev_sn[6];              /* 表号(BCD) */
    uint8_t meter_port_identify_sn[17];   /* 枪口识别号(BCD)--加密或参与签名计算开始 */
    uint32_t meter_stimestamp;            /* 计量开始时间(秒时戳,HEX) */
    uint32_t meter_etimestamp;            /* 计量结束时间(秒时戳,HEX) */
    uint32_t meter_positive_elect;        /* 正向充电电量(3 位小数,HEX) */
    uint32_t meter_install_timestamp;     /* 电表安装时间(秒时戳,HEX) */
    uint8_t meter_history_state;          /* 端钮历史状态(0 正常，1 发生过端钮盖打开时间)--加密或参与签名计算域结束 */
#endif /* APP_INCLUDE_YKC17_PROTOCOL */

    uint8_t run_mode;                     /* 运行模式：0：4G、以太网联网，1：离线计费，2：蓝牙联网，3：wifi联网，4：非联网、非离线计费 */
    struct{
        uint8_t volt_abnormal : 2;        /* 电压异常 */
        uint8_t over_current : 2;         /* 电流过大 */
        uint8_t reserve : 4;              /* 预留 */
    }bms_error_reason;

    uint8_t reserve[14];                  /* 预留 */
}thaisen_transaction_t;
/*******************************************************************************************/

/****************************************** 系统状态、数据信息 *************************************************/
/****************************************** 系统状态、数据信息 *************************************************/
typedef struct{
    struct{
        uint32_t is_local_charging : 1;                      /* 是否本地启动标志(对于需要平台鉴权的启动方式：在未接收到平台允许前都认为是本地启动) */
        uint32_t is_charge_complete : 1;                     /* 是否充电完成标志 */
        uint32_t vin_authorization_success : 1;              /* 是否VIN鉴权成功标志 */
        uint32_t vin_is_authorized : 1;                      /* 已进行VIN鉴权上报 */
        uint32_t card_authorization : 1;                     /* 是否进行刷卡鉴权标志 */
        uint32_t is_fault_stop : 1;                          /* 是否故障停充标志 */
        uint32_t connect_state : 2;                          /* 枪连接状态 */
        uint32_t is_pay_by_card : 1;                         /* 卡结算标志 */
        uint32_t start_result : 1;                           /* 启动结果 */
        uint32_t card_info_is_uid : 1;                       /* 卡信息是UID */
        uint32_t is_overtemp : 1;                            /* 过温 */
        uint32_t is_curr_decreased : 1;                      /* 过温已降流 */
        uint32_t permit_judge_complete : 1;                  /* 判断是否允许充电的过程已完成 */
        uint32_t is_starting : 1;                            /* 已发指令启动充电 */
        uint32_t is_adjust_power : 1;                        /* 已进行功率调整 */
        uint32_t is_resume_power : 1;                        /* 需要恢复功率 */
        uint32_t is_reservation : 1;                         /* 预约中 */
        uint32_t bms_require_decrease : 1;                   /* BMS 需求减小 */
        uint32_t is_deputygun_stop : 1;                      /* 这是副枪停止(副枪故障时停止，用于并充时) */
        uint32_t is_pay_complete : 1;                        /* 已结算完成(用于离线计费) */
        uint32_t is_ammeter_elect_error : 1;                 /* 电表电量错误(防止一开始时读取到的电表电量是0) */
        uint32_t paracharge_is_identified : 1;               /* 是否并充已识别 */
        uint32_t recved_paracharge_identify_id : 1;          /* 是否已接收到并充识别CANID */
        uint32_t is_local_reservation : 1;                   /* 是否本地预约 */
        uint32_t is_reser_normal_started : 1;                /* 是否本地预约已正常启动(用于预约时间一分钟内多次启动限制) */
        uint32_t is_reser_timeout_started : 1;               /* 是否本地预约已超时启动(用于超过预约时间10分钟内启动检测) */
        uint32_t is_meter_elect_error : 1;                   /* 是否检测出电表电量有错 */
        uint32_t is_ob_authenticated : 1;                    /* 离线计费模式下已进行预约鉴权(ob:offline billing) */
    }flag;

    uint8_t cc1_state;                /* CC1 状态 */
    uint8_t net_state;                /* 网络 状态 */
    uint8_t ota_state;                /* OTA 状态 */

    uint8_t start_type;               /* 启动类型 */
    uint8_t system_fault;             /* 系统故障代码  */
    uint8_t charge_fault;             /* 充电故障代码  */
    struct{
        uint8_t last;                 /* 前一次状态机状态码 */
        uint8_t current;              /* 当前状态机状态码 */
    }state;

    struct{
        uint8_t last;                 /* 前一次充电控制状态码 */
        uint8_t current;              /* 当前充电控制状态码 */
    }charctrl_state;

    uint8_t soft_ver_main;            /* 主版本号 */
    uint8_t soft_ver_sub;             /* 次版本号 */
    uint8_t soft_ver_revise;          /* 修订版本号 */

    uint8_t transaction_number[40 + 1]; /* 流水号 */
    uint8_t car_vin[17];              /* VIN 码 */
    uint8_t card_number[16];          /* 卡号 */
    uint8_t card_uid[8];              /* 卡UID */
    uint8_t card_uid_len;             /* 卡UID长度 */
    uint8_t user_number[32];         /* 用户号 */

    uint8_t start_soc;                /* 起始SOC */
    uint8_t current_soc;              /* 当前SOC */
    uint32_t system_power_max;        /* 系统最大功率 */
    uint16_t gun_set_curr;            /* 枪设置电流 */

    int32_t system_temperature;       /* 系统温度(精度：0.1) */
    int32_t gunline_temperature[2];   /* 枪线正负极温度(精度：0.1) */
    uint32_t voltage_a;               /* 电压A相(精度：0.01) */
    uint32_t current_a;               /* 电流A相(精度：0.01) */
    uint32_t power_a;                 /* 功率A相(精度：1) */
    uint32_t elect_a;                 /* 电量A相(精度：0.001) */
    uint32_t voltage_b;               /* 电压B相 */
    uint32_t current_b;               /* 电流B相 */
    uint32_t power_b;                 /* 功率B相 */
    uint32_t elect_b;                 /* 电量B相 */
    uint32_t voltage_c;               /* 电压C相 */
    uint32_t current_c;               /* 电流C相 */
    uint32_t power_c;                 /* 功率C相 */
    uint32_t elect_c;                 /* 电量C相 */
    uint32_t fees_total;              /* 总费用(精度：0.0001) */
    uint32_t elect_fees_total;        /* 电费总费用(精度：0.0001) */
    uint32_t service_fees_total;      /* 服务费总费用(精度：0.0001) */
    uint32_t start_elect;             /* 起始电量(精度：0.001) */
    uint32_t current_elect;           /* 当前电量(精度：0.001) */
    uint32_t ammeter_elect;           /* 电表电量(精度：0.001) */
    uint32_t charge_elect_last;       /* 上一次计算所充电量(精度：0.001) */
    uint32_t current_time;            /* 当前时间(时间戳) */
    uint32_t start_time;              /* 充电开始时间(时间戳) */
    uint32_t stop_time;               /* 充电结束时间(时间戳) */
    uint32_t charge_time;             /* 充电时长(单位：s) */
    uint32_t start_charge_tick;       /* 启动时的时基 */
    uint32_t reason_code;             /* 原因码  */
    uint8_t charge_strategy;          /* 充电策略参数 */
    uint32_t charge_strategy_para;    /* 充电策略参数 */
    uint8_t start_period;             /* 开始时段 */
    uint8_t current_period;           /* 当前时段 */
    uint8_t period_num;               /* 跨时段总数 */
    uint32_t account_ballance_before; /* 充电前卡(账户)余额 */
    uint32_t account_ballance_after;  /* 充电后卡(账户)余额 */

    uint8_t reservation_strategy;     /* 预约策略 */
    uint8_t reservation_strategy_para;/* 预约策略参数 */
    int32_t reservation_time_remain;  /* 预约剩余时间(单位:s) */
    uint32_t reservation_time_base;   /* 预约基时间(单位:s(时间戳)) */

    uint8_t power_strategy;           /* 调功率策略 */
    uint8_t power_strategy_para;      /* 调功率策略参数 */

    uint16_t offline_chargetime;      /* 离线可充电时长(单位s) */
    uint32_t offline_tick;            /* 离线时基 */
    uint32_t order_fixes_tick;        /* 断电订单电量矫正时基 */
#ifdef APP_INCLUDE_YKC17_PROTOCOL
    /** 电表加密 */
    uint32_t meter_reading_tick;      /* 抄表时基 */
    uint8_t meter_step;               /* 步骤 */
#endif /* APP_INCLUDE_YKC17_PROTOCOL */
    uint8_t device_state;             /* 设备状态 */
#ifdef APP_INCLUDE_SGCC_PROTOCOL
    uint8_t device_transaction_number[40 + 1];  /* 设备流水号 */
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    uint8_t main_gunno;              /* 并充主枪枪号 */
    uint8_t charge_way;              /* 充电方式 */
#if (defined(APP_USING_NO_BMS) && defined(APP_USING_OFFLINE_BILLING))
    uint8_t battery_type;            /* 电池类型 */
    uint8_t bat_charge_stage;        /* 电池充电阶段 */
    uint16_t setup_current;          /* 设置的电流 */
    uint32_t parameter_steady_tick;  /* 参数稳定时基 */
    uint32_t current_rise_tick;      /* 电流爬升时基 */
#endif /* defined(APP_USING_NO_BMS) && defined(APP_USING_OFFLINE_BILLING) */
    uint8_t run_mode;                /* 运行模式：0：4G、以太网联网，1：离线计费，2：离线模式，3.即插即充 */
    void *bms_data;                  /* BMS 数据 */
    uint32_t reset_reason;           /* 重启原因：RCC->CSR 寄存器 */
#ifdef APP_USING_METER_ELECT_DETECT_STRATEGY
    /** 电表电量检验 */
    uint8_t melect_check_time;       /* 电表电量检测周期 */
    uint32_t melect_last;            /* 上一次电量值 */
    uint8_t melect_check_stage;      /* 电表电量检测阶段 */
    uint8_t melect_err_count;        /* 电量错误次数 */
#endif /* APP_USING_METER_ELECT_DETECT_STRATEGY */

#ifdef APP_USING_BAT_VOLT_DETECT_STRATEGY
    /** 电池电压检验 */
    uint8_t bvolt_check_time;        /* 电池电压检测时基 */
    uint16_t bvolt_init;             /* 电池电压初始值 */
    uint8_t bvolt_err_i;             /* 电池电压错误计数 */
    uint8_t bvolt_err_count;         /* 电池电压错误计数 */
#endif /* APP_USING_BAT_VOLT_DETECT_STRATEGY */

#ifdef APP_USING_CHARGE_CURR_DETECT_STRATEGY
    /** 电流检验 */
    uint16_t total_period_time;      /* 检测总周期时基 */
    uint8_t current_check_time;      /* 电流检测时基 */
    uint32_t meter_curr_last;        /* 前一次电表电流 */
    uint32_t module_curr_last;       /* 前一次模块电流 */
    uint8_t meter_curr_steady_count; /* 电表电流稳定次数 */
    uint8_t module_curr_steady_count; /* 模块电流稳定次数 */
#endif /* APP_USING_CHARGE_CURR_DETECT_STRATEGY */

#ifdef APP_USING_FB_DETECT
    /** 电子锁、继电器状态检验 */
    uint8_t elock_resume_time;       /* 电子锁故障恢复时基 */
    uint8_t elock_check_time;        /* 电子锁状态检测时基 */
    uint8_t dcrealy_resume_time;     /* 直流继电器故障恢复时基 */
    uint8_t dcrealy_check_time;      /* 直流继电器状态检测时基 */
    uint8_t acrelay_resume_time;     /* 交流接触器故障恢复时基 */
    uint8_t acrelay_check_time;      /* 交流接触器状态检测时基 */
#endif /* APP_USING_FB_DETECT */
}System_BaseData;

struct ofsm_info {
    enum ofsm_state state;
    System_BaseData base;
    uint32_t elect_calculate_tick; /* 电量计算时基 */
    uint32_t charge_timeout;     /* 启动超时退出 */
    uint32_t timing_tick;        /* 计时tick(用于订单时段计算) */
};
/*******************************************************************************************/

#pragma pack()

void ofsm_fun_list_init(void);
struct ofsm_info *get_ofsm_info(uint8_t gunno);
uint8_t ofsm_get_current_period(void);
uint32_t ofsm_get_period_price(uint8_t gunno, uint8_t period);
int32_t ofsm_get_current_period_time_hm(uint8_t period, uint8_t *buf, uint8_t blen);

#ifdef APP_DESIGNATE_REGION
void app_ofsm_info_init(void);
#endif /* APP_DESIGNATE_REGION */

void ofsm_thread_entry(void *parameter);

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
