#include "serialScreen.h"
#include "string.h"
#include "app_data_info_interface.h"
#include "thaisen7102Public.h"
#include "thaisenChargModuleLib.h"

#define DBG_LEVEL DBG_LOG

#define DEBUGMSG(...)       do{\
                                printf(__VA_ARGS__);\
                            }while(0)

#define qDEBUG_FRAME(msg,data,len) \
								do{\
									DEBUGMSG(msg);\
									for(int ii=0;ii<len;ii++)\
									{\
										DEBUGMSG("%02x ",data[ii]);\
									}\
									DEBUGMSG("\r\n");\
								}while(0);
								
								
//#define sSCREEN_DATA_OUTPUT_ON
//#define sSCREEN_EVENT_DEBUG_OUTPUT_ON
//#define sSCREEN_RXDATA_OUTPUT_ON
//#define sSCREEN_TXDATA_OUTPUT_ON
//#define sSCREEN_DATA_PORCESS_ON
//#define sSCREEN_KEY_PORCESS_ON


#ifdef	sSCREEN_EVENT_DEBUG_OUTPUT_ON
#define	sSCREEN_EVENT_DEBUGMSG(...)			DEBUGMSG(__VA_ARGS__)
#else
#define	sSCREEN_EVENT_DEBUGMSG(...)
#endif


#ifdef	sSCREEN_RXDATA_OUTPUT_ON
#define sSCREEN_DEBUGRxDATA(str,buf,len)		qDEBUG_FRAME(str,buf,len)
#else
#define sSCREEN_DEBUGRxDATA(str,buf,len)
#endif

#ifdef	sSCREEN_TXDATA_OUTPUT_ON
#define sSCREEN_DEBUGTxDATA(str,buf,len)		qDEBUG_FRAME(str,buf,len)
#else
#define sSCREEN_DEBUGTxDATA(str,buf,len)
#endif

#ifdef sSCREEN_DATA_PORCESS_ON
#define	sSCREEN_DEBUGPROMSG(...)			DEBUGMSG(__VA_ARGS__)
#else
#define	sSCREEN_DEBUGPROMSG(...)
#endif

#ifdef sSCREEN_KEY_PORCESS_ON
#define	sSCREEN_DEBUGKEYMSG(...)			DEBUGMSG(__VA_ARGS__)
#else
#define	sSCREEN_DEBUGKEYMSG(...)
#endif


							
#ifdef	sSCREEN_DEBUG_OUTPUT_ON
#define	sSCREEN_DEBUGMSG(...)			DEBUGMSG(__VA_ARGS__)
#else
#define	sSCREEN_DEBUGMSG(...)
#endif

#define SYSTEM_WARNNING_INFO_NORMAL                       0x00    /* 告警信息：正常 */
#define SYSTEM_WARNNING_INFO_INSULT_PROPERTIES            0x01    /* 告警信息：绝缘性能异常 */
#define SYSTEM_WARNNING_INFO_INSULT_VOLTAGE               0x02    /* 告警信息：绝缘电压异常 */
#define SYSTEM_WARNNING_INFO_REQUEST_CURRENT              0x03    /* 告警信息：请求电流异常 */

#ifdef APP_DESIGNATE_REGION
#define SERIALSCREEN_DESIGNATE_REGION
#endif /* APP_DESIGNATE_REGION */

#ifdef SERIALSCREEN_DESIGNATE_REGION
#define SERIALSCREEN_DEF_TCMRAM CFG_DEF_TCMRAM
#define SERIALSCREEN_DEF_SRAM0 CFG_DEF_SRAM0
#define SERIALSCREEN_DEF_SRAM1 CFG_DEF_SRAM1
#define SERIALSCREEN_DEF_SRAM2 CFG_DEF_SRAM2
#else
#define SERIALSCREEN_DEF_TCMRAM
#define SERIALSCREEN_DEF_SRAM0
#define SERIALSCREEN_DEF_SRAM1
#define SERIALSCREEN_DEF_SRAM2
#endif /* SERIALSCREEN_DESIGNATE_REGION */


#define SCREEN_USING_TXT_RTC                 /* 发送RTC TXT */
#ifdef SCREEN_USING_TXT_RTC
#define SCREEN_TXT_RTC_STRLEN       21       /* RTC TXT 字符串长度 */
#define SCREEN_TXT_RTC_PERIOD       500      /* RTC TXT 发送间隔(ms) */
#define SCREEN_TXT_RTC_ADDR         0x6600   /* RTC TXT 控件地址 */
#endif /* SCREEN_USING_TXT_RTC */

#ifdef CP_CONFIG_USING_DUPU
#define SCREEN_USING_DUPU                /* 使用度普屏幕 */
#endif /* CP_CONFIG_USING_DUPU */

#ifdef CP_CONFIG_USING_QBJ
#define SCREEN_USING_QBJ                /* 使用柒捌玖屏幕 */
#endif /* CP_CONFIG_USING_QBJ */

#ifdef CP_USING_OFFLINE_BILLING
#define SCREEN_USING_OFFLINE_BILLING     /* 使用离线计费 */
#endif /* CP_USING_OFFLINE_BILLING */

#ifdef CP_CONFIG_USING_QBJ
#define SCREEN_LIGHTSCREEN_LOCATION_X   700 /* 模拟点亮屏幕的坐标X轴 */
#define SCREEN_LIGHTSCREEN_LOCATION_Y   0   /* 模拟点亮屏幕的坐标Y轴 */
#endif /* CP_CONFIG_USING_QBJ */

#define SCREEN_TRIGGER_WARN_ICON_CARD_LOCKED              0x00    /* 外部触发告警ICON：卡被锁 */
#define SCREEN_TRIGGER_WARN_ICON_INVALID_CARD             0x01    /* 外部触发告警ICON：无效卡 */
#define SCREEN_TRIGGER_WARN_ICON_NO_BALLANCE              0x02    /* 外部触发告警ICON：余额不足 */
#define SCREEN_TRIGGER_WARN_ICON_ILLEGAL_CARD             0x03    /* 外部触发告警ICON：非法卡 */
#define SCREEN_TRIGGER_WARN_ICON_GUN_FIRST                0x04    /* 外部触发告警ICON：先插枪再刷卡 */
#define SCREEN_TRIGGER_WARN_ICON_FEES_ERROR               0x05    /* 外部触发告警ICON：计费信息设置错误 */
#define SCREEN_TRIGGER_WARN_ICON_PAY                      0x06    /* 外部触发告警ICON：充电结束, 刷卡结算 */
#define SCREEN_TRIGGER_WARN_ICON_PAYING                   0x07    /* 外部触发告警ICON：结算中(用于查询历史订单时) */
#define SCREEN_TRIGGER_WARN_ICON_SWITCH_GUN               0x08    /* 外部触发告警ICON：不是启动卡，请切换枪号 */
#define SCREEN_TRIGGER_PAGE_PAY_COMPLETE                  0x09    /* 外部触发页面：结算完成 */
#define SCREEN_TRIGGER_WARN_IS_STARTING                   0x0A    /* 外部触发页面：启动中 */
#define SCREEN_TRIGGER_WARN_IS_CHARGING                   0x0B    /* 外部触发页面：此卡已启动充电 */
#define SCREEN_TRIGGER_WARN_FAULT_STOP                    0x0C    /* 外部触发页面：故障停止，请重新拔、插枪 */
#define SCREEN_TRIGGER_WARN_ICON_SIZE                     0x0D    /* 外部触发告警ICON： */

#define sSCREEN_RX_CMD_MIN_LEN	6 //AA BB len cmd addh addl
								
#define DWIN_FRAM_HEAD1 0x5A
#define DWIN_FRAM_HEAD2 0xA5
#define BASE_SYS_TIMER (10)


#define str_len(src)			strlen((s8 *)src)		//字符串的长度
#define mem_cpy(des,src,n)		memcpy((char *)des,(char *)src,n)
#define mem_ncmp(des,src,n)		memcmp((char *)des,(char *)src,n)
#define mem_set(des,src,n)		memset((char *)des,src,n)
#define str_cpy(des,src)		strcpy((char *)des,(char *)src)
#define str_ncpy(des,src,n)		strncpy((char *)des,(char *)src,n)
#define str_cat(des,src)		strcat((char *)des,(char *)src)
#define  str_ncmp(des,src,len)		strncmp((char*)des,(char*)src,len)
								
#define str_tofloat(src)		atof((s8 *)src)
#define str_toInt(src)			atoi((s8 *)src)
#define str_toLong(src,des,n)	strtol((s8 *)src,(s8 **)des,n)//strtol((gsp_s8 *)src,(gsp_s8 **)des,n)


#define SIZEOF(a) sizeof(a)/sizeof(a[0])
#define SIZE_OF(a,b) sizeof(a)/sizeof(b)


#define LCD_POW_1	10
#define LCD_POW_2	100
#define LCD_POW_3	1000
#define LCD_POW_4	10000
#define LCD_POW_5	100000


#define UI_READ_SINGLE_CFG_STR(n,i)    thaisen_app_read_config_item_port(n, i)
#define UI_SYNC_SINGLE_CFG_STR(n,d,i)  thaisen_app_sync_config_item_port(n,d,i)
#define UI_READ_SINGLE_CFG_DATA(n,i)   thaisen_app_read_config_item_port(n, i)
#define UI_SYNC_SINGLE_CFG_DATA(n,d,i) thaisen_app_sync_config_item_port(n,d,i)
#define UI_STORAGE_CFG_DATA            thaisen_app_storage_config_port()


//Modbus收发数据缓冲区
SERIALSCREEN_DEF_SRAM2 u8 SerialScreenAddr = 1;
SERIALSCREEN_DEF_SRAM2 static ota_info* s_ota_info = NULL;
SERIALSCREEN_DEF_SRAM2 u8 SerialScreenRxbuf[sSCREEN_RX_CMD_MAX_LEN+sSCREEN_RX_CMD_MIN_LEN];


#define LCD_GUN_NUM			2//初始化赋值
#define LCD_MODULE_GROUP_MAX 4//模块组数
#define LCD_GUN_1			0 //Gun_0
#define LCD_GUN_2			1 //Gun_1
#define QRCODE_LEN          150  // 二维码显示长度
#define VIN_LIST_NUM        6  // VIN 白名单个数

#ifdef SCREEN_USING_OFFLINE_BILLING
#define SERIALSCREEN_CONFIG_PAGE_MAX   44  // 屏幕页面总数
#define SERIALSCREEN_PAGE_ITEM_MAX     54  // 屏幕每页信息项总数
#define SERIALSCREEN_TRIGGER_PAGE_MAX  14   // 外部触发页面总数

#define SERIALSCREEN_OB_COUNTDOWN_STRING_MAX   4  //离线计费告警倒计时字符串最大长度
#else
#define SERIALSCREEN_CONFIG_PAGE_MAX   40  // 屏幕页面总数
#define SERIALSCREEN_PAGE_ITEM_MAX     54  // 屏幕每页信息项总数
#endif /* SCREEN_USING_OFFLINE_BILLING */
#define CONFIG_ITEM_MODULE_GROUP_NUM_(X) 

#ifdef USING_DOUBLE_GUN
#define SCREEN_USING_DOUBLE_GUN  /* 使用双枪 */
#endif /* USING_DOUBLE_GUN */

typedef enum SerialScreenReflashTimer
{
	LCD_NoReflash=0,
	LCD_1sReflash=1,
	LCD_5sReflash=5,
	LCD_10sReflash=10,
	LCD_30sReflash=30,
	LCD_60sReflash=60,
	LCD_240Reflash=240,
	LCD_MAX_Reflash = 480
}SerialScreenReflashTimer;


typedef enum SerialScreenCheckStatus
{
	sSCREEN_BUF_ERROR = -3,
	sSCREEN_ADDR_ERROR = -2,
	sSCREEN_CHK_ERROR = -1,
	sSCREEN_CHK_LESS  = 0,
	sSCREEN_CHK_OK	 = 1
}sSCREEN_CHECK_STATUS;


enum LCD_DISPLAY_ITEM_TYPE{
	LCD_IconType,
	LCD_DataType,
	LCD_TextType,
	LCD_QRCodeType,
	LCD_InputType,
	LCD_inputPwdType,	//密码
	LCD_BtnType,	//切换界面
	LCD_TrigType,	//触发，刷新界面
	LCD_BtnAType,	//A枪，切换界面
	LCD_BtnBType,	//B枪，切换界面
	LCD_BtnHomeType,	//主页，切换界面
    LCD_Timeype,    //对时
	LCD_ITEM_MAX_Type
}LCD_DISPLAY_ITEM_TYPE;

enum LCD_RXDATA_STEP_TYPE{
	LCD_DATA_STEP_HEAD = 1,
	LCD_DATA_STEP_LEN,
	LCD_DATA_STEP_DATA,
	LCD_DATA_STEP_END
}LCD_RXDATA_STEP_TYPE;

#ifdef SCREEN_USING_QBJ
enum LCD_IDLE_ICON{
    LCD_IDLE_ICON_LOGO_QBJ = 19,         //柒捌玖首页空闲icon：logo
    LCD_IDLE_ICON_PACCOUNT_QBJ = 20,     //柒捌玖首页空闲icon：公众号
};
#endif /* SCREEN_USING_QBJ */

enum LCD_DISPLAY_PAGE_TYPE{
	LCD_PAGE_NONE = 0,
	LCD_PAGE_STANDBY = 1,		//待机界面
	LCD_PAGE_A_SELECT = 2,		//充电模式选择
	LCD_PAGE_B_SELECT = 3,		//充电模式选择
	LCD_PAGE_A_START = 4,		//启动中
	LCD_PAGE_B_START = 5,		//启动中
	LCD_PAGE_A_CHGING = 6,		//A充电中
	LCD_PAGE_B_CHGING = 7,	//A充电中(电池信息)
	LCD_PAGE_A_CHGING_BAT = 8,		//B充电中
	LCD_PAGE_B_CHGING_BAT = 9,	//B充电中(电池信息)
	LCD_PAGE_A_ACOUNT = 10,		//充电账单
	LCD_PAGE_B_ACOUNT = 11,		//充电账单
	LCD_PAGE_A_ERR = 12,			//故障
	LCD_PAGE_B_ERR = 13,			//故障
	LCD_PAGE_A_STOPING = 14,		//充电停机中
	LCD_PAGE_B_STOPING = 15,		//充电停机中
	LCD_PAGE_A_START_FAIL  = 16,	//启动失败
	LCD_PAGE_B_START_FAIL  = 17,	//启动失败
	LCD_PAGE_MENU_COM_1 = 18,		//系统设置1     桩信息
	LCD_PAGE_MENU_COM_2 = 19,		//系统设置2 服务器信息
	LCD_PAGE_MENU_COM_3 = 20,		//系统设置3 电表
	LCD_PAGE_MENU_COM_4 = 21,		//系统设置4 模块
	LCD_PAGE_MENU_COM_5 = 22,		//系统设置5 故障信息 A
	LCD_PAGE_MENU_COM_6 = 23,		//系统设置6 充电信息 A
	LCD_PAGE_ADMIN_PASWD = 24,		//管理员密码
	LCD_PAGE_MENU_COM_6B = 27,		//系统设置6B
	LCD_PAGE_MENU_COM_5B = 28,		//系统设置5B
	LCD_PAGE_PASWD_ERR = 29,
	LCD_PAGE_SYS_UPDATE = 37,		//远程升级
	LCD_PAGE_SYS_INFO = 38,			//系统信息
	LCD_PAGE_ROOT_MAIN = 39,		//ROOT用户
	LCD_PAGE_MENU_COM_7 = 40,	//VIN码白名单
	LCD_PAGE_MENU_INPUT = 41, 	//输入信息
	LCD_PAGE_MENU_OUTPUT = 42, 	//输出信息
	LCD_PAGE_MENU_PROTECT = 43, 	//保护信息
	LCD_PAGE_MENU_CONFIG = 44,	//功能配置
	LCD_PAGE_MENU_MONITOR = 45, //监控信息
	LCD_PAGE_MENU_MONITOR_B = 46, //监控信息
	LCD_PAGE_MENU_INOUT = 47,	//输入输出
	LCD_PAGE_MENU_INOUT_B = 48,	//输入输出
	LCD_PAGE_MENU_STATE_MODULE = 49,//模块状态
	LCD_PAGE_MENU_STATE_MODULE_B = 50,//模块状态
	LCD_PAGE_MENU_CONTROL_MODULE = 51,//模块控制
	LCD_PAGE_MENU_CONTROL_MODULE_B = 52,//模块控制
	LCD_PAGE_MENU_SYS = 54,	//系统

#ifdef SCREEN_USING_OFFLINE_BILLING
    LCD_PAGE_OFFLINE_BILLING = 62, //离线计费
    LCD_PAGE_WARNNING_INFO = 78, //告警信息
    LCD_PAGE_OB_PYA_A = 79, //离线计费A枪结算
    LCD_PAGE_OB_PYA_B = 80, //离线计费B枪结算
#endif /* SCREEN_USING_OFFLINE_BILLING */
    LCD_PAGE_STORAGE_WAITING = 66, //保存等待
    LCD_PAGE_CLEAR_RECORD_WAITING = 67, //清除记录等待
};

extern struct SerialScreenObj SerialScreen;

typedef s32 (*ConfigExecutPool)(u8, void*, void*, void*);

#pragma pack(1)
//辅组数据
struct LCD_ASSISTANT_DATA{
    struct{
        u16 IsLongLiSerialScreen : 1;    //龙立屏幕
        u16 IsEnableParaCharge : 1;      //已打开并充使能
        u16 IsEnableAuxPower24V : 1;     //已打开并充使能
        u16 ParaChargeSelect : 1;        //已选择并充
        u16 IsSetPowerPercent : 1;       //已设置功率百分比
        u16 IsSetELossProportion : 1;    //已设置电损比
        u16 IsClickReboot : 1;           //已点击重启
        u16 IsCountDownFinish : 1;       //启动倒计时已结束
        u16 IsConfigFail : 1;            //配置保存失败
        u16 NeedReboot : 1;              //需要重启
    }Flag;

    struct{
        u8 AuxPower24VSelect : 1;        //24V辅源选择状态
        u8 AuxPower24VSelectLast : 1;    //24V辅源前一次选择状态
        u8 IsPowerOn : 1;                //上电开机
        u8 IsVinStart : 1;               //启动方式为VIN码
        u8 DataIsVerify : 1;             //配置数据已确认
        u8 IsPWStartAuthen : 1;          //填密码是密码启动鉴权
    }SeveralGunFlag[LCD_GUN_NUM];

    u8 OccupyGunNum;                     //处于占用但未充电的枪数量
    u8 DeviceType;                       //设备类型
    u8 RefrenshPeriod;                   //实时数据更新周期
};

#ifdef SCREEN_USING_OFFLINE_BILLING
struct LCD_TRIGGER_ITEM{
    u8 page;                             //页面
    u16 time;                            //持续时间
    u8 ShowPara;                         //展示页面参数
    u8 ShieldPara;                       //隐藏页面参数
    struct{
        u8 JustNotice;                   //这只是弹icon提示, 不需要网页面上发数据
    }Flag;
};

struct LCD_TRIGGER{
    u16 CountDown;                       //倒计时
    u8 Warnning;                         //告警ICON
    u8 TimeBaseTick;                     //倒计时时基
    u8 LastPage;                         //上一页面
    struct{
        u8 IsModify : 1;                //触发事件已修改
        u8 IsTriggerExternal : 1;       //有外部触发事件
        u8 IsPayed : 1;                 //已支付
        u8 IsWaitPay : 1;               //触发了等待支付事件
    }Flag;
    struct LCD_TRIGGER_ITEM item;
};

struct LCD_TRIGGER_PAGE{
    u8 page;                            //页面
    u8 ShowPara;                        //展示页面参数
    u8 ShieldPara;                      //隐藏页面参数
};

struct Period_Time{
    u8 shour;                                         /** 时段开始：小时 */
    u8 smin;                                          /** 时段开始：分钟 */
    u8 ehour;                                         /** 时段结束：小时 */
    u8 emin;                                          /** 时段结束：分钟 */
    u8 rate_number;                                   /** 费率号 */
};
#endif /* SCREEN_USING_OFFLINE_BILLING */

#pragma pack()

struct LCD_RXDATA_VALUE_TYPE{
	u8 rstep;
	u16 rlen;
	u16 tlen;
	u8 rxbuf[sSCREEN_RX_CMD_MAX_LEN];
	struct LCD_DATA_FIFO_TYPE *pDataFifo;
}LCD_RXDATA_VALUE_TYPE_t;


struct LCD_TXDATA_VALUE_TYPE {
    u8 len;
    u8 buf[sSCREEN_TX_CMD_MAX_LEN];
};

typedef int  (*FuncIsOk)(int);

struct LCD_DISPLAY_INDEX_TYPE{
    FuncIsOk isOk;  //1 允许 0 不允许 条件
    uint8_t have_name;
    u8 type;
    u16 reflash; //需要刷新 0 不用刷新 低字节刷新间隔s
    u16 regaddr;
    u8 valtype;
    u16 vallen;
    void *valaddr;
}LCD_DISPLAY_INDEX_TYPE_t;


struct LCD_DISPLAY_PAGE_INDEX_TYPE{
	u8 page; //页面
    struct LCD_DISPLAY_INDEX_TYPE item[SERIALSCREEN_PAGE_ITEM_MAX];
}LCD_DISPLAY_PAGE_INDEX_TYPE_t;

struct LCD_DISPLAY_RUNDATA_TYPE{
//	u8 timer[20]; //2021-03-16 21:30:30
//	u8 temp[6]; //-30.5
	

	u8 bothChgFlg;	//1 并充 0 单充
	u8 netstate;  //1ok 2nok
	u8 chgcode[LCD_GUN_NUM][30];			//设备编码
	u8 pileID[20];							//设备地址
	s8 syncTimeFlg;	//同步时间标志 0:需要
}LCD_DISPLAY_RUNDATA_TYPE_t;


struct LCD_DISPLAY_DATE_TYPE{
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 min;
	u8 sec;
}LCD_DISPLAY_DATE_TYPE_t;

struct LCD_DISPLAY_SETDATA_TYPE{
//	u8 chgcode[LCD_GUN_NUM][30];			//设备编码
	u8 pileID[20];							//设备地址
	u8 MeterAddr[LCD_GUN_NUM][13];
    u16 MeterModel;
    u16 MeterCheckWay;
    u16 MeterBaudrate;
	u8 ErWeiCode[LCD_GUN_NUM][QRCODE_LEN];
	u8 ErWeiCodePre[128];
	u8 SerialScreen_PassWordShow;           //屏幕密码显示
//	f32 Magnification;						//电表倍率
//	u32 Accuracy;							//电表精度(小数点位数)
	u8 GunNum;								//枪总数
    u8 TimeSync_Flag;                       //对时设置标志
//	u8 PowerDividerIsSupport;
//	u8 passwd[20];	

//	struct LCD_DISPLAY_DATE_TYPE Date;

//	u8 servetype;							//后台类型
//	u8 netmode;								//联网方式 LAN 0-disable 1-dhcp 2-static
//	u8 G4Type;								//4G 选项 pcie LAN 0-disable 1-luat 2-quectel //0禁止4g ，1合轴模块，2移远模
#if 0
	struct in_addr ip;						//ip地址[0][APPCFG_NO_LIMIT,0,APPCFG_NO_LIMIT]
	struct in_addr gwip;					//网关[0][APPCFG_NO_LIMIT,0,APPCFG_NO_LIMIT]
	struct in_addr Dns; 					//dns[0][APPCFG_NO_LIMIT,0,APPCFG_NO_LIMIT]

	struct in_addr NetMask;					//子网掩码[0][APPCFG_NO_LIMIT,0,APPCFG_NO_LIMIT]
#endif
	u8 svrIp[20];					//服务器IP[0][APPCFG_NO_LIMIT,0,APPCFG_NO_LIMIT]
	u16 svrPort; 						//服务器端口[0][APPCFG_NO_LIMIT,0,APPCFG_NO_LIMIT]
//	u8 YuMing[256]; 

    u16 AllocWay;                           //分配方式
    u16 DevType;                            //设备类型
    u16 NetType;                            //联网方式
	u16 RmType;								//模块类型
	u8 AgunRmNum;							//A枪模块个数
	u8 BgunRmNum;							//B枪模块个数
	u8 ModuleGroupNum;					//模块组数
	u8 ModuleNum[LCD_MODULE_GROUP_MAX];	//模块组个数
	u16 Rated_Output_Voltage;           // 额定输出电压
    u16 Max_Output_Voltage;             // 最高输出电压
    u16 Min_Output_Voltage;             // 最低输出电压
    u16 Rated_Limit_Current;            // 额定限电流
    u16 Max_Limit_Current;              // 最高限电流
    u16 Min_Limit_Current;              // 最低限电流
//	u8 ChargePasswd[6];					//充电密码
	//u8 AdmindPasswd[10];				//管理员密码     1314
	u8 UserPasswd[10];					//用户密码 0909
    u8 UserPasswdShow[10];              //用户密码(用于展示)

	u32 Longitude;					//经度 0.000000
	u32 Latitude;					//纬度 0.000000

    u8 parallel_iocn;
    /***********************warnning***************************/
	u8 warnning[LCD_GUN_NUM];
	/***********************factory set***************************/
	/***********************support set**************************/
	u8 supin_scram;							//急停输入启用
	u8 supin_gate;							//门禁输入启用
	u8 supin_ac;							//交流输入接触器启用
	u8 supin_dc;							//直流输入接触器启用
	u8 supin_fan;							//风扇输入启用
	u8 supin_elock;							//电子输入锁启用
    u8 supin_temp_pro;                      //温度保护启用
    u8 supin_protectlight;                  //防雷器检测启用
    u8 supin_gunsite;                       //枪座检测启用
    u8 supin_circuit_breaker;               //断路器检测启用
    u8 supin_flood;                         //水浸检测启用
    u8 supin_smoke;                         //烟感检测启用
    u8 supin_pour;                          //倾倒检测启用
    u8 supin_liquid;                        //液冷检测启用
    u8 supin_fuse;                          //熔断器检测启用
	/***********************neg set***************************/
	u8 neg_scram;							//急停输入取反
	u8 neg_gate;							//门禁输入取反
	u8 neg_ac;								//交流接触器输入取反
	u8 neg_dc;								//直流接触器输入取反
	u8 neg_fan;								//风扇输入取反
	u8 neg_elcok;							//电子锁输入取反
    u8 neg_protectlight;                    //防雷器输入取反
    u8 neg_gunsite;                         //枪座输入取反
    u8 neg_circuit_breaker;                 //断路器输入取反
    u8 neg_flood;                           //水浸输入取反
    u8 neg_smoke;                           //烟感输入取反
    u8 neg_pour;                            //倾倒输入取反
    u8 neg_liquid;                          //液冷输入取反
    u8 neg_fuse;                            //熔断器输入取反

	u8 supout_ac;							//交流输入接触器启用
	u8 supout_elock;						//电子输入锁启用
	u8 supout_fan;							//风扇输入启用

	u8 sup_Local;							//本地充电支持
    u8 sup_pw_start;                        //密码充电支持
    u8 sup_Local_stop;                      //本地停止支持
	u8 sup_insulation;						//绝缘检测支持
	u8 sup_usecard;							//刷卡支持
    u8 sup_mslience;                        //模块静音支持
    u8 sup_offbilling;                      //离线计费支持
	u8 sup_Qrcode;							//APP支持
	u8 sup_VIN;								//VIN码支持
	u8 sup_net;								//网络支持
	u8 mode_auth;							//认证模式
	u8 sup_elelock;							//电子锁支持
	u8 elelockLogic;						//电子锁逻辑

    u8 sup_auxp_24V;                        //24V辅源支持
	u8 sup_parallelchg;						//并充支持
	u8 sup_parallelrelay;                   //支持并联
    u8 sup_offline_card;                    //离线卡支持

	u8 fan_type;							//风扇类型
	u8 fan_frequency;						//风扇频率
	u8 FanCtrlPulse;						//风扇占空比
	u8 App_SoftWareVersion[20];				//软件版本号
	u8 SIM_card[21];						//SIM卡
    u32 SIM_Strength;                        //SIM卡 信号强度
	u8 Help_Number[32];						//帮助电话
	u8 Sup_Stop;                            //是否支持本地停止
    u8 Sup_PlugAndPlay;                     //是否支持即插即充
	u8 Sup_StartStyle[3][LCD_MODULE_GROUP_MAX];	//[0]:本地 [1]:VIN [2]:并充
	u8 manufacturer;						// 0:thaisen 1:NULL
    u8 s_selectaux[LCD_GUN_NUM];            // 辅源选择
    /***********************set icon***************************/
    u8 Icon_SuplocalStop;                   //本地停止使能icon
    u8 Icon_SupPlugAndPlay;                 //即插即充使能icon
    u8 Icon_SupOfflineBilling;              //离线计费使能icon
    u8 Icon_SupPWStart;                     //密码启动使能icon
    u8 Icon_SupOffCard;                     //离线卡支持icon
    u8 Icon_SupProtectLight;                //防雷器检测支持icon
    u8 Icon_SupGunSite;                     //枪座检测支持icon
    u8 Icon_SupCircuitBreaker;              //断路器检测支持icon
    u8 Icon_SupFlood;                       //水浸检测支持icon
    u8 Icon_SupSmoke;                       //烟感检测支持icon
    u8 Icon_SupPour;                        //倾倒检测支持icon
    u8 Icon_SupLiquid;                      //液冷检测支持icon
    u8 Icon_SupFuse;                        //熔断器检测支持icon
    /***********************neg icon***************************/
    u8 Icon_NegProtectLight;                //防雷器输入取反icon
    u8 Icon_NegGunSite;                     //枪座输入取反icon
    u8 Icon_NegCircuitBreaker;              //断路器输入取反icon
    u8 Icon_NegFlood;                       //水浸输入取反icon
    u8 Icon_NegSmoke;                       //烟感输入取反icon
    u8 Icon_NegPour;                        //倾倒输入取反icon
    u8 Icon_NegLiquid;                      //液冷输入取反icon
    u8 Icon_NegFuse;                        //熔断器输入取反icon
    /***********************protect info***************************/
	u32 Input_OverVolt;                     // 输入过压
    u32 Input_UnderVolt;                    // 输入欠压
    u32 Onput_OverVolt;                     // 输出过压
    u32 Onput_UnderVolt;                    // 输出欠压
    u32 Onput_OverCurr;                     // 输出过流
    u32 Stop_SOC;                           // 停充SOC
    u32 OverTemp_Warnning;                  // 过温告警
    u32 OverTemp_Stop;                      // 过温停充
    u32 OverTemp_Resume;                    // 过温恢复
    u32 OverTemp_LimitCurr;                 // 过温限流
    u16 PowerPercent;                       // 功率百分比
    u16 ElossProprotion;                    // 电损比
    u16 GunVolt_LimitValue;                 // 枪头电压限值
	/***********************factory debug***************************/
	u8 s_elElock[LCD_GUN_NUM];				//电磁锁设置
	u32 g_elElock[LCD_GUN_NUM];				//电磁锁反馈
	u8 s_dcRelay[LCD_GUN_NUM];				//输出接触器设置
	u32 g_dcRelay[LCD_GUN_NUM];				//输出接触器反馈
	u8 s_auxRelay[LCD_GUN_NUM];				//辅电接触器设置
	u32 g_auxRelay[LCD_GUN_NUM];			//辅电接触器反馈
	u8 s_acRely;							//AC接触器设置
	u32 g_acRely;							//AC接触器反馈
	u8 s_paraRely0;							//并联接触器0设置
    u8 s_paraRely1;                         //并联接触器1设置
    u8 s_paraRely2;                         //并联接触器2设置
	u32 g_paraRely0;					    //并联接触器0反馈
    u32 g_paraRely1;                        //并联接触器1反馈
    u32 g_paraRely2;                        //并联接触器2反馈
	u32 g_emergency;							//急停按钮反馈
	u8 s_fan[LCD_GUN_NUM];								//风扇控制
	u32 g_door;								//门禁反馈
    u32 g_aux24v[LCD_GUN_NUM];              //24V辅源反馈
    u32 g_pour;                             //倾倒反馈
    u32 g_protect_light;                    //防雷反馈
    u32 g_flood;                            //水浸反馈
    u32 g_smoke;                            //烟感反馈
    u32 g_gunsite[LCD_GUN_NUM];             //枪座反馈
    u32 g_fuse[LCD_GUN_NUM];                //熔断器反馈

    u8 aux24v_set[LCD_GUN_NUM];             //24v辅电接触器设置
	/***********************Module debug***************************/
	u32 g_chargeVol[LCD_GUN_NUM];			//充电电压
	u32 g_chargeCur[LCD_GUN_NUM];			//充电电流
	u32 g_meterVol[LCD_GUN_NUM];			//电表电压
	u32 g_meterCur[LCD_GUN_NUM];			//电表电流
	u32 g_cc1Vol[LCD_GUN_NUM];				//CC1电压
	u32 g_uiVol[LCD_GUN_NUM];				//输入电压U
	u32 g_viVol[LCD_GUN_NUM];				//输入电压V
	u32 g_wiVol[LCD_GUN_NUM];				//输入电压W
	u32 g_portTemp[LCD_GUN_NUM];			//枪头温度
	u32 s_moduleVol[LCD_GUN_NUM];			//设置电压
	u32 s_moduleCur[LCD_GUN_NUM];			//设置电流
    /***********************VIN list***************************/
    u8 s_vin_lists[VIN_LIST_NUM][18];           //VIN 码白名单
    /***********************OTA***************************/
    u16 ota_progress;

	u32 s_TimeSync[6];                 //对时
    /***********************fees***************************/
	uint32_t period_time[4];                //当前时段
    uint32_t period_price;                  //当前时段电费单价
#ifdef SCREEN_USING_DUPU
    /***********************stored energy***************************/
    u16 StoredEnergy_Soc;              //储能SOC
    u32 StoredEnergy_Power;            //储能最大功率
#endif /* SCREEN_USING_DUPU */
    /***********************starting info***************************/
    u8 chargeState[LCD_GUN_NUM];            //充电状态
    s32 samplingVolt[LCD_GUN_NUM];          //采样电压
    s32 moduleVolt[LCD_GUN_NUM];            //模块电压
    s32 batteryVolt[LCD_GUN_NUM];           //电池电压
    s32 maxChargeVolt[LCD_GUN_NUM];         //最大充电电压
#ifdef SCREEN_USING_OFFLINE_BILLING
    /***********************offline billing***************************/
    u8 OBCountDown;                         //离线计费倒计时
    u8 OBwarning;                           //offline billing信息告警
    u8 OBEventwarning;                      //offline billing触发事件告警
    u32 ServicePrice;                       //服务费
    u32 SsElectPrice;                       //尖尖电费(Ss:sharp sharp)
    u32 SElectPrice;                        //尖电费(S:sharp)
    u32 PElectPrice;                        //峰电费(P:peak)
    u32 FElectPrice;                        //平电费(F:flat)
    u32 VElectPrice;                        //谷电费(V:valley)
    struct Period_Time PeriodTime[CP_RATED_TYPE_NUM_MAX][CP_RATED_TYPE_PERIOD_NUM];  //时段时间

    u8 OB_CountDownString[SERIALSCREEN_OB_COUNTDOWN_STRING_MAX];
#endif /* SCREEN_USING_OFFLINE_BILLING */
#ifdef SCREEN_USING_TXT_RTC
    u8 rtc_time[SCREEN_TXT_RTC_STRLEN];
    u32 ScreenBaseTime;
    u32 ScreenBaseTick;
#endif /* SCREEN_USING_TXT_RTC */
    /***********************保护信息底图icon***************************/
    u8 Icon_ProtectInfo;                        //保护信息底图icon

}LCD_DISPLAY_SETDATA_TYPE_t;


struct LCD_DISPLAY_GUN_VALUE_TYPE{
	u16 startCountTimer;	//启动倒计时
	f32 Unit_Price;

	u8 ErrCode[10];	//错误码
    u8 ErrCode_Chinese[32]; //错误码(中文)
	u8 code_stopResaon[6];	//错误码
    u8 code_stopResaon_Chinese[32]; //错误码(中文)
//	u8 stopReson[80];	//停止原因
//	u8 Aux12v;	//1 12 2 24
	u32 vol;	//输出电压
	u32 cur; 	//输出电流
	u32 engery;	//电量
	u32 curSoc;//100
	u8 soc[5];		//首页soc
	u32 VolNeed;	//电压需求
	u32 CurNeed;	//电流需求
	u32 BatTemp;
	u32 SigleVol; 
//	u8 ChrgeRunTime[6]; //00:25
	u32 ChrgeTime;
	u32 ChrgeTimeHour;
	u32 ChrgeTImeMin;
#ifdef SCREEN_USING_QBJ
	u32 RemainTime[2]; //小时：分钟
#endif /* SCREEN_USING_QBJ */
//	u8 waitCardReadTimer;
    u8 workStateLast;
	u8 workState;
	u8 iocnState;	
	u8 portState;	//枪口状态 1 连接 0 断开
	u32 totalFee;      //费用：精度3位
	u32 AccountBallance;  //账户余额
}LCD_DISPLAY_GUN_VALUE_TYPE_t;

struct LCD_DISPLAY_VALUE_TYPE{
	u8 gunIndex;
	//u16 CurrentPage[LCD_GUN_NUM];	//当前页
	//u16 CurrentPageBack[LCD_GUN_NUM];
	u8 CurrentPage;	//当前页
	u8 CurrentPageBack;
	struct LCD_DISPLAY_PAGE_INDEX_TYPE *pPageIndex[LCD_GUN_NUM];
	Sq_Queue List;//序列
	
	u16 KeyReg;	//寄存器地址
	u16 KeyVal; //寄存器值
	u16 KeyTimer;	//控制按键间隔
	u8 KeyInput[100]; //键盘输入字符串
	u8 KeyInputLen;   //键盘输入字符串有效长度

	u8 BillIndex_Overreturn[LCD_GUN_NUM];
	u8 BillCurrentIndex[LCD_GUN_NUM]; // 订单当前下标
    u8 BillLable[LCD_GUN_NUM]; // 订单标号
	s16 BillIndex[LCD_GUN_NUM];	//账单索引
	s16 BillNum[LCD_GUN_NUM];	//充电记录总数
	u8 billInfo[10][100];

    u8 ErrIndex_Overreturn[LCD_GUN_NUM];
    u8 ErrCurrentIndex[LCD_GUN_NUM]; // 故障当前下标
    u8 ErrLable[LCD_GUN_NUM]; // 故障标号
	s16 ErrIndex[LCD_GUN_NUM];	//故障索引
	s16 ErrNum[LCD_GUN_NUM]; 	//故障记录总数
	u8 ErrInfo[10][100];
	
	u8 menuflg;	   //进入menu标志
	u8 debugIOflg;	//进入IO Debug标志
	u8 NeedMenuOffFlg;		//触发界面自动消失
	u8 Homeflg;			//主页按钮触发标志
	u16 NeedMenuOffTimer;	//触发界面自动消失计时
	u32 PageCountDown;      //页面倒计时
	u8 AccountPageCountDown_Over;  //结算页面倒计时结束
    u8 ChargingPageCountDown_Over;  //充电页面倒计时结束
//	u8 RxData;
    /***********************Module state***************************/
    u8 ModuleStateString[16][30];  // 模块状态

	struct LCD_DISPLAY_RUNDATA_TYPE runData;
	struct LCD_DISPLAY_SETDATA_TYPE setData;
    struct LCD_DISPLAY_GUN_VALUE_TYPE gun[LCD_GUN_NUM];
}LCD_DISPLAY_VALUE_TYPE_t;

#if 0
typedef struct
{
	u8 id[40];
	u8 start[20];		//开始时间
	u8 stop[20];		//结束时间
	u8 startsoc; 		//开始SOC
	u8 stopsoc; 		//结束SOC
	f32 khw;			//已充电量
	f32 moneny;			//已充金额
	u8 gunId;			//枪号
	u16 stopreson;		//停止原因 hex
}DEALINFO_STRUCT;

typedef struct
{
	u16 num;
	u16 numCounter;
	DEALINFO_STRUCT DealInfo[1024];
}DEAL_STRUCT;


DEAL_STRUCT DealSInfo;
#endif


SERIALSCREEN_DEF_SRAM2 struct LCD_DISPLAY_VALUE_TYPE LcdData;
SERIALSCREEN_DEF_SRAM2 struct LCD_RXDATA_VALUE_TYPE LcdRxData;
SERIALSCREEN_DEF_SRAM2 struct LCD_TXDATA_VALUE_TYPE LcdTxData;
SERIALSCREEN_DEF_SRAM2 struct LCD_ASSISTANT_DATA LcdAssistantData;
#ifdef SCREEN_USING_OFFLINE_BILLING
SERIALSCREEN_DEF_SRAM2 struct LCD_TRIGGER LcdTriggerEvent[LCD_GUN_NUM];
#endif /* SCREEN_USING_OFFLINE_BILLING */

SERIALSCREEN_DEF_SRAM2 ConfigExecutPool LcdConfigExecutPool[THAISEN_CONFIG_PAGE_SIZE];

enum SYSMAIN_STATUS{
	SysMainStatus_StandBy,		//空闲状态(主状态)0
	SysMainStatus_PlugIn,		//已插枪 1
	SysMainStatus_StartReady,	//过程状态 2
	SysMainStatus_SelfCheck,	//自检状态 3
	SysMainStatus_SelfCheck_Wait,//自检确认状态4
	SysMainStatus_Chrging,		//充电状态5
	SysMainStatus_StopChg,		//停止状态6
	SysMainStatus_Account,		//结算状态7
	SysMainStatus_Parking,		//停车费收取状态8
	SysMainStatus_ParkAccount,	//停车费结算状态9
	SysMainStatus_Setting,		//设置状态10
	SysMainStatus_Other,		//其他状态11
	SysMainStatus_Err,			//故障状态12
};

enum ICON_CHARGING_SOC{
	ICON_CHARGING_SOC0 = 13,
	ICON_CHARGING_SOC20,
	ICON_CHARGING_SOC40,
	ICON_CHARGING_SOC60,
	ICON_CHARGING_SOC80,
	ICON_CHARGING_SOC100,
};

enum ICON_CHARGE_STYLE{
	ICON_CHARGE_LOCAL = 0,
	ICON_CHARGE_NULL ,
	ICON_CHARGE_VIN ,
    ICON_CHARGE_DOUBLE,
	ICON_CHARGE_PW,
};

enum CHARGE_STYLE{
    CHARGE_STYLE_START_LOCAL,
    CHARGE_STYLE_START_VIN,
    CHARGE_STYLE_START_PW,
};

enum ICON_AUXPOWER_SELECT{
    ICON_AUXPOWER_12V,
    ICON_AUXPOWER_24V ,
    ICON_AUXPOWER_NONE = 4 ,
};

enum ICON_CHARGEWAY_SELECT{
    ICON_CHARGEWAY_PARACHARGE = 9,
    ICON_CHARGEWAY_SINGLECHARGE = 8,
    ICON_CHARGEWAY_NONE = 4 ,
};
//离线计费信息告警
enum ICON_OB_WARNNING{
    ICON_OB_NULL,
    ICON_OB_TIME_DUPLICATE,  //时间段重复
    ICON_OB_NOT_CONTINUOUS,  //时间段不连续
    ICON_OB_IS_CHARGING,     //启动或充电中不允许修改
};


enum MODEL_DATA_ENUM{
    pu8_type,
    pu16_type,
    pu32_type,
    ps8_type,
    ps16_type,
    ps32_type,
    pu8x10_type,
    pu16x10_type,
    pu32x10_type,
    ps8x10_type,
    ps16x10_type,
    ps32x10_type,
    pu8x100_type,
    pu16x100_type,
    pu32x100_type,
    ps8x100_type,
    ps16x100_type,
    ps32x100_type,
    pu16x1000_type,
    pu32x1000_type,
    ps16x1000_type,
    ps32x1000_type,
    pf32_type, //默认2为小数
    pf32_1type,
    pf32_3type,
    pf32_4type,
    pf32_5type, //5位小数
    pf64_type,
    pstr_type,
    date_type,  //日期
    date_year_type,  //2000~2200
    date_month_type, //1~12
    date_day_type, //1~31
    date_hour_type, //0~24
    date_min_type, //0~59
    ip_type,
    pu8_nH_type,	//n个数据 hex格式显示
    pu8_nD_type,	//n个数据
    pQString_type,
    pQStingList_type,
    pQStingListTab_u8type, //翻译表
    pQStingListTab_u16type, //翻译表
    pQStingListTab_u32type, //翻译表
    page_type,		//表示第几页或第几组
    menu_type,
    MODE_DATA_TYPE_MAX
};

enum CONNECT_STATE{
    GUN_CONNECT_STATE_NO,    /* 未连接 */
    GUN_CONNECT_STATE_YES,   /* 已连接 */
};

enum REALAY_STATE{
	REALAY_OFF = 0,			//继电器断开
    REALAY_CLOSE,	//继电器闭合
    
};

#ifdef SCREEN_USING_OFFLINE_BILLING
SERIALSCREEN_DEF_SRAM2 static struct LCD_TRIGGER_PAGE Trigger_Page[SERIALSCREEN_TRIGGER_PAGE_MAX] =
{
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_CARD_LOCKED, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_INVALID_CARD, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_NO_BALLANCE, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_ILLEGAL_CARD, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_GUN_FIRST, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_FEES_ERROR, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_PAY, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_PAYING, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_ICON_SWITCH_GUN, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_OB_PYA_A, 0x00, 0x00},
    {LCD_PAGE_OB_PYA_B, 0x00, 0x00},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_IS_STARTING, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_IS_CHARGING, SCREEN_TRIGGER_WARN_ICON_SIZE},
    {LCD_PAGE_WARNNING_INFO, SCREEN_TRIGGER_WARN_FAULT_STOP, SCREEN_TRIGGER_WARN_ICON_SIZE},
};
#endif /* SCREEN_USING_OFFLINE_BILLING */

#ifdef SERIALSCREEN_DESIGNATE_REGION
SERIALSCREEN_DEF_TCMRAM static struct LCD_DISPLAY_PAGE_INDEX_TYPE LCD_ALL_PAGE_TAB[SERIALSCREEN_CONFIG_PAGE_MAX];
#else
__attribute__((section(".ARM.__at_0x10000000"))) struct LCD_DISPLAY_PAGE_INDEX_TYPE LCD_ALL_PAGE_TAB[SERIALSCREEN_CONFIG_PAGE_MAX];
#endif /* SERIALSCREEN_DESIGNATE_REGION */

static void SerialScreen_ScreenSet_TimeSync_Flag(void);
static void SerialScreen_BtnModuleStateClear(void);
static void SerialScreen_BtnModuleStateShow(int port);
void SerialScreen_SendIco(struct SerialScreenObj *cmd,u16 addr, u16 par);
void SerialScreen_JumpPage(struct SerialScreenObj *cmd,u8 page);

static void SerialScreen_BtnSystemFuncJudge(u32 *ret);
static void SerialScreen_BtnMeterNoInfoJudge(u32 *ret);
static void SerialScreen_BtnModuleInfoJudge(u32 *ret);
static void SerialScreen_BtnProtectInfoJudge(u32 *ret);
static void SerialScreen_IsSupportInfoJudge(u32 *ret);
static void SerialScreen_OfflineBillingInfoJudge(u32 *ret);

typedef void(*dofun_void)(int);

static void SerialScreen_ItemSetUp(u8 page, FuncIsOk isOk, const char *name, u8 type,   \
        u16 reflash, u16 regaddr, u8 valtype, u16 vallen, void *valaddr)
{
    static u8 count = 0, item_index = 0, last_page = LCD_PAGE_STANDBY;

    if(count >= SERIALSCREEN_CONFIG_PAGE_MAX){
        return;
    }
    if((item_index >= SERIALSCREEN_PAGE_ITEM_MAX) && (last_page == page)){
        return;
    }
    if(last_page != page){
        if(last_page > page){
            return;
        }
        item_index = 1;
        last_page = page;
        count++;
    }else{
        item_index++;
    }

    if(item_index < 1){
        return;
    }

    LCD_ALL_PAGE_TAB[count].page = page;
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].isOk = isOk;
    if(name){
        LCD_ALL_PAGE_TAB[count].item[item_index - 1].have_name = 1;
    }else{
        LCD_ALL_PAGE_TAB[count].item[item_index - 1].have_name = 0;
    }
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].type = type;
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].reflash = reflash;
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].regaddr = regaddr;
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].valtype = valtype;
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].vallen = vallen;
    LCD_ALL_PAGE_TAB[count].item[item_index - 1].valaddr = valaddr;
}

#define LCD_ALL_PAGE_TAB_LEN 	sizeof(LCD_ALL_PAGE_TAB)/sizeof(LCD_ALL_PAGE_TAB[0])


s32 String2BCD(s8* pSrc, u8* pDest)
{
	s32 index = 0;
	s32 h = 0;
	s32 l = 0;
	s32 i = 0;
	s32 sign = 0;
	u8 temp = 0;
	s32 length = strlen(pSrc);
	if (length%2!=0){
		sign = 1;
	}

	memset(pDest,0,length/2+length%2);

	for(i=0; i<length; i+=2)
	{
		if(!isdigit(pSrc[i]))
			return -1;
		index = length-1-i;
		if((sign==1)&&(index==0)){
			h = 0;
		} else {
			h = pSrc[index-1]-0x30;
		}
		l = pSrc[index]-0x30;
		temp = h<<4|l;
		*(pDest++) = temp ;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static u32 SerialScreen_Calculate_Data_From_Byte(u8* data, u8 len, u8 flag)
{
    if(len >sizeof(u32)){
        return 0;
    }

    u32 result = 0;
    s8 i = 0;

    /* 低字节在前 */
    if(flag &(1 <<1)){
        for(i = len - 1; i >= 0; i--){
            result |= data[i];
            if(i > 0)       result <<= 8;
        }
    }
    /* 高字节在前 */
    else if(flag &(1 <<0)){
        for(i = 0; i < len; i++){
            result |= data[i];
            if(i < len - 1)       result <<= 8;
        }
    }
    return result;
}


static void DataToString(char* dbuff, u8 dlen, u8* sbuff, u8 slen)
{
    if((dbuff == NULL) || (sbuff == NULL)){
        return;
    }
    if(dlen < 2 *slen){
        return;
    }
    u8 count = 0;
    for(; count < slen; count++){
        if(sbuff[count] < 0x10){
            sprintf((dbuff + 2 *count), "0%x", sbuff[count]);
        }else{
            sprintf((dbuff + 2 *count), "%x", sbuff[count]);
        }
    }
}

static u8 Get_Data_Value_Nob(u32 value)
{
    u32 base = 1, i = 0, j = 0;
    for(i = 0; ; i++){
        base = 1;
        for(j = 0; j < i; j++){
            base *= 10;
        }
        if((value /base) == 0)
            return i;
    }
}

static void MakeString(char* buff, u8 slen, u32 value, u32 ratio)
{
    if(buff == NULL){
        return;
    }
    if(ratio == 0){
        memset((char*)buff, '\0', slen);
        sprintf((char*)buff, "%lu", value);
        return;
    }

    u8 nob_ratio, nob_diff, nob_int;
    u16 _int, _diff;
    _int = value /ratio;
    _diff = value %ratio;
    nob_ratio = Get_Data_Value_Nob(ratio);
    nob_ratio = nob_ratio == 0 ? 0 : nob_ratio - 1;

    nob_diff = Get_Data_Value_Nob(_diff);
    nob_diff = nob_diff > nob_ratio ? nob_ratio : nob_diff;

    nob_int = Get_Data_Value_Nob(_int);
    if(nob_int > slen){
        return;
    }

    memset(buff, '\0', slen);
    sprintf((char*)buff, "%u%c", _int, '.');
    if((strlen((char*)buff) + nob_ratio + 1) > slen){
        return;
    }

    memset((buff + strlen((char*)buff)), '0', nob_ratio);
    if(_diff != 0){
        sprintf((buff + strlen((char*)buff) - nob_diff), "%u", _diff);
    }

    if(strlen((char*)buff) < slen){
        u8 remain_len = slen - strlen((char*)buff);
        memset((buff + strlen((char*)buff)), ' ', remain_len);
    }
}

static u32 SerialScreen_GetPara_ValidValue(u32 Para, u32 ValueDefault, u32 ValueMin, u32 ValueMax)
{
    if((Para > ValueMax) || (Para < ValueMin)){
        Para = ValueDefault;
    }
    return Para;
}

void SerialScreen_ScreenGet_TimeSync(u16* buf, u8 len)
{
    if(buf && (sizeof(LcdData.setData.s_TimeSync) <= 2 *len)){
        buf[0] = LcdData.setData.s_TimeSync[0];
        buf[1] = LcdData.setData.s_TimeSync[1];
        buf[2] = LcdData.setData.s_TimeSync[2];
        buf[3] = LcdData.setData.s_TimeSync[3];
        buf[4] = LcdData.setData.s_TimeSync[4];
        buf[5] = LcdData.setData.s_TimeSync[5];
    }
}

s8 SerialScreen_Get_ScreenTimeSync_Flag(void)
{
    if(LcdData.setData.TimeSync_Flag){
        LcdData.setData.TimeSync_Flag = FALSE;
        return TRUE;
    }
    return FALSE;
}

s8 SerialScreen_Get_SetPowerPercent_Flag(void)
{
    if(LcdAssistantData.Flag.IsSetPowerPercent){
        LcdAssistantData.Flag.IsSetPowerPercent = FALSE;
        return TRUE;
    }
    return FALSE;
}

s8 SerialScreen_Get_SetELossProportion_Flag(void)
{
    if(LcdAssistantData.Flag.IsSetELossProportion){
        LcdAssistantData.Flag.IsSetELossProportion = FALSE;
        return TRUE;
    }
    return FALSE;
}

static void SerialScreen_ScreenSet_TimeSync_Flag(void)
{
    LcdData.setData.TimeSync_Flag = 1;
}

void SerialScreen_ScreenSet_Reboot_Flag(void)
{
    LcdAssistantData.Flag.IsClickReboot = TRUE;
}

u8 SerialScreen_ScreenGet_Reboot_Flag(void)
{
    return LcdAssistantData.Flag.IsClickReboot;
}

void SerialScreen_ScreenClear_Reboot_Flag(void)
{
    LcdAssistantData.Flag.IsClickReboot = FALSE;
}

u8 SerialScreen_ScreenGet_Is_FirstPage(void)
{
    if(LcdData.CurrentPage == LCD_PAGE_STANDBY){
        return TRUE;
    }
    return FALSE;
}

void SerialScreen_ScreenSet_CouDownFin_Flag(u8 sta)
{
    if(sta)
        LcdAssistantData.Flag.IsCountDownFinish = TRUE;
    else
        LcdAssistantData.Flag.IsCountDownFinish = FALSE;
}

u8 SerialScreen_Screen_IsCouDownFin_Flag(u8 port)
{
    if(port >= LCD_GUN_NUM)
        return TRUE;

    if(LcdData.gun[port].startCountTimer == 0)
        return TRUE;
    else
        return FALSE;
}

/*********************************************** 外部触发执行配置 ****************************************************/
/*********************************************** 外部触发执行配置 ****************************************************/
/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_System
 * 功能         外部触发执行系统信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_System(u8 port, void *data, void *sub_data, void *sub_sub_data)   // OK
{
    u32 ret = 0;
    thaisen_cfg_info_system *config = (thaisen_cfg_info_system*)data;

    LcdData.setData.AllocWay = config->allocate_way;
    LcdData.setData.DevType = config->dev_function;
    /** 终端地址需要终端程序赋值 */
#if 0
     = config->terminal_addr[0];
     = config->terminal_addr[1];
#endif
     /** 系统信息有效性判断 */
    SerialScreen_BtnSystemFuncJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_BtnSystemFuncSet();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Pile
 * 功能         外部触发执行桩信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Pile(u8 port, void *data, void *sub_data, void *sub_sub_data)  // OK
{
#define SSCREEN_PASSWORD_POSITION    3         /* 屏幕密码(配置条目)在结构体 thaisen_cfg_info_pile 中的成员次序(从0开始)*/

    u32 ret = 0;
    u16 valid_len = 0;
    thaisen_cfg_info_pile *config = (thaisen_cfg_info_pile*)data;

    /** 获取桩信息 */
    SerialScreen_BtnChgInfoGet(port);
    /** >2 是因为前两个字节是设置格式、生成格式，一定有 */
    if((valid_len = strlen((char*)config->qrcode_prefix)) > 2){
        valid_len = valid_len > (sizeof(LcdData.setData.ErWeiCodePre) - 1) ? (sizeof(LcdData.setData.ErWeiCodePre) - 1) : valid_len;
        memset(LcdData.setData.ErWeiCodePre, 0, sizeof(LcdData.setData.ErWeiCodePre));
        memcpy(LcdData.setData.ErWeiCodePre, config->qrcode_prefix, valid_len);
    }
#if 0
    if((valid_len = strlen((char*)config->qrcode_suffix)) > 2){
        valid_len = valid_len > (sizeof(LcdData.setData.ErWeiCodePre) - 1) ? (sizeof(LcdData.setData.ErWeiCodePre) - 1) : valid_len;
        memset(LcdData.setData.ErWeiCodePre, 0, sizeof(LcdData.setData.ErWeiCodePre));
        memcpy(LcdData.setData.ErWeiCodePre, config->qrcode_prefix, valid_len);
    }
#endif
    /** 屏幕密码有效性判断 */
    if((valid_len = strlen((char*)config->screen_password)) > 0){
        u8 i = 0, len = 0;
        for(i = 0; i < sizeof(config->screen_password); i++){
            if((config->screen_password[i] < 0x20) || (config->screen_password[i] > 0x7E) || (config->screen_password[i] == 0x00)){
                len = i;
                break;
            }
        }

        if(len > 0){
            memset(LcdData.setData.UserPasswdShow, '\0', sizeof(LcdData.setData.UserPasswdShow));
            memcpy(LcdData.setData.UserPasswdShow, config->screen_password, len);
            memcpy(LcdData.setData.UserPasswd, LcdData.setData.UserPasswdShow, sizeof(LcdData.setData.UserPasswdShow));

            UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_SCREEN_PASSWORD, (u8 *)(LcdData.setData.UserPasswd), len);
        }else{
            return (SSCREEN_PASSWORD_POSITION + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    if((valid_len = strlen((char*)config->help_number)) > 0){
        valid_len = valid_len > (sizeof(LcdData.setData.Help_Number) - 1) ? (sizeof(LcdData.setData.Help_Number) - 1) : valid_len;
        UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_HELP_PHONE, config->help_number, valid_len);
        memset(LcdData.setData.Help_Number, 0, sizeof(LcdData.setData.Help_Number));
        memcpy(LcdData.setData.Help_Number, config->help_number, valid_len);
    }
#if 0
    /** 桩信息有效性判断 */
    SerialScreen_BtnChgInfoJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
#endif
    SerialScreen_BtnChgInfoSet(LCD_GUN_1);

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Server
 * 功能         外部触发执行服务器信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Server(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
#define SSCREEN_NET_MODE_POSITION    2         /* 网络模式(配置条目)在结构体 thaisen_cfg_info_server 中的成员次序(从0开始)*/
    thaisen_cfg_info_server *config = (thaisen_cfg_info_server*)data;

    if(config->net_mode >= CP_NETTYPE_SIZE){
        return (SSCREEN_NET_MODE_POSITION + THAISEN_CONFIG_FAIL_OFFSET);
    }
    SerialScreen_BtnServerGet();
    LcdData.setData.NetType = config->net_mode;
    SerialScreen_BtnServerSet();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Ammeter
 * 功能         外部触发执行电表信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Ammeter(u8 port, void *data, void *sub_data, void *sub_sub_data)   //OK
{
// @ thaisen_cfg_info_ammeter
#define SCONFIG_METER_ADDR_A_POSITION    0         /* A枪电表地址(配置条目)在结构体 thaisen_cfg_info_ammeter 中的成员次序(从0开始)*/
#define SCONFIG_METER_ADDR_B_POSITION    1         /* B枪电表地址(配置条目)在结构体 thaisen_cfg_info_ammeter 中的成员次序(从0开始)*/
#define SCONFIG_METER_BAUD_POSITION      2         /* 电表通信波特率(配置条目)在结构体 thaisen_cfg_info_ammeter 中的成员次序(从0开始)*/

#define SCONFIG_METER_BAUD_2400          0         /* 电表波特率：2400 */
#define SCONFIG_METER_BAUD_4800          1         /* 电表波特率：4800 */
#define SCONFIG_METER_BAUD_9600          2         /* 电表波特率：9600 */
#define SCONFIG_METER_BAUD_38400         3         /* 电表波特率：38400 */
#define SCONFIG_METER_BAUD_115200        4         /* 电表波特率：115200 */

    u16 valid_len = 0;
    u32 ret = 0;
    thaisen_cfg_info_ammeter *config = (thaisen_cfg_info_ammeter*)data;

    /** 上电时已获取了电表信息 */

    switch(config->baudrate){
    case CP_AMMETER_BAUDRATE_2400:
        LcdData.setData.MeterBaudrate = SCONFIG_METER_BAUD_2400;
        break;
    case CP_AMMETER_BAUDRATE_4800:
        LcdData.setData.MeterBaudrate = SCONFIG_METER_BAUD_4800;
        break;
    case CP_AMMETER_BAUDRATE_9600:
        LcdData.setData.MeterBaudrate = SCONFIG_METER_BAUD_9600;
        break;
    case CP_AMMETER_BAUDRATE_38400:
        LcdData.setData.MeterBaudrate = SCONFIG_METER_BAUD_38400;
        break;
    case CP_AMMETER_BAUDRATE_115200:
        LcdData.setData.MeterBaudrate = SCONFIG_METER_BAUD_115200;
        break;
    default:
        return (SCONFIG_METER_BAUD_POSITION + THAISEN_CONFIG_FAIL_OFFSET);
    }

    if((valid_len = strlen((char*)config->ammeter_addr[LCD_GUN_1])) > 0){   /** 电表地址最小12字节(6字节的BCD) */
        if(valid_len >= 12){
            valid_len = valid_len > (sizeof(LcdData.setData.MeterAddr[LCD_GUN_1]) - 1) ? (sizeof(LcdData.setData.MeterAddr[LCD_GUN_1]) - 1) : valid_len;
            memset(LcdData.setData.MeterAddr[LCD_GUN_1], 0, sizeof(LcdData.setData.MeterAddr[LCD_GUN_1]));
            memcpy(LcdData.setData.MeterAddr[LCD_GUN_1], config->ammeter_addr[LCD_GUN_1], valid_len);
        }else{
            return (SCONFIG_METER_ADDR_A_POSITION + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    if((valid_len = strlen((char*)config->ammeter_addr[LCD_GUN_2])) > 0){   /** 电表地址最小12字节(6字节的BCD) */
        if(valid_len >= 12){
            valid_len = valid_len > (sizeof(LcdData.setData.MeterAddr[LCD_GUN_2]) - 1) ? (sizeof(LcdData.setData.MeterAddr[LCD_GUN_2]) - 1) : valid_len;
            memset(LcdData.setData.MeterAddr[LCD_GUN_2], 0, sizeof(LcdData.setData.MeterAddr[LCD_GUN_2]));
            memcpy(LcdData.setData.MeterAddr[LCD_GUN_2], config->ammeter_addr[LCD_GUN_2], valid_len);
        }else{
            return (SCONFIG_METER_ADDR_B_POSITION + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdData.setData.MeterModel = config->ammeter_model;
    LcdData.setData.MeterCheckWay = config->check_way;

    /** 电表信息有效性判断 */
    SerialScreen_BtnMeterNoInfoJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_BtnMeterNoInfoSet();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Module
 * 功能         外部触发执行模块信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Module(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
    u32 ret = 0;
    u16 valid_len = 0;
    thaisen_cfg_info_module *config = (thaisen_cfg_info_module*)data;

//    SerialScreen_BtnModuleGet();

    LcdData.setData.RmType = config->module_protocol;
    LcdData.setData.ModuleGroupNum = config->module_group;
    valid_len = sizeof(config->module_num_single);
    valid_len = valid_len > sizeof(LcdData.setData.ModuleNum) ? sizeof(LcdData.setData.ModuleNum) : valid_len;
    memset(LcdData.setData.ModuleNum, 0, valid_len);
    memcpy(LcdData.setData.ModuleNum, config->module_num_single, valid_len);

    LcdData.setData.Rated_Output_Voltage = config->module_rated_voltage;
    LcdData.setData.Rated_Limit_Current = config->module_rated_current;
    LcdData.setData.Max_Output_Voltage = config->pile_outvoltage_max;
    LcdData.setData.Min_Output_Voltage = config->pile_outvoltage_min;
    LcdData.setData.Max_Limit_Current = config->pile_outcurrent_max;
    LcdData.setData.Min_Limit_Current = config->pile_outcurrent_min;

    /** 模块信息有效性判断 */
    SerialScreen_BtnModuleInfoJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_BtnModuleSet();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Vin
 * 功能         外部触发执行VIN码信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Vin(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
    if(sub_data == NULL){
        return -0x01;
    }
    u8 valid_count = *(u8*)sub_data;
    thaisen_cfg_info_vin *config = (thaisen_cfg_info_vin*)data;

//    SerialScreen_BtnVinListGet();

    valid_count = valid_count > VIN_LIST_NUM ? VIN_LIST_NUM : valid_count;
    memset(LcdData.setData.s_vin_lists, 0, sizeof(LcdData.setData.s_vin_lists));
    for(u8 i = 0; i < valid_count; i++){
        memcpy(LcdData.setData.s_vin_lists[i], config->vin_whitelist[i], sizeof(LcdData.setData.s_vin_lists[i]));
    }
    SerialScreen_BtnVinListSet();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Protect
 * 功能         外部触发执行保护信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Protect(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
    u32 ret = 0;
    thaisen_cfg_info_protect *config = (thaisen_cfg_info_protect*)data;

    LcdData.setData.Stop_SOC = config->soc_stop;
    LcdData.setData.GunVolt_LimitValue = config->gunvolt_limit;

    LcdData.setData.PowerPercent = config->power_percent;
    LcdData.setData.ElossProprotion = config->eloss_proportion;

    LcdData.setData.OverTemp_Warnning = config->overtemp_alarm;
    LcdData.setData.OverTemp_Stop = config->overtemp_stop;
    LcdData.setData.OverTemp_Resume = config->overtemp_recovery;
    LcdData.setData.OverTemp_LimitCurr = config->overtemp_limitcur;

    /** 保护信息有效性判断 */
    SerialScreen_BtnProtectInfoJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_BtnProtectInfoSet();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Function
 * 功能         外部触发执行功能信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Function(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
    u32 ret = 0;
    thaisen_cfg_info_function *config = (thaisen_cfg_info_function*)data;

    LcdData.setData.sup_Local = config->local_start;
    LcdData.setData.Icon_SupPlugAndPlay = config->plug_charge;
    LcdData.setData.Icon_SuplocalStop = config->local_stop;
    LcdData.setData.sup_VIN = config->vin_charge;
    LcdData.setData.sup_insulation = config->insult_detect;
    LcdData.setData.sup_usecard = config->card_reader;
    LcdData.setData.sup_auxp_24V = config->auxpower_24V;
    LcdData.setData.sup_parallelchg = config->parallel_charge;
    LcdData.setData.sup_parallelrelay = config->parallel_relay;
    LcdData.setData.sup_mslience = config->module_silence;
    LcdData.setData.Icon_SupOfflineBilling = config->offline_billing;

    /** 功能配置信息有效性判断 */
    SerialScreen_IsSupportInfoJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_IsSupportSetFlash();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_OfflineBilling
 * 功能         外部触发执行离线计费信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_OfflineBilling(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    u32 ret = 0;
    thaisen_cfg_info_offline_billing *config = (thaisen_cfg_info_offline_billing*)data;

    LcdData.setData.ServicePrice = config->service_price;
    LcdData.setData.SsElectPrice = config->sharp_sharp_price;
    LcdData.setData.SElectPrice = config->sharp_price;
    LcdData.setData.PElectPrice = config->peak_price;
    LcdData.setData.FElectPrice = config->flat_price;
    LcdData.setData.VElectPrice = config->valley_price;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].shour = config->sstime1.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].smin = config->sstime1.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].ehour = config->sstime1.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].emin = config->sstime1.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].rate_number = config->sstime1.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].shour = config->sstime2.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].smin = config->sstime2.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].ehour = config->sstime2.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].emin = config->sstime2.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].rate_number = config->sstime2.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].shour = config->stime1.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].smin = config->stime1.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].ehour = config->stime1.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].emin = config->stime1.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].rate_number = config->stime1.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].shour = config->stime2.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].smin = config->stime2.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].ehour = config->stime2.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].emin = config->stime2.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].rate_number = config->stime2.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].shour = config->ptime1.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].smin = config->ptime1.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].ehour = config->ptime1.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].emin = config->ptime1.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].rate_number = config->ptime1.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].shour = config->ptime2.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].smin = config->ptime2.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].ehour = config->ptime2.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].emin = config->ptime2.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].rate_number = config->ptime2.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].shour = config->ftime1.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].smin = config->ftime1.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].ehour = config->ftime1.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].emin = config->ftime1.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].rate_number = config->ftime1.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].shour = config->ftime2.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].smin = config->ftime2.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].ehour = config->ftime2.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].emin = config->ftime2.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].rate_number = config->ftime2.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].shour = config->vtime1.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].smin = config->vtime1.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].ehour = config->vtime1.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].emin = config->vtime1.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].rate_number = config->vtime1.rated_number;

    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].shour = config->vtime2.start_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].smin = config->vtime2.start_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].ehour = config->vtime2.end_hour;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].emin = config->vtime2.end_min;
    LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].rate_number = config->vtime2.rated_number;

    /** 离线计费信息有效性判断 */
    SerialScreen_OfflineBillingInfoJudge(&ret);
    if(ret != 0){
        for(u8 i = 0; i < 32; i++){
            if(ret &(1 <<i))   return (i + THAISEN_CONFIG_FAIL_OFFSET);
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_OfflineBillingSet();

    return LcdAssistantData.Flag.IsConfigFail;
#else
    return TRUE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_Input_7103_7101
 * 功能         外部触发执行 7103/7101输入信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_Input_7103_7101(u8 port, void *data, void *sub_data, void *sub_sub_data)  //OK
{
    thaisen_cfg_info_input_7103_7101 *config = (thaisen_cfg_info_input_7103_7101*)data;

    LcdData.setData.supin_scram = FALSE;
    if(config->scram.enable){
        LcdData.setData.supin_scram = TRUE;
    }
    LcdData.setData.neg_scram = FALSE;
    if(config->scram.reversal){
        LcdData.setData.neg_scram = TRUE;
    }

    LcdData.setData.supin_gate = FALSE;
    if(config->door.enable){
        LcdData.setData.supin_gate = TRUE;
    }
    LcdData.setData.neg_gate = FALSE;
    if(config->door.reversal){
        LcdData.setData.neg_gate = TRUE;
    }

    LcdData.setData.supin_ac = FALSE;
    if(config->acrelay.enable){
        LcdData.setData.supin_ac = TRUE;
    }
    LcdData.setData.neg_ac = FALSE;
    if(config->acrelay.reversal){
        LcdData.setData.neg_ac = TRUE;
    }

    LcdData.setData.supin_dc = FALSE;
    if(config->dcrelay.enable){
        LcdData.setData.supin_dc = TRUE;
    }
    LcdData.setData.neg_dc = FALSE;
    if(config->dcrelay.reversal){
        LcdData.setData.neg_dc = TRUE;
    }

    LcdData.setData.supin_fan = FALSE;
    if(config->fan.enable){
        LcdData.setData.supin_fan = TRUE;
    }
    LcdData.setData.neg_fan = FALSE;
    if(config->fan.reversal){
        LcdData.setData.neg_fan = TRUE;
    }

    LcdData.setData.supin_elock = FALSE;
    if(config->elock.enable){
        LcdData.setData.supin_elock = TRUE;
    }
    LcdData.setData.neg_elcok = FALSE;
    if(config->elock.reversal){
        LcdData.setData.neg_elcok = TRUE;
    }

    LcdData.setData.supin_temp_pro = FALSE;
    if(config->tempprotect.enable){
        LcdData.setData.supin_temp_pro = TRUE;
    }

    SerialScreen_InputSetFlash();

    return LcdAssistantData.Flag.IsConfigFail;
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_PublicInput_7104
 * 功能         外部触发执行 7104通用输入信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_PublicInput_7104(u8 port, void *data, void *sub_data, void *sub_sub_data)
{
#define SSCREEN_7104_INPORT_DUPLICATE     0         /* 7104 通用输入端口重复错误码*/
/******************************* 这是7104的配置 **********************************/
#if 0
    thaisen_cfg_info_public_input_7104 *config = (thaisen_cfg_info_public_input_7104*)data;

    LcdData.setData.input_protectlight = config->protectlight.port_number;
    LcdData.setData.input_parallel[PARA_0] = config->parallel_relay1.port_number;
    LcdData.setData.input_parallel[PARA_1] = config->parallel_relay2.port_number;
    LcdData.setData.input_parallel[PARA_2] = config->parallel_relay3.port_number;
    LcdData.setData.input_scram = config->scram.port_number;
    LcdData.setData.input_breakers = config->breaker.port_number;
    LcdData.setData.input_ac = config->acrelay.port_number;
    LcdData.setData.input_fan = config->fan.port_number;
    LcdData.setData.input_water = config->flooding.port_number;
    LcdData.setData.input_gate = config->door.port_number;
    LcdData.setData.input_smoke = config->smoke.port_number;
    LcdData.setData.input_fall = config->fall.port_number;

    LcdData.setData.supin_protectlight = FALSE;
    if(config->protectlight.state.enable)    LcdData.setData.supin_protectlight = TRUE;
    LcdData.setData.neg_protectlight = FALSE;
    if(config->protectlight.state.reversal)    LcdData.setData.neg_protectlight = TRUE;

    LcdData.setData.supin_Parallel[PARA_0] = FALSE;
    if(config->parallel_relay1.state.enable)    LcdData.setData.supin_Parallel[PARA_0] = TRUE;
    LcdData.setData.neg_Parallel[PARA_0] = FALSE;
    if(config->parallel_relay1.state.reversal)    LcdData.setData.neg_Parallel[PARA_0] = TRUE;

    LcdData.setData.supin_Parallel[PARA_1] = FALSE;
    if(config->parallel_relay2.state.enable)    LcdData.setData.supin_Parallel[PARA_1] = TRUE;
    LcdData.setData.neg_Parallel[PARA_1] = FALSE;
    if(config->parallel_relay2.state.reversal)    LcdData.setData.neg_Parallel[PARA_1] = TRUE;

    LcdData.setData.supin_Parallel[PARA_2] = FALSE;
    if(config->parallel_relay3.state.enable)    LcdData.setData.supin_Parallel[PARA_2] = TRUE;
    LcdData.setData.neg_Parallel[PARA_2] = FALSE;
    if(config->parallel_relay3.state.reversal)    LcdData.setData.neg_Parallel[PARA_2] = TRUE;

    LcdData.setData.supin_scram = FALSE;
    if(config->scram.state.enable)    LcdData.setData.supin_scram = TRUE;
    LcdData.setData.neg_scram = FALSE;
    if(config->scram.state.reversal)    LcdData.setData.neg_scram = TRUE;

    LcdData.setData.supin_breakers = FALSE;
    if(config->breaker.state.enable)    LcdData.setData.supin_breakers = TRUE;
    LcdData.setData.neg_breakers = FALSE;
    if(config->breaker.state.reversal)    LcdData.setData.neg_breakers = TRUE;

    LcdData.setData.supin_ac = FALSE;
    if(config->acrelay.state.enable)    LcdData.setData.supin_ac = TRUE;
    LcdData.setData.neg_ac = FALSE;
    if(config->acrelay.state.reversal)    LcdData.setData.neg_ac = TRUE;

    LcdData.setData.supin_fan = FALSE;
    if(config->fan.state.enable)    LcdData.setData.supin_fan = TRUE;
    LcdData.setData.neg_fan = FALSE;
    if(config->fan.state.reversal)    LcdData.setData.neg_fan = TRUE;

    LcdData.setData.supin_water = FALSE;
    if(config->flooding.state.enable)    LcdData.setData.supin_water = TRUE;
    LcdData.setData.neg_water = FALSE;
    if(config->flooding.state.reversal)    LcdData.setData.neg_water = TRUE;

    LcdData.setData.supin_gate = FALSE;
    if(config->door.state.enable)    LcdData.setData.supin_gate = TRUE;
    LcdData.setData.neg_gate = FALSE;
    if(config->door.state.reversal)    LcdData.setData.neg_gate = TRUE;

    LcdData.setData.supin_smoke = FALSE;
    if(config->smoke.state.enable)    LcdData.setData.supin_smoke = TRUE;
    LcdData.setData.neg_smoke = FALSE;
    if(config->smoke.state.reversal)    LcdData.setData.neg_smoke = TRUE;

    LcdData.setData.supin_fall = FALSE;
    if(config->fall.state.enable)    LcdData.setData.supin_fall = TRUE;
    LcdData.setData.neg_fall = FALSE;
    if(config->fall.state.reversal)    LcdData.setData.neg_fall = TRUE;

    /** 输入端口信息有效性判断 */
    if(SerialScreen_CheckInputPort_Valid(0) == 0){
        return (SSCREEN_7104_INPORT_DUPLICATE + THAISEN_CONFIG_FAIL_OFFSET);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_InputSetFlash();

    return LcdAssistantData.Flag.IsConfigFail;
#else
    return (SSCREEN_7104_INPORT_DUPLICATE + THAISEN_CONFIG_FAIL_OFFSET);
#endif
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_PublicInput_7104
 * 功能         外部触发执行 7104枪输入信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_GunInput_7104(u8 port, void *data, void *sub_data, void *sub_sub_data)
{
#define SSCREEN_7104_GUN_INPORT_DUPLICATE     0         /* 7104 枪输入端口重复错误码*/
/******************************* 这是7104的配置 **********************************/
#if 0
    thaisen_cfg_info_gun_input_7104 *config = (thaisen_cfg_info_gun_input_7104*)data;

    LcdData.setData.input_dc[port] = config->dcrelay.port_number;
    LcdData.setData.input_elock[port] = config->elock.port_number;
    LcdData.setData.input_gunsite[port] = config->gunsite.port_number;
    LcdData.setData.input_fuse[port] = config->fuse.port_number;
    LcdData.setData.input_liquid[port] = config->liquid.port_number;

    LcdData.setData.supin_dc[port] = FALSE;
    if(config->dcrelay.state.enable)    LcdData.setData.supin_dc[port] = TRUE;
    LcdData.setData.neg_dc[port] = FALSE;
    if(config->dcrelay.state.reversal)    LcdData.setData.neg_dc[port] = TRUE;

    LcdData.setData.supin_elock[port] = FALSE;
    if(config->elock.state.enable)    LcdData.setData.supin_elock[port] = TRUE;
    LcdData.setData.neg_elcok[port] = FALSE;
    if(config->elock.state.reversal)    LcdData.setData.neg_elcok[port] = TRUE;

    LcdData.setData.supin_gunsite[port] = FALSE;
    if(config->gunsite.state.enable)    LcdData.setData.supin_gunsite[port] = TRUE;
    LcdData.setData.neg_gunsite[port] = FALSE;
    if(config->gunsite.state.reversal)    LcdData.setData.neg_gunsite[port] = TRUE;

    LcdData.setData.supin_Fuse[port] = FALSE;
    if(config->fuse.state.enable)    LcdData.setData.supin_Fuse[port] = TRUE;
    LcdData.setData.neg_Fuse[port] = FALSE;
    if(config->fuse.state.reversal)    LcdData.setData.neg_Fuse[port] = TRUE;

    LcdData.setData.supin_Liquid[port] = FALSE;
    if(config->liquid.state.enable)    LcdData.setData.supin_Liquid[port] = TRUE;
    LcdData.setData.neg_Liquid[port] = FALSE;
    if(config->liquid.state.reversal)    LcdData.setData.neg_Liquid[port] = TRUE;

    LcdData.setData.supin_temp_pro[port] = FALSE;
    if(config->temp_detect.state.enable)    LcdData.setData.supin_temp_pro[port] = TRUE;

    return 0;
#else
    return (SSCREEN_7104_GUN_INPORT_DUPLICATE + THAISEN_CONFIG_FAIL_OFFSET);
#endif
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_PublicInput_7104
 * 功能         外部触发执行 7104通用输出信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_PublicOutput_7104(u8 port, void *data, void *sub_data, void *sub_sub_data)
{
#define SSCREEN_7104_OUTPORT_DUPLICATE     0         /* 7104 通用输出端口重复错误码*/
/******************************* 这是7104的配置 **********************************/
#if 0
    thaisen_cfg_info_public_output_7104 *config = (thaisen_cfg_info_public_output_7104*)data;

    LcdData.setData.output_fan = config->fan.port_number;
    LcdData.setData.output_para[PARA_0] = config->parallel_relay1.port_number;
    LcdData.setData.output_para[PARA_1] = config->parallel_relay2.port_number;
    LcdData.setData.output_para[PARA_2] = config->parallel_relay3.port_number;
    LcdData.setData.output_ac = config->acrelay.port_number;

    LcdData.setData.supin_ofan = FALSE;
    if(config->fan.enable)    LcdData.setData.supin_ofan = TRUE;

    LcdData.setData.supin_opara[PARA_0] = FALSE;
    if(config->parallel_relay1.enable)    LcdData.setData.supin_opara[PARA_0] = TRUE;

    LcdData.setData.supin_opara[PARA_1] = FALSE;
    if(config->parallel_relay2.enable)    LcdData.setData.supin_opara[PARA_1] = TRUE;

    LcdData.setData.supin_opara[PARA_2] = FALSE;
    if(config->parallel_relay3.enable)    LcdData.setData.supin_opara[PARA_2] = TRUE;

    LcdData.setData.supin_oac = FALSE;
    if(config->acrelay.enable)    LcdData.setData.supin_oac = TRUE;

    /** 输出端口信息有效性判断 */
    if(SerialScreen_CheckOutputPort_Valid(0) == 0){
        return (SSCREEN_7104_OUTPORT_DUPLICATE + THAISEN_CONFIG_FAIL_OFFSET);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = TRUE;
    SerialScreen_OutputSetFlash();

    return LcdAssistantData.Flag.IsConfigFail;
#else
    return (SSCREEN_7104_OUTPORT_DUPLICATE + THAISEN_CONFIG_FAIL_OFFSET);
#endif
}

/*******************************************************************
 * 函数名      SerialScreen_ConfigExecute_GunOutput_7104
 * 功能         外部触发执行 7104枪输出信息配置
 * 参数          data         配置数据
 *        port         枪口号
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 *******************************************************************/
static s32 SerialScreen_ConfigExecute_GunOutput_7104(u8 port, void *data, void *sub_data, void *sub_sub_data)
{
#define SSCREEN_7104_GUN_OUTPORT_DUPLICATE     0         /* 7104 枪输出端口重复错误码*/
/******************************* 这是7104的配置 **********************************/
#if 0
    thaisen_cfg_info_gun_output_7104 *config = (thaisen_cfg_info_gun_output_7104*)data;

    LcdData.setData.output_aux12v[port] = config->auxpower_12V.port_number;
    LcdData.setData.output_aux24v[port] = config->auxpower_24V.port_number;
    LcdData.setData.output_dc[port] = config->dcrelay.port_number;
    LcdData.setData.output_elock[port] = config->elock.port_number;
    LcdData.setData.output_relief[port] = config->relief.port_number;
    LcdData.setData.output_liquid[port] = config->liquid.port_number;

    LcdData.setData.supin_AUX12V[port] = FALSE;
    if(config->auxpower_12V.enable)    LcdData.setData.supin_AUX12V[port] = TRUE;

    LcdData.setData.supin_AUX24V[port] = FALSE;
    if(config->auxpower_24V.enable)    LcdData.setData.supin_AUX24V[port] = TRUE;

    LcdData.setData.supin_odc[port] = FALSE;
    if(config->dcrelay.enable)    LcdData.setData.supin_odc[port] = TRUE;

    LcdData.setData.supin_oelock[port] = FALSE;
    if(config->elock.enable)    LcdData.setData.supin_oelock[port] = TRUE;

    LcdData.setData.supin_relief[port] = FALSE;
    if(config->relief.enable)    LcdData.setData.supin_relief[port] = TRUE;

    LcdData.setData.supin_oliquid[port] = FALSE;
    if(config->liquid.enable)    LcdData.setData.supin_oliquid[port] = TRUE;

    return 0;
#else
    return (SSCREEN_7104_GUN_OUTPORT_DUPLICATE + THAISEN_CONFIG_FAIL_OFFSET);
#endif
}

/*******************************************************************************************
 * 函数名      SerialScreen_Trigger_ConfigExecute
 * 功能         外部触发执行信息配置
 * 参数          cfg_page     配置所在页
 *        data         配置数据
 * 返回          <0:配置失败(参数无效)，=0:配置成功  1:配置存储失败， >1:写入配置成功，但是写入的内容与输入的内容有差异
 ******************************************************************************************/
s32 SerialScreen_Trigger_ConfigExecute(u8 port, u8 cfg_page, void *main_data, void *sub_data, void *sub_sub_data)
{
#define SERIAL_SCREEN_ASSERT_BASE       -0x10      /* 配置信息，系统断言失败(函数调用)偏移，此函数断言失败已使用了-1，-2 */

    if((cfg_page >= THAISEN_CONFIG_PAGE_SIZE) || (main_data == NULL) || (port >= LCD_GUN_NUM))
        return -1;
    if(LcdConfigExecutPool[cfg_page] == NULL)
        return -2;

    s32 ret = LcdConfigExecutPool[cfg_page](port, main_data, sub_data, sub_sub_data);

    if(ret < 0){
        return (ret + SERIAL_SCREEN_ASSERT_BASE);   /** 这样操作是为了更好地反应出内部函数调用的断言失败情况 */
    }

    return ret;
}

/*******************************************************************************************
 * 函数名      SerialScreen_ScreenSet_Trigger_Event
 * 功能         设置屏幕外部触发事件
 * 参数          Event            事件
 *      DurationTime    持续时长, 单位s(为0时作用是为了让当前提示页面消除)
 *      JustNotice      是否仅用于提醒(1：是，0：否)
 *      port            端口号号
 * 返回           >0：成功，<=0：失败
 ******************************************************************************************/
s32 SerialScreen_ScreenSet_Trigger_Event(u8 Event, u16 DurationTime, u8 JustNotice, u8 port)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling == FALSE){
        LcdTriggerEvent[port].Flag.IsModify = FALSE;
        LcdTriggerEvent[port].Flag.IsTriggerExternal = FALSE;
        return FALSE;
    }
    if(Event > SCREEN_TRIGGER_PAGE_PAY_COMPLETE){
        Event += 0x01;
    }
    if(Event >= SERIALSCREEN_TRIGGER_PAGE_MAX){
        return FALSE;
    }
    if(port >= LCD_GUN_NUM){
        return FALSE;
    }

    LcdTriggerEvent[port].Flag.IsModify = TRUE;

    LcdTriggerEvent[port].item.Flag.JustNotice = FALSE;
    if(JustNotice){
        LcdTriggerEvent[port].item.Flag.JustNotice = TRUE;
    }
    if(Event == SCREEN_TRIGGER_PAGE_PAY_COMPLETE){
        LcdTriggerEvent[port].Flag.IsWaitPay = FALSE;
        LcdTriggerEvent[port].Flag.IsPayed = TRUE;
    }

    if(Event == SCREEN_TRIGGER_WARN_ICON_PAY){
        LcdTriggerEvent[port].Flag.IsWaitPay = TRUE;
    }

    LcdTriggerEvent[port].item.page = Trigger_Page[Event].page;
    if(Event == SCREEN_TRIGGER_PAGE_PAY_COMPLETE){
        LcdTriggerEvent[port].item.page = Trigger_Page[Event + port].page;
    }
    LcdTriggerEvent[port].item.ShowPara = Trigger_Page[Event].ShowPara;
    LcdTriggerEvent[port].item.ShieldPara = Trigger_Page[Event].ShieldPara;
    LcdTriggerEvent[port].item.time = DurationTime;

    LcdData.gunIndex = port;
    LcdTriggerEvent[port].Flag.IsTriggerExternal = TRUE;

    return TRUE;
#else
    return FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
}

/*****************************************************************************************************
 * 函数名            SerialScreen_TriggerItem_Execute
 * 功能                执行触发事件
 * 参数                cmd   屏幕总句柄
 *        item  触发事件句柄
 * 返回               1：执行完成      0：执行中
 ****************************************************************************************************/
static u8 SerialScreen_TriggerItem_Execute(struct LCD_TRIGGER_ITEM *item)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling == FALSE){
        LcdTriggerEvent[LcdData.gunIndex].Flag.IsModify = FALSE;
        LcdTriggerEvent[LcdData.gunIndex].Flag.IsTriggerExternal = FALSE;
        return TRUE;
    }

    if(LcdData.gunIndex >= LCD_GUN_NUM){
        for(u8 i = 0; i < LCD_GUN_NUM; i++){
            LcdTriggerEvent[i].Flag.IsModify = FALSE;
        }
        return TRUE;             /* 变量信息错误, 结束触发事件 */
    }

    if(LcdTriggerEvent[LcdData.gunIndex].Flag.IsModify){
        LcdTriggerEvent[LcdData.gunIndex].Flag.IsModify = FALSE;

        if((LcdTriggerEvent[LcdData.gunIndex].LastPage != LcdData.CurrentPage) &&
                (LcdData.CurrentPage != LcdTriggerEvent[LcdData.gunIndex].item.page)){
            LcdTriggerEvent[LcdData.gunIndex].LastPage = LcdData.CurrentPage;
        }
        LcdTriggerEvent[LcdData.gunIndex].TimeBaseTick = 0x00;
        LcdTriggerEvent[LcdData.gunIndex].CountDown = LcdTriggerEvent[LcdData.gunIndex].item.time;             /* 提示显示倒计时 */
        LcdTriggerEvent[LcdData.gunIndex].Warnning = LcdTriggerEvent[LcdData.gunIndex].item.ShowPara;          /* 提示显示参数 */

        LcdData.CurrentPage = LcdTriggerEvent[LcdData.gunIndex].item.page;                   /* 强制跳页 */
        for(u8 i = 0; i < LCD_GUN_NUM; i++){
            LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
        }
    }

    LcdData.setData.OBEventwarning = LcdTriggerEvent[LcdData.gunIndex].Warnning;
    LcdData.setData.OBCountDown = LcdTriggerEvent[LcdData.gunIndex].CountDown;
    if(LcdData.setData.OBCountDown > 999){                    /* 目前OB_CountDownString长度是5字节，要预留一字节尾和一字节秒(S)，剩余最大只能是3位数 */
        sprintf((char*)LcdData.setData.OB_CountDownString, "%uS", 999);
    }else{
        sprintf((char*)LcdData.setData.OB_CountDownString, "%uS", LcdData.setData.OBCountDown);
    }

    if(LcdTriggerEvent[LcdData.gunIndex].CountDown > 0x00){
        if(++LcdTriggerEvent[LcdData.gunIndex].TimeBaseTick > 1000/ BASE_SYS_TIMER){
            LcdTriggerEvent[LcdData.gunIndex].TimeBaseTick = 0;
            LcdTriggerEvent[LcdData.gunIndex].CountDown--;
        }
    }else{
        if(LcdTriggerEvent[LcdData.gunIndex].item.Flag.JustNotice){
            LcdTriggerEvent[LcdData.gunIndex].Warnning = LcdTriggerEvent[LcdData.gunIndex].item.ShieldPara;     /* 如果是icon提示, 提示完后, 跳页前先发多次隐藏icon的指令 */
            if(++LcdTriggerEvent[LcdData.gunIndex].TimeBaseTick > 500/ BASE_SYS_TIMER){
                LcdTriggerEvent[LcdData.gunIndex].TimeBaseTick = 0;

                LcdTriggerEvent[LcdData.gunIndex].Flag.IsModify = FALSE;
                return TRUE;
            }
        }else{
            LcdTriggerEvent[LcdData.gunIndex].Flag.IsModify = FALSE;
            return TRUE;
        }
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */
    return FALSE;
}

/***********************************************************
 * 函数名     SerialScreen_TriggerItem_Repeat_WaitPay
 * 功能         未结算前，回到结算页面时要展示请算卡结算页面
 * 参数         port  端口号
 * 返回
 **********************************************************/
static void SerialScreen_TriggerItem_Repeat_WaitPay(u8 port)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling == FALSE){
        return;
    }
    LcdTriggerEvent[port].TimeBaseTick = 0x00;
    LcdTriggerEvent[port].CountDown = 30;             /* 提示显示倒计时 */
    LcdTriggerEvent[port].Warnning = SCREEN_TRIGGER_WARN_ICON_PAY;          /* 提示显示参数 */
    LcdTriggerEvent[LcdData.gunIndex].item.ShowPara = SCREEN_TRIGGER_WARN_ICON_PAY;
    LcdTriggerEvent[LcdData.gunIndex].item.ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;

    LcdData.CurrentPage = LCD_PAGE_WARNNING_INFO;                   /* 强制跳页 */
    for(u8 i = 0; i < LCD_GUN_NUM; i++){
        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */
}
/***********************************************************
 * 函数名     SerialScreen_TriggerItem_Page_Warnning
 * 功能        当前执行完成的触发项属于告警页面，根据当前信息决定跳转到那一页
 * 参数
 * 返回        1：所有触发项已执行完毕  0：还有触发项需要执行
 **********************************************************/
static u8 SerialScreen_TriggerItem_Page_Warnning(void)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling == FALSE){
        return TRUE;
    }
    /** 当前触发项是充电结束请刷卡结算，此项已展示完，跳到首页 */
    if(LcdTriggerEvent[LcdData.gunIndex].item.ShowPara == SCREEN_TRIGGER_WARN_ICON_PAY){
        LcdData.Homeflg = 1;
        LcdData.CurrentPage = LCD_PAGE_STANDBY;
    }
    /** 上一页属于结算页，这是在结算完成后触发提示的告警，告警展示完后跳到充电结束未拔枪页面 */
    else if(LcdTriggerEvent[LcdData.gunIndex].LastPage == (LCD_PAGE_OB_PYA_A + LcdData.gunIndex)){
        LcdData.CurrentPage = LCD_PAGE_A_ACOUNT + LcdData.gunIndex;
    }
    /** 告警提示完成，返回上一页 */
    else{
        LcdData.CurrentPage = LcdTriggerEvent[LcdData.gunIndex].LastPage;
    }

    for(u8 i = 0; i < LCD_GUN_NUM; i++){
        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */
    return TRUE;
}
/***********************************************************
 * 函数名     SerialScreen_TriggerItem_Page_Payed
 * 功能         当前执行完成的触发项属于结算完成页面，此页面完成后跳到充电结束未拔枪页面
 * 参数
 * 返回        1
 **********************************************************/
static u8 SerialScreen_TriggerItem_Page_Payed(void)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling == FALSE){
        return TRUE;
    }
    LcdData.CurrentPage = LCD_PAGE_A_ACOUNT + LcdData.gunIndex;
    for(u8 i = 0; i < LCD_GUN_NUM; i++){
        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */
    return TRUE;
}
/***********************************************************
 * 函数名     SerialScreen_TriggerItem_Complete
 * 功能         当前触发项已执行完成，检查是否还有触发项需要执行
 * 参数
 * 返回        1：所有触发项已执行完毕  0：还有触发项需要执行
 **********************************************************/
static u8 SerialScreen_TriggerItem_Complete(void)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.CurrentPage == LCD_PAGE_WARNNING_INFO){
        return SerialScreen_TriggerItem_Page_Warnning();
    }else{
        return SerialScreen_TriggerItem_Page_Payed();
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */
    return TRUE;
}

/***********************************************************
 * 函数名     SerialScreen_TriggerEvent_Process
 * 功能         外部跳页触发事件处理
 * 参数
 * 返回
 **********************************************************/
static void SerialScreen_TriggerEvent_Process(void)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling == FALSE){
        LcdTriggerEvent[LcdData.gunIndex].Flag.IsTriggerExternal = FALSE;
        return;
    }

    if(LcdTriggerEvent[LcdData.gunIndex].Flag.IsTriggerExternal == TRUE){
        if(SerialScreen_TriggerItem_Execute(&LcdTriggerEvent[LcdData.gunIndex].item)){
            if(SerialScreen_TriggerItem_Complete()){
                LcdTriggerEvent[LcdData.gunIndex].Flag.IsTriggerExternal = FALSE;
            }
        }
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */
}

static void SerialScreen_RealTime_InfoGet(void)
{
    u8 buf[4];

    if(++LcdAssistantData.RefrenshPeriod > 1000 /100){   //线程运行时基10ms， 每1s更新一次数据
        u8 valid_len = sizeof(LcdData.setData.Help_Number), *data = NULL;

        LcdData.setData.SIM_Strength = thaisen_app_get_signal_strength();
        memcpy(LcdData.setData.SIM_card, thaisen_app_get_sim_number(), 21);
#ifdef SCREEN_USING_DUPU
        LcdData.setData.StoredEnergy_Soc = terminal_get_ems_soc();
        LcdData.setData.StoredEnergy_Power = (*(u32*)UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_SYSTEM_POWER_TOTAL, 0));
        if(LcdData.setData.StoredEnergy_Power > (terminal_get_ems_set_power() *100)){
            LcdData.setData.StoredEnergy_Power = (terminal_get_ems_set_power() *100);
        }
#endif /* SCREEN_USING_DUPU */
        if(LcdData.CurrentPage != LCD_PAGE_MENU_COM_1){
            data = UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_HELP_PHONE, 0);

            valid_len = valid_len > strlen((char*)data) ? strlen((char*)data) : valid_len;
            str_ncpy((char *)(LcdData.setData.Help_Number), data, valid_len);
        }

        for(int i = 0; i< LCD_GUN_NUM; i++)
        {
            thaisen_get_device_sn((char*)LcdData.runData.chgcode[i], sizeof(LcdData.runData.chgcode[i]), i);
        }

        if(LcdData.CurrentPage != LCD_PAGE_ADMIN_PASWD){
            data = UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_SCREEN_PASSWORD, 0);
            valid_len = strlen((char*)data);
            valid_len = valid_len > sizeof(LcdData.setData.UserPasswd) ? sizeof(LcdData.setData.UserPasswd) : valid_len;

            if(memcmp(data, LcdData.setData.UserPasswd, valid_len)){
                u8 i = 0;
                for(i = 0; i < valid_len; i++){
                    if((data[i] < 0x20) || (data[i] > 0x7E)){
                        break;
                    }
                }
                if(i == valid_len){
                    memcpy(LcdData.setData.UserPasswd, data, valid_len);
                }
            }
        }
    }

    LcdData.setData.ota_progress = s_ota_info->progress;

    if(thaisen_get_current_period_time_hm(buf, sizeof(buf)) >= 0){
        LcdData.setData.period_time[0] = buf[0];
        LcdData.setData.period_time[1] = buf[1];
        LcdData.setData.period_time[2] = buf[2];
        LcdData.setData.period_time[3] = buf[3];
    }

    for(u8 gunno = 0; gunno < LCD_GUN_NUM; gunno++){
        LcdData.setData.g_cc1Vol[gunno] = thaisen_get_cc1_voltage(gunno) *10;
        LcdData.setData.g_portTemp[gunno] = thaisen_get_gun_temp(gunno);
        LcdData.setData.g_meterVol[gunno] = thaisen_get_ammeter_voltage(gunno);
        LcdData.setData.g_meterCur[gunno] = thaisen_get_ammeter_current(gunno);
        if(thaisenGetModuleOutputVoltage(gunno) < 100){
            LcdData.setData.g_chargeVol[gunno] = 0;
        }else{
            LcdData.setData.g_chargeVol[gunno] = thaisenGetModuleOutputVoltage(gunno);
        }
        LcdData.setData.g_chargeCur[gunno] = thaisen_get_ammeter_current(gunno);
#if 0
        if(thaisen_get_InsultInfo(gunno) == thaisenInsultAnomaly){
            LcdData.setData.warnning[gunno] = SYSTEM_WARNNING_INFO_INSULT_PROPERTIES;
        }else if(thaisen_get_InsultVoltInfo(gunno) == thaisenInsultVoltAlarm){
            LcdData.setData.warnning[gunno] = SYSTEM_WARNNING_INFO_INSULT_VOLTAGE;
        }else if(thaisen_get_BMSCurrentInfo(gunno) == thaisenBMSCurrentAlarm){
            LcdData.setData.warnning[gunno] = SYSTEM_WARNNING_INFO_REQUEST_CURRENT;
        }else{
            LcdData.setData.warnning[gunno] = SYSTEM_WARNNING_INFO_NORMAL;
        }
#else
        LcdData.setData.warnning[gunno] = SYSTEM_WARNNING_INFO_NORMAL;
#endif
        LcdData.setData.period_price = thaisen_get_period_price(gunno, 0x00);

        LcdData.setData.chargeState[gunno] = thaisen_get_charge_state(gunno);
        LcdData.setData.batteryVolt[gunno] = thaisen_get_bcp_voltage(gunno) *10;
        LcdData.setData.maxChargeVolt[gunno] = thaisen_get_bhm_voltage(gunno) *10;
        LcdData.setData.moduleVolt[gunno] = thaisen_get_module_voltage(gunno) *10;
        LcdData.setData.samplingVolt[gunno] = thaisen_get_insult_voltage(gunno) *10;
        if(LcdData.setData.samplingVolt[gunno] > (LcdData.setData.moduleVolt[gunno] + 500)){   //因采样误差，暂时做限制处理(5V)
            LcdData.setData.samplingVolt[gunno] = (LcdData.setData.moduleVolt[gunno] + 500);
        }
    }
}

u8 SerialScreen_GetChargeWay(void)
{
    if(LcdAssistantData.Flag.ParaChargeSelect)
        return APP_CHARGE_WAY_PARACHARGE_LOCAL;
    else
        return APP_CHARGE_WAY_SINGLEGUN;
}

void SerialScreen_SetChargeWay(u8 way)
{
    if(way <= APP_CHARGE_WAY_NONE){
        thaisen_set_charg_mode(thaisenSingleChargeMode);
        thaisenModuleSetChargeWay(thaisenModuleChargeWay_singleGun);
        LcdAssistantData.Flag.ParaChargeSelect = FALSE;
    }else if(way == APP_CHARGE_WAY_SINGLEGUN){
        thaisen_set_charg_mode(thaisenSingleChargeMode);
        thaisenModuleSetChargeWay(thaisenModuleChargeWay_singleGun);
        LcdAssistantData.Flag.ParaChargeSelect = FALSE;
    }else if((way == APP_CHARGE_WAY_PARACHARGE_LOCAL) || (way == APP_CHARGE_WAY_PARACHARGE_CLOUD)){
        thaisen_set_charg_mode(thaisenParallelCharging);
        thaisenModuleSetChargeWay(thaisenModuleChargeWay_parallelCharge);
        LcdAssistantData.Flag.ParaChargeSelect = TRUE;
    }else{
        thaisen_set_charg_mode(thaisenSingleChargeMode);
        thaisenModuleSetChargeWay(thaisenModuleChargeWay_singleGun);
        LcdAssistantData.Flag.ParaChargeSelect = FALSE;
    }
}

static void SerialScreen_TimeingRefrensh(void)
{
    static uint8_t _time = 0;
    if(++_time > 100){
        _time = 0;

        if(LcdData.CurrentPage == LCD_PAGE_MENU_STATE_MODULE){
            SerialScreen_BtnModuleStateA();
        }else if(LcdData.CurrentPage == LCD_PAGE_MENU_STATE_MODULE_B){
            SerialScreen_BtnModuleStateB();
        }
    }
}

void SerialScreen_BtnReturn1(int port)
{
    sSCREEN_EVENT_DEBUGMSG("##########Returnt1###########\r\n");
	#if 0
	tcuMain_Obj.btApi[LCD_GUN_1].Get_Lcd_ReadCard_ButtonBack = 1;
	tcuMain_Obj.btApi[LCD_GUN_1].Get_Lcd_GetErWei_ButtonBack = 1;
	tcuMain_Obj.btApi[LCD_GUN_1].Get_Lcd_OfflineCode_ButtonBack = 1;

	tcuMain_Obj.btApi[LCD_GUN_1].Get_Lcd_Card_ButtonClick = 0;
	#endif
	SerialScreen_DataClean(port);
}

void SerialScreen_StartCharge(int port)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[%d] Start Charge###########\r\n",port);
	if(TRUE == LcdData.setData.sup_Local)
    	thaisen_app_set_screen_start_charge(port);
	else ;
}

void SerialScreen_StartChargeA(void)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[LCD_GUN_1] Start Charge###########\r\n");
	if(TRUE == LcdData.setData.sup_Local)
    	thaisen_app_set_screen_start_charge(LCD_GUN_1);
	else ;
}


void SerialScreen_StartChargeB(void)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[LCD_GUN_2] Start Charge###########\r\n");
	if(TRUE == LcdData.setData.sup_Local)
    	thaisen_app_set_screen_start_charge(LCD_GUN_2);
	else ;
}


void SerialScreen_VinStartCharge(int port)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[%d] Vin Start Charge###########\r\n",port);
	if(TRUE == LcdData.setData.sup_VIN)
    	thaisen_app_set_vin_start_charge(port);
	else ;
}

void SerialScreen_VinStartChargeA(void)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[LCD_GUN_1] Vin Start Charge###########\r\n");
    if(TRUE == LcdData.setData.sup_VIN){
         LcdAssistantData.SeveralGunFlag[LCD_GUN_1].IsVinStart = TRUE;
    	thaisen_app_set_vin_start_charge(LCD_GUN_1);
    }else;
}


void SerialScreen_VinStartChargeB(void)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[LCD_GUN_2] Vin Start Charge###########\r\n");
    if(TRUE == LcdData.setData.sup_VIN){
        LcdAssistantData.SeveralGunFlag[LCD_GUN_2].IsVinStart = TRUE;
        thaisen_app_set_vin_start_charge(LCD_GUN_2);
    }else;
}

void SerialScreen_PWStartCharge(u8 port)
{
    if(TRUE == LcdData.setData.sup_pw_start){
        thaisen_app_set_password_start_charge(port);
    }else;
}

void SerialScreen_StopCharge(int port)
{
    sSCREEN_EVENT_DEBUGMSG("##########Port[%d] Stop Charge###########\r\n");
    if((TRUE == LcdData.setData.sup_Local) ||   \
            (LcdAssistantData.SeveralGunFlag[port].IsVinStart) ||   \
            ((TRUE == LcdData.setData.sup_Local_stop) && (thaisen_is_allow_loacl_stop(port))))
    	thaisen_app_set_screen_stop_charge(port);
	else ;
}

void SerialScreen_BtnChgInfoGet(int port)
{
    u8 *qrcode_pre = NULL;
    u8 qrcode_pre_len = 0;

	sSCREEN_EVENT_DEBUGMSG("##########ChgInfo###########\r\n");
	mem_set(LcdData.setData.pileID, 0, sizeof(LcdData.setData.pileID));
	mem_set(LcdData.setData.Help_Number, 0, sizeof(LcdData.setData.Help_Number));
	mem_set(LcdData.setData.ErWeiCodePre, 0, sizeof(LcdData.setData.ErWeiCodePre));
	str_ncpy((char *)(LcdData.setData.pileID), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_PILE_NUMBER, 0)), \
	         sizeof(LcdData.setData.pileID));
	str_ncpy((char *)(LcdData.setData.Help_Number), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_HELP_PHONE, 0)), \
					 sizeof(LcdData.setData.Help_Number));

    qrcode_pre = thaisen_app_get_qrcode_prefix(&qrcode_pre_len);
    if(qrcode_pre_len && qrcode_pre){
        str_ncpy((char *)(LcdData.setData.ErWeiCodePre), qrcode_pre, qrcode_pre_len);
    }

	sSCREEN_EVENT_DEBUGMSG("##########pileID=%s helpnum:%s  qrcodefrex:%s###########\r\n",(char *)(LcdData.setData.pileID),(char *)(LcdData.setData.Help_Number),(char *)(LcdData.setData.ErWeiCodePre));
}

void SerialScreen_BtnChgInfoSet(int port)
{
    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    /** 平台配置 */
    if(((LcdData.setData.ErWeiCodePre[0] >= CP_SET_QRCODE_FORMAT_PREFIX) && (LcdData.setData.ErWeiCodePre[0] < CP_SET_QRCODE_FORMAT_SIZE)) &&
            ((LcdData.setData.ErWeiCodePre[1] >= CP_GENERATE_QRCODE_FORMAT_PREFIX) && (LcdData.setData.ErWeiCodePre[1] < CP_GENERATE_QRCODE_FORMAT_SIZE))){
        UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_QRCODE_PRE,LcdData.setData.ErWeiCodePre,str_len(LcdData.setData.ErWeiCodePre));
    }
    /** 屏幕配置 */
    else{
        u8 len = sizeof(LcdData.setData.ErWeiCodePre), i, j;
        u8 temp[len + 2];

        for(i = 2, j = 0; (i < len) && (j < len); i++, j++){
            temp[i] = LcdData.setData.ErWeiCodePre[j];
        }

        temp[0] = CP_SET_QRCODE_FORMAT_PREFIX;
        temp[1] = CP_GENERATE_QRCODE_FORMAT_PREFIX_DEVICE_SN_PORT;
        if(str_len(temp) > 0x02){
            UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_QRCODE_PRE,temp,str_len(temp));
        }
    }
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_PILE_NUMBER,LcdData.setData.pileID,str_len(LcdData.setData.pileID));
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_HELP_PHONE,LcdData.setData.Help_Number,str_len(LcdData.setData.Help_Number));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

    str_ncpy((char *)(LcdData.runData.pileID), (char *)(LcdData.setData.pileID), \
             sizeof(LcdData.runData.pileID));
    sSCREEN_EVENT_DEBUGMSG("##########pileID=%s helpnum:%s  qrcodefrex:%s###########\r\n",(char *)(LcdData.setData.pileID),(char *)(LcdData.setData.Help_Number),(char *)(LcdData.setData.ErWeiCodePre));
}

static void SerialScreen_BtnMeterNoInfoJudge(u32 *ret)
{
#define SERIALSCREEN_AMMETER_BAUD_2400       0      //电表波特率：2400
#define SERIALSCREEN_AMMETER_BAUD_4800       1      //电表波特率：4800
#define SERIALSCREEN_AMMETER_BAUD_9600       2      //电表波特率：9600
#define SERIALSCREEN_AMMETER_BAUD_38400      3      //电表波特率：38400
#define SERIALSCREEN_AMMETER_BAUD_115200     4      //电表波特率：115200

// @ thaisen_cfg_info_ammeter
#define SERIALSCREEN_METER_MODEL_POSITION    2      /* 电表型号在结构体 thaisen_cfg_info_ammeter 中的成员次序(从0开始)  */
#define SERIALSCREEN_METER_BAUD_POSITION     3      /* 电表串口波特率在结构体 thaisen_cfg_info_ammeter 中的成员次序(从0开始)  */
#define SERIALSCREEN_METER_CHECK_POSITION    4      /* 电表串口校验位在结构体 thaisen_cfg_info_ammeter 中的成员次序(从0开始)  */

    u32 result = 0;

    if(LcdData.setData.MeterModel > thaisenAmmeterModel_Other){
        LcdData.setData.MeterModel = thaisenAmmeterModel_RuiYin;
        result |= (1 <<SERIALSCREEN_METER_MODEL_POSITION);
    }
    if((LcdData.setData.MeterBaudrate < SERIALSCREEN_AMMETER_BAUD_2400) ||\
        (LcdData.setData.MeterBaudrate > SERIALSCREEN_AMMETER_BAUD_115200)){
        LcdData.setData.MeterBaudrate = SERIALSCREEN_AMMETER_BAUD_9600;
        result |= (1 <<SERIALSCREEN_METER_BAUD_POSITION);
    }
    if((LcdData.setData.MeterCheckWay < CP_AMMETER_CHECK_WAY_EVEN) ||\
            (LcdData.setData.MeterCheckWay > CP_AMMETER_CHECK_WAY_NONE)){
        LcdData.setData.MeterCheckWay = CP_AMMETER_CHECK_WAY_EVEN;
        result |= (1 <<SERIALSCREEN_METER_CHECK_POSITION);
    }
    if(ret){
        *ret = result;
    }

#undef SERIALSCREEN_AMMETER_BAUD_2400
#undef SERIALSCREEN_AMMETER_BAUD_4800
#undef SERIALSCREEN_AMMETER_BAUD_9600
#undef SERIALSCREEN_AMMETER_BAUD_38400
#undef SERIALSCREEN_AMMETER_BAUD_115200

#undef SERIALSCREEN_METER_MODEL_POSITION
#undef SERIALSCREEN_METER_BAUD_POSITION
#undef SERIALSCREEN_METER_CHECK_POSITION
}

void SerialScreen_BtnMeterNoInfoSet(void)
{
#define SERIALSCREEN_AMMETER_BAUD_2400       0      //电表波特率：2400
#define SERIALSCREEN_AMMETER_BAUD_4800       1      //电表波特率：4800
#define SERIALSCREEN_AMMETER_BAUD_9600       2      //电表波特率：9600
#define SERIALSCREEN_AMMETER_BAUD_38400      3      //电表波特率：38400
#define SERIALSCREEN_AMMETER_BAUD_115200     4      //电表波特率：115200

    u8 para = 0;
    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if(LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify == FALSE){
        SerialScreen_BtnMeterNoInfoJudge(NULL);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = FALSE;

    switch(LcdData.setData.MeterBaudrate){
    case SERIALSCREEN_AMMETER_BAUD_2400:
        para = CP_AMMETER_BAUDRATE_2400;
        break;
    case SERIALSCREEN_AMMETER_BAUD_4800:
        para = CP_AMMETER_BAUDRATE_4800;
        break;
    case SERIALSCREEN_AMMETER_BAUD_9600:
        para = CP_AMMETER_BAUDRATE_9600;
        break;
    case SERIALSCREEN_AMMETER_BAUD_38400:
        para = CP_AMMETER_BAUDRATE_38400;
        break;
    case SERIALSCREEN_AMMETER_BAUD_115200:
        para = CP_AMMETER_BAUDRATE_115200;
        break;
    default:
        para = CP_AMMETER_BAUDRATE_9600;
        LcdData.setData.MeterBaudrate = SERIALSCREEN_AMMETER_BAUD_9600;
        break;
    }
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_METER_BAUDRATE,&para,sizeof(para));

    para = LcdData.setData.MeterCheckWay;
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_METER_CHECK_WAY,&para,sizeof(para));

    para = LcdData.setData.MeterModel;
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_METER_MODEL,&para,sizeof(para));
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_METER_NOA,LcdData.setData.MeterAddr[LCD_GUN_1],str_len(LcdData.setData.MeterAddr[LCD_GUN_1]));
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_METER_NOB,LcdData.setData.MeterAddr[LCD_GUN_2],str_len(LcdData.setData.MeterAddr[LCD_GUN_2]));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

    SerialScreen_SetMeterInfo();

#undef SERIALSCREEN_AMMETER_BAUD_2400
#undef SERIALSCREEN_AMMETER_BAUD_4800
#undef SERIALSCREEN_AMMETER_BAUD_9600
#undef SERIALSCREEN_AMMETER_BAUD_38400
#undef SERIALSCREEN_AMMETER_BAUD_115200
}

void SerialScreen_SetMeterInfo(void)
{
#define SERIALSCREEN_AMMETER_BAUD_2400       0      //电表波特率：2400
#define SERIALSCREEN_AMMETER_BAUD_4800       1      //电表波特率：4800
#define SERIALSCREEN_AMMETER_BAUD_9600       2      //电表波特率：9600
#define SERIALSCREEN_AMMETER_BAUD_38400      3      //电表波特率：38400
#define SERIALSCREEN_AMMETER_BAUD_115200     4      //电表波特率：115200

	u8 meterNo[LCD_GUN_NUM][6];
	u8 len[LCD_GUN_NUM]; 
	u8 i,j;
	for(i = 0; i < LCD_GUN_NUM; i++)
	{
	    len[i] = str_len(LcdData.setData.MeterAddr[i]);
        sSCREEN_EVENT_DEBUGMSG("LcdData.setData.MeterAddr[%d][0] = %d \r\n",i,LcdData.setData.MeterAddr[i][0]);
        if((len[i])&&  \
                ((LcdData.setData.MeterAddr[i][0] != 'A') && (LcdData.setData.MeterAddr[i][0] != 'a')) &&   \
                ((LcdData.setData.MeterAddr[i][0] != 'F') && (LcdData.setData.MeterAddr[i][0] != 'f')))
		{
			String2BCD(LcdData.setData.MeterAddr[i],meterNo[i]);
		}
		else
		{
            if((LcdData.setData.MeterAddr[i][0] == 'F') || (LcdData.setData.MeterAddr[i][0] == 'f')){
                for(j = 0; j < sizeof(meterNo[i]); j++ )
                    meterNo[i][j] = 0xFF;
            }else{
                for(j = 0; j < sizeof(meterNo[i]); j++ )
                    meterNo[i][j] = 0xAA;
            }
		}
#if 1
		sSCREEN_EVENT_DEBUGMSG("SetMeter[%d] No \r\n",i);
		for(j = 0; j < sizeof(meterNo[i]); j++ )
               sSCREEN_EVENT_DEBUGMSG("%02x ",meterNo[i][j]);
		sSCREEN_EVENT_DEBUGMSG("\r\n");
#endif
		thaisen_set_ammeterAddress(meterNo[i],i);
	}

    if(LcdData.setData.MeterModel > thaisenAmmeterModel_Other)
        LcdData.setData.MeterModel = thaisenAmmeterModel_RuiYin;

    if((LcdData.setData.MeterCheckWay < CP_AMMETER_CHECK_WAY_EVEN) ||\
            (LcdData.setData.MeterCheckWay > CP_AMMETER_CHECK_WAY_NONE)){
        LcdData.setData.MeterCheckWay = CP_AMMETER_CHECK_WAY_EVEN;
    }
    switch(LcdData.setData.MeterCheckWay){
    case CP_AMMETER_CHECK_WAY_EVEN:
        thaisen_set_ammeterCheckWay(THAISEN_CHECK_WAY_EVEN);
        break;
    case CP_AMMETER_CHECK_WAY_ODD:
        thaisen_set_ammeterCheckWay(THAISEN_CHECK_WAY_ODD);
        break;
    case CP_AMMETER_CHECK_WAY_NONE:
        thaisen_set_ammeterCheckWay(THAISEN_CHECK_WAY_NONE);
        break;
    }

    if((LcdData.setData.MeterBaudrate < SERIALSCREEN_AMMETER_BAUD_2400) ||\
            (LcdData.setData.MeterBaudrate > SERIALSCREEN_AMMETER_BAUD_115200)){
        LcdData.setData.MeterBaudrate = SERIALSCREEN_AMMETER_BAUD_9600;
    }
    switch(LcdData.setData.MeterBaudrate){
    case SERIALSCREEN_AMMETER_BAUD_2400:
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_2400);
        break;
    case SERIALSCREEN_AMMETER_BAUD_4800:
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_4800);
        break;
    case SERIALSCREEN_AMMETER_BAUD_9600:
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_9600);
        break;
    case SERIALSCREEN_AMMETER_BAUD_38400:
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_38400);
        break;
    case SERIALSCREEN_AMMETER_BAUD_115200:
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_115200);
        break;
    }

    thaisen_set_ammnterModel(LcdData.setData.MeterModel);

#undef SERIALSCREEN_AMMETER_BAUD_2400
#undef SERIALSCREEN_AMMETER_BAUD_4800
#undef SERIALSCREEN_AMMETER_BAUD_9600
#undef SERIALSCREEN_AMMETER_BAUD_38400
#undef SERIALSCREEN_AMMETER_BAUD_115200
}


void SerialScreen_BtnMeterNoInfoGet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########MeterNoInfo###########\r\n");
	mem_set(LcdData.setData.MeterAddr, 0, sizeof(LcdData.setData.MeterAddr));
	str_ncpy((char *)(LcdData.setData.MeterAddr[LCD_GUN_1]), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_METER_NOA, 0)), \
	         sizeof(LcdData.setData.MeterAddr[LCD_GUN_1]) - 1);
	str_ncpy((char *)(LcdData.setData.MeterAddr[LCD_GUN_2]), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_METER_NOB, 0)), \
	         sizeof(LcdData.setData.MeterAddr[LCD_GUN_2]) - 1);	
	sSCREEN_EVENT_DEBUGMSG("##########MeterAddr[0]=%s MeterAddr[1]=%s###########\r\n",(char *)(LcdData.setData.MeterAddr[LCD_GUN_1]),(char *)(LcdData.setData.MeterAddr[LCD_GUN_2]));
}



void SerialScreen_BtnServerGet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########ServerInfo###########\r\n");
	mem_set(LcdData.setData.svrIp, 0, sizeof(LcdData.setData.svrIp));
	mem_set(LcdData.setData.svrPort,0,sizeof(LcdData.setData.svrPort));
	str_ncpy((char *)(LcdData.setData.svrIp), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_IP_DOMAIN, 0)), \
	         sizeof(LcdData.setData.svrIp));
	LcdData.setData.svrPort=*((u16*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_PORT, 0));
	sSCREEN_EVENT_DEBUGMSG("##########serverip=%s port=%d###########\r\n",(char *)(LcdData.setData.svrIp),LcdData.setData.svrPort);
}

void SerialScreen_BtnServerSet(void)
{
    u8 nettype = CP_NETTYPE_4G;
    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if(LcdData.setData.NetType >= CP_NETTYPE_SIZE){
        LcdData.setData.NetType = CP_NETTYPE_4G;
    }
    nettype = (u8)LcdData.setData.NetType;

    rt_kprintf("SerialScreen_BtnServerSet(%d)\n", nettype);
	UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_IP_DOMAIN, LcdData.setData.svrIp, str_len(LcdData.setData.svrIp));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_PORT, (u16 *)&(LcdData.setData.svrPort), sizeof(LcdData.setData.svrPort));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_NET_TYPE, &(nettype), sizeof(nettype));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

	sSCREEN_EVENT_DEBUGMSG("##########svrIp=%s svrPort=%d###########\r\n",LcdData.setData.svrIp,LcdData.setData.svrPort);
}

void SerialScreen_BtnVinListSet(void)
{
    uint8_t *vin = UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_VIN_WHITELIST, 0);

    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    thaisen_vin_whitelists_clear();
    for(uint8_t count = 0; count < VIN_LIST_NUM; count++){
        thaisen_vin_whitelists_add(LcdData.setData.s_vin_lists[count], sizeof(LcdData.setData.s_vin_lists[count]));
    }
    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_VIN_WHITELIST, NULL, 1);

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }
}

void SerialScreen_BtnVinListGet(void)
{
    uint8_t *vin = UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_VIN_WHITELIST, 0);
    memcpy(LcdData.setData.s_vin_lists, vin, sizeof(LcdData.setData.s_vin_lists));
}

void SerialScreen_BtnModuleGet(void)
{
	u8 i = 0;
	u8 ModuleModel = 0, ModuleGroup = 0, ModuleNumberSingle = 0;
	u16 Rated_Output_Voltage = 0;
    u16 Max_Output_Voltage = 0;
    u16 Min_Output_Voltage = 0;
    u16 Rated_Limit_Current = 0;
    u16 Max_Limit_Current = 0;
    u16 Min_Limit_Current = 0;

	sSCREEN_EVENT_DEBUGMSG("##########ModuleGet###########\r\n");
	mem_set(LcdData.setData.ModuleGroupNum, 0, sizeof(LcdData.setData.ModuleGroupNum));
	mem_set(LcdData.setData.ModuleNum,0,sizeof(LcdData.setData.ModuleNum));
	mem_set(LcdData.setData.RmType,0,sizeof(LcdData.setData.RmType));
	ModuleModel = *(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MODULE_MODEL, 0));
	if(ModuleModel > MODULE_MODEL_NUMBER)
	    ModuleModel = MODULE_MODEL_DEFAULT;

	LcdData.setData.RmType = ModuleModel;
	ModuleGroup = *(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MODULE_GROUP_NUM, 0));
    if(ModuleGroup > MODULE_GROUP_NUMBER_MAX)
        ModuleGroup = MODULE_GROUP_NUMBER_DEFAULT;

	LcdData.setData.ModuleGroupNum = ModuleGroup;
	sSCREEN_EVENT_DEBUGMSG("ModuleGroupNum=%d RmType=%d\r\n",LcdData.setData.ModuleGroupNum,LcdData.setData.RmType);
	for(i= 0; i < LcdData.setData.ModuleGroupNum; i++)
	{
	    ModuleNumberSingle = *(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MODULE_NUM_GROUP_1+i, 0));
	    if(ModuleNumberSingle > MODULE_NUMBER_SINGLE_MAX)
	        ModuleNumberSingle = MODULE_NUMBER_SINGLE_DEFAULT;

		LcdData.setData.ModuleNum[i] = ModuleNumberSingle;
		sSCREEN_EVENT_DEBUGMSG("ModuleNum[%d]=%d ",i,LcdData.setData.ModuleNum[i]);
	}

    LcdData.setData.Rated_Output_Voltage = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, 0));
    LcdData.setData.Max_Output_Voltage = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, 0));
    LcdData.setData.Min_Output_Voltage = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, 0));
    LcdData.setData.Rated_Limit_Current = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_RATED_LIMIT_CURRENT, 0));
    LcdData.setData.Max_Limit_Current = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MAX_LIMIT_CURRENT, 0));
    LcdData.setData.Min_Limit_Current = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_MIN_LIMIT_CURRENT, 0));

    Rated_Output_Voltage = SerialScreen_GetPara_ValidValue(LcdData.setData.Rated_Output_Voltage,
            MODULE_RATED_OUTVOLT_DEF, MODULE_RATED_OUTVOLT_MIN, MODULE_RATED_OUTVOLT_MAX);

    Max_Output_Voltage = SerialScreen_GetPara_ValidValue(LcdData.setData.Max_Output_Voltage,
            CHARGEPILE_MAX_OUTVOLT_DEF, CHARGEPILE_MAX_OUTVOLT_MIN, CHARGEPILE_MAX_OUTVOLT_MAX);

    Min_Output_Voltage = SerialScreen_GetPara_ValidValue(LcdData.setData.Min_Output_Voltage,
            CHARGEPILE_MIN_OUTVOLT_DEF, CHARGEPILE_MIN_OUTVOLT_MIN, CHARGEPILE_MIN_OUTVOLT_MAX);

    Rated_Limit_Current = SerialScreen_GetPara_ValidValue(LcdData.setData.Rated_Limit_Current,
            MODULE_RATED_LIMIT_CURR_DEF, MODULE_RATED_LIMIT_CURR_MIN, MODULE_RATED_LIMIT_CURR_MAX);

    Max_Limit_Current = SerialScreen_GetPara_ValidValue(LcdData.setData.Max_Limit_Current,
            MODULE_MAX_LIMIT_CURR_DEF, MODULE_MAX_LIMIT_CURR_MIN, MODULE_MAX_LIMIT_CURR_MAX);

    Min_Limit_Current = SerialScreen_GetPara_ValidValue(LcdData.setData.Min_Limit_Current,
            MODULE_MIN_LIMIT_CURR_DEF, MODULE_MIN_LIMIT_CURR_MIN, MODULE_MIN_LIMIT_CURR_MAX);

    LcdData.setData.Rated_Output_Voltage = Rated_Output_Voltage;
    LcdData.setData.Max_Output_Voltage = Max_Output_Voltage;
    LcdData.setData.Min_Output_Voltage = Min_Output_Voltage;
    LcdData.setData.Rated_Limit_Current = Rated_Limit_Current;
    LcdData.setData.Max_Limit_Current = Max_Limit_Current;
    LcdData.setData.Min_Limit_Current = Min_Limit_Current;

    extern void thaisenSetModuleMaxVolt(uint16_t volt);
    extern void thaisenSetModuleMinVolt(uint16_t volt);
    extern void thaisenSetModuleMaxCurr(uint16_t curr);
    extern void thaisenSetModuleMinCurr(uint16_t curr);
    extern void thaisenSetModuleMaxChargVolt(uint16_t volt);
    extern void thaisenSetModuleMaxChargCurr(uint16_t curr);

    thaisenSetModuleMaxVolt(LcdData.setData.Rated_Output_Voltage *10);
    thaisenSetModuleMinVolt(LcdData.setData.Min_Output_Voltage *10);

    thaisenSetModuleMaxCurr(LcdData.setData.Rated_Limit_Current *10);
    thaisenSetModuleMinCurr(LcdData.setData.Min_Limit_Current *10);

    thaisenSetModuleMaxChargVolt(LcdData.setData.Max_Output_Voltage *10);
    thaisenSetModuleMaxChargCurr(LcdData.setData.Max_Limit_Current *10);

#ifdef SCREEN_USING_DOUBLE_GUN
    thaisenSetModuleMaxChargCurrGroup(0, LcdData.setData.Max_Limit_Current *10 /2);
    thaisenSetModuleMaxChargCurrGroup(1, LcdData.setData.Max_Limit_Current *10 /2);
#else
    thaisenSetModuleMaxChargCurrGroup(0, LcdData.setData.Max_Limit_Current *10);
    thaisenSetModuleMaxChargCurrGroup(1, 0);
#endif

    rt_kprintf("thaisenSetModuleMaxVolt|%d    thaisenSetModuleMinVolt|%d\n", LcdData.setData.Rated_Output_Voltage, LcdData.setData.Min_Output_Voltage);
    rt_kprintf("thaisenSetModuleMaxCurr|%d    thaisenSetModuleMinCurr|%d\n", LcdData.setData.Rated_Limit_Current, LcdData.setData.Min_Limit_Current);
    rt_kprintf("thaisenSetModuleMaxChargVolt|%d    thaisenSetModuleMaxChargCurr|%d\n", LcdData.setData.Max_Output_Voltage, LcdData.setData.Max_Limit_Current);
}

static void SerialScreen_BtnModuleInfoJudge(u32 *ret)
{
//@ thaisen_cfg_info_module
#define SSCREEN_MODULE_PROTOCOL_POSITION                    0    /* 模块型号在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_MGROUP_POSITION                      1    /* 模块组数在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_SMNUM_POSITION                       2    /* 组内模块数在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_RATED_OVOLT_POSITION                 10   /* 模块额定输出在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_RATED_LCURR_POSITION                 11   /* 模块额定限电流在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_MAX_OVOLT_POSITION                   12   /* 桩最大输出电压在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_MIN_OVOLT_POSITION                   13   /* 桩最小输出电压在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_MAX_LCURR_POSITION                   14   /* 桩最大输出电流在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */
#define SSCREEN_MODULE_MIN_LCURR_POSITION                   15   /* 桩最小输出电流在结构体 thaisen_cfg_info_module 中的成员次序(从0开始)  */

    u32 result = 0;
    u16 Rated_Output_Voltage = 0;
    u16 Max_Output_Voltage = 0;
    u16 Min_Output_Voltage = 0;
    u16 Rated_Limit_Current = 0;
    u16 Max_Limit_Current = 0;
    u16 Min_Limit_Current = 0;

    if(LcdData.setData.RmType > MODULE_MODEL_NUMBER){
        LcdData.setData.RmType = MODULE_MODEL_DEFAULT;
        result |= (1 <<SSCREEN_MODULE_PROTOCOL_POSITION);
    }
    if(LcdData.setData.ModuleGroupNum > MODULE_GROUP_NUMBER_MAX){
        LcdData.setData.ModuleGroupNum = MODULE_GROUP_NUMBER_DEFAULT;
        result |= (1 <<SSCREEN_MODULE_MGROUP_POSITION);
    }
    for(u8 i = 0; i < sizeof(LcdData.setData.ModuleNum); i++)
    {
        if(LcdData.setData.ModuleNum[i] > MODULE_NUMBER_SINGLE_MAX){
            LcdData.setData.ModuleNum[i] = MODULE_NUMBER_SINGLE_DEFAULT;
            result |= (1 <<(SSCREEN_MODULE_SMNUM_POSITION + i));
        }
    }

    Rated_Output_Voltage = SerialScreen_GetPara_ValidValue(LcdData.setData.Rated_Output_Voltage,
            MODULE_RATED_OUTVOLT_DEF, MODULE_RATED_OUTVOLT_MIN, MODULE_RATED_OUTVOLT_MAX);

    Max_Output_Voltage = SerialScreen_GetPara_ValidValue(LcdData.setData.Max_Output_Voltage,
            CHARGEPILE_MAX_OUTVOLT_DEF, CHARGEPILE_MAX_OUTVOLT_MIN, CHARGEPILE_MAX_OUTVOLT_MAX);

    Min_Output_Voltage = SerialScreen_GetPara_ValidValue(LcdData.setData.Min_Output_Voltage,
            CHARGEPILE_MIN_OUTVOLT_DEF, CHARGEPILE_MIN_OUTVOLT_MIN, CHARGEPILE_MIN_OUTVOLT_MAX);

    Rated_Limit_Current = SerialScreen_GetPara_ValidValue(LcdData.setData.Rated_Limit_Current,
            MODULE_RATED_LIMIT_CURR_DEF, MODULE_RATED_LIMIT_CURR_MIN, MODULE_RATED_LIMIT_CURR_MAX);

    Max_Limit_Current = SerialScreen_GetPara_ValidValue(LcdData.setData.Max_Limit_Current,
            MODULE_MAX_LIMIT_CURR_DEF, MODULE_MAX_LIMIT_CURR_MIN, MODULE_MAX_LIMIT_CURR_MAX);

    Min_Limit_Current = SerialScreen_GetPara_ValidValue(LcdData.setData.Min_Limit_Current,
            MODULE_MIN_LIMIT_CURR_DEF, MODULE_MIN_LIMIT_CURR_MIN, MODULE_MIN_LIMIT_CURR_MAX);

    if(LcdData.setData.Rated_Output_Voltage != Rated_Output_Voltage){
        result |= (1 <<SSCREEN_MODULE_RATED_OVOLT_POSITION);
    }
    if(LcdData.setData.Rated_Limit_Current != Rated_Limit_Current){
        result |= (1 <<SSCREEN_MODULE_RATED_LCURR_POSITION);
    }
    if(LcdData.setData.Max_Output_Voltage != Max_Output_Voltage){
        result |= (1 <<SSCREEN_MODULE_MAX_OVOLT_POSITION);
    }
    if(LcdData.setData.Min_Output_Voltage != Min_Output_Voltage){
        result |= (1 <<SSCREEN_MODULE_MIN_OVOLT_POSITION);
    }
    if(LcdData.setData.Max_Limit_Current != Max_Limit_Current){
        result |= (1 <<SSCREEN_MODULE_MAX_LCURR_POSITION);
    }
    if(LcdData.setData.Min_Limit_Current != Min_Limit_Current){
        result |= (1 <<SSCREEN_MODULE_MIN_LCURR_POSITION);
    }

    LcdData.setData.Rated_Output_Voltage = Rated_Output_Voltage;
    LcdData.setData.Max_Output_Voltage = Max_Output_Voltage;
    LcdData.setData.Min_Output_Voltage = Min_Output_Voltage;
    LcdData.setData.Rated_Limit_Current = Rated_Limit_Current;
    LcdData.setData.Max_Limit_Current = Max_Limit_Current;
    LcdData.setData.Min_Limit_Current = Min_Limit_Current;

    if(ret){
        *ret = result;
    }

#undef SSCREEN_MODULE_PROTOCOL_POSITION
#undef SSCREEN_MODULE_MGROUP_POSITION
#undef SSCREEN_MODULE_SMNUM_POSITION
#undef SSCREEN_MODULE_RATED_OVOLT_POSITION
#undef SSCREEN_MODULE_RATED_LCURR_POSITION
#undef SSCREEN_MODULE_MAX_OVOLT_POSITION
#undef SSCREEN_MODULE_MIN_OVOLT_POSITION
#undef SSCREEN_MODULE_MAX_LCURR_POSITION
#undef SSCREEN_MODULE_MIN_LCURR_POSITION
}

void SerialScreen_BtnModuleSet(void)
{
    u32 power = 0;
    u8 ModuleModel = LcdData.setData.RmType;

    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if(LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify == FALSE){
        SerialScreen_BtnModuleInfoJudge(NULL);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = FALSE;

    for(u8 i= 0; i < LCD_MODULE_GROUP_MAX; i++){
        UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MODULE_NUM_GROUP_1+i,&LcdData.setData.ModuleNum[i], sizeof(LcdData.setData.ModuleNum[i]));
    }
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MODULE_GROUP_NUM, &LcdData.setData.ModuleGroupNum, sizeof(LcdData.setData.ModuleGroupNum));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MODULE_MODEL, &ModuleModel, sizeof(ModuleModel));

    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_RATED_OUTPUT_VOLTAGE, &LcdData.setData.Rated_Output_Voltage, sizeof(LcdData.setData.Rated_Output_Voltage));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MAX_OUTPUT_VOLTAGE, &LcdData.setData.Max_Output_Voltage, sizeof(LcdData.setData.Max_Output_Voltage));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MIN_OUTPUT_VOLTAGE, &LcdData.setData.Min_Output_Voltage, sizeof(LcdData.setData.Min_Output_Voltage));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_RATED_LIMIT_CURRENT, &LcdData.setData.Rated_Limit_Current, sizeof(LcdData.setData.Rated_Limit_Current));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MAX_LIMIT_CURRENT, &LcdData.setData.Max_Limit_Current, sizeof(LcdData.setData.Max_Limit_Current));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_MIN_LIMIT_CURRENT, &LcdData.setData.Min_Limit_Current, sizeof(LcdData.setData.Min_Limit_Current));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

    power = thaisen_get_power_from_percent(thaisen_get_power_percent());
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SYSTEM_POWER_TOTAL, &power, sizeof(power));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

    extern void thaisenSetModuleMaxVolt(uint16_t volt);
    extern void thaisenSetModuleMinVolt(uint16_t volt);
    extern void thaisenSetModuleMaxCurr(uint16_t curr);
    extern void thaisenSetModuleMinCurr(uint16_t curr);
    extern void thaisenSetModuleMaxChargVolt(uint16_t volt);
    extern void thaisenSetModuleMaxChargCurr(uint16_t curr);

    thaisenSetModuleMaxVolt(LcdData.setData.Rated_Output_Voltage *10);
    thaisenSetModuleMinVolt(LcdData.setData.Min_Output_Voltage *10);

    thaisenSetModuleMaxCurr(LcdData.setData.Rated_Limit_Current *10);
    thaisenSetModuleMinCurr(LcdData.setData.Min_Limit_Current *10);

    thaisenSetModuleMaxChargVolt(LcdData.setData.Max_Output_Voltage *10);
    thaisenSetModuleMaxChargCurr(LcdData.setData.Max_Limit_Current *10);

#ifdef SCREEN_USING_DOUBLE_GUN
    thaisenSetModuleMaxChargCurrGroup(0, LcdData.setData.Max_Limit_Current *10 /2);
    thaisenSetModuleMaxChargCurrGroup(1, LcdData.setData.Max_Limit_Current *10 /2);
#else
    thaisenSetModuleMaxChargCurrGroup(0, LcdData.setData.Max_Limit_Current *10);
    thaisenSetModuleMaxChargCurrGroup(1, 0);
#endif

    LcdAssistantData.Flag.IsSetPowerPercent = TRUE;

    rt_kprintf("thaisenSetModuleMaxVolt|%d    thaisenSetModuleMinVolt|%d\n", LcdData.setData.Rated_Output_Voltage, LcdData.setData.Min_Output_Voltage);
    rt_kprintf("thaisenSetModuleMaxCurr|%d    thaisenSetModuleMinCurr|%d\n", LcdData.setData.Rated_Limit_Current, LcdData.setData.Min_Limit_Current);
    rt_kprintf("thaisenSetModuleMaxChargVolt|%d    thaisenSetModuleMaxChargCurr|%d\n", LcdData.setData.Max_Output_Voltage, LcdData.setData.Max_Limit_Current);

    sSCREEN_EVENT_DEBUGMSG("ModuleGroupNum=%d  RmType =%d\r\n",LcdData.setData.ModuleGroupNum,LcdData.setData.RmType);
}

static void SerialScreen_BtnSystemFuncJudge(u32 *ret)
{
//@ thaisen_cfg_info_system
#define SSCREEN_ALLOCATE_WAY_POSITION                    0    /* 分配方式在结构体 thaisen_cfg_info_system 中的成员次序(从0开始)  */
#define SSCREEN_DEVICE_TYPE_POSITION                     1    /* 设备类型在结构体 thaisen_cfg_info_system 中的成员次序(从0开始)  */

    u32 result = 0;
    u8 way = LcdData.setData.AllocWay;
    u8 type = LcdData.setData.DevType;

    if(type >= SYSTEM_FUNCTION_SIZE){        /* 设备类型默认双枪一体 */
        type = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
        result |= (1 <<SSCREEN_DEVICE_TYPE_POSITION);
    }
    if(way >= POWER_ALLOCATION_WAY_SIZE){
        way = POWER_ALLOCATION_WAY_SEQ_PRIORITY;
#if 0
        result |= (1 <<SSCREEN_ALLOCATE_WAY_POSITION);
#endif
    }

    switch(type){
    case SYSTEM_FUNCTION_DYNAMIC_SWITCH:
        if(LcdAssistantData.Flag.ParaChargeSelect == FALSE){
            thaisenSetChargGunRunType(thaisenDeviceType_average);
            LcdData.setData.DevType = type;
            LcdAssistantData.DeviceType = type;
        }else{
            LcdData.setData.DevType = LcdAssistantData.DeviceType;
            result |= (1 <<SSCREEN_DEVICE_TYPE_POSITION);
        }
        break;
    default:
        LcdData.setData.DevType = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
        LcdAssistantData.DeviceType = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
        thaisenSetChargGunRunType(thaisenDeviceType_doubleGun);
        break;
    }
    if(ret){
        *ret = result;
    }

#undef SSCREEN_ALLOCATE_WAY_POSITION
#undef SSCREEN_DEVICE_TYPE_POSITION
}

void SerialScreen_BtnSystemFuncSet(void)
{
    u8 para = 0;
    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if(LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify == FALSE){
        SerialScreen_BtnSystemFuncJudge(NULL);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = FALSE;

    /* 此处要设置功率分配方式 */
//    thaisenSetAllocateStrategy(LcdData.setData.AllocWay);
//    LcdData.setData.AllocWay = POWER_ALLOCATION_WAY_AVERAGE;
//    LcdData.setData.AllocWay = way;
//
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_ALLOCATION_WAY, &way, sizeof(way));
    para = LcdData.setData.DevType;
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_DEVICE_TYPE, &para, sizeof(para));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

//    thaisenSetAllocateStrategy(LcdData.setData.AllocWay);
    rt_kprintf("current power allocation way(%d) device type(%d)\n", LcdData.setData.AllocWay, LcdData.setData.DevType);
}

void SerialScreen_BtnProtectInfoGet(void)
{
#if 0
    LcdData.setData.Input_OverVolt = *(u32 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INPUT_OVERVOL, 0));
    LcdData.setData.Input_UnderVolt = *(u32 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INPUT_UNDERVOL, 0));
    LcdData.setData.Onput_OverVolt = *(u32 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OUTPUT_OVERVOL, 0));
    LcdData.setData.Onput_UnderVolt = *(u32 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OUTPUT_UNDERVOL, 0));
    LcdData.setData.Onput_OverCurr = *(u32 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OUTPUT_OVERCUR, 0));
#endif

    LcdData.setData.Stop_SOC = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SOC_STOP, 0));
    LcdData.setData.OverTemp_Warnning = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_WARN, 0));
    LcdData.setData.OverTemp_Stop = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_STOP, 0));
    LcdData.setData.OverTemp_Resume = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_RECOVER, 0));
    LcdData.setData.OverTemp_LimitCurr = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_SETCUR, 0));
    LcdData.setData.GunVolt_LimitValue = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_GUNVOLT_LIMIT, 0));
    LcdData.setData.ElossProprotion = *(u16 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_ELOSS_PROPORTION, 0));
    LcdData.setData.PowerPercent = thaisen_get_power_percent();

#if 0
    LcdData.setData.Input_OverVolt = SerialScreen_GetPara_ValidValue(LcdData.setData.Input_OverVolt,
            750, 500, 1200);

    LcdData.setData.Input_UnderVolt = SerialScreen_GetPara_ValidValue(LcdData.setData.Input_UnderVolt,
            750, 500, 1200);

    LcdData.setData.Onput_OverVolt = SerialScreen_GetPara_ValidValue(LcdData.setData.Onput_OverVolt,
            750, 500, 1200);

    LcdData.setData.Onput_UnderVolt = SerialScreen_GetPara_ValidValue(LcdData.setData.Onput_UnderVolt,
            750, 500, 1200);

    LcdData.setData.Onput_OverCurr = SerialScreen_GetPara_ValidValue(LcdData.setData.Onput_OverCurr,
            750, 500, 1200);
#endif
    LcdData.setData.Stop_SOC = SerialScreen_GetPara_ValidValue(LcdData.setData.Stop_SOC,
            PROTECT_STOP_SOC_VALUE_DEFAULT, PROTECT_STOP_SOC_VALUE_MIN, PROTECT_STOP_SOC_VALUE_MAX);

    LcdData.setData.OverTemp_Warnning = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_Warnning,
            PROTECT_OVERTEMP_WARNNING_VALUE_DEFAULT, PROTECT_OVERTEMP_WARNNING_VALUE_MIN, PROTECT_OVERTEMP_WARNNING_VALUE_MAX);

    LcdData.setData.OverTemp_Stop = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_Stop,
            PROTECT_OVERTEMP_STOP_VALUE_DEFAULT, PROTECT_OVERTEMP_STOP_VALUE_MIN, PROTECT_OVERTEMP_STOP_VALUE_MAX);

    LcdData.setData.OverTemp_Resume = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_Resume,
            PROTECT_OVERTEMP_RESUME_VALUE_DEFAULT, PROTECT_OVERTEMP_RESUME_VALUE_MIN, PROTECT_OVERTEMP_RESUME_VALUE_MAX);

    LcdData.setData.OverTemp_LimitCurr = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_LimitCurr,
            PROTECT_OVERTEMP_LIMITCURR_VALUE_DEFAULT, PROTECT_OVERTEMP_LIMITCURR_VALUE_MIN, PROTECT_OVERTEMP_LIMITCURR_VALUE_MAX);

    if((LcdData.setData.GunVolt_LimitValue < GUNVOLT_LIMIT_VALUE_MIN) || (LcdData.setData.GunVolt_LimitValue > GUNVOLT_LIMIT_VALUE_MAX)){
        LcdData.setData.GunVolt_LimitValue = GUNVOLT_LIMIT_VALUE_MIN;
    }

    rt_kprintf("GunVolt_LimitValue Limit|%d\n", LcdData.setData.GunVolt_LimitValue);
    rt_kprintf("Stop_SOC|%d    OverTemp_Warnning|%d\n", LcdData.setData.Stop_SOC, LcdData.setData.OverTemp_Warnning);
    rt_kprintf("OverTemp_Stop|%d    OverTemp_Resume|%d\n", LcdData.setData.OverTemp_Stop, LcdData.setData.OverTemp_Resume);
    rt_kprintf("OverTemp_LimitCurr|%d    OverTemp_LimitCurr|%d\n", LcdData.setData.OverTemp_LimitCurr, LcdData.setData.OverTemp_LimitCurr);
}

static void SerialScreen_BtnProtectInfoJudge(u32 *ret)
{
//@ thaisen_cfg_info_protect
#define SSCREEN_OT_WARNNING_POSITION                        0   /* 过温告警值在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_OT_STOP_POSITION                            1   /* 过温停充值在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_OT_RESUME_POSITION                          2   /* 过温恢复值在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_OT_LIMIT_POSITION                           3   /* 过温限流值出在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_GUN_VOLTAGE_POSITION                        4   /* 枪头电压限值在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_STOP_SOC_POSITION                           5   /* 停充 SOC在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_POWER_PERCENT_POSITION                      6   /* 功率百分比在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */
#define SSCREEN_ELOSS_PROPROTION_POSITION                   7   /* 电损比在结构体 thaisen_cfg_info_protect 中的成员次序(从0开始)  */

    u32 result = 0;
    u32 Stop_SOC = 0;
    u32 OverTemp_Warnning = 0;
    u32 OverTemp_Stop = 0;
    u32 OverTemp_Resume = 0;
    u32 OverTemp_LimitCurr = 0;

    Stop_SOC = SerialScreen_GetPara_ValidValue(LcdData.setData.Stop_SOC,
            PROTECT_STOP_SOC_VALUE_DEFAULT, PROTECT_STOP_SOC_VALUE_MIN, PROTECT_STOP_SOC_VALUE_MAX);

    OverTemp_Warnning = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_Warnning,
            PROTECT_OVERTEMP_WARNNING_VALUE_DEFAULT, PROTECT_OVERTEMP_WARNNING_VALUE_MIN, PROTECT_OVERTEMP_WARNNING_VALUE_MAX);

    OverTemp_Stop = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_Stop,
            PROTECT_OVERTEMP_STOP_VALUE_DEFAULT, PROTECT_OVERTEMP_STOP_VALUE_MIN, PROTECT_OVERTEMP_STOP_VALUE_MAX);

    OverTemp_Resume = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_Resume,
            PROTECT_OVERTEMP_RESUME_VALUE_DEFAULT, PROTECT_OVERTEMP_RESUME_VALUE_MIN, PROTECT_OVERTEMP_RESUME_VALUE_MAX);

    OverTemp_LimitCurr = SerialScreen_GetPara_ValidValue(LcdData.setData.OverTemp_LimitCurr,
            PROTECT_OVERTEMP_LIMITCURR_VALUE_DEFAULT, PROTECT_OVERTEMP_LIMITCURR_VALUE_MIN, PROTECT_OVERTEMP_LIMITCURR_VALUE_MAX);

    if((LcdData.setData.GunVolt_LimitValue < GUNVOLT_LIMIT_VALUE_MIN) || (LcdData.setData.GunVolt_LimitValue > GUNVOLT_LIMIT_VALUE_MAX)){
        LcdData.setData.GunVolt_LimitValue = GUNVOLT_LIMIT_VALUE_MIN;
        result |= (1 <<SSCREEN_GUN_VOLTAGE_POSITION);
    }

    if(LcdData.setData.PowerPercent < PROTECT_POWER_PERCENT_VALUE_MIN){
        LcdData.setData.PowerPercent = PROTECT_POWER_PERCENT_VALUE_MIN;
        result |= (1 <<SSCREEN_POWER_PERCENT_POSITION);
    }else if(LcdData.setData.PowerPercent > PROTECT_POWER_PERCENT_VALUE_MAX){
        LcdData.setData.PowerPercent = PROTECT_POWER_PERCENT_VALUE_MAX;
        result |= (1 <<SSCREEN_POWER_PERCENT_POSITION);
    }

    if((LcdData.setData.ElossProprotion < CHARGEPILE_ELOSS_PROPORTION_MIN) || (LcdData.setData.ElossProprotion > CHARGEPILE_ELOSS_PROPORTION_MAX)){
        LcdData.setData.ElossProprotion = CHARGEPILE_ELOSS_PROPORTION_DEF;
        result |= (1 <<SSCREEN_ELOSS_PROPROTION_POSITION);
    }

    if(LcdData.setData.Stop_SOC != Stop_SOC){
        result |= (1 <<SSCREEN_STOP_SOC_POSITION);
    }
    if(LcdData.setData.OverTemp_Warnning != OverTemp_Warnning){
        result |= (1 <<SSCREEN_OT_WARNNING_POSITION);
    }
    if(LcdData.setData.OverTemp_Stop != OverTemp_Stop){
        result |= (1 <<SSCREEN_OT_STOP_POSITION);
    }
    if(LcdData.setData.OverTemp_Resume != OverTemp_Resume){
        result |= (1 <<SSCREEN_OT_RESUME_POSITION);
    }
    if(LcdData.setData.OverTemp_LimitCurr != OverTemp_LimitCurr){
        result |= (1 <<SSCREEN_OT_LIMIT_POSITION);
    }

    LcdData.setData.Stop_SOC = Stop_SOC;
    LcdData.setData.OverTemp_Warnning = OverTemp_Warnning;
    LcdData.setData.OverTemp_Stop = OverTemp_Stop;
    LcdData.setData.OverTemp_Resume = OverTemp_Resume;
    LcdData.setData.OverTemp_LimitCurr = OverTemp_LimitCurr;

    if(ret){
        *ret = result;
    }

#undef SSCREEN_OT_WARNNING_POSITION
#undef SSCREEN_OT_STOP_POSITION
#undef SSCREEN_OT_RESUME_POSITION
#undef SSCREEN_OT_LIMIT_POSITION
#undef SSCREEN_GUN_VOLTAGE_POSITION
#undef SSCREEN_STOP_SOC_POSITION
#undef SSCREEN_POWER_PERCENT_POSITION
#undef SSCREEN_ELOSS_PROPROTION_POSITION
}

void SerialScreen_BtnProtectInfoSet(void)
{
    u32 power = 0;
    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if(LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify == FALSE){
        SerialScreen_BtnProtectInfoJudge(NULL);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = FALSE;

    thaisen_set_ChargGunVolt((LcdData.setData.GunVolt_LimitValue /10), 0);
    thaisen_set_ChargGunVolt((LcdData.setData.GunVolt_LimitValue /10), 1);

    power = thaisen_get_power_from_percent(LcdData.setData.PowerPercent);

    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SYSTEM_POWER_TOTAL, &power, sizeof(power));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SOC_STOP, &LcdData.setData.Stop_SOC, sizeof(LcdData.setData.Stop_SOC) - 0x02);
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_WARN, &LcdData.setData.OverTemp_Warnning, sizeof(LcdData.setData.OverTemp_Warnning) - 0x02);
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_STOP, &LcdData.setData.OverTemp_Stop, sizeof(LcdData.setData.OverTemp_Stop) - 0x02);
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_RECOVER, &LcdData.setData.OverTemp_Resume, sizeof(LcdData.setData.OverTemp_Resume) - 0x02);
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_OVERTEMP_SETCUR, &LcdData.setData.OverTemp_LimitCurr, sizeof(LcdData.setData.OverTemp_LimitCurr) - 0x02);
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_GUNVOLT_LIMIT, &LcdData.setData.GunVolt_LimitValue, sizeof(LcdData.setData.GunVolt_LimitValue));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_ELOSS_PROPORTION, &LcdData.setData.ElossProprotion, sizeof(LcdData.setData.ElossProprotion));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

    LcdAssistantData.Flag.IsSetPowerPercent = TRUE;
    LcdAssistantData.Flag.IsSetELossProportion = TRUE;

    rt_kprintf("GunVolt_LimitValue Limit|%d   PowerPercent|%d  ElossProprotion|%d\n", LcdData.setData.GunVolt_LimitValue, LcdData.setData.PowerPercent, LcdData.setData.ElossProprotion);
    rt_kprintf("Stop_SOC|%d    OverTemp_Warnning|%d\n", LcdData.setData.Stop_SOC, LcdData.setData.OverTemp_Warnning);
    rt_kprintf("OverTemp_Stop|%d    OverTemp_Resume|%d\n", LcdData.setData.OverTemp_Stop, LcdData.setData.OverTemp_Resume);
    rt_kprintf("OverTemp_LimitCurr|%d    OverTemp_LimitCurr|%d\n", LcdData.setData.OverTemp_LimitCurr, LcdData.setData.OverTemp_LimitCurr);
}

void SerialScreen_ParaChargeSet(void)
{
    if(LcdAssistantData.Flag.IsEnableParaCharge == TRUE){
        if(LcdAssistantData.DeviceType != SYSTEM_FUNCTION_DYNAMIC_SWITCH){
            LcdAssistantData.Flag.ParaChargeSelect = TRUE;
        }
    }
}

void SerialScreen_SingleChargeSet(void)
{
    if(LcdAssistantData.Flag.IsEnableParaCharge  == TRUE){
        LcdAssistantData.Flag.ParaChargeSelect = FALSE;
    }
}

void SerialScreen_IsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########IsSupportSet = %d###########\r\n",LcdData.setData.sup_Local);
	if(LcdData.setData.sup_Local != TRUE)
		LcdData.setData.sup_Local = TRUE;
	else
		LcdData.setData.sup_Local = FALSE;
	sSCREEN_EVENT_DEBUGMSG("sup_Local=%d\r\n",LcdData.setData.sup_Local );
}

void SerialScreen_IsSupportPlugAndPlaySet(void)
{
    if(LcdData.setData.Icon_SupPlugAndPlay != TRUE)
        LcdData.setData.Icon_SupPlugAndPlay = TRUE;
    else
        LcdData.setData.Icon_SupPlugAndPlay = FALSE;
}

void SerialScreen_IsSupportLocalStopSet(void)
{
    if(LcdData.setData.Icon_SuplocalStop != TRUE)
        LcdData.setData.Icon_SuplocalStop = TRUE;
    else
        LcdData.setData.Icon_SuplocalStop = FALSE;
}

void SerialScreen_IsSupportReaderSet(void)
{
    if(LcdData.setData.sup_usecard != TRUE)
        LcdData.setData.sup_usecard = TRUE;
    else
        LcdData.setData.sup_usecard = FALSE;
}

void SerialScreen_IsSupportAuxp24VSet(void)
{
    if(LcdData.setData.sup_auxp_24V != TRUE)
        LcdData.setData.sup_auxp_24V = TRUE;
    else
        LcdData.setData.sup_auxp_24V = FALSE;
}

void SerialScreen_IsSupportParaChargeSet(void)
{
    if(LcdData.setData.sup_parallelchg != TRUE)
        LcdData.setData.sup_parallelchg = TRUE;
    else
        LcdData.setData.sup_parallelchg = FALSE;
}

void SerialScreen_IsSupportParaRelaySet(void)
{
    if(LcdData.setData.sup_parallelrelay != TRUE)
        LcdData.setData.sup_parallelrelay = TRUE;
    else
        LcdData.setData.sup_parallelrelay = FALSE;
}

void SerialScreen_IsSupportVINSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########IsSupportVINSet = %d###########\r\n",LcdData.setData.sup_VIN);
	if(LcdData.setData.sup_VIN != TRUE)
		LcdData.setData.sup_VIN = TRUE;
	else
		LcdData.setData.sup_VIN = FALSE;
	sSCREEN_EVENT_DEBUGMSG("sup_VIN=%d\r\n",LcdData.setData.sup_VIN );
}

void SerialScreen_IsSupportIsulationSet(void)
{
    if(LcdData.setData.sup_insulation != TRUE)
        LcdData.setData.sup_insulation = TRUE;
    else
        LcdData.setData.sup_insulation = FALSE;
}

void SerialScreen_IsSupportModuleSlienceSet(void)
{
    if(LcdData.setData.sup_mslience != TRUE)
        LcdData.setData.sup_mslience = TRUE;
    else
        LcdData.setData.sup_mslience = FALSE;
}

void SerialScreen_IsSupportOfflineBillingSet(void)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.Icon_SupOfflineBilling != TRUE)
        LcdData.setData.Icon_SupOfflineBilling = TRUE;
    else
        LcdData.setData.Icon_SupOfflineBilling = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
}

void SerialScreen_IsSupportPWStartSet(void)
{
    if(LcdData.setData.Icon_SupPWStart != TRUE)
        LcdData.setData.Icon_SupPWStart = TRUE;
    else
        LcdData.setData.Icon_SupPWStart = FALSE;
}

void SerialScreen_IsSupportOfflineCardSet(void)
{
    if(LcdData.setData.Icon_SupOffCard != TRUE)
        LcdData.setData.Icon_SupOffCard = TRUE;
    else
        LcdData.setData.Icon_SupOffCard = FALSE;
}

/********************************输入信息*******************************************/
void SerialScreen_ScramIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########ScramIsSupportSet = %d###########\r\n",LcdData.setData.supin_scram);
	if(LcdData.setData.supin_scram != TRUE)
		LcdData.setData.supin_scram = TRUE;
	else
		LcdData.setData.supin_scram = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_scram=%d\r\n",LcdData.setData.supin_scram );
}

void SerialScreen_ScramNegIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########ScramIsSupportSet = %d###########\r\n",LcdData.setData.neg_scram);
	if(LcdData.setData.neg_scram != TRUE)
		LcdData.setData.neg_scram = TRUE;
	else
		LcdData.setData.neg_scram = FALSE;
	sSCREEN_EVENT_DEBUGMSG("neg_scram=%d\r\n",LcdData.setData.neg_scram );
}

void SerialScreen_GateIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########GateIsSupportSet = %d###########\r\n",LcdData.setData.supin_gate);
	if(LcdData.setData.supin_gate != TRUE)
		LcdData.setData.supin_gate = TRUE;
	else
		LcdData.setData.supin_gate = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_gate=%d\r\n",LcdData.setData.supin_gate );
}

void SerialScreen_GateNegIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########GateNegIsSupportSet = %d###########\r\n",LcdData.setData.neg_gate);
	if(LcdData.setData.neg_gate != TRUE)
		LcdData.setData.neg_gate = TRUE;
	else
		LcdData.setData.neg_gate = FALSE;
	sSCREEN_EVENT_DEBUGMSG("neg_gate=%d\r\n",LcdData.setData.neg_gate );
}

void SerialScreen_DcIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########GateIsSupportSet = %d###########\r\n",LcdData.setData.supin_dc);
	if(LcdData.setData.supin_dc != TRUE)
		LcdData.setData.supin_dc = TRUE;
	else
		LcdData.setData.supin_dc = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_gate=%d\r\n",LcdData.setData.supin_dc );
}

void SerialScreen_DcNegIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########GateNegIsSupportSet = %d###########\r\n",LcdData.setData.neg_dc);
	if(LcdData.setData.neg_dc != TRUE)
		LcdData.setData.neg_dc = TRUE;
	else
		LcdData.setData.neg_dc = FALSE;
	sSCREEN_EVENT_DEBUGMSG("neg_gate=%d\r\n",LcdData.setData.neg_dc );
}


void SerialScreen_AcIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########AcIsSupportSet = %d###########\r\n",LcdData.setData.supin_ac);
	if(LcdData.setData.supin_ac != TRUE)
		LcdData.setData.supin_ac = TRUE;
	else
		LcdData.setData.supin_ac = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_ac=%d\r\n",LcdData.setData.supin_ac );
}

void SerialScreen_AcNegIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########AcNegIsSupportSet = %d###########\r\n",LcdData.setData.neg_ac);
	if(LcdData.setData.neg_ac != TRUE)
		LcdData.setData.neg_ac = TRUE;
	else
		LcdData.setData.neg_ac = FALSE;
	sSCREEN_EVENT_DEBUGMSG("neg_ac=%d\r\n",LcdData.setData.neg_ac );
}

void SerialScreen_FanIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########FanIsSupportSet = %d###########\r\n",LcdData.setData.supin_fan);
	if(LcdData.setData.supin_fan != TRUE)
		LcdData.setData.supin_fan = TRUE;
	else
		LcdData.setData.supin_fan = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_fan=%d\r\n",LcdData.setData.supin_fan );
}

void SerialScreen_FanNegIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########FanNegIsSupportSet = %d###########\r\n",LcdData.setData.neg_fan);
	if(LcdData.setData.neg_fan != TRUE)
		LcdData.setData.neg_fan = TRUE;
	else
		LcdData.setData.neg_fan = FALSE;
	sSCREEN_EVENT_DEBUGMSG("neg_fan=%d\r\n",LcdData.setData.neg_fan );
}

void SerialScreen_ElockIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########ElockIsSupportSet = %d###########\r\n",LcdData.setData.supin_elock);
	if(LcdData.setData.supin_elock != TRUE)
		LcdData.setData.supin_elock = TRUE;
	else
		LcdData.setData.supin_elock = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_elock=%d\r\n",LcdData.setData.supin_elock );
}

void SerialScreen_TempProIsSupportSet(void)
{
    if(LcdData.setData.supin_temp_pro != TRUE)
        LcdData.setData.supin_temp_pro = TRUE;
    else
        LcdData.setData.supin_temp_pro = FALSE;
}

void SerialScreen_ElockNegIsSupportSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########ElockNegIsSupportSet = %d###########\r\n",LcdData.setData.neg_elcok);
	if(LcdData.setData.neg_elcok != TRUE)
		LcdData.setData.neg_elcok = TRUE;
	else
		LcdData.setData.neg_elcok = FALSE;
	sSCREEN_EVENT_DEBUGMSG("neg_elcok=%d\r\n",LcdData.setData.neg_elcok );
}

void SerialScreen_ProLightIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupProtectLight != TRUE)
        LcdData.setData.Icon_SupProtectLight = TRUE;
    else
        LcdData.setData.Icon_SupProtectLight = FALSE;
}

void SerialScreen_ProLightNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegProtectLight != TRUE)
        LcdData.setData.Icon_NegProtectLight = TRUE;
    else
        LcdData.setData.Icon_NegProtectLight = FALSE;
}

void SerialScreen_GunSiteIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupGunSite != TRUE)
        LcdData.setData.Icon_SupGunSite = TRUE;
    else
        LcdData.setData.Icon_SupGunSite = FALSE;
}

void SerialScreen_GunSiteNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegGunSite != TRUE)
        LcdData.setData.Icon_NegGunSite = TRUE;
    else
        LcdData.setData.Icon_NegGunSite = FALSE;
}

void SerialScreen_BreakerIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupCircuitBreaker != TRUE)
        LcdData.setData.Icon_SupCircuitBreaker = TRUE;
    else
        LcdData.setData.Icon_SupCircuitBreaker = FALSE;
}

void SerialScreen_BreakerNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegCircuitBreaker != TRUE)
        LcdData.setData.Icon_NegCircuitBreaker = TRUE;
    else
        LcdData.setData.Icon_NegCircuitBreaker = FALSE;
}

void SerialScreen_FloodIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupFlood != TRUE)
        LcdData.setData.Icon_SupFlood = TRUE;
    else
        LcdData.setData.Icon_SupFlood = FALSE;
}

void SerialScreen_FloodNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegFlood != TRUE)
        LcdData.setData.Icon_NegFlood = TRUE;
    else
        LcdData.setData.Icon_NegFlood = FALSE;
}

void SerialScreen_SmokeIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupSmoke != TRUE)
        LcdData.setData.Icon_SupSmoke = TRUE;
    else
        LcdData.setData.Icon_SupSmoke = FALSE;
}

void SerialScreen_SmokeNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegSmoke != TRUE)
        LcdData.setData.Icon_NegSmoke = TRUE;
    else
        LcdData.setData.Icon_NegSmoke = FALSE;
}

void SerialScreen_PourIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupPour != TRUE)
        LcdData.setData.Icon_SupPour = TRUE;
    else
        LcdData.setData.Icon_SupPour = FALSE;
}

void SerialScreen_PourNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegPour != TRUE)
        LcdData.setData.Icon_NegPour = TRUE;
    else
        LcdData.setData.Icon_NegPour = FALSE;
}

void SerialScreen_LiquidIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupLiquid != TRUE)
        LcdData.setData.Icon_SupLiquid = TRUE;
    else
        LcdData.setData.Icon_SupLiquid = FALSE;
}

void SerialScreen_LiquidNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegLiquid != TRUE)
        LcdData.setData.Icon_NegLiquid = TRUE;
    else
        LcdData.setData.Icon_NegLiquid = FALSE;
}

void SerialScreen_FuseIsSupportSet(void)
{
    if(LcdData.setData.Icon_SupFuse != TRUE)
        LcdData.setData.Icon_SupFuse = TRUE;
    else
        LcdData.setData.Icon_SupFuse = FALSE;
}

void SerialScreen_FuseNegIsSupportSet(void)
{
    if(LcdData.setData.Icon_NegFuse != TRUE)
        LcdData.setData.Icon_NegFuse = TRUE;
    else
        LcdData.setData.Icon_NegFuse = FALSE;
}
/********************************输出信息*******************************************/

void SerialScreen_AcIsSupportOutSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########AcIsSupportOutSet = %d###########\r\n",LcdData.setData.supout_ac);	
	if(LcdData.setData.supout_ac != TRUE)
		LcdData.setData.supout_ac = TRUE;
	else
		LcdData.setData.supout_ac = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supout_ac=%d\r\n",LcdData.setData.supout_ac );	
}

void SerialScreen_ElockIsSupportOutSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########ElockIsSupportOutSet = %d###########\r\n",LcdData.setData.supout_elock);	
	if(LcdData.setData.supout_elock != TRUE)
		LcdData.setData.supout_elock = TRUE;
	else
		LcdData.setData.supout_elock = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_elock=%d\r\n",LcdData.setData.supout_elock );	
}

void SerialScreen_FanIsSupportOutSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########FanIsSupportOutSet = %d###########\r\n",LcdData.setData.supout_fan);	
	if(LcdData.setData.supout_fan != TRUE)
		LcdData.setData.supout_fan = TRUE;
	else
		LcdData.setData.supout_fan = FALSE;
	sSCREEN_EVENT_DEBUGMSG("supin_elock=%d\r\n",LcdData.setData.supout_fan );	
}

static void SerialScreen_OfflineBillingInfoJudge(u32 *ret)
{
#define SSCREEN_OB_CHARGING_POSITION                        0   /* 离线计费配置失败原因：在充电  */
#define SSCREEN_OB_FORMAT_POSITION                          1   /* 离线计费配置失败原因：格式错误  */
#define SSCREEN_OB_NOT_CONTINOUS_POSITION                   2   /* 离线计费配置失败原因：不连续  */
#define SSCREEN_OB_DUPLICATE_POSITION                       3   /* 离线计费配置失败原因：重复  */

    u32 result = 0;
#ifdef SCREEN_USING_OFFLINE_BILLING
    struct Period_Time time[CP_RATED_TYPE_NUM_MAX *CP_RATED_TYPE_PERIOD_NUM];  /* 时段时间 */
    u8 i = 0x00, j = 0x00, valid_count = 0;
    s8 res = 0;

    for(i = 0x00; i < LCD_GUN_NUM; i++){
        if((LcdData.gun[i].workState >= SysMainStatus_StartReady) && (LcdData.gun[i].workState <= SysMainStatus_StopChg)){
            result |= (1 <<SSCREEN_OB_CHARGING_POSITION);
            rt_kprintf("is starting or charging, not allow to modify offline billing rule\n");
            break;
        }
    }

    if((res = sys_period_time_format_valid(LcdData.setData.PeriodTime)) != 0x01){
        result |= (1 <<SSCREEN_OB_FORMAT_POSITION);
        rt_kprintf("offline billing period time format error\n");
    }else{
        memset(time, 0x00, sizeof(time));
        for(i = 0x00; i < CP_RATED_TYPE_NUM_MAX; i++){
            for(j = 0x00; j < CP_RATED_TYPE_PERIOD_NUM; j++){
                if(LcdData.setData.PeriodTime[i][j].shour < 24){
                    time[valid_count] = LcdData.setData.PeriodTime[i][j];
                    valid_count++;
                }
            }
        }
        if((res = sys_period_time_continuous_valid(time, sizeof(time), valid_count)) != 0x01){
            if(res == 0){
                result |= (1 <<SSCREEN_OB_NOT_CONTINOUS_POSITION);
                rt_kprintf("offline billing time not continous\n");
            }else{
                result |= (1 <<SSCREEN_OB_DUPLICATE_POSITION);
                rt_kprintf("offline billing time duplicate\n");
            }
        }
    }
#else
    result |= (1 <<SSCREEN_OB_FORMAT_POSITION);
    rt_kprintf("system not support offline billing mode\n");
#endif /* SCREEN_USING_OFFLINE_BILLING */
    if(ret){
        *ret = result;
    }

#undef SSCREEN_OB_CHARGING_POSITION
#undef SSCREEN_OB_FORMAT_POSITION
#undef SSCREEN_OB_NOT_CONTINOUS_POSITION
#undef SSCREEN_OB_DUPLICATE_POSITION
}

void SerialScreen_OfflineBillingSet(void)
{
#define SSCREEN_OB_CHARGING_POSITION                        0   /* 离线计费配置失败原因：在充电  */
#define SSCREEN_OB_FORMAT_POSITION                          1   /* 离线计费配置失败原因：格式错误  */
#define SSCREEN_OB_NOT_CONTINOUS_POSITION                   2   /* 离线计费配置失败原因：不连续  */
#define SSCREEN_OB_DUPLICATE_POSITION                       3   /* 离线计费配置失败原因：重复  */

#ifdef SCREEN_USING_OFFLINE_BILLING
    struct sys_billing_rule *rule = (struct sys_billing_rule*)sys_read_config_item_content(CONFIG_ITEM_BILLING_RULE, 0);
    u8 i = 0x00;
    u32 ret = 0;

    if(LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify == FALSE){
        SerialScreen_OfflineBillingInfoJudge(&ret);
        if(ret &(1 <<SSCREEN_OB_CHARGING_POSITION)){
            /* 弹出提示：启动或充电中不允许修改 */
            LcdData.setData.OBwarning = ICON_OB_IS_CHARGING;
            /* 此时不进行切页 */
            if(LcdData.CurrentPage != LcdData.CurrentPageBack){
                LcdData.CurrentPage = LcdData.CurrentPageBack;
            }
            return;
        }else if(ret &(1 <<SSCREEN_OB_FORMAT_POSITION)){
            /* 弹出提示：设置的时间段不连续 */
            LcdData.setData.OBwarning = ICON_OB_NOT_CONTINUOUS;
            /* 此时不进行切页 */
            if(LcdData.CurrentPage != LcdData.CurrentPageBack){
                LcdData.CurrentPage = LcdData.CurrentPageBack;
            }
            return;
        }else if(ret &(1 <<SSCREEN_OB_NOT_CONTINOUS_POSITION)){
            /* 弹出提示：设置的时间段不连续 */
            LcdData.setData.OBwarning = ICON_OB_NOT_CONTINUOUS;
            /* 此时不进行切页 */
            if(LcdData.CurrentPage != LcdData.CurrentPageBack){
                LcdData.CurrentPage = LcdData.CurrentPageBack;
            }
            return;
        }else if(ret &(1 <<SSCREEN_OB_DUPLICATE_POSITION)){
            /* 弹出提示：设置的时间段重复 */
            LcdData.setData.OBwarning = ICON_OB_TIME_DUPLICATE;
            /* 此时不进行切页 */
            if(LcdData.CurrentPage != LcdData.CurrentPageBack){
                LcdData.CurrentPage = LcdData.CurrentPageBack;
            }
            return;
        }
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = FALSE;

    LcdData.setData.OBwarning = ICON_OB_NULL;
    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if((LcdData.setData.ServicePrice < CP_VALLEY_RATED_SERVICE_PRICE_MIN) || (LcdData.setData.ServicePrice > CP_VALLEY_RATED_SERVICE_PRICE_MAX)){
        LcdData.setData.ServicePrice = CP_VALLEY_RATED_SERVICE_PRICE_DEF;
    }
    if((LcdData.setData.SsElectPrice < CP_SHARP_SHARP_RATED_ELECT_PRICE_MIN) || (LcdData.setData.SsElectPrice > CP_SHARP_SHARP_RATED_ELECT_PRICE_MAX)){
        LcdData.setData.SsElectPrice = CP_SHARP_SHARP_RATED_ELECT_PRICE_DEF;
    }
    if((LcdData.setData.SElectPrice < CP_SHARP_RATED_ELECT_PRICE_MIN) || (LcdData.setData.SElectPrice > CP_SHARP_RATED_ELECT_PRICE_MAX)){
        LcdData.setData.SElectPrice = CP_SHARP_RATED_ELECT_PRICE_DEF;
    }
    if((LcdData.setData.PElectPrice < CP_PEAK_RATED_ELECT_PRICE_MIN) || (LcdData.setData.PElectPrice > CP_PEAK_RATED_ELECT_PRICE_MAX)){
        LcdData.setData.PElectPrice = CP_PEAK_RATED_ELECT_PRICE_DEF;
    }
    if((LcdData.setData.FElectPrice < CP_FLAT_RATED_ELECT_PRICE_MIN) || (LcdData.setData.FElectPrice > CP_FLAT_RATED_ELECT_PRICE_MAX)){
        LcdData.setData.FElectPrice = CP_FLAT_RATED_ELECT_PRICE_DEF;
    }
    if((LcdData.setData.VElectPrice < CP_VALLEY_RATED_ELECT_PRICE_MIN) || (LcdData.setData.VElectPrice > CP_VALLEY_RATED_ELECT_PRICE_MAX)){
        LcdData.setData.VElectPrice = CP_VALLEY_RATED_ELECT_PRICE_DEF;
    }

    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][i].rate_number = CP_RATED_TYPE_SHARP_SHARP;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][i].rate_number = CP_RATED_TYPE_SHARP;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][i].rate_number = CP_RATED_TYPE_PEAK;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][i].rate_number = CP_RATED_TYPE_FLAT;
    }
    for(i = 0x00; i < CP_RATED_TYPE_PERIOD_NUM; i++){
        LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][i].rate_number = CP_RATED_TYPE_VALLEY;
    }

    rule->rate_service_price[CP_RATED_TYPE_SHARP_SHARP] = LcdData.setData.ServicePrice;
    rule->rate_service_price[CP_RATED_TYPE_SHARP] = LcdData.setData.ServicePrice;
    rule->rate_service_price[CP_RATED_TYPE_PEAK] = LcdData.setData.ServicePrice;
    rule->rate_service_price[CP_RATED_TYPE_FLAT] = LcdData.setData.ServicePrice;
    rule->rate_service_price[CP_RATED_TYPE_VALLEY] = LcdData.setData.ServicePrice;

    rule->rate_elect_price[CP_RATED_TYPE_SHARP_SHARP] = LcdData.setData.SsElectPrice;
    rule->rate_elect_price[CP_RATED_TYPE_SHARP] = LcdData.setData.SElectPrice;
    rule->rate_elect_price[CP_RATED_TYPE_PEAK] = LcdData.setData.PElectPrice;
    rule->rate_elect_price[CP_RATED_TYPE_FLAT] = LcdData.setData.FElectPrice;
    rule->rate_elect_price[CP_RATED_TYPE_VALLEY] = LcdData.setData.VElectPrice;

    memcpy(rule->time, LcdData.setData.PeriodTime, sizeof(LcdData.setData.PeriodTime));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */

#undef SSCREEN_OB_CHARGING_POSITION
#undef SSCREEN_OB_FORMAT_POSITION
#undef SSCREEN_OB_NOT_CONTINOUS_POSITION
#undef SSCREEN_OB_DUPLICATE_POSITION
}

void SerialScreen_OfflineBillingGet(void)
{
#ifdef SCREEN_USING_OFFLINE_BILLING
    struct sys_billing_rule *rule = (struct sys_billing_rule*)sys_read_config_item_content(CONFIG_ITEM_BILLING_RULE, 0);

    LcdData.setData.OBwarning = ICON_OB_NULL;

    LcdData.setData.ServicePrice = rule->rate_service_price[CP_PERIOD_RATED_NUMBER_DEFAULT];
    LcdData.setData.SsElectPrice = rule->rate_elect_price[CP_RATED_TYPE_SHARP_SHARP];
    LcdData.setData.SElectPrice = rule->rate_elect_price[CP_RATED_TYPE_SHARP];
    LcdData.setData.PElectPrice = rule->rate_elect_price[CP_RATED_TYPE_PEAK];
    LcdData.setData.FElectPrice = rule->rate_elect_price[CP_RATED_TYPE_FLAT];
    LcdData.setData.VElectPrice = rule->rate_elect_price[CP_RATED_TYPE_VALLEY];

    rt_kprintf("SerialScreen_OfflineBillingGet(%d, %d, %d, %d, %d)\n", LcdData.setData.ServicePrice,
            LcdData.setData.SsElectPrice, LcdData.setData.SElectPrice, LcdData.setData.PElectPrice,
            LcdData.setData.PElectPrice, LcdData.setData.FElectPrice, LcdData.setData.VElectPrice);

    memcpy(LcdData.setData.PeriodTime, rule->time, sizeof(LcdData.setData.PeriodTime));

    rt_kprintf("sharp sharp 00[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].emin);

    rt_kprintf("sharp sharp 11[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].emin);

    rt_kprintf("sharp 00[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].emin);

    rt_kprintf("sharp 11[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].emin);

    rt_kprintf("peak 00[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].emin);

    rt_kprintf("peak 11[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].emin);

    rt_kprintf("flat 00[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].emin);

    rt_kprintf("flat 11[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].emin);

    rt_kprintf("valley 00[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].emin);

    rt_kprintf("valley 11[%d:%d-%d:%d]\n", LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].shour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].smin,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].ehour,
            LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].emin);
#endif /* SCREEN_USING_OFFLINE_BILLING */
}

static void SerialScreen_IsSupportInfoJudge(u32 *ret)
{
//@ thaisen_cfg_info_function
#define SSCREEN_PLUG_AND_PLAY_POSITION                        6   /* 即插即充功能在结构体 thaisen_cfg_info_function 中的成员次序(从0开始)  */
    u32 result = 0;

#ifndef SCREEN_USING_OFFLINE_BILLING
    LcdData.setData.Icon_SupOfflineBilling = FALSE;
    LcdData.setData.sup_offbilling = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */

    LcdAssistantData.Flag.NeedReboot = FALSE;

    if(LcdData.setData.Icon_SupOfflineBilling != LcdData.setData.sup_offbilling){
        if(LcdData.setData.Icon_SupOfflineBilling){
            if(LcdData.setData.Icon_SupPlugAndPlay){
                result |= (1 <<SSCREEN_PLUG_AND_PLAY_POSITION);
            }
            LcdData.setData.Icon_SupPlugAndPlay = FALSE;
        }
        LcdAssistantData.Flag.NeedReboot = TRUE;
    }

    if(LcdData.setData.Icon_SupOfflineBilling == FALSE){
        if(LcdData.setData.Icon_SupPlugAndPlay != LcdData.setData.Sup_PlugAndPlay){
            if(LcdData.setData.Icon_SupPlugAndPlay){
                LcdData.setData.Icon_SupOfflineBilling = FALSE;
            }
            LcdAssistantData.Flag.NeedReboot = TRUE;
        }
    }else{
        if(LcdData.setData.Icon_SupPlugAndPlay){
            result |= (1 <<SSCREEN_PLUG_AND_PLAY_POSITION);
        }
        LcdData.setData.Icon_SupPlugAndPlay = FALSE;
    }

    LcdData.setData.sup_Local_stop = LcdData.setData.Icon_SuplocalStop;
    LcdData.setData.sup_offbilling = LcdData.setData.Icon_SupOfflineBilling;
    LcdData.setData.Sup_PlugAndPlay = LcdData.setData.Icon_SupPlugAndPlay;
    LcdData.setData.sup_pw_start = LcdData.setData.Icon_SupPWStart;
    LcdData.setData.sup_offline_card = LcdData.setData.Icon_SupOffCard;

    if(ret){
        *ret = result;
    }

#undef SSCREEN_PLUG_AND_PLAY_POSITION
}
void SerialScreen_IsSupportSetFlash(void)
{
    u8 function_disable = TRUE, len = 0, count = 0;

    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

    if(LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify == FALSE){
        SerialScreen_IsSupportInfoJudge(NULL);
    }
    LcdAssistantData.SeveralGunFlag[LCD_GUN_1].DataIsVerify = FALSE;

    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_LOCAL, (u8 *)&(LcdData.setData.sup_Local), sizeof(LcdData.setData.sup_Local));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PLUGCHARGE, (u8 *)&(LcdData.setData.Sup_PlugAndPlay), sizeof(LcdData.setData.Sup_PlugAndPlay));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_LOCAL_STOP, (u8 *)&(LcdData.setData.sup_Local_stop), sizeof(LcdData.setData.sup_Local_stop));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_VIN, (u8 *)&(LcdData.setData.sup_VIN), sizeof(LcdData.setData.sup_VIN));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_INSULATION, (u8 *)&(LcdData.setData.sup_insulation), sizeof(LcdData.setData.sup_insulation));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_CARD, (u8 *)&(LcdData.setData.sup_usecard), sizeof(LcdData.setData.sup_usecard));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_AUXPOWER24V, (u8 *)&(LcdData.setData.sup_auxp_24V), sizeof(LcdData.setData.sup_auxp_24V));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PARALLEL, (u8 *)&(LcdData.setData.sup_parallelchg), sizeof(LcdData.setData.sup_parallelchg));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PARALLELRELAY, (u8 *)&(LcdData.setData.sup_parallelrelay), sizeof(LcdData.setData.sup_parallelrelay));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_MODULE_SLIENCE, (u8 *)&(LcdData.setData.sup_mslience), sizeof(LcdData.setData.sup_mslience));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, (u8 *)&(LcdData.setData.sup_offbilling), sizeof(LcdData.setData.sup_offbilling));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PASSWORD_START, (u8 *)&(LcdData.setData.sup_pw_start), sizeof(LcdData.setData.sup_pw_start));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_OFFLINE_CARD, (u8 *)&(LcdData.setData.Icon_SupOffCard), sizeof(LcdData.setData.Icon_SupOffCard));

    for(u8 i = 0; i < sizeof(LcdData.setData.UserPasswdShow); i++){
        if((LcdData.setData.UserPasswdShow[i] < 0x20) ||  \
                (LcdData.setData.UserPasswdShow[i] > 0x7E) ||  \
                (LcdData.setData.UserPasswdShow[i] == 0x00)){
            len = i;
            break;
        }
    }
    for(u8 i = 0; i < len; i++){
        if(LcdData.setData.UserPasswdShow[i] == '*'){
            count++;
        }
    }
    if(count != len){
        memcpy(LcdData.setData.UserPasswd, LcdData.setData.UserPasswdShow, sizeof(LcdData.setData.UserPasswdShow));
    }

    UI_SYNC_SINGLE_CFG_STR(CONFIG_ITEM_SCREEN_PASSWORD, (u8 *)(LcdData.setData.UserPasswd), len);

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

    if(LcdData.setData.sup_insulation == FALSE){
        function_disable = TRUE;
    }else{
        function_disable = FALSE;
    }
    thaisen_set_insult_mode(function_disable, FALSE);
    thaisen_set_insult_mode(function_disable, TRUE);

    if(LcdData.setData.sup_mslience == FALSE){
        thaisenSetYouYouSlienceMode(FALSE);
    }else{
        thaisenSetYouYouSlienceMode(TRUE);
    }

    if(LcdData.setData.sup_parallelrelay == FALSE){
        thaisenModuleSetParallelEnable(thaisenFunction_disable);
    }else{
        thaisenModuleSetParallelEnable(thaisenFunction_enable);
    }

    LcdAssistantData.Flag.IsEnableParaCharge = LcdData.setData.sup_parallelchg;
    LcdAssistantData.Flag.IsEnableAuxPower24V = LcdData.setData.sup_auxp_24V;

    for(u8 i = 0; i < LCD_GUN_NUM; i++)
        LcdAssistantData.SeveralGunFlag[i].AuxPower24VSelect = LcdData.setData.sup_auxp_24V;

    if(LcdAssistantData.Flag.NeedReboot == TRUE){
        SerialScreen_ScreenSet_Reboot_Flag();
    }
}

void SerialScreen_SetInputInfo(void)
{
	if(FALSE == LcdData.setData.supin_scram)
    	thaisenClearSysFaultCheckBit(thaisenFaultScram);
	else 
		thaisenSetSysFaultCheckBit(thaisenFaultScram);
	if(FALSE == LcdData.setData.supin_gate)
    	thaisenClearSysFaultCheckBit(thaisenDoor);
	else 
		thaisenSetSysFaultCheckBit(thaisenDoor); 

    if(FALSE == LcdData.setData.supin_ac){
#ifdef SCREEN_USING_DOUBLE_GUN
        thaisenClearSysFaultCheckBit(thaisenRelayAc);
#else
        thaisenSetACRelayEnableState(0);
#endif /* SCREEN_USING_DOUBLE_GUN */
    }else{
#ifdef SCREEN_USING_DOUBLE_GUN
        thaisenSetSysFaultCheckBit(thaisenRelayAc);
#else
        thaisenSetACRelayEnableState(1);
#endif /* SCREEN_USING_DOUBLE_GUN */
    }
    if(FALSE == LcdData.setData.supin_dc){
        thaisenClearSysFaultCheckBit(thaisenRelay);
        thaisenClearSysFaultCheckBit(thaisenRelayParallel);
    }else{
        thaisenSetSysFaultCheckBit(thaisenRelay);
        thaisenSetSysFaultCheckBit(thaisenRelayParallel);
    }

	if(FALSE == LcdData.setData.supin_elock)
        thaisenClearSysFaultCheckBit(thaisenElock);
	else
        thaisenSetSysFaultCheckBit(thaisenElock);

    if(FALSE == LcdData.setData.supin_protectlight)                           //防雷器
        thaisenClearSysFaultCheckBit(thaisenFaultLightProtect);
    else
        thaisenSetSysFaultCheckBit(thaisenFaultLightProtect);

    if(FALSE == LcdData.setData.supin_gunsite)                                //枪座
        thaisenClearSysFaultCheckBit(thaisenFaultGunSite);
    else
        thaisenSetSysFaultCheckBit(thaisenFaultGunSite);

//    if(FALSE == LcdData.setData.supin_circuit_breaker)                        //断路器
//        thaisenClearSysFaultCheckBit(thaisenFaultCircuitBreaker);
//    else
//        thaisenSetSysFaultCheckBit(thaisenFaultCircuitBreaker);

    if(FALSE == LcdData.setData.supin_flood)                                  //水浸
        thaisenClearSysFaultCheckBit(thaisenFaultFlooding);
    else
        thaisenSetSysFaultCheckBit(thaisenFaultFlooding);

    if(FALSE == LcdData.setData.supin_smoke)                                  //烟感
        thaisenClearSysFaultCheckBit(thaisenFaultSmoke);
    else
        thaisenSetSysFaultCheckBit(thaisenFaultSmoke);

    if(FALSE == LcdData.setData.supin_pour)                                   //倾倒
        thaisenClearSysFaultCheckBit(thaisenFaultPour);
    else
        thaisenSetSysFaultCheckBit(thaisenFaultPour);

//    if(FALSE == LcdData.setData.supin_liquid)                                 //液冷
//        thaisenClearSysFaultCheckBit(thaisenFaultLiquidCooling);
//    else
//        thaisenSetSysFaultCheckBit(thaisenFaultLiquidCooling);
//
//    if(FALSE == LcdData.setData.supin_fuse)                                    //熔断器
//        thaisenClearSysFaultCheckBit(thaisenFaultFuse);
//    else
//        thaisenSetSysFaultCheckBit(thaisenFaultFuse);

	thaisenSetScramPressStatua(LcdData.setData.neg_scram);
//	thaisenSetDoorPressStatua(LcdData.setData.neg_gate);
	thaisenSetDoorOpendStatua(LcdData.setData.neg_gate);
    thaisenSetElectLockFeedbackSta(LcdData.setData.neg_elcok);
	thaisenSetElectLockBFeedbackSta(LcdData.setData.neg_elcok);

	if(LcdData.setData.neg_protectlight)
	    thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortLightningProtection, thaisenGeneralInPortAbnormalHigh);
	else
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortLightningProtection, thaisenGeneralInPortAbnormalLow);

    if(LcdData.setData.neg_gunsite){
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortGunSit_A, thaisenGeneralInPortAbnormalHigh);
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortGunSit_B, thaisenGeneralInPortAbnormalHigh);
    }else{
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortGunSit_A, thaisenGeneralInPortAbnormalLow);
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortGunSit_B, thaisenGeneralInPortAbnormalLow);
    }

//    if(LcdData.setData.neg_circuit_breaker)
//        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortCircuitBreaker, thaisenGeneralInPortAbnormalHigh);
//    else
//        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortCircuitBreaker, thaisenGeneralInPortAbnormalLow);

    if(LcdData.setData.neg_flood)
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortFlooding, thaisenGeneralInPortAbnormalHigh);
    else
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortFlooding, thaisenGeneralInPortAbnormalLow);

    if(LcdData.setData.neg_smoke)
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortSmoke, thaisenGeneralInPortAbnormalHigh);
    else
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortSmoke, thaisenGeneralInPortAbnormalLow);

    if(LcdData.setData.neg_pour)
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortPour, thaisenGeneralInPortAbnormalHigh);
    else
        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortPour, thaisenGeneralInPortAbnormalLow);

//    if(LcdData.setData.neg_liquid)
//        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortLiquid, thaisenGeneralInPortAbnormalHigh);
//    else
//        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortLiquid, thaisenGeneralInPortAbnormalLow);
//
//    if(LcdData.setData.neg_fuse)
//        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortFuse, thaisenGeneralInPortAbnormalHigh);
//    else
//        thaisenSetGeneralInPortAbnormalSta(thaisenGeneralInPortFuse, thaisenGeneralInPortAbnormalLow);
}

void SerialScreen_InputSetFlash(void)
{
	sSCREEN_EVENT_DEBUGMSG("set Flash supin_scram[%d] neg_scram[%d] supin_gate[%d] neg_gate[%d] supin_ac[%d]\r\n neg_ac[%d] supin_dc[%d] neg_dc[%d] supin_fan[%d] neg_fan[%d] supin_elock[%d] neg_elcok[%d]\r\n",
		LcdData.setData.supin_scram,LcdData.setData.neg_scram,\
		LcdData.setData.supin_gate,LcdData.setData.neg_gate,LcdData.setData.supin_ac,LcdData.setData.neg_ac,LcdData.setData.supin_dc,LcdData.setData.neg_dc,\
		LcdData.setData.supin_fan,LcdData.setData.neg_fan,LcdData.setData.supin_elock,LcdData.setData.neg_elcok);

	LcdData.setData.supin_protectlight = LcdData.setData.Icon_SupProtectLight;
	LcdData.setData.neg_protectlight = LcdData.setData.Icon_NegProtectLight;

    LcdData.setData.supin_gunsite = LcdData.setData.Icon_SupGunSite;
    LcdData.setData.neg_gunsite = LcdData.setData.Icon_NegGunSite;

//    LcdData.setData.supin_circuit_breaker = LcdData.setData.Icon_SupCircuitBreaker;
//    LcdData.setData.neg_circuit_breaker = LcdData.setData.Icon_NegCircuitBreaker;

    LcdData.setData.supin_flood = LcdData.setData.Icon_SupFlood;
    LcdData.setData.neg_flood = LcdData.setData.Icon_NegFlood;

    LcdData.setData.supin_smoke = LcdData.setData.Icon_SupSmoke;
    LcdData.setData.neg_smoke = LcdData.setData.Icon_NegSmoke;

    LcdData.setData.supin_pour = LcdData.setData.Icon_SupPour;
    LcdData.setData.neg_pour = LcdData.setData.Icon_NegPour;

//    LcdData.setData.supin_liquid = LcdData.setData.Icon_SupLiquid;
//    LcdData.setData.neg_liquid = LcdData.setData.Icon_NegLiquid;

//    LcdData.setData.supin_fuse = LcdData.setData.Icon_SupFuse;
//    LcdData.setData.neg_fuse = LcdData.setData.Icon_NegFuse;

    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_STORAGE_WAITING);

	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_SCRAM, (u8 *)&(LcdData.setData.supin_scram), sizeof(LcdData.setData.supin_scram));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_SCRAM, (u8 *)&(LcdData.setData.neg_scram), sizeof(LcdData.setData.neg_scram));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_GATE, (u8 *)&(LcdData.setData.supin_gate), sizeof(LcdData.setData.supin_gate));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_GATE, (u8 *)&(LcdData.setData.neg_gate), sizeof(LcdData.setData.neg_gate));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_ACRELAY, (u8 *)&(LcdData.setData.supin_ac), sizeof(LcdData.setData.supin_ac));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_ACRELAY, (u8 *)&(LcdData.setData.neg_ac), sizeof(LcdData.setData.neg_ac));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_DCRELAY, (u8 *)&(LcdData.setData.supin_dc), sizeof(LcdData.setData.supin_dc));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_DCRELAY, (u8 *)&(LcdData.setData.neg_dc), sizeof(LcdData.setData.neg_dc));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_FAN, (u8 *)&(LcdData.setData.supin_fan), sizeof(LcdData.setData.supin_fan));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_FAN, (u8 *)&(LcdData.setData.neg_fan), sizeof(LcdData.setData.neg_fan));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_ELOCK, (u8 *)&(LcdData.setData.supin_elock), sizeof(LcdData.setData.supin_elock));
	UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_ELOCK, (u8 *)&(LcdData.setData.neg_elcok), sizeof(LcdData.setData.neg_elcok));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_TEMPPRO, (u8 *)&(LcdData.setData.supin_temp_pro), sizeof(LcdData.setData.supin_temp_pro));

    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_PROTECT_LIGHT, (u8 *)&(LcdData.setData.supin_protectlight), sizeof(LcdData.setData.supin_protectlight));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_PROTECT_LIGHT, (u8 *)&(LcdData.setData.neg_protectlight), sizeof(LcdData.setData.neg_protectlight));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_GUNSITE, (u8 *)&(LcdData.setData.supin_gunsite), sizeof(LcdData.setData.supin_gunsite));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_GUNSITE, (u8 *)&(LcdData.setData.neg_gunsite), sizeof(LcdData.setData.neg_gunsite));
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_CIRCUIT_BREAKER, (u8 *)&(LcdData.setData.supin_circuit_breaker), sizeof(LcdData.setData.supin_circuit_breaker));
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_CIRCUIT_BREAKER, (u8 *)&(LcdData.setData.neg_circuit_breaker), sizeof(LcdData.setData.neg_circuit_breaker));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_FLOOD, (u8 *)&(LcdData.setData.supin_flood), sizeof(LcdData.setData.supin_flood));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_FLOOD, (u8 *)&(LcdData.setData.neg_flood), sizeof(LcdData.setData.neg_flood));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_SMOKE, (u8 *)&(LcdData.setData.supin_smoke), sizeof(LcdData.setData.supin_smoke));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_SMOKE, (u8 *)&(LcdData.setData.neg_smoke), sizeof(LcdData.setData.neg_smoke));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_POUR, (u8 *)&(LcdData.setData.supin_pour), sizeof(LcdData.setData.supin_pour));
    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_POUR, (u8 *)&(LcdData.setData.neg_pour), sizeof(LcdData.setData.neg_pour));
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_LIQUID, (u8 *)&(LcdData.setData.supin_liquid), sizeof(LcdData.setData.supin_liquid));
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_LIQUID, (u8 *)&(LcdData.setData.neg_liquid), sizeof(LcdData.setData.neg_liquid));
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_FUSE, (u8 *)&(LcdData.setData.supin_fuse), sizeof(LcdData.setData.supin_fuse));
//    UI_SYNC_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_FUSE, (u8 *)&(LcdData.setData.neg_fuse), sizeof(LcdData.setData.neg_fuse));

    LcdAssistantData.Flag.IsConfigFail = TRUE;
    if(UI_STORAGE_CFG_DATA >= 0){
        LcdAssistantData.Flag.IsConfigFail = FALSE;
    }

	SerialScreen_SetInputInfo();
}

void SerialScreen_InitInfo_Pro(void)
{
    SerialScreen_SendIco(&SerialScreen, 0x1103, 0x01);     /* 上电厂商LOGO先隐藏 */
}

void SerialScreen_IsSupportGet(void)
{
    u8 function_disable = 1;
	sSCREEN_EVENT_DEBUGMSG("##########IsSupportGet###########\r\n");

	if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_LOCAL, 0)))            /* 本地启动默认不启用 */
		LcdData.setData.sup_Local = FALSE;
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PLUGCHARGE, 0)))       /* 即插即充默认不启用 */
        LcdData.setData.Sup_PlugAndPlay = FALSE;
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_LOCAL_STOP, 0)))       /* 本地停止默认不启用 */
        LcdData.setData.sup_Local_stop = FALSE;
	if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_VIN, 0)))              /* VIN启动默认不启用 */
		LcdData.setData.sup_VIN = FALSE;
    if(FALSE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_INSULATION, 0)))      /* 绝缘配置默认启用 */
        LcdData.setData.sup_insulation = TRUE;
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_CARD, 0)))             /* 读卡器配置默认不启用 */
        LcdData.setData.sup_usecard = FALSE;
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_AUXPOWER24V, 0)))      /* 24V辅源配置默认不启用 */
        LcdData.setData.sup_auxp_24V = FALSE;
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PARALLEL, 0)))         /* 并充配置默认不启用 */
        LcdData.setData.sup_parallelchg = FALSE;
    if(FALSE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PARALLELRELAY, 0)))   /* 并联配置默认启用 */
        LcdData.setData.sup_parallelrelay = TRUE;
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_MODULE_SLIENCE, 0)))   /* 模块静音配置默认不启用 */
        LcdData.setData.sup_mslience = FALSE;
#ifdef SCREEN_USING_OFFLINE_BILLING
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, 0)))   /* 离线计费配置默认不启用 */
        LcdData.setData.sup_offbilling = FALSE;
#else
    LcdData.setData.sup_offbilling = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
    if(TRUE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PASSWORD_START, 0)))   /* 密码启动配置默认不启用 */
        LcdData.setData.sup_pw_start = FALSE;
    if(FALSE != *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_OFFLINE_CARD, 0)))     /* 离线刷卡配置默认启用 */
        LcdData.setData.sup_offline_card = TRUE;

    LcdData.setData.Icon_SuplocalStop = FALSE;
    if(LcdData.setData.sup_Local_stop){
        LcdData.setData.Icon_SuplocalStop = TRUE;
    }

    LcdData.setData.Icon_SupPlugAndPlay = FALSE;
    if(LcdData.setData.Sup_PlugAndPlay){
        LcdData.setData.Icon_SupPlugAndPlay = TRUE;
    }
#ifdef SCREEN_USING_OFFLINE_BILLING
    LcdData.setData.Icon_SupOfflineBilling = FALSE;
    if(LcdData.setData.sup_offbilling){
        LcdData.setData.Icon_SupOfflineBilling = TRUE;
    }
#else
    LcdData.setData.Icon_SupOfflineBilling = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */

    LcdData.setData.Icon_SupPWStart = FALSE;
    if(LcdData.setData.sup_pw_start){
        LcdData.setData.Icon_SupPWStart = TRUE;
    }

    LcdData.setData.Icon_SupOffCard = FALSE;
    if(LcdData.setData.sup_offline_card){
        LcdData.setData.Icon_SupOffCard = TRUE;
    }

    if(LcdData.setData.sup_insulation == FALSE){
        function_disable = 1;
    }else{
        function_disable = 0;
    }
    thaisen_set_insult_mode(function_disable, 0);
    thaisen_set_insult_mode(function_disable, 1);

    if(LcdData.setData.sup_mslience == FALSE){
        thaisenSetYouYouSlienceMode(0);
    }else{
        thaisenSetYouYouSlienceMode(1);
    }

	sSCREEN_EVENT_DEBUGMSG("Get sup_Local=%d sup_VIN=%d\r\n",LcdData.setData.sup_Local,LcdData.setData.sup_VIN );
}

void SerialScreen_InputInfoGet(void)
{
    u8 function_enable = 0;
    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_SCRAM, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 急停故障检测默认启用 */
    LcdData.setData.supin_scram = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_SCRAM, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 急停故障默认不取反 */
    LcdData.setData.neg_scram = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_GATE, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 门禁故障检测默认启用 */
    LcdData.setData.supin_gate = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_GATE, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 门禁故障默认不取反 */
    LcdData.setData.neg_gate = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_ACRELAY, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 交流继电器故障检测默认启用 */
    LcdData.setData.supin_ac = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_ACRELAY, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 交流继电器故障默认不取反 */
    LcdData.setData.neg_ac = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_DCRELAY, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 直流继电器故障检测默认启用 */
    LcdData.setData.supin_dc = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_DCRELAY, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 直流继电器故障默认不取反 */
    LcdData.setData.neg_dc = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_FAN, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 风扇故障检测默认启用 */
    LcdData.setData.supin_fan = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_FAN, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 风扇故障默认不取反 */
    LcdData.setData.neg_fan = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_ELOCK, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 电子锁故障检测默认启用 */
    LcdData.setData.supin_elock = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_ELOCK, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 电子锁故障默认不取反 */
    LcdData.setData.neg_elcok = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_TEMPPRO, 0));
    if(function_enable > TRUE)
        function_enable = TRUE;          /* 温度保护默认启用 */
    LcdData.setData.supin_temp_pro = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_PROTECT_LIGHT, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 防雷器故障检测默认关闭 */
    LcdData.setData.supin_protectlight = function_enable;
    LcdData.setData.Icon_SupProtectLight = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_PROTECT_LIGHT, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 防雷器故障默认不取反 */
    LcdData.setData.neg_protectlight = function_enable;
    LcdData.setData.Icon_NegProtectLight = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_GUNSITE, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 枪座故障检测默认关闭 */
    LcdData.setData.supin_gunsite = function_enable;
    LcdData.setData.Icon_SupGunSite = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_GUNSITE, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 枪座故障默认不取反 */
    LcdData.setData.neg_gunsite = function_enable;
    LcdData.setData.Icon_NegGunSite = function_enable;

//    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_CIRCUIT_BREAKER, 0));
//    if(function_enable > TRUE)
//        function_enable = FALSE;          /* 断路器故障检测默认关闭 */
//    LcdData.setData.supin_circuit_breaker = function_enable;
//    LcdData.setData.Icon_SupCircuitBreaker = function_enable;
//
//    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_CIRCUIT_BREAKER, 0));
//    if(function_enable > TRUE)
//        function_enable = FALSE;          /* 断路器故障默认不取反 */
//    LcdData.setData.neg_circuit_breaker = function_enable;
//    LcdData.setData.Icon_NegCircuitBreaker = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_FLOOD, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 水浸故障检测默认关闭 */
    LcdData.setData.supin_flood = function_enable;
    LcdData.setData.Icon_SupFlood = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_FLOOD, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 水浸故障默认不取反 */
    LcdData.setData.neg_flood = function_enable;
    LcdData.setData.Icon_NegFlood = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_SMOKE, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 烟感故障检测默认关闭 */
    LcdData.setData.supin_smoke = function_enable;
    LcdData.setData.Icon_SupSmoke = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_SMOKE, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 烟感故障默认不取反 */
    LcdData.setData.neg_smoke = function_enable;
    LcdData.setData.Icon_NegSmoke = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_POUR, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 倾倒故障检测默认关闭 */
    LcdData.setData.supin_pour = function_enable;
    LcdData.setData.Icon_SupPour = function_enable;

    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_POUR, 0));
    if(function_enable > TRUE)
        function_enable = FALSE;          /* 倾倒故障默认不取反 */
    LcdData.setData.neg_pour = function_enable;
    LcdData.setData.Icon_NegPour = function_enable;

//    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_LIQUID, 0));
//    if(function_enable > TRUE)
//        function_enable = FALSE;          /* 液冷故障检测默认关闭 */
//    LcdData.setData.supin_liquid = function_enable;
//    LcdData.setData.Icon_SupLiquid = function_enable;
//
//    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_LIQUID, 0));
//    if(function_enable > TRUE)
//        function_enable = FALSE;          /* 液冷故障默认不取反 */
//    LcdData.setData.neg_liquid = function_enable;
//    LcdData.setData.Icon_NegLiquid = function_enable;

//    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_FUSE, 0));
//    if(function_enable > TRUE)
//        function_enable = FALSE;          /* 熔断器故障检测默认关闭 */
//    LcdData.setData.supin_fuse = function_enable;
//    LcdData.setData.Icon_SupFuse = function_enable;
//
//    function_enable = *(u8 *)(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INNEG_FUSE, 0));
//    if(function_enable > TRUE)
//        function_enable = FALSE;          /* 熔断器故障默认不取反 */
//    LcdData.setData.neg_fuse = function_enable;
//    LcdData.setData.Icon_NegFuse = function_enable;
}



int SerialScreen_IsCarConnect(int port)
{
//	port = LcdData.gunIndex;
	return thaisen_app_get_connect_state(port);//0:未连接 1:连接
}

int SerialScreen_IsSupportLocal(int port)
{
	sSCREEN_EVENT_DEBUGMSG("##########IsSupport %d###########\r\n",LcdData.setData.sup_Local);
	if(LcdData.setData.sup_Local != TRUE)
		return 	FALSE;
	else return TRUE;
}


struct charge_data *SerialScreen_GetChargeInfo(int port)
{
	return thaisen_app_get_charge_info(port);
}

struct bms_info *SerialScreen_GetBmsInfo(int port)
{
	return thaisen_app_get_bms_info(port);
}


struct temperature* SerialScreen_GetBatTemp(int port)
{
    return get_battery_temp_info(port);
}

u8 SerialScreen_GetPagePos()
{
	return LcdData.gunIndex;
}


void SerialScreen_GetSysFault(int port)
{
	struct fault_info *chgFault;
	chgFault = thaisen_app_get_fault_info(port);
	//静态故障
	mem_set(LcdData.gun[port].ErrCode, 0, sizeof(LcdData.gun[port].ErrCode));
	if(chgFault->system_fault != APP_SYS_FAULT_NO_ERROR)
	{
        sprintf((s8 *)LcdData.gun[port].ErrCode,"%04d",chgFault->system_fault);
	}
	else
	{
		LcdData.gun[port].ErrCode[0] = 0x20;
	}
	thaisen_app_get_fault_chinese(chgFault->system_fault, 0, LcdData.gun[port].ErrCode_Chinese, sizeof(LcdData.gun[port].ErrCode_Chinese));
}

enum system_stop_way SerialScreen_DataGetstopReson(int port)
{
	enum system_stop_way stopReason;
	stopReason = thaisen_app_get_charge_stop_way(port);
	mem_set(LcdData.gun[port].code_stopResaon, 0,sizeof(LcdData.gun[port].code_stopResaon));
	if(stopReason != APP_SYSTEM_STOP_WAY_SIZE)
	{
        sprintf((s8 *)LcdData.gun[port].code_stopResaon,"%04d",stopReason);		
	}
	else
	{
		LcdData.gun[port].code_stopResaon[0] = 0x20;
	}
	thaisen_app_get_charge_stopway_chinese(stopReason, 0, LcdData.gun[port].code_stopResaon_Chinese, sizeof(LcdData.gun[port].code_stopResaon_Chinese));
}

void SerialScreen_BtnModuleStartA(void)
{
    u8 gunno = 0;
    for(gunno = 0; gunno < LCD_GUN_NUM; gunno++){
        if((LcdData.gun[gunno].workState == SysMainStatus_StartReady) || (LcdData.gun[gunno].workState == SysMainStatus_Chrging)){
            break;
        }
    }
    if(gunno < LCD_GUN_NUM){
        return;
    }
    LcdData.setData.s_moduleVol[LCD_GUN_1] = SerialScreen_GetPara_ValidValue(LcdData.setData.s_moduleVol[LCD_GUN_1],
            COMPULSION_SET_VOLTAGE_DEF, COMPULSION_SET_VOLTAGE_MIN, COMPULSION_SET_VOLTAGE_MAX);
    LcdData.setData.s_moduleCur[LCD_GUN_1] = SerialScreen_GetPara_ValidValue(LcdData.setData.s_moduleCur[LCD_GUN_1],
            COMPULSION_SET_CURRENT_DEF, COMPULSION_SET_CURRENT_MIN, COMPULSION_SET_CURRENT_MAX);
    thaisenSetModuleSetupVolt(LcdData.setData.s_moduleVol[LCD_GUN_1] *10, LCD_GUN_1);
    thaisenSetModuleSetupCurr(LcdData.setData.s_moduleCur[LCD_GUN_1] *10, LCD_GUN_1);
    thaisenSetModuleDebugEnableOutput(LCD_GUN_1);

    LcdData.setData.s_dcRelay[LCD_GUN_1]= !LcdData.setData.s_dcRelay[LCD_GUN_1];
}

void SerialScreen_BtnModuleStartB(void)
{
    u8 gunno = 0;
    for(gunno = 0; gunno < LCD_GUN_NUM; gunno++){
        if((LcdData.gun[gunno].workState == SysMainStatus_StartReady) || (LcdData.gun[gunno].workState == SysMainStatus_Chrging)){
            break;
        }
    }
    if(gunno < LCD_GUN_NUM){
        return;
    }
    LcdData.setData.s_moduleVol[LCD_GUN_2] = SerialScreen_GetPara_ValidValue(LcdData.setData.s_moduleVol[LCD_GUN_2],
            COMPULSION_SET_VOLTAGE_DEF, COMPULSION_SET_VOLTAGE_MIN, COMPULSION_SET_VOLTAGE_MAX);
    LcdData.setData.s_moduleCur[LCD_GUN_2] = SerialScreen_GetPara_ValidValue(LcdData.setData.s_moduleCur[LCD_GUN_2],
            COMPULSION_SET_CURRENT_DEF, COMPULSION_SET_CURRENT_MIN, COMPULSION_SET_CURRENT_MAX);
    thaisenSetModuleSetupVolt(LcdData.setData.s_moduleVol[LCD_GUN_2] *10, LCD_GUN_2);
    thaisenSetModuleSetupCurr(LcdData.setData.s_moduleCur[LCD_GUN_2] *10, LCD_GUN_2);
    thaisenSetModuleDebugEnableOutput(LCD_GUN_2);

    LcdData.setData.s_dcRelay[LCD_GUN_2]= !LcdData.setData.s_dcRelay[LCD_GUN_2];
}

void SerialScreen_BtnModuleStopA(void)
{
    u8 gunno = 0;
    for(gunno = 0; gunno < LCD_GUN_NUM; gunno++){
        if((LcdData.gun[gunno].workState == SysMainStatus_StartReady) || (LcdData.gun[gunno].workState == SysMainStatus_Chrging)){
            break;
        }
    }
    if(gunno < LCD_GUN_NUM){
        return;
    }
    if(thaisenGetModuleDebugEnableOutput(LCD_GUN_1) == 0){
        return;
    }
    thaisenClearModuleDebugEnableOutput(LCD_GUN_1);
    thaisenSetModuleDebugDisableOutput(LCD_GUN_1);

    LcdData.setData.s_dcRelay[LCD_GUN_1]= !LcdData.setData.s_dcRelay[LCD_GUN_1];
}

void SerialScreen_BtnModuleStopB(void)
{
    u8 gunno = 0;
    for(gunno = 0; gunno < LCD_GUN_NUM; gunno++){
        if((LcdData.gun[gunno].workState == SysMainStatus_StartReady) || (LcdData.gun[gunno].workState == SysMainStatus_Chrging)){
            break;
        }
    }
    if(gunno < LCD_GUN_NUM){
        return;
    }
    if(thaisenGetModuleDebugEnableOutput(LCD_GUN_2) == 0){
        return;
    }
    thaisenClearModuleDebugEnableOutput(LCD_GUN_2);
    thaisenSetModuleDebugDisableOutput(LCD_GUN_2);

    LcdData.setData.s_dcRelay[LCD_GUN_2]= !LcdData.setData.s_dcRelay[LCD_GUN_2];
}

static void SerialScreen_BtnModuleStateClear(void)
{
    for(uint8_t count = 0; count < 16; count++){
        memset(LcdData.ModuleStateString[count], '\0', sizeof(LcdData.ModuleStateString[count]));
    }
}

void SerialScreen_BtnModuleStateA(void)
{
    SerialScreen_BtnModuleStateShow(LCD_GUN_1);
}

void SerialScreen_BtnModuleStateB(void)
{
    SerialScreen_BtnModuleStateShow(LCD_GUN_2);
}

static void SerialScreen_BtnModuleStateShow(int port)
{
    if(port >= LCD_GUN_NUM){
        return;
    }
#define INOVERVOLT_INDEX         (1 <<0)            /* 输入过压 */
#define INUNDERVOLT_INDEX        (1 <<1)            /* 输入欠压 */
#define OUTOVERVOLT_INDEX        (1 <<2)            /* 输出过压 */
#define OUTUNDERVOLT_INDEX       (1 <<3)            /* 输出欠压 */
#define SAMEID_INDEX             (1 <<4)            /* 相同ID */
#define MODULEFAULT_INDEX        (1 <<5)            /* 模块故障 */
#define OVERCURR_INDEX           (1 <<6)            /* 过流 */
#define OVERTEMP_INDEX           (1 <<7)            /* 过温 */
#define FAN_INDEX                (1 <<8)            /* 风扇 */

    SerialScreen_BtnModuleStateClear();

    u8 length = 0, group_max = 0, base = 0;
    u16 warnning = 0;
    thaisenModuleFaultInfoStruct * fault = NULL;
    thaisenModuleVoltCurrStruct * voltcurr = NULL;

    if(thaisenGetChargGunRunType() == thaisenDeviceType_average){
        extern void *sys_get_module_config_info(void);
        group_max = ((struct thasienModuleSetStruct *)(sys_get_module_config_info()))->moduleGroupNum;
    }else{
        group_max = 1;
    }

    for(u8 group = 0; group < group_max; group++){
        if(thaisenGetChargGunRunType() == thaisenDeviceType_average){
            voltcurr = thaisenGetModuleVoltCurrInfo(&length, group);
            fault = thaisenGetModuleFaultInfo(&length, group);
        }else{
            voltcurr = thaisenGetModuleVoltCurrInfo(&length, port);
            fault = thaisenGetModuleFaultInfo(&length, port);
        }

        for(u8 count = 0; count < length; count++){
            u8 used_len = 0, remain_len = 0;
            warnning = 0;
            memset(LcdData.ModuleStateString[count + base], '\0', sizeof(LcdData.ModuleStateString[count + base]));
            sprintf((char*)(LcdData.ModuleStateString[count + base] + used_len), "%2X", voltcurr[count].addr);
            used_len += 4;
            if(used_len > strlen((char*)LcdData.ModuleStateString[count + base])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.ModuleStateString[count + base]);
                if(remain_len){
                    memset(((char*)LcdData.ModuleStateString[count + base] + (used_len - remain_len)), ' ', remain_len);
                }
            }

            if(fault[count].state.fault.fault_val != 0){
                if(fault[count].state.fault.bit.OverCurr){
                    warnning |= OVERCURR_INDEX;
                }
                if(fault[count].state.fault.bit.InputOverVolt){
                    warnning |= INOVERVOLT_INDEX;
                }
                if(fault[count].state.fault.bit.InputUnderVolt){
                    warnning |= INUNDERVOLT_INDEX;
                }
                if(fault[count].state.fault.bit.OutputOverVolt){
                    warnning |= OUTOVERVOLT_INDEX;
                }
                if(fault[count].state.fault.bit.OutputUnderVolt){
                    warnning |= OUTUNDERVOLT_INDEX;
                }
                if(fault[count].state.fault.bit.ModuleFault){
                    warnning |= MODULEFAULT_INDEX;
                }
                if(fault[count].state.fault.bit.SameId){
                    warnning |= SAMEID_INDEX;
                }
            }
            if(fault[count].state.warn.warn_val != 0){
                if(fault[count].state.fault.bit.OverTemp){
                    warnning |= OVERTEMP_INDEX;
                }
                if(fault[count].state.warn.bit.Fan){
                    warnning |= FAN_INDEX;
                }
            }
            sprintf((char*)(LcdData.ModuleStateString[count + base] + used_len), "%2d", fault[count].state.state.bit.BootState);
            used_len += 4;
            if(used_len > strlen((char*)LcdData.ModuleStateString[count + base])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.ModuleStateString[count + base]);
                if(remain_len){
                    memset(((char*)LcdData.ModuleStateString[count + base] + (used_len - remain_len)), ' ', remain_len);
                }
            }

            if(voltcurr[count].voltage <= 150){
                voltcurr[count].voltage = 0;
            }
            MakeString(((char*)LcdData.ModuleStateString[count + base] + used_len), 7, voltcurr[count].voltage, 10);
            used_len += 6;
            if(used_len > strlen((char*)LcdData.ModuleStateString[count + base])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.ModuleStateString[count + base]);
                if(remain_len){
                    memset(((char*)LcdData.ModuleStateString[count + base] + (used_len - remain_len)), ' ', remain_len);
                }
            }

            if(voltcurr[count].current <= 5){
                voltcurr[count].current = 0;
            }
            MakeString(((char*)LcdData.ModuleStateString[count + base] + used_len), 7, voltcurr[count].current, 10);
            used_len += 6;
            if(used_len > strlen((char*)LcdData.ModuleStateString[count + base])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.ModuleStateString[count + base]);
                if(remain_len){
                    memset(((char*)LcdData.ModuleStateString[count + base] + (used_len - remain_len)), ' ', remain_len);
                }
            }

            if(fault[count].state.temperature < 0){
                fault[count].state.temperature = 0 - fault[count].state.temperature;
                sprintf((char*)(LcdData.ModuleStateString[count + base] + used_len), "-%2d", fault[count].state.temperature);
            }else{
                sprintf((char*)(LcdData.ModuleStateString[count + base] + used_len), "%2d", fault[count].state.temperature);
            }
            used_len += 4;
            if(used_len > strlen((char*)LcdData.ModuleStateString[count + base])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.ModuleStateString[count + base]);
                if(remain_len){
                    memset(((char*)LcdData.ModuleStateString[count + base] + (used_len - remain_len)), ' ', remain_len);
                }
            }

            sprintf((char*)(LcdData.ModuleStateString[count + base] + used_len), "%2x", warnning);
            used_len += 2;
            if(used_len > strlen((char*)LcdData.ModuleStateString[count + base])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.ModuleStateString[count + base]);
                if(remain_len){
                    memset(((char*)LcdData.ModuleStateString[count + base] + (used_len - remain_len)), ' ', remain_len);
                }
            }
        }

        base += length;
    }

#undef INOVERVOLT_INDEX
#undef INUNDERVOLT_INDEX
#undef OUTOVERVOLT_INDEX
#undef OUTUNDERVOLT_INDEX
#undef SAMEID_INDEX
#undef MODULEFAULT_INDEX
#undef OVERCURR_INDEX
#undef OVERTEMP_INDEX
#undef FAN_INDEX
}

void SerialScreen_BtnBillClear(int port)
{
    if(port >= LCD_GUN_NUM){
        return;
    }
    for(int i = 0; i < 10; i++)
    {
        memset(LcdData.billInfo[i], ' ', sizeof(LcdData.billInfo[i]));
    }
}

void SerialScreen_BtnBillGet(int port)
{
#define ACTUAL_TRADE_LEN  0x10         /* 实际流水号长度 */

#define SERIAL_LEN        0x02         /* 序列号所占长度 */
#define TRADE_LEN         0x12         /* 流水号号所占长度 */
#define START_TIME_LEN    0x10         /* 开始时间所占长度 */
#define STOP_TIMEL_LEN    0x0C         /* 结束时间所占长度 */
#define START_R_LEN       0x04         /* 启动原因所占长度 */
#define STOP_R_LEN        0x04         /* 停止原因所占长度 */
#define ELECT_LEN         0x08         /* 电量所占长度 */
#define MONEY_LEN         0x06         /* 金额所占长度 */

#define BILL_RECORD_MAX   0x64         /* 充电记录最大存储数量 */

    if(port >= LCD_GUN_NUM){
        return;
    }
    u8 _region = RECORD_REGION_CHARGE_RECORDA;
    if(port == LCD_GUN_2){
        _region = RECORD_REGION_CHARGE_RECORDB;
    }
    LcdData.BillCurrentIndex[port] = thaisen_app_get_current_region_index(_region);
    LcdData.BillNum[port] = thaisen_app_get_region_record_total_num(_region);

    rt_kprintf("BillIndex[%d]|%d   BillNum[%d]|%d\n", port, LcdData.BillCurrentIndex[port], port, LcdData.BillNum[port]);

    SerialScreen_BtnBillClear(port);

    if((LcdData.BillCurrentIndex[port] < 0) || (LcdData.BillNum[port] <= 0)){
        return;
    }

    LcdData.BillIndex[port] = LcdData.BillCurrentIndex[port];
    LcdData.BillLable[port] = 0;
    LcdData.BillIndex_Overreturn[port] = 0;

    thaisen_transaction_t billBuf;
    struct tm *_tm;

    for(int i=0;i<10;i++)
    {
        rt_kprintf("bill current(%d, %d, %d)\n", LcdData.BillLable[port], LcdData.BillIndex[port], billBuf.total_fee);
        uint8_t used_len = 0, remain_len = 0;
        memset((char *)LcdData.billInfo[i], '\0', sizeof(LcdData.billInfo[i]));
        thaisen_app_get_index_charge_record(&billBuf, LcdData.BillIndex[port], _region);
        /** 序列号 **/
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d", LcdData.BillLable[port]);
        used_len += SERIAL_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 流水号 **/
        DataToString(((char*)LcdData.billInfo[i] + used_len), TRADE_LEN, (billBuf.serial_number + 7),
                (ACTUAL_TRADE_LEN - 7));  /* 流水号 16位 */
        used_len += TRADE_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 开始时间 **/
        thaisen_enter_critical();
        _tm = localtime((time_t*)(&(billBuf.start_time)));
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),
                (_tm->tm_mon + 1),_tm->tm_mday, _tm->tm_hour, _tm->tm_min);
        thaisen_exit_critical();

        used_len += START_TIME_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 结束时间 **/
        if(billBuf.order_state.is_charging == 0x00){
            thaisen_enter_critical();
            _tm = localtime((time_t*)(&(billBuf.end_time)));
            sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d/%02d %02d:%02d ", (_tm->tm_mon + 1),
                    _tm->tm_mday, _tm->tm_hour,_tm->tm_min);
            thaisen_exit_critical();
        }
        used_len += STOP_TIMEL_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 启动方式 **/
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%03ud", ' ', billBuf.start_type);
        used_len += START_R_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 停充原因 **/
        if(billBuf.order_state.is_charging == 0x00){
            if(billBuf.order_state.verify_fail == 0x01){
                sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%c%02ud", ' ', 'A', mw_system_stop_way_convert(billBuf.stop_reason));
            }else{
                sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%03ud", ' ', mw_system_stop_way_convert(billBuf.stop_reason));
            }
            used_len += STOP_R_LEN;
            if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
                if(remain_len){
                    memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
                }
            }
        }else{
            memset(((char*)LcdData.billInfo[i] + used_len), ' ', STOP_R_LEN);
            used_len += STOP_R_LEN;
        }

        /** 电量 **/
        MakeString(((char*)LcdData.billInfo[i] + used_len), ELECT_LEN, billBuf.total_elect, 1000);
        used_len += ELECT_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 金额 **/
        MakeString(((char*)LcdData.billInfo[i] + used_len), (MONEY_LEN + 1), billBuf.total_fee /100, 100);

        if(LcdData.BillIndex[port] > 0){
            if(LcdData.BillIndex_Overreturn[port] == 1){
                if(LcdData.BillIndex[port] <= LcdData.BillCurrentIndex[port]){
                    break;
                }
            }
            LcdData.BillLable[port]++;
            LcdData.BillIndex[port]--;
        }else{
            if(LcdData.BillNum[port] == BILL_RECORD_MAX){
                if(LcdData.BillIndex_Overreturn[port] == 1){     /* 预防订单总数为100条且当前下标为0的情况 */
                    break;
                }
                LcdData.BillLable[port]++;
                LcdData.BillIndex[port] = (BILL_RECORD_MAX - 1);
                LcdData.BillIndex_Overreturn[port] = 1;
            }else{
                LcdData.BillLable[port]++;
                LcdData.BillIndex[port] = (BILL_RECORD_MAX - 1);
                break;
            }
        }
    }
#undef ACTUAL_TRADE_LEN

#undef SERIAL_LEN
#undef TRADE_LEN
#undef START_TIME_LEN
#undef STOP_TIMEL_LEN
#undef START_R_LEN
#undef STOP_R_LEN
#undef ELECT_LEN
#undef MONEY_LEN

#undef BILL_RECORD_MAX
}

void SerialScreen_BtnBillGetB(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########SerialScreen_BtnBillGetB###########\r\n");
	SerialScreen_BtnBillGet(LCD_GUN_2);
}

void SerialScreen_BtnBillGetA(void)
{
    SerialScreen_BtnBillGet(LCD_GUN_1);
}

void SerialScreen_BtnBillUp(int port)
{
#define ACTUAL_TRADE_LEN  0x10         /* 实际流水号长度 */

#define SERIAL_LEN        0x02         /* 序列号所占长度 */
#define TRADE_LEN         0x12         /* 流水号号所占长度 */
#define START_TIME_LEN    0x10         /* 开始时间所占长度 */
#define STOP_TIMEL_LEN    0x0C         /* 结束时间所占长度 */
#define START_R_LEN       0x04         /* 启动原因所占长度 */
#define STOP_R_LEN        0x04         /* 停止原因所占长度 */
#define ELECT_LEN         0x08         /* 电量所占长度 */
#define MONEY_LEN         0x06         /* 金额所占长度 */
#define BILL_NUM_PAGE     0x0A         /* 一页的订单数量 */

#define BILL_RECORD_MAX   0x64         /* 充电记录最大存储数量 */
    if(port >= LCD_GUN_NUM){
        return;
    }

    if(LcdData.BillLable[port] <= BILL_NUM_PAGE){
        return;
    }
    u8 _BLable = LcdData.BillLable[port];
    s16 _BIndex = LcdData.BillIndex[port];
    u8 _region = RECORD_REGION_CHARGE_RECORDA;
    if(port == LCD_GUN_2){
        _region = RECORD_REGION_CHARGE_RECORDB;
    }

    _BLable -= BILL_NUM_PAGE;
    _BIndex += BILL_NUM_PAGE;
    if(_BLable %BILL_NUM_PAGE){   /* 最后一页小于10条 */
        _BIndex += (_BLable %BILL_NUM_PAGE);
        _BLable -= (_BLable %BILL_NUM_PAGE);
    }else{
        if(_BLable < BILL_NUM_PAGE){
            return;
        }
        _BLable -= BILL_NUM_PAGE;
        _BIndex += BILL_NUM_PAGE;
    }
    SerialScreen_BtnBillClear(port);
    LcdData.BillLable[port] = _BLable;
    LcdData.BillIndex[port] = _BIndex;
    LcdData.BillIndex[port] %= BILL_RECORD_MAX;

    thaisen_transaction_t billBuf;
    struct tm *_tm;

    for(int i=0;i<10;i++)
    {
        rt_kprintf("bill up(%d, %d)\n", LcdData.BillLable[port], LcdData.BillIndex[port]);
        uint8_t used_len = 0, remain_len = 0;
        memset((char *)LcdData.billInfo[i], '\0', sizeof(LcdData.billInfo[i]));
        thaisen_app_get_index_charge_record(&billBuf, LcdData.BillIndex[port], _region);
        /** 序列号 **/
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d", LcdData.BillLable[port]);
        used_len += SERIAL_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 流水号 **/
        DataToString(((char*)LcdData.billInfo[i] + used_len), TRADE_LEN, (billBuf.serial_number + 7),
                (ACTUAL_TRADE_LEN - 7));  /* 流水号 16位 */
        used_len += TRADE_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 开始时间 **/
        thaisen_enter_critical();
        _tm = localtime((time_t*)(&(billBuf.start_time)));
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),
                (_tm->tm_mon + 1),_tm->tm_mday, _tm->tm_hour, _tm->tm_min);
        thaisen_exit_critical();

        used_len += START_TIME_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 结束时间 **/
        if(billBuf.order_state.is_charging == 0x00){
            thaisen_enter_critical();
            _tm = localtime((time_t*)(&(billBuf.end_time)));
            sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d/%02d %02d:%02d ", (_tm->tm_mon + 1),
                    _tm->tm_mday, _tm->tm_hour,_tm->tm_min);
            thaisen_exit_critical();
        }
        used_len += STOP_TIMEL_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 启动方式 **/
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%03ud", ' ', billBuf.start_type);
        used_len += START_R_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 停充原因 **/
        if(billBuf.order_state.is_charging == 0x00){
            if(billBuf.order_state.verify_fail == 0x01){
                sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%c%02ud", ' ', 'A', mw_system_stop_way_convert(billBuf.stop_reason));
            }else{
                sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%03ud", ' ', mw_system_stop_way_convert(billBuf.stop_reason));
            }
            used_len += STOP_R_LEN;
            if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
                if(remain_len){
                    memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
                }
            }
        }else{
            memset(((char*)LcdData.billInfo[i] + used_len), ' ', STOP_R_LEN);
            used_len += STOP_R_LEN;
        }

        /** 电量 **/
        MakeString(((char*)LcdData.billInfo[i] + used_len), ELECT_LEN, billBuf.total_elect, 1000);
        used_len += ELECT_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 金额 **/
        MakeString(((char*)LcdData.billInfo[i] + used_len), (MONEY_LEN + 1), billBuf.total_fee /100, 100);

        if(LcdData.BillIndex[port] > 0){
            LcdData.BillLable[port]++;
            LcdData.BillIndex[port]--;
        }else{
            if(LcdData.BillNum[port] == BILL_RECORD_MAX){
                LcdData.BillLable[port]++;
                LcdData.BillIndex[port] = (BILL_RECORD_MAX - 1);
                LcdData.BillIndex_Overreturn[port] = 0;
            }else{
                break;
            }
        }
    }
    SerialScreen_PageNeedRefresh(port);
#undef ACTUAL_TRADE_LEN

#undef SERIAL_LEN
#undef TRADE_LEN
#undef START_TIME_LEN
#undef STOP_TIMEL_LEN
#undef START_R_LEN
#undef STOP_R_LEN
#undef ELECT_LEN
#undef MONEY_LEN
#undef BILL_NUM_PAGE

#undef BILL_RECORD_MAX
}

void SerialScreen_BtnBillDown(int port)
{
#define ACTUAL_TRADE_LEN  0x10         /* 实际流水号长度 */

#define SERIAL_LEN        0x02         /* 序列号所占长度 */
#define TRADE_LEN         0x12         /* 流水号号所占长度 */
#define START_TIME_LEN    0x10         /* 开始时间所占长度 */
#define STOP_TIMEL_LEN    0x0C         /* 结束时间所占长度 */
#define START_R_LEN       0x04         /* 启动原因所占长度 */
#define STOP_R_LEN        0x04         /* 停止原因所占长度 */
#define ELECT_LEN         0x08         /* 电量所占长度 */
#define MONEY_LEN         0x06         /* 金额所占长度 */

#define BILL_RECORD_MAX   0x64         /* 充电记录最大存储数量 */

    if(port >= LCD_GUN_NUM){
        return;
    }
    rt_kprintf("before BillLable|%d   BillIndex|%d\n", LcdData.BillLable[port], LcdData.BillIndex[port]);
    if(LcdData.BillLable[port] >= LcdData.BillNum[port]){
        return;
    }
    SerialScreen_BtnBillClear(port);
    rt_kprintf("after0 BillLable|%d   BillIndex|%d\n", LcdData.BillLable[port], LcdData.BillIndex[port]);
    thaisen_transaction_t billBuf;
    struct tm *_tm;
    u8 _region = RECORD_REGION_CHARGE_RECORDA;
    if(port == LCD_GUN_2){
        _region = RECORD_REGION_CHARGE_RECORDB;
    }

    for(int i=0;i<10;i++)
    {
        rt_kprintf("bill down(%d, %d)\n", LcdData.BillLable[port], LcdData.BillIndex[port]);
        uint8_t used_len = 0, remain_len = 0;
        memset((char *)LcdData.billInfo[i], '\0', sizeof(LcdData.billInfo[i]));
        thaisen_app_get_index_charge_record(&billBuf, LcdData.BillIndex[port], _region);
        /** 序列号 **/
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d", LcdData.BillLable[port]);
        used_len += SERIAL_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 流水号 **/
        DataToString(((char*)LcdData.billInfo[i] + used_len), TRADE_LEN, (billBuf.serial_number + 7),
                (ACTUAL_TRADE_LEN - 7));  /* 流水号 16位 */
        used_len += TRADE_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 开始时间 **/
        thaisen_enter_critical();
        _tm = localtime((time_t*)(&(billBuf.start_time)));
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),
                (_tm->tm_mon + 1),_tm->tm_mday, _tm->tm_hour, _tm->tm_min);
        thaisen_exit_critical();

        used_len += START_TIME_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 结束时间 **/
        if(billBuf.order_state.is_charging == 0x00){
            thaisen_enter_critical();
            _tm = localtime((time_t*)(&(billBuf.end_time)));
            sprintf(((char*)LcdData.billInfo[i] + used_len), "%02d/%02d %02d:%02d ", (_tm->tm_mon + 1),
                    _tm->tm_mday, _tm->tm_hour,_tm->tm_min);
            thaisen_exit_critical();
        }
        used_len += STOP_TIMEL_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 启动方式 **/
        sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%03ud", ' ', billBuf.start_type);
        used_len += START_R_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        /** 停充原因 **/
        if(billBuf.order_state.is_charging == 0x00){
            if(billBuf.order_state.verify_fail == 0x01){
                sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%c%02ud", ' ', 'A', mw_system_stop_way_convert(billBuf.stop_reason));
            }else{
                sprintf(((char*)LcdData.billInfo[i] + used_len), "%c%03ud", ' ', mw_system_stop_way_convert(billBuf.stop_reason));
            }
            used_len += STOP_R_LEN;
            if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
                remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
                if(remain_len){
                    memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
                }
            }
        }else{
            memset(((char*)LcdData.billInfo[i] + used_len), ' ', STOP_R_LEN);
            used_len += STOP_R_LEN;
        }

        /** 电量 **/
        MakeString(((char*)LcdData.billInfo[i] + used_len), ELECT_LEN, billBuf.total_elect, 1000);
        used_len += ELECT_LEN;
        if(used_len > strlen((char*)LcdData.billInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.billInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.billInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }
        /** 金额 **/
        MakeString(((char*)LcdData.billInfo[i] + used_len), (MONEY_LEN + 1), billBuf.total_fee /100, 100);

        LcdData.BillLable[port]++;
        if(LcdData.BillIndex[port] > 0){
            if(LcdData.BillIndex_Overreturn[port] == 1){
                if(LcdData.BillIndex[port] <= LcdData.BillCurrentIndex[port]){
                    break;
                }
            }
            LcdData.BillIndex[port]--;
        }else{
            if(LcdData.BillNum[port] == BILL_RECORD_MAX){
                if(LcdData.BillIndex_Overreturn[port] == 1){     /* 预防订单总数为100条且当前下标为0的情况 */
                    break;
                }
                LcdData.BillIndex[port] = (BILL_RECORD_MAX - 1);
                LcdData.BillIndex_Overreturn[port] = 1;
            }else{
                LcdData.BillIndex[port] = (BILL_RECORD_MAX - 1);
                break;
            }
        }
    }
    SerialScreen_PageNeedRefresh(port);
#undef ACTUAL_TRADE_LEN

#undef SERIAL_LEN
#undef TRADE_LEN
#undef START_TIME_LEN
#undef STOP_TIMEL_LEN
#undef START_R_LEN
#undef STOP_R_LEN
#undef ELECT_LEN
#undef MONEY_LEN

#undef BILL_RECORD_MAX
}

void SerialScreen_BtnErrClear(int port)
{
    if(port >= LCD_GUN_NUM){
        return;
    }
    for(int i = 0; i < 10; i++)
    {
        memset(LcdData.ErrInfo[i], ' ', sizeof(LcdData.ErrInfo[i]));
    }
}

void SerialScreen_BtnErrGet(int port)
{
#define ACTUAL_SERIAL_LEN 0x02         /* 实际序列号长度 */

#define SERIAL_LEN        0x04         /* 序列号所占长度 */
#define FAULT_CODE_LEN    0x04         /* 故障代码所占长度 */
#define FAULT_STATE_LEN   0x02         /* 故障状态所占长度 */
#define START_TIME_LEN    0x10         /* 故障发生时间所占长度 */
#define STOP_TIME_LEN     0x10         /* 故障恢复时间所占长度 */
#define FAULT_REASON_LEN  0x14         /* 故障原因所占长度 */

#define FAULT_RECORD_MAX  0x64         /* 故障记录最大存储数量 */
    if(port >= LCD_GUN_NUM){
        return;
    }

    u8 _region = RECORD_REGION_FAULT_RECORDA;
    if(port == LCD_GUN_2){
        _region = RECORD_REGION_FAULT_RECORDB;
    }
    LcdData.ErrCurrentIndex[port] = thaisen_app_get_current_region_index(_region);
    LcdData.ErrNum[port] = thaisen_app_get_region_record_total_num(_region);

    rt_kprintf("ErrIndex[%d]|%d   ErrNum[%d]|%d\n", port, LcdData.ErrCurrentIndex[port], port, LcdData.ErrNum[port]);

    SerialScreen_BtnErrClear(port);

    if((LcdData.ErrCurrentIndex[port] < 0) || (LcdData.ErrNum[port] <= 0)){
        return;
    }

    LcdData.ErrIndex[port] = LcdData.ErrCurrentIndex[port];
    LcdData.ErrLable[port] = 0;
    LcdData.ErrIndex_Overreturn[port] = 0;

    struct error_info errBuf;
    struct tm *_tm;

    for(int i = 0; i < 10; i++)
    {
        rt_kprintf("ErrIndex|%d   ErrLable|%d\n", LcdData.ErrIndex[port], LcdData.ErrLable[port]);
        memset((char *)LcdData.ErrInfo[i], '\0', sizeof(LcdData.ErrInfo[i]));
        thaisen_app_get_index_fault_record(&errBuf, LcdData.ErrIndex[port], _region);

        uint8_t used_len = 0, remain_len;
        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d", LcdData.ErrLable[port]);   /* 序列号 2位 */
        used_len += SERIAL_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%04d", errBuf.error_code); /* 故障代码 4位 */
        used_len += FAULT_CODE_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d", errBuf.error_flag); /* 故障状态 2位 */
        used_len += FAULT_STATE_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        thaisen_enter_critical();
        _tm = localtime((time_t*)(&(errBuf.occur_time)));
        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),     /* 发生开始时间 */
                (_tm->tm_mon + 1), _tm->tm_mday, _tm->tm_hour, _tm->tm_min);
        thaisen_exit_critical();

        used_len += START_TIME_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        if(errBuf.error_flag == 0x01){
            thaisen_enter_critical();
            _tm = localtime((time_t*)(&(errBuf.resume_time)));
            sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),    /* 恢复时间 */
                    (_tm->tm_mon + 1), _tm->tm_mday, _tm->tm_hour, _tm->tm_min);
            thaisen_exit_critical();
        }
        used_len +=STOP_TIME_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%s", thaisen_get_fault_string(errBuf.error_code));     /* 故障原因 */

        if(LcdData.ErrIndex[port] > 0){
            if(LcdData.ErrIndex_Overreturn[port] == 1){
                if(LcdData.ErrIndex[port] <= LcdData.ErrCurrentIndex[port]){
                    break;
                }
            }
            LcdData.ErrLable[port]++;
            LcdData.ErrIndex[port]--;
        }else{
            if(LcdData.ErrNum[port] == FAULT_RECORD_MAX){
                if(LcdData.ErrIndex_Overreturn[port] == 1){     /* 预防订单总数为100条且当前下标为0的情况 */
                    break;
                }
                LcdData.ErrLable[port]++;
                LcdData.ErrIndex[port] = (FAULT_RECORD_MAX - 1);
                LcdData.ErrIndex_Overreturn[port] = 1;
            }else{
                LcdData.ErrLable[port]++;
                LcdData.ErrIndex[port] = (FAULT_RECORD_MAX - 1);
                break;
            }
        }
    }
#undef ACTUAL_SERIAL_LEN

#undef SERIAL_LEN
#undef FAULT_CODE_LEN
#undef FAULT_STATE_LEN
#undef START_TIME_LEN
#undef STOP_TIME_LEN
#undef FAULT_REASON_LEN

#undef FAULT_RECORD_MAX
}

void SerialScreen_BtnErrGetB(void)
{
	SerialScreen_BtnErrGet(LCD_GUN_2);
}

void SerialScreen_BtnErrGetA(void)
{
    SerialScreen_BtnErrGet(LCD_GUN_1);
}

void SerialScreen_BtnErrUp(int port)
{
#define ACTUAL_SERIAL_LEN 0x02         /* 实际序列号长度 */

#define SERIAL_LEN        0x04         /* 序列号所占长度 */
#define FAULT_CODE_LEN    0x04         /* 故障代码所占长度 */
#define FAULT_STATE_LEN   0x02         /* 故障状态所占长度 */
#define START_TIME_LEN    0x10         /* 故障发生时间所占长度 */
#define STOP_TIME_LEN     0x10         /* 故障恢复时间所占长度 */
#define FAULT_REASON_LEN  0x14         /* 故障原因所占长度 */

#define FAULT_RECORD_MAX  0x64         /* 故障记录最大存储数量 */
    if(port >= LCD_GUN_NUM){
        return;
    }

    if(LcdData.ErrLable[port] <= 10){
        return;
    }
    u8 _ELable = LcdData.ErrLable[port];
    s16 _EIndex = LcdData.ErrIndex[port];
    u8 _region = RECORD_REGION_FAULT_RECORDA;
    if(port == LCD_GUN_2){
        _region = RECORD_REGION_FAULT_RECORDB;
    }

    _ELable -= 10;
    _EIndex += 10;
    if(_ELable %10){   /* 最后一页小于10条 */
        _EIndex += (_ELable %10);
        _ELable -= (_ELable %10);
    }else{
        if(_ELable < 10){
            return;
        }
        _ELable -= 10;
        _EIndex += 10;
    }
    SerialScreen_BtnErrClear(port);
    LcdData.ErrLable[port] = _ELable;
    LcdData.ErrIndex[port] = _EIndex;
    LcdData.ErrIndex[port] %= FAULT_RECORD_MAX;

    struct error_info errBuf;
    struct tm *_tm;

    for(int i=0;i<10;i++)
    {
        memset((char *)LcdData.ErrInfo[i], '\0', sizeof(LcdData.ErrInfo[i]));
        thaisen_app_get_index_fault_record(&errBuf, LcdData.ErrIndex[port], _region);

        uint8_t used_len = 0, remain_len;
        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d", LcdData.ErrLable[port]);   /* 序列号 2位 */
        used_len += SERIAL_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%04d", errBuf.error_code); /* 故障代码 4位 */
        used_len += FAULT_CODE_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d", errBuf.error_flag); /* 故障状态 2位 */
        used_len += FAULT_STATE_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        thaisen_enter_critical();
        _tm = localtime((time_t*)(&(errBuf.occur_time)));
        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),     /* 发生开始时间 */
                (_tm->tm_mon + 1), _tm->tm_mday, _tm->tm_hour, _tm->tm_min);
        thaisen_exit_critical();

        used_len += START_TIME_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        if(errBuf.error_flag == 0x01){
            thaisen_enter_critical();
            _tm = localtime((time_t*)(&(errBuf.resume_time)));
            sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),    /* 恢复时间 */
                    (_tm->tm_mon + 1), _tm->tm_mday, _tm->tm_hour, _tm->tm_min);
            thaisen_exit_critical();
        }
        used_len +=STOP_TIME_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%s", thaisen_get_fault_string(errBuf.error_code));     /* 故障原因 */

        if(LcdData.ErrIndex[port] > 0){
            LcdData.ErrLable[port]++;
            LcdData.ErrIndex[port]--;
        }else{
            if(LcdData.ErrNum[port] == FAULT_RECORD_MAX){
                LcdData.ErrLable[port]++;
                LcdData.ErrIndex[port] = (FAULT_RECORD_MAX - 1);
                LcdData.ErrIndex_Overreturn[port] = 0;
            }else{
                break;
            }
        }
    }
    SerialScreen_PageNeedRefresh(port);
#undef ACTUAL_SERIAL_LEN

#undef SERIAL_LEN
#undef FAULT_CODE_LEN
#undef FAULT_STATE_LEN
#undef START_TIME_LEN
#undef STOP_TIME_LEN
#undef FAULT_REASON_LEN

#undef FAULT_RECORD_MAX
}

void SerialScreen_BtnErrDown(int port)
{
#define ACTUAL_SERIAL_LEN 0x02         /* 实际序列号长度 */

#define SERIAL_LEN        0x04         /* 序列号所占长度 */
#define FAULT_CODE_LEN    0x04         /* 故障代码所占长度 */
#define FAULT_STATE_LEN   0x02         /* 故障状态所占长度 */
#define START_TIME_LEN    0x10         /* 故障发生时间所占长度 */
#define STOP_TIME_LEN     0x10         /* 故障恢复时间所占长度 */
#define FAULT_REASON_LEN  0x14         /* 故障原因所占长度 */

#define FAULT_RECORD_MAX  0x64         /* 故障记录最大存储数量 */
    if(port >= LCD_GUN_NUM){
        return;
    }
    if(LcdData.ErrLable[port] >= LcdData.ErrNum[port]){
        return;
    }
    SerialScreen_BtnErrClear(port);
    struct error_info errBuf;
    struct tm *_tm;
    u8 _region = RECORD_REGION_FAULT_RECORDA;
    if(port == LCD_GUN_2){
        _region = RECORD_REGION_FAULT_RECORDB;
    }

    for(int i=0;i<10;i++)
    {
        memset((char *)LcdData.ErrInfo[i], '\0', sizeof(LcdData.ErrInfo[i]));
        thaisen_app_get_index_fault_record(&errBuf, LcdData.ErrIndex[port], _region);

        uint8_t used_len = 0, remain_len;
        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d", LcdData.ErrLable[port]);   /* 序列号 2位 */
        used_len += SERIAL_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%04d", errBuf.error_code); /* 故障代码 4位 */
        used_len += FAULT_CODE_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d", errBuf.error_flag); /* 故障状态 2位 */
        used_len += FAULT_STATE_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        thaisen_enter_critical();
        _tm = localtime((time_t*)(&(errBuf.occur_time)));
        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),     /* 发生开始时间 */
                (_tm->tm_mon + 1), _tm->tm_mday, _tm->tm_hour, _tm->tm_min);
        thaisen_exit_critical();

        used_len += START_TIME_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        if(errBuf.error_flag == 0x01){
            thaisen_enter_critical();
            _tm = localtime((time_t*)(&(errBuf.resume_time)));
            sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%02d/%02d/%02d %02d:%02d", (_tm->tm_year + 1900),    /* 恢复时间 */
                    (_tm->tm_mon + 1), _tm->tm_mday, _tm->tm_hour, _tm->tm_min);
            thaisen_exit_critical();
        }
        used_len +=STOP_TIME_LEN;
        if(used_len > strlen((char*)LcdData.ErrInfo[i])){    /* 未使用字节填充空格字符 */
            remain_len = used_len - strlen((char*)LcdData.ErrInfo[i]);
            if(remain_len){
                memset(((char*)LcdData.ErrInfo[i] + (used_len - remain_len)), ' ', remain_len);
            }
        }

        sprintf(((char*)LcdData.ErrInfo[i] + used_len), "%s", thaisen_get_fault_string(errBuf.error_code));     /* 故障原因 */

        LcdData.ErrLable[port]++;
        if(LcdData.ErrIndex[port] > 0){
            if(LcdData.ErrIndex_Overreturn[port] == 1){
                if(LcdData.ErrIndex[port] <= LcdData.ErrCurrentIndex[port]){
                    break;
                }
            }
            LcdData.ErrIndex[port]--;
        }else{
            if(LcdData.ErrNum[port] == FAULT_RECORD_MAX){
                if(LcdData.ErrIndex_Overreturn[port] == 1){     /* 预防订单总数为100条且当前下标为0的情况 */
                    break;
                }
                LcdData.ErrIndex[port] = (FAULT_RECORD_MAX - 1);
                LcdData.ErrIndex_Overreturn[port] = 1;
            }else{
                LcdData.ErrIndex[port] = (FAULT_RECORD_MAX - 1);
                break;
            }
        }
    }
    SerialScreen_PageNeedRefresh(port);
#undef ACTUAL_SERIAL_LEN

#undef SERIAL_LEN
#undef FAULT_CODE_LEN
#undef FAULT_STATE_LEN
#undef START_TIME_LEN
#undef STOP_TIME_LEN
#undef FAULT_REASON_LEN

#undef FAULT_RECORD_MAX
}

s32 SerialScreen_BtnClearBill(int port)
{
	sSCREEN_EVENT_DEBUGMSG("##########SerialScreen_BtnClearBill[%d]###########\r\n",port);
	if(port == LCD_GUN_1){
	    return thaisen_app_clear_region_record_info(RECORD_REGION_CHARGE_RECORDA);
	}else if(port == LCD_GUN_2){
	    return thaisen_app_clear_region_record_info(RECORD_REGION_CHARGE_RECORDB);
	}
	return -1;
}

s32 SerialScreen_BtnClearErr(int port)
{
	sSCREEN_EVENT_DEBUGMSG("##########SerialScreen_BtnClearErr[%d]###########\r\n",port);
    if(port == LCD_GUN_1){
        return thaisen_app_clear_region_record_info(RECORD_REGION_FAULT_RECORDA);
    }else if(port == LCD_GUN_2){
        return thaisen_app_clear_region_record_info(RECORD_REGION_FAULT_RECORDB);
    }
    return -1;
}

s32 SerialScreen_BtnClearBillErr(int port)
{
    int ret, val = 0;
	ret = SerialScreen_BtnClearBill(port);
	val = SerialScreen_BtnClearErr(port);
	if((ret < 0 )||(val < 0))
	    return FALSE;
	else return TRUE;
}

s32 SerialScreen_BtnClearAll(void)
{
	u8 i; u8 ret = TRUE;
	u8 val[LCD_GUN_NUM];

    SerialScreen_JumpPage(&SerialScreen, LCD_PAGE_CLEAR_RECORD_WAITING);

	for(i = 0; i <LCD_GUN_NUM; i++)
	{
	    val[i] = SerialScreen_BtnClearBillErr(i);
	    if(val[i] < 0)
	        ret = FALSE;
	}
	return ret;
}

void SerialScreen_GetIOStatusA(void)
{
    SerialScreen_GetIOStatus(LCD_GUN_1);
}

void SerialScreen_GetIOStatusB(void)
{
    SerialScreen_GetIOStatus(LCD_GUN_2);
}

void SerialScreen_QuitDebugIO(void)
{
	LcdData.debugIOflg = FALSE;	
	thaisen_set_debug_mode(0);
}

void SerialScreen_GetIOStatus(int port)
{
	u8 res, gunno = 0;
	if((port < LCD_GUN_1)&&(port >= LCD_GUN_NUM))
		return ;
	
    if(thaisenGetModuleDebugEnableOutput(port))
        return;

    if(port == 0)
        gunno = 1;

	LcdData.debugIOflg = TRUE;
	thaisen_set_debug_mode(1);
	
    if(thaisenGetModuleDebugEnableOutput(gunno) == 0)
    {
        res = (u8)thaisen_relay_parallel_off_z();
        thaisen_relay_parallel_off_f();
        if(res == 0)//ok
            LcdData.setData.g_paraRely0 = REALAY_OFF;
        else
            LcdData.setData.g_paraRely0 = REALAY_CLOSE;

        LcdData.setData.g_paraRely0 = REALAY_OFF;
        LcdData.setData.s_paraRely0 = LcdData.setData.g_paraRely0;

        LcdData.setData.g_paraRely1 = REALAY_OFF;
        LcdData.setData.s_paraRely1 = LcdData.setData.g_paraRely1;

        LcdData.setData.g_paraRely2 = REALAY_OFF;
        LcdData.setData.s_paraRely2 = LcdData.setData.g_paraRely2;


        res = (u8)thaisen_relay_ac_off();
        if(res == 0)//ok
            LcdData.setData.g_acRely = REALAY_OFF;
        else
            LcdData.setData.g_acRely = REALAY_CLOSE;

        LcdData.setData.g_acRely = REALAY_OFF;
        LcdData.setData.s_acRely  = LcdData.setData.g_acRely;
    }
	
	if(port == LCD_GUN_1)
		res = (u8)thaisen_relay_off_A();
	else if(port == LCD_GUN_2)
        res = (u8)thaisen_relay_off_B();
	if(res == 0)//ok
		LcdData.setData.g_dcRelay[port] = REALAY_OFF;
	else
		LcdData.setData.g_dcRelay[port] = REALAY_CLOSE;
			
    LcdData.setData.g_dcRelay[port] = REALAY_OFF;
	LcdData.setData.s_dcRelay[port]  = LcdData.setData.g_dcRelay[port];

#if 0
	if(port == LCD_GUN_1)
		res = (u8)thaisenElectUnlock();
	else if(port == LCD_GUN_2)
		res = (u8)thaisenElectUnlockB();
	if(res == 0)//ok
		LcdData.setData.g_elElock[port] = REALAY_OFF;
	else
		LcdData.setData.g_elElock[port] = REALAY_CLOSE;	
#endif
//	if(port == LCD_GUN_1)
//		thaisenElectUnlockA_debug();
//	else if(port == LCD_GUN_2)
//		thaisenElectUnlockB_debug();
	LcdData.setData.g_elElock[port]= REALAY_OFF;
	LcdData.setData.s_elElock[port]  = LcdData.setData.g_elElock[port];

	if(port == LCD_GUN_1)
		res = (u8)thaisen_auxPower_off_A();
	else if(port == LCD_GUN_2)
		res = (u8)thaisen_auxPower_off_B();
	if(res == 0)//ok
		LcdData.setData.g_auxRelay[port] = REALAY_CLOSE;
	else
		LcdData.setData.g_auxRelay[port] = REALAY_OFF;

    LcdData.setData.g_auxRelay[port] = REALAY_OFF;
	LcdData.setData.g_auxRelay[port] = LcdData.setData.s_auxRelay[port];

    if(port == LCD_GUN_1)
        thaisenAux_A_24V_Disable();
    else if(port == LCD_GUN_2)
        thaisenAux_B_24V_Disable();

    LcdData.setData.aux24v_set[port] = REALAY_OFF;

	return ;
}

void SerialScreen_BtnAcSet(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########BtnAcSet = %d###########\r\n",LcdData.setData.s_acRely);
	LcdData.setData.s_acRely = !LcdData.setData.s_acRely;
	if(LcdData.setData.s_acRely != TRUE)
		thaisen_relay_AC_off();
	else
		thaisen_relay_AC_on();
	sSCREEN_EVENT_DEBUGMSG("s_acRely=%d\r\n",LcdData.setData.s_acRely);	
}

void SerialScreen_BtnDcSetA()
{
	SerialScreen_BtnDcSet(LCD_GUN_1);	
}

void SerialScreen_BtnDcSetB()
{
	SerialScreen_BtnDcSet(LCD_GUN_2);
}


void SerialScreen_BtnDcSet(u8 port)
{
	sSCREEN_EVENT_DEBUGMSG("##########BtnDC%dSet = %d###########\r\n",port,LcdData.setData.s_dcRelay[port]);
    if(thaisenGetModuleDebugEnableOutput(port))
        return;

	LcdData.setData.s_dcRelay[port]= !LcdData.setData.s_dcRelay[port];
	if(LcdData.setData.s_dcRelay[port] != TRUE)
	{
		if(port == LCD_GUN_1)
			thaisen_relay_off_A();
		else if(port == LCD_GUN_2)
            thaisen_relay_off_B();
	}
	else
	{
	if(port == LCD_GUN_1)
		thaisen_relay_on_A();
	else if(port == LCD_GUN_2)
		thaisen_relay_on_B();
	}
	sSCREEN_EVENT_DEBUGMSG("s_dcRelay[%d]=%d\r\n",port,LcdData.setData.s_dcRelay[port]);	
}

void SerialScreen_BtnParaSet1(void)
{
	sSCREEN_EVENT_DEBUGMSG("##########s_paraRely0 = %d###########\r\n",LcdData.setData.s_paraRely0);
	LcdData.setData.s_paraRely0 = !LcdData.setData.s_paraRely0;
	if(LcdData.setData.s_paraRely0 != TRUE)
	{
		thaisen_relay_parallel_off_f();
		thaisen_relay_parallel_off_z();
	}	
	else
	{
		thaisen_relay_parallel_on_f();
		thaisen_relay_parallel_on_z();
	}
	sSCREEN_EVENT_DEBUGMSG("s_paraRely0=%d\r\n",LcdData.setData.s_paraRely0);
}

void SerialScreen_BtnParaSet2(void)
{
    sSCREEN_EVENT_DEBUGMSG("##########s_paraRely1 = %d###########\r\n",LcdData.setData.s_paraRely1);
    LcdData.setData.s_paraRely1 = !LcdData.setData.s_paraRely1;
    if(LcdData.setData.s_paraRely1 != TRUE)
    {
        thaisen_relay_k7k8_off();
    }
    else
    {
        thaisen_relay_k7_k8_on();
    }
    sSCREEN_EVENT_DEBUGMSG("s_paraRely1=%d\r\n",LcdData.setData.s_paraRely1);
}

void SerialScreen_BtnParaSet3(void)
{
    sSCREEN_EVENT_DEBUGMSG("##########s_paraRely2 = %d###########\r\n",LcdData.setData.s_paraRely2);
    LcdData.setData.s_paraRely2 = !LcdData.setData.s_paraRely2;
    if(LcdData.setData.s_paraRely2 != TRUE)
    {
        thaisen_relay_k9k10_off();
    }
    else
    {
        thaisen_relay_k9_k10_on();
    }
    sSCREEN_EVENT_DEBUGMSG("s_paraRely2=%d\r\n",LcdData.setData.s_paraRely2);
}

void SerialScreen_BtnUnElockA(void)
{
    thaisenElectUnlock();
    LcdData.setData.s_elElock[LCD_GUN_1] = !thaisenGetElectLockStaA();
}

void SerialScreen_BtnUnElockB(void)
{
    thaisenElectUnlockB();
    LcdData.setData.s_elElock[LCD_GUN_2] = !thaisenGetElectLockStaB();
}

void SerialScreen_BtnElockSetA()
{
	SerialScreen_BtnElockSet(LCD_GUN_1);
}

void SerialScreen_BtnElockSetB()
{
	SerialScreen_BtnElockSet(LCD_GUN_2);
}

void SerialScreen_BtnElockSet(u8 port)
{
	sSCREEN_EVENT_DEBUGMSG("##########BtnElock%dSet = %d###########\r\n",port,LcdData.setData.s_elElock[port]);
	LcdData.setData.s_elElock[port]= !LcdData.setData.s_elElock[port];
	if(LcdData.setData.s_elElock[port] != TRUE)
	{
		if(port == LCD_GUN_1)
			thaisenElectUnlockA_debug();
		else if(port == LCD_GUN_2)
			thaisenElectUnlockB_debug();
	}
	else
	{
	if(port == LCD_GUN_1)
		thaisenElectLockA_debug();
	else if(port == LCD_GUN_2)
		thaisenElectLockB_debug();
	}
	sSCREEN_EVENT_DEBUGMSG("BtnElock%dSet = %d\r\n",port,LcdData.setData.s_elElock[port]);	
}

void SerialScreen_BtnAuxSetA()
{
	SerialScreen_BtnAuxSet(LCD_GUN_1);
}

void SerialScreen_BtnAuxSetB()
{
	SerialScreen_BtnAuxSet(LCD_GUN_2);
}


void SerialScreen_BtnAuxSet(u8 port)
{
	sSCREEN_EVENT_DEBUGMSG("##########BtnAux%dSet = %d###########\r\n",port,LcdData.setData.s_auxRelay[port]);
	LcdData.setData.s_auxRelay[port]= !LcdData.setData.s_auxRelay[port];
	if(LcdData.setData.s_auxRelay[port] != TRUE)
	{
		if(port == LCD_GUN_1)
			thaisenAux_A_Disable();
		else if(port == LCD_GUN_2)
			thaisenAux_B_Disable();
	}
	else
	{
	if(port == LCD_GUN_1)
		thaisenAux_A_Enable();
	else if(port == LCD_GUN_2)
		thaisenAux_B_Enable();
	}
	sSCREEN_EVENT_DEBUGMSG("BtnFan%dSet = %d\r\n",port,LcdData.setData.s_auxRelay[port]);

}

void SerialScreen_BtnAux24VSetA()
{
    SerialScreen_BtnAux24VSet(LCD_GUN_1);
}

void SerialScreen_BtnAux24VSetB()
{
    SerialScreen_BtnAux24VSet(LCD_GUN_2);
}

void SerialScreen_BtnAux24VSet(u8 port)
{
    LcdData.setData.aux24v_set[port]= !LcdData.setData.aux24v_set[port];
    if(LcdData.setData.aux24v_set[port] != TRUE)
    {
        if(port == LCD_GUN_1)
            thaisenAux_A_24V_Disable();
        else if(port == LCD_GUN_2)
            thaisenAux_B_24V_Disable();
    }
    else
    {
        if(port == LCD_GUN_1)
            thaisenAux_A_24V_Enable();
        else if(port == LCD_GUN_2)
            thaisenAux_B_24V_Enable();
    }
}

void SerialScreen_BtnFanSetA()
{
	SerialScreen_BtnFanSet(LCD_GUN_1);
}

void SerialScreen_BtnFanSetB() 
{
	SerialScreen_BtnFanSet(LCD_GUN_2);
}

void SerialScreen_BtnFanSet(u8 port)
{
	sSCREEN_EVENT_DEBUGMSG("##########BtnFan%dSet = %d###########\r\n",port,LcdData.setData.s_fan[port]);
    port = LCD_GUN_1;
	LcdData.setData.s_fan[port]= !LcdData.setData.s_fan[port];
	if(LcdData.setData.s_fan[port] != TRUE)
	{
		if(port == LCD_GUN_1)
			thaisen_fan_A_off();
		else if(port == LCD_GUN_2)
			thaisen_fan_B_off();
	}
	else
	{
	if(port == LCD_GUN_1)
		thaisen_fan_A_on();
	else if(port == LCD_GUN_2)
		thaisen_fan_B_on();
	}
	sSCREEN_EVENT_DEBUGMSG("BtnFan%dSet = %d\r\n",port,LcdData.setData.s_fan[port]);	
}

void SerialScreen_AuxsetA()
{
    if(LcdAssistantData.Flag.IsEnableAuxPower24V == TRUE){
        if(LcdData.setData.s_selectaux[LCD_GUN_1] != ICON_AUXPOWER_NONE){
            if(LcdData.setData.s_selectaux[LCD_GUN_1] == (ICON_AUXPOWER_12V + 2 *LCD_GUN_1)){
                LcdData.setData.s_selectaux[LCD_GUN_1] = (ICON_AUXPOWER_24V + 2 *LCD_GUN_1);
                thaisenSetAuxPowerTypeA(thaisen_auxPowerType_24V);
            }else{
                LcdData.setData.s_selectaux[LCD_GUN_1] = (ICON_AUXPOWER_12V + 2 *LCD_GUN_1);
                thaisenSetAuxPowerTypeA(thaisen_auxPowerType_12V);
            }
        }
    }
}

void SerialScreen_AuxsetB()
{
    if(LcdAssistantData.Flag.IsEnableAuxPower24V == TRUE){
        if(LcdData.setData.s_selectaux[LCD_GUN_2] != ICON_AUXPOWER_NONE){
            if(LcdData.setData.s_selectaux[LCD_GUN_2] == (ICON_AUXPOWER_12V + 2 *LCD_GUN_2)){
                LcdData.setData.s_selectaux[LCD_GUN_2] = (ICON_AUXPOWER_24V + 2 *LCD_GUN_2);
                thaisenSetAuxPowerTypeB(thaisen_auxPowerType_24V);
            }else{
                LcdData.setData.s_selectaux[LCD_GUN_2] = (ICON_AUXPOWER_12V + 2 *LCD_GUN_2);
                thaisenSetAuxPowerTypeB(thaisen_auxPowerType_12V);
            }
        }
    }
}


//NET--读取INT16
u16 NetReadBigU16(u8* buf)
{
	u16 data = 0;
	data = (buf[0]<<8)|buf[1];
	return data;
}

u8 getSecToTimeStr(u8 *str,u32 dat)
{
	u32 tmin = dat/60;

    sprintf((char *)str,"%02d:%02d",tmin/60,tmin%60);

	return 1;
}


void SerialScreen_NeedPageReset(int port)
{
	//port = port;
	LcdData.menuflg = 1;
	LcdData.Homeflg = 0;
	//LcdData.NeedMenuOffFlg = 1;
	//LcdData.NeedMenuOffTimer = 2000; //100s*20
	for(u8 i = 0; i < LCD_GUN_NUM; i++){
	    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
	}
}

void SerialScreen_PWStartAuthen(void)
{
    if(LcdData.setData.sup_pw_start){
        LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen = TRUE;
    }else{
        LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen = FALSE;
    }
}


void SerialScreen_SysInfoGet(void)
{
	SerialScreen_NeedPageReset(LCD_GUN_1);
	SerialScreen_BtnServerGet();
	SerialScreen_BtnChgInfoGet(LCD_GUN_1);	
}


int SerialScreen_DataToStr(u8 valtype,u16 maxlen,void *valaddr,u8 *tag)
{
	int ret = 1,i;
	u8 *ptr;
	u32 udat;
	s32 sdat;
	f32 fdat;
	u8 tbuf[10];
	
	switch(valtype)
	{
		case   pu8_type		:
            udat = *((u8 *)valaddr);
			sprintf((s8 *)tag,"%d",udat);
			break;
		case   pu16_type		:
			udat = *((u16 *)valaddr);
			sprintf((s8 *)tag,"%d",udat);
			break;
		case   pu32_type		:
			udat = *((u32 *)valaddr);
			sprintf((s8 *)tag,"%d",udat);
			break;
        case   ps8_type		:
			sdat = *((s8 *)valaddr);
            sprintf((s8 *)tag,"%d",sdat);
			break;
        case   ps16_type		:
			sdat = *((s16 *)valaddr);
            sprintf((s8 *)tag,"%d",sdat);
			break;
        case   ps32_type		:
			sdat = *((s32 *)valaddr);
            sprintf((s8 *)tag,"%d",sdat);
			break;
		case   pu8x10_type		:
			udat = *((u8 *)valaddr);
			fdat = udat;
			fdat *= 0.1;
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   pu16x10_type		:
			udat = *((u16 *)valaddr);
			fdat = udat;
			fdat *= 0.1;
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   pu32x10_type		:
			udat = *((u32 *)valaddr);
			fdat = udat;
			fdat *= 0.1;
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   ps8x10_type		:	
			sdat = *((s8 *)valaddr);
			fdat = sdat;
			fdat *= 0.1;
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   ps16x10_type		:	
			sdat = *((s16 *)valaddr);
			fdat = sdat;
			fdat *= 0.1;
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   ps32x10_type		:	
			sdat = *((s32 *)valaddr);
			fdat = sdat;
			fdat *= 0.1;
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   pu8x100_type		:
			udat = *((u8 *)valaddr);
			fdat = udat;
			fdat *= 0.01;
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   pu16x100_type		:
			udat = *((u16 *)valaddr);
			fdat = udat;
			fdat *= 0.01;
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   pu32x100_type		:
			udat = *((u32 *)valaddr);
			fdat = udat;
			fdat *= 0.01;
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   ps8x100_type		:	
			sdat = *((s8 *)valaddr);
			fdat = sdat;
			fdat *= 0.01;
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   ps16x100_type		:	
			sdat = *((s16 *)valaddr);
			fdat = sdat;
			fdat *= 0.01;
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   ps32x100_type		:	
			sdat = *((s32 *)valaddr);
			fdat = sdat;
			fdat *= 0.01;
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   pu16x1000_type		:
			udat = *((u16 *)valaddr);
			fdat = udat;
			fdat *= 0.001;
			sprintf((s8 *)tag,"%.3f",fdat);
			break;
		case   pu32x1000_type		:
			udat = *((u32 *)valaddr);
			fdat = udat;
			fdat *= 0.001;
			sprintf((s8 *)tag,"%.3f",fdat);
			break;
		case   ps16x1000_type		:	
			sdat = *((s16 *)valaddr);
			fdat = sdat;
			fdat *= 0.001;
			sprintf((s8 *)tag,"%.3f",fdat);
			break;
		case   ps32x1000_type		:	
            sdat = *((s32 *)valaddr);
			fdat = sdat;
			fdat *= 0.001;
			sprintf((s8 *)tag,"%.3f",fdat);
			break;	
		case   pf32_type		:
            fdat = *((f32 *)valaddr);
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   pf32_1type		:
            fdat = *((f32 *)valaddr);
			sprintf((s8 *)tag,"%.1f",fdat);
			break;
		case   pf32_3type		:
            fdat = *((f32 *)valaddr);
			sprintf((s8 *)tag,"%.3f",fdat);
			break;
		case   pf32_4type		:
            fdat = *((f32 *)valaddr);
			sprintf((s8 *)tag,"%.4f",fdat);
			break;
		case   pf32_5type		:
            fdat = *((f32 *)valaddr);
			sprintf((s8 *)tag,"%.5f",fdat);
			break;
		case   pf64_type		:
            fdat = *((f32 *)valaddr);
			sprintf((s8 *)tag,"%.2f",fdat);
			break;
		case   pstr_type		:
			ptr = ((u8 *)valaddr);
			str_ncpy(tag, ptr, maxlen);
			tag[maxlen-1] = 0;
			break;
		case   ip_type		:
           	udat = *(u32*)(valaddr);
			sprintf((s8*)tag,"%d.%d.%d.%d",((udat>>0)&0xff),((udat>>8)&0xff),((udat>>16)&0xff),((udat>>24)&0xff));
			break;
		case   pu8_nH_type		:
			ptr = ((u8 *)valaddr);
			tag[0] = 0;
			for(i=0;i<maxlen;i++)
			{
                sprintf((s8 *)tbuf,"%02d",*(ptr+i));
				str_cat((s8 *)tag, tbuf);
			}
			break;	//n个数据 hex格式显示
		case menu_type:
			#if 1
			udat = *((u16 *)valaddr);
			sSCREEN_EVENT_DEBUGMSG("udat = %d\r\n",udat);
			tag[1] = udat;
			tag[0] = udat>>8;					
			#else
			udat = *((u16 *)valaddr);
			sprintf((s8 *)tag,"%d",udat);
			#endif
			break;
		default:
			ret = 0;
			tag[0] = 0;
			break;
	}
	return ret;
}

int SerialScreen_CombineData(u8 valtype,void *valaddr,u8 *src,u8 slen,u8 flag)
{
    u32 data = SerialScreen_Calculate_Data_From_Byte(src, slen, flag);
    switch(valtype){
    case pu8_type:
    case ps8_type:
        *((u8 *)valaddr) = (u8)data;
        break;
    case pu16_type:
    case ps16_type:
        *((u16 *)valaddr) = (u16)data;
        break;
    case pu32_type:
    case ps32_type:
        *((u32 *)valaddr) = (u32)data;
        break;
    default:
        break;
    }
    return 0;
}

u32 SerialScreen_GetData_WithType(u8 valtype,void *valaddr)
{
    u32 data = 0;
    switch(valtype){
    case pu8_type:
    case ps8_type:
        data = (uint32_t)(*((u8 *)valaddr));
        break;
    case pu16_type:
    case ps16_type:
        data = (uint32_t)(*((u16 *)valaddr));
        break;
    case pu32_type:
    case ps32_type:
        data = *((u32 *)valaddr);
        break;
    default:
        break;
    }
    return data;
}

int SerialScreen_StrToData(u8 valtype,u16 maxlen,void *valaddr,u8 *src)
{
    int ret = 1;
	u8 *ptr;
	u32 udat;
	s32 sdat;
	f32 fdat;
	//u8 tbuf[10];
	
	switch(valtype)
	{
		case   pu8_type		:
			udat = str_toInt(src);
            *((u8 *)valaddr) = udat;
			break;
		case   pu16_type		:
			udat = str_toInt(src);
			*((u16 *)valaddr) = udat;
			break;
		case   pu32_type		:
			udat = str_toInt(src);
			*((u32 *)valaddr) = udat;
			break;
        case   ps8_type		:
			sdat = str_toInt(src);
			*((s8 *)valaddr) = sdat;
			break;
        case   ps16_type		:
			sdat = str_toInt(src);
			*((s16 *)valaddr) = sdat;
			break;
        case   ps32_type		:
			sdat = str_toInt(src);
			*((s32 *)valaddr) = sdat;
			break;
		case   ps8x10_type		:		
		case   pu8x10_type		:
			fdat = str_tofloat(src);
			fdat *= 10;
			*((u8 *)valaddr) = fdat;
			break;
		case   ps16x10_type		:		
		case   pu16x10_type		:
			fdat = str_tofloat(src);
			fdat *= 10;
			*((u16 *)valaddr) = fdat;
			break;
		case   ps32x10_type		:		
		case   pu32x10_type		:
			fdat = str_tofloat(src);
			fdat *= 10;
			*((u32 *)valaddr) = fdat;
			break;
		case   ps8x100_type		:		
		case   pu8x100_type		:
			fdat = str_tofloat(src);
			fdat *= 100;
			*((u8 *)valaddr) = fdat;
			break;
		case   ps16x100_type		:		
		case   pu16x100_type		:
			fdat = str_tofloat(src);
			fdat *= 100;
			*((u16 *)valaddr) = fdat;
			break;
		case   ps32x100_type		:		
		case   pu32x100_type		:
			fdat = str_tofloat(src);
			fdat *= 100;
			*((u32 *)valaddr) = fdat;
			break;
		case   ps16x1000_type		:	
		case   pu16x1000_type		:
			fdat = str_tofloat(src);
			fdat *= 1000;
			*((u16 *)valaddr) = fdat;
			break;
		case   ps32x1000_type		:		
		case   pu32x1000_type		:
			fdat = str_tofloat(src);
			fdat *= 1000;
			*((u32 *)valaddr) = fdat;
			break;
		case   pf32_type		:
		case   pf32_1type		:
		case   pf32_3type		:
		case   pf32_4type		:
		case   pf32_5type		:		
			fdat = str_tofloat(src);
            *((f32 *)valaddr) = fdat;
			break;
		case   pf64_type		:
            fdat = str_tofloat(src);
            *((f32 *)valaddr) = fdat;
			break;
		case   pstr_type		:
			ptr = ((u8 *)valaddr);
			str_ncpy(ptr, src, maxlen);
			ptr[maxlen-1] = 0;
			break;
		#if 0
		case   ip_type		:
			instr.sprintf("%s",src);
			if(SerialScreen_isIpAddress(instr))
			{
				qstrlist.clear();
                qstrlist = instr.split(".");
                bool ok;
                tudat = 0;
                for(int i=0;(i<qstrlist.length()&&i<4);i++)
                {
                    udat = qstrlist.at(i).toInt(&ok,10);
                    if(ok)
                    {
                        udat<<=(8*i);
                        tudat |= udat;
                    }
                }
                *(u32*)valaddr = tudat;
				//UI_WRITE_SINGLE_CFG(*(u32*)valaddr,tudat);
			}
			//*((u32 *)valaddr) = udat;            
			break;
		#endif
		case   pu8_nH_type		:
			break;	//n个数据 hex格式显示
		default:
			ret = 0;
			break;
	}
	return ret;
}

void SerialScreen_DataClean(int index)
{
	sSCREEN_DEBUGPROMSG("------DataClean[%d]",index);
	LcdData.runData.bothChgFlg = 0;
	
	LcdData.gun[index].Unit_Price = 0;

    //LcdData.setData.servetype;							//后台类型
    //LcdData.setData.netmode;								//联网方式

   // LcdData.setData.ChargePasswd[6];					//充电密码
    //LcdData.setData.AdmindPasswd[10];					//管理员密码


	//LcdData.setData.sup_parallelchg = Tcu_Flash.s.BothChgSupport;
	//LcdData.setData.sup_AUXpower = Tcu_Flash.s.DcSelSupport;						//辅源支持
	//LcdData.setData.sup_usecard = Tcu_Flash.s.idCardSupport;							//刷卡支持
	//LcdData.setData.sup_Qrcode = Tcu_Flash.s.ErWeiIsCodeSupport;						//APP支持
	//LcdData.setData.sup_VIN = Tcu_Flash.s.VinSupport;								//VIN码支持
	//LcdData.setData.sup_net = Tcu_Flash.s.WlanSupport;								//网络支持
	//LcdData.setData.sup_elelock = 1;							//电子锁支持
	//LcdData.setData.elelockLogic;						//电子锁逻辑

/*
	if(Tcu_Flash.s.DcSelSupport==0)
	{
		LcdData.gun[index].Aux12v = 0;
	}
	else
	{
		LcdData.gun[index].Aux12v = 1;
	}
*/	
	//Tcu_Flash.s.idCardSupport = 0;
	//Tcu_Flash.s.CardType = 1;
//	tcuMain_Obj.St[index].Set_St_DCMode = LcdData.gun[index].Aux12v;

//	mem_set(LcdData.gun[index].code_stopResaon,0, sizeof(LcdData.gun[index].code_stopResaon));
	
	LcdData.gun[index].startCountTimer = 90;
	SerialScreen_ScreenSet_CouDownFin_Flag(FALSE);
	/** 此处清除启动倒计时结束标志 */
}

//1 : online  0:offline
void SerialScreen_SendIco(struct SerialScreenObj *cmd,u16 addr, u16 par)
{
	memset(LcdTxData.buf,0,sizeof(LcdTxData.buf));
    LcdTxData.buf[0] = DWIN_FRAM_HEAD1;
    LcdTxData.buf[1] = DWIN_FRAM_HEAD2;
    LcdTxData.buf[2] = 0x05;
    LcdTxData.buf[3] = 0x82;
    LcdTxData.buf[4] = (u8)(addr >> 8);//0x60
    LcdTxData.buf[5] = (u8)addr;	//0x01
    LcdTxData.buf[6] = (u8)(par >> 8);
    LcdTxData.buf[7] = (u8)par;
    LcdTxData.len = 8;

    cmd->SendData(cmd, LcdTxData.buf, LcdTxData.len);
}


void SerialScreen_SendTxt(struct SerialScreenObj *cmd,u16 addr, u8 *buf, u8 len)
{    
    LcdTxData.buf[0] = DWIN_FRAM_HEAD1;
    LcdTxData.buf[1] = DWIN_FRAM_HEAD2;
    LcdTxData.buf[2] = len + 5;
    LcdTxData.buf[3] = 0x82;
    LcdTxData.buf[4] = (u8)(addr >> 8);
    LcdTxData.buf[5] = (u8)addr;
	mem_cpy(&LcdTxData.buf[6], buf,len);
    LcdTxData.buf[6 + len] = 0XFF;
    LcdTxData.buf[7 + len] = 0XFF;
    LcdTxData.len = 8 + len;

    cmd->SendData( cmd, LcdTxData.buf, LcdTxData.len);
}


void SerialScreen_JumpPage(struct SerialScreenObj *cmd,u8 page)
{
    LcdTxData.buf[0] = DWIN_FRAM_HEAD1;
    LcdTxData.buf[1] = DWIN_FRAM_HEAD2;
    LcdTxData.buf[2] = 0x07;
    LcdTxData.buf[3] = 0x82;
    LcdTxData.buf[4] = 0x00;
    LcdTxData.buf[5] = 0x84;
    LcdTxData.buf[6] = 0x5A;
    LcdTxData.buf[7] = 0x01;
    LcdTxData.buf[8] = 0x00;
    LcdTxData.buf[9] = page;
    LcdTxData.len = 10;

    cmd->SendData( cmd ,LcdTxData.buf, LcdTxData.len);
}

void SerialScreen_SendData(struct SerialScreenObj *cmd, u16 addr, u32 value)
{

    LcdTxData.buf[0] = DWIN_FRAM_HEAD1;
    LcdTxData.buf[1] = DWIN_FRAM_HEAD2;
    LcdTxData.buf[2] = 7;

    LcdTxData.buf[3] = 0x82;

	LcdTxData.buf[4] = (u8)(addr>>8);
	LcdTxData.buf[5] = (u8)addr;
	
	LcdTxData.buf[6] = (u8)(value >> 24);
	LcdTxData.buf[7] = (u8)(value >> 16);
	LcdTxData.buf[8] = (u8)(value >> 8);
	LcdTxData.buf[9] = (u8)value;
    LcdTxData.len = 10;

    cmd->SendData( cmd ,LcdTxData.buf, LcdTxData.len);
}

void SerialScreen_ReadData(struct SerialScreenObj *cmd, u16 addr, u32 len)
{

    LcdTxData.buf[0] = DWIN_FRAM_HEAD1;
    LcdTxData.buf[1] = DWIN_FRAM_HEAD2;
    LcdTxData.buf[2] = 4;

    LcdTxData.buf[3] = 0x83;

    LcdTxData.buf[4] = (u8)(addr>>8);
    LcdTxData.buf[5] = (u8)addr;

    LcdTxData.buf[6] = (u8)len;
    LcdTxData.len = 7;

    cmd->SendData( cmd ,LcdTxData.buf, LcdTxData.len);
}

void SerialScreen_RtcShow(struct SerialScreenObj *cmd)
{
    u8 rtc_buf[8];
	u8 rtc_len;
    memset(rtc_buf, 0x00, sizeof(rtc_buf));
	#if 0
	cp56time2a_t RTC_time = get_cp56time2a();
	u32 timestamp = get_timestamp_s();
	struct tm *t = localtime((const time_t*)&timestamp);
	#endif

	struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    struct tm tsystm,*systm = &tsystm;
    localtime_r(&tv.tv_sec,systm);

    rtc_buf[0] = DWIN_FRAM_HEAD1;
    rtc_buf[1] = DWIN_FRAM_HEAD2;
	rtc_buf[2] =(systm->tm_year - 100);
	rtc_buf[3] = systm->tm_mon + 1;
	rtc_buf[4] = systm->tm_mday;
	rtc_buf[5] = systm->tm_hour;
	rtc_buf[6]  = systm->tm_min;
	rtc_buf[7]  = systm->tm_sec;
	rtc_len = 8;
    SerialScreen_SendTxt(cmd, 0x9C, rtc_buf, rtc_len);
}

#ifdef CP_CONFIG_USING_QBJ
static void SerialScreen_ScreenLight(struct SerialScreenObj *cmd)
{
    u8 buf[14];

    memset(buf, 0x00, sizeof(buf));

    buf[0] = DWIN_FRAM_HEAD1;
    buf[1] = DWIN_FRAM_HEAD2;
    buf[2] = 0x0B;
    buf[3] = 0x82;
    buf[4] = 0x00;
    buf[5] = 0xD4;
    buf[6] = DWIN_FRAM_HEAD1;
    buf[7] = DWIN_FRAM_HEAD2;
    buf[8] = 0x00;
    buf[9] = 0x04;
    buf[10] = (u8)(SCREEN_LIGHTSCREEN_LOCATION_X >>8);
    buf[11] = (u8)(SCREEN_LIGHTSCREEN_LOCATION_X);
    buf[12] = (u8)(SCREEN_LIGHTSCREEN_LOCATION_Y >>8);
    buf[13] = (u8)(SCREEN_LIGHTSCREEN_LOCATION_Y);

    cmd->SendData(cmd ,buf, sizeof(buf));
}
#endif /* CP_CONFIG_USING_QBJ */

#ifdef SCREEN_USING_TXT_RTC
static void SerialScreen_RtcTxtSend(struct SerialScreenObj *cmd)
{
    time_t _time = 0;
    struct tm _tm;
    u32 CurrTick = 0;

    if(LcdData.setData.ScreenBaseTime != 0){
        CurrTick = thaisen_app_get_system_tick();
        _time = LcdData.setData.ScreenBaseTime;
        if(LcdData.setData.ScreenBaseTick > CurrTick){
            _time += ((CurrTick + 0xFFFFFFFF - LcdData.setData.ScreenBaseTick) /1000);
        }else{
            _time += ((CurrTick - LcdData.setData.ScreenBaseTick) /1000);
        }
    }else{
        _time = time(NULL);
    }

    localtime_r(&_time, &_tm);
    sprintf((char*)LcdData.setData.rtc_time, "%d-%02d-%02d %02d:%02d:%02d", (_tm.tm_year + 1900), (_tm.tm_mon + 1), \
            _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);

    SerialScreen_SendTxt(cmd, SCREEN_TXT_RTC_ADDR, LcdData.setData.rtc_time, strlen((char*)LcdData.setData.rtc_time));
}
#endif /* SCREEN_USING_TXT_RTC */

void SerialScreen_ClearPageReset()
{
	LcdData.NeedMenuOffFlg = 0;
	LcdData.NeedMenuOffTimer = 0;
}


void SerialScreen_PageNeedRefresh(int index)
{
	SerialScreen_ClearPageReset();
    LcdData.CurrentPageBack = 0;
}



s32 SerialScreen_RecvFifo(struct LCD_DATA_FIFO_TYPE *dataFifo,u8 *data, u32 length)
{

    if(length <= 0)
        return 0;//

    if(dataFifo == NULL)
        return 0;//

    int size = length;
    struct LCD_DATA_FIFO_TYPE *rx_fifo = dataFifo;

    /* read from software FIFO */
    while (length)
    {
        int ch;

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == FALSE))
        {
            /*no data*/
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= sizeof(rx_fifo->buffer)) 
            rx_fifo->get_index = 0;

        if (rx_fifo->is_full == TRUE)
        {
            rx_fifo->is_full = FALSE;
        }

        *data = ch & 0xff;
        data ++; 
		length --;
    }
    return size - length;
}


struct LCD_DATA_FIFO_TYPE *LCD_FifoOpen(void)
{
    struct LCD_DATA_FIFO_TYPE *pDataFifo;
    pDataFifo = (struct LCD_DATA_FIFO_TYPE *)malloc(sizeof(struct LCD_DATA_FIFO_TYPE));
	LcdRxData.pDataFifo = pDataFifo;
    memset(pDataFifo, 0, sizeof(struct LCD_DATA_FIFO_TYPE));
    return LcdRxData.pDataFifo;
}

s8 SerialScreen_MakeFrame(u8 ucByte)
{
    switch(LcdRxData.rstep)
    {
    case LCD_DATA_STEP_HEAD:
        if(ucByte == DWIN_FRAM_HEAD1 && 0 == LcdRxData.rlen)
        {
            LcdRxData.rxbuf[0] = ucByte;
            LcdRxData.rlen = 1;                   //已扫描的数据帧长度从1开始计数
        }
        else if(ucByte == DWIN_FRAM_HEAD2 && 1 == LcdRxData.rlen)
        {
            LcdRxData.rxbuf[1] = ucByte;
            LcdRxData.rlen = 2;                   //已扫描的数据帧长度从1开始计数
            LcdRxData.rstep = LCD_DATA_STEP_LEN;  //帧头已找到，下一步寻找帧长度域
        }
        break;
    case LCD_DATA_STEP_LEN:
        if(LcdRxData.rlen == 2)//判断收到数据长度
        {
            LcdRxData.rxbuf[2] = ucByte;
            if(LcdRxData.rxbuf[2] > sSCREEN_RX_CMD_MAX_LEN)   // 数据长度超过一定值(暂定15)时不予接收，此时的长度字节可能是不对的
            {
                return -1;            //  返回负值是接收错误
            }

            LcdRxData.rlen++;
            LcdRxData.tlen = LcdRxData.rxbuf[2] + 3;//数据帧总长度从1开始计数
            LcdRxData.rstep = LCD_DATA_STEP_DATA;
        }
        break;
    case LCD_DATA_STEP_DATA:
        LcdRxData.rxbuf[LcdRxData.rlen]  = ucByte;
        LcdRxData.rlen++;

        if(LcdRxData.rlen > (LcdRxData.tlen - 1))
        {
            return TRUE;  // 返回正值是一帧数据已接收完成
        }
        break;
    }

    return FALSE;  // 返回0是正在组帧
}


u8 SerialScreen_Read(void)
{
    u8 ucByte;
    s8 result = 0;
    u32 start_tick = thaisen_app_get_system_tick();

    LcdRxData.rstep = LCD_DATA_STEP_HEAD;
    LcdRxData.tlen = 0;
    LcdRxData.rlen = 0;

    if(LcdRxData.pDataFifo->get_index == LcdRxData.pDataFifo->put_index)   /* 没有数据 */
    {
        return 0;
    }


    while(1)
    {
        while(SerialScreen_RecvFifo(LcdRxData.pDataFifo ,&ucByte, 1) != 1)
        {
            if(start_tick > thaisen_app_get_system_tick())
            {
                start_tick = thaisen_app_get_system_tick();
            }

            if(thaisen_app_get_system_tick() - start_tick > 20)  // 一个完整的帧两个字节之间的时延不得超过一定时间(暂定20ms)，防止脏数据或数据丢失时造成错误
            {
                if(SerialScreen_RecvFifo(LcdRxData.pDataFifo ,&ucByte, 1) == 1)  // 预防正在接收中线程被抢占，回来时已超时
                {
                    break;
                }
                /* 接收数据错误 */
                LcdRxData.rstep = LCD_DATA_STEP_HEAD;
                LcdRxData.tlen = 0;
                LcdRxData.rlen = 0;
                return 0;
            }

        }

        result = SerialScreen_MakeFrame(ucByte);

        if(result == TRUE)
        {
            return LcdRxData.tlen;
        }
        else if(result < 0)
        {
            /* 接收数据错误 */
            LcdRxData.rstep = LCD_DATA_STEP_HEAD;
            LcdRxData.tlen = 0;
            LcdRxData.rlen = 0;
            return 0;
        }
    }
}

struct LCD_DATA_FIFO_TYPE *SerialScreen_Init(struct SerialScreenObj *cmd)
{
#define SERIALSCREEN_AMMETER_BAUD_2400       0      //电表波特率：2400
#define SERIALSCREEN_AMMETER_BAUD_4800       1      //电表波特率：4800
#define SERIALSCREEN_AMMETER_BAUD_9600       2      //电表波特率：9600
#define SERIALSCREEN_AMMETER_BAUD_38400      3      //电表波特率：38400
#define SERIALSCREEN_AMMETER_BAUD_115200     4      //电表波特率：115200

    u32 _tick = 0;
    u8 len = 0, count = 0, *data, entry = 0, baud = 0;
	memset(&LcdRxData,0,sizeof(LcdRxData));
	//LcdData init
    LcdData.gunIndex = 0;
	LcdData.menuflg = 0;
	LcdData.runData.netstate = thaisen_app_get_net_state(); //
	LcdData.Homeflg = 1;
	LcdData.CurrentPage = LCD_PAGE_STANDBY;
    LcdData.CurrentPageBack = 0;
    LcdData.PageCountDown = thaisen_app_get_system_tick();
	sSCREEN_DEBUGPROMSG("LcdData.setData.pileID==%s\r\n",(char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_PILE_NUMBER, 0)));
	str_ncpy((char *)(LcdData.runData.pileID), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_PILE_NUMBER, 0)), \
	         sizeof(LcdData.runData.pileID));
	str_ncpy((char *)LcdData.setData.pileID,(char *)(LcdData.runData.pileID),sizeof(LcdData.setData.pileID));
	sSCREEN_DEBUGPROMSG("LcdData.setData.pileID==%s\r\n",LcdData.runData.pileID);

	data = thaisen_app_get_qrcode_prefix(&len);
    if(len && data){
        str_ncpy((char *)(LcdData.setData.ErWeiCodePre), data, len);
    }

    for(int i=0;i<LCD_GUN_NUM;i++)
    {
        thaisen_get_device_sn((char*)LcdData.runData.chgcode[i], sizeof(LcdData.runData.chgcode[i]), i);
        LcdData.pPageIndex[i] = &LCD_ALL_PAGE_TAB[0];
    }

    len = *(UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SCREEN_PASSWORD, 1));
    data = UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SCREEN_PASSWORD, 0);

    str_ncpy((char *)LcdData.setData.UserPasswdShow,UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SCREEN_PASSWORD, 0),sizeof(LcdData.setData.UserPasswd));
	str_ncpy((char *)LcdData.setData.App_SoftWareVersion,(char *)(thaisen_app_get_app_version()->version), strlen((char *)(thaisen_app_get_app_version()->version)));
	str_ncpy((char *)(LcdData.setData.Help_Number), (char *)(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_HELP_PHONE, 0)), \
					 sizeof(LcdData.setData.Help_Number));	
	LcdData.setData.sup_Local = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_LOCAL, 0));
    LcdData.setData.Sup_PlugAndPlay = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PLUGCHARGE, 0));
    LcdData.setData.sup_Local_stop = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_LOCAL_STOP, 0));
	LcdData.setData.sup_insulation = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_INSULATION, 0));
	LcdData.setData.sup_VIN =*((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_VIN, 0));
    LcdData.setData.sup_usecard =*((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_CARD, 0));
    LcdData.setData.AllocWay = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_ALLOCATION_WAY, 0));
    LcdData.setData.DevType = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_DEVICE_TYPE, 0));
    LcdData.setData.NetType = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_NET_TYPE, 0));
    LcdData.setData.GunVolt_LimitValue = *((u16*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_GUNVOLT_LIMIT, 0));
    LcdData.setData.sup_auxp_24V = *(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_SUPORT_AUXPOWER24V, 0));
    LcdData.setData.MeterModel = *(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_METER_MODEL, 0));
    LcdData.setData.MeterCheckWay = *(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_METER_CHECK_WAY, 0));
    LcdData.setData.MeterBaudrate = *(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_METER_BAUDRATE, 0));
    LcdData.setData.sup_parallelchg = *(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_SUPORT_PARALLEL, 0));
    LcdData.setData.sup_parallelrelay = *(UI_READ_SINGLE_CFG_STR(CONFIG_ITEM_SUPORT_PARALLELRELAY, 0));
    LcdData.setData.sup_mslience = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_MODULE_SLIENCE, 0));
    LcdData.setData.sup_offbilling = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, 0));
    LcdData.setData.sup_pw_start = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_PASSWORD_START, 0));
    LcdData.setData.sup_offline_card = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_SUPORT_OFFLINE_CARD, 0));
    LcdData.setData.supin_ac = *((u8*) UI_READ_SINGLE_CFG_DATA(CONFIG_ITEM_INEN_ACRELAY, 0));

    memset(LcdData.setData.UserPasswdShow, '\0', sizeof(LcdData.setData.UserPasswdShow));
    memcpy(LcdData.setData.UserPasswdShow, data, sizeof(LcdData.setData.UserPasswdShow));
    if(len == 0){
        memset(LcdData.setData.UserPasswdShow, '\0', sizeof(LcdData.setData.UserPasswdShow));
        memcpy(LcdData.setData.UserPasswdShow, "0909", strlen("0909"));
    }else{
        if(len > sizeof(LcdData.setData.UserPasswdShow)){
            len = sizeof(LcdData.setData.UserPasswdShow);
        }
        for(count = 0; count < len; count++){
            if((data[count] < 0x20) || (data[count] > 0x7E)){
                if((data[count] == 0) && (count != 0)){
                    memset(LcdData.setData.UserPasswdShow, '\0', sizeof(LcdData.setData.UserPasswdShow));
                    memcpy(LcdData.setData.UserPasswdShow, data, count);
                }else{
                    memset(LcdData.setData.UserPasswdShow, '\0', sizeof(LcdData.setData.UserPasswdShow));
                    memcpy(LcdData.setData.UserPasswdShow, "0909", strlen("0909"));
                }
                break;
            }
        }
    }

    len = strlen((char*)(LcdData.setData.UserPasswdShow));
    memcpy(LcdData.setData.UserPasswd, LcdData.setData.UserPasswdShow, sizeof(LcdData.setData.UserPasswdShow));
    memset(LcdData.setData.UserPasswdShow, '*', len);

    SerialScreen_InitInfo_Pro();

    SerialScreen_BtnModuleGet();
	SerialScreen_IsSupportGet();

	SerialScreen_BtnMeterNoInfoGet();
	SerialScreen_SetMeterInfo();
	SerialScreen_BtnVinListGet();

    if(LcdData.setData.sup_auxp_24V > TRUE)         /* 24V辅源默认不启用 */
        LcdData.setData.sup_auxp_24V = FALSE;

    if(LcdData.setData.sup_parallelchg > TRUE)      /* 并充默认不启用 */
        LcdData.setData.sup_parallelchg = FALSE;

	if(LcdData.setData.sup_Local > TRUE)         /* 本地启动默认不启用 */
        LcdData.setData.sup_Local = FALSE;

    if(LcdData.setData.sup_Local_stop > TRUE)         /* 本地停止默认不启用 */
        LcdData.setData.sup_Local_stop = FALSE;


    if(LcdData.setData.sup_VIN > TRUE)           /* VIN启动默认不启用 */
        LcdData.setData.sup_VIN = FALSE;

    if(LcdData.setData.sup_insulation > TRUE)    /* 绝缘配置默认启用 */
        LcdData.setData.sup_insulation = TRUE;

    if(LcdData.setData.sup_parallelrelay > TRUE)    /* 并联配置默认启用 */
        LcdData.setData.sup_parallelrelay = TRUE;

    if(LcdData.setData.sup_usecard > TRUE)           /* 读卡器配置默认不启用 */
        LcdData.setData.sup_usecard = FALSE;

    if(LcdData.setData.sup_mslience > TRUE)       /* 模块静音默认不启用 */
        LcdData.setData.sup_mslience = FALSE;

#ifdef SCREEN_USING_OFFLINE_BILLING
    if(LcdData.setData.sup_offbilling > TRUE)       /* 离线计费默认不启用 */
        LcdData.setData.sup_offbilling = FALSE;
#else
    LcdData.setData.sup_offbilling = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */

    if(LcdData.setData.sup_pw_start > TRUE)        /* 密码启动默认不启用 */
        LcdData.setData.sup_pw_start = FALSE;

    if(LcdData.setData.sup_offline_card > TRUE)        /* 离线刷卡默认启用 */
        LcdData.setData.sup_offline_card = TRUE;

    if(LcdData.setData.Sup_PlugAndPlay > TRUE)       /* 即插即充默认不启用 */
        LcdData.setData.Sup_PlugAndPlay = FALSE;

    if(LcdData.setData.AllocWay >= POWER_ALLOCATION_WAY_SIZE){        /* 功率分配默认使用先到先得 */
        LcdData.setData.AllocWay = POWER_ALLOCATION_WAY_SEQ_PRIORITY;
    }
    LcdData.setData.AllocWay = POWER_ALLOCATION_WAY_AVERAGE;
    /* 此处要设置功率分配方式 */
//    thaisenSetAllocateStrategy(LcdData.setData.AllocWay);

    if(LcdData.setData.MeterModel > thaisenAmmeterModel_Other)
        LcdData.setData.MeterModel = thaisenAmmeterModel_RuiYin;

    thaisen_set_ammnterModel(LcdData.setData.MeterModel);

    if((LcdData.setData.MeterCheckWay < CP_AMMETER_CHECK_WAY_EVEN) ||\
            (LcdData.setData.MeterCheckWay > CP_AMMETER_CHECK_WAY_NONE)){
        LcdData.setData.MeterCheckWay = CP_AMMETER_CHECK_WAY_EVEN;
    }
    switch(LcdData.setData.MeterCheckWay){
    case CP_AMMETER_CHECK_WAY_EVEN:
        thaisen_set_ammeterCheckWay(THAISEN_CHECK_WAY_EVEN);
        break;
    case CP_AMMETER_CHECK_WAY_ODD:
        thaisen_set_ammeterCheckWay(THAISEN_CHECK_WAY_ODD);
        break;
    case CP_AMMETER_CHECK_WAY_NONE:
        thaisen_set_ammeterCheckWay(THAISEN_CHECK_WAY_NONE);
        break;
    }

    if((LcdData.setData.MeterBaudrate < CP_AMMETER_BAUDRATE_9600) ||\
            (LcdData.setData.MeterBaudrate > CP_AMMETER_BAUDRATE_115200)){
        LcdData.setData.MeterBaudrate = CP_AMMETER_BAUDRATE_9600;
    }
    switch(LcdData.setData.MeterBaudrate){
    case CP_AMMETER_BAUDRATE_2400:
        baud = SERIALSCREEN_AMMETER_BAUD_2400;
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_2400);
        break;
    case CP_AMMETER_BAUDRATE_4800:
        baud = SERIALSCREEN_AMMETER_BAUD_4800;
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_4800);
        break;
    case CP_AMMETER_BAUDRATE_9600:
        baud = SERIALSCREEN_AMMETER_BAUD_9600;
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_9600);
        break;
    case CP_AMMETER_BAUDRATE_38400:
        baud = SERIALSCREEN_AMMETER_BAUD_38400;
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_38400);
        break;
    case CP_AMMETER_BAUDRATE_115200:
        baud = SERIALSCREEN_AMMETER_BAUD_115200;
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_115200);
        break;
    default:
        baud = SERIALSCREEN_AMMETER_BAUD_9600;
        thaisen_set_ammeterBaudrate(THAISEN_BAUDRATE_9600);
        break;
    }
    LcdData.setData.MeterBaudrate = baud;
    rt_kprintf("MeterModel(%d)  MeterCheckWay(%d)  MeterBaudrate(%d)\n", LcdData.setData.MeterModel, LcdData.setData.MeterCheckWay, LcdData.setData.MeterBaudrate);

    if((LcdData.setData.GunVolt_LimitValue < GUNVOLT_LIMIT_VALUE_MIN) || (LcdData.setData.GunVolt_LimitValue > GUNVOLT_LIMIT_VALUE_MAX)){
        LcdData.setData.GunVolt_LimitValue = GUNVOLT_LIMIT_VALUE_MIN;
    }
    thaisen_set_ChargGunVolt((LcdData.setData.GunVolt_LimitValue /10), 0);
    thaisen_set_ChargGunVolt((LcdData.setData.GunVolt_LimitValue /10), 1);

    rt_kprintf("LcdData.setData.GunVolt_LimitValue(%d)\n", LcdData.setData.GunVolt_LimitValue);
	sSCREEN_DEBUGPROMSG("LcdData.setData.App_SoftWareVersion==%s\r\n",(char *)LcdData.setData.App_SoftWareVersion);
	sSCREEN_DEBUGPROMSG("LcdData.setData.sup_Local=%d sup_VIN=%d\r\n",LcdData.setData.sup_Local, LcdData.setData.sup_VIN);
	
	LcdAssistantData.Flag.IsEnableAuxPower24V = 0;
    if(LcdData.setData.sup_auxp_24V == TRUE){
        LcdAssistantData.Flag.IsEnableAuxPower24V = 1;
    }
    LcdData.setData.s_selectaux[LCD_GUN_1] = ICON_AUXPOWER_NONE;
    LcdData.setData.s_selectaux[LCD_GUN_2] = ICON_AUXPOWER_NONE;

	for(int i=0;i<LCD_GUN_NUM;i++)
	{
	//	LcdData.gun[i].Aux12v = 1;
		LcdData.gun[i].Unit_Price = -0.31f;
		LcdData.gun[i].startCountTimer = 90;
	    SerialScreen_ScreenSet_CouDownFin_Flag(FALSE);
		LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_LOCAL][i] = ~LcdData.setData.sup_Local;
		if(TRUE == LcdData.setData.sup_VIN)
			LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_VIN][i] = ICON_CHARGE_VIN;
		else LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_VIN][i] = ICON_CHARGE_NULL;

        if(TRUE == LcdData.setData.sup_pw_start)
            LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_PW][i] = ICON_CHARGE_PW;
        else LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_PW][i] = ICON_CHARGE_NULL;

	    LcdAssistantData.SeveralGunFlag[i].DataIsVerify = FALSE;
	    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
	}

    LcdData.setData.Sup_Stop = ICON_CHARGE_NULL;
    LcdData.setData.Icon_SuplocalStop = FALSE;
	if(LcdData.setData.sup_Local_stop){
        LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
        LcdData.setData.Icon_SuplocalStop = TRUE;
	}

    LcdData.setData.Icon_SupPlugAndPlay = FALSE;
    if(LcdData.setData.Sup_PlugAndPlay){
        LcdData.setData.Icon_SupPlugAndPlay = TRUE;
    }
#ifdef SCREEN_USING_OFFLINE_BILLING
    LcdData.setData.Icon_SupOfflineBilling = FALSE;
    if(LcdData.setData.sup_offbilling){
        LcdData.setData.Icon_SupOfflineBilling = TRUE;
    }
#else
    LcdData.setData.Icon_SupOfflineBilling = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */

    LcdData.setData.Icon_SupPWStart = FALSE;
    if(LcdData.setData.sup_pw_start){
        LcdData.setData.Icon_SupPWStart = TRUE;
    }

    LcdData.setData.Icon_SupOffCard = FALSE;
    if(LcdData.setData.sup_offline_card){
        LcdData.setData.Icon_SupOffCard = TRUE;
    }

	LcdData.setData.SerialScreen_PassWordShow = FALSE;
	LcdData.setData.manufacturer = 1;//NULL

	if(LcdData.setData.DevType >= SYSTEM_FUNCTION_SIZE){        /* 设备类型默认双枪一体 */
        LcdData.setData.DevType = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
    }
    LcdAssistantData.DeviceType = LcdData.setData.DevType;

    switch(LcdData.setData.DevType){
    case SYSTEM_FUNCTION_DYNAMIC_SWITCH:
        thaisenSetChargGunRunType(thaisenDeviceType_average);
        break;
    default:
        LcdData.setData.DevType = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
        LcdAssistantData.DeviceType = SYSTEM_FUNCTION_AVERAGE_DOUBLE;
        thaisenSetChargGunRunType(thaisenDeviceType_doubleGun);
        break;
    }

    if(LcdData.setData.NetType >= CP_NETTYPE_SIZE){
        LcdData.setData.NetType = CP_NETTYPE_4G;
    }
	LcdAssistantData.Flag.IsEnableParaCharge = FALSE;
	LcdData.setData.parallel_iocn = ICON_CHARGEWAY_NONE;
	LcdAssistantData.Flag.ParaChargeSelect = FALSE;
	if(LcdData.setData.sup_parallelchg == TRUE){
	    LcdAssistantData.Flag.IsEnableParaCharge = TRUE;
	}

    if(LcdData.setData.sup_parallelrelay == FALSE){
        thaisenModuleSetParallelEnable(thaisenFunction_disable);
    }else{
        thaisenModuleSetParallelEnable(thaisenFunction_enable);
    }

#ifndef SCREEN_USING_DOUBLE_GUN
    if(LcdData.setData.supin_ac > TRUE)
        LcdData.setData.supin_ac = FALSE;

    thaisenSetACRelayEnableState(LcdData.setData.supin_ac);
#else
        if(LcdData.setData.supin_ac > TRUE)
            LcdData.setData.supin_ac = TRUE;
#endif /* SCREEN_USING_DOUBLE_GUN */

	Q_INIT(LcdData.List, u8, 250);
//	SerialScreen_RtcShow(cmd);
	SerialScreen_ReadData(cmd, 0x0010, 4);

	LcdRxData.rstep = LCD_DATA_STEP_HEAD;
	LcdRxData.pDataFifo = LCD_FifoOpen();

	while(entry <= 3){
	    if((thaisen_app_get_system_tick() - _tick) > 100){
	        _tick = thaisen_app_get_system_tick();
	        SerialScreen_JumpPage(cmd,LcdData.CurrentPage);
	        entry++;
	    }
	}

#undef SERIALSCREEN_AMMETER_BAUD_2400
#undef SERIALSCREEN_AMMETER_BAUD_4800
#undef SERIALSCREEN_AMMETER_BAUD_9600
#undef SERIALSCREEN_AMMETER_BAUD_38400
#undef SERIALSCREEN_AMMETER_BAUD_115200

	return LcdRxData.pDataFifo;
}


void SerialScreen_SaveKey(u16 regaddr,u16 regvalue,u8 *ptr,u16 len)
{
	sSCREEN_DEBUGPROMSG("KeyTimer=%d\r\n",LcdData.KeyTimer);
	{
		if(len>0)
		{
			LcdData.KeyReg = regaddr;
			LcdData.KeyVal = regvalue;
			mem_set(LcdData.KeyInput, 0, sizeof(LcdData.KeyInput));
			
            for(u32 i=0;(i<len)&&(i<sizeof(LcdData.KeyInput));i++)
			{
                LcdData.KeyInput[i] = *(ptr+i);
			}
            LcdData.KeyInputLen = len;

			sSCREEN_DEBUGPROMSG("KeyInputlen=%d,KeyInput=%s\r\n",str_len(LcdData.KeyInput),LcdData.KeyInput);
		}
		else if(LcdData.KeyTimer==0)
		{
			LcdData.KeyReg = regaddr;
			LcdData.KeyVal = regvalue;
			LcdData.KeyTimer = 0;

			sSCREEN_DEBUGPROMSG("KeyReg=%04x,KeyVal=%04x\r\n",LcdData.KeyReg,LcdData.KeyVal);
		}
	}
}




int SerialScreen_CheckFrame(struct SerialScreenObj *cmd, void *frameBuf,u16 *count) 
{
	u8 *ptr = NULL;
    u16 len = 0;
	
	if(NULL == frameBuf || NULL == cmd)
		return sSCREEN_BUF_ERROR;
	
    //检测地址
    ptr = (u8 *)frameBuf;
	
	if((*(ptr)!=DWIN_FRAM_HEAD1)||(*(ptr+1)!=DWIN_FRAM_HEAD2))	
	{
        //sSCREEN_DEBUGMSG("start addr =%02x\r\n",*(ptr));
		return sSCREEN_ADDR_ERROR;
	}

	len = *(ptr+2)+3;

	//sSCREEN_DEBUGMSG("total Len =%d,datalen =%d,cmd =%02x\r\n",len,*(ptr+2),*(ptr+3));
#if 0
	// 检测帧类型 ,根据不同命令确定实际的字节数
	switch(*(ptr+3))
	{
		case 0x81:
			break;
		case 0x83 :
			break;
		default:
			//return sSCREEN_CHK_ERROR;
			break;
	}
#endif
	//如果接收到字节数不够帧长度，返回错误
	if(len > *count)							
		return sSCREEN_CHK_LESS;

	//计算校验，完整帧计算结果为0，则正确
	//checkSum = cmd->CheckSum(ptr, len);		
	//if(0 != checkSum)
	//	return sSCREEN_CHK_ERROR;

	//返回实际处理的数据
	*count = len;
	
	return sSCREEN_CHK_OK;
	
}


void SerialScreen_PageReset(int GunIdx)
{
	if((LcdData.menuflg>0)||(LcdData.NeedMenuOffFlg>0)){
	    if((LcdData.gun[LCD_GUN_1].workState != SysMainStatus_StartReady) && (LcdData.gun[LCD_GUN_2].workState != SysMainStatus_StartReady)){
	        return;
	    }
	}

	if(LcdData.Homeflg>0) //首页标志
	{
		#if 0
		if((LcdData.CurrentPage == LCD_PAGE_FEE_INFO)
			||(LcdData.CurrentPage == LCD_PAGE_BILL_INFO))
		{
			return;
		}
		#endif
        if((LcdData.gun[LCD_GUN_1].workState != SysMainStatus_StartReady) && (LcdData.gun[LCD_GUN_2].workState != SysMainStatus_StartReady)){
            switch(LCD_GUN_NUM)//枪个数
            {
                case LCD_GUN_NUM:
#ifdef SCREEN_USING_OFFLINE_BILLING
                    if(LcdData.setData.sup_offbilling == FALSE){
                        LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
                    }
                    if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                        LcdData.CurrentPage = LCD_PAGE_STANDBY;
                    }
#else
                    LcdData.CurrentPage = LCD_PAGE_STANDBY;
#endif /* SCREEN_USING_OFFLINE_BILLING */
                break;
                default:
#ifdef SCREEN_USING_OFFLINE_BILLING
                    if(LcdData.setData.sup_offbilling == FALSE){
                        LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
                    }
                    if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                        LcdData.CurrentPage = LCD_PAGE_STANDBY;
                    }

#else
                    LcdData.CurrentPage = LCD_PAGE_STANDBY;
#endif /* SCREEN_USING_OFFLINE_BILLING */
                break;
            }
            return;
        }else{
            if(LcdData.gun[LCD_GUN_1].workState == SysMainStatus_StartReady){
                LcdData.gunIndex = LCD_GUN_1;
                LcdData.CurrentPage = LCD_PAGE_A_START;
                LcdData.Homeflg = 0;
                LcdData.menuflg = 0;
                LcdData.NeedMenuOffFlg = 0;
#ifdef SCREEN_USING_OFFLINE_BILLING
                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                    LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                }
                LcdTriggerEvent[LCD_GUN_1].Flag.IsWaitPay = FALSE;
                LcdTriggerEvent[LCD_GUN_1].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
            }else if(LcdData.gun[LCD_GUN_2].workState == SysMainStatus_StartReady){
                LcdData.gunIndex = LCD_GUN_2;
                LcdData.CurrentPage = LCD_PAGE_B_START;
                LcdData.Homeflg = 0;
                LcdData.menuflg = 0;
                LcdData.NeedMenuOffFlg = 0;
#ifdef SCREEN_USING_OFFLINE_BILLING
                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                    LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                }
                LcdTriggerEvent[LCD_GUN_2].Flag.IsWaitPay = FALSE;
                LcdTriggerEvent[LCD_GUN_2].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
            }else{
                switch(LCD_GUN_NUM)//枪个数
                {
                    case LCD_GUN_NUM:
                        LcdData.CurrentPage = LCD_PAGE_STANDBY;
                    break;

                    default:
                        LcdData.CurrentPage = LCD_PAGE_STANDBY;
                    break;
                }
                return;
            }
        }
	}

    if(LcdData.gun[LCD_GUN_1].workState == SysMainStatus_StartReady){
        LcdData.gunIndex = LCD_GUN_1;
        LcdData.CurrentPage = LCD_PAGE_A_START;
        LcdData.Homeflg = 0;
        LcdData.menuflg = 0;
        LcdData.NeedMenuOffFlg = 0;
#ifdef SCREEN_USING_OFFLINE_BILLING
        for(u8 i = 0; i < LCD_GUN_NUM; i++){
            LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
        }
        LcdTriggerEvent[LCD_GUN_1].Flag.IsWaitPay = FALSE;
        LcdTriggerEvent[LCD_GUN_1].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
        return;
    }else if(LcdData.gun[LCD_GUN_2].workState == SysMainStatus_StartReady){
        LcdData.gunIndex = LCD_GUN_2;
        LcdData.CurrentPage = LCD_PAGE_B_START;
        LcdData.Homeflg = 0;
        LcdData.menuflg = 0;
        LcdData.NeedMenuOffFlg = 0;
#ifdef SCREEN_USING_OFFLINE_BILLING
        for(u8 i = 0; i < LCD_GUN_NUM; i++){
            LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
        }
        LcdTriggerEvent[LCD_GUN_2].Flag.IsWaitPay = FALSE;
        LcdTriggerEvent[LCD_GUN_2].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
        return;
    }
	//主状态巡检
	switch(LcdData.gun[GunIdx].workState)
	{
		//空闲状态---处理离线与否的判定
		case SysMainStatus_StandBy:
		case SysMainStatus_PlugIn:
		switch(LcdData.CurrentPage)
		{
			case LCD_PAGE_NONE:
			case LCD_PAGE_A_START:
			case LCD_PAGE_B_START:
			case LCD_PAGE_A_ERR:
			case LCD_PAGE_B_ERR:
				LcdData.CurrentPage = LCD_PAGE_STANDBY;
				break;
            case LCD_PAGE_A_ACOUNT:
            case LCD_PAGE_B_ACOUNT:
                if(LcdData.gun[GunIdx].workState == SysMainStatus_PlugIn){
                    LcdData.CurrentPage = LCD_PAGE_STANDBY;
                }
                break;
			default:
				break;
		}
        break;
		//过程状态的数据处理
		case SysMainStatus_StartReady: //开始启动
			{
				if(GunIdx==LCD_GUN_1)
				{
					switch(LcdData.CurrentPage)
					{
						case LCD_PAGE_STANDBY:
						case LCD_PAGE_A_SELECT:
                            LcdData.CurrentPage = LCD_PAGE_A_START;
#ifdef SCREEN_USING_OFFLINE_BILLING
                            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                            }
                            LcdTriggerEvent[GunIdx].Flag.IsWaitPay = FALSE;
                            LcdTriggerEvent[GunIdx].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
							break;
						
						default:
							LcdData.CurrentPage = LCD_PAGE_A_START;
#ifdef SCREEN_USING_OFFLINE_BILLING
                            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                            }
                            LcdTriggerEvent[GunIdx].Flag.IsWaitPay = FALSE;
                            LcdTriggerEvent[GunIdx].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
							break;
					}
				}
				else
				{
					switch(LcdData.CurrentPage)
					{
						case LCD_PAGE_STANDBY:
						case LCD_PAGE_B_SELECT:
                            LcdData.CurrentPage = LCD_PAGE_B_START;
#ifdef SCREEN_USING_OFFLINE_BILLING
                            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                            }
                            LcdTriggerEvent[GunIdx].Flag.IsWaitPay = FALSE;
                            LcdTriggerEvent[GunIdx].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
                            break;

                        default:
                            LcdData.CurrentPage = LCD_PAGE_B_START;
#ifdef SCREEN_USING_OFFLINE_BILLING
                            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                            }
                            LcdTriggerEvent[GunIdx].Flag.IsWaitPay = FALSE;
                            LcdTriggerEvent[GunIdx].Flag.IsPayed = FALSE;
#endif /* SCREEN_USING_OFFLINE_BILLING */
                            break;
					}
				}
			}
			break;

		case SysMainStatus_SelfCheck:
			if(GunIdx==LCD_GUN_1)
				LcdData.CurrentPage = LCD_PAGE_A_START;
								
			else 
				LcdData.CurrentPage = LCD_PAGE_B_START; //预留
			break;

		case SysMainStatus_SelfCheck_Wait:
			if(GunIdx==LCD_GUN_1)
				LcdData.CurrentPage = LCD_PAGE_A_START;
			else 
				LcdData.CurrentPage = LCD_PAGE_B_START;//预留
			break;
		
		//停止状态......
		//充电中，都需要进行停止......
		case SysMainStatus_Chrging:
			if(GunIdx==LCD_GUN_1)
			{
#ifdef SCREEN_USING_OFFLINE_BILLING
                if(LcdData.setData.sup_offbilling == FALSE){
                    LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
                }
                if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                    if(LcdData.ChargingPageCountDown_Over == 0){
                        switch(LcdData.CurrentPage)
                        {
                            case LCD_PAGE_A_CHGING_BAT:
                                LcdData.CurrentPage = LCD_PAGE_A_CHGING_BAT;
                                break;
                            default:
                                LcdData.CurrentPage = LCD_PAGE_A_CHGING;
                                break;
                        }
                    }
                }
                LcdTriggerEvent[LCD_GUN_1].Flag.IsPayed = FALSE;
#else
                if(LcdData.ChargingPageCountDown_Over == 0){
                    switch(LcdData.CurrentPage)
                    {
                        case LCD_PAGE_A_CHGING_BAT:
                            LcdData.CurrentPage = LCD_PAGE_A_CHGING_BAT;
                            break;
                        default:
                            LcdData.CurrentPage = LCD_PAGE_A_CHGING;
                            break;
                    }
                }
#endif /* SCREEN_USING_OFFLINE_BILLING */
			}

			else 
			{
#ifdef SCREEN_USING_OFFLINE_BILLING
                if(LcdData.setData.sup_offbilling == FALSE){
                    LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
                }
                if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                    if(LcdData.ChargingPageCountDown_Over == 0){
                        switch(LcdData.CurrentPage)
                        {
                            case LCD_PAGE_B_CHGING_BAT:
                                LcdData.CurrentPage = LCD_PAGE_B_CHGING_BAT;
                                break;
                            default:
                                LcdData.CurrentPage = LCD_PAGE_B_CHGING;
                                break;
                        }
                    }
                }
                LcdTriggerEvent[LCD_GUN_2].Flag.IsPayed = FALSE;
#else
                if(LcdData.ChargingPageCountDown_Over == 0){
                    switch(LcdData.CurrentPage)
                    {
                        case LCD_PAGE_B_CHGING_BAT:
                            LcdData.CurrentPage = LCD_PAGE_B_CHGING_BAT;
                            break;
                        default:
                            LcdData.CurrentPage = LCD_PAGE_B_CHGING;
                            break;
                    }
                }
#endif /* SCREEN_USING_OFFLINE_BILLING */
			}
			break;
			
		case SysMainStatus_StopChg:
#ifdef SCREEN_USING_OFFLINE_BILLING
            if(LcdData.setData.sup_offbilling == FALSE){
                LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
            }
            if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                if(GunIdx==LCD_GUN_1){
                    LcdData.CurrentPage = LCD_PAGE_A_STOPING;
                }else{
                    LcdData.CurrentPage = LCD_PAGE_B_STOPING;//预留
                }
            }
#else
            if(GunIdx==LCD_GUN_1){
                LcdData.CurrentPage = LCD_PAGE_A_STOPING;
            }else{
                LcdData.CurrentPage = LCD_PAGE_B_STOPING;//预留
            }
#endif /* SCREEN_USING_OFFLINE_BILLING */
			break;
		
		//充电结算中...
		case SysMainStatus_Account:
		//停车收费状态
		case SysMainStatus_Parking:
		case SysMainStatus_ParkAccount:
		case SysMainStatus_Other:
#ifdef SCREEN_USING_OFFLINE_BILLING
            if(LcdData.setData.sup_offbilling == FALSE){
                LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
            }
			if(GunIdx==LCD_GUN_1){
                if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                    if((LcdTriggerEvent[GunIdx].Flag.IsPayed == FALSE) &&
                            (LcdTriggerEvent[GunIdx].Flag.IsWaitPay == TRUE) &&
                            (LcdData.setData.sup_offbilling == TRUE)){
                        SerialScreen_TriggerItem_Repeat_WaitPay(LCD_GUN_1);
                        LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = TRUE;
                        break;
                    }
                    if(LcdData.AccountPageCountDown_Over == 0){
                        LcdData.CurrentPage = LCD_PAGE_A_ACOUNT;
                    }
                }
			}else{
                if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                    if((LcdTriggerEvent[GunIdx].Flag.IsPayed == FALSE) &&
                            (LcdTriggerEvent[GunIdx].Flag.IsWaitPay == TRUE) &&
                            (LcdData.setData.sup_offbilling == TRUE)){
                        SerialScreen_TriggerItem_Repeat_WaitPay(LCD_GUN_2);
                        LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = TRUE;
                        break;
                    }
                    if(LcdData.AccountPageCountDown_Over == 0){
                        LcdData.CurrentPage = LCD_PAGE_B_ACOUNT;
                    }
                }
			}
#else
            if(GunIdx==LCD_GUN_1){
                if(LcdData.AccountPageCountDown_Over == 0){
                    LcdData.CurrentPage = LCD_PAGE_A_ACOUNT;
                }
            }else{
                if(LcdData.AccountPageCountDown_Over == 0){
                    LcdData.CurrentPage = LCD_PAGE_B_ACOUNT;
                }
            }
#endif /* SCREEN_USING_OFFLINE_BILLING */
			break;
		case SysMainStatus_Err:
#ifdef SCREEN_USING_OFFLINE_BILLING
            if(LcdData.setData.sup_offbilling == FALSE){
                LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal = FALSE;
            }
            if(LcdTriggerEvent[GunIdx].Flag.IsTriggerExternal == FALSE){     /** 这些是由外部触发跳的页 */
                if(GunIdx==LCD_GUN_1)
                    LcdData.CurrentPage = LCD_PAGE_A_ERR;
                else
                    LcdData.CurrentPage = LCD_PAGE_B_ERR;
            }
#else
            if(GunIdx==LCD_GUN_1)
                LcdData.CurrentPage = LCD_PAGE_A_ERR;
            else
                LcdData.CurrentPage = LCD_PAGE_B_ERR;
#endif /* SCREEN_USING_OFFLINE_BILLING */
			break;			
			
		default: 
			break;
		
	}
}

void SerialScreen_Analyze(struct SerialScreenObj *cmd, u16 len)
{
	//sSCREEN_DEBUGDATA("------SerialScreen RxCmd=", cmd->rxptr, len);

	u16 regaddr=0,regvalue=0;
    u8 *ptr = NULL;
	
	ptr = (u8 *)cmd->rxptr;

	//len = *(ptr+2)+3;
	
	//sSCREEN_DEBUGMSG("total Len =%d,datalen =%d,cmd =%02x\r\n",len,*(ptr+2),*(ptr+3));

	switch(*(ptr+3))
	{
		case 0x81:
			//如果是页面信息 可能有变动 0x81
			if(*(ptr+2)==5)
			{
				regaddr = NetReadBigU16((ptr+4));
				if(regaddr==0x0302) 
				{
					regvalue = NetReadBigU16((ptr+6));
                    //page refesh
                    LcdData.CurrentPageBack = regvalue;
				}
			}
			break;
		case 0x83:
			regaddr = NetReadBigU16((ptr+4));
			regvalue = NetReadBigU16((ptr+7));
			
			//如果是键值
			if((*(ptr+2)==6)&&(regvalue!=0xffff))
			{
                SerialScreen_SaveKey(regaddr,regvalue,(u8*)"",0);
			}
			//键盘输入字符串
			else
			{
				regvalue = 0;
				SerialScreen_SaveKey(regaddr,regvalue,(ptr+7),*(ptr+6)*2);
			}
			
			break;
		default:
			break;
	}
}


void SerialScreen_Recvie(struct SerialScreenObj *cmd, u8 *buf, u16 len)
{
	s16 i;
	u16 used;
	u8 tbuf[sSCREEN_RX_CMD_MAX_LEN];
	u8 *ptr = tbuf,ch;
	u8 chkStatus = sSCREEN_CHK_ERROR;
	u8 flag = 0;

	if(len > 0)
	{
		sSCREEN_DEBUGRxDATA("------serialScreen RxCmd=", buf, len);
		for(i=0;i<len;i++)
		{
			LcdData.List.EnQueue(&LcdData.List,(void *)&buf[i]);//插入buf为队列的新元素
		}
		
	}
	
	len = 0;
	len = LcdData.List.Traverse(&LcdData.List,(void *)ptr,sSCREEN_RX_CMD_MAX_LEN);////遍历队列

	if(len<sSCREEN_RX_CMD_MIN_LEN)
		return;

	// Modbus通信数据处理
	for(i = 0; i < len - sSCREEN_RX_CMD_MIN_LEN; i++)
	{
		//剩余字节
		used = len - i;
		//进行帧判断,实际处理的字节数已更新
		chkStatus = SerialScreen_CheckFrame(cmd, ptr+i, &used);
		if(sSCREEN_CHK_OK == chkStatus)
		{
			if(used <= sSCREEN_RX_CMD_MAX_LEN && used >= sSCREEN_RX_CMD_MIN_LEN)
			{
				//获取完整的数据帧
				mem_cpy(cmd->rxptr, (ptr+i), used);
				i += used - 1;
				
				//数据处理 保存键值以及当前页备份
				cmd->Analyze(cmd,used);
				//
                for(int n=0;n<used;n++)
					LcdData.List.DeQueue(&LcdData.List,(void *)&ch,0);
			}
		}
	}
	
	//如果Modbus应答所有命令，则在上面处理完成
	//否则置位标志位，在此处处理最后一帧命令
	if(flag)
	{
		//cmd->Analyze(cmd,used);
	}
}

void SerialScreen_GetKeyProcess(struct SerialScreenObj *cmd)
{
	u16 keyreg,keyval;
	
	if(LcdData.KeyReg==0)
		return;
	
	keyreg = LcdData.KeyReg;
	keyval = LcdData.KeyVal;
	LcdData.KeyReg = 0;
	LcdData.KeyVal = 0;

	sSCREEN_DEBUGKEYMSG("GetKey LcdData.CurrentPage[%d]=%d,KeyReg=%04x,KeyVal=%04x\r\n",LcdData.gunIndex,LcdData.CurrentPage,keyreg,keyval);

	//当前页显示
    struct LCD_DISPLAY_PAGE_INDEX_TYPE *pPageIndex = LcdData.pPageIndex[LcdData.gunIndex];

    if(keyreg == 0x0010){               /* 读取RTC响应 */
        u8* ptr = (cmd->rxptr + 7);
        LcdData.setData.s_TimeSync[0] = ptr[0] + 2000;
        LcdData.setData.s_TimeSync[1] = ptr[1];
        LcdData.setData.s_TimeSync[2] = ptr[2];
        LcdData.setData.s_TimeSync[3] = ptr[4];
        LcdData.setData.s_TimeSync[4] = ptr[5];
        LcdData.setData.s_TimeSync[5] = ptr[6];

        SerialScreen_ScreenSet_TimeSync_Flag();
    }

    //离线计费页面
#ifdef SCREEN_USING_OFFLINE_BILLING
    if((LcdData.CurrentPage == LCD_PAGE_WARNNING_INFO) ||
            (LcdData.CurrentPage == LCD_PAGE_OB_PYA_A) ||
            (LcdData.CurrentPage == LCD_PAGE_OB_PYA_B)){
        if((keyreg == 0x1000) && (keyval == 0x0002)){
            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
            }
        }
    }
#endif /* SCREEN_USING_OFFLINE_BILLING */

    if((LcdData.CurrentPage == LCD_PAGE_MENU_MONITOR) ||
            (LcdData.CurrentPage == LCD_PAGE_MENU_MONITOR_B) ||
            (LcdData.CurrentPage == LCD_PAGE_MENU_STATE_MODULE) ||
            (LcdData.CurrentPage == LCD_PAGE_MENU_STATE_MODULE_B)){
        if((keyreg == 0x1000) && (keyval == 0x23)){
            for(u8 gunno = 0; gunno < LCD_GUN_NUM; gunno++){
                if((LcdData.gun[gunno].workState == SysMainStatus_StartReady) || (LcdData.gun[gunno].workState == SysMainStatus_Chrging)){
                    return;
                }
            }
        }
    }

    /** 屏幕密码显示控制 */
    if(LcdData.setData.SerialScreen_PassWordShow){
//        if(((keyreg == 0x1000) && (keyval == 0x1E)) ||         \
//                ((keyreg == 0x1000) && (keyval == 0x1F)) ||    \
//                ((keyreg == 0x1000) && (keyval == 0x20)) ||    \
//                ((keyreg == 0x1000) && (keyval == 0x21)) ||    \
//                ((keyreg == 0x1000) && (keyval == 0x53)) ||    \
//                ((keyreg == 0x1000) && (keyval == 0x50)) ||    \
//                ((keyreg == 0x1000) && (keyval == 0x14)) ||    \
//                ((keyreg == 0x1000) && (keyval == 0x02))){
//            LcdData.setData.SerialScreen_PassWordShow = 0;
//        }
        if(((keyreg == 0x1000) && (keyval == 0x1E)) ||         \
                ((keyreg == 0x1000) && (keyval == 0x20)) ||    \
                ((keyreg == 0x1000) && (keyval == 0x21)) ||    \
                ((keyreg == 0x1000) && (keyval == 0x50)) ||    \
                ((keyreg == 0x1000) && (keyval == 0x02))){
            u8 len = strlen((char*)(LcdData.setData.UserPasswd));
            LcdData.setData.SerialScreen_PassWordShow = 0;

            memset(LcdData.setData.UserPasswdShow, '\0', sizeof(LcdData.setData.UserPasswdShow));
            memset(LcdData.setData.UserPasswdShow, '*', len);
        }
    }else{
        if(LcdData.CurrentPage == LCD_PAGE_MENU_CONFIG){
            if(keyreg == 0x3928){
                LcdData.setData.SerialScreen_PassWordShow = 1;
            }
        }
    }

#ifndef SCREEN_USING_DOUBLE_GUN
//    if((keyreg == 0x1000) && ((keyval == 0x15) || (keyval == 0x26))){   /* 这是A枪/B枪切换按钮，单枪情况不响应此按钮，该按钮的地址和键值一体必须是唯一的 */
//        return;
//    }
#endif /* SCREEN_USING_DOUBLE_GUN */

    for(u32 i=0;i<sizeof(pPageIndex->item)/sizeof(pPageIndex->item[0]);i++)
	{
		if(pPageIndex->item[i].have_name == 0)
		{
			break;
		}
		else
		{
			#if 1
			//密码进菜单
			if(pPageIndex->item[i].type == LCD_inputPwdType)
			{
				//寄存器地址相同
				if(pPageIndex->item[i].regaddr == keyreg)
				{
					//进入菜单
                    s8 tbuf[10];
					struct tm pt;
					//getLocalCurTime_r(&pt);
					//sprintf(tbuf,"%02d%02d",pt.tm_hour,pt.tm_min);

					sSCREEN_DEBUGPROMSG("input %s==%d\r\n",LcdData.KeyInput,str_len(LcdData.KeyInput));
                    if((str_ncmp(LcdData.KeyInput, LcdData.setData.UserPasswd, strlen(LcdData.setData.UserPasswd))==0)
                            ||((str_ncmp((u8 *)pPageIndex->item[i].valaddr, LcdData.KeyInput, pPageIndex->item[i].reflash)==0)&&(str_len(LcdData.KeyInput)>5)))
					{
                        if(LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen == TRUE){
                            SerialScreen_PWStartCharge(LcdData.gunIndex);
                        }else{
                            if(LcdAssistantData.Flag.IsLongLiSerialScreen){
                                LcdData.CurrentPage = LCD_PAGE_SYS_INFO;
                            }else{
                                LcdData.CurrentPage = pPageIndex->item[i].vallen;
                            }
                        }
						 LcdData.menuflg = 1;
						 LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen = FALSE;
					}
					else
					{
						LcdData.CurrentPage = LCD_PAGE_PASWD_ERR;
						LcdData.menuflg = 1;

						#if 0
						if((str_ncmp(LcdData.KeyInput, LcdData.setData.UserPasswd, 5)==0)
								||((str_ncmp((u8 *)pPageIndex->item[i].valaddr, LcdData.KeyInput, pPageIndex->item[i].reflash)==0)&&(str_len(LcdData.KeyInput)>5)))
						{
							 LcdData.CurrentPage = LCD_PAGE_MENU_COM_5;
							 LcdData.menuflg = 1;
						}
						else
						#endif
							//passwd err
							SerialScreen_PageNeedRefresh(LcdData.gunIndex);
					}
					sSCREEN_DEBUGPROMSG("input next page=%d\r\n",LcdData.CurrentPage);

					break;
				}
			}
			else if(pPageIndex->item[i].type == LCD_InputType)
			{
				//寄存器地址相同
				if(pPageIndex->item[i].regaddr == keyreg)
				{
					if(pPageIndex->item[i].valaddr != NULL)
					{
						if(pPageIndex->item[i].valtype == menu_type)
						{
							*((u16 *)(pPageIndex->item[i].valaddr)) = keyval;
							sSCREEN_EVENT_DEBUGMSG("#########valaddr = %d\r\n",*((u16 *)(pPageIndex->item[i].valaddr)));
						}
						else
						{
						    if(LcdData.CurrentPage == LCD_PAGE_MENU_SYS ||
						            LcdData.CurrentPage == LCD_PAGE_MENU_MONITOR ||
						            LcdData.CurrentPage == LCD_PAGE_MENU_MONITOR_B ||
						            LcdData.CurrentPage == LCD_PAGE_MENU_PROTECT
#ifdef SCREEN_USING_OFFLINE_BILLING
						            || LcdData.CurrentPage == LCD_PAGE_OFFLINE_BILLING){
#else
						        ){
#endif /* SCREEN_USING_OFFLINE_BILLING */
						        SerialScreen_CombineData(pPageIndex->item[i].valtype,pPageIndex->item[i].valaddr,LcdData.KeyInput,4,(1 <<0));
						    }else{
	                            sSCREEN_EVENT_DEBUGMSG("#########valtype = %d vallen = %d  valaddr = %s  KeyInput = %s\r\n",pPageIndex->item[i].valtype,pPageIndex->item[i].vallen,pPageIndex->item[i].valaddr,LcdData.KeyInput);

	                            for(u32 i=0;(i<LcdData.KeyInputLen)&&(i<sizeof(LcdData.KeyInput));i++)
	                            {
	                                if(LcdData.KeyInput[i]==0xff){
	                                    LcdData.KeyInput[i] = 0;          //如果输入的是字符串，则帧的最后2字节是0xFF
	                                }
	                            }
	                            SerialScreen_StrToData(pPageIndex->item[i].valtype,pPageIndex->item[i].vallen,pPageIndex->item[i].valaddr,LcdData.KeyInput);
						    }
						}

					}
					
					sSCREEN_DEBUGPROMSG("LCD_InputType CurrentPage=%d\r\n",LcdData.CurrentPage);

					break;
				}
			}
			else 
			#endif
			if(pPageIndex->item[i].type == LCD_TrigType)
			{
				//寄存器地址相同，刷新界面，pPageIndex->item[i].vallen 为枪口号
				if((pPageIndex->item[i].regaddr == keyreg)&&(pPageIndex->item[i].reflash == keyval))
				{
					if(pPageIndex->item[i].valaddr != NULL)
					{
                        dofun_void pdofun = (dofun_void)pPageIndex->item[i].valaddr;
						//执行控件对应动作
                        (*pdofun)(pPageIndex->item[i].vallen);

						sSCREEN_DEBUGPROMSG("LCD_TrigType CurrentPage=%d\r\n",LcdData.CurrentPage);
						break;
					}
				}
			}
			//页面切换
			else if((pPageIndex->item[i].type == LCD_BtnType)
				||(pPageIndex->item[i].type == LCD_BtnAType)
				||(pPageIndex->item[i].type == LCD_BtnBType)
				||(pPageIndex->item[i].type == LCD_BtnHomeType))
			{
				if((pPageIndex->item[i].reflash == keyval)
					&&(pPageIndex->item[i].regaddr == keyreg))
				{
					if(pPageIndex->item[i].type == LCD_BtnAType)
					{
						LcdData.gunIndex = LCD_GUN_1;
						LcdData.Homeflg = 0;
						//启动时清数据
						if(LcdData.CurrentPage == LCD_PAGE_STANDBY)
						{
							sSCREEN_DEBUGPROMSG("##########key clean###########\r\n");
							SerialScreen_DataClean(LcdData.gunIndex);
						}
						if(!SerialScreen_IsCarConnect(LCD_GUN_1)){
						    break;
						}
						sSCREEN_DEBUGPROMSG("gunIndex1=%d\r\n",LcdData.gunIndex);
					}
					if(pPageIndex->item[i].type == LCD_BtnBType)
					{
						LcdData.gunIndex = LCD_GUN_2;
						LcdData.Homeflg = 0;
						//启动时清数据
						if(LcdData.CurrentPage == LCD_PAGE_STANDBY)
						{
							sSCREEN_DEBUGPROMSG("##########key clean###########\r\n");
							SerialScreen_DataClean(LcdData.gunIndex);
						}
                        if(!SerialScreen_IsCarConnect(LCD_GUN_2)){
                            break;
                        }
						sSCREEN_DEBUGPROMSG("gunIndex2=%d\r\n",LcdData.gunIndex);
					}

					if(pPageIndex->item[i].isOk != NULL)
					{
                        if(pPageIndex->item[i].isOk(LcdData.gunIndex)==0)
                        {
                        	sSCREEN_DEBUGPROMSG("##########not allow###########\r\n");
                        	return;
                        }
					}

					if(pPageIndex->item[i].type == LCD_BtnHomeType)
					{
						sSCREEN_DEBUGPROMSG("##########BtnHome Go to HomePage###########\r\n");
						LcdData.Homeflg = 1;
						//LcdData.gun[LcdData.gunIndex].stopTip = 0;
					}

					if(pPageIndex->item[i].type == LCD_Timeype)
					{
					    if(pPageIndex->item[i].valaddr != NULL)
					    {
					        ((void (*)(void))pPageIndex->item[i].valaddr)();
					    }
					}
#if 0
					//退出时，清数据
					if(pPageIndex->item[i].valaddr != NULL)
					{
                        if(((LcdData.CurrentPage == LCD_PAGE_SYS_INFO) ||
                                (LcdData.CurrentPage == LCD_PAGE_MENU_COM_5) ||
                                (LcdData.CurrentPage == LCD_PAGE_MENU_COM_6) ||
                                (LcdData.CurrentPage == LCD_PAGE_MENU_COM_6B) ||
                                (LcdData.CurrentPage == LCD_PAGE_MENU_COM_5B))
                                && LcdAssistantData.Flag.IsLongLiSerialScreen){
                            if(!((keyreg == 0x1000) && (keyval == 0x0017))){
                                dofun_void pdofun = (dofun_void)pPageIndex->item[i].valaddr;
                                (*pdofun)(LcdData.gunIndex);
                            }
                        }else{
                            dofun_void pdofun = (dofun_void)pPageIndex->item[i].valaddr;
                            (*pdofun)(LcdData.gunIndex);
                        }
						sSCREEN_DEBUGPROMSG("LCD_BtnType CurrentPage=%d\r\n",LcdData.CurrentPage);
					}
#endif /* 0 */
					//pPageIndex->item[i].vallen 页面
					//如果目标页面为0，未知
					{
					    u8 page_selected = 0;
						 //如果目标页面相同，则刷新
						 sSCREEN_DEBUGPROMSG("LCD_BtnType menuflg=%d CurrentPage[%d]=%d\r\n",LcdData.menuflg,LcdData.gunIndex,LcdData.CurrentPage);

						 if(LcdData.CurrentPage == LCD_PAGE_STANDBY){        /* 只有第一页能进入系统信息页面 */
                             if((keyreg == 0x1003) && (keyval == 0x0001)){
                                 LcdAssistantData.Flag.IsLongLiSerialScreen = 1;
                                  LcdData.CurrentPage = LCD_PAGE_ADMIN_PASWD;
                             }else if((keyreg == 0x1000) && (keyval == 0x0013)){
                                 LcdAssistantData.Flag.IsLongLiSerialScreen = 0;
                                 LcdData.CurrentPage = pPageIndex->item[i].vallen;
                             }else{
                                 LcdData.CurrentPage = pPageIndex->item[i].vallen;
                             }
                             page_selected = 1;
                         }

                         if(((LcdData.CurrentPage == LCD_PAGE_SYS_INFO) ||
                                 (LcdData.CurrentPage == LCD_PAGE_MENU_COM_5) ||
                                 (LcdData.CurrentPage == LCD_PAGE_MENU_COM_6) ||
                                 (LcdData.CurrentPage == LCD_PAGE_MENU_COM_6B) ||
                                 (LcdData.CurrentPage == LCD_PAGE_MENU_COM_5B))
                                 && LcdAssistantData.Flag.IsLongLiSerialScreen){
                             if(page_selected == 0){
                                 if((keyreg == 0x1000) && (keyval == 0x0017)){
                                     LcdData.CurrentPage = LCD_PAGE_ROOT_MAIN;
                                 }else{
                                     LcdData.CurrentPage = pPageIndex->item[i].vallen;
                                 }
                                 page_selected = 1;
                             }
                         }

                         if(LcdData.CurrentPage == LCD_PAGE_ADMIN_PASWD){
                             if(page_selected == 0){
                                 LcdData.CurrentPage = pPageIndex->item[i].vallen;
                                 if((keyreg == 0x1000) && (keyval == 0x0013)){        //输密码页面点击返回
                                     if(LcdAssistantData.Flag.IsLongLiSerialScreen){
                                         LcdData.CurrentPage = LCD_PAGE_STANDBY;
                                     }else if(LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen == TRUE){
                                         if(LcdData.gunIndex == LCD_GUN_1){
                                             LcdData.CurrentPage = LCD_PAGE_A_SELECT;
                                         }else{
                                             LcdData.CurrentPage = LCD_PAGE_B_SELECT;
                                         }
                                         LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen = FALSE;
                                     }
                                     LcdData.menuflg = 0;
                                 }
                                 page_selected = 1;
                             }
                         }else{
                             if(page_selected == 0){
                                 LcdData.CurrentPage = pPageIndex->item[i].vallen;
                                 page_selected = 1;
                             }
                         }

                         //退出时，清数据
                         if(pPageIndex->item[i].valaddr != NULL)
                         {
                             if(((LcdData.CurrentPage == LCD_PAGE_SYS_INFO) ||
                                     (LcdData.CurrentPage == LCD_PAGE_MENU_COM_5) ||
                                     (LcdData.CurrentPage == LCD_PAGE_MENU_COM_6) ||
                                     (LcdData.CurrentPage == LCD_PAGE_MENU_COM_6B) ||
                                     (LcdData.CurrentPage == LCD_PAGE_MENU_COM_5B))
                                     && LcdAssistantData.Flag.IsLongLiSerialScreen){
                                 if(!((keyreg == 0x1000) && (keyval == 0x0017))){
                                     dofun_void pdofun = (dofun_void)pPageIndex->item[i].valaddr;
                                     (*pdofun)(LcdData.gunIndex);
                                 }
                             }else{
                                 dofun_void pdofun = (dofun_void)pPageIndex->item[i].valaddr;
                                 (*pdofun)(LcdData.gunIndex);
                             }
                             sSCREEN_DEBUGPROMSG("LCD_BtnType CurrentPage=%d\r\n",LcdData.CurrentPage);
                         }

                         if(LcdAssistantData.SeveralGunFlag[LcdData.gunIndex].IsPWStartAuthen == TRUE){
                             LcdData.CurrentPage = LCD_PAGE_ADMIN_PASWD;
                         }

						 sSCREEN_DEBUGPROMSG("pPageIndex->item[i].vallen = %d\r\n",pPageIndex->item[i].vallen);
						 SerialScreen_PageReset(LcdData.gunIndex);
						 //
						 if(LcdData.CurrentPage == LcdData.CurrentPageBack)
						 {
						 	SerialScreen_PageNeedRefresh(LcdData.gunIndex);
						 }
						 sSCREEN_DEBUGPROMSG("==============next page[%d]=%d\r\n",LcdData.gunIndex,LcdData.CurrentPage );
					}
					break;
				}
			}
			
		}
	}
}






void SerialScreen_CurrentPageItem(struct SerialScreenObj *cmd,struct LCD_DISPLAY_PAGE_INDEX_TYPE *pPageIndex,u8 index,u8 state)
{
	u8 tbuf[250];
	if(pPageIndex->item[index].have_name == 0)
	{
        //break;
	}
	else
	{
		sSCREEN_DEBUGPROMSG("name[%d]=%s type =%d\r\n",index,pPageIndex->item[index].have_name,pPageIndex->item[index].type);
		
		switch(pPageIndex->item[index].type)
		{
			case LCD_IconType:
				if(state)
				{
					if(pPageIndex->item[index].reflash==0)
						break;
				}
				if(pPageIndex->item[index].valaddr != NULL)
				{
					SerialScreen_SendIco(cmd,pPageIndex->item[index].regaddr,*((u8 *)pPageIndex->item[index].valaddr));
				}
				break;
			case LCD_DataType:
				if(state)
				{
					if(pPageIndex->item[index].reflash==0)
						break;	
				}
				if(pPageIndex->item[index].valaddr != NULL)
				{
					sSCREEN_DEBUGMSG("--valaddr=%d\r\n",*((u32 *)(pPageIndex->item[index].valaddr)));
					SerialScreen_SendData(cmd,pPageIndex->item[index].regaddr,*((u32 *)(pPageIndex->item[index].valaddr)));
				}
				break;

			case LCD_TextType:
			case LCD_QRCodeType:
			case LCD_InputType:
				if(state)
				{
					if(pPageIndex->item[index].reflash==0)
						break;
				}
				if(pPageIndex->item[index].valaddr != NULL)
				{
					mem_set(tbuf, 0, sizeof(tbuf));
					if(SerialScreen_DataToStr(pPageIndex->item[index].valtype,pPageIndex->item[index].vallen,pPageIndex->item[index].valaddr,tbuf))
					{
						if(pPageIndex->item[index].valtype == menu_type)
						{
							SerialScreen_SendTxt(cmd,pPageIndex->item[index].regaddr,tbuf,2);
						}
						else
						{
                            if(LcdData.CurrentPage == LCD_PAGE_MENU_SYS ||
                                    LcdData.CurrentPage == LCD_PAGE_MENU_MONITOR ||
                                    LcdData.CurrentPage == LCD_PAGE_MENU_MONITOR_B ||
                                    LcdData.CurrentPage == LCD_PAGE_MENU_PROTECT ||
                                    LcdData.CurrentPage == LCD_PAGE_SYS_UPDATE
#ifdef SCREEN_USING_OFFLINE_BILLING
                                    || LcdData.CurrentPage == LCD_PAGE_OFFLINE_BILLING){
#else
                                ){
#endif /* SCREEN_USING_OFFLINE_BILLING */
                                u32 data = SerialScreen_GetData_WithType(pPageIndex->item[index].valtype, pPageIndex->item[index].valaddr);
                                SerialScreen_SendData(cmd, pPageIndex->item[index].regaddr, data);
                            }else{
                                sSCREEN_DEBUGMSG("--screen type %d=%d strlen=%d \r\n",index, pPageIndex->item[index].type, str_len(tbuf));
                                u16 string_len = str_len(tbuf);

                                if((LcdData.CurrentPage == LCD_PAGE_MENU_CONFIG) && ( pPageIndex->item[index].regaddr == 0x3928)){
                                    if(!LcdData.setData.SerialScreen_PassWordShow){
                                        memset(tbuf, '*', string_len);
                                    }
                                }

                                if(string_len == 0){
                                    SerialScreen_SendTxt(cmd,pPageIndex->item[index].regaddr,tbuf,pPageIndex->item[index].vallen);   /* 此目的是为了清空屏幕字符显示(传入长度参数为0时清空不了) */
                                }else{
                                    SerialScreen_SendTxt(cmd,pPageIndex->item[index].regaddr,tbuf,string_len);
                                }
                            }
						}
					}
				}
				break;
			default:
				break;
		}
	}
}



void SerialScreen_CurrentPageShow(struct SerialScreenObj *cmd,u8 state)
{
	for(int i=0;i<LCD_ALL_PAGE_TAB_LEN;i++)
	{
		if(LcdData.CurrentPage == LCD_ALL_PAGE_TAB[i].page)
		{
			LcdData.pPageIndex[LcdData.gunIndex]  = &LCD_ALL_PAGE_TAB[i];
			break;
		}
	}

	//当前页显示
    struct LCD_DISPLAY_PAGE_INDEX_TYPE *pPageIndex = LcdData.pPageIndex[LcdData.gunIndex];

	static u8 index=0;
	static u32 pagetimer=0;
	static u32 timeshow=0;

	//页面刷新
	if(state==0)
	{
		index = 0;
		pagetimer = 0;
		SerialScreen_JumpPage(cmd,pPageIndex->page);
	}
	
    //定时跳页
    if(pagetimer++>=1000/BASE_SYS_TIMER) //
    {
        pagetimer = 0;
        SerialScreen_JumpPage(cmd,pPageIndex->page);
        return;
    }

	if(pagetimer%100==0)
	{
		sSCREEN_DEBUGPROMSG("pagetimer==%d\r\n",pagetimer);
	}
    if(timeshow++>=600/BASE_SYS_TIMER)
    {
    	if(TRUE == LcdData.runData.syncTimeFlg)
    	{
			SerialScreen_RtcShow(cmd);
			LcdData.runData.syncTimeFlg = FALSE;
			sSCREEN_EVENT_DEBUGMSG("##################RTC Sync");
#ifdef SCREEN_USING_TXT_RTC
			LcdData.setData.ScreenBaseTick = thaisen_app_get_system_tick();
			LcdData.setData.ScreenBaseTime = time(NULL);
#endif /* SCREEN_USING_TXT_RTC */
		}	
        timeshow = 0;
       // sSCREEN_DEBUGKEYMSG("##################RTC Show");
    }
	
#if 0
	if(state==0)
	{
		for(int i=0;i<SIZEOF(pPageIndex->item);i++)
		{
			SerialScreen_CurrentPageItem(cmd,pPageIndex,i,state);
		}
	}
	else
#endif
	{
	    int i = index;
		index++;
		if(index >= SIZEOF(pPageIndex->item))
			index = 0;
	    if(pPageIndex->item[i].have_name == 0)
	    {
	        index = 0;
	    }
		state = 0;

		sSCREEN_DEBUGPROMSG("------index=%d\r\n", index);
		SerialScreen_CurrentPageItem(cmd,pPageIndex,i,state);
		
	}
}


void SerialScreen_CheckKey()
{
	if(LcdData.KeyTimer>0)
	{
		LcdData.KeyTimer--;
		sSCREEN_DEBUGPROMSG("---------------------------screen Tx ack=%d\r\n",LcdData.KeyTimer);
	}
}

void SerialScreen_CheckPageReset()
{
	if(LcdData.NeedMenuOffFlg)
	{
		//if(LcdData.NeedMenuOffTimer%10==0)
		//	sSCREEN_DEBUGMSG("NeedMenuOffTimer=%d\r\n",LcdData.NeedMenuOffTimer);
		if(LcdData.NeedMenuOffTimer>0)
		{
			LcdData.NeedMenuOffTimer--;
		}
		else
		{
			SerialScreen_PageNeedRefresh(LcdData.gunIndex);
		}
	}
}



int SerialScreen_DataProcess()
{
	int i,ret=0;
	static u32 timesecbak;
	u8 timeSync = FALSE;
//	u32 timesec = thaisen_app_get_current_timestamp();
	enum ofsm_state chargeState[LCD_GUN_NUM]; 
	struct charge_data *chargeInfo[LCD_GUN_NUM];
	struct bms_info *bmsInfo[LCD_GUN_NUM];
	struct temperature bmsTemp[LCD_GUN_NUM];
	static u8 LcdState[LCD_GUN_NUM]={0xFF,0xFF};
	static u32 nSocIcon = 0; 
#ifdef SCREEN_USING_QBJ
	static u8 ScreenLightCount = 0;
#endif /* SCREEN_USING_QBJ */

#ifdef SCREEN_USING_TXT_RTC
	static u32 RtcTxtTick = 0;
#endif /* SCREEN_USING_TXT_RTC */

	//备份app状态 如果是待机清除屏幕数据
	for(i=0;i<LCD_GUN_NUM;i++)
	{
		chargeState[i] = thaisen_app_get_ofsm_charge_state(i);
		switch(chargeState[i])
		{
			case APP_OFSM_STATE_WAIT_NET:
			case APP_OFSM_STATE_IDLEING:
			    if(i == LCD_GUN_1)
	                thaisenSetAuxPowerTypeA(thaisen_auxPowerType_12V);
			    else
			        thaisenSetAuxPowerTypeB(thaisen_auxPowerType_12V);

		        LcdData.setData.batteryVolt[i] = 0;
		        LcdData.setData.maxChargeVolt[i] = 0;

                LcdAssistantData.SeveralGunFlag[i].IsVinStart = FALSE;

				LcdData.gun[i].workState = SysMainStatus_StandBy;
		        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
				break;
			case APP_OFSM_STATE_READYING:
				LcdData.gun[i].workState = SysMainStatus_PlugIn;
				break;				
			case APP_OFSM_STATE_STARTING:
				LcdData.gun[i].workState = SysMainStatus_StartReady;
                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
                }
				break;	
			case APP_OFSM_STATE_CHARGING:
				LcdData.gun[i].workState = SysMainStatus_Chrging;
		        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
				break;
			case APP_OFSM_STATE_STOPING:
				LcdData.gun[i].workState = SysMainStatus_StopChg;
                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
                }
				break;	
			case APP_OFSM_STATE_FINISHING:
				LcdData.gun[i].workState = SysMainStatus_Account;
		        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
				break;
			case APP_OFSM_STATE_FAULTING:
				LcdData.gun[i].workState = SysMainStatus_Err;
		        LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
				break;	
			default:
				break;				
		}
#ifdef SCREEN_USING_QBJ
		// 亮屏
		if(LcdData.gun[i].workState == SysMainStatus_PlugIn){
		    if(++ScreenLightCount > (1000 /BASE_SYS_TIMER)){
		        ScreenLightCount = 0;
		        SerialScreen_ScreenLight(&SerialScreen);
		    }
		}
		if(LcdData.gun[i].workState == SysMainStatus_StandBy){
		    if(i == LCD_GUN_1)
		        LcdData.gun[i].iocnState = LCD_IDLE_ICON_LOGO_QBJ;
		    else
                LcdData.gun[i].iocnState = LCD_IDLE_ICON_PACCOUNT_QBJ;
		}
		else
#endif /* SCREEN_USING_QBJ */
		{
	        LcdData.gun[i].iocnState = LcdData.gun[i].workState;
		}

#ifdef SCREEN_USING_TXT_RTC
		if((thaisen_app_get_system_tick() - RtcTxtTick) > SCREEN_TXT_RTC_PERIOD){
		    RtcTxtTick = thaisen_app_get_system_tick();
		    SerialScreen_RtcTxtSend(&SerialScreen);
		}
#endif /* SCREEN_USING_TXT_RTC */

		#if 1
		if(LcdData.gun[i].workState == SysMainStatus_Chrging)
		{
			nSocIcon++;
			switch(nSocIcon/100)
			{
				case 0:
					LcdData.gun[i].iocnState = ICON_CHARGING_SOC0;
					break;
				case 1:
					LcdData.gun[i].iocnState = ICON_CHARGING_SOC20;
					break;					
				case 2:
					LcdData.gun[i].iocnState = ICON_CHARGING_SOC40;
					break;	
				case 3:
					LcdData.gun[i].iocnState = ICON_CHARGING_SOC60;
					break;	
				case 4:
					LcdData.gun[i].iocnState = ICON_CHARGING_SOC80;
					break;
                case 5:
                    LcdData.gun[i].iocnState = ICON_CHARGING_SOC100;
                    break;
				default:
					nSocIcon = 0;
					break;
			}
		}
		#endif
		if(LcdState[i] != chargeState[i])
		{
			LcdState[i] = chargeState[i];
			if((LcdData.gun[i].workState==SysMainStatus_PlugIn) ||   \
			        (LcdData.gun[i].workState==SysMainStatus_Account))
			{
				sSCREEN_DEBUGPROMSG("*******************mainStatus change to StandBy***********\r\n");
				//清数据
				SerialScreen_DataClean(i);
			}
		}

        if(LcdData.gun[i].workState == SysMainStatus_Chrging){    /** 枪在充电且是VIN码启动，如果本地启动未使能则显示停止ICON */
            if(LcdData.setData.sup_Local == FALSE){
                if(LcdAssistantData.SeveralGunFlag[i].IsVinStart == TRUE){
                    LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
                }else if((LcdData.setData.sup_Local_stop == TRUE) && (thaisen_is_allow_loacl_stop(i))){ /** 停止使能目前只针对于在线启动或密码启动方式(因为前面出去的屏幕工程没有停止使能按键，如果从flash中读出配置停止使能是开启则无法关闭) */
                    LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
                }else{
                    u8 AnotherGun = LCD_GUN_1;
                    if(i == LCD_GUN_1){
                        AnotherGun = LCD_GUN_2;
                    }
                    if(LcdData.gun[AnotherGun].workState == SysMainStatus_Chrging){
                        if(LcdAssistantData.SeveralGunFlag[AnotherGun].IsVinStart == TRUE){
                            LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
                        }else if((LcdData.setData.sup_Local_stop == TRUE) && (thaisen_is_allow_loacl_stop(AnotherGun))){
                            LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
                        }else{
                            LcdData.setData.Sup_Stop = ICON_CHARGE_NULL;
                        }
                    }else{
                        LcdData.setData.Sup_Stop = ICON_CHARGE_NULL;
                    }
                }
            }else{
                LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
            }
        }else{                                                    /** 只要有一把枪在充电且是VIN码启动，如果本地启动未使能则显示停止ICON */
            u8 AnotherGun = LCD_GUN_1;
            if(i == LCD_GUN_1){
                AnotherGun = LCD_GUN_2;
            }
            if(LcdData.setData.sup_Local == FALSE){
                if(LcdData.gun[AnotherGun].workState == SysMainStatus_Chrging){
                    if(LcdAssistantData.SeveralGunFlag[AnotherGun].IsVinStart == TRUE){
                        LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
                    }else if((LcdData.setData.sup_Local_stop == TRUE) && (thaisen_is_allow_loacl_stop(AnotherGun))){
                        LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
                    }else{
                        LcdData.setData.Sup_Stop = ICON_CHARGE_NULL;
                    }
                }else{
                    LcdData.setData.Sup_Stop = ICON_CHARGE_NULL;
                }
            }else{
                LcdData.setData.Sup_Stop = ICON_CHARGE_LOCAL;
            }
        }
	}

	if(TRUE == LcdData.debugIOflg)
	{
		LcdData.setData.g_door = !thaisenGetDoorStatus();
		LcdData.setData.g_emergency = !thaisenGetScramStatus();
		LcdData.setData.g_elElock[LCD_GUN_1] = !thaisenGetElectLockStaA();
		LcdData.setData.g_elElock[LCD_GUN_2] = !thaisenGetElectLockStaB();
		LcdData.setData.g_acRely = !thaisen_relay_AC_FB();
		LcdData.setData.g_dcRelay[LCD_GUN_1] = !(thaisen_relay_A_FB_Z()) + (!thaisen_relay_A_FB_F())*10;
		LcdData.setData.g_dcRelay[LCD_GUN_2] = !(thaisen_relay_B_FB_Z()) + (!thaisen_relay_B_FB_F())*10;
        LcdData.setData.g_paraRely0 = !(thaisen_relay_parallel_FB_Z()) + (!thaisen_relay_parallel_FB_F())*10;
        LcdData.setData.g_paraRely1 = !(thaisen_relay_K7_FB()) + (!thaisen_relay_K8_FB())*10;
        LcdData.setData.g_paraRely2 = !(thaisen_relay_K9_FB()) + (!thaisen_relay_K10_FB())*10;

		LcdData.setData.g_auxRelay[LCD_GUN_1] =!(thaisenGetAux_A_Status_debug());
		LcdData.setData.g_auxRelay[LCD_GUN_2] =!(thaisenGetAux_B_Status_debug());
        LcdData.setData.g_aux24v[LCD_GUN_1] = LcdData.setData.g_auxRelay[LCD_GUN_1];
        LcdData.setData.g_aux24v[LCD_GUN_2] = LcdData.setData.g_auxRelay[LCD_GUN_2];
		LcdData.setData.g_gunsite[LCD_GUN_1] = !(thaisenGetGeneralInPortSta(thaisenGeneralInPortGunSit_A));
        LcdData.setData.g_gunsite[LCD_GUN_2] = !(thaisenGetGeneralInPortSta(thaisenGeneralInPortGunSit_B));
        LcdData.setData.g_fuse[LCD_GUN_1] = 0;
        LcdData.setData.g_fuse[LCD_GUN_2] = 0;
        LcdData.setData.g_pour = !(thaisenGetGeneralInPortSta(thaisenGeneralInPortPour));
        LcdData.setData.g_protect_light = !(thaisenGetGeneralInPortSta(thaisenGeneralInPortLightningProtection));
        LcdData.setData.g_flood = !(thaisenGetGeneralInPortSta(thaisenGeneralInPortFlooding));
        LcdData.setData.g_smoke = !(thaisenGetGeneralInPortSta(thaisenGeneralInPortSmoke));
	}
	
    LcdData.runData.netstate = thaisen_app_get_net_state();
    if((LcdData.setData.sup_offbilling == TRUE) ||
            (LcdData.setData.Sup_PlugAndPlay == TRUE) ||
            (LcdData.setData.NetType == CP_NETTYPE_OFFLINE)){
        LcdData.runData.netstate = 0x05;   //离线计费不显示网络图标
    }

	if(++timesecbak>1000/BASE_SYS_TIMER)
	{
		timesecbak = 0;
		//倒计时
		for(i=0;i<LCD_GUN_NUM;i++)
		{
			if((LcdData.gun[i].workState == SysMainStatus_StartReady)
				||(LcdData.gun[i].workState == SysMainStatus_SelfCheck)
				||(LcdData.gun[i].workState == SysMainStatus_SelfCheck_Wait))
			{
				if(LcdData.gun[i].startCountTimer>0)
					LcdData.gun[i].startCountTimer--;
				sSCREEN_DEBUGPROMSG("LcdData.gun[%d].startCountTimer = %02x\r\n",i,LcdData.gun[i].startCountTimer);	
			}
		}

		timeSync = thaisen_app_get_time_sync_flag();
		if(TRUE == timeSync)
		{
			LcdData.runData.syncTimeFlg = TRUE;	
		}
			
		//getSecToTimeStr(LcdData.runData.timer,timesec);
		//
	    //sprintf((s8 *)LcdData.runData.temp,"%d",(tcuMain_Obj.tcuApi[0].ChgTemp-50));
	    //sprintf((s8 *)LcdData.runData.temp,"%.1f",(Power_Obj.Elc.gAI[powerAI_tempOver00].dfloat));

		//sSCREEN_DEBUGMSG("LcdData.runData.netstate = %02x\r\n",LcdData.runData.netstate);
		//

	    LcdAssistantData.OccupyGunNum = 0;

		for(i=0;i<LCD_GUN_NUM;i++)
		{
			
			LcdData.gun[i].portState = SerialScreen_IsCarConnect(i);
#ifdef SCREEN_USING_OFFLINE_BILLING
			if(((LcdData.gun[i].workState >= SysMainStatus_StartReady)&&(LcdData.gun[i].workState <= SysMainStatus_Account)) ||
			        ((LcdTriggerEvent[i].Flag.IsTriggerExternal == TRUE) && (LcdData.setData.sup_offbilling == TRUE)))
#else
            if((LcdData.gun[i].workState >= SysMainStatus_StartReady)&&(LcdData.gun[i].workState <= SysMainStatus_Account))
#endif /* SCREEN_USING_OFFLINE_BILLING */
			{
				chargeInfo[i] = SerialScreen_GetChargeInfo(i);
				bmsInfo[i] = SerialScreen_GetBmsInfo(i);
				bmsTemp[i] = *(SerialScreen_GetBatTemp(i));

				#if 0
				LcdData.gun[i].cur = (f32)chargeInfo[i].charge_current/LCD_POW_2;
				LcdData.gun[i].vol=  (f32)chargeInfo[i].charge_voltage/LCD_POW_2;
				LcdData.gun[i].engery= (f32) chargeInfo[i].charge_elect/LCD_POW_3;
				getSecToTimeStr(LcdData.gun[i].ChrgeRunTime,chargeInfo[i].charge_time);
				#endif
				if(LcdData.gun[i].workState != SysMainStatus_Account){
	                LcdData.gun[i].cur = chargeInfo[i]->charge_current;
	                LcdData.gun[i].vol = chargeInfo[i]->charge_voltage;
				}else{
	                LcdData.gun[i].cur = 0;
	                LcdData.gun[i].vol = 0;
				}
				LcdData.gun[i].engery = chargeInfo[i]->charge_elect;
				LcdData.gun[i].curSoc = chargeInfo[i]->current_soc;
				LcdData.gun[i].ChrgeTime = chargeInfo[i]->charge_time/60;
				LcdData.gun[i].ChrgeTimeHour = LcdData.gun[i].ChrgeTime/60;
				LcdData.gun[i].ChrgeTImeMin = LcdData.gun[i].ChrgeTime%60;
				LcdData.gun[i].totalFee = chargeInfo[i]->charge_total_fee/LCD_POW_2;
				LcdData.gun[i].AccountBallance = chargeInfo[i]->account_ballance;
				LcdData.gun[i].VolNeed = bmsInfo[i]->bms_require_voltage*LCD_POW_1;
				LcdData.gun[i].CurNeed = bmsInfo[i]->bms_require_current*LCD_POW_1;
				LcdData.gun[i].SigleVol = bmsInfo[i]->single_battery_max_voltage;
#ifdef CP_CONFIG_USING_QBJ
                LcdData.gun[i].RemainTime[0] = bmsInfo[i]->bms_remain_time /60;
                LcdData.gun[i].RemainTime[1] = bmsInfo[i]->bms_remain_time %60;
#endif /* CP_CONFIG_USING_QBJ */
				LcdData.gun[i].BatTemp = bmsTemp[i].temp;
				
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].cur = %.2f A\r\n",i,(f32)chargeInfo[i].charge_current/LCD_POW_2);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].vol = %.2f V\r\n",i,(f32)chargeInfo[i].charge_voltage/LCD_POW_2);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].engery = %.3f KWH\r\n",i,(f32) chargeInfo[i].charge_elect/LCD_POW_3);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].soc = %d \r\n",i,LcdData.gun[i].curSoc);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].ChrgeTime = %d \r\n",i,LcdData.gun[i].ChrgeTime);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].ErrCode = %s \r\n",i,LcdData.gun[i].ErrCode);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].VolNeed = %.1f V\r\n",i,LcdData.gun[i].VolNeed/LCD_POW_1);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].CurNeed = %.1f A\r\n",i,LcdData.gun[i].CurNeed/LCD_POW_1);
				//sSCREEN_DEBUGMSG("LcdData.gun[%d].SigleVol = %.2f \r\n",i,LcdData.gun[i].SigleVol/LCD_POW_2);
			}else{
#ifdef CP_CONFIG_USING_QBJ
			    LcdData.gun[i].RemainTime[0] = 0;
			    LcdData.gun[i].RemainTime[1] = 0;
                LcdData.gun[i].cur = 0;
                LcdData.gun[i].vol = 0;
                if((LcdData.gun[i].workState >= SysMainStatus_StandBy) && (LcdData.gun[i].workState <= SysMainStatus_PlugIn)){
                    LcdData.gun[i].engery = 0;
                    LcdData.gun[i].ChrgeTime = 0;
                    LcdData.gun[i].ChrgeTimeHour = 0;
                    LcdData.gun[i].ChrgeTImeMin = 0;
                }
#endif /* CP_CONFIG_USING_QBJ */
			}
			SerialScreen_GetSysFault(i);
			SerialScreen_DataGetstopReson(i);
			//故障和停止原因
			#if 0
			//
			SerialScreen_GetUnitPrice(timesec,i);
			//
			//
			sprintf((s8 *)LcdData.gun[i].curSoc,"%3d",tcuMain_Obj.tcuApi[i].curtSoc);
			//
			//
			//电池类型
			if(tcuMain_Obj.tcuApi[i].BmsBatType>10)
				temp = 10;
			else
				temp = tcuMain_Obj.tcuApi[i].BmsBatType;
            sprintf((char *)LcdData.gun[i].BatType,SerialScreen_BatType[temp]);
			#endif

            mem_set(LcdData.setData.ErWeiCode[i],0,sizeof(LcdData.setData.ErWeiCode[i]));
			if((LcdData.gun[i].portState == GUN_CONNECT_STATE_YES)&&(LcdData.gun[i].workState==SysMainStatus_PlugIn)){
			    u8 validLen = sizeof(LcdData.setData.ErWeiCode[i]);
			    if((LcdData.setData.sup_offbilling == FALSE) &&
			            (LcdData.setData.Sup_PlugAndPlay == FALSE) &&
			            (LcdData.setData.NetType != CP_NETTYPE_OFFLINE)){
	                if(validLen > thaisen_app_get_gunno_qrcode(i)->qrcode_len){
	                    str_ncpy(LcdData.setData.ErWeiCode[i],thaisen_app_get_gunno_qrcode(i)->qrcode,thaisen_app_get_gunno_qrcode(i)->qrcode_len);
	                }else{
	                    str_ncpy(LcdData.setData.ErWeiCode[i],thaisen_app_get_gunno_qrcode(i)->qrcode,validLen);
	                }
			    }else{
	                if(validLen > strlen((char*)LcdData.runData.chgcode[i])){
	                    str_ncpy(LcdData.setData.ErWeiCode[i],LcdData.runData.chgcode[i],strlen((char*)LcdData.runData.chgcode[i]));
	                }else{
	                    str_ncpy(LcdData.setData.ErWeiCode[i],LcdData.runData.chgcode[i],validLen);
	                }
			    }
                LcdAssistantData.OccupyGunNum++;
			}else{
                LcdData.setData.s_selectaux[i] = ICON_AUXPOWER_NONE;
		    }

            if((LcdData.gun[i].workStateLast != LcdData.gun[i].workState) || (LcdAssistantData.SeveralGunFlag[i].IsPowerOn == 0) || \
                    (LcdAssistantData.SeveralGunFlag[i].AuxPower24VSelect != LcdAssistantData.SeveralGunFlag[i].AuxPower24VSelectLast)){
                LcdAssistantData.SeveralGunFlag[i].IsPowerOn = 1;
                LcdAssistantData.SeveralGunFlag[i].AuxPower24VSelectLast = LcdAssistantData.SeveralGunFlag[i].AuxPower24VSelect;

                if(LcdData.gun[i].workState == SysMainStatus_PlugIn){
                    if(LcdAssistantData.Flag.IsEnableAuxPower24V == TRUE){
                        LcdData.setData.s_selectaux[i] = (ICON_AUXPOWER_12V + i *2);
                    }
                }
            }
            if(LcdAssistantData.Flag.IsEnableAuxPower24V != TRUE){
                LcdData.setData.s_selectaux[i] = ICON_AUXPOWER_NONE;
            }

			#if 1
			if(LcdData.gun[i].workState==SysMainStatus_Chrging)
				sprintf(LcdData.gun[i].soc,"%3d%%",LcdData.gun[i].curSoc);
			else
				mem_set(LcdData.gun[i].soc,0,sizeof(LcdData.gun[i].soc));

			#endif
			LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_LOCAL][i] = !LcdData.setData.sup_Local;
			if(TRUE == LcdData.setData.sup_VIN)
				LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_VIN][i] = ICON_CHARGE_VIN;
			else LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_VIN][i] = ICON_CHARGE_NULL;

            if(TRUE == LcdData.setData.sup_pw_start)
                LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_PW][i] = ICON_CHARGE_PW;
            else LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_PW][i] = ICON_CHARGE_NULL;

			sSCREEN_DEBUGMSG(" ErWeiCode[%d]=%s",i,LcdData.setData.ErWeiCode[i]);
			//
			//sSCREEN_DEBUGMSG(" portState[%d] = %02x",i,LcdData.gun[i].portState);
			sSCREEN_DEBUGMSG("Sup_StartStyle[%d] = %d",i,LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_LOCAL][i]);
			sSCREEN_DEBUGMSG(" chargeState[%d]= %02x\r\n",i,chargeState[i]);
			sSCREEN_DEBUGMSG(" code_stopResaon[%d]=%s",i,LcdData.gun[i].code_stopResaon);
			sSCREEN_DEBUGMSG(" workState[%d]= %02x\r\n",i,LcdData.gun[i].workState);
			//sSCREEN_EVENT_DEBUGMSG(" chgStatus[%d]= %02x\r\n",i,thaisen_get_charg_status(i));

            if(LcdData.gun[i].workStateLast != LcdData.gun[i].workState){
                if(LcdData.setData.Sup_PlugAndPlay == FALSE){
                    if(LcdData.gun[i].workState == SysMainStatus_PlugIn){
                        LcdData.Homeflg = 0;
                        if(i == LCD_GUN_1){
                            if(LcdData.gun[LCD_GUN_2].workState != SysMainStatus_StartReady){
                                LcdData.CurrentPage = LCD_PAGE_A_SELECT;
                                LcdData.gunIndex = LCD_GUN_1;
#ifdef SCREEN_USING_OFFLINE_BILLING
                                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                    LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                                }
#endif /* SCREEN_USING_OFFLINE_BILLING */
                                SerialScreen_QuitDebugIO();
                                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
								}
                                LcdData.menuflg = 0;
                            }
                        }else{
                            if(LcdData.gun[LCD_GUN_1].workState != SysMainStatus_StartReady){
                                LcdData.CurrentPage = LCD_PAGE_B_SELECT;
                                LcdData.gunIndex = LCD_GUN_2;
#ifdef SCREEN_USING_OFFLINE_BILLING
                                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                    LcdTriggerEvent[i].Flag.IsTriggerExternal = FALSE;
                                }
#endif /* SCREEN_USING_OFFLINE_BILLING */
                                SerialScreen_QuitDebugIO();
                                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
                                }
                                LcdData.menuflg = 0;
                            }
                        }
                    }

                    if(LcdData.gun[i].workState == SysMainStatus_StandBy){
                        if(i == LCD_GUN_1){
                            if((LcdData.CurrentPage == LCD_PAGE_A_SELECT) && (LcdData.gunIndex == LCD_GUN_1)){
                                LcdData.CurrentPage = LCD_PAGE_STANDBY;
                                LcdData.Homeflg = 1;
                                SerialScreen_QuitDebugIO();
                                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
                                }
                            }
                        }else{
                            if((LcdData.CurrentPage == LCD_PAGE_B_SELECT) && (LcdData.gunIndex == LCD_GUN_2)){
                                LcdData.CurrentPage = LCD_PAGE_STANDBY;
                                LcdData.Homeflg = 1;
                                SerialScreen_QuitDebugIO();
                                for(u8 i = 0; i < LCD_GUN_NUM; i++){
                                    LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
                                }
                            }
                        }
                    }
                }
                LcdData.gun[i].workStateLast = LcdData.gun[i].workState;
            }
		}
		ret = 1;
        if((LcdAssistantData.OccupyGunNum == LCD_GUN_NUM) && (LcdAssistantData.DeviceType != SYSTEM_FUNCTION_DYNAMIC_SWITCH)){
	        if(LcdAssistantData.Flag.IsEnableParaCharge){
	            if(LcdAssistantData.Flag.ParaChargeSelect)
	                LcdData.setData.parallel_iocn = ICON_CHARGEWAY_PARACHARGE;
	            else
	                LcdData.setData.parallel_iocn = ICON_CHARGEWAY_SINGLECHARGE;
	        }else{
	            LcdAssistantData.Flag.ParaChargeSelect = FALSE;
	            LcdData.setData.parallel_iocn = ICON_CHARGEWAY_NONE;
	        }
		}else{
	        LcdAssistantData.Flag.ParaChargeSelect = FALSE;
            LcdData.setData.parallel_iocn = ICON_CHARGEWAY_NONE;
		}
	}

	return ret;
}

//20ms 

void SerialScreen_Process(struct SerialScreenObj *cmd)
{
    /** 外部跳页触发事件处理 */
    SerialScreen_TriggerEvent_Process();

    if(SerialScreen_Read() > 0)  /* 读到一帧完整数据在进行接收到数据的处理 */
    {
        SerialScreen_Recvie(cmd, LcdRxData.rxbuf, LcdRxData.tlen);
    }

    SerialScreen_RealTime_InfoGet();   /* 获取外部实时信息 */
    SerialScreen_TimeingRefrensh();    /* 定时刷新 */
    SerialScreen_GetKeyProcess(cmd);
    SerialScreen_CheckKey();//按键间隔
    SerialScreen_CheckPageReset();//触发界面自动消失
    //返回或页面重置
    if(LcdData.CurrentPage==0)
    {
        sSCREEN_DEBUGPROMSG("**************menu exit*******************\r\n");
        LcdData.menuflg = 0;
        SerialScreen_PageNeedRefresh(LcdData.gunIndex);
        for(u8 i = 0; i < LCD_GUN_NUM; i++){
            LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
        }
        //
    }

    if(LcdData.Homeflg == 1){
        LcdData.menuflg = 0;
        LcdData.NeedMenuOffFlg = 0;
    }

    //sSCREEN_DEBUGMSG("**************SerialScreen_PageReset*******************\r\n");
    SerialScreen_PageReset(LcdData.gunIndex);
    //sSCREEN_DEBUGMSG("**************SerialScreen_DataProcess*******************\r\n");
    SerialScreen_DataProcess();
    if(s_ota_info->start_flag){
        LcdData.CurrentPage = LCD_PAGE_SYS_UPDATE;
        for(u8 i = 0; i < LCD_GUN_NUM; i++){
            LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
        }
    }

    if((LcdData.CurrentPage == LCD_PAGE_A_ACOUNT) && (LcdData.gunIndex == LCD_GUN_1)){
        if(LcdData.PageCountDown > thaisen_app_get_system_tick()){
            LcdData.PageCountDown = thaisen_app_get_system_tick();
        }
        if((thaisen_app_get_system_tick() - LcdData.PageCountDown) > 30 *1000){
            LcdData.CurrentPage = LCD_PAGE_STANDBY;
            LcdData.AccountPageCountDown_Over = 1;
            LcdData.PageCountDown = thaisen_app_get_system_tick();
            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
            }
        }
    }else if((LcdData.CurrentPage == LCD_PAGE_B_ACOUNT) && (LcdData.gunIndex == LCD_GUN_2)){
        if(LcdData.PageCountDown > thaisen_app_get_system_tick()){
            LcdData.PageCountDown = thaisen_app_get_system_tick();
        }
        if((thaisen_app_get_system_tick() - LcdData.PageCountDown) > 30 *1000){
            LcdData.CurrentPage = LCD_PAGE_STANDBY;
            LcdData.AccountPageCountDown_Over = 1;
            LcdData.PageCountDown = thaisen_app_get_system_tick();
            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
            }
        }
    }else if(((LcdData.CurrentPage == LCD_PAGE_A_CHGING) || (LcdData.CurrentPage == LCD_PAGE_A_CHGING_BAT)) && (LcdData.gunIndex == LCD_GUN_1)){
        if(LcdData.PageCountDown > thaisen_app_get_system_tick()){
            LcdData.PageCountDown = thaisen_app_get_system_tick();
        }
        if((thaisen_app_get_system_tick() - LcdData.PageCountDown) > 180 *1000){
            LcdData.CurrentPage = LCD_PAGE_STANDBY;
            LcdData.ChargingPageCountDown_Over = 1;
            LcdData.PageCountDown = thaisen_app_get_system_tick();
            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
            }
        }
    }else if(((LcdData.CurrentPage == LCD_PAGE_B_CHGING) || (LcdData.CurrentPage == LCD_PAGE_B_CHGING_BAT)) && (LcdData.gunIndex == LCD_GUN_2)){
        if(LcdData.PageCountDown > thaisen_app_get_system_tick()){
            LcdData.PageCountDown = thaisen_app_get_system_tick();
        }
        if((thaisen_app_get_system_tick() - LcdData.PageCountDown) > 180 *1000){
            LcdData.CurrentPage = LCD_PAGE_STANDBY;
            LcdData.ChargingPageCountDown_Over = 1;
            LcdData.PageCountDown = thaisen_app_get_system_tick();
            for(u8 i = 0; i < LCD_GUN_NUM; i++){
                LcdAssistantData.SeveralGunFlag[i].IsPWStartAuthen = FALSE;
            }
        }
    }else{
        LcdData.PageCountDown = thaisen_app_get_system_tick();
    }

    if(LcdData.CurrentPage != LcdData.CurrentPageBack)
    {
        if((LcdData.CurrentPageBack != LCD_PAGE_A_ACOUNT) && (LcdData.CurrentPageBack != LCD_PAGE_B_ACOUNT)){
            LcdData.AccountPageCountDown_Over = 0;
        }

        if(((LcdData.CurrentPageBack != LCD_PAGE_B_CHGING) && (LcdData.CurrentPageBack != LCD_PAGE_B_CHGING_BAT) &&
                (LcdData.CurrentPageBack != LCD_PAGE_A_CHGING) && (LcdData.CurrentPageBack != LCD_PAGE_A_CHGING_BAT))){
            LcdData.ChargingPageCountDown_Over = 0;
        }
        LcdData.CurrentPageBack = LcdData.CurrentPage;
        //sSCREEN_DEBUGMSG("**************SerialScreen_CurrentPageShow*******************\r\n");
        SerialScreen_CurrentPageShow(cmd,0);
        LcdData.PageCountDown = thaisen_app_get_system_tick();
        //readCurrentPageNo(cmd);
    }
    else
    {
        static u8 timer = 0;

        if(s_ota_info->start_flag){
            if(++timer>5)
            {
                timer = 0;
                SerialScreen_CurrentPageShow(cmd,1);
            }
        }else{
            SerialScreen_CurrentPageShow(cmd,1);
        }
    }
    LcdData.setData.Icon_ProtectInfo = 0;
    LcdData.setData.Input_OverVolt = 0xFF00;
    LcdData.setData.Input_UnderVolt = 0xFF00;
    LcdData.setData.Onput_OverVolt = 0xFF00;
    LcdData.setData.Onput_UnderVolt = 0xFF00;
    LcdData.setData.Onput_OverCurr = 0xFF00;
}

int SerialScreen_chksum(u8 *ptr,s32 len)
{
    return 0;
}

int SerialScreen_Send(struct SerialScreenObj *cmd, u8 *ptr,u32 len)
{

	if(ptr != NULL)
	{
		sSCREEN_DEBUGTxDATA("------screen Tx=", ptr, len);
		thaisen_app_send_data_to_hci_uart(ptr,len);
	}
    return 0;
}

//串口对象初始化
struct SerialScreenObj SerialScreen =
{
	&SerialScreenAddr,
	SerialScreenRxbuf,

	SerialScreen_Init,
	SerialScreen_Analyze,
	SerialScreen_Process,
	SerialScreen_chksum,
    SerialScreen_Send
};

void SerialScreen_ScreenRequest_Time(void)
{
    if(LcdData.CurrentPage != LCD_PAGE_MENU_SYS){
        SerialScreen_ReadData(&SerialScreen, 0x0010, 4);
    }
}

struct LCD_DATA_FIFO_TYPE *serialScreen_ObjectAi_Init(void)
{
    memset(LCD_ALL_PAGE_TAB, 0, sizeof(LCD_ALL_PAGE_TAB));

    /** 1.首页 [page:01]*/
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Agun_QrCode", LCD_QRCodeType, LCD_NoReflash, 0x6000, pstr_type, sizeof(LcdData.setData.ErWeiCode[LCD_GUN_1]), (void *)LcdData.setData.ErWeiCode[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Bgun_QrCode", LCD_QRCodeType, LCD_NoReflash, 0x6080, pstr_type, sizeof(LcdData.setData.ErWeiCode[LCD_GUN_2]), (void *)LcdData.setData.ErWeiCode[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Agun_Soc", LCD_TextType, LCD_1sReflash, 0x1110, pstr_type, sizeof(LcdData.gun[LCD_GUN_1].soc), (void *)LcdData.gun[LCD_GUN_1].soc);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Bgun_Soc", LCD_TextType, LCD_1sReflash, 0x1116, pstr_type, sizeof(LcdData.gun[LCD_GUN_2].soc), (void *)LcdData.gun[LCD_GUN_2].soc);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "state_Agun", LCD_IconType, LCD_10sReflash, 0x1001, pu8_type, 1, (void *)&LcdData.gun[LCD_GUN_1].iocnState);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "state_Bgun", LCD_IconType, LCD_10sReflash, 0x1002, pu8_type,1, (void *)&LcdData.gun[LCD_GUN_2].iocnState);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, 1, (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "help number", LCD_TextType, LCD_NoReflash, 0x11A0, pstr_type, sizeof(LcdData.setData.Help_Number), (void *)LcdData.setData.Help_Number);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, SerialScreen_IsCarConnect, LCD_BtnAType, 0x0000, 0x1000, page_type, LCD_PAGE_A_SELECT, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, SerialScreen_IsCarConnect, LCD_BtnBType, 0x0001, 0x1000, page_type, LCD_PAGE_B_SELECT, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Agun_ChgCode", LCD_TextType, LCD_NoReflash, 0x1015, pstr_type, sizeof(LcdData.runData.chgcode[LCD_GUN_1]), (void *)&LcdData.runData.chgcode[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Bgun_ChgCode", LCD_TextType, LCD_NoReflash, 0x1020, pstr_type, sizeof(LcdData.runData.chgcode[LCD_GUN_2]), (void *)&LcdData.runData.chgcode[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Sys Info", LCD_BtnType, 0x0013, 0x1000, page_type, LCD_PAGE_SYS_INFO, (void *)SerialScreen_SysInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "factory id", LCD_IconType, LCD_10sReflash, 0x1103, pu8_type, 1, (void *)&LcdData.setData.manufacturer);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "root paswd", LCD_BtnType, 0x0001, 0x1003, page_type, LCD_PAGE_ADMIN_PASWD, (void *)SerialScreen_NeedPageReset);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Icon Saux", LCD_IconType, LCD_10sReflash, 0x4630, pu8_type, sizeof(LcdData.setData.s_selectaux[LCD_GUN_1]), (void *)&LcdData.setData.s_selectaux[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Icon Saux", LCD_IconType, LCD_10sReflash, 0x4632, pu8_type, sizeof(LcdData.setData.s_selectaux[LCD_GUN_2]), (void *)&LcdData.setData.s_selectaux[LCD_GUN_2]);

    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "paracharge_icon", LCD_IconType, LCD_10sReflash, 0x4656, pu8_type, 1, (void *)&LcdData.setData.parallel_iocn);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "set paracharge", LCD_BtnType, 0x0004, 0x1007, page_type, LCD_PAGE_STANDBY, (void *)SerialScreen_ParaChargeSet);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "set singlecharge", LCD_BtnType, 0x0003, 0x1007, page_type, LCD_PAGE_STANDBY, (void *)SerialScreen_SingleChargeSet);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "auxA set", LCD_BtnType, 0x0001, 0x1007, page_type, LCD_PAGE_STANDBY, (void *)SerialScreen_AuxsetA);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "auxB set", LCD_BtnType, 0x0002, 0x1007, page_type, LCD_PAGE_STANDBY, (void *)SerialScreen_AuxsetB);

    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "period0", LCD_DataType, LCD_1sReflash, 0x4306, pu32_type, sizeof(LcdData.setData.period_time[0]), (void *)&LcdData.setData.period_time[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "period1", LCD_DataType, LCD_1sReflash, 0x4308, pu32_type, sizeof(LcdData.setData.period_time[1]), (void *)&LcdData.setData.period_time[1]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "period2", LCD_DataType, LCD_1sReflash, 0x430A, pu32_type, sizeof(LcdData.setData.period_time[2]), (void *)&LcdData.setData.period_time[2]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "period3", LCD_DataType, LCD_1sReflash, 0x430C, pu32_type, sizeof(LcdData.setData.period_time[3]), (void *)&LcdData.setData.period_time[3]);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "period price", LCD_DataType, LCD_1sReflash, 0x474A, pu32_type, sizeof(LcdData.setData.period_price), (void *)&LcdData.setData.period_price);
#ifdef SCREEN_USING_QBJ
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Output VoltA", LCD_DataType, LCD_1sReflash, 0x1400, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].vol), (void *)&LcdData.gun[LCD_GUN_1].vol);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Output CurrA", LCD_DataType, LCD_1sReflash, 0x1406, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].cur), (void *)&LcdData.gun[LCD_GUN_1].cur);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Time UsedA", LCD_DataType, LCD_1sReflash, 0x140E, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].ChrgeTime), (void *)&LcdData.gun[LCD_GUN_1].ChrgeTime);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "EnergyA", LCD_DataType, LCD_1sReflash, 0x1410, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].engery), (void *)&LcdData.gun[LCD_GUN_1].engery);

    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Output VoltB", LCD_DataType, LCD_1sReflash, 0x2400, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].vol), (void *)&LcdData.gun[LCD_GUN_2].vol);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Output CurrB", LCD_DataType, LCD_1sReflash, 0x2406, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].cur), (void *)&LcdData.gun[LCD_GUN_2].cur);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "Time UsedB", LCD_DataType, LCD_1sReflash, 0x240E, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].ChrgeTime), (void *)&LcdData.gun[LCD_GUN_2].ChrgeTime);
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "EnergyB", LCD_DataType, LCD_1sReflash, 0x2410, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].engery), (void *)&LcdData.gun[LCD_GUN_2].engery);
#endif /* SCREEN_USING_QBJ */
    SerialScreen_ItemSetUp(LCD_PAGE_STANDBY, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 2.A枪选择 [page:02]*/
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, sizeof(LcdData.runData.netstate), (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "Agun_QrCode", LCD_QRCodeType, LCD_NoReflash, 0x6000, pstr_type, sizeof(LcdData.setData.ErWeiCode[LCD_GUN_1]), (void *)LcdData.setData.ErWeiCode[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "back", LCD_BtnType, 0x0002, 0x1000, page_type, LCD_PAGE_STANDBY, (void *)SerialScreen_BtnReturn1);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "Agun_Charge", LCD_BtnType, 0x0008, 0x1010, page_type, LCD_PAGE_A_SELECT, (void *)SerialScreen_StartChargeA);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "Agun_VinCharge", LCD_BtnType, 0x0016, 0x1010, page_type, LCD_PAGE_A_SELECT, (void *)SerialScreen_VinStartChargeA);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "start charge", LCD_IconType, LCD_10sReflash, 0x1056, pu8_type, 1, (void *)&LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_LOCAL][LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "Vin charge", LCD_IconType, LCD_10sReflash, 0x1057, pu8_type, 1, (void *)&LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_VIN][LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "PW charge", LCD_IconType, LCD_10sReflash, 0x6500, pu8_type, 1, (void *)&LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_PW][LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "Icon Saux", LCD_IconType, LCD_10sReflash, 0x4630, pu8_type, sizeof(LcdData.setData.s_selectaux[LCD_GUN_1]), (void *)&LcdData.setData.s_selectaux[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "auxA set", LCD_BtnType, 0x0001, 0x1007, page_type, LCD_PAGE_A_SELECT, (void *)SerialScreen_AuxsetA);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "unlock_ela", LCD_BtnType, 0x0007, 0x1000, page_type, LCD_PAGE_A_SELECT, (void *)SerialScreen_BtnUnElockA);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "pw authen", LCD_BtnType, 0x0001, 0x1001, page_type, LCD_PAGE_A_SELECT, (void *)SerialScreen_PWStartAuthen);
    SerialScreen_ItemSetUp(LCD_PAGE_A_SELECT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 3.B枪选择 [page:03]*/
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, sizeof(LcdData.runData.netstate), (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "Bgun_QrCode", LCD_QRCodeType, LCD_NoReflash, 0x6080, pstr_type, sizeof(LcdData.setData.ErWeiCode[LCD_GUN_2]), (void *)LcdData.setData.ErWeiCode[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "back", LCD_BtnType, 0x0002, 0x1000, page_type, LCD_PAGE_STANDBY, (void *)SerialScreen_BtnReturn1);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "Bgun_Charge", LCD_BtnType, 0x0008, 0x1010, page_type, LCD_PAGE_B_SELECT, (void *)SerialScreen_StartChargeB);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "Bgun_VinCharge", LCD_BtnType, 0x0016, 0x1010, page_type, LCD_PAGE_B_SELECT, (void *)SerialScreen_VinStartChargeB);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "start charge", LCD_IconType, LCD_10sReflash, 0x1056, pu8_type, 1, (void *)&LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_LOCAL][LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "Vin charge", LCD_IconType, LCD_10sReflash, 0x1057, pu8_type, 1, (void *)&LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_VIN][LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "PW charge", LCD_IconType, LCD_10sReflash, 0x6501, pu8_type, 1, (void *)&LcdData.setData.Sup_StartStyle[CHARGE_STYLE_START_PW][LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "Icon Saux", LCD_IconType, LCD_10sReflash, 0x4632, pu8_type, sizeof(LcdData.setData.s_selectaux[LCD_GUN_2]), (void *)&LcdData.setData.s_selectaux[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "auxB set", LCD_BtnType, 0x0002, 0x1007, page_type, LCD_PAGE_B_SELECT, (void *)SerialScreen_AuxsetB);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "unlock_elb", LCD_BtnType, 0x0008, 0x1000, page_type, LCD_PAGE_B_SELECT, (void *)SerialScreen_BtnUnElockB);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "pw authen", LCD_BtnType, 0x0003, 0x1001, page_type, LCD_PAGE_B_SELECT, (void *)SerialScreen_PWStartAuthen);
    SerialScreen_ItemSetUp(LCD_PAGE_B_SELECT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 4.A枪启动 [page:04]*/
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "count down", LCD_DataType, LCD_1sReflash, 0x1040, pu16_type, sizeof(LcdData.gun[LCD_GUN_1].startCountTimer), (void *)&LcdData.gun[LCD_GUN_1].startCountTimer);
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "chargeSta", LCD_IconType, LCD_1sReflash, 0x1710, pu8_type, 1, (void *)&LcdData.setData.chargeState[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "sampVolt", LCD_DataType, LCD_1sReflash, 0x1712, pu32_type, sizeof(LcdData.setData.samplingVolt[LCD_GUN_1]), (void *)&LcdData.setData.samplingVolt[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "moduleVolt", LCD_DataType, LCD_1sReflash, 0x1714, pu32_type, sizeof(LcdData.setData.moduleVolt[LCD_GUN_1]), (void *)&LcdData.setData.moduleVolt[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "batVolt", LCD_DataType, LCD_1sReflash, 0x1716, pu32_type, sizeof(LcdData.setData.batteryVolt[LCD_GUN_1]), (void *)&LcdData.setData.batteryVolt[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "maxVolt", LCD_DataType, LCD_1sReflash, 0x1718, pu32_type, sizeof(LcdData.setData.maxChargeVolt[LCD_GUN_1]), (void *)&LcdData.setData.maxChargeVolt[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_START, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 5.B枪启动 [page:05] */
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "count down", LCD_DataType, LCD_1sReflash, 0x2040, pu16_type, sizeof(LcdData.gun[LCD_GUN_2].startCountTimer), (void *)&LcdData.gun[LCD_GUN_2].startCountTimer);
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "chargeSta", LCD_IconType, LCD_1sReflash, 0x171A, pu8_type, 1, (void *)&LcdData.setData.chargeState[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "sampVolt", LCD_DataType, LCD_1sReflash, 0x171C, pu32_type, sizeof(LcdData.setData.samplingVolt[LCD_GUN_2]), (void *)&LcdData.setData.samplingVolt[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "moduleVolt", LCD_DataType, LCD_1sReflash, 0x171E, pu32_type, sizeof(LcdData.setData.moduleVolt[LCD_GUN_2]), (void *)&LcdData.setData.moduleVolt[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "batVolt", LCD_DataType, LCD_1sReflash, 0x1720, pu32_type, sizeof(LcdData.setData.batteryVolt[LCD_GUN_2]), (void *)&LcdData.setData.batteryVolt[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "maxVolt", LCD_DataType, LCD_1sReflash, 0x1722, pu32_type, sizeof(LcdData.setData.maxChargeVolt[LCD_GUN_2]), (void *)&LcdData.setData.maxChargeVolt[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_START, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 6.A枪充电信息 [page:06] */
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, sizeof(LcdData.runData.netstate), (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Output Volt", LCD_DataType, LCD_1sReflash, 0x1400, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].vol), (void *)&LcdData.gun[LCD_GUN_1].vol);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Output Curr", LCD_DataType, LCD_1sReflash, 0x1406, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].cur), (void *)&LcdData.gun[LCD_GUN_1].cur);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Time Used", LCD_DataType, LCD_1sReflash, 0x140E, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].ChrgeTime), (void *)&LcdData.gun[LCD_GUN_1].ChrgeTime);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Energy", LCD_DataType, LCD_1sReflash, 0x1410, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].engery), (void *)&LcdData.gun[LCD_GUN_1].engery);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x1030, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].curSoc), (void *)&LcdData.gun[LCD_GUN_1].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Agun_Bms", LCD_BtnType, 0x0006, 0x1000, page_type, LCD_PAGE_A_CHGING_BAT, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Agun_StopCharge", LCD_TrigType, 0x0007, 0x1010, page_type, LCD_GUN_1, (void *)SerialScreen_StopCharge);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "warnning", LCD_IconType, LCD_10sReflash, 0x4300, pu8_type, sizeof(LcdData.setData.warnning[LCD_GUN_1]), (void *)&LcdData.setData.warnning[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Home", LCD_BtnBType, 0x0004, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "SupStop", LCD_IconType, LCD_10sReflash, 0x1054, pu8_type, sizeof(LcdData.setData.Sup_Stop), (void *)&LcdData.setData.Sup_Stop);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "period price", LCD_DataType, LCD_1sReflash, 0x474A, pu32_type, sizeof(LcdData.setData.period_price), (void *)&LcdData.setData.period_price);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "ballance", LCD_DataType, LCD_1sReflash, 0x172C, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].AccountBallance), (void *)&LcdData.gun[LCD_GUN_1].AccountBallance);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "consume", LCD_DataType, LCD_1sReflash, 0x1618, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].totalFee), (void *)&LcdData.gun[LCD_GUN_1].totalFee);
#ifdef SCREEN_USING_QBJ
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "RemainH_A", LCD_DataType, LCD_1sReflash, 0x6250, pu8_type, sizeof(LcdData.gun[LCD_GUN_1].RemainTime[0]), (void *)&LcdData.gun[LCD_GUN_1].RemainTime[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "RemainM_A", LCD_DataType, LCD_1sReflash, 0x6252, pu8_type, sizeof(LcdData.gun[LCD_GUN_1].RemainTime[1]), (void *)&LcdData.gun[LCD_GUN_1].RemainTime[1]);
#endif /* SCREEN_USING_QBJ */
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 7.B枪充电信息 [page:07] */
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, sizeof(LcdData.runData.netstate), (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Output Volt", LCD_DataType, LCD_1sReflash, 0x2400, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].vol), (void *)&LcdData.gun[LCD_GUN_2].vol);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Output Curr", LCD_DataType, LCD_1sReflash, 0x2406, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].cur), (void *)&LcdData.gun[LCD_GUN_2].cur);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Time Used", LCD_DataType, LCD_1sReflash, 0x240E, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].ChrgeTime), (void *)&LcdData.gun[LCD_GUN_2].ChrgeTime);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Energy", LCD_DataType, LCD_1sReflash, 0x2410, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].engery), (void *)&LcdData.gun[LCD_GUN_2].engery);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x2030, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].curSoc), (void *)&LcdData.gun[LCD_GUN_2].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Bgun_Bms", LCD_BtnType, 0x0006, 0x1000, page_type, LCD_PAGE_B_CHGING_BAT, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Bgun_StopCharge", LCD_TrigType, 0x0007, 0x1010, page_type, LCD_GUN_2, (void *)SerialScreen_StopCharge);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "warnning", LCD_IconType, LCD_10sReflash, 0x4302, pu8_type, sizeof(LcdData.setData.warnning[LCD_GUN_2]), (void *)&LcdData.setData.warnning[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Agun", LCD_BtnHomeType, 0x0003, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "SupStop", LCD_IconType, LCD_10sReflash, 0x1054, pu8_type, sizeof(LcdData.setData.Sup_Stop), (void *)&LcdData.setData.Sup_Stop);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "period price", LCD_DataType, LCD_1sReflash, 0x474A, pu32_type, sizeof(LcdData.setData.period_price), (void *)&LcdData.setData.period_price);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "ballance", LCD_DataType, LCD_1sReflash, 0x174A, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].AccountBallance), (void *)&LcdData.gun[LCD_GUN_2].AccountBallance);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "consume", LCD_DataType, LCD_1sReflash, 0x2618, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].totalFee), (void *)&LcdData.gun[LCD_GUN_2].totalFee);
#ifdef SCREEN_USING_QBJ
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "RemainH_B", LCD_DataType, LCD_1sReflash, 0x6254, pu8_type, sizeof(LcdData.gun[LCD_GUN_2].RemainTime[0]), (void *)&LcdData.gun[LCD_GUN_2].RemainTime[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "RemainM_B", LCD_DataType, LCD_1sReflash, 0x6256, pu8_type, sizeof(LcdData.gun[LCD_GUN_2].RemainTime[1]), (void *)&LcdData.gun[LCD_GUN_2].RemainTime[1]);
#endif /* SCREEN_USING_QBJ */
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 8.A枪电池信息 [page:08] */
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, sizeof(LcdData.runData.netstate), (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Volt Need", LCD_DataType, LCD_1sReflash, 0x1600, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].VolNeed), (void *)&LcdData.gun[LCD_GUN_1].VolNeed);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Cur Need", LCD_DataType, LCD_1sReflash, 0x1606, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].CurNeed), (void *)&LcdData.gun[LCD_GUN_1].CurNeed);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Bat Tempture", LCD_DataType, LCD_1sReflash, 0x160E, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].BatTemp), (void *)&LcdData.gun[LCD_GUN_1].BatTemp);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Sigle Vol", LCD_DataType, LCD_1sReflash, 0x1610, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].SigleVol), (void *)&LcdData.gun[LCD_GUN_1].SigleVol);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x1030, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].curSoc), (void *)&LcdData.gun[LCD_GUN_1].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Agun_Charge", LCD_BtnType, 0x0003, 0x1000, page_type, LCD_PAGE_A_CHGING, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Agun_StopCharge", LCD_TrigType, 0x0007, 0x1010, page_type, LCD_GUN_1, (void *)SerialScreen_StopCharge);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "warnning", LCD_IconType, LCD_10sReflash, 0x4300, pu8_type, sizeof(LcdData.setData.warnning[LCD_GUN_1]), (void *)&LcdData.setData.warnning[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Bgun", LCD_BtnHomeType, 0x0004, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "SupStop", LCD_IconType, LCD_10sReflash, 0x1054, pu8_type, sizeof(LcdData.setData.Sup_Stop), (void *)&LcdData.setData.Sup_Stop);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "period price", LCD_DataType, LCD_1sReflash, 0x474A, pu32_type, sizeof(LcdData.setData.period_price), (void *)&LcdData.setData.period_price);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "ballance", LCD_DataType, LCD_1sReflash, 0x172C, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].AccountBallance), (void *)&LcdData.gun[LCD_GUN_1].AccountBallance);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "consume", LCD_DataType, LCD_1sReflash, 0x1618, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].totalFee), (void *)&LcdData.gun[LCD_GUN_1].totalFee);
#ifdef SCREEN_USING_QBJ
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "RemainH_A", LCD_DataType, LCD_1sReflash, 0x6250, pu8_type, sizeof(LcdData.gun[LCD_GUN_1].RemainTime[0]), (void *)&LcdData.gun[LCD_GUN_1].RemainTime[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "RemainM_A", LCD_DataType, LCD_1sReflash, 0x6252, pu8_type, sizeof(LcdData.gun[LCD_GUN_1].RemainTime[1]), (void *)&LcdData.gun[LCD_GUN_1].RemainTime[1]);
#endif /* SCREEN_USING_QBJ */
    SerialScreen_ItemSetUp(LCD_PAGE_A_CHGING_BAT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 9.B枪电池信息 [page:09] */
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "net_sign", LCD_IconType, LCD_10sReflash, 0x1700, pu8_type, sizeof(LcdData.runData.netstate), (void *)&LcdData.runData.netstate);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Volt Need", LCD_DataType, LCD_1sReflash, 0x2600, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].VolNeed), (void *)&LcdData.gun[LCD_GUN_2].VolNeed);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Cur Need", LCD_DataType, LCD_1sReflash, 0x2606, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].CurNeed), (void *)&LcdData.gun[LCD_GUN_2].CurNeed);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Bat Tempture", LCD_DataType, LCD_1sReflash, 0x260E, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].BatTemp), (void *)&LcdData.gun[LCD_GUN_2].BatTemp);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Sigle Vol", LCD_DataType, LCD_1sReflash, 0x2610, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].SigleVol), (void *)&LcdData.gun[LCD_GUN_2].SigleVol);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x2030, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].curSoc), (void *)&LcdData.gun[LCD_GUN_2].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Bgun_Charge", LCD_BtnType, 0x0003, 0x1000, page_type, LCD_PAGE_B_CHGING, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Bgun_StopCharge", LCD_TrigType, 0x0007, 0x1010, page_type, LCD_GUN_2, (void *)SerialScreen_StopCharge);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "warnning", LCD_IconType, LCD_10sReflash, 0x4302, pu8_type, sizeof(LcdData.setData.warnning[LCD_GUN_2]), (void *)&LcdData.setData.warnning[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Agun", LCD_BtnHomeType, 0x0003, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "SupStop", LCD_IconType, LCD_10sReflash, 0x1054, pu8_type, sizeof(LcdData.setData.Sup_Stop), (void *)&LcdData.setData.Sup_Stop);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "period price", LCD_DataType, LCD_1sReflash, 0x474A, pu32_type, sizeof(LcdData.setData.period_price), (void *)&LcdData.setData.period_price);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "ballance", LCD_DataType, LCD_1sReflash, 0x174A, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].AccountBallance), (void *)&LcdData.gun[LCD_GUN_2].AccountBallance);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "consume", LCD_DataType, LCD_1sReflash, 0x2618, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].totalFee), (void *)&LcdData.gun[LCD_GUN_2].totalFee);
#ifdef SCREEN_USING_QBJ
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "RemainH_B", LCD_DataType, LCD_1sReflash, 0x6254, pu8_type, sizeof(LcdData.gun[LCD_GUN_2].RemainTime[0]), (void *)&LcdData.gun[LCD_GUN_2].RemainTime[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "RemainM_B", LCD_DataType, LCD_1sReflash, 0x6256, pu8_type, sizeof(LcdData.gun[LCD_GUN_2].RemainTime[1]), (void *)&LcdData.gun[LCD_GUN_2].RemainTime[1]);
#endif /* SCREEN_USING_QBJ */
    SerialScreen_ItemSetUp(LCD_PAGE_B_CHGING_BAT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 10.A枪结算 [page:10] */
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "Energy", LCD_DataType, LCD_1sReflash, 0x1610, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].engery), (void *)&LcdData.gun[LCD_GUN_1].engery);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "Hour Used", LCD_DataType, LCD_1sReflash, 0x1612, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].ChrgeTimeHour), (void *)&LcdData.gun[LCD_GUN_1].ChrgeTimeHour);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "Min Used", LCD_DataType, LCD_1sReflash, 0x1614, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].ChrgeTImeMin), (void *)&LcdData.gun[LCD_GUN_1].ChrgeTImeMin);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x1616, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].curSoc), (void *)&LcdData.gun[LCD_GUN_1].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "Chg Money", LCD_DataType, LCD_NoReflash, 0x1618, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].totalFee), (void *)&LcdData.gun[LCD_GUN_1].totalFee);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "ChgStopReason", LCD_TextType, LCD_NoReflash, 0x161A, pstr_type, (sizeof(LcdData.gun[LCD_GUN_1].code_stopResaon) - 1), (void *)&LcdData.gun[LCD_GUN_1].code_stopResaon[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "ChgStopReason", LCD_TextType, LCD_NoReflash, 0x6A81, pstr_type, (sizeof(LcdData.gun[LCD_GUN_1].code_stopResaon_Chinese) - 1), (void *)&LcdData.gun[LCD_GUN_1].code_stopResaon_Chinese[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "unlock_ela", LCD_BtnType, 0x0007, 0x1000, page_type, LCD_PAGE_A_ACOUNT, (void *)SerialScreen_BtnUnElockA);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ACOUNT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 11.B枪结算 [page:11] */
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "Energy", LCD_DataType, LCD_1sReflash, 0x2610, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].engery), (void *)&LcdData.gun[LCD_GUN_2].engery);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "Hour Used", LCD_DataType, LCD_1sReflash, 0x2612, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].ChrgeTimeHour), (void *)&LcdData.gun[LCD_GUN_2].ChrgeTimeHour);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "Min Used", LCD_DataType, LCD_1sReflash, 0x2614, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].ChrgeTImeMin), (void *)&LcdData.gun[LCD_GUN_2].ChrgeTImeMin);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x2616, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].curSoc), (void *)&LcdData.gun[LCD_GUN_2].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "Chg Money", LCD_DataType, LCD_NoReflash, 0x2618, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].totalFee), (void *)&LcdData.gun[LCD_GUN_2].totalFee);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "ChgStopReason", LCD_TextType, LCD_NoReflash, 0x261A, pstr_type, (sizeof(LcdData.gun[LCD_GUN_2].code_stopResaon) - 1), (void *)&LcdData.gun[LCD_GUN_2].code_stopResaon[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "ChgStopReason", LCD_TextType, LCD_NoReflash, 0x6AA1, pstr_type, (sizeof(LcdData.gun[LCD_GUN_2].code_stopResaon_Chinese) - 1), (void *)&LcdData.gun[LCD_GUN_2].code_stopResaon_Chinese[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "unlock_elb", LCD_BtnType, 0x0008, 0x1000, page_type, LCD_PAGE_B_ACOUNT, (void *)SerialScreen_BtnUnElockB);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ACOUNT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 12.A枪故障  [page:12]*/
    SerialScreen_ItemSetUp(LCD_PAGE_A_ERR, NULL, "Agun_Fault", LCD_TextType, LCD_NoReflash, 0x1530, pstr_type, (sizeof(LcdData.gun[LCD_GUN_1].ErrCode) - 1), (void *)&LcdData.gun[LCD_GUN_1].ErrCode[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ERR, NULL, "Agun_Fault", LCD_TextType, LCD_NoReflash, 0x6A01, pstr_type, (sizeof(LcdData.gun[LCD_GUN_1].ErrCode_Chinese) - 1), (void *)&LcdData.gun[LCD_GUN_1].ErrCode_Chinese[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ERR, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ERR, NULL, "unlock_ela", LCD_BtnType, 0x0007, 0x1000, page_type, LCD_PAGE_A_ERR, (void *)SerialScreen_BtnUnElockA);
    SerialScreen_ItemSetUp(LCD_PAGE_A_ERR, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 13.B枪故障 [page:13] */
    SerialScreen_ItemSetUp(LCD_PAGE_B_ERR, NULL, "Bgun_Fault", LCD_TextType, LCD_NoReflash, 0x2530, pstr_type, (sizeof(LcdData.gun[LCD_GUN_2].ErrCode) - 1), (void *)&LcdData.gun[LCD_GUN_2].ErrCode[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ERR, NULL, "Bgun_Fault", LCD_TextType, LCD_NoReflash, 0x6A51, pstr_type, (sizeof(LcdData.gun[LCD_GUN_2].ErrCode_Chinese) - 1), (void *)&LcdData.gun[LCD_GUN_2].ErrCode_Chinese[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ERR, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ERR, NULL, "unlock_elb", LCD_BtnType, 0x0008, 0x1000, page_type, LCD_PAGE_B_ERR, (void *)SerialScreen_BtnUnElockB);
    SerialScreen_ItemSetUp(LCD_PAGE_B_ERR, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 14.A枪停机中 [page:14] */
    SerialScreen_ItemSetUp(LCD_PAGE_A_STOPING, NULL, NULL, LCD_IconType, LCD_NoReflash, 0, 0, 0, NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_A_STOPING, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 15.B枪停机中 [page:15] */
    SerialScreen_ItemSetUp(LCD_PAGE_B_STOPING, NULL, NULL, LCD_IconType, LCD_NoReflash, 0, 0, 0, NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_B_STOPING, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 16.系统信息-桩信息 [page:18] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "menu sys", LCD_BtnType, 0x0025, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "ver_software", LCD_TextType, LCD_NoReflash, 0x1300, pstr_type, sizeof(LcdData.setData.App_SoftWareVersion), (void *)&LcdData.setData.App_SoftWareVersion[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "pileID", LCD_InputType, 0, 0x1310, pstr_type, sizeof(LcdData.setData.pileID), (void *)LcdData.setData.pileID);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "help number", LCD_InputType, 0, 0x11A0, pstr_type, sizeof(LcdData.setData.Help_Number), (void *)LcdData.setData.Help_Number);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "Qrcode pre", LCD_InputType, 0, 0x10A0, pstr_type, (sizeof(LcdData.setData.ErWeiCodePre) - 1), (void *)LcdData.setData.ErWeiCodePre);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "COM_2", LCD_BtnType, 0x000B, 0x1000, page_type, LCD_PAGE_MENU_COM_2, (void *)SerialScreen_BtnServerGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "COM_3", LCD_BtnType, 0x000C, 0x1000, page_type, LCD_PAGE_MENU_COM_3, (void *)SerialScreen_BtnMeterNoInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "COM_4", LCD_BtnType, 0x000D, 0x1000, page_type, LCD_PAGE_MENU_COM_4, (void *)SerialScreen_BtnModuleGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "COM_7", LCD_BtnType, 0x001C, 0x1000, page_type, LCD_PAGE_MENU_COM_7, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnChgInfoSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_1, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 17.系统信息-服务器信息 [page:19] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "menu sys", LCD_BtnType, 0x0025, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "COM_1", LCD_BtnType, 0x000A, 0x1000, page_type, LCD_PAGE_MENU_COM_1, (void *)SerialScreen_BtnChgInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "Server IP", LCD_InputType, 0, 0x1320, pstr_type, sizeof(LcdData.setData.svrIp), (void *)LcdData.setData.svrIp);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "Server port", LCD_InputType, 0, 0x133B, pu16_type, sizeof(LcdData.setData.svrPort), (void *)&LcdData.setData.svrPort);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "COM_3", LCD_BtnType, 0x000C, 0x1000, page_type, LCD_PAGE_MENU_COM_3, (void *)SerialScreen_BtnMeterNoInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "COM_4", LCD_BtnType, 0x000D, 0x1000, page_type, LCD_PAGE_MENU_COM_4, (void *)SerialScreen_BtnModuleGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "COM_5", LCD_BtnType, 0x000E, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "COM_7", LCD_BtnType, 0x001C, 0x1000, page_type, LCD_PAGE_MENU_COM_7, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "NetType", LCD_InputType, 0, 0x4652, menu_type, sizeof(LcdData.setData.NetType), (void *)&LcdData.setData.NetType);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnServerSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_2, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 18.系统信息-电表 [page:20] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "menu sys", LCD_BtnType, 0x0025, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "COM_1", LCD_BtnType, 0x000A, 0x1000, page_type, LCD_PAGE_MENU_COM_1, (void *)SerialScreen_BtnChgInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "COM_2", LCD_BtnType, 0x000B, 0x1000, page_type, LCD_PAGE_MENU_COM_2, (void *)SerialScreen_BtnServerGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "COM_4", LCD_BtnType, 0x000D, 0x1000, page_type, LCD_PAGE_MENU_COM_4, (void *)SerialScreen_BtnModuleGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "COM_5", LCD_BtnType, 0x000E, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "COM_7", LCD_BtnType, 0x001C, 0x1000, page_type, LCD_PAGE_MENU_COM_7, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "meter noA", LCD_InputType, 0, 0x4053, pstr_type, sizeof(LcdData.setData.MeterAddr[LCD_GUN_1]), (void *)LcdData.setData.MeterAddr[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "meter noB", LCD_InputType, 0, 0x4063, pstr_type, sizeof(LcdData.setData.MeterAddr[LCD_GUN_2]), (void *)LcdData.setData.MeterAddr[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnMeterNoInfoSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "meter model", LCD_InputType, 0, 0x4640, menu_type, sizeof(LcdData.setData.MeterModel), (void *)&LcdData.setData.MeterModel);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "meter check", LCD_InputType, 0, 0x53C2, menu_type, sizeof(LcdData.setData.MeterCheckWay), (void *)&LcdData.setData.MeterCheckWay);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "meter baudrate", LCD_InputType, 0, 0x53B2, menu_type, sizeof(LcdData.setData.MeterBaudrate), (void *)&LcdData.setData.MeterBaudrate);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_3, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 19.系统信息-模块信息 [page:21] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "menu sys", LCD_BtnType, 0x0025, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "COM_1", LCD_BtnType, 0x000A, 0x1000, page_type, LCD_PAGE_MENU_COM_1, (void *)SerialScreen_BtnChgInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "COM_2", LCD_BtnType, 0x000B, 0x1000, page_type, LCD_PAGE_MENU_COM_2, (void *)SerialScreen_BtnServerGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "COM_3", LCD_BtnType, 0x000C, 0x1000, page_type, LCD_PAGE_MENU_COM_3, (void *)SerialScreen_BtnMeterNoInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "COM_7", LCD_BtnType, 0x001C, 0x1000, page_type, LCD_PAGE_MENU_COM_7, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Rm Pro", LCD_InputType, 0, 0x1340, menu_type, sizeof(LcdData.setData.RmType), (void *)&LcdData.setData.RmType);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Group Num", LCD_InputType, 0, 0x1350, pu8_type, sizeof(LcdData.setData.ModuleGroupNum), (void *)&LcdData.setData.ModuleGroupNum);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Group1 Num", LCD_InputType, 0, 0x1352, pu8_type, sizeof(LcdData.setData.ModuleNum[0]), (void *)&LcdData.setData.ModuleNum[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Group2 Num", LCD_InputType, 0, 0x1354, pu8_type, sizeof(LcdData.setData.ModuleNum[1]), (void *)&LcdData.setData.ModuleNum[1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Group3 Num", LCD_InputType, 0, 0x136B, pu8_type, sizeof(LcdData.setData.ModuleNum[2]), (void *)&LcdData.setData.ModuleNum[2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Group4 Num", LCD_InputType, 0, 0x1370, pu8_type, sizeof(LcdData.setData.ModuleNum[3]), (void *)&LcdData.setData.ModuleNum[3]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Rated_Voltage", LCD_InputType, 0, 0x6100, pu16_type, sizeof(LcdData.setData.Rated_Output_Voltage), (void *)&LcdData.setData.Rated_Output_Voltage);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Max_Output_Voltage", LCD_InputType, 0, 0x135B, pu16_type, sizeof(LcdData.setData.Max_Output_Voltage), (void *)&LcdData.setData.Max_Output_Voltage);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Min_Output_Voltage", LCD_InputType, 0, 0x1364, pu16_type, sizeof(LcdData.setData.Min_Output_Voltage), (void *)&LcdData.setData.Min_Output_Voltage);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Rated_Current", LCD_InputType, 0, 0x1358, pu16_type, sizeof(LcdData.setData.Rated_Limit_Current), (void *)&LcdData.setData.Rated_Limit_Current);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Max_Limit_Current", LCD_InputType, 0, 0x1360, pu16_type, sizeof(LcdData.setData.Max_Limit_Current), (void *)&LcdData.setData.Max_Limit_Current);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "Min_Limit_Curren", LCD_InputType, 0, 0x1368, pu16_type, sizeof(LcdData.setData.Min_Limit_Current), (void *)&LcdData.setData.Min_Limit_Current);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnModuleSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "COM_5", LCD_BtnType, 0x000E, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_4, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 20.A枪故障记录信息 [page:22] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "COM_6", LCD_BtnType, 0x0010, 0x1000, page_type, LCD_PAGE_MENU_COM_6, (void *)SerialScreen_BtnBillGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "COM_5B", LCD_BtnType, 0x0015, 0x1000, page_type, LCD_PAGE_MENU_COM_5B, (void *)SerialScreen_BtnErrGetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "sys info", LCD_BtnType, 0x0018, 0x1000, page_type, LCD_PAGE_SYS_INFO, (void *)SerialScreen_SysInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "root paswd", LCD_BtnType, 0x0017, 0x1000, page_type, LCD_PAGE_ADMIN_PASWD, (void *)SerialScreen_NeedPageReset);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-0", LCD_TextType, LCD_NoReflash, 0x3000, pstr_type, sizeof(LcdData.ErrInfo[0]), (void *)(LcdData.ErrInfo[0]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-1", LCD_TextType, LCD_NoReflash, 0x3080, pstr_type, sizeof(LcdData.ErrInfo[1]), (void *)(LcdData.ErrInfo[1]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-2", LCD_TextType, LCD_NoReflash, 0x3100, pstr_type, sizeof(LcdData.ErrInfo[2]), (void *)(LcdData.ErrInfo[2]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-3", LCD_TextType, LCD_NoReflash, 0x3180, pstr_type, sizeof(LcdData.ErrInfo[3]), (void *)(LcdData.ErrInfo[3]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-4", LCD_TextType, LCD_NoReflash, 0x3200, pstr_type, sizeof(LcdData.ErrInfo[4]), (void *)(LcdData.ErrInfo[4]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-5", LCD_TextType, LCD_NoReflash, 0x3280, pstr_type, sizeof(LcdData.ErrInfo[5]), (void *)(LcdData.ErrInfo[5]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-6", LCD_TextType, LCD_NoReflash, 0x3300, pstr_type, sizeof(LcdData.ErrInfo[6]), (void *)(LcdData.ErrInfo[6]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-7", LCD_TextType, LCD_NoReflash, 0x3380, pstr_type, sizeof(LcdData.ErrInfo[7]), (void *)(LcdData.ErrInfo[7]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-8", LCD_TextType, LCD_NoReflash, 0x3400, pstr_type, sizeof(LcdData.ErrInfo[8]), (void *)(LcdData.ErrInfo[8]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "err-9", LCD_TextType, LCD_NoReflash, 0x3480, pstr_type, sizeof(LcdData.ErrInfo[9]), (void *)(LcdData.ErrInfo[9]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "up", LCD_TrigType, 0x0009, 0x1003, page_type, LCD_GUN_1, (void *)SerialScreen_BtnErrUp);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "down", LCD_TrigType, 0x000E, 0x1004, page_type, LCD_GUN_1, (void *)SerialScreen_BtnErrDown);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 21.A枪充电记录信息 [page:23] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "COM_6", LCD_BtnType, 0x000F, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-0", LCD_TextType, LCD_NoReflash, 0x3500, pstr_type, sizeof(LcdData.billInfo[0]), (void *)(LcdData.billInfo[0]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-1", LCD_TextType, LCD_NoReflash, 0x3580, pstr_type, sizeof(LcdData.billInfo[1]), (void *)(LcdData.billInfo[1]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-2", LCD_TextType, LCD_NoReflash, 0x3600, pstr_type, sizeof(LcdData.billInfo[2]), (void *)(LcdData.billInfo[2]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-3", LCD_TextType, LCD_NoReflash, 0x3680, pstr_type, sizeof(LcdData.billInfo[3]), (void *)(LcdData.billInfo[3]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-4", LCD_TextType, LCD_NoReflash, 0x3700, pstr_type, sizeof(LcdData.billInfo[4]), (void *)(LcdData.billInfo[4]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-5", LCD_TextType, LCD_NoReflash, 0x3780, pstr_type, sizeof(LcdData.billInfo[5]), (void *)(LcdData.billInfo[5]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-6", LCD_TextType, LCD_NoReflash, 0x3800, pstr_type, sizeof(LcdData.billInfo[6]), (void *)(LcdData.billInfo[6]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-7", LCD_TextType, LCD_NoReflash, 0x3880, pstr_type, sizeof(LcdData.billInfo[7]), (void *)(LcdData.billInfo[7]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-8", LCD_TextType, LCD_NoReflash, 0x3900, pstr_type, sizeof(LcdData.billInfo[8]), (void *)(LcdData.billInfo[8]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "bill-9", LCD_TextType, LCD_NoReflash, 0x3980, pstr_type, sizeof(LcdData.billInfo[9]), (void *)(LcdData.billInfo[9]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "COM_6B", LCD_BtnType, 0x0015, 0x1000, page_type, LCD_PAGE_MENU_COM_6B, (void *)SerialScreen_BtnBillGetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "sys info", LCD_BtnType, 0x0018, 0x1000, page_type, LCD_PAGE_SYS_INFO, (void *)SerialScreen_SysInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "root paswd", LCD_BtnType, 0x0017, 0x1000, page_type, LCD_PAGE_ADMIN_PASWD, (void *)SerialScreen_NeedPageReset);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "up", LCD_TrigType, 0x0009, 0x1003, page_type, LCD_GUN_1, (void *)SerialScreen_BtnBillUp);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "down", LCD_TrigType, 0x000E, 0x1004, page_type, LCD_GUN_1, (void *)SerialScreen_BtnBillDown);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 22.密码输入 [page:24] */
    SerialScreen_ItemSetUp(LCD_PAGE_ADMIN_PASWD, NULL, "passwd", LCD_inputPwdType, 10, 0x1210, pstr_type, LCD_PAGE_ROOT_MAIN, (void *)&LcdData.setData.UserPasswdShow);
    SerialScreen_ItemSetUp(LCD_PAGE_ADMIN_PASWD, NULL, "back", LCD_BtnType, 0x0013, 0x1000, page_type, LCD_PAGE_SYS_INFO, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_ADMIN_PASWD, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 23.B枪充电记录信息 [page:27] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "COM_5", LCD_BtnType, 0x000F, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-0", LCD_TextType, LCD_NoReflash, 0x3500, pstr_type, sizeof(LcdData.billInfo[0]), (void *)(LcdData.billInfo[0]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-1", LCD_TextType, LCD_NoReflash, 0x3580, pstr_type, sizeof(LcdData.billInfo[1]), (void *)(LcdData.billInfo[1]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-2", LCD_TextType, LCD_NoReflash, 0x3600, pstr_type, sizeof(LcdData.billInfo[2]), (void *)(LcdData.billInfo[2]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-3", LCD_TextType, LCD_NoReflash, 0x3680, pstr_type, sizeof(LcdData.billInfo[3]), (void *)(LcdData.billInfo[3]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-4", LCD_TextType, LCD_NoReflash, 0x3700, pstr_type, sizeof(LcdData.billInfo[4]), (void *)(LcdData.billInfo[4]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-5", LCD_TextType, LCD_NoReflash, 0x3780, pstr_type, sizeof(LcdData.billInfo[5]), (void *)(LcdData.billInfo[5]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-6", LCD_TextType, LCD_NoReflash, 0x3800, pstr_type, sizeof(LcdData.billInfo[6]), (void *)(LcdData.billInfo[6]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-7", LCD_TextType, LCD_NoReflash, 0x3880, pstr_type, sizeof(LcdData.billInfo[7]), (void *)(LcdData.billInfo[7]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-8", LCD_TextType, LCD_NoReflash, 0x3900, pstr_type, sizeof(LcdData.billInfo[8]), (void *)(LcdData.billInfo[8]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "bill-9", LCD_TextType, LCD_NoReflash, 0x3980, pstr_type, sizeof(LcdData.billInfo[9]), (void *)(LcdData.billInfo[9]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "COM_6", LCD_BtnType, 0x0015, 0x1000, page_type, LCD_PAGE_MENU_COM_6, (void *)SerialScreen_BtnBillGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "sys info", LCD_BtnType, 0x0018, 0x1000, page_type, LCD_PAGE_SYS_INFO, (void *)SerialScreen_SysInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "root paswd", LCD_BtnType, 0x0017, 0x1000, page_type, LCD_PAGE_ADMIN_PASWD, (void *)SerialScreen_NeedPageReset);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "up", LCD_TrigType, 0x0009, 0x1000, page_type, LCD_GUN_2, (void *)SerialScreen_BtnBillUp);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "down", LCD_TrigType, 0x000E, 0x1000, page_type, LCD_GUN_2, (void *)SerialScreen_BtnBillDown);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_6B, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 24.B枪故障记录信息 [page:28] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "COM_6", LCD_BtnType, 0x0010, 0x1000, page_type, LCD_PAGE_MENU_COM_6, (void *)SerialScreen_BtnBillGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "COM_5B", LCD_BtnType, 0x0015, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "sys info", LCD_BtnType, 0x0018, 0x1000, page_type, LCD_PAGE_SYS_INFO, (void *)SerialScreen_SysInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "root paswd", LCD_BtnType, 0x0017, 0x1000, page_type, LCD_PAGE_ADMIN_PASWD, (void *)SerialScreen_NeedPageReset);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-0", LCD_TextType, LCD_NoReflash, 0x3000, pstr_type, sizeof(LcdData.ErrInfo[0]), (void *)(LcdData.ErrInfo[0]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-1", LCD_TextType, LCD_NoReflash, 0x3080, pstr_type, sizeof(LcdData.ErrInfo[1]), (void *)(LcdData.ErrInfo[1]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-2", LCD_TextType, LCD_NoReflash, 0x3100, pstr_type, sizeof(LcdData.ErrInfo[2]), (void *)(LcdData.ErrInfo[2]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-3", LCD_TextType, LCD_NoReflash, 0x3180, pstr_type, sizeof(LcdData.ErrInfo[3]), (void *)(LcdData.ErrInfo[3]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-4", LCD_TextType, LCD_NoReflash, 0x3200, pstr_type, sizeof(LcdData.ErrInfo[4]), (void *)(LcdData.ErrInfo[4]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-5", LCD_TextType, LCD_NoReflash, 0x3280, pstr_type, sizeof(LcdData.ErrInfo[5]), (void *)(LcdData.ErrInfo[5]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-6", LCD_TextType, LCD_NoReflash, 0x3300, pstr_type, sizeof(LcdData.ErrInfo[6]), (void *)(LcdData.ErrInfo[6]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-7", LCD_TextType, LCD_NoReflash, 0x3380, pstr_type, sizeof(LcdData.ErrInfo[7]), (void *)(LcdData.ErrInfo[7]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-8", LCD_TextType, LCD_NoReflash, 0x3400, pstr_type, sizeof(LcdData.ErrInfo[8]), (void *)(LcdData.ErrInfo[8]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "err-9", LCD_TextType, LCD_NoReflash, 0x3480, pstr_type, sizeof(LcdData.ErrInfo[9]), (void *)(LcdData.ErrInfo[9]));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "up", LCD_TrigType, 0x0009, 0x1000, page_type, LCD_GUN_2, (void *)SerialScreen_BtnErrUp);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "down", LCD_TrigType, 0x000E, 0x1000, page_type, LCD_GUN_2, (void *)SerialScreen_BtnErrDown);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "back", LCD_BtnHomeType, 0x0002, 0x1000, page_type, 0, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_5B, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 25.密码错误 [page:29] */
    SerialScreen_ItemSetUp(LCD_PAGE_PASWD_ERR, NULL, "cdup", LCD_BtnType, 0x0016, 0x1000, page_type, LCD_PAGE_ADMIN_PASWD, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_PASWD_ERR, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 26.远程升级 [page:37] */
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_UPDATE, NULL, "Progress value", LCD_InputType, 0, 0x1800, pu16_type, sizeof(LcdData.setData.ota_progress), (void *)&LcdData.setData.ota_progress);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_UPDATE, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 27.系统信息 [page:38] */
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "Longitude", LCD_DataType, LCD_NoReflash, 0x4000, pu32_type, sizeof(LcdData.setData.Longitude), (void *)&LcdData.setData.Longitude);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "Latitude", LCD_DataType, LCD_NoReflash, 0x4004, pu32_type, sizeof(LcdData.setData.Latitude), (void *)&LcdData.setData.Latitude);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "Server IP", LCD_TextType, LCD_NoReflash, 0x1320, pstr_type, sizeof(LcdData.setData.svrIp), (void *)LcdData.setData.svrIp);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "pileID", LCD_TextType, LCD_NoReflash, 0x1310, pstr_type, sizeof(LcdData.setData.pileID), (void *)LcdData.setData.pileID);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "ver_software", LCD_TextType, LCD_NoReflash, 0x1300, pstr_type, sizeof(LcdData.setData.App_SoftWareVersion), (void *)(void *)&LcdData.setData.App_SoftWareVersion[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "Sim Card", LCD_TextType, LCD_NoReflash, 0x4008, pstr_type, sizeof(LcdData.setData.SIM_card), (void *)&LcdData.setData.SIM_card[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "Sim Strength", LCD_DataType, LCD_NoReflash, 0x4200, pu32_type, sizeof(LcdData.setData.SIM_Strength), (void *)&LcdData.setData.SIM_Strength);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "COM_6", LCD_BtnType, 0x0010, 0x1000, page_type, LCD_PAGE_MENU_COM_6, (void *)SerialScreen_BtnBillGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "COM_5", LCD_BtnType, 0x000F, 0x1000, page_type, LCD_PAGE_MENU_COM_5, (void *)SerialScreen_BtnErrGetA);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "root paswd", LCD_BtnType, 0x0017, 0x1000, page_type, LCD_PAGE_ADMIN_PASWD, (void *)SerialScreen_NeedPageReset);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_SYS_INFO, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 28.主界面 [page:39] */
    SerialScreen_ItemSetUp(LCD_PAGE_ROOT_MAIN, NULL, "sys info", LCD_BtnType, 0x0019, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_ROOT_MAIN, NULL, "factory set", LCD_BtnType, 0x001A, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_InputInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_ROOT_MAIN, NULL, "factory debug", LCD_BtnType, 0x001B, 0x1000, page_type, LCD_PAGE_MENU_MONITOR, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_ROOT_MAIN, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_ROOT_MAIN, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 29.系统信息-VIN码 [page:40] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "COM_1", LCD_BtnType, 0x000A, 0x1000, page_type, LCD_PAGE_MENU_COM_1, (void *)SerialScreen_BtnChgInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "COM_2", LCD_BtnType, 0x000B, 0x1000, page_type, LCD_PAGE_MENU_COM_2, (void *)SerialScreen_BtnServerGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "COM_3", LCD_BtnType, 0x000C, 0x1000, page_type, LCD_PAGE_MENU_COM_3, (void *)SerialScreen_BtnMeterNoInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "COM_4", LCD_BtnType, 0x000D, 0x1000, page_type, LCD_PAGE_MENU_COM_4, (void *)SerialScreen_BtnModuleGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnVinListSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "menu sys", LCD_BtnType, 0x0025, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "vin1", LCD_InputType, 0, 0x4073, pstr_type, sizeof(LcdData.setData.s_vin_lists[0]), (void *)LcdData.setData.s_vin_lists[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "vin2", LCD_InputType, 0, 0x4084, pstr_type, sizeof(LcdData.setData.s_vin_lists[1]), (void *)LcdData.setData.s_vin_lists[1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "vin3", LCD_InputType, 0, 0x4095, pstr_type, sizeof(LcdData.setData.s_vin_lists[2]), (void *)LcdData.setData.s_vin_lists[2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "vin4", LCD_InputType, 0, 0x40A6, pstr_type, sizeof(LcdData.setData.s_vin_lists[3]), (void *)LcdData.setData.s_vin_lists[3]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "vin5", LCD_InputType, 0, 0x40B7, pstr_type, sizeof(LcdData.setData.s_vin_lists[4]), (void *)LcdData.setData.s_vin_lists[4]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "vin6", LCD_InputType, 0, 0x40C8, pstr_type, sizeof(LcdData.setData.s_vin_lists[5]), (void *)LcdData.setData.s_vin_lists[5]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_COM_7, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 30.出厂设置-输入信息 [page:41] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "menu protect", LCD_BtnType, 0x0020, 0x1000, page_type, LCD_PAGE_MENU_PROTECT, (void *)SerialScreen_BtnProtectInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "menu config", LCD_BtnType, 0x0021, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportGet);
#ifdef SCREEN_USING_OFFLINE_BILLING
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "menu offbilling", LCD_BtnType, 0x0053, 0x1000, page_type, LCD_PAGE_OFFLINE_BILLING, (void *)SerialScreen_OfflineBillingGet);
#endif /* SCREEN_USING_OFFLINE_BILLING */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Scram sup", LCD_IconType, LCD_10sReflash, 0x4100, pu8_type, sizeof(LcdData.setData.supin_scram), (void *)&LcdData.setData.supin_scram);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Gate sup", LCD_IconType, LCD_10sReflash, 0x4102, pu8_type, sizeof(LcdData.setData.supin_gate), (void *)&LcdData.setData.supin_gate);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon AC sup", LCD_IconType, LCD_10sReflash, 0x4104, pu8_type, sizeof(LcdData.setData.supin_ac), (void *)&LcdData.setData.supin_ac);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon DC sup", LCD_IconType, LCD_10sReflash, 0x4106, pu8_type, sizeof(LcdData.setData.supin_dc), (void *)&LcdData.setData.supin_dc);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Fan sup", LCD_IconType, LCD_10sReflash, 0x4108, pu8_type, sizeof(LcdData.setData.supin_fan), (void *)&LcdData.setData.supin_fan);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Elock sup", LCD_IconType, LCD_10sReflash, 0x410A, pu8_type, sizeof(LcdData.setData.supin_elock), (void *)&LcdData.setData.supin_elock);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon TempPro sup", LCD_IconType, LCD_10sReflash, 0x411E, pu8_type, sizeof(LcdData.setData.supin_temp_pro), (void *)&LcdData.setData.supin_temp_pro);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon ProLight sup", LCD_IconType, LCD_10sReflash, 0x6508, pu8_type, sizeof(LcdData.setData.Icon_SupProtectLight), (void *)&LcdData.setData.Icon_SupProtectLight);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon GunSite sup", LCD_IconType, LCD_10sReflash, 0x650E, pu8_type, sizeof(LcdData.setData.Icon_SupGunSite), (void *)&LcdData.setData.Icon_SupGunSite);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Breaker sup", LCD_IconType, LCD_10sReflash, 0x6512, pu8_type, sizeof(LcdData.setData.Icon_SupCircuitBreaker), (void *)&LcdData.setData.Icon_SupCircuitBreaker);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Flood sup", LCD_IconType, LCD_10sReflash, 0x650A, pu8_type, sizeof(LcdData.setData.Icon_SupFlood), (void *)&LcdData.setData.Icon_SupFlood);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Smoke sup", LCD_IconType, LCD_10sReflash, 0x650C, pu8_type, sizeof(LcdData.setData.Icon_SupSmoke), (void *)&LcdData.setData.Icon_SupSmoke);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Pour sup", LCD_IconType, LCD_10sReflash, 0x6506, pu8_type, sizeof(LcdData.setData.Icon_SupPour), (void *)&LcdData.setData.Icon_SupPour);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Liquid sup", LCD_IconType, LCD_10sReflash, 0x6514, pu8_type, sizeof(LcdData.setData.Icon_SupLiquid), (void *)&LcdData.setData.Icon_SupLiquid);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Fuse sup", LCD_IconType, LCD_10sReflash, 0x6510, pu8_type, sizeof(LcdData.setData.Icon_SupFuse), (void *)&LcdData.setData.Icon_SupFuse);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Scram neg", LCD_IconType, LCD_10sReflash, 0x410C, pu8_type, sizeof(LcdData.setData.neg_scram), (void *)&LcdData.setData.neg_scram);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Gate neg", LCD_IconType, LCD_10sReflash, 0x410E, pu8_type, sizeof(LcdData.setData.neg_gate), (void *)&LcdData.setData.neg_gate);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Ac neg", LCD_IconType, LCD_10sReflash, 0x4110, pu8_type, sizeof(LcdData.setData.neg_ac), (void *)&LcdData.setData.neg_ac);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Dc neg", LCD_IconType, LCD_10sReflash, 0x4112, pu8_type, sizeof(LcdData.setData.neg_dc), (void *)&LcdData.setData.neg_dc);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Fan neg", LCD_IconType, LCD_10sReflash, 0x4114, pu8_type, sizeof(LcdData.setData.neg_fan), (void *)&LcdData.setData.neg_fan);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Elock neg", LCD_IconType, LCD_10sReflash, 0x4116, pu8_type, sizeof(LcdData.setData.neg_elcok), (void *)&LcdData.setData.neg_elcok);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon ProLight neg", LCD_IconType, LCD_10sReflash, 0x6509, pu8_type, sizeof(LcdData.setData.Icon_NegProtectLight), (void *)&LcdData.setData.Icon_NegProtectLight);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon GunSite neg", LCD_IconType, LCD_10sReflash, 0x650F, pu8_type, sizeof(LcdData.setData.Icon_NegGunSite), (void *)&LcdData.setData.Icon_NegGunSite);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Breaker neg", LCD_IconType, LCD_10sReflash, 0x6513, pu8_type, sizeof(LcdData.setData.Icon_NegCircuitBreaker), (void *)&LcdData.setData.Icon_NegCircuitBreaker);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Flood neg", LCD_IconType, LCD_10sReflash, 0x650B, pu8_type, sizeof(LcdData.setData.Icon_NegFlood), (void *)&LcdData.setData.Icon_NegFlood);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Smoke neg", LCD_IconType, LCD_10sReflash, 0x650D, pu8_type, sizeof(LcdData.setData.Icon_NegSmoke), (void *)&LcdData.setData.Icon_NegSmoke);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Pour neg", LCD_IconType, LCD_10sReflash, 0x6507, pu8_type, sizeof(LcdData.setData.Icon_NegPour), (void *)&LcdData.setData.Icon_NegPour);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Liquid neg", LCD_IconType, LCD_10sReflash, 0x6515, pu8_type, sizeof(LcdData.setData.Icon_NegLiquid), (void *)&LcdData.setData.Icon_NegLiquid);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Icon Fuse neg", LCD_IconType, LCD_10sReflash, 0x6511, pu8_type, sizeof(LcdData.setData.Icon_NegFuse), (void *)&LcdData.setData.Icon_NegFuse);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Scram set", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_ScramIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Scram neg set", LCD_BtnType, 0x002C, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_ScramNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Gate set", LCD_BtnType, 0x0027, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_GateIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Gate neg set", LCD_BtnType, 0x002D, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_GateNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Ac set", LCD_BtnType, 0x0028, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_AcIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Ac neg set", LCD_BtnType, 0x002E, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_AcNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Dc set", LCD_BtnType, 0x0029, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_DcIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Dc neg set", LCD_BtnType, 0x002F, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_DcNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Fan set", LCD_BtnType, 0x002A, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_FanIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Fan neg set", LCD_BtnType, 0x0030, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_FanNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Elock set", LCD_BtnType, 0x002B, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_ElockIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Elock neg set", LCD_BtnType, 0x0031, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_ElockNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "TempPro set", LCD_BtnType, 0x0032, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_TempProIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "ProLight set", LCD_BtnType, 0x0052, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_ProLightIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "ProLight neg set", LCD_BtnType, 0x0053, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_ProLightNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "GunSite set", LCD_BtnType, 0x0058, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_GunSiteIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "GunSite neg set", LCD_BtnType, 0x0059, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_GunSiteNegIsSupportSet);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Breaker set", LCD_BtnType, 0x0059, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_BreakerIsSupportSet);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Breaker neg set", LCD_BtnType, 0x0059, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_BreakerNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Flood set", LCD_BtnType, 0x0054, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_FloodIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Flood neg set", LCD_BtnType, 0x0055, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_FloodNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Smoke set", LCD_BtnType, 0x0056, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_SmokeIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Smoke neg set", LCD_BtnType, 0x0057, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_SmokeNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Pour set", LCD_BtnType, 0x0050, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_PourIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Pour neg set", LCD_BtnType, 0x0051, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_PourNegIsSupportSet);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Liquid set", LCD_BtnType, 0x0050, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_LiquidIsSupportSet);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Liquid neg set", LCD_BtnType, 0x0051, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_LiquidNegIsSupportSet);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Fuse set", LCD_BtnType, 0x005A, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_FuseIsSupportSet);
//    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Fuse neg set", LCD_BtnType, 0x005B, 0x1003, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_FuseNegIsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_InputSetFlash);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INPUT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 31.出厂设置-输出信息 [page:42] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_OUTPUT, NULL, "menu input", LCD_BtnType, 0x001E, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_InputInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_OUTPUT, NULL, "menu protect", LCD_BtnType, 0x0020, 0x1000, page_type, LCD_PAGE_MENU_PROTECT, (void *)SerialScreen_BtnProtectInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_OUTPUT, NULL, "menu config", LCD_BtnType, 0x0021, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_OUTPUT, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_OUTPUT, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_OUTPUT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 32.出厂设置-保护信息 [page:43] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "menu input", LCD_BtnType, 0x001E, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_InputInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "menu config", LCD_BtnType, 0x0021, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportGet);
#ifdef SCREEN_USING_OFFLINE_BILLING
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "menu offbilling", LCD_BtnType, 0x0053, 0x1000, page_type, LCD_PAGE_OFFLINE_BILLING, (void *)SerialScreen_OfflineBillingGet);
#endif /* SCREEN_USING_OFFLINE_BILLING */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnProtectInfoSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Icon shield", LCD_IconType, LCD_10sReflash, 0x6516, pu8_type, sizeof(LcdData.setData.Icon_ProtectInfo), (void *)&LcdData.setData.Icon_ProtectInfo);
#if 0
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "In_OV", LCD_InputType, 0, 0x414E, pu32_type, sizeof(LcdData.setData.Input_OverVolt), (void *)&LcdData.setData.Input_OverVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "In_UV", LCD_InputType, 0, 0x4130, pu32_type, sizeof(LcdData.setData.Input_UnderVolt), (void *)&LcdData.setData.Input_UnderVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Ou_OV", LCD_InputType, 0, 0x4132, pu32_type, sizeof(LcdData.setData.Onput_OverVolt), (void *)&LcdData.setData.Onput_OverVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Ou_UV", LCD_InputType, 0, 0x4134, pu32_type, sizeof(LcdData.setData.Onput_UnderVolt), (void *)&LcdData.setData.Onput_UnderVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Ou_OC", LCD_InputType, 0, 0x4136, pu32_type, sizeof(LcdData.setData.Onput_OverCurr), (void *)&LcdData.setData.Onput_OverCurr);
#else
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "In_OV", LCD_InputType, 0, 0x8010, pu32_type, sizeof(LcdData.setData.Input_OverVolt), (void *)&LcdData.setData.Input_OverVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "In_UV", LCD_InputType, 0, 0x8020, pu32_type, sizeof(LcdData.setData.Input_UnderVolt), (void *)&LcdData.setData.Input_UnderVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Ou_OV", LCD_InputType, 0, 0x8030, pu32_type, sizeof(LcdData.setData.Onput_OverVolt), (void *)&LcdData.setData.Onput_OverVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Ou_UV", LCD_InputType, 0, 0x8040, pu32_type, sizeof(LcdData.setData.Onput_UnderVolt), (void *)&LcdData.setData.Onput_UnderVolt);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "Ou_OC", LCD_InputType, 0, 0x8050, pu32_type, sizeof(LcdData.setData.Onput_OverCurr), (void *)&LcdData.setData.Onput_OverCurr);
#endif
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "StopSOC", LCD_InputType, 0, 0x4138, pu32_type, sizeof(LcdData.setData.Stop_SOC), (void *)&LcdData.setData.Stop_SOC);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "OT_W", LCD_InputType, 0, 0x413A, pu32_type, sizeof(LcdData.setData.OverTemp_Warnning), (void *)&LcdData.setData.OverTemp_Warnning);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "OT_S", LCD_InputType, 0, 0x413C, pu32_type, sizeof(LcdData.setData.OverTemp_Stop), (void *)&LcdData.setData.OverTemp_Stop);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "OT_R", LCD_InputType, 0, 0x413E, pu32_type, sizeof(LcdData.setData.OverTemp_Resume), (void *)&LcdData.setData.OverTemp_Resume);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "OT_L", LCD_InputType, 0, 0x4140, pu32_type, sizeof(LcdData.setData.OverTemp_LimitCurr), (void *)&LcdData.setData.OverTemp_LimitCurr);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "GunVolt", LCD_InputType, 0, 0x4144, pu16_type, sizeof(LcdData.setData.GunVolt_LimitValue), (void *)&LcdData.setData.GunVolt_LimitValue);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "powerP", LCD_InputType, 0, 0x4148, pu16_type, sizeof(LcdData.setData.PowerPercent), (void *)&LcdData.setData.PowerPercent);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "ElossP", LCD_InputType, 0, 0x4660, pu16_type, sizeof(LcdData.setData.ElossProprotion), (void *)&LcdData.setData.ElossProprotion);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_PROTECT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 33.出厂设置-功能配置 [page:44] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "locoal set", LCD_BtnType, 0x003d, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "plug and play set", LCD_BtnType, 0x003B, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportPlugAndPlaySet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "locoal stop", LCD_BtnType, 0x0002, 0x1009, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportLocalStopSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "auxp24v set", LCD_BtnType, 0x0006, 0x1007, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportAuxp24VSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "paracharge set", LCD_BtnType, 0x0005, 0x1007, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportParaChargeSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "pararelay set", LCD_BtnType, 0x0002, 0x1001, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportParaRelaySet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "reader set", LCD_BtnType, 0x003C, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportReaderSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "vin set", LCD_BtnType, 0x0039, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportVINSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "insulation set", LCD_BtnType, 0x0038, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportIsulationSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "menu input", LCD_BtnType, 0x001E, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_InputInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "menu protect", LCD_BtnType, 0x0020, 0x1000, page_type, LCD_PAGE_MENU_PROTECT, (void *)SerialScreen_BtnProtectInfoGet);
#ifdef SCREEN_USING_OFFLINE_BILLING
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "menu offbilling", LCD_BtnType, 0x0053, 0x1000, page_type, LCD_PAGE_OFFLINE_BILLING, (void *)SerialScreen_OfflineBillingGet);
#endif /* SCREEN_USING_OFFLINE_BILLING */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "module slience set", LCD_BtnType, 0x0001, 0x1005, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportModuleSlienceSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "offline billing set", LCD_BtnType, 0x0001, 0x1009, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportOfflineBillingSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "pw start set", LCD_BtnType, 0x0004, 0x1001, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportPWStartSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "off card set", LCD_BtnType, 0x006E, 0x1003, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportOfflineCardSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "plug and play", LCD_IconType, LCD_10sReflash, 0x412A, pu8_type, sizeof(LcdData.setData.Icon_SupPlugAndPlay), (void *)&LcdData.setData.Icon_SupPlugAndPlay);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "offline card", LCD_IconType, LCD_10sReflash, 0x6A00, pu8_type, sizeof(LcdData.setData.Icon_SupOffCard), (void *)&LcdData.setData.Icon_SupOffCard);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "locoal charge", LCD_IconType, LCD_10sReflash, 0x412E, pu8_type, sizeof(LcdData.setData.sup_Local), (void *)&LcdData.setData.sup_Local);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "locoal stop", LCD_IconType, LCD_10sReflash, 0x6110, pu8_type, sizeof(LcdData.setData.Icon_SuplocalStop), (void *)&LcdData.setData.Icon_SuplocalStop);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "auxp24v", LCD_IconType, LCD_10sReflash, 0x465A, pu8_type, sizeof(LcdData.setData.sup_auxp_24V), (void *)&LcdData.setData.sup_auxp_24V);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "paracharge", LCD_IconType, LCD_10sReflash, 0x4658, pu8_type, sizeof(LcdData.setData.sup_parallelchg), (void *)&LcdData.setData.sup_parallelchg);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "pararelay", LCD_IconType, LCD_10sReflash, 0x465C, pu8_type, sizeof(LcdData.setData.sup_parallelrelay), (void *)&LcdData.setData.sup_parallelrelay);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "reader enable", LCD_IconType, LCD_10sReflash, 0x412C, pu8_type, sizeof(LcdData.setData.sup_usecard), (void *)&LcdData.setData.sup_usecard);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "insulation set", LCD_IconType, LCD_10sReflash, 0x4124, pu8_type, sizeof(LcdData.setData.sup_insulation), (void *)&LcdData.setData.sup_insulation);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "vin set", LCD_IconType, LCD_10sReflash, 0x4126, pu8_type, sizeof(LcdData.setData.sup_VIN), (void *)&LcdData.setData.sup_VIN);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "module slience", LCD_IconType, LCD_10sReflash, 0x4304, pu8_type, sizeof(LcdData.setData.sup_mslience), (void *)&LcdData.setData.sup_mslience);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "offline billing", LCD_IconType, LCD_10sReflash, 0x475D, pu8_type, sizeof(LcdData.setData.Icon_SupOfflineBilling), (void *)&LcdData.setData.Icon_SupOfflineBilling);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "pw start", LCD_IconType, LCD_10sReflash, 0x6502, pu8_type, sizeof(LcdData.setData.Icon_SupPWStart), (void *)&LcdData.setData.Icon_SupPWStart);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_IsSupportSetFlash);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "password", LCD_InputType, 0, 0x3928, pstr_type, (sizeof(LcdData.setData.UserPasswdShow) + 1), (void *)(LcdData.setData.UserPasswdShow));
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_CONFIG, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 34.出厂调试-A枪监控信息 [page:45] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "b gun", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_MONITOR_B, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "in out", LCD_BtnType, 0x0023, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_GetIOStatusA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "modeule state", LCD_BtnType, 0x0024, 0x1000, page_type, LCD_PAGE_MENU_STATE_MODULE, (void *)SerialScreen_BtnModuleStateA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "charge vol", LCD_DataType, LCD_1sReflash, 0x4150, pu32_type, sizeof(LcdData.setData.g_chargeVol[LCD_GUN_1]), (void *)&LcdData.setData.g_chargeVol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "charge cur", LCD_DataType, LCD_1sReflash, 0x4152, pu32_type, sizeof(LcdData.setData.g_chargeCur[LCD_GUN_1]), (void *)&LcdData.setData.g_chargeCur[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "meter vol", LCD_DataType, LCD_1sReflash, 0x4158, pu32_type, sizeof(LcdData.setData.g_meterVol[LCD_GUN_1]), (void *)&LcdData.setData.g_meterVol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "meter cur", LCD_DataType, LCD_1sReflash, 0x415A, pu32_type, sizeof(LcdData.setData.g_meterCur[LCD_GUN_1]), (void *)&LcdData.setData.g_meterCur[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "cc1 vol", LCD_DataType, LCD_1sReflash, 0x41B0, pu32_type, sizeof(LcdData.setData.g_cc1Vol[LCD_GUN_1]), (void *)&LcdData.setData.g_cc1Vol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "input volu", LCD_DataType, LCD_1sReflash, 0x415C, pu32_type, sizeof(LcdData.setData.g_uiVol[LCD_GUN_1]), (void *)&LcdData.setData.g_uiVol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "input volv", LCD_DataType, LCD_1sReflash, 0x415E, pu32_type, sizeof(LcdData.setData.g_viVol[LCD_GUN_1]), (void *)&LcdData.setData.g_viVol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "input volw", LCD_DataType, LCD_1sReflash, 0x4160, pu32_type, sizeof(LcdData.setData.g_wiVol[LCD_GUN_1]), (void *)&LcdData.setData.g_wiVol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "port temp", LCD_DataType, LCD_1sReflash, 0x4162, pu32_type, sizeof(LcdData.setData.g_portTemp[LCD_GUN_1]), (void *)&LcdData.setData.g_portTemp[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "setup volt", LCD_InputType, 0, 0x5180, pu32_type, sizeof(LcdData.setData.s_moduleVol[LCD_GUN_1]), (void *)&LcdData.setData.s_moduleVol[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "setup curr", LCD_InputType, 0, 0x5182, pu32_type, sizeof(LcdData.setData.s_moduleCur[LCD_GUN_1]), (void *)&LcdData.setData.s_moduleCur[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "start", LCD_BtnType, 0x004E, 0x1000, page_type, LCD_PAGE_MENU_MONITOR, (void *)SerialScreen_BtnModuleStartA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "stop", LCD_BtnType, 0x004F, 0x1000, page_type, LCD_PAGE_MENU_MONITOR, (void *)SerialScreen_BtnModuleStopA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 35.出厂调试-B枪监控信息 [page:46] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "a gun", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_MONITOR, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "in out", LCD_BtnType, 0x0023, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_GetIOStatusB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "modeule state", LCD_BtnType, 0x0024, 0x1000, page_type, LCD_PAGE_MENU_STATE_MODULE_B, (void *)SerialScreen_BtnModuleStateB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "charge vol", LCD_DataType, LCD_1sReflash, 0x5150, pu32_type, sizeof(LcdData.setData.g_chargeVol[LCD_GUN_2]), (void *)&LcdData.setData.g_chargeVol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "charge cur", LCD_DataType, LCD_1sReflash, 0x5152, pu32_type, sizeof(LcdData.setData.g_chargeCur[LCD_GUN_2]), (void *)&LcdData.setData.g_chargeCur[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "meter vol", LCD_DataType, LCD_1sReflash, 0x5158, pu32_type, sizeof(LcdData.setData.g_meterVol[LCD_GUN_2]), (void *)&LcdData.setData.g_meterVol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "meter cur", LCD_DataType, LCD_1sReflash, 0x515A, pu32_type, sizeof(LcdData.setData.g_meterCur[LCD_GUN_2]), (void *)&LcdData.setData.g_meterCur[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "cc1 vol", LCD_DataType, LCD_1sReflash, 0x51B0, pu32_type, sizeof(LcdData.setData.g_cc1Vol[LCD_GUN_2]), (void *)&LcdData.setData.g_cc1Vol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "input volu", LCD_DataType, LCD_1sReflash, 0x515C, pu32_type, sizeof(LcdData.setData.g_uiVol[LCD_GUN_2]), (void *)&LcdData.setData.g_uiVol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "input volv", LCD_DataType, LCD_1sReflash, 0x515E, pu32_type, sizeof(LcdData.setData.g_viVol[LCD_GUN_2]), (void *)&LcdData.setData.g_viVol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "input volw", LCD_DataType, LCD_1sReflash, 0x5160, pu32_type, sizeof(LcdData.setData.g_wiVol[LCD_GUN_2]), (void *)&LcdData.setData.g_wiVol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "port temp", LCD_DataType, LCD_1sReflash, 0x5162, pu32_type, sizeof(LcdData.setData.g_portTemp[LCD_GUN_2]), (void *)&LcdData.setData.g_portTemp[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "setup volt", LCD_InputType, 0, 0x5180, pu32_type, sizeof(LcdData.setData.s_moduleVol[LCD_GUN_2]), (void *)&LcdData.setData.s_moduleVol[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "setup curr", LCD_InputType, 0, 0x5182, pu32_type, sizeof(LcdData.setData.s_moduleCur[LCD_GUN_2]), (void *)&LcdData.setData.s_moduleCur[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "start", LCD_BtnType, 0x004E, 0x1000, page_type, LCD_PAGE_MENU_MONITOR_B, (void *)SerialScreen_BtnModuleStartB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "stop", LCD_BtnType, 0x004F, 0x1000, page_type, LCD_PAGE_MENU_MONITOR_B, (void *)SerialScreen_BtnModuleStopB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_MONITOR_B, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 36.出厂调试-A枪输入输出 [page:47] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "AC set", LCD_BtnType, 0x003E, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnAcSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "DCA set", LCD_BtnType, 0x003F, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnDcSetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Parallel1 set", LCD_BtnType, 0x0040, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnParaSet1);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Parallel2 set", LCD_BtnType, 0x0055, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnParaSet2);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Parallel3 set", LCD_BtnType, 0x0056, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnParaSet3);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "ElockA set", LCD_BtnType, 0x0041, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnElockSetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "FanA set", LCD_BtnType, 0x0042, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnFanSetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "aux set", LCD_BtnType, 0x0053, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnAuxSetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "aux24v set", LCD_BtnType, 0x0060, 0x1003, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_BtnAux24VSetA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "b gun", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_GetIOStatusB);

    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "monitor info", LCD_BtnType, 0x0022, 0x1000, page_type, LCD_PAGE_MENU_MONITOR, (void *)NULL);

    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Relay AC", LCD_TextType, LCD_1sReflash, 0x4164, pu32_type, sizeof(LcdData.setData.g_acRely), (void *)&LcdData.setData.g_acRely);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Relay A", LCD_TextType, LCD_1sReflash, 0x4166, pu8_nH_type, 1, (void *)&LcdData.setData.g_dcRelay[LCD_GUN_1]);

    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Relay Parallel1", LCD_TextType, LCD_1sReflash, 0x4168, pu8_nH_type, 1, (void *)&LcdData.setData.g_paraRely0);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Electlock A", LCD_TextType, LCD_1sReflash, 0x416A, pu32_type, sizeof(LcdData.setData.g_elElock[LCD_GUN_1]), (void *)&LcdData.setData.g_elElock[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Fan A", LCD_TextType, LCD_1sReflash, 0x416C, pu8_type, sizeof(LcdData.setData.s_fan[LCD_GUN_1]), (void *)&LcdData.setData.s_fan[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Scram Status", LCD_TextType, LCD_1sReflash, 0x416E, pu32_type, sizeof(LcdData.setData.g_emergency), (void *)&LcdData.setData.g_emergency);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Door Status", LCD_TextType, LCD_1sReflash, 0x4170, pu32_type, sizeof(LcdData.setData.g_door), (void *)&LcdData.setData.g_door);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "aux Status", LCD_TextType, LCD_1sReflash, 0x41B2, pu32_type, sizeof(LcdData.setData.g_auxRelay[LCD_GUN_1]), (void *)&LcdData.setData.g_auxRelay[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Relay Parallel2", LCD_TextType, LCD_1sReflash, 0x5184, pu8_nH_type, 1, (void *)&LcdData.setData.g_paraRely1);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Relay Parallel3", LCD_TextType, LCD_1sReflash, 0x5188, pu8_nH_type, 1, (void *)&LcdData.setData.g_paraRely2);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "aux24v", LCD_TextType, LCD_1sReflash, 0x6700, pu32_type, 1, (void *)&LcdData.setData.g_aux24v[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "pour", LCD_TextType, LCD_1sReflash, 0x6704, pu32_type, 1, (void *)&LcdData.setData.g_pour);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "protect light", LCD_TextType, LCD_1sReflash, 0x6706, pu32_type, 1, (void *)&LcdData.setData.g_protect_light);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "flood", LCD_TextType, LCD_1sReflash, 0x6708, pu32_type, 1, (void *)&LcdData.setData.g_flood);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "smoke", LCD_TextType, LCD_1sReflash, 0x670A, pu32_type, 1, (void *)&LcdData.setData.g_smoke);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "gunsite", LCD_TextType, LCD_1sReflash, 0x670C, pu32_type, 1, (void *)&LcdData.setData.g_gunsite[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "fuse", LCD_TextType, LCD_1sReflash, 0x670E, pu32_type, 1, (void *)&LcdData.setData.g_fuse[LCD_GUN_1]);

    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon AC", LCD_IconType, LCD_10sReflash, 0x4172, pu8_type, sizeof(LcdData.setData.s_acRely), (void *)&LcdData.setData.s_acRely);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon DC", LCD_IconType, LCD_10sReflash, 0x4174, pu8_type, sizeof(LcdData.setData.s_dcRelay[LCD_GUN_1]), (void *)&LcdData.setData.s_dcRelay[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon Parallel1", LCD_IconType, LCD_10sReflash, 0x4176, pu8_type, sizeof(LcdData.setData.s_paraRely0), (void *)&LcdData.setData.s_paraRely0);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon Parallel2", LCD_IconType, LCD_10sReflash, 0x5186, pu8_type, sizeof(LcdData.setData.s_paraRely1), (void *)&LcdData.setData.s_paraRely1);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon Parallel3", LCD_IconType, LCD_10sReflash, 0x518A, pu8_type, sizeof(LcdData.setData.s_paraRely2), (void *)&LcdData.setData.s_paraRely2);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon ElectlockA", LCD_IconType, LCD_10sReflash, 0x4178, pu8_type, sizeof(LcdData.setData.s_elElock[LCD_GUN_1]), (void *)&LcdData.setData.s_elElock[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon FanA", LCD_IconType, LCD_10sReflash, 0x417A, pu8_type, sizeof(LcdData.setData.s_fan[LCD_GUN_1]), (void *)&LcdData.setData.s_fan[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon aux", LCD_IconType, LCD_10sReflash, 0x41B4, pu8_type, sizeof(LcdData.setData.s_auxRelay[LCD_GUN_1]), (void *)&LcdData.setData.s_auxRelay[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Icon aux24v", LCD_IconType, LCD_10sReflash, 0x6702, pu8_type, sizeof(LcdData.setData.aux24v_set[LCD_GUN_1]), (void *)&LcdData.setData.aux24v_set[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "modeule state", LCD_BtnType, 0x0024, 0x1000, page_type, LCD_PAGE_MENU_STATE_MODULE, (void *)SerialScreen_BtnModuleStateA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 37.出厂调试-B枪输入输出 [page:48] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "AC set", LCD_BtnType, 0x0045, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnAcSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "DCB set", LCD_BtnType, 0x0046, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnDcSetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Parallel1 set", LCD_BtnType, 0x0047, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnParaSet1);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Parallel2 set", LCD_BtnType, 0x0057, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnParaSet2);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Parallel3 set", LCD_BtnType, 0x0058, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnParaSet3);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "ElockB set", LCD_BtnType, 0x0048, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnElockSetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "FanB set", LCD_BtnType, 0x0049, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnFanSetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "aux set", LCD_BtnType, 0x0054, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnAuxSetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "aux24v set", LCD_BtnType, 0x0061, 0x1003, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_BtnAux24VSetB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "a gun", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_GetIOStatusA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "monitor info", LCD_BtnType, 0x0022, 0x1000, page_type, LCD_PAGE_MENU_MONITOR_B, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Relay AC", LCD_TextType, LCD_1sReflash, 0x5164, pu32_type, sizeof(LcdData.setData.g_acRely), (void *)&LcdData.setData.g_acRely);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Relay B", LCD_TextType, LCD_1sReflash, 0x5166, pu8_nH_type, 1, (void *)&LcdData.setData.g_dcRelay[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Relay Parallel1", LCD_TextType, LCD_1sReflash, 0x5168, pu8_nH_type, 1, (void *)&LcdData.setData.g_paraRely0);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Relay Parallel2", LCD_TextType, LCD_1sReflash, 0x518C, pu8_nH_type, 1, (void *)&LcdData.setData.g_paraRely1);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Relay Parallel3", LCD_TextType, LCD_1sReflash, 0x5190, pu8_nH_type, 1, (void *)&LcdData.setData.g_paraRely2);

    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Electlock B", LCD_TextType, LCD_1sReflash, 0x516A, pu32_type, sizeof(LcdData.setData.g_elElock[LCD_GUN_2]), (void *)&LcdData.setData.g_elElock[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Fan B", LCD_TextType, LCD_1sReflash, 0x516C, pu8_type, sizeof(LcdData.setData.s_fan[LCD_GUN_1]), (void *)&LcdData.setData.s_fan[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Scram Status", LCD_TextType, LCD_1sReflash, 0x516E, pu32_type, sizeof(LcdData.setData.g_emergency), (void *)&LcdData.setData.g_emergency);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Door Status", LCD_TextType, LCD_1sReflash, 0x5170, pu32_type, sizeof(LcdData.setData.g_door), (void *)&LcdData.setData.g_door);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "aux Status", LCD_TextType, LCD_1sReflash, 0x51B2, pu32_type, sizeof(LcdData.setData.g_auxRelay[LCD_GUN_2]), (void *)&LcdData.setData.g_auxRelay[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "aux24v", LCD_TextType, LCD_1sReflash, 0x6780, pu32_type, 1, (void *)&LcdData.setData.g_aux24v[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "pour", LCD_TextType, LCD_1sReflash, 0x6784, pu32_type, 1, (void *)&LcdData.setData.g_pour);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "protect light", LCD_TextType, LCD_1sReflash, 0x6786, pu32_type, 1, (void *)&LcdData.setData.g_protect_light);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "flood", LCD_TextType, LCD_1sReflash, 0x6788, pu32_type, 1, (void *)&LcdData.setData.g_flood);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "smoke", LCD_TextType, LCD_1sReflash, 0x678A, pu32_type, 1, (void *)&LcdData.setData.g_smoke);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "gunsite", LCD_TextType, LCD_1sReflash, 0x678C, pu32_type, 1, (void *)&LcdData.setData.g_gunsite[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "fuse", LCD_TextType, LCD_1sReflash, 0x678E, pu32_type, 1, (void *)&LcdData.setData.g_fuse[LCD_GUN_2]);

    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon AC", LCD_IconType, LCD_10sReflash, 0x5172, pu8_type, sizeof(LcdData.setData.s_acRely), (void *)&LcdData.setData.s_acRely);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon DC", LCD_IconType, LCD_10sReflash, 0x5174, pu8_type, sizeof(LcdData.setData.s_dcRelay[LCD_GUN_2]), (void *)&LcdData.setData.s_dcRelay[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon Parallel1", LCD_IconType, LCD_10sReflash, 0x5176, pu8_type, sizeof(LcdData.setData.s_paraRely0), (void *)&LcdData.setData.s_paraRely0);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon Parallel2", LCD_IconType, LCD_10sReflash, 0x518E, pu8_type, sizeof(LcdData.setData.s_paraRely1), (void *)&LcdData.setData.s_paraRely1);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon Parallel3", LCD_IconType, LCD_10sReflash, 0x5192, pu8_type, sizeof(LcdData.setData.s_paraRely2), (void *)&LcdData.setData.s_paraRely2);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon ElectlockB", LCD_IconType, LCD_10sReflash, 0x5178, pu8_type, sizeof(LcdData.setData.s_elElock[LCD_GUN_2]), (void *)&LcdData.setData.s_elElock[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon FanB", LCD_IconType, LCD_10sReflash, 0x517A, pu8_type, sizeof(LcdData.setData.s_fan[LCD_GUN_1]), (void *)&LcdData.setData.s_fan[LCD_GUN_1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon aux", LCD_IconType, LCD_10sReflash, 0x51B4, pu8_type, sizeof(LcdData.setData.s_auxRelay[LCD_GUN_2]), (void *)&LcdData.setData.s_auxRelay[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Icon aux24v", LCD_IconType, LCD_10sReflash, 0x6782, pu8_type, sizeof(LcdData.setData.aux24v_set[LCD_GUN_2]), (void *)&LcdData.setData.aux24v_set[LCD_GUN_2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "modeule state", LCD_BtnType, 0x0024, 0x1000, page_type, LCD_PAGE_MENU_STATE_MODULE_B, (void *)SerialScreen_BtnModuleStateB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_INOUT_B, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 38.出厂调试-A枪模块信息 [page:49] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "b gun", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_STATE_MODULE_B, (void *)SerialScreen_BtnModuleStateB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "monitor info", LCD_BtnType, 0x0022, 0x1000, page_type, LCD_PAGE_MENU_MONITOR, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "in out", LCD_BtnType, 0x0023, 0x1000, page_type, LCD_PAGE_MENU_INOUT, (void *)SerialScreen_GetIOStatusA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5200, pstr_type, sizeof(LcdData.ModuleStateString[0]), (void *)LcdData.ModuleStateString[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5210, pstr_type, sizeof(LcdData.ModuleStateString[1]), (void *)LcdData.ModuleStateString[1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5220, pstr_type, sizeof(LcdData.ModuleStateString[2]), (void *)LcdData.ModuleStateString[2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5230, pstr_type, sizeof(LcdData.ModuleStateString[3]), (void *)LcdData.ModuleStateString[3]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5240, pstr_type, sizeof(LcdData.ModuleStateString[4]), (void *)LcdData.ModuleStateString[4]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5250, pstr_type, sizeof(LcdData.ModuleStateString[5]), (void *)LcdData.ModuleStateString[5]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5260, pstr_type, sizeof(LcdData.ModuleStateString[6]), (void *)LcdData.ModuleStateString[6]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5270, pstr_type, sizeof(LcdData.ModuleStateString[7]), (void *)LcdData.ModuleStateString[7]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5280, pstr_type, sizeof(LcdData.ModuleStateString[8]), (void *)LcdData.ModuleStateString[8]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5290, pstr_type, sizeof(LcdData.ModuleStateString[9]), (void *)LcdData.ModuleStateString[9]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52A0, pstr_type, sizeof(LcdData.ModuleStateString[10]), (void *)LcdData.ModuleStateString[10]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52B0, pstr_type, sizeof(LcdData.ModuleStateString[11]), (void *)LcdData.ModuleStateString[11]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52C0, pstr_type, sizeof(LcdData.ModuleStateString[12]), (void *)LcdData.ModuleStateString[12]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52D0, pstr_type, sizeof(LcdData.ModuleStateString[13]), (void *)LcdData.ModuleStateString[13]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52E0, pstr_type, sizeof(LcdData.ModuleStateString[14]), (void *)LcdData.ModuleStateString[14]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52F0, pstr_type, sizeof(LcdData.ModuleStateString[15]), (void *)LcdData.ModuleStateString[15]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 39.出厂调试-B枪模块信息 [page:50] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "a gun", LCD_BtnType, 0x0026, 0x1000, page_type, LCD_PAGE_MENU_STATE_MODULE, (void *)SerialScreen_BtnModuleStateA);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "monitor info", LCD_BtnType, 0x0022, 0x1000, page_type, LCD_PAGE_MENU_MONITOR_B, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "in out", LCD_BtnType, 0x0023, 0x1000, page_type, LCD_PAGE_MENU_INOUT_B, (void *)SerialScreen_GetIOStatusB);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)SerialScreen_QuitDebugIO);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5200, pstr_type, sizeof(LcdData.ModuleStateString[0]), (void *)LcdData.ModuleStateString[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5210, pstr_type, sizeof(LcdData.ModuleStateString[1]), (void *)LcdData.ModuleStateString[1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5220, pstr_type, sizeof(LcdData.ModuleStateString[2]), (void *)LcdData.ModuleStateString[2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5230, pstr_type, sizeof(LcdData.ModuleStateString[3]), (void *)LcdData.ModuleStateString[3]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5240, pstr_type, sizeof(LcdData.ModuleStateString[4]), (void *)LcdData.ModuleStateString[4]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5250, pstr_type, sizeof(LcdData.ModuleStateString[5]), (void *)LcdData.ModuleStateString[5]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5260, pstr_type, sizeof(LcdData.ModuleStateString[6]), (void *)LcdData.ModuleStateString[6]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5270, pstr_type, sizeof(LcdData.ModuleStateString[7]), (void *)LcdData.ModuleStateString[7]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5280, pstr_type, sizeof(LcdData.ModuleStateString[8]), (void *)LcdData.ModuleStateString[8]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x5290, pstr_type, sizeof(LcdData.ModuleStateString[9]), (void *)LcdData.ModuleStateString[9]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52A0, pstr_type, sizeof(LcdData.ModuleStateString[10]), (void *)LcdData.ModuleStateString[10]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52B0, pstr_type, sizeof(LcdData.ModuleStateString[11]), (void *)LcdData.ModuleStateString[11]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52C0, pstr_type, sizeof(LcdData.ModuleStateString[12]), (void *)LcdData.ModuleStateString[12]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52D0, pstr_type, sizeof(LcdData.ModuleStateString[13]), (void *)LcdData.ModuleStateString[13]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52E0, pstr_type, sizeof(LcdData.ModuleStateString[14]), (void *)LcdData.ModuleStateString[14]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "modulestate", LCD_TextType, LCD_NoReflash, 0x52F0, pstr_type, sizeof(LcdData.ModuleStateString[15]), (void *)LcdData.ModuleStateString[15]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_STATE_MODULE_B, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 40.系统信息-系统 [page:54] */
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "clear info", LCD_BtnType, 0x001d, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnClearAll);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "COM_1", LCD_BtnType, 0x000A, 0x1000, page_type, LCD_PAGE_MENU_COM_1, (void *)SerialScreen_BtnChgInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "COM_2", LCD_BtnType, 0x000B, 0x1000, page_type, LCD_PAGE_MENU_COM_2, (void *)SerialScreen_BtnServerGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "COM_3", LCD_BtnType, 0x000C, 0x1000, page_type, LCD_PAGE_MENU_COM_3, (void *)SerialScreen_BtnMeterNoInfoGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "COM_4", LCD_BtnType, 0x000D, 0x1000, page_type, LCD_PAGE_MENU_COM_4, (void *)SerialScreen_BtnModuleGet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "COM_7", LCD_BtnType, 0x001C, 0x1000, page_type, LCD_PAGE_MENU_COM_7, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "cd up", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "Home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "Alloc_way", LCD_InputType, 0, 0x2100, menu_type, sizeof(LcdData.setData.AllocWay), (void *)&LcdData.setData.AllocWay);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "DevType", LCD_InputType, 0, 0x2102, menu_type, sizeof(LcdData.setData.DevType), (void *)&LcdData.setData.DevType);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "subok", LCD_BtnType, 0x0059, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_BtnSystemFuncSet);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeYear", LCD_InputType, 0, 0x4194, pu32_type, sizeof(LcdData.setData.s_TimeSync[0]), (void *)&LcdData.setData.s_TimeSync[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeMonth", LCD_InputType, 0, 0x4196, pu32_type, sizeof(LcdData.setData.s_TimeSync[1]), (void *)&LcdData.setData.s_TimeSync[1]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeDay", LCD_InputType, 0, 0x4198, pu32_type, sizeof(LcdData.setData.s_TimeSync[2]), (void *)&LcdData.setData.s_TimeSync[2]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeHour", LCD_InputType, 0, 0x419A, pu32_type, sizeof(LcdData.setData.s_TimeSync[3]), (void *)&LcdData.setData.s_TimeSync[3]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeMIN", LCD_InputType, 0, 0x419C, pu32_type, sizeof(LcdData.setData.s_TimeSync[4]), (void *)&LcdData.setData.s_TimeSync[4]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeSecond", LCD_InputType, 0, 0x419E, pu32_type, sizeof(LcdData.setData.s_TimeSync[5]), (void *)&LcdData.setData.s_TimeSync[5]);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "TimeSyncSet", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_MENU_SYS, (void *)SerialScreen_ScreenSet_TimeSync_Flag);

#ifdef SCREEN_USING_DUPU
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "ems soc", LCD_DataType, LCD_1sReflash, 0x4764, pu16_type, sizeof(LcdData.setData.StoredEnergy_Soc), (void *)&LcdData.setData.StoredEnergy_Soc);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "ems power", LCD_DataType, LCD_1sReflash, 0x4760, pu32_type, sizeof(LcdData.setData.StoredEnergy_Power), (void *)&LcdData.setData.StoredEnergy_Power);
#endif /* SCREEN_USING_DUPU */
	SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "Reboot", LCD_BtnType, 0x0002, 0x1005, page_type, LCD_PAGE_MENU_SYS, (void *)SerialScreen_ScreenSet_Reboot_Flag);
    SerialScreen_ItemSetUp(LCD_PAGE_MENU_SYS, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

#ifdef SCREEN_USING_OFFLINE_BILLING
    /** 41.出厂设置-离线费率 [page:62] */
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "input info", LCD_BtnType, 0x001E, 0x1000, page_type, LCD_PAGE_MENU_INPUT, (void *)SerialScreen_InputInfoGet);   //OK
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "protect info", LCD_BtnType, 0x0020, 0x1000, page_type, LCD_PAGE_MENU_PROTECT, (void *)SerialScreen_BtnProtectInfoGet);  //OK
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "func config", LCD_BtnType, 0x0021, 0x1000, page_type, LCD_PAGE_MENU_CONFIG, (void *)SerialScreen_IsSupportGet);  //OK
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "subok", LCD_BtnType, 0x0014, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)SerialScreen_OfflineBillingSet);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "Icon warn", LCD_IconType, LCD_10sReflash, 0x1740, pu8_type, sizeof(LcdData.setData.OBwarning), (void *)&LcdData.setData.OBwarning);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "service fees", LCD_InputType, 0, 0x474D, pu32_type, sizeof(LcdData.setData.ServicePrice), (void *)&LcdData.setData.ServicePrice);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SsElect fees", LCD_InputType, 0, 0x4800, pu32_type, sizeof(LcdData.setData.SsElectPrice), (void *)&LcdData.setData.SsElectPrice);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SElect fees", LCD_InputType, 0, 0x4813, pu32_type, sizeof(LcdData.setData.SElectPrice), (void *)&LcdData.setData.SElectPrice);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PElect fees", LCD_InputType, 0, 0x4826, pu32_type, sizeof(LcdData.setData.PElectPrice), (void *)&LcdData.setData.PElectPrice);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FElect fees", LCD_InputType, 0, 0x4839, pu32_type, sizeof(LcdData.setData.FElectPrice), (void *)&LcdData.setData.FElectPrice);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VElect fees", LCD_InputType, 0, 0x484C, pu32_type, sizeof(LcdData.setData.VElectPrice), (void *)&LcdData.setData.VElectPrice);

    /** SstSh0: sharp sharp time start hour 0,  SstEh0: sharp sharp time end hour 0 */
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstSh0", LCD_InputType, 0, 0x4803, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstSm0", LCD_InputType, 0, 0x4805, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstEh0", LCD_InputType, 0, 0x4807, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstEm0", LCD_InputType, 0, 0x4809, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][0].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstSh1", LCD_InputType, 0, 0x480B, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstSm1", LCD_InputType, 0, 0x480D, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstEh1", LCD_InputType, 0, 0x480F, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "SstEm1", LCD_InputType, 0, 0x4811, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP_SHARP][1].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StSh0", LCD_InputType, 0, 0x4816, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StSm0", LCD_InputType, 0, 0x4818, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StEh0", LCD_InputType, 0, 0x481A, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StEm0", LCD_InputType, 0, 0x481C, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][0].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StSh1", LCD_InputType, 0, 0x481E, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StSm1", LCD_InputType, 0, 0x4820, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StEh1", LCD_InputType, 0, 0x4822, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "StEm1", LCD_InputType, 0, 0x4824, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_SHARP][1].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtSh0", LCD_InputType, 0, 0x4829, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtSm0", LCD_InputType, 0, 0x482B, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtEh0", LCD_InputType, 0, 0x482D, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtEm0", LCD_InputType, 0, 0x482F, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][0].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtSh1", LCD_InputType, 0, 0x4831, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtSm1", LCD_InputType, 0, 0x4833, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtEh1", LCD_InputType, 0, 0x4835, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "PtEm1", LCD_InputType, 0, 0x4837, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_PEAK][1].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtSh0", LCD_InputType, 0, 0x483C, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtSm0", LCD_InputType, 0, 0x483E, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtEh0", LCD_InputType, 0, 0x4840, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtEm0", LCD_InputType, 0, 0x4842, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][0].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtSh1", LCD_InputType, 0, 0x4844, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtSm1", LCD_InputType, 0, 0x4846, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtEh1", LCD_InputType, 0, 0x4848, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "FtEm1", LCD_InputType, 0, 0x484A, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_FLAT][1].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtSh0", LCD_InputType, 0, 0x484F, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtSm0", LCD_InputType, 0, 0x4851, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtEh0", LCD_InputType, 0, 0x4853, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtEm0", LCD_InputType, 0, 0x4855, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][0].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtSh1", LCD_InputType, 0, 0x4857, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].shour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].shour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtSm1", LCD_InputType, 0, 0x4859, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].smin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].smin);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtEh1", LCD_InputType, 0, 0x485B, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].ehour), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].ehour);
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "VtEm1", LCD_InputType, 0, 0x485D, pu8_type, sizeof(LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].emin), (void *)&LcdData.setData.PeriodTime[CP_RATED_TYPE_VALLEY][1].emin);

    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "back", LCD_BtnType, 0x0050, 0x1000, page_type, LCD_PAGE_ROOT_MAIN, (void *)NULL);  //OK
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);    //0K
    SerialScreen_ItemSetUp(LCD_PAGE_OFFLINE_BILLING, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 42.告警信息 [page:78] */
    SerialScreen_ItemSetUp(LCD_PAGE_WARNNING_INFO, NULL, "home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);    //0K
    SerialScreen_ItemSetUp(LCD_PAGE_WARNNING_INFO, NULL, "count down", LCD_TextType, LCD_1sReflash, 0x2040, pstr_type, sizeof(LcdData.setData.OB_CountDownString), (void *)LcdData.setData.OB_CountDownString);
    SerialScreen_ItemSetUp(LCD_PAGE_WARNNING_INFO, NULL, "Icon warn", LCD_IconType, LCD_10sReflash, 0x173F, pu8_type, sizeof(LcdData.setData.OBEventwarning), (void *)&LcdData.setData.OBEventwarning);
    SerialScreen_ItemSetUp(LCD_PAGE_WARNNING_INFO, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 43.A枪离线计费结算 [page:79] */
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);    //0K
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "count down", LCD_TextType, LCD_1sReflash, 0x1734, pstr_type, sizeof(LcdData.setData.OB_CountDownString), (void *)LcdData.setData.OB_CountDownString);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "Energy", LCD_DataType, LCD_1sReflash, 0x1610, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].engery), (void *)&LcdData.gun[LCD_GUN_1].engery);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "Hour Used", LCD_DataType, LCD_1sReflash, 0x1612, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].ChrgeTimeHour), (void *)&LcdData.gun[LCD_GUN_1].ChrgeTimeHour);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "Min Used", LCD_DataType, LCD_1sReflash, 0x1614, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].ChrgeTImeMin), (void *)&LcdData.gun[LCD_GUN_1].ChrgeTImeMin);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x1616, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].curSoc), (void *)&LcdData.gun[LCD_GUN_1].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "Chg Money", LCD_DataType, LCD_NoReflash, 0x1618, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].totalFee), (void *)&LcdData.gun[LCD_GUN_1].totalFee);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "Acc Ballance", LCD_DataType, LCD_NoReflash, 0x172C, pu32_type, sizeof(LcdData.gun[LCD_GUN_1].AccountBallance), (void *)&LcdData.gun[LCD_GUN_1].AccountBallance);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "ChgStopReason", LCD_TextType, LCD_NoReflash, 0x161A, pstr_type, sizeof(LcdData.gun[LCD_GUN_1].code_stopResaon), (void *)&LcdData.gun[LCD_GUN_1].code_stopResaon[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "help number", LCD_TextType, LCD_NoReflash, 0x11A0, pstr_type, sizeof(LcdData.setData.Help_Number), (void *)LcdData.setData.Help_Number);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_A, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);

    /** 44.B枪离线计费结算 [page:80] */
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "home", LCD_BtnHomeType, 0x0002, 0x1000, page_type, LCD_PAGE_NONE, (void *)NULL);    //0K
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "count down", LCD_TextType, LCD_1sReflash, 0x1736, pstr_type, sizeof(LcdData.setData.OB_CountDownString), (void *)LcdData.setData.OB_CountDownString);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "Energy", LCD_DataType, LCD_1sReflash, 0x2610, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].engery), (void *)&LcdData.gun[LCD_GUN_2].engery);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "Hour Used", LCD_DataType, LCD_1sReflash, 0x2612, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].ChrgeTimeHour), (void *)&LcdData.gun[LCD_GUN_2].ChrgeTimeHour);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "Min Used", LCD_DataType, LCD_1sReflash, 0x2614, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].ChrgeTImeMin), (void *)&LcdData.gun[LCD_GUN_2].ChrgeTImeMin);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "Soc", LCD_DataType, LCD_1sReflash, 0x2616, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].curSoc), (void *)&LcdData.gun[LCD_GUN_2].curSoc);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "Chg Money", LCD_DataType, LCD_NoReflash, 0x2618, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].totalFee), (void *)&LcdData.gun[LCD_GUN_2].totalFee);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "Acc Ballance", LCD_DataType, LCD_NoReflash, 0x174A, pu32_type, sizeof(LcdData.gun[LCD_GUN_2].AccountBallance), (void *)&LcdData.gun[LCD_GUN_2].AccountBallance);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "ChgStopReason", LCD_TextType, LCD_NoReflash, 0x261A, pstr_type, sizeof(LcdData.gun[LCD_GUN_2].code_stopResaon), (void *)&LcdData.gun[LCD_GUN_2].code_stopResaon[0]);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "help number", LCD_TextType, LCD_NoReflash, 0x11A0, pstr_type, sizeof(LcdData.setData.Help_Number), (void *)LcdData.setData.Help_Number);
    SerialScreen_ItemSetUp(LCD_PAGE_OB_PYA_B, NULL, "", 0, 0, 0, 0, 0, (void *)NULL);
#endif /* SCREEN_USING_OFFLINE_BILLING */

#ifdef SERIALSCREEN_DESIGNATE_REGION
    SerialScreenAddr = 1;
    s_ota_info = NULL;
#ifdef SCREEN_USING_OFFLINE_BILLING
    /** 锁卡 */
    Trigger_Page[0x00].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x00].ShowPara = SCREEN_TRIGGER_WARN_ICON_CARD_LOCKED;
    Trigger_Page[0x00].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 无效卡 */
    Trigger_Page[0x01].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x01].ShowPara = SCREEN_TRIGGER_WARN_ICON_INVALID_CARD;
    Trigger_Page[0x01].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 余额不足 */
    Trigger_Page[0x02].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x02].ShowPara = SCREEN_TRIGGER_WARN_ICON_NO_BALLANCE;
    Trigger_Page[0x02].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 非法卡 */
    Trigger_Page[0x03].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x03].ShowPara = SCREEN_TRIGGER_WARN_ICON_ILLEGAL_CARD;
    Trigger_Page[0x03].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 先插枪后刷卡 */
    Trigger_Page[0x04].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x04].ShowPara = SCREEN_TRIGGER_WARN_ICON_GUN_FIRST;
    Trigger_Page[0x04].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 计费信息设置错误 */
    Trigger_Page[0x05].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x05].ShowPara = SCREEN_TRIGGER_WARN_ICON_FEES_ERROR;
    Trigger_Page[0x05].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 充电结束, 刷卡结算 */
    Trigger_Page[0x06].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x06].ShowPara = SCREEN_TRIGGER_WARN_ICON_PAY;
    Trigger_Page[0x06].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 结算中(用于查询历史订单时) */
    Trigger_Page[0x07].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x07].ShowPara = SCREEN_TRIGGER_WARN_ICON_PAYING;
    Trigger_Page[0x07].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 不是启动卡，请切换枪号 */
    Trigger_Page[0x08].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x08].ShowPara = SCREEN_TRIGGER_WARN_ICON_SWITCH_GUN;
    Trigger_Page[0x08].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 结算成功 */
    Trigger_Page[0x09].page = LCD_PAGE_OB_PYA_A;
    Trigger_Page[0x09].ShowPara = 0x00;
    Trigger_Page[0x09].ShieldPara = 0x00;
    /** 结算成功 */
    Trigger_Page[0x0A].page = LCD_PAGE_OB_PYA_B;
    Trigger_Page[0x0A].ShowPara = 0x00;
    Trigger_Page[0x0A].ShieldPara = 0x00;
    /** 启动中 */
    Trigger_Page[0x0B].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x0B].ShowPara = SCREEN_TRIGGER_WARN_IS_STARTING;
    Trigger_Page[0x0B].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 正在充电中 */
    Trigger_Page[0x0C].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x0C].ShowPara = SCREEN_TRIGGER_WARN_IS_CHARGING;
    Trigger_Page[0x0C].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;
    /** 故障停、请重新拔、插枪 */
    Trigger_Page[0x0D].page = LCD_PAGE_WARNNING_INFO;
    Trigger_Page[0x0D].ShowPara = SCREEN_TRIGGER_WARN_FAULT_STOP;
    Trigger_Page[0x0D].ShieldPara = SCREEN_TRIGGER_WARN_ICON_SIZE;

    memset(LcdTriggerEvent, 0x00, sizeof(LcdTriggerEvent));
#endif /* SCREEN_USING_OFFLINE_BILLING */

    memset(&LcdData, 0x00, sizeof(LcdData));
    memset(&LcdRxData, 0x00, sizeof(LcdRxData));
    memset(&LcdTxData, 0x00, sizeof(LcdTxData));
    memset(&LcdAssistantData, 0x00, sizeof(LcdAssistantData));

    memset(SerialScreenRxbuf, 0x00, sizeof(SerialScreenRxbuf));
#endif /* SERIALSCREEN_DESIGNATE_REGION */

    for(u8 i = 0; i < LCD_GUN_NUM; i++)
        LcdAssistantData.SeveralGunFlag[i].IsPowerOn = 0;

    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_SYSTEM_INFO] = SerialScreen_ConfigExecute_System;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_PILE_INFO] = SerialScreen_ConfigExecute_Pile;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_SERVER_INFO] = SerialScreen_ConfigExecute_Server;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_AMMETER_INFO] = SerialScreen_ConfigExecute_Ammeter;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_MODULE_INFO] = SerialScreen_ConfigExecute_Module;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_VIN_INFO] = SerialScreen_ConfigExecute_Vin;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_PROTECT_INFO] = SerialScreen_ConfigExecute_Protect;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_FUNCTION_INFO] = SerialScreen_ConfigExecute_Function;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_OFFLINE_BILLING_INFO] = SerialScreen_ConfigExecute_OfflineBilling;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_INPUT_7103_7101_INFO] = SerialScreen_ConfigExecute_Input_7103_7101;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_PUBLIC_INPUT_7104_INFO] = SerialScreen_ConfigExecute_PublicInput_7104;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_GUN_INPUT_7104_INFO] = SerialScreen_ConfigExecute_GunInput_7104;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_PUBLIC_OUTPUT_7104_INFO] = SerialScreen_ConfigExecute_PublicOutput_7104;
    LcdConfigExecutPool[THAISEN_CONFIG_PAGE_GUN_OUTPUT_7104_INFO] = SerialScreen_ConfigExecute_GunOutput_7104;

    return SerialScreen.Initialize(&SerialScreen);
}

u16 serialScreen_ObjectAi_main(void)
{
    s_ota_info = thaisen_app_get_ota_info();
	SerialScreen.Process(&SerialScreen);
    return 0;
}
