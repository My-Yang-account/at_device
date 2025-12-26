/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-07     Lenovo       the first version
 */
#ifndef APPLICATIONS_THAISEN7102PUBLIC_H_
#define APPLICATIONS_THAISEN7102PUBLIC_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "stm32f4xx_hal.h"

/*******************************协议一致性/互操作测试版本使能****************************************************/

/* 功能说明:
 *      thaisenDrvSetProtocolTestEnable:设置协议一致性/互操作测试版本使能状态
 * 输入参数:   state   >0：使能   0：不使能
 *         无
 * 返回参数:
 *         无
 * 调用方法:
 *          实时调用
 */
void thaisenDrvSetProtocolTestEnable(uint8_t state);

/* 功能说明:
 *      thaisenDrvIsEnableProtocolTest: 检查是否是否使能了协议一致性/互操作测试版本
 * 输入参数:
 *         无
 * 返回参数:
 *         1：使能   0：不使能
 * 调用方法:
 *          实时调用
 */
uint8_t thaisenDrvIsEnableProtocolTest(void);

/*******************************系统函数****************************************************/
/* 功能说明:
 *      thaisen_board_bsp_init:系统驱动初始化
 *      主频:180MHz
 *      采用外部晶振:25MHz
 * 输入参数:
 *         无
 * 返回参数:
 *         无
 * 调用方法:
 *          在mian函数最初位置调用
 */
void thaisen_board_bsp_init(void);


/* 功能说明:
 *      thaisenBubbleSort:冒泡排序
 * 输入参数:
 *      arr:排序的数组
 *      n:排序的个数
 * 返回参数:
 *         无
 * 调用方法:
 *
 */
void thaisenBubbleSort(uint16_t arr[], uint8_t n);


/* 功能说明:
 *      thaisenCRC32:CRC32校验算法
 * 输入参数:
 *      buf:校验的数组
 *      size:校验的大小
 * 返回参数:
 *         无
 * 调用方法:
 *
 */
unsigned int thaisenCRC32(const unsigned char *buf, unsigned int size);

/*********************************************************************************************/

/*******************************看门狗函数******************************************************/
/* 功能说明:
 *        thaisenIwdgRefresh:看门狗喂狗函数
 *          看门狗复位时间为24s
 * 输入参数:
 *          无
 * 返回参数:
 *          无
 * 调用方法:
 *          可实时调用
 */
void thaisenIwdgRefresh(void);

/* 功能说明:
 *        thaisenIwdgRefresh:看门狗初始化函数
 *          看门狗复位时间为24s
 * 输入参数:
 *          无
 * 返回参数:
 *          无
 * 调用方法:
 *         初始化调用
 */
void MX_IWDG_Init(void);
/*********************************************************************************************/


/*******************************RTC函数********************************************************/

typedef struct thaisenRTCStruct
{
    RTC_DateTypeDef thaisenGetData;
    RTC_TimeTypeDef thaisenGetTime;
}thaisenRTCSt;


/* 功能说明:
 *          thaisenGetRTC:获取时间日期
 * 输入参数:
 *
 * 返回参数:
 *          thaisenRTCSt
 * 调用方法:
 *          调用时只调用一次
 */

thaisenRTCSt thaisenGetRTC(void);


/* 功能说明:
 *          thaisenSetRTC:设置时间日期
 * 输入参数:
 *          thaisenRTCSt
 * 返回参数:
 *          无
 * 调用方法:
 *          调用时只调用一次
 */
void thaisenSetRTC(thaisenRTCSt data);

/*********************************************************************************************/


/*******************************急停函数********************************************************/

typedef enum thaisenScramEnum
{
    thaisenScarmStatusRelease = 0,                            //急停释放
    thaisenScarmStatusPress = !thaisenScarmStatusRelease,     //急停按下
}thaisenScramStausEn;


/* 功能说明:
 *          thaisenGetScramStatus:获取急停状态
 * 输入参数:
 *
 * 返回参数:
 *          无
 * 调用方法:
 *          可实时调用
 */

thaisenScramStausEn thaisenGetScramStatus(void);

/* 功能说明:
 *          thaisenSetScramPressStatua:设置急停按下时的状态
 * 输入参数:
 *          Presssta:只能输入0或1,输入其他值保持原先状态
 * 返回参数:
 *          无
 * 调用方法:
 *          可实时调用
 */
void thaisenSetScramPressStatua(uint8_t Presssta);


/* 功能说明:
 *        thaisenGetScramPressStatus:获取急停按下时的状态
 * 输入参数:
 *                        无
 * 返回参数:
 *        uint8_t:返回设置的急停按下时的状态，只有0或1,若有其他值非法
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetScramPressStatus(void);

/****************************************************************************/

/*******************************门禁信号检测**********************************/

typedef enum thaisenDoorEnum
{
    thaisenDoorStatusRelease = 0,                            //门禁打开
    thaisenDoorStatusPress = !thaisenDoorStatusRelease,      //门禁闭合
}thaisenDoorStausEn;





/* 功能说明:
 *          thaisenSetDoorPressStatua:获取门禁状态
 * 输入参数:
 *
 * 返回参数:
 *          无
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetDoorStatus(void);



/* 功能说明:
 *          thaisenSetDoorPressStatua:设置门禁闭合时的状态
 * 输入参数:
 *          Presssta:只能输入0或1,输入其他值保持原先状态
 * 返回参数:
 *          无
 * 调用方法:
 *          可实时调用
 */
void thaisenSetDoorPressStatua(uint8_t Presssta);

/*********************************************************************************************/

/*********************************电磁锁采样函数******************************************************/

/* 功能说明:
 *   电磁锁是否锁止或解锁成功
 *
 */
typedef enum thaisenElectLockEnum
{
    thaisen_elect_lock_ok,
    thaisen_elect_lock_fail,
}thaisenElectLockEn;


/* 功能说明:
 *         thaisenSetElectLockFeedbackSta:设置电磁锁锁止的反馈状态
 * 输入参数:
 *         sta:只能输入0或1,输入其他值保持原先状态
 * 返回参数:
 * 调用方法:
 *          可实时调用
 */
void thaisenSetElectLockFeedbackSta(uint8_t sta);
void thaisenSetElectLockBFeedbackSta(uint8_t sta);

/* 功能说明:
 *      thaisenGetElectLockFeedbackSta:获取电磁锁反馈状态
 * 输入参数:
 *
 * 返回参数:
 *      uint8_t:返回设置的电磁锁锁止的状态，只有0或1,若有其他值非法
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetElectLockFeedbackSta(void);
uint8_t thaisenGetElectLockBFeedbackSta(void);

/* 功能说明:
 *          thaisenElectLock:闭合电子锁
 *
 * 输入参数:
 *         无
 * 返回参数:
 *          thaisenElectLockEn:
 *          thaisen_elect_lock_ok:锁成功
 *          thaisen_elect_lock_fail:锁失败
 * 调用方法:
 *          可实时调用
 */
thaisenElectLockEn thaisenElectLock(void);
thaisenElectLockEn thaisenElectLockB(void);

/* 功能说明:
 *          thaisenElectUnlock:解开电子锁
 *
 * 输入参数:
 *         无
 * 返回参数:
 *          thaisenElectLockEn:
 *          thaisen_elect_lock_ok:解锁成功
 *          thaisen_elect_lock_fail:解锁失败
 * 调用方法:
 *          可实时调用
 */
thaisenElectLockEn thaisenElectUnlock(void);
thaisenElectLockEn thaisenElectUnlockB(void);

typedef enum
{
    thaisen_elock_close,
    thaisen_elock_break,
}thaisenElectLockSta;

/* 功能说明:
 *          thaisenElectLock_StateQuery:查询电子锁状态
 *
 * 输入参数:
 *         gunNum      枪号
 * 返回参数:
 *          @thaisenElectLockSta
 * 调用方法:
 *          可实时调用
 */
thaisenElectLockSta thaisenElectLock_StateQuery(uint8_t gunNum);

/***************************** 以下是调试函数 *****************************/
/* 功能说明:
 *          thaisenElectLockA_Directly:直接上锁A枪电子锁(不检测反馈、仅操作IO口)
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenElectLockA_Directly(void);

/* 功能说明:
 *          thaisenElectUnlockA_Directly:直接解锁A枪电子锁(不检测反馈、仅操作IO口)
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenElectUnlockA_Directly(void);

/* 功能说明:
 *          thaisenElectLockB_Directly:直接上锁B枪电子锁(不检测反馈、仅操作IO口)
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenElectLockB_Directly(void);

/* 功能说明:
 *          thaisenElectUnlockB_Directly:直接解锁B枪电子锁(不检测反馈、仅操作IO口)
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenElectUnlockB_Directly(void);

/*********************** 手动解锁 ***********************/

/* 功能说明:
 *          thaisenElectUnlockA_Manual:人工手动解锁A枪电子锁(不检测反馈、先上锁再解锁)
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenElectUnlockA_Manual(void);

/* 功能说明:
 *          thaisenElectUnlockB_Directly:人工手动解锁B枪电子锁(不检测反馈、先上锁再解锁)
 *
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenElectUnlockB_Manual(void);

/*****************************************************************************************************/
/************************************系统故障信息*********************************************************/
typedef enum thaisenFaultEnum
{
  thaisenFaultScram,
  thaisenCardReader,
  thaisenDoor,
  thaisenFaultAmmeter,
  thaisenChargModule,
  thaisenFaultOverTemp,
  thaisenFaultOverVolt,
  thaisenFaultUnderVolt,
  thaisenFaultOverCurrent,
  thaisenRelay,
  thaisenRelayParallel,
  thaisenRelayAc,
  thaisenElock,
  thaisenAuxPower,
  thaisenFaultFlash,
  thaisenFaultEeprom,
  thaisenFaultLightProtect,
  thaisenFaultGunSite,
  thaisenFaultCircuitBreaker,
  thaisenFaultFlooding,
  thaisenFaultSmoke,
  thaisenFaultPour,
  thaisenFaultLiquidCooling,
  thaisenFaultFuse,
  thaisenFaultMainCabinet_Offline,                                            /** 系统故障：主机柜离线 */
  thaisenFaultMatrixRelay_KPN1_1,                                             /** 系统故障：矩阵正负接触器KPN1-1 */
  thaisenFaultMatrixRelay_KPN1_2,                                             /** 系统故障：矩阵正负接触器KPN1-2 */
  thaisenFaultMatrixRelay_KPN1_3,                                             /** 系统故障：矩阵正负接触器KPN1-3 */
  thaisenFaultMatrixRelay_KPN2_1,                                             /** 系统故障：矩阵正负接触器KPN2-1 */
  thaisenFaultMatrixRelay_KPN2_2,                                             /** 系统故障：矩阵正负接触器KPN2-2 */
  thaisenFaultMatrixRelay_KPN3_1,                                             /** 系统故障：矩阵正负接触器KPN3-1 */
  thaisenFaultSlaveDevice_Offline,                                            /** 系统故障：从设备离线 */
  thaisenFaultFan,                                                            /** 系统故障：风扇 */
  thaisenFaultMainCabinet_Scram,                                              /** 系统故障：主机柜急停 */
  thaisenFaultMainCabinet_Gate,                                               /** 系统故障：主机柜门禁 */
  thaisenFaultMainCabinet_PduFault,                                           /** 系统故障：主机柜开关板故障 */
  thaisenFaultMainCabinet_ModuleFault,                                        /** 系统故障：主机柜模块 */
  thaisenFaultMainCabinet_Config,                                             /** 系统故障：主机柜配置项 */
  thaisenFaultMainCabinet_AcRelay,                                            /** 系统故障：主机柜交流接触器 */
  thaisenFaultMainCabinet_Smoke,                                              /** 系统故障：主机柜烟感报警 */
  thaisenFaultMainCabinet_Pour,                                               /** 系统故障：主机柜倾倒 */
  thaisenFaultMainCabinet_Flooding,                                           /** 系统故障：主机柜水浸 */
  thaisenFaultMainCabinet_Other,                                              /** 系统故障：主机柜其它故障 */
  thaisenFaultMainCabinet_LightProtect,                                       /** 系统故障：主机柜防雷故障 */
  thaisenFaultDeviceIsLocked,                                                 /** 系统故障：设备已锁定 */

  thaisenFaultSize,
}thaisenFaultTy;

/*********************************继电器控制函数******************************************************/

typedef enum thaisenRelayEnum
{
    thaisenRelayClose,//继电器闭合
    thaisenRelayBreak,//继电器断开
}thaisenRelayEn;


enum
{
    thaisenDCRelayClose = 0,//继电器闭合
    thaisenDCRelayOff,//继电器断开
};

/* 功能说明:
 *   继电器闭合或断开是否成功
 *
 */
typedef enum thaisenRelayUEnum
{
    thaisen_Relay_ok,
    thaisen_Relay_fail,
}thaisenRelayUEn;

thaisenRelayUEn thaisen_relay_on_A(void);
thaisenRelayUEn thaisen_relay_off_A(void);
thaisenRelayUEn thaisen_relay_on_B(void);
thaisenRelayUEn thaisen_relay_off_B(void);
thaisenRelayUEn thaisen_relay_parallel_on_z(void);
thaisenRelayUEn thaisen_relay_parallel_off_z(void);
thaisenRelayUEn thaisen_relay_parallel_on_f(void);
thaisenRelayUEn thaisen_relay_parallel_off_f(void);
thaisenRelayUEn thaisen_relay_ac_on(void);
thaisenRelayUEn thaisen_relay_ac_off(void);
thaisenRelayEn thaisen_relay_AC_SetFB(void);
thaisenRelayUEn thaisen_relay_k7_k8_on(void);
thaisenRelayUEn thaisen_relay_k7k8_off(void);
thaisenRelayUEn thaisen_relay_k9_k10_on(void);
thaisenRelayUEn thaisen_relay_k9k10_off(void);
thaisenRelayEn thaisen_relay_A_FB_Z(void);
thaisenRelayEn thaisen_relay_A_FB_F(void);
thaisenRelayEn thaisen_relay_B_FB_Z(void);
thaisenRelayEn thaisen_relay_B_FB_F(void);
thaisenRelayEn thaisen_relay_parallel_FB_Z(void);
thaisenRelayEn thaisen_relay_parallel_FB_F(void);
thaisenRelayEn thaisen_relay_K7_FB(void);
thaisenRelayEn thaisen_relay_K8_FB(void);
thaisenRelayEn thaisen_relay_K9_FB(void);
thaisenRelayEn thaisen_relay_K10_FB(void);

/* 功能说明:
 *          thaisen_relay_AC_PositivePlus_Magnetic:磁保持继电器正脉冲控制
 * 输入参数:
 *          option   控制选项(0：拉低   1：拉高)
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
thaisenRelayUEn thaisen_relay_AC_PositivePlus_Magnetic(uint8_t option);

/* 功能说明:
 *          thaisen_relay_AC_NegtivePlus_Magnetic:磁保持继电器负脉冲控制
 * 输入参数:
 *          option   控制选项(0：拉低   1：拉高)
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
thaisenRelayUEn thaisen_relay_AC_NegtivePlus_Magnetic(uint8_t option);

/* 功能说明:
 *          thaisen_relay_LightingLamp_on:照明灯打开
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
thaisenRelayUEn thaisen_relay_LightingLamp_on(void);

/* 功能说明:
 *          thaisen_relay_LightingLamp_off:照明灯关闭
 * 输入参数:
 *
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
thaisenRelayUEn thaisen_relay_LightingLamp_off(void);

thaisenRelayEn thaisen_dcrelayA_pfeedback(void);
thaisenRelayEn thaisen_dcrelayA_nfeedback(void);
thaisenRelayEn thaisen_dcrelayB_pfeedback(void);
thaisenRelayEn thaisen_dcrelayB_nfeedback(void);
thaisenRelayEn thaisen_parallel_relay_pfeedback(void);
thaisenRelayEn thaisen_parallel_relay_nfeedback(void);
thaisenRelayEn thaisen_relayK7_feedback(void);
thaisenRelayEn thaisen_relayK8_feedback(void);
thaisenRelayEn thaisen_relayK9_feedback(void);
thaisenRelayEn thaisen_relayK10_feedback(void);

void thaisenSetACRelayCloseStaus(uint8_t sta);
uint8_t thaisenGetACRelayCloseStaus(void);

void thaisenSetDCRelayACloseStaus(uint8_t sta);
uint8_t thaisenGetDCRelayACloseStaus(void);

void thaisenSetDCRelayBCloseStaus(uint8_t sta);
uint8_t thaisenGetDCRelayBCloseStaus(void);

void thaisenSetParaRelayCloseStaus(uint8_t sta);
uint8_t thaisenGetParaRelayCloseStaus(void);

typedef enum{
    THADRV_ACRELAY_TYPE_NORMAL,                       /** 交流接触器类型：正常的 */
    THADRV_ACRELAY_TYPE_MAGNETIC,                     /** 交流接触器类型：磁保持的 */
    THADRV_ACRELAY_TYPE_SIZE,                         /** 交流接触器类型： */
}thaDrv_ACRelayType_t;

/* 功能说明:
 *          thaisenSetACRelayType:设置交流接触器类型
 * 输入参数:
 *          type   交流接触器类型
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
void thaisenSetACRelayType(uint8_t type);

/* 功能说明:
 *          thaisenGetACRelayType:获取交流接触器类型
 * 输入参数:
 *
 * 返回参数:       交流接触器类型
 *
 * 调用方法:
 *          实时调用
 */
uint8_t thaisenGetACRelayType(void);

typedef enum
{
    thaisenAcRelayIoEn_AcRelay,               //交流接触器控制IO作为:交流接触器控制使用
    thaisenAcRelayIoEn_Fan,                   //交流接触器控制IO作为:风扇控制使用
    thaisenAcRelayIoEn_Null,                  //交流接触器控制IO作为:不做任何使用
    thaisenAcRelayIoEn_Size,                  //交流接触器控制IO作为:
}thaisenAcRelayIoEn_enum;

/* 功能说明:
 *      thaisenSetACRelayEnableState:设置交流接触器使能状态
 * 输入参数:  状态：0：不使能    其它：使能
 *
 * 返回参数:
 *      无
 * 调用方法:
 *      反馈时调用
 */
void thaisenSetACRelayIoEnableState(thaisenAcRelayIoEn_enum en);

/* 功能说明:
 *      thaisenGetACRelayIoEnableState:获取交流接触器使能状态
 * 输入参数:
 *
 * 返回参数:    状态：0：不使能    1：使能
 *      无
 * 调用方法:
 *      反馈时调用
 */
thaisenAcRelayIoEn_enum thaisenGetACRelayIoEnableState(void);


/* 功能说明:
 *          thaisenDcRelay_StateQuery:查询直流继电器状态
 * 输入参数:
 *          gunNum      枪号
 *          sta         期望状态
 * 返回参数:
 *          1：与期望状态相符      0：与期望状态不符
 * 调用方法:
 *          实时调用
 */
uint8_t thaisenDcRelay_StateQuery(uint8_t gunNum, thaisenRelayEn sta);

/* 功能说明:
 *          thaisenAcRelay_StateQuery:查询交流接触器状态
 * 输入参数:
 *
 * 返回参数:
 *          @thaisenRelayEn
 * 调用方法:
 *          实时调用
 */
thaisenRelayEn thaisenAcRelay_StateQuery(void);

/******************************************* 以下是调试函数 *******************************************/
/************* 直流继电器 *************/
void thaisenDcRelay_A_Enable_Debug(void);
void thaisenDcRelay_A_Disable_Debug(void);

void thaisenDcRelay_B_Enable_Debug(void);
void thaisenDcRelay_B_Disable_Debug(void);

/************* 母联继电器 *************/
void thaisenParallelRelay_1_Enable_Debug(void);
void thaisenParallelRelay_1_Disable_Debug(void);

void thaisenParallelRelay_2_Enable_Debug(void);
void thaisenParallelRelay_2_Disable_Debug(void);

void thaisenParallelRelay_3_Enable_Debug(void);
void thaisenParallelRelay_3_Disable_Debug(void);

/************* 交流接触器 *************/
void thaisenAcRelay_Enable_Debug(void);
void thaisenAcRelay_Disable_Debug(void);

/*****************************************************************************************************/

/*************************************器件状态变化******************************************************/
/** 器件枚举 */
typedef enum
{
    THAISEN_DEVICE_ENUM_DCRELAY,                          /** 器件枚举：直流继电器 */
    THAISEN_DEVICE_ENUM_ACRELAY,                          /** 器件枚举：交流接触器 */
    THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_0,             /** 器件枚举：母联继电器0正极 */
    THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_0,             /** 器件枚举：母联继电器0负极 */
    THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_1,             /** 器件枚举：母联继电器1正极 */
    THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_1,             /** 器件枚举：母联继电器1负极 */
    THAISEN_DEVICE_ENUM_POS_PARALLEL_RELAY_2,             /** 器件枚举：母联继电器2正极 */
    THAISEN_DEVICE_ENUM_NEG_PARALLEL_RELAY_2,             /** 器件枚举：母联继电器2负极 */
    THAISEN_DEVICE_ENUM_AUXPOWER_12V,                     /** 器件枚举：12V辅源 */
    THAISEN_DEVICE_ENUM_AUXPOWER_24V,                     /** 器件枚举：24V辅源 */
    THAISEN_DEVICE_ENUM_ELOCK,                            /** 器件枚举：电子锁 */
    THAISEN_DEVICE_ENUM_FAN,                              /** 器件枚举：风扇 */
    THAISEN_DEVICE_ENUM_LIQUID,                           /** 器件枚举：液冷 */
    THAISEN_DEVICE_ENUM_SIZE,                             /** 器件枚举 */
}thaisenDeviceEnum;

/** 器件所执行的操作 */
typedef enum
{
    THAISEN_DEVICE_OPT_CONTROL,                           /** 器件操作枚举：控制 */
    THAISEN_DEVICE_OPT_RELEASE,                           /** 器件操作枚举：控制 */
    THAISEN_DEVICE_OPT_DEBUG_CONTROL,                     /** 器件操作枚举：控制 */
    THAISEN_DEVICE_OPT_DEBUG_RELEASE,                     /** 器件操作枚举：控制 */
    THAISEN_DEVICE_OPT_SIZE,                              /** 器件操作枚举 */
}thaisenDeviceOptEnum;

/** 直流继电器操作结果 */
typedef enum
{
    THAISEN_DEV_DCRELAY_RESULT_SUCCESS_0,                 /** 直流继电器器件操作结果：第一次操作成功 */
    THAISEN_DEV_DCRELAY_RESULT_SUCCESS_1,                 /** 直流继电器器件操作结果：第二次操作成功 */
    THAISEN_DEV_DCRELAY_RESULT_SUCCESS_2,                 /** 直流继电器器件操作结果：第三次操作成功 */
    THAISEN_DEV_DCRELAY_RESULT_PRESS_SCRAM,               /** 直流继电器器件操作结果：操作成功-急停按下 */
    THAISEN_DEV_DCRELAY_RESULT_OPT_DIRECTLT,              /** 直流继电器器件操作结果：直接操作，不检反馈 */
    THAISEN_DEV_DCRELAY_RESULT_FAIL,                      /** 直流继电器器件操作结果：操作失败 */
    THAISEN_DEV_DCRELAY_RESULT_SIZE,                      /** 直流继电器器件操作结果：无 */
}thaisenDeviceDCRelayOptResult;

/** 母联继电器操作结果 */
typedef enum
{
    THAISEN_DEV_PARARELAY_RESULT_SUCCESS_0,               /** 母联继电器器件操作结果：第一次操作成功 */
    THAISEN_DEV_PARARELAY_RESULT_SUCCESS_1,               /** 母联继电器器件操作结果：第二次操作成功 */
    THAISEN_DEV_PARARELAY_RESULT_SUCCESS_2,               /** 母联继电器器件操作结果：第三次操作成功 */
    THAISEN_DEV_PARARELAY_RESULT_PRESS_SCRAM,             /** 母联继电器器件操作结果：操作成功-急停按下 */
    THAISEN_DEV_PARARELAY_RESULT_OPT_DIRECTLT,            /** 母联继电器器件操作结果：直接操作，不检反馈 */
    THAISEN_DEV_PARARELAY_RESULT_FAIL,                    /** 母联继电器器件操作结果：操作失败 */
    THAISEN_DEV_PARARELAY_RESULT_SIZE,                    /** 母联继电器器件操作结果：无 */
}thaisenDeviceParaRelayOptResult;

/** 交流接触器操作结果 */
typedef enum
{
    THAISEN_DEV_ACRELAY_RESULT_SUCCESS_0,                 /** 交流接触器器件操作结果：第一次操作成功(正常模式) */
    THAISEN_DEV_ACRELAY_RESULT_SUCCESS_1,                 /** 交流接触器器件操作结果：第二次操作成功(正常模式)  */
    THAISEN_DEV_ACRELAY_RESULT_SUCCESS_2,                 /** 交流接触器器件操作结果：第三次操作成功(正常模式)  */
    THAISEN_DEV_ACRELAY_RESULT_OPT_DIRECTLT,              /** 交流接触器器件操作结果：直接操作，不检反馈(正常模式)  */
    THAISEN_DEV_ACRELAY_RESULT_SUCCESS_MAGNRTIC,          /** 交流接触器器件操作结果：操作成功(磁保持模式) */
    THAISEN_DEV_ACRELAY_RESULT_FAIL_FB_N,                 /** 交流接触器器件操作结果：操作失败-反馈不对(正常模式)  */
    THAISEN_DEV_ACRELAY_RESULT_FAIL_FB_M,                 /** 交流接触器器件操作结果：操作失败-反馈不对(磁保持模式)  */
    THAISEN_DEV_ACRELAY_RESULT_FAIL_NORMAL,               /** 交流接触器器件操作结果：操作失败-不是正常模式 */
    THAISEN_DEV_ACRELAY_RESULT_FAIL_MAGNRTIC,             /** 交流接触器器件操作结果：操作失败-不是磁保持模式 */
    THAISEN_DEV_ACRELAY_RESULT_SIZE,                      /** 交流接触器器件操作结果：无 */
}thaisenDeviceACRelayOptResult;

/** 电子锁操作结果 */
typedef enum
{
    THAISEN_DEV_ELOCK_RESULT_SUCCESS_0,                   /** 电子锁器件操作结果：第一次操作成功 */
    THAISEN_DEV_ELOCK_RESULT_SUCCESS_1,                   /** 电子锁器件操作结果：第二次操作成功 */
    THAISEN_DEV_ELOCK_RESULT_SUCCESS_2,                   /** 电子锁器件操作结果：第二次操作成功 */
    THAISEN_DEV_ELOCK_RESULT_OPT_DIRECTLT,                /** 电子锁器件操作结果：直接操作，不检反馈 */
    THAISEN_DEV_ELOCK_RESULT_FAIL,                        /** 电子锁器件操作结果：操作失败 */
    THAISEN_DEV_ELOCK_RESULT_SIZE,                        /** 电子锁器件操作结果：无 */
}thaisenDeviceELockOptResult;

/** 风扇操作结果 */
typedef enum
{
    THAISEN_DEV_FAN_RESULT_SUCCESS,                      /** 风扇器件操作结果：操作成功 */
    THAISEN_DEV_FAN_RESULT_SIZE,                         /** 风扇器件操作结果：无 */
}thaisenDeviceFanOptResult;

/** 辅源操作结果 */
typedef enum
{
    THAISEN_DEV_AUXPOWER_RESULT_SUCCESS,                 /** 辅源器件操作结果：操作成功 */
    THAISEN_DEV_AUXPOWER_RESULT_SIZE,                    /** 辅源器件操作结果：无 */
}thaisenDeviceAuxPowerOptResult;

/** 液冷操作结果 */
typedef enum
{
    THAISEN_DEV_LIQUID_RESULT_SUCCESS,                  /** 液冷器件操作结果：操作成功 */
    THAISEN_DEV_LIQUID_RESULT_FAIL_OFFLINE,             /** 液冷器件操作结果：操作失败-离线 */
    THAISEN_DEV_LIQUID_RESULT_SIZE,                     /** 液冷器件操作结果：无 */
}thaisenDeviceLiquidOptResult;

/** 器件变化参数 */
typedef struct
{
    uint8_t opt;                                          /** 所执行的操作(0：控制，1：释放，2：调试控制，3：调试释放) */
    uint8_t result;                                       /** 操作结果 */
}thaisenDeviceParameter;

/* 功能说明:
 *          thaisenDeviceChangedCallbackRegister:器件状态变化处理回调注册
 * 输入参数:           cb    回调
 *
 * 返回参数:
 *
 * 调用方法:
 *          上电初始化调用
 */
void thaisenDeviceChangedCallbackRegister(void *cb);

/*****************************************************************************************************/

/*************************************辅助电源函数******************************************************/

/* 功能说明:
 *   辅助电源是否上电或下电成功
 *
 */
typedef enum thaisenAuxPowerEnum
{
    thaisen_auxPower_ok,
    thaisen_auxPower_fail,
}thaisenAuxPowerEn;

typedef enum thaisenAuxPowerType
{
    thaisen_auxPowerType_12V,
    thaisen_auxPowerType_24V,
    thaisen_auxPowerType_size,
}thaisenAuxPowerTypeEn;

thaisenAuxPowerEn thaisen_auxPower_on_A(void);
thaisenAuxPowerEn thaisen_auxPower_off_A(void);

thaisenAuxPowerEn thaisen_auxPower_on_B(void);
thaisenAuxPowerEn thaisen_auxPower_off_B(void);

void thaisenSetAuxPowerTypeA(uint8_t type);
thaisenAuxPowerTypeEn thaisenGetAuxPowerTypeA(void);

void thaisenSetAuxPowerTypeB(uint8_t type);
thaisenAuxPowerTypeEn thaisenGetAuxPowerTypeB(void);

/******************************************** 以下是调试用函数 ********************************************/
/**************** 24V辅源控制24V ****************/
thaisenAuxPowerEn thaisenAux_A_24V_Enable_Debug(void);
thaisenAuxPowerEn thaisenAux_A_24V_Disable_Debug(void);
thaisenAuxPowerEn thaisenAux_B_24V_Enable_Debug(void);
thaisenAuxPowerEn thaisenAux_B_24V_Disable_Debug(void);

/**************** 12V辅源控制12V ****************/
thaisenAuxPowerEn thaisenAux_A_12V_Enable_Debug(void);
thaisenAuxPowerEn thaisenAux_A_12V_Disable_Debug(void);
thaisenAuxPowerEn thaisenAux_B_12V_Enable_Debug(void);
thaisenAuxPowerEn thaisenAux_B_12V_Disable_Debug(void);

/**************** 辅源反馈 ****************/

thaisenAuxPowerEn thaisenGetAux_A_Status_Debug(void);
thaisenAuxPowerEn thaisenGetAux_B_Status_Debug(void);

/*****************************************************************************************************/

/*************************************风机控制函数******************************************************/

void thaisen_fan_A_on(void);
void thaisen_fan_A_off(void);
void thaisen_fan_B_on(void);
void thaisen_fan_B_off(void);


/****************************************************************************/

/**********************************CC1检测**************************************************************/
enum
{
    thaisenCC1_12v,
    thaisenCC1_6v,
    thaisenCC1_4v,
    thaisenCC1_0v,
};

/* 功能说明:
 *          thaisen_get_CC1_status:获取CC1状态值
 *
 * 输入参数:
 *          无
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
uint8_t thaisen_get_CC1_status(void);
uint8_t thaisen_get_CC1_statusB(void);

typedef enum{
    THAISEN_GUIDANCE_CHANGED_12V_TO_6V,                           /** 导引状态变化：12V-6V */
    THAISEN_GUIDANCE_CHANGED_12V_TO_4V,                           /** 导引状态变化：12V-4V */
    THAISEN_GUIDANCE_CHANGED_12V_TO_0V,                           /** 导引状态变化：12V-0V */
    THAISEN_GUIDANCE_CHANGED_6V_TO_12V,                           /** 导引状态变化：6V-12V */
    THAISEN_GUIDANCE_CHANGED_6V_TO_4V,                            /** 导引状态变化：6V-4V */
    THAISEN_GUIDANCE_CHANGED_6V_TO_0V,                            /** 导引状态变化：6V-0V */
    THAISEN_GUIDANCE_CHANGED_4V_TO_12V,                           /** 导引状态变化：4V-12V */
    THAISEN_GUIDANCE_CHANGED_4V_TO_6V,                            /** 导引状态变化：4V-6V */
    THAISEN_GUIDANCE_CHANGED_4V_TO_0V,                            /** 导引状态变化：4V-0V */
    THAISEN_GUIDANCE_CHANGED_0V_TO_12V,                           /** 导引状态变化：0V-12V */
    THAISEN_GUIDANCE_CHANGED_0V_TO_6V,                            /** 导引状态变化：0V-6V */
    THAISEN_GUIDANCE_CHANGED_0V_TO_4V,                            /** 导引状态变化：0V-4V */
    THAISEN_GUIDANCE_CHANGED_SIZE,                                /** 导引状态变化 */
}thaisenGuidanceChanged_t;

typedef struct{
    int voltage;                                                  /** 导引当前电压值(0.01V) */
    int voltage_last;                                             /** 导引前一次电压值(0.01V) */
    uint16_t channel_adc;                                         /** 当前ADC */
    uint16_t channel_adc_last;                                    /** 前一次ADC */
}thaisenGuidanceInfo_t;

/******************************************************************
 * 函数名            thaisen_GuidanceChangedCallback_Register
 * 功能               导引状态变化回调注册
 * 参数
 * 返回
 * 注      void (*GuidanceChanged)(thaisenGuidanceInfo_t info, uint8_t flag, uint8_t gunNum);
 *     info        导引变化信息@thaisenGuidanceInfo_t
 *     flag        导引变化标志@thaisenGuidanceChanged_t
 *     gunNum      枪号
 *****************************************************************/
void thaisen_GuidanceChangedCallback_Register(void *cb);

/************************************************************* CC1 设置 CC1 *************************************************************/
/***********************************************************
 * 函数名           thaisen_set_CC12V_Uplimit
 * 功能               设置CC12V 上限
 * 参数               gunNum    枪号
 *        volt      CC12V电压值(0.001V)
 * 返回
 **********************************************************/
void thaisen_set_CC12V_Uplimit(uint8_t gunNum, int16_t volt);
/***********************************************************
 * 函数名           thaisen_set_CC12V_Lowlimit
 * 功能               设置CC12V 下限
 * 参数               gunNum    枪号
 *        volt      CC12V电压值(0.001V)
 * 返回
 **********************************************************/
void thaisen_set_CC12V_Lowlimit(uint8_t gunNum, int16_t volt);

/***********************************************************
 * 函数名           thaisen_set_CC6V_Uplimit
 * 功能               设置CC6V 上限
 * 参数               gunNum    枪号
 *        volt      CC6V电压值(0.001V)
 * 返回
 **********************************************************/
void thaisen_set_CC6V_Uplimit(uint8_t gunNum, int16_t volt);
/***********************************************************
 * 函数名           thaisen_set_CC6V_Lowlimit
 * 功能               设置CC6V 下限
 * 参数               gunNum    枪号
 *        volt      CC6V电压值(0.001V)
 * 返回
 **********************************************************/
void thaisen_set_CC6V_Lowlimit(uint8_t gunNum, int16_t volt);

/***********************************************************
 * 函数名           thaisen_set_CC4V_Uplimit
 * 功能               设置CC4V 上限
 * 参数               gunNum    枪号
 *        volt      CC4V电压值(0.001V)
 * 返回
 **********************************************************/
void thaisen_set_CC4V_Uplimit(uint8_t gunNum, int16_t volt);
/***********************************************************
 * 函数名           thaisen_set_CC4V_Lowlimit
 * 功能               设置CC4V 下限
 * 参数               gunNum    枪号
 *        volt      CC4V电压值(0.001V)
 * 返回
 **********************************************************/
void thaisen_set_CC4V_Lowlimit(uint8_t gunNum, int16_t volt);

/************************************************************* CC1 获取 CC1 *************************************************************/
/***********************************************************
 * 函数名           thaisen_get_CC12V_Uplimit
 * 功能               获取CC12V 上限
 * 参数               gunNum    枪号
 * 返回               CC12V电压值(0.001V)
 **********************************************************/
int16_t thaisen_get_CC12V_Uplimit(uint8_t gunNum);
/***********************************************************
 * 函数名           thaisen_get_CC12V_Lowlimit
 * 功能               获取CC12V 下限
 * 参数               gunNum    枪号
 * 返回               CC12V电压值(0.001V)
 **********************************************************/
int16_t thaisen_get_CC12V_Lowlimit(uint8_t gunNum);

/***********************************************************
 * 函数名           thaisen_get_CC6V_Uplimit
 * 功能               获取CC6V 上限
 * 参数               gunNum    枪号
 * 返回               CC6V电压值(0.001V)
 **********************************************************/
int16_t thaisen_get_CC6V_Uplimit(uint8_t gunNum);
/***********************************************************
 * 函数名           thaisen_get_CC6V_Lowlimit
 * 功能               获取CC6V 下限
 * 参数               gunNum    枪号
 * 返回               CC6V电压值(0.001V)
 **********************************************************/
int16_t thaisen_get_CC6V_Lowlimit(uint8_t gunNum);

/***********************************************************
 * 函数名           thaisen_get_CC4V_Uplimit
 * 功能               获取CC4V 上限
 * 参数               gunNum    枪号
 * 返回               CC4V电压值(0.001V)
 **********************************************************/
int16_t thaisen_get_CC4V_Uplimit(uint8_t gunNum);
/***********************************************************
 * 函数名           thaisen_get_CC4V_Lowlimit
 * 功能               获取CC4V 下限
 * 参数               gunNum    枪号
 * 返回               CC4V电压值(0.001V)
 **********************************************************/
int16_t thaisen_get_CC4V_Lowlimit(uint8_t gunNum);

/****************************************************************************/

/*******************************充电枪头温度检测**********************************/
/* 功能说明:
 *          thaisen_get_DCPos_temp:获取DC+枪头温度
 *                  精度:0.1
 * 输入参数:
 *          无
 * 返回参数:
 *          温度值
 * 调用方法:
 *          实时调用
 */

int thaisen_get_DCPos_tempA(void);
int thaisen_get_DCPos_tempB(void);
/* 功能说明:
 *          thaisen_get_DCCat_temp:获取DC-枪头温度
 *                  精度:0.1
 * 输入参数:
 *          无
 * 返回参数:
 *          温度值
 * 调用方法:
 *          实时调用
 */

int thaisen_get_DCCat_tempA(void);
int thaisen_get_DCCat_tempB(void);


/****************************************************************************/


/**********************************LED控制***********************************************************/

/* 功能说明:
 *      thaisen_led_board_on:板载运行灯点亮
 * 输入参数:
 *          无
 * 返回参数:
 *      无
 * 调用方法:
 *      实时调用
 */
void thaisen_led_board_on(void);

/* 功能说明:
 *      thaisen_led_board_off:板载运行灯熄灭
 * 输入参数:
 *          无
 * 返回参数:
 *      无
 * 调用方法:
 *      实时调用
 */
void thaisen_led_board_off(void);

/* 功能说明:
 *      thaisen_led_board_toggle:板载运行灯状态反转
 * 输入参数:
 *          无
 * 返回参数:
 *      无
 * 调用方法:
 *      实时调用
 */
void thaisen_led_board_toggle(void);


void thaisen_led_blue_A_on(void);
void thaisen_led_blue_A_off(void);

void thaisen_led_green_A_on(void);
void thaisen_led_green_A_off(void);


void thaisen_led_red_A_on(void);
void thaisen_led_red_A_off(void);



void thaisen_led_blue_B_on(void);
void thaisen_led_blue_B_off(void);

void thaisen_led_green_B_on(void);
void thaisen_led_green_B_off(void);


void thaisen_led_red_B_on(void);
void thaisen_led_red_B_off(void);


/****************************************************************************/


/**************************************CAN总线***********************************************************/
typedef struct
{
    uint32_t CANID;
    uint8_t  data[8];
    uint8_t  length;
    uint8_t  priority;
    CAN_HandleTypeDef Message;
    uint32_t DLC;
    uint8_t newFlag;
}can_msg_buf;


/* 功能说明:
 *      TH_MSCAN1_Transmit:CAN总线数据发送
 *          充电模块CAN:波特率为125K
 * 输入参数:
*           can_msg_buf:发送数据结构
 * 返回参数:
 *      无
 * 调用方法:
 *      发送时调用
 */

void thaisen_chargmodule_can_send(can_msg_buf* pbuffer);


/* 功能说明:
 *      TH_MSCAN2_Transmit:CAN总线数据发送
 *          BMS交互CAN:波特率为250K
 * 输入参数:
*           can_msg_buf:发送数据结构
 * 返回参数:
 *      无
 * 调用方法:
 *      发送时调用
 */
void thaisen_bmsA_can_send(can_msg_buf* pbuffer);
void thaisen_bmsB_can_send(can_msg_buf* pbuffer);


/* 功能说明:
 *      TH_MSCAN4_Transmit:CAN总线数据发送
 *          充电模块CAN:波特率为125K
 * 输入参数:
*           can_msg_buf:发送数据结构
 * 返回参数:
 *      无
 * 调用方法:
 *      发送时调用
 */

void thaisen_tcu_can_send(can_msg_buf* pbuffer);



/* 功能说明:
 *      thaisen_user_chargModule_isrCallback:充电模块CAN数据中断函数,此函数需在应用层重写实现,配合thaisen_get_chargModule_msg函数使用
 * 输入参数:
*               无
 * 返回参数:
 *      无
 * 调用方法:
 *      无需调用
 */
void thaisen_user_chargModule_isrCallback(void);


/* 功能说明:
 *      thaisen_can_bms1_isrCallback模块CAN数据中断函数,此函数需在应用层重写实现,thaisen_get_BMS_msg函数使用
 * 输入参数:
*               无
 * 返回参数:
 *      无
 * 调用方法:
 *      无需调用
 */
void thaisen_can_bmsA_isrCallback(void);
void thaisen_can_bmsB_isrCallback(void);

/* 功能说明:
 *      thaisen_get_chargModule_msg:充电模块CAN数据
 * 输入参数:
*               无
 * 返回参数:
 *      can_msg_buf:帧数据
 * 调用方法:
 *      在thaisen_user_chargModule_isrCallback函数内调用
 */
can_msg_buf *thaisen_get_can_charg_module_dat(void);



/* 功能说明:
 *      thaisen_get_BMS_msg:BMS模块CAN数据
 * 输入参数:
*               无
 * 返回参数:
 *      can_msg_buf:帧数据
 * 调用方法:
 *      thaisen_user_BMS_isrCallback函数内调用
 */
can_msg_buf thaisen_get_can_bmsA_dat(void);
can_msg_buf thaisen_get_can_bmsB_dat(void);

/* 功能说明:
 *      thaisen_get_tcu_dat TCUCAN数据
 * 输入参数:
*               无
 * 返回参数:
 *      can_msg_buf:帧数据
 * 调用方法:
 *      实时调用
 */
can_msg_buf thaisen_get_tcu_dat(void);

typedef enum{
    THAISEN_BMS_A_CAN_ENUM,       //BMS A CAN
    THAISEN_BMS_B_CAN_ENUM,       //BMS B CAN
    THAISEN_TCU_CAN_ENUM,         //TCU CAN
    THAISEN_MODULE_CAN_ENUM,      //充电模块 CAN
    THAISEN_CAN_ENUM_SIZE,
}thaisenIsCANEnum;

/* 功能说明:
 *      thaisen_is_can_recved 查询CAN是否接收到了数据
 * 输入参数:
*              en CAN 枚举
 * 返回参数:
 *             1：已接收到   0：未接收到
 * 调用方法:
 *      CAN 空闲时调用
 */
unsigned char thaisen_is_can_recved(thaisenIsCANEnum en);

/* 功能说明:
 *      thaisen_clear_can_recved 清除CAN接收数据标志
 * 输入参数:
*              en CAN 枚举
 * 返回参数:
 *
 * 调用方法:
 *      CAN 空闲时调用
 */
void thaisen_clear_can_recved(thaisenIsCANEnum en);

/* 功能说明:
 *      thaisen_user_can_cb_register 应用层CAN数据接收回调函数注册
 * 输入参数:
*              en CAN 枚举
*              cb 回调句柄
 * 返回参数:
 *
 * 调用方法:
 *      上电时注册，类型为：void (*)(can_msg_buf*)
 */
void thaisen_user_can_cb_register(thaisenIsCANEnum en, void *cb);

/*******************************************************************************/


/*****************************电表函数*****************************************/
enum
{
    thaisenAmmeterModel_RuiYin,
    thaisenAmmeterModel_YaDa,
    thaisenAmmeterModel_KeDaRui,
    thaisenAmmeterModel_YingLiDa,
    thaisenAmmeterModel_AnKeRui,
    thaisenAmmeterModel_KeWei,
    thaisenAmmeterModel_Other,
};


/* 功能说明:
 *          thaisen_get_ammeterVolt:获取电表电压
 *                  分辨率:0.01
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          电压值
 * 调用方法:
 *          实时调用
 */
int32_t thaisen_get_ammeterVolt(uint8_t gunNum);



/* 功能说明:
 *          thaisen_get_ammeterCurrent:获取电表电流
 *                  分辨率:0.0001
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          电流值
 * 调用方法:
 *          实时调用
 */
int32_t thaisen_get_ammeterCurrent(uint8_t gunNum);



/* 功能说明:
 *          thaisen_get_ammeterPower:获取电表功率
 *                  分辨率:0.1
 *                  单位:w
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          功率值
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_ammeterPower(uint8_t gunNum);


/* 功能说明:
 *          thaisen_get_ammeterEnergy:获取电表总计电量
 *                  分辨率:0.001
 *                  单位:kW.h
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          电量值
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_ammeterEnergy(uint8_t gunNum);

/* 功能说明:
 *          thaisen_get_ammeterReverseEnergy:获取电表反向总计电量
 *                  分辨率:0.001
 *                  单位:kW.h
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          电量值
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_ammeterReverseEnergy(uint8_t gunNum);

/* 功能说明:
 *          thaisen_set_ammnterModel:设置电表型号
 * 输入参数:
 *                  model:型号
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
void thaisen_set_ammnterModel(uint8_t model);

/* 功能说明:
 *          thaisen_get_ammnterModel:获取电表型号
 * 输入参数:
 *
 * 返回参数:
 *          电表型号
 * 调用方法:
 *          实时调用
 */
uint8_t thaisen_get_ammnterModel(void);

/* 功能说明:
 *          thaisen_set_gunVolt:设置枪端电压(用于无电表时)
 * 输入参数:           volt   电压(0.1)
 *          gunNum  枪号
 *
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
void thaisen_set_gunVolt(uint32_t volt, uint8_t gunNum);

/* 功能说明:
 *          thaisen_get_gunVolt:获取枪端电压(用于无电表时)
 * 输入参数:           gunNum  枪号
 *
 * 返回参数:
 *                             枪端电压(0.1)
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_gunVolt(uint8_t gunNum);

/* 功能说明:
 *          thaisen_set_gunCurr:设置枪端电流(用于无电表时)
 * 输入参数:           curr   电流(0.1)
 *          gunNum  枪号
 *
 * 返回参数:
 *
 * 调用方法:
 *          实时调用
 */
void thaisen_set_gunCurr(uint32_t curr, uint8_t gunNum);

/* 功能说明:
 *          thaisen_get_gunCurr:获取枪端电流(用于无电表时)
 * 输入参数:           gunNum  枪号
 *
 * 返回参数:
 *                             枪端电流(0.1)
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_gunCurr(uint8_t gunNum);

/* 电表串口校验方式 */
typedef enum
{
    THAISEN_CHECK_WAY_NONE,
    THAISEN_CHECK_WAY_EVEN,
    THAISEN_CHECK_WAY_ODD,
    THAISEN_CHECK_WAY_SIZE,
}thaisenCheckWayEnum;
/* 电表串口通信波特率 */
typedef enum
{
    THAISEN_BAUDRATE_2400,
    THAISEN_BAUDRATE_4800,
    THAISEN_BAUDRATE_9600,
    THAISEN_BAUDRATE_19200,
    THAISEN_BAUDRATE_38400,
    THAISEN_BAUDRATE_57600,
    THAISEN_BAUDRATE_115200,
    THAISEN_BAUDRATE_SIZE,
}thaisenBaudrateEnum;

/* 功能说明:
 *          thaisen_set_ammeterCheckWay:设置电表校验方式
 * 输入参数:
 *
 * 返回参数:
 *          校验方式枚举
 * 调用方法:
 *          实时调用
 */
void thaisen_set_ammeterCheckWay(thaisenCheckWayEnum way);

/* 功能说明:
 *          thaisen_get_ammeterCheckWay:获取电表校验方式
 * 输入参数:
 *
 * 返回参数:
 *          校验方式枚举
 * 调用方法:
 *          实时调用
 */
thaisenCheckWayEnum thaisen_get_ammeterCheckWay(void);

/* 功能说明:
 *          thaisen_set_ammeterBaudrate:设置电表通信波特率
 * 输入参数:
 *
 * 返回参数:
 *          波特率枚举
 * 调用方法:
 *          实时调用
 */
void thaisen_set_ammeterBaudrate(thaisenBaudrateEnum baudrate);

/* 功能说明:
 *          thaisen_get_ammeterBaudrate:获取电表通信波特率
 * 输入参数:
 *
 * 返回参数:
 *          波特率枚举
 * 调用方法:
 *          实时调用
 */
thaisenBaudrateEnum thaisen_get_ammeterBaudrate(void);

/*****************************************************************************************************/

/*********************************FLASH***************************************************************/
//设备端信息基地址，暂未启用,64k
#define THAISEN_FLASH_DEVICE_ADDRESSS  0

//远程升级包信息基地址,64k
#define THAISEN_FLASH_UPDATE_PACKAGE_INFO_ADDRESS     0x10000

//远程升级文件存储基地址,2M
#define THAISEN_FLASH_UPDATE_PACKAGE_DATA_ADDESSS     0x20000

//历史故障存储基地址,64k
#define THAISEN_FLASH_HISTORY_FAULT_INFO_ADDRESS      0x220000

//历史订单存储基地址,64k
#define THAISEN_FLASH_HISTORY_ORDER_INFO_ADDRESS      0x230000

//未结算订单存储基地址,64k
#define THAISEN_FLASH_NO_ACCOUNT_ORDER_INFO_ADDRESS   0x240000


//订单预留1存储基地址,64k
#define THAISEN_FLASH_ORDER1_INFO_ADDRESS   0x250000


//订单预留2存储基地址,64k
#define THAISEN_FLASH_ORDER2_INFO_ADDRESS   0x260000


//订单预留3存储基地址,64k
#define THAISEN_FLASH_ORDER3_INFO_ADDRESS   0x270000


//费率存储基地址,64k
#define THAISEN_FLASH_RATE_INFO_ADDRESS   0x280000

//费率预留1存储基地址,64k
#define THAISEN_FLASH_RATE_RESERVED1_INFO_ADDRESS   0x290000

//费率预留2存储基地址,64k
#define THAISEN_FLASH_RATE_RESERVED2_INFO_ADDRESS   0x2A0000

//扩展地址基地址,即自定义数据，尽量不使用
#define THAISEN_FLASH_CUSTOM_ADDRESS   0x2B0000


/* 功能说明:
 *          thaisenW25qxxReadID:获取FLASH的ID
 *
 * 输入参数:
 *
 * 返回参数:
 *         unsigned short:ID号
 * 调用方法:
 *         获取时调用
 */
unsigned short thaisenW25qxxReadID(void);

/* 功能说明:
 *          thaisenW25qxxWrite:FLASH写数据
 *
 * 输入参数:
 *         pBuffer:写入数据的缓冲区
 *         WriteAddr:写入FLASH地址
 *         NumByteToWrite:写入的字节数
 * 返回参数:
 *         无
 * 调用方法:
 *          写入时调用
 */
 void thaisenW25qxxWrite(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite);


 /* 功能说明:
  *          thaisenW25qxxRead:FLASH数据读出
  *
  * 输入参数:
  *         pBuffer:读出数据的缓冲区
  *         WriteAddr:读出FLASH地址
  *         NumByteToWrite:读出的字节数
  * 返回参数:
  *         无
  * 调用方法:
  *          读取时调用
  */
 void thaisenW25qxxRead(unsigned char* pBuffer,unsigned int ReadAddr,unsigned short NumByteToRead);


 /* 功能说明:
  *          thaisenW25qxxWriteNoCheck：写入FLASH数据,无擦除环节
  *
  * 输入参数:
  *         pBuffer:写入数据的缓冲区
  *         WriteAddr:写入FLASH地址
  *         NumByteToWrite:写入的字节数
  * 返回参数:
  *         无
  * 调用方法:
  *          读取时调用
  */
 void thaisenW25qxxWriteNoCheck(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short NumByteToWrite);



 /* 功能说明:
  *          thaisenW25qxxErase：擦除数据
  *
  * 输入参数:
  *         Addr:擦除的地址
  *         NumByte:擦除的字节数
  *
  * 返回参数:
  *         无
  * 调用方法:
  *          读取时调用
  */
 void thaisenW25qxxErase(unsigned int Addr,unsigned int NumByte);

 /* 功能说明:
  *          thaisenIflashWriteDirectly：写入内部FLASH数据,无擦除环节
  *
  * 输入参数:
  *         buffer:写入数据的缓冲区
  *         address:写入FLASH地址
  *         size:写入的字节数
  *         nesting:是否与内部flah操作接口嵌套使用(涉及到解锁问题)
  * 返回参数:
  *         >= 0:成功，<0: 失败
  * 调用方法:
  *          读取时调用
  */
 int thaisenIflashWriteDirectly(unsigned int address, const unsigned char *buffer, int size, unsigned char nesting);

 /* 功能说明:
  *          thaisenIflashRead :内部FLASH数据读出
  *
  * 输入参数:
  *         buffer:读出数据的缓冲区
  *         address:读出FLASH地址
  *         size:读出的字节数
  * 返回参数:
  *         >= 0:成功，<0: 失败
  * 调用方法:
  *          读取时调用
  */
 int thaisenIflashRead(unsigned int address, unsigned char *buffer, int size);

 /* 功能说明:
  *          thaienIflashWrite:内部FLASH写数据
  *
  * 输入参数:
  *         address:写入数据的缓冲区
  *         buffer:写入FLASH地址
  *         size:写入的字节数
  * 返回参数:
  *         >= 0:成功，<0: 失败
  * 调用方法:
  *          写入时调用
  */
 int thaienIflashWrite(unsigned int address, const unsigned char *buffer, int size);

 /* 功能说明:
  *          thaisenIflashEraseSector：内部flash擦除数据
  *
  * 输入参数:
  *         address:擦除的地址
  *         size:擦除的字节数
  *         nesting:是否与内部flah操作接口嵌套使用(涉及到解锁问题)
  *
  * 返回参数:
  *         >= 0:成功，<0: 失败
  * 调用方法:
  *          读取时调用
  */
 int thaisenIflashEraseSector(unsigned int address, unsigned int size, unsigned char nesting);

 /******************************************************************************/


 /*****************************绝缘采样*****************************************/

 /* 功能说明:
  *          thasien_A_switch_relay_on：A枪投切继电器开关
  *          thasien_A_switch_relay_off:
  *
  * 输入参数:
  *
  * 返回参数:
  *         无
  * 调用方法:
  *          绝缘时切换
  */
 void thasien_A_switch_relay_on(void);
 void thasien_A_switch_relay_off(void);


 /* 功能说明:
  *          thasien_A_pos_relay_on：A枪正极对PE继电器开关
  *          thasien_A_pos_relay_off:
  *
  * 输入参数:
  *
  * 返回参数:
  *         无
  * 调用方法:
  *          绝缘时切换
  */
 void thasien_A_pos_relay_on(void);
 void thasien_A_pos_relay_off(void);

 /* 功能说明:
   *          thasien_A_cat_relay_on：A枪负极对PE继电器开关
   *          thasien_A_cat_relay_off:
   *
   * 输入参数:
   *
   * 返回参数:
   *         无
   * 调用方法:
   *          绝缘时切换
   */
 void thasien_A_cat_relay_on(void);
 void thasien_A_cat_relay_off(void);


 /* 功能说明:
  *          thasien_B_switch_relay_on：B枪投切继电器开关
  *          thasien_B_switch_relay_off:
  *
  * 输入参数:
  *
  * 返回参数:
  *         无
  * 调用方法:
  *          绝缘时切换
  */
 void thasien_B_switch_relay_on(void);
 void thasien_B_switch_relay_off(void);


 /* 功能说明:
  *          thasien_B_pos_relay_on：B枪正极对PE继电器开关
  *          thasien_B_pos_relay_off:
  *
  * 输入参数:
  *
  * 返回参数:
  *         无
  * 调用方法:
  *          绝缘时切换
  */
 void thasien_B_pos_relay_on(void);
 void thasien_B_pos_relay_off(void);


 /* 功能说明:
   *          thasien_A_cat_relay_on：B枪负极对PE继电器开关
   *          thasien_A_cat_relay_off:
   *
   * 输入参数:
   *
   * 返回参数:
   *         无
   * 调用方法:
   *          绝缘时切换
   */
 void thasien_B_cat_relay_on(void);
 void thasien_B_cat_relay_off(void);

 /* 功能说明:
   *          TH_get_A_Insult_Positive_PE_Volt:A枪的正极对PE电压值
   *
   * 输入参数:
   *
   * 返回参数:
   *         无
   * 调用方法:
   *          实时
   */
 int16_t TH_get_A_Insult_Positive_PE_Volt(void);
 int16_t TH_get_B_Insult_Positive_PE_Volt(void);
 /* 功能说明:
    *          TH_get_A_Insult_Cathode_PE_Volt:A枪的负极对PE电压值
    *
    * 输入参数:
    *
    * 返回参数:
    *         无
    * 调用方法:
    *          实时
    */
 int16_t TH_get_A_Insult_Cathode_PE_Volt(void);
 int16_t TH_get_B_Insult_Cathode_PE_Volt(void);

 /* 功能说明:
    *          TH_get_A_Insult_Volt:A枪的绝缘电压值
    *
    * 输入参数:
    *
    * 返回参数:
    *         无
    * 调用方法:
    *          实时
    */
 int16_t TH_get_A_Insult_Volt(void);
 int16_t TH_get_B_Insult_Volt(void);




 typedef enum thaisenInsultModeEnum
 {
    thaisen_insult_mode_enable,//使能绝缘
    thaisen_insult_mode_disable,//不使能绝缘
 }thaisenInsultModeEn;

 /* 功能说明:
  *          thaisen_set_insult_mode:设置绝缘模式
  * 输入参数:
  *         thaisenInsultModeEn:绝缘模式
  *         gunNum:枪号
  * 返回参数:
  *          无
  * 调用方法:
  *     系统初始设置
  */
 void thaisen_set_insult_mode(thaisenInsultModeEn status,uint8_t gunNum);

 /* 功能说明:
  *          thaisen_get_insult_mode:获取设置的绝缘模式
  * 输入参数:
  *
  *         gunNum:枪号
  * 返回参数:
  *          无
  * 调用方法:
  *             可实时调用
  */
 thaisenInsultModeEn thaisen_get_insult_mode(uint8_t gunNum);

 /****************************************************************************/


 /*****************************故障信息*****************************************/
 /* 功能说明:
  *          thaisenGetSysFault:查询系统故障信息
  * 输入参数:
  *
  * 返回参数:
  *          无
  * 调用方法:
  *          实时调用
  */

 uint32_t* thaisenGetSysFault(uint8_t gunNum);

 /* 功能说明:
  *          thaisenSetSysFaultLib:设置系统故障
  * 输入参数:
  *         thaisenFaultTy:故障类型
  *         gunNum:枪号
  * 返回参数:
  *          无
  * 调用方法:
  *          故障变化时调用
  */
 void thaisenSetSysFaultLib(thaisenFaultTy faultValue,uint8_t gunNum);

 /* 功能说明:
  *          thaisenClearSysFaultLib:清除系统故障
  * 输入参数:
  *         thaisenFaultTy:故障类型
  *         gunNum:枪号
  * 返回参数:
  *          无
  * 调用方法:
  *          故障变化时调用
  */
 void thaisenClearSysFaultLib(thaisenFaultTy faultValue,uint8_t gunNum);

 /* 功能说明:
  *          thaisenSetSysFaultCheckAllBit:故障检测使能,按照全位使能或清除
  *          例如:0x00000002:代表使能急停检测
  * 输入参数:
  *         uint32_t:故障码
  *         每个Bit代表一位故障
  *         gunNum     枪号
  *         set        故障集号
  * 返回参数:
  *          无
  * 调用方法:
  *          可实时调用
  */
 void thaisenSetSysFaultCheckAllBit(uint32_t faultValue, uint8_t gunNum, uint8_t set);

 /* 功能说明:
  *          thaisenSetSysFaultCheckBit:故障检测使能,按照BIT位使能
  *          例如:填入值为thaisenFaultTy类型中的当前值
  * 输入参数:
  *         uint32_t:故障码
  *         每个Bit代表一位故障
  *         gunNum      枪号
  * 返回参数:
  *          无
  * 调用方法:
  *          可实时调用
  */
 void thaisenSetSysFaultCheckBit(thaisenFaultTy faultValue, uint8_t gunNum);


 /* 功能说明:
  *          thaisenClearSysFaultCheckBit:故障检测使能,按照BIT位清除
  *          例如:填入值为thaisenFaultTy类型中的当前值
  * 输入参数:
  *         uint32_t:故障码
  *         每个Bit代表一位故障
  *         gunNum     枪号
  * 返回参数:
  *          无
  * 调用方法:
  *          可实时调用
  */
 void thaisenClearSysFaultCheckBit(thaisenFaultTy faultValue, uint8_t gunNum);


 /* 功能说明:
  *          thaisenGetSysFaultCheckBit:获取故障检测使能
  *
  * 输入参数:
  *          gunNum     枪号
  *          set        故障集号
  *
  * 返回参数:
  *          uint32_t
  * 调用方法:
  *          可实时调用
  */
 uint32_t thaisenGetSysFaultCheckBit(uint8_t gunNum, uint8_t set);


 /* 功能说明:
  *          thaisenGetSysFaultCheckEnBit:获取故障检测是否使能
  *
  * 输入参数:
  *          gunNum    枪号
  *
  * 返回参数:
  *          uint8_t
  * 调用方法:
  *          可实时调用
  */
 uint8_t thaisenGetSysFaultCheckEnBit(thaisenFaultTy faultBit, uint8_t gunNum);

 /* 功能说明:
  *          thaisenGetSysFaultSetNum:获取系统故障集数量

  * 返回参数:
  *          gunNum：枪号
  * 调用方法:
  *          空闲状态中调用
  */
 uint8_t thaisenGetSysFaultSetNum(uint8_t gunNum);

 /**********************************************************************************/
 /*****************************屏幕调试模式*****************************************/
 typedef enum thaisenDebugEnum
 {
     thaisen_debug_mode_exit,
     thaisen_debug_mode_enter,
 }thaisenDebugEn;

 void thaisen_set_debug_mode(thaisenDebugEn status);
 thaisenDebugEn thaisen_get_debug_mode(void);


 /**********************************************************************************/
 /*****************************FCT标定参数*****************************************/
 /* 功能说明:
  *          __thaisen_get_test_number:获取测试编码
  * 输入参数:
  *
  *        无
  * 返回参数:
  *          存储数据地址
  * 调用方法:
  *             可实时调用
  */
 uint8_t *__thaisen_get_test_number(void);

 /* 功能说明:
  *          __thaisen_get_test_time:获取测试时间
  * 输入参数:
  *
  *        无
  * 返回参数:
  *          存储数据地址
  * 调用方法:
  *             可实时调用
  */
 uint8_t *__thaisen_get_test_time(void);

 /* 功能说明:
  *          __thaisen_get_test_hard_version:获取测试硬件版本
  * 输入参数:
  *
  *        无
  * 返回参数:
  *          存储数据地址
  * 调用方法:
  *             可实时调用
  */
 uint8_t *__thaisen_get_test_hard_version(void);

 /*************************************输入、输出端口配置函数******************************************************/
 /* 功能说明:
  *          thaisenSetDCRelayOutPortA:设置A枪直流继电器输出端口
  * 输入参数:
  *         port:端口
  * 返回参数:
  *          无
  * 调用方法:
  *      实时调用
  */
void thaisenSetDCRelayOutPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetDCRelayOutPortA:获取A枪直流继电器输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪直流继电器输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetDCRelayOutPortA(void);

/* 功能说明:
 *          thaisenSetDCRelayOutPortB:设置B枪直流继电器输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetDCRelayOutPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetDCRelayOutPortB:获取B枪直流继电器输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪直流继电器输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetDCRelayOutPortB(void);

/* 功能说明:
 *          thaisenSetParaRelayOutPort:设置并联继电器输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetParaRelayOutPort(uint8_t port);
/* 功能说明:
 *          thaisenGetParaRelayOutPort:获取并联继电器输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         并联继电器输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetParaRelayOutPort(void);

/* 功能说明:
 *          thaisenSetACRelayOutPort:设置交流继电器输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetACRelayOutPort(uint8_t port);
/* 功能说明:
 *          thaisenGetACRelayOutPort:获取交流继电器输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         交流继电器输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetACRelayOutPort(void);


/* 功能说明:
 *          thaisenSetElectLockOutPortA:设置A枪电子锁输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetElectLockOutPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetElectLockOutPortA:获取A枪电子锁输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪电子锁输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetElectLockOutPortA(void);

/* 功能说明:
 *          thaisenSetElectLockOutPortB:设置B枪电子锁输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetElectLockOutPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetElectLockOutPortB:获取B枪电子锁输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪电子锁输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetElectLockOutPortB(void);

/* 功能说明:
 *          thaisenSetFanOutPort:设置风扇输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFanOutPort(uint8_t port);
/* 功能说明:
 *          thaisenGetFanOutPort:获取风扇输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         风扇输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFanOutPort(void);

/* 功能说明:
 *          thaisenSetAuxPowerOutPortA_12V:设置A枪12V辅源输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetAuxPowerOutPortA_12V(uint8_t port);
/* 功能说明:
 *          thaisenGetAuxPowerOutPortA_12V:获取A枪12V辅源输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪12V辅源输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetAuxPowerOutPortA_12V(void);

/* 功能说明:
 *          thaisenSetAuxPowerOutPortB_12V:设置B枪12V辅源输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetAuxPowerOutPortB_12V(uint8_t port);
/* 功能说明:
 *          thaisenGetAuxPowerOutPortB_12V:获取B枪12V辅源输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪12V辅源输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetAuxPowerOutPortB_12V(void);

/* 功能说明:
 *          thaisenSetAuxPowerOutPortA_24V:设置A枪24V辅源输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetAuxPowerOutPortA_24V(uint8_t port);
/* 功能说明:
 *          thaisenGetAuxPowerOutPortA_24V:获取A枪24V辅源输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪24V辅源输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetAuxPowerOutPortA_24V(void);

/* 功能说明:
 *          thaisenSetAuxPowerOutPortB_24V:设置B枪24V辅源输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetAuxPowerOutPortB_24V(uint8_t port);
/* 功能说明:
 *          thaisenGetAuxPowerOutPortB_24V:获取B枪24V辅源输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪24V辅源输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetAuxPowerOutPortB_24V(void);

/* 功能说明:
 *          thaisenSetReliefOutPortA: 设置A枪泄放输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetReliefOutPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetReliefOutPortA:获取A枪泄放输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪泄放输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetReliefOutPortA(void);

/* 功能说明:
 *          thaisenSetReliefOutPortB: 设置B枪泄放输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetReliefOutPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetReliefOutPortB:获取B枪泄放输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪泄放输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetReliefOutPortB(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingOutPortA: 设置A枪液冷输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingOutPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingOutPortA:获取A枪液冷输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪液冷输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingOutPortA(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingOutPortB: 设置B枪液冷输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingOutPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingOutPortB:获取B枪液冷输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪液冷输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingOutPortB(void);

 /******************************************** 输入口配置 **************************************************/
 /******************************************** 输入口配置 **************************************************/
/* 功能说明:
 *          thaisenSetDCRelayInPortA:设置A枪直流继电器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetDCRelayInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetDCRelayInPortA:获取A枪直流继电器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪直流继电器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetDCRelayInPortA(void);

/* 功能说明:
 *          thaisenSetDCRelayInPortB:设置B枪直流继电器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetDCRelayInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetDCRelayInPortB:获取B枪直流继电器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪直流继电器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetDCRelayInPortB(void);

/* 功能说明:
 *          thaisenSetParaRelayInPort:设置并联继电器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetParaRelayInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetParaRelayInPort:获取并联继电器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         并联继电器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetParaRelayInPort(void);

/* 功能说明:
 *          thaisenSetACRelayInPort:设置交流继电器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetACRelayInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetACRelayInPort:获取交流继电器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         并联继电器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetACRelayInPort(void);

/* 功能说明:
 *          thaisenSetScramInPort:设置急停输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetScramInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetScramInPort:获取急停输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         急停输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetScramInPort(void);

/* 功能说明:
 *          thaisenSetElectLockInPortA:设置A枪电子锁输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetElectLockInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetElectLockInPortA:获取A枪电子锁输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪电子锁输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetElectLockInPortA(void);

/* 功能说明:
 *          thaisenSetElectLockInPortB:设置B枪电子锁输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetElectLockInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetElectLockInPortB:获取B枪电子锁输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪电子锁输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetElectLockInPortB(void);

/* 功能说明:
 *          thaisenSetGateInPort:设置门禁输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetGateInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetGateInPort:获取门禁输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         门禁输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetGateInPort(void);

/* 功能说明:
 *          thaisenSetTravelSwitchInPort:设置行程开关输入端口号
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetTravelSwitchInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetTravelSwitchInPort:获取行程开关输入端口号
 * 输入参数:
 *         无
 * 返回参数:
 *         行程开关输入端口号
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetTravelSwitchInPort(void);

/* 功能说明:
 *          thaisenSetLightProtectorInPort:设置防雷器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLightProtectorInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetLightProtectorInPort:获取防雷器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         防雷器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLightProtectorInPort(void);

/* 功能说明:
 *          thaisenSetCircuitBreakerInPort:设置断路器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetCircuitBreakerInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetCircuitBreakerInPort:获取断路器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         断路器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetCircuitBreakerInPort(void);

/* 功能说明:
 *          thaisenSetFanInPort:设置风扇输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFanInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetFanInPort:获取风扇输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         风扇输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFanInPort(void);

/* 功能说明:
 *          thaisenSetFloodingInPort:设置水浸输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFloodingInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetFloodingInPort:获取水浸输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         水浸输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFloodingInPort(void);

/* 功能说明:
 *          thaisenSetSmokeInPort:设置烟感输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetSmokeInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetSmokeInPort:获取烟感输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         烟感输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetSmokeInPort(void);

/* 功能说明:
 *          thaisenSetPourInPort:设置倾倒输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetPourInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetPourInPort:获取倾倒输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         倾倒输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetPourInPort(void);

/* 功能说明:
 *          thaisenSetGunMountInPortA:设置A枪枪座输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetGunMountInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetGunMountInPortA:获取A枪枪座输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪枪座输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetGunMountInPortA(void);

/* 功能说明:
 *          thaisenSetGunMountInPortB:设置B枪枪座输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetGunMountInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetGunMountInPortB:获取B枪枪座输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪枪座输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetGunMountInPortB(void);

/* 功能说明:
 *          thaisenSetFuseInPortA: 设置A枪熔断器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFuseInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetFuseInPortA:获取A枪熔断器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪熔断器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFuseInPortA(void);

/* 功能说明:
 *          thaisenSetFuseInPortB: 设置B枪熔断器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFuseInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetFuseInPortB:获取B枪熔断器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪熔断器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFuseInPortB(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingInPortA: 设置A液冷输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingInPortA:获取A液冷输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A液冷输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingInPortA(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingInPortB: 设置B液冷输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingInPortB:获取B液冷输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B液冷输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingInPortB(void);


/******************************************** 预留输出口配置 **************************************************/
/******************************************** 预留输出口配置 **************************************************/
/* 功能说明:
 *          thaisenSetReliefOutPortA: 设置A枪泄放输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetReliefOutPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetReliefOutPortA:获取A枪泄放输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪泄放输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetReliefOutPortA(void);

/* 功能说明:
 *          thaisenSetReliefOutPortB: 设置B枪泄放输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
/*******************************************
 * 函数名   thaisenSetReliefOutPortB
 * 功能       设置B枪泄放输出端口
 ******************************************/
void thaisenSetReliefOutPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetReliefOutPortB:获取B枪泄放输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪泄放输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetReliefOutPortB(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingOutPortA: 设置A枪液冷输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingOutPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingOutPortA:获取A枪液冷输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪液冷输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingOutPortA(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingOutPortB: 设置B枪液冷输出端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingOutPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingOutPortB:获取B枪液冷输出端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪液冷输出端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingOutPortB(void);

/******************************************** 预留输入口配置 **************************************************/
/******************************************** 预留输入口配置 **************************************************/
/* 功能说明:
 *          thaisenSetLightProtectorInPort: 设置防雷器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLightProtectorInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetLightProtectorInPort:获取防雷器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         防雷器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLightProtectorInPort(void);

/* 功能说明:
 *          thaisenSetCircuitBreakerInPort: 设置断路器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetCircuitBreakerInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetCircuitBreakerInPort:获取断路器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         断路器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetCircuitBreakerInPort(void);

/* 功能说明:
 *          thaisenSetFanInPort: 设置风扇输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFanInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetFanInPort:获取风扇输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         风扇输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFanInPort(void);

/* 功能说明:
 *          thaisenSetFloodingInPort: 设置水浸输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFloodingInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetFloodingInPort:获取水浸输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         水浸输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFloodingInPort(void);

/* 功能说明:
 *          thaisenSetSmokeInPort: 设置烟感输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetSmokeInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetSmokeInPort:获取烟感输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         烟感输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetSmokeInPort(void);

/* 功能说明:
 *          thaisenSetPourInPort: 设置倾倒输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetPourInPort(uint8_t port);
/* 功能说明:
 *          thaisenGetPourInPort:获取倾倒输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         倾倒输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetPourInPort(void);

/* 功能说明:
 *          thaisenSetGunMountInPortA: 设置A枪枪座输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetGunMountInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetGunMountInPortA:获取A枪枪座输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪枪座输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetGunMountInPortA(void);

/* 功能说明:
 *          thaisenSetGunMountInPortB: 设置B枪枪座输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetGunMountInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetGunMountInPortB:获取B枪枪座输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪枪座输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetGunMountInPortB(void);

/* 功能说明:
 *          thaisenSetFuseInPortA: 设置A枪熔断器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFuseInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetFuseInPortA:获取A枪熔断器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A枪熔断器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFuseInPortA(void);

/* 功能说明:
 *          thaisenSetFuseInPortB: 设置B枪熔断器输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetFuseInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetFuseInPortB:获取B枪熔断器输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B枪熔断器输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetFuseInPortB(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingInPortA: 设置A液冷输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingInPortA(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingInPortA:获取A液冷输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         A液冷输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingInPortA(void);

/* 功能说明:
 *          thaisenSetLiquidCoolingInPortA: 设置A液冷输入端口
 * 输入参数:
 *         port:端口
 * 返回参数:
 *          无
 * 调用方法:
 *      实时调用
 */
void thaisenSetLiquidCoolingInPortB(uint8_t port);
/* 功能说明:
 *          thaisenGetLiquidCoolingInPortB:获取B液冷输入端口
 * 输入参数:
 *         无
 * 返回参数:
 *         B液冷输入端口
 * 调用方法:
 *     实时调用
 */
uint8_t thaisenGetLiquidCoolingInPortB(void);



typedef enum
{
    thaisenSensorOutIn_Close,
    thaisenSensorOutIn_Break,
}thaisenSensorOutIn_State;

void thaisenSetLightProNormalState(uint8_t state);
uint8_t thaisenGetLightProNormalState(void);

void thaisenSetFloodingNormalState(uint8_t port);
uint8_t thaisenGetFloodingNormalState(void);

void thaisenSetSmokeNormalState(uint8_t port);
uint8_t thaisenGetSmokeNormalState(void);

void thaisenSetPourNormalState(uint8_t port);
uint8_t thaisenGetPourNormalState(void);

void thaisenSetFuseNormalStateA(uint8_t state);
uint8_t thaisenGetFuseNormalStateA(void);

void thaisenSetFuseNormalStateB(uint8_t state);
uint8_t thaisenGetFuseNormalStateB(void);

void thaisenSetGunSiteNormalStateA(uint8_t state);
uint8_t thaisenGetGunSiteNormalStateA(void);

void thaisenSetGunSiteNormalStateB(uint8_t state);
uint8_t thaisenGetGunSiteNormalStateB(void);

void thaisenSetCircuitBreakerNormalState(uint8_t state);
uint8_t thaisenGetCircuitBreakerNormalState(void);

void thaisen_Relief_A_on(void);
void thaisen_Relief_A_off(void);
void thaisen_Relief_B_on(void);
void thaisen_Relief_B_off(void);

void thaisen_LiquidCooling_A_on(void);
void thaisen_LiquidCooling_A_off(void);
void thaisen_LiquidCooling_B_on(void);
void thaisen_LiquidCooling_B_off(void);

thaisenSensorOutIn_State thaisen_LightProtector_FB(void);
thaisenSensorOutIn_State thaisen_CircuitBreaker_FB(void);
thaisenSensorOutIn_State thaisen_Fan_FB(void);
thaisenSensorOutIn_State thaisen_Flooding_FB(void);
thaisenSensorOutIn_State thaisen_Smoke_FB(void);
thaisenSensorOutIn_State thaisen_Pour_FB(void);
thaisenSensorOutIn_State thaisen_GunMount_A_FB(void);
thaisenSensorOutIn_State thaisen_GunMount_B_FB(void);
thaisenSensorOutIn_State thaisen_Fuse_A_FB(void);
thaisenSensorOutIn_State thaisen_Fuse_B_FB(void);
thaisenSensorOutIn_State thaisen_LiquidCooling_A_FB(void);
thaisenSensorOutIn_State thaisen_LiquidCooling_B_FB(void);

/**********************************************************************************/
/*****************************风机调速*****************************************/

/* 功能说明:
 *        thaisen_pwm_fan_duty:风机调整转速
 * 输入参数:
 *
 *        duty:0~1000:
 *        0.1分辨率
 *        0:不转
 *        1000:全速
 * 返回参数:
 *          无
 * 调用方法:
 *             可实时调用
 */
void thaisen_pwm_fan_duty(uint16_t duty);

/**********************************************************************************/
/*****************************充电枪模式*******************************************/

typedef enum
{
    thaisenDeviceType_doubleGun,           /* 双枪终端 */
    thaisenDeviceType_singleGun,           /* 单枪终端 */
    thaisenDeviceType_average,             /* 动态切换 */
    thaisenDeviceType_Rectifier_cabinet,   /* 整流柜 */
    thaisenDeviceType_size,
}thaisenDeviceType;

/* 功能说明:
 *          thaisenSetChargGunRunType:设置设备类型
 *
 * 输入参数:
 *          type:类型
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisenSetChargGunRunType(uint8_t type);

/* 功能说明:
 *          thaisenGetChargGunRunType: 获取设备类型
 *
 * 输入参数:
 *
 * 返回参数:
 *          设备类型
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetChargGunRunType(void);

typedef enum
{
    THAISEN_GUNSTATE_SELFCHECK,
    THAISEN_GUNSTATE_IDLE,
    THAISEN_GUNSTATE_READY,
    THAISEN_GUNSTATE_STARTING,
    THAISEN_GUNSTATE_CHARGING,
    THAISEN_GUNSTATE_STOPING,
    THAISEN_GUNSTATE_FINISH,
    THAISEN_GUNSTATE_FAULTING,
}thaisenLedGunState;

#pragma pack(1)
/* 充电枪信息 */
typedef struct
{
    uint8_t state;                        /* 充电状态 */
    uint16_t fault_code;                  /* 故障码 */
    uint16_t reason_code;                 /* 停止原因码 */
    uint16_t soc;                         /* SOC(精度：0.1) */
    uint16_t voltage;                     /* 充电电压(精度：0.1) */
    uint16_t current;                     /* 充电电流(精度：0.1)  */
    uint16_t gunTemp;                     /* 枪头温度(精度：0.1)  */
    uint16_t gunLineTemp;                 /* 枪线温度(精度：0.1)  */
    uint16_t chargeElect;                 /* 已充电量(精度：0.1)  */
    uint16_t chargeTime;                  /* 充电时长(精度：1min)  */
}thaisenChargeGunInfo;
#pragma pack()
/* 功能说明:
 *          thaisenSetGunState: 设置枪状态信息(用于灯带、数码管处理)
 *
 * 输入参数:
 *         gunNum:枪号
 *         info：枪信息
 * 返回参数:
 *          设备类型
 * 调用方法:
 *          可实时调用
 */
void thaisenSetGunState(uint8_t gunNum, thaisenChargeGunInfo info);

/********************************** 电表加密 ************************************/
enum encry_type
{
 THAISEN_AMMETER_ENCRY_TYPE_START,           //指令：开始充电
 THAISEN_AMMETER_ENCRY_TYPE_STOP,            //指令：结束充电
 THAISEN_AMMETER_ENCRY_TYPE_METER_READING,   //指令：抄表
};

#pragma pack(1)
//数据标识:E4 03 00 00 加密数据
typedef struct{
    uint16_t ver;                       //通讯协议版本号(HEX)
    uint8_t encry_type;                 //加密方式
    uint8_t reserve[7];                 //预留
    uint8_t trade_number[16];           //流水号(BCD)
    uint8_t dev_sn[6];                  //表号(BCD)
    uint8_t port_identify_sn[17];       //枪口识别号(BCD)--加密或参与签名计算开始
    uint32_t stimestamp;                //计量开始时间(秒时戳,HEX)
    uint32_t etimestamp;                //计量结束时间(秒时戳,HEX)
    uint32_t positive_elect;            //正向充电电量(3 位小数,HEX)
    uint32_t install_timestamp;         //电表安装时间(秒时戳,HEX)
    uint8_t history_state;              //端钮历史状态(0 正常，1 发生过端钮盖打开时间)--加密或参与签名计算域结束
//    uint8_t sign_data[64];              //64 字节签名数据(当加密方式为 ECC256 时有该域)
}thaisen_encry_data_t;
#pragma pack()

/* 功能说明:
 *          thaisen_ammeter_encry_scmd: 发送电表加密指令
 *
 * 输入参数:
 *         gunNum:枪号
 *         type：指令类型(enum encry_type)
 * 返回参数:
 *
 * 调用方法:
 *          可实时调用
 */
void thaisen_ammeter_encry_scmd(uint8_t gunNum, enum encry_type type);

/* 功能说明:
 *          thaisen_ammeter_is_replied: 查询电表是否已回复指令
 *
 * 输入参数:
 *         gunNum:枪号
 * 返回参数:
 *         0：未回复      1：已回复
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisen_ammeter_is_replied(uint8_t gunNum);

/* 功能说明:
 *          thaisen_ammeter_query_encrypt_data: 获取加密数据
 *
 * 输入参数:
 *         gunNum:枪号
 * 返回参数:
 *         (thaisen_encry_data_t*)型数据
 * 调用方法:
 *          可实时调用
 */
void *thaisen_ammeter_query_encrypt_data(uint8_t gunNum);

/********************************** 电流偏移 ************************************/
/* 功能说明:
 *          thaisen_get_current_offset: 获取电流偏移量(10倍)
 *
 * 输入参数:
 *         gunNum:枪号
 * 返回参数:
 *         电流偏移量
 * 调用方法:
 *          可实时调用
 */
uint16_t thaisen_get_current_offset(uint8_t gunNum);



 /**********************************************************************************/
 /*****************************液冷*******************************************/

 typedef struct thaisenLiquidStruct
 {
     uint32_t Atemp;                  //环境温度                0.01℃ -50
     uint32_t flow_rate;              //系统流量                0.01L/min
     uint32_t systemstress;           //系统压力(HL:出液压力)               0.01Bar
     uint32_t returnstress;           //回液压力                0.01Bar
     uint32_t liquidtemperatur;       //回液温度                0.01℃ -50
     uint32_t supplytemperature;      //供液温度                0.01℃ -50
     uint32_t circulationspeed;       //循环泵转速              RPM
     uint32_t fanspeed;               //风机转速                RPM
     uint8_t  onoffstatus;            //工作状态
     uint8_t offlineflag;               //离线标志位

     uint8_t duty;                    //占空比(京工电浸没式液冷/TPS)
     uint8_t runningmode;             //当前模式(TPS)
     union
     {
         struct
         {
             uint32_t highpressurealarm                : 1;         /* 0：高压报警反馈 */
             uint32_t lowpressurealarm                 : 1;         /* 1：低压报警反馈*/
             uint32_t overheatedgunalarm               : 1;         /* 2：枪头超温反馈 */
             uint32_t fan1alarm                        : 1;         /* 3：风机1出错反馈 */
             uint32_t fan2alarm                        : 1;         /* 4：风机2出错反馈 */
             uint32_t highliquidalarm                  : 1;         /* 5：高液位报警 */
             uint32_t lowliquidalarm                   : 1;         /* 6：低液位报警 */
             uint32_t overflowpumpalarm                : 1;         /* 7：循环泵过流报警 */
             uint32_t pumpunderpressurealarm           : 1;         /* 8：循环泵欠压报警*/
             uint32_t pumpoverpressurealarm            : 1;         /* 9：循环泵过压报警 */
             uint32_t pumpovertempalarm                : 1;         /* 10：过温报警 */
             uint32_t blockagepumpalarm                : 1;         /* 11：循环泵堵转报警 */
             uint32_t retainalarm                      : 1;         /* 12：预留 */
             uint32_t lowflowalarm                     : 1;         /* 13：低流量报警 */
             uint32_t highflowalarm                    : 1;         /* 14：高流量报警*/
             uint32_t fanoverflowalarm                 : 1;         /* 15：风机过流报警*/
             uint32_t Reserve                          : 16;        /* 预留 */
         }bit;
         struct
         {
             uint32_t lowliquiderr                      : 1;        //0:液位极低
             uint32_t lowliquidfault                    : 1;        //1:液位过低
             uint32_t returnfilterclogged               : 1;        //2:回液过滤器堵塞
             uint32_t supplyfilterclogged               : 1;        //3:出液过滤器堵塞
             uint32_t liquidgunclogged                  : 1;        //4:液冷枪堵塞
             uint32_t radiatorclogged                   : 1;        //5:散热器脏堵
             uint32_t returnliquidovertemp              : 1;        //6:回液温度过高
             uint32_t ambinetovertemp                   : 1;        //7:环境温度过高
             uint32_t supplytempsensorfault             : 1;        //8:出液温度传感器故障
             uint32_t returntempsensorfault             : 1;        //9:回液温度传感器故障
             uint32_t supplypressuresensorfault         : 1;        //10:出液压力传感器故障
             uint32_t returnpressuresensorfault         : 1;        //11:回液压力传感器故障
             uint32_t pumpfault                         : 1;        //12:水泵故障
             uint32_t fansfault                         : 1;        //13:风机故障
             uint32_t reserve                           : 18;       //预留
         }bit_HL;
         struct
         {
             uint32_t returnliquidovertemp              : 1;        //回液温度过高
             uint32_t fansfault                         : 1;        //风扇故障
             uint32_t pumpsupplyoverpressure            : 1;        //泵出口压力过高
             uint32_t lowliquidfault                    : 1;        //冷却液液位过低
             uint32_t returntempsensorfault             : 1;        //回液温度传感器故障
             uint32_t temp_pressuresensorfault          : 1;        //温压传感器故障
             uint32_t pumprunningdry                    : 1;        //泵空转
             uint32_t pumpjam                           : 1;        //泵堵转
             uint32_t pumppcbovertempwarning            : 1;        //泵PCB过温降功率
             uint32_t pumppcbovertempfault              : 1;        //泵PCB过温停转
             uint32_t pumppcbundertempwarning           : 1;        //泵PCB低温降功率
             uint32_t pumppcbundertempfault             : 1;        //泵PCB低温停转
             uint32_t pumpovervolt                      : 1;        //泵过电压
             uint32_t pumpundervolt                     : 1;        //泵欠电压
             uint32_t pumpovercurr                      : 1;        //泵过电流
             uint32_t pumpoverload                      : 1;        //泵过载
             uint32_t pumpdrivefault                    : 1;        //泵驱动故障
             uint32_t pumpmcufault                      : 1;        //泵MCU故障
             uint32_t pumpunresponsive                  : 1;        //泵无响应
             uint32_t highliquidfault                   : 1;        //冷却液液位过高
             uint32_t gunAleakage                       : 1;        //A枪漏液
             uint32_t gunBleakage                       : 1;        //B枪漏液
             uint32_t powersupplyfault                  : 1;        //12V供电故障
             uint32_t reserve                           : 9;        //预留
         }bit_TPS;
         uint32_t fault_code;
     }state_flag;
 }thaisenLiquidSt;

 typedef enum
 {
     thaisenLiquidDev_YTND,
     thaisenLiquidDev_HL,
     thaisenLiquidDev_ImmersionJGD,
     thaisenLiquidDev_TPS,
     thaisenLiquidDevSize,
 }thaisenLiquidDevType;

 /**
  * @brief 设置液冷数量
  * @param cnt
  */
 void thaisenSetLiquidNum(uint8_t cnt);

 /**
  * @brief 获取液冷数量
  * @return
  */
 uint8_t thaisenGetLiquidNum(void);

 /*
  * @note 设置液冷设备类型
  * @param DevType
  */
 void thaisenLiquid_set_LiquidDev(uint8_t DevType);

 /*
  * @note 获取液冷设备类型
  * @return
  */
 thaisenLiquidDevType thaisenLiquid_get_LiquidDev(void);

 /* 功能说明:
  *          thaisenSetLiquidStart: 启动液冷
  *
  * 输入参数:
  *         gunNum:枪号
  * 返回参数:
  *          设备类型
  * 调用方法:
  *          可实时调用
  */
 void thaisenSetLiquidStart(uint8_t gunNum);

 /* 功能说明:
  *          thaisenSetLiquidStop: 停止液冷
  *
  * 输入参数:
  *         gunNum:枪号
  * 返回参数:
  *          设备类型
  * 调用方法:
  *          可实时调用
  */
 void thaisenSetLiquidStop(uint8_t gunNum);


 /* 功能说明:
  *          thaisenGetLiquidPara: 获取液冷数据
  *
  * 输入参数:
  *         gunNum:枪号
  * 返回参数:
  *          设备类型
  * 调用方法:
  *          可实时调用
  */
 thaisenLiquidSt *thaisenGetLiquidPara(uint8_t gunNum);




/*********************************************************** 输入输出端口 *************************************************************/
/** 输入端口 */
typedef enum
{
    thaisenGeneralInPortStaAbnormal,                   //通用输入端口状态: 异常
    thaisenGeneralInPortStaNormal,                     //通用输入端口状态: 正常
    thaisenGeneralInPortStaSize,                       //通用输入端口状态
}thaisenGeneralInPortSta;

typedef enum
{
    thaisenGeneralInPortAbnormalLow,                   //通用输入端口异常时状态: 低电平
    thaisenGeneralInPortAbnormalHigh,                  //通用输入端口异常时状态: 高电平
    thaisenGeneralInPortAbnormaSize,                   //通用输入端口状态
}thaisenGeneralInPortAbnormalStaEnum;

typedef enum
{
    thaisenGeneralInPortLightningProtection,           //通用输入端口: 防雷
    thaisenGeneralInPortGunSit_A,                      //通用输入端口: A枪枪座
    thaisenGeneralInPortGunSit_B,                      //通用输入端口: B枪枪座
//    thaisenGeneralInPortCircuitBreaker,                //通用输入端口: 断路器
    thaisenGeneralInPortFlooding,                      //通用输入端口: 水浸
    thaisenGeneralInPortSmoke,                         //通用输入端口: 烟感
    thaisenGeneralInPortPour,                          //通用输入端口: 倾倒
//    thaisenGeneralInPortLiquid,                        //通用输入端口: 液冷
//    thaisenGeneralInPortFuse,                          //通用输入端口: 熔断器
    thaisenGeneralInPortSize,                          //通用输入端口
}thaisenGeneralInPortEnum;

/* 功能说明:
 *          thaisenGetGeneralInPortSta:获取通用输入端口状态
 * 输入参数:
 *          port    输入端口枚举
 * 返回参数:
 *          @thaisenGeneralInPortSta
 * 调用方法:
 *          可实时调用
 */
uint8_t thaisenGetGeneralInPortSta(thaisenGeneralInPortEnum port);



/* 功能说明:
 *          thaisenSetGeneralInPortAbnormalSta:设置通用输入端口异常时的状态
 * 输入参数:
 *          sta:    只能输入0或1,输入其他值保持原先状态
 *          port    输入端口枚举
 * 返回参数:
 *          无
 * 调用方法:
 *          可实时调用
 */
void thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortEnum port, thaisenGeneralInPortAbnormalStaEnum sta);

#endif /* APPLICATIONS_THAISEN7102PUBLIC_H_ */
