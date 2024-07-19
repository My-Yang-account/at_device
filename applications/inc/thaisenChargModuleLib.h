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
        };
        uint8_t fault_val;
    }fault;   /* 故障 */
    union{
        struct{
            uint8_t Fan                    : 1;       /* 风扇 */
        };
        uint8_t warn_val;
    }warn;   /* 警告 */
    union{
        struct{
            uint8_t BootState              : 1;       /* 开机状态(1:开机) */
        };
        uint8_t state_val;
    }state;   /* 状态 */
    int16_t temperature;
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

typedef struct thasienModuleSetStruct
{
    uint8_t moduleProNo;//模块类型
    uint8_t moduleGroupNum;//模块组数
    uint8_t moduleSingleGroupNum[2];//模块组内个数
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

typedef enum
{
    thaisenDeviceType_doubleGun,           /* 双枪终端 */
    thaisenDeviceType_singleGun,           /* 单枪终端 */
    thaisenDeviceType_average,             /* 动态切换 */
    thaisenDeviceType_Rectifier_cabinet,   /* 整流柜 */
    thaisenDeviceType_size,
}thaisenDeviceType;

/* 功能说明:
 *          thaisenModuleSetDeviceType:设置设备类型
 *
 * 输入参数:
 *          type:类型
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
int32_t thaisenModuleSetDeviceType(uint8_t type);

/* 功能说明:
 *          thaisenModuleGetDeviceType: 获取设备类型
 *
 * 输入参数:
 *
 * 返回参数:
 *          设备类型
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenModuleGetDeviceType(void);

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

#endif /* APPLICATIONS_THAISENCHARGMODULELIB_H_ */
