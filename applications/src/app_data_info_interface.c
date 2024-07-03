/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-24     我的杨yang       the first version
 */
#include "app_data_info_interface.h"
#include "serialScreen.h"
#include "chargepile_config.h"
#include "mw_temp.h"
#include "mw_meter.h"

static struct temperature s_temp_info;
static ota_info s_ota_info;

static struct app_version s_version;
static struct qrcode_info s_qrcode;
static struct charge_data s_data_of_charging;
static struct battery_info s_battery_info;
static struct bms_info s_bms_info;
static struct account_info s_account_info;
static struct fault_info s_fault_info;

/********************************************
 * 函数名      thaisen_app_get_ofsm_charge_state
 * 功能          获取充电状态
 * 参数         gunno            枪号
 * 返回         当前充电状态 (枪号不对则返回 OFSM_STATE_SIZE)
 *******************************************/
enum ofsm_state thaisen_app_get_ofsm_charge_state(uint8_t gunno)    // OK
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_OFSM_STATE_SIZE;
    }
    struct ofsm_info *ofsm_temp = get_ofsm_info(gunno);

    return ofsm_temp->state;
}

/********************************************
 * 函数名      thaisen_app_get_net_state
 * 功能          获取网络状态
 * 参数         无
 * 返回         当前网络状态
 *******************************************/
enum net_state thaisen_app_get_net_state(void)
{
    struct ofsm_info *ofsm_temp = get_ofsm_info(0x00);
    return (enum net_state)ofsm_temp->base.net_state;
}

/********************************************
 * 函数名      thaisen_app_get_app_version
 * 功能          获取版本信息
 * 参数         无
 * 返回         版本信息
 *******************************************/
struct app_version *thaisen_app_get_app_version(void)
{
    memset(s_version.version, '\0', sizeof(s_version.version));
	s_version.version[0] = SOFTWARE_VERSION + '0';
	s_version.version[1] = '.';
    sprintf((char *)&s_version.version[2], "%.2ld", SOFTWARE_SUBVERSION);
	s_version.version[4] = '.';
	sprintf((char *)&s_version.version[5], "%.2ld", SOFTWARE_REVISION);
	return &s_version;
}


/********************************************
 * 函数名      thaisen_app_get_gunno_qrcode
 * 功能          获取枪号二维码
 * 参数         gunno      枪号
 * 返回         二维码信息(枪号不对则返回 全0)
 *******************************************/
struct qrcode_info *thaisen_app_get_gunno_qrcode(uint8_t gunno) // OK
{
#define QRCODE_FEFAULT    "http://www.ykccn.com/MPAGE/index.html?pNum="          /* 云快充默认二维码 */

    uint8_t i, *p = NULL, qrcode_len = 0;

    memset(&s_qrcode, 0x00, sizeof(s_qrcode));
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return &s_qrcode;
    }

    if((*(sys_read_config_item_content(CONFIG_ITEM_QRCODE_PRE, 1))) == 0){
        memcpy(s_qrcode.qrcode, QRCODE_FEFAULT, strlen(QRCODE_FEFAULT));
        qrcode_len = strlen(QRCODE_FEFAULT);
        s_qrcode.qrcode_len = qrcode_len;
    }else{
        if(sizeof(s_qrcode.qrcode) < (*(sys_read_config_item_content(CONFIG_ITEM_QRCODE_PRE, 1)))){
            return &s_qrcode;
        }
        for (i = 0, p = sys_read_config_item_content(CONFIG_ITEM_QRCODE_PRE, 0); i < (*(sys_read_config_item_content(CONFIG_ITEM_QRCODE_PRE, 1))); i++) {
            s_qrcode.qrcode[i] = (*(p + i));
        }
        qrcode_len = i;
        s_qrcode.qrcode_len = qrcode_len;
    }

#ifndef USING_DAQI
    if(sizeof(s_qrcode.qrcode) < (*(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 1)) + qrcode_len)){
        return &s_qrcode;
    }
    sprintf((char*)(s_qrcode.qrcode + qrcode_len), "%s", sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0));
    qrcode_len = qrcode_len + (*(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 1)));
    s_qrcode.qrcode_len = qrcode_len;

    if(sizeof(s_qrcode.qrcode) < (3 + qrcode_len)){
        return &s_qrcode;
    }
    sprintf((char*)(s_qrcode.qrcode + qrcode_len), "%c%u", '0', gunno + 1);
    qrcode_len = qrcode_len + 2;
#endif /* USING_DAQI */

    s_qrcode.qrcode_len = qrcode_len;
    return &s_qrcode;
}

/********************************************
 * 函数名      thaisen_get_device_sn
 * 功能          获取设备编号
 * 参数         gunno      枪号
 * 返回         二维码信息(枪号不对则返回 全0)
 *******************************************/
void thaisen_get_device_sn(char *src, uint8_t length, uint8_t gunno)
{
    memset(src, '\0', length);
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    uint8_t pile_number_len = (*(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 1)));
    if(pile_number_len < length){
        sprintf(src, "%s", sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0));
    }else{
        return;
    }

    if(length > (pile_number_len + 3)){
        sprintf((src + strlen((const char*)src)), "0%d", (gunno + 1));
    }else{
        return;
    }
}

/********************************************
 * 函数名      thaisen_app_get_connect_state
 * 功能          获取充电枪连接状态
 * 参数         gunno      枪号
 * 返回         枪连接状态(枪号不对则返回 GUN_CONNECT_STATE_NO)
 *******************************************/
uint8_t thaisen_app_get_connect_state(uint8_t gunno) // OK
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_GUN_CONNECT_STATE_NO;
    }

    enum connect_state state;
    struct ofsm_info *ofsm_temp = get_ofsm_info(gunno);

    switch(ofsm_temp->base.cc1_state)
    {
    case CC1_12V:
    case CC1_6V:
        state = APP_GUN_CONNECT_STATE_NO;
        break;
    case CC1_4V:
        state = APP_GUN_CONNECT_STATE_YES;
        break;
    }

    return state;
}


//enum system_stop_way
//{
//    APP_SYSTEM_STOP_WAY_SCRAM = thaisen_chargeCtl_stopWay_scram,                      /* 急停 */
//    APP_SYSTEM_STOP_WAY_CARDREADER = thaisen_chargeCtl_stopWay_cardReader,            /* 读卡器 */
//    APP_SYSTEM_STOP_WAY_DOOR = thaisen_chargeCtl_stopWay_door,                        /* 门禁 */
//    APP_SYSTEM_STOP_WAY_AMMETER = thaisen_chargeCtl_stopWay_ammeter,                  /* 电表 */
//    APP_SYSTEM_STOP_WAY_CHARGEMODULE = thaisen_chargeCtl_stopWay_chargModule,         /* 充电模块 */
//    APP_SYSTEM_STOP_WAY_OVERTEMP = thaisen_chargeCtl_stopWay_OverTemp,                /* 过温 */
//    APP_SYSTEM_STOP_WAY_OVERVOLT = thaisen_chargeCtl_stopWay_overVolt,                /* 过压 */
//    APP_SYSTEM_STOP_WAY_UNDERVOLT = thaisen_chargeCtl_stopWay_underVolt,              /* 欠压 */
//    APP_SYSTEM_STOP_WAY_OVERCURRENT = thaisen_chargeCtl_stopWay_OverCurrent,          /* 过流 */
//    APP_SYSTEM_STOP_WAY_RELAY = thaisen_chargeCtl_stopWay_relay,                      /* 继电器 */
//    APP_SYSTEM_STOP_WAY_PARALLEL_RELAY = thaisen_chargeCtl_stopWay_Parallel_relay,    /* 继电器 */
//    APP_SYSTEM_STOP_WAY_AC_RELAY = thaisen_chargeCtl_stopWay_Ac_relay,                /* 继电器 */
//    APP_SYSTEM_STOP_WAY_ELECTRY_LOCK = thaisen_chargeCtl_stopWay_elock,               /* 电子锁 */
//    APP_SYSTEM_STOP_WAY_AUXPOWER = thaisen_chargeCtl_stopWay_AuxPower,                /* 辅助电源 */
//    APP_SYSTEM_STOP_WAY_FLASH = thaisen_chargeCtl_stopWay_flash,                      /* flash */
//    APP_SYSTEM_STOP_WAY_EEPROM = thaisen_chargeCtl_stopWay_eeprom,                    /* eeprom */
//    APP_SYSTEM_STOP_WAY_SHORTS = thaisen_chargeCtl_stopWay_short,                     /* 短路 */
//    APP_SYSTEM_STOP_WAY_GUNVOLT = thaisen_chargeCtl_stopWay_GunVolt,                  /* 枪头电压 */
//    APP_SYSTEM_STOP_WAY_INSULT = thaisen_chargeCtl_stopWay_Insult,                    /* 绝缘 */
//    APP_SYSTEM_STOP_WAY_COMMINICATION = thaisen_chargeCtl_stopWay_Common,             /* 通信 */
//    APP_SYSTEM_STOP_WAY_BATTERY_VOLT = thaisen_chargeCtl_stopWay_BatteryVolt,         /* 电池电压 */
//    APP_SYSTEM_STOP_WAY_PULL_GUN = thaisen_chargeCtl_stopWay_gun,                     /* 拔枪 */
//    APP_SYSTEM_STOP_WAY_CHARGE_FULL = thaisen_chargeCtl_stopWay_full,                 /* 充满 */
//    APP_SYSTEM_STOP_WAY_PASSIVE = thaisen_chargeCtl_stopWay_passive,                  /*  */
//    APP_SYSTEM_STOP_WAY_BST = thaisen_chargeCtl_stopWay_BST,
//    APP_SYSTEM_STOP_WAY_READY_VOLT = thaisen_chargeCtl_stopWay_ReadyVolt,
//    APP_SYSTEM_STOP_WAY_INSULT_VOLT = thaisen_chargeCtl_stopWay_InsultVolt,         /* 充电模块通讯 */
//    APP_SYSTEM_STOP_WAY_NULL = thaisen_chargeCtl_stopWay_size,
//
//    APP_SYSTEM_STOP_WAY_APP_STOP = 47,          /* APP */
//    APP_SYSTEM_STOP_WAY_ONLINECARD_STOP,        /* 在线卡 */
//    APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP,       /* 离线卡 */
//    APP_SYSTEM_STOP_WAY_SCREEN_STOP,            /* 屏幕 */
//    APP_SYSTEM_STOP_WAY_NO_BALLANCE,            /* 余额不足 */
//    APP_SYSTEM_STOP_WAY_REACH_ELECT,            /* 到达设定电量 */
//    APP_SYSTEM_STOP_WAY_REACH_TIME,             /* 到达设定时间 */
//    APP_SYSTEM_STOP_WAY_REACH_MONEY,            /* 到达设定余额 */
//    APP_SYSTEM_STOP_WAY_AUTHEN_FAIL,            /* 鉴权失败 */
//    APP_SYSTEM_STOP_WAY_POWER_OFF,              /* 断电 */
//    APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL,       /* 电流异常 */
//
//    APP_SYSTEM_STOP_WAY_SIZE,
//};
/********************************************
 * 函数名      thaisen_app_get_charge_stop_way
 * 功能          获取停充原因
 * 参数         gunno      枪号
 * 返回         原因(枪号不对则返回 SYSTEM_STOP_WAY_SIZE)
 *******************************************/
enum system_stop_way thaisen_app_get_charge_stop_way(uint8_t gunno) // OK
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_SYSTEM_STOP_WAY_SIZE;
    }

    enum system_stop_way charge_stop_way;
    struct ofsm_info *ofsm_temp = get_ofsm_info(gunno);

    if(ofsm_temp->base.reason_code >= APP_SYSTEM_STOP_WAY_SIZE){
        charge_stop_way = APP_SYSTEM_STOP_WAY_SIZE;
    }else{
        charge_stop_way = (enum system_stop_way)ofsm_temp->base.reason_code;
    }

    return charge_stop_way;
}


/********************************************
 * 函数名      thaisen_app_get_charge_info
 * 功能          获取充电信息
 * 参数         gunno      枪号
 * 返回         充电信息(枪号不对则返回 全0)
 *******************************************/
struct charge_data *thaisen_app_get_charge_info(uint8_t gunno) // OK
{
    memset(&s_data_of_charging, 0x00, sizeof(s_data_of_charging));

    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return &s_data_of_charging;
    }

    struct ofsm_info *ofsm_temp = get_ofsm_info(gunno);
    struct thaisenBMS_Charger_struct* bms_data = mw_get_bms_data(gunno);

    memcpy(s_data_of_charging.trade_number, ofsm_temp->base.transaction_number, sizeof(s_data_of_charging.trade_number));
    s_data_of_charging.charge_voltage = ofsm_temp->base.voltage_a;
    s_data_of_charging.charge_current = ofsm_temp->base.current_a;
    s_data_of_charging.charge_power = ofsm_temp->base.power_a;
    s_data_of_charging.charge_elect = ofsm_temp->base.elect_a;
    s_data_of_charging.charge_total_fee = ofsm_temp->base.fees_total;
    s_data_of_charging.charge_start_time = ofsm_temp->base.start_time;
    s_data_of_charging.charge_stop_time = ofsm_temp->base.stop_time;
    s_data_of_charging.charge_time = ofsm_temp->base.charge_time;
    s_data_of_charging.remain_charge_time = bms_data->BCS.SurplChgTime;
    s_data_of_charging.current_soc = bms_data->BCS.SOC;

    return &s_data_of_charging;
}


/********************************************
 * 函数名      thaisen_app_get_battery_info
 * 功能          获取电池信息
 * 参数         无
 * 返回         电池信息(枪号不对则返回 全0)
 *******************************************/
struct battery_info *thaisen_app_get_battery_info(uint8_t gunno)    // OK
{
    memset(&s_battery_info, 0x00, sizeof(s_battery_info));

    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return &s_battery_info;
    }

    struct thaisenBMS_Charger_struct* bms_data = mw_get_bms_data(gunno);

    s_battery_info.battery_type = bms_data->BRM.BatType;

    return &s_battery_info;
}

/********************************************
 * 函数名      thaisen_app_get_bms_info
 * 功能          获取BMS信息
 * 参数         无
 * 返回         BMS信息(枪号不对则返回 全0)
 *******************************************/
struct bms_info *thaisen_app_get_bms_info(uint8_t gunno)    // OK
{
    memset(&s_bms_info, 0x00, sizeof(s_bms_info));

    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return &s_bms_info;
    }
    struct thaisenBMS_Charger_struct* bms_data = mw_get_bms_data(gunno);

    s_bms_info.single_battery_max_voltage = bms_data->BCS.CellHigVolt;
    s_bms_info.bms_require_voltage = bms_data->BCL.BMSneedVolt;
    s_bms_info.bms_require_current = bms_data->BCL.BMSneedCurlt;

    return &s_bms_info;
}

///** 系统故障 */
//enum system_fault_t
//{
//    APP_SYS_FAULT_SCRAM = thaisenFaultScram,             /* 急停故障 */
//    APP_SYS_FAULT_CARD_READER = thaisenCardReader,       /* 读卡器故障 */
//    APP_SYS_FAULT_DOOR = thaisenDoor,                    /* 门禁故障 */
//    APP_SYS_FAULT_AMMETER = thaisenFaultAmmeter,         /* 电表故障 */
//    APP_SYS_FAULT_CHARGE_MODULE = thaisenChargModule,    /* 充电模块故障 */
//    APP_SYS_FAULT_OVER_TEMP = thaisenFaultOverTemp,      /* 过温故障 */
//    APP_SYS_FAULT_OVER_VOLT = thaisenFaultOverVolt,      /* 过压故障 */
//    APP_SYS_FAULT_UNDER_VOLT = thaisenFaultUnderVolt,    /* 欠压故障 */
//    APP_SYS_FAULT_OVER_CURR = thaisenFaultOverCurrent,   /* 过流故障 */
//    APP_SYS_FAULT_PARALLEL_RELAY = thaisenRelayParallel,  /* 并联继电器故障 */
//    APP_SYS_FAULT_AC_RELAY = thaisenRelayAc,              /* AC继电器故障 */
//    APP_SYS_FAULT_ELOCK = thaisenElock,                   /* 电子锁故障 */
//    APP_SYS_FAULT_AUXPOWER = thaisenAuxPower,             /* 辅源故障 */
//    APP_SYS_FAULT_FLASH = thaisenFaultFlash,             /* flash故障 */
//    APP_SYS_FAULT_EEPROM = thaisenFaultEeprom,           /* eeprom故障 */
//    APP_SYS_FAULT_NO_ERROR = thaisenFaultSize,
//
//    APP_USER_INFO_DEVICE_ERROR_PILE_NUMBER_LEN,          /* 桩号实际长度与记录长度不符 */
//    APP_USER_INFO_DEVICE_ERROR_PILE_NUMBER_NONE,         /* 桩号未配置 */
//    APP_USER_INFO_DEVICE_ERROR_SIZE,
//};


// 充电故障
//enum charge_fault_t
//{
//    APP_CHARGE_FAULT_GUN_VOLT = thaisenGunVolt,
//    APP_CHARGE_FAULT_INSULTA = thaisenInsult,
//    APP_CHARGE_FAULT_COMMON = thaisenCommon,
//    APP_CHARGE_FAULT_BATTERY_VOLT = thaisenBatteryVolt,
//    APP_CHARGE_FAULT_READY_VOLT = thaisenReadyVolt,
//    APP_CHARGE_FAULT_INSULT_VOLT = thaisenInsultVolt,
//    APP_CHARGE_FAULT_NO_ERROR,
//};

/********************************************
 * 函数名      thaisen_app_get_fault_info
 * 功能          获取故障信息
 * 参数         gunno     枪号
 * 返回         故障信息(枪号不对时充电故障返回CHARGE_FAULT_NO_ERROR 系统故障返回SYS_FAULT_NO_ERROR )
 *******************************************/
struct fault_info *thaisen_app_get_fault_info(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        s_fault_info.charge_fault = APP_CHARGE_FAULT_NO_ERROR;
        s_fault_info.system_fault = APP_SYS_FAULT_NO_ERROR;
        return &s_fault_info;
    }

    s_fault_info.charge_fault = app_get_highest_priority_charge_fault(gunno);
    s_fault_info.system_fault = app_get_highest_priority_system_fault(gunno);

    return &s_fault_info;
}

/*
enum record_type{
    RECORD_OF_CHARGE,    // 充电记录
    RECORD_OF_FAULT,     // 故障记录

    RECORD_OF_SIZE,
};
*/
/************************************************
 * 函数名     get_current_region_index
 * 功能         获取故障记录或充电记录当前下标
 * 参数        type    记录类型(故障记录或充电记录)
 *       gunno   枪号
 * 返回        < 0：失败   其它：成功(返回值为下标)
 ************************************************/
int32_t thaisen_app_get_current_region_index(enum record_region region)
{
    return mw_storage_record_get_current_index((enum notfs_subregion)region);
}
/************************************************
 * 函数名     get_region_record_total_num
 * 功能         获取故障记录或充电记录总数(记录数量超过最大数据总数后将重头开始写)
 * 参数        type    记录类型(故障记录或充电记录)
 *       gunno   枪号
 * 返回        < 0：失败   其它：成功(返回值为记录总数)
 ************************************************/
int32_t thaisen_app_get_region_record_total_num(enum record_region region)
{
    return mw_storage_record_get_record_total_num((enum notfs_subregion)region);
}

/************************************************
 * 函数名     thaisen_app_clear_region_record_info
 * 功能         清除指定区域的记录信息
 * 参数        type    记录类型(故障记录或充电记录)
 *       gunno   枪号
 * 返回        < 0：失败   其它：成功
 ************************************************/
int32_t thaisen_app_clear_region_record_info(enum record_region region)
{
    return mw_storage_record_clear_record_info((enum notfs_subregion)region);
}

//单次充电数据
//typedef struct
//{
//    uint8_t charge_pile_id[20];           /* 充电桩ID */
//    uint8_t gunno;                        /* 枪口号 */
//
//    uint8_t physics_card_number[20];      /* 物理卡号 ascii */
//    uint8_t serial_number[40];            /* 流水号 */
//
//    struct time_format start_time;        /* 充电开始时间 */
//    struct time_format stop_time;         /* 充电结束时间 */
//    uint32_t charge_time;                 /* 充电时间, 单位s */
//
//    uint8_t charge_mode;                  /* 充电模式 (APP 离线卡 在线卡 VIN 并充模式) */
//    uint8_t charge_type;                  /* 充电方式 (0x01://按电量 0x02://按时间 0x03://按金额  0x04://充满为止) */
//
//    uint8_t boot_result;                  /* 启动结果 1 成功 2是 失败 */
//    uint32_t boot_fault_code;             /* 启动故障代码 */
//    uint32_t system_fault_code;           /* 系统故障代码 */
//    uint32_t charge_fault_code;           /* 充电故障代码 */
//
//    uint8_t car_vin[20];                  /* VIN码 ascii */
//    uint16_t start_soc;                   /* 起始SOC */
//    uint16_t stop_soc;                    /* 结束SOC */
//
//    uint32_t ammeter_start_elect;         /* 电表电量起始值 */
//    uint32_t ammeter_stop_elect;          /* 电表电量结束值 */
//
//    uint32_t sharp_unit;                  /* 尖单价 */
//    uint32_t sharp_elect;                 /* 尖电量 */
//    uint32_t sharp_amount;                /* 尖金额 */
//    uint32_t sharp_loss_elect;            /* 尖计损电量 */
//
//    uint32_t peak_unit;                   /* 峰单价 */
//    uint32_t peak_elect;                  /* 峰电量 */
//    uint32_t peak_amount;                 /* 峰金额 */
//    uint32_t peak_loss_elect;             /* 峰计损电量 */
//
//    uint32_t flat_unit;                   /* 平单价 */
//    uint32_t flat_elect;                  /* 平电量 */
//    uint32_t flat_amount;                 /* 平金额 */
//    uint32_t flat_loss_elect;             /* 平计损电量 */
//
//    uint32_t valley_unit;                 /* 谷单价 */
//    uint32_t valley_elect;                /* 谷电量 */
//    uint32_t valley_amount;               /* 谷金额 */
//    uint32_t valley_loss_elect;           /* 谷计损电量 */
//
//    uint32_t total_elect;                 /* 充电总电量 */
//    uint32_t total_loss_elect;            /* 总计损电量 */
//    uint32_t charge_fee;                  /* 充电费用 */
//    uint32_t service_fee;                 /* 服务费用 */
//    uint32_t total_fee;                   /* 总费用 */
//    uint32_t delay_fee;                   /* 延迟费用 */
//
//    uint32_t transaction_date;            /* 交易日期 */
//    uint32_t stop_reason;                 /* 停止原因 */
//    uint8_t stop_reason_type;             /* 停止原因类型 */
//    uint8_t order_state;                  /* 订单状态(平台确认:1, 未确认：2) */
//    uint8_t order_type;                   /* 订单类型：本地、在线 */
//
//}thaisen_default_bill_t;

/************************************************
 * 函数名     get_index_charge_record
 * 功能         获取指定下标的充电记录
 * 参数        bill_buff    用于存放读取到的数据
 *       index         指定下标
 *       gunno   枪号
 * 返回        < 0：失败   其它：成功
 ************************************************/
int32_t thaisen_app_get_index_charge_record(thaisen_transaction_t* bill_buff, uint8_t index, enum record_region region)
{
    return mw_storage_record_get_designate_index_record((uint8_t*)bill_buff, sizeof(thaisen_transaction_t), (enum notfs_subregion)region, index);
}

///////////////////////////////////////////////////////////////////////// 故障记录
/// /* 故障下标及故障码 */
//  下标           故障码              故障
//  0x01           0x0001              急停
//  0x02           0x0002              读卡器
//  0x03           0x0003              门禁
//  0x04           0x0004              电表
//  0x05           0x0005              充电模块
//  0x06           0x0006              过温
//  0x07           0x0007              过压
//  0x08           0x0008              欠压
//  0x08           0x0008              过流
//  0x0A           0x000A              flash
//  0x0B           0x000B              eeprom

//  0x40           0x0040              枪电压
//  0x41           0x0041              继电器
//  0x42           0x0042              电子锁
//  0x43           0x0043              辅源
//  0x44           0x0044              绝缘
//  0x45           0x0045              通信
//  0x46           0x0046              电池电压

//struct error_info
//{
//    uint32_t error_flag  : 1;     /* 标志：0：产生，1：恢复 */
//    uint32_t error_index : 7;     /* 故障下标 */
//    uint32_t error_code  : 16;    /* 故障码 */
//    uint32_t reserve     : 8;
//    struct time_format occur_time;   /* 故障发生时间 */
//    struct time_format resume_time;  /* 故障发生时间 */
//};
/************************************************
 * 函数名     get_index_fault_record
 * 功能         获取指定下标的故障记录
 * 参数        error_buff    用于存放读取到的数据
 *       index         指定下标
 *       gunno   枪号
 * 返回        < 0：失败   其它：成功
 ************************************************/
int32_t thaisen_app_get_index_fault_record(struct error_info *error_buff, uint8_t index, enum record_region region)
{
    return mw_storage_record_get_designate_index_record((uint8_t*)error_buff, sizeof(struct error_info), (enum notfs_subregion)region, index);
}




/*
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

    CONFIG_ITEM_SIZE
};

*/

/********************************************
 * 函数名      thaisen_app_storage_config_port
 * 功能          触发存储配置项数据
 * 参数         无
 * 返回         0        配置成功
 *    其它                配置失败
 *******************************************/
int32_t thaisen_app_storage_config_port(void)
{
    return sys_storage_config_item();
}

/********************************************
 * 函数名      thaisen_app_sync_config_item_port
 * 功能          同步配置项数据
 * 参数         name     配置项名
 *       data     配置数据
 *       len      配置数据长度
 * 返回         0        配置成功
 *       其它                配置失败
 *******************************************/
int32_t thaisen_app_sync_config_item_port(enum config_name name, void* data, uint32_t len)
{
    return sys_sync_config_item_content(name, data, len);
}

/********************************************
 * 函数名      read_config_item_port
 * 功能          读配置项
 * 参数         name     配置项名
 *       is_user_content     读取配置项数据是否为 user 部分(该部分目前用于存储配置项数据实际长度)
 * 返回         配置项数据第一字节
 *******************************************/
uint8_t* thaisen_app_read_config_item_port(enum config_name name, uint8_t is_user_content)
{
    return sys_read_config_item_content(name, is_user_content);
}

/********************************************
 * 函数名      send_data_to_hci_uart
 * 功能          响屏幕串口发送数据
 * 参数          data     数据
 *        len      数据长度
 * 返回          无
 *******************************************/
void thaisen_app_send_data_to_hci_uart(uint8_t* data, uint8_t len)
{
    hci_send_data(data, len);
}

/********************************************
 * 函数名      thaisen_app_get_current_timestamp
 * 功能          获取当前时间戳
 * 参数          无
 * 返回          当前时间戳
 *******************************************/
uint32_t thaisen_app_get_current_timestamp(void)
{
    uint32_t timesec;
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    struct tm tsystm,*systm = &tsystm;
    localtime_r(&tv.tv_sec,systm);

    timesec = systm->tm_hour*3600+systm->tm_min*60+systm->tm_sec;
    return timesec;
}

/********************************************
 * 函数名      thaisen_app_get_system_tick
 * 功能          获取系统 tick
 * 参数          无
 * 返回         tick
 *******************************************/
uint32_t thaisen_app_get_system_tick(void)
{
    uint32_t tick = rt_tick_get();
    return tick;
}

/********************************************
 * 函数名      set_screen_start_charge
 * 功能          设置屏幕启动充电指令
 * 参数          gunno     枪号
 * 返回          无
 *******************************************/
void thaisen_app_set_screen_start_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    rt_kprintf("screen_start_charge gunno(%d)\n", gunno);
    app_set_hci_event(gunno, HCI_EVENT_SCREEN_START);
}

/********************************************
 * 函数名      set_screen_stop_charge
 * 功能          设置屏幕停止充电指令
 * 参数          gunno     枪号
 * 返回          无
 *******************************************/
void thaisen_app_set_screen_stop_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    rt_kprintf("screen_stop_charge gunno(%d)\n", gunno);
    app_set_hci_event(gunno, HCI_EVENT_SCREEN_STOP);
}

/********************************************
 * 函数名      thaisen_app_get_vin_start_charge
 * 功能          获取VIN启动充电状态
 * 参数          gunno     枪号
 * 返回          无
 *******************************************/
uint8_t thaisen_app_get_vin_start_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0;
    }
    return app_get_hci_event(gunno, HCI_EVENT_VIN_START, 0x01);
}

/********************************************
 * 函数名      thaisen_app_clear_vin_start_charge
 * 功能          清除VIN启动充电指令
 * 参数          gunno     枪号
 * 返回          无
 *******************************************/
void thaisen_app_clear_vin_start_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    app_get_hci_event(gunno, HCI_EVENT_VIN_START, 0x01);
}

/********************************************
 * 函数名      thaisen_app_set_vin_start_charge
 * 功能          设置VIN启动充电状态
 * 参数          gunno     枪号
 * 返回          无
 *******************************************/
void thaisen_app_set_vin_start_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    app_set_hci_event(gunno, HCI_EVENT_VIN_START);
}

struct temperature* get_battery_temp_info(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return &s_temp_info;
    }
    struct thaisenBMS_Charger_struct* bms_data = mw_get_bms_data(gunno);

    s_temp_info.temp = bms_data->BSM.HigTemp;
    s_temp_info.number = bms_data->BSM.HigTempNum;

    return &s_temp_info;
}

ota_info* thaisen_app_get_ota_info(void)
{
    s_ota_info.start_flag = app_nsal_get_ota_start_flag();
    s_ota_info.state = app_nsal_get_ota_state();
    s_ota_info.result = app_nsal_get_ota_result();
    s_ota_info.progress = app_nsal_get_ota_progress();

    return &s_ota_info;
}

/********************************************
 * 函数名      thaisen_app_get_time_sync_flag
 * 功能          获取时间同步标志
 * 参数          无
 * 返回         1：时间已同步， 0：时间未同步
 *******************************************/
uint8_t thaisen_app_get_time_sync_flag(void)
{
    extern int8_t mw_get_time_sync_flag(uint8_t gunno);
    extern void mw_clear_time_sync_flag(uint8_t gunno);

    if(mw_get_time_sync_flag(APP_TIME_SYNC_FLAG_SCREEN)){
        mw_clear_time_sync_flag(APP_TIME_SYNC_FLAG_SCREEN);
        return 1;
    }

    return 0;
}

uint8_t thaisen_get_hci_page_pos(void)
{
    extern unsigned char SerialScreen_GetPagePos();
    return SerialScreen_GetPagePos();
}

/********************************************
 * 函数名      thaisen_app_get_signal_strength
 * 功能          获取信号强度
 * 参数          无
* 返回         信号强度
 *******************************************/
int thaisen_app_get_signal_strength(void)
{
    extern int get_at_device_appinfo_signal_strength(void);
    return get_at_device_appinfo_signal_strength();
}

/********************************************
 * 函数名      thaisen_app_get_sim_number
 * 功能          获取SIM卡卡号
 * 参数          无
* 返回         SIM卡卡号
 *******************************************/
char *thaisen_app_get_sim_number(void)
{
    extern char *get_at_device_appinfo_iccid(void);
    return get_at_device_appinfo_iccid();
}

/********************************************
 * 函数名      thaisen_get_screen_timesync_flag
 * 功能          获取屏幕时间同步标志
 * 参数          无
* 返回           1：同步   0：未同步
 *******************************************/
int8_t thaisen_get_screen_timesync_flag(void)
{
    return SerialScreen_Get_ScreenTimeSync_Flag();
}

/********************************************
 * 函数名      thaisen_get_screen_timesync_time
 * 功能         获取屏幕时间同步时间值
 * 参数          buf    缓存
 *        len    缓存长度
* 返回           无
 *******************************************/
void thaisen_get_screen_timesync_time(uint16_t* buf, uint8_t len)
{
    SerialScreen_ScreenGet_TimeSync(buf, len);
}

/********************************************
 * 函数名      thaisen_request_screen_time
 * 功能         请求屏幕时间
* 返回           无
 *******************************************/
void thaisen_request_screen_time(void)
{
    SerialScreen_ScreenRequest_Time();
}

/*********************************************
 * 函数名             thaisen_get_stopway_string
 * 功能                 根据停充码获取停充原因字符串
 * 参数                code  停充码
 * 返回                停充原因字符串
 ********************************************/
const char* thaisen_get_stopway_string(uint16_t code)
{
    extern const char* get_stopway_string(uint16_t code);
    return get_stopway_string(code);
}

/********************************************
 * 函数名      thaisen_get_cc1_voltage
 * 功能         获取CC1电压
* 返回           无
 *******************************************/
uint8_t thaisen_get_cc1_voltage(uint8_t gunno)
{
    return mw_get_cc1_value(mw_get_cc1(gunno));
}

/********************************************
 * 函数名      thaisen_get_gun_temp
 * 功能         获取枪头温度
* 返回           无
 *******************************************/
int32_t thaisen_get_gun_temp(uint8_t gunno)
{
    if(mw_get_temp_dcp(gunno) >= mw_get_temp_dcn(gunno)){
        return mw_get_temp_dcp(gunno);
    }
    return mw_get_temp_dcn(gunno);
}

/********************************************
 * 函数名      thaisen_get_ammeter_voltage
 * 功能         获取电表电压
* 返回           无
 *******************************************/
uint32_t thaisen_get_ammeter_voltage(uint8_t gunno)
{
    return mw_get_meter_ua(gunno);
}

/********************************************
 * 函数名      thaisen_get_ammeter_current
 * 功能         获取电表电流
* 返回           无
 *******************************************/
uint32_t thaisen_get_ammeter_current(uint8_t gunno)
{
    return mw_get_meter_ia(gunno);
}

/********************************************
 * 函数名      thaisen_get_power_percent
 * 功能         获取功率百分比
* 返回           功率百分比(0-100), 负值的话是没有配置，默认100
 *******************************************/
int16_t thaisen_get_power_percent(void)
{
    return sys_get_power_percent();
}

/********************************************
 * 函数名      thaisen_vin_whitelists_add
 * 功能         添加VIN码白名单
* 返回           >= 0:成功, 其它：失败
 *******************************************/
int32_t thaisen_vin_whitelists_add(uint8_t *data, uint8_t len)
{
    return sys_vin_whitelists_add(data, len);
}

/********************************************
 * 函数名      thaisen_get_charge_way
 * 功能         获取充电方式
* 返回           充电方式
 *******************************************/
enum charge_way thaisen_get_charge_way(void)
{
    return APP_CHARGE_WAY_PARACHARGE;
}

