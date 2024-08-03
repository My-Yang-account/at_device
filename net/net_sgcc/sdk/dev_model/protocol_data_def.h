#if !defined(_PROTOCOL_DATA_DEF_H_)
#define _PROTOCOL_DATA_DEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "infra_types.h"
#include "infra_defs.h"

/*******************************用户可配置项***************************************/

#define EVS_MAX_PORT_NUM (2)		// 充电接口数量
#define EVS_MAX_INPUT_METER_NUM (1) // 直流充电桩输入电表数量
#define EVS_MAX_ALARM_LEN (255)		// 最大故障告警数
#define EVS_MAX_DATA_LEN (128)		// 响应数据区域长度

/*********************************************************************************/

#define EVS_MAX_SEG_LEN (96)			  // 最大功率调节时段数
#define EVS_MAX_TRADE_LEN (40 + 1)		  // 最大订单号长度：设备唯一标识（充电桩资产码：24位）+充电接口标识（2 位）+ 序列号(12 位)
#define EVS_MAX_CAR_VIN_LEN (17 + 1)	  // 最大车辆唯一识别码长度
#define EVS_MAX_MAC_ADDR_LEN (32 + 1)	  // 设备mac地址长度
#define EVS_MAX_MODEL_ID_LEN (16 + 1)	  // 最大计费模型id长度
#define EVS_MAX_QRCODE_LEN (256)		  // 二维码长度
#define EVS_MAX_ICCID_LEN (24)			  // sim卡iccid长度
#define EVS_MAX_PILE_TYPE_LEN (20)		  // 桩类型长度
#define EVS_MAX_DEV_SN_LEN (16 + 1)		  // 厂商出厂编码
#define EVS_MAX_MODEL_DEVSEG (96)		  // 最大计费模型时段数
#define EVS_MAX_METER_ADDR_LEN (6)		  // 电表地址长度
#define EVS_MAX_METER_ASSET_LEN (32 + 1)  // 电表资产码长度
#define EVS_MAX_TIMESTAMP_LEN (15 + 1)	  // 时间戳长度
#define EVS_MAX_SOFTWAREVER_LEN (255 + 1) // 充电桩软件版本号长度
#define EVS_MAX_HARDWAREVER_LEN (255 + 1) // 充电桩硬件版本号长度
#define EVS_MAX_SDKVER_LEN (255 + 1)	  // SDK版本号长度
#define EVS_MAX_LOGQUERY_LEN (38 + 1)	  // 查询日志流水号长度
#define EVS_MAX_LOGAREA_LEN (255 + 1)	  // 日志响应数据最大长度
#define EVS_MAX_CARD_LEN (20 + 1)		  // 卡片序列号长度
#define EVS_MAX_PSYCARD_LEN (16 + 1)	  // 物理卡号长度
#define EVS_MAX_RANDDOM_LEN (8 + 1)		  // 随机数长度
#define EVS_MAX_TRADE_MAC_LEN (8 + 1)	  // 交易验证码长度
#define EVS_MAX_AUTHCODE_LEN (255 + 1)	  // 本地鉴权鉴权码长度
#define EVS_MAX_NETMDUINFO_LEN (64 + 1)	  // 网络模块型号长度
#define EVS_MAX_MDUINT_NUM (10)			  // 模块int型数组长度
#define EVS_MAX_MDUSTRING_NUM (10)		  // 模块string型数组长度
#define EVS_MAX_MDUSTRING_LEN (255 + 1)	  // 模块string长度
#define EVS_MAX_FUNCONF_LEN (255 + 1)	  // 功能配置参数最大长度
#define EVS_MAX_TERMINAL_SN_LEN (12 + 1)  // 终端机编号最大长度
#define EVS_MAX_TIME_STRING_LEN (14 + 1)  // 鉴权时间戳最大长度
#define EVS_MAX_TRADE_MAC2_LEN (8 + 1)	  // 加密机鉴权码最大长度
#define EVS_MAX_DEVINFO_LEN (64 + 1)	  // 设备信息长度

#define EVS_MAX_BLE_SECRETKEY_LEN (24 + 1) // 蓝牙密钥最大长度
#define EVS_MAX_BLE_MAC_NUM (10 + 1)	   // 蓝牙MAC最大数量
#define EVS_MAX_BLE_MACLIST_LEN (64 + 1)   // 蓝牙鉴权信息最大长度
#define EVS_MAX_BLE_MAC_LEN (12 + 1)	   // 蓝牙MAC最大长度
#define EVS_MAX_VIN_LIST_LEN (100)		   // 白名单最大VIN条数
#define EVS_MAX_VIN_SN_LEN (38 + 1)		   // 查询日志流水号长度
#define EVS_MAX_OPT_SN_LEN (39 + 1)		   // 功能配置更新操作流水号长度
#define EVS_MAX_PARTS_SWITCH_NUM 10		   // 部件开关最大数量
#define EVS_MAX_NET_SIG_QUA_LEN (255 + 1)  // 网络信号质量最大长度
#define EVS_MAX_SAMPLE_VAL_LEN (255 + 1)   // 设备运行采样数据最大长度

typedef struct
{
	char product_key[IOTX_PRODUCT_KEY_LEN + 1];			// 设备品类标识字符串
	char product_secret[IOTX_PRODUCT_SECRET_LEN + 1];	// 设备品类密钥
	char device_name[IOTX_DEVICE_NAME_LEN + 1];			// 某台设备的标识字符串:未注册前为设备出厂编号（16位长度），注册后为设备在物联管理平台的资产码（24位长度）
	char device_secret[IOTX_DEVICE_SECRET_LEN + 1];		// 某台设备的设备密钥
	char device_reg_code[IOTX_DEVICE_REG_CODE_LEN + 1]; // 某台设备的设备注册码
	char device_uid[IOTX_DEVICE_UID_LEN + 1];			// 某台设备的出厂编号
} evs_device_meta;

typedef enum
{
	EVS_CMD_READER_INIT = 0,	// 读卡器初始化
	EVS_CMD_READER_CHECK,		// 读卡器状态检查
	EVS_CMD_OPEN_RF,			// 打开射频
	EVS_CMD_FOUND_CARD,			// 卡片激活
	EVS_CMD_GET_USER_ID,		// 获取卡片序列号
	EVS_CMD_GET_PSY_ID,			// 获取卡片物理卡号
	EVS_CMD_CHECK_PIN,			// 卡密验证
	EVS_CMD_GET_KEY_VER,		// 获取卡片密钥版本
	EVS_CMD_GET_CARD_AUTH,		// 获取卡片鉴权信息
	EVS_CMD_CONFIRM_AUTHCODE_S, // 确认平台鉴权信息
	EVS_CMD_CLOSE_RF,			// 关闭射频
} evs_cmd_card_enum;

typedef enum
{
	EVS_CMD_EVENT_FIRMWARE_INFO = 0,
	EVS_CMD_EVENT_ASK_FEEMODEL,
	EVS_CMD_EVENT_STARTCHARGE,
	EVS_CMD_EVENT_STARTRESULT,
	EVS_CMD_EVENT_STOPCHARGE,
	EVS_CMD_EVENT_TRADEINFO,
	EVS_CMD_EVENT_ALARM,
	EVS_CMD_EVENT_ACPILE_CHANGE,
	EVS_CMD_EVENT_DCPILE_CHANGE,
	EVS_CMD_EVENT_GROUNDLOCK_CHANGE,
	EVS_CMD_EVENT_GATELOCK_CHANGE,
	EVS_CMD_EVENT_ASK_DEV_CONFIG,
	EVS_CMD_EVENT_CAR_INFO,
	EVS_CMD_EVENT_VER_INFO,
	EVS_CMD_EVENT_LOGQUERY_RESULT,
	EVS_CMD_EVENT_CARD_INFO,
	EVS_CMD_EVENT_CARD_AUTH,
	EVS_CMD_EVENT_CARD_AUTH_RESULT,
	EVS_CMD_EVENT_CARD_CHECK_ERROR,
	EVS_CMD_EVENT_DEVMDU_INFO,
	EVS_CMD_EVENT_CHARGE_CONNECT,
	EVS_CMD_EVENT_BLE_PLUG_CHARGE_INFO,
	EVS_CMD_EVENT_BLE_CONN_CHANGE,

	EVS_CMD_EVENT_VIN_CHECK_RESULT,
	EVS_CMD_EVENT_PILE_PARTS_CCU_INFO,
	EVS_CMD_EVENT_PILE_PARTS_PCU_INFO,
	EVS_CMD_EVENT_PILE_PARTS_CU_INFO,
	EVS_CMD_EVENT_PILE_PARTS_SWITCH_INFO,
	EVS_CMD_EVENT_PILE_PARTS_EDAS_INFO,
	EVS_CMD_EVENT_PILE_PARTS_CU_WORK_INFO,
	EVS_CMD_EVENT_RAW_DATA,
	EVS_CMD_EVENT_DEV_MAINTAIN_RESULT,
	EVS_CMD_EVENT_LOCK_CTRL_RESULT,
	EVS_CMD_EVENT_TIME_SYNC_RESULT,

} evs_cmd_event_enum;

typedef enum
{
	EVS_CMD_PROPERTY_DCPILE = 0,
	EVS_CMD_PROPERTY_ACPILE,
	EVS_CMD_PROPERTY_AC_WORK,
	EVS_CMD_PROPERTY_AC_NONWORK,
	EVS_CMD_PROPERTY_DC_WORK,
	EVS_CMD_PROPERTY_DC_NONWORK,
	EVS_CMD_PROPERTY_DC_OUTMETER,
	EVS_CMD_PROPERTY_AC_OUTMETER,
	EVS_CMD_PROPERTY_BMS,
	EVS_CMD_PROPERTY_DC_INPUT_METER,
} evs_cmd_property_enum;

// 固件信息上报事件参数

typedef struct
{
	char simNo[EVS_MAX_ICCID_LEN];								   // 1		SIM卡号
	char feeModelId[EVS_MAX_MODEL_ID_LEN];						   // 2		电费计费模型编号
	char stakeModel[EVS_MAX_PILE_TYPE_LEN];						   // 3		充电桩型号
	unsigned int vendorCode;									   // 4		生产厂商编码
	char devSn[EVS_MAX_DEV_SN_LEN];								   // 5		出厂编号//字符串
	unsigned char devType;										   // 6		桩类型
	unsigned char portNum;										   // 7		充电接口数量
	char simMac[EVS_MAX_MAC_ADDR_LEN];							   // 8		网络MAC地址//字符串
	unsigned int longitude;										   // 9		经度
	unsigned int latitude;										   // 10		纬度
	unsigned int height;										   // 11		高度
	unsigned int gridType;										   // 12		坐标类型
	char btMac[EVS_MAX_MAC_ADDR_LEN];							   // 13		蓝牙MAC地址
	unsigned char meaType;										   // 14		计量方式
	unsigned int otRate;										   // 15		额定功率
    unsigned int otMinVol;										   // 16		输出最小电压
	unsigned int otMaxVol;										   // 17		输出最大电压
	unsigned int otCur;										       // 18		输出最大电流
	char inMeter[EVS_MAX_INPUT_METER_NUM][EVS_MAX_METER_ADDR_LEN]; // 19		交流输入电表地址//压缩BCD
	char outMeter[EVS_MAX_PORT_NUM][EVS_MAX_METER_ADDR_LEN];	   // 20		计量用电能表地址//压缩BCD
	unsigned int CT;											   // 21		电流互感器系数 默认值1
	unsigned char isGateLock;									   // 22		是否有智能门锁
	unsigned char isGroundLock;									   // 23		是否有地锁
	unsigned char mutliChargingMode;							   // 24		是否支持多时段类型计费模型
} evs_event_firmware_info;

// 即插即充智能卡信息
typedef struct
{
	unsigned char gunNo;				 // 1		充电枪编号
	char cardSn[EVS_MAX_CARD_LEN];		 // 2		智能卡序列号
	char cardPsySn[EVS_MAX_PSYCARD_LEN]; // 3		智能卡物理卡号
	unsigned char startMode;			 // 4		充电模式
} evs_event_card_info;

// 即插即充智能卡鉴权信息
typedef struct
{
	unsigned char gunNo;				   // 1		充电枪编号
	char randomC[EVS_MAX_RANDDOM_LEN];	   // 2		智能卡随机数
	char cardBalance[8 + 1];			   // 3		智能卡余额
	char cardTradeSn[4 + 1];			   // 4		智能卡联机交易序号
	char keyVer[3];						   // 5		秘钥版本
	char keyFlag[3];					   // 6		密钥标识
	char tradeMac1[EVS_MAX_TRADE_MAC_LEN]; // 7		鉴权码
} evs_event_card_auth;

// 即插即充智能卡鉴权结果信息
typedef struct
{
	unsigned char gunNo;				  // 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN];	  // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	  // 3	设备交易流水号
	char tradeTac[EVS_MAX_TRADE_MAC_LEN]; // 4	鉴权确认码
} evs_event_card_auth_result;

// 设备版本信息
typedef struct
{
	unsigned char devRegMethod;					   // 1		设备注册方式
	char pileSoftwareVer[EVS_MAX_SOFTWAREVER_LEN]; // 2		充电桩软件版本号
	char pileHardwareVer[EVS_MAX_HARDWAREVER_LEN]; // 3		充电桩硬件版本号
	char sdkVer[EVS_MAX_SDKVER_LEN];			   // 4		SDK版本号
} evs_event_ver_info;

// 设备配置数据
typedef struct
{
	unsigned int equipParamFreq;					   // 1		充电设备实时监测属性上报频率
	unsigned int gunElecFreq;						   // 2		充电枪充电中实时监测属性上报频率
	unsigned int nonElecFreq;						   // 3		充电枪非充电中实时监测属性上报频率
	unsigned int faultWarnings;						   // 4		故障告警全信息上传频率
	//unsigned int acMeterFreq;						   // 5		充电设备交流电表底值监测属性上报频率
	//unsigned int dcMeterFreq;						   // 6		直流输出电表底值监测属性上报频率
	unsigned int offlinChaLen;						   // 5	    离线后可充电时长
	unsigned int grndLock;							   // 6		地锁监测上送频率
	unsigned int doorLock;							   // 7		网门锁监测上送频率
	char qrCode[EVS_MAX_PORT_NUM][EVS_MAX_QRCODE_LEN]; // 8	    二维码数据
	unsigned int s2CloseTime;						   // 9	    交流启动S2闭合超时
	unsigned int s2OpenTime;						   // 10    交流充电中S2长时间断开超时
} evs_data_dev_config;

// 设备日志查询服务下发参数

typedef struct
{
	unsigned char gunNo;				   // 枪号	gunNo
	unsigned int startDate;				   // 查询起始时间戳	startDate
	unsigned int stopDate;				   // 查询终止时间戳	stopDate
	unsigned char askType;				   // 查询类型	askType
	char logQueryNo[EVS_MAX_LOGQUERY_LEN]; // 查询流水号
} evs_service_query_log;

// 日志查询服务回复参数

typedef struct
{
	unsigned char gunNo;				   // 1	枪号
	unsigned int startDate;				   // 2	查询起始时间
	unsigned int stopDate;				   // 3	查询终止时间
	unsigned char askType;				   // 4	查询类型
	unsigned char result;				   // 5	响应结果
	char logQueryNo[EVS_MAX_LOGQUERY_LEN]; // 6	查询流水号
} evs_service_feedback_query_log;

// 设备维护指令服务下发参数
typedef struct
{
	unsigned char gunNo;	// 1	枪号
	unsigned char ctrlType; // 2	控制类型
} evs_service_dev_maintain;

// 设备维护状态查询服务回复参数
typedef struct
{
	unsigned char ctrlType; // 1		当前类型
	unsigned char result;	// 2		查询结果
} evs_service_feedback_maintain_query;

// 充电枪电子锁控制服务下发参数

typedef struct
{
	unsigned char gunNo;	 // 1	充电枪编号
	unsigned char lockParam; // 2	控制
} evs_service_lockCtrl;

// 计费模型请求事件

typedef struct
{
	char feeModelId[EVS_MAX_MODEL_ID_LEN]; // 1	计费模型编号
} evs_event_ask_feeModel;

// 计费模型更新服务下发参数

typedef struct
{
	char feeModelId[EVS_MAX_MODEL_ID_LEN];		   // 1		电费计费模型编号
	unsigned char timeNum;						   // 2		电费模型时段数N 取值范围：1—96
	char timeSeg[EVS_MAX_MODEL_DEVSEG][5];		   // 3		电费模型时段开始时间点
	unsigned int chargeFee[EVS_MAX_MODEL_DEVSEG];  // 4		电费模型
	unsigned int serviceFee[EVS_MAX_MODEL_DEVSEG]; // 5 	服务费费模型
} evs_service_issue_feeModel;

// 计费模型更新结果设备回复参数

typedef struct
{
	char feeModelId[EVS_MAX_MODEL_ID_LEN]; // 1		电费计费模型编号
	unsigned char result;				   // 2		失败原因
} evs_service_feedback_feeModel;

// 远程启动充电服务下发参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char startType;			// 4	启动方式
	unsigned char chargeMode;			// 5	充电模式
	unsigned int limitData;				// 6	限制值
	unsigned int stopCode;				// 7	停机码
	unsigned char startMode;			// 8	启动模式
	unsigned int insertGunTime;			// 10	插枪事件时间戳
} evs_service_startCharge;

// 启动充电服务设备回复参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
} evs_service_feedback_startCharge;

// 启动充电结果事件参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char startResult;			// 4	启动结果
	unsigned int faultCode;				// 5	故障代码
	char vinCode[EVS_MAX_CAR_VIN_LEN];	// 6	vin码
} evs_event_startResult;

// 启动充电鉴权事件参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char startType;			// 4	启动方式
	char authCode[EVS_MAX_CAR_VIN_LEN]; // 5	鉴权码 若启动方式为反向扫码，填入二维码信息；若为即插即充则为VIN码。
	unsigned char batterySOC;			// 6	电池SOC
	unsigned int batteryCap;			// 7	电车容量
	unsigned int chargeTimes;			// 8	已充电次数
	unsigned int batteryVol;			// 9	当前电池电压
} evs_event_startCharge;

// 鉴权充电服务下发参数

typedef struct
{
	unsigned char gunNo;				 // 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN];	 // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	 // 3	设备交易流水号
	unsigned char startType;			 // 4	启动方式
	char authCode[EVS_MAX_AUTHCODE_LEN]; // 5	鉴权码
	unsigned char result;				 // 6	鉴权结果
	unsigned char chargeMode;			 // 7	充电模式
	unsigned int limitData;				 // 8	限制值
	unsigned int stopCode;				 // 9	停机码
	unsigned char startMode;			 // 10	启动模式
	unsigned int insertGunTime;			 // 11	插枪事件时间戳
} evs_service_authCharge;

// 鉴权启动充电服务设备回复参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
} evs_service_feedback_authCharge;

// 远程停止充电服务下发参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char stopReason;			// 4	停止原因
} evs_service_stopCharge;

// 远程停止充电服务设备回复参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
} evs_service_feedback_stopCharge;

// 设备部件配置参数
typedef struct
{
	unsigned char gunNo;				  // 1 充电枪编号
	char optSn[EVS_MAX_OPT_SN_LEN];		  // 2 平台操作序号
	unsigned int funCode;				  // 3	功能代码
	unsigned int confInt;				  // 4	参数信息
	char confString[EVS_MAX_FUNCONF_LEN]; // 5	字符信息
										  // unsigned int powerAlloc;		// 3 功率动态分配策略
										  // char partsType;					// 4 组件类型
										  // char sendMode;					// 5 上送方式
										  // char ccuPeriod;					// 6 控制器上送周期
										  // char tcuPeriod;					// 7 tcu上送周期
} evs_service_config_parts;

// 设备部件配置参数结果
typedef struct
{
	unsigned int resCode;			// 1 结果
	char optSn[EVS_MAX_OPT_SN_LEN]; // 2 平台操作序号
	unsigned int funCode;			// 3	功能代码
} evs_service_feedback_config_parts;

// 设备部件配置参数查询
typedef struct
{
	unsigned char gunNo;	 // 1	充电枪编号
	unsigned char partsType; // 2	部件类型
	unsigned int funCode;	 // 3	功能代码
} evs_service_parts_config_get;

// 设备部件配置结果

typedef struct
{
	unsigned int result;				  // 1 结果
	unsigned int confInt;				  // 2 配置参数信息
	char confString[EVS_MAX_FUNCONF_LEN]; // 3 配置字符信息
} evs_service_feedback_config_parts_get;

// 交易记录召测服务
typedef struct
{
	unsigned char gunNo;				   // 1 充电枪编号
	unsigned char askType;				   // 2 召测类型
	char preTradeNo[EVS_MAX_TRADE_LEN];	   // 3	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	   // 4	设备交易流水号
	char startDate[EVS_MAX_TIMESTAMP_LEN]; // 5 查询起始时间
	char stopDate[EVS_MAX_TIMESTAMP_LEN];  // 6 查询结束时间
} evs_service_trade_get;

// 交易记录召测结果

typedef struct
{
	unsigned char gunNo;				   // 1 充电枪编号
	unsigned char askType;				   // 2 召测类型
	char preTradeNo[EVS_MAX_TRADE_LEN];	   // 3	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	   // 4	设备交易流水号
	char startDate[EVS_MAX_TIMESTAMP_LEN]; // 5 查询起始时间
	char stopDate[EVS_MAX_TIMESTAMP_LEN];  // 6 查询结束时间
	unsigned char askResult;			   // 7 查询结果
	unsigned int tradeCnt;				   // 8 上报条数
} evs_service_feedback_trade_get;

// 电表底值召测服务

typedef struct
{
	unsigned char gunNo;				 // 1 充电枪编号
	unsigned char askType;				 // 2 召测类型
	char askDate[EVS_MAX_TIMESTAMP_LEN]; // 3 查询时间
} evs_service_meter_get;

// 电表底值召测结果

typedef struct
{
	unsigned char gunNo;				 // 1 充电枪编号
	unsigned char askType;				 // 2 召测类型
	char askDate[EVS_MAX_TIMESTAMP_LEN]; // 3 查询时间
	unsigned char askResult;			 // 4 召测结果
} evs_service_feedback_meter_get;

// 设备维护指令结果

typedef struct
{
	unsigned char ctrlType; // 1	当前控制类型
	unsigned int reason;	// 2	失败原因
} evs_event_feedback_dev_maintain;

// 电子锁控制结果

typedef struct
{
	unsigned char gunNo;	  // 1	充电枪编号
	unsigned char lockStatus; // 2	电子锁状态
	unsigned int resCode;	  // 3	结果
} evs_event_feedback_lockCtrl;

// 停止充电结果事件上传参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char stopResult;			// 4	停止结果
	unsigned int resultCode;			// 5	停止原因
	unsigned char stopFailReson;		// 6	停止失败原因
} evs_event_stopCharge;

// 交易记录事件上传参数

typedef struct
{
	unsigned char gunNo;							// 1 充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN];				// 2 平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];				// 3 设备交易流水号
	char vinCode[EVS_MAX_CAR_VIN_LEN];				// 4 VIN
	unsigned char timeDivType;						// 5 计量计费类型
	unsigned char startType;						// 6 启动方式
	unsigned int chargeStartTime;					// 7 开始充电时间
	unsigned int chargeEndTime;						// 8 结束充电时间
	unsigned char startSoc;							// 9 启动时SOC
	unsigned char endSoc;							// 10 停止时SOC
	unsigned int reason;							// 11 停止充电原因
	char feeModelId[EVS_MAX_MODEL_ID_LEN];			// 12 计量计费模型编号
	long long sumStart;								// 13 电表总起示值
	long long sumEnd;								// 14 电表总止示值
	unsigned int totalElect;						// 15 总电量
	unsigned int totalPowerCost;					// 16 总电费
	unsigned int totalServCost;						// 17 总服务费
	unsigned int totalCost;							// 18 总消费金额
	unsigned char timeNum;							// 19 时段数
	unsigned int partElect[EVS_MAX_MODEL_DEVSEG];	// 20 时段电量
	unsigned int chargeFee[EVS_MAX_MODEL_DEVSEG];	// 21 时段电费
	unsigned int serviceFee[EVS_MAX_MODEL_DEVSEG];	// 22 时段服务费
	unsigned char startPoint;						// 23 起始点标识
	unsigned char crossPoints;						// 24 跨越点数
	unsigned int pointsElect[EVS_MAX_MODEL_DEVSEG]; // 25 跨越点电量
} evs_event_tradeInfo;

// 交易记录确认服务下发参数

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char errcode;				// 4	交易记录上传结果
} evs_service_confirmTrade;

// 故障告警事件上传参数

typedef struct
{
	unsigned char gunNo;						  // 1	枪编号
	unsigned short faultSum;					  // 2	故障总
	unsigned short warnSum;						  // 3	告警总
	unsigned short faultValue[EVS_MAX_ALARM_LEN]; // 4	故障点数据
	unsigned short warnValue[EVS_MAX_ALARM_LEN];  // 5	告警点数据
} evs_event_alarm;

// VIN白名单更新服务下发参数

typedef struct
{
	unsigned char updateSN[EVS_MAX_VIN_SN_LEN];				 // 1  白名单编号
	unsigned char optType;									 // 2  操作类型
	unsigned int totalVINCnt;								 // 3  白名单总数
	unsigned int packTotalCnt;								 // 4  更新总包数
	unsigned int packCnt;									 // 5  更新包序号
	unsigned int packVINCnt;								 // 6  当前包VIN条数
	char vinList[EVS_MAX_VIN_LIST_LEN][EVS_MAX_CAR_VIN_LEN]; // 7  vin列表
} evs_service_vinList_update;

// VIN白名单更新服务设备回复参数

typedef struct
{
	unsigned char updateSN[EVS_MAX_VIN_SN_LEN]; // 1  白名单编号
	unsigned char optType;						// 2  操作类型
	unsigned char result;						// 3  结果
} evs_service_feedback_vinList_update;

// 预约充电服务下发参数

typedef struct
{
	unsigned char gunNo;		  // 1	    充电枪编号
	unsigned char appoMethod;	  // 2		预约方式
	unsigned short appoDelay;	  // 3		预留
	unsigned short appoData;	  // 4		预留
	unsigned int startChargeTime; // 5 	启动充电时间
	unsigned int stopChargeTime;  // 6 	停止充电时间
} evs_service_rsvCharge;

// 预约充电服务设备回复参数

typedef struct
{
	unsigned char gunNo;	  // 1	充电枪编号
	unsigned char appoMethod; // 2	预约方式
	unsigned char result;	  // 3	预约结果
} evs_service_feedback_rsvCharge;

// 地锁控制服务下发参数

typedef struct
{
	unsigned char gunNo;	// 1	充电枪编号
	unsigned char ctrlFlag; // 2	控制指令
} evs_service_groundLock_ctrl;

// 地锁控制服务回复参数

typedef struct
{
	unsigned char gunNo;  // 1	充电枪编号
	unsigned char result; // 2	控制结果
	unsigned char reason; // 3	失败原因
} evs_service_feedback_groundLock_ctrl;

// 地锁状态变化事件上传参数

typedef struct
{
	unsigned char gunNo;		// 1	充电枪编号
	unsigned char lockState;	// 2	地锁状态
	unsigned char powerType;	// 3	供电方式
	unsigned char cellState;	// 4	电池状态
	unsigned char lockerState;	// 5	锁舌状态
	unsigned char lockerForced; // 6	锁舌受外力强制动作
	unsigned char lowPower;		// 7	电池低电量报警
	unsigned char soc;			// 8	电池SOC
	unsigned int openCnt;		// 9	开闭次数
} evs_event_groundLock_change;

// 智能门锁控制服务下发参数

typedef struct
{
	unsigned char lockNo;	// 1	门锁编号
	unsigned char ctrlFlag; // 2	控制指令
} evs_service_gateLock_ctrl;

// 智能门锁控制服务回复参数

typedef struct
{
	unsigned char lockNo; // 1	充电枪编号
	unsigned char result; // 2	控制结果
} evs_service_feedback_gateLock_ctrl;

// 智能门锁状态变化事件上传参数

typedef struct
{
	unsigned char lockNo;	 // 1	充电枪编号
	unsigned char lockState; // 2	智能门锁状态
} evs_event_gateLock_change;

// 有序充电策略服务下发参数

typedef struct
{
	char preTradeNo[EVS_MAX_TRADE_LEN];			 // 1	订单流水号
	unsigned char num;							 // 2	策略配置时间段数量
	unsigned char validTime[EVS_MAX_SEG_LEN][5]; // 3	策略生效时间//字符串数组。时间格式采用HHMM，24小时制。例如 ：[time1,time2,time3…]。
	unsigned short kw[EVS_MAX_SEG_LEN];			 // 4	策略配置功率//整型数组。功率精确到0.1KW[kw1,kw2,kw3…]
} evs_service_orderCharge;

// 有序充电策略服务设备回复参数

typedef struct
{
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 1	订单流水号
	unsigned char result;				// 2	返回结果
	unsigned char reason;				// 3	失败原因
} evs_service_feedback_orderCharge;

// 交直流充电设备枪状态变化事件上传参数

typedef struct
{
	unsigned char gunNo;		   // 1	充电枪编号
	unsigned int yxOccurTime;	   // 2	发生时刻
	unsigned char connCheckStatus; // 3	变位点数据
} evs_event_pile_stutus_change;

// 交直流充电设备充电前车辆信息上报事件上传参数

typedef struct
{
	unsigned char gunNo;			   // 1	充电枪编号
	unsigned char batterySOC;		   // 2	电池SOC
	unsigned int batteryCap;		   // 3	电车容量
	char vinCode[EVS_MAX_CAR_VIN_LEN]; // 4	vin码
	unsigned char state;			   // 5	获取车辆信息状态
} evs_event_car_info;

// 设备组件信息上报事件参数

typedef struct
{
	char netMduInfo[EVS_MAX_NETMDUINFO_LEN];						  // 1	网络模块型号
	char netMduSoftVer[EVS_MAX_NETMDUINFO_LEN];						  // 2	网络模块软件版本号
	char netMduImei[EVS_MAX_NETMDUINFO_LEN];						  // 3	网络模块IMEI
	unsigned char smartGun;											  // 4	智能充电枪功能 10支持，11不支持
	unsigned int mduInfoInt[EVS_MAX_MDUINT_NUM];					  // 5	int型数组
	char mduInfoString[EVS_MAX_MDUSTRING_NUM][EVS_MAX_MDUSTRING_LEN]; // 6	string型数组
} evs_event_devmdu_info;

// VIN白名单查询结果数据

typedef struct
{
	unsigned char result;
	char updateSN[EVS_MAX_VIN_SN_LEN];						 // 1  白名单编号
	unsigned int totalVINCnt;								 // 3  白名单总数
	unsigned int packTotalCnt;								 // 4  更新总包数
	unsigned int packCnt;									 // 5  更新包序号
	unsigned int packVINCnt;								 // 6  当前包VIN条数
	char vinList[EVS_MAX_VIN_LIST_LEN][EVS_MAX_CAR_VIN_LEN]; // 7  vin列表
} evs_event_vinList_result;

// 时钟同步结果

typedef struct
{
	char srvTime[EVS_MAX_TIMESTAMP_LEN]; // 1  平台时间
	char devTime[EVS_MAX_TIMESTAMP_LEN]; // 2  设备当前时间
	unsigned char resCode;				 // 3  同步结果
} evs_event_time_sync_result;

// 设备功能配置数据

typedef struct
{
	unsigned int funCode;				  // 1	功能代码
	unsigned int confInt;				  // 2	参数信息
	char confString[EVS_MAX_FUNCONF_LEN]; // 3	字符信息
	char optSn[EVS_MAX_OPT_SN_LEN];		  // 4	操作流水号
} evs_service_dev_fun_config;

// 查询设备功能配置回复参数

typedef struct
{
	unsigned int resCode;			// 1	结果
	unsigned int funCode;			// 2	功能代码
	char optSn[EVS_MAX_OPT_SN_LEN]; // 3	操作流水号
} evs_service_feedback_dev_fun_config;

// 查询设备功能配置

typedef struct
{
	unsigned int funCode; // 1	功能代码
} evs_service_get_dev_fun_config;

// 计费模型查询服务下发参数

typedef struct
{
	unsigned char gunNo; // 1	充电枪编号
} evs_service_feeModel_query;

// 计费模型查询结果设备回复参数

typedef struct
{

	char feeModelId[EVS_MAX_MODEL_ID_LEN]; // 1		电费模型编号
	unsigned char timeNum;				   // 2		电费模型时段数N 取值范围：1—96
	char timeSeg[EVS_MAX_MODEL_DEVSEG][5]; // 3		电费模型时段开始时间点

	unsigned int chargeFee[EVS_MAX_MODEL_DEVSEG];  // 4		电费模型
	unsigned int serviceFee[EVS_MAX_MODEL_DEVSEG]; // 5 	服务费费模型
} evs_service_feedback_feeModel_qurey;

// 充电中连接状态变化事件参数

typedef struct
{
	unsigned char gunNo;	  // 1	充电枪编号
	unsigned char cpStatus;	  // 2	CP状态
	unsigned int cpVolt;	  // 3	CP电压采集值
	unsigned char s3Status;	  // 4	S3开关状态
	unsigned int yxOccurTime; // 5	发生时刻
} evs_event_charge_connect_change;

// 智能卡检测异常事件参数

typedef struct
{
	unsigned char gunNo;				 // 1	充电枪编号
	char cardSn[EVS_MAX_CARD_LEN];		 // 2	智能卡序列号
	char cardPsySn[EVS_MAX_PSYCARD_LEN]; // 3	智能卡物理卡号
	unsigned char faultValue;			 // 4	失败原因
} evs_event_card_check_error;
// 蓝牙秘钥更新服务参数

typedef struct
{
	char bleSecretKey[EVS_MAX_BLE_SECRETKEY_LEN]; // 1	蓝牙秘钥
	unsigned int updateTime;					  // 2	更新时间
} evs_service_blesecret_update;

// 蓝牙秘钥更新服务设备回复参数

typedef struct
{
	char bleSecretKey[EVS_MAX_BLE_SECRETKEY_LEN]; // 1	蓝牙秘钥
	unsigned char result;						  // 2	返回结果
} evs_service_feedback_blesecret_update;

// 蓝牙即插即充信息上报事件参数

typedef struct
{
	unsigned char plugChgStatus;								// 1    即插即充状态
	unsigned char macNum;										// 2	MAC数量
	char macList[EVS_MAX_BLE_MAC_NUM][EVS_MAX_BLE_MACLIST_LEN]; // 3	蓝牙列表
} evs_event_ble_plug_charge_info;

// 蓝牙即插即充信息确认服务下发参数

typedef struct
{
	unsigned char macNum; // 1	MAC数量
} evs_service_ble_plug_charge_info_conf;

// 蓝牙鉴权列表清除服务参数

typedef struct
{
	unsigned char cleanType;			// 1	清除方式
	char macInfo[EVS_MAX_MAC_ADDR_LEN]; // 2	蓝牙MAC

} evs_service_blelist_clean;

// 蓝牙MAC鉴权列表清除服务设备回复参数

typedef struct
{
	unsigned char cleanType; // 1	清除方式
	unsigned char result;	 // 2	清除结果
} evs_service_feedback_blelist_clean;

// 蓝牙连接状态变更事件参数

typedef struct
{
	unsigned int yxOccurTime;		   // 1	发生时刻
	unsigned char connCheckStatus;	   // 2	变位点数据
	char macInfo[EVS_MAX_BLE_MAC_LEN]; // 3	MAC信息
} evs_event_ble_conn_change;

// 蓝牙即插即充重置服务设备回复参数

typedef struct
{
	unsigned char result; // 1	重置结果
} evs_service_feedback_ble_reset;

/*********************************************交流业务相关************************************************************/
// 交流设备属性上报参数

typedef struct
{
	unsigned char netType;						// 1 网络类型
	unsigned char sigVal;						// 2 网络信号等级
	unsigned char netId;						// 3 网络运营商
	unsigned int acVolA;						// 4 A相采集电压
	unsigned int acCurA;						// 5 A相采集电流
	unsigned int acVolB;						// 6 B相采集电压
	unsigned int acCurB;						// 7 B相采集电流
	unsigned int acVolC;						// 8 C相采集电压
	unsigned int acCurC;						// 9 C相采集电流
	unsigned short caseTemp;					// 10 桩内温度
	char feeModelId[EVS_MAX_MODEL_ID_LEN];		// 11 计费模型编号
	unsigned int totalRam;						// 12 总内存 单位：KB
	unsigned char ramUseRate;					// 13 内存使用率
	unsigned int totalRom;						// 14 总硬盘空间 单位：KB
	unsigned char romUseRate;					// 15 硬盘使用率
	unsigned char cpuUseRate;					// 16 CPU使用率
	char netSigQua[EVS_MAX_NET_SIG_QUA_LEN];	// 17 网络信号质量
	char devRunSampVal[EVS_MAX_SAMPLE_VAL_LEN]; // 18 设备运行采样数据
} evs_property_acPile;

// 交流充电枪充电中实时监测属性

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	unsigned char workStatus;			// 3	工作状态
	unsigned char conStatus;			// 4	连接确认开关状态
	unsigned char outRelayStatus;		// 5	输出继电器状态
	unsigned char eLockStatus;			// 6	充电接口电子锁状态
	unsigned short gunTemp;				// 7	充电枪头温度
	unsigned int acVolA;				// 8	充电设备A相输出电压
	unsigned int acCurA;				// 9	充电设备A相输出电流
	unsigned int acVolB;				// 10	充电设备B相输出电压
	unsigned int acCurB;				// 11	充电设备B相输出电流
	unsigned int acVolC;				// 12	充电设备C相输出电压
	unsigned int acCurC;				// 13	充电设备C相输出电流
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 14	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 15	设备交易流水号
	unsigned int realPower;				// 16	充电实际功率
	unsigned int chgTime;				// 17	累计充电时间
	unsigned int PwmDutyRadio;			// 18	PWM占空比

	unsigned int s2SwhActNum; // 19	S2开关动作次数
	long long meterStartVal;  // 20	表底值起始值
	long long meterRealVal;	  // 21	表底值当前值

	unsigned int totalElect;	 // 22	总电量
	unsigned int totalCost;		 // 23	总金额
	unsigned int totalPowerCost; // 24	总电费
	unsigned int totalServCost;	 // 25	总服务费

	unsigned char timeNum;						   // 26	时段数
	unsigned int partElect[EVS_MAX_MODEL_DEVSEG];  // 27	时段电量
	unsigned int chargeFee[EVS_MAX_MODEL_DEVSEG];  // 28	时段电费
	unsigned int serviceFee[EVS_MAX_MODEL_DEVSEG]; // 29	时段服务费

	unsigned char startPoint;						// 30	起始点标识
	unsigned char crossPoints;						// 31	跨越点数
	unsigned int pointsElect[EVS_MAX_MODEL_DEVSEG]; // 32	跨越点电量

} evs_property_ac_work;

// 交流充电枪非充电中实时监测属性

typedef struct
{
	unsigned char gunNo;		  // 1	充电枪编号
	unsigned char workStatus;	  // 3	工作状态
	unsigned char conStatus;	  // 4	连接确认开关状态
	unsigned char outRelayStatus; // 5	输出继电器状态
	unsigned char eLockStatus;	  // 6	充电接口电子锁状态
	unsigned short gunTemp;		  // 7	充电枪头温度
	unsigned int acVolA;		  // 8	充电设备A相输出电压
	unsigned int acCurA;		  // 9	充电设备A相输出电流
	unsigned int acVolB;		  // 10	充电设备B相输出电压
	unsigned int acCurB;		  // 11	充电设备B相输出电流
	unsigned int acVolC;		  // 12	充电设备C相输出电压
	unsigned int acCurC;		  // 13	充电设备C相输出电流
	long long sumMeter;			  // 14 电表底值
} evs_property_ac_nonWork;

/*********************************************直流业务相关************************************************************/

// 直流设备属性上报参数

typedef struct
{
	unsigned char netType;				   // 1 网络类型
	unsigned char sigVal;				   // 2 网络信号等级
	unsigned char netId;				   // 3 网络运营商
	unsigned int acVolA;				   // 4 A相采集电压
	unsigned int acCurA;				   // 5 A相采集电流
	unsigned int acVolB;				   // 6 B相采集电压
	unsigned int acCurB;				   // 7 B相采集电流
	unsigned int acVolC;				   // 8 C相采集电压
	unsigned int acCurC;				   // 9 C相采集电流
	unsigned char caseHumidity;			   // 10 设备内湿度
	unsigned short caseTemp;			   // 11 设备内温度
	unsigned short inletTemp;			   // 12 设备入风口温度
	unsigned short outletTemp;			   // 13 设备出风口温度
	char feeModelId[EVS_MAX_MODEL_ID_LEN]; // 14 计费模型编号
	unsigned int totalRam;				   // 15 总内存
	unsigned char ramUseRate;			   // 16 内存使用率
	unsigned int totalRom;				   // 17 总硬盘空间
	unsigned char romUseRate;			   // 18 磁盘使用率
	unsigned char cpuUseRate;			   // 19 cpu使用率
	unsigned int sysStartTime;			   // 20 系统启动时间
	unsigned int sysWorkTime;			   // 21 系统累计运行时间
} evs_property_dcPile;

// 直流充电枪BMS监测属性

typedef struct
{
	unsigned char gunNo;				// 1	充电枪编号
	char preTradeNo[EVS_MAX_TRADE_LEN]; // 2	平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];	// 3	设备交易流水号
	unsigned char socVal;				// 4	SOC
	unsigned char BMSVer;				// 5	BMS通信协议版本号
	unsigned short BMSMaxVol;			// 6	最高允许充电总电压
	unsigned char batType;				// 7	电池类型
	unsigned short batRatedCap;			// 8	整车动力蓄电池额定容量
	unsigned short batRatedTotalVol;	// 9	整车动力蓄电池额定总电压
	unsigned short singlBatMaxAllowVol; // 10	单体动力蓄电池最高允许充电电压
	unsigned short maxAllowCur;			// 11	最高允许充电电流
	unsigned short battotalEnergy;		// 12	整车动力蓄电池标称总能量
	unsigned short maxVol;				// 13	最高允许充电总电压
	unsigned short maxTemp;				// 14	最高允许温度
	unsigned short batCurVol;			// 15	整车动力蓄电池当前电池电压
	char batManufacturer[11];			// 16	电池组厂商
	char batSN[4];						// 17	电池组序号
	char batMadeDay[9];					// 18	电池组生产日期
	unsigned int chargeTimes;			// 19	电池组充电次数
	unsigned char batProperty;			// 20	电池产权
	char bmsSoftVer[11];				// 21	BMS软件版本号
} evs_property_BMS;

// 直流充电枪充电中实时监测属性

typedef struct
{
	unsigned char gunNo;							// 1 充电枪编号
	unsigned char workStatus;						// 2 工作状态
	unsigned char gunStatus;						// 3 充电枪连接状态
	unsigned char eLockStatus;						// 4 充电枪电子锁状态
	unsigned char DCK1Status;						// 5 直流输出接触器K1状态
	unsigned char DCK2Status;						// 6 直流输出接触器K2状态
	unsigned char DCPlusFuseStatus;					// 7 DC+熔断器状态
	unsigned char DCMinusFuseStatus;				// 8 DC-熔断器状态
	unsigned short conTemp1;						// 9 充电接口DC+温度
	unsigned short conTemp2;						// 10 充电接口DC-温度
	unsigned int dcVol;								// 11 输出电压
	unsigned int dcCur;								// 12 输出电流
	char preTradeNo[EVS_MAX_TRADE_LEN];				// 13 平台交易流水号
	char tradeNo[EVS_MAX_TRADE_LEN];				// 14 设备交易流水号
	unsigned char chgType;							// 15 充电类型
	unsigned int realPower;							// 16 充电设备输出功率
	unsigned int chgTime;							// 17 累计充电时间
	unsigned short remainT;							// 18 估算充满剩余充电时间
	unsigned char socVal;							// 19 SOC
	unsigned short needVol;							// 20 充电需求电压
	unsigned short needCur;							// 21 充电需求电流
	unsigned char chargeMode;						// 22 充电模式
	unsigned short bmsVol;							// 23 BMS充电电压测量值
	unsigned short bmsCur;							// 24 BMS充电电流测量值
	unsigned short SingleMHV;						// 25 最高单体动力蓄电池电压
	unsigned short SingleMLV;						// 26 最高单体动力蓄电池电压
	unsigned short MHTemp;							// 27 最高动力蓄电池温度
	unsigned short MLTemp;							// 28 最低动力蓄电池温度
	unsigned int SingleMHVNo;						// 29 最高单体动力蓄电池电压所在编号
	unsigned int MHTempNo;							// 30 最高动力蓄电池温度检测点编号
	unsigned int MLTempNo;							// 31 最低动力蓄电池温度检测点编号
	unsigned short guidanceVol;						// 32 控制导引电压
	unsigned char acInputContactorState;			// 33 交流输入接触器状态
	unsigned char acInputContactorCtrlState;		// 34 交流输入接触器控制状态
	unsigned char k1CtrlState;						// 35 直流接触器K1控制状态
	unsigned char k2CtrlState;						// 36 直流接触器K2控制状态
	unsigned char apsSwtichCtrlState;				// 37 交流输入接触器控制状态
	unsigned char carbinFanCtrlState;				// 38 风机开关控制状态
	unsigned char elockCtrlState;					// 39 电子锁控制状态
	long long meterStartVal;						// 40 表底值起始值
	long long meterRealVal;							// 41 表底值当前值
	unsigned int totalElect;						// 42 总电量
	unsigned int totalCost;							// 43 总金额
	unsigned int totalPowerCost;					// 44 总电费
	unsigned int totalServCost;						// 45 总服务费
	unsigned char timeNum;							// 46 时段数
	unsigned int partElect[EVS_MAX_MODEL_DEVSEG];	// 47 时段电量
	unsigned int chargeFee[EVS_MAX_MODEL_DEVSEG];	// 48 时段电费
	unsigned int serviceFee[EVS_MAX_MODEL_DEVSEG];	// 49 时段服务费
	unsigned char startPoint;						// 50 起始点标识
	unsigned char crossPoints;						// 51 跨越点数
	unsigned int pointsElect[EVS_MAX_MODEL_DEVSEG]; // 52 跨越点电量

} evs_property_dc_work;

// 直流充电枪非充电中实时监测属性

typedef struct
{
	unsigned char gunNo;				// 1 充电枪编号
	unsigned char workStatus;			// 2 工作状态
	unsigned char gunStatus;			// 3 充电枪连接状态
	unsigned char eLockStatus;			// 4 充电枪电子锁状态
	unsigned char DCK1Status;			// 5 直流输出接触器K1状态
	unsigned char DCK2Status;			// 6 直流输出接触器K2状态
	unsigned char DCPlusFuseStatus;		// 7 DC+熔断器状态
	unsigned char DCMinusFuseStatus;	// 8 DC-熔断器状态
	unsigned short conTemp1;			// 9 充电接口DC+温度
	unsigned short conTemp2;			// 10 充电接口DC-温度
	unsigned int dcVol;					// 11 输出电压
	unsigned int dcCur;					// 12 输出电流
	unsigned int chargeCnt;				// 13 充电枪总充电次数
	unsigned int chargeTime;			// 14 充电枪总充电时长
	unsigned int k1Cnt;					// 15 K1总动作次数
	unsigned int k2Cnt;					// 16 K2总动作次数
	unsigned char acContactorState;		// 17 交流输入接触器状态
	unsigned char guidanceVal;			// 18 控制导引电压
	unsigned char auxiliaryPowerStatus; // 19 辅助电源开关控制状态
	unsigned char elockCtrlState;		// 20 电子锁控制状态
	unsigned char fanSwitchCtrlStatus;	// 21 风机开关控制状态
	long long sumMeter;					// 22 电表底值

} evs_property_dc_nonWork;

// 直流输入电表底值监测属性

typedef struct
{
	unsigned char gunNo;							// 充电枪编号
	char acqTime[EVS_MAX_TIMESTAMP_LEN];			// 采集时间
	unsigned char mailAddr[EVS_MAX_METER_ADDR_LEN]; // 通信地址 压缩BCD
	unsigned char meterNo[EVS_MAX_METER_ADDR_LEN];	// 电表表号 压缩BCD
	char assetId[EVS_MAX_METER_ASSET_LEN];			// 电表资产编码
	long long sumMeter;								// 电表底值
	unsigned int ApElect;							// A相正向总电量
	unsigned int BpElect;							// B相正向总电量
	unsigned int CpElect;							// C相正向总电量
} evs_property_dc_input_meter;

/******************************************公共属性区*************************************************/

// 交直流输出电表底值监测属性

typedef struct
{
	unsigned char gunNo;							// 1	充电枪编号
	char acqTime[EVS_MAX_TIMESTAMP_LEN];			// 2	采集时间
	unsigned char mailAddr[EVS_MAX_METER_ADDR_LEN]; // 3	通信地址 压缩BCD
	unsigned char meterNo[EVS_MAX_METER_ADDR_LEN];	// 4	表号 压缩BCD
	char assetId[EVS_MAX_METER_ASSET_LEN];			// 5	电表资产编码
	long long sumMeter;								// 6	电表底值
	char lastTrade[EVS_MAX_TRADE_LEN];				// 7	最后交易流水
	unsigned int elec;								// 8	充电中订单的已充电量
} evs_property_meter;

// 设备日志查询结果上报事件
typedef union
{
	char rawData[EVS_MAX_LOGAREA_LEN];
	evs_event_tradeInfo tradeInfo;
	evs_property_meter meterData;
	evs_property_BMS BMSData;
} u_logData;

typedef struct
{
	unsigned char gunNo;				   // 1	充电枪编号
	unsigned int startDate;				   // 2	查询起始时间戳	startDate
	unsigned int stopDate;				   // 3	查询终止时间戳	stopDate
	unsigned char askType;				   // 4	查询类型
	unsigned char result;				   // 5	响应结果
	char logQueryNo[EVS_MAX_LOGQUERY_LEN]; // 6	查询流水号
	unsigned char retType;				   // 7	响应类型
	unsigned int logQueryEvtSum;		   // 8	日志结果上报事件总帧数
	unsigned int logQueryEvtNo;			   // 9	日志结果上报帧序号
	u_logData dataArea;					   // 10 响应数据区
} evs_event_logQuery_Result;

// 充电桩部件上报事件
typedef struct
{
	unsigned int partsAddr;
	unsigned int residualLife;
	unsigned int workTime;
	unsigned int alarmTimes;
	unsigned int faultTimes;
	unsigned int k1Cnt;
	unsigned int k2Cnt;
	unsigned int reliefCircuitCnt;
	unsigned int gunPlugCnt;
	unsigned int gunLockCnt;
	unsigned int totalChargeCnt;
	unsigned int totalChargeTime;
} evs_event_ccu_info; // 充电控制器

typedef struct
{
	unsigned int partsAddr;
	unsigned int residualLife;
	unsigned int workTime;
	unsigned int alarmTimes;
	unsigned int faultTimes;
} evs_event_pcu_info; // 功率控制模块

typedef struct
{
	unsigned int partsAddr;
	char devType[EVS_MAX_DEVINFO_LEN];
	char devSN[EVS_MAX_DEVINFO_LEN];
	unsigned int residualLife;
	unsigned int workTime;
	unsigned int alarmTimes;
	unsigned int faultTimes;

	unsigned int upTimes;
	unsigned int pfcTemper;
	unsigned int transformerTemper;
	unsigned int frontFanTemper;
	unsigned int backFanTemper;
	unsigned int inletTemer;
	unsigned int phaseVa;
	unsigned int phaseVb;
	unsigned int phaseVc;
	unsigned int status;
	unsigned int outputVol;
	unsigned int outputCur;
	unsigned int currentGID;
	unsigned int alarm1;
	unsigned int alarm2;
	unsigned int alarm3;
} evs_event_cu_info; // 充电模块

typedef struct
{
	unsigned int partsAddr;
	unsigned int residualLife;
	unsigned int workTime;
	unsigned int alarmTimes;
	unsigned int faultTimes;
	unsigned char switchType;
	unsigned char commState;
	unsigned int switchAnodeState[EVS_MAX_PARTS_SWITCH_NUM];
	unsigned int switchAnodeActCnt[EVS_MAX_PARTS_SWITCH_NUM];
	unsigned int switchCathodeActCnt[EVS_MAX_PARTS_SWITCH_NUM];
} evs_event_switch_info; // 开关模块

typedef struct
{
	unsigned int partsAddr;
	unsigned int residualLife;
	unsigned int workTime;
	unsigned int alarmTimes;
	unsigned int faultTimes;
} evs_event_edas_info; // 环境信息采集模块

typedef struct
{
	unsigned char gunNo;			// 1	枪编号
	unsigned char sumModul;			// 2	总模块数
	unsigned int ratedPower;		// 3	充电模块额定功率
	unsigned char workModulCnt;		// 4	在用充电模块总数
	unsigned char faultModulCnt;	// 5	故障充电模块总数
	unsigned char portWorkModulCnt; // 6	当前充电端口在用充电模块总数
} evs_event_cu_work_info;

typedef struct
{
	unsigned char gunNo;			 // 1	枪编号
	unsigned char startMode;		 // 2	启动模式
	char tradeNo[EVS_MAX_TRADE_LEN]; // 3	设备交易流水号
	unsigned char gunIsReady;		 // 4 	插枪状态 0未插枪  1已插枪
} evs_smart_gun_auth_param;

#endif
