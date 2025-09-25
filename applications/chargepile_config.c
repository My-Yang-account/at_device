/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-25     我的杨yang       the first version
 */
#include "chargepile_config.h"
#include "mw_norflash.h"

#include <rtthread.h>

#define DBG_TAG "config"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define SYSTEM_INIT_KEY    ((uint32_t)(0x12345678))

#define SYS_DESIGNATE_REGION           /* 变量定义到指定区 */

static uint8_t s_sys_config_lock = 0x01;

#pragma pack(1)

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)

#define APP_TARGET_PLATFORM_ADDITIONAL_REGION_SIZE            (100 *1024)
struct system_config_tp_additional{
    uint32_t init_flag;
    uint8_t verify_result;         /* 数据校验结果 */
    uint8_t reserve[APP_TARGET_PLATFORM_ADDITIONAL_REGION_SIZE - 0x09];
    uint32_t crc;
};

static uint8_t s_config_tp_additional_lock = 0x01;

#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */

struct card_whitelist_info{
    uint8_t card_number[CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX][CARD_NUMBER_LENGTH_DEF + 0x01];
    uint8_t card_uid[CP_INFO_CARD_UID_WHITELIST_NUM_MAX][CARD_UID_LENGTH_DEF];
};

struct _network{
    uint8_t domain[CP_NETWORK_DOMAIN_LEN_MAX];                        /* 域名 */
    uint8_t domain_len;                                               /* 域名实际长度 */

    uint16_t port;                                                    /* 端口 */

    uint8_t mac[CP_NETWORK_MAC_ADDR_LEN_MAX];                         /* MAC地址 */

    uint8_t gateway[CP_NETWORK_GATEWAY_LEN_MAX];                      /* 网关地址 */

    uint8_t nettype;                                                  /* 网络类型(联网方式) */
    uint8_t reserve[31];                                              /* 预留 */
};

struct _encrypt{
    uint8_t key[CP_ENCRYPT_KEY_LEN_MAX];                              /* 密钥 */
    uint8_t key_len;                                                  /* 密钥实际长度 */

    uint8_t random_str[CP_ENCRYPT_RANDOM_STR_LEN_MAX];                /* 随机串 */
    uint8_t random_str_len;                                           /* 随机串实际长度 */

    uint8_t id[CP_ENCRYPT_ID_LEN_MAX];                                /* ID */
    uint8_t id_len;                                                   /* ID实际长度 */

    uint8_t sign[CP_ENCRYPT_SIGN_LEN_MAX];                            /* 签名 */
    uint8_t sign_len;                                                 /* 签名实际长度 */

    uint8_t reserve[32];                                              /* 预留 */
};

struct _pile_info{
    uint8_t pile_number[CP_INFO_PILE_NUMBER_LEN_MAX];                 /* 桩号 */
    uint8_t pile_num_len;                                             /* 桩号实际长度 */

    uint8_t serial_number[CP_INFO_SERIAL_NUMBER_LEN_MAX];             /* 序列号 */
    uint8_t serial_number_len;                                        /* 序列号实际场地 */

    uint8_t help_number[CP_INFO_HELP_NUMBER_LEN_MAX];                 /* 帮助电话 */
    uint8_t help_number_len;                                          /* 帮助电话实际长度 */

    uint8_t user_identity[CP_INFO_USER_IDENTITY_LEN_MAX];             /* 用户识别码 */

    uint8_t reserve[32 - 5];                                          /* 预留 */
};

struct _config_para{
    uint16_t cc1_12_max;                                              /* CC1 12V 上限 */
    uint16_t cc1_12_min;                                              /* CC1 12V 下限 */

    uint16_t cc1_6_max;                                               /* CC1 6V 上限 */
    uint16_t cc1_6_min;                                               /* CC1 6V 下限 */

    uint16_t cc1_4_max;                                               /* CC1 4V 上限 */
    uint16_t cc1_4_min;                                               /* CC1 4V 下限 */

    uint16_t module_rated_outvolt;                                    /* 模块额定输出电压 */
    uint16_t pile_max_outvolt;                                        /* 桩最大输出电压 */
    uint16_t pile_min_outvolt;                                        /* 桩最小输出电压 */
    uint16_t module_rated_limit_curr;                                 /* 模块额定限电流 */
    uint16_t pile_max_limit_curr;                                     /* 桩最大限电流 */
    uint16_t pile_min_limit_curr;                                     /* 桩最小限电流 */
    uint8_t reserve0[8];

    uint16_t soc_stop;                                                /* 停充 SOC */

    uint32_t input_overvol;                                           /* 输入过压值 */
    uint32_t input_undervol;                                          /* 输入欠压值 */
    uint32_t output_overvol;                                          /* 输出过压值 */
    uint32_t output_undervol;                                         /* 输出欠压值 */
    uint32_t output_overcur;                                          /* 输出过流值 */

    uint16_t overtemp_alarm;                                          /* 过温告警值 */
    uint16_t overtemp_stop;                                           /* 过温停充值 */
    uint16_t overtemp_recovery;                                       /* 过温恢复值 */
    uint16_t overtemp_limitcur;                                       /* 过温限流值 */

    uint16_t eloss_proportion;                                        /* 电损比 */
    uint32_t mode_parameter[2];                                       /* 模式参数 */
    uint8_t reserve1[256 - 10];                                       /* 预留 */
};

struct _config_info{
    uint8_t qrcode_prefix[CP_INFO_QRCODE_PREFIX_LEN_MAX];             /* 二维码前缀 */
    uint8_t qrcode_suffix[CP_INFO_QRCODE_SUFFIX_LEN_MAX];             /* 二维码后缀 */
    uint8_t prefix_length;                                            /* 二维码前缀实际长度 */
    uint8_t suffix_length;                                            /* 二维码后缀实际长度 */

    uint8_t vin_whitelist[CP_INFO_VIN_WHITELIST_NUM_MAX][VIN_CODE_LENGTH_DEF + 0x01];      /* VIN 码白名单 */
    struct card_whitelist_info card_whitelist;                        /* 卡 白名单 */

    uint8_t meter_address[2][CP_INFO_METER_ADDRESS_LEN_MAX];          /* 电表地址 */
    uint8_t meter_model;                                              /* 电表型号 */

    uint8_t card_type;                                                /* 卡类型 */

    uint8_t module_model;                                             /* 模块型号 */
    uint8_t module_group_num;                                         /* 模块组数 */
    uint8_t module_num_singlegroup[8];                                /* 单组模块最大数量 */

    uint8_t screen_password[CP_INFO_SCREEN_PASSWORD_LEN_MAX];         /* 屏幕密码 */

    uint32_t system_power_total;                                      /* 系统总功率 */
    uint8_t power_alloc_way;                                          /* 功率分配方式 */
    uint16_t gunvolt_limit;                                           /* 枪头电压限值 */
    uint8_t system_function;                                          /* 本机功能 */
    uint8_t gun_num;                                                  /* 枪数量 */

    uint16_t teminal_addrA;                                           /* A枪终端地址 */
    uint16_t teminal_addrB;                                           /* B枪终端地址 */

    uint8_t ammeter_check_way;                                        /* 电表串口校验方式 */
    uint8_t ammeter_baudrate;                                         /* 电表串口波特率 */
    uint8_t register_code[CP_INFO_REGISTER_CODE_LEN_MAX];             /* 注册码 */
    uint8_t card_key[CP_INFO_CARD_KEY_LEN_MAX];                       /* 卡密钥 */
    uint8_t lp_consumption_module;                                    /* 低功耗模块(lp:low power) */
    uint8_t reserve[256 - 83];                                        /* 保留 */
};

struct _function_enable{
    uint8_t local_charge;          /* 本地启动 */
    uint8_t insulation_detect;     /* 绝缘检测 */
    uint8_t vin_charge;            /* VIN */
    uint8_t parallel_charge;       /* 并充 */
    uint8_t reserve0;              /*  */
    uint8_t bcs;                   /* BCS功能 */
    uint8_t bsm;                   /* BSM功能 */
    uint8_t acrelay_out;           /* 交流接触器 */
    uint8_t elock_out;             /* 电子锁 */
    uint8_t fan_out;               /* 风扇 */
    uint8_t emergency_stop;        /* 急停 */
    uint8_t gate_in;               /* 门禁 */
    uint8_t acrelay_in;            /* 交流继电器 */
    uint8_t dcrelay_in;            /* 直流继电器 */
    uint8_t fan_in;                /* 风扇 */
    uint8_t elock_in;              /* 电子锁反馈 */
    uint8_t temp_protect;          /* 温度保护 */
    uint8_t rfid_card_reader;      /* 读卡器 */
    uint8_t auxpower_24V;          /* 24V辅源 */
    uint8_t parallel_relay;        /* 并联 */
    uint8_t module_slience;        /* 模块静音 */
    uint8_t offline_billing;       /* 离线计费 */
    uint8_t local_stop;            /* 本地停止 */
    uint8_t plug_charge;           /* 即插即充 */
    uint8_t password_start;        /* 密码启动 */
    uint8_t offline_card;          /* 离线卡 */
    uint8_t protectlight_in;       /* 防雷器 */
    uint8_t gunsite_in;            /* 枪座 */
    uint8_t circuit_breaker_in;    /* 断路器 */
    uint8_t flood_in;              /* 水浸 */
    uint8_t smoke_in;              /* 烟感 */
    uint8_t pour_in;               /* 倾倒 */
    uint8_t liquid_in;             /* 液冷 */
    uint8_t fuse_in;               /* 熔断器 */
    uint8_t mode_select;           /* 模式选择功能启用 */
    uint8_t current_mode[2];       /* 已选择的模式 */
    uint8_t mode_v2g;              /* 是否启用V2G */
    uint8_t reserve[77];
};

struct _state_reversal{
    uint8_t emergency_stop;        /* 状态取反：急停 */
    uint8_t gate;                  /* 状态取反：门禁 */
    uint8_t acrelay;               /* 状态取反：交流继电器 */
    uint8_t dcrelay;               /* 状态取反：直流继电器 */
    uint8_t parallel_relay;        /* 状态取反：直流继电器 */
    uint8_t fan;                   /* 状态取反：风扇 */
    uint8_t elock;                 /* 状态取反：电子锁反馈 */
    uint8_t protectlight;          /* 防雷器 */
    uint8_t gunsite;               /* 枪座 */
    uint8_t circuit_breaker;       /* 断路器 */
    uint8_t flood;                 /* 水浸 */
    uint8_t smoke;                 /* 烟感 */
    uint8_t pour;                  /* 倾倒 */
    uint8_t liquid;                /* 液冷 */
    uint8_t fuse;                  /* 熔断器 */
    uint8_t reserve[56];
};

struct _target_plat{
    uint32_t storage_init_flag;    /* 存储初始化标志 */
    uint8_t verify_result;         /* 数据校验结果 */
    uint8_t reserve[CP_INFO_TARGET_PLAT_LEN_MAX - 0x05];
};

struct _monitor_plat{
    uint32_t storage_init_flag;    /* 存储初始化标志 */
    uint8_t verify_result;         /* 数据校验结果 */
    uint8_t reserve[CP_INFO_MONITOR_PLAT_LEN_MAX - 0x05];
};

struct module_info{
    uint8_t module_model;
    uint8_t module_group_num;
    uint8_t module_num_singlegroup[4];
};

struct chargepile_config_info{
    struct _network network;
    struct _encrypt encrypt;
    struct _pile_info pile_info;
    struct _config_para config_para;
    struct _config_info config_info;
    struct _function_enable function_enable;
    struct _state_reversal state_reversal;
    struct _target_plat target_plat;
    struct _monitor_plat monitor_plat;
#ifdef CP_USING_OFFLINE_BILLING
    struct sys_billing_rule billing_rule;
#else
    uint8_t ob_reserve[1358];    /** 不使用离线计费时也要占用对应大小的配置内存 */
#endif /* CP_USING_OFFLINE_BILLING */
    uint8_t reserve[1024];
    uint32_t crc;
};

#pragma pack()

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
static struct system_config_tp_additional s_system_config_tp_additional;
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */

#ifdef SYS_DESIGNATE_REGION
#define SYS_DEF_TCMRAM CFG_DEF_TCMRAM
#define SYS_DEF_SRAM0 CFG_DEF_SRAM0
#define SYS_DEF_SRAM1 CFG_DEF_SRAM1
#define SYS_DEF_SRAM2 CFG_DEF_SRAM2
#else
#define SYS_DEF_TCMRAM
#define SYS_DEF_SRAM0
#define SYS_DEF_SRAM1
#define SYS_DEF_SRAM2
#endif /* SYS_DESIGNATE_REGION */

SYS_DEF_SRAM2 static struct chargepile_config_info s_chargepile_config_info;
SYS_DEF_SRAM1 static uint8_t s_storage_chip_entry = 0x00;
SYS_DEF_SRAM1 static uint32_t s_config_info_address = SYSTEM_CONFIG_MAIN_ADDRESS;
SYS_DEF_SRAM1 static uint32_t s_system_power_max = 0x00;
SYS_DEF_SRAM1 static struct module_info s_module_info;

/** 初始化排列必须要按照  enum config_name 枚举一致并按顺序连续排列*/
APP_DEF_SRAM2 static struct config_item s_config_item_set[CONFIG_ITEM_SIZE] =
{
#ifndef SYS_DESIGNATE_REGION
        {CONFIG_ITEM_PILE_NUMBER,                                                             /* 配置项：桩号 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.pile_info.pile_number) - 1),
        (uint8_t*)s_chargepile_config_info.pile_info.pile_number,
        &s_chargepile_config_info.pile_info.pile_num_len},

        {CONFIG_ITEM_IP_DOMAIN,                                                               /* 配置项：域名/IP */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.network.domain) - 1),
        (uint8_t*)s_chargepile_config_info.network.domain,
        &s_chargepile_config_info.network.domain_len},

        {CONFIG_ITEM_PORT,                                                                    /* 配置项：端口号 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.network.port)),
        (uint8_t*)&s_chargepile_config_info.network.port,
        NULL},

        {CONFIG_ITEM_MODULE_MODEL,                                                            /* 配置项：模块型号 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_model)),
        (uint8_t*)&s_chargepile_config_info.config_info.module_model,
        NULL},

        {CONFIG_ITEM_MODULE_GROUP_NUM,                                                        /* 配置项：模块组数 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_group_num)),
        (uint8_t*)&s_chargepile_config_info.config_info.module_group_num,
        NULL},

        {CONFIG_ITEM_MODULE_NUM_GROUP_1,                                                      /* 配置项：组1模块数 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[0])),
        (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[0],
        NULL},

        {CONFIG_ITEM_MODULE_NUM_GROUP_2,                                                      /* 配置项：组2模块数 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[1])),
        (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[1],
        NULL},

        {CONFIG_ITEM_MODULE_NUM_GROUP_3,                                                      /* 配置项：组3模块数 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[2])),
        (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[2],
        NULL},

        {CONFIG_ITEM_MODULE_NUM_GROUP_4,                                                      /* 配置项：组4模块数 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[3])),
        (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[3],
        NULL},

        {OCONFIG_ITEM_GUN_NUMBER,                                                            /* 配置项：枪个数 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.gun_num)),
        (uint8_t*)&s_chargepile_config_info.config_info.gun_num,
        NULL},

        {CONFIG_ITEM_SUPORT_LOCAL,                                                           /* 配置项：本地启动 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.local_charge)),
        (uint8_t*)&s_chargepile_config_info.function_enable.local_charge,
        NULL},

        {CONFIG_ITEM_SUPORT_LOCAL_STOP,                                                      /* 配置项：本地停止 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.local_stop)),
        (uint8_t*)&s_chargepile_config_info.function_enable.local_stop,
        NULL},

        {CONFIG_ITEM_SUPORT_INSULATION,                                                          /* 配置项：绝缘检测*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.insulation_detect)),
        (uint8_t*)&s_chargepile_config_info.function_enable.insulation_detect,
        NULL},

        {CONFIG_ITEM_SUPORT_VIN,                                                                /* 配置项：VIN支持*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.vin_charge)),
        (uint8_t*)&s_chargepile_config_info.function_enable.vin_charge,
        NULL},

        {CONFIG_ITEM_SUPORT_PARALLEL,                                                           /* 配置项：并充支持*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.parallel_charge)),
        (uint8_t*)&s_chargepile_config_info.function_enable.parallel_charge,
        NULL},

        {CONFIG_ITEM_SUPORT_PARALLELRELAY,                                                      /* 配置项：并联支持*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.parallel_relay)),
        (uint8_t*)&s_chargepile_config_info.function_enable.parallel_relay,
        NULL},

        {CONFIG_ITEM_SUPORT_PLUGCHARGE,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.plug_charge)),     /*配置项：即插即充*/
        (uint8_t*)&s_chargepile_config_info.function_enable.plug_charge,
        NULL},

        {CONFIG_ITEM_SUPORT_CARD,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.rfid_card_reader)),       /*配置项：读卡器*/
        (uint8_t*)&s_chargepile_config_info.function_enable.rfid_card_reader,
        NULL},

        {CONFIG_ITEM_SUPORT_MODULE_SLIENCE,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.module_slience)),       /*配置项：模块静音*/
        (uint8_t*)&s_chargepile_config_info.function_enable.module_slience,
        NULL},

        {CONFIG_ITEM_SUPORT_PASSWORD_START,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.password_start)),       /*配置项：密码启动*/
        (uint8_t*)&s_chargepile_config_info.function_enable.password_start,
        NULL},

        {CONFIG_ITEM_SUPORT_OFFLINE_CARD,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.offline_card)),       /*配置项：离线卡支持*/
        (uint8_t*)&s_chargepile_config_info.function_enable.offline_card,
        NULL},

        {CONFIG_ITEM_SUPORT_MODE_SELECT,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.mode_select)),       /*配置项：模式选择支持*/
        (uint8_t*)&s_chargepile_config_info.function_enable.mode_select,
        NULL},

        {CONFIG_ITEM_CURRENT_MODE_A,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.current_mode[0x00])),      /*配置项：当前模式*/
        (uint8_t*)&s_chargepile_config_info.function_enable.current_mode[0x00],
        NULL},

        {CONFIG_ITEM_CURRENT_MODE_B,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.current_mode[0x01])),      /*配置项：当前模式*/
        (uint8_t*)&s_chargepile_config_info.function_enable.current_mode[0x01],
        NULL},

        {CONFIG_ITEM_CARD_TYPE,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.card_type)),       /*配置项：读卡器密钥*/
        (uint8_t*)&s_chargepile_config_info.config_info.card_type,
        NULL},

        {CONFIG_ITEM_CC14V_MAX,                                                     /* 配置项：CC1 4V电压上限 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_4_max)),
        (uint8_t*)&s_chargepile_config_info.config_para.cc1_4_max,
        NULL},

        {CONFIG_ITEM_CC14V_MIN,                                                     /* 配置项：CC1 4V电压下限 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_4_min)),
        (uint8_t*)&s_chargepile_config_info.config_para.cc1_4_min,
        NULL},

        {CONFIG_ITEM_CC16V_MAX,                                                     /* 配置项：CC1 6V电压上限 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_6_max)),
        (uint8_t*)&s_chargepile_config_info.config_para.cc1_6_max,
        NULL},

        {CONFIG_ITEM_CC16V_MIN,                                                     /* 配置项：CC1 6V电压下限 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_6_min)),
        (uint8_t*)&s_chargepile_config_info.config_para.cc1_6_min,
        NULL},

        {CONFIG_ITEM_CC112V_MAX,                                                     /* 配置项：CC1 12V电压上限 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_12_max)),
        (uint8_t*)&s_chargepile_config_info.config_para.cc1_12_max,
        NULL},

        {CONFIG_ITEM_CC112V_MIN,                                                     /* 配置项：CC1 12V电压下限 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_12_min)),
        (uint8_t*)&s_chargepile_config_info.config_para.cc1_12_min,
        NULL},

        {CONFIG_ITEM_SUPORT_BSM,                                                        /* 配置项：BSM功能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.bsm)),
        (uint8_t*)&s_chargepile_config_info.function_enable.bsm,
        NULL},

        {CONFIG_ITEM_SUPORT_BCS,                                                        /* 配置项：BCS功能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.bcs)),
        (uint8_t*)&s_chargepile_config_info.function_enable.bcs,
        NULL},

        {CONFIG_ITEM_SUPORT_AUXPOWER24V,                                                /* 配置项：24V辅源 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.auxpower_24V)),
        (uint8_t*)&s_chargepile_config_info.function_enable.auxpower_24V,
        NULL},

        {CONFIG_ITEM_SUPORT_OFFLINE_BILLING,                                            /* 配置项：离线计费 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.offline_billing)),
        (uint8_t*)&s_chargepile_config_info.function_enable.offline_billing,
        NULL},

        {CONFIG_ITEM_INPUT_OVERVOL,                                                     /* 配置项：输入过压 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.input_overvol)),
        (uint8_t*)&s_chargepile_config_info.config_para.input_overvol,
        NULL},

        {CONFIG_ITEM_INPUT_UNDERVOL,                                                    /* 配置项：输入欠压 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.input_undervol)),
        (uint8_t*)&s_chargepile_config_info.config_para.input_undervol,
        NULL},

        {CONFIG_ITEM_OUTPUT_OVERVOL,                                                    /* 配置项：输出过压*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.output_overvol)),
        (uint8_t*)&s_chargepile_config_info.config_para.output_overvol,
        NULL},

        {CONFIG_ITEM_OUTPUT_UNDERVOL,                                                   /* 配置项：输出欠压 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.output_undervol)),
        (uint8_t*)&s_chargepile_config_info.config_para.output_undervol,
        NULL},

        {CONFIG_ITEM_OUTPUT_OVERCUR,                                                    /* 配置项：输出过流 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.output_overcur)),
        (uint8_t*)&s_chargepile_config_info.config_para.output_overcur,
        NULL},

        {CONFIG_ITEM_SOC_STOP,                                                  /* 配置项：停止SOC */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.soc_stop)),
        (uint8_t*)&s_chargepile_config_info.config_para.soc_stop,
        NULL},

        {CONFIG_ITEM_OVERTEMP_WARN,                                                 /* 配置项：过温告警 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_alarm)),
        (uint8_t*)&s_chargepile_config_info.config_para.overtemp_alarm,
        NULL},

        {CONFIG_ITEM_OVERTEMP_STOP,                                                 /* 配置项：过温停充*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_stop)),
        (uint8_t*)&s_chargepile_config_info.config_para.overtemp_stop,
        NULL},

        {CONFIG_ITEM_OVERTEMP_RECOVER,                                              /* 配置项：过温恢复*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_recovery)),
        (uint8_t*)&s_chargepile_config_info.config_para.overtemp_recovery,
        NULL},

        {CONFIG_ITEM_OVERTEMP_SETCUR,                                               /* 配置项：过温限流*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_limitcur)),
        (uint8_t*)&s_chargepile_config_info.config_para.overtemp_limitcur,
        NULL},

        {CONFIG_ITEM_ELOSS_PROPORTION,                                              /* 配置项：电损比*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.eloss_proportion)),
        (uint8_t*)&s_chargepile_config_info.config_para.eloss_proportion,
        NULL},

        {CONFIG_ITEM_MODE_PARAMETER_A,                                              /* 配置项：模式参数*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.mode_parameter[0x00])),
        (uint8_t*)&s_chargepile_config_info.config_para.mode_parameter[0x00],
        NULL},

        {CONFIG_ITEM_MODE_PARAMETER_B,                                              /* 配置项：模式参数*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.mode_parameter[0x01])),
        (uint8_t*)&s_chargepile_config_info.config_para.mode_parameter[0x01],
        NULL},

        {CONFIG_ITEM_OUTEN_AC,                                                      /* 配置项：交流接触器输出*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.acrelay_out)),
        (uint8_t*)&s_chargepile_config_info.function_enable.acrelay_out,
        NULL},

        {CONFIG_ITEM_OUTEN_ELOCK,                                                   /* 配置项：电子锁输出*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.elock_out)),
        (uint8_t*)&s_chargepile_config_info.function_enable.elock_out,
        NULL},

        {CONFIG_ITEM_OUTEN_FAN,                                                 /* 配置项：风扇输出*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.fan_out)),
        (uint8_t*)&s_chargepile_config_info.function_enable.fan_out,
        NULL},

        {CONFIG_ITEM_INEN_SCRAM,                                                /* 配置项：急停输入*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.emergency_stop)),
        (uint8_t*)&s_chargepile_config_info.function_enable.emergency_stop,
        NULL},

        {CONFIG_ITEM_INEN_GATE,                                             /* 配置项: 门禁输入*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.gate_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.gate_in,
        NULL},

        {CONFIG_ITEM_INEN_ACRELAY,                                          /* 配置项: 交流继电器输入*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.acrelay_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.acrelay_in,
        NULL},

        {CONFIG_ITEM_INEN_DCRELAY,                                          /* 配置项: 直流继电器输入*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.dcrelay_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.dcrelay_in,
        NULL},

        {CONFIG_ITEM_INEN_FAN,                                              /* 配置项: 风扇输入*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.fan_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.fan_in,
        NULL},

        {CONFIG_ITEM_INEN_ELOCK,                                            /* 配置项: 电磁锁输入*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.elock_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.elock_in,
        NULL},

        {CONFIG_ITEM_INEN_TEMPPRO,                                            /* 配置项: 温度保护*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.temp_protect)),
        (uint8_t*)&s_chargepile_config_info.function_enable.temp_protect,
        NULL},

        {CONFIG_ITEM_INEN_PROTECT_LIGHT,                                            /* 配置项: 防雷器检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.protectlight_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.protectlight_in,
        NULL},

        {CONFIG_ITEM_INEN_GUNSITE,                                                    /* 配置项: 枪座检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.gunsite_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.gunsite_in,
        NULL},

        {CONFIG_ITEM_INEN_CIRCUIT_BREAKER,                                            /* 配置项: 断路器检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.circuit_breaker_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.circuit_breaker_in,
        NULL},

        {CONFIG_ITEM_INEN_FLOOD,                                                      /* 配置项: 水浸检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.flood_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.flood_in,
        NULL},

        {CONFIG_ITEM_INEN_SMOKE,                                                      /* 配置项: 烟感检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.smoke_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.smoke_in,
        NULL},

        {CONFIG_ITEM_INEN_POUR,                                                      /* 配置项: 倾倒检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.pour_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.pour_in,
        NULL},

        {CONFIG_ITEM_INEN_LIQUID,                                                      /* 配置项: 液冷检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.liquid_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.liquid_in,
        NULL},

        {CONFIG_ITEM_INEN_FUSE,                                                      /* 配置项: 熔断器检测使能 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.fuse_in)),
        (uint8_t*)&s_chargepile_config_info.function_enable.fuse_in,
        NULL},

        {CONFIG_ITEM_INNEG_SCRAM,                                           /* 配置项: 急停输入取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.emergency_stop)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.emergency_stop,
        NULL},

        {CONFIG_ITEM_INNEG_GATE,                                            /* 配置项: 门禁输入取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.gate)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.gate,
        NULL},

        {CONFIG_ITEM_INNEG_ACRELAY,                                         /* 配置项: 交流继电器输入取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.acrelay)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.acrelay,
        NULL},

        {CONFIG_ITEM_INNEG_DCRELAY,                                         /* 配置项: 直流继电器输入取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.dcrelay)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.dcrelay,
        NULL},

        {CONFIG_ITEM_INNEG_FAN,                                         /* 配置项: 风扇输入取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.fan)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.fan,
        NULL},

        {CONFIG_ITEM_INNEG_ELOCK,                                       /* 配置项: 电子锁反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.elock)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.elock,
        NULL},

        {CONFIG_ITEM_INNEG_PROTECT_LIGHT,                                       /* 配置项: 防雷器反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.protectlight)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.protectlight,
        NULL},

        {CONFIG_ITEM_INNEG_GUNSITE,                                       /* 配置项: 枪座反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.gunsite)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.gunsite,
        NULL},

        {CONFIG_ITEM_INNEG_CIRCUIT_BREAKER,                                       /* 配置项: 断路器反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.circuit_breaker)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.circuit_breaker,
        NULL},

        {CONFIG_ITEM_INNEG_FLOOD,                                       /* 配置项: 水浸反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.flood)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.flood,
        NULL},

        {CONFIG_ITEM_INNEG_SMOKE,                                       /* 配置项:烟感反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.smoke)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.smoke,
        NULL},

        {CONFIG_ITEM_INNEG_POUR,                                       /* 配置项: 倾倒反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.pour)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.pour,
        NULL},

        {CONFIG_ITEM_INNEG_LIQUID,                                       /* 配置项: 液冷反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.liquid)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.liquid,
        NULL},

        {CONFIG_ITEM_INNEG_FUSE,                                       /* 配置项: 熔断器反馈取反*/
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.fuse)),
        (uint8_t*)&s_chargepile_config_info.state_reversal.fuse,
        NULL},

        {CONFIG_ITEM_QRCODE_PRE,                                                              /* 配置项：二维码前缀 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.qrcode_prefix) - 1),
        (uint8_t*)s_chargepile_config_info.config_info.qrcode_prefix,
        &s_chargepile_config_info.config_info.prefix_length},

        {CONFIG_ITEM_QRCODE_SUF,                                                              /* 配置项：二维码后缀 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.qrcode_suffix) - 1),
        (uint8_t*)s_chargepile_config_info.config_info.qrcode_suffix,
        &s_chargepile_config_info.config_info.suffix_length},

        {CONFIG_ITEM_METER_NOA,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.meter_address[0]) - 1),
        (uint8_t*)s_chargepile_config_info.config_info.meter_address[0],
        NULL},

        {CONFIG_ITEM_METER_NOB,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.meter_address[1]) - 1),
        (uint8_t*)s_chargepile_config_info.config_info.meter_address[1],
        NULL},

        {CONFIG_ITEM_METER_MODEL,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.meter_model)),
        (uint8_t*)&s_chargepile_config_info.config_info.meter_model,
        NULL},

        {CONFIG_ITEM_METER_CHECK_WAY,                                                           /* 电表校验方式 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.ammeter_check_way)),
        (uint8_t*)&s_chargepile_config_info.config_info.ammeter_check_way,
        NULL},

        {CONFIG_ITEM_METER_BAUDRATE,                                                            /* 电表波特率 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.ammeter_baudrate)),
        (uint8_t*)&s_chargepile_config_info.config_info.ammeter_baudrate,
        NULL},

        {CONFIG_ITEM_RATED_OUTPUT_VOLTAGE,                                                    /* 额定输出电压 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.module_rated_outvolt)),
        (uint8_t*)&s_chargepile_config_info.config_para.module_rated_outvolt,
        NULL},

        {CONFIG_ITEM_MAX_OUTPUT_VOLTAGE,                                                      /* 最大输出电压 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_max_outvolt)),
        (uint8_t*)&s_chargepile_config_info.config_para.pile_max_outvolt,
        NULL},

        {CONFIG_ITEM_MIN_OUTPUT_VOLTAGE,                                                      /* 最小输出电压 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_min_outvolt)),
        (uint8_t*)&s_chargepile_config_info.config_para.pile_min_outvolt,
        NULL},

        {CONFIG_ITEM_RATED_LIMIT_CURRENT,                                                     /* 额定限电流 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.module_rated_limit_curr)),
        (uint8_t*)&s_chargepile_config_info.config_para.module_rated_limit_curr,
        NULL},

        {CONFIG_ITEM_MAX_LIMIT_CURRENT,                                                       /* 最大限电流 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_max_limit_curr)),
        (uint8_t*)&s_chargepile_config_info.config_para.pile_max_limit_curr,
        NULL},

        {CONFIG_ITEM_MIN_LIMIT_CURRENT,                                                       /* 最小限电流 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_min_limit_curr)),
        (uint8_t*)&s_chargepile_config_info.config_para.pile_min_limit_curr,
        NULL},

        {CONFIG_ITEM_SYSTEM_POWER_TOTAL,                                                       /* 总功率 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.system_power_total)),
        (uint8_t*)&s_chargepile_config_info.config_info.system_power_total,
        NULL},

        {CONFIG_ITEM_ALLOCATION_WAY,                                                       /* 分配方式 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.power_alloc_way)),
        (uint8_t*)&s_chargepile_config_info.config_info.power_alloc_way,
        NULL},

        {CONFIG_ITEM_DEVICE_TYPE,                                                        /* 设备类型 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.system_function)),
        (uint8_t*)&s_chargepile_config_info.config_info.system_function,
        NULL},


        {CONFIG_ITEM_LP_MODULE,                                                        /* 低功耗模块 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.lp_consumption_module)),
        (uint8_t*)&s_chargepile_config_info.config_info.lp_consumption_module,
        NULL},

        {CONFIG_ITEM_GUNVOLT_LIMIT,                                                       /* 枪头电压限值 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.gunvolt_limit)),
        (uint8_t*)&s_chargepile_config_info.config_info.gunvolt_limit,
        NULL},

        {CONFIG_ITEM_VIN_WHITELIST,                                                       /* VIN 码白名单 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.vin_whitelist)),
        (uint8_t*)&s_chargepile_config_info.config_info.vin_whitelist,
        NULL},

        {CONFIG_ITEM_CARD_WHITELIST,                                                       /* 卡号白名单 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number)),
        (uint8_t*)&s_chargepile_config_info.config_info.card_whitelist.card_number,
        NULL},

        {CONFIG_ITEM_SCREEN_PASSWORD,                                                       /* 屏幕密码 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.screen_password)),
        (uint8_t*)&s_chargepile_config_info.config_info.screen_password,
        NULL},

        {CONFIG_ITEM_HELP_PHONE,                                                           /* 帮助电话 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.pile_info.help_number)),
        (uint8_t*)&s_chargepile_config_info.pile_info.help_number,
        &s_chargepile_config_info.pile_info.help_number_len},

        {CONFIG_ITEM_USER_IDENTITY,                                                        /* 用户识别码 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.pile_info.user_identity)),
        (uint8_t*)&s_chargepile_config_info.pile_info.user_identity,
        NULL},

        {CONFIG_ITEM_REGISTER_CODE,                                                        /* 注册码 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.register_code)),
        (uint8_t*)&s_chargepile_config_info.config_info.register_code,
        NULL},

        {CONFIG_ITEM_NET_TYPE,                                                             /* 联网方式 */
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.network.nettype)),
        (uint8_t*)&s_chargepile_config_info.network.nettype,
        NULL},

        {CONFIG_ITEM_TEMINAL_ADDRA,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.teminal_addrA)),
        (uint8_t*)&s_chargepile_config_info.config_info.teminal_addrA,
        NULL},

        {CONFIG_ITEM_TEMINAL_ADDRB,
        (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.teminal_addrB)),
        (uint8_t*)&s_chargepile_config_info.config_info.teminal_addrB,
        NULL},


#ifdef CP_USING_OFFLINE_BILLING
        {CONFIG_ITEM_BILLING_RULE,                                                              /* 计费规则数据：为倒数第三项 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.billing_rule)),
        (uint8_t*)&s_chargepile_config_info.billing_rule,
        NULL},
#endif /* CP_USING_OFFLINE_BILLING */

        {CONFIG_ITEM_TARGET_PLATFORM,                                                           /* 目标平台数据：为倒数第二项 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.target_plat)),
        (uint8_t*)&s_chargepile_config_info.target_plat,
        NULL},

        {CONFIG_ITEM_MONITOR_PLATFORM,                                                          /* 监控平台数据：为倒数第一项 */
        (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.monitor_plat)),
        (uint8_t*)&s_chargepile_config_info.monitor_plat,
        NULL},

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
        {CONFIG_ITEM_TARGET_PLATFORM_ADDITIONAL,                                                /* 目标平台数据：为倒数第一项 */
        (1 <<(32 - 4))| (sizeof(s_system_config_tp_additional)),
        (uint8_t*)&s_system_config_tp_additional,
        NULL},
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */

#endif /* SYS_DESIGNATE_REGION */
};

/******************************************************************************/

#ifdef SYS_DESIGNATE_REGION
/*************************************
 * 函数名       sys_config_item_init
 * 功能           初始化配置项
 * 参数
 * 返回
 ************************************/
/** 初始化排列必须要按照  enum config_name 枚举一致并按顺序连续排列*/
static void sys_config_item_init(uint8_t item, uint32_t user, uint8_t* config_ptr, void* user_data)
{
    if(item >= CONFIG_ITEM_SIZE){
        return;
    }

    s_config_item_set[item].name = item;
    s_config_item_set[item].user_section = user;
    s_config_item_set[item].config_index = config_ptr;
    s_config_item_set[item].user_data = user_data;
}
/*************************************
 * 函数名       app_chargeplie_config_info_init
 * 功能           桩配置信息、变量初始化
 * 参数
 * 返回
 ************************************/
void sys_chargeplie_config_info_init(void)
{
    s_storage_chip_entry = 0x00;
    s_config_info_address = SYSTEM_CONFIG_MAIN_ADDRESS;
    s_system_power_max = 0x00;

    memset(&s_chargepile_config_info, 0xFF, sizeof(s_chargepile_config_info));
    memset(&s_module_info, 0x00, sizeof(s_module_info));
    /** 桩号 */
    sys_config_item_init(CONFIG_ITEM_PILE_NUMBER, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.pile_info.pile_number) - 1), \
            (uint8_t*)s_chargepile_config_info.pile_info.pile_number, &s_chargepile_config_info.pile_info.pile_num_len);
    /** 域名 */
    sys_config_item_init(CONFIG_ITEM_IP_DOMAIN, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.network.domain) - 1), \
            (uint8_t*)s_chargepile_config_info.network.domain, &s_chargepile_config_info.network.domain_len);
    /** 端口号 */
    sys_config_item_init(CONFIG_ITEM_PORT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.network.port)), \
            (uint8_t*)&s_chargepile_config_info.network.port, NULL);
    /** 模块型号 */
    sys_config_item_init(CONFIG_ITEM_MODULE_MODEL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_model)), \
            (uint8_t*)&s_chargepile_config_info.config_info.module_model, NULL);
    /** 模块组数 */
    sys_config_item_init(CONFIG_ITEM_MODULE_GROUP_NUM, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_group_num)), \
            (uint8_t*)&s_chargepile_config_info.config_info.module_group_num, NULL);
    /** 组1模块数 */
    sys_config_item_init(CONFIG_ITEM_MODULE_NUM_GROUP_1, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[0])), \
            (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[0], NULL);
    /** 组2模块数 */
    sys_config_item_init(CONFIG_ITEM_MODULE_NUM_GROUP_2, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[1])), \
            (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[1], NULL);
    /** 组3模块数 */
    sys_config_item_init(CONFIG_ITEM_MODULE_NUM_GROUP_3, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[2])), \
            (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[2], NULL);
    /** 组4模块数 */
    sys_config_item_init(CONFIG_ITEM_MODULE_NUM_GROUP_4, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.module_num_singlegroup[3])), \
            (uint8_t*)&s_chargepile_config_info.config_info.module_num_singlegroup[3], NULL);
    /** 枪数量 */
    sys_config_item_init(OCONFIG_ITEM_GUN_NUMBER, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.gun_num)), \
            (uint8_t*)&s_chargepile_config_info.config_info.gun_num, NULL);
    /** 启用本地启动功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_LOCAL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.local_charge)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.local_charge, NULL);
    /** 启用本地停止功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_LOCAL_STOP, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.local_stop)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.local_stop, NULL);
    /** 启用绝缘检测功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_INSULATION, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.insulation_detect)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.insulation_detect, NULL);
    /** 启用VIN码启动功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_VIN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.vin_charge)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.vin_charge, NULL);
    /** 启用并充功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_PARALLEL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.parallel_charge)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.parallel_charge, NULL);
    /** 启用并联功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_PARALLELRELAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.parallel_relay)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.parallel_relay, NULL);
    /** 启用即插即充功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_PLUGCHARGE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.plug_charge)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.plug_charge, NULL);
    /** 启用刷卡功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_CARD, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.rfid_card_reader)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.rfid_card_reader, NULL);
    /** 启用模块静音功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_MODULE_SLIENCE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.module_slience)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.module_slience, NULL);
    /** 启用密码启动功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_PASSWORD_START, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.password_start)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.password_start, NULL);
    /** 启用离线卡功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_OFFLINE_CARD, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.offline_card)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.offline_card, NULL);
    /** 启用模式选择功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_MODE_SELECT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.mode_select)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.mode_select, NULL);
    /** 当前模式-A */
    sys_config_item_init(CONFIG_ITEM_CURRENT_MODE_A, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.current_mode[0x00])), \
            (uint8_t*)&s_chargepile_config_info.function_enable.current_mode[0x00], NULL);
    /** 当前模式-B */
    sys_config_item_init(CONFIG_ITEM_CURRENT_MODE_B, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.current_mode[0x01])), \
            (uint8_t*)&s_chargepile_config_info.function_enable.current_mode[0x01], NULL);
    /** 卡类型 */
    sys_config_item_init(CONFIG_ITEM_CARD_TYPE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.card_type)), \
            (uint8_t*)&s_chargepile_config_info.config_info.card_type, NULL);
    /** CC1 4V 最大值 */
    sys_config_item_init(CONFIG_ITEM_CC14V_MAX, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_4_max)), \
            (uint8_t*)&s_chargepile_config_info.config_para.cc1_4_max, NULL);
    /** CC1 4V 最小值 */
    sys_config_item_init(CONFIG_ITEM_CC14V_MIN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_4_min)), \
            (uint8_t*)&s_chargepile_config_info.config_para.cc1_4_min, NULL);
    /** CC1 6V 最大值  */
    sys_config_item_init(CONFIG_ITEM_CC16V_MAX, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_6_max)), \
            (uint8_t*)&s_chargepile_config_info.config_para.cc1_6_max, NULL);
    /** CC1 6V 最小值 */
    sys_config_item_init(CONFIG_ITEM_CC16V_MIN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_6_min)), \
            (uint8_t*)&s_chargepile_config_info.config_para.cc1_6_min, NULL);
    /** CC1 12V 最大值 */
    sys_config_item_init(CONFIG_ITEM_CC112V_MAX, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_12_max)), \
            (uint8_t*)&s_chargepile_config_info.config_para.cc1_12_max, NULL);
    /** CC1 12V 最小值 */
    sys_config_item_init(CONFIG_ITEM_CC112V_MIN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.cc1_12_min)), \
            (uint8_t*)&s_chargepile_config_info.config_para.cc1_12_min, NULL);
    /** 启用BSM功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_BSM, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.bsm)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.bsm, NULL);
    /** 启用BCS功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_BCS, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.bcs)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.bcs, NULL);
    /** 启用24V辅源功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_AUXPOWER24V, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.auxpower_24V)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.auxpower_24V, NULL);
    /** 启用离线计费功能 */
    sys_config_item_init(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.offline_billing)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.offline_billing, NULL);
    /** 输入过压值 */
    sys_config_item_init(CONFIG_ITEM_INPUT_OVERVOL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.input_overvol)), \
            (uint8_t*)&s_chargepile_config_info.config_para.input_overvol, NULL);
    /** 输入欠压值 */
    sys_config_item_init(CONFIG_ITEM_INPUT_UNDERVOL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.input_undervol)), \
            (uint8_t*)&s_chargepile_config_info.config_para.input_undervol, NULL);
    /** 输出过压值 */
    sys_config_item_init(CONFIG_ITEM_OUTPUT_OVERVOL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.output_overvol)), \
            (uint8_t*)&s_chargepile_config_info.config_para.output_overvol, NULL);
    /** 输出欠压值 */
    sys_config_item_init(CONFIG_ITEM_OUTPUT_UNDERVOL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.output_undervol)), \
            (uint8_t*)&s_chargepile_config_info.config_para.output_undervol, NULL);
    /** 输出过流值 */
    sys_config_item_init(CONFIG_ITEM_OUTPUT_OVERCUR, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.output_overcur)), \
            (uint8_t*)&s_chargepile_config_info.config_para.output_overcur, NULL);
    /** 停充SOC值 */
    sys_config_item_init(CONFIG_ITEM_SOC_STOP, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.soc_stop)), \
            (uint8_t*)&s_chargepile_config_info.config_para.soc_stop, NULL);
    /** 过温告警值 */
    sys_config_item_init(CONFIG_ITEM_OVERTEMP_WARN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_alarm)), \
            (uint8_t*)&s_chargepile_config_info.config_para.overtemp_alarm, NULL);
    /** 过温停充值 */
    sys_config_item_init(CONFIG_ITEM_OVERTEMP_STOP, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_stop)), \
            (uint8_t*)&s_chargepile_config_info.config_para.overtemp_stop, NULL);
    /** 过温恢复值 */
    sys_config_item_init(CONFIG_ITEM_OVERTEMP_RECOVER, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_recovery)), \
            (uint8_t*)&s_chargepile_config_info.config_para.overtemp_recovery, NULL);
    /** 过温限流值 */
    sys_config_item_init(CONFIG_ITEM_OVERTEMP_SETCUR, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.overtemp_limitcur)), \
            (uint8_t*)&s_chargepile_config_info.config_para.overtemp_limitcur, NULL);
    /** 电损比 */
    sys_config_item_init(CONFIG_ITEM_ELOSS_PROPORTION, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.eloss_proportion)), \
            (uint8_t*)&s_chargepile_config_info.config_para.eloss_proportion, NULL);
    /** 模式参数-A */
    sys_config_item_init(CONFIG_ITEM_MODE_PARAMETER_A, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.mode_parameter[0x00])), \
            (uint8_t*)&s_chargepile_config_info.config_para.mode_parameter[0x00], NULL);
    /** 模式参数-B */
    sys_config_item_init(CONFIG_ITEM_MODE_PARAMETER_B, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.mode_parameter[0x01])), \
            (uint8_t*)&s_chargepile_config_info.config_para.mode_parameter[0x01], NULL);
    /** 启用交流接触器 */
    sys_config_item_init(CONFIG_ITEM_OUTEN_AC, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.acrelay_out)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.acrelay_out, NULL);
    /** 启用电子锁 */
    sys_config_item_init(CONFIG_ITEM_OUTEN_ELOCK, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.elock_out)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.elock_out, NULL);
    /**启用风扇 */
    sys_config_item_init(CONFIG_ITEM_OUTEN_FAN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.fan_out)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.fan_out, NULL);
    /** 急停反馈使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_SCRAM, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.emergency_stop)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.emergency_stop, NULL);
    /** 门禁反馈使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_GATE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.gate_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.gate_in, NULL);
    /** 交流接触器反馈使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_ACRELAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.acrelay_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.acrelay_in, NULL);
    /** 直流继电器反馈使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_DCRELAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.dcrelay_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.dcrelay_in, NULL);
    /** 风扇反馈使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_FAN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.fan_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.fan_in, NULL);
    /** 电子锁反馈使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_ELOCK, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.elock_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.elock_in, NULL);
    /** 温度保护使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_TEMPPRO, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.temp_protect)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.temp_protect, NULL);
    /** 防雷器检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_PROTECT_LIGHT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.protectlight_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.protectlight_in, NULL);
    /** 枪座检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_GUNSITE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.gunsite_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.gunsite_in, NULL);
    /** 断路器检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_CIRCUIT_BREAKER, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.circuit_breaker_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.circuit_breaker_in, NULL);
    /** 水浸检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_FLOOD, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.flood_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.flood_in, NULL);
    /** 烟感检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_SMOKE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.smoke_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.smoke_in, NULL);
    /** 倾倒检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_POUR, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.pour_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.pour_in, NULL);
    /**  液冷检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_LIQUID, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.liquid_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.liquid_in, NULL);
    /** 熔断器检测使能 */
    sys_config_item_init(CONFIG_ITEM_INEN_FUSE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.function_enable.fuse_in)), \
            (uint8_t*)&s_chargepile_config_info.function_enable.fuse_in, NULL);
    /** 急停反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_SCRAM, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.emergency_stop)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.emergency_stop, NULL);
    /** 门禁反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_GATE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.gate)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.gate, NULL);
    /** 交流接触器反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_ACRELAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.acrelay)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.acrelay, NULL);
    /** 直流继电器反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_DCRELAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.dcrelay)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.dcrelay, NULL);
    /** 风扇反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_FAN, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.fan)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.fan, NULL);
    /** 电子锁反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_ELOCK, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.elock)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.elock, NULL);
    /** 防雷器反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_PROTECT_LIGHT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.protectlight)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.protectlight, NULL);
    /** 枪座反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_GUNSITE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.gunsite)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.gunsite, NULL);
    /** 断路器反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_CIRCUIT_BREAKER, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.circuit_breaker)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.circuit_breaker, NULL);
    /** 水浸反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_FLOOD, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.flood)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.flood, NULL);
    /** 烟感反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_SMOKE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.smoke)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.smoke, NULL);
    /** 倾倒反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_POUR, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.pour)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.pour, NULL);
    /** 液冷反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_LIQUID, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.liquid)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.liquid, NULL);
    /** 熔断器反馈取反 */
    sys_config_item_init(CONFIG_ITEM_INNEG_FUSE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.state_reversal.fuse)), \
            (uint8_t*)&s_chargepile_config_info.state_reversal.fuse, NULL);
    /** 二维码前缀 */
    sys_config_item_init(CONFIG_ITEM_QRCODE_PRE, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.qrcode_prefix) - 1), \
            (uint8_t*)s_chargepile_config_info.config_info.qrcode_prefix, &s_chargepile_config_info.config_info.prefix_length);
    /** 二维码后缀 */
    sys_config_item_init(CONFIG_ITEM_QRCODE_SUF, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.qrcode_suffix) - 1), \
            (uint8_t*)s_chargepile_config_info.config_info.qrcode_suffix, &s_chargepile_config_info.config_info.suffix_length);
    /** A枪电表地址 */
    sys_config_item_init(CONFIG_ITEM_METER_NOA, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.meter_address[0]) - 1), \
            (uint8_t*)s_chargepile_config_info.config_info.meter_address[0], NULL);
    /** B枪电表地址 */
    sys_config_item_init(CONFIG_ITEM_METER_NOB, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.meter_address[1]) - 1), \
            (uint8_t*)s_chargepile_config_info.config_info.meter_address[1], NULL);
    /** 电表型号 */
    sys_config_item_init(CONFIG_ITEM_METER_MODEL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.meter_model)), \
            (uint8_t*)&s_chargepile_config_info.config_info.meter_model, NULL);
    /** 电表校验方式 */
    sys_config_item_init(CONFIG_ITEM_METER_CHECK_WAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.ammeter_check_way)), \
            (uint8_t*)&s_chargepile_config_info.config_info.ammeter_check_way, NULL);
    /** 电表波特率 */
    sys_config_item_init(CONFIG_ITEM_METER_BAUDRATE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.ammeter_baudrate)), \
            (uint8_t*)&s_chargepile_config_info.config_info.ammeter_baudrate, NULL);
    /** 模块额定输出电压 */
    sys_config_item_init(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.module_rated_outvolt)), \
            (uint8_t*)&s_chargepile_config_info.config_para.module_rated_outvolt, NULL);
    /** 桩最大输出电压 */
    sys_config_item_init(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_max_outvolt)), \
            (uint8_t*)&s_chargepile_config_info.config_para.pile_max_outvolt, NULL);
    /** 桩最小输出电压 */
    sys_config_item_init(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_min_outvolt)), \
            (uint8_t*)&s_chargepile_config_info.config_para.pile_min_outvolt, NULL);
    /** 模块额定输出电流 */
    sys_config_item_init(CONFIG_ITEM_RATED_LIMIT_CURRENT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.module_rated_limit_curr)), \
            (uint8_t*)&s_chargepile_config_info.config_para.module_rated_limit_curr, NULL);
    /** 桩最大输出电流 */
    sys_config_item_init(CONFIG_ITEM_MAX_LIMIT_CURRENT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_max_limit_curr)), \
            (uint8_t*)&s_chargepile_config_info.config_para.pile_max_limit_curr, NULL);
    /** 桩最小输出电流 */
    sys_config_item_init(CONFIG_ITEM_MIN_LIMIT_CURRENT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_para.pile_min_limit_curr)), \
            (uint8_t*)&s_chargepile_config_info.config_para.pile_min_limit_curr, NULL);
    /** 桩最大输出功率 */
    sys_config_item_init(CONFIG_ITEM_SYSTEM_POWER_TOTAL, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.system_power_total)), \
            (uint8_t*)&s_chargepile_config_info.config_info.system_power_total, NULL);
    /** 功率分配方式 */
    sys_config_item_init(CONFIG_ITEM_ALLOCATION_WAY, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.power_alloc_way)), \
            (uint8_t*)&s_chargepile_config_info.config_info.power_alloc_way, NULL);
    /** 设备类型 */
    sys_config_item_init(CONFIG_ITEM_DEVICE_TYPE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.system_function)), \
            (uint8_t*)&s_chargepile_config_info.config_info.system_function, NULL);
    /** 枪头电压 */
    sys_config_item_init(CONFIG_ITEM_GUNVOLT_LIMIT, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.gunvolt_limit)), \
            (uint8_t*)&s_chargepile_config_info.config_info.gunvolt_limit, NULL);
    /** VIN码白名单 */
    sys_config_item_init(CONFIG_ITEM_VIN_WHITELIST, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.vin_whitelist)), \
            (uint8_t*)&s_chargepile_config_info.config_info.vin_whitelist, NULL);
    /** 卡号白名单 */
    sys_config_item_init(CONFIG_ITEM_CARD_WHITELIST, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number)), \
            (uint8_t*)&s_chargepile_config_info.config_info.card_whitelist.card_number, NULL);
    /** 屏幕密码 */
    sys_config_item_init(CONFIG_ITEM_SCREEN_PASSWORD, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.screen_password)), \
            (uint8_t*)&s_chargepile_config_info.config_info.screen_password, NULL);
    /** 帮助电话 */
    sys_config_item_init(CONFIG_ITEM_HELP_PHONE, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.pile_info.help_number)), \
            (uint8_t*)&s_chargepile_config_info.pile_info.help_number, &s_chargepile_config_info.pile_info.help_number_len);
    /** 用户识别码 */
    sys_config_item_init(CONFIG_ITEM_USER_IDENTITY, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.pile_info.user_identity)), \
            (uint8_t*)&s_chargepile_config_info.pile_info.user_identity, NULL);
    /** 注册码 */
    sys_config_item_init(CONFIG_ITEM_REGISTER_CODE, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.register_code)), \
            (uint8_t*)&s_chargepile_config_info.config_info.register_code, NULL);
    /** 网络类型 */
    sys_config_item_init(CONFIG_ITEM_NET_TYPE, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.network.nettype)), \
            (uint8_t*)&s_chargepile_config_info.network.nettype, NULL);
    /** A枪终端地址 */
    sys_config_item_init(CONFIG_ITEM_TEMINAL_ADDRA, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.teminal_addrA)), \
            (uint8_t*)&s_chargepile_config_info.config_info.teminal_addrA, NULL);
    /** B枪终端地址 */
    sys_config_item_init(CONFIG_ITEM_TEMINAL_ADDRB, (0 <<(32 - 4))| (sizeof(s_chargepile_config_info.config_info.teminal_addrB)), \
            (uint8_t*)&s_chargepile_config_info.config_info.teminal_addrB, NULL);
#ifdef CP_USING_OFFLINE_BILLING
    /** 计费规则信息 */
    sys_config_item_init(CONFIG_ITEM_BILLING_RULE, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.billing_rule)), \
            (uint8_t*)&s_chargepile_config_info.billing_rule, NULL);
#endif /* #ifdef CP_USING_OFFLINE_BILLING */
    /** 目标平台信息 */
    sys_config_item_init(CONFIG_ITEM_TARGET_PLATFORM, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.target_plat)), \
            (uint8_t*)&s_chargepile_config_info.target_plat, NULL);
    /** 监控平台信息 */
    sys_config_item_init(CONFIG_ITEM_MONITOR_PLATFORM, (1 <<(32 - 4))| (sizeof(s_chargepile_config_info.monitor_plat)), \
            (uint8_t*)&s_chargepile_config_info.monitor_plat, NULL);

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
    sys_config_item_init(CONFIG_ITEM_TARGET_PLATFORM_ADDITIONAL, (1 <<(32 - 4))| (sizeof(s_system_config_tp_additional)), \
            (uint8_t*)&s_system_config_tp_additional, NULL);
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */
}
#endif /* SYS_DESIGNATE_REGION */

/*******************************************************
 * 函数名               sys_config_enter_critical
 * 功能                  进入临界区
 * 参数
 * 返回
 ******************************************************/
static void sys_config_enter_critical(void)
{
    rt_enter_critical();
}

/*******************************************************
 * 函数名               sys_config_exit_critical
 * 功能                  退出临界区
 * 参数
 * 返回
 ******************************************************/
static void sys_config_exit_critical(void)
{
    rt_exit_critical();
}

static uint32_t crc32_ieee_update(uint32_t crc, const uint8_t *data, size_t len)
{
    /* crc table generated from polynomial 0xedb88320 */
    static const uint32_t table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    crc = ~crc;

    for (size_t i = 0; i < len; i++) {
        uint8_t byte = data[i];

        crc = (crc >> 4) ^ table[(crc ^ byte) & 0x0f];
        crc = (crc >> 4) ^ table[(crc ^ (byte >> 4)) & 0x0f];
    }

    return (~crc);
}

/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)

static void sys_config_tp_additional_lock(void)
{
    s_config_tp_additional_lock = 0x00;
}

static void sys_config_tp_additional_unlock(void)
{
    s_config_tp_additional_lock = 0x01;
}

static void sys_config_tp_additional_wait_unlock(void)
{
    while(s_config_tp_additional_lock == 0x00){
        SYS_CONFIG_OSDELAY(50);
    }
}

/************************************************************************************
 * 函数名：      do_storage_config_tp_additional_content
 * 功能              执行配置数据存储操作
 * 参数             addr：存储地址
 *       buff：数据检验缓存
 *       blen：数据检验缓存长度
 * 返回            < 0：失败，= 0：成功
 ***********************************************************************************/
static int32_t do_storage_config_tp_additional_content(uint32_t addr, uint8_t *buff, uint32_t blen)
{
    if((buff == NULL) || (blen < sizeof(s_system_config_tp_additional))){
        return -1;
    }

    uint32_t crc = 0;
    int8_t rentry = 3;

    while(rentry > 0){
        s_system_config_tp_additional.crc = crc32_ieee_update(0, (const uint8_t *)&s_system_config_tp_additional, (sizeof(s_system_config_tp_additional) - sizeof(s_system_config_tp_additional.crc)));

        mw_norflash_write(addr, (uint8_t *)&s_system_config_tp_additional, sizeof(s_system_config_tp_additional));

        SYS_CONFIG_OSDELAY(10);

        mw_norflash_read(addr, (uint8_t *)buff, blen);

        crc = crc32_ieee_update(0, (const uint8_t *)buff, (sizeof(s_system_config_tp_additional) - sizeof(s_system_config_tp_additional.crc)));

        if((memcmp(buff, &s_system_config_tp_additional, sizeof(s_system_config_tp_additional))) || (s_system_config_tp_additional.crc != crc)){
            rentry--;
            LOG_W("target platform additional storage_config_content fail, rentry(%d) crc(%x, %x)", rentry, crc, s_system_config_tp_additional.crc);
            SYS_CONFIG_OSDELAY(100);

            crc = 0x00;

            continue;
        }
        break;
    }

    if(rentry <= 0){
        return -1;
    }

    return 0;
}

/************************************************************************************
 * 函数名：      sys_storage_config_tp_additional_item
 * 功能              触发存储配置项数据
 * 参数             无
 * 返回            < 0：失败，= 0：成功
 ***********************************************************************************/
int32_t sys_storage_config_tp_additional_region(void)
{
    uint8_t* _data_temp = NULL;
    int32_t result = 0;

    sys_config_tp_additional_wait_unlock();
    sys_config_tp_additional_lock();

    _data_temp = (uint8_t*)(malloc(sizeof(s_system_config_tp_additional)));
    if(_data_temp == NULL){
        LOG_E("no enough memery for target platform config buff|%d\n", sizeof(s_system_config_tp_additional));
        sys_config_tp_additional_unlock();
        return -0x01;
    }
    memset(_data_temp, 0x00, sizeof(s_system_config_tp_additional));

    if((result = do_storage_config_tp_additional_content(SYSTEM_CONFIG_TP_ADDITIONALREGION_ADDRESS, _data_temp, sizeof(s_system_config_tp_additional))) < 0){
        LOG_E("chargepile config target platform storage failed|%x", SYSTEM_CONFIG_TP_ADDITIONALREGION_ADDRESS);
    }

    free(_data_temp);

    if(result < 0){
        sys_config_tp_additional_unlock();
        return -0x01;
    }

    sys_config_tp_additional_unlock();
    return 0;
}

static void sys_tp_additional_config_data_reset(void)
{

}

int32_t sys_tp_additional_config_init(void)
{
    uint32_t crc = 0x00, init_flag = 0x00;

    mw_norflash_read(SYSTEM_CONFIG_TP_ADDITIONALREGION_ADDRESS, (uint8_t *)&init_flag, sizeof(init_flag));
    rt_kprintf("sys_tp_additional_config_init(%x)\n", init_flag);
    if (init_flag != SYSTEM_INIT_KEY) {
        sys_tp_additional_config_data_reset();
        s_system_config_tp_additional.init_flag = SYSTEM_INIT_KEY;
        s_system_config_tp_additional.verify_result = 0x00;
    }else{
        mw_norflash_read(SYSTEM_CONFIG_TP_ADDITIONALREGION_ADDRESS, (uint8_t *)&s_system_config_tp_additional, sizeof(s_system_config_tp_additional));

        crc = crc32_ieee_update(0x00, (const uint8_t *)&s_system_config_tp_additional, (sizeof(s_system_config_tp_additional) - sizeof(s_system_config_tp_additional.crc)));

        s_system_config_tp_additional.verify_result = 0x01;

        if (crc != s_system_config_tp_additional.crc) {
            if(++s_storage_chip_entry > 0x03){
                LOG_E("target platform additional config crc error");
                s_storage_chip_entry = 0x00;
                s_system_config_tp_additional.verify_result = 0x00;
            }else{
                return -0x01;
            }
        }
        s_storage_chip_entry = 0x00;
    }

    s_storage_chip_entry = 0x00;
    LOG_D("target platform additional config success");

    return 0x00;
}

int32_t sys_tp_additional_check_config(void)
{
    return 0x00;
}

#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/

static void sys_config_lock(void)
{
    s_sys_config_lock = 0x00;
}

static void sys_config_unlock(void)
{
    s_sys_config_lock = 0x01;
}

static void sys_config_wait_unlock(void)
{
    while(s_sys_config_lock == 0x00){
        SYS_CONFIG_OSDELAY(50);
    }
}

/************************************************************************************
 * 函数名：      do_storage_config_content
 * 功能              执行配置数据存储操作
 * 参数             addr：存储地址
 *       buff：数据检验缓存
 *       blen：数据检验缓存长度
 * 返回            < 0：失败，= 0：成功
 ***********************************************************************************/
static int32_t do_storage_config_content(uint32_t addr, uint8_t *buff, uint32_t blen)
{
    if((buff == NULL) || (blen < sizeof(s_chargepile_config_info))){
        return -1;
    }
    uint32_t crc = 0;
    int8_t rentry = 3;

    while(rentry > 0){
        s_chargepile_config_info.crc = crc32_ieee_update(0, (const uint8_t *)&s_chargepile_config_info, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));
        mw_norflash_write(addr, (uint8_t *)&s_chargepile_config_info, sizeof(s_chargepile_config_info));

        SYS_CONFIG_OSDELAY(10);

        mw_norflash_read(addr, buff, sizeof(s_chargepile_config_info));
        crc = crc32_ieee_update(0, (const uint8_t *)buff, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));

        if((memcmp(buff, &s_chargepile_config_info, sizeof(s_chargepile_config_info))) || (s_chargepile_config_info.crc != crc)){
            rentry--;
            LOG_W("storage_config_content fail, rentry(%d) crc(%x, %x)", rentry, crc, s_chargepile_config_info.crc);
            SYS_CONFIG_OSDELAY(100);
            continue;
        }
        break;
    }

    if(rentry <= 0){
        return -1;
    }

    return 0;
}

/************************************************************************************
 * 函数名：      do_storage_if_config_content
 * 功能              执行配置数据存储操作(内部flash)
 * 参数             addr：存储地址
 *       buff：数据检验缓存
 *       blen：数据检验缓存长度
 * 返回            < 0：失败，= 0：成功
 ***********************************************************************************/
static int32_t do_storage_if_config_content(uint32_t addr, uint8_t *buff, uint32_t blen)
{
    if((buff == NULL) || (blen < sizeof(s_chargepile_config_info))){
        return -1;
    }
    uint32_t crc = 0, init_flag = SYSTEM_INIT_KEY;
    int8_t rentry = 3;

    while(rentry > 0){
        if(mw_iflash_erase_sector(addr, blen, 0x00) < 0x00){
            LOG_E("execute flash erase fail when call do_storage_if_config_content");
        }

        if(mw_iflash_write_directly(addr, (const uint8_t*)&init_flag , sizeof(init_flag), 0x00) < 0x00){
            LOG_E("execute write init flag fail when call do_storage_if_config_content");
        }

        s_chargepile_config_info.crc = crc32_ieee_update(0, (const uint8_t *)&s_chargepile_config_info, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));
        mw_iflash_write_directly((addr + sizeof(init_flag)), (uint8_t *)&s_chargepile_config_info, sizeof(s_chargepile_config_info), 0x00);

        SYS_CONFIG_OSDELAY(10);

        mw_iflash_read((addr + sizeof(init_flag)), buff, sizeof(s_chargepile_config_info));

        crc = crc32_ieee_update(0, (const uint8_t *)buff, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));

        if((memcmp(buff, &s_chargepile_config_info, sizeof(s_chargepile_config_info))) || (s_chargepile_config_info.crc != crc)){
            rentry--;
            LOG_W("storage_if_config_content fail, rentry(%d) crc(%x, %x)", rentry, crc, s_chargepile_config_info.crc);
            SYS_CONFIG_OSDELAY(100);
            continue;
        }
        break;
    }

    if(rentry <= 0){
        return -1;
    }

    return 0;
}

/************************************************************************************
 * 函数名：      sys_storage_config_item
 * 功能              触发存储配置项数据
 * 参数             无
 * 返回            < 0：失败，= 0：成功
 ***********************************************************************************/
int32_t sys_storage_config_item(void)
{
    uint8_t* _data_temp = NULL;
    int32_t result = 0;

    sys_config_wait_unlock();
    sys_config_lock();

    _data_temp = (uint8_t*)(malloc(sizeof(s_chargepile_config_info)));
    if(_data_temp == NULL){
        LOG_E("no enough memery for charge pile config buff|%d\n", sizeof(s_chargepile_config_info));
        sys_config_unlock();
        return -0x01;
    }
    memset(_data_temp, 0x00, sizeof(s_chargepile_config_info));

    LOG_D("system storage config in main address");
    if((result = do_storage_config_content(SYSTEM_CONFIG_MAIN_ADDRESS, _data_temp, sizeof(s_chargepile_config_info))) < 0){
        LOG_E("chargepile config storage failed in main address|%x", SYSTEM_CONFIG_MAIN_ADDRESS);
    }
    LOG_D("system storage config in internal flash");
    if((result = do_storage_if_config_content(SYSTEM_CONFIG_INFO_ADDR_IF, _data_temp, sizeof(s_chargepile_config_info))) < 0){
        LOG_E("chargepile config storage fail in internal flash|%x", SYSTEM_CONFIG_INFO_ADDR_IF);
    }

    free(_data_temp);

    if(result < 0){
        sys_config_unlock();
        return -0x01;
    }

    sys_config_unlock();
    return 0;
}

/************************************************************************************
 * 函数名：      sys_sync_config_item_content
 * 功能              同步配置项数据
 * 参数             name：配置项名
 *       data：配置项数据
 *       len：配置项数据长度
 * 返回            < 0：失败，= 0：成功
 ***********************************************************************************/
int32_t sys_sync_config_item_content(enum config_name name, void* data, uint32_t len)
{
    if(name >= CONFIG_ITEM_SIZE){
        return -0x01;
    }
    if(data == NULL && len == 0){
        return -0x02;
    }

    sys_config_wait_unlock();
    sys_config_lock();

    LOG_D("sys_sync_config_item_content name(%d, %d)[%s]\n", name, len, (char*)data);

    uint8_t user_data_len = s_config_item_set[name].user_section >>(32 - 4);
    uint16_t config_item_len = s_config_item_set[name].user_section &0x3ff;     /** 低10位保存着该配置项的最大长度 */

    if(len > config_item_len){
        sys_config_unlock();
        return -0x03;
    }
    if(s_config_item_set[name].config_index == NULL){   /** 未初始化配置的配置项不予处理 */
        sys_config_unlock();
        return -0x04;
    }

    if(data != NULL){
        memset(s_config_item_set[name].config_index, '\0', config_item_len);
        memcpy(s_config_item_set[name].config_index, (uint8_t*)data, len);
    }else{
        data = (void*)s_config_item_set[name].config_index;   /** 若传入的配置项数据为空则表示该配置项数据已在外部进行更新 */
        len = config_item_len;
    }

    if(s_config_item_set[name].user_data){
        switch(user_data_len){
        case 1:  /* 一字节 */
            *((uint8_t*)(s_config_item_set[name].user_data)) = len;
            break;
        case 2:  /* 二字节 */
            *((uint16_t*)(s_config_item_set[name].user_data)) = len;
            break;
        case 4:  /* 四字节 */
            *((uint32_t*)(s_config_item_set[name].user_data)) = len;
            break;
        default:
            sys_config_unlock();
            return -0x04;
            break;
        }
    }

    sys_config_unlock();
    return 0;
}

uint8_t* sys_read_config_item_content(enum config_name name, uint8_t is_user_content)
{
    if(name >= CONFIG_ITEM_SIZE){
        return NULL;
    }

    if(is_user_content){
        return (uint8_t*)(s_config_item_set[name].user_data);
    }

    return (uint8_t*)(s_config_item_set[name].config_index);
}

static void chargepile_config_data_reset(void)
{
    s_chargepile_config_info.network.domain_len = 0x00;
    memset(s_chargepile_config_info.network.domain, '\0', sizeof(s_chargepile_config_info.network.domain));
    s_chargepile_config_info.network.port = 0x00;
    memset(s_chargepile_config_info.network.mac, '\0', sizeof(s_chargepile_config_info.network.mac));
    memset(s_chargepile_config_info.network.gateway, '\0', sizeof(s_chargepile_config_info.network.gateway));
    s_chargepile_config_info.network.nettype = CP_NETTYPE_4G;

    s_chargepile_config_info.encrypt.key_len = 0x00;
    memset(s_chargepile_config_info.encrypt.key, '\0', sizeof(s_chargepile_config_info.encrypt.key));
    s_chargepile_config_info.encrypt.random_str_len = 0x00;
    memset(s_chargepile_config_info.encrypt.random_str, '\0', sizeof(s_chargepile_config_info.encrypt.random_str));
    s_chargepile_config_info.encrypt.id_len = 0x00;
    memset(s_chargepile_config_info.encrypt.id, '\0', sizeof(s_chargepile_config_info.encrypt.id));
    s_chargepile_config_info.encrypt.sign_len = 0x00;
    memset(s_chargepile_config_info.encrypt.sign, '\0', sizeof(s_chargepile_config_info.encrypt.sign));

    s_chargepile_config_info.pile_info.pile_num_len = 0x00;
    memset(s_chargepile_config_info.pile_info.pile_number, '\0', sizeof(s_chargepile_config_info.pile_info.pile_number));
    s_chargepile_config_info.pile_info.serial_number_len = 0x00;
    memset(s_chargepile_config_info.pile_info.serial_number, '\0', sizeof(s_chargepile_config_info.pile_info.serial_number));
    s_chargepile_config_info.pile_info.help_number_len = 0x00;
    memset(s_chargepile_config_info.pile_info.help_number, '\0', sizeof(s_chargepile_config_info.pile_info.help_number));
    memset(s_chargepile_config_info.pile_info.user_identity, '\0', sizeof(s_chargepile_config_info.pile_info.user_identity));
    memset(s_chargepile_config_info.config_info.register_code, '\0', sizeof(s_chargepile_config_info.config_info.register_code));

    s_chargepile_config_info.config_para.cc1_12_max = CHARGEPILE_CC12V_MAX;
    s_chargepile_config_info.config_para.cc1_12_min = CHARGEPILE_CC12V_MIN;
    s_chargepile_config_info.config_para.cc1_6_max = CHARGEPILE_CC6V_MAX;
    s_chargepile_config_info.config_para.cc1_6_min = CHARGEPILE_CC6V_MIN;
    s_chargepile_config_info.config_para.cc1_4_max = CHARGEPILE_CC4V_MAX;
    s_chargepile_config_info.config_para.cc1_4_min = CHARGEPILE_CC4V_MIN;

    s_chargepile_config_info.config_para.module_rated_outvolt = MODULE_RATED_OUTVOLT_DEF;
    s_chargepile_config_info.config_para.pile_max_outvolt = CHARGEPILE_MAX_OUTVOLT_DEF;
    s_chargepile_config_info.config_para.pile_min_outvolt = CHARGEPILE_MIN_OUTVOLT_DEF;
    s_chargepile_config_info.config_para.module_rated_limit_curr = MODULE_RATED_LIMIT_CURR_DEF;
    s_chargepile_config_info.config_para.pile_max_limit_curr = MODULE_MAX_LIMIT_CURR_DEF;
    s_chargepile_config_info.config_para.pile_min_limit_curr = MODULE_MIN_LIMIT_CURR_DEF;

    s_chargepile_config_info.config_para.soc_stop = PROTECT_STOP_SOC_VALUE_DEFAULT;

    s_chargepile_config_info.config_para.overtemp_alarm = PROTECT_OVERTEMP_WARNNING_VALUE_DEFAULT;
    s_chargepile_config_info.config_para.overtemp_stop = PROTECT_OVERTEMP_STOP_VALUE_DEFAULT;
    s_chargepile_config_info.config_para.overtemp_recovery = PROTECT_OVERTEMP_RESUME_VALUE_DEFAULT;
    s_chargepile_config_info.config_para.overtemp_limitcur = PROTECT_OVERTEMP_LIMITCURR_VALUE_DEFAULT;
    s_chargepile_config_info.config_para.eloss_proportion = CHARGEPILE_ELOSS_PROPORTION_DEF;
    memset(s_chargepile_config_info.config_para.mode_parameter, 0x00, sizeof(s_chargepile_config_info.config_para.mode_parameter));

    s_chargepile_config_info.config_para.input_overvol = CHARGEPILE_INPUT_OVERVOLT_DEF;
    s_chargepile_config_info.config_para.input_undervol = CHARGEPILE_INPUT_UNDERVOLT_DEF;
    s_chargepile_config_info.config_para.output_overvol = CHARGEPILE_OUTPUT_OVERVOLT_DEF;
    s_chargepile_config_info.config_para.output_undervol = CHARGEPILE_OUTPUT_UNDERVOLT_DEF;
    s_chargepile_config_info.config_para.output_overcur = CHARGEPILE_OUTPUT_OVERCURR_DEF;

    s_chargepile_config_info.config_info.prefix_length = 0x00;
    memset(s_chargepile_config_info.config_info.qrcode_prefix, '\0', sizeof(s_chargepile_config_info.config_info.qrcode_prefix));
    s_chargepile_config_info.config_info.qrcode_prefix[0x00] = CP_SET_QRCODE_FORMAT_PREFIX;
    s_chargepile_config_info.config_info.qrcode_prefix[0x01] = CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
    s_chargepile_config_info.config_info.suffix_length = 0x00;
    memset(s_chargepile_config_info.config_info.qrcode_suffix, '\0', sizeof(s_chargepile_config_info.config_info.qrcode_suffix));
    memset(s_chargepile_config_info.config_info.vin_whitelist, '\0', sizeof(s_chargepile_config_info.config_info.vin_whitelist));
    memset(s_chargepile_config_info.config_info.card_whitelist.card_number, '\0', sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number));
    memset(s_chargepile_config_info.config_info.card_whitelist.card_uid, '\0', sizeof(s_chargepile_config_info.config_info.card_whitelist.card_uid));
    memset(s_chargepile_config_info.config_info.meter_address, '\0', sizeof(s_chargepile_config_info.config_info.meter_address));
    memset(s_chargepile_config_info.config_info.screen_password, '\0', sizeof(s_chargepile_config_info.config_info.screen_password));
    memcpy(s_chargepile_config_info.config_info.screen_password, "0909", strlen("0909"));

    s_chargepile_config_info.config_info.module_model = MODULE_MODEL_DEFAULT;
    s_chargepile_config_info.config_info.module_group_num = MODULE_GROUP_NUMBER_DEFAULT;
    memset(s_chargepile_config_info.config_info.module_num_singlegroup, 0x00, sizeof(s_chargepile_config_info.config_info.module_num_singlegroup));
    for(uint8_t count = 0x00; count < s_chargepile_config_info.config_info.module_group_num; count++){
        s_chargepile_config_info.config_info.module_num_singlegroup[count] = MODULE_NUMBER_SINGLE_DEFAULT;
    }
	s_chargepile_config_info.config_info.lp_consumption_module = CONFIG_LP_CONSUMPTION_MODULE_NULL;

    s_chargepile_config_info.config_info.gunvolt_limit = GUNVOLT_LIMIT_VALUE_MIN;
    s_chargepile_config_info.config_info.gun_num = 0x02;
    s_chargepile_config_info.config_info.ammeter_check_way = CP_AMMETER_CHECK_WAY_EVEN;
    s_chargepile_config_info.config_info.ammeter_baudrate = CP_AMMETER_BAUDRATE_9600;

    s_chargepile_config_info.function_enable.local_charge = 0x00;
    s_chargepile_config_info.function_enable.local_stop = 0x00;
    s_chargepile_config_info.function_enable.insulation_detect = 0x01;
    s_chargepile_config_info.function_enable.vin_charge = 0x00;
    s_chargepile_config_info.function_enable.parallel_charge = 0x00;
    s_chargepile_config_info.function_enable.plug_charge = 0x00;
    s_chargepile_config_info.function_enable.bcs = 0x00;
    s_chargepile_config_info.function_enable.bsm = 0x00;
    s_chargepile_config_info.function_enable.auxpower_24V = 0x00;
    s_chargepile_config_info.function_enable.parallel_charge = 0x00;
    s_chargepile_config_info.function_enable.acrelay_out = 0x01;
    s_chargepile_config_info.function_enable.elock_out = 0x01;
    s_chargepile_config_info.function_enable.fan_out = 0x01;
    s_chargepile_config_info.function_enable.emergency_stop = 0x01;
    s_chargepile_config_info.function_enable.gate_in = 0x01;
    s_chargepile_config_info.function_enable.acrelay_in = 0x01;
    s_chargepile_config_info.function_enable.dcrelay_in = 0x01;
    s_chargepile_config_info.function_enable.fan_in = 0x01;
    s_chargepile_config_info.function_enable.elock_in = 0x01;
    s_chargepile_config_info.function_enable.temp_protect = 0x01;
    s_chargepile_config_info.function_enable.rfid_card_reader = 0x00;
    s_chargepile_config_info.function_enable.parallel_relay = 0x00;
    s_chargepile_config_info.function_enable.module_slience = 0x00;
    s_chargepile_config_info.function_enable.password_start = 0x00;
    s_chargepile_config_info.function_enable.offline_billing = 0x00;
    s_chargepile_config_info.function_enable.offline_card = 0x00;
    s_chargepile_config_info.function_enable.protectlight_in = 0x00;
    s_chargepile_config_info.function_enable.gunsite_in = 0x00;
    s_chargepile_config_info.function_enable.circuit_breaker_in = 0x00;
    s_chargepile_config_info.function_enable.flood_in = 0x00;
    s_chargepile_config_info.function_enable.smoke_in = 0x00;
    s_chargepile_config_info.function_enable.pour_in = 0x00;
    s_chargepile_config_info.function_enable.liquid_in = 0x00;
    s_chargepile_config_info.function_enable.fuse_in = 0x00;
    s_chargepile_config_info.function_enable.mode_select = 0x00;
    memset(s_chargepile_config_info.function_enable.current_mode, 0x00, sizeof(s_chargepile_config_info.function_enable.current_mode));

    s_chargepile_config_info.state_reversal.emergency_stop = 0x00;
    s_chargepile_config_info.state_reversal.gate = 0x00;
    s_chargepile_config_info.state_reversal.acrelay = 0x00;
    s_chargepile_config_info.state_reversal.dcrelay = 0x00;
    s_chargepile_config_info.state_reversal.parallel_relay = 0x00;
    s_chargepile_config_info.state_reversal.fan = 0x00;
    s_chargepile_config_info.state_reversal.elock = 0x00;
    s_chargepile_config_info.state_reversal.protectlight = 0x00;
    s_chargepile_config_info.state_reversal.gunsite = 0x00;
    s_chargepile_config_info.state_reversal.circuit_breaker = 0x00;
    s_chargepile_config_info.state_reversal.flood = 0x00;
    s_chargepile_config_info.state_reversal.smoke = 0x00;
    s_chargepile_config_info.state_reversal.pour = 0x00;
    s_chargepile_config_info.state_reversal.liquid = 0x00;
    s_chargepile_config_info.state_reversal.fuse = 0x00;

    s_chargepile_config_info.target_plat.verify_result = 0x00;
    s_chargepile_config_info.monitor_plat.verify_result = 0x00;

#ifdef CP_USING_OFFLINE_BILLING
    for(uint8_t period = 0x00; period < CP_PERIOD_MAX; period++){
        s_chargepile_config_info.billing_rule.period_price[period].elect = CP_PERIOD_ELECT_PRICE_DEF;
        s_chargepile_config_info.billing_rule.period_price[period].service = CP_PERIOD_SERVICE_PRICE_DEF;
        s_chargepile_config_info.billing_rule.period_price[period].delay = CP_PERIOD_DELAY_PRICE_DEF;
        s_chargepile_config_info.billing_rule.period_price[period].reserve = 0x00;

        s_chargepile_config_info.billing_rule.rate_number[period] = CP_PERIOD_RATED_NUMBER_DEFAULT;
    }

    s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP_SHARP] = CP_SHARP_SHARP_RATED_ELECT_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP_SHARP] = CP_SHARP_SHARP_RATED_SERVICE_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP_SHARP] = CP_SHARP_SHARP_RATED_DELAY_PRICE_DEF;

    s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP] = CP_SHARP_RATED_ELECT_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP] = CP_SHARP_RATED_SERVICE_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP] = CP_SHARP_RATED_DELAY_PRICE_DEF;

    s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_PEAK] = CP_PEAK_RATED_ELECT_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_PEAK] = CP_PEAK_RATED_SERVICE_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_PEAK] = CP_PEAK_RATED_DELAY_PRICE_DEF;

    s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_FLAT] = CP_FLAT_RATED_ELECT_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_FLAT] = CP_FLAT_RATED_SERVICE_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_FLAT] = CP_FLAT_RATED_DELAY_PRICE_DEF;

    s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_VALLEY] = CP_VALLEY_RATED_ELECT_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_VALLEY] = CP_VALLEY_RATED_SERVICE_PRICE_DEF;
    s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_VALLEY] = CP_VALLEY_RATED_DELAY_PRICE_DEF;

    sys_period_time_resume_default(s_chargepile_config_info.billing_rule.time, sizeof(s_chargepile_config_info.billing_rule.time));
#endif /* CP_USING_OFFLINE_BILLING */

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
    s_system_config_tp_additional.verify_result = 0x00;
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* APP_INCLUDE_TARGET_PLATFORM */
}

int32_t chargepile_config_init(void)
{
    uint32_t crc = 0x00, init_flag = 0x00;

    mw_norflash_read(SYSTEM_CONFIG_INIT_FLAG_ADDRESS, (uint8_t *)&init_flag, sizeof(init_flag));
    if (init_flag != SYSTEM_INIT_KEY) {
        if(init_flag == 0xFFFFFFFF){
            LOG_D("new board configure start initialization(%d)", sizeof(s_chargepile_config_info));
            chargepile_config_data_reset();

            crc = crc32_ieee_update(0x00, (const uint8_t *)&s_chargepile_config_info, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));
            s_chargepile_config_info.crc = crc;

            init_flag = SYSTEM_INIT_KEY;
            mw_norflash_write(SYSTEM_CONFIG_INIT_FLAG_ADDRESS, (uint8_t *)&init_flag, sizeof(init_flag));
            s_storage_chip_entry = 0x00;
            return 0x00;
        }else{
            if(++s_storage_chip_entry > 0x03){
                if(init_flag != 0x00){
                    LOG_D("config init flag error, configure start initialization(%d)", sizeof(s_chargepile_config_info));
                    chargepile_config_data_reset();

                    crc = crc32_ieee_update(0x00, (const uint8_t *)&s_chargepile_config_info, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));
                    s_chargepile_config_info.crc = crc;

                    init_flag = SYSTEM_INIT_KEY;
                    mw_norflash_write(SYSTEM_CONFIG_INIT_FLAG_ADDRESS, (uint8_t *)&init_flag, sizeof(init_flag));
                    s_storage_chip_entry = 0x00;
                }
                return 0x00;
            }
        }
        s_config_info_address = SYSTEM_CONFIG_MAIN_ADDRESS;
        return -0x01;
    }else{
        memset(&s_chargepile_config_info, 0xFF, sizeof(s_chargepile_config_info));
        mw_norflash_read(s_config_info_address, (uint8_t *)&s_chargepile_config_info, sizeof(s_chargepile_config_info));

        crc = crc32_ieee_update(0x00, (const uint8_t *)&s_chargepile_config_info, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));

        s_chargepile_config_info.target_plat.verify_result = 0x01;
        s_chargepile_config_info.monitor_plat.verify_result = 0x01;

        if (crc != s_chargepile_config_info.crc) {
            if(++s_storage_chip_entry > 0x03){
//                if(s_config_info_address == SYSTEM_CONFIG_MAIN_ADDRESS){
                if(0){
                    s_config_info_address = SYSTEM_CONFIG_BACKUP_ADDRESS;
                    s_storage_chip_entry = 0x00;
                    LOG_W("config info check error in main addr, read backup addr");
                    return -0x01;
                }else{
                    LOG_E("system config crc error");
                    s_chargepile_config_info.target_plat.verify_result = 0x00;
                    s_chargepile_config_info.monitor_plat.verify_result = 0x00;
                    s_storage_chip_entry = 0x00;
                }
            }else{
                return -0x01;
            }
        }
        s_storage_chip_entry = 0x00;
    }

    s_storage_chip_entry = 0x00;
    LOG_D("system config success");

    return 0x00;
}

int32_t system_config_init_if(void)
{
    uint8_t init_flag_is_correct = 0x00;
    uint32_t crc = 0x00, init_flag = 0x00;

    while(1){
        mw_iflash_read(SYSTEM_CONFIG_INFO_ADDR_IF, (uint8_t *)&init_flag, sizeof(init_flag));
        if(init_flag != SYSTEM_INIT_KEY) {
            if(init_flag == 0xFFFFFFFF){
                LOG_D("new board system configure if(%d)", sizeof(s_chargepile_config_info));
                return -0x01;
            }else{
                if(++s_storage_chip_entry > 0x03){
                    break;
                }
            }
            rt_thread_mdelay(1000);
            mw_iwdg_refresh();
            continue;
        }
        init_flag_is_correct = 0x01;
        break;
    }

    s_storage_chip_entry = 0x00;
    while(1){
        memset(&s_chargepile_config_info, 0xFF, sizeof(s_chargepile_config_info));
        mw_iflash_read((SYSTEM_CONFIG_INFO_ADDR_IF + sizeof(init_flag)), (uint8_t *)&s_chargepile_config_info, sizeof(s_chargepile_config_info));

        crc = crc32_ieee_update(0x00, (const uint8_t *)&s_chargepile_config_info, (sizeof(s_chargepile_config_info) - sizeof(s_chargepile_config_info.crc)));

        s_chargepile_config_info.target_plat.verify_result = 0x01;
        s_chargepile_config_info.monitor_plat.verify_result = 0x01;

        if(crc != s_chargepile_config_info.crc){
            if(++s_storage_chip_entry > 0x03){
                LOG_E("system config if crc error");
                if(init_flag_is_correct){
                    s_storage_chip_entry = 0x00; /** 只有初始标志和校验码都不对时才认为里面的数据不对 */
                    s_chargepile_config_info.target_plat.verify_result = 0x00;
                    s_chargepile_config_info.monitor_plat.verify_result = 0x00;
                }
            }else{
                rt_thread_mdelay(1000);
                mw_iwdg_refresh();
                continue;
            }
        }
        break;
    }

    if(s_storage_chip_entry > 0x03){
        return -0x01;
    }

    LOG_D("system config if success");

    return 0x00;
}

/**********************************************************************
 * 函数           sys_string_contain_ctrl_char
 * 功能           判断字符串是否含有控制字符
 * 参数           string    字符串
 *        slen      字符串长度
 * 返回           1：含有控制字符        0：不含控制字符
 *********************************************************************/
int32_t sys_string_contain_ctrl_char(const char* string, uint16_t slen)
{
    if((string == NULL) || (slen == 0x00)){
        return 0x01;
    }
    for(uint16_t index = 0x00; index < slen; index++){
        if(!((string[index] >= 0x20) && (string[index] < 0x7F))){
            return 0x01;
        }
    }

    return 0x00;
}

/**********************************************************************
 * 函数           sys_string_is_pure_digital_
 * 功能           判断字符串是否是纯数字字符
 * 参数           string    字符串
 *        slen      字符串长度
 * 返回           1：是纯数字字符        0：不是纯数字字符
 *********************************************************************/
int32_t sys_string_is_pure_digital_(const char* string, uint16_t slen)
{
    if((string == NULL) || (slen == 0x00)){
        return 0x00;
    }
    for(uint16_t index = 0x00; index < slen; index++){
        if(!((string[index] >= '0') && (string[index] <= '9'))){
            return 0x00;
        }
    }

    return 0x01;
}

/**********************************************************************
 * 函数           sys_string_is_pure_digital_alphabet
 * 功能           判断字符串是否是纯数字、字母字符
 * 参数           string    字符串
 *        slen      字符串长度
 * 返回           1：是纯数字、字母字符        0：不是纯数字、字母字符
 *********************************************************************/
int32_t sys_string_is_pure_digital_alphabet(const char* string, uint16_t slen)
{
    if((string == NULL) || (slen == 0x00)){
        return 0x00;
    }
    for(uint16_t index = 0x00; index < slen; index++){
        if(!(((string[index] >= '0') && (string[index] <= '9')) || ((string[index] >= 'a') && (string[index] <= 'z')) ||
                ((string[index] >= 'A') && (string[index] <= 'Z')))){
            return 0x00;
        }
    }

    return 0x01;
}

int32_t chargepile_check_config(void)
{
#ifdef CP_USING_OFFLINE_BILLING
    struct period_time time[CP_RATED_TYPE_NUM_MAX *CP_RATED_TYPE_PERIOD_NUM];  /* 时段时间 */
    uint8_t i = 0x00, j = 0x00, valid_count = 0;
#else
    s_chargepile_config_info.function_enable.offline_billing = 0;
#endif /* CP_USING_OFFLINE_BILLING */

    uint16_t valid_len = 0x00;
    uint32_t single_module_power = 0x00;       /* 单个模块能输出的最大(额定)功率 */
    s_system_power_max = 0x00;

    if(s_chargepile_config_info.config_info.module_group_num > MODULE_GROUP_NUMBER_MAX){
        s_chargepile_config_info.config_info.module_group_num = MODULE_GROUP_NUMBER_DEFAULT;
    }
    if(s_chargepile_config_info.config_info.module_model > MODULE_MODEL_NUMBER){
        s_chargepile_config_info.config_info.module_model = MODULE_MODEL_DEFAULT;
    }
    for(uint8_t count = 0; count < s_module_info.module_group_num; count++){
        if(s_chargepile_config_info.config_info.module_num_singlegroup[count] > MODULE_NUMBER_SINGLE_MAX){
            s_chargepile_config_info.config_info.module_num_singlegroup[count] = MODULE_NUMBER_SINGLE_DEFAULT;
        }
    }

    if(s_chargepile_config_info.network.nettype >= CP_NETTYPE_SIZE){         /* 联网方式默认4G */
        s_chargepile_config_info.network.nettype = CP_NETTYPE_4G;
    }
    /** 设备类型默认均充双枪(注：这是普通双枪版本做法，其它版本需要根据实际来) */
    if(s_chargepile_config_info.config_info.system_function != SYSTEM_FUNCTION_DYNAMIC_SWITCH){
        s_chargepile_config_info.config_info.system_function = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
    }
    /** 低功耗模块默认无 */
    s_chargepile_config_info.config_info.lp_consumption_module = CONFIG_LP_CONSUMPTION_MODULE_NULL;
    if(s_chargepile_config_info.config_info.lp_consumption_module >= CONFIG_LP_CONSUMPTION_MODULE_SIZE){
        s_chargepile_config_info.config_info.lp_consumption_module = CONFIG_LP_CONSUMPTION_MODULE_NULL;
    }

    /** 电表串口校验方式模式偶校验 */
    if((s_chargepile_config_info.config_info.ammeter_check_way < CP_AMMETER_CHECK_WAY_EVEN) ||\
            (s_chargepile_config_info.config_info.ammeter_check_way > CP_AMMETER_CHECK_WAY_NONE)){
        s_chargepile_config_info.config_info.ammeter_check_way = CP_AMMETER_CHECK_WAY_EVEN;
    }
    /** 电表串口波特率默认9600 */
    if((s_chargepile_config_info.config_info.ammeter_baudrate < CP_AMMETER_BAUDRATE_9600) ||\
            (s_chargepile_config_info.config_info.ammeter_baudrate > CP_AMMETER_BAUDRATE_115200)){
        s_chargepile_config_info.config_info.ammeter_baudrate = CP_AMMETER_BAUDRATE_9600;
    }

    if((s_chargepile_config_info.config_para.module_rated_outvolt < MODULE_RATED_OUTVOLT_MIN) ||
            (s_chargepile_config_info.config_para.module_rated_outvolt > MODULE_RATED_OUTVOLT_MAX)){
        s_chargepile_config_info.config_para.module_rated_outvolt = MODULE_RATED_OUTVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.pile_max_outvolt < CHARGEPILE_MAX_OUTVOLT_MIN) ||
            (s_chargepile_config_info.config_para.pile_max_outvolt > CHARGEPILE_MAX_OUTVOLT_MAX)){
        s_chargepile_config_info.config_para.pile_max_outvolt = CHARGEPILE_MAX_OUTVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.pile_min_outvolt < CHARGEPILE_MIN_OUTVOLT_MIN) ||
            (s_chargepile_config_info.config_para.pile_min_outvolt > CHARGEPILE_MIN_OUTVOLT_MAX)){
        s_chargepile_config_info.config_para.pile_min_outvolt = CHARGEPILE_MIN_OUTVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.module_rated_limit_curr < MODULE_RATED_LIMIT_CURR_MIN) ||
            (s_chargepile_config_info.config_para.module_rated_limit_curr > MODULE_RATED_LIMIT_CURR_MAX)){
        s_chargepile_config_info.config_para.module_rated_limit_curr = MODULE_RATED_LIMIT_CURR_DEF;
    }
    if((s_chargepile_config_info.config_para.pile_max_limit_curr < MODULE_MAX_LIMIT_CURR_MIN) ||
            (s_chargepile_config_info.config_para.pile_max_limit_curr > MODULE_MAX_LIMIT_CURR_MAX)){
        s_chargepile_config_info.config_para.pile_max_limit_curr = MODULE_MAX_LIMIT_CURR_DEF;
    }
    if((s_chargepile_config_info.config_para.pile_min_limit_curr < MODULE_MIN_LIMIT_CURR_MIN) ||
            (s_chargepile_config_info.config_para.pile_min_limit_curr > MODULE_MIN_LIMIT_CURR_MAX)){
        s_chargepile_config_info.config_para.pile_min_limit_curr = MODULE_MIN_LIMIT_CURR_DEF;
    }

    if((s_chargepile_config_info.config_para.soc_stop < PROTECT_STOP_SOC_VALUE_MIN) ||
            (s_chargepile_config_info.config_para.soc_stop > PROTECT_STOP_SOC_VALUE_MAX)){
        s_chargepile_config_info.config_para.soc_stop = PROTECT_STOP_SOC_VALUE_DEFAULT;
    }
    if((s_chargepile_config_info.config_para.overtemp_alarm < PROTECT_OVERTEMP_WARNNING_VALUE_MIN) ||
            (s_chargepile_config_info.config_para.overtemp_alarm > PROTECT_OVERTEMP_WARNNING_VALUE_MAX)){
        s_chargepile_config_info.config_para.overtemp_alarm = PROTECT_OVERTEMP_WARNNING_VALUE_DEFAULT;
    }
    if((s_chargepile_config_info.config_para.overtemp_stop < PROTECT_OVERTEMP_STOP_VALUE_MIN) ||
            (s_chargepile_config_info.config_para.overtemp_stop > PROTECT_OVERTEMP_STOP_VALUE_MAX)){
        s_chargepile_config_info.config_para.overtemp_stop = PROTECT_OVERTEMP_STOP_VALUE_DEFAULT;
    }
    if((s_chargepile_config_info.config_para.overtemp_recovery < PROTECT_OVERTEMP_RESUME_VALUE_MIN) ||
            (s_chargepile_config_info.config_para.overtemp_recovery > PROTECT_OVERTEMP_RESUME_VALUE_MAX)){
        s_chargepile_config_info.config_para.overtemp_recovery = PROTECT_OVERTEMP_RESUME_VALUE_DEFAULT;
    }
    if((s_chargepile_config_info.config_para.overtemp_limitcur < PROTECT_OVERTEMP_LIMITCURR_VALUE_MIN) ||
            (s_chargepile_config_info.config_para.overtemp_limitcur > PROTECT_OVERTEMP_LIMITCURR_VALUE_MAX)){
        s_chargepile_config_info.config_para.overtemp_limitcur = PROTECT_OVERTEMP_LIMITCURR_VALUE_DEFAULT;
    }
    if((s_chargepile_config_info.config_para.input_overvol < CHARGEPILE_INPUT_OVERVOLT_MIN) || \
            (s_chargepile_config_info.config_para.input_overvol > CHARGEPILE_INPUT_OVERVOLT_MAX)){
        s_chargepile_config_info.config_para.input_overvol = CHARGEPILE_INPUT_OVERVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.input_undervol < CHARGEPILE_INPUT_UNDERVOLT_MIN) || \
            (s_chargepile_config_info.config_para.input_undervol > CHARGEPILE_INPUT_UNDERVOLT_MAX)){
        s_chargepile_config_info.config_para.input_undervol = CHARGEPILE_INPUT_UNDERVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.output_overvol < CHARGEPILE_OUTPUT_OVERVOLT_MIN) || \
            (s_chargepile_config_info.config_para.output_overvol > CHARGEPILE_OUTPUT_OVERVOLT_MAX)){
        s_chargepile_config_info.config_para.output_overvol = CHARGEPILE_OUTPUT_OVERVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.output_undervol < CHARGEPILE_OUTPUT_UNDERVOLT_MIN) || \
            (s_chargepile_config_info.config_para.output_undervol > CHARGEPILE_OUTPUT_UNDERVOLT_MAX)){
        s_chargepile_config_info.config_para.output_undervol = CHARGEPILE_OUTPUT_UNDERVOLT_DEF;
    }
    if((s_chargepile_config_info.config_para.output_overcur < CHARGEPILE_OUTPUT_OVERCURR_MIN) || \
            (s_chargepile_config_info.config_para.output_overcur > CHARGEPILE_OUTPUT_OVERCURR_MAX)){
        s_chargepile_config_info.config_para.output_overcur = CHARGEPILE_OUTPUT_OVERCURR_DEF;
    }

    if((s_chargepile_config_info.config_para.eloss_proportion < CHARGEPILE_ELOSS_PROPORTION_MIN) ||
            (s_chargepile_config_info.config_para.eloss_proportion > CHARGEPILE_ELOSS_PROPORTION_MAX)){
        s_chargepile_config_info.config_para.eloss_proportion = CHARGEPILE_ELOSS_PROPORTION_DEF;
    }

    if(s_chargepile_config_info.function_enable.emergency_stop > 0x01){    /* 急停故障检测默认开启 */
        s_chargepile_config_info.function_enable.emergency_stop = 0x01;
    }
    if(s_chargepile_config_info.function_enable.gate_in > 0x01){     /* 门禁故障检测默认开启 */
        s_chargepile_config_info.function_enable.gate_in = 0x01;
    }
    if(s_chargepile_config_info.function_enable.acrelay_in > 0x01){       /* 交流接触器故障检测默认开启 */
        s_chargepile_config_info.function_enable.acrelay_in = 0x01;
    }
    if(s_chargepile_config_info.function_enable.dcrelay_in > 0x01){       /* 直流继电器故障检测默认开启 */
        s_chargepile_config_info.function_enable.dcrelay_in = 0x01;
    }
    if(s_chargepile_config_info.function_enable.fan_in > 0x01){      /* 风扇故障检测默认开启 */
        s_chargepile_config_info.function_enable.fan_in = 0x01;
    }
    if(s_chargepile_config_info.function_enable.elock_out > 0x01){   /* 电子锁故障检测默认开启 */
        s_chargepile_config_info.function_enable.elock_out = 0x01;
    }
    if(s_chargepile_config_info.function_enable.local_charge > 0x01){       /* 本地启动默认关闭 */
        s_chargepile_config_info.function_enable.local_charge = 0x00;
    }
    if(s_chargepile_config_info.function_enable.local_stop > 0x01){       /* 本地停止默认关闭 */
        s_chargepile_config_info.function_enable.local_stop = 0x00;
    }
    if(s_chargepile_config_info.function_enable.insulation_detect > 0x01){  /* 绝缘检测默认开启 */
        s_chargepile_config_info.function_enable.insulation_detect = 0x01;
    }
    if(s_chargepile_config_info.function_enable.vin_charge > 0x01){         /* VIN启动默认关闭 */
        s_chargepile_config_info.function_enable.vin_charge = 0x00;
    }
    if(s_chargepile_config_info.function_enable.temp_protect > 0x01){    /* 温度保护默认开启 */
        s_chargepile_config_info.function_enable.temp_protect = 0x01;
    }
    if(s_chargepile_config_info.function_enable.rfid_card_reader > 0x01){   /* 读卡器默认关闭 */
        s_chargepile_config_info.function_enable.rfid_card_reader = 0x00;
    }
    if(s_chargepile_config_info.function_enable.parallel_relay > 0x01){    /* 并联默认启用 */
        s_chargepile_config_info.function_enable.parallel_relay = 0x01;
    }
    if(s_chargepile_config_info.function_enable.auxpower_24V > 0x01){   /* 24V辅源默认关闭 */
        s_chargepile_config_info.function_enable.auxpower_24V = 0x00;
    }
    if(s_chargepile_config_info.function_enable.parallel_charge > 0x01){   /* 并充默认关闭 */
        s_chargepile_config_info.function_enable.parallel_charge = 0x00;
    }
    if(s_chargepile_config_info.function_enable.module_slience > 0x01){   /* 模块静音默认关闭 */
        s_chargepile_config_info.function_enable.module_slience = 0x00;
    }
    if(s_chargepile_config_info.function_enable.password_start > 0x01){   /* 密码启动默认关闭 */
        s_chargepile_config_info.function_enable.password_start = 0x00;
    }
    if(s_chargepile_config_info.function_enable.offline_billing > 0x01){   /* 离线计费默认关闭 */
        s_chargepile_config_info.function_enable.offline_billing = 0x00;
    }
    if(s_chargepile_config_info.function_enable.plug_charge > 0x01){       /* 即插即充默认关闭 */
        s_chargepile_config_info.function_enable.plug_charge = 0x00;
    }
    if(s_chargepile_config_info.function_enable.bcs > 0x01){               /* BCS功能默认关闭 */
        s_chargepile_config_info.function_enable.bcs = 0x00;
    }
    if(s_chargepile_config_info.function_enable.bsm > 0x01){               /* BSM功能默认关闭 */
        s_chargepile_config_info.function_enable.bsm = 0x00;
    }
    if(s_chargepile_config_info.function_enable.offline_card > 0x01){      /* 离线卡功能默认开启 */
        s_chargepile_config_info.function_enable.offline_card = 0x01;
    }
    if(s_chargepile_config_info.function_enable.mode_select > 0x01){      /* 模式选择功能默认关闭 */
        s_chargepile_config_info.function_enable.mode_select = 0x00;
    }
    for(uint8_t mode = 0x00; mode < sizeof(s_chargepile_config_info.function_enable.current_mode); mode++){
        if((s_chargepile_config_info.function_enable.current_mode[mode] >= CP_MODE_SIZE) || \
                ((s_chargepile_config_info.function_enable.current_mode[mode] != CP_MODE_CHARGE_FULL) && \
                        (s_chargepile_config_info.function_enable.current_mode[mode] != CP_MODE_LIMIT_RESERVATION))){    /* 当前模式默认充满 */
            s_chargepile_config_info.function_enable.current_mode[mode] = CP_MODE_CHARGE_FULL;
        }
    }

    if(s_chargepile_config_info.function_enable.acrelay_out > 0x01){       /* 交流接触器输出启用默认关闭 */
        s_chargepile_config_info.function_enable.acrelay_out = 0x00;
    }
    if(s_chargepile_config_info.function_enable.fan_out > 0x01){           /* 风扇输出启用默认关闭 */
        s_chargepile_config_info.function_enable.fan_out = 0x00;
    }
    if(s_chargepile_config_info.function_enable.elock_in > 0x01){          /* 电子锁输入检测默认开启 */
        s_chargepile_config_info.function_enable.elock_in = 0x01;
    }
    if(s_chargepile_config_info.function_enable.protectlight_in > 0x01){     /* 防雷器输入检测默认关闭 */
        s_chargepile_config_info.function_enable.protectlight_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.gunsite_in > 0x01){          /* 枪座输入检测默认关闭 */
        s_chargepile_config_info.function_enable.gunsite_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.circuit_breaker_in > 0x01){  /* 断路器输入检测默认关闭 */
        s_chargepile_config_info.function_enable.circuit_breaker_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.flood_in > 0x01){          /* 水浸输入检测默认关闭 */
        s_chargepile_config_info.function_enable.flood_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.smoke_in > 0x01){          /* 烟感输入检测默认关闭 */
        s_chargepile_config_info.function_enable.smoke_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.pour_in > 0x01){          /* 倾倒输入检测默认关闭 */
        s_chargepile_config_info.function_enable.pour_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.liquid_in > 0x01){          /* 液冷输入检测默认关闭 */
        s_chargepile_config_info.function_enable.liquid_in = 0x00;
    }
    if(s_chargepile_config_info.function_enable.fuse_in > 0x01){          /* 熔断器输入检测默认关闭 */
        s_chargepile_config_info.function_enable.fuse_in = 0x00;
    }

    if(s_chargepile_config_info.state_reversal.emergency_stop > 0x01){   /* 急停默认不取反 */
        s_chargepile_config_info.state_reversal.emergency_stop = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.gate > 0x01){   /* 门禁默认不取反 */
        s_chargepile_config_info.state_reversal.gate = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.acrelay > 0x01){   /* 交流接触器默认不取反 */
        s_chargepile_config_info.state_reversal.acrelay = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.dcrelay > 0x01){   /* 直流继电器默认不取反 */
        s_chargepile_config_info.state_reversal.dcrelay = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.parallel_relay > 0x01){   /* 并联继电器默认不取反 */
        s_chargepile_config_info.state_reversal.parallel_relay = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.fan > 0x01){   /* 风扇默认不取反 */
        s_chargepile_config_info.state_reversal.fan = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.elock > 0x01){   /* 电子锁默认不取反 */
        s_chargepile_config_info.state_reversal.elock = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.protectlight > 0x01){   /* 防雷器默认不取反 */
        s_chargepile_config_info.state_reversal.protectlight = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.gunsite > 0x01){   /* 枪座默认不取反 */
        s_chargepile_config_info.state_reversal.gunsite = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.circuit_breaker > 0x01){   /* 断路器默认不取反 */
        s_chargepile_config_info.state_reversal.circuit_breaker = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.flood > 0x01){   /* 水浸默认不取反 */
        s_chargepile_config_info.state_reversal.flood = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.smoke > 0x01){   /* 烟感默认不取反 */
        s_chargepile_config_info.state_reversal.smoke = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.pour > 0x01){   /* 倾倒默认不取反 */
        s_chargepile_config_info.state_reversal.pour = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.liquid > 0x01){   /* 液冷默认不取反 */
        s_chargepile_config_info.state_reversal.liquid = 0x00;
    }
    if(s_chargepile_config_info.state_reversal.fuse > 0x01){   /* 熔断器默认不取反 */
        s_chargepile_config_info.state_reversal.fuse = 0x00;
    }

    if(s_chargepile_config_info.function_enable.offline_billing == 0x01){        /* 离线计费第一优先 */
        s_chargepile_config_info.function_enable.plug_charge = 0x00;
    }else if(s_chargepile_config_info.function_enable.plug_charge == 0x01){      /* 即插即充第二优先 */
        s_chargepile_config_info.function_enable.offline_billing = 0x00;
    }else if(s_chargepile_config_info.network.nettype == CP_NETTYPE_OFFLINE){    /* 离线模式第三优先 */
        s_chargepile_config_info.function_enable.offline_billing = 0x00;
        s_chargepile_config_info.function_enable.plug_charge = 0x00;
    }

    single_module_power = s_chargepile_config_info.config_para.module_rated_outvolt *s_chargepile_config_info.config_para.module_rated_limit_curr;
    for(uint8_t count = 0; count < s_chargepile_config_info.config_info.module_group_num; count++){
        s_system_power_max += single_module_power *s_chargepile_config_info.config_info.module_num_singlegroup[count];
    }

#ifdef APP_INCLUDE_NET
    if((s_chargepile_config_info.config_info.system_power_total > s_system_power_max) ||
            (s_chargepile_config_info.config_info.system_power_total < (s_system_power_max /100))){   /** 最小为总功率的1% */
        s_chargepile_config_info.config_info.system_power_total = s_system_power_max;
    }
#else
    s_chargepile_config_info.config_info.system_power_total = s_system_power_max;
#endif /* APP_INCLUDE_NET */
    rt_kprintf("system_power_total(%d, %d)\n", s_chargepile_config_info.config_info.system_power_total, s_system_power_max);

    for(uint8_t count = 0x00; count < CP_INFO_VIN_WHITELIST_NUM_MAX; count++){
        if(sys_string_is_pure_digital_alphabet((const char*)s_chargepile_config_info.config_info.vin_whitelist[count], VIN_CODE_LENGTH_MAX) == 0x00){
            memset(s_chargepile_config_info.config_info.vin_whitelist[count], 0x00, sizeof(s_chargepile_config_info.config_info.vin_whitelist[count]));
        }
    }

    for(uint8_t count = 0x00; count < CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX; count++){
        valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number[count]);
        valid_len = valid_len > strlen((char*)s_chargepile_config_info.config_info.card_whitelist.card_number[count]) ? \
                strlen((char*)s_chargepile_config_info.config_info.card_whitelist.card_number[count]) : valid_len;

        if(sys_string_contain_ctrl_char((const char*)s_chargepile_config_info.config_info.card_whitelist.card_number[count], valid_len)){
            memset(s_chargepile_config_info.config_info.card_whitelist.card_number[count], 0x00, sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number[count]));
        }
    }

    if(s_chargepile_config_info.network.port == 0x00 || s_chargepile_config_info.network.port == 0xFFFF){
        s_chargepile_config_info.network.port = CP_PORT_DEFAULT;
        memset(s_chargepile_config_info.network.domain, 0x00, sizeof(s_chargepile_config_info.network.domain));

        valid_len = sizeof(s_chargepile_config_info.network.domain);
        valid_len = valid_len > strlen((char*)CP_DOMAIN_DEFAULT) ? strlen((char*)CP_DOMAIN_DEFAULT) : valid_len;
        memcpy(s_chargepile_config_info.network.domain, CP_DOMAIN_DEFAULT, valid_len);
    }else{
        valid_len = sizeof(s_chargepile_config_info.network.domain);
        valid_len = valid_len > strlen((char*)s_chargepile_config_info.network.domain) ? \
                strlen((char*)s_chargepile_config_info.network.domain) : valid_len;

        if(sys_string_contain_ctrl_char((const char*)s_chargepile_config_info.network.domain, valid_len)){
            s_chargepile_config_info.network.port = CP_PORT_DEFAULT;
            memset(s_chargepile_config_info.network.domain, 0x00, sizeof(s_chargepile_config_info.network.domain));

            valid_len = sizeof(s_chargepile_config_info.network.domain);
            valid_len = valid_len > strlen((char*)CP_DOMAIN_DEFAULT) ? strlen((char*)CP_DOMAIN_DEFAULT) : valid_len;
            memcpy(s_chargepile_config_info.network.domain, CP_DOMAIN_DEFAULT, valid_len);
        }
    }

    if(strlen((char*)s_chargepile_config_info.config_info.qrcode_prefix) <= 0x02){
        s_chargepile_config_info.config_info.prefix_length = strlen(CP_QRCODE_PREFIX_DEFAULT) + 0x02;
        memset(s_chargepile_config_info.config_info.qrcode_prefix, 0x00, sizeof(s_chargepile_config_info.config_info.qrcode_prefix));
        s_chargepile_config_info.config_info.qrcode_prefix[0x00] = CP_QRCODE_CONFIG_FORMAT_DEFAULT;
        s_chargepile_config_info.config_info.qrcode_prefix[0x01] = CP_QRCODE_GENERATE_FORMAT_DEFAULT;

        valid_len = (sizeof(s_chargepile_config_info.config_info.qrcode_prefix) - 0x02);
        valid_len = valid_len > strlen((char*)CP_QRCODE_PREFIX_DEFAULT) ? strlen((char*)CP_QRCODE_PREFIX_DEFAULT) : valid_len;
        memcpy(&s_chargepile_config_info.config_info.qrcode_prefix[0x02], CP_QRCODE_PREFIX_DEFAULT, valid_len);
    }else{
        valid_len = (sizeof(s_chargepile_config_info.config_info.qrcode_prefix) - 0x02);
        valid_len = valid_len > strlen((char*)&s_chargepile_config_info.config_info.qrcode_prefix[0x02]) ? \
                strlen((char*)&s_chargepile_config_info.config_info.qrcode_prefix[0x02]) : valid_len;

        if(sys_string_contain_ctrl_char((const char*)&s_chargepile_config_info.config_info.qrcode_prefix[0x02], valid_len)){
            s_chargepile_config_info.config_info.prefix_length = strlen(CP_QRCODE_PREFIX_DEFAULT) + 0x02;
            memset(s_chargepile_config_info.config_info.qrcode_prefix, 0x00, sizeof(s_chargepile_config_info.config_info.qrcode_prefix));
            s_chargepile_config_info.config_info.qrcode_prefix[0x00] = CP_QRCODE_CONFIG_FORMAT_DEFAULT;
            s_chargepile_config_info.config_info.qrcode_prefix[0x01] = CP_QRCODE_GENERATE_FORMAT_DEFAULT;

            valid_len = (sizeof(s_chargepile_config_info.config_info.qrcode_prefix) - 0x02);
            valid_len = valid_len > strlen((char*)CP_QRCODE_PREFIX_DEFAULT) ? strlen((char*)CP_QRCODE_PREFIX_DEFAULT) : valid_len;
            memcpy(&s_chargepile_config_info.config_info.qrcode_prefix[0x02], CP_QRCODE_PREFIX_DEFAULT, valid_len);
        }
    }


    valid_len = sizeof(s_chargepile_config_info.pile_info.pile_number);
    valid_len = valid_len > strlen((char*)s_chargepile_config_info.pile_info.pile_number) ? \
            strlen((char*)s_chargepile_config_info.pile_info.pile_number) : valid_len;

    if(sys_string_contain_ctrl_char((const char*)&s_chargepile_config_info.pile_info.pile_number, valid_len)){
        s_chargepile_config_info.pile_info.pile_num_len = 0x00;
        memset(s_chargepile_config_info.pile_info.pile_number, 0x00, sizeof(s_chargepile_config_info.pile_info.pile_number));

#if 0
        valid_len = sizeof(s_chargepile_config_info.pile_info.pile_number);
        valid_len = valid_len > strlen((char*)CP_PILE_NUMBER_DEFAULT) ? strlen((char*)CP_PILE_NUMBER_DEFAULT) : valid_len;
        memcpy(&s_chargepile_config_info.pile_info.pile_number, CP_PILE_NUMBER_DEFAULT, valid_len);
#endif
    }


    valid_len = sizeof(s_chargepile_config_info.pile_info.help_number);
    valid_len = valid_len > strlen((char*)s_chargepile_config_info.pile_info.help_number) ? \
            strlen((char*)s_chargepile_config_info.pile_info.help_number) : valid_len;

    if(sys_string_contain_ctrl_char((const char*)&s_chargepile_config_info.pile_info.help_number, valid_len)){
        s_chargepile_config_info.pile_info.help_number_len = strlen(CP_HELP_PHONE_DEFAULT);
        memset(s_chargepile_config_info.pile_info.help_number, 0x00, sizeof(s_chargepile_config_info.pile_info.help_number));
#if 0
        valid_len = sizeof(s_chargepile_config_info.pile_info.help_number);
        valid_len = valid_len > strlen((char*)CP_HELP_PHONE_DEFAULT) ? strlen((char*)CP_HELP_PHONE_DEFAULT) : valid_len;
        memcpy(&s_chargepile_config_info.pile_info.help_number, CP_HELP_PHONE_DEFAULT, valid_len);
#endif
    }

    valid_len = sizeof(s_chargepile_config_info.pile_info.user_identity);
    valid_len = valid_len > strlen((char*)s_chargepile_config_info.pile_info.user_identity) ? \
            strlen((char*)s_chargepile_config_info.pile_info.user_identity) : valid_len;

    if(sys_string_contain_ctrl_char((const char*)&s_chargepile_config_info.pile_info.user_identity, valid_len)){
#ifdef CP_QRCODE_CONFIG_USING_SGCC
        memset(s_chargepile_config_info.pile_info.user_identity, 0x00, sizeof(s_chargepile_config_info.pile_info.user_identity));
        valid_len = sizeof(s_chargepile_config_info.pile_info.user_identity);
        valid_len = valid_len > strlen((char*)CP_QRCODE_PARA_VENDOR_CODE) ? strlen((char*)CP_QRCODE_PARA_VENDOR_CODE) : valid_len;
        memcpy(&s_chargepile_config_info.pile_info.user_identity, CP_QRCODE_PARA_VENDOR_CODE, valid_len);
#endif /* CP_QRCODE_CONFIG_USING_SGCC */
    }
    valid_len = sizeof(s_chargepile_config_info.config_info.register_code);
    valid_len = valid_len > strlen((char*)s_chargepile_config_info.config_info.register_code) ? \
            strlen((char*)s_chargepile_config_info.config_info.register_code) : valid_len;

    if(sys_string_contain_ctrl_char((const char*)&s_chargepile_config_info.config_info.register_code, valid_len)){
        memset(s_chargepile_config_info.config_info.register_code, 0x00, sizeof(s_chargepile_config_info.config_info.register_code));
#if 0
        valid_len = sizeof(s_chargepile_config_info.config_info.register_code);
        valid_len = valid_len > strlen((char*)CP_QRCODE_PARA_VENDOR_CODE) ? strlen((char*)CP_QRCODE_PARA_VENDOR_CODE) : valid_len;
        memcpy(&s_chargepile_config_info.config_info.register_code, CP_QRCODE_PARA_VENDOR_CODE, valid_len);
#endif
    }

    valid_len = sizeof(s_chargepile_config_info.config_info.screen_password);
    valid_len = valid_len > strlen((char*)s_chargepile_config_info.config_info.screen_password) ? \
            strlen((char*)s_chargepile_config_info.config_info.screen_password) : valid_len;

    if(sys_string_contain_ctrl_char((const char*)&s_chargepile_config_info.config_info.screen_password, valid_len)){
        memset(s_chargepile_config_info.config_info.screen_password, 0x00, sizeof(s_chargepile_config_info.config_info.screen_password));
        memcpy(&s_chargepile_config_info.config_info.screen_password, CP_SCREEN_PASSWORD_DEFAULT, strlen(CP_SCREEN_PASSWORD_DEFAULT));
    }

    for(uint8_t count = 0x00; count < 0x02; count++){
        if(sys_string_contain_ctrl_char((const char*)s_chargepile_config_info.config_info.meter_address[count], (CP_INFO_METER_ADDRESS_LEN_MAX - 0x01))){
            memset(s_chargepile_config_info.config_info.meter_address[count], 0x00, CP_INFO_METER_ADDRESS_LEN_MAX);
            memset(s_chargepile_config_info.config_info.meter_address[count], 'A', (CP_INFO_METER_ADDRESS_LEN_MAX - 0x01));
        }
    }

#ifdef CP_USING_OFFLINE_BILLING
    /***************************************************[离线计费部分]*****************************************************/
    /***************************************************[离线计费部分]*****************************************************/
    for(uint8_t period = 0x00; period < CP_PERIOD_MAX; period++){
        if((s_chargepile_config_info.billing_rule.period_price[period].elect < CP_PERIOD_ELECT_PRICE_MIN) ||
                (s_chargepile_config_info.billing_rule.period_price[period].elect > CP_PERIOD_ELECT_PRICE_MAX)){
            s_chargepile_config_info.billing_rule.period_price[period].elect = CP_PERIOD_ELECT_PRICE_DEF;
        }
        if((s_chargepile_config_info.billing_rule.period_price[period].service < CP_PERIOD_SERVICE_PRICE_MIN) ||
                (s_chargepile_config_info.billing_rule.period_price[period].service > CP_PERIOD_SERVICE_PRICE_MAX)){
            s_chargepile_config_info.billing_rule.period_price[period].service = CP_PERIOD_SERVICE_PRICE_DEF;
        }
        if((s_chargepile_config_info.billing_rule.period_price[period].delay < CP_PERIOD_DELAY_PRICE_MIN) ||
                (s_chargepile_config_info.billing_rule.period_price[period].delay > CP_PERIOD_DELAY_PRICE_MAX)){
            s_chargepile_config_info.billing_rule.period_price[period].delay = CP_PERIOD_DELAY_PRICE_DEF;
        }

        if((s_chargepile_config_info.billing_rule.rate_number[period] < CP_RATED_TYPE_MIN) ||
                (s_chargepile_config_info.billing_rule.rate_number[period] > CP_RATED_TYPE_MAX)){
            s_chargepile_config_info.billing_rule.rate_number[period] = CP_PERIOD_RATED_NUMBER_DEFAULT;
        }
    }
    /************************************************************************************************************************/
    if((s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP_SHARP] < CP_SHARP_SHARP_RATED_ELECT_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP_SHARP] > CP_SHARP_SHARP_RATED_ELECT_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP_SHARP] = CP_SHARP_SHARP_RATED_ELECT_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP_SHARP] < CP_SHARP_SHARP_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP_SHARP] > CP_SHARP_SHARP_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP_SHARP] = CP_SHARP_SHARP_RATED_DELAY_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP_SHARP] < CP_SHARP_SHARP_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP_SHARP] > CP_SHARP_SHARP_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP_SHARP] = CP_SHARP_SHARP_RATED_DELAY_PRICE_DEF;
    }
    /************************************************************************************************************************/
    if((s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP] < CP_SHARP_RATED_ELECT_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP] > CP_SHARP_RATED_ELECT_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP] = CP_SHARP_RATED_ELECT_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP] < CP_SHARP_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP] > CP_SHARP_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP] = CP_SHARP_RATED_DELAY_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP] < CP_SHARP_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP] > CP_SHARP_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP] = CP_SHARP_RATED_DELAY_PRICE_DEF;
    }
    /************************************************************************************************************************/
    if((s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_PEAK] < CP_PEAK_RATED_ELECT_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_PEAK] > CP_PEAK_RATED_ELECT_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_PEAK] = CP_PEAK_RATED_ELECT_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_PEAK] < CP_PEAK_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_PEAK] > CP_PEAK_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_PEAK] = CP_PEAK_RATED_DELAY_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_PEAK] < CP_PEAK_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_PEAK] > CP_PEAK_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_PEAK] = CP_PEAK_RATED_DELAY_PRICE_DEF;
    }
    /************************************************************************************************************************/
    if((s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_FLAT] < CP_FLAT_RATED_ELECT_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_FLAT] > CP_FLAT_RATED_ELECT_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_FLAT] = CP_FLAT_RATED_ELECT_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_FLAT] < CP_FLAT_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_FLAT] > CP_FLAT_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_FLAT] = CP_FLAT_RATED_DELAY_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_FLAT] < CP_FLAT_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_FLAT] > CP_FLAT_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_FLAT] = CP_FLAT_RATED_DELAY_PRICE_DEF;
    }
    /************************************************************************************************************************/
    if((s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_VALLEY] < CP_VALLEY_RATED_ELECT_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_VALLEY] > CP_VALLEY_RATED_ELECT_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_VALLEY] = CP_VALLEY_RATED_ELECT_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_VALLEY] < CP_VALLEY_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_VALLEY] > CP_VALLEY_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_VALLEY] = CP_VALLEY_RATED_DELAY_PRICE_DEF;
    }

    if((s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_VALLEY] < CP_VALLEY_RATED_DELAY_PRICE_MIN) ||
            (s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_VALLEY] > CP_VALLEY_RATED_DELAY_PRICE_MAX)){
        s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_VALLEY] = CP_VALLEY_RATED_DELAY_PRICE_DEF;
    }
    /************************************************************************************************************************/

    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP_SHARP] = s_chargepile_config_info.billing_rule.rate_service_price[CP_PERIOD_RATED_NUMBER_DEFAULT];
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP] = s_chargepile_config_info.billing_rule.rate_service_price[CP_PERIOD_RATED_NUMBER_DEFAULT];
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_PEAK] = s_chargepile_config_info.billing_rule.rate_service_price[CP_PERIOD_RATED_NUMBER_DEFAULT];
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_FLAT] = s_chargepile_config_info.billing_rule.rate_service_price[CP_PERIOD_RATED_NUMBER_DEFAULT];
    s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_VALLEY] = s_chargepile_config_info.billing_rule.rate_service_price[CP_PERIOD_RATED_NUMBER_DEFAULT];

    if(sys_period_time_format_valid(s_chargepile_config_info.billing_rule.time) != 0x01){
        LOG_W("offline billing period time invalid");
        sys_period_time_resume_default(s_chargepile_config_info.billing_rule.time, sizeof(s_chargepile_config_info.billing_rule.time));
    }else{
        for(i = 0x00; i < CP_RATED_TYPE_NUM_MAX; i++){
            for(j = 0x00; j < CP_RATED_TYPE_PERIOD_NUM; j++){
                if(s_chargepile_config_info.billing_rule.time[i][j].shour < 24){
                    time[valid_count] = s_chargepile_config_info.billing_rule.time[i][j];
                    valid_count++;
                }
            }
        }
        if(sys_period_time_continuous_valid(time, sizeof(time), valid_count) != 0x01){
            LOG_W("offline billing period not continuous");
            sys_period_time_resume_default(s_chargepile_config_info.billing_rule.time, sizeof(s_chargepile_config_info.billing_rule.time));
        }
    }

    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][i].rate_number = CP_RATED_TYPE_SHARP_SHARP;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][i].rate_number = CP_RATED_TYPE_SHARP;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][i].rate_number = CP_RATED_TYPE_PEAK;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][i].rate_number = CP_RATED_TYPE_FLAT;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][i].rate_number = CP_RATED_TYPE_VALLEY;
    }

    rt_kprintf("=============================================================\n");
    rt_kprintf("sharp sharp price:\n");
    rt_kprintf("elect:%d\n", s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP_SHARP]);
    rt_kprintf("service:%d\n", s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP_SHARP]);
    rt_kprintf("delay:%d\n", s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP_SHARP]);
    rt_kprintf("=============================================================\n");
    rt_kprintf("sharp price:\n");
    rt_kprintf("elect:%d\n", s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_SHARP]);
    rt_kprintf("service:%d\n", s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_SHARP]);
    rt_kprintf("delay:%d\n", s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_SHARP]);
    rt_kprintf("=============================================================\n");
    rt_kprintf("peak price:\n");
    rt_kprintf("elect:%d\n", s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_PEAK]);
    rt_kprintf("service:%d\n", s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_PEAK]);
    rt_kprintf("delay:%d\n", s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_PEAK]);
    rt_kprintf("=============================================================\n");
    rt_kprintf("flat price:\n");
    rt_kprintf("elect:%d\n", s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_FLAT]);
    rt_kprintf("service:%d\n", s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_FLAT]);
    rt_kprintf("delay:%d\n", s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_FLAT]);
    rt_kprintf("=============================================================\n");
    rt_kprintf("valley price:\n");
    rt_kprintf("elect:%d\n", s_chargepile_config_info.billing_rule.rate_elect_price[CP_RATED_TYPE_VALLEY]);
    rt_kprintf("service:%d\n", s_chargepile_config_info.billing_rule.rate_service_price[CP_RATED_TYPE_VALLEY]);
    rt_kprintf("delay:%d\n", s_chargepile_config_info.billing_rule.rate_delay_price[CP_RATED_TYPE_VALLEY]);
    rt_kprintf("=============================================================\n");

    rt_kprintf("sharp sharp 00[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][0].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][0].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][0].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][0].emin);

    rt_kprintf("sharp sharp 11[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][1].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][1].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][1].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP_SHARP][1].emin);

    rt_kprintf("sharp 00[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][0].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][0].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][0].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][0].emin);

    rt_kprintf("sharp 11[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][1].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][1].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][1].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_SHARP][1].emin);

    rt_kprintf("peak 00[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][0].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][0].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][0].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][0].emin);

    rt_kprintf("peak 11[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][1].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][1].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][1].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_PEAK][1].emin);

    rt_kprintf("flat 00[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][0].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][0].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][0].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][0].emin);

    rt_kprintf("flat 11[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][1].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][1].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][1].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_FLAT][1].emin);

    rt_kprintf("valley 00[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][0].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][0].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][0].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][0].emin);

    rt_kprintf("valley 11[%d:%d-%d:%d]\n", s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][1].shour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][1].smin,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][1].ehour,
            s_chargepile_config_info.billing_rule.time[CP_RATED_TYPE_VALLEY][1].emin);

    /********************************************************************************************************/
    /********************************************************************************************************/
#endif /* CP_USING_OFFLINE_BILLING */
    /*************************************************** 模式选择 *****************************************************/
    /*************************************************** 模式选择 *****************************************************/
    for(uint8_t i = 0x00; i < sizeof(s_chargepile_config_info.function_enable.current_mode); i++){
        switch(s_chargepile_config_info.function_enable.current_mode[i]){
        case CP_MODE_CHARGE_FULL:
            s_chargepile_config_info.config_para.mode_parameter[i] = 0x00;
            break;
        case CP_MODE_LIMIT_RESERVATION:
            if((s_chargepile_config_info.config_para.mode_parameter[i] > CP_MODE_PARA_RESERVATION_MAX) || \
                    (s_chargepile_config_info.config_para.mode_parameter[i] < CP_MODE_PARA_RESERVATION_MIN)){
                s_chargepile_config_info.config_para.mode_parameter[i] = CP_MODE_PARA_RESERVATION_DEF;
            }
            break;
        default:
            s_chargepile_config_info.function_enable.current_mode[i] = CP_MODE_CHARGE_FULL;
            s_chargepile_config_info.config_para.mode_parameter[i] = 0x00;
            break;
        }
    }
    /********************************************************************************************************/
    /********************************************************************************************************/
    return 0;
}

void *sys_get_module_config_info(void)
{
    s_module_info.module_model = s_chargepile_config_info.config_info.module_model;
    s_module_info.module_group_num = s_chargepile_config_info.config_info.module_group_num;
    memcpy(s_module_info.module_num_singlegroup, s_chargepile_config_info.config_info.module_num_singlegroup, sizeof(s_module_info.module_num_singlegroup));

    if(s_module_info.module_group_num > MODULE_GROUP_NUMBER_MAX){
        s_module_info.module_group_num = MODULE_GROUP_NUMBER_DEFAULT;
    }
    if(s_module_info.module_model > MODULE_MODEL_NUMBER){
        s_module_info.module_model = MODULE_MODEL_DEFAULT;
    }
    for(uint8_t count = 0; count < s_module_info.module_group_num; count++){
        if(s_module_info.module_num_singlegroup[count] > MODULE_NUMBER_SINGLE_MAX){
            s_module_info.module_num_singlegroup[count] = MODULE_NUMBER_SINGLE_DEFAULT;
        }
    }
    return &s_module_info;
}

uint8_t sys_get_single_group_module_num(uint8_t group)
{
    if(group >= MODULE_GROUP_NUMBER_MAX){
        return 0x00;
    }
    return s_chargepile_config_info.config_info.module_num_singlegroup[group];
}

int16_t sys_get_power_percent(void)
{
    int16_t percent = s_chargepile_config_info.config_info.system_power_total /s_system_power_max;
    if(s_chargepile_config_info.config_info.system_power_total >= s_system_power_max){
        percent = 1000;
    }else{
        percent = s_chargepile_config_info.config_info.system_power_total *1000 /s_system_power_max;
    }

    return percent;
}

uint32_t sys_percent_convert_to_power(uint16_t percent)
{
    uint32_t single_module_power = 0x00, power_max = 0x00;       /* 单个模块能输出的最大(额定)功率 */

    single_module_power = s_chargepile_config_info.config_para.module_rated_outvolt *s_chargepile_config_info.config_para.module_rated_limit_curr;
    for(uint8_t count = 0; count < s_chargepile_config_info.config_info.module_group_num; count++){
        power_max += single_module_power *s_chargepile_config_info.config_info.module_num_singlegroup[count];
    }
    s_system_power_max = power_max;

    return (s_system_power_max *percent /1000);
}

uint32_t sys_query_system_max_power(void)
{
    return s_system_power_max;   /** 系统最大功率(W) */
}

uint8_t sys_get_module_model(void)
{
    return s_chargepile_config_info.config_info.module_model;
}

uint8_t sys_get_module_group_num(void)
{
    return s_chargepile_config_info.config_info.module_group_num;
}

uint8_t* sys_get_module_num_single_group(void)
{
    return s_chargepile_config_info.config_info.module_num_singlegroup;
}

/**********************************************[VIN白名单相关]********************************************************/
/**********************************************[VIN白名单相关]********************************************************/
int32_t sys_vin_whitelists_storage(void)
{
    return sys_storage_config_item();
}

uint8_t *sys_vin_code_get(uint8_t index)
{
    if(index >= CP_INFO_VIN_WHITELIST_NUM_MAX){
        return NULL;
    }

    return s_chargepile_config_info.config_info.vin_whitelist[index];
}

int32_t sys_vin_whitelists_add(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < VIN_CODE_LENGTH_MIN) || (len > VIN_CODE_LENGTH_MAX + 0x01)){
        return -0x01;
    }

    uint8_t index = 0x00, free_index = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.vin_whitelist[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_VIN_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.vin_whitelist[index], valid_len) == 0x00){
            for(free_index = (index + 0x01); free_index < CP_INFO_VIN_WHITELIST_NUM_MAX; free_index++){
                if(strlen((const char *)&(s_chargepile_config_info.config_info.vin_whitelist[free_index])) < VIN_CODE_LENGTH_MIN){
                    break;
                }
                memcpy(s_chargepile_config_info.config_info.vin_whitelist[free_index - 0x01],  \
                        s_chargepile_config_info.config_info.vin_whitelist[free_index], VIN_CODE_LENGTH_DEF);
            }
            memset(s_chargepile_config_info.config_info.vin_whitelist[free_index - 0x01], '\0', (VIN_CODE_LENGTH_DEF + 0x01));
        }
    }

    for(index = 0x00; index < CP_INFO_VIN_WHITELIST_NUM_MAX; index++){
        if(strlen((const char *)&(s_chargepile_config_info.config_info.vin_whitelist[index])) < VIN_CODE_LENGTH_MIN){
            break; /* 不足 最小长度不算 */
        }
    }
    if(index == CP_INFO_VIN_WHITELIST_NUM_MAX){
        /* 列表已经满了，把最旧的卡号移出去，把最新的位置让出来放入最新的卡号 */
        for(index = 0x00; index < (CP_INFO_VIN_WHITELIST_NUM_MAX - 0x01); index++){
            memcpy(s_chargepile_config_info.config_info.vin_whitelist[index],  \
                    s_chargepile_config_info.config_info.vin_whitelist[index + 0x01], (VIN_CODE_LENGTH_DEF + 0x01));
        }
        memset(s_chargepile_config_info.config_info.vin_whitelist[index], '\0', (VIN_CODE_LENGTH_DEF + 0x01));
        memcpy(s_chargepile_config_info.config_info.vin_whitelist[index], data, len); /* 将卡号放入列表中 */
    }else{
        memset(s_chargepile_config_info.config_info.vin_whitelist[index], '\0', (VIN_CODE_LENGTH_DEF + 0x01));
        memcpy(s_chargepile_config_info.config_info.vin_whitelist[index], data, len); /* 将卡号放入列表中 */
    }

    return index;
}

int32_t sys_vin_whitelists_query(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < VIN_CODE_LENGTH_MIN) || (len > VIN_CODE_LENGTH_MAX + 0x01)){
        return -0x01;
    }
    if(strlen((char*)data) < VIN_CODE_LENGTH_MIN){
        return -0x01;
    }

    uint8_t index = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.vin_whitelist[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_VIN_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.vin_whitelist[index], valid_len) == 0x00){
            return index;
        }
    }

    return -0x01;
}

int32_t sys_vin_whitelists_delete(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < VIN_CODE_LENGTH_MIN) || (len > VIN_CODE_LENGTH_MAX + 0x01)){
        return -0x01;
    }

    uint8_t index = 0x00, count = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.vin_whitelist[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_VIN_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.vin_whitelist[index], valid_len) == 0x00){
            count = index;
            if(index != (CP_INFO_VIN_WHITELIST_NUM_MAX - 0x01)){
                for(count = index; count < (CP_INFO_VIN_WHITELIST_NUM_MAX - 0x01); count++){
                    if(strlen((const char *)&(s_chargepile_config_info.config_info.vin_whitelist[count])) < VIN_CODE_LENGTH_MIN){
                        break;
                    }
                    memcpy(s_chargepile_config_info.config_info.vin_whitelist[count],
                            s_chargepile_config_info.config_info.vin_whitelist[count + 0x01], VIN_CODE_LENGTH_DEF);
                }
            }
            memset(s_chargepile_config_info.config_info.vin_whitelist[count], '\0', (VIN_CODE_LENGTH_DEF + 0x01));
            break;
        }
    }

    if(index >= CP_INFO_VIN_WHITELIST_NUM_MAX){
        return -0x01;
    }
    return index;
}

int32_t sys_vin_whitelists_clear(void)
{
    memset(s_chargepile_config_info.config_info.vin_whitelist, 0x00, sizeof(s_chargepile_config_info.config_info.vin_whitelist));
    return 0x00;
}

/**********************************************[卡号白名单相关]********************************************************/
/**********************************************[卡号白名单相关]********************************************************/
int32_t sys_card_number_whitelists_storage(void)
{
    return sys_storage_config_item();
}

uint8_t *sys_card_number_get(uint8_t index)
{
    if(index >= CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX){
        return NULL;
    }

    return s_chargepile_config_info.config_info.card_whitelist.card_number[index];
}

int32_t sys_card_number_whitelists_add(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < CARD_NUMBER_LENGTH_MIN) || (len > CARD_NUMBER_LENGTH_MAX + 0x01)){
        return -0x01;
    }
    if(strlen((const char *)data) < CARD_NUMBER_LENGTH_MIN){
        return -0x01;
    }

    uint8_t index = 0x00, free_index = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.card_whitelist.card_number[index], valid_len) == 0x00){
            for(free_index = (index + 0x01); free_index < CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX; free_index++){
                if(strlen((const char *)&(s_chargepile_config_info.config_info.card_whitelist.card_number[free_index])) < CARD_NUMBER_LENGTH_MIN){
                    break;
                }
                memcpy(s_chargepile_config_info.config_info.card_whitelist.card_number[free_index - 0x01],  \
                        s_chargepile_config_info.config_info.card_whitelist.card_number[free_index], CARD_NUMBER_LENGTH_DEF);
            }
            memset(s_chargepile_config_info.config_info.card_whitelist.card_number[free_index - 0x01], '\0', (CARD_NUMBER_LENGTH_DEF + 0x01));
        }
    }

    for(index = 0x00; index < CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX; index++){
        if(strlen((const char *)&(s_chargepile_config_info.config_info.card_whitelist.card_number[index])) < CARD_NUMBER_LENGTH_MIN){
            break; /* 不足 最小长度不算 */
        }
    }
    if(index == CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX){
        /* 列表已经满了，把最旧的卡号移出去，把最新的位置让出来放入最新的卡号 */
        for(index = 0x00; index < (CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX - 0x01); index++){
            memcpy(s_chargepile_config_info.config_info.card_whitelist.card_number[index],  \
                    s_chargepile_config_info.config_info.card_whitelist.card_number[index + 0x01], (CARD_NUMBER_LENGTH_DEF + 0x01));
        }
        memset(s_chargepile_config_info.config_info.card_whitelist.card_number[index], '\0', (CARD_NUMBER_LENGTH_DEF + 0x01));
        memcpy(s_chargepile_config_info.config_info.card_whitelist.card_number[index], data, len); /* 将卡号放入列表中 */
    }else{
        memset(s_chargepile_config_info.config_info.card_whitelist.card_number[index], '\0', (CARD_NUMBER_LENGTH_DEF + 0x01));
        memcpy(s_chargepile_config_info.config_info.card_whitelist.card_number[index], data, len); /* 将卡号放入列表中 */
    }

    return index;
}

int32_t sys_card_number_whitelists_query(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < CARD_NUMBER_LENGTH_MIN) || (len > CARD_NUMBER_LENGTH_MAX + 0x01)){
        return -0x01;
    }
    if(strlen((char*)data) < CARD_NUMBER_LENGTH_MIN){
        return -0x01;
    }

    uint8_t index = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.card_whitelist.card_number[index], valid_len) == 0x00){
            return index;
        }
    }
    return -0x01;
}

int32_t sys_card_number_whitelists_delete(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < CARD_NUMBER_LENGTH_MIN) || (len > CARD_NUMBER_LENGTH_MAX + 0x01)){
        return -0x01;
    }

    uint8_t index = 0x00, count = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.card_whitelist.card_number[index], valid_len) == 0x00){
            count = index;
            if(index != (CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX - 0x01)){
                for(count = index; count < (CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX - 0x01); count++){
                    if(strlen((const char *)&(s_chargepile_config_info.config_info.card_whitelist.card_number[count])) < CARD_NUMBER_LENGTH_MIN){
                        break;
                    }
                    memcpy(s_chargepile_config_info.config_info.card_whitelist.card_number[count],
                            s_chargepile_config_info.config_info.card_whitelist.card_number[count + 0x01], CARD_NUMBER_LENGTH_DEF);
                }
            }
            memset(s_chargepile_config_info.config_info.card_whitelist.card_number[count], '\0', (CARD_NUMBER_LENGTH_DEF + 0x01));
            break;
        }
    }

    if(index >= CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX){
        return -0x01;
    }
    return index;
}

int32_t sys_card_number_whitelists_clear(void)
{
    memset(s_chargepile_config_info.config_info.card_whitelist.card_number, 0x00, sizeof(s_chargepile_config_info.config_info.card_whitelist.card_number));
    return 0x00;
}

/**********************************************[卡UID白名单相关]********************************************************/
/**********************************************[卡UID白名单相关]********************************************************/
int32_t sys_card_uid_whitelists_storage(void)
{
    return sys_storage_config_item();
}

uint8_t *sys_card_uid_get(uint8_t index)
{
    if(index >= CP_INFO_CARD_UID_WHITELIST_NUM_MAX){
        return NULL;
    }

    return s_chargepile_config_info.config_info.card_whitelist.card_uid[index];
}

int32_t sys_card_uid_whitelists_add(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < CARD_UID_LENGTH_MIN) || (len > CARD_UID_LENGTH_MAX)){
        return -0x01;
    }

    uint8_t index = 0x00, free_index = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_uid[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_CARD_UID_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.card_whitelist.card_uid[index], valid_len) == 0x00){
            for(free_index = (index + 0x01); free_index < CP_INFO_CARD_UID_WHITELIST_NUM_MAX; free_index++){
                if(strlen((const char *)&(s_chargepile_config_info.config_info.card_whitelist.card_uid[free_index])) < CARD_UID_LENGTH_MIN){
                    break;
                }
                memcpy(s_chargepile_config_info.config_info.card_whitelist.card_uid[free_index - 0x01],  \
                        s_chargepile_config_info.config_info.card_whitelist.card_uid[free_index], CARD_UID_LENGTH_DEF);
            }
            memset(s_chargepile_config_info.config_info.card_whitelist.card_uid[free_index - 0x01], '\0', CARD_UID_LENGTH_DEF);
        }
    }

    for(index = 0x00; index < CP_INFO_CARD_UID_WHITELIST_NUM_MAX; index++){
        if(strlen((const char *)&(s_chargepile_config_info.config_info.card_whitelist.card_uid[index])) < CARD_UID_LENGTH_MIN){
            break; /* 不足 最小长度不算 */
        }
    }
    if(index == CP_INFO_CARD_UID_WHITELIST_NUM_MAX){
        /* 列表已经满了，把最旧的卡号移出去，把最新的位置让出来放入最新的卡号 */
        for(index = 0x00; index < (CP_INFO_CARD_UID_WHITELIST_NUM_MAX - 0x01); index++){
            memcpy(s_chargepile_config_info.config_info.card_whitelist.card_uid[index],  \
                    s_chargepile_config_info.config_info.card_whitelist.card_uid[index + 0x01], CARD_UID_LENGTH_DEF);
        }
        memset(s_chargepile_config_info.config_info.card_whitelist.card_uid[index], '\0', CARD_UID_LENGTH_DEF);
        memcpy(s_chargepile_config_info.config_info.card_whitelist.card_uid[index], data, len); /* 将卡号放入列表中 */
    }else{
        memset(s_chargepile_config_info.config_info.card_whitelist.card_uid[index], '\0', CARD_UID_LENGTH_DEF);
        memcpy(s_chargepile_config_info.config_info.card_whitelist.card_uid[index], data, len); /* 将卡号放入列表中 */
    }

    return index;
}

int32_t sys_card_uid_whitelists_query(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < CARD_UID_LENGTH_MIN) || (len > CARD_UID_LENGTH_MAX)){
        return -0x01;
    }

    uint8_t index = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_uid[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_CARD_UID_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.card_whitelist.card_uid[index], valid_len) == 0x00){
            return index;
        }
    }
    return -0x01;
}

int32_t sys_card_uid_whitelists_delete(uint8_t *data, uint8_t len)
{
    if(data == NULL){
        return -0x01;
    }
    if((len < CARD_UID_LENGTH_MIN) || (len > CARD_UID_LENGTH_MAX)){
        return -0x01;
    }

    uint8_t index = 0x00, count = 0x00, valid_len = sizeof(s_chargepile_config_info.config_info.card_whitelist.card_uid[0x00]);
    valid_len = valid_len > len ? len : valid_len;
    for(index = 0x00; index < CP_INFO_CARD_UID_WHITELIST_NUM_MAX; index++){
        if(memcmp(data, s_chargepile_config_info.config_info.card_whitelist.card_uid[index], valid_len) == 0x00){
            count = index;
            if(index != (CP_INFO_CARD_UID_WHITELIST_NUM_MAX - 0x01)){
                for(count = index; count < (CP_INFO_CARD_UID_WHITELIST_NUM_MAX - 0x01); count++){
                    if(strlen((const char *)&(s_chargepile_config_info.config_info.card_whitelist.card_uid[count])) < CARD_UID_LENGTH_MIN){
                        break;
                    }
                    memcpy(s_chargepile_config_info.config_info.card_whitelist.card_uid[count],
                            s_chargepile_config_info.config_info.card_whitelist.card_uid[count + 0x01], CARD_UID_LENGTH_DEF);
                }
            }
            memset(s_chargepile_config_info.config_info.card_whitelist.card_uid[count], '\0', CARD_UID_LENGTH_DEF);
            break;
        }
    }

    if(index >= CP_INFO_CARD_UID_WHITELIST_NUM_MAX){
        return -0x01;
    }
    return index;
}

int32_t sys_card_uid_whitelists_clear(void)
{
    memset(s_chargepile_config_info.config_info.card_whitelist.card_uid, 0x00, sizeof(s_chargepile_config_info.config_info.card_whitelist.card_uid));
    return 0x00;
}


/**********************************************[离线计费相关]********************************************************/
/**********************************************[离线计费相关]********************************************************/
/*********************************************************
 * 函数名        sys_period_time_format_valid
 * 功能            判断时段时间格式的合法性
 * 参数           t   时段时间信息
 * 返回           1：合法  0：空白   -1：非法
 ********************************************************/
int32_t sys_period_time_format_valid(void *t)
{
#ifdef CP_USING_OFFLINE_BILLING
    if(t == NULL){
        return -0x01;
    }

    uint8_t count = 0x00;
    struct period_time (*time)[CP_RATED_TYPE_PERIOD_NUM] = (struct period_time (*)[CP_RATED_TYPE_PERIOD_NUM])t;

    for(uint8_t i = 0x00; i < CP_RATED_TYPE_NUM_MAX; i++){
        for(uint8_t j = 0x00; j < CP_RATED_TYPE_PERIOD_NUM; j++){
            if(((time[i][j].shour >= 24) && (time[i][j].shour < 0xFF)) || ((time[i][j].ehour >= 24) && (time[i][j].ehour < 0xFF))){
                return -0x01;
            }
            if(((time[i][j].smin >= 60) && (time[i][j].smin < 0xFF)) || ((time[i][j].emin >= 60) && (time[i][j].emin < 0xFF))){
                return -0x01;
            }

            /** 有一个是0xFF, 则必须这一时间段的另外3个值也是0xFF */
            if((time[i][j].shour < 24) && (time[i][j].smin >= 60)){
                return -0x01;
            }
            if((time[i][j].shour >= 24) && (time[i][j].smin < 60)){
                return -0x01;
            }

            if((time[i][j].ehour < 24) && (time[i][j].emin >= 60)){
                return -0x01;
            }
            if((time[i][j].ehour >= 24) && (time[i][j].emin < 60)){
                return -0x01;
            }

            if((time[i][j].shour >= 0xFF) && (time[i][j].smin >= 0xFF) && (time[i][j].ehour >= 0xFF) && (time[i][j].emin >= 0xFF)){
                count++;
            }
        }
    }
    /** 值全是 0xFF, 这是新板子 */
    if(count >= (CP_RATED_TYPE_NUM_MAX *CP_RATED_TYPE_PERIOD_NUM)){
        return 0x00;
    }

    return 0x01;
#else
    return -0x01;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_period_time_continuous_valid
 * 功能            判断时段时间是否连续
 * 参数           t   时段时间信息
 *      tlen  时段时间信息长度
 *      valid_count   有效时间段个数
 * 返回           -1：重复    1：连续  0：间断
 ********************************************************/
int32_t sys_period_time_continuous_valid(void *t, uint8_t tlen, uint8_t valid_count)
{
#ifdef CP_USING_OFFLINE_BILLING
    if(t == NULL){
        return 0x00;    /** 时间间断 */
    }
    if(tlen < (sizeof(struct period_time) *CP_RATED_TYPE_NUM_MAX *CP_RATED_TYPE_PERIOD_NUM)){
        return 0x00;    /** 时间间断 */
    }
    if(!((valid_count > 0x00) && (valid_count <= CP_RATED_TYPE_NUM_MAX *CP_RATED_TYPE_PERIOD_NUM))){
        return 0x00;    /** 时间间断 */
    }

    uint8_t i = 0x00, j = 0x00, min = 0x00;
    struct period_time *time = (struct period_time*)t, temp;

    /** 选择排序，按开始时间的小时进行升序排列 */
    for(i = 0x00; i < (valid_count - 0x01); i++){
        min = i;
        for(j = i + 1; j < valid_count; j++){
            if (time[min].shour > time[j].shour){
                min = j;
            }
        }
        if(min != i){
            temp = time[min];
            time[min] = time[i];
            time[i] = temp;
        }
    }
    /** 选择排序，按开始时间的分钟进行升序排列 */
    for(i = 0x00; i < (valid_count - 0x01); i++){
        min = i;
        for(j = i + 1; j < valid_count; j++){
            if ((time[min].smin > time[j].smin) && (time[min].shour == time[j].shour)){
                min = j;
            }
        }
        if(min != i){
            temp = time[min];
            time[min] = time[i];
            time[i] = temp;
        }
    }

    for(i = 0x00; i < valid_count; i++){
        rt_kprintf("1111 period time(%d)[%d:%d-%d:%d]\n", i, time[i].shour,
                time[i].smin,
                time[i].ehour,
                time[i].emin);
    }

    /** 至少有两个时段 */
    if(valid_count >= 0x02){
        /** 判断是否连续 */
        for(i = 0x00; i < (valid_count - 0x01); i++){
            if(time[i].shour == time[i].ehour){
                if(time[i].smin >= time[i].emin){   /** 此时这个时间段内的开始小时等于结束小时、开始分钟等于结束分钟；这段时间将毫无意义(可以去掉不使用0xFF) */
                    return -0x01;    /** 时间重复 */
                }
            }else{
                if(time[i].shour > time[i].ehour){
                    return -0x01;    /** 时间重复 */
                }
            }

            if(time[i + 0x01].shour != time[i].ehour){
                if(time[i].ehour > time[i + 0x01].shour){
                    return -0x01;    /** 时间重复 */
                }else{
                    return 0x00;     /** 时间间断 */
                }
            }

            if(time[i + 0x01].smin != time[i].emin){
                if(time[i].emin > time[i + 0x01].smin){
                    return -0x01;    /** 时间重复 */
                }else{
                    return 0x00;     /** 时间间断 */
                }
            }
            /** 最后一个时间要和最开始的时间进行判断 */
            if((i + 0x01) == (valid_count - 0x01)){
                if((time[i + 0x01].emin != time[0x00].smin) || (time[i + 0x01].ehour != time[0x00].shour)){
                    if(time[i + 0x01].ehour != time[0x00].shour){
                        if((time[0x00].shour < time[i + 0x01].ehour) && (time[0x01].shour > time[i + 0x01].ehour)){
                            return -0x01;    /** 时间重复 */
                        }else{
                            return 0x00;     /** 时间间断 */
                        }
                    }

                    if(time[i + 0x01].emin != time[0x00].smin){
                        if(time[0x00].smin < time[i + 0x01].emin){
                            return -0x01;    /** 时间重复 */
                        }else{
                            return 0x00;     /** 时间间断 */
                        }
                    }
                }
            }
        }
    }
    /** 只有一个时段 */
    else{
        if((time[0x00].shour != 0x00) || (time[0x00].smin != 0x00) ||     \
                (time[0x00].ehour != 0x00) || (time[0x00].emin != 0x00)){
            return 0x00;    /** 时间间断 */
        }
    }

    return 0x01;
#else
    return 0x00;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_period_time_resume_default
 * 功能            将时段时间恢复默认值
 * 参数           t   时段时间信息
 *      tlen  时段时间信息长度
 * 返回           <=0：失败    >0：成功
 ********************************************************/
int32_t sys_period_time_resume_default(void *t, uint8_t tlen)
{
#ifdef CP_USING_OFFLINE_BILLING
    if(t == NULL){
        return 0x00;
    }
    if(tlen < (sizeof(struct period_time) *CP_RATED_TYPE_NUM_MAX *CP_RATED_TYPE_PERIOD_NUM)){
        return 0x00;
    }

    uint8_t count = 0x00, i = 0x00, j = 0x00, segment = 0x00;
    struct period_time (*time)[CP_RATED_TYPE_PERIOD_NUM] = (struct period_time (*)[CP_RATED_TYPE_PERIOD_NUM])t;

    segment = (24 *4) /(CP_RATED_TYPE_PERIOD_NUM *CP_RATED_TYPE_NUM_MAX);     /* 将一天分为96段，15分钟一时段 */
    // 96 /10 = 9, 余6，  10个可配置时间段，每个可配置的时间段均占 15 *9 = 135min   count 记录着经过了多少个时段

    for(i = 0x00; i < CP_RATED_TYPE_NUM_MAX; i++){
        for(j = 0x00; j < CP_RATED_TYPE_PERIOD_NUM; j++){
            time[i][j].shour = (15 *(count+ j *segment)) /60;
            time[i][j].smin = (15 *(count+ j *segment)) %60;
            time[i][j].ehour = (15 *(count+ (j + 0x01) *segment)) /60;
            time[i][j].emin = (15 *(count+ (j + 0x01) *segment)) %60;
        }
        count += CP_RATED_TYPE_PERIOD_NUM *segment;
    }

    i = 0x00;
    j = 0x00;

    if(CP_RATED_TYPE_PERIOD_NUM > 0x01){
        i = CP_RATED_TYPE_PERIOD_NUM - 0x01;
    }
    if(CP_RATED_TYPE_NUM_MAX > 0x01){
        j = CP_RATED_TYPE_NUM_MAX - 0x01;
    }

    time[j][i].ehour = 0x00;
    time[j][i].emin = 0x00;

    return 0x01;
#else
    return 0x00;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_get_offbilling_rate_number
 * 功能            获取当前时间费率号
 * 参数           curr_time  当前时间(s)
 * 返回           当前时间费率号
 ********************************************************/
uint8_t sys_get_offbilling_rate_number(uint32_t curr_time)
{
#ifdef CP_USING_OFFLINE_BILLING
    uint8_t i = 0x00, j = 0x00, end_hour = 0x00;
    struct tm _tm;

    sys_config_enter_critical();
    _tm = *(localtime((time_t*)&curr_time));
    sys_config_exit_critical();

    for(i = 0x00; i < CP_RATED_TYPE_NUM_MAX; i++){
        for(j = 0x00; j < CP_RATED_TYPE_PERIOD_NUM; j++){
            if((s_chargepile_config_info.billing_rule.time[i][j].shour >= 24) ||
                    (s_chargepile_config_info.billing_rule.time[i][j].ehour >= 24)){
                continue;
            }
            if((s_chargepile_config_info.billing_rule.time[i][j].smin >= 60) ||
                    (s_chargepile_config_info.billing_rule.time[i][j].emin >= 60)){
                continue;
            }

            end_hour = s_chargepile_config_info.billing_rule.time[i][j].ehour;
            if(end_hour == 0x00){  /** 结束时间是 00：00，相当于小时是 24 */
                end_hour = 24;
            }

            if((_tm.tm_hour > s_chargepile_config_info.billing_rule.time[i][j].shour) && (_tm.tm_hour < end_hour)){
                return s_chargepile_config_info.billing_rule.time[i][j].rate_number;
            }else if(_tm.tm_hour == s_chargepile_config_info.billing_rule.time[i][j].shour){
                if(s_chargepile_config_info.billing_rule.time[i][j].shour == end_hour){
                    if((_tm.tm_min >= s_chargepile_config_info.billing_rule.time[i][j].smin) &&
                            (_tm.tm_min <= s_chargepile_config_info.billing_rule.time[i][j].emin)){
                        return s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }else{
                    if(_tm.tm_min >= s_chargepile_config_info.billing_rule.time[i][j].smin){
                        return s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }
            }else if(_tm.tm_hour == end_hour){
                if(s_chargepile_config_info.billing_rule.time[i][j].shour == end_hour){
                    if((_tm.tm_min >= s_chargepile_config_info.billing_rule.time[i][j].smin) &&
                            (_tm.tm_min <= s_chargepile_config_info.billing_rule.time[i][j].emin)){
                        return s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }else{
                    if(_tm.tm_min <= s_chargepile_config_info.billing_rule.time[i][j].emin){
                        return s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }
            }
        }
    }

    return CP_PERIOD_RATED_NUMBER_DEFAULT;
#else
    return 0x04;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_get_offbilling_unit_price
 * 功能            获取当前时间电费总单价
 * 参数            curr_time  当前时间(s)
 * 返回           当前时间电费总单价
 ********************************************************/
uint32_t sys_get_offbilling_unit_price(uint32_t curr_time)
{
#ifdef CP_USING_OFFLINE_BILLING
    uint8_t rate_number = CP_RATED_TYPE_NUM_MAX, i = 0x00, j = 0x00, end_hour = 0x00;
    uint32_t price = 0x00;
    struct tm _tm;

    sys_config_enter_critical();
    _tm = *(localtime((time_t*)&curr_time));
    sys_config_exit_critical();

    for(i = 0x00; i < CP_RATED_TYPE_NUM_MAX; i++){
        for(j = 0x00; j < CP_RATED_TYPE_PERIOD_NUM; j++){
            if((s_chargepile_config_info.billing_rule.time[i][j].shour >= 24) ||
                    (s_chargepile_config_info.billing_rule.time[i][j].ehour >= 24)){
                continue;
            }
            if((s_chargepile_config_info.billing_rule.time[i][j].smin >= 60) ||
                    (s_chargepile_config_info.billing_rule.time[i][j].emin >= 60)){
                continue;
            }

            end_hour = s_chargepile_config_info.billing_rule.time[i][j].ehour;
            if(end_hour == 0x00){  /** 结束时间是 00：00，相当于小时是 24 */
                end_hour = 24;
            }

            if((_tm.tm_hour > s_chargepile_config_info.billing_rule.time[i][j].shour) && (_tm.tm_hour < end_hour)){
                rate_number = s_chargepile_config_info.billing_rule.time[i][j].rate_number;
            }else if(_tm.tm_hour == s_chargepile_config_info.billing_rule.time[i][j].shour){
                if(s_chargepile_config_info.billing_rule.time[i][j].shour == end_hour){
                    if((_tm.tm_min >= s_chargepile_config_info.billing_rule.time[i][j].smin) &&
                            (_tm.tm_min <= s_chargepile_config_info.billing_rule.time[i][j].emin)){
                        rate_number = s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }else{
                    if(_tm.tm_min >= s_chargepile_config_info.billing_rule.time[i][j].smin){
                        rate_number = s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }
            }else if(_tm.tm_hour == end_hour){
                if(s_chargepile_config_info.billing_rule.time[i][j].shour == end_hour){
                    if((_tm.tm_min >= s_chargepile_config_info.billing_rule.time[i][j].smin) &&
                            (_tm.tm_min <= s_chargepile_config_info.billing_rule.time[i][j].emin)){
                        rate_number = s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }else{
                    if(_tm.tm_min <= s_chargepile_config_info.billing_rule.time[i][j].emin){
                        rate_number = s_chargepile_config_info.billing_rule.time[i][j].rate_number;
                    }
                }
            }
        }
    }

    if(rate_number >= CP_RATED_TYPE_NUM_MAX){
        return 0x00;
    }
    price = s_chargepile_config_info.billing_rule.rate_elect_price[rate_number] + \
            s_chargepile_config_info.billing_rule.rate_service_price[rate_number] + \
            s_chargepile_config_info.billing_rule.rate_delay_price[rate_number];

    return price;
#else
    return 0x00;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_get_offbilling_elect_price
 * 功能            获取当前时间电费价格
 * 参数            rate_number  费率号
 * 返回           当前时间当前时间电费价格
 ********************************************************/
uint32_t sys_get_offbilling_elect_price(uint8_t rate_number)
{
#ifdef CP_USING_OFFLINE_BILLING
    if(rate_number >= CP_RATED_TYPE_NUM_MAX){
        return 0x00;
    }

    return s_chargepile_config_info.billing_rule.rate_elect_price[rate_number];
#else
    return 0x00;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_get_offbilling_service_price
 * 功能            获取当前时间服务费价格
 * 参数            rate_number  费率号
 * 返回           当前时间当前时间服务费价格
 ********************************************************/
uint32_t sys_get_offbilling_service_price(uint8_t rate_number)
{
#ifdef CP_USING_OFFLINE_BILLING
    if(rate_number >= CP_RATED_TYPE_NUM_MAX){
        return 0x00;
    }

    return s_chargepile_config_info.billing_rule.rate_service_price[rate_number];
#else
    return 0x00;
#endif /* CP_USING_OFFLINE_BILLING */
}

/*********************************************************
 * 函数名        sys_get_offbilling_delay_price
 * 功能            获取当前时间延迟费价格
 * 参数            rate_number  费率号
 * 返回           当前时间当前时间延迟费价格
 ********************************************************/
uint32_t sys_get_offbilling_delay_price(uint8_t rate_number)
{
#ifdef CP_USING_OFFLINE_BILLING
    if(rate_number >= CP_RATED_TYPE_NUM_MAX){
        return 0x00;
    }

    return s_chargepile_config_info.billing_rule.rate_delay_price[rate_number];
#else
    return 0x00;
#endif /* CP_USING_OFFLINE_BILLING */
}


