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

#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
/*************************************************** 故障集 1 ***************************************************/
#define YKC_MONITOR_REALTIME_FAULT_SCRAM                         (0x01 <<0)       /* 云快充实时故障：急停按钮动作故障 */
#define YKC_MONITOR_REALTIME_FAULT_CARDREADER                    (0x01 <<1)       /* 云快充实时故障：读卡器故障 */
#define YKC_MONITOR_REALTIME_FAULT_DOOR                          (0x01 <<2)       /* 云快充实时故障：门禁故障 */
#define YKC_MONITOR_REALTIME_FAULT_AMMETER                       (0x01 <<3)       /* 云快充实时故障：电表故障 */
#define YKC_MONITOR_REALTIME_FAULT_CHARGE_MODULE                 (0x01 <<4)       /* 云快充实时故障：充电模块故障 */
#define YKC_MONITOR_REALTIME_FAULT_OVERTEMP                      (0x01 <<5)       /* 云快充实时故障：过温故障 */
#define YKC_MONITOR_REALTIME_FAULT_OVER_VOLT                     (0x01 <<6)       /* 云快充实时故障：过压故障 */
#define YKC_MONITOR_REALTIME_FAULT_UNDER_VOLT                    (0x01 <<7)       /* 云快充实时故障：欠压故障 */
#define YKC_MONITOR_REALTIME_FAULT_OVER_CURR                     (0x01 <<8)       /* 云快充实时故障：过流故障 */
#define YKC_MONITOR_REALTIME_FAULT_DC_RELAY                      (0x01 <<9)       /* 云快充实时故障：直流继电器故障 */
#define YKC_MONITOR_REALTIME_FAULT_PARALLEL_RELAY                (0x01 <<10)      /* 云快充实时故障：母联继电器故障 */
#define YKC_MONITOR_REALTIME_FAULT_AC_RELAY                      (0x01 <<11)      /* 云快充实时故障：交流接触器故障 */
#define YKC_MONITOR_REALTIME_FAULT_ELOCK                         (0x01 <<12)      /* 云快充实时故障：电子锁故障 */
#define YKC_MONITOR_REALTIME_FAULT_AUXPOWER                      (0x01 <<13)      /* 云快充实时故障：辅助电源故障 */
#define YKC_MONITOR_REALTIME_FAULT_FLASH                         (0x01 <<14)      /* 云快充实时故障：FLASH故障 */
#define YKC_MONITOR_REALTIME_FAULT_EEPROM                        (0x01 <<15)      /* 云快充实时故障：EEPROM故障 */
#define YKC_MONITOR_REALTIME_FAULT_LIGHTPROTECT                  (0x01 <<16)      /* 云快充实时故障：防雷器故障 */
#define YKC_MONITOR_REALTIME_FAULT_GUNSITE                       (0x01 <<17)      /* 云快充实时故障：枪座故障 */
#define YKC_MONITOR_REALTIME_FAULT_CIRCUIT_BREAKER               (0x01 <<18)      /* 云快充实时故障：断路器故障 */
#define YKC_MONITOR_REALTIME_FAULT_FLOODING                      (0x01 <<19)      /* 云快充实时故障：水浸故障 */
#define YKC_MONITOR_REALTIME_FAULT_SMOKE                         (0x01 <<20)      /* 云快充实时故障：烟感故障 */
#define YKC_MONITOR_REALTIME_FAULT_POUR                          (0x01 <<21)      /* 云快充实时故障：倾倒故障 */
#define YKC_MONITOR_REALTIME_FAULT_LIQUID_COOLING                (0x01 <<22)      /* 云快充实时故障：液冷故障 */
#define YKC_MONITOR_REALTIME_FAULT_FUSE                          (0x01 <<23)      /* 云快充实时故障：熔断器故障 */
#define YKC_MONITOR_REALTIME_FAULT_MAIN_CABINET                  (0x01 <<24)      /* 云快充实时故障：主机柜故障 */
#define YKC_MONITOR_REALTIME_FAULT_LOCK_DEVICE                   (0x01 <<25)      /* 云快充实时故障：锁桩 */
#define YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_1           (0x01 <<26)      /* 云快充实时故障：矩阵正负接触器KPN1-1 */
#define YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_2           (0x01 <<27)      /* 云快充实时故障：矩阵正负接触器KPN1-2 */
#define YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_3           (0x01 <<28)      /* 云快充实时故障：矩阵正负接触器KPN1-3 */
#define YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN2_1           (0x01 <<29)      /* 云快充实时故障：矩阵正负接触器KPN2-1 */
#define YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN2_2           (0x01 <<30)      /* 云快充实时故障：矩阵正负接触器KPN2-2 */
#define YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN3_1           (0x01 <<31)      /* 云快充实时故障：矩阵正负接触器KPN3-1 */
/*************************************************** 故障集 2 ***************************************************/
#define YKC_MONITOR_REALTIME_FAULT_SLAVE_DEVICE_OFFLINE          (0x01 <<0)       /* 云快充实时故障：从设备离线 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_SCRAM             (0x01 <<1)       /* 云快充实时故障：主机柜急停 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_GATE              (0x01 <<2)       /* 云快充实时故障：主机柜门禁 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_PDUFAULT          (0x01 <<3)       /* 云快充实时故障：主机柜开关板故障 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_MODULEFAULT       (0x01 <<4)       /* 云快充实时故障：主机柜模块 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_CONFIG            (0x01 <<5)       /* 云快充实时故障：主机柜配置项 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_ACRELAY           (0x01 <<6)       /* 云快充实时故障：主机柜交流接触器 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_SMOKE             (0x01 <<7)       /* 云快充实时故障：主机柜烟感报警 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_POUR              (0x01 <<8)       /* 云快充实时故障：主机柜倾倒 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_FLOODING          (0x01 <<9)       /* 云快充实时故障：主机柜水浸 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_OTHER             (0x01 <<10)      /* 云快充实时故障：主机柜其它故障 */
#define YKC_MONITOR_REALTIME_FAULT_MAINCABINET_LIGHT_PROTECT     (0x01 <<11)      /* 云快充实时故障：主机柜防雷故障 */
#define YKC_MONITOR_REALTIME_FAULT_FAN                           (0x01 <<12)       /* 云快充实时故障：风扇 */
#else
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
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */

#pragma pack(1)
struct ykc_monitor_fault_head{
    uint8_t count;
};

struct ykc_monitor_fault_body{
    struct{
        uint8_t is_resume : 4;
        uint8_t onging : 4;
    }flag;
    uint32_t code;
};

struct ykc_monitor_fault_info{
    struct ykc_monitor_fault_head head;
    struct ykc_monitor_fault_body body[YKC_MONITOR_FAULT_MSG_NUM_MAX];
};
#pragma pack()

#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
NET_DEF_SRAM2 static uint32_t s_ykc_monitor_realtime_fault[NET_SYSTEM_GUN_NUMBER][NET_YKC_MONITOR_FAULT_SET_NUM];
#else
NET_DEF_SRAM2 static uint16_t s_ykc_monitor_realtime_fault[NET_SYSTEM_GUN_NUMBER];
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
NET_DEF_SRAM2 static struct ykc_monitor_fault_info s_ykc_monitor_fault_info[NET_SYSTEM_GUN_NUMBER];

#ifdef NET_DESIGNATE_REGION
/*************************************************
 * 函数名      ykc_monitor_fault_info_init
 * 功能          监控平台故障信息、变量初始化
 * **********************************************/
void ykc_monitor_fault_info_init(void)
{
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        memset(&s_ykc_monitor_realtime_fault[gunno], 0x00, sizeof(s_ykc_monitor_realtime_fault[gunno]));
#else
        s_ykc_monitor_realtime_fault[gunno] = 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        memset(&s_ykc_monitor_fault_info[gunno], 0x00, sizeof(s_ykc_monitor_fault_info[gunno]));
    }
}
#endif /* NET_DESIGNATE_REGION */

/*************************************************
 * 函数名      ykc_monitor_fault_event_detect_callback
 * 功能          故障发生变化时调用，用于记录变化的故障
 * **********************************************/
void ykc_monitor_fault_event_detect_callback(uint8_t gunno, uint32_t code, uint8_t is_resume)
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
        s_ykc_monitor_fault_info[gunno].body[index].flag.is_resume = 0x01;
    }else{
        s_ykc_monitor_fault_info[gunno].body[index].flag.is_resume = 0x00;
    }
    s_ykc_monitor_fault_info[gunno].body[index].flag.onging = 0x01;
    s_ykc_monitor_fault_info[gunno].body[index].code = code;
    s_ykc_monitor_fault_info[gunno].head.count++;

    rt_exit_critical();
}

static int32_t ykc_monitor_get_fault_code(uint32_t bit, uint8_t gunno, uint8_t is_resume)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    extern uint16_t ykc_monitor_chargepile_fault_converted(uint16_t bit);
    bit = ykc_monitor_chargepile_fault_converted(bit);

    switch(bit){
    /** 急停故障 */
    case NET_GENERAL_FAULT_SCRAM:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_SCRAM;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_SCRAM;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_SCRAM;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_SCRAM;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
     /** 读卡器故障 */
    case NET_GENERAL_FAULT_CARD_READER:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_CARDREADER;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_CARDREADER;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_CARD_READER_COMMUNICATION;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_CARD_READER_COMMUNICATION;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 门禁故障 */
    case NET_GENERAL_FAULT_DOOR:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_DOOR;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_DOOR;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_DOOR;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_DOOR;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 电表故障 */
    case NET_GENERAL_FAULT_AMMETER:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_AMMETER;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_AMMETER;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_AMMETER_COMMUNICATION;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_AMMETER_COMMUNICATION;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 充电模块故障 */
    case NET_GENERAL_FAULT_CHARGE_MODULE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_CHARGE_MODULE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_CHARGE_MODULE;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_RECTIFIER;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_RECTIFIER;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 过温故障 */
    case NET_GENERAL_FAULT_OVER_TEMP:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_OVERTEMP;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_OVERTEMP;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_AIR_OUTLET_OVERTEMP;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_AIR_OUTLET_OVERTEMP;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 过压故障 */
    case NET_GENERAL_FAULT_OVER_VOLT:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_OVER_VOLT;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_OVER_VOLT;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_OVER_VOLTAGE;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_OVER_VOLTAGE;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 欠压故障 */
    case NET_GENERAL_FAULT_UNDER_VOLT:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_UNDER_VOLT;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_UNDER_VOLT;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_UNDER_VOLTAGE;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_UNDER_VOLTAGE;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 过流故障 */
    case NET_GENERAL_FAULT_OVER_CURR:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_OVER_CURR;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_OVER_CURR;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 直流继电器故障 */
    case NET_GENERAL_FAULT_MAIN_RELAY:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_DC_RELAY;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_DC_RELAY;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_HV_RELAY;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_HV_RELAY;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 母联继电器故障 */
    case NET_GENERAL_FAULT_PARALLEL_RELAY:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_PARALLEL_RELAY;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_PARALLEL_RELAY;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 交流接触器故障 */
    case NET_GENERAL_FAULT_AC_RELAY:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_AC_RELAY;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_AC_RELAY;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 电子锁故障 */
    case NET_GENERAL_FAULT_ELOCK:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_ELOCK;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_ELOCK;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 辅助电源故障 */
    case NET_GENERAL_FAULT_AUXPOWER:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_AUXPOWER;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_AUXPOWER;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** FLASH故障 */
    case NET_GENERAL_FAULT_FLASH:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_FLASH;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_FLASH;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_FLASH;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_FLASH;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** EEPROM故障 */
    case NET_GENERAL_FAULT_EEPROM:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_EEPROM;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_EEPROM;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 防雷器故障 */
    case NET_GENERAL_FAULT_LIGHT_PRPTECT:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_LIGHTPROTECT;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_LIGHTPROTECT;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_AC_LIGHTNING_ARRETER;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_AC_LIGHTNING_ARRETER;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 枪座故障 */
    case NET_GENERAL_FAULT_GUN_SITE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_GUNSITE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_GUNSITE;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 断路器故障 */
    case NET_GENERAL_FAULT_CIRCUIT_BREAKER:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_CIRCUIT_BREAKER;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_CIRCUIT_BREAKER;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 水浸故障 */
    case NET_GENERAL_FAULT_FLOODING:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_FLOODING;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_FLOODING;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 烟感故障 */
    case NET_GENERAL_FAULT_SMOKE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_SMOKE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_SMOKE;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 倾倒故障 */
    case NET_GENERAL_FAULT_POUR:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_POUR;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_POUR;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 液冷故障 */
    case NET_GENERAL_FAULT_LIQUID_COOLING:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_LIQUID_COOLING;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_LIQUID_COOLING;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 熔断器故障 */
    case NET_GENERAL_FAULT_FUSE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_FUSE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_FUSE;
        }
#else
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno] &= ~YKC_MONITOR_REALTIME_FAULT_DC_FUSE;
        }else{
            s_ykc_monitor_realtime_fault[gunno] |= YKC_MONITOR_REALTIME_FAULT_DC_FUSE;
        }
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        break;
    /** 主机柜故障 */
    case NET_GENERAL_FAULT_MAIN_CABINET_OFFLINE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MAIN_CABINET;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MAIN_CABINET;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 设备已锁定 */
    case NET_GENERAL_FAULT_DEVICE_IS_LOCKED:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_LOCK_DEVICE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_LOCK_DEVICE;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 矩阵继电器KPN1-1故障 */
    case NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_1:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_1;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_1;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 矩阵继电器KPN1-2故障 */
    case NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_2:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_2;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_2;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 矩阵继电器KPN1-3故障 */
    case NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_3:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_3;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN1_3;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 矩阵继电器KPN2-1故障 */
    case NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_1:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN2_1;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN2_1;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 矩阵继电器KPN2-2故障 */
    case NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_2:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN2_2;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN2_2;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 矩阵继电器KPN3-1故障 */
    case NET_GENERAL_FAULT_MATRIX_RELAY_KPN3_1:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] &= ~YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN3_1;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_1] |= YKC_MONITOR_REALTIME_FAULT_MATRIX_RELAY_KPN3_1;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 从设备离线故障 */
    case NET_GENERAL_FAULT_SLAVE_DEVICE_OFFLINE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_SLAVE_DEVICE_OFFLINE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_SLAVE_DEVICE_OFFLINE;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 风扇故障 */
    case NET_GENERAL_FAULT_FAN:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_FAN;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_FAN;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜急停故障 */
    case NET_GENERAL_FAULT_MAINCABINET_SCRAM:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_SCRAM;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_SCRAM;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜门禁故障 */
    case NET_GENERAL_FAULT_MAINCABINET_GATE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_GATE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_GATE;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜PDU故障 */
    case NET_GENERAL_FAULT_MAINCABINET_PDUFAULT:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_PDUFAULT;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_PDUFAULT;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜模块故障 */
    case NET_GENERAL_FAULT_MAINCABINET_MODULEFAULT:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_MODULEFAULT;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_MODULEFAULT;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜配置故障 */
    case NET_GENERAL_FAULT_MAINCABINET_CONFIG:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_CONFIG;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_CONFIG;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜交流接触器故障 */
    case NET_GENERAL_FAULT_MAINCABINET_ACRELAY:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_ACRELAY;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_ACRELAY;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜烟感故障 */
    case NET_GENERAL_FAULT_MAINCABINET_SMOKE:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_SMOKE;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_SMOKE;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜倾倒故障 */
    case NET_GENERAL_FAULT_MAINCABINET_POUR:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_POUR;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_POUR;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜水浸故障 */
    case NET_GENERAL_FAULT_MAINCABINET_FLOODING:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_FLOODING;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_FLOODING;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜其它故障 */
    case NET_GENERAL_FAULT_MAINCABINET_OTHER:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_OTHER;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_OTHER;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
    /** 主机柜防雷故障 */
    case NET_GENERAL_FAULT_MAINCABINET_LIGHT_PROTECT:
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
        if(is_resume){
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] &= ~YKC_MONITOR_REALTIME_FAULT_MAINCABINET_LIGHT_PROTECT;
        }else{
            s_ykc_monitor_realtime_fault[gunno][NET_YKC_MONITOR_FAULT_SET_2] |= YKC_MONITOR_REALTIME_FAULT_MAINCABINET_LIGHT_PROTECT;
        }
        break;
#else
        return 0x00;
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
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
#ifdef NET_YKC_MONITOR_FAULT_USING_EXTEND
            extern void ykc_monitor_chargepile_fault_report(uint8_t gunno, uint32_t *code);
            ykc_monitor_chargepile_fault_report(gunno, s_ykc_monitor_realtime_fault[gunno]);
#else
            extern void ykc_monitor_chargepile_fault_report(uint8_t gunno, uint16_t code);
            ykc_monitor_chargepile_fault_report(gunno, s_ykc_monitor_realtime_fault[gunno]);
#endif /* NET_YKC_MONITOR_FAULT_USING_EXTEND */
        }
    }

    s_ykc_monitor_fault_info[gunno].body[index].flag.onging = 0x00;
    for(int8_t count = (YKC_MONITOR_FAULT_MSG_NUM_MAX - 0x01); count > 0x00; count--){
        if(s_ykc_monitor_fault_info[gunno].body[count - 0x01].flag.onging == 0x00){
            break;
        }
        memcpy(&(s_ykc_monitor_fault_info[gunno].body[count]), &(s_ykc_monitor_fault_info[gunno].body[count - 0x01]), sizeof(struct ykc_monitor_fault_body));
    }

    if(s_ykc_monitor_fault_info[gunno].head.count > 0x00){
        s_ykc_monitor_fault_info[gunno].head.count--;
    }

    rt_exit_critical();
}

#endif /* NET_PACK_USING_YKC_MONITOR */
