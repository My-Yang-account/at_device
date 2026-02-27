#ifndef _thaisenBMS_h
#define _thaisenBMS_h

#include "thaisen7102Public.h"

#define THAISEN_CHARGELIB_INCLUDE_V2G    // 包含V2G //




struct thaisenCM_RTSstruct
{
    uint16_t ControlByte;//控制字节 1
    uint16_t MessageByte;//整个消息的字节数
    uint16_t  Packet;//全部数据的包数 2
    uint32_t  PGN;
    uint16_t  rev_info;
};


struct thaisenCM_CTSstruct
{
    uint8_t   ControlByte;//控制字节
    uint8_t   CanSendDataPacket;//可以发送的数据包数
    uint8_t   NextSendDataPacketNum;//下一个要发送的数据包编号
    uint32_t  PGN;
};


struct thaisenCHMstruct
{
    uint8_t ver[3];//版本号
};

struct thaisenBHMstruct
{
     uint16_t  MaxAllowVol; //最高允许电压
     uint16_t  rev_info;
};


struct thaisenCRMstruct
{
     uint8_t  Discern;     //辨识结果 00:未识别 AA:识别
     uint32_t ChagNum;     //充电机编号
     uint8_t  ChagPlace[3];//充电机所在区域编号 ASCII码 3byte
};


struct thaisenBRMstruct
{
        uint8_t  BMSVer[3];         //BMS版本号 3byte
        uint8_t  BatType;           //电池类型 01:铅酸 02:镍氢 03:磷酸铁锂 04:锰酸锂 05:钴酸锂 06:三元材料 07:聚合物锂 08:钛酸锂 FF:其他
        uint16_t BatRateCap;        //动力电池额定容量       0.1AH/bit 0-1000AH
        uint16_t BatRateVolt;       //动力电池额定总电压     0.1V/bit 0-750V
      uint8_t  BatFirm[4];      //电池生产厂商 ASCII码 4byte
      uint8_t  SerialNum[4];    //电池组序号 4byte
        uint8_t  BatBuldyear;   //电池生产日期 1年/bit 偏移1985 1985-2235
        uint8_t  BatBuldmonth;  //1月/bit
        uint8_t  BatBuldday;      //1日/bit
      uint8_t  Chagtimer[3];  //电池充电次数 3byte
        uint8_t  BatProperty;   //电池组产权标识 0:租赁 1:自有
        uint8_t  reserved ;         //预留
        uint8_t  CarDiscern[17];//车辆识别信息 17byte
        uint8_t  BMSVerNum[8];
        uint16_t  rev_info;
};



struct thaisenBCPstruct
{
        uint16_t CellAlowHigVolt;   //单体最高允许电压   0.01V/bit  0-24V
        int16_t  AlowCurlt;         //最高允许充电电流   0.1A/bit  偏移-400
        uint16_t BatRateKW;         //动力电池额定总能量 0.1KW/bit
        uint16_t BatAlowHigVolt;      //最高允许充电总电压 0.1V/bit
        int16_t  BatAlowHigTemp;    //最高允许充电温度   0.1°/bit   偏移-50
        uint16_t SOC;               //动力电池电荷状态   0.1%/bit
        uint16_t BatVolt;             //动力电池当前电压   0.1V/bit
        uint16_t BatVoltTemp;       //动力电池当前电压   0.1V/bit
        uint16_t  rev_info;
};

struct thaisenCTSstruct
{
        uint8_t Time[7];
};

struct thaisenCMLstruct
{
        uint16_t  ChagHigOutVolt;    //充电机最高输出电压   0.1V/bit  0-750V
        uint16_t  ChagLowOutVolt;    //充电机最低输出电压   0.1V/bit  0-750V
        uint16_t  ChagHigOutCurlt;   //充电机最高输出电流   0.1A/bit  -400-0 偏移-400
        uint16_t  ChagLowOutCurlt;   //充电机最小输出电流   0.1A/bit  -400-0 偏移-400
};


struct thaisenBROstruct
{
        uint8_t  BMSReady;
        uint16_t  rev_info;
};


struct thaisenCROstruct
{
        uint8_t  ChagReady;
};

struct thaisenBCLstruct
{
        uint16_t  BMSneedVolt;     //BMS需求电压   0.1V/bit  0-750V
        int16_t     BMSneedCurlt;    //BMS需求电流   0.1A/bit  -400-0 偏移-400
        uint8_t   ChagModel;       //充电模式      01:恒压充电  02:恒流充电
        uint16_t  rev_info;
};


struct thaisenBCSstruct
{
        uint16_t  ChargVolt;          //BMS检测电压   0.1V/bit  0-750V
        int16_t   ChargCurlt;         //BMS检测电流   0.1A/bit  -400-0 偏移-400
        uint16_t  CellHigVolt   :12;    //最高单体电压  0.01V/bit  0-24V
        uint16_t  HigVoltCellNum:4;     //最高单体电压编号
        uint16_t    SOC;                    //当前SOC
        uint16_t  SurplChgTime;         //剩余充电时间
        uint16_t  rev_info;
};


struct thaisenCCSstruct
{
        uint16_t  OutputVolt;     //充电机输出电压   0.1V/bit  0-750V
        uint16_t  OutputCurlt;    //充电机输出电流   0.1A/bit  -400-0 偏移-400
        uint16_t  TotalChgTime;   //累计充电时间
        uint8_t   AllowStop;      //允许暂停位
};

struct thaisenBSMstruct
{
        uint8_t  HigVoltCellNum;  //最高单体电压单体所在编号
        int8_t   HigTemp;         //动力电池最高温度   0.1°/bit  -50-200 偏移-50
        uint8_t  HigTempNum;      //最高温度检测点编号
        int8_t   LowTemp;         //动力电池最低温度   0.1°/bit  -50-200 偏移-50
        uint8_t  LowTempNum;      //最低温度检测点编号

        uint8_t  CellOverVolt:2;  //单体过压           00:正常  01:过高 01:过低
        uint8_t  SOCState    :2;  //SOC状态            00:正常  01:过高 01:过低
        uint8_t  BatOverCurlt:2;  //电池充电过流       00:正常  01:过高 01:过低
        uint8_t  BatOverTemp :2;  //电池温度过高       00:正常  01:过高 01:过低

        uint8_t  Insulat     :2;  //电池绝缘状态       00:正常  01:不正常 01:不可信状态
        uint8_t  OutConect   :2;  //输出连接器状态     00:正常  01:不正常 01:不可信状态
        uint8_t  AllowChg    :2;  //允许充电           00:禁止  01:允许
        uint8_t  :2;
        uint16_t  rev_info;
};


struct thaisenBSTstruct
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
    uint16_t  rev_info;
};



struct thaisenCSTstruct
{
    uint8_t  AutoStop            :2; //自动终止充电       00:正常    01:达到充电机设定  01:不可信状态
    uint8_t  ManStop             :2; //人工终止充电       00:正常    01:人工终止        01:不可信状态
    uint8_t  FaultStop           :2; //故障终止           00:正常    01:故障终止        01:不可信状态
    uint8_t  BMSInitiStop        :2; //BMS主动终止        00:正常    01:BMS中止(收到BST_A)10:不可信状态

    uint8_t  ChgOVtemp           :2; //充电机过温故障     00:正常  01:故障 01:不可信状态
    uint8_t  ChgConectfault      :2; //充电机连接器故障   00:正常  01:故障 01:不可信状态
    uint8_t  ChgOVtempIn         :2; //充电机内部过温故障 00:正常  01:故障 01:不可信状态
    uint8_t  EnergyTranFault     :2; //能量不能传送       00:正常  01:故障 01:不可信状态
    uint8_t  EmgcyStop           :2; //急停故障           00:正常  01:故障 01:不可信状态
    uint8_t  OtherFault          :2; //其他故障           00:正常  01:故障 01:不可信状态
    uint8_t                      :4;

    uint8_t  Curltfault          :2; //电流不匹配         00:电流匹配  01:电流不匹配   01:不可信状态
    uint8_t  Voltfault           :2; //电压异常           00:正常      01:电压异常     01:不可信状态
    uint8_t                      :4;
};


struct thaisenBSDstruct
{
    uint8_t  StopSOC;      //终止电荷状态       1%/bit    0-100%
    uint16_t CellLowVolt;  //最低单体电压       0.01V/bit  0-24V
    uint16_t CellHigVolt;  //最高单体电压       0.01V/bit  0-24V
    uint8_t  LowTemp;      //动力电池最低温度   0.1°/bit  -50-200 偏移-50
    uint8_t  HigTemp;      //动力电池最高温度   0.1°/bit  -50-200 偏移-50
    uint16_t  rev_info;
};

struct thaisenCSDstruct
{
    uint16_t TotalChgTime;//累计充电时间
    uint16_t OutputKWh;   //输出能量            0.1KWh/bit  0-1000KWh
    uint8_t  ChgNum[4];   //充电机编号
};

struct thaisenBEMstruct
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

    uint16_t  rev_info;
};

struct thaisenBEMstructV2G
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

    uint8_t  AuxDisOVtime     :2; //接收充（放）电机 A+、 A-断开超时      00:正常   01:超时  01:不可信状态
    uint8_t  CMLPOVtime       :2; //接收充（放）电机最大输出能力扩展超时      00:正常   01:超时  01:不可信状态
    uint8_t  CCDOVtime        :2; //接收充（放）电机充（放）电方向请求报文超时      00:正常   01:超时  01:不可信状态
    uint8_t  CSDPOVtime       :2; //接收充（放）电机统计数据扩展超时超时      00:正常   01:超时  01:不可信状态

    uint8_t  ParaNoMatch      :2; //车辆充电参数不匹配      00:正常   01:异常  其它:不可信状态
    uint8_t  Reserve1          :6;

    uint8_t  Reserve2;
    uint8_t  Reserve3;

    uint16_t  rev_info;
};


struct thaisenCEMstruct
{
    uint8_t  BRMOVtime        :2; //接收BRM超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :6;
    uint8_t  BCPOVtime        :2; //接收BCP超时     00:正常   01:超时  01:不可信状态
    uint8_t  BROOVtime        :2; //接收BRO_A超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :4;

    uint8_t  BCSOVtime        :2; //接收BCS超时     00:正常   01:超时  01:不可信状态
    uint8_t  BCLOVtime        :2; //接收BCL_A超时     00:正常   01:超时  01:不可信状态
    uint8_t  BSTOVtime        :2; //接收BST_A超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :2;

    uint8_t  BSDOVtime        :2; //接收BSD_A超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :6;
};

struct thaisenCEMstructV2G
{
    uint8_t  BRMOVtime        :2; //接收BRM超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :6;
    uint8_t  BCPOVtime        :2; //接收BCP超时     00:正常   01:超时  01:不可信状态
    uint8_t  BROOVtime        :2; //接收BRO_A超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :4;

    uint8_t  BCSOVtime        :2; //接收BCS超时     00:正常   01:超时  01:不可信状态
    uint8_t  BCLOVtime        :2; //接收BCL_A超时     00:正常   01:超时  01:不可信状态
    uint8_t  BSTOVtime        :2; //接收BST_A超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :2;

    uint8_t  BSDOVtime        :2; //接收BSD_A超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :6;

    uint8_t  BDCOVtime        :2; //接收车辆功能兼容性报文超时     00:正常   01:超时  01:不可信状态
    uint8_t  BCPPOVtime       :2; //接收电池充电参数扩展报文超时     00:正常   01:超时  01:不可信状态
    uint8_t                   :4;

    uint8_t  InsultRes        :2; //充电机绝缘检测结果    00:正常   01:异常  其它:不可信状态
    uint8_t  RelayAdh         :2; //充电机接触器粘连检测结果     00:正常   01:异常  其它:不可信状态
    uint8_t  BatVoltSta       :2; //充电机检测电池电压状态     00:正常   01:异常  其它:不可信状态
    uint8_t  SoftBoot         :2; //充电机软启动     00:正常   01:异常  其它:不可信状态

    uint8_t  Reserve1;
    uint8_t  Reserve2;
};

struct thaisenCFCstruct
{
    uint8_t  CurrOffset;          //电流偏移(8bits，100A/bit；范围 4-20， 双枪充电默认为6，受电弓充电默认 12)
    uint8_t  GunNum           :4; //表示充电机检测到的有效插枪数量
    uint8_t  Ack              :2; //应答信号(00：与 CRM 一起发送，作为通知 BMS 充电机的协议为电流偏移量自动识别
                                  //         01：已经收到 BFC 的反馈，但 BMS 回复的电流偏移量或检测到的充电枪数量等信息不符
                                  //         10：无效
                                  //         11：已经收到 BFC 的反馈，且 BMS 回复的电流偏移量和检测到的充电枪数量等信息均一致)
    uint8_t  Reserve0         :2; //保留(填充不做要求)
    uint8_t  Reserve1;            //保留(填充0xFF)
    uint8_t  Reserve2;            //保留(填充0xFF)
    uint8_t  Reserve3;            //保留(填充0xFF)
    uint8_t  Reserve4;            //保留(填充0xFF)
    uint8_t  Reserve5;            //保留(填充0xFF)
    uint8_t  Reserve6;            //保留(填充0xFF)
    uint8_t  IsStop;              //停止
};

struct thaisenBFCstruct
{
    uint8_t  CurrOffset;          //电流偏移(8bits，100A/bit；范围 4-20)(电流偏移量随CFC 发送的值而改动 )
    uint8_t  GunNum           :4; //表示 BMS 检测到的有效插枪数量
    uint8_t  Ack              :2; //应答信号(00：没有收到充电机发送的 CFC 报文
                                  //         01：已经收到 CFC 报文，但报文中的充电枪数量等与 BMS 检测到的不一致；电流偏移量由充电机决定，BMS 随之更改，此不做检测
                                  //         10：无效
                                  //         11：已经收到 CFC 报文，且报文中的充电枪数量与 BMS 检测到的一致)
    uint8_t  Reserve0         :2; //保留(填充不做要求)
    uint8_t  Reserve1;            //保留(填充0xFF)
    uint8_t  Reserve2;            //保留(填充0xFF)
    uint8_t  Reserve3;            //保留(填充0xFF)
    uint8_t  Reserve4;            //保留(填充0xFF)
    uint8_t  Reserve5;            //保留(填充0xFF)
    uint8_t  Reserve6;            //保留(填充0xFF)
    uint16_t rev_info;
};

/********************** V2G 报文 **********************/
struct thaisenCDCstruct
{
    uint8_t ver[3];               //版本号
};

struct thaisenBDCstruct
{
    uint8_t  CarDiscern[17];      //车辆识别信息 17byte
    uint8_t SOC;                  //动力电池电荷状态   0.1%/bit
    uint16_t BatRateCap;          //动力电池额定容量       0.1AH/bit
    uint16_t BatRateKW;           //动力电池额定总能量 0.1KW/bit
    uint16_t V2GLoopedCount;      //参与 V2G 的循环次数/次， 0.1 次/位， 0 次偏移量
    uint16_t RemainV2GLoopedCount;//剩余 V2G 的循环次数/次， 0.1 次/位， 0 次偏移量
    uint16_t RemainMileage;       //剩余续航里程/公里， 0.1 公里/位， 0 公里偏移量
    uint16_t  rev_info;
};

struct thaisenBCPPstruct
{
    int16_t  AlowCurlt;           //最高允许放电电流   0.1 A/位， -400A 偏移量；数据范围： 0~400A
    uint16_t BatAlowLowVolt;      //最低允许放电总电压 0.1 V/位， 0 V 偏移量
    uint16_t CellAlowLowVolt;     //最低允许单体电池电压   0 V 偏移量；数据范围： 0~24 V
    uint8_t BatAlowLowSOC;        //最低允许动力蓄电池荷电状态   1%/位， 0%偏移量；数据范围： 0~100%
    uint16_t rev_info;
};

struct thaisenCMLPstruct
{
    uint16_t  DisChagHigOutVolt;  //最高放电电压（V）   0.1 V /位， 0 V 偏移量
    uint16_t  DisChagLowOutVolt;  //最低放电电压（V）   0.1 V /位， 0 V 偏移量
    uint16_t  DisChagHigOutCurlt; //最大放电电流（A）   0.1 A/位， -400A 偏移量；数据范围：0~400A
    uint16_t  DisChagLowOutCurlt; //最小放电电流（A）   0.1 A/位， -400A 偏移量；数据范围： 0~400A
};

struct thaisenCCDstruct
{
    uint8_t  Direction;           //充电机充放电方向（<0x00 >： =充电 ; <0x01 >： =放电
    uint8_t  AlowHigSOC;          //本次调度最高允许 SOC 值   1%/位， 0%偏移量；数据范围： 0~100%
    uint8_t  AlowLowSOC;          //本次调度最低允许 SOC 值   1%/位， 0%偏移量；数据范围： 0~100%
};

struct thaisenBCSPstruct
{
    uint16_t  SurplDisChgTime;    //估算剩余放电时间(当车辆以实际电流为准进行测算的剩余时间超过 600 min 时，按 600 min 发送。数据
                                  //             分辨率： 1 min/位， 0 min 偏移量；数据范围： 0~600 min。处于充电状态时，缺省值为FF)
    uint16_t  ChargVolt;          //剩余续航里程/公里， 0.1 公里/位， 0 公里偏移量
    uint32_t  Reserve;            //预留
    uint16_t  rev_info;
};

struct thaisenCSDPstruct
{
    uint16_t TotalDisChgTime;     //累计放电时间  1 min/位， 0 min 偏移量；数据范围： 0~600 min；缺省值为 FF
    uint16_t OutputKWh;           //放电电量    0.1 kWh/位， 0 kWh 偏移量；数据范围： 0~1000 kWh；缺省值为FF
    uint8_t  DisChgNum[4];        //充（放）电机编号， 1/位， 1 偏移量，数据范围： 0～0xFFFFFFFF
};


struct thaisenBMS_Charger_struct
{
    struct thaisenCM_RTSstruct CM_RTS; /* 充电机握手报文 */
    struct thaisenCM_CTSstruct CM_CTS; /* 充电机发送时间同步信息报文 */
    struct thaisenCHMstruct CHM;       /* 充电机握手报文 */
    struct thaisenBHMstruct BHM;       /* 车辆握手报文 */
    struct thaisenCRMstruct CRM;       /* 充电机辨识报文 */
    struct thaisenBRMstruct BRM;       /* BMS和车辆辨识报文 */
    struct thaisenBCPstruct BCP;       /* 动力蓄电池充电参数报文 */
    struct thaisenCTSstruct CTS;       /* 充电机发送时间同步信息报文 */
    struct thaisenCMLstruct CML;       /* 充电机最大输出能力报文 */
    struct thaisenBROstruct BRO;       /* 电池充电准备就绪状态报文 */
    struct thaisenCROstruct CRO;       /* 充电机输出准备就绪状态报文 */
    struct thaisenBCLstruct BCL;       /* 电池充电需求报文 */
    struct thaisenBCSstruct BCS;       /* 电池充电总状态报文 */
    struct thaisenCCSstruct CCS;       /* 充电机充电状态报文 */
    struct thaisenBSMstruct BSM;       /* 动力蓄电池状态信息报文 */
    struct thaisenBSTstruct BST;       /* BMS终止充电报文报文 */
    struct thaisenCSTstruct CST;       /* 充电机终止充电文 */
    struct thaisenBSDstruct BSD;       /* BMS统计数据报文报文 */
    struct thaisenCSDstruct CSD;       /* 充电机统计数据报文 */
    struct thaisenBEMstruct BEM;       /* BMS错误报文 */
    struct thaisenCEMstruct CEM;       /* 充电机错误报文 */
    struct thaisenCFCstruct CFC;
    struct thaisenBFCstruct BFC;
#ifdef THAISEN_CHARGELIB_INCLUDE_V2G
    struct thaisenBEMstructV2G BEM_V2G;
    struct thaisenCEMstructV2G CEM_V2G;
    struct thaisenCDCstruct CDC;
    struct thaisenBDCstruct BDC;
    struct thaisenBCPPstruct BCPP;
    struct thaisenCMLPstruct CMLP;
    struct thaisenCCDstruct CCD;
    struct thaisenBCSPstruct BCSP;
    struct thaisenCSDPstruct CSDP;
    uint32_t overTick[5];
    uint8_t commonCnt;
#else
    uint32_t overTick[4];
    uint8_t commonCnt;
#endif /* THAISEN_CHARGELIB_INCLUDE_V2G */

};


struct thaisenBMS_ChargerID_struct
{
    can_msg_buf CM_RTS_ID;
    can_msg_buf CM_CTS_ID;
    can_msg_buf CM_DT_ID;
    can_msg_buf DM1_ID;
    can_msg_buf DM3_ID;
    can_msg_buf CHM_ID;
    can_msg_buf BHM_ID;
    can_msg_buf CRM_ID;
    can_msg_buf CTS_ID;
    can_msg_buf CML_ID;
    can_msg_buf BRO_ID;
    can_msg_buf CRO_ID;
    can_msg_buf BCL_ID;
    can_msg_buf CCS_ID;
    can_msg_buf BSM_ID;
    can_msg_buf BST_ID;
    can_msg_buf CST_ID;
    can_msg_buf BSD_ID;
    can_msg_buf CSD_ID;
    can_msg_buf BEM_ID;
    can_msg_buf CEM_ID;
    can_msg_buf CFC_ID;
    can_msg_buf BFC_ID;
#ifdef THAISEN_CHARGELIB_INCLUDE_V2G
    can_msg_buf CDC_ID;
    can_msg_buf BDC_ID;
    can_msg_buf BCPP_ID;
    can_msg_buf CMLP_ID;
    can_msg_buf CCD_ID;
    can_msg_buf BCSP_ID;
    can_msg_buf CSDP_ID;
#endif /* THAISEN_CHARGELIB_INCLUDE_V2G */
};

#endif



