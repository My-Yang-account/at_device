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
    thaisenYTBFC,
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

/*****************************车主动停祥因(BST)*****************************************/
typedef struct
{
    uint8_t  SOCGetObj           :2; //SOC达到目标值           00:未达到  01:达到 11:不可信状态
    uint8_t  VoltGetObj          :2; //总电压达到目标值        00:未达到  01:达到 11:不可信状态
    uint8_t  CeliVoltGetObj      :2; //单体电压达到目标值      00:未达到  01:达到 11:不可信状态
    uint8_t  ChargInitiStop      :2; //充电机主动停止

    uint8_t  InsltFault          :2; //绝缘故障           00:正常  01:故障 10:不可信状态
    uint8_t  OutConectOVtemp     :2; //输出连接器故障     00:正常  01:故障 10:不可信状态
    uint8_t  BMSCompOVtemp       :2; //BMS元件故障        00:正常  01:故障 10:不可信状态
    uint8_t  Conectfault         :2; //充电连接故障

    uint8_t  BatOVtemp           :2; //电池组温度故障     00:正常  01:故障 10:不可信状态
    uint8_t  HVRelaysFault       :2; //高压继电器故障     00:正常  01:故障 10:不可信状态
    uint8_t  Check2Ft            :2; //检测点2电压检测故障00:正常  01:故障 10:不可信状态
    uint8_t  OtherFt             :2; //其他故障

    uint8_t  OverCurlt           :2; //充电电流过流       00:正常  01:超过需求值 01:不可信状态
    uint8_t  Voltfault           :2; //充电电压异常       00:正常  01:电压异常   01:不可信状态
    uint8_t  :4;
}thaisenBSTDetailed_t;

/* 功能说明:
 *          thaisenGetBSTDetailed:获取BST报文详细原因
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         BST报文详细原因@thaisenBSTDetailed_t
 * 调用方法:
 *          充电结束调用
 */
thaisenBSTDetailed_t thaisenGetBSTDetailed(uint8_t gunNum);

/*****************************车故障停祥因(BSM)*****************************************/
typedef struct
{
    uint8_t  CellOverVolt:2;  //单体过压           00:正常  01:过高 01:过低
    uint8_t  SOCState    :2;  //SOC状态            00:正常  01:过高 01:过低
    uint8_t  BatOverCurlt:2;  //电池充电过流       00:正常  01:过高 01:过低
    uint8_t  BatOverTemp :2;  //电池温度过高       00:正常  01:过高 01:过低

    uint8_t  Insulat     :2;  //电池绝缘状态       00:正常  01:不正常 01:不可信状态
    uint8_t  OutConect   :2;  //输出连接器状态     00:正常  01:不正常 01:不可信状态
    uint8_t  AllowChg    :2;  //允许充电           00:禁止  01:允许
    uint8_t  :2;
}thaisenBSMDetailed_t;

/* 功能说明:
 *          thaisenBSMDetailed_t:获取BSM报文详细原因
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         BSM报文详细原因@thaisenBSMDetailed_t
 * 调用方法:
 *          充电结束调用
 */
thaisenBSMDetailed_t thaisenGetBSMDetailed(uint8_t gunNum);

/*****************************桩通讯超时祥因(BEM)*****************************************/
typedef struct
{
    uint8_t  CRM00OVtime      :2; //接收CRM_A 00超时   00:正常   01:超时  01:不可信状态
    uint8_t  CRMAAOVtime      :2; //接收CRM_A AA超时   00:正常   01:超时  01:不可信状态
    uint8_t                   :4;

    uint8_t  CTSCMLOVtime     :2; //接收CTS_A.CML_A超时  00:正常   01:超时  01:不可信状态
    uint8_t  CROOVtime        :2; //接收CRO_A超时      00:正常   01:超时  01:不可信状态
    uint8_t                   :4;

    uint8_t  CCSOVtime        :2; //接收CCS_A超时      00:正常   01:超时  01:不可信状态
    uint8_t  CSTOVtime        :2; //接收CST_A超时      00:正常   01:超时  01:不可信状态
    uint8_t                   :4;

    uint8_t  CSDOVtime        :2; //接收CSD_A超时      00:正常   01:超时  01:不可信状态
    uint8_t                   :6;
}thaisenBEMDetailed_t;

/* 功能说明:
 *          thaisenGetBSMDetailed:获取BEM报文详细原因
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         BEM报文详细原因@thaisenBEMDetailed_t
 * 调用方法:
 *          充电结束调用
 */
thaisenBEMDetailed_t thaisenGetBEMDetailed(uint8_t gunNum);

/*****************************车通讯超时祥因*****************************************/
typedef enum
{
    THAISEN_COMMUTIMEOUT_BRM,
    THAISEN_COMMUTIMEOUT_BCP,
    THAISEN_COMMUTIMEOUT_BRO,
    THAISEN_COMMUTIMEOUT_BRO_AA,
    THAISEN_COMMUTIMEOUT_BCL,
    THAISEN_COMMUTIMEOUT_BCS,
    THAISEN_COMMUTIMEOUT_SIZE,
}thaisenCommuTimeoutEnum;

/* 功能说明:
 *          thaisenGetCommuTimeoutDetailed:获取BMS通讯超时详细原因
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         BMS通讯超时详细原因@thaisenCommuTimeoutEnum(枪号不对返回THAISEN_COMMUTIMEOUT_SIZE)
 * 调用方法:
 *          充电结束调用
 */
thaisenCommuTimeoutEnum thaisenGetCommuTimeoutDetailed(uint8_t gunNum);

/*****************************报文接收情况(1：收到  0：未收到)*****************************************/
typedef struct
{
    uint16_t BHM : 1;                           // 报文接收：BHM
    uint16_t BRM : 1;                           // 报文接收：BRM
    uint16_t BCP : 1;                           // 报文接收：BCP
    uint16_t BRO : 1;                           // 报文接收：BRO
    uint16_t BRO_AA : 1;                        // 报文接收：BRO_AA
    uint16_t BCL : 1;                           // 报文接收：BCL
    uint16_t BCS : 1;                           // 报文接收：BCS
    uint16_t BSM : 1;                           // 报文接收：BSM
    uint16_t BST : 1;                           // 报文接收：BST
    uint16_t BSD : 1;                           // 报文接收：BSD
    uint16_t BEM : 1;                           // 报文接收：BEM
    uint16_t BFC : 1;                           // 报文接收：BFC
    uint16_t Reserve : 4;
}thaisenMsgRecved_t;

/* 功能说明:
 *          thaisenGetMsgRecved:获取报文接收详情
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         报文接收详情 @thaisenMsgRecved_t
 * 调用方法:
 *          实时调用
 */
thaisenMsgRecved_t thaisenGetMsgRecved(uint8_t gunNum);

/*****************************报文发送情况(1：已发送  0：未发送)*****************************************/
typedef struct
{
    uint16_t CHM : 1;                           // 报文发送：CHM
    uint16_t CRM : 1;                           // 报文发送：CRM
    uint16_t CRM_AA : 1;                        // 报文发送：CRM_AA
    uint16_t CFC : 1;                           // 报文发送：CFC
    uint16_t CTS : 1;                           // 报文发送：CTS
    uint16_t CML : 1;                           // 报文发送：CML
    uint16_t CRO : 1;                           // 报文发送：CRO
    uint16_t CRO_AA : 1;                        // 报文发送：CRO_AA
    uint16_t CCS : 1;                           // 报文发送：CCS
    uint16_t CST : 1;                           // 报文发送：CST
    uint16_t CSD : 1;                           // 报文发送：CSD
    uint16_t CEM : 1;                           // 报文发送：CEM
    uint16_t Reserve : 4;
}thaisenMsgSended_t;

/* 功能说明:
 *          thaisenGetMsgSended:获取报文发送详情
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         报文发送详情 @thaisenMsgSended_t
 * 调用方法:
 *          实时调用
 */
thaisenMsgSended_t thaisenGetMsgSended(uint8_t gunNum);

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
    thaisen_chargeCtl_stopWay_BFC,
    thaisen_chargeCtl_stopWay_BST_TargetSOC,                                              /** 系统停充原因：车端停详细原因：SOC达到目标值 */
    thaisen_chargeCtl_stopWay_BST_TargetTotalVolt,                                        /** 系统停充原因：车端停详细原因：总电压达到目标值 */
    thaisen_chargeCtl_stopWay_BST_TargetSingleVolt,                                       /** 系统停充原因：车端停详细原因：单体电压达到目标值 */
    thaisen_chargeCtl_stopWay_BST_ChargerEnd,                                             /** 系统停充原因：车端停详细原因：充电机主动停止 */
    thaisen_chargeCtl_stopWay_BST_InsultionFault,                                         /** 系统停充原因：车端停详细原因：绝缘故障 */
    thaisen_chargeCtl_stopWay_BST_OutLinkerFault,                                         /** 系统停充原因：车端停详细原因：输出连接器故障 */
    thaisen_chargeCtl_stopWay_BST_BMSElement,                                             /** 系统停充原因：车端停详细原因：BMS元件故障 */
    thaisen_chargeCtl_stopWay_BST_ChargeLinkerFault,                                      /** 系统停充原因：车端停详细原因：充电连接故障 */
    thaisen_chargeCtl_stopWay_BST_BatGroupOT,                                             /** 系统停充原因：车端停详细原因：电池组温度故障 */
    thaisen_chargeCtl_stopWay_BST_HV_Relay,                                               /** 系统停充原因：车端停详细原因：高压继电器故障 */
    thaisen_chargeCtl_stopWay_BST_DetectPiont_2,                                          /** 系统停充原因：车端停详细原因：检测点2电压检测故障 */
    thaisen_chargeCtl_stopWay_BST_OverCurrent,                                            /** 系统停充原因：车端停详细原因：充电电流过流 */
    thaisen_chargeCtl_stopWay_BST_AbnormalVoltage,                                        /** 系统停充原因：车端停详细原因：充电电压异常 */
    thaisen_chargeCtl_stopWay_BSM_SingleBat_OV,                                           /** 系统停充原因：BSM详细原因：单体电压异常 */
    thaisen_chargeCtl_stopWay_BSM_AbnormalSOC,                                            /** 系统停充原因：BSM详细原因：SOC状态异常 */
    thaisen_chargeCtl_stopWay_BSM_OverCurrent,                                            /** 系统停充原因：BSM详细原因：电池充电过流 */
    thaisen_chargeCtl_stopWay_BSM_BatteryOT,                                              /** 系统停充原因：BSM详细原因：电池温度过高 */
    thaisen_chargeCtl_stopWay_BSM_BatInsultionAbnormal,                                   /** 系统停充原因：BSM详细原因：电池绝缘状态异常 */
    thaisen_chargeCtl_stopWay_BSM_OutLinkerAbnormal,                                      /** 系统停充原因：BSM详细原因：输出连接器状态异常 */
    thaisen_chargeCtl_stopWay_BSM_Forbid,                                                 /** 系统停充原因：BSM详细原因：禁止充电 */

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

enum
{
    thaisenChargeWarnOk,
    thaisenChargeWarnCommu,
};


uint8_t thaisen_get_InsultInfo(uint8_t gunNum);
uint8_t thaisen_get_InsultVoltInfo(uint8_t gunNum);
uint8_t thaisen_get_BMSCurrentInfo(uint8_t gunNum);
uint8_t thaisen_get_ChargeWarnningInfo(uint8_t gunNum);

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

/*****************************************************************************/
/************************充电中暂停和启动选择*********************************************/
typedef enum thaisenChargingInfoEnum
{
  thaisenChargingPause,   //暂停充电
  thaisenChargingActivate,//允许充电
}thaisenChargingInfo;


void thaisen_set_charging_pause_activate(uint8_t gunNum,thaisenChargingInfo info);
thaisenChargingInfo thaisen_get_charging_pause_activate(uint8_t gunNum);

/*****************************************************************************/
/************************功能控制*********************************************/
typedef enum
{
    thaisenChargFunctionEnable_NoOffset,                  /** 功能使能：无电流偏移协议 */
    thaisenChargFunctionEnable_YuTong,                    /** 功能使能：宇通 协议 */
    thaisenChargFunctionEnable_Size,                      /** 功能使能 */
}thaisenChargFunctionEnable_t;

typedef enum
{
    thaisenChargFunctionExecute_Init,                     /** 功能执行：启动前信息初始化 */
    thaisenChargFunctionExecute_Size,                     /** 功能执行 */
}thaisenChargFunctionExecute_t;

#pragma pack(1)
typedef struct
{
    /****************************************************************************
     * 函数名       SetupFunctionEnable
     * 功能           设置功能项使能状态
     * 参数           gunNum    枪号
     *         function   功能项@thaisenChargFunctionEnable_t
     *         state     使能状态(1：使能    0：不使能)
     * 返回
     ***************************************************************************/
    void (*SetupFunctionEnable)(uint8_t gunNum, uint8_t function, uint8_t state);
    /****************************************************************************
     * 函数名       QueryFunctionEnable
     * 功能           查询功能项使能状态
     * 参数           gunNum    枪号
     *         function   功能项@thaisenChargFunctionEnable_t
     * 返回           1：使能       0：未使能
     ***************************************************************************/
    uint8_t (*QueryFunctionEnable)(uint8_t gunNum, uint8_t function);
    /****************************************************************************
     * 函数名       FunctionExecute
     * 功能           功能执行
     * 参数           gunNum    枪号
     *         function   功能码@thaisenChargFunctionExecute_t
     * 返回          1：执行成功    0：执行失败
     ***************************************************************************/
    uint8_t (*FunctionExecute)(uint8_t gunNum, uint8_t function);
}thaisenChargCtrlHandle_t;
#pragma pack()

/***********************************************************************************
 * 函数名       thaisenChargGetCtrlHandle
 * 功能           获取控制句柄
 * 参数
 * 返回           控制句柄
 *********************************************************************************/
thaisenChargCtrlHandle_t *thaisenChargGetCtrlHandle(void);


#endif
