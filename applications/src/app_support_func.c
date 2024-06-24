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
#define STOPWAY_LIBRARY_MAX       0x001A       /* 库停止码最大值 */
#define STOPWAY_OWNER_MAX         0x0039       /* 自定义停止码最大值 */
#define STOPWAY_OWNER_MIN         0x002F       /* 自定义停止码最小值 */

#define STOPWAY_CHARGE_MAX        0x0045       /* 充电故障码最大值 */
#define STOPWAY_CHARGE_MIN        0x0040       /* 充电故障码最小值 */

/** 以下是充电故障码 */
#define STOPWAY_GUN_VOLT          0x0040       /* 停充原因：枪头电压 */
#define STOPWAY_INSULTA           0x0041       /* 停充原因：绝缘 */
#define STOPWAY_COMMON            0x0042       /* 停充原因：BMS通信 */
#define STOPWAY_BATTERY_VOLT      0x0043       /* 停充原因：电池电压 */
#define STOPWAY_READY_VOLT        0x0044       /* 停充原因：预充电压 */
#define STOPWAY_INSULT_VOLT       0x0045       /* 停充原因：绝缘电压 */

    if(code <= (STOPWAY_LIBRARY_MAX + 1)){                                       /** 库停充原因 */
        return library_fault_str[code];
    }else if((code >= STOPWAY_OWNER_MIN) && (code <= (STOPWAY_OWNER_MAX + 1))){  /** 自定义停充原因 */
        return owner_fault_str[code - STOPWAY_OWNER_MIN];
    }else if((code >= STOPWAY_CHARGE_MIN) && (code <= STOPWAY_CHARGE_MAX)){      /** 充电故障码 */
        switch(code - STOPWAY_CHARGE_MIN){
        case (STOPWAY_GUN_VOLT - STOPWAY_CHARGE_MIN):
                return library_fault_str[17];
        break;
        case (STOPWAY_INSULTA - STOPWAY_CHARGE_MIN):
                return library_fault_str[18];
        break;
        case (STOPWAY_COMMON - STOPWAY_CHARGE_MIN):
                return library_fault_str[19];
        break;
        case (STOPWAY_BATTERY_VOLT - STOPWAY_GUN_VOLT):
                return library_fault_str[20];
        break;
        case (STOPWAY_READY_VOLT - STOPWAY_GUN_VOLT):
                return library_fault_str[25];
        break;
        case (STOPWAY_INSULT_VOLT - STOPWAY_GUN_VOLT):
                return library_fault_str[26];
        break;
        default:
        {
            uint8_t unknow_index = (sizeof(owner_fault_str) /4 - 1);
            return owner_fault_str[unknow_index];
            break;
        }
        }
    }else{
        uint8_t unknow_index = (sizeof(owner_fault_str) /4 - 1);
        return owner_fault_str[unknow_index];
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



