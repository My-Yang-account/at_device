/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-04-15     Yunhell       the first version
 */
#include <rtthread.h>
#include <stdio.h>
#include "thaisen7102Public.h"
char g_liquidfault[2][9][30];                //液冷故障

#define LiquidFaultPrint(gunno, faultnum, fault)      \
        do{\
            if(faultnum < 9){\
                memset(g_liquidfault[gunno][faultnum], 0, sizeof(g_liquidfault[gunno][faultnum]));\
                sprintf((char*)g_liquidfault[gunno][faultnum], "%d:"fault, faultnum + 1);\
                faultnum ++;\
            }\
        }while(0)

/**
 * @note 获取液冷故障向屏幕发送(编码格式为GBK)
 * @param gunno
 */
void SerialScreen_Liquid_FaultGet(uint8_t gunno)
{
    uint8_t faultnum = 0;
    if(gunno >= thaisenGetLiquidNum())
    {
        LiquidFaultPrint(gunno, faultnum, "无液冷设备");
    }
    else if(thaisenGetLiquidPara(gunno)->offlineflag)
    {
        LiquidFaultPrint(gunno, faultnum, "液冷离线");
    }
    else if(thaisenGetLiquidPara(gunno)->state_flag.fault_code)
    {
        switch(thaisenLiquid_get_LiquidDev())
        {
        case thaisenLiquidDev_YTND:
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.highpressurealarm)
                LiquidFaultPrint(gunno, faultnum, "高压报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.lowpressurealarm)
                LiquidFaultPrint(gunno, faultnum, "低压报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.overheatedgunalarm)
                LiquidFaultPrint(gunno, faultnum, "枪头超温");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.fan1alarm)
                LiquidFaultPrint(gunno, faultnum, "风机1出错");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.fan2alarm)
                LiquidFaultPrint(gunno, faultnum, "风机2出错");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.highliquidalarm)
                LiquidFaultPrint(gunno, faultnum, "高液位报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.lowliquidalarm)
                LiquidFaultPrint(gunno, faultnum, "低液位报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.overflowpumpalarm)
                LiquidFaultPrint(gunno, faultnum, "泵循环过流");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.pumpunderpressurealarm)
                LiquidFaultPrint(gunno, faultnum, "循环泵欠压");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.pumpoverpressurealarm)
                LiquidFaultPrint(gunno, faultnum, "循环泵过压");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.pumpovertempalarm)
                LiquidFaultPrint(gunno, faultnum, "过温报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.blockagepumpalarm)
                LiquidFaultPrint(gunno, faultnum, "循环泵堵转");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.lowflowalarm)
                LiquidFaultPrint(gunno, faultnum, "低流量报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.highflowalarm)
                LiquidFaultPrint(gunno, faultnum, "高流量报警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit.fanoverflowalarm)
                LiquidFaultPrint(gunno, faultnum, "风机过温报警");
            break;
        case thaisenLiquidDev_HL:
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.lowliquiderr)
                LiquidFaultPrint(gunno, faultnum, "液位极低");
            else if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.lowliquidfault)
                LiquidFaultPrint(gunno, faultnum, "液位过低");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.returnfilterclogged)
                LiquidFaultPrint(gunno, faultnum, "回液过滤器堵塞");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.supplyfilterclogged)
                LiquidFaultPrint(gunno, faultnum, "出液过滤器堵塞");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.liquidgunclogged)
                LiquidFaultPrint(gunno, faultnum, "液冷枪堵塞");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.radiatorclogged)
                LiquidFaultPrint(gunno, faultnum, "散热器脏堵");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.returnliquidovertemp)
                LiquidFaultPrint(gunno, faultnum, "回液温度过高");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.ambinetovertemp)
                LiquidFaultPrint(gunno, faultnum, "环境温度过高");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.supplytempsensorfault)
                LiquidFaultPrint(gunno, faultnum, "出液温度传感器故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.returntempsensorfault)
                LiquidFaultPrint(gunno, faultnum, "回液温度传感器故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.supplypressuresensorfault)
                LiquidFaultPrint(gunno, faultnum, "出液压力传感器故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.returnpressuresensorfault)
                LiquidFaultPrint(gunno, faultnum, "回液压力传感器故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.pumpfault)
                LiquidFaultPrint(gunno, faultnum, "水泵故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_HL.fansfault)
                LiquidFaultPrint(gunno, faultnum, "风机故障");
            break;
        case thaisenLiquidDev_TPS:
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.returnliquidovertemp)
                LiquidFaultPrint(gunno, faultnum, "回液温度过高");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.fansfault)
                LiquidFaultPrint(gunno, faultnum, "风扇故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpsupplyoverpressure)
                LiquidFaultPrint(gunno, faultnum, "泵出口压力过高");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.lowliquidfault)
                LiquidFaultPrint(gunno, faultnum, "冷却液液位过低");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.returntempsensorfault)
                LiquidFaultPrint(gunno, faultnum, "回液温度传感器故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.temp_pressuresensorfault)
                LiquidFaultPrint(gunno, faultnum, "温压传感器故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumprunningdry)
                LiquidFaultPrint(gunno, faultnum, "泵空转");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpjam)
                LiquidFaultPrint(gunno, faultnum, "泵堵转");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumppcbovertempwarning)
                LiquidFaultPrint(gunno, faultnum, "泵PCB过温告警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumppcbovertempfault)
                LiquidFaultPrint(gunno, faultnum, "泵PCB过温故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumppcbundertempwarning)
                LiquidFaultPrint(gunno, faultnum, "泵PCB低温告警");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumppcbundertempfault)
                LiquidFaultPrint(gunno, faultnum, "泵PCB低温故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpovervolt)
                LiquidFaultPrint(gunno, faultnum, "泵过电压");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpundervolt)
                LiquidFaultPrint(gunno, faultnum, "泵欠电压");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpovercurr)
                LiquidFaultPrint(gunno, faultnum, "泵过电流");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpoverload)
                LiquidFaultPrint(gunno, faultnum, "泵过载");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpdrivefault)
                LiquidFaultPrint(gunno, faultnum, "泵驱动故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpmcufault)
                LiquidFaultPrint(gunno, faultnum, "泵MCU故障");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.pumpunresponsive)
                LiquidFaultPrint(gunno, faultnum, "泵无响应");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.highliquidfault)
                LiquidFaultPrint(gunno, faultnum, "冷却液液位过高");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.gunAleakage)
                LiquidFaultPrint(gunno, faultnum, "A枪漏液");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.gunBleakage)
                LiquidFaultPrint(gunno, faultnum, "B枪漏液");
            if(thaisenGetLiquidPara(gunno)->state_flag.bit_TPS.powersupplyfault)
                LiquidFaultPrint(gunno, faultnum, "12V供电故障");
            break;
        default:
            break;
        }
    }

    if(faultnum == 0)
    {
        memset(g_liquidfault[gunno][faultnum], 0, sizeof(g_liquidfault[gunno][faultnum]));
        sprintf((char*)g_liquidfault[gunno][faultnum], "液冷无故障");
        faultnum ++;
    }
    for(;faultnum < 9; faultnum ++)
    {
        memset(g_liquidfault[gunno][faultnum], 0, sizeof(g_liquidfault[gunno][faultnum]));
    }
}

