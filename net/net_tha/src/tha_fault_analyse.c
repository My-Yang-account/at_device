/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-15     我的杨yang       the first version
 */
#include "tha_fault_analyse.h"
#include "tha_message_send.h"

#ifdef NET_PACK_USING_THA

#define THA_FAULT_MSG_NUM_MAX          0x05

#pragma pack(1)
struct tha_fault_head{
    uint8_t count;
};

struct tha_fault_body{
    struct{
        uint8_t is_resume : 4;
        uint8_t onging : 4;
    }flag;
    uint8_t code;
    uint32_t timestamp;
};

struct tha_fault_info{
    struct tha_fault_head head;
    struct tha_fault_body body[THA_FAULT_MSG_NUM_MAX];
};
#pragma pack()

static uint32_t s_tha_current_fault_set[NET_SYSTEM_GUN_NUMBER];
static struct tha_fault_info s_tha_fault_info[NET_SYSTEM_GUN_NUMBER];

/*************************************************
 * 函数名      tha_fault_event_detect_callback
 * 功能          故障发生变化时调用，用于记录变化的故障
 * **********************************************/
void tha_fault_event_detect_callback(uint8_t gunno, uint8_t code, uint32_t timestamp, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t index = 0x00;
    if(s_tha_fault_info[gunno].head.count >= THA_FAULT_MSG_NUM_MAX){
        return;
    }

    rt_enter_critical();

    index = (THA_FAULT_MSG_NUM_MAX - 0x01 - s_tha_fault_info[gunno].head.count);

    if(is_resume){
        s_tha_current_fault_set[gunno] &= (~(0x01 <<code));
        s_tha_fault_info[gunno].body[index].flag.is_resume = 0x01;
    }else{
        s_tha_current_fault_set[gunno] |= (0x01 <<code);
        s_tha_fault_info[gunno].body[index].flag.is_resume = 0x00;
    }
    s_tha_fault_info[gunno].body[index].flag.onging = 0x00;
    s_tha_fault_info[gunno].body[index].code = code;
    s_tha_fault_info[gunno].body[index].timestamp = timestamp;
    s_tha_fault_info[gunno].head.count++;

    rt_exit_critical();
}

/*************************************************
 * 函数名      tha_get_current_fault_set
 * 功能          获取当前故障集
 * **********************************************/
uint32_t tha_get_current_fault_set(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_tha_current_fault_set[gunno];
}

static void tha_clear_fault_event(uint8_t gunno, uint8_t code)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    s_tha_current_fault_set[gunno] &= (~(0x01 <<code));
}

static void tha_get_fault_string(uint8_t code, char *str, uint8_t *rank)
{
    code = tha_chargepile_fault_converted(code);
    switch(code){
    case NET_GENERAL_FAULT_SCRAM:
        memcpy(str, "101", strlen("101"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_CARD_READER:
        memcpy(str, "316", strlen("316"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_DOOR:
        memcpy(str, "102", strlen("102"));
        if(rank){
            *rank = 0xFD;
        }
        break;
    case NET_GENERAL_FAULT_AMMETER:
        memcpy(str, "230", strlen("230"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_CHARGE_MODULE:
        memcpy(str, "208", strlen("208"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_OVER_TEMP:
        memcpy(str, "234", strlen("234"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_OVER_VOLT:
        memcpy(str, "104", strlen("104"));
        if(rank){
            *rank = 0xFD;
        }
        break;
    case NET_GENERAL_FAULT_UNDER_VOLT:
        memcpy(str, "231", strlen("231"));
        if(rank){
            *rank = 0xFD;
        }
        break;
    case NET_GENERAL_FAULT_OVER_CURR:
        memcpy(str, "106", strlen("106"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_MAIN_RELAY:
        memcpy(str, "224", strlen("224"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_PARALLEL_RELAY:
        memcpy(str, "F00", strlen("F00"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_AC_RELAY:
        memcpy(str, "F01", strlen("F01"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_ELOCK:
        memcpy(str, "229", strlen("229"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_AUXPOWER:
        memcpy(str, "F02", strlen("F02"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_FLASH:
        memcpy(str, "F03", strlen("F03"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    case NET_GENERAL_FAULT_EEPROM:
        memcpy(str, "F04", strlen("F04"));
        if(rank){
            *rank = 0xFE;
        }
        break;
    default:
        break;
    }
}

/*************************************************
 * 函数名      net_fault_detect_report
 * 功能          故障检测并解析上报，由外部实时调用
 * **********************************************/
void tha_fault_detect_report(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(s_tha_fault_info[gunno].head.count == 0x00){
        return;
    }
    uint32_t event;
    if(tha_exist_message_wait_response(gunno, &event) > 0x00){
        if(event &(0x01 <<NET_THA_PREQ_EVENT_ERROR_INFO)){
            return;
        }
    }
    char code_str[3];
    uint8_t rank = 0x00, index = (THA_FAULT_MSG_NUM_MAX - 0x01);

    rt_enter_critical();

    memset(code_str, '\0', sizeof(code_str));
    tha_get_fault_string(s_tha_fault_info[gunno].body[index].code, code_str, &rank);
    if(strlen(code_str) != 0x00){
        tha_chargepile_fault_report(gunno, code_str, rank, s_tha_fault_info[gunno].body[index].timestamp, s_tha_fault_info[gunno].body[index].flag.is_resume);
    }
    if(s_tha_fault_info[gunno].body[index].flag.is_resume){
        tha_clear_fault_event(gunno, s_tha_fault_info[gunno].body[index].code);
    }

    s_tha_fault_info[gunno].body[index].flag.onging = 0x00;
    for(int8_t count = (THA_FAULT_MSG_NUM_MAX - 0x01); count > 0x00; count--){
        if(s_tha_fault_info[gunno].body[count - 0x01].flag.onging == 0x00){
            break;
        }
        memcpy(&(s_tha_fault_info[gunno].body[count]), &(s_tha_fault_info[gunno].body[count - 0x01]), sizeof(struct tha_fault_body));
    }
    s_tha_fault_info[gunno].head.count--;

    rt_exit_critical();
}

#endif /* NET_PACK_USING_THA */
