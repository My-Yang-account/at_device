/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-24     我的杨yang       the first version
 */
#ifndef APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_
#define APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_
#include "string.h"
#include "app_ofsm.h"
#include "app_hci.h"
#include "applications/version.h"
#include "chargepile_config.h"
#include "notfs_cfg.h"
#include "app_billing_rule.h"
#include "app_osupport.h"

#include "bsp_serial.h"

#include "mw_time.h"
#include "mw_cc1.h"
#include "mw_fault_check.h"
#include "mw_storage.h"
#include "mw_charge_control.h"

/**
 * 获取充电状态
 **/
enum ofsm_state thaisen_app_get_ofsm_charge_state(uint8_t gunno);
/**
 * 获取网络状态
 **/
enum net_state thaisen_app_get_net_state(void);

struct app_version{
    uint8_t version[8];
};
struct app_version *thaisen_app_get_app_version(void);

/**
 * 获取二维码
 **/
#define QRCODE_BUFF_LEN    200     /* 二维码缓存长度 */

struct qrcode_info{
    uint8_t qrcode[QRCODE_BUFF_LEN];   /* 二维码缓存 */
    uint16_t qrcode_len;               /* 二维码长度 */
};
struct qrcode_info *thaisen_app_get_gunno_qrcode(uint8_t gunno);
/**
 * 获取设备枪号名
 **/
void thaisen_get_device_sn(char *src, uint8_t length, uint8_t gunno);

/**
 * 获取枪连接状态
 **/
enum connect_state{
    APP_GUN_CONNECT_STATE_NO,    /* 未连接 */
    APP_GUN_CONNECT_STATE_YES,   /* 已连接 */
};
uint8_t thaisen_app_get_connect_state(uint8_t gunno);
/**
 * 获取停充方式
 **/
enum system_stop_way thaisen_app_get_charge_stop_way(uint8_t gunno);
/**
 * 获取充电数据
 **/
struct charge_data{
    uint8_t trade_number[16];       /* 流水号 */
    uint32_t charge_voltage;        /* 充电电压：精度2位 ,单位：V*/
    uint32_t charge_current;        /* 充电电流：精度2位 ,单位：A */
    uint32_t charge_power;          /* 充电功率：精度1位 ,单位：W */
    uint32_t charge_elect;          /* 充电电量：精度3位 ,单位：KWh */
    uint32_t charge_total_fee;      /* 费用：精度4位 */
    uint32_t charge_start_time;     /* 充电开始时间：单位s */
    uint32_t charge_stop_time;      /* 充电结束时间：单位s */
    uint32_t charge_time;           /* 充电时间：单位s */
    uint32_t remain_charge_time;    /* 剩余充电时间：单位s */
    uint16_t current_soc;           /* 当前SOC */
};
struct charge_data *thaisen_app_get_charge_info(uint8_t gunno);
/**
 * 获取电池信息
 **/
struct battery_info{
    uint8_t battery_type;
};
struct battery_info *thaisen_app_get_battery_info(uint8_t gunno);
/**
 * 获取BMS信息
 **/
struct bms_info{
    uint16_t single_battery_max_voltage;
    uint16_t bms_require_voltage;
    uint16_t bms_require_current;
};
struct bms_info *thaisen_app_get_bms_info(uint8_t gunno);
/**
 * 获取计费信息
 **/
struct account_info{
    uint32_t sharp_unit_fee;    /* 尖单价 精度：4位 */
    uint32_t sharp_elect;       /* 尖电量 精度：4位 */
    uint32_t sharp_account;     /* 尖金额 精度：4位 */

    uint32_t peak_unit_fee;     /* 峰单价 精度：4位 */
    uint32_t peak_elect;        /* 峰电量 精度：4位 */
    uint32_t peak_account;      /* 峰金额 精度：4位 */

    uint32_t flat_unit_fee;    /* 平单价 精度：4位 */
    uint32_t flat_elect;        /* 平电量 精度：4位 */
    uint32_t flat_account;      /* 平金额 精度：4位 */

    uint32_t valley_unit_fee;   /* 谷单价 精度：4位 */
    uint32_t valley_elect;      /* 谷电量 精度：4位 */
    uint32_t valley_account;    /* 谷金额 精度：4位 */

    uint32_t current_unit_fee;  /* 当前单价 精度：4位 */

};
struct account_info *thaisen_app_get_account_info(uint8_t gunno);
/**
 * 获取故障信息
 **/
struct fault_info{
    enum charge_fault_t charge_fault;
    enum system_fault_t system_fault;
};
struct fault_info *thaisen_app_get_fault_info(uint8_t gunno);

/**
 * 清除指定区域记录信息
 */
int32_t thaisen_app_clear_region_record_info(enum record_region region);
/**
 * 获取故障或充电记录当前下标
 */
int32_t thaisen_app_get_current_region_index(enum record_region region);
/**
 * 获取故障或充电记录总数
 */
int32_t thaisen_app_get_region_record_total_num(enum record_region region);
/**
 * 获取指定下标的故障记录
 */
int32_t thaisen_app_get_index_fault_record(struct error_info *error_buff, uint8_t index, enum record_region region);
/**
 * 获取指定下标的充电记录
 */
int32_t thaisen_app_get_index_charge_record(thaisen_transaction_t* bill_buff, uint8_t index, enum record_region region);

/**
 * 触发存储配置项数据
 **/
int32_t thaisen_app_storage_config_port(void);
/**
 * 同步配置项数据
 **/
int32_t thaisen_app_sync_config_item_port(enum config_name name, void* data, uint32_t len);
/**
 * 读配置项
 **/
uint8_t* thaisen_app_read_config_item_port(enum config_name name, uint8_t is_user_content);


/**
 * 向屏幕发数据
 **/
void thaisen_app_send_data_to_hci_uart(uint8_t* data, uint8_t len);
/**
 * 获取时间戳
 **/
uint32_t thaisen_app_get_current_timestamp(void);
/**
 * 获取系统tick(1ms)
 **/
uint32_t thaisen_app_get_system_tick(void);
/**
 * 设置屏幕启动充电指令
 **/
void thaisen_app_set_screen_start_charge(uint8_t gunno);
/**
 * 设置屏幕停止充电指令
 **/
void thaisen_app_set_screen_stop_charge(uint8_t gunno);
/**
 * 获取VIN码启动充电状态
 **/
uint8_t thaisen_app_get_vin_start_charge(uint8_t gunno);
/**
 * 清除VIN码启动充电指令
 **/
void thaisen_app_clear_vin_start_charge(uint8_t gunno);
/**
 * 设置VIN码启动充电指令
 **/
void thaisen_app_set_vin_start_charge(uint8_t gunno);

struct temperature{
    int8_t temp;       /* 动力电池最高温度 */
    uint8_t number;    /* 最高温度检测点编号 */
};

struct temperature* get_battery_temp_info(uint8_t gunno);
/**
 * 获取OTA 信息
 **/
ota_info* thaisen_app_get_ota_info(void);
/**
 * 获取时间同步标志
 **/
uint8_t thaisen_app_get_time_sync_flag(void);
/**
 * 获取当前页面的枪号
 **/
uint8_t thaisen_get_hci_page_pos(void);
/**
 * 获取信号强度
 **/
int thaisen_app_get_signal_strength(void);
/**
 * 获取SIM卡卡号
 **/
char *thaisen_app_get_sim_number(void);
/**
 * 获取屏幕时间同步标志
 **/
int8_t thaisen_get_screen_timesync_flag(void);
/**
 * 获取屏幕时间同步时间值
 **/
void thaisen_get_screen_timesync_time(uint16_t* buf, uint8_t len);
/**
 * 请求屏幕时间
 **/
void thaisen_request_screen_time(void);
/**
 * 根据停充码获取停充原因字符串
 **/
const char* thaisen_get_stopway_string(uint16_t code);
/**
 * 获取CC1电压
 **/
uint8_t thaisen_get_cc1_voltage(uint8_t gunno);
/**
 * 获取枪头温度
 **/
int32_t thaisen_get_gun_temp(uint8_t gunno);
/**
 * 获取电表电压
 **/
uint32_t thaisen_get_ammeter_voltage(uint8_t gunno);
/**
 * 获取电表电流
 **/
uint32_t thaisen_get_ammeter_current(uint8_t gunno);
/**
 * 获取功率百分比
 **/
int16_t thaisen_get_power_percent(void);
/**
 * 添加VIN码白名单
 **/
int32_t thaisen_vin_whitelists_add(uint8_t *data, uint8_t len);
/**
 * 获取充电方式
 **/
enum charge_way{
    APP_CHARGE_WAY_NONE,
    APP_CHARGE_WAY_SINGLEGUN,
    APP_CHARGE_WAY_PARACHARGE,
};
enum charge_way thaisen_get_charge_way(void);
/**
 * 设置充电方式
 **/
void thaisen_set_charge_way(uint8_t way);

#endif /* APPLICATIONS_INC_APP_DATA_INFO_INTERFACE_H_ */


