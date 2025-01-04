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
#include "app_ofsm.h"
#include "chargepile_config.h"

#include "mw_time.h"
#include "mw_fault_check.h"

#define DBG_TAG "support"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

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
#define STOPWAY_LIBRARY_MAX       APP_SYSTEM_STOP_WAY_NULL - 1       /* 库停止码最大值 */
#define STOPWAY_OWNER_MAX         APP_SYSTEM_STOP_WAY_SOC_LIMIT       /* 自定义停止码最大值 */
#define STOPWAY_OWNER_MIN         APP_SYSTEM_STOP_WAY_APP_STOP       /* 自定义停止码最小值 */

#define STOPWAY_LIBRARY_NONE_FAULT_MAX        APP_SYSTEM_STOP_WAY_NULL - 1       /* 库非故障停充码最大值 */
#define STOPWAY_LIBRARY_NONE_FAULT_MIN        APP_SYSTEM_STOP_WAY_SHORTS       /* 库非故障停充码最大值 */

    if(code <= STOPWAY_LIBRARY_MAX){                                       /** 库停充原因 */
        if((code >= STOPWAY_LIBRARY_NONE_FAULT_MIN) && (code <= STOPWAY_LIBRARY_NONE_FAULT_MAX)){
            return library_fault_str[code - STOPWAY_LIBRARY_NONE_FAULT_MIN];
        }
        return library_fault_str[code];
    }else if((code >= STOPWAY_OWNER_MIN) && (code <= STOPWAY_OWNER_MAX)){  /** 自定义停充原因 */
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
    }

    return "unknow";
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

