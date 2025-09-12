/**
 ******************************************************************************
 * @file app_card.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef APP_CARD_H_
#define APP_CARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_rfid_reader.h"
#include <stdio.h>

#define APP_CARD_EVENT_QUERY_BILL                    (0x01 <<0x00)    /* 卡事件：查询是否有未结算订单 */
#define APP_CARD_EVENT_QUERY_BALLANCE                (0x01 <<0x01)    /* 卡事件：查询卡内余额 */
#define APP_CARD_EVENT_CHARGEPILE_READY              (0x01 <<0x02)    /* 卡事件：桩处于可充电状态 */
#define APP_CARD_EVENT_CHARGE_START                  (0x01 <<0x03)    /* 卡事件：充电开始 */
#define APP_CARD_EVENT_CHARGE_STOP                   (0x01 <<0x04)    /* 卡事件：充电结束(使用内存中的订单信息结算) */
#define APP_CARD_EVENT_IS_PAYING                     (0x01 <<0x05)    /* 卡事件：卡正在结算(业务不能再计算电量、金额) */
#define APP_CARD_EVENT_PAY_COMPLETE                  (0x01 <<0x06)    /* 卡事件：卡结算完成 */
#define APP_CARD_EVENT_IS_NOT_SAME_PORT              (0x01 <<0x07)    /* 卡事件：结算枪号不是当前枪号 */

enum{
    APP_CARD_OPERATE_RET_START_AFTER_OBR = 0x01,                              /** 卡操作结果：离线计费模式下预约后刷卡启动(OBR:offline billing reservation) */
    APP_CARD_OPERATE_RET_SUCCESS = 0x00,                                      /** 卡操作结果：成功 */
    APP_CARD_OPERATE_RET_INTERNAL_ERROR = -0x01,                              /** 卡操作结果：系统内部错误 */
    APP_CARD_OPERATE_RET_NO_BALLANCE = -0x02,                                 /** 卡操作结果：余额不足 */
    APP_CARD_OPERATE_RET_READ_ERROR = -0x03,                                  /** 卡操作结果：读取数据失败 */
    APP_CARD_OPERATE_RET_STORAGE_ERROR = -0x04,                               /** 卡操作结果：保存数据失败 */
    APP_CARD_OPERATE_RET_HISTORY_BILL_ERROR = -0x05,                          /** 卡操作结果：未查询到历史订单 */
    APP_CARD_OPERATE_RET_IS_LOCKED = -0x06,                                   /** 卡操作结果：卡被锁 */
    APP_CARD_OPERATE_RET_INVALID_CARD = -0x07,                                /** 卡操作结果：无效卡 */
    APP_CARD_OPERATE_RET_PAYED = -0x08,                                       /** 卡操作结果：已结算 */
    APP_CARD_OPERATE_RET_NOT_START_CARD = -0x09,                              /** 卡操作结果：非启动卡(刷卡停使用的卡必须是启动的卡) */
    APP_CARD_OPERATE_RET_IS_CHARGING = -0x0A,                                 /** 卡操作结果：正在充电 */
    APP_CARD_OPERATE_RET_NULL = -0x0B,                                        /** 卡操作结果：不做任何操作 */
};

/*****************************************************************************
 *  函数名   app_card_event_send
 *  功能       卡事件发送
 *  参数       event      事件
 *     set        用于保存当前事件集
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_event_send(uint32_t event, uint8_t gunno, uint32_t *set);

/*****************************************************************************
 *  函数名   app_card_event_recv
 *  功能       卡事件接收
 *  参数       event        事件
 *     timeout      事件等待时长
 *     set          用于保存当前事件集
 *     is_clear     事件接收完是否清除事件
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_event_recv(uint32_t event, uint32_t timeout, uint8_t gunno, uint32_t *set, uint8_t is_clear);

/*****************************************************************************
 *  函数名   app_card_ipc_init
 *  功能       卡IPC初始化
 *  参数
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_ipc_init(void);

/*****************************************************************************
 *  函数名   app_card_query_ballance
 *  功能       查询卡内余额
 *  参数      gunno    枪号
 * 返回       卡内余额
 ****************************************************************************/
uint32_t app_card_query_ballance(uint8_t gunno);

/*****************************************************************************
 *  函数名   app_card_query_operate_ret
 *  功能       查询卡信息处理结果
 *  参数      gunno    枪号
 * 返回       卡信息处理结果
 ****************************************************************************/
int8_t app_card_query_operate_ret(uint8_t gunno);

#ifdef RFIDR_USING_XJ_CARD
/*****************************************************************************
 *  函数名   app_query_xj_crc_serial_number
 *  功能       查询小桔卡计算用CRC序列号
 *  参数
 *  返回       计算用CRC序列号
 ****************************************************************************/
uint8_t *app_query_xj_crc_serial_number(void);

/*****************************************************************************
 *  函数名   app_query_xj_crc
 *  功能       查询小桔卡CRC值
 *  参数
 * 返回       CRC值
 ****************************************************************************/
uint8_t *app_query_xj_crc(void);

/*****************************************************************************
 *  函数名   app_query_xj_random_number_1
 *  功能       查询小桔卡随机数1
 *  参数
 *  返回       随机数1
 ****************************************************************************/
uint8_t *app_query_xj_random_number_1(void);

/*****************************************************************************
 *  函数名   app_query_xj_random_number_2
 *  功能       查询小桔卡随机数2
 *  参数
 *  返回       随机数2
 ****************************************************************************/
uint8_t *app_query_xj_random_number_2(void);

/*****************************************************************************
 *  函数名   app_query_xj_random_number_3
 *  功能       查询小桔卡随机数3
 *  参数
 *  返回       随机数3
 ****************************************************************************/
uint8_t *app_query_xj_random_number_3(void);

#endif /* #ifdef RFIDR_USING_XJ_CARD */
/*****************************************************************************
 *  函数名   app_card_init
 *  功能       卡部分初始化
 *  参数
 * 返回       >=0：成功   <0：失败
 ****************************************************************************/
int32_t app_card_init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CARD_H_ */
