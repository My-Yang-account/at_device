/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#ifndef RFID_READER_RFID_DEV_INC_RFID_DEV_API_H_
#define RFID_READER_RFID_DEV_INC_RFID_DEV_API_H_

#include "rfid_dev_hardware.h"

enum rfid_dev_type{
    RFID_DEV_TYPE_THA = 0x01,
    RFID_DEV_TYPE_ZLG = 0x02,
    RFID_DEV_TYPE_SIZE = 0x04,
};

enum rfid_dev_ctrl{
    RFID_DEV_CTRL_CMD_RESET,
    RFID_DEV_CTRL_CMD_BAUDRATE,
    RFID_DEV_CTRL_CMD_SIZE,
};

/**************************************************
 *  函数名   rfid_set_dev_type
 *  功能       设置射频识别设备类型
 *  参数       type          射频识别设备类型
 *     is_append     这是增加射频识别设备
 *  返回
 *************************************************/
void rfid_set_dev_type(uint8_t type, uint8_t is_append);

/**************************************************
 *  函数名   rfid_clear_dev_type
 *  功能       清除射频识别设备类型
 *  参数       type     射频识别设备类型
 *  返回
 *************************************************/
void rfid_clear_dev_type(uint8_t type);

/**************************************************
 *  函数名   rfid_query_dev_type
 *  功能       获取射频识别设备类型
 *  参数
 *  返回      射频识别设备类型
 *************************************************/
enum rfid_dev_type rfid_query_dev_type(void);

/**************************************************
 *  函数名   rfid_dev_api_dev_control
 *  功能       射频识别设备控制
 *  参数       cmd         控制命令
 *     para        控制命令参数
 *     para_len    控制命令参数长度
 *     ret         控制命令返回参数
 *     ret_len     控制命令返回参数长度
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int32_t rfid_dev_api_dev_control(uint8_t cmd, void *para, uint16_t para_len, void *ret, uint16_t ret_len);

/*****************************************************************************
 *  函数名   rfid_dev_api_buzzer
 *  功能       蜂鸣器控制
 *  参数       count     蜂鸣器响的次数
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_buzzer(unsigned char count);

/*****************************************************************************
 *  函数名   rfid_dev_api_active_card
 *  功能       寻卡(卡激活)
 *  参数       uuid     用于保存接收到的UUID缓存
 *        ulen     缓存长度
 *        olen     用来保存UUID实际长度
 * 返回        >0：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_dev_api_active_card(unsigned char *uuid, unsigned char ulen, unsigned char *olen);

/*****************************************************************************
 *  函数名   rfid_dev_api_key_authentication
 *  功能       对卡进行密钥鉴权
 *  参数       key     密钥
 *     klen    密钥长度
 *     block   验证的块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_key_authentication(unsigned char block, unsigned char *key, unsigned char klen);

/*****************************************************************************
 *  函数名   rfid_dev_api_read_block_info
 *  功能       读取指定块信息
 *  参数       buf     信息缓存
 *     blen    缓存长度
 *     block   验证的块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_read_block_info(unsigned char block, unsigned char *buf, unsigned char blen);

/*****************************************************************************
 *  函数名   rfid_dev_api_write_block_info
 *  功能       修改指定块信息
 *  参数       data    数据
 *     dlen    数据长度
 *     block   块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_write_block_info(unsigned char block, unsigned char *data, unsigned char dlen);

/**************************************************
 *  函数名   rfid_dev_api_init
 *  功能       初始化射频识别设备
 *  参数
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int32_t rfid_dev_api_init(void);

#endif /* RFID_READER_RFID_DEV_INC_RFID_DEV_API_H_ */
