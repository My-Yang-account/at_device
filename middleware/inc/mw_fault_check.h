/**
 ******************************************************************************
 * @file mw_fault_check.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_FAULT_CHECK_H_
#define MW_FAULT_CHECK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "thaisenChargLib.h"
#include "app_ofsm.h"

#define APP_ORIGIN_SYSFAULT_MAX                   16            /* 原来系统故障最大值(包含SIZE) */
#define APP_NEW_SYSFAULT_NUM                      500           /* 新系统故障数量 */
#define APP_SYSFAULT_OFFSET_MIN                   (360 - APP_ORIGIN_SYSFAULT_MAX)                     /* 系统故障偏移最小值 */
#define APP_SYSFAULT_OFFSET_MAX                   (APP_SYSFAULT_OFFSET_MIN + APP_NEW_SYSFAULT_NUM)  /* 系统故障偏移最大值 */
#define APP_USER_SYSFAULT_MIN_NEW_DEF             360           /* 新定义的系统故障最小值 */

#define APP_ORIGIN_SYSFAULT_STOPWAY_MAX           16            /* 原来因系统故障停充的停充原因最大值(为边界值) */
#define APP_SYSFAULT_STOPWAY_OFFSET               APP_SYSFAULT_OFFSET_MIN       /* 因系统故障停充的停充原因偏移 */
#define APP_USER_NONE_SYSFAULT_STOPWAY_MIN_NEW_DEF     860      /* 新定义的不是因系统故障停充的停充原因最小值 */

#define APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX      27            /* 原来不是因系统故障停充的停充原因最大值(为边界值) */
#define APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN      16            /* 原来不是因系统故障停充的停充原因最小值(为边界值) */
#define APP_ORIGIN_NONE_SYSFAULT_STOPWAY_NUM      (APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN + 0x01)  /* 原来不是因系统故障停充的停充原因最大数量 */
#define APP_NONE_SYSFAULT_STOPWAY_OFFSET          (860 - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX)    /* 不是因系统故障停充的停充原因偏移 */

#define APP_ORIGIN_USER_STOPWAY_MIN               47            /* 原来的自定义的停充原因最小值(为边界值) */
#define APP_USER_STOPWAY_NUM_MAX                  313           /* 自定义的停充原因最大数量 */
#define APP_USER_STOPWAY_OFFSET                   (APP_ORIGIN_USER_STOPWAY_MIN - APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX - 0x01)    /* 自定义的停充原因偏移 */
#define APP_ORIGIN_USER_STOPWAY_POWEROFF          56            /* 原来的自定义的停充原因:断电 */

/** 通信故障 */
enum communication_fault_t{
    APP_COMMUNICATE_FAULT_BHM = 0x01,                              /* BMS通讯故障：接收BHM 报文超时 */
    APP_COMMUNICATE_FAULT_BRM,                                     /* BMS通讯故障：接收BRM 报文超时 */
    APP_COMMUNICATE_FAULT_BCP,                                     /* BMS通讯故障：接收BCP 报文超时 */
    APP_COMMUNICATE_FAULT_BRO,                                     /* BMS通讯故障：接收BRO 报文超时 */
    APP_COMMUNICATE_FAULT_BRO_AA,                                  /* BMS通讯故障：接收BRO_AA 报文超时 */
    APP_COMMUNICATE_FAULT_BRO_AA_TO_00,                            /* BMS通讯故障：接收BRO 由AA变为00 */
    APP_COMMUNICATE_FAULT_BCS,                                     /* BMS通讯故障：接收BCS 报文超时 */
    APP_COMMUNICATE_FAULT_BCL,                                     /* BMS通讯故障：接收BCL 报文超时 */
    APP_COMMUNICATE_FAULT_BSM,                                     /* BMS通讯故障：接收BSM 报文超时 */
    APP_COMMUNICATE_FAULT_BMV,                                     /* BMS通讯故障：接收BMV 报文超时 */
    APP_COMMUNICATE_FAULT_BMT,                                     /* BMS通讯故障：接收BMT 报文超时 */
    APP_COMMUNICATE_FAULT_BSP,                                     /* BMS通讯故障：接收BSP 报文超时 */
    APP_COMMUNICATE_FAULT_BST,                                     /* BMS通讯故障：接收BST 报文超时 */
    APP_COMMUNICATE_FAULT_BSD,                                     /* BMS通讯故障：接收BSD 报文超时 */
    APP_COMMUNICATE_FAULT_SIZE,                                    /* BMS通讯故障： */
};

/** 充电故障 */
enum charge_fault_t{
    APP_CHARGE_FAULT_GUN_VOLT = thaisenGunVolt,                    /* 枪头电压故障 */
    APP_CHARGE_FAULT_INSULTA = thaisenInsult,                      /* 绝缘故障 */
    APP_CHARGE_FAULT_COMMON = thaisenCommon,                       /* BMS通信故障 */
    APP_CHARGE_FAULT_BATTERY_VOLT = thaisenBatteryVolt,            /* 电池电压故障 */
    APP_CHARGE_FAULT_READY_VOLT = thaisenReadyVolt,                /* 准备电压故障 */
    APP_CHARGE_FAULT_INSULT_VOLT = thaisenInsultVolt,              /* 绝缘电压故障 */
    APP_CHARGE_FAULT_YT_BFC = thaisenYTBFC,                        /* 宇通协议BFC故障 */

    APP_CHARGE_FAULT_NO_ERROR,                                     /* 无故障 */
};

/** 新的系统故障格式 */
#if 0
enum system_fault_t{
    ////////////////////////////////////////////////////////////////////////////////////
    //       原系统故障
    ////////////////////////////////////////////////////////////////////////////////////
    //       新增系统故障
    ////////////////////////////////////////////////////////////////////////////////////
    APP_SYS_FAULT_NO_ERROR = thaisenFaultSize,                     /* 无故障 */
};
#endif /* 0 */
/** 系统故障 */
enum system_fault_t{
    APP_SYS_FAULT_SCRAM = thaisenFaultScram,                       /* 急停故障 */                       //!< APP_SYS_FAULT_SCRAM
    APP_SYS_FAULT_CARD_READER = thaisenCardReader,                 /* 读卡器故障 */                      //!< APP_SYS_FAULT_CARD_READER
    APP_SYS_FAULT_DOOR = thaisenDoor,                              /* 门禁故障 */                       //!< APP_SYS_FAULT_DOOR
    APP_SYS_FAULT_AMMETER = thaisenFaultAmmeter,                   /* 电表故障 */                       //!< APP_SYS_FAULT_AMMETER
    APP_SYS_FAULT_CHARGE_MODULE = thaisenChargModule,              /* 充电模块故障 */                     //!< APP_SYS_FAULT_CHARGE_MODULE
    APP_SYS_FAULT_OVER_TEMP = thaisenFaultOverTemp,                /* 过温故障 */                       //!< APP_SYS_FAULT_OVER_TEMP
    APP_SYS_FAULT_OVER_VOLT = thaisenFaultOverVolt,                /* 过压故障 */                       //!< APP_SYS_FAULT_OVER_VOLT
    APP_SYS_FAULT_UNDER_VOLT = thaisenFaultUnderVolt,              /* 欠压故障 */                       //!< APP_SYS_FAULT_UNDER_VOLT
    APP_SYS_FAULT_OVER_CURR = thaisenFaultOverCurrent,             /* 过流故障 */                       //!< APP_SYS_FAULT_OVER_CURR
    APP_SYS_FAULT_RELAY = thaisenRelay,                            /* 主继电器故障 */                     //!< APP_SYS_FAULT_RELAY
    APP_SYS_FAULT_PARALLEL_RELAY = thaisenRelayParallel,           /* 并联继电器故障 */                    //!< APP_SYS_FAULT_PARALLEL_RELAY
    APP_SYS_FAULT_AC_RELAY = thaisenRelayAc,                       /* AC继电器故障 */                    //!< APP_SYS_FAULT_AC_RELAY
    APP_SYS_FAULT_ELOCK = thaisenElock,                            /* 电子锁故障 */                      //!< APP_SYS_FAULT_ELOCK
    APP_SYS_FAULT_AUXPOWER = thaisenAuxPower,                      /* 辅源故障 */                       //!< APP_SYS_FAULT_AUXPOWER
    APP_SYS_FAULT_FLASH = thaisenFaultFlash,                       /* flash故障 */                    //!< APP_SYS_FAULT_FLASH
    APP_SYS_FAULT_EEPROM = thaisenFaultEeprom,                     /* eeprom故障 */                   //!< APP_SYS_FAULT_EEPROM

    APP_SYS_FAULT_LIGHT_PRPTECT = thaisenFaultLightProtect + APP_SYSFAULT_OFFSET_MIN,        /* 防雷器故障 *///!< APP_SYS_FAULT_LIGHT_PRPTECT
    APP_SYS_FAULT_GUN_SITE = thaisenFaultGunSite + APP_SYSFAULT_OFFSET_MIN,                  /* 枪座故障 */ //!< APP_SYS_FAULT_GUN_SITE
    APP_SYS_FAULT_CIRCUIT_BREAKER = thaisenFaultCircuitBreaker + APP_SYSFAULT_OFFSET_MIN,    /* 断路器故障 *///!< APP_SYS_FAULT_CIRCUIT_BREAKER
    APP_SYS_FAULT_FLOODING = thaisenFaultFlooding + APP_SYSFAULT_OFFSET_MIN,                 /* 水浸故障 */ //!< APP_SYS_FAULT_FLOODING
    APP_SYS_FAULT_SMOKE = thaisenFaultSmoke + APP_SYSFAULT_OFFSET_MIN,                       /* 烟感故障 */ //!< APP_SYS_FAULT_SMOKE
    APP_SYS_FAULT_POUR = thaisenFaultPour + APP_SYSFAULT_OFFSET_MIN,                         /* 倾倒故障 */ //!< APP_SYS_FAULT_POUR
    APP_SYS_FAULT_LIQUID_COOLING = thaisenFaultLiquidCooling + APP_SYSFAULT_OFFSET_MIN,      /* 液冷故障 */ //!< APP_SYS_FAULT_LIQUID_COOLING
    APP_SYS_FAULT_FUSE = thaisenFaultFuse + APP_SYSFAULT_OFFSET_MIN,                         /* 熔断器故障 *///!< APP_SYS_FAULT_FUSE
    APP_SYS_FAULT_MAIN_CABINET = thaisenFaultMainCabinet + APP_SYSFAULT_OFFSET_MIN,          /* 主机柜故障 *///!< APP_SYS_FAULT_FUSE

    APP_SYS_FAULT_MAX,
    APP_SYS_FAULT_NO_ERROR = thaisenFaultSize,                     /* 无故障 */                        //!< APP_SYS_FAULT_NO_ERROR
};

/** 新的停充原因格式 */
#if 0
enum system_stop_way{
    ////////////////////////////////////////////////////////////////////////////////////
    //       原来因系统故障停充的停充原因
    ////////////////////////////////////////////////////////////////////////////////////
    //       原来不是因系统故障停充的停充原因
    ////////////////////////////////////////////////////////////////////////////////////
    APP_SYS_FAULT_NO_ERROR = thaisenFaultSize,                     /* 无故障 */
};
#endif /* 0 */
enum system_stop_way{
    APP_SYSTEM_STOP_WAY_SCRAM = thaisen_chargeCtl_stopWay_scram,                      /* 急停 */
    APP_SYSTEM_STOP_WAY_CARDREADER = thaisen_chargeCtl_stopWay_cardReader,            /* 读卡器 */
    APP_SYSTEM_STOP_WAY_DOOR = thaisen_chargeCtl_stopWay_door,                        /* 门禁 */
    APP_SYSTEM_STOP_WAY_AMMETER = thaisen_chargeCtl_stopWay_ammeter,                  /* 电表 */
    APP_SYSTEM_STOP_WAY_CHARGEMODULE = thaisen_chargeCtl_stopWay_chargModule,         /* 充电模块 */
    APP_SYSTEM_STOP_WAY_OVERTEMP = thaisen_chargeCtl_stopWay_OverTemp,                /* 过温 */
    APP_SYSTEM_STOP_WAY_OVERVOLT = thaisen_chargeCtl_stopWay_overVolt,                /* 过压 */
    APP_SYSTEM_STOP_WAY_UNDERVOLT = thaisen_chargeCtl_stopWay_underVolt,              /* 欠压 */
    APP_SYSTEM_STOP_WAY_OVERCURRENT = thaisen_chargeCtl_stopWay_OverCurrent,          /* 过流 */
    APP_SYSTEM_STOP_WAY_RELAY = thaisen_chargeCtl_stopWay_relay,                      /* 继电器 */
    APP_SYSTEM_STOP_WAY_PARALLEL_RELAY = thaisen_chargeCtl_stopWay_Parallel_relay,    /* 继电器 */
    APP_SYSTEM_STOP_WAY_AC_RELAY = thaisen_chargeCtl_stopWay_Ac_relay,                /* 继电器 */
    APP_SYSTEM_STOP_WAY_ELECTRY_LOCK = thaisen_chargeCtl_stopWay_elock,               /* 电子锁 */
    APP_SYSTEM_STOP_WAY_AUXPOWER = thaisen_chargeCtl_stopWay_AuxPower,                /* 辅助电源 */
    APP_SYSTEM_STOP_WAY_FLASH = thaisen_chargeCtl_stopWay_flash,                      /* flash */
    APP_SYSTEM_STOP_WAY_EEPROM = thaisen_chargeCtl_stopWay_eeprom,                    /* eeprom */

    APP_SYSTEM_STOP_WAY_SHORTS = thaisen_chargeCtl_stopWay_short,              /* 短路 */
    APP_SYSTEM_STOP_WAY_GUNVOLT = thaisen_chargeCtl_stopWay_GunVolt,           /* 枪头电压 */
    APP_SYSTEM_STOP_WAY_INSULT = thaisen_chargeCtl_stopWay_Insult,             /* 绝缘 */
    APP_SYSTEM_STOP_WAY_COMMINICATION = thaisen_chargeCtl_stopWay_Common,      /* 通信 */
    APP_SYSTEM_STOP_WAY_BATTERY_VOLT = thaisen_chargeCtl_stopWay_BatteryVolt,  /* 电池电压 */
    APP_SYSTEM_STOP_WAY_PULL_GUN = thaisen_chargeCtl_stopWay_gun,              /* 拔枪 */
    APP_SYSTEM_STOP_WAY_CHARGE_FULL = thaisen_chargeCtl_stopWay_full,          /* 充满 */
    APP_SYSTEM_STOP_WAY_PASSIVE = thaisen_chargeCtl_stopWay_passive,           /*  */
    APP_SYSTEM_STOP_WAY_BST = thaisen_chargeCtl_stopWay_BST,
    APP_SYSTEM_STOP_WAY_READY_VOLT = thaisen_chargeCtl_stopWay_ReadyVolt,
    APP_SYSTEM_STOP_WAY_INSULT_VOLT = thaisen_chargeCtl_stopWay_InsultVolt,    /* 绝缘电压 */
    APP_SYSTEM_STOP_WAY_BSM =  thaisen_chargeCtl_stopWay_BSM,                  /* BSM */
    APP_SYSTEM_STOP_WAY_NULL = thaisen_chargeCtl_stopWay_size,

    APP_SYSTEM_STOP_WAY_APP_STOP = APP_SYSTEM_STOP_WAY_NULL + 0x01,               /* APP */
    APP_SYSTEM_STOP_WAY_ONLINECARD_STOP,        /* 在线卡 */
    APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP,       /* 离线卡 */
    APP_SYSTEM_STOP_WAY_SCREEN_STOP,            /* 屏幕 */
    APP_SYSTEM_STOP_WAY_NO_BALLANCE,            /* 余额不足 */
    APP_SYSTEM_STOP_WAY_REACH_ELECT,            /* 到达设定电量 */
    APP_SYSTEM_STOP_WAY_REACH_TIME,             /* 到达设定时间 */
    APP_SYSTEM_STOP_WAY_REACH_MONEY,            /* 到达设定余额 */
    APP_SYSTEM_STOP_WAY_AUTHEN_FAIL,            /* 鉴权失败 */
    APP_SYSTEM_STOP_WAY_POWER_OFF,              /* 断电 */
    APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL,       /* 电流异常 */
    APP_SYSTEM_STOP_WAY_SOC_LIMIT,              /* SOC限制 */
    APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID,    /* 主机柜禁止充电 */
    APP_SYSTEM_STOP_WAY_BRM_TIMEOUT,            /* 接收BRM超时 */
    APP_SYSTEM_STOP_WAY_BCP_TIMEOUT,            /* 接收BCP超时 */
    APP_SYSTEM_STOP_WAY_BRO_TIMEOUT,            /* 接收BRO超时 */
    APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT,         /* 接收BRO_AA超时 */
    APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT,   /* 启动中接收BCL超时 */
    APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT,   /* 启动中接收BCS超时 */
    APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT,   /* 充电中接收BCL超时 */
    APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT,  /* 充电中接收BCS超时 */
//    APP_SYSTEM_STOP_WAY_OFFLINE_CHARGE_TIME,    /* 达到离线可充电最长时间 */

    APP_SYSTEM_STOP_WAY_LIGHTPROTECT = thaisen_chargeCtl_stopWay_LightProtect + APP_SYSFAULT_STOPWAY_OFFSET,        /* 防雷器 */
    APP_SYSTEM_STOP_WAY_GUNSITE = thaisen_chargeCtl_stopWay_GunSite + APP_SYSFAULT_STOPWAY_OFFSET,                  /* 枪座 */
    APP_SYSTEM_STOP_WAY_CIRCUIT_BREAKER = thaisen_chargeCtl_stopWay_CircuitBreaker + APP_SYSFAULT_STOPWAY_OFFSET,   /* 断路器 */
    APP_SYSTEM_STOP_WAY_FLOODING = thaisen_chargeCtl_stopWay_Flooding + APP_SYSFAULT_STOPWAY_OFFSET,                /* 水浸 */
    APP_SYSTEM_STOP_WAY_SMOKE = thaisen_chargeCtl_stopWay_Smoke + APP_SYSFAULT_STOPWAY_OFFSET,                      /* 烟感 */
    APP_SYSTEM_STOP_WAY_POUR = thaisen_chargeCtl_stopWay_Pour + APP_SYSFAULT_STOPWAY_OFFSET,                        /* 倾倒 */
    APP_SYSTEM_STOP_WAY_LIQUIDCOOLING = thaisen_chargeCtl_stopWay_LiquidCooling + APP_SYSFAULT_STOPWAY_OFFSET,      /* 液冷 */
    APP_SYSTEM_STOP_WAY_FUSE = thaisen_chargeCtl_stopWay_Fuse + APP_SYSFAULT_STOPWAY_OFFSET,                        /* 熔断器 */
    APP_SYSTEM_STOP_WAY_MAIN_CABINET = tthaisen_chargeCtl_stopWay_MainCabinet + APP_SYSFAULT_STOPWAY_OFFSET,        /* 主机柜故障 */

    APP_SYSTEM_STOP_WAY_YT_BFC = thaisen_chargeCtl_stopWay_BFC + APP_NONE_SYSFAULT_STOPWAY_OFFSET,                  /* 宇通协议BFC故障 */

    APP_SYSTEM_STOP_WAY_SIZE,
};

#if 0
/** 这是原来的故障码和停止原因码(要兼容) */
/** 充电故障 */
enum charge_fault_t
{
    CHARGE_FAULT_GUN_VOLT = thaisenGunVolt,
    CHARGE_FAULT_INSULTA = thaisenInsult,
    CHARGE_FAULT_COMMON = thaisenCommon,
    CHARGE_FAULT_BATTERY_VOLT = thaisenBatteryVolt,
    CHARGE_FAULT_READY_VOLT = thaisenReadyVolt,
    CHARGE_FAULT_INSULT_VOLT = thaisenInsultVolt,
    CHARGE_FAULT_NO_ERROR,
};

typedef enum thaisenFaultEnum
{
  thaisenFaultScram,
  thaisenCardReader,
  thaisenDoor,
  thaisenFaultAmmeter,
  thaisenChargModule,
  thaisenFaultOverTemp,
  thaisenFaultOverVolt,
  thaisenFaultUnderVolt,
  thaisenFaultOverCurrent,
  thaisenRelay,
  thaisenRelayParallel,
  thaisenRelayAc,
  thaisenElock,
  thaisenAuxPower,
  thaisenFaultFlash,
  thaisenFaultEeprom,
  thaisenFaultSize,
}thaisenFaultTy;

/** 系统故障 */
enum system_fault_t
{
    SYS_FAULT_SCRAM = thaisenFaultScram,             /* 急停故障 */
    SYS_FAULT_CARD_READER = thaisenCardReader,       /* 读卡器故障 */
    SYS_FAULT_DOOR = thaisenDoor,                    /* 门禁故障 */
    SYS_FAULT_AMMETER = thaisenFaultAmmeter,         /* 电表故障 */
    SYS_FAULT_CHARGE_MODULE = thaisenChargModule,    /* 充电模块故障 */
    SYS_FAULT_OVER_TEMP = thaisenFaultOverTemp,      /* 过温故障 */
    SYS_FAULT_OVER_VOLT = thaisenFaultOverVolt,      /* 过压故障 */
    SYS_FAULT_UNDER_VOLT = thaisenFaultUnderVolt,    /* 欠压故障 */
    SYS_FAULT_OVER_CURR = thaisenFaultOverCurrent,   /* 过流故障 */
    SYS_FAULT_RELAY = thaisenRelay,                   /* 主继电器故障 */
    SYS_FAULT_PARALLEL_RELAY = thaisenRelayParallel,  /* 并联继电器故障 */
    SYS_FAULT_AC_RELAY = thaisenRelayAc,              /* AC继电器故障 */
    SYS_FAULT_ELOCK = thaisenElock,                   /* 电子锁故障 */
    SYS_FAULT_AUXPOWER = thaisenAuxPower,             /* 辅源故障 */
    SYS_FAULT_FLASH = thaisenFaultFlash,             /* flash故障 */
    SYS_FAULT_EEPROM = thaisenFaultEeprom,           /* eeprom故障 */

    /* 新的故障码要在此处增加 */

    SYS_FAULT_NO_ERROR = thaisenFaultSize, -------------> /* 原来系统故障最大值(包含SIZE) */
};

typedef enum{
    thaisen_chargeCtl_stopWay_scram = 0,
    thaisen_chargeCtl_stopWay_cardReader,
    thaisen_chargeCtl_stopWay_door,
    thaisen_chargeCtl_stopWay_ammeter,
    thaisen_chargeCtl_stopWay_chargModule,
    thaisen_chargeCtl_stopWay_OverTemp,
    thaisen_chargeCtl_stopWay_overVolt,
    thaisen_chargeCtl_stopWay_underVolt,
    thaisen_chargeCtl_stopWay_OverCurrent,
    thaisen_chargeCtl_stopWay_relay,
    thaisen_chargeCtl_stopWay_Parallel_relay,
    thaisen_chargeCtl_stopWay_Ac_relay,
    thaisen_chargeCtl_stopWay_elock,
    thaisen_chargeCtl_stopWay_AuxPower,
    thaisen_chargeCtl_stopWay_flash,
    thaisen_chargeCtl_stopWay_eeprom,
    thaisen_chargeCtl_stopWay_short,
    thaisen_chargeCtl_stopWay_GunVolt,
    thaisen_chargeCtl_stopWay_Insult,
    thaisen_chargeCtl_stopWay_Common,
    thaisen_chargeCtl_stopWay_BatteryVolt,
    thaisen_chargeCtl_stopWay_gun,
    thaisen_chargeCtl_stopWay_full,
    thaisen_chargeCtl_stopWay_passive,
    thaisen_chargeCtl_stopWay_BST,
    thaisen_chargeCtl_stopWay_ReadyVolt,
    thaisen_chargeCtl_stopWay_InsultVolt,
    thaisen_chargeCtl_stopWay_BSM,
    thaisen_chargeCtl_stopWay_size,
}thaisenChargeCtlStopWayEn;

enum system_stop_way
{
    SYSTEM_STOP_WAY_SCRAM = thaisen_chargeCtl_stopWay_scram,                      /* 急停 */
    SYSTEM_STOP_WAY_CARDREADER = thaisen_chargeCtl_stopWay_cardReader,            /* 读卡器 */
    SYSTEM_STOP_WAY_DOOR = thaisen_chargeCtl_stopWay_door,                        /* 门禁 */
    SYSTEM_STOP_WAY_AMMETER = thaisen_chargeCtl_stopWay_ammeter,                  /* 电表 */
    SYSTEM_STOP_WAY_CHARGEMODULE = thaisen_chargeCtl_stopWay_chargModule,         /* 充电模块 */
    SYSTEM_STOP_WAY_OVERTEMP = thaisen_chargeCtl_stopWay_OverTemp,                /* 过温 */
    SYSTEM_STOP_WAY_OVERVOLT = thaisen_chargeCtl_stopWay_overVolt,                /* 过压 */
    SYSTEM_STOP_WAY_UNDERVOLT = thaisen_chargeCtl_stopWay_underVolt,              /* 欠压 */
    SYSTEM_STOP_WAY_OVERCURRENT = thaisen_chargeCtl_stopWay_OverCurrent,          /* 过流 */
    SYSTEM_STOP_WAY_RELAY = thaisen_chargeCtl_stopWay_relay,                      /* 继电器 */
    SYSTEM_STOP_WAY_PARALLEL_RELAY = thaisen_chargeCtl_stopWay_Parallel_relay,    /* 继电器 */
    SYSTEM_STOP_WAY_AC_RELAY = thaisen_chargeCtl_stopWay_Ac_relay,                /* 继电器 */
    SYSTEM_STOP_WAY_ELECTRY_LOCK = thaisen_chargeCtl_stopWay_elock,               /* 电子锁 */
    SYSTEM_STOP_WAY_AUXPOWER = thaisen_chargeCtl_stopWay_AuxPower,                /* 辅助电源 */
    SYSTEM_STOP_WAY_FLASH = thaisen_chargeCtl_stopWay_flash,                      /* flash */
    SYSTEM_STOP_WAY_EEPROM = thaisen_chargeCtl_stopWay_eeprom,                    /* eeprom */ // APP_ORIGIN_SYSFAULT_STOPWAY_MAX

    SYSTEM_STOP_WAY_SHORTS = thaisen_chargeCtl_stopWay_short,                     /* 短路 */   // APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MIN
    SYSTEM_STOP_WAY_GUNVOLT = thaisen_chargeCtl_stopWay_GunVolt,                  /* 枪头电压 */
    SYSTEM_STOP_WAY_INSULT = thaisen_chargeCtl_stopWay_Insult,                    /* 绝缘 */
    SYSTEM_STOP_WAY_COMMINICATION = thaisen_chargeCtl_stopWay_Common,             /* 通信 */
    SYSTEM_STOP_WAY_BATTERY_VOLT = thaisen_chargeCtl_stopWay_BatteryVolt,         /* 电池电压 */
    SYSTEM_STOP_WAY_PULL_GUN = thaisen_chargeCtl_stopWay_gun,                     /* 拔枪 */
    SYSTEM_STOP_WAY_CHARGE_FULL = thaisen_chargeCtl_stopWay_full,                 /* 充满 */
    SYSTEM_STOP_WAY_PASSIVE = thaisen_chargeCtl_stopWay_passive,                  /*  */
    SYSTEM_STOP_WAY_BST = thaisen_chargeCtl_stopWay_BST,
    SYSTEM_STOP_WAY_READY_VOLT = thaisen_chargeCtl_stopWay_ReadyVolt,
    SYSTEM_STOP_WAY_INSULT_VOLT = thaisen_chargeCtl_stopWay_InsultVolt,           /* 绝缘电压 */
    SYSTEM_STOP_WAY_BSM = thaisen_chargeCtl_stopWay_BSM,                          /* BSM */
    SYSTEM_STOP_WAY_NULL = thaisen_chargeCtl_stopWay_size,                                  // APP_ORIGIN_NONE_SYSFAULT_STOPWAY_MAX

    /** 从 SYSTEM_STOP_WAY_NULL - SYSTEM_STOP_WAY_APP_STOP 之间的暂时不使用 */

    SYSTEM_STOP_WAY_APP_STOP = 47,          /* APP */
    SYSTEM_STOP_WAY_ONLINECARD_STOP,        /* 在线卡 */
    SYSTEM_STOP_WAY_OFFLINECARD_STOP,       /* 离线卡 */
    SYSTEM_STOP_WAY_SCREEN_STOP,            /* 屏幕 */
    SYSTEM_STOP_WAY_NO_BALLANCE,            /* 余额不足 */
    SYSTEM_STOP_WAY_REACH_ELECT,            /* 到达设定电量 */
    SYSTEM_STOP_WAY_REACH_TIME,             /* 到达设定时间 */
    SYSTEM_STOP_WAY_REACH_MONEY,            /* 到达设定余额 */
    SYSTEM_STOP_WAY_AUTHEN_FAIL,            /* 鉴权失败 */
    SYSTEM_STOP_WAY_POWER_OFF,              /* 断电 */
    SYSTEM_STOP_WAY_CURRENT_ABNORMAL,       /* 电流异常 */
    SYSTEM_STOP_WAY_SOC_LIMIT,              /* SOC限制 */

    /** 从SYSTEM_STOP_WAY_SOC_LIMIT-159之间 的供上层定义的停充原因  */

    /** 原64-69是启动失败码(修改掉， 启动失败码从160开始， 102个) */
    /** 此处保留一定数量的启动失败码(暂定200个) */

    /** 新的因故障停充的和非故障停充的停充原因码要在此处增加(从360开始) */
    /** 和系统故障码一样的停充原因码(从360开始 500个) */
    /** 和系统故障码不一样的停充原因码(从860开始 500个) */

    SYSTEM_STOP_WAY_SIZE,
};
#endif /* 0 */

uint32_t* mw_get_system_fault_set(uint8_t gunno);
uint32_t* mw_get_charge_fault_set(uint8_t gunno);
uint16_t mw_get_system_stop_way(uint8_t gunno);
uint16_t mw_system_fault_convert(uint16_t code);
uint16_t mw_system_stop_way_convert(uint16_t stopway);

/**********************************************************************
 * 函数名         mw_query_bms_communicate_fault
 * 功能             查询BMS具体通讯故障
 * 参数             gunno      枪号
 * 返回             @enum communication_fault_t
 *********************************************************************/
enum communication_fault_t mw_query_bms_communicate_fault(uint8_t gunno);

/**********************************************************************
 * 函数名         mw_is_bms_communicate_repeat
 * 功能             判断是否与BMS进行了通讯重连
 * 参数             gunno      枪号
 * 返回             1：是     0：否
 *********************************************************************/
uint8_t mw_is_bms_communicate_repeat(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* MW_FAULT_CHECK_H_ */
