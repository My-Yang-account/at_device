/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-24     我的杨yang       the first version
 */
#ifndef MIDDLEWARE_INC_MW_CAN_CONTROL_H_
#define MIDDLEWARE_INC_MW_CAN_CONTROL_H_

#include "thaisen7102Public.h"

typedef struct{
    uint32_t id;
    uint8_t length;
    uint8_t data[8];
}mw_can_info;

uint8_t mw_is_can_recved(thaisenIsCANRecv en);
void mw_clear_can_recved(thaisenIsCANRecv en);

void mw_bmsa_can_send(uint32_t id, uint8_t *data, uint8_t dlen);
void mw_bmsb_can_send(uint32_t id, uint8_t *data, uint8_t dlen);
void mw_tcu_can_send(uint32_t id, uint8_t *data, uint8_t dlen);
void mw_module_can_send(uint32_t id, uint8_t *data, uint8_t dlen);

void mw_bmsa_can_recv(mw_can_info *buf);
void mw_bmsb_can_recv(mw_can_info *buf);
void mw_tcu_can_recv(mw_can_info *buf);
void mw_module_can_recv(mw_can_info *buf);

#endif /* MIDDLEWARE_INC_MW_CAN_CONTROL_H_ */
