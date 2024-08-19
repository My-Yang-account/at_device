/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-25     leven       the first version
 */

#include "sgcc_fault_analyse.h"
#include "sgcc_message_send.h"

#ifdef NET_PACK_USING_SGCC

#define SGCC_FAULT_MSG_NUM_MAX                            0x05

#pragma pack(1)
struct sgcc_fault_head{
    uint8_t count;
};

struct sgcc_fault_body{
    struct{
        uint8_t is_resume : 4;
        uint8_t onging : 4;
    }flag;
    uint8_t code;
};

struct sgcc_fault_info{
    struct sgcc_fault_head head;
    struct sgcc_fault_body body[SGCC_FAULT_MSG_NUM_MAX];
};
#pragma pack()

static uint32_t s_sgcc_current_fault_set[NET_SYSTEM_GUN_NUMBER];
static uint16_t s_sgcc_realtime_fault[NET_SYSTEM_GUN_NUMBER];
static struct sgcc_fault_info s_sgcc_fault_info[NET_SYSTEM_GUN_NUMBER];

/*************************************************
 * 函数名      sgcc_fault_event_detect_callback
 * 功能          故障发生变化时调用，用于记录变化的故障
 * **********************************************/
void sgcc_fault_event_detect_callback(uint8_t gunno, uint8_t code, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t index = 0x00;
    if(s_sgcc_fault_info[gunno].head.count >= SGCC_FAULT_MSG_NUM_MAX){
        return;
    }

    rt_enter_critical();

    index = (SGCC_FAULT_MSG_NUM_MAX - 0x01 - s_sgcc_fault_info[gunno].head.count);

    if(is_resume){
        s_sgcc_current_fault_set[gunno] &= (~(0x01 <<code));
        s_sgcc_fault_info[gunno].body[index].flag.is_resume = 0x01;
    }else{
        s_sgcc_current_fault_set[gunno] |= (0x01 <<code);
        s_sgcc_fault_info[gunno].body[index].flag.is_resume = 0x00;
    }
    s_sgcc_fault_info[gunno].body[index].flag.onging = 0x01;
    s_sgcc_fault_info[gunno].body[index].code = code;
    s_sgcc_fault_info[gunno].head.count++;

    rt_exit_critical();
}

/*************************************************
 * 函数名      sgcc_get_current_fault_set
 * 功能          获取当前故障集
 * **********************************************/
uint32_t sgcc_get_current_fault_set(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_sgcc_current_fault_set[gunno];
}

static void sgcc_clear_fault_event(uint8_t gunno, uint8_t code)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_sgcc_current_fault_set[gunno] &= (~(0x01 <<code));
}

static int32_t sgcc_get_fault_code(uint8_t bit, uint8_t gunno, uint8_t *rank)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    extern uint16_t sgcc_chargepile_fault_converted(uint16_t bit);
    bit = sgcc_chargepile_fault_converted(bit);

    switch(bit){
    case NET_GENERAL_FAULT_SCRAM:
        s_sgcc_realtime_fault[gunno] = 3033;
        break;
    case NET_GENERAL_FAULT_CARD_READER:
        s_sgcc_realtime_fault[gunno] = 3037;
        break;
    case NET_GENERAL_FAULT_DOOR:
        s_sgcc_realtime_fault[gunno] = 3032;
        break;
    case NET_GENERAL_FAULT_AMMETER:
        s_sgcc_realtime_fault[gunno] = 3043;
        break;
    case NET_GENERAL_FAULT_CHARGE_MODULE:
        s_sgcc_realtime_fault[gunno] = 3038;
        break;
    case NET_GENERAL_FAULT_OVER_TEMP:
        s_sgcc_realtime_fault[gunno] = 3053;
        break;
    case NET_GENERAL_FAULT_OVER_VOLT:
        s_sgcc_realtime_fault[gunno] = 4009;
        break;
    case NET_GENERAL_FAULT_UNDER_VOLT:
        s_sgcc_realtime_fault[gunno] = 4011;
        break;
    case NET_GENERAL_FAULT_OVER_CURR:
        s_sgcc_realtime_fault[gunno] = 4010;
        break;
    case NET_GENERAL_FAULT_MAIN_RELAY:
        s_sgcc_realtime_fault[gunno] = 3046;
        break;
    case NET_GENERAL_FAULT_PARALLEL_RELAY:
        s_sgcc_realtime_fault[gunno] = 3068;
        break;
    case NET_GENERAL_FAULT_AC_RELAY:
        s_sgcc_realtime_fault[gunno] = 3065;
        break;
    case NET_GENERAL_FAULT_ELOCK:
        s_sgcc_realtime_fault[gunno] = 3054;
        break;
    case NET_GENERAL_FAULT_AUXPOWER:
        s_sgcc_realtime_fault[gunno] = 3049;
        break;
    case NET_GENERAL_FAULT_FLASH:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_EEPROM:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_LIGHT_PRPTECT:
        s_sgcc_realtime_fault[gunno] = 3084;
        break;
    case NET_GENERAL_FAULT_GUN_SITE:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_CIRCUIT_BREAKER:
        s_sgcc_realtime_fault[gunno] = 4013;
        break;
    case NET_GENERAL_FAULT_FLOODING:
        s_sgcc_realtime_fault[gunno] = 3055;
        break;
    case NET_GENERAL_FAULT_SMOKE:
        s_sgcc_realtime_fault[gunno] = 3085;
        break;
    case NET_GENERAL_FAULT_POUR:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_LIQUID_COOLING:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_FUSE:
        s_sgcc_realtime_fault[gunno] = 3047;
        break;
    default:
        return 0x00;
        break;
    }
    return 0x01;
}

/*************************************************
 * 函数名      sgcc_fault_detect_report
 * 功能          故障检测并解析上报，由外部实时调用
 * **********************************************/
void sgcc_fault_detect_report(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(s_sgcc_fault_info[gunno].head.count == 0x00){
        return;
    }
    uint32_t event;
    if(sgcc_exist_message_wait_response(gunno, &event) > 0x00){
        if(event &(0x01 <<NET_SGCC_PREQ_EVENT_REPORT_FAULT_WARNNING)){
            return;
        }
    }
    int32_t result = 0x00;
    uint8_t index = (SGCC_FAULT_MSG_NUM_MAX - 0x01), rank = 0x00;

    rt_enter_critical();

    if((result = sgcc_get_fault_code(s_sgcc_fault_info[gunno].body[index].code, gunno, &rank)) >= 0x00){
        if(result > 0x00){
            extern int8_t  sgcc_chargepile_fault_report(uint8_t gunno, uint16_t code, uint8_t is_resume, uint8_t rank);
            if(sgcc_chargepile_fault_report(gunno, s_sgcc_realtime_fault[gunno], s_sgcc_fault_info[gunno].body[index].flag.is_resume, rank) >= 0x00){
                if(s_sgcc_fault_info[gunno].body[index].flag.is_resume){
                    sgcc_clear_fault_event(gunno, s_sgcc_fault_info[gunno].body[index].code);
                }

                s_sgcc_fault_info[gunno].body[index].flag.onging = 0x00;
                for(int8_t count = (SGCC_FAULT_MSG_NUM_MAX - 0x01); count > 0x00; count--){
                    if(s_sgcc_fault_info[gunno].body[count - 0x01].flag.onging == 0x00){
                        break;
                    }
                    memcpy(&(s_sgcc_fault_info[gunno].body[count]), &(s_sgcc_fault_info[gunno].body[count - 0x01]), sizeof(struct sgcc_fault_body));
                }
                if(s_sgcc_fault_info[gunno].head.count > 0x00){
                    s_sgcc_fault_info[gunno].head.count--;
                }
            }
        }
    }

    rt_exit_critical();
}

#endif /* NET_PACK_USING_SGCC */
