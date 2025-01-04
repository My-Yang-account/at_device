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
#include "sgcc_device_register.h"

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
    uint32_t code;
};

struct sgcc_fault_info{
    struct sgcc_fault_head head;
    struct sgcc_fault_body body[SGCC_FAULT_MSG_NUM_MAX];
};
#pragma pack()

NET_DEF_SRAM2 static uint16_t s_sgcc_realtime_fault[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct sgcc_fault_info s_sgcc_fault_info[NET_SYSTEM_GUN_NUMBER];

#ifdef NET_DESIGNATE_REGION
/*************************************************
 * 函数名      sgcc_fault_info_init
 * 功能          国网平台故障信息、变量初始化
 * **********************************************/
void sgcc_fault_info_init(void)
{
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_sgcc_realtime_fault[gunno] = 0x00;
        memset(&s_sgcc_fault_info[gunno], 0x00, sizeof(s_sgcc_fault_info[gunno]));
    }
}
#endif /* NET_DESIGNATE_REGION */

/*************************************************
 * 函数名      sgcc_fault_event_detect_callback
 * 功能          故障发生变化时调用，用于记录变化的故障
 * **********************************************/
void sgcc_fault_event_detect_callback(uint8_t gunno, uint32_t code, uint8_t is_resume)
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
        s_sgcc_fault_info[gunno].body[index].flag.is_resume = 0x01;
    }else{
        s_sgcc_fault_info[gunno].body[index].flag.is_resume = 0x00;
    }
    s_sgcc_fault_info[gunno].body[index].flag.onging = 0x01;
    s_sgcc_fault_info[gunno].body[index].code = code;
    s_sgcc_fault_info[gunno].head.count++;

    rt_exit_critical();
}

#ifdef NET_SGCC_PRO_USING_DC
static int32_t sgcc_get_fault_code(uint32_t bit, uint8_t gunno, uint8_t *rank)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    uint16_t fault = 0x00;
    extern uint16_t sgcc_chargepile_fault_converted(uint16_t bit, uint8_t *rank);
    fault = sgcc_chargepile_fault_converted(bit, rank);
    if(fault){
        s_sgcc_realtime_fault[gunno] = fault;
        return 0x01;
    }

    return 0x00;
}
#else
static int32_t sgcc_get_fault_code(uint32_t bit, uint8_t gunno, uint8_t *rank)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    uint16_t fault = 0x00;
    extern uint16_t sgcc_chargepile_fault_converted(uint16_t bit, uint8_t *rank);
    fault = sgcc_chargepile_fault_converted(bit, rank);
    if(fault){
        s_sgcc_realtime_fault[gunno] = fault;
        return 0x01;
    }

    return 0x00;
}
#endif /* NET_SGCC_PRO_USING_DC */

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
