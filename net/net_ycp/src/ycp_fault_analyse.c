/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#include "ycp_fault_analyse.h"
#include "ycp_message_send.h"

#ifdef NET_PACK_USING_YCP

#define YCP_FAULT_MSG_NUM_MAX                            0x05

#pragma pack(1)
struct ycp_fault_head{
    uint8_t count;
};

struct ycp_fault_body{
    struct{
        uint8_t is_resume : 4;
        uint8_t onging : 4;
    }flag;
    uint32_t code;
};

struct ycp_fault_info{
    struct ycp_fault_head head;
    struct ycp_fault_body body[YCP_FAULT_MSG_NUM_MAX];
};
#pragma pack()

static uint16_t s_ycp_realtime_fault[NET_SYSTEM_GUN_NUMBER];
static struct ycp_fault_info s_ycp_fault_info[NET_SYSTEM_GUN_NUMBER];

/*************************************************
 * 函数名      ycp_fault_event_detect_callback
 * 功能          故障发生变化时调用，用于记录变化的故障
 * **********************************************/
void ycp_fault_event_detect_callback(uint8_t gunno, uint32_t code, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t index = 0x00;
    if(s_ycp_fault_info[gunno].head.count >= YCP_FAULT_MSG_NUM_MAX){
        return;
    }

    rt_enter_critical();

    index = (YCP_FAULT_MSG_NUM_MAX - 0x01 - s_ycp_fault_info[gunno].head.count);

    if(is_resume){
        s_ycp_fault_info[gunno].body[index].flag.is_resume = 0x01;
    }else{
        s_ycp_fault_info[gunno].body[index].flag.is_resume = 0x00;
    }
    s_ycp_fault_info[gunno].body[index].flag.onging = 0x01;
    s_ycp_fault_info[gunno].body[index].code = code;
    s_ycp_fault_info[gunno].head.count++;

    rt_exit_critical();
}

static int32_t ycp_get_fault_code(uint32_t bit, uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    extern uint16_t ycp_chargepile_fault_converted(uint16_t bit);
    bit = ycp_chargepile_fault_converted(bit);

    switch(bit){
    case NET_GENERAL_FAULT_SCRAM:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_EMERGENCY;
        break;
    case NET_GENERAL_FAULT_CARD_READER:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_CARD_READER;
        break;
    case NET_GENERAL_FAULT_DOOR:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_GATE;
        break;
    case NET_GENERAL_FAULT_AMMETER:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_AMMETER;
        break;
    case NET_GENERAL_FAULT_CHARGE_MODULE:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_COMMUNICATION_MODULE;
        break;
    case NET_GENERAL_FAULT_OVER_TEMP:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_GUN_OVERTEMP;
        break;
    case NET_GENERAL_FAULT_OVER_VOLT:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_INPUT_OVERVOLT;
        break;
    case NET_GENERAL_FAULT_UNDER_VOLT:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_INPUT_UNDERVOLT;
        break;
    case NET_GENERAL_FAULT_OVER_CURR:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_OVERCURR;
        break;
    case NET_GENERAL_FAULT_MAIN_RELAY:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_RELAY_ADH;
        break;
    case NET_GENERAL_FAULT_PARALLEL_RELAY:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_RELAY_ADH;
        break;
    case NET_GENERAL_FAULT_AC_RELAY:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_AC_RELAY;
        break;
    case NET_GENERAL_FAULT_ELOCK:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_AUXPOWER:
        return 0x00;
        break;
    case NET_GENERAL_FAULT_FLASH:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_STORAGE_CHIP;
        break;
    case NET_GENERAL_FAULT_EEPROM:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_STORAGE_CHIP;
        break;
    case NET_GENERAL_FAULT_LIGHT_PRPTECT:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_LIGHTNING_PROTECTOR;
        break;
    case NET_GENERAL_FAULT_GUN_SITE:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_GUN_SITE;
        break;
    case NET_GENERAL_FAULT_CIRCUIT_BREAKER:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_CIRCUIT_BREAKER;
        break;
    case NET_GENERAL_FAULT_FLOODING:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_FLOODING;
        break;
    case NET_GENERAL_FAULT_SMOKE:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_FUMES;
        break;
    case NET_GENERAL_FAULT_POUR:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_POUR;
        break;
    case NET_GENERAL_FAULT_LIQUID_COOLING:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_LIQUID_COOLING;
        break;
    case NET_GENERAL_FAULT_FUSE:
        s_ycp_realtime_fault[gunno] = NET_YCP_FAULT_CODE_FUSE;
        break;
    default:
        return 0x00;
        break;
    }
    return 0x01;
}

/*************************************************
 * 函数名      ycp_fault_detect_report
 * 功能          故障检测并解析上报，由外部实时调用
 * **********************************************/
void ycp_fault_detect_report(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(s_ycp_fault_info[gunno].head.count == 0x00){
        return;
    }
    uint32_t event;
    if(ycp_exist_message_wait_response(gunno, &event) > 0x00){
        if(event &(0x01 <<NET_YCP_PREQ_EVENT_REPORT_DEVICE_FAULT)){
            return;
        }
    }
    int32_t result = 0x00;
    uint8_t index = (YCP_FAULT_MSG_NUM_MAX - 0x01);

    rt_enter_critical();

    if((result = ycp_get_fault_code(s_ycp_fault_info[gunno].body[index].code, gunno)) >= 0x00){
        if(result > 0x00){
            extern int8_t  ycp_chargepile_fault_report(uint8_t gunno, uint16_t code, uint8_t is_resume);
            if(ycp_chargepile_fault_report(gunno, s_ycp_realtime_fault[gunno], s_ycp_fault_info[gunno].body[index].flag.is_resume) >= 0x00){

                s_ycp_fault_info[gunno].body[index].flag.onging = 0x00;
                for(int8_t count = (YCP_FAULT_MSG_NUM_MAX - 0x01); count > 0x00; count--){
                    if(s_ycp_fault_info[gunno].body[count - 0x01].flag.onging == 0x00){
                        break;
                    }
                    memcpy(&(s_ycp_fault_info[gunno].body[count]), &(s_ycp_fault_info[gunno].body[count - 0x01]), sizeof(struct ycp_fault_body));
                }
                if(s_ycp_fault_info[gunno].head.count > 0x00){
                    s_ycp_fault_info[gunno].head.count--;
                }
            }
        }
    }

    rt_exit_critical();
}

#endif /* NET_PACK_USING_YCP */
