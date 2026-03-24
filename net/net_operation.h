/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-02     我的杨yang       the first version
 */
#ifndef NET_PACK_NET_OPERATION_H_
#define NET_PACK_NET_OPERATION_H_

#include "net_pack_config.h"
#include "thaisenBMS.h"

#define NETDATA_DEBUG_DIR_RECV                         0x00
#define NETDATA_DEBUG_DIR_SEND                         0x01

/** general fault code */
#define NET_GENERAL_FAULT_SCRAM                        0x00    /* 急停故障 */
#define NET_GENERAL_FAULT_CARD_READER                  0x01    /* 读卡器故障 */
#define NET_GENERAL_FAULT_DOOR                         0x02    /* 门禁故障 */
#define NET_GENERAL_FAULT_AMMETER                      0x03    /* 电表故障 */
#define NET_GENERAL_FAULT_CHARGE_MODULE                0x04    /* 充电模块故障 */
#define NET_GENERAL_FAULT_OVER_TEMP                    0x05    /* 过温故障 */
#define NET_GENERAL_FAULT_OVER_VOLT                    0x06    /* 过压故障 */
#define NET_GENERAL_FAULT_UNDER_VOLT                   0x07    /* 欠压故障 */
#define NET_GENERAL_FAULT_OVER_CURR                    0x08    /* 过流故障 */
#define NET_GENERAL_FAULT_MAIN_RELAY                   0x09    /* 主继电器故障 */
#define NET_GENERAL_FAULT_PARALLEL_RELAY               0x0A    /* 并联继电器故障 */
#define NET_GENERAL_FAULT_AC_RELAY                     0x0B    /* AC继电器故障 */
#define NET_GENERAL_FAULT_ELOCK                        0x0C    /* 电子锁故障 */
#define NET_GENERAL_FAULT_AUXPOWER                     0x0D    /* 辅源故障 */
#define NET_GENERAL_FAULT_FLASH                        0x0E    /* flash故障 */
#define NET_GENERAL_FAULT_EEPROM                       0x0F    /* eeprom故障 */
#define NET_GENERAL_FAULT_LIGHT_PRPTECT                0x10    /* 防雷器故障 */
#define NET_GENERAL_FAULT_GUN_SITE                     0x11    /* 枪座故障 */
#define NET_GENERAL_FAULT_CIRCUIT_BREAKER              0x12    /* 断路器故障 */
#define NET_GENERAL_FAULT_FLOODING                     0x13    /* 水浸故障 */
#define NET_GENERAL_FAULT_SMOKE                        0x14    /* 烟感故障 */
#define NET_GENERAL_FAULT_POUR                         0x15    /* 倾倒故障 */
#define NET_GENERAL_FAULT_LIQUID_COOLING               0x16    /* 液冷故障 */
#define NET_GENERAL_FAULT_FUSE                         0x17    /* 熔断器故障 */
#define NET_GENERAL_FAULT_MAIN_CABINET_OFFLINE         0x18    /* 主机柜离线故障 */
#define NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_1          0x19    /* 系统故障：矩阵正负接触器KPN1-1 */
#define NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_2          0x1A    /* 系统故障：矩阵正负接触器KPN1-2 */
#define NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_3          0x1B    /* 系统故障：矩阵正负接触器KPN1-3 */
#define NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_1          0x1C    /* 系统故障：矩阵正负接触器KPN2-1 */
#define NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_2          0x1D    /* 系统故障：矩阵正负接触器KPN2-2 */
#define NET_GENERAL_FAULT_MATRIX_RELAY_KPN3_1          0x1E    /* 系统故障：矩阵正负接触器KPN3-1 */
#define NET_GENERAL_FAULT_SLAVE_DEVICE_OFFLINE         0x1F    /* 系统故障：从设备离线 */
#define NET_GENERAL_FAULT_FAN                          0x20    /* 系统故障：风扇 */
#define NET_GENERAL_FAULT_MAINCABINET_SCRAM            0x21    /* 系统故障：主机柜急停 */
#define NET_GENERAL_FAULT_MAINCABINET_GATE             0x22    /* 系统故障：主机柜门禁 */
#define NET_GENERAL_FAULT_MAINCABINET_PDUFAULT         0x23    /* 系统故障：主机柜开关板故障 */
#define NET_GENERAL_FAULT_MAINCABINET_MODULEFAULT      0x24    /* 系统故障：主机柜模块 */
#define NET_GENERAL_FAULT_MAINCABINET_CONFIG           0x25    /* 系统故障：主机柜配置项 */
#define NET_GENERAL_FAULT_MAINCABINET_ACRELAY          0x26    /* 系统故障：主机柜交流接触器 */
#define NET_GENERAL_FAULT_MAINCABINET_SMOKE            0x27    /* 系统故障：主机柜烟感报警 */
#define NET_GENERAL_FAULT_MAINCABINET_POUR             0x28    /* 系统故障：主机柜倾倒 */
#define NET_GENERAL_FAULT_MAINCABINET_FLOODING         0x29    /* 系统故障：主机柜水浸 */
#define NET_GENERAL_FAULT_MAINCABINET_OTHER            0x2A    /* 系统故障：主机柜其它故障 */
#define NET_GENERAL_FAULT_MAINCABINET_LIGHT_PROTECT    0x2B    /* 系统故障：主机柜防雷故障 */
#define NET_GENERAL_FAULT_DEVICE_IS_LOCKED             0x2C    /* 系统故障：设备已锁定 */
#define NET_GENERAL_FAULT_SIZE                         0x2D    /* 无故障 */

/** system stop reason code */
#define NET_STOP_REASON_SCRAM                          0x00    /* 急停故障 */
#define NET_STOP_REASON_CARD_READER                    0x01    /* 读卡器故障 */
#define NET_STOP_REASON_DOOR                           0x02    /* 门禁故障 */
#define NET_STOP_REASON_AMMETER                        0x03    /* 电表故障 */
#define NET_STOP_REASON_CHARGE_MODULE                  0x04    /* 充电模块故障 */
#define NET_STOP_REASON_OVER_TEMP                      0x05    /* 过温故障 */
#define NET_STOP_REASON_OVER_VOLT                      0x06    /* 过压故障 */
#define NET_STOP_REASON_UNDER_VOLT                     0x07    /* 欠压故障 */
#define NET_STOP_REASON_OVER_CURR                      0x08    /* 过流故障 */
#define NET_STOP_REASON_MAIN_RELAY                     0x09    /* 主继电器故障 */
#define NET_STOP_REASON_PARALLEL_RELAY                 0x0A    /* 并联继电器故障 */
#define NET_STOP_REASON_AC_RELAY                       0x0B    /* AC继电器故障 */
#define NET_STOP_REASON_ELOCK                          0x0C    /* 电子锁故障 */
#define NET_STOP_REASON_AUXPOWER                       0x0D    /* 辅源故障 */
#define NET_STOP_REASON_FLASH                          0x0E    /* flash故障 */
#define NET_STOP_REASON_EEPROM                         0x0F    /* eeprom故障 */
#define NET_STOP_REASON_SIZE                           0x10    /* 无故障 */

/** networked way */
#define NET_NETWORKED_WAY_4G                           "1"     /* 联网方式：4G */
#define NET_NETWORKED_WAY_ETH                          "2"     /* 联网方式：以太网 */

/** device type */
#define NET_DEV_TYPE_AVERAGE_DOUBLEGUN                 "1"     /* 设备类型：均充双枪 */
#define NET_DEV_TYPE_DYNAMIC_DOUBLEGUN                 "2"     /* 设备类型：动态双枪 */
#define NET_DEV_TYPE_SINGLEGUN_TERMINAL                "3"     /* 设备类型：单枪终端 */
#define NET_DEV_TYPE_DOUBLEGUN_TERMINAL                "4"     /* 设备类型：双枪终端 */
#define NET_DEV_TYPE_MAIN_CABINET                      "5"     /* 设备类型：主机柜 */
#define NET_DEV_TYPE_SUPER_SINGLEGUN                   "6"     /* 设备类型：单枪超充 */

/** net fault */
#define NET_FAULT_PHYSICAL_LAYER                       (1 <<0) /* 物理层故障 */
#define NET_FAULT_SIM_CARD                             (1 <<1) /* SIM卡故障 */
#define NET_FAULT_DATA_LINK_LAYER                      (1 <<2) /* 数据链路层故障 */
#define NET_FAULT_MODULE_INIT                          (1 <<3) /* 模块初始化故障 */

/** external socket state */
#define NET_ESOCKET_STATE_OPEN                         0x00    /* 外部socket 状态：打开 */
#define NET_ESOCKET_STATE_CLOSE                        0x01    /* 外部socket 状态：关闭 */

/** boot result */
#define NET_CHARGE_BOOT_RESULT_SUCCESS                 0x00    /* 充电启动结果：成功 */
#define NET_CHARGE_BOOT_RESULT_NO_GUN                  0x01    /* 充电启动结果：未插枪 */
#define NET_CHARGE_BOOT_RESULT_NO_PULL_GUN             0x02    /* 充电启动结果：未拔枪 */
#define NET_CHARGE_BOOT_RESULT_IS_CHARGING             0x03    /* 充电启动结果：充电中 */
#define NET_CHARGE_BOOT_RESULT_IS_FAULTING             0x04    /* 充电启动结果：故障中 */

/** system data name */
#define NET_SYSTEM_DATA_NAME_PILE_NUMBER               0x00    /* 系统数据名：桩号 */
#define NET_SYSTEM_DATA_NAME_NETWORKED_WAY             0x01    /* 系统数据名：联网方式 '1'为4G， '2'为以太网 */
#define NET_SYSTEM_DATA_NAME_MCC                       0x02    /* 系统数据名：国家码，中国代码460即0x0C1C */
#define NET_SYSTEM_DATA_NAME_MNC                       0x03    /* 系统数据名：0移动，1联通(电信对应sid) 2非sim卡方式 */
#define NET_SYSTEM_DATA_NAME_LAC                       0x04    /* 系统数据名：lac(电信对应nid) 非sim卡方式时全0 */
#define NET_SYSTEM_DATA_NAME_CI                        0x05    /* 系统数据名：cellid(电信对应bid) 非sim卡方式时全0 */
#define NET_SYSTEM_DATA_NAME_IMEI                      0x06    /* 系统数据名：移动设备身份码（电子串号），不足部分后缀补0，非sim卡方式时全0 */
#define NET_SYSTEM_DATA_NAME_RAM                       0x07    /* 系统数据名：单位为K字节，充电设备RAM大小 */
#define NET_SYSTEM_DATA_NAME_ROM                       0x08    /* 系统数据名：单位为 K字节，充电设备ROM大小 */
#define NET_SYSTEM_DATA_NAME_MAC                       0x09    /* 系统数据名：sim卡方式时全0 */
#define NET_SYSTEM_DATA_NAME_ICCID                     0x0A    /* 系统数据名：SIM卡卡号 */
#define NET_SYSTEM_DATA_NAME_IMSI                      0x0B    /* 系统数据名： */
#define NET_SYSTEM_DATA_NAME_OPERATOR                  0x0C    /* 系统数据名： 运营商名称*/
#define NET_SYSTEM_DATA_NAME_DOMAIN                    0x0D    /* 系统数据名： 域名*/
#define NET_SYSTEM_DATA_NAME_PORT                      0x0E    /* 系统数据名： 端口号*/
#define NET_SYSTEM_DATA_NAME_VOLTAGE_MAX               0x0F    /* 系统数据名： 最大充电电压*/
#define NET_SYSTEM_DATA_NAME_VOLTAGE_MIN               0x10    /* 系统数据名： 最小充电电压*/
#define NET_SYSTEM_DATA_NAME_CURRENT_MAX               0x11    /* 系统数据名： 最大充电电流*/
#define NET_SYSTEM_DATA_NAME_MODULE_NUM                0x12    /* 系统数据名： 充电模块总数*/
#define NET_SYSTEM_DATA_NAME_SINGLE_MODULE_POWER       0x13    /* 系统数据名： 单个模块的功率*/
#define NET_SYSTEM_DATA_NAME_SYSTEM_POWER_MAX          0x14    /* 系统数据名： 系统最大功率 */
#define NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MAX        0x15    /* 系统数据名： 扩充最大电压*/
#define NET_SYSTEM_DATA_NAME_EXPAND_VOLTAGE_MIN        0x16    /* 系统数据名： 扩充最小电压*/
#define NET_SYSTEM_DATA_NAME_QRCODE                    0x17    /* 系统数据名： 二维码*/
#define NET_SYSTEM_DATA_NAME_HELP_PHONE                0x18    /* 系统数据名： 帮助电话*/
#define NET_SYSTEM_DATA_NAME_HARDWARE_VERSION          0x19    /* 系统数据名： 硬件版本(只用于OTA)*/
#define NET_SYSTEM_DATA_NAME_HARDWARE_INFO             0x1A    /* 系统数据名： 硬件信息(实际硬件版本)*/
#define NET_SYSTEM_DATA_NAME_PLATFORM_DATA             0x1B    /* 系统数据名： 平台存储数据*/
#define NET_SYSTEM_DATA_NAME_SIGNAL_STRENGTH           0x1C    /* 系统数据名： 信号强度*/
#define NET_SYSTEM_DATA_NAME_AMMETER_ADDRESS           0x1D    /* 系统数据名： 电表地址*/
#define NET_SYSTEM_DATA_NAME_TEMP_PROTECT_SWITCH       0x1E    /* 系统数据名： 温度保护开关*/
#define NET_SYSTEM_DATA_NAME_TEMP_PROTECT_STOP_VAL     0x1F    /* 系统数据名： 温度保护：停充温度*/
#define NET_SYSTEM_DATA_NAME_SOFTWARE_MODEL            0x20    /* 系统数据名： 软件型号*/
#define NET_SYSTEM_DATA_NAME_DEV_TYPE                  0x21    /* 系统数据名： 设备类型*/
#define NET_SYSTEM_DATA_NAME_SCREEN_PW                 0x22    /* 系统数据名： 屏幕密码*/
//////////////////////////////////////
#define NET_SYSTEM_DATA_NAME_SUPPORT_PLUG_AND_PLAY     0x23        /* 系统数据名：即插即充(VIN码充电)支持开关 */
#define NET_SYSTEM_DATA_NAME_SUPPORT_SWIP_CARD         0x24        /* 系统数据名：刷卡充电支持开关 */
#define NET_SYSTEM_DATA_NAME_ENABLE_GUN_VOLTAGE_10V    0x25        /* 系统数据名：外侧电压大于10V功能开关（开启关闭绝缘检查兼容逻辑设计） */
//#define NET_SYSTEM_DATA_NAME_ENABLE_BMS_FAULT_STOP     0x26      /* 系统数据名：BSM故障停止充电的开关 */
#define NET_SYSTEM_DATA_NAME_ENABLE_ELOCK_DETECT       0x27        /* 系统数据名：电子锁故障检测的开关 */
#define NET_SYSTEM_DATA_NAME_ENABLE_DOOR_DETECT        0x28        /* 系统数据名：门禁检测的开关 */
#define NET_SYSTEM_DATA_NAME_ENABLE_DOUBLEGUN_CHARGE   0x29        /* 系统数据名：双枪并充功能 */
#define NET_SYSTEM_DATA_NAME_ENABLE_CHARGE_OFFLINE     0x2A        /* 系统数据名：离线充电功能 */
#define NET_SYSTEM_DATA_NAME_ENABLE_AUX_DETECT         0x2B        /* 系统数据名：12/24V辅助电源选择功能开关 */
#define NET_SYSTEM_DATA_NAME__GUN_VOLTAGE_RANGE        0x2C        /* 系统数据名：外侧电压大于阀值 */
//////////////////////////////////////
#define NET_SYSTEM_DATA_NAME_DEVICE_ID                 0x2D        /* 系统数据名： 设备ID*/
#define NET_SYSTEM_DATA_NAME_SIZE                      0x2E

/** operator name */
#define NET_OPERATOR_NAME_CHINA_MOBILE                 0x00    /* 运营商名称：中国移动*/
#define NET_OPERATOR_NAME_CHINA_TELECOM                0x01    /* 运营商名称：中国电信*/
#define NET_OPERATOR_NAME_CHINA_UNICOM                 0x02    /* 运营商名称：中国联通*/
#define NET_OPERATOR_NAME_SIZE                         0x03

/** system data option */
#define NET_SYSTEM_DATA_OPTION_DATA_CONTENT            (0x01 <<0x00)    /* 系统数据选项：数据内容(此位不置位则返回内容长度) */
#define NET_SYSTEM_DATA_OPTION_PLAT_THA                (0x01 <<0x01)    /* 系统数据选项：钛享平台数据 */
#define NET_SYSTEM_DATA_OPTION_PLAT_YKC                (0x01 <<0x02)    /* 系统数据选项：云快充平台数据 */
#define NET_SYSTEM_DATA_OPTION_PLAT_YKC_MONITOR        (0x01 <<0x03)    /* 系统数据选项：云快充监控平台数据 */
#define NET_SYSTEM_DATA_OPTION_PLAT_XJ                 (0x01 <<0x04)    /* 系统数据选项：小桔平台数据 */
#define NET_SYSTEM_DATA_OPTION_PLAT_SL                 (0x01 <<0x05)    /* 系统数据选项：阳光乐通平台数据 */
#define NET_SYSTEM_DATA_OPTION_PLAT_YCP                (0x01 <<0x06)    /* 系统数据选项：越城平台数据 */
#define NET_SYSTEM_DATA_OPTION_PLAT_SGCC               (0x01 <<0x07)    /* 系统数据选项：国网平台数据 */
#define NET_SYSTEM_DATA_OPTION_SELECT_ALL              (0x01 <<0x08)    /* 系统数据选项：选择所有 */
#define NET_SYSTEM_DATA_OPTION_CARD_NUMBER_WHITELIST   (0x01 <<0x09)    /* 系统数据选项：卡号白名单数据 */
#define NET_SYSTEM_DATA_OPTION_CARD_NUMBER_JOINT       (0x01 <<0x0A)    /* 系统数据选项：连带卡号白名单数据 */
#define NET_SYSTEM_DATA_OPTION_CARD_UID_WHITELIST      (0x01 <<0x0B)    /* 系统数据选项：卡UID白名单数据 */
#define NET_SYSTEM_DATA_OPTION_CARD_UID_JOINT          (0x01 <<0x0C)    /* 系统数据选项：连带卡UID白名单数据 */
#define NET_SYSTEM_DATA_OPTION_VIN_WHITELIST           (0x01 <<0x0D)    /* 系统数据选项：VIN码白名单数据 */
#define NET_SYSTEM_DATA_OPTION_TARGET_PLAT             (0x01 <<0x0E)    /* 系统数据选项：目标平台数据 */
#define NET_SYSTEM_DATA_OPTION_TARGET_PLAT_ADDITIONAL  (0x01 <<0x0F)    /* 系统数据选项：目标平台数据(额外区域) */
#define NET_SYSTEM_DATA_OPTION_MONITOR_PLAT            (0x01 <<0x10)    /* 系统数据选项：监控平台数据 */

/** net device operate option */
#define NET_DEV_OPERATE_OPTION_RESET                   (0x01 <<0x00)    /* 网络设备操作选项：重启 */
#define NET_DEV_OPERATE_OPTION_CLOSE_SOCKET            (0x01 <<0x01)    /* 网络设备操作选项：关闭socket(修改外部socket状态为：关闭) */

/** platform mask */
#define NET_PLATFORM_MASK_MONITOR                      (0x01 <<0x00)    /* 监控平台掩码(用于网络设备操作) */
#define NET_PLATFORM_MASK_TARGET                       (0x01 <<0x01)    /* 目标平台掩码(用于网络设备操作) */
#define NET_PLATFORM_MASK_ALL                          (0x03 <<0x00)    /* 双平台掩码(用于网络设备操作) */

#define NET_SET_QRCODE_FORMAT_PREFIX                   0x01             /* 平台下发的二维码格式类型：前缀 */
#define NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN         0x02             /* 平台下发的二维码格式类型 ：前缀+设备号*/
#define NET_SET_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT    0x03             /* 平台下发的二维码格式类型 ：前缀+设备号+枪号 */
#define NET_SET_QRCODE_FORMAT_PORT                     0x04             /* 平台下发的二维码格式类型 ：枪号*/

#define NET_GENERATE_QRCODE_FORMAT_PREFIX              0x01             /* 终端生成的二维码格式类型：前缀 */
#define NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN    0x02             /* 终端生成的二维码格式类型：前缀+设备号 */
#define NET_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT  0x03          /* 终端生成的二维码格式类型：前缀+设备号+枪号 */
#define NET_GENERATE_QRCODE_FORMAT_PORT                0x04             /* 终端生成的二维码格式类型：枪号 */

/** system control state option */
#define NET_SYSTEM_CTRL_STATE_ACTION                   0x00             /* 系统控制项状态选项：动作时对应的反馈状态 */
#define NET_SYSTEM_CTRL_STATE_RELEASE                  0x01             /* 系统控制项状态选项：释放时对应的反馈状态 */

/** system control option */
#define NET_SYSTEM_CTRL_OPTION_ACTION                  0x00             /* 系统控制选项：动作 */
#define NET_SYSTEM_CTRL_OPTION_RELEASE                 0x01             /* 系统控制选项：释放 */
#define NET_SYSTEM_CTRL_OPTION_STATE                   0x02             /* 系统控制选项：获取状态 */

/** system control item */
#define NET_SYSTEM_CTRL_ITEM_ELOCK                     0x00             /* 系统控制项：电子锁 */
#define NET_SYSTEM_CTRL_ITEM_SIZE                      0x01             /* 系统控制项 */

/** system record option */
#define NET_SYSTEM_RECORD_OPTION_CHARGE                0x00             /* 系统数据选项：充电记录 */
#define NET_SYSTEM_RECORD_OPTION_FAULT                 0x01             /* 系统数据选项：故障记录 */
#define NET_SYSTEM_RECORD_OPTION_CNUM                  0x02             /* 系统数据选项：充电记录总数 */
#define NET_SYSTEM_RECORD_OPTION_FNUM                  0x03             /* 系统数据选项：故障记录总数 */

#define NET_THREAD_MONITOR_NAME_MAX                    0x08             /* 线程监控名字最大长度 */

#define NET_THREAD_RUNNING_OPTION_DELETE               (1 <<0x00)       /* 线程运行选项字：线程信息删除 */
#define NET_THREAD_RUNNING_OPTION_ENTRY_MAX            (1 <<0x01)       /* 线程运行选项字：最大容忍次数 */
#define NET_THREAD_RUNNING_OPTION_URGENT               (1 <<0x02)       /* 线程运行选项字：紧急(无需判断，直接处理) */
#define NET_THREAD_RUNNING_OPTION_NAME                 (1 <<0x03)       /* 线程运行选项字：线程名字 */

/** net parameter config */
enum para_config{
    NET_PARA_CONFIG_INDEX_FLASH_ERASE = 1,             /* 参数配置下标：flash擦除函数 */    //!< NET_PARA_CONFIG_INDEX_FLASH_ERASE
    NET_PARA_CONFIG_INDEX_FLASH_READ = 2,              /* 参数配置下标：flash读取函数 */    //!< NET_PARA_CONFIG_INDEX_FLASH_READ
    NET_PARA_CONFIG_INDEX_FLASH_WRITE = 3,             /* 参数配置下标：flash写函数 */     //!< NET_PARA_CONFIG_INDEX_FLASH_WRITE
    NET_PARA_CONFIG_INDEX_FLASH_WRITE_DIRECTLY = 4,    /* 参数配置下标：flash不带擦除的写函数 *///!< NET_PARA_CONFIG_INDEX_FLASH_WRITE_DIRECTLY
    NET_PARA_CONFIG_INDEX_DATA_UPDATA = 5,             /* 参数配置下标：数据更新 */         //!< NET_PARA_CONFIG_INDEX_DATA_UPDATA
    NET_PARA_CONFIG_INDEX_TIME_SYNC = 6,               /* 参数配置下标：时间同步 */         //!< NET_PARA_CONFIG_INDEX_TIME_SYNC
    NET_PARA_CONFIG_INDEX_GET_SYSTEM_DATA = 7,         /* 参数配置下标：获取系统数据 */       //!< NET_PARA_CONFIG_INDEX_GET_SYSTEM_DATA
    NET_PARA_CONFIG_INDEX_SET_SYSTEM_DATA = 8,         /* 参数配置下标：设置系统数据 */       //!< NET_PARA_CONFIG_INDEX_SET_SYSTEM_DATA
    NET_PARA_CONFIG_INDEX_SET_CARD_VIN = 9,            /* 参数配置下标：设置卡、VIN白名单 */   //!< NET_PARA_CONFIG_INDEX_SET_CARD_VIN
    NET_PARA_CONFIG_INDEX_QUERY_CARD_VIN = 10,         /* 参数配置下标：查询卡、VIN白名单 */   //!< NET_PARA_CONFIG_INDEX_QUERY_CARD_VIN
    NET_PARA_CONFIG_INDEX_DELETE_CARD_VIN = 11,        /* 参数配置下标：删除卡、VIN白名单 */   //!< NET_PARA_CONFIG_INDEX_DELETE_CARD_VIN
    NET_PARA_CONFIG_INDEX_CRC16_8005 = 12,             /* 参数配置下标：CRC16 校验 */     //!< NET_PARA_CONFIG_INDEX_CRC16_8005
    NET_PARA_CONFIG_INDEX_CRC32_UPDATE = 13,           /* 参数配置下标：CRC32 校验 */     //!< NET_PARA_CONFIG_INDEX_CRC32_UPDATE
    NET_PARA_CONFIG_INDEX_GET_BASE_DATA = 14,          /* 参数配置下标：设置基本数据 */       //!< NET_PARA_CONFIG_INDEX_GET_BASE_DATA
    NET_PARA_CONFIG_INDEX_SYSTEM_DATA_STORAGE = 15,    /* 参数配置下标：系统数据存储 */       //!< NET_PARA_CONFIG_INDEX_SYSTEM_DATA_STORAGE
    NET_PARA_CONFIG_INDEX_SYSTEM_CONTROL = 16,         /* 参数配置下标：系统控制 */         //!< NET_PARA_CONFIG_INDEX_SYSTEM_CONTROL
    NET_PARA_CONFIG_INDEX_QUERY_SYSTEM_RECORD = 17,    /* 参数配置下标：查询系统数据 */       //!< NET_PARA_CONFIG_INDEX_QUERY_SYSTEM_RECORD
    NET_PARA_CONFIG_INDEX_NDEV_OPERATE = 18,           /* 参数配置下标：网络设备操作 */       //!< NET_PARA_CONFIG_INDEX_NDEV_OPERATE
    NET_PARA_CONFIG_INDEX_THREAD_INIT = 19,            /* 参数配置下标：线程初始化 */        //!< NET_PARA_CONFIG_INDEX_THREAD_INIT
    NET_PARA_CONFIG_INDEX_THREAD_RUNNING = 20,         /* 参数配置下标：线程运行 */         //!< NET_PARA_CONFIG_INDEX_THREAD_RUNNING
    NET_PARA_CONFIG_INDEX_SIZE = 21,                                             //!< NET_PARA_CONFIG_INDEX_SIZE
};

#define NET_MY_ASSERT(para, index)                             \
    if(!para){                                                 \
        LOG_E("parameter is not config, index(%d)", index);    \
        while(1);                                              \
    }                                                          \

enum net_event{
    NET_OPERATION_EVENT_START_CHARGE,                   /* 启动充电事件 */
    NET_OPERATION_EVENT_STOP_CHARGE,                    /* 停止充电事件 */
    NET_OPERATION_EVENT_START_UPDATE,                   /* 启动升级事件 */
    NET_OPERATION_EVENT_REBOOT,                         /* 系统重启事件 */
    NET_OPERATION_EVENT_TIME_SYNC,                      /* 时间同步事件 */
    NET_OPERATION_EVENT_UPDATE_BILLING_RULE,            /* 更新计费规则事件 */
    NET_OPERATION_EVENT_SET_CHARGE_POWER,               /* 设置充电功率事件 */
    NET_OPERATION_EVENT_CARDAUTHORITY_SUCCESS,          /* 刷卡权限认证成功事件 */
    NET_OPERATION_EVENT_VINAUTHORITY_SUCCESS,           /* VIN码权限认证成功事件 */
    NET_OPERATION_EVENT_PASSWORD_AUTHORITY_SUCCESS,     /* 密码权限认证成功事件 */
    NET_OPERATION_EVENT_CARDAUTHORITY_FAIL,             /* 刷卡权限认证失败事件 */
    NET_OPERATION_EVENT_VINAUTHORITY_FAIL,              /* VIN码权限认证失败事件 */
    NET_OPERATION_EVENT_PASSWORD_AUTHORITY_FAIL,        /* 密码权限认证失败事件 */
    NET_OPERATION_EVENT_BALLANCE_UPDATE,                /* 余额更新事件 */
    NET_OPERATION_EVENT_FORBIDDEN_USE,                  /* 锁桩事件 */
    NET_OPERATION_EVENT_GROUND_DROP,                    /* 地锁下降事件 */
    NET_OPERATION_EVENT_GROUND_RISE,                    /* 地锁上升事件 */
    NET_OPERATION_EVENT_START_MERGE_CHARGE,             /* 并充启动充电事件 */
    NET_OPERATION_EVENT_CARDAUTHORITY_MERGE_SUCCESS,    /* 刷卡权限认证并充成功事件 */
    NET_OPERATION_EVENT_VINAUTHORITY_MERGE_SUCCESS,     /* VIN码权限认证并充成功事件 */
    NET_OPERATION_EVENT_CARDAUTHORITY_MERGE_FAIL,       /* 刷卡权限认证并充失败事件 */
    NET_OPERATION_EVENT_VINAUTHORITY_MERGE_FAIL,        /* VIN码权限认证并充失败事件 */
    NET_OPERATION_EVENT_SET_RESERVATION,                /* 设置预约事件 */
    NET_OPERATION_EVENT_CANCEL_RESERVATION,             /* 取消预约事件 */
    NET_OPERATION_EVENT_OFFLINECHARGE_LIMIT,            /* 离线可充电时长限制事件 */
    NET_OPERATION_EVENT_SYNC_MONITOR,                   /* 监控对时事件 */
};

enum net_ota_state{
    NET_OTA_STATE_NULL,
    NET_OTA_STATE_READY,
    NET_OTA_STATE_OPEN_LINK,
    NET_OTA_STATE_LOGIN_WAIT,
    NET_OTA_STATE_UPDATING,
    NET_OTA_STATE_SUCCESS,
    NET_OTA_STATE_FAIL,
};

enum network_state{
    NET_SOCKET_STATE_PHY,
    NET_SOCKET_STATE_SIM,
    NET_SOCKET_STATE_DATA_LINK,
    NET_SOCKET_STATE_MODULE_INIT,
    NET_SOCKET_STATE_OPEN,
    NET_SOCKET_STATE_LOGIN_WAIT,
    NET_SOCKET_STATE_LOGIN_SUCCESS,
};

enum net_external_socket_state{
    NET_ESOCKET_CLOSE,
    NET_ESOCKET_UP,
    NET_ESOCKET_OPEN,
};

/********* 断网原因 *********/
enum net_dis_reason{
    NET_DIS_REASON_CLOSE_PASSIVE,                                    /** socket 被动关闭 */
    NET_DIS_REASON_CLOSE_ACTIVE,                                     /** socket 主动关闭 */
    NET_DIS_REASON_HEARTBEAT_TIMEOUT,                                /** socket 心跳超时 */
    NET_DIS_REASON_SOCKET_PDP,                                       /** 4G模块PDP场景失效 */
    NET_DIS_REASON_CLOSE_MODULE,                                     /** 通信模块关闭 */
    NET_DIS_REASON_AT_PHYSICS,                                       /** AT指令(以太网物理层：查是否在线、查版本、修改波特率) */
    NET_DIS_REASON_CPIN_LK_MAC,                                      /** 查找SIM卡(以太网数据链路层：初始化芯片、寻线、开DHCP) */
    NET_DIS_REASON_CIMI_LK_MAC,                                      /** 查找CIMI号(以太网数据链路层：初始化芯片、寻线、开DHCP) */
    NET_DIS_REASON_SIGNAL_STRENGTH_LK_MAC,                           /** 查询信号强度(以太网数据链路层：初始化芯片、寻线、开DHCP) */
    NET_DIS_REASON_GSM_REGISTERED,                                   /** 注册GSM网络(以太网网络层：判断DHCP是否启动、获取IP信息、查询MAC地址) */
    NET_DIS_REASON_GPRS_REGISTERED,                                  /** 注册GPRS网络(以太网网络层：判断DHCP是否启动、获取IP信息、查询MAC地址) */
    NET_DIS_REASON_SIZE,                                             /**  */
};

enum net_enum{
    NET_ENUM_FALSE,
    NET_ENUM_TRUE,
    NET_ENUM_UNKNOW,
};

#pragma pack(1)

/** 目标平台socket信息 */
typedef struct{
    uint8_t program_state;      /** 程序运行状态 */
    uint8_t socket_state;       /** socket状态 */
    uint8_t open_count;         /** 连续重复open socket次数 */
    uint8_t login_count;        /** 连续重复登录次数 */
    uint8_t heartbeat_count;    /** 心跳超时次数 */
    uint8_t domain_is_prase;    /** 目标平台域名已解析 */
    uint8_t domain[0x10];       /** 点分十进制式IP */
    uint16_t port;              /** 端口号 */
}net_plat_socket_info_t;

typedef struct{
    enum net_ota_state state;
    int fd;
    uint16_t result;
    uint32_t progress;
    struct{
        uint8_t start_ota : 4;
        uint8_t permit_ota : 4;
    }flag;
}net_ota_info_t;

typedef struct{
    uint8_t gunno;
    uint8_t sindex;
    uint32_t stime;
    uint32_t etime;
    void* buf;
    uint16_t len;
    uint32_t option;
}net_record_info_t;

struct net_handle{
    uint8_t net_fault;
    uint8_t net_state;
    uint8_t esocket_state;
    void (*start_func)(void* handle);
    void (*data_updata)(void);
    void (*time_sync)(uint32_t timestamp);
    void* (*get_base_data)(uint8_t gunno);
    uint8_t* (*get_system_data)(uint8_t name, void *vector, uint32_t vlen, uint32_t option);
    int32_t (*set_system_data)(uint8_t name, uint8_t *data, uint16_t len, uint32_t option);
    int32_t (*para_config)(uint8_t platform, uint8_t config_index, void* para, void* handle);
    int32_t (*flash_erase)(uint32_t addr, uint32_t size);
    int32_t (*flash_read)(uint32_t addr, uint8_t* data, uint32_t size);
    int32_t (*flash_write)(uint32_t addr, uint8_t* data, uint32_t size);
    int32_t (*flash_write_directly)(uint32_t addr, uint8_t* data, uint32_t size);
    int32_t (*card_vin_whitelists_set)(uint8_t* data, uint8_t len, uint32_t option);
    int32_t (*card_vin_whitelists_query)(uint8_t* data, uint8_t len, uint32_t option);
    int32_t (*card_vin_whitelists_delete)(uint8_t* data, uint8_t len, uint32_t option);
    int32_t (*system_data_storage)(uint32_t option);
    int32_t (*query_system_record)(net_record_info_t *info);
    int32_t (*system_control)(uint8_t gunno, uint16_t item, uint8_t *para, uint32_t option);
    int32_t (*ndev_operate)(void* para, uint32_t option);
    uint16_t (*crc16_8005)(uint16_t init, const uint8_t *data, uint32_t len);
    uint32_t (*crc32_updtae)(uint32_t init, const uint8_t *data, uint32_t len);
    int32_t (*thread_init_hook)(void *thread, void *para, uint32_t plen, uint32_t option);
    int32_t (*thread_running)(void *thread, void *para, uint32_t plen, uint32_t option);
};

#pragma pack()

void NETDATA_DEBUG(const char *id, void *data, int len, uint8_t dir);

int32_t net_thread_init_hook(void *thread, void *para, uint32_t plen, uint32_t option);
int32_t net_thread_running(void *thread, void *para, uint32_t plen, uint32_t option);

void net_set_clear_ndev_reset_state(uint8_t plat_mask, uint8_t is_clear);
void net_operation_set_event(uint8_t gunno, uint8_t event);
uint8_t net_operation_get_event(uint8_t gunno, uint8_t event);
void net_operation_clear_event(uint8_t gunno, uint8_t event);

void net_operation_clear_net_fault(uint8_t event);
void net_operation_set_net_fault(uint8_t event);

void net_operation_set_esock_state(uint8_t state);
uint8_t net_operation_get_esock_state(void);

void net_operation_set_total_power(uint32_t power, uint8_t gunno);
uint32_t net_operation_get_total_power(uint8_t gunno);

struct net_handle* net_get_net_handle(void);
net_ota_info_t *net_get_ota_info(void);
net_plat_socket_info_t *net_operation_get_target_socket_info(uint8_t sync_data);
void net_operation_set_target_socket_domain(char* domain, uint8_t domain_len);
void net_operation_set_target_socket_port(uint16_t port);
void net_operation_insert_tplat_log(void *data, uint16_t len, uint8_t verify_result, const char* label);
void net_operation_tplat_info_trigger(uint8_t sync_data);

void net_operation_set_fees_gunno(uint8_t gunno, uint8_t is_appand);
uint8_t net_operation_is_gunno_updated_fees(uint8_t gunno);
void net_operation_updated_billing_trigger(void);
void net_dis_reason_store(int fd, uint8_t en);

#endif /* NET_PACK_NET_OPERATION_H_ */
