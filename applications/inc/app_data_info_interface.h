/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-24     我的杨yang       the first version
 */
#ifndef APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_
#define APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_
#include "string.h"
#include "app_ofsm.h"
#include "app_hci.h"
#include "applications/version.h"
#include "chargepile_config.h"
#include "notfs_cfg.h"
#include "app_billing_rule.h"
#include "app_osupport.h"

#include "bsp_serial.h"

#include "mw_time.h"
#include "mw_cc1.h"
#include "mw_fault_check.h"
#include "mw_storage.h"
#include "mw_charge_control.h"

/** 此枚举需要与 Trigger_Page 数组的下标对应 */
enum thaisen_trig_event{
    THAISEN_TRIG_EVENT_CARD_LOCKED,                               /** 屏幕外部触发事件：卡被锁 */
    THAISEN_TRIG_EVENT_INVALID,                                   /** 屏幕外部触发事件：无效卡(非白名单卡、非场站卡) */
    THAISEN_TRIG_EVENT_CARD_NOBALLANCE,                           /** 屏幕外部触发事件：卡余额不足 */
    THAISEN_TRIG_EVENT_KEY_AUTHEN_FAIL,                           /** 屏幕外部触发事件：卡密钥认证失败 */
    THAISEN_TRIG_EVENT_INSERT_GUN,                                /** 屏幕外部触发事件：提示先插枪 */
    THAISEN_TRIG_EVENT_FEES_ERROR,                                /** 屏幕外部触发事件：提示计费信息设置错误 */
    THAISEN_TRIG_EVENT_FINISH,                                    /** 屏幕外部触发事件：充电结束, 刷卡结算 */
    THAISEN_TRIG_EVENT_PAYING,                                    /** 屏幕外部触发事件：结算中 */
    THAISEN_TRIG_EVENT_SWITCH_GUN,                                /** 屏幕外部触发事件：不是启动卡，请切换枪 */
    THAISEN_TRIG_EVENT_PAY_COMPLETE,                              /** 屏幕外部触发事件：结算完成 */
    THAISEN_TRIG_EVENT_IS_STARTING,                               /** 屏幕外部触发事件：启动中 */
    THAISEN_TRIG_EVENT_IS_CHARGING,                               /** 屏幕外部触发事件：此卡已启动充电 */
    THAISEN_TRIG_EVENT_FAULT_STOP,                                /** 屏幕外部触发事件：故障停止，请重新拔、插枪 */
#if 0
    THAISEN_TRIG_EVENT_WRITE_INFO,                                /** 屏幕外部触发事件：写卡信息失败 */
    THAISEN_TRIG_EVENT_START_FAIL,                                /** 屏幕外部触发事件：启动失败, 刷卡结算 */
#endif
    THAISEN_TRIG_EVENT_SIZE,                                      /** 屏幕外部触发事件： */
};

typedef enum{
    THAISEN_CONFIG_PAGE_SYSTEM_INFO,                              /** 屏幕配置页：系统信息 */
    THAISEN_CONFIG_PAGE_PILE_INFO,                                /** 屏幕配置页：桩信息 */
    THAISEN_CONFIG_PAGE_SERVER_INFO,                              /** 屏幕配置页：服务器信息 */
    THAISEN_CONFIG_PAGE_AMMETER_INFO,                             /** 屏幕配置页：电表信息 */
    THAISEN_CONFIG_PAGE_MODULE_INFO,                              /** 屏幕配置页：模块信息 */
    THAISEN_CONFIG_PAGE_VIN_INFO,                                 /** 屏幕配置页：VIN信息 */
    THAISEN_CONFIG_PAGE_PROTECT_INFO,                             /** 屏幕配置页：保护信息 */
    THAISEN_CONFIG_PAGE_FUNCTION_INFO,                            /** 屏幕配置页：功能配置信息 */
    THAISEN_CONFIG_PAGE_OFFLINE_BILLING_INFO,                     /** 屏幕配置页：离线计费信息 */
    THAISEN_CONFIG_PAGE_INPUT_7103_7101_INFO,                     /** 屏幕配置页：输入信息(7103/7101) */
    THAISEN_CONFIG_PAGE_PUBLIC_INPUT_7104_INFO,                   /** 屏幕配置页：通用输入信息(7104) */
    THAISEN_CONFIG_PAGE_GUN_INPUT_7104_INFO,                      /** 屏幕配置页：枪输入信息(7104) */
    THAISEN_CONFIG_PAGE_PUBLIC_OUTPUT_7104_INFO,                  /** 屏幕配置页：通用输出信息(7104) */
    THAISEN_CONFIG_PAGE_GUN_OUTPUT_7104_INFO,                     /** 屏幕配置页：枪输出信息(7104) */
    THAISEN_CONFIG_PAGE_SIZE,                                     /** 屏幕配置页： */
}thaisen_cfg_page;

/** 负值为失败 */
#define THAISEN_CONFIG_FAIL_OFFSET                  0x02         /** 配置条目失败结果偏移 */
#define THAISEN_CONFIG_SYSTEM_ASSERT                -0x01        /** 配置结果：系统断言失败最大值 */
#define THAISEN_CONFIG_SUCCESS                      0x00         /** 配置结果：成功 */
#define THAISEN_CONFIG_FAIL_STORAGR                 0x01         /** 配置结果：保存失败 */
/** > THAISEN_CONFIG_FAIL_OFFSET 表示某个配置项(成员顺序位置，从2开始)配置失败 */

/** 注：以下结构必须要和监控平台部分定义的一致 */
/** 注：以下结构必须要和监控平台部分定义的一致 */
/** 注：以下结构必须要和监控平台部分定义的一致 */
#pragma pack(1)
/** 参数配置页面:屏幕-设置-系统设置-系统 */
typedef struct{
    uint8_t dev_function;                                         /** 本机功能(0：单枪终端，1：均充双枪，2：双枪终端，3：整流柜，4：动态切换) */
    uint8_t allocate_way;                                         /** 分配方式(0：均充, 1：先到先得, 2：功率优先) */
    uint16_t terminal_addr[2];                                    /** 终端地址(两把枪：A枪在前) */
}thaisen_cfg_info_system;
/** 参数配置页面:屏幕-设置-系统设置-桩信息 */
typedef struct{
    /** 桩号使用其它报文 */
    uint8_t qrcode_prefix[128];                                   /** 二维码前缀(前2字节分别为设置格式和生成格式，第3字节开始是实际数据) */
    uint8_t qrcode_suffix[128];                                   /** 二维码后缀 */
    uint8_t help_number[32];                                      /** 帮助电话 */
    uint8_t screen_password[15];                                  /** 屏幕密码 */
}thaisen_cfg_info_pile;
/** 参数配置页面:屏幕-设置-系统设置-服务器信息 */
typedef struct{
    uint8_t domain[256];                                          /** 域名 */
    uint16_t port;                                                /** 端口 */
    uint8_t net_mode;                                             /** 网络模式(0：4G，1：以太网，2：离线) */
}thaisen_cfg_info_server;
/** 参数配置页面:屏幕-设置-系统设置-电表 */
typedef struct{
    uint8_t ammeter_addr[2][13];                                  /** 电表地址(两把枪：A枪在前; 地址12位，最后一位填空字符) */
    uint8_t ammeter_model;                                        /** 电表协议(0：瑞银，1：雅达，2：科达瑞，3：英利达，4：安科瑞，5：科为，6：预留) */
    uint8_t baudrate;                                             /** 波特率 */
    uint8_t check_way;                                            /** 校验位(0:偶校验, 1:奇校验, 2:无校验) */
}thaisen_cfg_info_ammeter;
/** 参数配置页面:屏幕-设置-系统设置-模块信息 */
typedef struct{
    uint8_t module_protocol;                                      /** 模块协议(0：英飞源，1：国网，2：永联，3：优优，4：易能，5：预留) */
    uint8_t module_group;                                         /** 模块组数 */
    uint8_t module_num_single[8];                                 /** 每组模块数(目前定死8组) */
    uint16_t module_rated_voltage;                                /** 模块额定电压 */
    uint16_t module_rated_current;                                /** 模块额定电流 */
    uint16_t pile_outvoltage_max;                                 /** 桩最大输出电压 */
    uint16_t pile_outvoltage_min;                                 /** 桩最小输出电压 */
    uint16_t pile_outcurrent_max;                                 /** 桩最大输出电流 */
    uint16_t pile_outcurrent_min;                                 /** 桩最小输出电流 */
}thaisen_cfg_info_module;
/** 参数配置页面:屏幕-设置-系统设置-VIN */
typedef struct{
    uint8_t vin_whitelist[6][18];                                 /** 目前最多6个VIN码，VIN码17位(最后一位填空字符) */
}thaisen_cfg_info_vin;


/** 参数配置页面:屏幕-设置-出厂设置-保护信息 */
typedef struct{
    uint16_t overtemp_alarm;                                      /** 过温告警值(范围：1-300) */
    uint16_t overtemp_stop;                                       /** 过温停充值(范围：1-300) */
    uint16_t overtemp_recovery;                                   /** 过温恢复值(范围：1-120) */
    uint16_t overtemp_limitcur;                                   /** 过温限流值(范围：1-300) */
    uint16_t gunvolt_limit;                                       /** 枪头电压限值(100倍) */
    uint16_t soc_stop;                                            /** 停充 SOC(范围：1-100) */
    uint16_t power_percent;                                       /** 功率百分比(10倍) */
    uint16_t eloss_proportion;                                    /** 电损比(10倍) */
    uint16_t cc1_12_max;                                          /** CC1 12V 上限(100倍) */
    uint16_t cc1_12_min;                                          /** CC1 12V 下限(100倍) */
    uint16_t cc1_6_max;                                           /** CC1 6V 上限(100倍) */
    uint16_t cc1_6_min;                                           /** CC1 6V 下限(100倍) */
    uint16_t cc1_4_max;                                           /** CC1 4V 上限(100倍) */
    uint16_t cc1_4_min;                                           /** CC1 4V 下限(100倍) */
}thaisen_cfg_info_protect;
/** 参数配置页面:屏幕-设置-出厂设置-功能配置 */
typedef struct{
    uint16_t insult_detect : 1;                                   /** 绝缘检测(1：启用，0：禁用) */
    uint16_t card_reader : 1;                                     /** 读卡器(1：启用，0：禁用) */
    uint16_t parallel_charge : 1;                                 /** 并充(1：启用，0：禁用) */
    uint16_t vin_charge : 1;                                      /** VIN码(1：启用，0：禁用) */
    uint16_t parallel_relay : 1;                                  /** 并联继电器(1：启用，0：禁用) */
    uint16_t module_silence : 1;                                  /** 模块静音(1：启用，0：禁用) */
    uint16_t plug_charge : 1;                                     /** 即插即充(1：启用，0：禁用) */
    uint16_t local_start : 1;                                     /** 本地启动(1：启用，0：禁用) */
    uint16_t local_stop : 1;                                      /** 本地停止(1：启用，0：禁用) */
    uint16_t auxpower_24V : 1;                                    /** 24V辅源(1：启用，0：禁用) */
    uint16_t offline_billing : 1;                                 /** 离线计费(1：启用，0：禁用) */
    uint16_t password_start : 1;                                  /** 密码启动(1：启用，0：禁用) */
}thaisen_cfg_info_function;
/** 参数配置页面:屏幕-设置-出厂设置-离线计费 */
struct _time_info{
    uint8_t start_hour;                                           /** 时段开始小时(范围：0-23) */
    uint8_t start_min;                                            /** 时段开始分钟(范围：0-59) */
    uint8_t end_hour;                                             /** 时段结束小时(范围：0-23) */
    uint8_t end_min;                                              /** 时段结束分钟(范围：0-59) */
    uint8_t rated_number;                                         /** 时段费率号(0：尖尖，1：尖，2：峰，3：平，4：谷) */
};
typedef struct{
    uint32_t service_price;                                       /** 服务费价格(单位：元，10000倍) */
    uint32_t sharp_sharp_price;                                   /** 尖尖电费价格(单位：元，10000倍) */
    struct _time_info sstime1;                                    /** 尖尖时段1 */
    struct _time_info sstime2;                                    /** 尖尖时段2 */
    uint32_t sharp_price;                                         /** 尖电费价格(单位：元，10000倍) */
    struct _time_info stime1;                                     /** 尖时段1 */
    struct _time_info stime2;                                     /** 尖时段2 */
    uint32_t peak_price;                                          /** 峰电费价格(单位：元，10000倍) */
    struct _time_info ptime1;                                     /** 峰时段1 */
    struct _time_info ptime2;                                     /** 峰时段2 */
    uint32_t flat_price;                                          /** 平电费价格(单位：元，10000倍) */
    struct _time_info ftime1;                                     /** 平时段1 */
    struct _time_info ftime2;                                     /** 平时段2 */
    uint32_t valley_price;                                        /** 谷电费价格(单位：元，10000倍) */
    struct _time_info vtime1;                                     /** 谷时段1 */
    struct _time_info vtime2;                                     /** 谷时段2 */
}thaisen_cfg_info_offline_billing;

/************************************* 7103/7101 *********************************************/
/** 参数配置页面:屏幕-设置-出厂设置-输入信息 */
struct _input_pair_7103_7101{
    uint8_t enable : 4;                                           /** 1：启用，0：禁用 */
    uint8_t reversal : 4;                                         /** 1：取反，0：不取反 */
};
typedef struct{
    struct _input_pair_7103_7101 scram;                           /** 急停 */
    struct _input_pair_7103_7101 door;                            /** 门禁 */
    struct _input_pair_7103_7101 acrelay;                         /** 交流接触器 */
    struct _input_pair_7103_7101 dcrelay;                         /** 直流接触器 */
    struct _input_pair_7103_7101 fan;                             /** 风扇 */
    struct _input_pair_7103_7101 elock;                           /** 电子锁 */
    struct _input_pair_7103_7101 tempprotect;                     /** 温度保护 */
}thaisen_cfg_info_input_7103_7101;

/************************************* 7104 *********************************************/
/** 参数配置页面:屏幕-设置-出厂设置-通用输入信息 */
struct _input_pair_7104{
    uint8_t port_number;                                          /** 输入口号(NET_YKC_MONITOR_INPUT_PORT_MIN - NET_YKC_MONITOR_INPUT_PORT_MAX) */
    struct{
        uint8_t enable : 4;                                       /** 1：启用，0：禁用 */
        uint8_t reversal : 4;                                     /** 1：取反，0：不取反 */
    }state;
};
typedef struct{
    struct _input_pair_7104 protectlight;                         /** 防雷器 */
    struct _input_pair_7104 parallel_relay1;                      /** 母联1 */
    struct _input_pair_7104 parallel_relay2;                      /** 母联2 */
    struct _input_pair_7104 parallel_relay3;                      /** 母联3 */
    struct _input_pair_7104 scram;                                /** 急停 */
    struct _input_pair_7104 breaker;                              /** 断路器 */
    struct _input_pair_7104 acrelay;                              /** 交流接触器 */
    struct _input_pair_7104 fan;                                  /** 风扇 */
    struct _input_pair_7104 flooding;                             /** 水浸 */
    struct _input_pair_7104 door;                                 /** 门禁 */
    struct _input_pair_7104 smoke;                                /** 烟感 */
    struct _input_pair_7104 fall;                                 /** 倾倒 */
}thaisen_cfg_info_public_input_7104;
/** 参数配置页面:屏幕-设置-出厂设置-A/B枪输入信息 */
typedef struct{
    struct _input_pair_7104 dcrelay;                              /** 直流继电器 */
    struct _input_pair_7104 elock;                                /** 电子锁 */
    struct _input_pair_7104 gunsite;                              /** 枪座 */
    struct _input_pair_7104 liquid;                               /** 液冷 */
    struct _input_pair_7104 fuse;                                 /** 熔断器 */
    struct _input_pair_7104 temp_detect;                          /** 温度检测 */
}thaisen_cfg_info_gun_input_7104;

/** 参数配置页面:屏幕-设置-出厂设置-通用输出信息 */
struct _output_config_7104{
    uint8_t port_number;                                          /** 输入口号(NET_YKC_MONITOR_OUTPUT_PORT_MIN - NET_YKC_MONITOR_OUTPUT_PORT_MAX) */
    uint8_t enable;                                               /** 1:启用, 0:禁用 */
};
typedef struct{
    struct _output_config_7104 fan;                               /** 风扇 */
    struct _output_config_7104 parallel_relay1;                   /** 母联1 */
    struct _output_config_7104 parallel_relay2;                   /** 母联2 */
    struct _output_config_7104 parallel_relay3;                   /** 母联3 */
    struct _output_config_7104 acrelay;                           /** 交流接触器 */
}thaisen_cfg_info_public_output_7104;
/** 参数配置页面:屏幕-设置-出厂设置-A/B枪输出信息 */
typedef struct{
    struct _output_config_7104 auxpower_24V;                      /** 24V辅源 */
    struct _output_config_7104 auxpower_12V;                      /** 12V辅源 */
    struct _output_config_7104 dcrelay;                           /** 直流继电器 */
    struct _output_config_7104 relief;                            /** 泄放 */
    struct _output_config_7104 elock;                             /** 电子锁 */
    struct _output_config_7104 liquid;                            /** 液冷 */
}thaisen_cfg_info_gun_output_7104;

#pragma pack()
/**
 * 外部触发执行信息配置
 **/
int32_t thaisen_trigger_config_execute(uint8_t gunno, thaisen_cfg_page page, void *config, void *sub_config, void *sub_sub_config);

/**
 * 获取充电状态
 **/
enum ofsm_state thaisen_app_get_ofsm_charge_state(uint8_t gunno);
/**
 * 获取网络状态
 **/
enum net_state thaisen_app_get_net_state(void);

struct app_version{
    uint8_t version[20];
};
struct app_version *thaisen_app_get_app_version(void);

/**
 * 获取二维码
 **/
#define QRCODE_BUFF_LEN    200     /* 二维码缓存长度 */

struct qrcode_info{
    uint8_t qrcode[QRCODE_BUFF_LEN];   /* 二维码缓存 */
    uint16_t qrcode_len;               /* 二维码长度 */
};
struct qrcode_info *thaisen_app_get_gunno_qrcode(uint8_t gunno);
/**
 * 获取设备枪号名
 **/
void thaisen_get_device_sn(char *src, uint8_t length, uint8_t gunno);

/**
 * 获取枪连接状态
 **/
enum connect_state{
    APP_GUN_CONNECT_STATE_NO,    /* 未连接 */
    APP_GUN_CONNECT_STATE_YES,   /* 已连接 */
};
uint8_t thaisen_app_get_connect_state(uint8_t gunno);
/**
 * 获取停充方式
 **/
enum system_stop_way thaisen_app_get_charge_stop_way(uint8_t gunno);
/**
 * 获取充电数据
 **/
struct charge_data{
    uint8_t trade_number[16];       /* 流水号 */
    uint32_t charge_voltage;        /* 充电电压：精度2位 ,单位：V*/
    uint32_t charge_current;        /* 充电电流：精度2位 ,单位：A */
    uint32_t charge_power;          /* 充电功率：精度1位 ,单位：W */
    uint32_t charge_elect;          /* 充电电量：精度3位 ,单位：KWh */
    uint32_t charge_total_fee;      /* 费用：精度4位 */
    uint32_t account_ballance;      /* 账户余额：精度4位 */
    uint32_t charge_start_time;     /* 充电开始时间：单位s */
    uint32_t charge_stop_time;      /* 充电结束时间：单位s */
    uint32_t charge_time;           /* 充电时间：单位s */
    uint32_t remain_charge_time;    /* 剩余充电时间：单位s */
    uint16_t current_soc;           /* 当前SOC */
};
struct charge_data *thaisen_app_get_charge_info(uint8_t gunno);
/**
 * 获取电池信息
 **/
struct battery_info{
    uint8_t battery_type;
};
struct battery_info *thaisen_app_get_battery_info(uint8_t gunno);
/**
 * 获取BMS信息
 **/
struct bms_info{
    uint16_t single_battery_max_voltage;
    uint16_t bms_require_voltage;
    uint16_t bms_require_current;
    uint16_t bms_remain_time;
};
struct bms_info *thaisen_app_get_bms_info(uint8_t gunno);
/**
 * 获取计费信息
 **/
struct account_info{
    uint32_t sharp_unit_fee;    /* 尖单价 精度：4位 */
    uint32_t sharp_elect;       /* 尖电量 精度：4位 */
    uint32_t sharp_account;     /* 尖金额 精度：4位 */

    uint32_t peak_unit_fee;     /* 峰单价 精度：4位 */
    uint32_t peak_elect;        /* 峰电量 精度：4位 */
    uint32_t peak_account;      /* 峰金额 精度：4位 */

    uint32_t flat_unit_fee;    /* 平单价 精度：4位 */
    uint32_t flat_elect;        /* 平电量 精度：4位 */
    uint32_t flat_account;      /* 平金额 精度：4位 */

    uint32_t valley_unit_fee;   /* 谷单价 精度：4位 */
    uint32_t valley_elect;      /* 谷电量 精度：4位 */
    uint32_t valley_account;    /* 谷金额 精度：4位 */

    uint32_t current_unit_fee;  /* 当前单价 精度：4位 */

};
struct account_info *thaisen_app_get_account_info(uint8_t gunno);
/**
 * 获取故障信息
 **/
struct fault_info{
    enum charge_fault_t charge_fault;
    enum system_fault_t system_fault;
};
struct fault_info *thaisen_app_get_fault_info(uint8_t gunno);

/**
 * 清除指定区域记录信息
 */
int32_t thaisen_app_clear_region_record_info(enum record_region region);
/**
 * 获取故障或充电记录当前下标
 */
int32_t thaisen_app_get_current_region_index(enum record_region region);
/**
 * 获取故障或充电记录总数
 */
int32_t thaisen_app_get_region_record_total_num(enum record_region region);
/**
 * 获取指定下标的故障记录
 */
int32_t thaisen_app_get_index_fault_record(struct error_info *error_buff, uint8_t index, enum record_region region);
/**
 * 获取指定下标的充电记录
 */
int32_t thaisen_app_get_index_charge_record(thaisen_transaction_t* bill_buff, uint8_t index, enum record_region region);

/**
 * 触发存储配置项数据
 **/
int32_t thaisen_app_storage_config_port(void);
/**
 * 同步配置项数据
 **/
int32_t thaisen_app_sync_config_item_port(enum config_name name, void* data, uint32_t len);
/**
 * 读配置项
 **/
uint8_t* thaisen_app_read_config_item_port(enum config_name name, uint8_t is_user_content);


/**
 * 向屏幕发数据
 **/
void thaisen_app_send_data_to_hci_uart(uint8_t* data, uint8_t len);
/**
 * 获取时间戳
 **/
uint32_t thaisen_app_get_current_timestamp(void);
/**
 * 获取系统tick(1ms)
 **/
uint32_t thaisen_app_get_system_tick(void);
/**
 * 设置屏幕启动充电指令
 **/
void thaisen_app_set_screen_start_charge(uint8_t gunno);
/**
 * 设置屏幕停止充电指令
 **/
void thaisen_app_set_screen_stop_charge(uint8_t gunno);
/**
 * 获取VIN码启动充电状态
 **/
uint8_t thaisen_app_get_vin_start_charge(uint8_t gunno);
/**
 * 清除VIN码启动充电指令
 **/
void thaisen_app_clear_vin_start_charge(uint8_t gunno);
/**
 * 设置VIN码启动充电指令
 **/
void thaisen_app_set_vin_start_charge(uint8_t gunno);
/**
 * 设置密码启动充电指令
 **/
void thaisen_app_set_password_start_charge(uint8_t gunno);

struct temperature{
    int8_t temp;       /* 动力电池最高温度 */
    uint8_t number;    /* 最高温度检测点编号 */
};

struct temperature* get_battery_temp_info(uint8_t gunno);
/**
 * 获取OTA 信息
 **/
ota_info* thaisen_app_get_ota_info(void);
/**
 * 获取时间同步标志
 **/
uint8_t thaisen_app_get_time_sync_flag(void);
/**
 * 获取当前页面的枪号
 **/
uint8_t thaisen_get_hci_page_pos(void);
/**
 * 获取信号强度
 **/
int thaisen_app_get_signal_strength(void);
/**
 * 获取SIM卡卡号
 **/
char *thaisen_app_get_sim_number(void);
/**
 * 获取屏幕时间同步标志
 **/
int8_t thaisen_get_screen_timesync_flag(void);
/**
 * 获取屏幕时间同步时间值
 **/
void thaisen_get_screen_timesync_time(uint16_t* buf, uint8_t len);
/**
 * 请求屏幕时间
 **/
void thaisen_request_screen_time(void);
/**
 * 根据停充码获取故障字符串
 **/
const char* thaisen_get_fault_string(uint16_t code);
/**
 * 获取CC1电压
 **/
uint8_t thaisen_get_cc1_voltage(uint8_t gunno);
/**
 * 获取枪头温度
 **/
int32_t thaisen_get_gun_temp(uint8_t gunno);
/**
 * 获取电表电压
 **/
uint32_t thaisen_get_ammeter_voltage(uint8_t gunno);
/**
 * 获取电表电流
 **/
uint32_t thaisen_get_ammeter_current(uint8_t gunno);
/**
 * 判断是否已经设置了电损比
 **/
uint8_t thaisen_is_set_eloss_proportion(void);
/**
 * 判断是否已经设置了功率百分比
 **/
uint8_t thaisen_is_set_power_percent(void);
/**
 * 获取功率百分比
 **/
int16_t thaisen_get_power_percent(void);
/**
 * 根据功率百分比获取功率值
 **/
uint32_t thaisen_get_power_from_percent(uint16_t percent);
/**
 * 添加VIN码白名单
 **/
int32_t thaisen_vin_whitelists_add(uint8_t *data, uint8_t len);
/**
 * 清空VIN码白名单
 **/
int32_t thaisen_vin_whitelists_clear(void);
/**
 * 获取充电方式
 **/
enum charge_way thaisen_get_charge_way(void);
/**
 * 设置充电方式
 **/
void thaisen_set_charge_way(uint8_t way);
/**
 * 获取当前时段时间
 **/
int32_t thaisen_get_current_period_time_hm(uint8_t *buf, uint8_t blen);
/**
 * 获取当前时段电费单价
 **/
uint32_t thaisen_get_period_price(uint8_t gunno, uint8_t period);
/**
 * 屏幕点击重启
 **/
uint8_t thaisen_query_screen_reboot(void);
void thaisen_clear_screen_reboot(void);
/**
 * 设置屏幕点击重启的事件
 **/
void thaisen_set_screen_reboot(void);

/**
 * 获取充电状态
 **/
uint8_t thaisen_get_charge_state(uint8_t gunno);
/**
 *  获取模块电压
 **/
int16_t thaisen_get_module_voltage(uint8_t gunno);
/**
 *  获取BCP电池电压
 **/
int16_t thaisen_get_bcp_voltage(uint8_t gunno);
/**
 * 获取BHM最大允许电压
 **/
int16_t thaisen_get_bhm_voltage(uint8_t gunno);
/**
 * 获取绝缘电压
 **/
int16_t thaisen_get_insult_voltage(uint8_t gunno);
/**
 * 设置屏幕外部触发事件
 **/
int32_t thaisen_set_trigger_event(enum thaisen_trig_event event, uint16_t duration_time, uint8_t just_notice, uint8_t gunno);
/**
 * 判断是否是在线启动
 **/
uint8_t thaisen_is_online_start(uint8_t gunno);

/**
 * 判断是否是不允许刷卡启动的页面
 **/
uint8_t thaisen_is_not_allow_swip_card(void);

/**
 * 进入临界区
 **/
void thaisen_enter_critical(void);

/**
 * 退出临界区
 **/
void thaisen_exit_critical(void);

/**
 * 判断屏幕启动倒计时是否已结束
 **/
uint8_t thaisen_is_countdown_finish(uint8_t gunno);

#endif /* APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_ */


