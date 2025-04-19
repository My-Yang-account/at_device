/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     我的杨yang       the first version
 */

#include "app_support_func.h"
#include "string.h"
#include "math.h"
#include "app_ofsm.h"
#include "chargepile_config.h"
#include "app_data_info_interface.h"

#include "mw_time.h"
#include "mw_fault_check.h"

#define DBG_TAG "support"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define APP_CHINESE_FAULT_LEN_MAX                   0x20               /* 中文故障码数据最大长度 */
#define APP_CHINESE_STOPWAY_LEN_MAX                 0x20               /* 中文停充原因数据最大长度 */

#if 0
static const char* library_fault_str[] =
{
     "scram",
     "card reader",
     "door",
     "ammeter",
     "charge module",
     "over temp",
     "over voltage",
     "under voltage",
     "over current",
     "dc relay",
     "parallel relay",
     "ac relay",
     "electronic lock",
     "auxiliary power",
     "flash chip",
     "eeprom chip",
     "lighting pro",
     "gun site",
     "circuit breaker",
     "flooding",
     "smoke",
     "pour",
     "liquid cool",
     "fuse"
     "shorts",
     "gun voltage",
     "insulation",
     "bms commu",
     "battery voltage",
     "pull gun",
     "charge full",
     "passive stop",
     "bms stop",
     "ready voltage",
     "insult voltage",
     "unknow",
};

static const char* owner_fault_str[] =
{
    "app stop",
    "online card",
    "offline card",
    "screen stop",
    "no account",
    "reach elect",
    "reach time",
    "reach money",
    "authen fail",
    "power off",
    "curr abnormal",
    "unknow",
};

/*********************************************
 * 函数名             get_stopway_string
 * 功能                 根据停充码获取停充原因字符串
 * 参数                code  停充码
 * 返回                停充原因字符串
 ********************************************/
const char* get_stopway_string(uint16_t code)
{
#define STOPWAY_LIBRARY_MAX       APP_SYSTEM_STOP_WAY_NULL - 1
#define STOPWAY_OWNER_MAX         APP_SYSTEM_STOP_WAY_SOC_LIMIT
#define STOPWAY_OWNER_MIN         APP_SYSTEM_STOP_WAY_APP_STOP

#define STOPWAY_LIBRARY_NONE_FAULT_MAX        APP_SYSTEM_STOP_WAY_NULL - 1
#define STOPWAY_LIBRARY_NONE_FAULT_MIN        APP_SYSTEM_STOP_WAY_SHORTS

    if(code <= STOPWAY_LIBRARY_MAX){
        if((code >= STOPWAY_LIBRARY_NONE_FAULT_MIN) && (code <= STOPWAY_LIBRARY_NONE_FAULT_MAX)){
            return library_fault_str[code - STOPWAY_LIBRARY_NONE_FAULT_MIN];
        }
        return library_fault_str[code];
    }else if((code >= STOPWAY_OWNER_MIN) && (code <= STOPWAY_OWNER_MAX)){
        return owner_fault_str[code - STOPWAY_OWNER_MIN];
    }else{
        uint8_t unknow_index = (sizeof(owner_fault_str) /4 - 1);
        return owner_fault_str[unknow_index];
    }
}
#endif /* 0 */

APP_DEF_SRAM1 static const char* system_fault_str[APP_SYS_FAULT_NO_ERROR] =
{
#ifndef APP_DESIGNATE_REGION
     "scram",                     /** 系统故障码字符串 0：急停 */
     "card reader",               /** 系统故障码字符串 1：读卡器 */
     "door",                      /** 系统故障码字符串 2：门禁 */
     "ammeter",                   /** 系统故障码字符串 3：电表 */
     "charge module",             /** 系统故障码字符串 4：充电模块 */
     "over temp",                 /** 系统故障码字符串 5：过温 */
     "over voltage",              /** 系统故障码字符串 6：过压 */
     "under voltage",             /** 系统故障码字符串 7欠压 */
     "over current",              /** 系统故障码字符串 8：过流 */
     "dc relay",                  /** 系统故障码字符串 9：直流继电器 */
     "parallel relay",            /** 系统故障码字符串 10：并联继电器 */
     "ac relay",                  /** 系统故障码字符串 11：交流接触器 */
     "electronic lock",           /** 系统故障码字符串 12：电子锁 */
     "auxiliary power",           /** 系统故障码字符串 13：辅源 */
     "flash chip",                /** 系统故障码字符串 14：FLASH */
     "eeprom chip",               /** 系统故障码字符串 15：EEPROM */
     "lighting pro",              /** 系统故障码字符串 16：防雷器 */
     "gun site",                  /** 系统故障码字符串 17：枪座 */
     "circuit breaker",           /** 系统故障码字符串 18：断路器 */
     "flooding",                  /** 系统故障码字符串 19：水浸 */
     "smoke",                     /** 系统故障码字符串 20：烟感 */
     "pour",                      /** 系统故障码字符串 21：倾倒 */
     "liquid cool",               /** 系统故障码字符串 22：液冷 */
     "fuse",                      /** 系统故障码字符串 23：熔断器 */
     "main cabinet",              /** 系统故障码字符串 24：主机柜 */
#endif /* APP_DESIGNATE_REGION */
};

APP_DEF_SRAM1 static const char* charge_fault_str[APP_CHARGE_FAULT_NO_ERROR] =
{
#ifndef APP_DESIGNATE_REGION
    "gun voltage",                /** 充电故障码字符串 0：枪头电压 */
    "IMD",                        /** 充电故障码字符串 1：绝缘 */
    "bms commu",                  /** 充电故障码字符串 2：BMS通讯 */
    "battery voltage",            /** 充电故障码字符串 3：电池电压 */
    "ready voltage",              /** 充电故障码字符串 4：准备电压 */
    "IMD voltage",                /** 充电故障码字符串 5：绝缘电压 */
    "YT BFC",                     /** 充电故障码字符串 6：宇通BFC */
#endif /* APP_DESIGNATE_REGION */
};

#ifdef APP_DESIGNATE_REGION
/*************************************
 * 函数名       app_support_func_info_init
 * 功能           辅助函数信息、变量初始化
 * 参数
 * 返回
 ************************************/
void app_support_func_info_init(void)
{
    system_fault_str[0] = "scram";
    system_fault_str[1] = "card reader";
    system_fault_str[2] = "door";
    system_fault_str[3] = "ammeter";
    system_fault_str[4] = "charge module";
    system_fault_str[5] = "over temp";
    system_fault_str[6] = "over voltage";
    system_fault_str[7] = "under voltage";
    system_fault_str[8] = "over current";
    system_fault_str[9] = "dc relay";
    system_fault_str[10] = "parallel relay";
    system_fault_str[11] = "ac relay";
    system_fault_str[12] = "electronic lock";
    system_fault_str[13] = "auxiliary power";
    system_fault_str[14] = "flash chip";
    system_fault_str[15] = "eeprom chip";
    system_fault_str[16] = "lighting pro";
    system_fault_str[17] = "gun site";
    system_fault_str[18] = "circuit breaker";
    system_fault_str[19] = "flooding";
    system_fault_str[20] = "smoke";
    system_fault_str[21] = "pour";
    system_fault_str[22] = "liquid cool";
    system_fault_str[23] = "fuse";
    system_fault_str[24] = "main cabinet";

    charge_fault_str[0] = "gun voltage";
    charge_fault_str[1] = "IMD";
    charge_fault_str[2] = "bms commu";
    charge_fault_str[3] = "battery voltage";
    charge_fault_str[4] = "ready voltage";
    charge_fault_str[5] = "IMD voltage";
    charge_fault_str[6] = "YT BFC";
}
#endif /* APP_DESIGNATE_REGION */

/*********************************************
 * 函数名             get_fault_string
 * 功能                 根据故障码获取故障字符串
 * 参数                code  故障码
 * 返回                故障字符串
 ********************************************/
const char* get_fault_string(uint16_t code)
{
    if(code < APP_ORIGIN_SYSFAULT_MAX){
        return system_fault_str[code];
    }

    if((code >= (APP_ORIGIN_SYSFAULT_MAX + APP_SYSFAULT_OFFSET_MIN)) &&
            (code < (APP_ORIGIN_SYSFAULT_MAX + APP_SYSFAULT_OFFSET_MAX))){
        uint16_t pos = (code - APP_SYSFAULT_OFFSET_MIN);
        if(pos >= APP_SYS_FAULT_NO_ERROR){
            return "unknow";
        }else{
            return system_fault_str[pos];
        }
    }

    if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_GUNVOLT)){
        return charge_fault_str[APP_CHARGE_FAULT_GUN_VOLT];
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_INSULT)){
        return charge_fault_str[APP_CHARGE_FAULT_INSULTA];
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_COMMINICATION)){
        return charge_fault_str[APP_CHARGE_FAULT_COMMON];
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_BATTERY_VOLT)){
        return charge_fault_str[APP_CHARGE_FAULT_BATTERY_VOLT];
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_READY_VOLT)){
        return charge_fault_str[APP_CHARGE_FAULT_READY_VOLT];
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_INSULT_VOLT)){
        return charge_fault_str[APP_CHARGE_FAULT_INSULT_VOLT];
    }else if(code == mw_system_stop_way_convert(thaisen_chargeCtl_stopWay_BFC)){
        return charge_fault_str[APP_CHARGE_FAULT_YT_BFC];
    }

    else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_BRM_TIMEOUT)){
        return "BRM timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_BCP_TIMEOUT)){
        return "BCP timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_BRO_TIMEOUT)){
        return "BRO timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT)){
        return "BROAA timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT)){
        return "S-BCL timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT)){
        return "S-BCS timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT)){
        return "C-BCL timeout";
    }else if(code == mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT)){
        return "C-BCS timeout";
    }

    return "unknow";
}


/********************************************
 * 函数名      app_get_fault_chinese
 * 功能          获取中文故障信息
 * 参数          code      故障码
 *        olen      用于保存中文故障信息实际长度
 *        buf       用于保存中文故障信息
 *        ilen      buf  的长度
 * 返回
 *******************************************/
void app_get_fault_chinese(uint32_t code, uint8_t *olen, uint8_t *buf, uint8_t ilen)
{
    memset(buf, 0x00, ilen);

    if(ilen < APP_CHINESE_FAULT_LEN_MAX){
        memcpy(buf, "未知", strlen("未知"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    }

    switch(code){
    case APP_SYS_FAULT_SCRAM:
        memcpy(buf, "急停", strlen("急停"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_CARD_READER:
        memcpy(buf, "读卡器", strlen("读卡器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_DOOR:
        memcpy(buf, "门禁", strlen("门禁"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_AMMETER:
        memcpy(buf, "电表", strlen("电表"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_CHARGE_MODULE:
        memcpy(buf, "充电模块", strlen("充电模块"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_OVER_TEMP:
        memcpy(buf, "枪头过温", strlen("枪头过温"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_OVER_VOLT:
        memcpy(buf, "过压", strlen("过压"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_UNDER_VOLT:
        memcpy(buf, "欠压", strlen("欠压"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_OVER_CURR:
        memcpy(buf, "过流", strlen("过流"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_RELAY:
        memcpy(buf, "直流继电器", strlen("直流继电器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_PARALLEL_RELAY:
        memcpy(buf, "母联继电器", strlen("母联继电器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_AC_RELAY:
        memcpy(buf, "交流接触器", strlen("交流接触器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_ELOCK:
        memcpy(buf, "电子锁", strlen("电子锁"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_AUXPOWER:
        memcpy(buf, "辅助电源", strlen("辅助电源"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_FLASH:
    case APP_SYS_FAULT_EEPROM:
        memcpy(buf, "存储芯片", strlen("存储芯片"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_LIGHT_PRPTECT:
        memcpy(buf, "防雷器", strlen("防雷器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_GUN_SITE:
        memcpy(buf, "枪座", strlen("枪座"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_CIRCUIT_BREAKER:
        memcpy(buf, "断路器", strlen("断路器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_FLOODING:
        memcpy(buf, "水浸", strlen("水浸"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_SMOKE:
        memcpy(buf, "烟感", strlen("烟感"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_POUR:
        memcpy(buf, "倾倒", strlen("倾倒"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_LIQUID_COOLING:
        memcpy(buf, "液冷", strlen("液冷"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_FUSE:
        memcpy(buf, "熔断器", strlen("熔断器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYS_FAULT_MAIN_CABINET:
        memcpy(buf, "主机柜", strlen("主机柜"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    default:
        memcpy(buf, "未知", strlen("未知"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    }
}

/********************************************
 * 函数名      app_get_charge_stopway_chinese
 * 功能          获取中文停充原因
 * 参数          code      故障码
 *        olen      用于保存中文停充原因信息实际长度
 *        buf       用于保存中文停充原因信息
 *        ilen      buf  的长度
 * 返回
 *******************************************/
void app_get_charge_stopway_chinese(uint32_t code, uint8_t *olen, uint8_t *buf, uint8_t ilen)
{
    uint8_t is_matched = 0x00;
    int16_t i = 0x00;
    memset(buf, 0x00, ilen);

    if(ilen < APP_CHINESE_STOPWAY_LEN_MAX){
        memcpy(buf, "未知", strlen("未知"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    }

    for(i = thaisen_chargeCtl_stopWay_BSM; i >= thaisen_chargeCtl_stopWay_short; i--){
        if(code == mw_system_stop_way_convert(i)){
            is_matched = 0x01;
            break;
        }
    }
    if((i < thaisen_chargeCtl_stopWay_short) && (is_matched == 0x00)){
        for(i = APP_SYSTEM_STOP_WAY_APP_STOP; i <= APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT; i++){
            if(code == mw_system_stop_way_convert(i)){
                is_matched = 0x01;
                break;
            }
        }
    }

    if(is_matched){
        switch(i){
        case APP_SYSTEM_STOP_WAY_SHORTS:
            memcpy(buf, "短路", strlen("短路"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_GUNVOLT:
            memcpy(buf, "枪头电压", strlen("枪头电压"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_INSULT:
            memcpy(buf, "绝缘故障", strlen("绝缘故障"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_COMMINICATION:
            memcpy(buf, "BMS 通讯", strlen("BMS 通讯"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BATTERY_VOLT:
            memcpy(buf, "电池电压", strlen("电池电压"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_PULL_GUN:
            memcpy(buf, "充电连接器", strlen("充电连接器"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
            memcpy(buf, "充满", strlen("充满"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_PASSIVE:
            memcpy(buf, "主动停止", strlen("主动停止"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BST:
            memcpy(buf, "车端停止", strlen("车端停止"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_READY_VOLT:
            memcpy(buf, "准备电压", strlen("准备电压"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_INSULT_VOLT:
            memcpy(buf, "绝缘电压", strlen("绝缘电压"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BSM:
            memcpy(buf, "车端故障", strlen("车端故障"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_APP_STOP:
            memcpy(buf, "服务器", strlen("服务器"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
            memcpy(buf, "在线卡", strlen("在线卡"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP:
            memcpy(buf, "离线卡", strlen("离线卡"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_SCREEN_STOP:
            memcpy(buf, "屏幕", strlen("屏幕"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
            memcpy(buf, "余额不足", strlen("余额不足"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_REACH_ELECT:
            memcpy(buf, "到达设定电量", strlen("到达设定电量"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_REACH_TIME:
            memcpy(buf, "到达设定时间", strlen("到达设定时间"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_REACH_MONEY:
            memcpy(buf, "到达设定余额", strlen("到达设定余额"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_AUTHEN_FAIL:
            memcpy(buf, "VIN 鉴权失败", strlen("VIN 鉴权失败"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_POWER_OFF:
            memcpy(buf, "断电", strlen("断电"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
            memcpy(buf, "电流异常", strlen("电流异常"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
            memcpy(buf, "达到指定SOC", strlen("达到指定SOC"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID:
            memcpy(buf, "主机柜禁止充电", strlen("主机柜禁止充电"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BRM_TIMEOUT:
            memcpy(buf, "接收BRM超时", strlen("接收BRM超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BCP_TIMEOUT:
            memcpy(buf, "接收BCP超时", strlen("接收BCP超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BRO_TIMEOUT:
            memcpy(buf, "接收BRO超时", strlen("接收BRO超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT:
            memcpy(buf, "接收BRO_AA超时", strlen("接收BRO_AA超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT:
            memcpy(buf, "启动-接收BCL超时", strlen("启动-接收BCL超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT:
            memcpy(buf, "启动-接收BCS超时", strlen("启动-接收BCS超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT:
            memcpy(buf, "充电-接收BCL超时", strlen("充电-接收BCL超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        case APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT:
            memcpy(buf, "充电-接收BCS超时", strlen("充电-接收BCS超时"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        default:
            memcpy(buf, "未知", strlen("未知"));
            if(olen)
                *olen = strlen((char*)buf);
            return;
        }
    }

    switch(code){
    case APP_SYSTEM_STOP_WAY_SCRAM:
        memcpy(buf, "急停", strlen("急停"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_CARDREADER:
        memcpy(buf, "读卡器", strlen("读卡器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_DOOR:
        memcpy(buf, "门禁", strlen("门禁"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_AMMETER:
        memcpy(buf, "电表", strlen("电表"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_CHARGEMODULE:
        memcpy(buf, "充电模块", strlen("充电模块"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_OVERTEMP:
        memcpy(buf, "枪头过温", strlen("枪头过温"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
        memcpy(buf, "过压", strlen("过压"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        memcpy(buf, "欠压", strlen("欠压"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        memcpy(buf, "过流", strlen("过流"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_RELAY:
        memcpy(buf, "直流继电器", strlen("直流继电器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_PARALLEL_RELAY:
        memcpy(buf, "母联继电器", strlen("母联继电器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_AC_RELAY:
        memcpy(buf, "交流接触器", strlen("交流接触器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_ELECTRY_LOCK:
        memcpy(buf, "电子锁", strlen("电子锁"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_AUXPOWER:
        memcpy(buf, "辅助电源", strlen("辅助电源"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_FLASH:
    case APP_SYSTEM_STOP_WAY_EEPROM:
        memcpy(buf, "存储芯片", strlen("存储芯片"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_LIGHTPROTECT:
        memcpy(buf, "防雷器", strlen("防雷器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_GUNSITE:
        memcpy(buf, "枪座", strlen("枪座"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_CIRCUIT_BREAKER:
        memcpy(buf, "断路器", strlen("断路器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_FLOODING:
        memcpy(buf, "水浸", strlen("水浸"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_SMOKE:
        memcpy(buf, "烟感", strlen("烟感"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_POUR:
        memcpy(buf, "倾倒", strlen("倾倒"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_LIQUIDCOOLING:
        memcpy(buf, "液冷", strlen("液冷"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_FUSE:
        memcpy(buf, "熔断器", strlen("熔断器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAIN_CABINET:
        memcpy(buf, "主机柜", strlen("主机柜"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_YT_BFC:
        memcpy(buf, "宇通协议BFC故障", strlen("宇通协议BFC故障"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    default:
        memcpy(buf, "未知", strlen("未知"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    }
}


/********************************************
 * 函数名      app_selfcheck_debug_info
 * 功能          一键自检中文信息
 * 参数          item      自检项
 *      language  语言
 *      ret       自检结果(1：成功   0：失败)
 *      buf       用于保存中文信息
 *      ilen      buf  的长度
 * 返回
 *******************************************/
void app_selfcheck_debug_info(uint8_t item, uint8_t language, uint8_t ret, uint8_t *buf, uint8_t ilen)
{
    memset(buf, 0x00, ilen);

    switch(item){
    case THA_DEBUG_ITEM_ACRELAY_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("AcRelayClose:OK"))
                    return;
                memcpy(buf, "AcRelayClose:OK", strlen("AcRelayClose:OK"));
            }else{
                if(ilen <= strlen("交流接触器闭合：正常"))
                    return;
                memcpy(buf, "交流接触器闭合：正常", strlen("交流接触器闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("AcRelayClose:Fail"))
                    return;
                memcpy(buf, "AcRelayClose:Fail", strlen("AcRelayClose:Fail"));
            }else{
                if(ilen <= strlen("交流接触器闭合：异常"))
                    return;
                memcpy(buf, "交流接触器闭合：异常", strlen("交流接触器闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_ACRELAY_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("AcRelayBreak:OK"))
                    return;
                memcpy(buf, "AcRelayBreak:OK", strlen("AcRelayBreak:OK"));
            }else{
                if(ilen <= strlen("交流接触器断开：正常"))
                    return;
                memcpy(buf, "交流接触器断开：正常", strlen("交流接触器断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("AcRelayBreak:Fail"))
                    return;
                memcpy(buf, "AcRelayBreak:Fail", strlen("AcRelayBreak:Fail"));
            }else{
                if(ilen <= strlen("交流接触器断开：异常"))
                    return;
                memcpy(buf, "交流接触器断开：异常", strlen("交流接触器断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_1_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para1RelayClose:OK"))
                    return;
                memcpy(buf, "Para1RelayClose:OK", strlen("Para1RelayClose:OK"));
            }else{
                if(ilen <= strlen("母联1闭合：正常"))
                    return;
                memcpy(buf, "母联1闭合：正常", strlen("母联1闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para1RelayClose:Fail"))
                    return;
                memcpy(buf, "Para1RelayClose:Fail", strlen("Para1RelayClose:Fail"));
            }else{
                if(ilen <= strlen("母联1闭合：异常"))
                    return;
                memcpy(buf, "母联1闭合：异常", strlen("母联1闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_1_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para1RelayBreak:OK"))
                    return;
                memcpy(buf, "Para1RelayBreak:OK", strlen("Para1RelayBreak:OK"));
            }else{
                if(ilen <= strlen("母联1断开：正常"))
                    return;
                memcpy(buf, "母联1断开：正常", strlen("母联1断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para1RelayBreak:Fail"))
                    return;
                memcpy(buf, "Para1RelayBreak:Fail", strlen("Para1RelayBreak:Fail"));
            }else{
                if(ilen <= strlen("母联1断开：异常"))
                    return;
                memcpy(buf, "母联1断开：异常", strlen("母联1断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_2_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para2RelayClose:OK"))
                    return;
                memcpy(buf, "Para2RelayClose:OK", strlen("Para2RelayClose:OK"));
            }else{
                if(ilen <= strlen("母联2闭合：正常"))
                    return;
                memcpy(buf, "母联2闭合：正常", strlen("母联2闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para2RelayClose:Fail"))
                    return;
                memcpy(buf, "Para2RelayClose:Fail", strlen("Para2RelayClose:Fail"));
            }else{
                if(ilen <= strlen("母联2闭合：异常"))
                    return;
                memcpy(buf, "母联2闭合：异常", strlen("母联2闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_2_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para2RelayBreak:OK"))
                    return;
                memcpy(buf, "Para2RelayBreak:OK", strlen("Para2RelayBreak:OK"));
            }else{
                if(ilen <= strlen("母联2断开：正常"))
                    return;
                memcpy(buf, "母联2断开：正常", strlen("母联2断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para2RelayBreak:Fail"))
                    return;
                memcpy(buf, "Para2RelayBreak:Fail", strlen("Para2RelayBreak:Fail"));
            }else{
                if(ilen <= strlen("母联2断开：异常"))
                    return;
                memcpy(buf, "母联2断开：异常", strlen("母联2断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_3_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para3RelayClose:OK"))
                    return;
                memcpy(buf, "Para3RelayClose:OK", strlen("Para3RelayClose:OK"));
            }else{
                if(ilen <= strlen("母联3闭合：正常"))
                    return;
                memcpy(buf, "母联3闭合：正常", strlen("母联3闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para3RelayClose:Fail"))
                    return;
                memcpy(buf, "Para3RelayClose:Fail", strlen("Para3RelayClose:Fail"));
            }else{
                if(ilen <= strlen("母联3闭合：异常"))
                    return;
                memcpy(buf, "母联3闭合：异常", strlen("母联3闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_3_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para3RelayBreak:OK"))
                    return;
                memcpy(buf, "Para3RelayBreak:OK", strlen("Para3RelayBreak:OK"));
            }else{
                if(ilen <= strlen("母联3断开：正常"))
                    return;
                memcpy(buf, "母联3断开：正常", strlen("母联3断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Para3RelayBreak:Fail"))
                    return;
                memcpy(buf, "Para3RelayBreak:Fail", strlen("Para3RelayBreak:Fail"));
            }else{
                if(ilen <= strlen("母联3断开：异常"))
                    return;
                memcpy(buf, "母联3断开：异常", strlen("母联3断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_FAN_ON_A:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanAOpen:OK"))
                    return;
                memcpy(buf, "FanAOpen:OK", strlen("FanAOpen:OK"));
            }else{
                if(ilen <= strlen("A枪风扇开启：正常"))
                    return;
                memcpy(buf, "A枪风扇开启：正常", strlen("A枪风扇开启：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanAOpen:Fail"))
                    return;
                memcpy(buf, "FanAOpen:Fail", strlen("FanAOpen:Fail"));
            }else{
                if(ilen <= strlen("A枪风扇开启：异常"))
                    return;
                memcpy(buf, "A枪风扇开启：异常", strlen("A枪风扇开启：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_FAN_OFF_A:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanAClose:OK"))
                    return;
                memcpy(buf, "FanAClose:OK", strlen("FanAClose:OK"));
            }else{
                if(ilen <= strlen("A枪风扇关闭：正常"))
                    return;
                memcpy(buf, "A枪风扇关闭：正常", strlen("A枪风扇关闭：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanAClose:Fail"))
                    return;
                memcpy(buf, "FanAClose:Fail", strlen("FanAClose:Fail"));
            }else{
                if(ilen <= strlen("A枪风扇关闭：异常"))
                    return;
                memcpy(buf, "A枪风扇关闭：异常", strlen("A枪风扇关闭：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_FAN_ON_B:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanBOpen:OK"))
                    return;
                memcpy(buf, "FanBOpen:OK", strlen("FanBOpen:OK"));
            }else{
                if(ilen <= strlen("B枪风扇开启：正常"))
                    return;
                memcpy(buf, "B枪风扇开启：正常", strlen("B枪风扇开启：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanBOpen:Fail"))
                    return;
                memcpy(buf, "FanBOpen:Fail", strlen("FanBOpen:Fail"));
            }else{
                if(ilen <= strlen("B枪风扇开启：异常"))
                    return;
                memcpy(buf, "B枪风扇开启：异常", strlen("B枪风扇开启：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_FAN_OFF_B:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanBClose:OK"))
                    return;
                memcpy(buf, "FanBClose:OK", strlen("FanBClose:OK"));
            }else{
                if(ilen <= strlen("B枪风扇关闭：正常"))
                    return;
                memcpy(buf, "B枪风扇关闭：正常", strlen("B枪风扇关闭：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("FanBClose:Fail"))
                    return;
                memcpy(buf, "FanBClose:Fail", strlen("FanBClose:Fail"));
            }else{
                if(ilen <= strlen("B枪风扇关闭：异常"))
                    return;
                memcpy(buf, "B枪风扇关闭：异常", strlen("B枪风扇关闭：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_A_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayAClose:OK"))
                    return;
                memcpy(buf, "DcRelayAClose:OK", strlen("DcRelayAClose:OK"));
            }else{
                if(ilen <= strlen("A枪直流继电器闭合：正常"))
                    return;
                memcpy(buf, "A枪直流继电器闭合：正常", strlen("A枪直流继电器闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayAClose:Fail"))
                    return;
                memcpy(buf, "DcRelayAClose:Fail", strlen("DcRelayAClose:Fail"));
            }else{
                if(ilen <= strlen("A枪直流继电器闭合：异常"))
                    return;
                memcpy(buf, "A枪直流继电器闭合：异常", strlen("A枪直流继电器闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_A_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayABreak:OK"))
                    return;
                memcpy(buf, "DcRelayABreak:OK", strlen("DcRelayABreak:OK"));
            }else{
                if(ilen <= strlen("A枪直流继电器断开：正常"))
                    return;
                memcpy(buf, "A枪直流继电器断开：正常", strlen("A枪直流继电器断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayABreak:Fail"))
                    return;
                memcpy(buf, "DcRelayABreak:Fail", strlen("DcRelayABreak:Fail"));
            }else{
                if(ilen <= strlen("A枪直流继电器断开：异常"))
                    return;
                memcpy(buf, "A枪直流继电器断开：异常", strlen("A枪直流继电器断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_B_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayBClose:OK"))
                    return;
                memcpy(buf, "DcRelayBClose:OK", strlen("DcRelayBClose:OK"));
            }else{
                if(ilen <= strlen("B枪直流继电器闭合：正常"))
                    return;
                memcpy(buf, "B枪直流继电器闭合：正常", strlen("B枪直流继电器闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayBClose:Fail"))
                    return;
                memcpy(buf, "DcRelayBClose:Fail", strlen("DcRelayBClose:Fail"));
            }else{
                if(ilen <= strlen("B枪直流继电器闭合：异常"))
                    return;
                memcpy(buf, "B枪直流继电器闭合：异常", strlen("B枪直流继电器闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_B_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayBBreak:OK"))
                    return;
                memcpy(buf, "DcRelayBBreak:OK", strlen("DcRelayBBreak:OK"));
            }else{
                if(ilen <= strlen("B枪直流继电器断开：正常"))
                    return;
                memcpy(buf, "B枪直流继电器断开：正常", strlen("B枪直流继电器断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("DcRelayBBreak:Fail"))
                    return;
                memcpy(buf, "DcRelayBBreak:Fail", strlen("DcRelayBBreak:Fail"));
            }else{
                if(ilen <= strlen("B枪直流继电器断开：异常"))
                    return;
                memcpy(buf, "B枪直流继电器断开：异常", strlen("B枪直流继电器断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_A_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockA Lock:OK"))
                    return;
                memcpy(buf, "ELockA Lock:OK", strlen("ELockA Lock:OK"));
            }else{
                if(ilen <= strlen("A枪电子锁上锁：正常"))
                    return;
                memcpy(buf, "A枪电子锁上锁：正常", strlen("A枪电子锁上锁：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockA Lock:Fail"))
                    return;
                memcpy(buf, "ELockA Lock:Fail", strlen("ELockA Lock:Fail"));
            }else{
                if(ilen <= strlen("A枪电子锁上锁：异常"))
                    return;
                memcpy(buf, "A枪电子锁上锁：异常", strlen("A枪电子锁上锁：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_A_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockA UnLock:OK"))
                    return;
                memcpy(buf, "ELockA UnLock:OK", strlen("ELockA UnLock:OK"));
            }else{
                if(ilen <= strlen("A枪电子锁解锁：正常"))
                    return;
                memcpy(buf, "A枪电子锁解锁：正常", strlen("A枪电子锁解锁：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockA UnLock:Fail"))
                    return;
                memcpy(buf, "ELockA UnLock:Fail", strlen("ELockA UnLock:Fail"));
            }else{
                if(ilen <= strlen("A枪电子锁解锁：异常"))
                    return;
                memcpy(buf, "A枪电子锁解锁：异常", strlen("A枪电子锁解锁：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_B_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockB Lock:OK"))
                    return;
                memcpy(buf, "ELockB Lock:OK", strlen("ELockB Lock:OK"));
            }else{
                if(ilen <= strlen("B枪电子锁上锁：正常"))
                    return;
                memcpy(buf, "B枪电子锁上锁：正常", strlen("B枪电子锁上锁：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockB Lock:Fail"))
                    return;
                memcpy(buf, "ELockB Lock:Fail", strlen("ELockB Lock:Fail"));
            }else{
                if(ilen <= strlen("B枪电子锁上锁：异常"))
                    return;
                memcpy(buf, "B枪电子锁上锁：异常", strlen("B枪电子锁上锁：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_B_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockB UnLock:OK"))
                    return;
                memcpy(buf, "ELockB UnLock:OK", strlen("ELockB UnLock:OK"));
            }else{
                if(ilen <= strlen("B枪电子锁解锁：正常"))
                    return;
                memcpy(buf, "B枪电子锁解锁：正常", strlen("B枪电子锁解锁：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("ELockB UnLock:Fail"))
                    return;
                memcpy(buf, "ELockB UnLock:Fail", strlen("ELockB UnLock:Fail"));
            }else{
                if(ilen <= strlen("B枪电子锁解锁：异常"))
                    return;
                memcpy(buf, "B枪电子锁解锁：异常", strlen("B枪电子锁解锁：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_A_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VCloseA:OK"))
                    return;
                memcpy(buf, "Aux12VCloseA:OK", strlen("Aux12VCloseA:OK"));
            }else{
                if(ilen <= strlen("A枪12V辅源闭合：正常"))
                    return;
                memcpy(buf, "A枪12V辅源闭合：正常", strlen("A枪12V辅源闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VCloseA:Fail"))
                    return;
                memcpy(buf, "Aux12VCloseA:Fail", strlen("Aux12VCloseA:Fail"));
            }else{
                if(ilen <= strlen("A枪12V辅源闭合：异常"))
                    return;
                memcpy(buf, "A枪12V辅源闭合：异常", strlen("A枪12V辅源闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_A_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VBreakA:OK"))
                    return;
                memcpy(buf, "Aux12VBreakA:OK", strlen("Aux12VBreakA:OK"));
            }else{
                if(ilen <= strlen("A枪12V辅源断开：正常"))
                    return;
                memcpy(buf, "A枪12V辅源断开：正常", strlen("A枪12V辅源断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VBreakA:Fail"))
                    return;
                memcpy(buf, "Aux12VBreakA:Fail", strlen("Aux12VBreakA:Fail"));
            }else{
                if(ilen <= strlen("A枪12V辅源断开：异常"))
                    return;
                memcpy(buf, "A枪12V辅源断开：异常", strlen("A枪12V辅源断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_B_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VCloseB:OK"))
                    return;
                memcpy(buf, "Aux12VCloseB:OK", strlen("Aux12VCloseB:OK"));
            }else{
                if(ilen <= strlen("B枪12V辅源闭合：正常"))
                    return;
                memcpy(buf, "B枪12V辅源闭合：正常", strlen("B枪12V辅源闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VCloseB:Fail"))
                    return;
                memcpy(buf, "Aux12VCloseB:Fail", strlen("Aux12VCloseB:Fail"));
            }else{
                if(ilen <= strlen("B枪12V辅源闭合：异常"))
                    return;
                memcpy(buf, "B枪12V辅源闭合：异常", strlen("B枪12V辅源闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_B_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VBreakB:OK"))
                    return;
                memcpy(buf, "Aux12VBreakB:OK", strlen("Aux12VBreakB:OK"));
            }else{
                if(ilen <= strlen("B枪12V辅源断开：正常"))
                    return;
                memcpy(buf, "B枪12V辅源断开：正常", strlen("B枪12V辅源断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux12VBreakB:Fail"))
                    return;
                memcpy(buf, "Aux12VBreakB:Fail", strlen("Aux12VBreakB:Fail"));
            }else{
                if(ilen <= strlen("B枪12V辅源断开：异常"))
                    return;
                memcpy(buf, "B枪12V辅源断开：异常", strlen("B枪12V辅源断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_A_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VCloseA:OK"))
                    return;
                memcpy(buf, "Aux24VCloseA:OK", strlen("Aux24VCloseA:OK"));
            }else{
                if(ilen <= strlen("A枪24V辅源闭合：正常"))
                    return;
                memcpy(buf, "A枪24V辅源闭合：正常", strlen("A枪24V辅源闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VCloseA:Fail"))
                    return;
                memcpy(buf, "Aux24VCloseA:Fail", strlen("Aux24VCloseA:Fail"));
            }else{
                if(ilen <= strlen("A枪24V辅源闭合：异常"))
                    return;
                memcpy(buf, "A枪24V辅源闭合：异常", strlen("A枪24V辅源闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_A_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VBreakA:OK"))
                    return;
                memcpy(buf, "Aux24VBreakA:OK", strlen("Aux24VBreakA:OK"));
            }else{
                if(ilen <= strlen("A枪24V辅源断开：正常"))
                    return;
                memcpy(buf, "A枪24V辅源断开：正常", strlen("A枪24V辅源断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VBreakA:Fail"))
                    return;
                memcpy(buf, "Aux24VBreakA:Fail", strlen("Aux24VBreakA:Fail"));
            }else{
                if(ilen <= strlen("A枪24V辅源断开：异常"))
                    return;
                memcpy(buf, "A枪24V辅源断开：异常", strlen("A枪24V辅源断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_B_ON:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VCloseB:OK"))
                    return;
                memcpy(buf, "Aux24VCloseB:OK", strlen("Aux24VCloseB:OK"));
            }else{
                if(ilen <= strlen("B枪24V辅源闭合：正常"))
                    return;
                memcpy(buf, "B枪24V辅源闭合：正常", strlen("B枪24V辅源闭合：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VCloseB:Fail"))
                    return;
                memcpy(buf, "Aux24VCloseB:Fail", strlen("Aux24VCloseB:Fail"));
            }else{
                if(ilen <= strlen("B枪24V辅源闭合：异常"))
                    return;
                memcpy(buf, "B枪24V辅源闭合：异常", strlen("B枪24V辅源闭合：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_B_OFF:
        if(ret){
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VBreakB:OK"))
                    return;
                memcpy(buf, "Aux24VBreakB:OK", strlen("Aux24VBreakB:OK"));
            }else{
                if(ilen <= strlen("B枪24V辅源断开：正常"))
                    return;
                memcpy(buf, "B枪24V辅源断开：正常", strlen("B枪24V辅源断开：正常"));
            }
        }else{
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= strlen("Aux24VBreakB:Fail"))
                    return;
                memcpy(buf, "Aux24VBreakB:Fail", strlen("Aux24VBreakB:Fail"));
            }else{
                if(ilen <= strlen("B枪24V辅源断开：异常"))
                    return;
                memcpy(buf, "B枪24V辅源断开：异常", strlen("B枪24V辅源断开：异常"));
            }
        }
        break;
    case THA_DEBUG_ITEM_COMPLETE:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= strlen("SelfCheck Complete"))
                return;
            memcpy(buf, "SelfCheck Complete", strlen("SelfCheck Complete"));
        }else{
            if(ilen <= strlen("自检完成"))
                return;
            memcpy(buf, "自检完成", strlen("自检完成"));
        }
        break;
    }
}

/********************************************
 * 函数名      app_get_mode_info
 * 功能          获取模式显示信息
 * 参数          gunno     枪号
 *      language  语言
 *      mode      模式
 *      parameter 模式参数
 *      buf       用于保存显示信息
 *      ilen      缓存长度
 * 返回
 *******************************************/
void app_get_mode_info(uint8_t gunno, uint8_t language, uint8_t mode, uint32_t parameter, uint8_t *buf, uint8_t ilen)
{
    memset(buf, 0x00, ilen);

    switch(mode){
    case THAISEN_MODE_CHARGE_FULL:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= strlen("Mode:Charge Full"))
                return;
            memcpy(buf, "Mode:Charge Full", strlen("Mode:Charge Full"));
        }else{
            if(ilen <= strlen("模式:自动充满"))
                return;
            memcpy(buf, "模式:自动充满", strlen("模式:自动充满"));
        }
        break;
    case THAISEN_MODE_LIMIT_MONEY:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:LimitMoney  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x03 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%s", "Mode:LimitMoney  ", (parameter /100), ((parameter /10) %10), (parameter %10), "RMB");   /** 两位小数 */
        }else{
            if(ilen <= (strlen("模式:定额充电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x02 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%s", "模式:定额充电  ", (parameter /100), ((parameter /10) %10), (parameter %10), "元");   /** 两位小数 */
        }
        break;
    case THAISEN_MODE_LIMIT_ELECT:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:LimitElect  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x04 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%lu%s", "Mode:LimitElect ", (parameter /1000), ((parameter /100) %10), ((parameter /10) %10), (parameter %10), "KW*h");   /** 三位小数 */
        }else{
            if(ilen <= (strlen("模式:定量充电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x02 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%lu%s", "模式:定量充电  ", (parameter /1000), ((parameter /100) %10), ((parameter /10) %10), (parameter %10), "度");   /** 三位小数 */
        }
        break;
    case THAISEN_MODE_LIMIT_TIMING:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:Timing  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x03))
                return;
            sprintf((char*)buf, "%s%lu%s", "Mode:Timing ", parameter, "min");   /** 分钟 */
        }else{
            if(ilen <= (strlen("模式:定时充电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x04))
                return;
            sprintf((char*)buf, "%s%lu%s", "模式:定时充电  ", parameter, "分钟");   /** 分钟 */
        }
    }
        break;
    case THAISEN_MODE_LIMIT_RESERVATION:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:Reservation  ") + (uint32_t)(log10((double)(parameter /3600)) + 0x01) + 0x01 + (uint32_t)(log10((double)((parameter %3600) /60)) + 0x01)))
                return;
            sprintf((char*)buf, "%s%02lu%c%02lu", "Mode:Reservation ", (parameter /3600), ':', ((parameter %3600) /60));   /** 启动充电时间 */
        }else{
            if(ilen <= (strlen("模式:定时充电  ") + (uint32_t)(log10((double)(parameter /3600)) + 0x01) + 0x01 + (uint32_t)(log10((double)((parameter %3600) /60)) + 0x01)))
                return;
            sprintf((char*)buf, "%s%02lu%c%02lu", "模式:预约充电  ", (parameter /3600), ':', ((parameter %3600) /60));   /** 启动充电时间 */
        }
    }
        break;
    default:
        break;
    }
}

/*************************************************************
 * 函数名           packing_data
 * 功能                                                                  将数据封装到指定缓存
 * 参数               buff                指向缓存
 *        buff_free_len       缓存可用长度
 *        data                指向被封装的数据
 *        data_len            被封装数据长度
 *        flag                处理选择
 * 返回               RT_ERROR             失败
 *        RT_EOK               成功
 * 作者               Yang
 ************************************************************/
int8_t packing_data(uint8_t* buff, uint8_t buff_free_len, uint32_t data, uint8_t data_len, uint8_t flag)
{
    if(buff == NULL || data_len > buff_free_len)
    {
        LOG_E("input para error|%p |%d |%d", buff, data_len, buff_free_len);
        return 0;
    }

    for(uint8_t i = 0; i < data_len; i++)
    {
        if(flag &START_FROM_HIGH_BYTE)     buff[i] = (data >>(8 *(data_len - 1 - i))) &0xff;
        else                               buff[i] = (data >>(8 *i)) &0xff;
    }
    return data_len;
}

/*************************************************
 * 函数名         calculate_data_from_byte
 * 功能                                                             将被拆分成字节的数据重新合成
 *       data               字节数据数据体
 *       len                字节长度
 *       flag               处理标志(高字节在前或低字节在前)
 * 返回             result             合成结果
 * 作者            Yang
 ************************************************/
uint32_t calculate_data_from_byte(uint8_t* data, uint8_t len, uint8_t flag)
{
    if(len >sizeof(uint32_t))
    {
        return 0;
    }

    uint32_t result = 0;
    int8_t i = 0;

    /* 高字节在前 */
    if(flag &START_FROM_HIGH_BYTE)
    {
        for(i = len - 1; i >= 0; i--)
        {
            result |= data[i];
            if(i > 0)       result <<= 8;
        }
    }
    /* 低字节在前 */
    else if(flag &START_FROM_LOW_BYTE)
    {
        for(i = 0; i < len; i++)
        {
            result |= data[i];
            if(i < len - 1)       result <<= 8;
        }
    }
    return result;
}


uint32_t get_check_sum(uint8_t* data, uint32_t len)
{
    uint32_t result = 0;

    if(data == NULL)
    {
        return result;
    }

    for(uint32_t count = 0; count < len; count++)
    {
        result += data[count];
    }

    return result;
}

uint32_t crc32_ieee(uint32_t crc, const uint8_t *data, uint32_t len)
{
    /* crc table generated from polynomial 0xedb88320 */
    static const uint32_t table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    crc = ~crc;

    for (uint32_t i = 0; i < len; i++) {
        uint8_t byte = data[i];

        crc = (crc >> 4) ^ table[(crc ^ byte) & 0x0f];
        crc = (crc >> 4) ^ table[(crc ^ (byte >> 4)) & 0x0f];
    }

    return (~crc);
}

uint16_t get_crc16_modbus(uint16_t crc, uint8_t *data, uint32_t len)
{
    static uint16_t crc_talbe[] = {
        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
    };

    uint32_t i = 0;
    uint8_t ch;

    for (i = 0; i < len; i++){
        ch = *data++;
        crc = crc_talbe[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = crc_talbe[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    }
    return crc;
}

