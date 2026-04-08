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
     "cabinet offline",           /** 系统故障码字符串 24：主机柜离线 */
     "matrelayK1-1",              /** 系统故障码字符串 25：矩阵正负接触器KPN1-1 */
     "matrelayK1-2",              /** 系统故障码字符串 26：矩阵正负接触器KPN1-2 */
     "matrelayK1-3",              /** 系统故障码字符串 27：矩阵正负接触器KPN1-3 */
     "matrelayK2-1",              /** 系统故障码字符串 28：矩阵正负接触器KPN2-1 */
     "matrelayK2-2",              /** 系统故障码字符串 29：矩阵正负接触器KPN2-2 */
     "matrelayK3-1",              /** 系统故障码字符串 30：矩阵正负接触器KPN3-1 */
     "slave offline",             /** 系统故障码字符串 31：从设备离线 */
     "fan",                       /** 系统故障码字符串 32：风扇 */
     "cabinet scram",             /** 系统故障码字符串 33：主机柜急停 */
     "cabinet door",              /** 系统故障码字符串 34：主机柜门禁 */
     "cabinet pdu",               /** 系统故障码字符串 35：主机柜开关板故障 */
     "cabinet module",            /** 系统故障码字符串 36：主机柜模块 */
     "cabinet config",            /** 系统故障码字符串 37：主机柜配置项 */
     "cabinet acrelay",           /** 系统故障码字符串 38：主机柜交流接触器 */
     "cabinet smoke",             /** 系统故障码字符串 39：主机柜烟感报警 */
     "cabinet pour",              /** 系统故障码字符串 40：主机柜倾倒 */
     "cabinet flood",             /** 系统故障码字符串 41：主机柜水浸 */
     "cabinet other",             /** 系统故障码字符串 42：主机柜其它故障 */
     "cabinet lightprotect",      /** 系统故障码字符串 43：主机柜防雷故障 */
     "dev locked",                /** 系统故障码字符串 44：设备锁定 */
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
    system_fault_str[24] = "cabinet offline";           /** 主机柜离线 */
    system_fault_str[25] = "matrelayK1-1";              /** 系统故障码字符串 25：矩阵正负接触器KPN1-1 */
    system_fault_str[26] = "matrelayK1-2";              /** 系统故障码字符串 26：矩阵正负接触器KPN1-2 */
    system_fault_str[27] = "matrelayK1-3";              /** 系统故障码字符串 27：矩阵正负接触器KPN1-3 */
    system_fault_str[28] = "matrelayK2-1";              /** 系统故障码字符串 28：矩阵正负接触器KPN2-1 */
    system_fault_str[29] = "matrelayK2-2";              /** 系统故障码字符串 29：矩阵正负接触器KPN2-2 */
    system_fault_str[30] = "matrelayK3-1";              /** 系统故障码字符串 30：矩阵正负接触器KPN3-1 */
    system_fault_str[31] = "slave offline";             /** 系统故障码字符串 31：从设备离线 */
    system_fault_str[32] = "fan";                      /** 系统故障码字符串 32：风扇 */
    system_fault_str[33] = "cabinet scram";            /** 系统故障码字符串 33：主机柜急停 */
    system_fault_str[34] = "cabinet door";              /** 系统故障码字符串 34：主机柜门禁 */
    system_fault_str[35] = "cabinet pdu";               /** 系统故障码字符串 35：主机柜开关板故障 */
    system_fault_str[36] = "cabinet module";            /** 系统故障码字符串 36：主机柜模块 */
    system_fault_str[37] = "cabinet config";            /** 系统故障码字符串 37：主机柜配置项 */
    system_fault_str[38] = "cabinet acrelay";           /** 系统故障码字符串 38：主机柜交流接触器 */
    system_fault_str[39] = "cabinet smoke";             /** 系统故障码字符串 39：主机柜烟感报警 */
    system_fault_str[40] = "cabinet pour";              /** 系统故障码字符串 40：主机柜倾倒 */
    system_fault_str[41] = "cabinet flood";             /** 系统故障码字符串 41：主机柜水浸 */
    system_fault_str[42] = "cabinet other";             /** 系统故障码字符串 42：主机柜其它故障 */
    system_fault_str[43] = "cabinet lightprotect";      /** 系统故障码字符串 43：主机柜防雷故障 */
    system_fault_str[44] = "dev locked";                /** 系统故障码字符串 44：设备锁定 */

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
    case APP_SYS_FAULT_MAIN_CABINET_OFFLINE:
        memcpy(buf, "主机柜离线", strlen("主机柜离线"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_1:
        memcpy(buf, "矩阵继电器K1-1", strlen("矩阵继电器K1-1"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_2:
        memcpy(buf, "矩阵继电器K1-2", strlen("矩阵继电器K1-2"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_3:
        memcpy(buf, "矩阵继电器K1-3", strlen("矩阵继电器K1-3"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN2_1:
        memcpy(buf, "矩阵继电器K2-1", strlen("矩阵继电器K2-1"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN2_2:
        memcpy(buf, "矩阵继电器K2-2", strlen("矩阵继电器K2-2"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN3_1:
        memcpy(buf, "矩阵继电器K3-1", strlen("矩阵继电器K3-1"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_SLAVE_DEVICE_OFFLINE:
        memcpy(buf, "从设备离线", strlen("从设备离线"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_FAN:
        memcpy(buf, "风扇", strlen("风扇"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_SCRAM:
        memcpy(buf, "主机柜急停", strlen("主机柜急停"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_GATE:
        memcpy(buf, "主机柜门禁", strlen("主机柜门禁"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_PDUFAULT:
        memcpy(buf, "主机柜PDU", strlen("主机柜PDU"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_MODULEFAULT:
        memcpy(buf, "主机柜模块", strlen("主机柜模块"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_CONFIG:
        memcpy(buf, "主机柜配置", strlen("主机柜配置"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_ACRELAY:
        memcpy(buf, "主机柜交流接触器", strlen("主机柜交流接触器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_SMOKE:
        memcpy(buf, "主机柜烟感", strlen("主机柜烟感"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_POUR:
        memcpy(buf, "主机柜倾倒", strlen("主机柜倾倒"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_FLOODING:
        memcpy(buf, "主机柜水浸", strlen("主机柜水浸"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_OTHER:
        memcpy(buf, "主机柜其它", strlen("主机柜其它"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_MAINCABINET_LIGHT_PROTECT:
        memcpy(buf, "主机柜防雷", strlen("主机柜防雷"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_FAULT_DEVICE_IS_LOCKED:
        memcpy(buf, "设备锁定", strlen("设备锁定"));
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
    case APP_SYSTEM_STOP_WAY_MAIN_CABINET_OFFLINE:
        memcpy(buf, "主机柜离线", strlen("主机柜离线"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN1_1:
        memcpy(buf, "矩阵继电器K1-1", strlen("矩阵继电器K1-1"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN1_2:
        memcpy(buf, "矩阵继电器K1-2", strlen("矩阵继电器K1-2"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN1_3:
        memcpy(buf, "矩阵继电器K1-3", strlen("矩阵继电器K1-3"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN2_1:
        memcpy(buf, "矩阵继电器K2-1", strlen("矩阵继电器K2-1"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN2_2:
        memcpy(buf, "矩阵继电器K2-2", strlen("矩阵继电器K2-2"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MATRIX_RELAY_KPN3_1:
        memcpy(buf, "矩阵继电器K3-1", strlen("矩阵继电器K3-1"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_SLAVE_DEVICE_OFFLINE:
        memcpy(buf, "从设备离线", strlen("从设备离线"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_FAN:
        memcpy(buf, "风扇", strlen("风扇"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_SCRAM:
        memcpy(buf, "主机柜急停", strlen("主机柜急停"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_GATE:
        memcpy(buf, "主机柜门禁", strlen("主机柜门禁"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_PDUFAULT:
        memcpy(buf, "主机柜PDU", strlen("主机柜PDU"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_MODULEFAULT:
        memcpy(buf, "主机柜模块", strlen("主机柜模块"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_CONFIG:
        memcpy(buf, "主机柜配置", strlen("主机柜配置"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_ACRELAY:
        memcpy(buf, "主机柜交流接触器", strlen("主机柜交流接触器"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_SMOKE:
        memcpy(buf, "主机柜烟感", strlen("主机柜烟感"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_POUR:
        memcpy(buf, "主机柜倾倒", strlen("主机柜倾倒"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_FLOODING:
        memcpy(buf, "主机柜水浸", strlen("主机柜水浸"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_OTHER:
        memcpy(buf, "主机柜其它", strlen("主机柜其它"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_MAINCABINET_LIGHT_PROTECT:
        memcpy(buf, "主机柜防雷", strlen("主机柜防雷"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_DEVICE_IS_LOCKED:
        memcpy(buf, "设备锁定", strlen("设备锁定"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_YT_BFC:
        memcpy(buf, "宇通协议BFC故障", strlen("宇通协议BFC故障"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    case APP_SYSTEM_STOP_WAY_BST_TARGET_SOC:
        memcpy(buf, "BST-SOC达到目标值", strlen("BST-SOC达到目标值"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_TARGET_TOTAL_VOLT:
        memcpy(buf, "BST-总电压达到目标值", strlen("BST-总电压达到目标值"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_TARGET_SINGLE_VOLT:
        memcpy(buf, "BST-单体电压达到目标值", strlen("BST-单体电压达到目标值"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_CHARGER_END:
        memcpy(buf, "BST-充电机主动停止", strlen("BST-充电机主动停止"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_INSULATION_FAULT:
        memcpy(buf, "BST-车端绝缘故障", strlen("BST-车端绝缘故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_OUT_LINKER_FAULT:
        memcpy(buf, "BST-输出连接器故障", strlen("BST-输出连接器故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_BMS_ELEMENT:
        memcpy(buf, "BST-BMS元件故障", strlen("BST-BMS元件故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_CHARGE_LINKER_FAULT:
        memcpy(buf, "BST-充电连接故障", strlen("BST-充电连接故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_BAT_GROUP_OT:
        memcpy(buf, "BST-电池组温度故障", strlen("BST-电池组温度故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_HV_RELAY:
        memcpy(buf, "BST-高压继电器故障", strlen("BST-高压继电器故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_DETECT_PIONT_2:
        memcpy(buf, "BST-检测点2电压故障", strlen("BST-检测点2电压故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_OVER_CURRENT:
        memcpy(buf, "BST-充电电流过流", strlen("BST-充电电流过流"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BST_ABNORMAL_VOLTAGE:
        memcpy(buf, "BST-充电电压异常", strlen("BST-充电电压异常"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM:
        memcpy(buf, "BSM-车端故障", strlen("BSM-车端故障"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_SINGLE_BAT_OV:
        memcpy(buf, "BSM-单体电压异常", strlen("BSM-单体电压异常"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_ABNORMAL_SOC:
        memcpy(buf, "BSM-SOC状态异常", strlen("BSM-SOC状态异常"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_OVER_CURRENT:
        memcpy(buf, "BSM-电池充电过流", strlen("BSM-电池充电过流"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_BATTERY_OT:
        memcpy(buf, "BSM-电池温度过高", strlen("BSM-电池温度过高"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_BAT_INSULATION_ABNORMAL:
        memcpy(buf, "BSM-电池绝缘状态异常", strlen("BSM-电池绝缘状态异常"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_OUT_LINKER_ABNORMAL:
        memcpy(buf, "BSM-车端输出连接器状态异常", strlen("BSM-车端输出连接器状态异常"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    case APP_SYSTEM_STOP_WAY_BSM_FORBID:
        memcpy(buf, "BSM-车端禁止充电", strlen("BSM-车端禁止充电"));
        if(olen)
            *olen = strlen((char*)buf);
        return;
    default:
        memcpy(buf, "未知", strlen("未知"));
        if(olen)
            *olen = strlen((char*)buf);
        break;
    }
}


static uint8_t app_padding_debug_info(uint8_t *buff, const char *prefix, const char *ret, uint8_t ilen)
{
    if(buff == NULL){
        return 0x00;
    }
    if((strlen(prefix) + strlen(ret)) >= ilen){
        return 0x00;
    }
    if(prefix){
        sprintf((char*)buff, "%s", prefix);
    }
    if(ret){
        sprintf((char*)(buff + strlen((char*)buff)), "%s", ret);
    }

    return 0x01;
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
    const char *cstr[2] = {"异常", "正常"};
    const char *estr[2] = {"Fail", "OK"};

    memset(buf, 0x00, ilen);
    ret = ret == 0x00 ? 0x00 : 0x01;

    switch(item){
    case THA_DEBUG_ITEM_ACRELAY_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "AcRelayClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "交流接触器闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_ACRELAY_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "AcRelayBreak:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "交流接触器断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_1_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Para1RelayClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "母联1闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_1_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Para1RelayBreak:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "母联1断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_2_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Para2RelayClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "母联2闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_2_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Para2RelayBreak:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "母联2断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_3_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Para3RelayClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "母联3闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_PARARELAY_3_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Para3RelayBreak:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "母联3断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_FAN_ON_A:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "FanAOpen:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪风扇开启：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_FAN_OFF_A:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "FanAClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪风扇关闭：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_FAN_ON_B:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "FanBOpen:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪风扇开启：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_FAN_OFF_B:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "FanBClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪风扇关闭：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "DcRelayAClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪直流继电器闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "DcRelayABreak:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪直流继电器断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_B_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "DcRelayBClose:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪直流继电器闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_DCRELAY_B_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "DcRelayBBreak:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪直流继电器断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "ELockA Lock:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪电子锁上锁：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "ELockA UnLock:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪电子锁解锁：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_B_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "ELockB Lock:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪电子锁上锁：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_ELOCK_B_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "ELockB UnLock:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪电子锁解锁：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux12VCloseA:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪12V辅源闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux12VBreakA:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪12V辅源断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_B_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux12VCloseB:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪12V辅源闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX12V_B_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux12VBreakB:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪12V辅源断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux24VCloseA:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪24V辅源闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux24VBreakA:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "A枪24V辅源断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_B_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux24VCloseB:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪24V辅源闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_AUX24V_B_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "Aux24VBreakB:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "B枪24V辅源断开：", cstr[ret], ilen);
        }
        break;
#ifdef APP_USING_CYCLE_MATRIX
    case THA_DEBUG_ITEM_MRELAY_K1_1_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK1-1Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K1-1闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K1_1_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK1-1Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K1-1断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K1_2_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK1-2Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K1-2闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K1_2_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK1-2Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K1-2断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K1_3_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK1-3Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K1-3闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K1_3_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK1-3Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K1-3断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K2_1_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK2-1Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K2-1闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K2_1_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK2-1Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K2-1断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K2_2_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK2-2Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K2-2闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K2_2_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK2-2Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K2-2断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K2_3_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK2-3Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K2-3闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K2_3_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK2-3Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K2-3断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K3_1_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK3-1Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K3-1闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K3_1_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK3-1Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K3-1断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K3_2_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK3-2Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K3-2闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K3_2_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK3-2Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K3-2断开：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K3_3_A_ON:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK3-3Close:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K3-3闭合：", cstr[ret], ilen);
        }
        break;
    case THA_DEBUG_ITEM_MRELAY_K3_3_A_OFF:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "MRelayK3-3Break:", estr[ret], ilen);
        }else{
            app_padding_debug_info(buf, "矩阵K3-3断开：", cstr[ret], ilen);
        }
        break;
#endif /* APP_USING_CYCLE_MATRIX */
    case THA_DEBUG_ITEM_COMPLETE:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            app_padding_debug_info(buf, "SelfCheck Complete", NULL, ilen);
        }else{
            app_padding_debug_info(buf, "自检完成", NULL, ilen);
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
    case THAISEN_MODE_CHARGE_LIMIT_MONEY:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:LimitMoney ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x03 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%s", "Mode:LimitMoney ", (parameter /100), ((parameter /10) %10), (parameter %10), "RMB");   /** 两位小数 */
        }else{
            if(ilen <= (strlen("模式:定额充电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x02 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%s", "模式:定额充电  ", (parameter /100), ((parameter /10) %10), (parameter %10), "元");   /** 两位小数 */
        }
        break;
    case THAISEN_MODE_CHARGE_LIMIT_ELECT:
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:LimitElect ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x04 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%lu%s", "Mode:LimitElect", (parameter /1000), ((parameter /100) %10), ((parameter /10) %10), (parameter %10), "KW*h");   /** 三位小数 */
        }else{
            if(ilen <= (strlen("模式:定量充电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x02 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%lu%s", "模式:定量充电  ", (parameter /1000), ((parameter /100) %10), ((parameter /10) %10), (parameter %10), "度");   /** 三位小数 */
        }
        break;
    case THAISEN_MODE_CHARGE_LIMIT_TIMING:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:Timing ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x03))
                return;
            sprintf((char*)buf, "%s%lu%s", "Mode:Timing ", parameter, "min");   /** 分钟 */
        }else{
            if(ilen <= (strlen("模式:定时充电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x04))
                return;
            sprintf((char*)buf, "%s%lu%s", "模式:定时充电  ", parameter, "分钟");   /** 分钟 */
        }
    }
        break;
    case THAISEN_MODE_CHARGE_LIMIT_RESERVATION:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Mode:Reservation ") + (uint32_t)(log10((double)(parameter /3600)) + 0x01) + 0x01 + (uint32_t)(log10((double)((parameter %3600) /60)) + 0x01)))
                return;
            sprintf((char*)buf, "%s%02lu%c%02lu", "Mode:Reservation ", (parameter /3600), ':', ((parameter %3600) /60));   /** 启动充电时间 */
        }else{
            if(ilen <= (strlen("模式:预约充电  ") + (uint32_t)(log10((double)(parameter /3600)) + 0x01) + 0x01 + (uint32_t)(log10((double)((parameter %3600) /60)) + 0x01)))
                return;
            sprintf((char*)buf, "%s%02lu%c%02lu", "模式:预约充电  ", (parameter /3600), ':', ((parameter %3600) /60));   /** 启动充电时间 */
        }
    }
        break;
#ifdef APP_INCLUDE_V2G
    case THAISEN_MODE_V2G_LIMIT_MONEY:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("V2G:LimitMoney ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x03 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%s", "V2G:LimitMoney ", (parameter /100), ((parameter /10) %10), (parameter %10), "RMB");   /** 两位小数 */
        }else{
            if(ilen <= (strlen("模式:定额放电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x02 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%s", "模式:定额放电  ", (parameter /100), ((parameter /10) %10), (parameter %10), "元");   /** 两位小数 */
        }
    }
        break;
    case THAISEN_MODE_V2G_LIMIT_ELECT:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("V2G:LimitElect ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x04 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%lu%s", "V2G:LimitElect", (parameter /1000), ((parameter /100) %10), ((parameter /10) %10), (parameter %10), "KW*h");   /** 三位小数 */
        }else{
            if(ilen <= (strlen("模式:定量放电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x02 + 0x01))
                return;
            sprintf((char*)buf, "%s%lu.%lu%lu%lu%s", "模式:定量放电  ", (parameter /1000), ((parameter /100) %10), ((parameter /10) %10), (parameter %10), "度");   /** 三位小数 */
        }
    }
        break;
    case THAISEN_MODE_V2G_LIMIT_TIMING:
    {
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("V2G:Timing ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x03))
                return;
            sprintf((char*)buf, "%s%lu%s", "V2G:Timing ", parameter, "min");   /** 分钟 */
        }else{
            if(ilen <= (strlen("模式:定时放电  ") + (uint32_t)(log10((double)parameter) + 0x01) + 0x04))
                return;
            sprintf((char*)buf, "%s%lu%s", "模式:定时放电  ", parameter, "分钟");   /** 分钟 */
        }
    }
        break;
    case THAISEN_MODE_V2G_AUTO:
    {
        /** 最大3位数 */
        while((parameter /1000)){
            parameter /= 10;
        }
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("V2G:Auto ") + 0x05))
                return;
            sprintf((char*)buf, "%s%02lu%c", "V2G:Auto ", parameter, '%');   /** 放电截至SOC */
        }else{
            if(ilen <= (strlen("模式:自动放电  ") + 0x04))
                return;
            sprintf((char*)buf, "%s%02lu%c", "模式:自动放电  ", parameter, '%');   /** 放电截至SOC */
        }
    }
        break;
#endif /* APP_INCLUDE_V2G */
    default:
        break;
    }
}

/********************************************
 * 函数名      app_get_module_fault_info
 * 功能          获取模块故障信息
 * 参数          gunno     枪号
 *      language   语言
 *      addr       模块地址
 *      fault_code 故障码
 *      buf        用于保存显示信息
 *      ilen       缓存长度
 * 返回
 *******************************************/
void app_get_module_fault_info(uint8_t gunno, uint8_t language, uint8_t addr, uint16_t fault_code, uint8_t *buf, uint8_t ilen)
{
#define INOVERVOLT_INDEX         (1 <<0)            /* 输入过压 */
#define INUNDERVOLT_INDEX        (1 <<1)            /* 输入欠压 */
#define OUTOVERVOLT_INDEX        (1 <<2)            /* 输出过压 */
#define OUTUNDERVOLT_INDEX       (1 <<3)            /* 输出欠压 */
#define SAMEID_INDEX             (1 <<4)            /* 相同ID */
#define MODULEFAULT_INDEX        (1 <<5)            /* 模块故障 */
#define OVERCURR_INDEX           (1 <<6)            /* 过流 */
#define OVERTEMP_INDEX           (1 <<7)            /* 过温 */
#define FAN_INDEX                (1 <<8)            /* 风扇 */

#define FAULT_MASK               0x1FF              /* 有效故障掩码 */

#define FAULT_SHOW_NUM_MAX       0x02               /* 同时显示故障的个数 */

    if((buf == NULL) || (ilen == 0x00)){
        return;
    }
    uint8_t used_len = strlen((char*)buf), used_count = 0x00, is_multiple = 0x20;  /** 是否有多个故障(用于添加分隔符“,”) */

    fault_code &= FAULT_MASK;
    if(fault_code){
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("addr:") + 0x02 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%s%02X", "addr:", addr);
        }else{
            if(ilen <= (strlen("地址:") + 0x02 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%s%02X", "地址:", addr);
        }
        used_len = strlen((char*)buf);
    }else{
        return;
    }

    if(fault_code &INOVERVOLT_INDEX){
//        if(used_count >= FAULT_SHOW_NUM_MAX)
//            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("in OverVolt") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "in OverVolt");
        }else{
            if(ilen <= (strlen("输入过压") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "输入过压");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &INUNDERVOLT_INDEX){
//        if(used_count >= FAULT_SHOW_NUM_MAX)
//            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("In UnderVolt") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "In UnderVolt");
        }else{
            if(ilen <= (strlen("输入欠压") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "输入欠压");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &OUTOVERVOLT_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Out OverVolt") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "Out OverVolt");
        }else{
            if(ilen <= (strlen("输出过压") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "输出过压");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &OUTUNDERVOLT_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Out UnderVolt") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "Out UnderVolt");
        }else{
            if(ilen <= (strlen("输出欠压") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "输出欠压");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &SAMEID_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("SameID") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "SameID");
        }else{
            if(ilen <= (strlen("同地址") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "同地址");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &MODULEFAULT_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Fault/Offline") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "Fault/Offline");
        }else{
            if(ilen <= (strlen("故障或离线") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "故障或离线");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &OVERCURR_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("in OverCurr") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "in OverCurr");
        }else{
            if(ilen <= (strlen("过流") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "过流");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &OVERTEMP_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("in OverTemp") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "in OverTemp");
        }else{
            if(ilen <= (strlen("过温") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "过温");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }

    if(fault_code &FAN_INDEX){
        if(used_count >= FAULT_SHOW_NUM_MAX)
            return;
        if(language == THA_DEBUG_LANGUAGE_ENGLISH){
            if(ilen <= (strlen("Fan") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "Fan");
        }else{
            if(ilen <= (strlen("风扇") + 0x01 + used_len))
                return;
            sprintf(((char*)buf + used_len), "%c%s", is_multiple, "风扇");
        }
        is_multiple = ',';
        used_len = strlen((char*)buf);
        used_count++;
    }
}


/********************************************
 * 函数名      app_cmd_debug_result_info
 * 功能          获取获取指令调试结果信息
 * 参数          gunno        枪号
 *         language  语言
 *         cmd       指令
 *         para      参数
 *         plen      参数长度
 *         buf       用于保存显示信息
 *         ilen      缓存长度
 * 返回
 *******************************************/
void app_cmd_debug_result_info(uint8_t gunno, uint8_t language, uint8_t cmd, uint8_t *para, uint8_t plen, uint8_t *buf, uint8_t ilen)
{
    if((buf == NULL) || (ilen == 0x00)){
        return;
    }
    memset(buf, 0x00, ilen);

    switch(cmd){
    case THAISEN_DEBUG_CMD_ISSUE_MODULE_CURR_MAX:
        if(para && (plen >= 0x04)){
            uint32_t curr = *(uint32_t*)para;
            /** 电流值最大6位:XXXX.XXA */
            curr = curr >= 1000000 ? 999999 : curr;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>ModuleCurrMax:") + (uint32_t)(log10((double)curr) + 0x01) + 0x01 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "Issue>ModuleCurrMax:", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }else{
                if(ilen <= (strlen("下发>模块最大输出电流：") + (uint32_t)(log10((double)curr) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "下发>模块最大输出电流：", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_MODULE_CURR_MIN:
        if(para && (plen >= 0x04)){
            uint32_t curr = *(uint32_t*)para;
            /** 电流值最大6位:XXXX.XXA */
            curr = curr >= 1000000 ? 999999 : curr;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>ModuleCurrMin:") + (uint32_t)(log10((double)curr) + 0x01) + 0x01 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "Issue>ModuleCurrMin:", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }else{
                if(ilen <= (strlen("下发>模块最小输出电流：") + (uint32_t)(log10((double)curr) + 0x01) + 0x01 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "下发>模块最小输出电流：", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_LED_LANGUAGE:
        if(para){
            uint8_t number = *(uint8_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>LedLanguage-") + 0x01))
                    return;
                sprintf((char*)buf, "%s%d", "Issue>LedLanguage-", number);
            }else{
                if(ilen <= (strlen("下发>灯语-") + 0x01))
                    return;
                sprintf((char*)buf, "%s%d", "下发>灯语-", number);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CC12V_UPLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CC12UpLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CC12UpLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("下发>枪1 CC12V上限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "CC12V上限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CC12V_LOWLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CC12LowLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CC12LowLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("下发>枪1 CC12V下限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "CC12V下限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CC6V_UPLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CC6UpLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CC6UpLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("下发>枪1 CC6V上限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "CC6V上限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CC6V_LOWLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CC6LowLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CC6LowLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("下发>枪1 CC6V下限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "CC6V下限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CC4V_UPLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CC4UpLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CC4UpLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("下发>枪1 CC4V上限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "CC4V上限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CC4V_LOWLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CC4LowLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CC4LowLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("下发>枪1 CC4V下限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "CC4V下限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_MELECT_STRATEGY:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>MElectStrategy:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Issue>MElectStrategy:", e_str[function]);
            }else{
                if(ilen <= (strlen("下发>电表电量检测策略：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "下发>电表电量检测策略：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_BATVOLT_STRATEGY:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x00 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>BatVoltStrategy:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Issue>BatVoltStrategy:", e_str[function]);
            }else{
                if(ilen <= (strlen("下发>电池电压检测策略：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "下发>电池电压检测策略：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_CURR_STRATEGY:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>CurrStrategy:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Issue>CurrStrategy:", e_str[function]);
            }else{
                if(ilen <= (strlen("下发>充电电流检测策略：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "下发>充电电流检测策略：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_SETUP_CURR_OFFSET:
        if(para && (plen >= 0x04)){
            int32_t offset = 0x00;

            if(*(uint16_t*)para >= CP_CURRENT_OFFSET_SEPARATE){
                offset = (*(uint16_t*)para - CP_CURRENT_OFFSET_SEPARATE);
            }else{
                offset = -(*(uint16_t*)para);
            }

            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>gun1 CurrOffset:") + (uint32_t)(log10((double)(*(uint16_t*)para)) + 0x01) + 0x02 + 0x01))
                    return;
                if(offset >= 0x00){
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CurrOffset:", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }else{
                    offset = 0x00 - offset;
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "Issue>gun", (uint32_t)(gunno + 0x01), "CurrOffset:-", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }
            }else{
                if(ilen <= (strlen("下发>枪1 设置电流偏移：") + (uint32_t)(log10((double)(*(uint16_t*)para)) + 0x01) + 0x02 + 0x01))
                    return;
                if(offset >= 0x00){
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "设置电流偏移：", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }else{
                    offset = 0x00 - offset;
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "下发>枪", (uint32_t)(gunno + 0x01), "设置电流偏移：-", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_TEL:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x00 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>GBT_ELock:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Issue>GBT_ELock:", e_str[function]);
            }else{
                if(ilen <= (strlen("下发>测试：电子锁检测：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "下发>测试：电子锁检测：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_TOC:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x00 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>GBT_OC:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Issue>GBT_OC:", e_str[function]);
            }else{
                if(ilen <= (strlen("下发>测试：过流检测：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "下发>测试：过流检测：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_TOCDT:
        if(para && (plen >= 0x02)){
            uint16_t value = *(uint16_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>Test OCDT:") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "Issue>Test OCDT:", value);
            }else{
                if(ilen <= (strlen("下发>过流测试检测时长：") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "下发>过流测试检测时长:", value);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_FPP:
        if(para && (plen >= 0x04)){
            uint32_t value = *(uint32_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>FPWMPeriod:") + (uint32_t)(log10((double)value) + 0x01) + 0x02))
                    return;
                sprintf((char*)buf, "%s%lu%s", "Issue>FPWMPeriod:", value, "Hz");
            }else{
                if(ilen <= (strlen("下发>风扇调速周期：") + (uint32_t)(log10((double)value) + 0x01) + 0x02))
                    return;
                sprintf((char*)buf, "%s%lu%s", "下发>风扇调速周期:", value, "Hz");
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_FPTP:
        if(para && (plen >= 0x02)){
            uint16_t value = *(uint16_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>FPWMTimPres:") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "Issue>FPWMTimPres:", value);
            }else{
                if(ilen <= (strlen("下发>风扇调速定时分频：") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "下发>风扇调速定时分频:", value);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_OPCS:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x00 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>OutPeakCurrSW:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Issue>OutPeakCurrSW:", e_str[function]);
            }else{
                if(ilen <= (strlen("下发>输出峰值电流功能：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "下发>输出峰值电流功能：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_ISSUE_OPCV:
        if(para && (plen >= 0x04)){
            uint32_t value = *(uint32_t*)para;

            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Issue>OutPeakCurrVal:") + (uint32_t)(log10((double)value) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "Issue>OutPeakCurrVal:", (value /100), ((value /10) %10), (value %10), 'A');
            }else{
                if(ilen <= (strlen("下发>输出峰值电流值：") + (uint32_t)(log10((double)value) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "下发>输出峰值电流值:", (value /100), ((value /10) %10), (value %10), 'A');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_MODULE_CURR_MAX:
        if(para && (plen >= 0x04)){
            uint32_t curr = *(uint32_t*)para;
            /** 电流值最大6位:XXXX.XXA */
            curr = curr >= 1000000 ? 999999 : curr;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>ModuleCurrMax:") + (uint32_t)(log10((double)curr) + 0x01) + 0x01 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "Read>ModuleCurrMax:", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }else{
                if(ilen <= (strlen("读取>模块最大输出电流：") + (uint32_t)(log10((double)curr) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "读取>模块最大输出电流：", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_MODULE_CURR_MIN:
        if(para && (plen >= 0x04)){
            uint32_t curr = *(uint32_t*)para;
            /** 电流值最大6位:XXXX.XXA */
            curr = curr >= 1000000 ? 999999 : curr;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>ModuleCurrMin:") + (uint32_t)(log10((double)curr) + 0x01) + 0x01 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "Read>ModuleCurrMin:", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }else{
                if(ilen <= (strlen("读取>模块最小输出电流：") + (uint32_t)(log10((double)curr) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "读取>模块最小输出电流：", (curr /100), ((curr /10) %10), (curr %10), 'A');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_LED_LANGUAGE:
        if(para){
            uint8_t number = *(uint8_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>LedLanguage-") + 0x01))
                    return;
                sprintf((char*)buf, "%s%d", "LedLanguage-", number);
            }else{
                if(ilen <= (strlen("读取>灯语-") + 0x01))
                    return;
                sprintf((char*)buf, "%s%d", "读取>灯语-", number);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CC12V_UPLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CC12UpLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CC12UpLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("读取>枪1 CC12V上限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "CC12V上限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CC12V_LOWLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CC12LowLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CC12LowLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("读取>枪1 CC12V下限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "CC12V下限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CC6V_UPLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CC6UpLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CC6UpLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("读取>枪1 CC6V上限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "CC6V上限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CC6V_LOWLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CC6LowLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CC6LowLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("读取>枪1 CC6V下限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "CC6V下限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CC4V_UPLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CC4UpLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CC4UpLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("读取>枪1 CC4V上限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "CC4V上限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CC4V_LOWLIMIT:
        if(para && (plen >= 0x04)){
            uint32_t cc1 = *(uint32_t*)para;
            /** CC值最大5位:XX.XXXV */
            cc1 = cc1 >= 100000 ? 99999 : cc1;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CC4LowLimit:") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CC4LowLimit:", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }else{
                if(ilen <= (strlen("读取>枪1 CC4V下限：") + (uint32_t)(log10((double)cc1) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "CC4V下限：", (cc1 /1000), ((cc1 /100) %10), ((cc1 /10) %10), (cc1 %10), 'V');
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_MELECT_STRATEGY:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>MElectStrategy:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Read>MElectStrategy:", e_str[function]);
            }else{
                if(ilen <= (strlen("读取>电表电量检测策略：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "读取>电表电量检测策略：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_BATVOLT_STRATEGY:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x00 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>BatVoltStrategy:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Read>BatVoltStrategy:", e_str[function]);
            }else{
                if(ilen <= (strlen("读取>电池电压检测策略：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "读取>电池电压检测策略：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_CURR_STRATEGY:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>CurrStrategy:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Read>CurrStrategy:", e_str[function]);
            }else{
                if(ilen <= (strlen("读取>充电电流检测策略：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "读取>充电电流检测策略：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_SETUP_CURR_OFFSET:
        if(para && (plen >= 0x02)){
            int32_t offset = 0x00;

            if(*(uint16_t*)para >= CP_CURRENT_OFFSET_SEPARATE){
                offset = (*(uint16_t*)para - CP_CURRENT_OFFSET_SEPARATE);
            }else{
                offset = -(*(uint16_t*)para);
            }

            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>gun1 CurrOffset:") + (uint32_t)(log10((double)(*(uint16_t*)para)) + 0x01) + 0x02 + 0x01))
                    return;
                if(offset >= 0x00){
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CurrOffset:", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }else{
                    offset = 0x00 - offset;
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "Read>gun", (uint32_t)(gunno + 0x01), "CurrOffset:-", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }
            }else{
                if(ilen <= (strlen("读取>枪1 设置电流偏移：") + (uint32_t)(log10((double)(*(uint16_t*)para)) + 0x01) + 0x02 + 0x01))
                    return;
                if(offset >= 0x00){
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "设置电流偏移：", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }else{
                    offset = 0x00 - offset;
                    sprintf((char*)buf, "%s%lu %s%lu.%lu%lu%c", "读取>枪", (uint32_t)(gunno + 0x01), "设置电流偏移：-", (offset /100), ((offset /10) %10), (offset %10), 'A');
                }
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_TEL:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>GBT_ELock:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Read>GBT_ELock:", e_str[function]);
            }else{
                if(ilen <= (strlen("读取>测试：电子锁检测：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "读取>测试：电子锁检测：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_TOC:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>GBT_OC:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Read>GBT_OC:", e_str[function]);
            }else{
                if(ilen <= (strlen("读取>测试：过流检测：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "读取>测试：过流检测：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_TOCDT:
        if(para && (plen >= 0x02)){
            uint16_t value = *(uint16_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>Test OCDT:") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "Read>Test OCDT:", value);
            }else{
                if(ilen <= (strlen("读取>过流测试检测时长：") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "读取>过流测试检测时长：", value);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_FPP:
        if(para && (plen >= 0x04)){
            uint32_t value = *(uint32_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>FPWMPeriod:") + (uint32_t)(log10((double)value) + 0x01) + 0x02))
                    return;
                sprintf((char*)buf, "%s%lu%s", "Read>FPWMPeriod:", value, "Hz");
            }else{
                if(ilen <= (strlen("读取>风机调速周期：") + (uint32_t)(log10((double)value) + 0x01) + 0x02))
                    return;
                sprintf((char*)buf, "%s%lu%s", "读取>风机调速周期：", value, "Hz");
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_FPTP:
        if(para && (plen >= 0x02)){
            uint16_t value = *(uint16_t*)para;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>FPWMTimPres:") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "Read>FPWMTimPres:", value);
            }else{
                if(ilen <= (strlen("读取>风机调速定时分频：") + (uint32_t)(log10((double)value) + 0x01)))
                    return;
                sprintf((char*)buf, "%s%lu", "读取>风机调速定时分频：", value);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_OPCS:
        if(para && (plen >= 0x01)){
            uint8_t function = *(uint8_t*)para;
            const char *e_str[2] = {"Close", "Open"};
            const char *c_str[2] = {"关闭", "开启"};

            function = function > 0x01 ? 0x01 : function;
            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>OutPeakCurrSW:") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "Read>OutPeakCurrSW:", e_str[function]);
            }else{
                if(ilen <= (strlen("读取>输出峰值电流功能：") + 0x06))
                    return;
                sprintf((char*)buf, "%s%s", "读取>输出峰值电流功能：", c_str[function]);
            }
        }
        break;
    case THAISEN_DEBUG_CMD_READ_OPCV:
        if(para && (plen >= 0x04)){
            uint32_t value = *(uint32_t*)para;

            if(language == THA_DEBUG_LANGUAGE_ENGLISH){
                if(ilen <= (strlen("Read>OutPeakCurrSW:") + (uint32_t)(log10((double)value) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "Read>OutPeakCurrSW:", (value /100), ((value /10) %10), (value %10), 'A');
            }else{
                if(ilen <= (strlen("读取>输出峰值电流值：") + (uint32_t)(log10((double)value) + 0x01) + 0x02 + 0x01))
                    return;
                sprintf((char*)buf, "%s%lu.%lu%lu%c", "读取>输出峰值电流值", (value /100), ((value /10) %10), (value %10), 'A');
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
