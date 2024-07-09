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

  thaisenFaultSize,
}thaisenFaultTy;


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

void thaisenSetACRelayCloseStaus(uint8_t sta);
uint8_t thaisenGetACRelayCloseStaus(void);

void thaisenSetDCRelayACloseStaus(uint8_t sta);
uint8_t thaisenGetDCRelayACloseStaus(void);

void thaisenSetDCRelayBCloseStaus(uint8_t sta);
uint8_t thaisenGetDCRelayBCloseStaus(void);

void thaisenSetParaRelayCloseStaus(uint8_t sta);
uint8_t thaisenGetParaRelayCloseStaus(void);
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


thaisenAuxPowerEn thaisenGetAux_A_Status_debug(void);
thaisenAuxPowerEn thaisenGetAux_B_Status_debug(void);

void thaisenSetAuxPowerTypeA(uint8_t type);
thaisenAuxPowerTypeEn thaisenGetAuxPowerTypeA(void);

void thaisenSetAuxPowerTypeB(uint8_t type);
thaisenAuxPowerTypeEn thaisenGetAuxPowerTypeB(void);

thaisenAuxPowerEn thaisenAux_A_12V_Enable(void);
thaisenAuxPowerEn thaisenAux_A_12V_Disable(void);

thaisenAuxPowerEn thaisenAux_A_24V_Enable(void);
thaisenAuxPowerEn thaisenAux_A_24V_Disable(void);

thaisenAuxPowerEn thaisenAux_B_12V_Enable(void);
thaisenAuxPowerEn thaisenAux_B_12V_Disable(void);

thaisenAuxPowerEn thaisenAux_B_24V_Enable(void);
thaisenAuxPowerEn thaisenAux_B_24V_Disable(void);

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


/*******************************************************************************/


/*****************************电表函数*****************************************/
enum
{
    thaisenAmmeterModel_RuiYin,
    thaisenAmmeterModel_YaDa,
    thaisenAmmeterModel_Other,
};

/* 功能说明:
 *          thaisen_get_ammeterVolt:获取电表电压
 *                  分辨率:0.1
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          电压值
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_ammeterVolt(uint8_t gunNum);



/* 功能说明:
 *          thaisen_get_ammeterCurrent:获取电表电流
 *                  分辨率:0.1
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          电流值
 * 调用方法:
 *          实时调用
 */
uint32_t thaisen_get_ammeterCurrent(uint8_t gunNum);



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
  * 返回参数:
  *          无
  * 调用方法:
  *          可实时调用
  */
 void thaisenSetSysFaultCheckAllBit(uint32_t faultValue);

 /* 功能说明:
  *          thaisenSetSysFaultCheckBit:故障检测使能,按照BIT位使能
  *          例如:填入值为thaisenFaultTy类型中的当前值
  * 输入参数:
  *         uint32_t:故障码
  *         每个Bit代表一位故障
  * 返回参数:
  *          无
  * 调用方法:
  *          可实时调用
  */
 void thaisenSetSysFaultCheckBit(thaisenFaultTy faultValue);


 /* 功能说明:
  *          thaisenClearSysFaultCheckBit:故障检测使能,按照BIT位清除
  *          例如:填入值为thaisenFaultTy类型中的当前值
  * 输入参数:
  *         uint32_t:故障码
  *         每个Bit代表一位故障
  * 返回参数:
  *          无
  * 调用方法:
  *          可实时调用
  */
 void thaisenClearSysFaultCheckBit(thaisenFaultTy faultValue);


 /* 功能说明:
  *          thaisenGetSysFaultCheckBit:获取故障检测使能
  *
  * 输入参数:
  *
  * 返回参数:
  *          uint32_t
  * 调用方法:
  *          可实时调用
  */
 uint32_t thaisenGetSysFaultCheckBit(void);


 /* 功能说明:
  *          thaisenGetSysFaultCheckEnBit:获取故障检测是否使能
  *
  * 输入参数:
  *
  * 返回参数:
  *          uint8_t
  * 调用方法:
  *          可实时调用
  */
 uint8_t thaisenGetSysFaultCheckEnBit(thaisenFaultTy faultBit);



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


#endif /* APPLICATIONS_THAISEN7102PUBLIC_H_ */
