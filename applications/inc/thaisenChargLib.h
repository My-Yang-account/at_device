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
    uint8_t  Reserve : 4;            //预留
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
    uint8_t  HigVoltCellNum;  //最高单体电压单体所在编号
    int8_t   HigTemp;         //动力电池最高温度   1°/bit  -50-200 偏移-50
    uint8_t  HigTempNum;      //最高温度检测点编号
    int8_t   LowTemp;         //动力电池最低温度   1°/bit  -50-200 偏移-50
    uint8_t  LowTempNum;      //最低温度检测点编号

    uint8_t  CellOverVolt:2;  //单体过压           00:正常  01:过高 01:过低
    uint8_t  SOCState    :2;  //SOC状态            00:正常  01:过高 01:过低
    uint8_t  BatOverCurlt:2;  //电池充电过流       00:正常  01:过高 01:过低
    uint8_t  BatOverTemp :2;  //电池温度过高       00:正常  01:过高 01:过低

    uint8_t  Insulat     :2;  //电池绝缘状态       00:正常  01:不正常 01:不可信状态
    uint8_t  OutConect   :2;  //输出连接器状态     00:正常  01:不正常 01:不可信状态
    uint8_t  AllowChg    :2;  //允许充电           00:禁止  01:允许
    uint8_t  Reserve : 2;     //预留
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

/*****************************BMS统计报文祥因(BSD)*****************************************/
typedef struct
{
    uint8_t  StopSOC;      //终止电荷状态       1%/bit    0-100%
    uint16_t CellLowVolt;  //最低单体电压       0.01V/bit  0-24V
    uint16_t CellHigVolt;  //最高单体电压       0.01V/bit  0-24V
    uint8_t  LowTemp;      //动力电池最低温度   0.1°/bit  -50-200 偏移-50
    uint8_t  HigTemp;      //动力电池最高温度   0.1°/bit  -50-200 偏移-50
}thaisenBSDDetailed_t;

/* 功能说明:
 *          thaisenGetBSDDetailed:获取BSD报文详细原因
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         BSD报文详细原因@thaisenBSDDetailed_t
 * 调用方法:
 *          充电结束调用
 */
thaisenBSDDetailed_t thaisenGetBSDDetailed(uint8_t gunNum);

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

/*****************************启动或充电中正在等待的车的报文*****************************************/
typedef enum
{
    THAISEN_WAITING_MSG_BHM,
    THAISEN_WAITING_MSG_BRM,
    THAISEN_WAITING_MSG_BCP,
    THAISEN_WAITING_MSG_BRO,
    THAISEN_WAITING_MSG_BRO_AA,
    THAISEN_WAITING_MSG_BCL,
    THAISEN_WAITING_MSG_BCS,
    THAISEN_WAITING_MSG_SIZE,
}thaisenWaitingMsgEnum;

/* 功能说明:
 *          thaisenGetWaitingMsgDetailed:获取启动或充电中正在等待的车的报文
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         启动或充电中正在等待的车的报文@thaisenWaitingMsgEnum(枪号不对返回THAISEN_WAITING_MSG_SIZE)
 * 调用方法:
 *          充电结束调用
 */
thaisenWaitingMsgEnum thaisenGetWaitingMsgDetailed(uint8_t gunNum);

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

/************************************************** 超级电流流协议信息 **************************************************/
typedef enum
{
    thaisen_superCurrProtocol_None,             //超级电流协议：无
    thaisen_superCurrProtocol_YuTong,           //超级电流协议：宇通CFC
    thaisen_superCurrProtocol_Fast,             //超级电流协议：FAST
    thaisen_superCurrProtocol_Size,             //超级电流协议
}thaisenSuperCurrProtocolEnum;

/************************* 宇通 CFC/BFC 协议 *************************/
typedef struct
{
    uint8_t  CurrOffset;                       //电流偏移(8bits，100A/bit；范围 4-20， 双枪充电默认为6，受电弓充电默认 12)
    uint8_t  GunNum           :4;              //表示充电机检测到的有效插枪数量
    uint8_t  Ack              :2;              //应答信号(00：与 CRM 一起发送，作为通知 BMS 充电机的协议为电流偏移量自动识别
                                               //         01：已经收到 BFC 的反馈，但 BMS 回复的电流偏移量或检测到的充电枪数量等信息不符
                                               //         10：无效
                                               //         11：已经收到 BFC 的反馈，且 BMS 回复的电流偏移量和检测到的充电枪数量等信息均一致)
    uint8_t  Reserve0         :2;              //保留(填充不做要求)
    uint8_t  Reserve1;                         //保留(填充0xFF)
    uint8_t  Reserve2;                         //保留(填充0xFF)
    uint8_t  Reserve3;                         //保留(填充0xFF)
    uint8_t  Reserve4;                         //保留(填充0xFF)
    uint8_t  Reserve5;                         //保留(填充0xFF)
    uint8_t  Reserve6;                         //保留(填充0xFF)
}thaisenYT_CFC;

typedef struct
{
    uint8_t  CurrOffset;                       //电流偏移(8bits，100A/bit；范围 4-20)(电流偏移量随CFC 发送的值而改动 )
    uint8_t  GunNum           :4;              //表示 BMS 检测到的有效插枪数量
    uint8_t  Ack              :2;              //应答信号(00：没有收到充电机发送的 CFC 报文
                                               //         01：已经收到 CFC 报文，但报文中的充电枪数量等与 BMS 检测到的不一致；电流偏移量由充电机决定，BMS 随之更改，此不做检测
                                               //         10：无效
                                               //         11：已经收到 CFC 报文，且报文中的充电枪数量与 BMS 检测到的一致)
    uint8_t  Reserve0         :2;              //保留(填充不做要求)
    uint8_t  Reserve1;                         //保留(填充0xFF)
    uint8_t  Reserve2;                         //保留(填充0xFF)
    uint8_t  Reserve3;                         //保留(填充0xFF)
    uint8_t  Reserve4;                         //保留(填充0xFF)
    uint8_t  Reserve5;                         //保留(填充0xFF)
    uint8_t  Reserve6;                         //保留(填充0xFF)
}thaisenYT_BFC;

/************************* 电流无偏移FAST 协议 *************************/
typedef struct
{
     uint8_t  Discern;     //辨识结果 00:未识别 AA:识别
     uint32_t ChagNum;     //充电机编号
     uint8_t  ChagPlace[3];//充电机所在区域编号 ASCII码 3byte
}thaisenCRM;

typedef struct
{
     uint8_t  BMSVer[3];         //BMS版本号 3byte
     uint8_t  BatType;           //电池类型 01:铅酸 02:镍氢 03:磷酸铁锂 04:锰酸锂 05:钴酸锂 06:三元材料 07:聚合物锂 08:钛酸锂 FF:其他
     uint16_t BatRateCap;        //动力电池额定容量       0.1AH/bit 0-1000AH
     uint16_t BatRateVolt;       //动力电池额定总电压     0.1V/bit 0-750V
     uint8_t  BatFirm[4];        //电池生产厂商 ASCII码 4byte
     uint8_t  SerialNum[4];      //电池组序号 4byte
     uint8_t  BatBuldyear;       //电池生产日期 1年/bit 偏移1985 1985-2235
     uint8_t  BatBuldmonth;      //1月/bit
     uint8_t  BatBuldday;        //1日/bit
     uint8_t  Chagtimer[3];      //电池充电次数 3byte
     uint8_t  BatProperty;       //电池组产权标识 0:租赁 1:自有
     uint8_t  reserved ;         //预留
     uint8_t  CarDiscern[17];    //车辆识别信息 17byte
     uint8_t  BMSVerNum[8];
}thaisenBRM;

typedef struct
{
    thaisenYT_CFC YT_CFC;                      //宇通CFC报文
    thaisenYT_BFC YT_BFC;                      //宇通BFC报文
    thaisenCRM    CRM_Start;                   //起始CRM报文
    thaisenCRM    CRM_End;                     //结束CRM报文
    thaisenBRM    BRM;                         //BRM报文

    uint8_t ProtocolType;                      //大电流协议类型@thaisenSuperCurrProtocolEnum
    uint16_t CurrOffset;                       //电流偏移(0.1A)
    struct
    {
        uint8_t IsMatchingProtocol : 1;        //已匹配到大电流协议
        uint8_t IsLockedYT_CFC : 1;            //已锁定宇通CFC报文
        uint8_t IsLockedYT_BFC : 1;            //已锁定宇通BFC报文
        uint8_t IsLockedCRM_Start : 1;         //已锁定宇通CRM起始报文
        uint8_t IsLockedCRM_End : 1;           //已锁定宇通CRM结束报文
        uint8_t IsLockedBRM : 1;               //已锁定BRM报文
        uint8_t reserve : 2;                   //预留
    }bit;
}thaisenSuperCurrProtocol;

/* 功能说明:
 *          thaisenGetSuperCurrProtocolInfo:获取超级电流协议信息
 * 输入参数:
 *                  gunNum:充电枪号
 * 返回参数:
 *         超级电流协议信息@thaisenSuperCurrProtocol(枪号不对返回NULL)
 * 调用方法:
 *         实时调用
 */
thaisenSuperCurrProtocol *thaisenGetSuperCurrProtocolInfo(uint8_t gunNum);

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
    thaisen_chargeCtl_stopWay_MainCabinet_Offline,                                        /** 系统停充原因：主机柜离线 */
    thaisen_chargeCtl_stopWay_MatrixRelay_KPN1_1,
    thaisen_chargeCtl_stopWay_MatrixRelay_KPN1_2,
    thaisen_chargeCtl_stopWay_MatrixRelay_KPN1_3,
    thaisen_chargeCtl_stopWay_MatrixRelay_KPN2_1,
    thaisen_chargeCtl_stopWay_MatrixRelay_KPN2_2,
    thaisen_chargeCtl_stopWay_MatrixRelay_KPN3_1,
    thaisen_chargeCtl_stopWay_SlaveDevice_Offline,
    thaisen_chargeCtl_stopWay_Fan,
    thaisen_chargeCtl_stopWay_MainCabinet_Scram,
    thaisen_chargeCtl_stopWay_MainCabinet_Gate,
    thaisen_chargeCtl_stopWay_MainCabinet_PduFault,
    thaisen_chargeCtl_stopWay_MainCabinet_ModuleFault,
    thaisen_chargeCtl_stopWay_MainCabinet_Config,
    thaisen_chargeCtl_stopWay_MainCabinet_AcRelay,
    thaisen_chargeCtl_stopWay_MainCabinet_Smoke,
    thaisen_chargeCtl_stopWay_MainCabinet_Pour,
    thaisen_chargeCtl_stopWay_MainCabinet_Flooding,
    thaisen_chargeCtl_stopWay_MainCabinet_Other,                                          /** 系统停充原因：主机柜其它故障 */
    thaisen_chargeCtl_stopWay_MainCabinet_LightProtect,                                   /** 系统停充原因：主机柜防雷故障 */
    thaisen_chargeCtl_stopWay_DeviceIsLocked,
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
/************************风扇工作时间*********************************************/

/* 功能说明:
 *        thaisen_Set_FanCtrlTime:设置停充后风扇工作时间(ms)
 * 输入参数:       _time    时间(ms)
 * 返回参数:
 *          无
 * 调用方法:
 *             可实时调用
 */
void thaisen_Set_FanCtrlTime(uint32_t _time);

/* 功能说明:
 *        thaisen_Get_FanCtrlTime:获取停充后风扇工作时间(ms)
 * 输入参数:
 * 返回参数:       停充后风扇工作时间(ms)
 *          无
 * 调用方法:
 *             可实时调用
 */
uint32_t thaisen_Get_FanCtrlTime(void);

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
    thaisenChargFunctionEnable_BayArea,                   /** 功能使能：湾区 协议 */
    thaisenChargFunctionEnable_BatVolt,                   /** 功能使能：预充电池电压检测 */
    thaisenChargFunctionEnable_BCLTimeout,                /** 功能使能：BCL报文超时检测 */
    thaisenChargFunctionEnable_BMSSFrame,                 /** 功能使能：BMS多帧支持 */
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
