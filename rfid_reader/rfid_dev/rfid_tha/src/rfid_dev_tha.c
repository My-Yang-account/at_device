/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-03     我的杨yang       the first version
 */
#include "rfid_dev_tha.h"
#include "rfid_dev_hardware.h"

#define DBG_TAG "rfidtha"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//#define RFIDR_THA_DEBUG

#ifdef RFID_DEV_INCLUDE_THA

#define RFID_THA_FRAME_STATE_REGION               0x04    /* 帧响应状态域 */
#define RFID_THA_FRAME_UUID_REGION                0x0C    /* 帧UUID域 */
#define RFID_THA_FRAME_UID_LEN_REGION             0x0B    /* 帧UUID长度域 */
#define RFID_THA_FRAME_LENGTH_REGION              0x08    /* 帧数据长度域 */
#define RFID_THA_DEVICE_RESPONSE_ADDRESS          0xB3    /* 射频识别设备响应地址 */
#define RFID_THA_DEVICE_REQUEST_ADDRESS           0xB2    /* 射频识别设备请求地址 */

#define RFID_THA_KEY_TYPE_A                       0x60    /* 密钥A */
#define RFID_THA_KEY_TYPE_B                       0x61    /* 密钥B */

#define RFID_THA_FRAME_FIX_LEN                    0x0A    /* 帧的固定长度(0x0A) */
#define RFID_THA_BLOCK_SIZE                       0x10    /* 一个块的大小(B) */
#define RFID_THA_CARD_UUID_LEN                    0x08    /* 卡的UUID长度(B) */
#define RFID_THA_WAIT_LOCK_TIME_MAX               10000   /* 等待操作锁最大时长 */
#define RFID_THA_WAIT_SEM_TIME_MAX                80      /* 等待数据信号量最大时长(从发送寻卡指令到接收到回复大概40-60ms) */
#define RFID_THA_WAIT_LOCK_TIME_MAX               10000   /* 等大操作锁最大时长(ms) */
#define RFID_THA_REQUEST_BUFF_SIZE                0x20    /* 数据请求缓存大小(B) */
#define RFID_THA_RESPONSE_BUFF_SIZE               0x40    /* 数据响应缓存大小(B) */

#pragma pack(1)
struct tha_card_info{
    unsigned short count;
    unsigned char data[RFID_THA_RESPONSE_BUFF_SIZE];

    unsigned char card_uid_len;
    unsigned char card_uid[RFID_THA_CARD_UUID_LEN];
};
#pragma pack()

static unsigned char s_rfid_tha_lock = 0x00;
static unsigned char s_rfid_tha_card_active_cmd[0x02] = {0x00, 0x26};
static unsigned char s_rfid_tha_request[RFID_THA_REQUEST_BUFF_SIZE];
static struct tha_card_info s_tha_card_info;

/*****************************************************************************
 *  函数名   rfid_tha_check_sum
 *  功能       计算数据校验和
 *  参数       data         数据
 *     dlen         数据长度(B)
 * 返回        校验和
 ****************************************************************************/
static unsigned int rfid_tha_check_sum(unsigned char *data, unsigned int dlen)
{
    unsigned int sum = 0x00;

    if(data == NULL){
        return sum;
    }

    for(unsigned int count = 0x00; count < dlen; count++){
        sum += data[count];
    }

    return sum;
}

/*****************************************************************************
 *  函数名   rfid_tha_clear_data
 *  功能       清除串口数据及信号量
 *  参数
 * 返回
 ****************************************************************************/
static void rfid_tha_clear_data(void)
{
    unsigned char ch;

    while(rfid_dev_recv(&ch, 0x01) == 0x01);
    while(take_rfid_dev_data_sem(0x00) >= 0x00);
}

/*****************************************************************************
 *  函数名   s_rfid_tha_padding_request
 *  功能       填充射频识别请求帧
 *  参数       cmd          指令码
 *     data         请求数据
 *     dlen         请求数据长度(B)
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
static int s_rfid_tha_padding_request(unsigned char cmd, unsigned char *data, unsigned short dlen)
{
    if((data == NULL) || (dlen == 0x00)){
        return -0x01;
    }
    if((dlen + RFID_THA_FRAME_FIX_LEN) > RFID_THA_REQUEST_BUFF_SIZE){
        return -0x01;
    }

    unsigned short count, check;

    memset(s_rfid_tha_request, 0x00, RFID_THA_REQUEST_BUFF_SIZE);

    s_rfid_tha_request[0x00] = RFID_THA_DEVICE_REQUEST_ADDRESS;
    s_rfid_tha_request[0x01] = 0x00;
    s_rfid_tha_request[0x02] = 0x00;
    s_rfid_tha_request[0x03] = RFID_THA_MIFARE_S50_S70_CLASS;
    s_rfid_tha_request[0x04] = cmd;
    s_rfid_tha_request[0x05] = cmd >>0x08;
    s_rfid_tha_request[0x06] = dlen;
    s_rfid_tha_request[0x07] = dlen >>0x08;

    for(count = 0x00; count < dlen; count++){
        s_rfid_tha_request[0x08 + count] = data[count];
    }
    check = rfid_tha_check_sum(s_rfid_tha_request, (dlen + RFID_THA_FRAME_FIX_LEN - 0x02));
    check = ~check;

    s_rfid_tha_request[dlen + RFID_THA_FRAME_FIX_LEN - 0x02] = check;
    s_rfid_tha_request[dlen + RFID_THA_FRAME_FIX_LEN - 0x01] = check >>0x08;

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_tha_recv_char
 *  功能       从接收缓冲区读取一个字节的数据
 *  参数
 * 返回        >=0：数据   <0：失败
 ****************************************************************************/
static signed short rfid_tha_recv_char(void)
{
    unsigned char ch = 0x00;

    while(rfid_dev_recv(&ch, 0x01) != 0x01) {
        if(take_rfid_dev_data_sem(RFID_THA_WAIT_SEM_TIME_MAX) < 0x00){
            return -0x01;
        }
    }

    return ch;
}

/*****************************************************************************
 *  函数名   rfid_tha_readline
 *  功能       从接收缓冲区读取一帧数据
 *  参数
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
static int rfid_tha_readline(void)
{
    unsigned char ch;
    signed short result = 0;
    unsigned short data_len = 0;
    unsigned short frame_check = 0, calculate_check = 0;
    s_tha_card_info.count = 0x00;

    while(1)
    {
        result = rfid_tha_recv_char();
        if(result < 0x00){                /** 射频识别设备未回复 */
            return -1;
        }
        ch = (unsigned char)result;
#ifdef RFIDR_THA_DEBUG
        LOG_D("tha recv|0x%x, %d", ch, s_tha_card_info.count);
#endif /* RFIDR_THA_DEBUG */
        if((ch != RFID_THA_DEVICE_RESPONSE_ADDRESS) && (s_tha_card_info.count == 0x00)){
            continue;
        }
        s_tha_card_info.data[s_tha_card_info.count++] = ch;

        if(s_tha_card_info.count == RFID_THA_FRAME_LENGTH_REGION){
            data_len = s_tha_card_info.data[RFID_THA_FRAME_LENGTH_REGION - 0x01];
            data_len <<=0x08;
            data_len |= s_tha_card_info.data[RFID_THA_FRAME_LENGTH_REGION - 0x02];
            if((data_len + RFID_THA_FRAME_FIX_LEN) > RFID_THA_RESPONSE_BUFF_SIZE){
                LOG_W("RFID tha response buf is too short(%d, %d)", RFID_THA_RESPONSE_BUFF_SIZE, (data_len + RFID_THA_FRAME_FIX_LEN));
                return -0x01;
            }
        }

        if(s_tha_card_info.count >= (RFID_THA_FRAME_LENGTH_REGION + data_len + 0x02)){
            frame_check = s_tha_card_info.data[RFID_THA_FRAME_LENGTH_REGION + data_len + 0x01];
            frame_check <<=0x08;
            frame_check |= s_tha_card_info.data[RFID_THA_FRAME_LENGTH_REGION + data_len];

            calculate_check = rfid_tha_check_sum(s_tha_card_info.data, (RFID_THA_FRAME_LENGTH_REGION + data_len));
            if(frame_check != (unsigned short)(~calculate_check)){
                LOG_E("RFID tha check code error|%x |%x", frame_check, (unsigned short)(~calculate_check));
                s_tha_card_info.count = 0x00;
                continue;
            }
            break;
        }
    }

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_tha_device_identify
 *  功能       判断设备是否支持此协议
 *  参数
 * 返回        1：支持   0：不支持     <0：读卡器未回复
 ****************************************************************************/
int rfid_tha_device_identify(void)
{
    if(rfid_tha_buzzer(0x00) >= 0x00){
        return 0x01;
    }

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_tha_buzzer
 *  功能       蜂鸣器控制
 *  参数       count     蜂鸣器响的次数
 * 返回        >0：成功   =0：失败     <0：读卡器未回复
 ****************************************************************************/
int rfid_tha_buzzer(unsigned char count)
{
    unsigned int tick = rt_tick_get();
    unsigned short status = 0x00;

    while(s_rfid_tha_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_THA_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_tha_lock = 0x01;

    rfid_tha_clear_data();

    if(s_rfid_tha_padding_request(RFID_THA_CMD_CARD_BUZZER, &count, sizeof(count)) < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }
    if(rfid_dev_send(s_rfid_tha_request, (RFID_THA_FRAME_FIX_LEN + sizeof(count))) < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    if(rfid_tha_readline() < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    status = s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION + 0x01];
    status <<=0x08;
    status |= s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION];

    if(status != 0x00){
        s_rfid_tha_lock = 0x00;
        return 0x00;
    }

    s_rfid_tha_lock = 0x00;

    return 0x01;
}

/*****************************************************************************
 *  函数名   rfid_tha_active_card
 *  功能       寻卡(卡激活)
 *  参数       uuid     用于保存接收到的UUID缓存
 *        ulen     缓存长度
 *        olen     用来保存UUID实际长度
 * 返回        >0：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_tha_active_card(unsigned char *uuid, unsigned char ulen, unsigned char *olen)
{
    unsigned int tick = rt_tick_get();
    unsigned short status = 0x00;

    while(s_rfid_tha_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_THA_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_tha_lock = 0x01;

    rfid_tha_clear_data();

    if(s_rfid_tha_padding_request(RFID_THA_CMD_CARD_ACTIVE, s_rfid_tha_card_active_cmd, sizeof(s_rfid_tha_card_active_cmd)) < 0x00){
        s_rfid_tha_lock = 0x00;
        return 0x00;
    }
    if(rfid_dev_send(s_rfid_tha_request, (RFID_THA_FRAME_FIX_LEN + sizeof(s_rfid_tha_card_active_cmd))) < 0x00){
        s_rfid_tha_lock = 0x00;
        return 0x00;
    }

    if(rfid_tha_readline() < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    status = s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION + 0x01];
    status <<=0x08;
    status |= s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION];

    if(status != 0x00){
        s_rfid_tha_lock = 0x00;
        return 0x00;
    }

    s_tha_card_info.card_uid_len = s_tha_card_info.data[RFID_THA_FRAME_UID_LEN_REGION];
    s_tha_card_info.card_uid_len = s_tha_card_info.card_uid_len > RFID_THA_CARD_UUID_LEN ? RFID_THA_CARD_UUID_LEN : s_tha_card_info.card_uid_len;

    memset(s_tha_card_info.card_uid, 0x00, RFID_THA_CARD_UUID_LEN);
    memcpy(s_tha_card_info.card_uid, &s_tha_card_info.data[RFID_THA_FRAME_UUID_REGION], s_tha_card_info.card_uid_len);

    if(uuid){
        memset(uuid, 0x00, ulen);
        if(ulen > s_tha_card_info.card_uid_len){
            memcpy(uuid, s_tha_card_info.card_uid, s_tha_card_info.card_uid_len);
        }else{
            memcpy(uuid, s_tha_card_info.card_uid, ulen);
        }
    }
    if(olen){
        *olen = s_tha_card_info.card_uid_len;
    }

    s_rfid_tha_lock = 0x00;

    return 0x01;
}

/*****************************************************************************
 *  函数名   rfid_tha_key_authentication
 *  功能       对卡进行密钥鉴权
 *  参数       key     密钥
 *     klen    密钥长度
 *     sector   验证的扇区
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_key_authentication(unsigned char sector, unsigned char *key, unsigned char klen)
{
    if(key == NULL){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned short status = 0x00;
    unsigned char card_key_authen[0x0C];   /** 鉴权时帧的数据域为：密钥类型(1字节) + UUID(4字节) + 密钥(6字节) + 卡块号(1字节)，12字节 */

    while(s_rfid_tha_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_THA_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_tha_lock = 0x01;

    card_key_authen[0x00] = RFID_THA_KEY_TYPE_A;
    card_key_authen[0x0B] = sector;

    memset((card_key_authen + 0x01), 0x00, 0x04);
    if(s_tha_card_info.card_uid_len > 0x04){
        memcpy((card_key_authen + 0x01), s_tha_card_info.card_uid, 0x04);
    }else{
        memcpy((card_key_authen + 0x01), s_tha_card_info.card_uid, s_tha_card_info.card_uid_len);
    }

    memset((card_key_authen + 0x01 + 0x04), 0x00, 0x06);
    if(klen > 0x06){
        memcpy((card_key_authen + 0x01 + 0x04), key, 0x06);
    }else{
        memcpy((card_key_authen + 0x01 + 0x04), key, klen);
    }

    rfid_tha_clear_data();

    if(s_rfid_tha_padding_request(RFID_THA_CMD_CARD_KEY_AUTHEN, card_key_authen, sizeof(card_key_authen)) < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }
    if(rfid_dev_send(s_rfid_tha_request, (RFID_THA_FRAME_FIX_LEN + sizeof(card_key_authen))) < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    if(rfid_tha_readline() < 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    status = s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION + 0x01];
    status <<=0x08;
    status |= s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION];

    if(status != 0x00){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    s_rfid_tha_lock = 0x00;

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_tha_read_block_info
 *  功能       读取指定块信息
 *  参数       buf     信息缓存
 *     blen    缓存长度
 *     block   读取的块号
 *     is_nest 是嵌套使用
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_read_block_info(unsigned char block, unsigned char *buf, unsigned char blen, unsigned char is_nest)
{
    if((buf == 0x00) || (blen == 0x00)){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned short status = 0x00;
    unsigned char rentry = 0x00;

    if(is_nest == 0x00){
        while(s_rfid_tha_lock == 0x01){
            if(tick > rt_tick_get()){
                tick = rt_tick_get();
            }
            if((rt_tick_get() - tick) > RFID_THA_WAIT_LOCK_TIME_MAX){
                break;
            }
            rt_thread_mdelay(5);
        }
        s_rfid_tha_lock = 0x01;
    }

    rfid_tha_clear_data();

    while(rentry < 0x05){    /** 如果写失败,最多尝试5次 */
        if(s_rfid_tha_padding_request(RFID_THA_CMD_CARD_READ_INFO, &block, sizeof(block)) < 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }
        if(rfid_dev_send(s_rfid_tha_request, (RFID_THA_FRAME_FIX_LEN + sizeof(block))) < 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        if(rfid_tha_readline() < 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        status = s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION + 0x01];
        status <<=0x08;
        status |= s_tha_card_info.data[RFID_THA_FRAME_STATE_REGION];

        if(status != 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        break;
    }

    if(rentry >= 0x05){
        if(is_nest == 0x00){
            s_rfid_tha_lock = 0x00;
        }
        return -0x01;
    }

    memset(buf, 0x00, blen);
    if(blen < RFID_THA_BLOCK_SIZE){
        memcpy(buf, (s_tha_card_info.data + RFID_THA_FRAME_LENGTH_REGION), blen);
    }else{
        memcpy(buf, (s_tha_card_info.data + RFID_THA_FRAME_LENGTH_REGION), RFID_THA_BLOCK_SIZE);
    }

    if(is_nest == 0x00){
        s_rfid_tha_lock = 0x00;
    }

    return 0x00;
}
/*****************************************************************************
 *  函数名   rfid_tha_write_block_info
 *  功能       修改指定块信息
 *  参数       data    数据
 *     dlen    数据长度
 *     block   块号
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
int rfid_tha_write_block_info(unsigned char block, unsigned char *data, unsigned char dlen)
{
    if((data == 0x00) || (dlen == 0x00)){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned char wbuff[RFID_THA_BLOCK_SIZE + 0x01], rbuff[RFID_THA_BLOCK_SIZE], rentry = 0x00;

    while(s_rfid_tha_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_THA_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_tha_lock = 0x01;

    if(dlen < RFID_THA_BLOCK_SIZE){
        if(rfid_tha_read_block_info(block, (wbuff + 0x01), RFID_THA_BLOCK_SIZE, 0x01) < 0x00){
            s_rfid_tha_lock = 0x00;
            return -0x01;
        }
        memcpy((wbuff + 0x01), data, dlen);
    }else{
        memcpy((wbuff + 0x01), data, RFID_THA_BLOCK_SIZE);
    }

    wbuff[0x00] = block;     /** 要写的块号 */

    rfid_tha_clear_data();

    while(rentry < 0x05){    /** 如果写失败,最多尝试5次 */
        if(s_rfid_tha_padding_request(RFID_THA_CMD_CARD_WRITE_INFO, wbuff, (RFID_THA_BLOCK_SIZE + 0x01)) < 0x00){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("tha reader not response when write block info(%d) entry(%d)", block, rentry);
            continue;
        }
        if(rfid_dev_send(s_rfid_tha_request, (RFID_THA_FRAME_FIX_LEN + (RFID_THA_BLOCK_SIZE + 0x01))) < 0x00){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("tha reader not response when write block info(%d) entry(%d)", block, rentry);
            continue;
        }

        if(rfid_tha_readline() < 0x00){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("tha reader not response when write block info(%d) entry(%d)", block, rentry);
            continue;
        }

        rfid_tha_read_block_info(block, rbuff, RFID_THA_BLOCK_SIZE, 0x01);                      /** 读数据 */

        if(memcmp((wbuff + 0x01), rbuff, RFID_THA_BLOCK_SIZE)){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("tha reader write block(%d) info check fail entry(%d)", block, rentry);
            continue;
        }

        break;
    }

    if(rentry >= 0x05){
        s_rfid_tha_lock = 0x00;
        return -0x01;
    }

    s_rfid_tha_lock = 0x00;

    return 0x00;
}

#endif /* RFID_DEV_INCLUDE_THA */

