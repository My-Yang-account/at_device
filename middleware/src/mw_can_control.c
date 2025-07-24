/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-24     我的杨yang       the first version
 */
#include "mw_can_control.h"


uint8_t mw_is_can_recved(thaisenIsCANEnum en)
{
#ifndef APP_USING_DOUBLEGUN
    return thaisen_is_can_recved(en);
#endif /* APP_USING_DOUBLEGUN */
}

void mw_clear_can_recved(thaisenIsCANEnum en)
{
#ifndef APP_USING_DOUBLEGUN
    thaisen_clear_can_recved(en);
#endif /* APP_USING_DOUBLEGUN */
}

void mw_bmsa_can_send(uint32_t id, uint8_t *data, uint8_t dlen)
{
    can_msg_buf message;

    memset(&message, 0x00, sizeof(message));
    message.CANID = id;
    message.DLC = 0x08;
    message.length = 0x08;
    message.priority = 0x06;
    if(dlen > message.length){
        memcpy(message.data, data, message.length);
    }else{
        memcpy(message.data, data, dlen);
    }

    thaisen_bmsA_can_send(&message);
}

void mw_bmsb_can_send(uint32_t id, uint8_t *data, uint8_t dlen)
{
    can_msg_buf message;

    memset(&message, 0x00, sizeof(message));
    message.CANID = id;
    message.DLC = 0x08;
    message.length = 0x08;
    message.priority = 0x06;
    if(dlen > message.length){
        memcpy(message.data, data, message.length);
    }else{
        memcpy(message.data, data, dlen);
    }

    thaisen_bmsB_can_send(&message);
}

void mw_tcu_can_send(uint32_t id, uint8_t *data, uint8_t dlen)
{
#if 0
    can_msg_buf message;

    memset(&message, 0x00, sizeof(message));
    message.CANID = id;
    message.DLC = 0x08;
    message.length = 0x08;
    message.priority = 0x06;
    if(dlen > message.length){
        memcpy(message.data, data, message.length);
    }else{
        memcpy(message.data, data, dlen);
    }

    thaisen_tcu_can_send(&message);
#endif
}

void mw_module_can_send(uint32_t id, uint8_t *data, uint8_t dlen)
{
    can_msg_buf message;

    memset(&message, 0x00, sizeof(message));
    message.CANID = id;
    message.DLC = 0x08;
    message.length = 0x08;
    message.priority = 0x06;
    if(dlen > message.length){
        memcpy(message.data, data, message.length);
    }else{
        memcpy(message.data, data, dlen);
    }

    thaisen_chargmodule_can_send(&message);
}



void mw_bmsa_can_recv(mw_can_info *buf)
{
    if(buf){
        can_msg_buf message = thaisen_get_can_bmsA_dat();

        buf->id = message.CANID;
        buf->length = message.length;
        if(buf->length > 0x08){
            memcpy(buf->data, message.data, 0x08);
        }else{
            memset(buf->data, 0x00, sizeof(buf->data));
            memcpy(buf->data, message.data, buf->length);
        }
    }
}

void mw_bmsb_can_recv(mw_can_info *buf)
{
    if(buf){
        can_msg_buf message = thaisen_get_can_bmsB_dat();

        buf->id = message.CANID;
        buf->length = message.length;
        if(buf->length > 0x08){
            memcpy(buf->data, message.data, 0x08);
        }else{
            memset(buf->data, 0x00, sizeof(buf->data));
            memcpy(buf->data, message.data, buf->length);
        }
    }
}

void mw_tcu_can_recv(mw_can_info *buf)
{
#if 0
    if(buf){
        can_msg_buf message = thaisen_get_tcu_dat();

        buf->id = message.CANID;
        buf->length = message.length;
        if(buf->length > 0x08){
            memcpy(buf->data, message.data, 0x08);
        }else{
            memset(buf->data, 0x00, sizeof(buf->data));
            memcpy(buf->data, message.data, buf->length);
        }
    }
#endif
}

void mw_module_can_recv(mw_can_info *buf)
{
    if(buf){
        can_msg_buf *message = thaisen_get_can_charg_module_dat();

        buf->id = message->CANID;
        buf->length = message->length;
        if(buf->length > 0x08){
            memcpy(buf->data, message->data, 0x08);
        }else{
            memset(buf->data, 0x00, sizeof(buf->data));
            memcpy(buf->data, message->data, buf->length);
        }
    }
}

