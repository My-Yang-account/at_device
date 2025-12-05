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
    THAISEN_CONFIG_PAGE_MODE_SELECT_NORMAL,                       /** 配置信息类型：模式选择：正常模式 */
    THAISEN_CONFIG_PAGE_MODE_SELECT_V2G,                          /** 配置信息类型：模式选择：V2G模式 */
    THAISEN_CONFIG_PAGE_OTHER_CONFIG,                             /** 配置信息类型：其它配置 */
    THAISEN_CONFIG_PAGE_DYNAMIC_CMD_INFO_ISSUE,                   /** 配置信息类型：动态类型指令信息-下发 */
    THAISEN_CONFIG_PAGE_DYNAMIC_CMD_INFO_READ,                    /** 配置信息类型：动态类型指令信息-读取 */
    THAISEN_CONFIG_PAGE_FIXED_CMD_INFO,                           /** 配置信息类型：固定类型指令信息 */
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
    /**  新增：2025/09/05*/
    uint8_t card_key[13];                                         /** 卡密钥：ASCII(前12字节有效) */
    uint8_t cardnumber_block;                                     /** 卡号所在块号(0-69) */
    uint8_t qrcode_rule;                                          /** 二维码规则(0：云快充，1：星星充电，2：新电途， 3：小桔， 4：云端下发，其它：非法) */
    uint8_t register_code[64];                                    /** 注册码：ASCII(前63字节有效) */
    uint8_t manufacturer_sn[9];                                   /** 厂商编码：ASCII(前8字节有效) */
    uint8_t random_str[32];                                       /** 随机串：ASCII(前31字节有效) */
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
    /** 新增 2025/11/04 */
    uint8_t lowpower_module;                                      /** 低功耗模块(0：无，1：易能) */
    uint16_t module_outcurrent_max;                               /** 模块最大输出电流(A) */
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
    /**  新增：2025/10/28*/
    uint32_t out_overvolt;                                        /** 输出过压值(0.01V) */
    uint32_t out_undervolt;                                       /** 输出欠压值(0.01V) */
    uint32_t in_overvolt;                                         /** 输入过压值(0.01V) */
    uint16_t in_undervolt;                                        /** 输入欠压值(0.01V) */
    uint32_t out_overcurr;                                        /** 输出过流值(0.01A) */
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
    /** 新增：2025/06/08 */
    uint16_t mode_select : 1;                                     /** 模式选择(1：启用，0：禁用) */
    uint16_t offline_card : 1;                                    /** 离线卡(1：启用，0：禁用) */
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

/** 模式选择页面：屏幕-选择枪-正常模式 */
typedef struct{
    uint8_t mode;                                                 /** 当前模式： 0：自动充满，1：限制金额，2：限制电量，3：限制时间，4：预约 */
    uint32_t mode_parameter;                                      /** 模式参数 ：
                                                                                                                                                                                      对于模式0：无用，默认填0
                                                                                                                                                                                      对于模式1：单位：0.01元
                                                                                                                                                                                      对于模式2：单位：0.001度
                                                                                                                                                                                      对于模式3：单位：1s
                                                                                                                                                                                      对于模式4：单位：1s(当天启动时间秒数：例 预约 13：56 充电，则为：13 *60 *60 + 56 *60)*/
}thaisen_mode_select_normal;

/** 模式选择页面：屏幕-选择枪-V2G模式 */
typedef struct{
    uint8_t mode;                                /** 当前模式 */
}thaisen_mode_select_v2g;

/** 参数配置页面:屏幕-设置-出厂设置-其它配置 */
/** 液冷 */
struct _liquid{
    uint8_t used : 4;                                             /** 液冷使用与否：1：使用   0：不使用 */
    uint8_t fdetect : 4;                                          /** 是否检测液冷故障：1：是   0：否 */
    uint8_t address;                                              /** 液冷地址 */
};

typedef struct {
    struct _liquid liquid[4];                                     /** 液冷 */
    uint16_t fan_work_time;                                       /** 停充后风扇工作时间(S) */
}thaisen_other_config;

/** 参数配置页面:屏幕-设置-出厂设置-固定类型指令调试修改 */
typedef struct{
    uint8_t msg_version;                                          /** 报文版本(初始版本为0) */
    struct{
        uint8_t batvolt_detect : 1;                               /** 电池电压检测(1：启用，0：禁用) */
        uint8_t bcltimeout_detect : 1;                            /** BCL超时检测(1：启用，0：禁用) */
        uint8_t fast_protocol : 1;                                /** FAST协议(1：启用，0：禁用) */
        uint8_t cfc_protocol : 1;                                 /** CFC协议(1：启用，0：禁用) */
        uint8_t bay_area_protocol : 1;                            /** 湾区协议(1：启用，0：禁用) */
        uint8_t protocol_gb_t : 1;                                /** 国标协议(27930)(1：启用，0：禁用) */
        uint8_t bms_several_frame : 1;                            /** BMS多帧(1：启用，0：禁用) */
        uint8_t reserve : 1;                                      /** 预留 */
    }info;
}thaisen_cfg_fixed_cmd_debug;

/** 参数配置页面:屏幕-设置-出厂设置-动态类型指令调试 */
/** 动态类型指令-修改，段 */
typedef struct{
    uint8_t parameter_len;                                        /** 参数长度 */
    uint8_t cmd[17];                                              /** 指令 */
    uint8_t parameter[33];                                        /** 参数 */
}thaisen_dynamic_cmd_modify_segment;
/** 动态类型指令修改 */
typedef struct{
    uint8_t msg_version;                                          /** 报文版本(初始版本为0) */
    uint8_t cmd_num;                                              /** 指令个数 */
    /** 以下是指令组数据 */
    /** thaisen_dynamic_cmd_modify_segment */
}thaisen_dynamic_cmd_modify;

/** 动态类型指令-读，段 */
typedef struct{
    uint8_t cmd[17];                                              /** 指令 */
}thaisen_dynamic_cmd_read_segment;
/** 动态类型指令读取 */
typedef struct{
    uint8_t msg_version;                                          /** 报文版本(初始版本为0) */
    uint8_t cmd_num;                                              /** 指令个数 */
    /** 以下是指令码数据，每个指令码占17字节 */
    /** @thaisen_dynamic_cmd_segment */
}thaisen_dynamic_cmd_read;

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
    struct _input_pair_7103_7101 pour;                            /** 倾倒 */
    struct _input_pair_7103_7101 protect_light;                   /** 防雷 */
    struct _input_pair_7103_7101 flood;                           /** 水浸 */
    struct _input_pair_7103_7101 smoke;                           /** 烟感 */
    struct _input_pair_7103_7101 gunsite;                         /** 枪座 */
    struct _input_pair_7103_7101 fuse;                            /** 熔断器 */
    struct _input_pair_7103_7101 liquid;                          /** 液冷 */
    struct _input_pair_7103_7101 circuit_breaker;                 /** 断路器 */
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
 * 获取中文停充方式
 **/
void thaisen_app_get_charge_stopway_chinese(uint32_t code, uint8_t *olen, uint8_t *buf, uint8_t ilen);

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
    uint16_t start_soc;             /* 结束SOC */
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
 * 获取中文故障信息
 **/
void thaisen_app_get_fault_chinese(uint32_t code, uint8_t *olen, uint8_t *buf, uint8_t ilen);

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
 * 获取电表数据
 **/
struct ammeter_data{
    int16_t voltage;       /* 电表电压(单位：0.1V) */
    int16_t current;       /* 电表电流(单位：0.1A) */
    uint32_t power;        /* 电表功率(单位：0.1W) */
    uint32_t elect;        /* 电表电量(单位：0.001度) */
};
struct ammeter_data *thaisen_get_ammeter_data(uint8_t gunno);
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
 * 根据指定枪号获取其并充主枪枪号
 **/
uint8_t thaisen_get_parallel_main_gunno(uint8_t gunno);
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
 * 判断是否允许本地停止
 **/
uint8_t thaisen_is_allow_loacl_stop(uint8_t gunno);

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

/**
 * 获取卡数据
 **/
struct card_data_info{
    uint8_t card_number[16];        /* 卡号 */
    uint8_t card_uuid[8];           /* 卡UUID */
    uint8_t card_type;              /* 卡类型 */
};

struct card_data_info *thaisen_get_card_info(uint8_t gunno);

/**
 * 判断屏幕是否处于调试模式
 **/
uint8_t thaisen_is_debug(void);

/**
 * 调试指令码
 */
enum thaisen_debug_cmd{
    THAISEN_DEBUG_CMD_ISSUE_MODULE_CURR_MAX,                     /** 调试指令码：下发模块最大输出电流 */
    THAISEN_DEBUG_CMD_ISSUE_MODULE_CURR_MIN,                     /** 调试指令码：下发模块最小输出电流 */
    THAISEN_DEBUG_CMD_READ_MODULE_CURR_MAX,                      /** 调试指令码：读取模块最大输出电流 */
    THAISEN_DEBUG_CMD_READ_MODULE_CURR_MIN,                      /** 调试指令码：读取模块最小输出电流 */
    THAISEN_DEBUG_CMD_SIZE,                                      /** 调试指令码 */
};
/**
 * 获取指令调试结果信息
 */
void thaisen_get_cmd_debug_result_info(uint8_t language, uint8_t cmd, uint8_t *para, uint8_t plen, uint8_t *buf, uint8_t ilen);

/**
 * 查询充电是否已停止
 **/
uint8_t thaisen_is_stoped_charge(uint8_t gunno);

typedef enum{
  THA_DEBUG_ITEM_ACRELAY_ON,                        /** 一键自检项信息：交流接触器闭合 */
  THA_DEBUG_ITEM_ACRELAY_OFF,                       /** 一键自检项信息：交流接触器断开 */
  THA_DEBUG_ITEM_PARARELAY_1_ON,                    /** 一键自检项信息：母联1闭合 */
  THA_DEBUG_ITEM_PARARELAY_1_OFF,                   /** 一键自检项信息：母联1断开 */
  THA_DEBUG_ITEM_PARARELAY_2_ON,                    /** 一键自检项信息：母联2闭合 */
  THA_DEBUG_ITEM_PARARELAY_2_OFF,                   /** 一键自检项信息：母联2断开 */
  THA_DEBUG_ITEM_PARARELAY_3_ON,                    /** 一键自检项信息：母联3闭合 */
  THA_DEBUG_ITEM_PARARELAY_3_OFF,                   /** 一键自检项信息：母联3断开 */
  THA_DEBUG_ITEM_FAN_ON_A,                          /** 一键自检项信息：A枪风扇闭合 */
  THA_DEBUG_ITEM_FAN_OFF_A,                         /** 一键自检项信息：A枪风扇断开 */
  THA_DEBUG_ITEM_FAN_ON_B,                          /** 一键自检项信息：B枪风扇闭合 */
  THA_DEBUG_ITEM_FAN_OFF_B,                         /** 一键自检项信息：B枪风扇断开 */
  THA_DEBUG_ITEM_DCRELAY_A_ON,                      /** 一键自检项信息：A枪直流继电器闭合 */
  THA_DEBUG_ITEM_DCRELAY_A_OFF,                     /** 一键自检项信息：A枪直流继电器断开 */
  THA_DEBUG_ITEM_DCRELAY_B_ON,                      /** 一键自检项信息：B枪直流继电器闭合 */
  THA_DEBUG_ITEM_DCRELAY_B_OFF,                     /** 一键自检项信息：B枪直流继电器断开 */
  THA_DEBUG_ITEM_ELOCK_A_ON,                        /** 一键自检项信息：A枪电子锁上锁 */
  THA_DEBUG_ITEM_ELOCK_A_OFF,                       /** 一键自检项信息：A枪电子锁解锁 */
  THA_DEBUG_ITEM_ELOCK_B_ON,                        /** 一键自检项信息：B枪电子锁上锁*/
  THA_DEBUG_ITEM_ELOCK_B_OFF,                       /** 一键自检项信息：B枪电子锁解锁 */
  THA_DEBUG_ITEM_AUX12V_A_ON,                      /** 一键自检项信息：A枪12V辅源闭合 */
  THA_DEBUG_ITEM_AUX12V_A_OFF,                     /** 一键自检项信息：A枪12V辅源断开 */
  THA_DEBUG_ITEM_AUX12V_B_ON,                      /** 一键自检项信息：B枪12V辅源闭合 */
  THA_DEBUG_ITEM_AUX12V_B_OFF,                     /** 一键自检项信息：B枪12V辅源断开 */
  THA_DEBUG_ITEM_AUX24V_A_ON,                      /** 一键自检项信息：A枪24V辅源闭合 */
  THA_DEBUG_ITEM_AUX24V_A_OFF,                     /** 一键自检项信息：A枪24V辅源断开 */
  THA_DEBUG_ITEM_AUX24V_B_ON,                      /** 一键自检项信息：B枪24V辅源闭合*/
  THA_DEBUG_ITEM_AUX24V_B_OFF,                     /** 一键自检项信息：B枪24V辅源断开 */

  THA_DEBUG_ITEM_COMPLETE,                         /** 一键自检项信息：自检完成 */
}tha_debug_chinese_en;

typedef enum{
  THA_DEBUG_LANGUAGE_ENGLISH,                          /** 一键自检信息语言：英文 */
  THA_DEBUG_LANGUAGE_CHINESE,                          /** 一键自检信息语言：中文 */
}tha_debug_language_en;

/**
 *  获取一键自检中文信息
 **/
void thaisen_selfcheck_debug_info(tha_debug_chinese_en item, uint8_t language, uint8_t ret, uint8_t *buf, uint8_t ilen);

enum thaisen_mode{
    THAISEN_MODE_CHARGE_FULL,                                     /** 充电模式：充满 */
    THAISEN_MODE_CHARGE_LIMIT_MONEY,                              /** 充电模式：限制金额(单位：0.01元) */
    THAISEN_MODE_CHARGE_LIMIT_ELECT,                              /** 充电模式：限制电量(单位：0.001度) */
    THAISEN_MODE_CHARGE_LIMIT_TIMING,                             /** 充电模式：定时(单位：1S) */
    THAISEN_MODE_CHARGE_LIMIT_RESERVATION,                        /** 充电模式：预约(单位：1S) */
    THAISEN_MODE_V2G_LIMIT_MONEY,                                 /** V2G模式：限制金额(单位：0.01元) */
    THAISEN_MODE_V2G_LIMIT_ELECT,                                 /** V2G模式：限制电量(单位：0.001度) */
    THAISEN_MODE_V2G_LIMIT_TIMING,                                /** V2G模式：定时(单位：1S) */
    THAISEN_MODE_V2G_AUTO,                                        /** V2G模式：自动(根据设置的放电截至SOC来) */
    THAISEN_MODE_SIZE,                                            /** 充电模式： */
};

enum thaisen_charge_mode{
    THAISEN_CHARGE_MODE_FULL,                                     /** 当前充电模式：充满 */
    THAISEN_CHARGE_MODE_LIMIT_MONEY,                              /** 当前充电模式：限制金额(单位：0.01元) */
    THAISEN_CHARGE_MODE_LIMIT_ELECT,                              /** 当前充电模式：限制电量(单位：0.001度) */
    THAISEN_CHARGE_MODE_LIMIT_TIMING,                             /** 当前充电模式：定时(单位：1S) */
    THAISEN_CHARGE_MODE_LIMIT_RESERVATION,                        /** 当前充电模式：预约(单位：1S) */
    THAISEN_CHARGE_MODE_SIZE,                                     /** 当前充电模式： */
};

enum thaisen_v2g_mode{
    THAISEN_V2G_MODE_LIMIT_MONEY,                                 /** 当前V2G模式：限制金额(单位：0.01元) */
    THAISEN_V2G_MODE_LIMIT_ELECT,                                 /** 当前V2G模式：限制电量(单位：0.001度) */
    THAISEN_V2G_MODE_LIMIT_TIMING,                                /** 当前V2G模式：定时(单位：1S) */
    THAISEN_V2G_MODE_AUTO,                                        /** 当前V2G模式：自动(根据设置的放电截至SOC来) */
    THAISEN_V2G_MODE_SIZE,                                        /** 当前V2G模式：空 */
};

 /**
 *  获取当前充电模式
 **/
enum thaisen_charge_mode thaisen_get_current_charge_mode(uint8_t gunno);

 /**
  *  获取充电模式选择参数
  **/
uint32_t thaisen_get_charge_mode_parameter(uint8_t gunno);

/**
 *  获取模式显示信息
 **/
void thaisen_get_mode_info(uint8_t gunno, uint8_t language, enum thaisen_mode mode, uint32_t parameter, uint8_t *buf, uint8_t ilen);

/**
 *  判断是否设置了预约模式
 **/
uint8_t thaisen_is_set_reservation_mode(uint8_t gunno);

/**
 *  清除预约模式设置标志
 **/
void thaisen_clear_reservation_mode_flag(uint8_t gunno);

/**
 * 获取离线计费卡信息
 **/
struct offline_billing_card_info{
    uint8_t card_number[17];        /* 卡号 */
    uint32_t card_ballance;         /* 卡余额 */
};
struct offline_billing_card_info *thaisen_get_offline_billing_card_info(uint8_t gunno);

/**
 * 获取模块故障显示信息
 **/
void thaisen_get_module_fault_info(uint8_t gunno, uint8_t language, uint8_t addr, uint16_t code, uint8_t *buf, uint8_t ilen);


enum thaisen_notice{
    THAISEN_NOTICE_PILE_INFO,                                     /** 信息修改提醒：桩信息 */
    THAISEN_NOTICE_REGISTER_CODE,                                 /** 信息修改提醒：注册码 */
    THAISEN_NOTICE_SIZE,                                          /** 信息修改提醒： */
};

 /**
 *  信息修改提醒
 **/
void thaisen_info_modify_notice(enum thaisen_notice info);

/**
*  启动充电模块(强制启动)
**/
void thaisen_open_charge_module(uint8_t gunno, uint32_t voltage, uint32_t current);

/**
*  关闭充电模块(强启后关闭)
**/
void thaisen_close_charge_module(uint8_t gunno);

#ifdef APP_INCLUDE_BATVOLT_DETECT_QRCODE
#define THAISEN_BATVOLT_DETECT_SN_LEN                       40
/**
*  设置电池电压检测报告交易号
**/
void thaisen_set_batvolt_detect_sn(uint8_t gunno, uint8_t *sn, uint8_t slen);

/**
*  获取电池电压检测报告二维码信息
**/
void thaisen_get_batvolt_detect_qrcode(uint8_t gunno, uint8_t *buf, uint8_t blen);

#endif /* APP_INCLUDE_BATVOLT_DETECT_QRCODE */

#ifdef APP_INCLUDE_V2G
typedef enum{
    THAISEN_GUN_RUNING_MODE_CHARGE,            /** 枪运行模式：充电 */
    THAISEN_GUN_RUNING_MODE_V2G,               /** 枪运行模式：V2G(车放电至电网) */
    THAISEN_GUN_RUNING_MODE_SIZE,              /** 枪运行模式 */
}thaisen_gun_run_mode_t;

/**
*  获取枪运行模式(@thaisen_gun_run_mode_t(默认充电模式))
**/
thaisen_gun_run_mode_t thaisen_get_gun_running_mode(uint8_t gunno);

/**
*  复位枪运行模式
**/
void thaisen_reset_gun_running_mode(uint8_t gunno);

#endif /* APP_INCLUDE_V2G */

#endif /* APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_ */


