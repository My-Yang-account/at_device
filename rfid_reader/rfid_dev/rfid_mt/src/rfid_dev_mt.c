/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-20     我的杨yang       the first version
 */
#include "rfid_dev_mt.h"
#include "rfid_dev_hardware.h"

#define DBG_TAG "rfidmt"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//#define RFIDR_MT_DEBUG

#ifdef RFID_DEV_INCLUDE_MT

#define RFID_MT_BLOCK_SIZE                        0x10    /* 一个块的大小(B) */
#define RFID_MT_CARD_UUID_LEN                     0x08    /* 卡的UUID长度(B) */

#define RFID_MT_FRAME_START_CODE                  0x02    /* 帧起始码 */
#define RFID_MT_FRAME_END_CODE                    0x03    /* 帧结束码 */

#define RFID_MT_FRAME_LENGTH_REGION               0x02    /* 帧数据长度域 */

#define RFID_MT_RES_CMD_REGION                    0x03    /* 响应帧指令域 */
#define RFID_MT_RES_CMD_PARA_REGION               0x04    /* 响应帧指令参数域 */
#define RFID_MT_RES_STATUS_REGION                 0x05    /* 响应帧状态域 */

#define RFID_MT_WAIT_SEM_TIME_MAX                 80      /* 等待数据信号量最大时长(待测试) */
#define RFID_MT_WAIT_LOCK_TIME_MAX                10000   /* 等大操作锁最大时长(ms) */

#define RFID_MT_FRAME_FIX_LEN                     0x05    /* 帧的固定长度 */
#define RFID_MT_REQ_DATA_REGION_FIX_LENGTH        0x02    /* 请求帧数据域固定长度 */
#define RFID_MT_RES_DATA_REGION_FIX_LENGTH        0x03    /* 响应帧数据域固定长度 */
#define RFID_MT_REQUEST_BUFF_SIZE                 0x20    /* 数据请求缓存大小(B) */
#define RFID_MT_RESPONSE_BUFF_SIZE                0x40    /* 数据响应缓存大小(B) */


#pragma pack(1)
struct mt_card_info{
    unsigned short count;
    unsigned char data[RFID_MT_RESPONSE_BUFF_SIZE];

    unsigned char card_uid_len;
    unsigned char card_uid[RFID_MT_CARD_UUID_LEN];
};
#pragma pack()

RFID_DEF_SRAM2 static unsigned char s_rfid_mt_lock = 0x00;
RFID_DEF_SRAM2 static unsigned char s_rfid_mt_request[RFID_MT_REQUEST_BUFF_SIZE];
RFID_DEF_SRAM2 static struct mt_card_info s_mt_card_info;

/*****************************************************************************
 *  函数名   rfid_mt_check_sum
 *  功能       计算数据的异或校验码
 *  参数       data         数据
 *       dlen         数据长度(B)
 * 返回        异或校验码
 ****************************************************************************/
static unsigned char rfid_mt_xor_check(unsigned char *data, unsigned int dlen)
{
    unsigned char check = 0x00;

    if(data == NULL){
        return check;
    }

    for(unsigned int count = 0x00; count < dlen; count++){
        check ^= data[count];
    }

    return check;
}

/*****************************************************************************
 *  函数名   rfid_mt_clear_data
 *  功能       清除串口数据及信号量
 *  参数
 * 返回
 ****************************************************************************/
static void rfid_mt_clear_data(void)
{
    unsigned char ch;

    while(rfid_dev_recv(&ch, 0x01) == 0x01);
    while(take_rfid_dev_data_sem(0x00) >= 0x00);
}

/*****************************************************************************
 *  函数名   s_rfid_mt_padding_request
 *  功能       填充射频识别请求帧
 *  参数       cmd          指令码
 *        cmd_para     指令参数
 *        data         请求数据
 *        dlen         请求数据长度(B)
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
static int s_rfid_mt_padding_request(unsigned char cmd, unsigned char cmd_para, unsigned char *data, unsigned short dlen)
{
    if((data == NULL) && (dlen != 0x00)){
        return -0x01;
    }
    if(((dlen + RFID_MT_REQ_DATA_REGION_FIX_LENGTH) + RFID_MT_FRAME_FIX_LEN) > RFID_MT_REQUEST_BUFF_SIZE){
        return -0x01;
    }

    unsigned short count, check, len = (dlen + RFID_MT_REQ_DATA_REGION_FIX_LENGTH);

    memset(s_rfid_mt_request, 0x00, RFID_MT_REQUEST_BUFF_SIZE);

    s_rfid_mt_request[0x00] = RFID_MT_FRAME_START_CODE;
    s_rfid_mt_request[0x01] = (unsigned char)(len >>0x08);
    s_rfid_mt_request[0x02] = (unsigned char)len;
    s_rfid_mt_request[0x03] = cmd;
    s_rfid_mt_request[0x04] = cmd_para;

    for(count = 0x00; count < dlen; count++){
        s_rfid_mt_request[0x05 + count] = data[count];
    }
    s_rfid_mt_request[0x05 + count] = RFID_MT_FRAME_END_CODE;
    check = rfid_mt_xor_check(s_rfid_mt_request, (len + RFID_MT_FRAME_FIX_LEN - 0x01));

    s_rfid_mt_request[len + RFID_MT_FRAME_FIX_LEN - 0x01] = check;

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_mt_recv_char
 *  功能       从接收缓冲区读取一个字节的数据
 *  参数
 * 返回        >=0：数据   <0：失败
 ****************************************************************************/
static signed short rfid_mt_recv_char(void)
{
    unsigned char ch = 0x00;

    while(rfid_dev_recv(&ch, 0x01) != 0x01) {
        if(take_rfid_dev_data_sem(RFID_MT_WAIT_SEM_TIME_MAX) < 0x00){
            return -0x01;
        }
    }

    return ch;
}

/*****************************************************************************
 *  函数名   rfid_mt_readline
 *  功能       从接收缓冲区读取一帧数据
 *  参数
 * 返回        >=0：成功   <0：失败
 ****************************************************************************/
static int rfid_mt_readline(void)
{
    unsigned char ch;
    signed short result = 0;
    unsigned short data_len = 0;
    unsigned short frame_check = 0, calculate_check = 0;
    s_mt_card_info.count = 0x00;

    while(1)
    {
        result = rfid_mt_recv_char();
        if(result < 0x00){                /** 射频识别设备未回复 */
            return -1;
        }
        ch = (unsigned char)result;
#ifdef RFIDR_MT_DEBUG
        LOG_D("mt recv|0x%x, %d", ch, s_mt_card_info.count);
#endif /* RFIDR_THA_DEBUG */
        if((ch != RFID_MT_FRAME_START_CODE) && (s_mt_card_info.count == 0x00)){
            continue;
        }
        s_mt_card_info.data[s_mt_card_info.count++] = ch;

        if(s_mt_card_info.count == (RFID_MT_FRAME_LENGTH_REGION + 0x01)){
            data_len = s_mt_card_info.data[RFID_MT_FRAME_LENGTH_REGION - 0x01];
            data_len <<=0x08;
            data_len |= s_mt_card_info.data[RFID_MT_FRAME_LENGTH_REGION];
            if((data_len + RFID_MT_FRAME_FIX_LEN) > RFID_MT_RESPONSE_BUFF_SIZE){
                LOG_W("RFID mt response buf is too short(%d, %d)", RFID_MT_RESPONSE_BUFF_SIZE, (data_len + RFID_MT_FRAME_FIX_LEN));
                return -0x01;
            }
        }

        if(s_mt_card_info.count >= (RFID_MT_FRAME_FIX_LEN + data_len)){
            frame_check = s_mt_card_info.data[RFID_MT_FRAME_FIX_LEN + data_len - 0x01];
            calculate_check = rfid_mt_xor_check(s_mt_card_info.data, (RFID_MT_FRAME_FIX_LEN + data_len - 0x01));
            if(frame_check != calculate_check){
                LOG_E("RFID mt check code error|%x |%x", frame_check, calculate_check);
                s_mt_card_info.count = 0x00;
                continue;
            }
            break;
        }
    }

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_mt_device_identify
 *  功能       判断设备是否支持此协议
 *  参数
 * 返回        1：支持   0：不支持     <0：读卡器未回复
 ****************************************************************************/
int rfid_mt_device_identify(void)
{
    if(rfid_mt_search_card() >= 0x00){
        return 0x01;
    }

    return 0x00;
}

/*****************************************************************************
 *  函数名   rfid_mt_query_version
 *  功能       查询版本
 *  参数       buf     存放版本缓存
 *     blen     缓存长度
 * 返回        >0：成功   =0：失败     <0：读卡器未回复
 ****************************************************************************/
int rfid_mt_query_version(unsigned char *buf, unsigned char blen)
{
    if((buf == NULL) || (blen == 0x00)){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned char version_len = 0x00, data_len = 0x00;

    memset(buf, 0x00, blen);

    while(s_rfid_mt_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_mt_lock = 0x01;

    rfid_mt_clear_data();

    if(s_rfid_mt_padding_request(RFID_MT_CMD_QUERY_VERSION, RFID_MT_CMD_PARA_QUERY_VERSION, NULL, 0x00) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }
    if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH)) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if(rfid_mt_readline() < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_QUERY_VERSION) ||
            (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_QUERY_VERSION)){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    data_len = s_mt_card_info.data[RFID_MT_FRAME_LENGTH_REGION - 0x01];
    data_len <<=0x08;
    data_len |= s_mt_card_info.data[RFID_MT_FRAME_LENGTH_REGION];

    if(data_len <= RFID_MT_RES_DATA_REGION_FIX_LENGTH){
        s_rfid_mt_lock = 0x00;
        return 0x00;
    }
    version_len = (data_len - RFID_MT_RES_DATA_REGION_FIX_LENGTH);

    if(blen > version_len){
        memcpy(buf, (s_mt_card_info.data + RFID_MT_RES_STATUS_REGION), version_len);
    }else{
        memcpy(buf, (s_mt_card_info.data + RFID_MT_RES_STATUS_REGION), blen);
    }

    s_rfid_mt_lock = 0x00;

    return 0x01;
}

/*****************************************************************************
 *  函数名   rfid_mt_search_card
 *  功能       寻卡
 *  参数
 * 返回        >0：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_search_card(void)
{
    unsigned int tick = rt_tick_get();
    unsigned char status = 0x00;

    while(s_rfid_mt_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_mt_lock = 0x01;

    rfid_mt_clear_data();

    if(s_rfid_mt_padding_request(RFID_MT_CMD_SEARCH_CARD, RFID_MT_CMD_PARA_SEARCH_CARD, NULL, 0x00) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }
    if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH)) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if(rfid_mt_readline() < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_SEARCH_CARD) ||
            (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_SEARCH_CARD)){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    status = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION];
    if(status == 'Y'){          /** yes：寻到卡 */
        s_rfid_mt_lock = 0x00;
        return 0x01;
    }else if(status == 'N'){    /** no：未寻到卡 */
        s_rfid_mt_lock = 0x00;
        return 0x00;
    }

    s_rfid_mt_lock = 0x00;

    return -0x01;
}

/*****************************************************************************
 *  函数名   rfid_mt_read_card_uuid
 *  功能       读取卡的uuid
 *  参数       uuid     用于保存接收到的UUID缓存
 *     ulen     缓存长度
 *     olen     用来保存UUID实际长度
 * 返回        >0：获取到uuid   0：未获取到uuid   <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_read_card_uuid(unsigned char *uuid, unsigned char ulen, unsigned char *olen)
{
    unsigned int tick = rt_tick_get();
    unsigned char status = 0x00, uuidlen = 0x00, data_len = 0x00, valid_len = 0x00;

    while(s_rfid_mt_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_mt_lock = 0x01;

    rfid_mt_clear_data();

    if(s_rfid_mt_padding_request(RFID_MT_CMD_QUERY_UUID, RFID_MT_CMD_PARA_QUERY_UUID, NULL, 0x00) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }
    if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH)) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if(rfid_mt_readline() < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_QUERY_UUID) ||
            (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_QUERY_UUID)){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    data_len = s_mt_card_info.data[RFID_MT_FRAME_LENGTH_REGION - 0x01];
    data_len <<=0x08;
    data_len |= s_mt_card_info.data[RFID_MT_FRAME_LENGTH_REGION];

    if(data_len <= RFID_MT_RES_DATA_REGION_FIX_LENGTH){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }
    uuidlen = (data_len - RFID_MT_RES_DATA_REGION_FIX_LENGTH);

    status = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION];
    if(status == 'Y'){          /** yes：获取到uuid */
        memset(uuid, 0x00, ulen);
        valid_len = ulen > uuidlen ? uuidlen : ulen;
        if(valid_len){
            for(signed char count = (valid_len - 0x01); count >= 0x00; count--){
                uuid[valid_len - 0x01 - count] = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION + 0x01 + count];
            }
        }

        memset(s_mt_card_info.card_uid, 0x00, RFID_MT_CARD_UUID_LEN);
        valid_len = uuidlen > RFID_MT_CARD_UUID_LEN ? RFID_MT_CARD_UUID_LEN : uuidlen;
        if(valid_len){
            for(signed char count = (valid_len - 0x01); count >= 0x00; count--){
                s_mt_card_info.card_uid[valid_len - 0x01 - count] = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION + 0x01 + count];
            }
        }

        if(olen){
            *olen = uuidlen;
        }
        s_rfid_mt_lock = 0x00;
        return 0x01;
    }else if(status == 'N'){    /** no：未获取到uuid */
        s_rfid_mt_lock = 0x00;
        return 0x00;
    }

    s_rfid_mt_lock = 0x00;

    return -0x01;
}

/***************************************************************************************
 *  函数名   rfid_mt_active_card
 *  功能       寻卡(卡激活)
 *  参数       uuid     用于保存接收到的UUID缓存
 *     ulen     缓存长度
 *     olen     用来保存UUID实际长度
 *     is_search_card  是否只是寻卡
 * 返回        3：获取到UUID(已寻到卡) 2：未获取到UUID(已寻到卡) 1：寻到卡   0：未寻到卡   <0：射频识别设备未回复(或回复有误)
 **************************************************************************************/
int rfid_mt_active_card(unsigned char is_search_card, unsigned char *uuid, unsigned char ulen, unsigned char *olen)
{
    int ret = 0x00;

    ret = rfid_mt_search_card();
    if(ret <= 0x00){
        return ret;
    }

    if(is_search_card == 0x00){
        rt_thread_mdelay(100);

        ret = rfid_mt_read_card_uuid(uuid, ulen, olen);
        if(ret < 0x00){
            return ret;
        }
    }

    return (ret + 0x02);
}

/*****************************************************************************
 *  函数名   rfid_mt_key_authentication
 *  功能       对卡进行密钥鉴权
 *  参数       key     密钥
 *     key_type 密钥类型 0:A密钥，1：B密钥
 *     klen    密钥长度
 *     sector   验证的扇区
 * 返回        >0：成功   =0：失败    <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_key_authentication(unsigned char key_type, unsigned char sector, unsigned char *key, unsigned char klen)
{
    if(key == NULL){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned short status = 0x00;
    unsigned char request[0x07];   /** 鉴权时帧的数据域为：扇区号(1字节) + 密钥(6字节)，7字节 */

    while(s_rfid_mt_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_mt_lock = 0x01;

    memset(request, 0x00, sizeof(request));
    request[0x00] = sector;
    if(klen > 0x06){
        memcpy(&request[0x01], key, 0x06);
    }else{
        memcpy(&request[0x01], key, klen);
    }

    rfid_mt_clear_data();

    /** 默认A密钥 */
    if(key_type == 0x01){
        if(s_rfid_mt_padding_request(RFID_MT_CMD_B_KEY_AUTHENTICATION, RFID_MT_CMD_PARA_B_KEY_AUTHENTICATION, \
                request, sizeof(request)) < 0x00){
            s_rfid_mt_lock = 0x00;
            return -0x01;
        }
    }else{
        if(s_rfid_mt_padding_request(RFID_MT_CMD_A_KEY_AUTHENTICATION, RFID_MT_CMD_PARA_A_KEY_AUTHENTICATION, \
                request, sizeof(request)) < 0x00){
            s_rfid_mt_lock = 0x00;
            return -0x01;
        }
    }

    if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH + sizeof(request))) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if(rfid_mt_readline() < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    /** 默认A密钥 */
    if(key_type == 0x01){
        if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_B_KEY_AUTHENTICATION) ||
                (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_B_KEY_AUTHENTICATION)){
            s_rfid_mt_lock = 0x00;
            return -0x01;
        }
    }else{
        if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_A_KEY_AUTHENTICATION) ||
                (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_A_KEY_AUTHENTICATION)){
            s_rfid_mt_lock = 0x00;
            return -0x01;
        }
    }

    if(s_mt_card_info.data[RFID_MT_RES_STATUS_REGION] != sector){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    status = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION + 0x01];
    if(status == 'Y'){          /** yes：密钥认证成功 */
        s_rfid_mt_lock = 0x00;
        return 0x01;
    }else if(status == 'N'){    /** no：密钥认证失败 */
        s_rfid_mt_lock = 0x00;
        return 0x00;
    }

    s_rfid_mt_lock = 0x00;

    return -0x01;
}

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
int rfid_mt_read_block_info(unsigned char sector, unsigned char block, unsigned char *buf, unsigned char blen, unsigned char is_nest)
{
    if((buf == 0x00) || (blen == 0x00)){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned short status = 0x00, rentry = 0x00;
    unsigned char request[0x02];   /** 扇区 + 块号 */

    if(is_nest == 0x00){
        while(s_rfid_mt_lock == 0x01){
            if(tick > rt_tick_get()){
                tick = rt_tick_get();
            }
            if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
                break;
            }
            rt_thread_mdelay(5);
        }
        s_rfid_mt_lock = 0x01;
    }

    request[0x00] = sector;
    request[0x01] = (block %0x04);     /** 每个扇区4个块，都从0开始算 */

    rfid_mt_clear_data();

    while(rentry < 0x05){    /** 如果写失败,最多尝试5次 */
        if(s_rfid_mt_padding_request(RFID_MT_CMD_READ_BLOCK_INFO, RFID_MT_CMD_PARA_READ_BLOCK_INFO, \
                request, sizeof(request)) < 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }
        if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH + sizeof(request))) < 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        if(rfid_mt_readline() < 0x00){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_READ_BLOCK_INFO) ||
                (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_READ_BLOCK_INFO)){
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        if((s_mt_card_info.data[RFID_MT_RES_STATUS_REGION] != sector) ||
                (s_mt_card_info.data[RFID_MT_RES_STATUS_REGION + 0x01] != (block %0x04))){    /** 每个扇区4个块，都从0开始算 */
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        status = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION + 0x02];

        if(status != 'Y'){            /** yes：操作成功 */
            rt_thread_mdelay(10);
            rentry++;
            continue;
        }

        break;
    }

    if(rentry >= 0x05){
        if(is_nest == 0x00){
            s_rfid_mt_lock = 0x00;
        }
        return 0x00;
    }

    memset(buf, 0x00, blen);
    if(blen < RFID_MT_BLOCK_SIZE){
        memcpy(buf, (s_mt_card_info.data + RFID_MT_RES_STATUS_REGION + 0x03), blen);
    }else{
        memcpy(buf, (s_mt_card_info.data + RFID_MT_RES_STATUS_REGION + 0x03), RFID_MT_BLOCK_SIZE);
    }

    if(is_nest == 0x00){
        s_rfid_mt_lock = 0x00;
    }

    return 0x01;
}

/*****************************************************************************
 *  函数名   rfid_mt_write_block_info
 *  功能       修改指定块信息
 *  参数       data    数据
 *     dlen    数据长度
 *     sector  块所属扇区
 *     block   块号
 * 返回        >0：成功   =0：失败     <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_write_block_info(unsigned char sector, unsigned char block, unsigned char *data, unsigned char dlen)
{
    if((data == 0x00) || (dlen == 0x00)){
        return -0x01;
    }

    unsigned int tick = rt_tick_get();
    unsigned char wbuff[RFID_MT_BLOCK_SIZE + 0x02],                /** 扇区 + 块号 + 缓存 */
                  rbuff[RFID_MT_BLOCK_SIZE], rentry = 0x00;

    while(s_rfid_mt_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_mt_lock = 0x01;

    if(dlen < RFID_MT_BLOCK_SIZE){
        if(rfid_mt_read_block_info(sector, block, (wbuff + 0x02), RFID_MT_BLOCK_SIZE, 0x01) < 0x00){
            s_rfid_mt_lock = 0x00;
            return -0x01;
        }
        memcpy((wbuff + 0x02), data, dlen);
    }else{
        memcpy((wbuff + 0x02), data, RFID_MT_BLOCK_SIZE);
    }

    wbuff[0x00] = sector;         /** 要写的块号所在扇区 */
    wbuff[0x01] = (block %4);     /** 要写的块号(每个扇区4个块，都从0开始算) */

    rfid_mt_clear_data();

    while(rentry < 0x05){    /** 如果写失败,最多尝试5次 */
        if(s_rfid_mt_padding_request(RFID_MT_CMD_WRITE_BLOCK_INFO, RFID_MT_CMD_PARA_WRITE_BLOCK_INFO, \
                wbuff, sizeof(wbuff)) < 0x00){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("mt reader not response when write block info(%d) entry(%d)", block, rentry);
            continue;
        }
        if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH + sizeof(wbuff))) < 0x00){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("mt reader not response when write block info(%d) entry(%d)", block, rentry);
            continue;
        }

        if(rfid_mt_readline() < 0x00){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("mt reader not response when write block info(%d) entry(%d)", block, rentry);
            continue;
        }

        rfid_mt_read_block_info(sector, block, rbuff, RFID_MT_BLOCK_SIZE, 0x01);                      /** 读数据 */

        if(memcmp((wbuff + 0x02), rbuff, RFID_MT_BLOCK_SIZE)){
            rentry++;
            rt_thread_mdelay(10);
            LOG_E("mt reader write block(%d) info check fail entry(%d)", block, rentry);
            continue;
        }

        break;
    }

    if(rentry >= 0x05){
        s_rfid_mt_lock = 0x00;
        return 0x00;
    }

    s_rfid_mt_lock = 0x00;

    return 0x01;
}

/*****************************************************************************
 *  函数名   rfid_mt_buzzer
 *  功能       蜂鸣器控制
 *  参数       count     蜂鸣器响的次数
 *     interval  鸣叫间隔
 * 返回        >0：成功   =0：失败     <0：射频识别设备未回复(或回复有误)
 ****************************************************************************/
int rfid_mt_buzzer(unsigned char count, unsigned short interval)
{
    unsigned int tick = rt_tick_get();
    unsigned char status = 0x00;
    unsigned char request[0x03];        /** 1字节鸣叫次数 + 2字节鸣叫间隔 */

    while(s_rfid_mt_lock == 0x01){
        if(tick > rt_tick_get()){
            tick = rt_tick_get();
        }
        if((rt_tick_get() - tick) > RFID_MT_WAIT_LOCK_TIME_MAX){
            break;
        }
        rt_thread_mdelay(5);
    }
    s_rfid_mt_lock = 0x01;

    request[0x00] = count;
    request[0x01] = (unsigned char)(interval >>0x08);
    request[0x02] = (unsigned char)interval;

    rfid_mt_clear_data();

    if(s_rfid_mt_padding_request(RFID_MT_CMD_BUZZER, RFID_MT_CMD_PARA_BUZZER, request, sizeof(request)) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }
    if(rfid_dev_send(s_rfid_mt_request, (RFID_MT_FRAME_FIX_LEN + RFID_MT_REQ_DATA_REGION_FIX_LENGTH + sizeof(request))) < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if(rfid_mt_readline() < 0x00){
        s_rfid_mt_lock = 0x00;
        return -0x01;
    }

    if((s_mt_card_info.data[RFID_MT_RES_CMD_REGION] != RFID_MT_CMD_BUZZER) ||
            (s_mt_card_info.data[RFID_MT_RES_CMD_PARA_REGION] != RFID_MT_CMD_PARA_BUZZER)){
        return -0x01;
    }

    status = s_mt_card_info.data[RFID_MT_RES_STATUS_REGION];
    if(status != 'Y'){
        s_rfid_mt_lock = 0x00;
        return 0x00;
    }

    s_rfid_mt_lock = 0x00;

    return 0x01;
}

#endif /* RFID_DEV_INCLUDE_MT */
