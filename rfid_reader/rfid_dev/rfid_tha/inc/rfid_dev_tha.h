/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#ifndef RFID_READER_RFID_DEV_RFID_THA_INC_RFID_DEV_THA_H_
#define RFID_READER_RFID_DEV_RFID_THA_INC_RFID_DEV_THA_H_

#include "rfid_dev_config.h"

#ifdef RFID_DEV_INCLUDE_THA

/** 读卡器指令 */
enum{
    RFID_THA_CMD_CARD_REQUEST = 0x41,
    RFID_THA_CMD_CARD_KEY_AUTHEN = 0x46,
    RFID_THA_CMD_CARD_READ_INFO = 0x47,
    RFID_THA_CMD_CARD_WRITE_INFO = 0x48,
    RFID_THA_CMD_CARD_ACTIVE = 0x4D,
    RFID_THA_CMD_CARD_AUTO_DETECT = 0x4E,
    RFID_THA_CMD_CARD_BUZZER = 0x5A,
};
/** 读卡器指令类型 */
enum{
    RFID_THA_DEVICE_CONTROL_CLASS = 0x01,
    RFID_THA_MIFARE_S50_S70_CLASS,
    RFID_THA_ISO7816_3_CLASS,
    RFID_THA_ISO14443_PICC_CLASS,
    RFID_THA_PLUS_CPU_CLASS,
    RFID_THA_ISO15693_VICC_CLASS,
    RFID_THA_ISO18000_6C_CLASS,
    RFID_THA_ISO18092_NFCIP_1_CLASS,
    RFID_THA_SGR_ID_CARD,
};

/*****************************************************************************
 *  函数名   rfid_tha_buzzer
 *  功能       蜂鸣器控制
 *  参数       count     蜂鸣器响的次数
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_buzzer(unsigned char count);

/*****************************************************************************
 *  函数名   rfid_tha_active_card
 *  功能       寻卡(卡激活)
 *  参数       uuid     用于保存接收到的UUID缓存
 *        ulen     缓存长度
 *        olen     用来保存UUID实际长度
 * 返回        >0：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_tha_active_card(unsigned char *uuid, unsigned char ulen, unsigned char *olen);

/*****************************************************************************
 *  函数名   rfid_tha_key_authentication
 *  功能       对卡进行密钥鉴权
 *  参数       key     密钥
 *     klen    密钥长度
 *     sector   验证的扇区
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_key_authentication(unsigned char sector, unsigned char *key, unsigned char klen);

/*****************************************************************************
 *  函数名   rfid_tha_read_block_info
 *  功能       读取指定块信息
 *  参数       buf     信息缓存
 *     blen    缓存长度
 *     block   读取的块号
 *     is_nest 是嵌套使用
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_read_block_info(unsigned char block, unsigned char *buf, unsigned char blen, unsigned char is_nest);

/*****************************************************************************
 *  函数名   rfid_tha_write_block_info
 *  功能       修改指定块信息
 *  参数       data    数据
 *     dlen    数据长度
 *     block   块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_write_block_info(unsigned char block, unsigned char *data, unsigned char dlen);

#endif /* RFID_DEV_INCLUDE_THA */

#endif /* RFID_READER_RFID_DEV_RFID_THA_INC_RFID_DEV_THA_H_ */
