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

#define APP_USING_DOUBLEGUN                            /* 使用双枪 */

#define START_CHARGE_TIMEOUT  (100 *1)
#define TINY_CURRENT_ABNOAMAL_VALUE          200       /* 小电流异常值 */
#define TINY_CURRENT_ABNOAMAL_TIMEOUT        1800000   /* 小电流异常持续时间 30 *60 *1000  */

#define APP_STORAGE_TRANSATION_INTERVAL      (30 *60 *1000)   /* 充电中保存交易记录间隔 */
#define APP_CARD_NUMBER_COMPARE_LEN          16        /* 卡号对比长度 */
#define OVERTEMP_DECREASE_CURR_PERCENT       5 /10     /* 过温降流百分比 */

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

enum charge_strategy_enum{
    APP_CHARGE_STRATEGY_TIME,                          /* 充电策略：按时间(单位是s) */
    APP_CHARGE_STRATEGY_ELECT,                         /* 充电策略：按电量(单位0.001度) */
    APP_CHARGE_STRATEGY_MONEY,                         /* 充电策略：按金额(单位0.0001元) */
    APP_CHARGE_STRATEGY_RESERVATION,                   /* 充电策略：预约 */
    APP_CHARGE_STRATEGY_FULL,                          /* 充电策略：充满 */
    APP_CHARGE_STRATEGY_SOC,                           /* 充电策略：按SOC */
};

enum system_start_way{
    APP_CHARGE_START_WAY_APP,                          /* 启动方式：APP */
    APP_CHARGE_START_WAY_ONLINE_CARD,                  /* 启动方式：在线卡 */
    APP_CHARGE_START_WAY_OFFLINE_CARD,                 /* 启动方式：离线卡 */
    APP_CHARGE_START_WAY_VIN,                          /* 启动方式：VIN码 */
    APP_CHARGE_START_WAY_SCREEN,                       /* 启动方式：屏幕 */
    APP_CHARGE_START_WAY_BLUE,                         /* 启动方式：蓝牙 */
    APP_CHARGE_START_WAY_PLUG_AND_CHARGE,              /* 启动方式：即插即充 */
    APP_CHARGE_START_WAY_TIMING,                       /* 启动方式：定时预约 */
};

enum net_state{
    APP_NET_STATE_NULL = 0,     //物理层故障（网卡故障）
    APP_NET_STATE_CARD,         //检测SIM卡异常
    APP_NET_STATE_UP,           //链路层故障（GPRS网络注册异常）
    APP_NET_STATE_INTERNET_UP,  //连接平台中
    APP_NET_STATE_AUTH_SECCESS, //连接平台成功

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
    APP_OFSM_STATE_WAIT_NET,  /** 状态机状态：等待网络 */
    APP_OFSM_STATE_IDLEING,   /** 状态机状态： 空闲中 */
    APP_OFSM_STATE_READYING,  /** 状态机状态： 准备中 */
    APP_OFSM_STATE_STARTING,  /** 状态机状态： 开始中 */
    APP_OFSM_STATE_CHARGING,  /** 状态机状态： 充电中 */
    APP_OFSM_STATE_STOPING,   /** 状态机状态： 停止中 */
    APP_OFSM_STATE_FINISHING, /** 状态机状态： 充电完成未拔枪 */
    APP_OFSM_STATE_FAULTING,  /** 状态机状态： 故障 */

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
    APP_CHARGE_WAY_PARACHARGE,          /* 充电方式：并充 */
};

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
struct _order_state{
    uint8_t verify_fail : 1;              /* 上报确认失败 */
    uint8_t is_start_fail : 1;            /* 启动失败 */
    uint8_t is_charging : 1;              /* 正在充电 */
    uint8_t online_order : 1;             /* 在线订单 */
    uint8_t reserve : 4;
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
    uint8_t reserve0[32];
#else
    uint8_t reserve0[96];
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
    struct _order_state order_state;      /* 订单状态(平台确认:1, 未确认：2,  启动失败：3，充电结束：4) */

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
    uint16_t period_elect[APP_BILLING_RULE_PERIOD_MAX];            /* 时段电量 */
    uint16_t period_elect_fees[APP_BILLING_RULE_PERIOD_MAX];       /* 时段电费 */
    uint16_t period_service_fees[APP_BILLING_RULE_PERIOD_MAX];     /* 时段服务费 */
    uint16_t period_occupy_fees[APP_BILLING_RULE_PERIOD_MAX];      /* 时段占位费 */
    struct billing_rule rule;             /* 计费规则 */
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
#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
    uint32_t card_ballance_before;        /* 充电前卡余额 */
    uint32_t card_ballance_after;         /* 充电后卡余额 */
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */
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
    uint8_t reserve[16];                  /* 预留 */
}thaisen_transaction_t;
/*******************************************************************************************/

/****************************************** 系统状态、数据信息 *************************************************/
/****************************************** 系统状态、数据信息 *************************************************/
typedef struct{
    struct{
        uint16_t is_local_charging : 1;                      /* 是否本地启动标志 */
        uint16_t is_charge_complete : 1;                     /* 是否充电完成标志 */
        uint16_t vin_authorization_success : 1;              /* 是否VIN鉴权成功标志 */
        uint16_t vin_is_authorized : 1;                      /* 已进行VIN鉴权上报 */
        uint16_t card_authorization : 1;                     /* 是否进行刷卡鉴权标志 */
        uint16_t is_fault_stop : 1;                          /* 是否故障停充标志 */
        uint16_t connect_state : 2;                          /* 枪连接状态 */
        uint16_t is_pay_by_card : 1;                         /* 卡结算标志 */
        uint16_t start_result : 1;                           /* 启动结果 */
        uint16_t card_info_is_uid : 1;                       /* 卡信息是UID */
        uint16_t is_overtemp : 1;                            /* 过温 */
        uint16_t is_curr_decreased : 1;                      /* 过温已降流 */
        uint16_t permit_judge_complete : 1;                  /* 判断是否允许充电的过程已完成 */
        uint16_t is_starting : 1;                            /* 已发指令启动充电 */
        uint16_t reserve : 1;
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
    uint32_t account_balance;         /* 账户余额(精度：0.001) */
    uint32_t card_balance;            /* 卡内余额(精度：0.001) */
    uint32_t fees_total;              /* 总费用(精度：0.0001) */
    uint32_t elect_fees_total;        /* 电费总费用(精度：0.0001) */
    uint32_t service_fees_total;      /* 服务费总费用(精度：0.0001) */
    uint32_t start_elect;             /* 起始电量(精度：0.001) */
    uint32_t current_elect;           /* 当前电量(精度：0.001) */
    uint32_t ammeter_elect;           /* 电表电量(精度：0.001) */
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
#if (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL))
    uint32_t card_ballance_before;    /* 充电前卡余额 */
    uint32_t card_ballance_after;     /* 充电后卡余额 */
#endif /* (defined (APP_INCLUDE_SL_PROTOCOL) || defined (APP_INCLUDE_XJ_PROTOCOL)) */

#ifdef APP_INCLUDE_SGCC_PROTOCOL
    uint8_t device_transaction_number[40 + 1];  /* 设备流水号 */
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    uint8_t main_gunno;              /* 并充主枪枪号 */
    uint8_t charge_way;              /* 充电方式 */
    void *bms_data;                   /* BMS 数据 */
}System_BaseData;

struct ofsm_info {
    enum ofsm_state state;
    System_BaseData base;
    uint32_t charge_timeout;     /* 启动超时退出 */
    uint32_t timing_tick;        /* 计时tick(用于订单时段计算) */
};
/*******************************************************************************************/

#pragma pack()

void ofsm_fun_list_init(void);
struct ofsm_info *get_ofsm_info(uint8_t gunno);

void ofsm_thread_entry(void *parameter);

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
