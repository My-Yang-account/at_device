#ifndef _SERIAL_SCREEN_H_
#define _SERIAL_SCREEN_H_

#include "lib_c_queue.h"

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef float  f32;
typedef double f64;

typedef char INT8;
typedef short INT16;
typedef int INT32;
typedef long long INT64;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
#define NULL ((void *)0)
typedef enum sbool
    {
    FALSE = 0,
    TRUE = 1
    }BOOL;

#define sSCREEN_TX_CMD_MAX_LEN 150
#define sSCREEN_RX_CMD_MAX_LEN 150

typedef u32 in_addr_t;
struct in_addr
{
    in_addr_t s_addr;
};

u8 SerialScreenVersion[20];


struct LCD_DATA_FIFO_TYPE
{
    u8 buffer[sSCREEN_RX_CMD_MAX_LEN]; //缓冲区
    u16 put_index;
    u16 get_index;
    u8 is_full; //
    u8 is_empety; //
}LCD_DATA_FIFO_TYPE_t;


struct SerialScreenObj
{
	u8 *addr;			//本机通信地址
	u8 *rxptr;			//接收缓冲指针

	struct LCD_DATA_FIFO_TYPE * (*Initialize)(struct SerialScreenObj *cmd);	
	void (*Analyze)(struct SerialScreenObj *cmd, u16 len);		//命令执行
	void (*Process)(struct SerialScreenObj *cmd);		//信息处理
	s32  (*CheckSum)(u8 *pMsg,s32 uDataLen);		//计算校验
	int  (*SendData)(struct SerialScreenObj *cmd,u8 *ptr,u32 len); 
};

//void mem_swap(char* des, char len);
s32 String2BCD(s8* pSrc, u8* pDest);
int SerialScreen_IsCarConnect(int port);
int SerialScreen_IsSupportLocal(int port);
void SerialScreen_IsSupportAuxp24VSet(void);
void SerialScreen_IsSupportParaChargeSet(void);
void SerialScreen_IsSupportParaRelaySet(void);
void SerialScreen_IsSupportVINSet(void);
void SerialScreen_IsSupportIsulationSet(void);
void SerialScreen_IsSupportModuleSlienceSet(void);
void SerialScreen_IsSupportOfflineBillingSet(void);
void SerialScreen_IsSupportPWStartSet(void);
void SerialScreen_IsSupportOfflineCardSet(void);
void SerialScreen_IsSupportModeSelectSet(void);
struct charge_data *SerialScreen_GetChargeInfo(int port);
struct bms_info *SerialScreen_GetBmsInfo(int port);
struct temperature* SerialScreen_GetBatTemp(int port);
u8 SerialScreen_GetPagePos();
void SerialScreen_BtnReturn1(int port);
void SerialScreen_StartCharge(int port);
void SerialScreen_StartChargeA(void);
void SerialScreen_StartChargeB(void);
void SerialScreen_VinStartCharge(int port);
void SerialScreen_VinStartChargeA(void);
void SerialScreen_VinStartChargeB(void);
void SerialScreen_PWStartCharge(u8 port);
void SerialScreen_StopCharge(int port);
void SerialScreen_NeedPageReset(int port);
void SerialScreen_BtnChgInfoGet(int port);
void SerialScreen_BtnChgInfoSet(int port);
void SerialScreen_PWStartAuthen(void);
void SerialScreen_BtnServerGet(void);
void SerialScreen_BtnServerSet(void);
void SerialScreen_BtnVinListSet(void);
void SerialScreen_BtnVinListGet(void);
void SerialScreen_BtnModuleStartA(void);
void SerialScreen_BtnModuleStartB(void);
void SerialScreen_BtnModuleStopA(void);
void SerialScreen_BtnModuleStopB(void);
void SerialScreen_BtnModuleStateA(void);
void SerialScreen_BtnModuleStateB(void);
void SerialScreen_BtnBillClear(int port);
void SerialScreen_BtnBillGet(int port);
void SerialScreen_BtnBillGetB(void);
void SerialScreen_BtnBillGetA(void);
void SerialScreen_BtnBillUp(int port);
void SerialScreen_BtnBillDown(int port);
void SerialScreen_BtnErrClear(int port);
void SerialScreen_BtnErrGet(int port);
void SerialScreen_BtnErrGetB(void);
void SerialScreen_BtnErrGetA(void);
void SerialScreen_BtnErrUp(int port);
void SerialScreen_BtnErrDown(int port);
void SerialScreen_BtnModuleSet(void);
void SerialScreen_BtnModuleGet(void);
void SerialScreen_BtnSystemFuncSet(void);
void SerialScreen_BtnProtectInfoSet(void);
void SerialScreen_BtnProtectInfoGet(void);
void SerialScreen_BtnAuxSetA();
void SerialScreen_BtnAuxSetB();
void SerialScreen_BtnAuxSet(u8 port);
void SerialScreen_BtnAux24VSetA();
void SerialScreen_BtnAux24VSetB();
void SerialScreen_BtnAux24VSet(u8 port);
void SerialScreen_SysInfoGet(void);
void SerialScreen_SetInputInfo(void);
void SerialScreen_InputInfoGet(void);
void SerialScreen_DisChargeInfoGet(void);
void SerialScreen_NormalModeInfoGet(void);
void SerialScreen_ParaChargeSet(void);
void SerialScreen_SingleChargeSet(void);
void SerialScreen_IsSupportSet(void);
void SerialScreen_IsSupportPlugAndPlaySet(void);
void SerialScreen_IsSupportLocalStopSet(void);
void SerialScreen_IsSupportReaderSet(void);
void SerialScreen_IsSupportGet(void);
void SerialScreen_IsSupportSetFlash(void);

void SerialScreen_BtnMeterNoInfoSet(void);
void SerialScreen_BtnMeterNoInfoGet(void);
void SerialScreen_MeterNoInfoSet(int port);
void SerialScreen_SetMeterInfo(void);

void SerialScreen_InputSetFlash(void);
s32 SerialScreen_BtnClearAll(void);
void SerialScreen_GetIOStatusA(void);
void SerialScreen_GetIOStatusB(void);
void SerialScreen_GetIOStatus(int port);
void SerialScreen_QuitDebugIO(void);
void SerialScreen_BtnAcSet(void);
void SerialScreen_BtnDcSetA();
void SerialScreen_BtnDcSetB();
void SerialScreen_BtnDcSet(u8 port);
void SerialScreen_BtnParaSet1(void);
void SerialScreen_BtnParaSet2(void);
void SerialScreen_BtnParaSet3(void);
void SerialScreen_BtnElockSetA();
void SerialScreen_BtnElockSetB();
void SerialScreen_BtnElockSet(u8 port);
void SerialScreen_BtnUnElockA(void);
void SerialScreen_BtnUnElockB(void);
void SerialScreen_BtnFanSetA();
void SerialScreen_BtnFanSetB();
void SerialScreen_BtnFanSet(u8 port);
void SerialScreen_BtnModeLimitMoneySet(void);
void SerialScreen_BtnModeLimitElectSet(void);
void SerialScreen_BtnModeChargeFullSet(void);
void SerialScreen_BtnModeLimitTimingSet(void);
void SerialScreen_BtnModeLimitReservationSet(void);
void SerialScreen_ScramIsSupportSet(void);
void SerialScreen_ScramNegIsSupportSet(void);
void SerialScreen_GateIsSupportSet(void);
void SerialScreen_GateNegIsSupportSet(void);
void SerialScreen_DcIsSupportSet(void);
void SerialScreen_DcNegIsSupportSet(void);
void SerialScreen_AcIsSupportSet(void);
void SerialScreen_AcNegIsSupportSet(void);
void SerialScreen_FanIsSupportSet(void);
void SerialScreen_FanNegIsSupportSet(void);
void SerialScreen_ElockIsSupportSet(void);
void SerialScreen_TempProIsSupportSet(void);
void SerialScreen_ElockNegIsSupportSet(void);

void SerialScreen_ProLightIsSupportSet(void);
void SerialScreen_ProLightNegIsSupportSet(void);

void SerialScreen_GunSiteIsSupportSet(void);
void SerialScreen_GunSiteNegIsSupportSet(void);

void SerialScreen_BreakerIsSupportSet(void);
void SerialScreen_BreakerNegIsSupportSet(void);

void SerialScreen_FloodIsSupportSet(void);
void SerialScreen_FloodNegIsSupportSet(void);

void SerialScreen_SmokeIsSupportSet(void);
void SerialScreen_SmokeNegIsSupportSet(void);

void SerialScreen_PourIsSupportSet(void);
void SerialScreen_PourNegIsSupportSet(void);

void SerialScreen_LiquidIsSupportSet(void);
void SerialScreen_LiquidNegIsSupportSet(void);

void SerialScreen_FuseIsSupportSet(void);
void SerialScreen_FuseNegIsSupportSet(void);

void SerialScreen_GetModeInfoA(void);
void SerialScreen_GetModeInfoB(void);
void SerialScreen_BtnSelfCheckSet(void);

void SerialScreen_ScreenRequest_Time(void);
s8 SerialScreen_Get_ScreenTimeSync_Flag(void);
s8 SerialScreen_Get_SetPowerPercent_Flag(void);
s8 SerialScreen_Get_SetELossProportion_Flag(void);
void SerialScreen_ScreenGet_TimeSync(u16* buf, u8 len);
void SerialScreen_ScreenSet_Reboot_Flag(void);
u8 SerialScreen_ScreenGet_Reboot_Flag(void);
void SerialScreen_ScreenClear_Reboot_Flag(void);
u8 SerialScreen_ScreenGet_Is_FirstPage(void);
void SerialScreen_ScreenSet_CouDownFin_Flag(u8 sta);
u8 SerialScreen_Screen_IsCouDownFin_Flag(u8 port);
s32 SerialScreen_Trigger_ConfigExecute(u8 port, u8 cfg_page, void *data, void *sub_data, void *sub_sub_data);
s32 SerialScreen_ScreenSet_Trigger_Event(u8 Event, u16 DurationTime, u8 JustNotice, u8 port);
enum thaisen_mode SerialScreen_Screen_GetCurrentMode(u8 port);
u32 SerialScreen_Screen_GetModeParameter(u8 port);
u8 SerialScreen_Screen_IsSetReservationMode(u8 port);
void SerialScreen_Screen_ClearReservationModeFlag(u8 port);
/********************************输出信息*******************************************/
void SerialScreen_AcIsSupportOutSet(void);
void SerialScreen_ElockIsSupportOutSet(void);
void SerialScreen_FanIsSupportOutSet(void);

void SerialScreen_V2GIsSupportSet(void);

void SerialScreen_OfflineBillingSet(void);
void SerialScreen_OfflineBillingGet(void);

u8 SerialScreen_GetChargeWay(void);
void SerialScreen_SetChargeWay(u8 way);

extern struct SerialScreenObj SerialScreen;		

extern struct LCD_DATA_FIFO_TYPE * serialScreen_ObjectAi_Init(void);
extern u16 serialScreen_ObjectAi_main(void);

#endif
