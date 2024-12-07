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
#include "app_ofsm.h"

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
#define SYSTEM_CONFIG_INFO_ADDR_IF        0x081E0000
/******************************************************************************/

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
#define CP_QRCODE_PARA_VENDOR_CODE                "1001"                                            /* 二维码配置参数：厂商代码 */
#define CP_QRCODE_PARA_RULE_VERSION               "1.0.1"                                           /* 二维码配置参数：二维码规则版本 */
#define CP_QRCODE_PARA_STRING_CODE_TYPE           "3"                                               /* 二维码配置参数：字符码类型 */

#define CP_QRCODE_PARA_PRODUCT_IDENTIFI           "FFFFFFFFFFFF"                                    /* 二维码配置参数：蓝牙MAC地址 */

#else
#define CP_QRCODE_CONFIG_FORMAT_DEFAULT           CP_SET_QRCODE_FORMAT_PREFIX                       /* 二维码配置格式 */
#define CP_QRCODE_GENERATE_FORMAT_DEFAULT         CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT   /* 二维码生成格式 */
#define CP_QRCODE_PREFIX_DEFAULT                  "http://www.ykccn.com/MPAGE/index.html?pNum="     /* 云快充二维码前缀 */
#endif

/** 桩号默认 */
/**------------------------------------------------------------*/
#define CP_PILE_NUMBER_DEFAULT                    "TX000000000001"                                  /* 默认桩号 */
/** 帮助电话默认 */
/**------------------------------------------------------------*/
#define CP_HELP_PHONE_DEFAULT                     "18621280288"                                     /* 默认帮助电话 */


/* module config */
#define MODULE_MODEL_DEFAULT                      2           /* 默认模块型号：永联 */
#define MODULE_MODEL_NUMBER                       5           /* 当前系统支持的模块型号数量 */
#define MODULE_GROUP_NUMBER_DEFAULT               4           /* 默认模块组数：4组 */
#define MODULE_GROUP_NUMBER_MAX                   4           /* 最大模块组数：4组 */
#define MODULE_NUMBER_SINGLE_DEFAULT              3           /* 默认单个组的模块数：3 */
#define MODULE_NUMBER_SINGLE_MAX                  8           /* 最大单个组的模块数：8 */

#define POWER_ALLOCATION_WAY_AVERAGE              0           /* 功率分配方式:均充 */
#define POWER_ALLOCATION_WAY_SEQ_PRIORITY         1           /* 功率分配方式:先到先得 */
#define POWER_ALLOCATION_WAY_POWER_PRIORITY       2           /* 功率分配方式:功率优先 */
#define POWER_ALLOCATION_WAY_SIZE                 3           /* 功率分配方式 */

#define SYSTEM_FUNCTION_SINGLE_TERMINAL           0         /* 单枪终端 */
#define SYSTEM_FUNCTION_AVERAGE_DOUBLE            1         /* 均充双枪 */
#define SYSTEM_FUNCTION_DOUBLE_WHOLE              2         /* 双枪终端 */
#define SYSTEM_FUNCTION_RECTIFIER_CABINET         3         /* 整流柜 */
#define SYSTEM_FUNCTION_DYNAMIC_SWITCH            4         /* 动态切换 */
#define SYSTEM_FUNCTION_SIZE                      5         /* 系统功能 */

/* protect info config */
#define PROTECT_POWER_PERCENT_VALUE_DEFAULT       100       /* 保护信息：默认功率百分比值 */
#define PROTECT_POWER_PERCENT_VALUE_MAX           100       /* 保护信息：功率百分比最大值 */
#define PROTECT_POWER_PERCENT_VALUE_MIN           1         /* 保护信息：功率百分比最小值 */

#define PROTECT_STOP_SOC_VALUE_DEFAULT            100       /* 保护信息：默认SOC停充值 */
#define PROTECT_STOP_SOC_VALUE_MAX                100       /* 保护信息：SOC停充值最大值 */
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

#define GUNVOLT_LIMIT_VALUE_MIN                   10 *100   /* 枪头电压最小值 */
#define GUNVOLT_LIMIT_VALUE_MAX                   600 *100  /* 枪头电压最大值 */

#define MODULE_RATED_OUTVOLT_DEF                  750       /* 模块额定输出电压默认值值 */
#define MODULE_RATED_OUTVOLT_MAX                  1200      /* 模块额定输出电压最大值 */
#define MODULE_RATED_OUTVOLT_MIN                  500       /* 模块额定输出电压最小值 */

#define CHARGEPILE_MAX_OUTVOLT_DEF                750       /* 桩最大输出电压默认值值 */
#define CHARGEPILE_MAX_OUTVOLT_MAX                1200      /* 桩最大输出电压最大值 */
#define CHARGEPILE_MAX_OUTVOLT_MIN                500       /* 桩最大输出电压最小值 */

#define CHARGEPILE_MIN_OUTVOLT_DEF                200       /* 桩最小输出电压默认值值 */
#define CHARGEPILE_MIN_OUTVOLT_MAX                300       /* 桩最小输出电压最大值 */
#define CHARGEPILE_MIN_OUTVOLT_MIN                100       /* 桩最小输出电压最小值 */

#define MODULE_RATED_LIMIT_CURR_DEF               30        /* 模块额定限电流默认值值 */
#define MODULE_RATED_LIMIT_CURR_MAX               150       /* 模块额定限电流最大值 */
#define MODULE_RATED_LIMIT_CURR_MIN               10        /* 模块额定限电流最小值 */

#define MODULE_MAX_LIMIT_CURR_DEF                 500       /* 模块最大限电流默认值值 */
#define MODULE_MAX_LIMIT_CURR_MAX                 1500      /* 模块最大限电流最大值 */
#define MODULE_MAX_LIMIT_CURR_MIN                 10        /* 模块最大限电流最小值 */

#define MODULE_MIN_LIMIT_CURR_DEF                 2         /* 模块最小限电流默认值值 */
#define MODULE_MIN_LIMIT_CURR_MAX                 2         /* 模块最小限电流最大值 */
#define MODULE_MIN_LIMIT_CURR_MIN                 0         /* 模块最小限电流最小值 */

#define COMPULSION_SET_VOLTAGE_DEF                750       /* 强制启动设定电压默认值 */
#define COMPULSION_SET_VOLTAGE_MAX                1200      /* 强制启动设定电压最大值 */
#define COMPULSION_SET_VOLTAGE_MIN                100       /* 强制启动设定电压最小值 */

#define COMPULSION_SET_CURRENT_DEF                0         /* 强制启动设定电流默认值 */
#define COMPULSION_SET_CURRENT_MAX                1500      /* 强制启动设定电流最大值 */
#define COMPULSION_SET_CURRENT_MIN                10        /* 强制启动设定电流最小值 */

#define CHARGEPILE_INPUT_OVERVOLT_DEF             550       /* 充电桩输入过压默认值值 */
#define CHARGEPILE_INPUT_UNDERVOLT_DEF            150       /* 充电桩输入欠压默认值值 */
#define CHARGEPILE_OUTPUT_OVERVOLT_DEF            2000      /* 充电桩输出过压默认值值 */
#define CHARGEPILE_OUTPUT_UNDERVOLT_DEF           100       /* 充电桩输出欠压默认值值 */
#define CHARGEPILE_OUTPUT_OVERCURR_DEF            500       /*充电桩输出过流默认值值 */

#define CHARGEPILE_CC12V_MAX                      125       /* CC1 12V上限 */
#define CHARGEPILE_CC12V_MIN                      115       /* CC1 12V下限 */

#define CHARGEPILE_CC6V_MAX                       65        /* CC1 6V上限 */
#define CHARGEPILE_CC6V_MIN                       55        /* CC1 6V下限 */

#define CHARGEPILE_CC4V_MAX                       45        /* CC1 4V上限 */
#define CHARGEPILE_CC4V_MIN                       35        /* CC1 4V下限 */

#define CHARGEPILE_ELOSS_PROPORTION_MIN           0         /* 电损比最小值(一位小数) */
#define CHARGEPILE_ELOSS_PROPORTION_MAX           100       /* 电损比最大值(一位小数) */
#define CHARGEPILE_ELOSS_PROPORTION_DEF           0         /* 电损比默认值(一位小数) */

/* net type */
#define CP_NETTYPE_4G                                 0x00             /* 联网方式：4G */
#define CP_NETTYPE_ETH                                0x01             /* 联网方式：以太网 */
#define CP_NETTYPE_SIZE                               0x02

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
    CONFIG_ITEM_CARD_TYPE,
    CONFIG_ITEM_CC14V_MAX,
    CONFIG_ITEM_CC14V_MIN,
    CONFIG_ITEM_CC16V_MAX,
    CONFIG_ITEM_CC16V_MIN,
    CONFIG_ITEM_CC112V_MAX,
    CONFIG_ITEM_CC112V_MIN,
    CONFIG_ITEM_SUPORT_BSM,
    CONFIG_ITEM_SUPORT_BCS,
    CONFIG_ITEM_SUPORT_AUXPOWER24V,
    CONFIG_ITEM_SUPORT_OFFLINE_BILLING,

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

    /**************out***************/
    CONFIG_ITEM_OUTEN_AC,
    CONFIG_ITEM_OUTEN_ELOCK,
    CONFIG_ITEM_OUTEN_FAN,
    /**************in***************/
    CONFIG_ITEM_INEN_SCRAM,
    CONFIG_ITEM_INEN_GATE,
    CONFIG_ITEM_INEN_ACRELAY,
    CONFIG_ITEM_INEN_DCRELAY,
    CONFIG_ITEM_INEN_FAN,
    CONFIG_ITEM_INEN_ELOCK,
    CONFIG_ITEM_INEN_TEMPPRO,

    CONFIG_ITEM_INNEG_SCRAM,
    CONFIG_ITEM_INNEG_GATE,
    CONFIG_ITEM_INNEG_ACRELAY,
    CONFIG_ITEM_INNEG_DCRELAY,
    CONFIG_ITEM_INNEG_FAN,
    CONFIG_ITEM_INNEG_ELOCK,

    CONFIG_ITEM_QRCODE_PRE,
    CONFIG_ITEM_QRCODE_SUF,
    CONFIG_ITEM_METER_NOA,
    CONFIG_ITEM_METER_NOB,
    CONFIG_ITEM_METER_MODEL,

    CONFIG_ITEM_RATED_OUTPUT_VOLTAGE,
    CONFIG_ITEM_MAX_OUTPUT_VOLTAGE,
    CONFIG_ITEM_MIN_OUTPUT_VOLTAGE,
    CONFIG_ITEM_RATED_LIMIT_CURRENT,
    CONFIG_ITEM_MAX_LIMIT_CURRENT,
    CONFIG_ITEM_MIN_LIMIT_CURRENT,
    CONFIG_ITEM_SYSTEM_POWER_TOTAL,
    CONFIG_ITEM_ALLOCATION_WAY,
    CONFIG_ITEM_DEVICE_TYPE,
    CONFIG_ITEM_GUNVOLT_LIMIT,
    CONFIG_ITEM_VIN_WHITELIST,
    CONFIG_ITEM_CARD_WHITELIST,
    CONFIG_ITEM_SCREEN_PASSWORD,
    CONFIG_ITEM_HELP_PHONE,
    CONFIG_ITEM_NET_TYPE,

    CONFIG_ITEM_BILLING_RULE,       /* 计费规则数据：为倒数第三项 */
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
uint32_t sys_percent_convert_to_power(uint8_t percent);
uint32_t sys_query_system_max_power(void);
uint8_t sys_get_module_model(void);
uint8_t sys_get_module_group_num(void);
uint8_t* sys_get_module_num_single_group(void);

int32_t sys_vin_whitelists_storage(void);
uint8_t *sys_vin_code_get(uint8_t index);
int32_t sys_vin_whitelists_add(uint8_t *data, uint8_t len);
int32_t sys_vin_whitelists_query(uint8_t *data, uint8_t len);
int32_t sys_vin_whitelists_delete(uint8_t *data, uint8_t len);

int32_t sys_card_number_whitelists_storage(void);
uint8_t *sys_card_number_get(uint8_t index);
int32_t sys_card_number_whitelists_add(uint8_t *data, uint8_t len);
int32_t sys_card_number_whitelists_query(uint8_t *data, uint8_t len);
int32_t sys_card_number_whitelists_delete(uint8_t *data, uint8_t len);

int32_t sys_card_uid_whitelists_storage(void);
uint8_t *sys_card_uid_get(uint8_t index);
int32_t sys_card_uid_whitelists_add(uint8_t *data, uint8_t len);
int32_t sys_card_uid_whitelists_query(uint8_t *data, uint8_t len);
int32_t sys_card_uid_whitelists_delete(uint8_t *data, uint8_t len);

int32_t sys_period_time_format_valid(void *t);
int32_t sys_period_time_continuous_valid(void *t, uint8_t tlen, uint8_t valid_count);
int32_t sys_period_time_resume_default(void *t, uint8_t tlen);

uint8_t sys_get_offbilling_rate_number(uint32_t curr_time);
uint32_t sys_get_offbilling_unit_price(uint32_t curr_time);
uint32_t sys_get_offbilling_elect_price(uint8_t rate_number);
uint32_t sys_get_offbilling_service_price(uint8_t rate_number);
uint32_t sys_get_offbilling_delay_price(uint8_t rate_number);

#endif /* APPLICATIONS_CHARGEPILE_CONFIG_H_ */
