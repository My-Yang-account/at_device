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

/********************************************
 * 函数名      app_get_fault_chinese
 * 功能          获取中文故障信息
 * 参数          code      故障码
 *        olen      用于保存中文故障信息实际长度
 *        buf       用于保存中文故障信息
 *        ilen      buf  的长度
 * 返回
 *******************************************/
void app_get_fault_chinese(uint32_t code, uint8_t *olen, uint8_t *buf, uint8_t ilen);

/********************************************
 * 函数名      app_get_charge_stopway_chinese
 * 功能          获取中文停充原因
 * 参数          code      故障码
 *        olen      用于保存中文停充原因信息实际长度
 *        buf       用于保存中文停充原因信息
 *        ilen      buf  的长度
 * 返回
 *******************************************/
void app_get_charge_stopway_chinese(uint32_t code, uint8_t *olen, uint8_t *buf, uint8_t ilen);

/********************************************
 * 函数名      app_get_selfcheck_chinese
 * 功能          一键自检中文信息
 * 参数          item      自检项
 *      ret       自检结果(1：成功   0：失败)
 *      buf       用于保存中文信息
 *      ilen      buf  的长度
 * 返回
 *******************************************/
void app_selfcheck_debug_info(uint8_t item, uint8_t language, uint8_t ret, uint8_t *buf, uint8_t ilen);

#endif /* APPLICATIONS_INC_APP_SUPPORT_FUNC_H_ */
