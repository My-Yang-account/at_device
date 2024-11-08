/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#include "rfid_dev_api.h"
#include "rfid_dev_tha.h"

static enum rfid_dev_type s_rfid_dev_type = RFID_DEV_TYPE_THA;

/**************************************************
 *  函数名   rfid_set_dev_type
 *  功能       设置射频识别设备类型
 *  参数       type          射频识别设备类型
 *     is_append     这是增加射频识别设备
 *  返回
 *************************************************/
void rfid_set_dev_type(uint8_t type, uint8_t is_append)
{
    if(is_append){
        s_rfid_dev_type |= type;
    }else{
        s_rfid_dev_type = type;
    }
}
/**************************************************
 *  函数名   rfid_clear_dev_type
 *  功能       清除射频识别设备类型
 *  参数       type     射频识别设备类型
 *  返回
 *************************************************/
void rfid_clear_dev_type(uint8_t type)
{
    s_rfid_dev_type &= (~type);
}
/**************************************************
 *  函数名   rfid_query_dev_type
 *  功能       获取射频识别设备类型
 *  参数
 *  返回      射频识别设备类型
 *************************************************/
enum rfid_dev_type rfid_query_dev_type(void)
{
    return s_rfid_dev_type;
}

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
int32_t rfid_dev_api_dev_control(uint8_t cmd, void *para, uint16_t para_len, void *ret, uint16_t ret_len)
{
    switch(cmd){
    case RFID_DEV_CTRL_CMD_RESET:
        return rfid_dev_hardware_ctrl(RFID_DEV_CTRL_HARDRESET, para, para_len);
        break;
    case RFID_DEV_CTRL_CMD_BAUDRATE:
        return rfid_dev_hardware_ctrl(RFID_DEV_CTRL_BAUDRATE, para, para_len);
        break;
    default:
        break;
    }

    return -0x01;
}

/*****************************************************************************
 *  函数名   rfid_dev_api_buzzer
 *  功能       蜂鸣器控制
 *  参数       count     蜂鸣器响的次数
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_buzzer(unsigned char count)
{
    if(s_rfid_dev_type &RFID_DEV_TYPE_THA){
#ifdef RFID_DEV_INCLUDE_THA
        return rfid_tha_buzzer(count);
#endif /* RFID_DEV_INCLUDE_THA */
    }
    if(s_rfid_dev_type &RFID_DEV_TYPE_ZLG){
#ifdef RFID_DEV_INCLUDE_ZLG
        return rfid_zlg_buzzer(count);
#endif /* RFID_DEV_INCLUDE_ZLG */
    }

    return -0x01;
}

/*****************************************************************************
 *  函数名   rfid_dev_api_active_card
 *  功能       寻卡(卡激活)
 *  参数       uuid     用于保存接收到的UUID缓存
 *       ulen     缓存长度
 *       olen     用来保存UUID实际长度
 * 返回        >0：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_dev_api_active_card(unsigned char *uuid, unsigned char ulen, unsigned char *olen)
{
    if(s_rfid_dev_type &RFID_DEV_TYPE_THA){
#ifdef RFID_DEV_INCLUDE_THA
        return rfid_tha_active_card(uuid, ulen, olen);
#endif /* RFID_DEV_INCLUDE_THA */
    }
    if(s_rfid_dev_type &RFID_DEV_TYPE_ZLG){
#ifdef RFID_DEV_INCLUDE_ZLG
        return rfid_zlg_active_card(uuid, ulen, olen);
#endif /* RFID_DEV_INCLUDE_ZLG */
    }

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_dev_api_key_authentication
 *  功能       对卡进行密钥鉴权
 *  参数       key     密钥
 *     klen    密钥长度
 *     block   验证的块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_key_authentication(unsigned char block, unsigned char *key, unsigned char klen)
{
    if(s_rfid_dev_type &RFID_DEV_TYPE_THA){
#ifdef RFID_DEV_INCLUDE_THA
        return rfid_tha_key_authentication(block, key, klen);
#endif /* RFID_DEV_INCLUDE_THA */
    }
    if(s_rfid_dev_type &RFID_DEV_TYPE_ZLG){
#ifdef RFID_DEV_INCLUDE_ZLG
        return rfid_zlg_key_authentication(block, key, klen);
#endif /* RFID_DEV_INCLUDE_ZLG */
    }

    return -0x01;
}

/*****************************************************************************
 *  函数名   rfid_dev_api_read_block_info
 *  功能       读取指定块信息
 *  参数       buf     信息缓存
 *     blen    缓存长度
 *     block   验证的块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_read_block_info(unsigned char block, unsigned char *buf, unsigned char blen)
{
    if(s_rfid_dev_type &RFID_DEV_TYPE_THA){
#ifdef RFID_DEV_INCLUDE_THA
        return rfid_tha_read_block_info(block, buf, blen, 0x00);
#endif /* RFID_DEV_INCLUDE_THA */
    }
    if(s_rfid_dev_type &RFID_DEV_TYPE_ZLG){
#ifdef RFID_DEV_INCLUDE_ZLG
        return rfid_zlg_read_block_info(block, buf, blen);
#endif /* RFID_DEV_INCLUDE_ZLG */
    }

    return -0x01;
}
/*****************************************************************************
 *  函数名   rfid_dev_api_write_block_info
 *  功能       修改指定块信息
 *  参数       data    数据
 *     dlen    数据长度
 *     block   块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_dev_api_write_block_info(unsigned char block, unsigned char *data, unsigned char dlen)
{
    if(s_rfid_dev_type &RFID_DEV_TYPE_THA){
#ifdef RFID_DEV_INCLUDE_THA
        return rfid_tha_write_block_info(block, data, dlen);
#endif /* RFID_DEV_INCLUDE_THA */
    }
    if(s_rfid_dev_type &RFID_DEV_TYPE_ZLG){
#ifdef RFID_DEV_INCLUDE_ZLG
        return rfid_zlg_write_block_info(block, data, dlen);
#endif /* RFID_DEV_INCLUDE_ZLG */
    }

    return -0x01;
}

/**************************************************
 *  函数名   rfid_dev_api_init
 *  功能       初始化射频识别设备
 *  参数
 *  返回      > =0 : 成功，< 0 ：失败
 *************************************************/
int32_t rfid_dev_api_init(void)
{
    /** 射频识别设备硬件初始化 */
    rfid_dev_hardware_init();
    /** 钛昕射频识别设备信息初始化 */

    /** 周立功射频识别设备信息初始化 */

    return 0x00;
}
