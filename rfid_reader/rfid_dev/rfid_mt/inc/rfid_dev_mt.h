/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-20     我的杨yang       the first version
 */
#ifndef RFID_READER_RFID_DEV_RFID_MT_INC_RFID_DEV_MT_H_
#define RFID_READER_RFID_DEV_RFID_MT_INC_RFID_DEV_MT_H_

#include "rfid_dev_config.h"

#ifdef RFID_DEV_INCLUDE_MT

/** 读卡器指令 */
enum{
    RFID_MT_CMD_QUERY_VERSION = 0x31,              /** 指令：查询版本 */
    RFID_MT_CMD_SEARCH_CARD = 0x34,                /** 指令：寻卡 */
    RFID_MT_CMD_QUERY_UUID = 0x34,                 /** 指令：查询卡uuid */
    RFID_MT_CMD_A_KEY_AUTHENTICATION = 0x34,       /** 指令：A密钥认证 */
    RFID_MT_CMD_B_KEY_AUTHENTICATION = 0x34,       /** 指令：B密钥认证 */
    RFID_MT_CMD_READ_BLOCK_INFO = 0x34,            /** 指令：读块信息 */
    RFID_MT_CMD_WRITE_BLOCK_INFO = 0x34,           /** 指令：写块信息 */
    RFID_MT_CMD_BUZZER = 0x31,                     /** 指令：蜂鸣器 */
};
/** 读卡器指令参数 */
enum{
    RFID_MT_CMD_PARA_QUERY_VERSION = 0x40,         /** 指令参数：查询版本 */
    RFID_MT_CMD_PARA_SEARCH_CARD = 0x30,           /** 指令参数：寻卡 */
    RFID_MT_CMD_PARA_QUERY_UUID = 0x31,            /** 指令参数：查询卡uuid */
    RFID_MT_CMD_PARA_A_KEY_AUTHENTICATION = 0x32,  /** 指令参数：A密钥认证 */
    RFID_MT_CMD_PARA_B_KEY_AUTHENTICATION = 0x39,  /** 指令参数：B密钥认证 */
    RFID_MT_CMD_PARA_READ_BLOCK_INFO = 0x33,       /** 指令参数：读块信息 */
    RFID_MT_CMD_PARA_WRITE_BLOCK_INFO = 0x34,      /** 指令参数：写块信息 */
    RFID_MT_CMD_PARA_BUZZER = 0x3E,                /** 指令参数：蜂鸣器 */
};

/*****************************************************************************
 *  函数名   rfid_mt_device_identify
 *  功能       判断设备是否支持此协议
 *  参数
 * 返回        1：支持   0：不支持     <0：读卡器未回复
 ****************************************************************************/
int rfid_mt_device_identify(void);

/*****************************************************************************
 *  函数名   rfid_mt_query_version
 *  功能       查询版本
 *  参数       buf     存放版本缓存
 *     blen     缓存长度
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_mt_query_version(unsigned char *buf, unsigned char blen);

/*****************************************************************************
 *  函数名   rfid_mt_search_card
 *  功能       寻卡
 *  参数
 * 返回        >0：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_search_card(void);

/*****************************************************************************
 *  函数名   rfid_mt_read_card_uuid
 *  功能       读取卡的uuid
 *  参数       uuid     用于保存接收到的UUID缓存
 *     ulen     缓存长度
 *     olen     用来保存UUID实际长度
 * 返回        >0：获取到uuid   0：未获取到uuid   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_read_card_uuid(unsigned char *uuid, unsigned char ulen, unsigned char *olen);

/***************************************************************************************
 *  函数名   rfid_mt_active_card
 *  功能       寻卡(卡激活)
 *  参数       uuid     用于保存接收到的UUID缓存
 *     ulen     缓存长度
 *     olen     用来保存UUID实际长度
 *     is_search_card  是否只是寻卡
 * 返回        3：获取到UUID(已寻到卡) 2：未获取到UUID(已寻到卡) 1：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 **************************************************************************************/
int rfid_mt_active_card(unsigned char is_search_card, unsigned char *uuid, unsigned char ulen, unsigned char *olen);

/*****************************************************************************
 *  函数名   rfid_mt_key_authentication
 *  功能       对卡进行密钥鉴权
 *  参数       key     密钥
 *     key_type 密钥类型 0:A密钥，1：B密钥
 *     klen    密钥长度
 *     sector   验证的扇区
 * 返回        >0：成功   =0：失败     <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_key_authentication(unsigned char key_type, unsigned char sector, unsigned char *key, unsigned char klen);

/*****************************************************************************
 *  函数名   rfid_mt_read_block_info
 *  功能       读取指定块信息
 *  参数       buf     信息缓存
 *     blen    缓存长度
 *     sector  块所属扇区
 *     block   读取的块号
 *     is_nest 是嵌套使用
 * 返回        >0：成功   =0：失败     <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_read_block_info(unsigned char sector, unsigned char block, unsigned char *buf, unsigned char blen, unsigned char is_nest);

/*****************************************************************************
 *  函数名   rfid_mt_write_block_info
 *  功能       修改指定块信息
 *  参数       data    数据
 *     dlen    数据长度
 *     sector  块所属扇区
 *     block   块号
 * 返回        >0：成功   =0：失败     <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_write_block_info(unsigned char sector, unsigned char block, unsigned char *data, unsigned char dlen);

/*****************************************************************************
 *  函数名   rfid_mt_buzzer
 *  功能       蜂鸣器控制
 *  参数       count     蜂鸣器响的次数
 *     interval  鸣叫间隔
 * 返回        >0：成功   =0：失败     <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_buzzer(unsigned char count, unsigned short interval);

#endif /* RFID_DEV_INCLUDE_MT */

#endif /* RFID_READER_RFID_DEV_RFID_MT_INC_RFID_DEV_MT_H_ */
