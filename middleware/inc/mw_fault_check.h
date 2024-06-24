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

#define APP_LIBRARY_DETECT_FAULT_MASK    0xFFFFFFFD    /* 库检测的故障：低16位 */

/** 充电故障 */
enum charge_fault_t{
    APP_CHARGE_FAULT_GUN_VOLT = thaisenGunVolt,
    APP_CHARGE_FAULT_INSULTA = thaisenInsult,
    APP_CHARGE_FAULT_COMMON = thaisenCommon,
    APP_CHARGE_FAULT_BATTERY_VOLT = thaisenBatteryVolt,
    APP_CHARGE_FAULT_READY_VOLT = thaisenReadyVolt,
    APP_CHARGE_FAULT_INSULT_VOLT = thaisenInsultVolt,
    APP_CHARGE_FAULT_NO_ERROR,
};

/** 系统故障 */
enum system_fault_t{
    APP_SYS_FAULT_SCRAM = thaisenFaultScram,             /* 急停故障 */
    APP_SYS_FAULT_CARD_READER = thaisenCardReader,       /* 读卡器故障 */
    APP_SYS_FAULT_DOOR = thaisenDoor,                    /* 门禁故障 */
    APP_SYS_FAULT_AMMETER = thaisenFaultAmmeter,         /* 电表故障 */
    APP_SYS_FAULT_CHARGE_MODULE = thaisenChargModule,    /* 充电模块故障 */
    APP_SYS_FAULT_OVER_TEMP = thaisenFaultOverTemp,      /* 过温故障 */
    APP_SYS_FAULT_OVER_VOLT = thaisenFaultOverVolt,      /* 过压故障 */
    APP_SYS_FAULT_UNDER_VOLT = thaisenFaultUnderVolt,    /* 欠压故障 */
    APP_SYS_FAULT_OVER_CURR = thaisenFaultOverCurrent,   /* 过流故障 */
    APP_SYS_FAULT_RELAY = thaisenRelay,                   /* 主继电器故障 */
    APP_SYS_FAULT_PARALLEL_RELAY = thaisenRelayParallel,  /* 并联继电器故障 */
    APP_SYS_FAULT_AC_RELAY = thaisenRelayAc,              /* AC继电器故障 */
    APP_SYS_FAULT_ELOCK = thaisenElock,                   /* 电子锁故障 */
    APP_SYS_FAULT_AUXPOWER = thaisenAuxPower,             /* 辅源故障 */
    APP_SYS_FAULT_FLASH = thaisenFaultFlash,             /* flash故障 */
    APP_SYS_FAULT_EEPROM = thaisenFaultEeprom,           /* eeprom故障 */
    APP_SYS_FAULT_NO_ERROR = thaisenFaultSize,

    APP_USER_INFO_DEVICE_ERROR_PILE_NUMBER_LEN,          /* 桩号实际长度与记录长度不符 */
    APP_USER_INFO_DEVICE_ERROR_PILE_NUMBER_NONE,         /* 桩号未配置 */
    APP_USER_INFO_DEVICE_ERROR_SIZE,
};

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
    APP_SYSTEM_STOP_WAY_SHORTS = thaisen_chargeCtl_stopWay_short,                     /* 短路 */
    APP_SYSTEM_STOP_WAY_GUNVOLT = thaisen_chargeCtl_stopWay_GunVolt,                  /* 枪头电压 */
    APP_SYSTEM_STOP_WAY_INSULT = thaisen_chargeCtl_stopWay_Insult,                    /* 绝缘 */
    APP_SYSTEM_STOP_WAY_COMMINICATION = thaisen_chargeCtl_stopWay_Common,             /* 通信 */
    APP_SYSTEM_STOP_WAY_BATTERY_VOLT = thaisen_chargeCtl_stopWay_BatteryVolt,         /* 电池电压 */
    APP_SYSTEM_STOP_WAY_PULL_GUN = thaisen_chargeCtl_stopWay_gun,                     /* 拔枪 */
    APP_SYSTEM_STOP_WAY_CHARGE_FULL = thaisen_chargeCtl_stopWay_full,                 /* 充满 */
    APP_SYSTEM_STOP_WAY_PASSIVE = thaisen_chargeCtl_stopWay_passive,                  /*  */
    APP_SYSTEM_STOP_WAY_BST = thaisen_chargeCtl_stopWay_BST,
    APP_SYSTEM_STOP_WAY_READY_VOLT = thaisen_chargeCtl_stopWay_ReadyVolt,
    APP_SYSTEM_STOP_WAY_INSULT_VOLT = thaisen_chargeCtl_stopWay_InsultVolt,           /* 绝缘电压 */
    APP_SYSTEM_STOP_WAY_BSM =  thaisen_chargeCtl_stopWay_BSM,                          /* BSM */
    APP_SYSTEM_STOP_WAY_NULL = thaisen_chargeCtl_stopWay_size,

    APP_SYSTEM_STOP_WAY_APP_STOP = 47,          /* APP */
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

    APP_SYSTEM_STOP_WAY_SIZE,
};

uint32_t* mw_get_system_fault_set(uint8_t gunno);
uint32_t* mw_get_charge_fault_set(uint8_t gunno);
enum system_stop_way mw_get_system_stop_way(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* MW_FAULT_CHECK_H_ */
