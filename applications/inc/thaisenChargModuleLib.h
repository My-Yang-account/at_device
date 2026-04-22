/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-27     Lenovo       the first version
 */
#ifndef APPLICATIONS_THAISENCHARGMODULELIB_H_
#define APPLICATIONS_THAISENCHARGMODULELIB_H_

#include "stdio.h"

/* 通用模块故障 */
typedef struct
{
    union{
        struct{
            uint8_t InputOverVolt          : 1;       /* 输入过压 */
            uint8_t InputUnderVolt         : 1;       /* 输入欠压 */
            uint8_t OutputOverVolt         : 1;       /* 输出过压 */
            uint8_t OutputUnderVolt        : 1;       /* 输出欠压 */
            uint8_t SameId                 : 1;       /* 有相同ID的多个模块 */
            uint8_t ModuleFault            : 1;       /* 模块故障 */
            uint8_t OverCurr               : 1;       /* 过流 */
            uint8_t OverTemp               : 1;       /* 过温 */
        }bit;
        uint8_t fault_val;
    }fault;   /* 故障 */
    union{
        struct{
            uint8_t Fan                    : 1;       /* 风扇 */
        }bit;
        uint8_t warn_val;
    }warn;   /* 警告 */
    union{
        struct{
            uint8_t BootState              : 1;       /* 开机状态(1:开机) */
        }bit;
        uint8_t state_val;
    }state;   /* 状态 */
    int16_t temperature;
    uint8_t addr;                                    /* 模块地址 */
}thaisenModuleGeneralFaultStruct;

/* 模块当前故障信息 */
typedef struct
{
    uint8_t addr;                              /* 模块地址 */
    thaisenModuleGeneralFaultStruct state;     /* 模块状态信息 */
}thaisenModuleFaultInfoStruct;

/* 模块当前电压、电流信息 */
typedef struct
{
    uint8_t addr;                          /* 模块地址 */
    uint16_t voltage;                      /* 模块当前电压 */
    uint16_t current;                      /* 模块当前电流 */
}thaisenModuleVoltCurrStruct;

/* 模块故障集 */
typedef struct
{
    uint32_t main_fault;                   /* 主故障集 */
    uint32_t sub_fault;                    /* 子故障集 */
}thaisenModuleFaultSetStruct;

typedef struct thasienModuleSetStruct
{
    uint8_t moduleProNo;//模块类型
    uint8_t moduleGroupNum;//模块组数
    uint8_t moduleSingleGroupNum[4];//模块组内个数
}thasienModuleSetStruct;


typedef enum thaisenChargModuleEnum
{
    thaisenChargModuleOnLine = 0,                           //充电模块在线
    thaisenChargModuleOffLine = !thaisenChargModuleOnLine,  //充电模块离线
}thaisenChargModuleEn;

typedef uint16_t (*thaisenModuleGetStatusFun_p)(uint8_t);

/* 功能说明:
 *          thaisen_chargModule_Init:充电模块系统初始化
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */

void thaisen_chargModule_Init(thaisenModuleGetStatusFun_p getStaFun,struct thaisenBMS_Charger_struct *chargerA,struct thaisenBMS_Charger_struct *chargerB,struct thasienModuleSetStruct *moduleS );

enum thaisenModuleStateEnum
{
    thaisenModuleStateIdle,
    thaisenModuleStateChargInsult,
    thaisenModuleStateChargCRO,
    thaisenModuleStateChargCCS,
    thaisenModuleStateChargStop,
    thaisenModuleStateChargParallel,
    thaisenModuleStateChargParallelDischarg,
    thaisenModuleStateFault,
    thaisenModuleStateMiddle,
    thaisenModuleStateAll,
};
/* 功能说明:
 *          thaisenModuleGetStatus:获取模块部分运行状态
 *
 * 输入参数:  gunNum   枪号
 *
 * 返回参数:
 *          模块部分运行状态@enum thaisenModuleStateEnum
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetStatus(uint8_t gunNum);

/* 功能说明:
 *          thaisenSetModuleMaxVolt:设置模块最高输出电压
 *
 * 输入参数:
 *      uint16_t:电压值  0.1v
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMaxVolt(uint16_t volt);


/* 功能说明:
 *          thaisenGetModuleMaxVolt:获取模块最高输出电压
 *
 * 输入参数:
 *
 * 返回参数:
 *      uint16_t:电压值  0.1v
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMaxVolt(void);

/* 功能说明:
 *          thaisenSetModuleMinVolt:设置模块最小输出电压
 *
 * 输入参数:
 *      uint16_t:电压值  0.1v
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMinVolt(uint16_t volt);

/* 功能说明:
 *          thaisenGetModuleMinVolt:获取模块最小输出电压
 *
 * 输入参数:
 *
 * 返回参数:
 *          uint16_t:电压值  0.1v
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMinVolt(void);



/* 功能说明:
 *          thaisenSetModuleMaxCurr:设置模块最大输出电流
 *
 * 输入参数:
 *      uint16_t:电压值  0.1A
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMaxCurr(uint16_t curr);


/* 功能说明:
 *          thaisenGetModuleMaxCurr:获取模块最大输出电流
 *
 * 输入参数:
 *
 * 返回参数:
 *          uint16_t:电压值  0.1A
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMaxCurr(void);


/* 功能说明:
 *          thaisenSetModuleMinCurr:设置模块最小输出电流
 *
 * 输入参数:
 *      uint16_t:电压值  0.1A
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMinCurr(uint16_t curr);


/* 功能说明:
 *          thaisenGetModuleMinCurr:获取模块最小输出电流
 *
 * 输入参数:
 *
 * 返回参数:
 *          uint16_t:电压值  0.1A
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMinCurr(void);


/* 功能说明:
 *          thaisenSetModuleMaxChargVolt:设置模块最大充电电压
 *
 * 输入参数:
 *      uint16_t:电压值  0.1V
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMaxChargVolt(uint16_t volt);

/* 功能说明:
 *          thaisenGetModuleMaxChargVolt:获取模块最大充电电压
 *
 * 输入参数:
 *
 * 返回参数:
 *          uint16_t:电压值  0.1V
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMaxChargVolt(void);


/* 功能说明:
 *          thaisenSetModuleMaxChargCurr:设置模块最大充电电流
 *
 * 输入参数:
 *      uint16_t:电压值  0.1A
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMaxChargCurr(uint16_t curr);

/* 功能说明:
 *          thaisenGetModuleMaxChargCurr:获取模块最大充电电流
 *
 * 输入参数:
 *
 * 返回参数:
 *          uint16_t:电压值  0.1A
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMaxChargCurr(void);

/* 功能说明:
 *          thaisenGetModuleMaxChargCurr:按组设置模块最大充电电流
 *
 * 输入参数:           groupNum 组号
 *          curr     电流 (0.1)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleMaxChargCurrGroup(uint8_t groupNum, uint16_t curr);

/* 功能说明:
 *          thaisenGetModuleMaxChargCurrGroup:按组获取模块最大充电电流
 *
 * 输入参数:           groupNum 组号
 *
 * 返回参数:           该组模块最大充电电流
 *
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleMaxChargCurrGroup(uint8_t groupNum);

/* 功能说明:
 *          thaisenSetModuleGroupOpenState:按组设置模块组的开机状态
 *
 * 输入参数:           groupNum 组号
 *          sta     状态 (0：关机，1：开机)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleGroupOpenState(uint8_t groupNum, uint8_t sta);

/* 功能说明:
 *          thaisenGetModuleGroupOpenState:按组获取模块组的开机状态
 *
 * 输入参数:           groupNum 组号
 *
 * 返回参数:           模块组的开机状态(0：关机，1：开机)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetModuleGroupOpenState(uint8_t groupNum);

/* 功能说明:
 *          thaisenStorageModuleSetVoltage:按组保存给模块设置的输出电压
 *
 * 输入参数:           groupNum 组号
 *          volt     电压 (0.1)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenStorageModuleSetVoltage(uint8_t groupNum, uint32_t volt);

/* 功能说明:
 *          thaisenGetModuleSetVoltage:按组获取给模块设置的输出电压
 *
 * 输入参数:           groupNum 组号
 *
 * 返回参数:           给模块设置的输出电压
 *
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenGetModuleSetVoltage(uint8_t groupNum);

/* 功能说明:
 *          thaisenStorageModuleSetCurrent:按组保存给模块设置的输出电流
 *
 * 输入参数:           groupNum 组号
 *          curr     电流 (0.01)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenStorageModuleSetCurrent(uint8_t groupNum, uint32_t curr);

/* 功能说明:
 *          thaisenGetModuleSetVoltage:按组获取给模块设置的输出电流
 *
 * 输入参数:           groupNum 组号
 *
 * 返回参数:           给模块设置的输出电流
 *
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenGetModuleSetCurrent(uint8_t groupNum);

/* 功能说明:
 *          thaisenSetModuleGroupNum:设置模块组数
 *
 * 输入参数:
 *          groupNum:模块组数
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleGroupNum(uint16_t groupNum,uint8_t gunNum);

/* 功能说明:
 *          thaisenGetModuleFaultInfo:获取一把枪运行的模块的故障
 *
 * 输入参数:
 *          gunNum:模块组数
 *          Number:该枪运行的模块数
 * 返回参数:           该枪模块电压电流结构体指针(包含该枪运行的模块数对应数量的结构体)
 *
 * 调用方法:
 *          可实时调用
 */
thaisenModuleFaultInfoStruct *thaisenGetModuleFaultInfo(uint8_t *Number, uint8_t gunNum);

/* 功能说明:
 *          thaisenGetModuleVoltCurrInfo:获取一把枪运行的模块的电压电流
 *
 * 输入参数:
 *          gunNum:模块组数
 *          Number:该枪运行的模块数
 * 返回参数:           该枪模块电压电流结构体指针(包含该枪运行的模块数对应数量的结构体)
 *
 * 调用方法:
 *          可实时调用
 */
thaisenModuleVoltCurrStruct *thaisenGetModuleVoltCurrInfo(uint8_t *Number, uint8_t gunNum);

/* 功能说明:
 *          thaisenGetModuleFaultSetInfo:获取模块故障集(单个模块)
 *
 * 输入参数:
 *          Group:模块归属组
 *          Sequence:模块在组内的序号(从0开始)
 * 返回参数:           @thaisenModuleFaultSetStruct
 *
 * 调用方法:
 *          可实时调用
 */
thaisenModuleFaultSetStruct thaisenGetModuleFaultSetInfo(uint8_t Group, uint8_t Sequence);

/* 功能说明:
 *          thaisenSetModuleSetupVolt:模块调试部分:设置模块电压输出设定值
 *
 * 输入参数:
 *          gunNum:模块组数
 *          setupVolt:电压输出设定值
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleSetupVolt(uint16_t setupVolt, uint8_t gunNum);

/* 功能说明:
 *          thaisenSetModuleSetupCurr:模块调试部分:设置模块电流输出设定值
 *
 * 输入参数:
 *          gunNum:模块组数
 *          setupCurr:电流输出设定值
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleSetupCurr(uint16_t setupCurr, uint8_t gunNum);

/* 功能说明:
 *          thaisenGetModuleSetupVolt:模块调试部分:获取模块电压输出设定值
 *
 * 输入参数:
 *          gunNum:模块组数
 * 返回参数:  电压输出设定值
 *
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleSetupVolt(uint8_t gunNum);

/* 功能说明:
 *          thaisenGetModuleSetupCurr:模块调试部分:获取模块电流输出设定值
 *
 * 输入参数:
 *          gunNum:模块组数
 * 返回参数:  电流输出设定值
 *
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleSetupCurr(uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetHignestVolt:获取获取此枪的模块的最高输出电压
 *
 * 输入参数:
 *          gunNum:枪号
 * 返回参数:      此枪的模块的最高输出电压(0.1V)
 *
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenModuleGetHignestVolt(uint8_t gunNum);

/* 功能说明:
 *          thaisenSetModuleDebugEnableOutput:模块调试部分:启动使能(模块强制启动)
 *
 * 输入参数:
 *          gunNum:模块组数
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleDebugEnableOutput(uint8_t gunNum);

/* 功能说明:
 *          thaisenClearModuleDebugEnableOutput:模块调试部分:启动失能(模块强制启动)
 *
 * 输入参数:
 *          gunNum:模块组数
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenClearModuleDebugEnableOutput(uint8_t gunNum);

/* 功能说明:
 *          thaisenSetModuleDebugDisableOutput:模块调试部分:停止使能(模块强制停止)
 *
 * 输入参数:
 *          gunNum:模块组数
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleDebugDisableOutput(uint8_t gunNum);

/* 功能说明:
 *          thaisenClearModuleDebugDisableOutput:模块调试部分:停止失能(模块强制停止)
 *
 * 输入参数:
 *          gunNum:模块组数
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenClearModuleDebugDisableOutput(uint8_t gunNum);

/* 功能说明:
 *          thaisenClearModuleDebugDisableOutput:模块调试部分:获取启动使能状态
 *
 * 输入参数:
 *          gunNum:模块组数
 *          返回参数:  启动使能状态
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetModuleDebugEnableOutput(uint8_t gunNum);

/* 功能说明:
 *          thaisenClearModuleDebugDisableOutput:模块调试部分:获取停止使能状态
 *
 * 输入参数:
 *          gunNum:模块组数
 *          返回参数:  停止使能状态
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetModuleDebugDisableOutput(uint8_t gunNum);

/* 功能说明:
 *          thaisenGetNormalModuleNum:获取一把枪正常运行的模块数
 *
 * 输入参数:
 *          gunNum:枪号
 * 返回参数:  正常运行的模块数
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetNormalModuleNum(uint8_t gunNum);

/* 功能说明:
 *          thaisenGetModuleOutputVoltage:获取一把枪模块的输出电压
 *
 * 输入参数:
 *          gunNum:枪号
 * 返回参数:  模块的输出电压
 *
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenGetModuleOutputVoltage(uint8_t gunNum);

typedef enum
{
    thaisenModuleChargeWay_singleGun,
    thaisenModuleChargeWay_parallelCharge,
    thaisenModuleChargeWay_size,
}thaisenChargeWay;

/* 功能说明:
 *          thaisenModuleSetChargeWay:设置充电方式
 *
 * 输入参数:
 *          way:充电方式
 * 返回参数:  > 0：成功，<0：失败
 *
 * 调用方法:
 *          可实时调用
 */
int32_t thaisenModuleSetChargeWay(uint8_t way);

/* 功能说明:
 *          thaisenModuleGetChargeWay:获取充电方式
 *
 * 输入参数:
 *          gunNum:枪号
 * 返回参数:  模块的输出电压
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetChargeWay(void);

/* 功能说明:
 *          thaisenSetAllocateStrategy:设置功率分配策略
 *
 * 输入参数:
 *          Strategy:策略
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetAllocateStrategy(uint8_t Strategy);

/* 功能说明:
 *          thaisenGetAllocateStrategy:获取功率分配策略
 *
 * 返回参数:  功率分配策略
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetAllocateStrategy(void);


/* 功能说明:
 *          thaisenSetYouYouSlienceMode:设置是否使能优优模块静音模式
 *
 * 输入参数:
 *          state:使能状态：1：使能，0：失能
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetYouYouSlienceMode(uint8_t state);
/* 功能说明:
 *          thaisenGetYouYouSlienceMode:获取是否使能优优模块静音模式
 *
 * 输入参数:
 *
 * 返回参数:  使能状态：1：使能，0：失能
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetYouYouSlienceMode(void);

typedef enum
{
    thaisenFunction_enable,
    thaisenFunction_disable,
    thaisenFunction_size,
}thaisenFunction;

/* 功能说明:
 *          thaisenModuleSetParallelEnable:设置是否使能并联
 *
 * 输入参数:
 *          state:使能状态：0：使能，1：失能
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
int32_t thaisenModuleSetParallelEnable(uint8_t state);

/* 功能说明:
 *          thaisenModuleGetParallelEnable: 获取是否使能并联
 *
 * 输入参数:
 *
 * 返回参数:
 *          是否使能并联
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetParallelEnable(void);

/* 功能说明:
 *          thaisenModuleSetOpsModuleGroup:设置所操作的模块组
 *
 * 输入参数:
 *          group:模块组
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
int32_t thaisenModuleSetOpsModuleGroup(uint8_t group);

/* 功能说明:
 *          thaisenModuleGetOpsModuleGroup: 获取所操作的模块组
 *
 * 输入参数:
 *
 * 返回参数:
 *          模块组
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetOpsModuleGroup(void);

typedef enum
{
    thaisenAllowCharge_allow,               /* 允许充电 */
    thaisenAllowCharge_forbid,              /* 禁止充电 */
    thaisenAllowCharge_size,
}thaisenAllowCharge;

/* 功能说明:
 *          thaisenModuleSetAllowChargeState:设置是否允许充电
 *
 * 输入参数:
 *          state:状态
 *          gunNum:枪号
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
int32_t thaisenModuleSetAllowChargeState(uint8_t state, uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetAllowChargeState: 获取是否允许充电
 *
 * 输入参数:
 *          gunNum：枪号
 *
 * 返回参数:
 *          状态
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetAllowChargeState(uint8_t gunNum);

typedef enum
{
    thaisenBMSAllowCharge_forbid,              /* BMS禁止充电 */
    thaisenBMSAllowCharge_allow,               /* BMS允许充电 */
    thaisenBMSAllowCharge_size,
}thaisenBMSAllowCharge;

/* 功能说明:
 *          thaisenModuleSetBMSAllowCharge:设置BMS是否允许充电
 *
 * 输入参数:
 *          state:状态
 *          gunNum:枪号
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
int32_t thaisenModuleSetBMSAllowCharge(uint8_t state, uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetBMSAllowCharge: 获取BMS是否允许充电
 *
 * 输入参数:
 *          gunNum：枪号
 *
 * 返回参数:
 *          状态
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetBMSAllowCharge(uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleSetMaxCurrSingleGun: 设置单枪最大充电电流(0.01)
 *
 * 输入参数:
 *          curr  单枪最大充电电流(0.01)
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetMaxCurrSingleGun(uint32_t curr, uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetMaxCurrSingleGun: 获取单枪最大充电电流(0.01)
 *
 * 输入参数:
 *
 * 返回参数:
 *                    单枪最大充电电流(0.01)
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenModuleGetMaxCurrSingleGun(uint8_t gunNum);

/* 功能说明:
 *          thaisenSetEnableModuleState: 设置使能模块状态
 *
 * 输入参数:   state(1:使能，0:不使能)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetEnableModuleState(uint8_t state);

/* 功能说明:
 *          thaisenGetEnableModuleState: 获取使能模块状态
 *
 * 输入参数:
 *
 * 返回参数:    (1:使能，0:不使能)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetEnableModuleState(void);

/* 功能说明:
 *          thaisenSetEnableModuleOperateResult: 设置使能模块操作结果
 *
 * 输入参数:  (1:成功，0:失败)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetEnableModuleOperateResult(uint8_t state);

/* 功能说明:
 *          thaisenGetEnableModuleOperateResult: 获取使能模块操作结果
 *
 * 输入参数:
 *
 * 返回参数:    (1:成功，0:失败)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetEnableModuleOperateResult(void);

/* 功能说明:
 *          thaisenSetEnableModuleOperateState: 设置模块使能操作状态
 *
 * 输入参数:           state (1:完成，0:未完成)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetEnableModuleOperateState(uint8_t state);

/* 功能说明:
 *          thaisenGetEnableModuleOperateState: 获取模块使能操作状态
 *
 * 输入参数:
 *
 * 返回参数:          (1:完成，0:未完成)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetEnableModuleOperateState(void);

/*************************************************** 剔除故障模块功能 ****************************************************/
/* 功能说明:
 *          thaisenModuleSetEliminateModuleState: 设置剔除模块功能使能状态
 *
 * 输入参数:   state      状态   0：不使能      1：使能
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetEliminateModuleState(uint8_t state);

/* 功能说明:
 *          thaisenModuleGetEliminateModuleState: 获取剔除模块功能使能状态
 *
 * 输入参数:
 *
 * 返回参数:  状态   0：不使能      1：使能
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetEliminateModuleState(void);

/* 功能说明:
 *          thaisenSetIncludeAcRelayState: 设置是否包含交流接触器状态
 *
 * 输入参数:           state (1:包含，0:不包含)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetIncludeAcRelayState(uint8_t state);

/* 功能说明:
 *          thaisenIsIncludeAcRelay: 是否包含交流接触器状态
 *
 * 输入参数:
 *
 * 返回参数:         (1:包含，0:不包含)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenIsIncludeAcRelay(void);

/*************************************************** 模块输入电源连接状态 ****************************************************/
/* 功能说明:
 *          thaisenSetIncludeAcRelayState: 设置模块输入电源连接状态
 *
 * 输入参数:           state (1:已连接，0:未连接)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModule_SetInPowerConnectState(uint8_t state);

/* 功能说明:
 *          thaisenModule_IsInPowerConnected: 获取模块输入电源连接状态
 *
 * 输入参数:
 *
 * 返回参数:         (1:已连接，0:未连接)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModule_IsInPowerConnected(void);

/*************************************************** 模块输入电源类型 ****************************************************/
enum
{
    THAISEN_MODULE_INPOWER_TYPE_CONTROL_FB,          /** 模块输入电源类型：需要控制而且有反馈 */
    THAISEN_MODULE_INPOWER_TYPE_ONLY_CONTROL,        /** 模块输入电源类型：需要控制但是控制无反馈 */
    THAISEN_MODULE_INPOWER_TYPE_DIRECTLY,            /** 模块输入电源类型：直连的 */
    THAISEN_MODULE_INPOWER_TYPE_SIZE,                /** 模块输入电源类型： */
};

/* 功能说明:
 *          thaisenModule_SetInPowerType: 设置模块输入电源类型
 *
 * 输入参数:           type     类型
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModule_SetInPowerType(uint8_t type);

/* 功能说明:
 *          thaisenModule_GetInPowerType: 获取模块输入电源类型
 *
 * 输入参数:
 *
 * 返回参数:        模块输入电源类型
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModule_GetInPowerType(void);


typedef enum
{
    THAISEN_MODULE_FAULT_OUT_OV,                          /* 故障：输出过压 */
    THAISEN_MODULE_FAULT_OUT_UV,                          /* 故障：输出欠压 */
    THAISEN_MODULE_FAULT_OUT_OC,                          /* 故障：输出过流 */
    THAISEN_MODULE_FAULT_SIZE,                            /* 故障 */
}thaisenModuleFEnum_t;

/* 功能说明:
 *          thaisenModuleSetFEnState: 设置故障使能状态
 *
 * 输入参数:           f       故障枚举
 *            state   状态(1：使能   0：不使能)
 *            gunNum  枪号
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetFEnState(thaisenModuleFEnum_t f, uint8_t state, uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetFEnState: 获取故障使能状态
 *
 * 输入参数:       f       故障枚举
 *          gunNum  枪号
 *
 * 返回参数:   故障使能状态(1：已使能   0：未使能)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetFEnState(thaisenModuleFEnum_t f, uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleSetFOccurState: 设置故障发生状态
 *
 * 输入参数:           f       故障枚举
 *            state   状态(1：已发生   0：未发生)
 *            gunNum  枪号
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetFOccurState(thaisenModuleFEnum_t f, uint8_t state, uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetFOccurState: 获取故障发生状态
 *
 * 输入参数:       f       故障枚举
 *          gunNum  枪号
 *
 * 返回参数:   故障使能状态(1：已发生   0：未发生)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetFOccurState(thaisenModuleFEnum_t f, uint8_t gunNum);

/*************************************************** 模块最大输出电流 ****************************************************/
/* 功能说明:
 *          thaisenSetModuleOutCurrMax: 设置模块最大输出电流
 *
 * 输入参数:  curr      模块最大输出电流值(0.01A)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleOutCurrMax(uint32_t curr);

/* 功能说明:
 *          thaisenGetModuleOutCurrMax: 获取模块最大输出电流
 *
 * 输入参数:
 *
 * 返回参数:   模块最大输出电流(0.01A)
 *
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenGetModuleOutCurrMax(void);

/*************************************************** 模块最小输出电流 ****************************************************/

/* 功能说明:
 *          thaisenSetModuleOutCurrMin: 设置模块最小输出电流
 *
 * 输入参数:  curr      模块最小输出电流值(0.01A)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetModuleOutCurrMin(uint16_t curr);

/* 功能说明:
 *          thaisenGetModuleOutCurrMin: 获取模块最大输出电流
 *
 * 输入参数:       f       故障枚举
 *          gunNum  枪号
 *
 * 返回参数:   模块最小输出电流(0.01A)
 *
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisenGetModuleOutCurrMin(void);

/*************************************************** 并充枪电压检测闭合 ****************************************************/
/* 功能说明:
 *          thaisenModuleSetParaGunVoltDetectEn: 设置并充枪电压检测使能状态
 *
 * 输入参数:          gunNum      枪号
 *         state       使能状态(1:使能  0:不使能)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetParaGunVoltDetectEn(uint8_t gunNum, uint8_t state);

/* 功能说明:
 *          thaisenModuleGetParaGunVoltDetectEn: 获取并充枪电压检测使能状态
 *
 * 输入参数:       gunNum    枪号
 *
 *
 * 返回参数:   使能状态(1:使能  0:不使能)
 *
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetParaGunVoltDetectEn(uint8_t gunNum);

/*************************************************** 模块工作模式 ****************************************************/
typedef enum
{
    THAISEN_MODULE_WORKMODE_RECTIFICATION,                   /* 模块工作模式：整流 */
    THAISEN_MODULE_WORKMODE_ON_CONTRAVARIANT,                /* 模块工作模式：并网逆变 */
    THAISEN_MODULE_WORKMODE_OFF_CONTRAVARIANT,               /* 模块工作模式：离网逆变 */
    THAISEN_MODULE_WORKMODE_SIZE,                            /* 模块工作模式 */
}thaisenModuleWorkMode_t;

/* 功能说明:
 *          thaisenModuleSetWorkMode: 设置模块工作模式
 *
 * 输入参数:  gunNum      枪号
 *          mode       模式@thaisenModuleWorkMode_t
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetWorkMode(uint8_t gunNum, thaisenModuleWorkMode_t mode);

/* 功能说明:
 *          thaisenModuleGetWorkMode: 获取模块工作模式
 *
 * 输入参数:   gunNum    枪号
 *
 *
 * 返回参数:   模块工作模式@thaisenModuleWorkMode_t
 *
 * 调用方法:
 *          可实时调用
 */
thaisenModuleWorkMode_t thaisenModuleGetWorkMode(uint8_t gunNum);

/*************************************************** 模块设置电流偏移 ****************************************************/
/* 功能说明:
 *          thaisenModuleSetMSetupCurrOffset: 设置模块设置电流偏移
 *
 * 输入参数:           gunNum      枪号
 *          Offset      电流偏移(0.01A)
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetMSetupCurrOffset(uint8_t gunNum, int16_t Offset);

/* 功能说明:
 *          thaisenModuleGetMSetupCurrOffset: 获取模块设置电流偏移
 *
 * 输入参数:   gunNum    枪号
 *
 *
 * 返回参数:   电流偏移(0.01A)
 *
 * 调用方法:
 *          可实时调用
 */
int16_t thaisenModuleGetMSetupCurrOffset(uint8_t gunNum);

/* 功能说明:
 *          thaisenModuleGetGunAllocateCurrent: 获取充电中枪所能分配到的最大电流(非充电情况下默认为单枪最大电流)
 *
 * 输入参数:   gunNum    枪号
 *
 *
 * 返回参数:   充电中枪所能分配到的最大电流(0.01A)
 *
 * 调用方法:
 *          可实时调用
 */
uint32_t thaisenModuleGetGunAllocateCurrent(uint8_t gunNum);

/*************************************************** BMS协议类型 ****************************************************/
typedef enum
{
    THAISEN_MODULE_BMS_PROTYPE_27930_2015,                   /* BMS协议类型：国标27930 2015 */
    THAISEN_MODULE_BMS_PROTYPE_33021_NB_T,                   /* BMS协议类型：能标33021 2024 */
    THAISEN_MODULE_BMS_PROTYPE_SIZE,                         /* BMS协议类型 */
}thaisenModule_BMSProType_t;

/* 功能说明:
 *          thaisenModuleSetBMSProtoclType: 设置BMS协议类型
 *
 * 输入参数:           gunNum      枪号
 *          type         BMS协议类型@thaisenModule_BMSProType_t
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenModuleSetBMSProtoclType(uint8_t gunNum, thaisenModule_BMSProType_t type);

/* 功能说明:
 *          thaisenModuleGetBMSProtoclType: 获取BMS协议类型
 *
 * 输入参数:   gunNum    枪号
 *
 *
 * 返回参数:    BMS协议类型@thaisenModule_BMSProType_t
 *
 * 调用方法:
 *          可实时调用
 */
thaisenModule_BMSProType_t thaisenModuleGetBMSProtoclType(uint8_t gunNum);

/*************************************************** 环矩/半矩部分 ***************************************************/
/***************************[子母机配置项]*********************************/
enum
{
    thaisen_masterSlaveCom_devType_Master,
    thaisen_masterSlaveCom_devType_Slave,
};

typedef struct
{
    uint8_t devType;//本机类型
    uint8_t gunAddr[2];//本机地址，1~(0为无效值)
}thaisen_masterSlaveCom_init_t;

/**
 * @brief 初始化子母机通信部分
 * @param initInfo
 */
void thaisenMasterSlave_Init(thaisen_masterSlaveCom_init_t initInfo);

/***************************[模块分配配置项]**********************************/
typedef enum
{
    ModuleAllo_share,  /** 均分 */
    ModuleAllo_fcfs,   /** 先到先得 */
    ModuleAllo_hpf,    /** 功率优先 */
}ModuleAlloMethod_e;

/* 矩阵类型 */
typedef enum
{
    thaisen_moduleallo_matrixtype_halfmatrix,       /* 半矩阵 */
    thaisen_moduleallo_matrixtype_fullmatrix,       /* 全矩阵 */
    thaisen_moduleallo_matrixtype_ringmatrix,       /* 环矩阵 */
    thaisen_moduleallo_matrixtype_none,             /* 无 */
}thaisen_moduleallo_matrixtype;

typedef enum
{
    server_disable = 0,
    server_enable,
    server_waiting,
}thaisen_ccu_pcuserversta;

#pragma pack(1)
typedef struct
{
    ModuleAlloMethod_e allomethod;//分配策略
    uint32_t module_preserpower;//模块额定功率1w
    uint8_t module_groupcnt;//模块组数
    uint8_t module_cntforgroup[4];//MAXGROUPCNT
    uint8_t module_mincurr;//模块最小电流，精度1A
    uint16_t module_maxcurr;//模块最大电流。精度0.01A
    uint16_t module_minvolt;//模块最小电压0.1V
    uint8_t module_addroffset;//模块地址偏移：默认设置0x20，英飞源模块不支持0x20，使用0x60
    uint8_t guncnt;//枪数量
    thaisen_moduleallo_matrixtype matrix_type;//矩阵类型
    uint8_t devType;
    /**
     * @note 设置接触器状态
     * @param 接触器编号
     * @param 接触器PN - 0：DC+ 1：DC-
     * @param 状态FML_relaystaType
     */
    void (*setrelaysta)(uint8_t, uint8_t, uint8_t);
    /**
     * @note 获取矩阵接触器状态
     * @param 接触器编号
     * @param 接触器PN - 0：DC+ 1：DC-
     * @return 接触器状态FML_relaystaType
     */
    uint8_t (*getrelaysta)(uint8_t, uint8_t);
    /**
     * @note 获取直流输出接触器状态(仅闭合/断开)
     * @param 枪号(1~)
     * @param 接触器PN - 0：DC+ 1：DC-
     * @return 直流接触器状态:断开(relayopen)/闭合(relayclose)
     */
    uint8_t (*getdcrelaysta)(uint8_t, uint8_t);
    /**
     * @note 接触器状态变化回调
     * @param 接触器编号
     * @param 接触器PN - 0：DC+ 1：DC-
     * @param 接触器状态
     */
    void (*stachangefb)(uint8_t, uint8_t, uint8_t);
    /**
     * @note 设置枪故障
     * @param 枪号(1~)
     * @param 接触器编号(1~)
     */
    void (*gunfaultset)(uint8_t, uint8_t);
    /**
     * @note 清除枪故障
     * @param 枪号(1~)
     * @param 接触器编号(1~)
     */
    void (*gunfaultclean)(uint8_t, uint8_t);

    /**
     * @brief NetLogSend:上报监控信息填报后回调(单次调用，不会反复调用)
     * @param 指向存储监控信息的缓存(下层缓存长度为50字节)
     * @param 此次报文长度
     */
    void (*NetLogSend)(uint8_t const* const, uint8_t);
}powerctrl_init_t;

typedef struct
{
    uint8_t step;
    uint32_t sysTick;
}stepTransRecord;

typedef struct
{
    uint8_t aimGunNum;//目标枪号
    uint8_t ofsmStepCnt;//跳转次数
    stepTransRecord *buf;//用于保存状态机跳转信息
}moduleStepInfo_t;
#pragma pack()

/**
 * @note 初始化函数
 * @param base_info
 */
void thaisen_base_init(powerctrl_init_t base_info);
void thaisen_base_deInit(void);

/**
 * @brief 请求充电时调用
 * @param gunNum 1~
 */
void thaisen_moduleallo_chargeReq(uint8_t gunNum);

/**
 * @brief 请求充电超时调用
 * @param gunNum
 */
void thaisen_moduleallo_chargeReqFinish(uint8_t gunNum);

/**
 * @brief thaisen_get_gun_serversta:获取枪允许充电状态
 * @param gunnum(1~)
 * @return
 */
thaisen_ccu_pcuserversta thaisen_get_gun_serversta(uint8_t gunnum);

/**
 * @note 设置模块故障检测标志，设为1后若模块状态为离线或故障则会踢出分配队列
 * @note 设置要求：上层认为可以进行模块分配的时候置1，仅置1后调用thaisen_gun_set_chargeinfo会分配模块并调度
 * @note        置1后模块状态仍在offline/err的模块组会被踢出分配队列
 * @note        置0后不会再将offline/err的模块组踢出分配队列，且模块分配函数不会分配模块并进行模块调度
 * @note        置0时会判断此时有被分配模块的枪，全部取消模块调度(模块关机、矩阵接触器断开)
 * @param sat
 */
void thaisen_module_set_ModuleSchedulingEnable(void);
void thaisen_module_set_ModuleSchedulingDisable(void);
uint8_t thaisen_module_get_ModuleScheduling(void);

/**
 * @note 模块为易能模块时，初次上电需要进行配置时调用
 */
void thaisen_guowang_set_moduletype_ensd(void);

/***************[以下函数供模块调试使用，充电流程通过设置枪功率分配的模块不需要调用以下函数 - Start]*****************/

/**
 * @brief 模块DEBUG使能(需先设置模块控制参数后再调用)
 * @param groupnum
 */
void thaisen_guowang_moduledebug_enable(uint8_t groupnum);

/**
 * @brief 模块DEBUG失能
 * @param groupnum
 */
void thaisen_guowang_moduledebug_disable(uint8_t groupnum);

/**
 * @note 设置模块控制参数
 * @param groupnum
 * @param cmd
 * @param volt
 * @param curr
 * @param batvolt
 */
void thaisen_guowang_set_controlparam(uint8_t groupnum, uint8_t cmd, uint16_t volt, uint16_t curr, uint16_t batvolt);

/**
 * @brief 获取模块组控制字
 * @param groupnum 1~
 * @return 0x01 - 快速开机，0x02 - 关机，0x03 - 软启，0x04 - ，0x05 - 调参，0x06 - 关机
 */
uint8_t thaisen_guowang_get_groupctrlcmd(uint8_t groupnum);

/**
 * @brief 获取模块组控制电压
 * @param groupnum 1~
 * @return 0.1V
 */
uint16_t thaisen_guowang_get_groupctrlvolt(uint8_t groupnum);

/**
 * @brief 获取模块组控制电流
 * @param groupnum 1~
 * @return 0.01A
 */
uint16_t thaisen_guowang_get_groupctrlcurr(uint8_t groupnum);


/***************[以上函数供模块调试使用，充电流程通过设置枪功率分配的模块不需要调用以下函数  - End]*****************/

/**
 * @note 设置接触器反馈使能
 * @param relaynum 接触器编号(1~)
 * @param 接触器PN - 0：DC+ 1：DC-
 * @param sta - 0:失能    1:使能
 */
void thaisen_relay_set_feedbackenbale(uint8_t relaynum, uint8_t pn, uint8_t sta);

/**
 * @note 设置接触器反馈取反
 * @param relaynum 接触器编号(1~)
 * @param 接触器PN - 0：DC+ 1：DC-
 * @param sta - 0：不取反       1:取反
 */
void thaisen_relay_set_feedbackinvert(uint8_t relaynum, uint8_t pn, uint8_t sta);

/**
 * @note 设置单个模块的最小输出电流
 * @param curr 电流值(1A)
 */
void thaisen_set_module_mincurr(uint32_t curr);

/**
 * @note 设置单个模块的最大输出电流
 * @param curr 电流值(0.01A)
 */
void thaisen_set_module_maxcurr(uint32_t curr);

/**
 * @brief 设置模块额定功率
 * @param power (1W)
 */
void thaisen_chargemain_set_ModulePreserPower(uint32_t power);

/**
 * @brief 获取模块额定功率
 * @return (1W)
 */
uint32_t thaisen_chargemain_get_ModulePresetPower(void);

/**
 * @brief 获取模块状态跳转信息
 * @param moduleNum 1~
 * @return
 */
moduleStepInfo_t thaisen_get_moduleStepInfo(uint8_t moduleNum);

/**
 * @brief 清除部分模块状态跳转信息
 * @param moduleNum 1~
 * @param stepCnt
 * @return 0 - 成功
 */
uint8_t thaisen_clean_moduleStepInfo(uint8_t moduleNum, uint8_t stepCnt);

/**
 * @brief 获取模块输出电流
 * @param moduleNum 1~
 * @return 0.01A
 */
uint16_t thaisen_module_getDcOutputCurr(uint8_t moduleNum);

/**
 * @brief 获取模块输出电压
 * @param moduleNum 1~
 * @return 0.1V
 */
uint16_t thaisen_module_getDcOutputVolt(uint8_t moduleNum);

/**
 * @brief 获取模块当前组号(模块上报)
 * @param moduleNum 1~
 * @return
 */
uint8_t thaisen_module_getModuleGroupNum(uint8_t moduleNum);

/**
 * @brief 获取模块工作状态
 * @param moduleNum 1~
 * @return 0 - 离线；1 - 在线；2 - 运行；3 - 故障
 */
uint8_t thaisen_module_getModuleWorkSta(uint8_t moduleNum);

/**
 * @brief 设置枪最大电流
 * @param gunNum 1~2
 * @param maxCurr 0.01A
 */
void thaisen_set_gun_maxCurr(uint8_t gunNum, uint32_t maxCurr);

/**
 * @brief 获取枪最大电流
 * @param gunNum 1~2
 * @return 0.01A
 */
uint32_t thaisen_get_gun_maxCurr(uint8_t gunNum);

/*********************************************************************************************************************/
/*********************************************************************************************************************/

#endif /* APPLICATIONS_THAISENCHARGMODULELIB_H_ */
