/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     我的杨yang       the first version
 */
#ifndef APPLICATIONS_INC_APP_SUPPORT_FUNC_H_
#define APPLICATIONS_INC_APP_SUPPORT_FUNC_H_

#include "rtthread.h"
#include "mw_charge_control.h"

#define START_FROM_HIGH_BYTE   (1 <<0)
#define START_FROM_LOW_BYTE    (1 <<1)

#define DC_SECTION_HEAD_LEN     24

uint16_t get_report_stop_way(uint16_t fault_bit, uint8_t stop_reason_type);

uint32_t calculate_data_from_byte(uint8_t* data, uint8_t len, uint8_t flag);
int8_t packing_data(uint8_t* buff, uint8_t buff_free_len, uint32_t data, uint8_t data_len, uint8_t flag);
uint32_t get_check_sum(uint8_t* data, uint32_t len);
uint32_t crc32_ieee(uint32_t crc, const uint8_t *data, uint32_t len);
uint16_t get_crc16_modbus(uint16_t crc, uint8_t *data, uint32_t len);

#endif /* APPLICATIONS_INC_APP_SUPPORT_FUNC_H_ */
