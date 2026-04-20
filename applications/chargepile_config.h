/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-25     我的杨yang       the first version
 */
#ifndef APPLICATIONS_CHARGEPILE_CONFIG_H_
#define APPLICATIONS_CHARGEPILE_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <rtthread.h>

#include "net_sal.h"
#include "app_billing_rule.h"

#define CFG_DEF_TCMRAM __attribute__((section(".TCM_RAM")))       /* 将变量定义在TCMRAM区，注：对于GD32F470ZGT6 TCMRAM 不能存放代码，不能被任何 DMA 访问，可以将一些变量定义在该地址空间；定义的变量初始值是未知的 */
#define CFG_DEF_SRAM0  __attribute__((section(".SRAM0_RAM")))     /* 将变量定义在SRAM0区，注：对于GD32F470ZGT6 SRAM0 可以存放代码，也可以存放变量，也可以作为线程的栈地址空间，可以被 DMA 访问；定义的变量初始值是未知的 */
#define CFG_DEF_SRAM1  __attribute__((section(".SRAM1_RAM")))     /* 将变量定义在SRAM1区，注：对于GD32F470ZGT6 SRAM1 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#define CFG_DEF_SRAM2  __attribute__((section(".SRAM2_RAM")))     /* 将变量定义在SRAM2区，注：对于GD32F470ZGT6 SRAM2 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */

/******************************************************************************/
#define FLASH_BASE_ADDRESS                ((uint32_t)(0x00000000))
#define W25Q64_SECTOR_SIZE                4096

#define DEVICE_INFO_RANGE_SIZE            ((uint32_t)(FLASH_BASE_ADDRESS + 0x00010000))                 /* 设备端信息存储范围 64k(0x00000000 - 0x00010000) */
#define OTA_INFO_ADDRESS                  ((uint32_t)(FLASH_BASE_ADDRESS + DEVICE_INFO_RANGE_SIZE))     /* OTA信息地址范围 64k(0x00010000 - 0x00020000) */
#define OTA_BASE_ADDRESS                   ((uint32_t)(FLASH_BASE_ADDRESS + 0x00020000))                /* OTA基地址 范围 2M(0x00020000 - 0x00220000) */
#define OTA_DATA_REGION_SIZE               0x200000                                                     /* OTA数据区大小：2M */

#define RECORD_INFO_ADDRESS               ((uint32_t)(FLASH_BASE_ADDRESS + 0x00220000))                 /* 记录信息基地址 0x00220000(0x00220000 - 0x00240000) */
#define RECORD_INFO_REGION_SIZE           0x2000                                                        /* 记录信息区大小：8K */

#define GUNNOA_CHARGE_RECORD_ADDRESS      ((uint32_t)(FLASH_BASE_ADDRESS + 0x00240000))                 /* A枪充电记录地址 范围 1M(0x00240000 - 0x00340000) */
#define GUNNOA_CHARGE_RECORD_REGION_SIZE  0x100000                                                      /* A枪充电记录信息区大小：1M */

#define GUNNOB_CHARGE_RECORD_ADDRESS      ((uint32_t)(FLASH_BASE_ADDRESS + 0x00340000))                 /* B枪充电记录地址 范围 1M(0x00340000 - 0x00440000) */
#define GUNNOB_CHARGE_RECORD_REGION_SIZE  0x100000                                                      /* B枪充电记录信息区大小：1M */

#define GUNNOA_FAULT_RECORD_ADDRESS       ((uint32_t)(FLASH_BASE_ADDRESS + 0x00440000))                 /* A枪故障记录地址 范围 1M(0x00440000 - 0x00540000) */
#define GUNNOA_FAULT_RECORD_REGION_SIZE   0x100000                                                      /* A枪故障记录信息区大小：1M */

#define GUNNOB_FAULT_RECORD_ADDRESS       ((uint32_t)(FLASH_BASE_ADDRESS + 0x00540000))                 /* B枪故障记录地址 范围 1M(0x00540000 - 0x00640000) */
#define GUNNOB_FAULT_RECORD_REGION_SIZE   0x100000                                                      /* B枪故障记录信息区大小：1M */

#define SYSTEM_CONFIG_INIT_FLAG_ADDRESS   ((uint32_t)(FLASH_BASE_ADDRESS + 0x00640000))                 /* 系统配置信息初始标志地址(0x00640000 - 0x00641000)*/
#define SYSTEM_CONFIG_MAIN_ADDRESS        ((uint32_t)(FLASH_BASE_ADDRESS + 0x00641000))                 /* 系统配置信息地址(0x00641000 - 0x00644000) */
#define SYSTEM_CONFIG_BACKUP_ADDRESS      ((uint32_t)(FLASH_BASE_ADDRESS + 0x00644000))                 /* 系统配置信息备份区地址 (0x00644000 - 0x00647000)*/

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
#define SYSTEM_CONFIG_TP_ADDITIONALREGION_ADDRESS      ((uint32_t)(FLASH_BASE_ADDRESS + 0x00647000))    /* 系统配置信息备份区地址 (0x00647000 - (0x00647000 + APP_TARGET_PLATFORM_ADDITIONAL_REGION_SIZE))*/
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */

/** internal flash */
#define SYSTEM_CONFIG_INFO_ADDR_IF        0x08010000
/******************************************************************************/
#define CONFIG_ENABLE_ENUM                 0xAA                    /** 功能使能 */
#define CONFIG_DISABLE_ENUM                0x55                    /** 功能失能 */

#define CP_NETWORK_DOMAIN_LEN_MAX          256                     /* 域名最大长度 */
#define CP_NETWORK_MAC_ADDR_LEN_MAX        6                       /* MAC地址最大长度 */
#define CP_NETWORK_GATEWAY_LEN_MAX         32                      /* 网关地址最大长度 */

#define CP_ENCRYPT_KEY_LEN_MAX             32                      /* 密钥最大长度 */
#define CP_ENCRYPT_RANDOM_STR_LEN_MAX      32                      /* 随机串最大长度 */
#define CP_ENCRYPT_ID_LEN_MAX              32                      /* ID最大长度 */
#define CP_ENCRYPT_SIGN_LEN_MAX            64                      /* 签名最大长度 */

#define CP_INFO_PILE_NUMBER_LEN_MAX        32                      /* 桩号最大长度 */
#define CP_INFO_SERIAL_NUMBER_LEN_MAX      32                      /* 序列号最大长度 */
#define CP_INFO_HELP_NUMBER_LEN_MAX        32                      /* 帮组电话最大长度 */

#define CP_INFO_QRCODE_PREFIX_LEN_MAX      256                     /* 二维码前醉最大长度 */
#define CP_INFO_QRCODE_SUFFIX_LEN_MAX      128                     /* 二维码后醉最大长度 */

#define CP_INFO_TARGET_PLAT_LEN_MAX        1024                    /* 目标平台存储信息最大长度 */
#define CP_INFO_MONITOR_PLAT_LEN_MAX       512                     /* 监控平台存储信息最大长度 */

#define CP_INFO_VIN_WHITELIST_NUM_MAX      25                      /* VIN 码白名单最大数量 */
#define CP_INFO_CARD_NUMBER_WHITELIST_NUM_MAX  25                  /* 卡 号码白名单最大数量 */
#define CP_INFO_CARD_UID_WHITELIST_NUM_MAX 25                      /* 卡 UID白名单最大数量 */

#define CP_INFO_METER_ADDRESS_LEN_MAX      13                      /* 电表地址最大长度 */
#define CP_INFO_SCREEN_PASSWORD_LEN_MAX    15                      /* 屏幕密码最大长度 */

#define CP_INFO_USER_IDENTITY_LEN_MAX      9                       /* 用户识别码最大长度 */
#define CP_INFO_REGISTER_CODE_LEN_MAX      64                      /* 注册码最大长度 */

#define CP_INFO_CARD_KEY_LEN_MAX           12                      /* 卡密钥最大长度 */

#define CP_INFO_LOGIN_USER_NAME_LEN_MAX    32                      /* 登录用户名最大长度 */
#define CP_INFO_LOGIN_USER_PASSWORD_LEN_MAX 32                     /* 登录密码最大长度 */

#define CARD_NUMBER_LENGTH_DEF             0x10                    /* 卡号长度默认值 */
#define CARD_NUMBER_LENGTH_MIN             0x06                    /* 卡号长度最小值 */
#define CARD_NUMBER_LENGTH_MAX             0x10                    /* 卡号长度最大值 */

#define CARD_UID_LENGTH_DEF                0x08                    /* 卡UID长度默认值 */
#define CARD_UID_LENGTH_MIN                0x01                    /* 卡UID长度最小值 */
#define CARD_UID_LENGTH_MAX                0x08                    /* 卡UID长度最大值 */

#define VIN_CODE_LENGTH_DEF                0x11                    /* VIN码长度默认值 */
#define VIN_CODE_LENGTH_MIN                0x11                    /* VIN码长度最小值 */
#define VIN_CODE_LENGTH_MAX                0x11                    /* VIN码长度最大值 */


#define CP_SET_QRCODE_FORMAT_PREFIX                   0x01             /* 平台下发的二维码格式类型：前缀 */
#define CP_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN         0x02             /* 平台下发的二维码格式类型 ：前缀+设备号*/
#define CP_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT    0x03             /* 平台下发的二维码格式类型 ：前缀+设备号+枪号 */
#define CP_SET_QRCODE_FORMAT_PORT                     0x04             /* 平台下发的二维码格式类型 ：枪号*/
#define CP_SET_QRCODE_FORMAT_SIZE                     0x05

#define CP_GENERATE_QRCODE_FORMAT_PREFIX              0x01             /* 终端生成的二维码格式类型：前缀 */
#define CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN    0x02             /* 终端生成的二维码格式类型：前缀+设备号 */
#define CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT  0x03          /* 终端生成的二维码格式类型：前缀+设备号+枪号 */
#define CP_GENERATE_QRCODE_FORMAT_PORT                0x04             /* 终端生成的二维码格式类型：枪号 */
#define CP_GENERATE_QRCODE_FORMAT_SIZE                0x05

/***************************************** [默认配置] *******************************************/
/** 以下所有配置默认云快充，除非有定义 */
//#define CP_DOMAIN_USING_NJ                            /* IP域名使用能佳 */
//#define CP_DOMAIN_USING_YCP                           /* IP域名使用越城公用 */

//#define CP_QRCODE_CONFIG_USING_XXCD                   /* 二维码配置使用星星充电 */
//#define CP_QRCODE_CONFIG_USING_TLD                    /* 二维码配置使用特来电 */
//#define CP_QRCODE_CONFIG_USING_DUPU                   /* 二维码配置使用度普 */
//#define CP_QRCODE_CONFIG_USING_NJ                     /* 二维码配置使用能佳 */
//#define CP_QRCODE_CONFIG_USING_SGCC                   /* 二维码配置使用国网 */
//#define CP_QRCODE_CONFIG_USING_QBJ                      /* 二维码配置使用柒捌玖 */

//#define CP_PLATFORM_USING_YCP                           /* 平台使用越城公用 */
//#define CP_PLATFORM_USING_XXCD                          /* 平台使用星星充电 */
//#define CP_PLATFORM_USING_TLD                           /* 平台使用特来电 */
//#define CP_PLATFORM_USING_SGCC                          /* 平台使用国网 */
//#define CP_PLATFORM_USING_QBJ                          /* 平台使用柒捌玖 */

#define CP_USING_OFFLINE_BILLING                        /* 包含离线计费 */
//#define CP_USING_NO_BMS                                 /* 使用无BMS版本 */
//#define CP_USING_LV_MODULE_BMS                          /* 使用带BMS的低压模块版本 */
//#define CP_USING_LV_MODULE                              /* 使用低压模块版本 */
#define CP_USING_METER_ELECT_DETECT_STRATEGY            /* 使用电表电量检测策略 */
//#define CP_USING_BAT_VOLT_DETECT_STRATEGY               /* 使用电池电压检测策略 */
#define CP_USING_CHARGE_CURR_DETECT_STRATEGY            /* 使用充电电流检测策略 */
#define CP_USING_FB_DETECT                              /* 使用反馈实时检测 */
//#define CP_INCLUDE_BATVOLT_DETECT_QRCODE                /* 包含电池电压报告检测二维码 */
//#define CP_USING_V2G                                    /* 使用V2G */
//#define CP_USING_CYCLE_MATRIX                             /* 使用环矩部分 */
//#define CP_USING_PEAK_OUT_STRATEGY                      /* 使用峰值电流输出策略 */

/** 域名默认 */
/**------------------------------------------------------------*/
#ifdef CP_DOMAIN_USING_NJ
#define CP_DOMAIN_DEFAULT                         "47.98.137.199"          /* 能佳IP */
#define CP_PORT_DEFAULT                           9350                     /* 能佳端口 */
#elif defined(CP_DOMAIN_USING_YCP)
#define CP_DOMAIN_DEFAULT                         "121.229.203.34"         /* 越城公用IP */
#define CP_PORT_DEFAULT                           6002                     /* 越城公用端口 */
#elif defined(CP_QRCODE_CONFIG_USING_SGCC)
#define CP_DOMAIN_DEFAULT                         "121.43.69.62"           /* 国网IP */
#define CP_PORT_DEFAULT                           8767                     /* 国网端口 */
#else
#define CP_DOMAIN_DEFAULT                         "121.43.69.62"           /* 云快充IP */
#define CP_PORT_DEFAULT                           8767                     /* 云快充端口 */
#endif /* APP_DOMAIN_USING_NJ */

/** 连接平台默认 */
/**------------------------------------------------------------*/
#ifdef CP_PLATFORM_USING_YCP
#define CP_PLATFORM_ID                            2L                       /* 越城公用平台ID */
#elif defined(CP_PLATFORM_USING_XXCD)
#define CP_PLATFORM_ID                            6L                       /* 星星充电平台ID */
#elif defined(CP_PLATFORM_USING_TLD)
#define CP_PLATFORM_ID                            7L                       /* 特来电平台ID */
#elif defined(CP_PLATFORM_USING_SGCC)
#define CP_PLATFORM_ID                            1L                       /* 国网平台ID */
#elif defined(CP_PLATFORM_USING_QBJ)
#define CP_PLATFORM_ID                            19L                      /* 柒捌玖平台ID */
#define CP_CONFIG_USING_QBJ                                                /* 配置部分使用柒捌玖 */
#else
#define CP_PLATFORM_ID                            0L                       /* 云快充平台ID */
#endif /* CP_PLATFORM_USING_YCP */

/** 二维码前缀默认 */
/**------------------------------------------------------------*/
#ifdef CP_QRCODE_CONFIG_USING_XXCD
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX                       /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */
#define CP_QRCODE_PREFIX_DEFAULT                  "https://qrcode.starcharge.com/#/"                /* 星星充电二维码前缀 */
#elif defined(CP_QRCODE_CONFIG_USING_TLD)
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT        /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */
#define CP_QRCODE_PREFIX_DEFAULT                  "hlht://"                                         /* 特来电二维码前缀 */
#elif defined(CP_QRCODE_CONFIG_USING_DUPU)
#define CP_CONFIG_USING_DUPU                                                                        /* 配置部分使用度普 */
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT        /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */
#define CP_QRCODE_PREFIX_DEFAULT                  "hlht://32010600208123D1.MA27YQ0R4/"              /* 度普二维码前缀 */
#elif defined(CP_QRCODE_CONFIG_USING_NJ)
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX                       /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */
#define CP_QRCODE_PREFIX_DEFAULT                  "https://wechat.xiangnengnengjia.com?scanid="     /* 能佳二维码前缀 */
#elif defined(CP_QRCODE_CONFIG_USING_SGCC)
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT        /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */

#ifdef APP_USING_DOUBLEGUN
#define CP_QRCODE_PREFIX_DEFAULT                  "https://cdn-evone-oss.echargenet.com/IntentServe/index.html?M&qrcode="     /* 国网二维码前缀 */
#else
#define CP_QRCODE_PREFIX_DEFAULT                  "https://cdn-evone-oss.echargenet.com/IntentServe/index.html?qrcode="     /* 国网二维码前缀 */
#endif /* APP_USING_DOUBLEGUN */

#define CP_QRCODE_PARA_PRODUCT_IDENTIFICATION     "gwwl//"                                          /* 二维码配置参数：产品标识 */
#define CP_QRCODE_PARA_VENDOR_CODE                "1002"                                            /* 二维码配置参数：厂商代码 */
#define CP_QRCODE_PARA_RULE_VERSION               "1.0.1"                                           /* 二维码配置参数：二维码规则版本 */
#define CP_QRCODE_PARA_STRING_CODE_TYPE           "3"                                               /* 二维码配置参数：字符码类型 */

#define CP_QRCODE_PARA_PRODUCT_IDENTIFI           "FFFFFFFFFFFF"                                    /* 二维码配置参数：蓝牙MAC地址 */

#else
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX                       /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */
#define CP_QRCODE_PREFIX_DEFAULT                  "http://www.ykccn.com/MPAGE/index.html?pNum="     /* 云快充二维码前缀 */
#endif

/** 屏幕密码默认 */
/**------------------------------------------------------------*/
#define CP_SCREEN_PASSWORD_DEFAULT                "0909"                                            /* 默认屏幕密码 */
/** 桩号默认 */
/**------------------------------------------------------------*/
#define CP_PILE_NUMBER_DEFAULT                    "TX000000000001"                                  /* 默认桩号 */
/** 帮助电话默认 */
/**------------------------------------------------------------*/
#define CP_HELP_PHONE_DEFAULT                     "18621280288"                                     /* 默认帮助电话 */


/* module config */
#define MODULE_MODEL_YFY                          0           /* 模块型号：英飞源 */
#define MODULE_MODEL_GW                           1           /* 模块型号：国网 */
#define MODULE_MODEL_WL                           2           /* 模块型号：永联 */
#define MODULE_MODEL_YY                           3           /* 模块型号：优优 */
#define MODULE_MODEL_YN                           4           /* 模块型号：易能  */
#define MODULE_MODEL_RESERVE0                     5           /* 模块型号：预留0 */
#define MODULE_MODEL_RESERVE1                     6           /* 模块型号：预留1 */

#define MODULE_MODEL_DEFAULT                      2           /* 默认模块型号：永联 */
#define MODULE_MODEL_NUMBER                       5           /* 当前系统支持的模块型号数量 */
#define MODULE_GROUP_NUMBER_DEFAULT               4           /* 默认模块组数：4组 */
#define MODULE_GROUP_NUMBER_MAX                   4           /* 最大模块组数：4组 */
#define MODULE_NUMBER_SINGLE_DEFAULT              3           /* 默认单个组的模块数：3 */
#define MODULE_NUMBER_SINGLE_MAX                  16          /* 最大单个组的模块数：8 */

#define POWER_ALLOCATION_WAY_AVERAGE              0           /* 功率分配方式:均充 */
#define POWER_ALLOCATION_WAY_SEQ_PRIORITY         1           /* 功率分配方式:先到先得 */
#define POWER_ALLOCATION_WAY_POWER_PRIORITY       2           /* 功率分配方式:功率优先 */
#define POWER_ALLOCATION_WAY_SIZE                 3           /* 功率分配方式 */

#define CONFIG_LP_CONSUMPTION_MODULE_NULL         0           /* 低功耗模块：无 */
#define CONFIG_LP_CONSUMPTION_MODULE_PLUSE        1           /* 低功耗模块：易能 */
#define CONFIG_LP_CONSUMPTION_MODULE_DLEVEL       2           /* 低功耗模块：电平 */
#define CONFIG_LP_CONSUMPTION_MODULE_SIZE         3           /* 低功耗模块 */

#define SYSTEM_FUNCTION_SINGLE_TERMINAL           0         /* 单枪超充 */
#define SYSTEM_FUNCTION_AVERAGE_DOUBLE            1         /* 均充双枪 */
#define SYSTEM_FUNCTION_DOUBLE_WHOLE              2         /* 双枪终端 */
#define SYSTEM_FUNCTION_SINGLE_FAST               3         /* 单枪快充 */
#define SYSTEM_FUNCTION_DYNAMIC_SWITCH            4         /* 动态切换 */
#ifdef CP_USING_CYCLE_MATRIX
#define SYSTEM_FUNCTION_MS_MACHINE_CYCLE          5         /* 设备类型(本机功能)：子母机(环矩) */
#define SYSTEM_FUNCTION_MS_MACHINE_HALF           6         /* 设备类型(本机功能)：子母机(半矩) */
#define SYSTEM_FUNCTION_WHOLE_CYCLE               7         /* 设备类型(本机功能)：一体机(环矩) */
#define SYSTEM_FUNCTION_SIZE                      8         /* 设备类型(本机功能) */
#else
#define SYSTEM_FUNCTION_SIZE                      5         /* 系统功能 */
#endif /* CP_USING_CYCLE_MATRIX */


#define CP_LIQUID_DEVTYPE_YTND                    0         /* 英特尼迪 */
#define CP_LIQUID_DEVTYPE_HL                      1         /* 毫厘 */
#define CP_LIQUID_DEVTYPE_IMMERSIONJGD            2         /* 京工电 */
#define CP_LIQUID_DEVTYPE_TPS                     3         /* 特倍斯 */
#define CP_LIQUID_DEVTYPE_SIZE                    4

#define CP_LIQUID_DEVCNT_MAX                      2         /* 最大液冷设备数量 */

#define CONFIG_CARD_BLOCK_SN_DEFAULT              9           /* 卡号所在块号：默认 */
#define CONFIG_CARD_BLOCK_SN_MIN                  0           /* 卡号所在块号：最小值 */
#define CONFIG_CARD_BLOCK_SN_MAX                  63          /* 卡号所在块号：最大值 */

/* protect info config */
#define PROTECT_POWER_PERCENT_VALUE_DEFAULT       1000      /* 保护信息：默认功率百分比值 */
#define PROTECT_POWER_PERCENT_VALUE_MAX           1000      /* 保护信息：功率百分比最大值 */
#define PROTECT_POWER_PERCENT_VALUE_MIN           10        /* 保护信息：功率百分比最小值 */

#define PROTECT_STOP_SOC_VALUE_DEFAULT            100       /* 保护信息：默认SOC停充值 */
#define PROTECT_STOP_SOC_VALUE_MAX                101       /* 保护信息：SOC停充值最大值 */
#define PROTECT_STOP_SOC_VALUE_MIN                1         /* 保护信息：SOC停充值最小值 */

#define PROTECT_OVERTEMP_WARNNING_VALUE_DEFAULT   75        /* 保护信息：默认过温告警值 */
#define PROTECT_OVERTEMP_WARNNING_VALUE_MAX       300       /* 保护信息：过温告警值最大值 */
#define PROTECT_OVERTEMP_WARNNING_VALUE_MIN       1         /* 保护信息：过温告警值最小值 */

#define PROTECT_OVERTEMP_STOP_VALUE_DEFAULT       105       /* 保护信息：默认过温停充值 */
#define PROTECT_OVERTEMP_STOP_VALUE_MAX           300       /* 保护信息：过温停充值最大值 */
#define PROTECT_OVERTEMP_STOP_VALUE_MIN           1         /* 保护信息：过温停充值最小值 */

#define PROTECT_OVERTEMP_RESUME_VALUE_DEFAULT     80        /* 保护信息：默认过温恢复值 */
#define PROTECT_OVERTEMP_RESUME_VALUE_MAX         120       /* 保护信息：过温恢复值最大值 */
#define PROTECT_OVERTEMP_RESUME_VALUE_MIN         1         /* 保护信息：过温恢复值最小值 */

#define PROTECT_OVERTEMP_LIMITCURR_VALUE_DEFAULT  95        /* 保护信息：默认过温限流值 */
#define PROTECT_OVERTEMP_LIMITCURR_VALUE_MAX      300       /* 保护信息：过温限流值最大值 */
#define PROTECT_OVERTEMP_LIMITCURR_VALUE_MIN      1         /* 保护信息：过温限流值最小值 */

#define PROTECT_DISCHARGE_AS_OF_SOC_DEFAULT       20        /* 保护信息：放电截至SOC值 */
#define PROTECT_DISCHARGE_AS_OF_SOC_MAX           101       /* 保护信息：放电截至SOC最大值 */
#define PROTECT_DISCHARGE_AS_OF_SOC_MIN           0         /* 保护信息：放电截至SOC最小值 */
#define PROTECT_DISCHARGE_AS_OF_SOC_OFFSET        100       /* 保护信息：放电截至SOC偏移(正偏移，范围 100 - 201, 防止老板子可能这部分空间被默认成了0或1) */

#define GUNVOLT_LIMIT_VALUE_MIN                   10 *100   /* 枪头电压最小值 */
#define GUNVOLT_LIMIT_VALUE_MAX                   600 *100  /* 枪头电压最大值 */

#define MODULE_RATED_OUTVOLT_DEF                  750       /* 模块额定输出电压默认值值 */
#define MODULE_RATED_OUTVOLT_MAX                  2000      /* 模块额定输出电压最大值 */
#define MODULE_RATED_OUTVOLT_MIN                  1         /* 模块额定输出电压最小值 */

#define CHARGEPILE_MAX_OUTVOLT_DEF                750       /* 桩最大输出电压默认值值 */
#define CHARGEPILE_MAX_OUTVOLT_MAX                2000      /* 桩最大输出电压最大值 */
#define CHARGEPILE_MAX_OUTVOLT_MIN                1         /* 桩最大输出电压最小值 */

#define CHARGEPILE_MIN_OUTVOLT_DEF                200       /* 桩最小输出电压默认值值 */
#define CHARGEPILE_MIN_OUTVOLT_MAX                1000      /* 桩最小输出电压最大值 */
#define CHARGEPILE_MIN_OUTVOLT_MIN                1         /* 桩最小输出电压最小值 */

#define MODULE_RATED_LIMIT_CURR_DEF               30        /* 模块额定限电流默认值值 */
#define MODULE_RATED_LIMIT_CURR_MAX               150       /* 模块额定限电流最大值 */
#define MODULE_RATED_LIMIT_CURR_MIN               1         /* 模块额定限电流最小值 */

#define MODULE_MAX_LIMIT_CURR_DEF                 500       /* 模块最大限电流默认值值 */
#define MODULE_MAX_LIMIT_CURR_MAX                 1500      /* 模块最大限电流最大值 */
#define MODULE_MAX_LIMIT_CURR_MIN                 1         /* 模块最大限电流最小值 */

#define MODULE_MIN_LIMIT_CURR_DEF                 2         /* 模块最小限电流默认值(要和模块库的兼容) */
#define MODULE_MIN_LIMIT_CURR_MAX                 10        /* 模块最小限电流最大值(要和模块库的兼容) */
#define MODULE_MIN_LIMIT_CURR_MIN                 0         /* 模块最小限电流最小值(要和模块库的兼容) */

#define MODULE_SMODULE_MAX_CURR_DEF               10000     /* 模块最大限电流值默认值(要和模块库的兼容) */
#define MODULE_SMODULE_MAX_CURR_MAX               10000     /* 模块最大限电流值最大值(要和模块库的兼容) */
#define MODULE_SMODULE_MAX_CURR_MIN               1         /* 模块最大限电流值最小值(要和模块库的兼容) */

#define CONFIG_LIGHTING_LAMP_SHOUR_MIN            0         /* 照明起始小时最小值(24小时制) */
#define CONFIG_LIGHTING_LAMP_SHOUR_MAX            23        /* 照明起始小时最大值(24小时制) */

#define CONFIG_LIGHTING_LAMP_EHOUR_MIN            0         /* 照明结束小时最小值(24小时制) */
#define CONFIG_LIGHTING_LAMP_EHOUR_MAX            23        /* 照明结束小时最大值(24小时制) */

#define CONFIG_LIGHTING_LAMP_SMIN_MIN             0         /* 照明起始分钟最小值 */
#define CONFIG_LIGHTING_LAMP_SMIN_MAX             59        /* 照明起始分钟最大值 */

#define CONFIG_LIGHTING_LAMP_EMIN_MIN             0         /* 照明结束分钟最小值 */
#define CONFIG_LIGHTING_LAMP_EMIN_MAX             59        /* 照明结束分钟最大值 */

#define COMPULSION_SET_VOLTAGE_DEF                750       /* 强制启动设定电压默认值 */
#define COMPULSION_SET_VOLTAGE_MAX                1500      /* 强制启动设定电压最大值 */
#define COMPULSION_SET_VOLTAGE_MIN                1         /* 强制启动设定电压最小值 */

#define COMPULSION_SET_CURRENT_DEF                10        /* 强制启动设定电流默认值 */
#define COMPULSION_SET_CURRENT_MAX                4000      /* 强制启动设定电流最大值 */
#define COMPULSION_SET_CURRENT_MIN                0         /* 强制启动设定电流最小值 */

#ifdef CP_USING_CYCLE_MATRIX
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_ALL_DEF   5000      /* 模块矩阵排布调试：设定所有组电压默认值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_ALL_MAX   12000     /* 模块矩阵排布调试：设定所有组电压最大值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_ALL_MIN   10        /* 模块矩阵排布调试：设定所有组电压最小值 */

#define CONFIG_MATRIX_DEBUG_SET_CURRENT_ALL_DEF   0         /* 模块矩阵排布调试：设定所有组电流默认值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_ALL_MAX   15000     /* 模块矩阵排布调试：设定所有组电流最大值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_ALL_MIN   0         /* 模块矩阵排布调试：设定所有组电流最小值 */

#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G1_DEF    3000      /* 模块矩阵排布调试：设定组1电压默认值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G1_MAX    12000     /* 模块矩阵排布调试：设定组1电压最大值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G1_MIN    1         /* 模块矩阵排布调试：设定组1电压最小值 */

#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G1_DEF    0         /* 模块矩阵排布调试：设定组1电流默认值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G1_MAX    15000     /* 模块矩阵排布调试：设定组1电流最大值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G1_MIN    0         /* 模块矩阵排布调试：设定组1电流最小值 */

#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G2_DEF    3500      /* 模块矩阵排布调试：设定组2电压默认值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G2_MAX    12000     /* 模块矩阵排布调试：设定组2电压最大值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G2_MIN    1         /* 模块矩阵排布调试：设定组2电压最小值 */

#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G2_DEF    0         /* 模块矩阵排布调试：设定组2电流默认值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G2_MAX    15000     /* 模块矩阵排布调试：设定组2电流最大值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G2_MIN    0         /* 模块矩阵排布调试：设定组2电流最小值 */

#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G3_DEF    4000      /* 模块矩阵排布调试：设定组3电压默认值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G3_MAX    12000     /* 模块矩阵排布调试：设定组3电压最大值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G3_MIN    1         /* 模块矩阵排布调试：设定组3电压最小值 */

#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G3_DEF    0         /* 模块矩阵排布调试：设定组3电流默认值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G3_MAX    15000     /* 模块矩阵排布调试：设定组3电流最大值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G3_MIN    0         /* 模块矩阵排布调试：设定组3电流最小值 */

#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G4_DEF    4500      /* 模块矩阵排布调试：设定组4电压默认值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G4_MAX    12000     /* 模块矩阵排布调试：设定组4电压最大值 */
#define CONFIG_MATRIX_DEBUG_SET_VOLTAGE_G4_MIN    1         /* 模块矩阵排布调试：设定组4电压最小值 */

#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G4_DEF    0         /* 模块矩阵排布调试：设定组4电流默认值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G4_MAX    15000     /* 模块矩阵排布调试：设定组4电流最大值 */
#define CONFIG_MATRIX_DEBUG_SET_CURRENT_G4_MIN    0         /* 模块矩阵排布调试：设定组4电流最小值 */
#endif /* CP_USING_CYCLE_MATRIX */

#define CHARGEPILE_INPUT_OVERVOLT_DEF             55000     /* 充电桩输入过压默认值(0.01V) */
#define CHARGEPILE_INPUT_OVERVOLT_MAX             100000    /* 充电桩输入过压最大值(0.01V) */
#define CHARGEPILE_INPUT_OVERVOLT_MIN             1         /* 充电桩输入过压最小值(0.01V) */

#define CHARGEPILE_INPUT_UNDERVOLT_DEF            0         /* 充电桩输入欠压默认值(0.01V) */
#define CHARGEPILE_INPUT_UNDERVOLT_MAX            50000     /* 充电桩输入欠压最大值(0.01V) */
#define CHARGEPILE_INPUT_UNDERVOLT_MIN            0         /* 充电桩输入欠压最小值(0.01V) */

#define CHARGEPILE_OUTPUT_OVERVOLT_DEF            200000    /* 充电桩输出过压默认值(0.01V) */
#define CHARGEPILE_OUTPUT_OVERVOLT_MAX            300000    /* 充电桩输出过压最大值(0.01V) */
#define CHARGEPILE_OUTPUT_OVERVOLT_MIN            1         /* 充电桩输出过压最小值(0.01V) */

#define CHARGEPILE_OUTPUT_UNDERVOLT_DEF           0         /* 充电桩输出欠压默认值(0.01V) */
#define CHARGEPILE_OUTPUT_UNDERVOLT_MAX           150000    /* 充电桩输出欠压最大值(0.01V) */
#define CHARGEPILE_OUTPUT_UNDERVOLT_MIN           0         /* 充电桩输出欠压最小值(0.01V) */

#define CHARGEPILE_OUTPUT_OVERCURR_DEF            160000    /*充电桩输出过流默认值(0.01A) */
#define CHARGEPILE_OUTPUT_OVERCURR_MAX            160000    /*充电桩输出过流最大值(0.01A) */
#define CHARGEPILE_OUTPUT_OVERCURR_MIN            1         /*充电桩输出过流最小值(0.01A) */

#ifdef APP_USING_DOUBLEGUN
#define CHARGEPILE_CC1_VOLT_MAX                   30000     /* CC1 电压最大值(0.001V) */
#define CHARGEPILE_CC12V_MAX_DEF                  13000     /* CC1 12V默认上限(0.001V, 包含边界值) */
#define CHARGEPILE_CC12V_MIN_DEF                  11000     /* CC1 12V默认下限(0.001V, 包含边界值) */
#define CHARGEPILE_CC12V_S                        12000     /* CC1 12V标准(0.001V) */

#define CHARGEPILE_CC6V_MAX_DEF                   7000      /* CC1 6V默认上限(0.001V, 包含边界值) */
#define CHARGEPILE_CC6V_MIN_DEF                   5000      /* CC1 6V默认下限(0.001V, 不包含边界值) */
#define CHARGEPILE_CC6V_S                         6000      /* CC1 6V标准(0.001V) */

#define CHARGEPILE_CC4V_MAX_DEF                   5000      /* CC1 4V默认上限(0.001V, 包含边界值) */
#define CHARGEPILE_CC4V_MIN_DEF                   1000      /* CC1 4V默认下限(0.001V, 包含边界值) */
#define CHARGEPILE_CC4V_S                         4000      /* CC1 4V标准(0.001V) */
#else
#define CHARGEPILE_CC1_VOLT_MAX                   30000     /* CC1 电压最大值(0.001V) */
#define CHARGEPILE_CC12V_MAX_DEF                  13000     /* CC1 12V默认上限(0.001V, 包含边界值) */
#define CHARGEPILE_CC12V_MIN_DEF                  8000      /* CC1 12V默认下限(0.001V, 包含边界值) */
#define CHARGEPILE_CC12V_S                        12000     /* CC1 12V标准(0.001V) */

#define CHARGEPILE_CC6V_MAX_DEF                   8000      /* CC1 6V默认上限(0.001V, 包含边界值) */
#define CHARGEPILE_CC6V_MIN_DEF                   5000      /* CC1 6V默认下限(0.001V, 不包含边界值) */
#define CHARGEPILE_CC6V_S                         6000      /* CC1 6V标准(0.001V) */

#define CHARGEPILE_CC4V_MAX_DEF                   5000      /* CC1 4V默认上限(0.001V, 包含边界值) */
#define CHARGEPILE_CC4V_MIN_DEF                   1000      /* CC1 4V默认下限(0.001V, 包含边界值) */
#define CHARGEPILE_CC4V_S                         4000      /* CC1 4V标准(0.001V) */
#endif /* APP_USING_DOUBLEGUN */

#define CHARGEPILE_ELOSS_PROPORTION_MIN           0         /* 电损比最小值(一位小数) */
#define CHARGEPILE_ELOSS_PROPORTION_MAX           100       /* 电损比最大值(一位小数) */
#define CHARGEPILE_ELOSS_PROPORTION_DEF           0         /* 电损比默认值(一位小数) */

#define CHARGEPILE_FAN_WORK_TIME_MIN              5         /* 停充后风扇工作时间最小值(s) */
#define CHARGEPILE_FAN_WORK_TIME_MAX              (0xFFFF - 0x01)    /* 停充后风扇工作时间最大值(s) */
#define CHARGEPILE_FAN_WORK_TIME_DEF              (2 *60)   /* 停充后风扇工作时间默认值(s) */

#define CP_AMMETER_CHECK_WAY_EVEN                 0         /* 电表串口校验方式：偶校验 */
#define CP_AMMETER_CHECK_WAY_ODD                  1         /* 电表串口校验方式：奇校验 */
#define CP_AMMETER_CHECK_WAY_NONE                 2         /* 电表串口校验方式：无校验 */

#define CP_AMMETER_BAUDRATE_9600                  0         /* 电表串口波特率：9600 */
#define CP_AMMETER_BAUDRATE_2400                  1         /* 电表串口波特率：2400 */
#define CP_AMMETER_BAUDRATE_4800                  2         /* 电表串口波特率：4800 */
#define CP_AMMETER_BAUDRATE_38400                 3         /* 电表串口波特率：38400 */
#define CP_AMMETER_BAUDRATE_115200                4         /* 电表串口波特率：115200 */

#define CP_MODE_CHARGE_FULL                       0         /* 当前充电模式：充满 */
#define CP_MODE_LIMIT_MONEY                       1         /* 当前充电模式：限制金额 */
#define CP_MODE_LIMIT_ELECT                       2         /* 当前充电模式：限制电量 */
#define CP_MODE_LIMIT_TIMING                      3         /* 当前充电模式：定时 */
#define CP_MODE_LIMIT_RESERVATION                 4         /* 当前充电模式：预约 */
#define CP_MODE_SIZE                              5         /* 当前充电模式：*/

#define CP_V2G_MODE_OFFSET                        0x55      /* 当前放电模式偏移 */
#define CP_V2G_MODE_LIMIT_MONEY                   0x55      /* 当前放电模式：限制金额 */
#define CP_V2G_MODE_LIMIT_ELECT                   0x56      /* 当前放电模式：限制电量 */
#define CP_V2G_MODE_LIMIT_TIMING                  0x57      /* 当前放电模式：限制时间 */
#define CP_V2G_MODE_AUTO                          0x58      /* 当前放电模式：自动(根据设置的放电截至SOC来) */
#define CP_V2G_MODE_SIZE                          0x59      /* 当前放电模式：*/
#define CP_V2G_MODE_NULL                          0xFF      /* 当前放电模式：空*/

#define CP_MODE_PARA_MONEY_DEF                    10000     /* 充电模式金额参数默认值(单位：0.01元)*/
#define CP_MODE_PARA_MONEY_MAX                    500000    /* 充电模式金额参最大值(单位：0.01元)：*/
#define CP_MODE_PARA_MONEY_MIN                    200       /* 充电模式金额参数最小值(单位：0.01元)*/

#define CP_MODE_PARA_ELECT_DEF                    100000    /* 充电模式电量参数默认值(单位：0.001度)*/
#define CP_MODE_PARA_ELECT_MAX                    1000000   /* 充电模式电量参最大值(单位：0.001度)：*/
#define CP_MODE_PARA_ELECT_MIN                    1000      /* 充电模式电量参数最小值(单位：0.001度)*/

#define CP_MODE_PARA_TIMING_DEF                   30        /* 充电模式定时参数默认值(单位：1min)*/
#define CP_MODE_PARA_TIMING_MAX                   (24 *60)  /* 充电模式定时参最大值(单位：1min)：*/
#define CP_MODE_PARA_TIMING_MIN                   1         /* 充电模式定时参数最小值(单位：1min)*/

#define CP_MODE_PARA_RESERVATION_DEF              (22 *3600) /* 充电模式预约参数默认值(单位：1S[是当天启动时间的秒数])*/
#define CP_MODE_PARA_RESERVATION_MAX              (24 *3600) /* 充电模式预约参最大值(单位：1S[是当天启动时间的秒数])*/
#define CP_MODE_PARA_RESERVATION_MIN              0         /* 充电模式预约参数最小值(单位：1S[是当天启动时间的秒数])*/

#define CP_V2G_MODE_PARA_MONEY_DEF                10000     /* 放电模式金额参数默认值(单位：0.01元)*/
#define CP_V2G_MODE_PARA_MONEY_MAX                500000    /* 放电模式金额参最大值(单位：0.01元)：*/
#define CP_V2G_MODE_PARA_MONEY_MIN                200       /* 放电模式金额参数最小值(单位：0.01元)*/

#define CP_V2G_MODE_PARA_ELECT_DEF                100000    /* 放电模式电量参数默认值(单位：0.001度)*/
#define CP_V2G_MODE_PARA_ELECT_MAX                1000000   /* 放电模式电量参最大值(单位：0.001度)：*/
#define CP_V2G_MODE_PARA_ELECT_MIN                1000      /* 放电模式电量参数最小值(单位：0.001度)*/

#define CP_V2G_MODE_PARA_TIMING_DEF               30        /* 放电模式定时参数默认值(单位：1min)*/
#define CP_V2G_MODE_PARA_TIMING_MAX               (24 *60)  /* 放电模式定时参最大值(单位：1min)：*/
#define CP_V2G_MODE_PARA_TIMING_MIN               1         /* 放电模式定时参数最小值(单位：1min)*/

#define CP_V2G_MODE_PARA_RESERVATION_DEF          (22 *3600) /* 放电模式预约参数默认值(单位：1S[是当天启动时间的秒数])*/
#define CP_V2G_MODE_PARA_RESERVATION_MAX          (24 *3600) /* 放电模式预约参最大值(单位：1S[是当天启动时间的秒数])*/
#define CP_V2G_MODE_PARA_RESERVATION_MIN          0         /* 放电模式预约参数最小值(单位：1S[是当天启动时间的秒数])*/

/* fan pwm period */
#define CP_FAN_PWM_PERIOD_DEFAULT                 10000     /* 风机调速周期默认值(Hz) */
#define CP_FAN_PWM_PERIOD_MIN                     1         /* 风机调速周期最小值(Hz) */
#define CP_FAN_PWM_PERIOD_MAX                     1000000   /* 风机调速周期最大值(Hz) */

/* fan pwm timer prescaler */
#define CP_FAN_PWM_TIMER_PRESCALER_DEFAULT        48        /* 风机调速定时器分频值 */
#define CP_FAN_PWM_TIMER_PRESCALER_MIN            1         /* 风机调速定时器分频值 */
#define CP_FAN_PWM_TIMER_PRESCALER_MAX            0xFFFF    /* 风机调速定时器分频值 */

/* output current peak */
#define CP_PEAK_CURRENT_DEFAULT                   0         /* 峰值电流默认值(0.01A) */
#define CP_PEAK_CURRENT_MIN                       0         /* 峰值电流最小值(0.01A) */
#define CP_PEAK_CURRENT_MAX                       0xFFFFF   /* 峰值电流最大值(0.01A) */

/* current offset */
#define CP_CURRENT_OFFSET_DEF                     0         /* 电流偏移默认值(0.01A) */
#define CP_CURRENT_OFFSET_MAX                     10000     /* 电流偏移最大值(0.01A) */
#define CP_CURRENT_OFFSET_MIN                     0         /* 电流偏移最小值(0.01A) */
#define CP_CURRENT_OFFSET_SEPARATE                32768     /* 电流偏移分隔值(0.01A)(大于等于此值的为正偏移，否则为负偏移) */

/* net type */
#define CP_NETTYPE_4G                                 0x00             /* 联网方式：4G */
#define CP_NETTYPE_ETH                                0x01             /* 联网方式：以太网 */
#define CP_NETTYPE_OFFLINE                            0x02             /* 联网方式：离线模式 */
#define CP_NETTYPE_SIZE                               0x03

/* led language */
#define CP_LED_LANGUAGE_OFFSET                        0x55             /* 灯语值偏移 */
#define CP_LED_LANGUAGE_0                             0x55             /* 灯语：0 */
#define CP_LED_LANGUAGE_1                             0x56             /* 灯语：1 */
#define CP_LED_LANGUAGE_2                             0x57             /* 灯语：2 */
#define CP_LED_LANGUAGE_3                             0x58             /* 灯语：3 */
#define CP_LED_LANGUAGE_4                             0x59             /* 灯语：4 */
#define CP_LED_LANGUAGE_SIZE                          0x60             /* 灯语 */

#if (defined(CP_USING_V2G) || defined(CP_USING_OFFLINE_BILLING))
/* period num */
#define CP_PERIOD_MAX                                 0x60             /* 时段总数 */

/* rated type period time num */
#define CP_RATED_TYPE_PERIOD_NUM                      0x02             /* 离线计费每个费率可设置的时段数量 */

/* rated type */
#define CP_RATED_TYPE_NUM_MAX                         0x05             /* 费率类型总数 */

#define CP_RATED_TYPE_MIN                             0x00             /* 费率类型最小值 */
#define CP_RATED_TYPE_SHARP_SHARP                     0x00             /* 费率类型：尖尖 */
#define CP_RATED_TYPE_SHARP                           0x01             /* 费率类型：尖 */
#define CP_RATED_TYPE_PEAK                            0x02             /* 费率类型：峰 */
#define CP_RATED_TYPE_FLAT                            0x03             /* 费率类型：平 */
#define CP_RATED_TYPE_VALLEY                          0x04             /* 费率类型：谷 */
#define CP_RATED_TYPE_MAX                             0x04             /* 费率类型最大值 */

/* price info */
#define CP_PERIOD_ELECT_PRICE_MAX                     50000            /* 时段电费最大值(0.0001) */
#define CP_PERIOD_ELECT_PRICE_MIN                     0                /* 时段电费最小值(0.0001) */
#define CP_PERIOD_ELECT_PRICE_DEF                     10000            /* 时段电费默认值(0.0001) */

#define CP_PERIOD_SERVICE_PRICE_MAX                   50000            /* 时段服务费最大值(0.0001) */
#define CP_PERIOD_SERVICE_PRICE_MIN                   0                /* 时段服务费最小值(0.0001) */
#define CP_PERIOD_SERVICE_PRICE_DEF                   5000             /* 时段服务费默认值(0.0001) */

#define CP_PERIOD_DELAY_PRICE_MAX                     50000            /* 时段延迟费最大值(0.0001) */
#define CP_PERIOD_DELAY_PRICE_MIN                     0                /* 时段延迟费最小值(0.0001) */
#define CP_PERIOD_DELAY_PRICE_DEF                     0                /* 时段延迟费默认值(0.0001) */
/**************************************************************************************************/
#define CP_SHARP_SHARP_RATED_ELECT_PRICE_MAX          50000            /* 尖尖费率电费最大值(0.0001) */
#define CP_SHARP_SHARP_RATED_ELECT_PRICE_MIN          0                /* 尖尖费率电费最小值(0.0001) */
#define CP_SHARP_SHARP_RATED_ELECT_PRICE_DEF          10000            /* 尖尖费率电费默认值(0.0001) */

#define CP_SHARP_SHARP_RATED_SERVICE_PRICE_MAX        50000            /* 尖尖费率服务费最大值(0.0001) */
#define CP_SHARP_SHARP_RATED_SERVICE_PRICE_MIN        0                /* 尖尖费率服务费最小值(0.0001) */
#define CP_SHARP_SHARP_RATED_SERVICE_PRICE_DEF        5000             /* 尖尖费率服务费默认值(0.0001) */

#define CP_SHARP_SHARP_RATED_DELAY_PRICE_MAX          50000            /* 尖尖费率延迟费最大值(0.0001) */
#define CP_SHARP_SHARP_RATED_DELAY_PRICE_MIN          0                /* 尖尖费率延迟费最小值(0.0001) */
#define CP_SHARP_SHARP_RATED_DELAY_PRICE_DEF          0                /* 尖尖费率延迟费默认值(0.0001) */
/**************************************************************************************************/
#define CP_SHARP_RATED_ELECT_PRICE_MAX                50000            /* 尖费率电费最大值(0.0001) */
#define CP_SHARP_RATED_ELECT_PRICE_MIN                0                /* 尖费率电费最小值(0.0001) */
#define CP_SHARP_RATED_ELECT_PRICE_DEF                10000            /* 尖费率电费默认值(0.0001) */

#define CP_SHARP_RATED_SERVICE_PRICE_MAX              50000            /* 尖费率服务费最大值(0.0001) */
#define CP_SHARP_RATED_SERVICE_PRICE_MIN              0                /* 尖费率服务费最小值(0.0001) */
#define CP_SHARP_RATED_SERVICE_PRICE_DEF              5000             /* 尖费率服务费默认值(0.0001) */

#define CP_SHARP_RATED_DELAY_PRICE_MAX                50000            /* 尖费率延迟费最大值(0.0001) */
#define CP_SHARP_RATED_DELAY_PRICE_MIN                0                /* 尖费率延迟费最小值(0.0001) */
#define CP_SHARP_RATED_DELAY_PRICE_DEF                0                /* 尖费率延迟费默认值(0.0001) */
/**************************************************************************************************/
#define CP_PEAK_RATED_ELECT_PRICE_MAX                 50000            /* 峰费率电费最大值(0.0001) */
#define CP_PEAK_RATED_ELECT_PRICE_MIN                 0                /* 峰费率电费最小值(0.0001) */
#define CP_PEAK_RATED_ELECT_PRICE_DEF                 10000            /* 峰费率电费默认值(0.0001) */

#define CP_PEAK_RATED_SERVICE_PRICE_MAX               50000            /* 峰费率服务费最大值(0.0001) */
#define CP_PEAK_RATED_SERVICE_PRICE_MIN               0                /* 峰费率服务费最小值(0.0001) */
#define CP_PEAK_RATED_SERVICE_PRICE_DEF               5000             /* 峰费率服务费默认值(0.0001) */

#define CP_PEAK_RATED_DELAY_PRICE_MAX                 50000            /* 峰费率延迟费最大值(0.0001) */
#define CP_PEAK_RATED_DELAY_PRICE_MIN                 0                /* 峰费率延迟费最小值(0.0001) */
#define CP_PEAK_RATED_DELAY_PRICE_DEF                 0                /* 峰费率延迟费默认值(0.0001) */
/**************************************************************************************************/
#define CP_FLAT_RATED_ELECT_PRICE_MAX                 50000            /* 平费率电费最大值(0.0001) */
#define CP_FLAT_RATED_ELECT_PRICE_MIN                 0                /* 平费率电费最小值(0.0001) */
#define CP_FLAT_RATED_ELECT_PRICE_DEF                 10000            /* 平费率电费默认值(0.0001) */

#define CP_FLAT_RATED_SERVICE_PRICE_MAX               50000            /* 平费率服务费最大值(0.0001) */
#define CP_FLAT_RATED_SERVICE_PRICE_MIN               0                /* 平费率服务费最小值(0.0001) */
#define CP_FLAT_RATED_SERVICE_PRICE_DEF               5000             /* 平费率服务费默认值(0.0001) */

#define CP_FLAT_RATED_DELAY_PRICE_MAX                 50000            /* 平费率延迟费最大值(0.0001) */
#define CP_FLAT_RATED_DELAY_PRICE_MIN                 0                /* 平费率延迟费最小值(0.0001) */
#define CP_FLAT_RATED_DELAY_PRICE_DEF                 0                /* 平费率延迟费默认值(0.0001) */
/**************************************************************************************************/
#define CP_VALLEY_RATED_ELECT_PRICE_MAX               50000            /* 谷费率电费最大值(0.0001) */
#define CP_VALLEY_RATED_ELECT_PRICE_MIN               0                /* 谷费率电费最小值(0.0001) */
#define CP_VALLEY_RATED_ELECT_PRICE_DEF               10000            /* 谷费率电费默认值(0.0001) */

#define CP_VALLEY_RATED_SERVICE_PRICE_MAX             50000            /* 谷费率服务费最大值(0.0001) */
#define CP_VALLEY_RATED_SERVICE_PRICE_MIN             0                /* 谷费率服务费最小值(0.0001) */
#define CP_VALLEY_RATED_SERVICE_PRICE_DEF             5000             /* 谷费率服务费默认值(0.0001) */

#define CP_VALLEY_RATED_DELAY_PRICE_MAX               50000            /* 谷费率延迟费最大值(0.0001) */
#define CP_VALLEY_RATED_DELAY_PRICE_MIN               0                /* 谷费率延迟费最小值(0.0001) */
#define CP_VALLEY_RATED_DELAY_PRICE_DEF               0                /* 谷费率延迟费默认值(0.0001) */
/**************************************************************************************************/

#define CP_UNIT_PRICE_MAX                             100000           /* 单价最大值：10元 */
#define CP_UNIT_PRICE_MIN                             3000             /* 单价最小值：0.3元 */
#define CP_UNIT_PRICE_DEFAULT                         10000            /* 单价默认值：1元 */
#define CP_PERIOD_RATED_NUMBER_DEFAULT                0x04             /* 时段费率号默认值：谷费率 */
#endif /* (defined(CP_USING_V2G) || defined(CP_USING_OFFLINE_BILLING)) */

enum config_name{
    CONFIG_ITEM_PILE_NUMBER,
    CONFIG_ITEM_IP_DOMAIN,
    CONFIG_ITEM_PORT,

    CONFIG_ITEM_MODULE_MODEL,
    CONFIG_ITEM_MODULE_GROUP_NUM,
    CONFIG_ITEM_MODULE_NUM_GROUP_1,
    CONFIG_ITEM_MODULE_NUM_GROUP_2,
    CONFIG_ITEM_MODULE_NUM_GROUP_3,
    CONFIG_ITEM_MODULE_NUM_GROUP_4,

    OCONFIG_ITEM_GUN_NUMBER,

    CONFIG_ITEM_SUPORT_LOCAL,
    CONFIG_ITEM_SUPORT_LOCAL_STOP,
    CONFIG_ITEM_SUPORT_INSULATION,
    CONFIG_ITEM_SUPORT_VIN,
    CONFIG_ITEM_SUPORT_PARALLEL,
    CONFIG_ITEM_SUPORT_PARALLELRELAY,
    CONFIG_ITEM_SUPORT_PLUGCHARGE,
    CONFIG_ITEM_SUPORT_CARD,
    CONFIG_ITEM_SUPORT_MODULE_SLIENCE,
    CONFIG_ITEM_SUPORT_PASSWORD_START,
    CONFIG_ITEM_SUPORT_OFFLINE_CARD,
    CONFIG_ITEM_SUPORT_MODE_SELECT,
    CONFIG_ITEM_SUPORT_V2G,
    CONFIG_ITEM_SUPORT_BATVOLT_DETECT,
    CONFIG_ITEM_SUPORT_BCLTIMOUT_DETECT,
    CONFIG_ITEM_SUPORT_FAST_PROTOCOL,
    CONFIG_ITEM_SUPORT_YT_PROTOCOL,
    CONFIG_ITEM_SUPORT_BAY_PROTOCOL,
    CONFIG_ITEM_SUPORT_PROTOCOL_GB_T,
    CONFIG_ITEM_SUPORT_BMS_SEVERAL_FRAME,
    CONFIG_ITEM_CURRENT_MODE_A,
    CONFIG_ITEM_CURRENT_MODE_B,
    CONFIG_ITEM_CURRENT_V2G_MODE_A,
    CONFIG_ITEM_CURRENT_V2G_MODE_B,

    CONFIG_ITEM_CARD_TYPE,
    CONFIG_ITEM_GUN1_CC14V_MAX,
    CONFIG_ITEM_GUN1_CC14V_MIN,
    CONFIG_ITEM_GUN1_CC16V_MAX,
    CONFIG_ITEM_GUN1_CC16V_MIN,
    CONFIG_ITEM_GUN1_CC112V_MAX,
    CONFIG_ITEM_GUN1_CC112V_MIN,
    CONFIG_ITEM_GUN2_CC14V_MAX,
    CONFIG_ITEM_GUN2_CC14V_MIN,
    CONFIG_ITEM_GUN2_CC16V_MAX,
    CONFIG_ITEM_GUN2_CC16V_MIN,
    CONFIG_ITEM_GUN2_CC112V_MAX,
    CONFIG_ITEM_GUN2_CC112V_MIN,

    CONFIG_ITEM_GUN1_CURR_OFFSET,
    CONFIG_ITEM_GUN2_CURR_OFFSET,
    CONFIG_ITEM_CHARGE_MODE_VALIDITY,
    CONFIG_ITEM_V2G_MODE_VALIDITY,
    CONFIG_ITEM_FAN_PWM_PERIOD,
    CONFIG_ITEM_FAN_TIMER_PRESCALER,
    CONFIG_ITEM_OUT_PEAK_CURRENT,

    CONFIG_ITEM_SUPORT_BSM,
    CONFIG_ITEM_SUPORT_BCS,
    CONFIG_ITEM_SUPORT_AUXPOWER24V,
    CONFIG_ITEM_SUPORT_OFFLINE_BILLING,

    CONFIG_ITEM_SUPORT_MELECT_STRATEGY,
    CONFIG_ITEM_SUPORT_BATVOLT_STRATEGY,
    CONFIG_ITEM_SUPORT_CHARGE_CURR_STRATEGY,
    CONFIG_ITEM_SUPORT_ELIMINATE_MODULE,
    CONFIG_ITEM_SUPORT_GBT_ELOCK,
    CONFIG_ITEM_SUPORT_GBT_OC,
    CONFIG_ITEM_SUPORT_PEAK_CURRENT_OUT,

    CONFIG_ITEM_INPUT_OVERVOL,
    CONFIG_ITEM_INPUT_UNDERVOL,
    CONFIG_ITEM_OUTPUT_OVERVOL,
    CONFIG_ITEM_OUTPUT_UNDERVOL,
    CONFIG_ITEM_OUTPUT_OVERCUR,
    CONFIG_ITEM_SOC_STOP,
    CONFIG_ITEM_OVERTEMP_WARN,
    CONFIG_ITEM_OVERTEMP_STOP,
    CONFIG_ITEM_OVERTEMP_RECOVER,
    CONFIG_ITEM_OVERTEMP_SETCUR,
    CONFIG_ITEM_ELOSS_PROPORTION,
    CONFIG_ITEM_MODE_PARAMETER_A,
    CONFIG_ITEM_MODE_PARAMETER_B,
    CONFIG_ITEM_V2G_MODE_PARAMETER_A,
    CONFIG_ITEM_V2G_MODE_PARAMETER_B,
    CONFIG_ITEM_FAN_WORK_TIME,
    CONFIG_ITEM_SMODULE_OUTCURR_MAX,
    CONFIG_ITEM_LIGHTING_LAMP_SHOUR,
    CONFIG_ITEM_LIGHTING_LAMP_EHOUR,
    CONFIG_ITEM_LIGHTING_LAMP_SMIN,
    CONFIG_ITEM_LIGHTING_LAMP_EMIN,
    CONFIG_ITEM_DISCHARGE_AS_OF_SOC,

    /**************out***************/
    CONFIG_ITEM_OUTEN_AC,
    CONFIG_ITEM_OUTEN_ELOCK,
    CONFIG_ITEM_OUTEN_FAN,
    /**************in***************/
    CONFIG_ITEM_INEN_SCRAM,
    CONFIG_ITEM_INEN_GATE,
    CONFIG_ITEM_INEN_ACRELAY,
    CONFIG_ITEM_INEN_DCRELAY,
    CONFIG_ITEM_INEN_PARALLEL_RELAY,
    CONFIG_ITEM_INEN_MATRIX_RELAY,
    CONFIG_ITEM_INEN_FAN,
    CONFIG_ITEM_INEN_ELOCK,
    CONFIG_ITEM_INEN_TEMPPRO,
    CONFIG_ITEM_INEN_PROTECT_LIGHT,
    CONFIG_ITEM_INEN_GUNSITE,
    CONFIG_ITEM_INEN_CIRCUIT_BREAKER,
    CONFIG_ITEM_INEN_FLOOD,
    CONFIG_ITEM_INEN_SMOKE,
    CONFIG_ITEM_INEN_POUR,
    CONFIG_ITEM_INEN_LIQUID,
    CONFIG_ITEM_INEN_FUSE,

    CONFIG_ITEM_INNEG_SCRAM,
    CONFIG_ITEM_INNEG_GATE,
    CONFIG_ITEM_INNEG_ACRELAY,
    CONFIG_ITEM_INNEG_DCRELAY,
    CONFIG_ITEM_INNEG_PARALLEL_RELAY,
    CONFIG_ITEM_INNEG_MATRIX_RELAY,
    CONFIG_ITEM_INNEG_FAN,
    CONFIG_ITEM_INNEG_ELOCK,
    CONFIG_ITEM_INNEG_PROTECT_LIGHT,
    CONFIG_ITEM_INNEG_GUNSITE,
    CONFIG_ITEM_INNEG_CIRCUIT_BREAKER,
    CONFIG_ITEM_INNEG_FLOOD,
    CONFIG_ITEM_INNEG_SMOKE,
    CONFIG_ITEM_INNEG_POUR,
    CONFIG_ITEM_INNEG_LIQUID,
    CONFIG_ITEM_INNEG_FUSE,

    CONFIG_ITEM_QRCODE_PRE,
    CONFIG_ITEM_QRCODE_SUF,
    CONFIG_ITEM_METER_NOA,
    CONFIG_ITEM_METER_NOB,
    CONFIG_ITEM_METER_MODEL,
    CONFIG_ITEM_METER_CHECK_WAY,
    CONFIG_ITEM_METER_BAUDRATE,

    CONFIG_ITEM_RATED_OUTPUT_VOLTAGE,
    CONFIG_ITEM_MAX_OUTPUT_VOLTAGE,
    CONFIG_ITEM_MIN_OUTPUT_VOLTAGE,
    CONFIG_ITEM_RATED_LIMIT_CURRENT,
    CONFIG_ITEM_MAX_LIMIT_CURRENT,
    CONFIG_ITEM_MIN_LIMIT_CURRENT,
    CONFIG_ITEM_SYSTEM_POWER_TOTAL,
    CONFIG_ITEM_ALLOCATION_WAY,
    CONFIG_ITEM_DEVICE_TYPE,
    CONFIG_ITEM_LP_MODULE,
    CONFIG_ITEM_LED_LANGUAGE,
    CONFIG_ITEM_GUNVOLT_LIMIT,
    CONFIG_ITEM_VIN_WHITELIST,
    CONFIG_ITEM_CARD_WHITELIST,
    CONFIG_ITEM_SCREEN_PASSWORD,
    CONFIG_ITEM_CARD_KEY,
    CONFIG_ITEM_CARD_BLOCK_SN,
    CONFIG_ITEM_HELP_PHONE,
    CONFIG_ITEM_USER_IDENTITY,
    CONFIG_ITEM_REGISTER_CODE,
    CONFIG_ITEM_NET_TYPE,
    CONFIG_ITEM_TEMINAL_ADDRA,
    CONFIG_ITEM_TEMINAL_ADDRB,
    CONFIG_ITEM_LOGIN_USER_NAME,
    CONFIG_ITEM_LOGIN_USER_PASSWORD,
    CONFIG_ITEM_LIQUID_DEV,
    CONFIG_ITEM_LIQUID_CNT,

#if (defined(CP_USING_V2G) || defined(CP_USING_OFFLINE_BILLING))
    CONFIG_ITEM_BILLING_RULE,       /* 计费规则数据：为倒数第三项 */
#endif /* (defined(CP_USING_V2G) || defined(CP_USING_OFFLINE_BILLING)) */
    CONFIG_ITEM_TARGET_PLATFORM,    /* 目标平台数据：为倒数第二项 */
    CONFIG_ITEM_MONITOR_PLATFORM,   /* 监控平台数据：为倒数第一项 */

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
    CONFIG_ITEM_TARGET_PLATFORM_ADDITIONAL,   /* 目标平台数据(额外存储区)：为倒数第一项 */
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */

    CONFIG_ITEM_SIZE,
};

#pragma pack(1)
#if (defined(CP_USING_V2G) || defined(CP_USING_OFFLINE_BILLING))
struct period_time{                                        /** 用于离线计费 */
    uint8_t shour;                                         /** 时段开始：小时 */
    uint8_t smin;                                          /** 时段开始：分钟 */
    uint8_t ehour;                                         /** 时段结束：小时 */
    uint8_t emin;                                          /** 时段结束：分钟 */
    uint8_t rate_number;                                   /** 费率号 */
};

struct sys_billing_rule{
    struct{
        uint32_t elect : 24;                               /** 时段电费价格(精度：0.0001) */
        uint32_t service : 24;                             /** 时段服务费价格(精度：0.0001) */
        uint32_t delay : 24;                               /** 时段延迟费价格(精度：0.0001) */
        uint32_t reserve : 24;
    }period_price[CP_PERIOD_MAX];            /** 时段价格 */
    uint8_t rate_number[CP_PERIOD_MAX];      /** 时段费率号 */
    struct period_time time[CP_RATED_TYPE_NUM_MAX][CP_RATED_TYPE_PERIOD_NUM];
    uint32_t rate_elect_price[CP_RATED_TYPE_NUM_MAX];   /** 尖尖、尖、峰、平、谷费率电费价格(精度：0.0001) */
    uint32_t rate_service_price[CP_RATED_TYPE_NUM_MAX]; /** 尖尖、尖、峰、平、谷费率服务费价格(精度：0.0001) */
    uint32_t rate_delay_price[CP_RATED_TYPE_NUM_MAX];   /** 尖尖、尖、峰、平、谷费率延迟价格(精度：0.0001) */
};
#endif /* (defined(CP_USING_V2G) || defined(CP_USING_OFFLINE_BILLING)) */

struct config_item{
    uint8_t name;
    uint32_t user_section;    /* 最高4位用于表示 user_data 的长度， 最低10位用于表示配置项的长度*/
    uint8_t* config_index;
    void* user_data;
};
#pragma pack()

#define SYS_CONFIG_OSDELAY(ms)   rt_thread_mdelay(ms)

int32_t sys_string_contain_ctrl_char(const char* string, uint16_t slen);
int32_t sys_string_is_pure_digital_(const char* string, uint16_t slen);
int32_t sys_string_is_pure_digital_alphabet(const char* string, uint16_t slen);

int32_t chargepile_config_init(void);
int32_t chargepile_check_config(void);
int32_t system_config_init_if(void);

#ifdef APP_INCLUDE_TARGET_PLATFORM
#if (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID)
int32_t sys_storage_config_tp_additional_region(void);
int32_t sys_tp_additional_check_config(void);
int32_t sys_tp_additional_config_init(void);
#endif /* (APP_TARGET_PLATFORM_ID == NET_OCPP_PLATFORM_ID) */
#endif /* #ifdef APP_INCLUDE_TARGET_PLATFORM */

int32_t sys_storage_config_item(void);
int32_t sys_sync_config_item_content(enum config_name name, void* data, uint32_t len);
uint8_t* sys_read_config_item_content(enum config_name name, uint8_t is_user_content);

uint8_t sys_get_single_group_module_num(uint8_t group);
int16_t sys_get_power_percent(void);
uint32_t sys_percent_convert_to_power(uint16_t percent);
uint32_t sys_get_single_module_power(void);
uint32_t sys_query_system_max_power(void);
uint8_t sys_get_module_model(void);
uint8_t sys_get_module_group_num(void);
uint8_t* sys_get_module_num_single_group(void);

int32_t sys_vin_whitelists_storage(void);
uint8_t *sys_vin_code_get(uint8_t index);
int32_t sys_vin_whitelists_add(uint8_t *data, uint8_t len);
int32_t sys_vin_whitelists_query(uint8_t *data, uint8_t len);
int32_t sys_vin_whitelists_delete(uint8_t *data, uint8_t len);
int32_t sys_vin_whitelists_clear(void);

int32_t sys_card_number_whitelists_storage(void);
uint8_t *sys_card_number_get(uint8_t index);
int32_t sys_card_number_whitelists_add(uint8_t *data, uint8_t len);
int32_t sys_card_number_whitelists_query(uint8_t *data, uint8_t len);
int32_t sys_card_number_whitelists_delete(uint8_t *data, uint8_t len);
int32_t sys_card_number_whitelists_clear(void);

int32_t sys_card_uid_whitelists_storage(void);
uint8_t *sys_card_uid_get(uint8_t index);
int32_t sys_card_uid_whitelists_add(uint8_t *data, uint8_t len);
int32_t sys_card_uid_whitelists_query(uint8_t *data, uint8_t len);
int32_t sys_card_uid_whitelists_delete(uint8_t *data, uint8_t len);
int32_t sys_card_uid_whitelists_clear(void);

int32_t sys_period_time_format_valid(void *t);
int32_t sys_period_time_continuous_valid(void *t, uint8_t tlen, uint8_t valid_count);
int32_t sys_period_time_resume_default(void *t, uint8_t tlen);

uint8_t sys_get_offbilling_rate_number(uint32_t curr_time);
uint32_t sys_get_offbilling_unit_price(uint32_t curr_time);
uint32_t sys_get_offbilling_elect_price(uint8_t rate_number);
uint32_t sys_get_offbilling_service_price(uint8_t rate_number);
uint32_t sys_get_offbilling_delay_price(uint8_t rate_number);

/**********************************************[照明灯相关]********************************************************/
/**********************************************[照明灯相关]********************************************************/
int32_t sys_lighting_lamp_time_valid(uint8_t shour, uint8_t ehour, uint8_t smin, uint8_t emin);

/**********************************************[CC1相关 CC1]********************************************************/
/**********************************************[CC1相关 CC1]********************************************************/
int32_t sys_cc1_range_valid(uint16_t cc12_max, uint16_t cc12_min, uint16_t cc6_max, uint16_t cc6_min, uint16_t cc4_max, uint16_t cc4_min);

/**********************************************[配置清除相关]********************************************************/
/**********************************************[配置清除相关]********************************************************/
/*********************************************************
 * 函数名        sys_config_info_clear_iflash
 * 功能            清除内部FLASH的配置信息
 * 参数
 * 返回           1：成功      0：失败
 ********************************************************/
int32_t sys_config_info_clear_iflash(void);

/*********************************************************
 * 函数名        sys_config_info_clear_eflash
 * 功能            清除外部FLASH的配置信息
 * 参数
 * 返回           1：成功      0：失败
 ********************************************************/
int32_t sys_config_info_clear_eflash(void);

/**********************************************[配置是否有效判断]********************************************************/
/**********************************************[配置是否有效判断]********************************************************/
/*********************************************************
 * 函数名        sys_config_valid_judge
 * 功能            用于判断指定配置项数据是否有效
 * 参数           name      配置名@enum config_name
 *        _config   配置数据
 *        len       配置数据长度
 * 返回           0：无效      1：有效
 ********************************************************/
int32_t sys_config_valid_judge(uint8_t name, void *_config, uint16_t len);

/*********************************************************
 * 函数名        sys_mode_validity_combine
 * 功能            模式有效性组合
 * 参数           name   模式名enum config_name
 *        mode   模式
 *        validity  有效性
 * 返回           组合值
 ********************************************************/
uint16_t sys_mode_validity_combine(uint8_t name, uint8_t mode, uint8_t validity);

/*********************************************************
 * 函数名        sys_mode_validity_divide
 * 功能            模式有效性分解
 * 参数           name   模式名enum config_name
 *        mode   模式
 *        port   枪口号
 * 返回           分解后的模式有效性真实值
 ********************************************************/
uint8_t sys_mode_validity_divide(uint8_t name, uint8_t mode, uint8_t port);

#endif /* APPLICATIONS_CHARGEPILE_CONFIG_H_ */
