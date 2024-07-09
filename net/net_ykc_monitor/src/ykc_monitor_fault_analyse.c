/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-01     我的杨yang       the first version
 */
#include "ykc_monitor_fault_analyse.h"
#include "ykc_monitor_message_send.h"

#ifdef NET_PACK_USING_YKC_MONITOR

#define YKC_MONITOR_FAULT_MSG_NUM_MAX                            0x05

#define YKC_MONITOR_REALTIME_FAULT_SCRAM                         (0x01 <<0)       /* 云快充实时故障：急停按钮动作故障 */
#define YKC_MONITOR_REALTIME_FAULT_RECTIFIER                     (0x01 <<1)       /* 云快充实时故障：无可用整流模块 */
#define YKC_MONITOR_REALTIME_FAULT_AIR_OUTLET_OVERTEMP           (0x01 <<2)       /* 云快充实时故障：出风口温度过高 */
#define YKC_MONITOR_REALTIME_FAULT_AC_LIGHTNING_ARRETER          (0x01 <<3)       /* 云快充实时故障：交流防雷故障 */
#define YKC_MONITOR_REALTIME_FAULT_DC20_MODULE                   (0x01 <<4)       /* 云快充实时故障：交直流模块 DC20 通信中断 */
#define YKC_MONITOR_REALTIME_FAULT_FC08_MODULE                   (0x01 <<5)       /* 云快充实时故障：绝缘检测模块 FC08 通信中断 */
#define YKC_MONITOR_REALTIME_FAULT_AMMETER_COMMUNICATION         (0x01 <<6)       /* 云快充实时故障：电度表通信中断 */
#define YKC_MONITOR_REALTIME_FAULT_CARD_READER_COMMUNICATION     (0x01 <<7)       /* 云快充实时故障：读卡器通信中断 */
#define YKC_MONITOR_REALTIME_FAULT_RC10_COMMUNICATION            (0x01 <<8)       /* 云快充实时故障：RC10 通信中断 */
#define YKC_MONITOR_REALTIME_FAULT_FAN_SPEED_PLATE               (0x01 <<9)       /* 云快充实时故障：风扇调速板故障 */
#define YKC_MONITOR_REALTIME_FAULT_DC_FUSE                       (0x01 <<10)      /* 云快充实时故障：直流熔断器故障 */
#define YKC_MONITOR_REALTIME_FAULT_HV_RELAY                      (0x01 <<11)      /* 云快充实时故障：高压接触器故障 */
#define YKC_MONITOR_REALTIME_FAULT_DOOR                          (0x01 <<12)      /* 云快充实时故障：门打开 */
#define YKC_MONITOR_REALTIME_FAULT_FLASH                         (0x01 <<13)      /* 云快充实时故障：flash */
#define YKC_MONITOR_REALTIME_FAULT_OVER_VOLTAGE                  (0x01 <<14)      /* 云快充实时故障：过压 */
#define YKC_MONITOR_REALTIME_FAULT_UNDER_VOLTAGE                 (0x01 <<15)      /* 云快充实时故障：欠压 */

#pragma pack(1)
struct ykc_monitor_fault_head{
    uint8_t count;
};

struct ykc_monitor_fault_body{
    struct{
        uint8_t is_resume : 4;
        uint8_t onging : 4;
    }flag;
    uint8_t code;
};

struct ykc_monitor_fault_info{
    struct ykc_monitor_fault_head head;
    struct ykc_monitor_fault_body body[YKC_MONITOR_FAULT_MSG_NUM_MAX];
};
#pragma pack()

static uint32_t s_ykc_monitor_current_fault_set[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_ykc_monitor_realtime_fault[NET_SYSTEM_GUN_NUMBER];
static struct ykc_monitor_fault_info s_ykc_monitor_fault_info[NET_SYSTEM_GUN_NUMBER];

/*************************************************
 * 函数名      ykc_monitor_fault_event_detect_callback
 * 功能          故障发生变化时调用，用于记录变化的故障
 * **********************************************/
void ykc_monitor_fault_event_detect_callback(uint8_t gunno, uint8_t code, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t index = 0x00;
    if(s_ykc_monitor_fault_info[gunno].head.count >= YKC_MONITOR_FAULT_MSG_NUM_MAX){
        return;
    }

    rt_enter_critical();

    index = (YKC_MONITOR_FAULT_MSG_NUM_MAX - 0x01 - s_ykc_monitor_fault_info[gunno].head.count);

    if(is_resume){
        s_ykc_monitor_current_fault_set[gunno] &= (~(0x01 <<code));
        s_ykc_monitor_fault_info[gunno].body[index].flag.is_resume = 0x01;
    }else{
        s_ykc_monitor_current_fault_set[gunno] |= (0x01 <<code);
        s_ykc_monitor_fault_info[gunno].body[index].flag.is_resume = 0x00;
    }
    s_ykc_monitor_fault_info[gunno].body[index].flag.onging = 0x00;
    s_ykc_monitor_fault_info[gunno].body[index].code = code;
    s_ykc_monitor_fault_info[gunno].head.count++;

    rt_exit_critical();
}

/*************************************************
 * 函数名      ykc_monitor_get_current_fault_set
 * 功能          获取当前故障集
 * **********************************************/
uint32_t ykc_monitor_get_current_fault_set(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ykc_monitor_current_fault_set[gunno];
}

static void ykc_monitor_clear_fault_event(uint8_t gunno, uint8_t code)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_ykc_monitor_current_fault_set[gunno] &= (~(0x01 <<code));
}

static int32_t ykc_monitor_get_fault_code(uint8_t bit, uint8_t gunno, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    bit = ykc_monitor_chargepile_fault_converted(bit);

    switch(bit){
    case NET_GENERAL_FAULT_SCRAM:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_SCRAM;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_SCRAM;
        }
        break;
    case NET_GENERAL_FAULT_CARD_READER:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_CARD_READER_COMMUNICATION;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_CARD_READER_COMMUNICATION;
        }
        break;
    case NET_GENERAL_FAULT_DOOR:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_DOOR;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_DOOR;
        }
        break;
    case NET_GENERAL_FAULT_AMMETER:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_AMMETER_COMMUNICATION;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_AMMETER_COMMUNICATION;
        }
        break;
    case NET_GENERAL_FAULT_CHARGE_MODULE:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_RECTIFIER;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_RECTIFIER;
        }
        break;
    case NET_GENERAL_FAULT_OVER_TEMP:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_AIR_OUTLET_OVERTEMP;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_AIR_OUTLET_OVERTEMP;
        }
        break;
    case NET_GENERAL_FAULT_OVER_VOLT:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_OVER_VOLTAGE;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_OVER_VOLTAGE;
        }
        break;
    case NET_GENERAL_FAULT_UNDER_VOLT:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_UNDER_VOLTAGE;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_UNDER_VOLTAGE;
        }
        break;
    case NET_GENERAL_FAULT_OVER_CURR:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_MAIN_RELAY:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_HV_RELAY;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_HV_RELAY;
        }
        break;
    case NET_GENERAL_FAULT_PARALLEL_RELAY:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_AC_RELAY:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_ELOCK:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_AUXPOWER:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_FLASH:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_FLASH;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_FLASH;
        }
        break;
    case NET_GENERAL_FAULT_EEPROM:
        return 0x00;
    case NET_GENERAL_FAULT_LIGHT_PRPTECT:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_AC_LIGHTNING_ARRETER;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_AC_LIGHTNING_ARRETER;
        }
        break;
    case NET_GENERAL_FAULT_GUN_SITE:
        return 0x00;
    case NET_GENERAL_FAULT_CIRCUIT_BREAKER:
        return 0x00;
    case NET_GENERAL_FAULT_FLOODING:
        return 0x00;
    case NET_GENERAL_FAULT_SMOKE:
        return 0x00;
    case NET_GENERAL_FAULT_POUR:
        return 0x00;
    case NET_GENERAL_FAULT_LIQUID_COOLING:
        return 0x00;
    case NET_GENERAL_FAULT_FUSE:
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_DC_FUSE;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_DC_FUSE;
        }
        break;
        break;
    default:
        return 0x00;
        break;
    }
    return 0x01;
}

/*************************************************
 * 函数名      ykc_monitor_fault_detect_report
 * 功能          故障检测并解析上报，由外部实时调用
 * **********************************************/
void ykc_monitor_fault_detect_report(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(s_ykc_monitor_fault_info[gunno].head.count == 0x00){
        return;
    }
    uint32_t event;
    if(ykc_monitor_exist_message_wait_response(gunno, &event) > 0x00){
        if(event &(0x01 <<NET_YKC_MONITOR_PREQ_EVENT_REPORT_REALTIME_DATA)){
            return;
        }
    }
    int32_t result = 0x00;
    uint8_t index = (YKC_MONITOR_FAULT_MSG_NUM_MAX - 0x01);

    rt_enter_critical();

    if((result = ykc_monitor_get_fault_code(s_ykc_monitor_fault_info[gunno].body[index].code, gunno, s_ykc_monitor_fault_info[gunno].body[index].flag.is_resume)) >= 0x00){
        if(result > 0x00){
            ykc_monitor_chargepile_fault_report(gunno, s_ykc_monitor_realtime_fault[gunno]);
        }
        if(s_ykc_monitor_fault_info[gunno].body[index].flag.is_resume){
            ykc_monitor_clear_fault_event(gunno, s_ykc_monitor_fault_info[gunno].body[index].code);
        }
    }

    s_ykc_monitor_fault_info[gunno].body[index].flag.onging = 0x00;
    for(int8_t count = (YKC_MONITOR_FAULT_MSG_NUM_MAX - 0x01); count > 0x00; count--){
        if(s_ykc_monitor_fault_info[gunno].body[count - 0x01].flag.onging == 0x00){
            break;
        }
        memcpy(&(s_ykc_monitor_fault_info[gunno].body[count]), &(s_ykc_monitor_fault_info[gunno].body[count - 0x01]), sizeof(struct ykc_monitor_fault_body));
    }
    s_ykc_monitor_fault_info[gunno].head.count--;

    rt_exit_critical();
}

#endif /* NET_PACK_USING_YKC_MONITOR */
