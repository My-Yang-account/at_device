#ifndef _chargLib_h
#define _chargLib_h


#include "thaisen7102Public.h"

/*******************************************************************************/
 
 
/*****************************系统初始化*****************************************/
/* 功能说明:
 *          thaisenChargInit:充电信息初始化及系统初始化
 * 输入参数:
 *
 * 返回参数:
 *          无
 * 调用方法:
 *          初始化时调用
 */
void thaisenChargInit(void);



/*******************************************************************************/
 
 
/*****************************启动停止*****************************************/
/* 功能说明:
 *          thaisen_start_charg:启动充电
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          无
 * 调用方法:
 *          启动时调用
 */
void thaisen_start_charg(uint8_t gunNum);


/* 功能说明:
 *          thaisen_stop_charg:停止充电
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          无
 * 调用方法:
 *          停止时调用
 */
void thaisen_stop_charg(uint8_t gunNum);

/*******************************************************************************/
 
 
/*****************************充电时故障信息函数*****************************************/


typedef enum thaisenFaultChargEnum
{
    thaisenGunVolt,
    thaisenInsult,
    thaisenCommon,
    thaisenBatteryVolt,
    thaisenReadyVolt,
    thaisenInsultVolt,
}thaisenFaultChargTy;

/* 功能说明:
 *          thaisenGetChargFault:查询充电故障信息
 * 输入参数:
 *
 * 返回参数:
 *          无
 * 调用方法:
 *          实时调用
 */
uint32_t* thaisenGetChargFault(uint8_t gunNum);



/* 功能说明:
 *          thaisen_get_charg_status:获取充电状态
 * 输入参数:
 *
 * 返回参数:
 *          无
 * 调用方法:
 *          实时调用
 */
uint16_t thaisen_get_charg_status(uint8_t gunNum);

/*******************************************************************************/
 
 
/*****************************工作状态信息*****************************************/


enum
{
    thaisen_charg_Idle, //空闲
    thaisen_charg_Handshake,//握手
    thaisen_charg_Insult,//绝缘
    thaisen_charg_Config,//配置
    thaisen_charg_Charg,//充电
    thaisen_charg_End,//结束
    thaisen_charg_Fault,//故障
    thaisen_charg_WaitGun,//等待拔枪
};




/* 功能说明:
 *          thaisenGetChargWorkStatus:获取充电状态
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *          状态信息
 * 调用方法:
 *          实时调用
 */
uint8_t thaisenGetChargWorkStatus(uint8_t gunNum);




/*******************************************************************************/
 
 
/*****************************BMS报文数据*****************************************/

/* 功能说明:
 *          thaisen_get_bms_data:获取BMS数据信息
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *         枪号对应的BMS数据地址
 * 调用方法:
 *          自定义地址指针，实时调用
 */
struct thaisenBMS_Charger_struct* thaisen_get_bms_data(uint8_t gunNum);


/*****************************************************************************/
/************************停止原因*********************************************/

/*
 * 充电停止方式：
 *0.急停故障终止
 *1.读卡器故障终止
 *2.门禁故障终止
 *3.电表故障
 *4.充电模块故障
 *5.过温故障
 *6.过压故障
 *7.欠压故障
 *8.过流故障
 *9.主继电器故障
 *10.并联继电器故障
 *11.交流接触器故障
 *12.电磁锁故障
 *13.辅助电源故障
 *14.FLASH故障
 *15.EEPROM故障
 *16.短路故障
 *17.枪端电压故障
 *18.绝缘故障
 *19.与BMS通讯故障
 *20.电池电压不匹配
 *21.拔枪停止
 *22.充满停止
 *23.人工主动停止
 *24.BST停止
 *25.准备电压不匹配
 *26.绝缘电压
 *27.BSM停止
 *
 * */


typedef enum{
    thaisen_chargeCtl_stopWay_scram = 0,
    thaisen_chargeCtl_stopWay_cardReader,
    thaisen_chargeCtl_stopWay_door,
    thaisen_chargeCtl_stopWay_ammeter,
    thaisen_chargeCtl_stopWay_chargModule,
    thaisen_chargeCtl_stopWay_OverTemp,
    thaisen_chargeCtl_stopWay_overVolt,
    thaisen_chargeCtl_stopWay_underVolt,
    thaisen_chargeCtl_stopWay_OverCurrent,
    thaisen_chargeCtl_stopWay_relay,
    thaisen_chargeCtl_stopWay_Parallel_relay,
    thaisen_chargeCtl_stopWay_Ac_relay,
    thaisen_chargeCtl_stopWay_elock,
    thaisen_chargeCtl_stopWay_AuxPower,
    thaisen_chargeCtl_stopWay_flash,
    thaisen_chargeCtl_stopWay_eeprom,
    thaisen_chargeCtl_stopWay_LightProtect,
    thaisen_chargeCtl_stopWay_GunSite,
    thaisen_chargeCtl_stopWay_CircuitBreaker,
    thaisen_chargeCtl_stopWay_Flooding,
    thaisen_chargeCtl_stopWay_Smoke,
    thaisen_chargeCtl_stopWay_Pour,
    thaisen_chargeCtl_stopWay_LiquidCooling,
    thaisen_chargeCtl_stopWay_Fuse,
    tthaisen_chargeCtl_stopWay_MainCabinet,
    thaisen_chargeCtl_stopWay_short,
    thaisen_chargeCtl_stopWay_GunVolt,
    thaisen_chargeCtl_stopWay_Insult,
    thaisen_chargeCtl_stopWay_Common,
    thaisen_chargeCtl_stopWay_BatteryVolt,
    thaisen_chargeCtl_stopWay_gun,
    thaisen_chargeCtl_stopWay_full,
    thaisen_chargeCtl_stopWay_passive,
    thaisen_chargeCtl_stopWay_BST,
    thaisen_chargeCtl_stopWay_ReadyVolt,
    thaisen_chargeCtl_stopWay_InsultVolt,
    thaisen_chargeCtl_stopWay_BSM,

    thaisen_chargeCtl_stopWay_size,
}thaisenChargeCtlStopWayEn;


/* 功能说明:
 *          thaisenGetChargCtlStopWay:充电停止原因
 * 输入参数:
 *                  gunNum:充电枪号，填0
 * 返回参数:
 *      停止原因
 * 调用方法:
 *          自定义地址指针，实时调用
 */
thaisenChargeCtlStopWayEn thaisenGetChargCtlStopWay(uint8_t num);



/*****************************************************************************/
/************************告警信息*********************************************/
enum
{
  thaisenInsultOk,
  thaisenInsultAnomaly,
  thaisenInsultFault,
};


enum
{
  thaisenInsultVoltOk,
  thaisenInsultVoltAlarm,
};

enum
{
  thaisenBMSCurrentOk,
  thaisenBMSCurrentAlarm,
};



uint8_t thaisen_get_InsultInfo(uint8_t gunNum);
uint8_t thaisen_get_InsultVoltInfo(uint8_t gunNum);
uint8_t thaisen_get_BMSCurrentInfo(uint8_t gunNum);

/*****************************************************************************/
/************************枪头电压信息*********************************************/
/* 功能说明:
 *          thaisen_set_ChargGunVolt:设置枪头检测电压
 * 输入参数:
 *          volt:电压值,0.1v精度   gunNum:充电枪号，填0
 * 返回参数:
 *          无
 * 调用方法:
 *
 */
void thaisen_set_ChargGunVolt(uint16_t volt,uint8_t gunNum);


/* 功能说明:
 *          thaisen_get_ChargGunVolt:获取设置的枪头检测电压
 * 输入参数:
 *          gunNum:充电枪号，填0
 * 返回参数:
 *                           电压值,0.1v精度
 * 调用方法:
 *
 */

uint16_t thaisen_get_ChargGunVolt(uint8_t gunNum);

/*****************************************************************************/
/************************充电模式选择*********************************************/
typedef enum thaisenChargModeEnum
{
  thaisenParallelCharging,
  thaisenSingleChargeMode,
}thaisenChargModeEn;


void thaisen_set_charg_mode(thaisenChargModeEn mode);
uint8_t thaisen_get_charg_mode(void);
#endif
