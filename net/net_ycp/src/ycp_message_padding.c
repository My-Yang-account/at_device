/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-09     我的杨yang       the first version
 */
#include "ycp_message_padding.h"
#include "ycp_message_send.h"
#include "ycp_message_receive.h"
#include "ycp_fault_analyse.h"

#include "app_ofsm.h"
#include "app_billing_rule.h"
#include "net_operation.h"

#define DBG_TAG "ycp_rl"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef NET_PACK_USING_YCP

#define YCP_HOST_FAULT_NUM_MAX_DEFAULT            0x10         /* 默认上报主机故障数目最大值 */
#define YCP_PORT_FAULT_NUM_MAX_DEFAULT            0x20         /* 默认上报枪口故障数目最大值 */

#define YCP_CHARGE_ELECT_MAX                      1000000       /* 最大充电电量值(精度：0.001) */
#define YCP_SPEND_AMOUNT_MAX                      30000000      /* 最大消费金额值(精度：0.0001) */

#define YCP_DISPOSABLE_EVENT_STATE                0x00          /* 漏报事件：桩状态 */

#define YCP_REQ_BILLINGRULE_AGAIN_INTERVAL        (30 *1000)    /* 外部触发重新请求计费模型间隔 */
#define YCP_STATE_DATA_INTERVAL_INIT              0x05          /* 刚连上网时状态数据上报间隔 */
#define YCP_STATE_DATA_INTERVAL_CHARGING          0x0F          /* 充电中状态数据上报间隔  */
#define YCP_STATE_DATA_INTERVAL_IDLE              0x05 *60      /* 空闲状态数据上报间隔  */
#define YCP_FAULT_INFO_INTERVAL                   0x05 *60      /* 故障信息上报间隔  */

#define YCP_REALTIME_PROCESS_THREAD_STACK_SIZE    1536          /* 实时处理线程栈大小 */

#pragma pack(1)

struct ycp_disposable_info{
    struct{
        uint8_t state : 4;
        uint8_t connect : 2;
        uint8_t reserve : 2;
    }state;                                       /* 桩状态 */
    uint8_t disposable_event;
};

struct ycp_flag_info{
    uint16_t is_start_charge : 1;                  /*  已启动充电 */
    uint16_t is_stop_charge : 1;                   /*  已停止充电 */
    uint16_t is_set_power : 1;                     /*  已设置功率百分比 */

    uint16_t start_success : 1;                    /*  启机成功 */
    uint16_t stop_success : 1;                     /*  停机成功 */
    uint16_t set_power_success : 1;                /*  设置功率百分比成功 */

    uint16_t chargedata_mode_last : 2;             /*  前一次的充电数据模式 */
    uint16_t is_vin_authorized : 1;                /*  已进行了VIN鉴权 */
};

/** 枪口故障 */
struct ycp_pfaut_node{
    uint8_t fault_type;                            /* 故障类型 */
    uint8_t gunno;                                 /* 枪号 */
    uint16_t fault_code;                           /* 故障码 */
};
struct ycp_pfaut_info{
    uint8_t fault_num;                             /* 枪口故障数 */
    struct ycp_pfaut_node fault[YCP_PORT_FAULT_NUM_MAX_DEFAULT];  /* 枪口故障 */
};

/** 主机故障 */
struct ycp_hfaut_node{
    uint8_t fault_type;                            /* 故障类型 */
    uint16_t fault_code;                           /* 故障码 */
};
struct ycp_hfaut_info{
    uint8_t fault_num;                             /* 主机故障数 */
    struct ycp_hfaut_node fault[YCP_HOST_FAULT_NUM_MAX_DEFAULT];  /* 主机故障 */
};

#pragma pack()

NET_DEF_SRAM2 static struct ycp_flag_info s_ycp_flag_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint16_t s_ycp_state_data_interval[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static uint32_t s_ycp_state_data_count[NET_SYSTEM_GUN_NUMBER], s_ycp_fault_report;
NET_DEF_SRAM2 static uint16_t s_ycp_local_start_sq;
NET_DEF_SRAM2 static struct ycp_disposable_info s_ycp_disposable_info[NET_SYSTEM_GUN_NUMBER];
NET_DEF_SRAM2 static struct rt_thread s_ycp_realtime_process_thread;
NET_DEF_SRAM0 static uint8_t s_ycp_realtime_process_thread_stack[YCP_REALTIME_PROCESS_THREAD_STACK_SIZE];
NET_DEF_SRAM2 static struct net_handle* s_ycp_handle = NULL;
NET_DEF_SRAM2 static struct ycp_pfaut_info s_ycp_pfaut_info;
NET_DEF_SRAM2 static struct ycp_hfaut_info s_ycp_hfaut_info;

static uint16_t ycp_chargepile_stop_reason_converted(void *handle, uint16_t bit, uint8_t stop_in_starting);
static uint8_t ycp_chargepile_transaction_identity_converted(uint8_t identity);

/*************************************************
 * 函数名      ycp_set_clear_disposable_event
 * 功能          设置漏报报文事件
 * **********************************************/
static void ycp_set_clear_disposable_event(uint8_t gunno, uint8_t event, uint8_t is_clear)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(is_clear){
        s_ycp_disposable_info[gunno].disposable_event &= (~(1 <<event));
    }else{
        s_ycp_disposable_info[gunno].disposable_event |= (1 <<event);
    }
}

/*************************************************
 * 函数名      ycp_get_disposable_event
 * 功能          获取漏报报文事件
 * **********************************************/
static uint8_t ycp_get_disposable_event(uint8_t gunno, uint8_t event, uint8_t *buf)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    if(buf){
        *buf = s_ycp_disposable_info[gunno].disposable_event;
    }
    if(s_ycp_disposable_info[gunno].disposable_event &(1 <<event)){
        return 0x01;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_get_start_charge_result
 * 功能          启动充电成功
 * **********************************************/
uint8_t ycp_is_start_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ycp_flag_info[gunno].start_success;
}

/*************************************************
 * 函数名      ycp_is_stop_charge_success
 * 功能          停止充电成功
 * **********************************************/
uint8_t ycp_is_stop_charge_success(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }
    return s_ycp_flag_info[gunno].stop_success;
}

/*************************************************
 * 函数名      ycp_is_set_power_success
 * 功能          设置功率成功
 * **********************************************/
uint8_t ycp_is_set_power_success(void)
{
    return s_ycp_flag_info[0x00].set_power_success;
}

/*************************************************
 * 函数名      ycp_storage_data_check
 * 功能          校验存储的平台数据
 * **********************************************/
static void ycp_storage_data_check(void)
{
    uint8_t verify_success = 0x01;
    ycp_storage_struct *config = (ycp_storage_struct*)(s_ycp_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));

    if(config == NULL){
        verify_success = 0x00;
    }else if((config->verify_result == 0x00) || (config->storage_init_flag != NET_YCP_STORAGE_INIT_FLAG)){
        verify_success = 0x00;
    }else if(((config->tip_elect_rate == 0x00) && (config->tip_service_rate == 0x00)) &&
            ((config->peak_elect_rate == 0x00) && (config->peak_service_rate == 0x00)) &&
            ((config->flat_elect_rate == 0x00) && (config->flat_service_rate == 0x00)) &&
            ((config->valley_elect_rate == 0x00) && (config->valley_service_rate == 0x00))){
        verify_success = 0x00;
    }

    if(verify_success){
        g_ycp_sreq_set_para.body.heartbeat_interval = config->heartbeat_interval;
        g_ycp_sreq_set_para.body.voice_volume = config->voice_volume;
        g_ycp_sreq_set_para.body.device_password = config->device_password;
        g_ycp_sreq_set_para.body.temp_protect = config->temp_protect;
        g_ycp_sreq_set_para.body.chargedata_type = config->chargedata_type;
        g_ycp_sreq_set_para.body.bmsdata_switch = config->bmsdata_switch;
        g_ycp_sreq_set_para.body.power_percent = config->power_percent;

        g_ycp_sreq_billing_model_set.body.model_number = config->model_number;
        g_ycp_sreq_billing_model_set.body.tip_elect_rate = config->tip_elect_rate;
        g_ycp_sreq_billing_model_set.body.tip_service_rate = config->tip_service_rate;
        g_ycp_sreq_billing_model_set.body.peak_elect_rate = config->peak_elect_rate;
        g_ycp_sreq_billing_model_set.body.peak_service_rate = config->peak_service_rate;
        g_ycp_sreq_billing_model_set.body.flat_elect_rate = config->flat_elect_rate;
        g_ycp_sreq_billing_model_set.body.flat_service_rate = config->flat_service_rate;
        g_ycp_sreq_billing_model_set.body.valley_elect_rate = config->valley_elect_rate;
        g_ycp_sreq_billing_model_set.body.valley_service_rate = config->valley_service_rate;
        g_ycp_sreq_billing_model_set.body.loss_proportion = config->loss_proportion;
        for(uint8_t count = 0x00; count < NET_YCP_RATE_PERIOD_COUNT_MAX; count++){
            g_ycp_sreq_billing_model_set.body.rate_number[count] = config->rate_number[count];
        }
        g_ycp_preq_billing_model_verify.body.model_sn = config->model_number;

        ycp_message_pro_billing_model_set_response(&g_ycp_sreq_billing_model_set, sizeof(g_ycp_sreq_billing_model_set), 0x01);
    }else{
        g_ycp_sreq_set_para.body.heartbeat_interval = NET_YCP_HEARTBEAT_INTERVAL_DEFAULT;
        g_ycp_sreq_set_para.body.voice_volume = NET_YCP_VOICE_VOLUME_DEFAULT;
        g_ycp_sreq_set_para.body.device_password = NET_YCP_DEVICE_PASSWORD_DEFAULT;
        g_ycp_sreq_set_para.body.temp_protect = NET_YCP_TEMP_PROTECT_DEFAULT;
        g_ycp_sreq_set_para.body.chargedata_type = NET_YCP_CHARGEDATA_TYPE_DEFAULT;
        g_ycp_sreq_set_para.body.bmsdata_switch = NET_YCP_BMSDATA_SWITCH_DEFAULT;
        g_ycp_sreq_set_para.body.power_percent = NET_YCP_POWER_PERCENT_DEFAULT;

        g_ycp_preq_billing_model_verify.body.model_sn = NET_YCP_MODEL_NUMBER_DEFAULT;
    }
//#if 0
    LOG_D("ycp_storage_data_check(%d, %d)\n", config->verify_result, config->storage_init_flag);
    LOG_D("heartbeat_interval(%x)\n", g_ycp_sreq_set_para.body.heartbeat_interval);
    LOG_D("voice_volume(%x)\n", g_ycp_sreq_set_para.body.voice_volume);
    LOG_D("device_password(%x)\n", g_ycp_sreq_set_para.body.device_password);
    LOG_D("temp_protect(%x)\n", g_ycp_sreq_set_para.body.temp_protect);
    LOG_D("chargedata_type(%x)\n", g_ycp_sreq_set_para.body.chargedata_type);
    LOG_D("bmsdata_switch(%x)\n", g_ycp_sreq_set_para.body.bmsdata_switch);
    LOG_D("power_percent(%x)\n", g_ycp_sreq_set_para.body.power_percent);

    LOG_D("model_sn(%x)\n", g_ycp_sreq_billing_model_set.body.model_number);
    LOG_D("tip_elect_rate(%x)\n", g_ycp_sreq_billing_model_set.body.tip_elect_rate);
    LOG_D("tip_service_rate(%x)\n", g_ycp_sreq_billing_model_set.body.tip_service_rate);
    LOG_D("peak_elect_rate(%x)\n", g_ycp_sreq_billing_model_set.body.peak_elect_rate);
    LOG_D("peak_service_rate(%x)\n", g_ycp_sreq_billing_model_set.body.peak_service_rate);
    LOG_D("flat_elect_rate(%x)\n", g_ycp_sreq_billing_model_set.body.flat_elect_rate);
    LOG_D("flat_service_rate(%x)\n", g_ycp_sreq_billing_model_set.body.flat_service_rate);
    LOG_D("valley_elect_rate(%x)\n", g_ycp_sreq_billing_model_set.body.valley_elect_rate);
    LOG_D("valley_service_rate(%x)\n", g_ycp_sreq_billing_model_set.body.valley_service_rate);
    LOG_D("loss_proportion(%x)\n", g_ycp_sreq_billing_model_set.body.loss_proportion);

    rt_kprintf("[\n");
    for(uint8_t count = 0x00; count < NET_YCP_RATE_PERIOD_COUNT_MAX; count++){
        rt_kprintf("%d ", g_ycp_sreq_billing_model_set.body.rate_number[count]);
    }
    rt_kprintf("]\n");
//#endif /* 0 */
}

/*************************************************
 * 函数名      ycp_response_padding_query_state_data
 * 功能          组包：查询单枪状态数据响应
 * **********************************************/
int8_t ycp_response_padding_query_state_data(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint8_t length = 0x00;
    Net_YcpPro_PRes_Query_PReq_Report_PileState_t *response = NULL;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    response = ((Net_YcpPro_PRes_Query_PReq_Report_PileState_t*)buf);
    if(g_ycp_sreq_query_device_state[gunno].body.data_type == 0x01){
        length = (uint8_t)((uint32_t)&g_ycp_preq_report_state_data[gunno].body.plug_gun - (uint32_t)&g_ycp_preq_report_state_data[gunno]) + \
                0x01 + NET_YCP_PROTOCOL_CHECK_REGION_SIZE;
        if(length > ilen){
            return -0x02;
        }
        memset(response, 0x00, length);
        memcpy(response, &g_ycp_preq_report_state_data[gunno], length);
    }else{
        uint8_t valid_len = 0x00;

        length = sizeof(Net_YcpPro_PRes_Query_PReq_Report_PileState_t);
        if(length > ilen){
            return -0x02;
        }
        memset(response, 0x00, length);
        memcpy(response, &g_ycp_preq_report_state_data[gunno], length);

        valid_len = sizeof(response->body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(response->body.serial_number, 0x00, sizeof(response->body.serial_number));
        memcpy(response->body.serial_number, base->transaction_number, valid_len);
    }

    if(olen){
        *olen = length;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_query_state_data_all
 * 功能          组包：查询所有枪状态数据响应
 * **********************************************/
int8_t ycp_response_padding_query_state_data_all(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    if(buf == NULL){
        return -0x01;
    }

    uint8_t valid_len = 0x00;
    uint8_t length = 0x00, *charging_gun_number = NULL;
    Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t *response = NULL;
    System_BaseData *base = NULL;

    response = ((Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t*)buf);
    if(g_ycp_sreq_query_device_state_all.body.data_type == 0x01){
        length = sizeof(Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t) + NET_SYSTEM_GUN_NUMBER *sizeof(struct gun_state);
        if(length > ilen){
            return -0x02;
        }
        struct gun_state *_gunno_state = (struct gun_state*)((uint8_t*)&(response->body.gun_num) + 0x01);

        memset(response, 0x00, length);
        response->body.data_type = g_ycp_sreq_query_device_state_all.body.data_type;
        response->body.gun_num = NET_SYSTEM_GUN_NUMBER;
        charging_gun_number = (uint8_t*)(&_gunno_state[NET_SYSTEM_GUN_NUMBER]);
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            _gunno_state[gunno].gunno = gunno + 0x01;
            _gunno_state[gunno].state = g_ycp_preq_report_state_data[gunno].body.state;
            _gunno_state[gunno].homing = g_ycp_preq_report_state_data[gunno].body.homing;
            _gunno_state[gunno].plug_gun = g_ycp_preq_report_state_data[gunno].body.plug_gun;
            if(base->state.current == APP_OFSM_STATE_CHARGING){
                (*charging_gun_number)++;
            }
        }
    }else{
        length = sizeof(Net_YcpPro_PRes_Query_PReq_Report_PileState_All_t) + NET_SYSTEM_GUN_NUMBER *(sizeof(struct gun_data) + sizeof(struct gun_state));
        if(length > ilen){
            return -0x02;
        }

        struct gun_state *_gunno_state = (struct gun_state*)((uint8_t*)&(response->body.gun_num) + 0x01);
        struct gun_data *_gun_data = NULL;

        memset(response, 0x00, length);
        response->body.data_type = g_ycp_sreq_query_device_state_all.body.data_type;
        response->body.gun_num = NET_SYSTEM_GUN_NUMBER;
        charging_gun_number = (uint8_t*)(&_gunno_state[NET_SYSTEM_GUN_NUMBER]);
        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            _gunno_state[gunno].gunno = gunno + 0x01;
            _gunno_state[gunno].state = g_ycp_preq_report_state_data[gunno].body.state;
            _gunno_state[gunno].homing = g_ycp_preq_report_state_data[gunno].body.homing;
            _gunno_state[gunno].plug_gun = g_ycp_preq_report_state_data[gunno].body.plug_gun;
            if(base->state.current == APP_OFSM_STATE_CHARGING){
                (*charging_gun_number)++;
            }
        }

        _gun_data = (struct gun_data*)(charging_gun_number + 0x01);

        for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            valid_len = sizeof(_gun_data[gunno].serial_number);
            valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
            memset(_gun_data[gunno].serial_number, 0x00, sizeof(_gun_data[gunno].serial_number));
            memcpy(_gun_data[gunno].serial_number, base->transaction_number, valid_len);

            _gun_data[gunno].gunno = gunno + 0x01;
            _gun_data[gunno].output_voltage = g_ycp_preq_report_state_data[gunno].body.output_voltage;
            _gun_data[gunno].output_current = g_ycp_preq_report_state_data[gunno].body.output_current;
            memcpy(_gun_data[gunno].gunline_number, g_ycp_preq_report_state_data[gunno].body.gunline_number, NET_YCP_GUNLINE_NUMBER_LENGTH_DEFAULT);
            _gun_data[gunno].gun_temperature = g_ycp_preq_report_state_data[gunno].body.gun_temperature;
            _gun_data[gunno].soc = g_ycp_preq_report_state_data[gunno].body.soc;
            _gun_data[gunno].battery_group_temp_max = g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max;
            _gun_data[gunno].charge_time = g_ycp_preq_report_state_data[gunno].body.charge_time;
            _gun_data[gunno].remain_time = g_ycp_preq_report_state_data[gunno].body.remain_time;
            _gun_data[gunno].charge_elect = g_ycp_preq_report_state_data[gunno].body.charge_elect;
            _gun_data[gunno].loss_elect = g_ycp_preq_report_state_data[gunno].body.loss_elect;
            _gun_data[gunno].consume_amount = g_ycp_preq_report_state_data[gunno].body.consume_amount;
        }
    }

    if(olen){
        *olen = length;
    }
    return 0x00;
}

/*************************************************
 * 函数名      tha_response_padding_general_message
 * 功能          组包：远程启机响应
 * **********************************************/
int8_t ycp_response_padding_remote_start_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Remote_StartCharge_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    uint8_t valid_len = 0x00;
    Net_YcpPro_PRes_Remote_StartCharge_t *response = NULL;

    response = ((Net_YcpPro_PRes_Remote_StartCharge_t*)buf);
    memset(response, 0x00, data_len);

    valid_len = sizeof(g_ycp_sreq_remote_start_charge[gunno].body.serial_number);
    valid_len = valid_len > sizeof(response->body.serial_number) ? sizeof(response->body.serial_number) : valid_len;
    memcpy(response->body.serial_number, g_ycp_sreq_remote_start_charge[gunno].body.serial_number, valid_len);

    response->body.gunno = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_remote_stop_charge
 * 功能          组包：远程停机响应
 * **********************************************/
int8_t ycp_response_padding_remote_stop_charge(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Remote_StopCharge_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YcpPro_PRes_Remote_StopCharge_t *response = NULL;
    response = ((Net_YcpPro_PRes_Remote_StopCharge_t*)buf);
    memset(response, 0x00, data_len);

    response->body.gunno = gunno + 0x01;

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_set_work_para
 * 功能          组包：设置工作参数响应
 * **********************************************/
int8_t ycp_response_padding_set_work_para(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_ParaSet_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_ParaSet_t *response = NULL;
    response = ((Net_YcpPro_PRes_ParaSet_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_remote_reboot
 * 功能          组包：远程重启响应
 * **********************************************/
int8_t ycp_response_padding_remote_reboot(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_RemoteReboot_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_RemoteReboot_t *response = NULL;
    response = ((Net_YcpPro_PRes_RemoteReboot_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_set_billing_model
 * 功能          组包：设置计费模型响应
 * **********************************************/
int8_t ycp_response_padding_set_billing_model(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_BillingModel_Set_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_BillingModel_Set_t *response = NULL;
    response = ((Net_YcpPro_PRes_BillingModel_Set_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_remote_update
 * 功能          组包：远程更新响应
 * **********************************************/
int8_t ycp_response_padding_remote_update(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_RemoteUpdate_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_RemoteUpdate_t *response = NULL;
    response = ((Net_YcpPro_PRes_RemoteUpdate_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_qrcode_config
 * 功能          组包：二维码配置响应
 * **********************************************/
int8_t ycp_response_padding_qrcode_config(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Qrcode_Config_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YcpPro_PRes_Qrcode_Config_t *response = NULL;
    response = ((Net_YcpPro_PRes_Qrcode_Config_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_set_service_phone
 * 功能          组包：设置客服电话响应
 * **********************************************/
int8_t ycp_response_padding_set_service_phone(uint8_t gunno, uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_ServicePhone_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    Net_YcpPro_PRes_ServicePhone_t *response = NULL;
    response = ((Net_YcpPro_PRes_ServicePhone_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_response_padding_modify_server_addr
 * 功能          组包：修改联网地址响应
 * **********************************************/
int8_t ycp_response_padding_modify_server_addr(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint8_t data_len = sizeof(Net_YcpPro_PRes_Modify_ServerAddr_t);

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PRes_Modify_ServerAddr_t *response = NULL;
    response = ((Net_YcpPro_PRes_Modify_ServerAddr_t*)buf);
    memset(response, 0x00, data_len);

    if(olen){
        *olen = data_len;
    }
    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_padding_device_fault
 * 功能          组包：设备故障信息
 * **********************************************/
int8_t ycp_message_padding_device_fault(uint8_t *buf, uint16_t ilen, uint16_t *olen)
{
    uint16_t data_len = sizeof(Net_YcpPro_PReq_Report_DeviceFault_t);

    data_len += ((s_ycp_pfaut_info.fault_num *sizeof(struct ycp_pfaut_node)) + (s_ycp_hfaut_info.fault_num *sizeof(struct ycp_hfaut_node)));

    if(buf == NULL){
        return -0x01;
    }
    if(data_len > ilen){
        return -0x02;
    }

    Net_YcpPro_PReq_Report_DeviceFault_t *message = NULL;
    struct ycp_pfaut_node *_pfault = NULL;
    struct ycp_hfaut_node *_hfault = NULL;

    message = ((Net_YcpPro_PReq_Report_DeviceFault_t*)buf);
    memset(message, 0x00, data_len);

    message->head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
    message->body.fault_num = s_ycp_pfaut_info.fault_num + s_ycp_hfaut_info.fault_num;
    _hfault = (struct ycp_hfaut_node*)((uint8_t*)&message->body.fault_num + 0x01);
    _pfault = (struct ycp_pfaut_node*)((uint8_t*)&message->body.fault_num + 0x01 + (s_ycp_hfaut_info.fault_num *sizeof(struct ycp_hfaut_node)));
    for(uint8_t i = 0x00; i < s_ycp_hfaut_info.fault_num; i++){
        memcpy(&_hfault[i], &s_ycp_hfaut_info.fault[i], sizeof(struct ycp_hfaut_node));
    }
    for(uint8_t i = 0x00; i < s_ycp_pfaut_info.fault_num; i++){
        memcpy(&_pfault[i], &s_ycp_pfaut_info.fault[i], sizeof(struct ycp_pfaut_node));
    }

    if(olen){
        *olen = data_len;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_request_padding_heartbeat
 * 功能          组包：心跳请求
 * **********************************************/
void ycp_request_padding_heartbeat(void)
{
    uint8_t signal = 0x00;
    int32_t temperature = 0x00;
    System_BaseData *base = NULL;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
        temperature = temperature > base->gunline_process_temp[0x00] ? temperature : base->gunline_process_temp[0x00];
        temperature = temperature > base->gunline_process_temp[0x01] ? temperature : base->gunline_process_temp[0x01];
    }
    (void)(s_ycp_handle->get_system_data(NET_SYSTEM_DATA_NAME_SIGNAL_STRENGTH, &signal, sizeof(signal), NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    g_ycp_preq_heartbeat.body.signal_value = signal;

    g_ycp_preq_heartbeat.body.temp = 0xFF;
    g_ycp_preq_heartbeat.body.output_voltage = 0x00;
    g_ycp_preq_heartbeat.body.output_current = 0x00;
    g_ycp_preq_heartbeat.body.input_voltage = 0x00;
    g_ycp_preq_heartbeat.body.input_current = 0x00;
}


/*************************************************
 * 函数名      ycp_message_pro_billing_model_set_response
 * 功能          处理服务器响应(下发)的计费模型
 * **********************************************/
int8_t ycp_message_pro_billing_model_set_response(void *data, uint8_t len, uint8_t is_init)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_BillingModel_Set_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    uint8_t is_updated = NET_ENUM_FALSE;
    System_BaseData *base = NULL;
    ycp_storage_struct *config = (ycp_storage_struct*)(s_ycp_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    Net_YcpPro_SReq_BillingModel_Set_t *response = (Net_YcpPro_SReq_BillingModel_Set_t*)data;

    if(is_init == 0x00){
        if(config == NULL){
            return -0x04;
        }

        config->storage_init_flag = NET_YCP_STORAGE_INIT_FLAG;

        config->model_number = response->body.model_number;
        config->tip_elect_rate = response->body.tip_elect_rate;
        config->tip_service_rate = response->body.tip_service_rate;
        config->peak_elect_rate = response->body.peak_elect_rate;
        config->peak_service_rate = response->body.peak_service_rate;
        config->flat_elect_rate = response->body.flat_elect_rate;
        config->flat_service_rate = response->body.flat_service_rate;
        config->valley_elect_rate = response->body.valley_elect_rate;
        config->valley_service_rate = response->body.valley_service_rate;
        config->loss_proportion = response->body.loss_proportion;
        for(uint8_t count = 0x00; count < NET_YCP_RATE_PERIOD_COUNT_MAX; count++){
            config->rate_number[count] = response->body.rate_number[count];
        }

        if(s_ycp_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT) < 0x00){
            config->storage_init_flag = NET_YCP_STORAGE_INIT_FLAG - 0x01;
            return -0x04;
        }
    }

    net_operation_set_fees_gunno(0xFF, 0x00);    /** 先清除信息 */
    for(uint8_t _gunno = 0x00; _gunno < NET_SYSTEM_GUN_NUMBER; _gunno++){
        uint8_t gunno = _gunno;
        base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
        if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING) ||
                (base->state.current == APP_OFSM_STATE_STOPING)){
            net_operation_set_event(gunno, NET_OPERATION_EVENT_UPDATE_BILLING_RULE);
            gunno = NET_SYSTEM_GUN_NUMBER;
        }else{
            is_updated = NET_ENUM_TRUE;
            net_operation_set_fees_gunno(_gunno, 0x01);
        }

        rt_kprintf("gunno(%d) billingrule info\n", gunno);
        rt_kprintf("ter(%d) tsr(%d) per(%d) psr(%d) fer(%d) fsr(%d) ver(%d) vsr(%d)\n", response->body.tip_elect_rate,
                response->body.tip_service_rate, response->body.peak_elect_rate, response->body.peak_service_rate,
                response->body.flat_elect_rate, response->body.flat_service_rate, response->body.valley_elect_rate,
                response->body.valley_service_rate);

        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_SHARP, (response->body.tip_elect_rate + response->body.tip_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_PEAK, (response->body.peak_elect_rate + response->body.peak_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_FLAT, (response->body.flat_elect_rate + response->body.flat_service_rate) /10);
        app_billingrule_set_rate_price(gunno, APP_RATE_TYPE_VALLEY, (response->body.valley_elect_rate + response->body.valley_service_rate) /10);
        app_billingrule_set_elect_model_sn(gunno, (uint8_t*)&response->body.model_number, sizeof(response->body.model_number));
        app_billingrule_set_service_model_sn(gunno, (uint8_t*)&response->body.model_number, sizeof(response->body.model_number));

        for(uint8_t period = 0x00, count = 0x00; period < APP_BILLING_RULE_PERIOD_MAX; period++, count++){
            app_billingrule_set_period_rate_number(gunno, period, response->body.rate_number[period /0x02]);
            switch(response->body.rate_number[period /0x02]){
            case APP_RATE_TYPE_SHARP :
                app_billingrule_set_period_elect_price(gunno, period, response->body.tip_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, response->body.tip_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_PEAK :
                app_billingrule_set_period_elect_price(gunno, period, response->body.peak_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, response->body.peak_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_FLAT :
                app_billingrule_set_period_elect_price(gunno, period, response->body.flat_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, response->body.flat_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            case APP_RATE_TYPE_VALLEY :
                app_billingrule_set_period_elect_price(gunno, period, response->body.valley_elect_rate /10);
                app_billingrule_set_period_service_price(gunno, period, response->body.valley_service_rate /10);
                app_billingrule_set_period_delay_price(gunno, period, 0x00);
                break;
            default:
                break;
            }
        }
        if(is_updated){
            net_operation_updated_billing_trigger();
        }

    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_pro_apply_charge_active
 * 功能          处理服务器响应的主动申请充电
 * **********************************************/
int8_t ycp_message_pro_apply_charge_active_response(uint8_t gunno, void *data, uint8_t len)
{
#ifndef NET_YCP_AS_MONITOR
    uint8_t data_len = sizeof(Net_YcpPro_SRes_ApplyCharge_Active_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x03;
    }

    if(app_billingrule_is_valid(gunno) == 0x00){
        return NET_YCP_START_FAIL_CODE_IS_BUSY;
    }

    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    Net_YcpPro_SRes_ApplyCharge_Active_t *response = (Net_YcpPro_SRes_ApplyCharge_Active_t*)data;

    valid_len = sizeof(response->body.logic_card_number);
    valid_len = valid_len > sizeof(base->card_number) ? sizeof(base->card_number) : valid_len;
    memset(base->card_number, 0x00, sizeof(base->card_number));
    memcpy(base->card_number, response->body.logic_card_number, valid_len);

    valid_len = sizeof(response->body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
    memcpy(base->transaction_number, response->body.serial_number, valid_len);

    valid_len = sizeof(base->card_number);
    valid_len = valid_len > sizeof(base->user_number) ? sizeof(base->user_number) : valid_len;
    memset(base->user_number, 0x00, sizeof(base->user_number));
    memcpy(base->user_number, base->card_number, valid_len);

    base->account_ballance_before = 0x00;
    base->account_ballance_after = 0x00;

    switch(response->body.strategy){
    case NET_YCP_CHARGE_STRATEGY_TIME:
        base->charge_strategy = APP_CHARGE_STRATEGY_TIME;
        base->charge_strategy_para = response->body.strategy_para *60 /100;
        break;
    case NET_YCP_CHARGE_STRATEGY_ELECT:
        base->charge_strategy = APP_CHARGE_STRATEGY_ELECT;
        base->charge_strategy_para = response->body.strategy_para *10;
        break;
    case NET_YCP_CHARGE_STRATEGY_MONEY:
        base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
        base->charge_strategy_para = response->body.strategy_para *100;
        base->account_ballance_before = response->body.strategy_para;
        base->account_ballance_after = response->body.strategy_para;
        break;
    case NET_YCP_CHARGE_STRATEGY_FULL:
        base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
        base->charge_strategy_para = response->body.strategy_para;
        break;
    default:
        base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
        base->charge_strategy_para = response->body.strategy_para;
        break;
    }

    return 0x00;
#else
    return -0x01;
#endif /* NET_YCP_AS_MONITOR */
}

/*************************************************
 * 函数名      ycp_message_pro_remote_start_charge_request
 * 功能          处理服务器下发的远程启机
 * **********************************************/
int16_t ycp_message_pro_remote_start_charge_request(uint8_t gunno, void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_Remote_StartCharge_t);

    if(data == NULL){
        return -0x01;
    }
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x02;
    }
    if(data_len > len){
        return -0x03;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return NET_YCP_START_FAIL_CODE_IS_BUSY;
    }

    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    Net_YcpPro_SReq_Remote_StartCharge_t *request = (Net_YcpPro_SReq_Remote_StartCharge_t*)data;

    /** 并充时不能让平台启动副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        return NET_YCP_START_FAIL_CODE_IS_CHARGING;
    }

    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
        return NET_YCP_START_FAIL_CODE_NO_GUN;
        break;
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_RESERVATION:
    case APP_OFSM_STATE_FINISHING:

        memset(base->card_number, 0x00, sizeof(base->card_number));
        memset(base->card_uid, 0x00, sizeof(base->card_uid));
        memset(base->user_number, 0x00, sizeof(base->user_number));

        valid_len = sizeof(request->body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(base->transaction_number, 0x00, sizeof(base->transaction_number));
        memcpy(base->transaction_number, request->body.serial_number, valid_len);

        base->account_ballance_before = 0x00;
        base->account_ballance_after = 0x00;

        switch(request->body.strategy){
        case NET_YCP_CHARGE_STRATEGY_TIME:
            base->charge_strategy = APP_CHARGE_STRATEGY_TIME;
            base->charge_strategy_para = request->body.strategy_para *60 /100;
            break;
        case NET_YCP_CHARGE_STRATEGY_ELECT:
            base->charge_strategy = APP_CHARGE_STRATEGY_ELECT;
            base->charge_strategy_para = request->body.strategy_para *10;
            break;
        case NET_YCP_CHARGE_STRATEGY_MONEY:
            base->charge_strategy = APP_CHARGE_STRATEGY_MONEY;
            base->charge_strategy_para = request->body.strategy_para *100;
            base->account_ballance_before = request->body.strategy_para;
            base->account_ballance_after = request->body.strategy_para;
            break;
        case NET_YCP_CHARGE_STRATEGY_FULL:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = request->body.strategy_para;
            break;
        default:
            base->charge_strategy = APP_CHARGE_STRATEGY_FULL;
            base->charge_strategy_para = request->body.strategy_para;
            break;
        }

        s_ycp_flag_info[gunno].is_start_charge = NET_ENUM_TRUE;

        return NET_YCP_START_FAIL_CODE_NONE;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        return NET_YCP_START_FAIL_CODE_IS_CHARGING;
        break;
    default:
        return NET_YCP_START_FAIL_CODE_NO_GUN;
        break;
    }
    return NET_YCP_START_FAIL_CODE_NO_GUN;
}

/*************************************************
 * 函数名      ycp_message_pro_remote_stop_charge_request
 * 功能          处理服务器下发的远程停机
 * **********************************************/
int16_t ycp_message_pro_remote_stop_charge_request(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    /** 并充时不能让平台停止副枪 */
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_LOCAL){
        if(gunno != base->main_gunno){
            return NET_YCP_START_FAIL_CODE_IS_CHARGING;
        }
    }

    if((base->state.current == APP_OFSM_STATE_CHARGING) ||
            (base->state.current == APP_OFSM_STATE_STARTING)){
        s_ycp_flag_info[gunno].is_stop_charge = NET_ENUM_TRUE;

        if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
            net_operation_set_event(base->main_gunno, NET_OPERATION_EVENT_STOP_CHARGE);
        }else{
            net_operation_set_event(gunno, NET_OPERATION_EVENT_STOP_CHARGE);
        }

        return NET_YCP_STOP_FAIL_CODE_NONE;
    }
    return NET_YCP_STOP_FAIL_CODE_NO_CHARGING;
}

/*************************************************
 * 函数名      ycp_message_pro_set_para_request
 * 功能          处理服务器下发的设置桩参数请求
 * **********************************************/
int8_t ycp_message_pro_set_para_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_ParaSet_t);

    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    uint8_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YCP |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    ycp_storage_struct *config = (ycp_storage_struct*)(s_ycp_handle->get_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_TARGET_PLAT));
    Net_YcpPro_SReq_ParaSet_t *request = (Net_YcpPro_SReq_ParaSet_t*)data;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(0x00));

    if(config == NULL){
        return -0x04;
    }

    config->storage_init_flag = NET_YCP_STORAGE_INIT_FLAG;

    config->heartbeat_interval = request->body.heartbeat_interval;
    config->voice_volume = request->body.voice_volume;
    config->device_password = request->body.device_password;
    config->temp_protect = request->body.temp_protect;
    config->chargedata_type = request->body.chargedata_type;
    config->bmsdata_switch = request->body.bmsdata_switch;
    config->power_percent = request->body.power_percent;

#if 0
    if(s_ycp_handle->set_system_data(NET_SYSTEM_DATA_NAME_PLATFORM_DATA, NULL, 0x00, NET_SYSTEM_DATA_OPTION_PLAT_YCP) < 0x00){
        config->storage_init_flag = NET_YCP_STORAGE_INIT_FLAG - 0x01;
        return -0x04;
    }
    /* 此数据由功率设置部分统一调接口存储 */
#endif

    if(request->body.temp_protect == 0xFF){
        uint8_t function = 0x00;
        if(s_ycp_handle->set_system_data(NET_SYSTEM_DATA_NAME_TEMP_PROTECT_SWITCH, &function, sizeof(function), option) < 0x00){
            return -0x04;
        }
    }else{
        uint8_t function = 0x01;
        uint16_t value = request->body.temp_protect;
        if(s_ycp_handle->set_system_data(NET_SYSTEM_DATA_NAME_TEMP_PROTECT_SWITCH, &function, sizeof(function), option) < 0x00){
            return -0x04;
        }
        if(s_ycp_handle->set_system_data(NET_SYSTEM_DATA_NAME_TEMP_PROTECT_STOP_VAL, (uint8_t*)&value, sizeof(value), option) < 0x00){
            return -0x04;
        }
    }

    LOG_D("ycp_message_pro_set_work_para_request(%d, %d, %d)", request->body.power_percent,
            base->system_power_max, (base->system_power_max * request->body.power_percent /100));
    if((request->body.power_percent > 0x00) && (request->body.power_percent <= 0x64)){
        uint32_t power = (base->system_power_max * request->body.power_percent /100);
        net_operation_set_total_power(power, 0x00);
        s_ycp_flag_info[0x00].is_set_power = 0x01;
        base->power_strategy = APP_POWER_STRATEGY_SET_LIMIT;
        base->power_strategy_para = 0x00;
    }else{
        return -0x03;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_pro_time_sync_response
 * 功能          处理服务器的对时响应
 * **********************************************/
int8_t ycp_message_pro_time_sync_response(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SRes_TimeSync_t);
    if(data == NULL){
        return -0x01;
    }
    if(data_len > len){
        return -0x02;
    }

    Net_YcpPro_SRes_TimeSync_t *response = (Net_YcpPro_SRes_TimeSync_t*)data;
    uint32_t timestamp = ycp_timebcd_to_timestamp(response->body.current_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
    s_ycp_handle->time_sync(timestamp);

    LOG_D("ycp_message_pro_time_sync_response(%x)[%x, %x, %x, %x, %x, %x, %x](%x)", timestamp,
            response->body.current_time[0x06], response->body.current_time[0x05],
            response->body.current_time[0x04], response->body.current_time[0x03],
            response->body.current_time[0x02], response->body.current_time[0x01],
            response->body.current_time[0x00], time(NULL));

    net_operation_set_event(0x00, NET_OPERATION_EVENT_TIME_SYNC);

    return 0x00;
}





/*************************************************
 * 函数名      ycp_message_pro_remote_reset_request
 * 功能          处理服务器的远程重启请求
 * **********************************************/
int8_t ycp_message_pro_remote_reset_request(void *data, uint8_t len)
{
    uint8_t data_len = sizeof(Net_YcpPro_SReq_RemoteReboot_t);

    if(data_len > len){
        return -0x02;
    }

    uint8_t gunno = 0x00;
    Net_YcpPro_SReq_RemoteReboot_t *request = (Net_YcpPro_SReq_RemoteReboot_t*)data;
    System_BaseData *base = NULL;

    if(request->body.control_cmd == 0x01){
        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            if(base->state.current != APP_OFSM_STATE_IDLEING){
                break;
            }
        }
    }else if(request->body.control_cmd == 0x00){
        return 0x01;
    }

    if(gunno == NET_SYSTEM_GUN_NUMBER){
        return 0x01;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_message_info_init
 * 功能          越城报文信息初始化
 * **********************************************/
void ycp_message_info_init(uint8_t gunno)  ///////// 这是网络部分外部调用的第一个函数(可以在里面进行相关初始化)
{
    static uint8_t ycp_is_init = 0x00;
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(ycp_is_init){
        return;
    }

    s_ycp_handle = net_get_net_handle();
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    ycp_is_init = 0x01;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ycp_state_data_interval[gunno] = YCP_STATE_DATA_INTERVAL_INIT;
        s_ycp_state_data_count[gunno] = rt_tick_get();
    }
    memset(&s_ycp_flag_info, 0x00, sizeof(s_ycp_flag_info));

    /** 初始化登录签到 */
    g_ycp_preq_login.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
    g_ycp_preq_login.head.sequence = 0x00;

    g_ycp_preq_login.body.pile_type = NET_YCP_PILE_TYPE_DC;
    g_ycp_preq_login.body.gun_count = NET_SYSTEM_GUN_NUMBER;
    g_ycp_preq_login.body.protocol_ver[0x00] = NET_YCP_PROTOCOL_VERSION_REV;
    g_ycp_preq_login.body.protocol_ver[0x01] = NET_YCP_PROTOCOL_VERSION_SUB;
    g_ycp_preq_login.body.protocol_ver[0x02] = NET_YCP_PROTOCOL_VERSION_MAIN;

    memset(g_ycp_preq_login.body.software_ver, '\0', sizeof(g_ycp_preq_login.body.software_ver));
    g_ycp_preq_login.body.software_ver[0] = base->soft_ver_main + '0';
    g_ycp_preq_login.body.software_ver[1] = '.';
    g_ycp_preq_login.body.software_ver[2] = base->soft_ver_sub + '0';
    g_ycp_preq_login.body.software_ver[3] = '.';
    sprintf((char *)&g_ycp_preq_login.body.software_ver[3 + 1], "%02d", base->soft_ver_revise);

    g_ycp_preq_login.body.net_link_type = NET_YCP_NET_LINK_TYPE_SIM;
    memset(g_ycp_preq_login.body.communicate_module_sn, '\0', sizeof(g_ycp_preq_login.body.communicate_module_sn));

    memset(g_ycp_preq_login.body.network_keys, 0, sizeof(g_ycp_preq_login.body.network_keys));
    g_ycp_preq_login.body.network_keys[13] = 0x12;
    g_ycp_preq_login.body.network_keys[14] = 0x34;
    g_ycp_preq_login.body.network_keys[15] = 0x56;

    g_ycp_preq_login.body.transmit_type = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        /** 初始化状态数据响应(实时数据请求) */
        g_ycp_preq_report_state_data[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_report_state_data[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电桩主动申请启动充电请求 */
        g_ycp_preq_apply_charge_active[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_apply_charge_active[gunno].body.gunno = gunno + 0x01;

        /** 初始化交易记录请求 */
        g_ycp_preq_transaction_records[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_transaction_records[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电握手请求 */
        g_ycp_preq_shake_hand[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_shake_hand[gunno].body.gunno = gunno + 0x01;

        /** 初始化参数配置请求 */
        g_ycp_preq_parameter_config[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_parameter_config[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电结束请求 */
        g_ycp_preq_charge_finish[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_charge_finish[gunno].body.gunno = gunno + 0x01;

        /** 初始化错误报文请求 */
        g_ycp_preq_error_message[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_error_message[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程中 BMS 终止请求 */
        g_ycp_preq_bms_end[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_bms_end[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程中充电机终止请求 */
        g_ycp_preq_charger_end[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_charger_end[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程 BMS 需求与充电机输出请求 */
        g_ycp_preq_bmscommand_chargerout[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_bmscommand_chargerout[gunno].body.gunno = gunno + 0x01;

        /** 初始化充电过程 BMS 信息请求 */
        g_ycp_preq_bms_info[gunno].head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
        g_ycp_preq_bms_info[gunno].body.gunno = gunno + 0x01;
    }
    /** 初始化计费模型验证 */
    g_ycp_preq_billing_model_verify.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    /** 初始化升级结果响应 */
    g_ycp_pres_remote_update.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    /** 初始化心跳请求 */
    g_ycp_preq_heartbeat.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;
    g_ycp_preq_heartbeat.body.signal_value = 0x00;
    g_ycp_preq_heartbeat.body.temp = 0xFF;
    g_ycp_preq_heartbeat.body.output_voltage = 0x00;
    g_ycp_preq_heartbeat.body.output_current = 0x00;
    g_ycp_preq_heartbeat.body.input_voltage = 0x00;
    g_ycp_preq_heartbeat.body.input_current = 0x00;

    /** 初始化对时 */
    g_ycp_preq_time_sync.head.flag = NET_YCP_MESSAGE_ENCRYPT_DISABLE;

    /** 平台存储数据校验 */
    ycp_storage_data_check();
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_state_data
 * 功能          充电桩请求报文填报：状态数据
 * **********************************************/
void ycp_chargepile_request_padding_state_data(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(g_ycp_sreq_set_para.body.chargedata_type == 0x01){
        g_ycp_preq_report_state_data[gunno].head.length = ((uint32_t)(g_ycp_preq_report_state_data[gunno].body.serial_number) -  \
                (uint32_t)&(g_ycp_preq_report_state_data[gunno]) - 0x04 + 0x02);
    }else{
        g_ycp_preq_report_state_data[gunno].head.length = sizeof(g_ycp_preq_report_state_data[gunno]) - 0x04;
    }
    if(is_init){
        uint8_t valid_len = sizeof(g_ycp_preq_report_state_data[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ycp_preq_report_state_data[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_report_state_data[gunno].body.serial_number));
        memcpy(g_ycp_preq_report_state_data[gunno].body.serial_number, base->transaction_number, valid_len);

        g_ycp_preq_report_state_data[gunno].body.homing = 0x02;
        g_ycp_preq_report_state_data[gunno].body.output_voltage = 0x00;
        g_ycp_preq_report_state_data[gunno].body.output_current = 0x00;
        g_ycp_preq_report_state_data[gunno].body.gun_temperature = 0x00;
        g_ycp_preq_report_state_data[gunno].body.soc = 0x00;
        g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max = 0x00;
        g_ycp_preq_report_state_data[gunno].body.charge_time = 0x00;
        g_ycp_preq_report_state_data[gunno].body.remain_time = 0x00;
        g_ycp_preq_report_state_data[gunno].body.charge_elect = 0x00;
        g_ycp_preq_report_state_data[gunno].body.loss_elect = 0x00;
        g_ycp_preq_report_state_data[gunno].body.consume_amount = 0x00;

        ycp_chargepile_request_padding_bmscommand_chargerout(gunno, 0x01);
        ycp_chargepile_request_padding_bmsinfo_duringcharge(gunno, 0x01);
    }else{
        if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA) == NET_YCP_SEND_STATE_COMPLETE){
            if((base->state.current == APP_OFSM_STATE_CHARGING) || (base->state.current == APP_OFSM_STATE_STARTING)){
                struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
                int32_t symbol_value = 0x00;

                symbol_value = base->current_a /10;
#ifdef APP_INCLUDE_V2G
                if(base->gun_running_mode == APP_GUN_RUNNING_MODE_V2G){
                    if(symbol_value < 0x00)
                        symbol_value = 0x00 - symbol_value;
                }else{
                    if(symbol_value < 0x00)
                        symbol_value = 0x00;
                }
#endif /* APP_INCLUDE_V2G */
                if(s_ycp_flag_info[gunno].chargedata_mode_last == 0x01){
                    uint8_t valid_len = sizeof(g_ycp_preq_report_state_data[gunno].body.serial_number);
                    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
                    memset(g_ycp_preq_report_state_data[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_report_state_data[gunno].body.serial_number));
                    memcpy(g_ycp_preq_report_state_data[gunno].body.serial_number, base->transaction_number, valid_len);
                }
                g_ycp_preq_report_state_data[gunno].body.output_voltage = base->voltage_a /10;
                g_ycp_preq_report_state_data[gunno].body.output_current = symbol_value;
                g_ycp_preq_report_state_data[gunno].body.gun_temperature = 0x00;
                if(base->state.current == APP_OFSM_STATE_CHARGING){
                    if(base->gunline_process_temp[0] > base->gunline_process_temp[1]){
                        g_ycp_preq_report_state_data[gunno].body.gun_temperature = (base->gunline_process_temp[0] /10 + 0x00);
                    }else{
                        g_ycp_preq_report_state_data[gunno].body.gun_temperature = (base->gunline_process_temp[1] /10 + 0x00);
                    }
                }
                g_ycp_preq_report_state_data[gunno].body.soc = base->current_soc *10;
                g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max = bms->BSM.HigTemp;
                g_ycp_preq_report_state_data[gunno].body.charge_time = base->charge_time /60;
                g_ycp_preq_report_state_data[gunno].body.remain_time = bms->BCS.SurplChgTime;
                g_ycp_preq_report_state_data[gunno].body.charge_elect = base->elect_a *10;
                g_ycp_preq_report_state_data[gunno].body.loss_elect = 0x00;
                g_ycp_preq_report_state_data[gunno].body.consume_amount = base->fees_total;
            }else{
                g_ycp_preq_report_state_data[gunno].body.homing = 0x02;
                g_ycp_preq_report_state_data[gunno].body.output_voltage = 0x00;
                g_ycp_preq_report_state_data[gunno].body.output_current = 0x00;
                g_ycp_preq_report_state_data[gunno].body.gun_temperature = 0x00;
                g_ycp_preq_report_state_data[gunno].body.soc = 0x00;
                g_ycp_preq_report_state_data[gunno].body.battery_group_temp_max = 0x00;
                g_ycp_preq_report_state_data[gunno].body.charge_time = 0x00;
                g_ycp_preq_report_state_data[gunno].body.remain_time = 0x00;
                g_ycp_preq_report_state_data[gunno].body.charge_elect = 0x00;
                g_ycp_preq_report_state_data[gunno].body.loss_elect = 0x00;
                g_ycp_preq_report_state_data[gunno].body.consume_amount = 0x00;
            }
        }
    }

    if(g_ycp_sreq_set_para.body.chargedata_type == 0x01){
        s_ycp_flag_info[gunno].chargedata_mode_last = 0x01;
    }else{
        s_ycp_flag_info[gunno].chargedata_mode_last = 0x02;
    }
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_shakehand
 * 功能          充电桩请求报文填报：BMS握手
 * **********************************************/
void ycp_chargepile_request_padding_bms_shakehand(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }

    uint16_t year;
    uint8_t valid_len = 0x00, yearh, yearl, byteh, bytel;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ycp_preq_shake_hand[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ycp_preq_shake_hand[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_shake_hand[gunno].body.serial_number));
    memcpy(g_ycp_preq_shake_hand[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_protocol_ver, bms->BRM.BMSVer, sizeof(bms->BRM.BMSVer));
    g_ycp_preq_shake_hand[gunno].body.bms_bat_type = bms->BRM.BatType;
    g_ycp_preq_shake_hand[gunno].body.bms_bat_rated_capacity = bms->BRM.BatRateCap;
    g_ycp_preq_shake_hand[gunno].body.bms_bat_rated_volt = bms->BRM.BatRateVolt;
    memset(g_ycp_preq_shake_hand[gunno].body.bms_bat_maker_name, '0', sizeof(g_ycp_preq_shake_hand[gunno].body.bms_bat_maker_name));
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_bat_maker_name, bms->BRM.BatFirm, sizeof(bms->BRM.BatFirm));
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_bat_sn, bms->BRM.SerialNum, sizeof(bms->BRM.SerialNum));

    if(bms->BRM.BatBuldday == 0xFF){
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x00] = 0xFF;
    }else{
        byteh = (uint8_t)(bms->BRM.BatBuldday /10);
        bytel = (uint8_t)(bms->BRM.BatBuldday %10);
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x00] = (uint8_t)(byteh <<0x04) |bytel;
    }

    if(bms->BRM.BatBuldmonth == 0xFF){
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x01] = 0xFF;
    }else{
        byteh = (uint8_t)(bms->BRM.BatBuldmonth /10);
        bytel = (uint8_t)(bms->BRM.BatBuldmonth %10);
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x01] = (uint8_t)(byteh <<0x04) |bytel;
    }

    if(bms->BRM.BatBuldyear == 0xFF){
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x02] = 0xFF;
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x03] = 0xFF;
    }else{
        year = bms->BRM.BatBuldyear + 1985;
        yearh = year /100;
        yearl = year %100;

        byteh = (uint8_t)(yearl /10);
        bytel = (uint8_t)(yearl %10);
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x02] = (uint8_t)(byteh <<0x04) |bytel;

        byteh = (uint8_t)(yearh /10);
        bytel = (uint8_t)(yearh %10);
        g_ycp_preq_shake_hand[gunno].body.bms_bat_date[0x03] = (uint8_t)(byteh <<0x04) |bytel;
    }

    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_bat_charge_num, bms->BRM.Chagtimer, sizeof(bms->BRM.Chagtimer));
    if(bms->BRM.BatProperty <= 0x01){
        g_ycp_preq_shake_hand[gunno].body.bms_bat_title_identification = bms->BRM.BatProperty;
    }else{
        g_ycp_preq_shake_hand[gunno].body.bms_bat_title_identification = 0x01;
    }
    for(uint8_t count = 0x00; count < NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX; count++){
        g_ycp_preq_shake_hand[gunno].body.vin[count] = bms->BRM.CarDiscern[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
    }
    memcpy(g_ycp_preq_shake_hand[gunno].body.bms_software_ver, bms->BRM.BMSVerNum, sizeof(bms->BRM.BMSVerNum));

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGE_SHAKE_HAND);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_paraconfig
 * 功能          充电桩请求报文填报：BMS参数配置
 * **********************************************/
void ycp_chargepile_request_padding_bms_paraconfig(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ycp_preq_parameter_config[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ycp_preq_parameter_config[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_parameter_config[gunno].body.serial_number));
    memcpy(g_ycp_preq_parameter_config[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    g_ycp_preq_parameter_config[gunno].body.bms_single_bat_allow_volt_max = bms->BCP.CellAlowHigVolt;
    g_ycp_preq_parameter_config[gunno].body.bms_allow_curr_max = bms->BCP.AlowCurlt;
    g_ycp_preq_parameter_config[gunno].body.bms_bat_nominal_energy_all = bms->BCP.BatRateKW;
    g_ycp_preq_parameter_config[gunno].body.bms_allow_volt_max = bms->BCP.BatAlowHigVolt;
    g_ycp_preq_parameter_config[gunno].body.bms_allow_temp_max = bms->BCP.BatAlowHigTemp;
    g_ycp_preq_parameter_config[gunno].body.soc = bms->BCP.SOC;
    g_ycp_preq_parameter_config[gunno].body.bms_bat_current_volt = bms->BCP.BatVolt;

    g_ycp_preq_parameter_config[gunno].body.pile_output_volt_max = bms->CML.ChagHigOutVolt;
    g_ycp_preq_parameter_config[gunno].body.pile_output_volt_min = bms->CML.ChagLowOutVolt;

    g_ycp_preq_parameter_config[gunno].body.pile_output_curr_max = bms->CML.ChagHigOutCurlt;
    g_ycp_preq_parameter_config[gunno].body.pile_output_curr_min = bms->CML.ChagLowOutCurlt;

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_PARA_CONFIG);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_chargeend
 * 功能          充电桩请求报文填报：BMS充电结束
 * **********************************************/
void ycp_chargepile_request_padding_bms_chargeend(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ycp_preq_charge_finish[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ycp_preq_charge_finish[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_charge_finish[gunno].body.serial_number));
    memcpy(g_ycp_preq_charge_finish[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    g_ycp_preq_charge_finish[gunno].body.bms_end_soc = bms->BSD.StopSOC *10;
    g_ycp_preq_charge_finish[gunno].body.bms_single_bat_volt_min = bms->BSD.CellLowVolt;
    g_ycp_preq_charge_finish[gunno].body.bms_single_bat_volt_max = bms->BSD.CellHigVolt;
    g_ycp_preq_charge_finish[gunno].body.bms_bat_temp_min = bms->BSD.LowTemp;
    g_ycp_preq_charge_finish[gunno].body.bms_bat_temp_max = bms->BSD.HigTemp;
    g_ycp_preq_charge_finish[gunno].body.total_charge_time = bms->CSD.TotalChgTime *60;
    g_ycp_preq_charge_finish[gunno].body.charge_elect = bms->CSD.OutputKWh;
    memcpy(g_ycp_preq_charge_finish[gunno].body.charger_number, bms->CSD.ChgNum, sizeof(bms->CSD.ChgNum));

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGE_END);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bms_error
 * 功能          充电桩请求报文填报：BMS错误报文
 * **********************************************/
void ycp_chargepile_request_padding_bms_error(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ycp_preq_error_message[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ycp_preq_error_message[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_error_message[gunno].body.serial_number));
    memcpy(g_ycp_preq_error_message[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    g_ycp_preq_error_message[gunno].body.timeout.spn2560_00_identify = bms->BEM.CRM00OVtime;
    g_ycp_preq_error_message[gunno].body.timeout.spn2560_aa_identify = bms->BEM.CRMAAOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_sync_and_output_max = bms->BEM.CTSCMLOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_charge_ready_ok = bms->BEM.CROOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_charge_state = bms->BEM.CCSOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_end_charge = bms->BEM.CSTOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.charger_charge_statistics = bms->BEM.CSDOVtime;

    g_ycp_preq_error_message[gunno].body.timeout.bms_and_car = bms->CEM.BRMOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bat_parameter = bms->CEM.BCPOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bms_charge_ready_ok = bms->CEM.BROOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bat_state_all = bms->CEM.BCSOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bat_charge_conmand = bms->CEM.BCLOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bms_end_charge = bms->CEM.BSTOVtime;
    g_ycp_preq_error_message[gunno].body.timeout.bms_charge_count = bms->CEM.BSDOVtime;

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_ERROR_MESSAGE);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bmsend_duringcharge
 * 功能          充电桩请求报文填报：充电过程中 BMS 终止
 * **********************************************/
void ycp_chargepile_request_padding_bmsend_duringcharge(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ycp_preq_bms_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ycp_preq_bms_end[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_bms_end[gunno].body.serial_number));
    memcpy(g_ycp_preq_bms_end[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    if(bms->BST.SOCGetObj){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x01;
    }else if(bms->BST.VoltGetObj){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x02;
    }else if(bms->BST.CeliVoltGetObj){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x03;
    }else if(bms->BST.ChargInitiStop){
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x04;
    }else{
        g_ycp_preq_bms_end[gunno].body.stop_reason = 0x00;
    }

    if(bms->BST.InsltFault){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.OutConectOVtemp){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.BMSCompOVtemp){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.Conectfault){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.BatOVtemp){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.HVRelaysFault){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.Check2Ft){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else if(bms->BST.OtherFt){
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }else{
        g_ycp_preq_bms_end[gunno].body.stop_fault = 0x00;
    }

    if(bms->BST.OtherFt){
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x00;
    }else if(bms->BST.OverCurlt){
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x01;
    }else if(bms->BST.Voltfault){
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x02;
    }else{
        g_ycp_preq_bms_end[gunno].body.stop_error = 0x00;
    }

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_BMS_STOP);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_chargerend_duringcharge
 * 功能          充电桩请求报文填报：充电过程中充电机终止
 * **********************************************/
void ycp_chargepile_request_padding_chargerend_duringcharge(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
        return;
    }
    uint8_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

    valid_len = sizeof(g_ycp_preq_charger_end[gunno].body.serial_number);
    valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
    memset(g_ycp_preq_charger_end[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_charger_end[gunno].body.serial_number));
    memcpy(g_ycp_preq_charger_end[gunno].body.serial_number, base->transaction_number, valid_len);

    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
        bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
    }
    if(bms->CST.AutoStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x01;
    }else if(bms->CST.ManStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x02;
    }else if(bms->CST.FaultStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x03;
    }else if(bms->CST.BMSInitiStop){
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x04;
    }else{
        g_ycp_preq_charger_end[gunno].body.stop_reason = 0x00;
    }

    if(bms->CST.ChgOVtemp){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.ChgConectfault){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.ChgOVtempIn){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.EnergyTranFault){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.EmgcyStop){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else if(bms->CST.OtherFault){
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }else{
        g_ycp_preq_charger_end[gunno].body.stop_fault = 0x00;
    }

    if(bms->CST.Curltfault){
        g_ycp_preq_charger_end[gunno].body.stop_error = 0x01;
    }else if(bms->CST.Voltfault){
        g_ycp_preq_charger_end[gunno].body.stop_error = 0x02;
    }else{
        g_ycp_preq_charger_end[gunno].body.stop_error = 0x00;
    }

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGER_STOP);
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bmscommand_chargerout
 * 功能          充电桩请求报文填报：充电过程 BMS 需求与充电机输出
 * **********************************************/
void ycp_chargepile_request_padding_bmscommand_chargerout(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(is_init){
        uint8_t valid_len = 0x00;
        System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_volt_command = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_curr_command = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_max_single_bat_volt = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.max_single_bat_volt_gn = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_current_soc = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.bms_remain_charge_time = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.pile_output_volt = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.pile_output_curr = 0x00;
        g_ycp_preq_bmscommand_chargerout[gunno].body.charge_time = 0x00;

        valid_len = sizeof(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number));
        memcpy(g_ycp_preq_bmscommand_chargerout[gunno].body.serial_number, base->transaction_number, valid_len);
    }else{
        if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
            return;
        }
        if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE) == NET_YCP_SEND_STATE_COMPLETE){
            System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            int32_t symbol_value = 0x00;

            symbol_value = (base->current_a /10);
#ifdef APP_INCLUDE_V2G
            if(base->gun_running_mode == APP_GUN_RUNNING_MODE_V2G){
                if(symbol_value < 0x00)
                    symbol_value = 0x00 - symbol_value;
            }else{
                if(symbol_value < 0x00)
                    symbol_value = 0x00;
            }
#endif /* APP_INCLUDE_V2G */
            g_ycp_preq_bmscommand_chargerout[gunno].body.pile_output_volt = (base->voltage_a /10);
            g_ycp_preq_bmscommand_chargerout[gunno].body.pile_output_curr = symbol_value;
            g_ycp_preq_bmscommand_chargerout[gunno].body.charge_time = base->charge_time;

            if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_volt_command = bms->BCL.BMSneedVolt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_curr_command = bms->BCL.BMSneedCurlt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_charge_mode = bms->BCL.ChagModel;

            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_volt_measure_value = bms->BCS.ChargVolt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_curr_measure_value = bms->BCS.ChargCurlt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_max_single_bat_volt = bms->BCS.CellHigVolt;
            g_ycp_preq_bmscommand_chargerout[gunno].body.max_single_bat_volt_gn = bms->BCS.HigVoltCellNum;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_current_soc = bms->BCS.SOC *10;
            g_ycp_preq_bmscommand_chargerout[gunno].body.bms_remain_charge_time = bms->BCS.SurplChgTime *60;
        }
    }
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_bmsinfo_duringcharge
 * 功能          充电桩请求报文填报：充电过程 BMS 信息
 * **********************************************/
void ycp_chargepile_request_padding_bmsinfo_duringcharge(uint8_t gunno, uint8_t is_init)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(is_init){
        uint8_t valid_len = 0x00;
        System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

        g_ycp_preq_bms_info[gunno].body.bms_max_single_volt_bat_number = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_temp_max = 0;
        g_ycp_preq_bms_info[gunno].body.bat_temp_max_measure_number = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_temp_min = 0;
        g_ycp_preq_bms_info[gunno].body.bat_temp_min_measure_number = 0;
        g_ycp_preq_bms_info[gunno].body.bms_single_volt = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_soc = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_curr = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_temp = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_isolate = 0;
        g_ycp_preq_bms_info[gunno].body.bms_bat_output_linker = 0;
        g_ycp_preq_bms_info[gunno].body.charge_forbid = 0;

        valid_len = sizeof(g_ycp_preq_bms_info[gunno].body.serial_number);
        valid_len = valid_len > sizeof(base->transaction_number) ? sizeof(base->transaction_number) : valid_len;
        memset(g_ycp_preq_bms_info[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_bms_info[gunno].body.serial_number));
        memcpy(g_ycp_preq_bms_info[gunno].body.serial_number, base->transaction_number, valid_len);
    }else{
        if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x00){
            return;
        }
        if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_BMS_INFO) == NET_YCP_SEND_STATE_COMPLETE){
            System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);

            if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
                base = (System_BaseData*)(s_ycp_handle->get_base_data(base->main_gunno));
                bms = (struct thaisenBMS_Charger_struct*)(base->bms_data);
            }
            g_ycp_preq_bms_info[gunno].body.bms_max_single_volt_bat_number = bms->BSM.HigVoltCellNum;
            g_ycp_preq_bms_info[gunno].body.bms_bat_temp_max = bms->BSM.HigTemp *10;
            g_ycp_preq_bms_info[gunno].body.bat_temp_max_measure_number = bms->BSM.HigTempNum;
            g_ycp_preq_bms_info[gunno].body.bms_bat_temp_min = bms->BSM.LowTemp *10;
            g_ycp_preq_bms_info[gunno].body.bat_temp_min_measure_number = bms->BSM.LowTempNum;
            g_ycp_preq_bms_info[gunno].body.bms_single_volt = bms->BSM.CellOverVolt;
            g_ycp_preq_bms_info[gunno].body.bms_bat_soc = bms->BSM.SOCState;
            g_ycp_preq_bms_info[gunno].body.bms_bat_curr = bms->BSM.BatOverCurlt;
            g_ycp_preq_bms_info[gunno].body.bms_bat_temp = bms->BSM.BatOverTemp;
            g_ycp_preq_bms_info[gunno].body.bms_bat_isolate = bms->BSM.Insulat;
            g_ycp_preq_bms_info[gunno].body.bms_bat_output_linker = bms->BSM.OutConect;
            g_ycp_preq_bms_info[gunno].body.charge_forbid = bms->BSM.AllowChg;
        }
    }
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_card_authority
 * 功能          充电桩请求报文填报：刷卡权限认证
 * **********************************************/
int8_t ycp_chargepile_request_padding_card_authority(uint8_t gunno)
{
#ifndef NET_YCP_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }
    if(ycp_get_socket_info()->socket_state != YCP_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    uint8_t valid_len = 0x00, ascii_valid_len = 0x00, value = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(!base->flag.card_info_is_uid){
        return -0x01;
    }
    g_ycp_preq_apply_charge_active[gunno].body.start_type = 0x01;

    ascii_valid_len = base->card_uid_len *0x02;
    ascii_valid_len = ascii_valid_len > (NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01) ? (NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01) : ascii_valid_len;
    ascii_valid_len -= (ascii_valid_len %0x02);

    valid_len = base->card_uid_len;
    valid_len = valid_len > (NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01) ? (NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01) : valid_len;
    memset(g_ycp_preq_apply_charge_active[gunno].body.phycard_number, ' ', (NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01));

    for(uint8_t i = 0x00, j = 0x00; (i < valid_len) && (j < ascii_valid_len); i++, j += 0x02){
        value = ((base->card_uid[i]) &0x0F);
        if(value > 0x09){
            value += ('A' - (0x09 + 0x01));
        }else{
            value += '0';
        }
        g_ycp_preq_apply_charge_active[gunno].body.phycard_number[j] = value;

        value = (((base->card_uid[i]) &0xF0) >>0x04);
        if(value > 0x09){
            value += ('A' - (0x09 + 0x01));
        }else{
            value += '0';
        }
        g_ycp_preq_apply_charge_active[gunno].body.phycard_number[j + 0x01] = value;
    }

    memset(g_ycp_preq_apply_charge_active[gunno].body.password, 0x00, NET_YCP_PASSWORD_LENGTH_DEFAULT);

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YCP_AS_MONITOR */
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_vin_authority
 * 功能          充电桩请求报文填报：VIN 码权限认证
 * **********************************************/
int8_t ycp_chargepile_request_padding_vin_authority(uint8_t gunno)
{
#ifndef NET_YCP_AS_MONITOR
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if(app_billingrule_is_valid(gunno) == 0x00){
        return -0x01;
    }
    if(ycp_get_socket_info()->socket_state != YCP_SOCKET_STATE_LOGIN_SUCCESS){
        return -0x01;
    }
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    g_ycp_preq_apply_charge_active[gunno].body.start_type = 0x03;
    memset(g_ycp_preq_apply_charge_active[gunno].body.phycard_number, 0x00, (NET_YCP_PHYCARD_NUMBER_LENGTH_MAX + 0x01));
    memset(g_ycp_preq_apply_charge_active[gunno].body.password, 0x00, NET_YCP_PASSWORD_LENGTH_DEFAULT);
    for(uint8_t count = 0x00; count < NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX; count++){
        g_ycp_preq_apply_charge_active[gunno].body.phycard_number[count] = base->car_vin[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
    }

    s_ycp_flag_info[gunno].is_vin_authorized = NET_ENUM_TRUE;

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);

    return 0x00;
#else
    return -0x01;
#endif /* NET_YCP_AS_MONITOR */
}

/*************************************************
 * 函数名      ycp_chargepile_request_padding_transaction_record
 * 功能          充电桩请求报文填报：交易记录信息
 * **********************************************/
uint8_t ycp_chargepile_request_padding_transaction_record(uint8_t gunno, void *transaction, uint8_t is_repeat)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    uint8_t valid_len = 0x00;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)transaction;

    ycp_set_transaction_verify_state(gunno, 0x00);
    if((ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD) == NET_YCP_SEND_STATE_COMPLETE)){
        valid_len = sizeof(g_ycp_preq_transaction_records[gunno].body.serial_number);
        valid_len = valid_len > sizeof(_transaction->serial_number) ? sizeof(_transaction->serial_number) : valid_len;
        memset(g_ycp_preq_transaction_records[gunno].body.serial_number, 0x00, sizeof(g_ycp_preq_transaction_records[gunno].body.serial_number));
        memcpy(g_ycp_preq_transaction_records[gunno].body.serial_number, _transaction->serial_number, valid_len);

        if(_transaction->end_time > _transaction->charge_time){
            _transaction->start_time = _transaction->end_time - _transaction->charge_time;
        }else{
            _transaction->start_time = _transaction->end_time;
            _transaction->charge_time = 0x00;
        }
        ycp_timestamp_to_timebcd(_transaction->start_time, g_ycp_preq_transaction_records[gunno].body.start_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
        ycp_timestamp_to_timebcd(_transaction->end_time, g_ycp_preq_transaction_records[gunno].body.end_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);

        g_ycp_preq_transaction_records[gunno].body.tip_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_SHARP] *10;
        g_ycp_preq_transaction_records[gunno].body.tip_elect = _transaction->rate_type_elect[APP_RATE_TYPE_SHARP] *10;
        g_ycp_preq_transaction_records[gunno].body.tip_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_SHARP] *10;
        if(g_ycp_preq_transaction_records[gunno].body.tip_elect > YCP_CHARGE_ELECT_MAX *10){
            g_ycp_preq_transaction_records[gunno].body.tip_elect = YCP_CHARGE_ELECT_MAX *10;
        }

        g_ycp_preq_transaction_records[gunno].body.peak_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_PEAK] *10;
        g_ycp_preq_transaction_records[gunno].body.peak_elect = _transaction->rate_type_elect[APP_RATE_TYPE_PEAK] *10;
        g_ycp_preq_transaction_records[gunno].body.peak_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_PEAK] *10;
        if(g_ycp_preq_transaction_records[gunno].body.peak_elect > YCP_CHARGE_ELECT_MAX *10){
            g_ycp_preq_transaction_records[gunno].body.peak_elect = YCP_CHARGE_ELECT_MAX *10;
        }

        g_ycp_preq_transaction_records[gunno].body.flat_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_FLAT] *10;
        g_ycp_preq_transaction_records[gunno].body.flat_elect = _transaction->rate_type_elect[APP_RATE_TYPE_FLAT] *10;
        g_ycp_preq_transaction_records[gunno].body.flat_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_FLAT] *10;
        if(g_ycp_preq_transaction_records[gunno].body.flat_elect > YCP_CHARGE_ELECT_MAX *10){
            g_ycp_preq_transaction_records[gunno].body.flat_elect = YCP_CHARGE_ELECT_MAX *10;
        }

        g_ycp_preq_transaction_records[gunno].body.valley_unit_price = _transaction->rate_type_unit[APP_RATE_TYPE_VALLEY] *10;
        g_ycp_preq_transaction_records[gunno].body.valley_elect = _transaction->rate_type_elect[APP_RATE_TYPE_VALLEY] *10;
        g_ycp_preq_transaction_records[gunno].body.valley_loss_elect = _transaction->rate_type_loss_elect[APP_RATE_TYPE_VALLEY] *10;
        if(g_ycp_preq_transaction_records[gunno].body.valley_elect > YCP_CHARGE_ELECT_MAX *10){
            g_ycp_preq_transaction_records[gunno].body.valley_elect = YCP_CHARGE_ELECT_MAX *10;
        }

        g_ycp_preq_transaction_records[gunno].body.consume_amount = _transaction->total_fee;
        if(g_ycp_preq_transaction_records[gunno].body.consume_amount > YCP_SPEND_AMOUNT_MAX){
            g_ycp_preq_transaction_records[gunno].body.consume_amount = YCP_SPEND_AMOUNT_MAX;
        }

        valid_len = sizeof(_transaction->car_vin);
        valid_len = valid_len > NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX ? NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX : valid_len;
        for(uint8_t count = 0x00; count < NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX; count++){
            g_ycp_preq_transaction_records[gunno].body.vin[count] = _transaction->car_vin[NET_YCP_CAR_VIN_NUMBER_LENGTH_MAX - count - 0x01];
        }

        g_ycp_preq_transaction_records[gunno].body.start_way = ycp_chargepile_transaction_identity_converted(_transaction->start_type);
        g_ycp_preq_transaction_records[gunno].body.stop_reason = ycp_chargepile_stop_reason_converted(_transaction, _transaction->stop_reason, _transaction->order_info.is_start_fail);

        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
        return 0x01;
    }

    return 0x00;
}

/*************************************************
 * 函数名      ycp_start_charge_response_asynchronously
 * 功能          充电桩报文响应事件：启动充电异步响应
 * **********************************************/
void ycp_start_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    if(s_ycp_flag_info[gunno].is_start_charge == 0x00){
        return;
    }
    if(result){
        s_ycp_flag_info[gunno].start_success = 0x01;
    }else{
        s_ycp_flag_info[gunno].start_success = 0x00;
    }

    s_ycp_flag_info[gunno].is_start_charge = 0x00;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_START_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ycp_stop_charge_response_asynchronously
 * 功能          充电桩报文响应事件：停止充电异步响应
 * **********************************************/
void ycp_stop_charge_response_asynchronously(uint8_t gunno, uint8_t result)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
    if(base->charge_way == APP_CHARGE_WAY_PARACHARGE_CLOUD){
        gunno = base->main_gunno;
    }
    if(s_ycp_flag_info[gunno].is_stop_charge == 0x00){
        return;
    }
    if(result){
        s_ycp_flag_info[gunno].stop_success = 0x01;
    }else{
        s_ycp_flag_info[gunno].stop_success = 0x00;
    }

    s_ycp_flag_info[gunno].is_stop_charge = 0x00;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, gunno, NET_YCP_PRES_EVENT_STOP_CHARGE_ASYNCHRONOUSLY);
}

/*************************************************
 * 函数名      ycp_set_power_percent_response_asynchronously
 * 功能          充电桩报文响应事件：设置功率百分比响应
 * **********************************************/
void ycp_set_power_percent_response_asynchronously(uint8_t result)
{
    if(s_ycp_flag_info[0x00].is_set_power == 0x00){
        return;
    }
    if(result){
        s_ycp_flag_info[0x00].set_power_success = 0x01;
    }else{
        s_ycp_flag_info[0x00].set_power_success = 0x00;
    }

    s_ycp_flag_info[0x00].is_set_power = 0x00;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_PRES_EVENT_SET_POWER_PERCENT_ASYNCHRONOUSLY);
}

void ycp_chargepile_state_changed(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    uint8_t state = 0xFF;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    if(base->flag.connect_state == APP_CONNECT_STATE_CONNECT){
        s_ycp_disposable_info[gunno].state.connect = 0x01;
    }else{
        s_ycp_disposable_info[gunno].state.connect = 0x00;
    }

    switch(base->state.current){
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_RESERVATION:
    case APP_OFSM_STATE_STARTING:
        state = 0x02;
        break;
    case APP_OFSM_STATE_CHARGING:
        state = 0x03;
        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        state = 0x02;
        break;
    case APP_OFSM_STATE_FAULTING:
        state = 0x01;
        break;
    default:
        break;
    }

    if(state == 0xFF){
        return;
    }

    s_ycp_disposable_info[gunno].state.state = state;
    if((s_ycp_disposable_info[gunno].state.state == g_ycp_preq_report_state_data[gunno].body.state) &&
            (s_ycp_disposable_info[gunno].state.connect == g_ycp_preq_report_state_data[gunno].body.plug_gun)){
        return;
    }
    if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA) == NET_YCP_SEND_STATE_ONGOING){
        ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_STATE, 0x00);
        return;
    }

    ycp_chargepile_request_padding_state_data(gunno, 0x00);
    g_ycp_preq_report_state_data[gunno].body.plug_gun = s_ycp_disposable_info[gunno].state.connect;
    g_ycp_preq_report_state_data[gunno].body.state = s_ycp_disposable_info[gunno].state.state;

    ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_STATE, 0x01);
    s_ycp_state_data_count[gunno] = rt_tick_get();
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
}

void ycp_chargepile_update_result_report(uint8_t result, uint8_t reason)
{
    g_ycp_pres_remote_update.body.result = result;
    g_ycp_pres_remote_update.body.reason = reason;
    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_RESPONSE, 0x00, NET_YCP_PRES_EVENT_REMOTE_UPDATE);
}

int8_t ycp_chargepile_fault_report(uint8_t gunno, uint16_t code, uint8_t is_resume, uint8_t rank)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }

    for(uint8_t i = 0x00; i < NET_SYSTEM_GUN_NUMBER; i++){
        if(ycp_get_message_send_state(i, NET_YCP_PREQ_EVENT_REPORT_DEVICE_FAULT) == NET_YCP_SEND_STATE_ONGOING){
            return -0x01;
        }
    }

    if(is_resume){
        uint8_t pos = 0x00, is_matched = NET_ENUM_FALSE;
        uint16_t temp = (uint8_t)((code &0xff00) >>0x08);
        temp |= (uint16_t)(code <<0x08);

        /** 这是枪口故障 */
        if(rank == NET_YCP_FAULT_CODE_TYPE_PORT){
            for(pos = 0x00; pos < s_ycp_pfaut_info.fault_num; pos++){
                if(temp == s_ycp_pfaut_info.fault[pos].fault_code){
                    for(uint8_t i = pos; i < s_ycp_pfaut_info.fault_num; i++){
                        if((i + 0x01) < s_ycp_pfaut_info.fault_num){
                            s_ycp_pfaut_info.fault[i].fault_type = s_ycp_pfaut_info.fault[i + 0x01].fault_type;
                            s_ycp_pfaut_info.fault[i].gunno = s_ycp_pfaut_info.fault[i + 0x01].gunno;
                            s_ycp_pfaut_info.fault[i].fault_code = s_ycp_pfaut_info.fault[i + 0x01].fault_code;
                        }
                    }

                    if(s_ycp_pfaut_info.fault_num > 0x00){
                        memset(&(s_ycp_pfaut_info.fault[s_ycp_pfaut_info.fault_num - 0x01]), 0x00, \
                                sizeof(s_ycp_pfaut_info.fault[s_ycp_pfaut_info.fault_num - 0x01]));
                    }else{
                        memset(&(s_ycp_pfaut_info.fault[0x00]), 0x00, \
                                sizeof(s_ycp_pfaut_info.fault[0x00]));
                    }

                    if(s_ycp_pfaut_info.fault_num > 0x00){
                        s_ycp_pfaut_info.fault_num--;
                    }
                    is_matched = NET_ENUM_TRUE;
                    break;
                }
            }
            if(is_matched == NET_ENUM_FALSE){
                return 0x00;
            }
        }
        /** 这是主机故障 */
        else{
            for(pos = 0x00; pos < s_ycp_hfaut_info.fault_num; pos++){
                if(temp == s_ycp_hfaut_info.fault[pos].fault_code){
                    for(uint8_t i = pos; i < s_ycp_hfaut_info.fault_num; i++){
                        if((i + 0x01) < s_ycp_hfaut_info.fault_num){
                            s_ycp_hfaut_info.fault[i].fault_type = s_ycp_hfaut_info.fault[i + 0x01].fault_type;
                            s_ycp_hfaut_info.fault[i].fault_code = s_ycp_hfaut_info.fault[i + 0x01].fault_code;
                        }
                    }

                    if(s_ycp_hfaut_info.fault_num > 0x00){
                        memset(&(s_ycp_hfaut_info.fault[s_ycp_hfaut_info.fault_num - 0x01]), 0x00, \
                                sizeof(s_ycp_hfaut_info.fault[s_ycp_hfaut_info.fault_num - 0x01]));
                    }else{
                        memset(&(s_ycp_hfaut_info.fault[0x00]), 0x00, \
                                sizeof(s_ycp_hfaut_info.fault[0x00]));
                    }

                    if(s_ycp_hfaut_info.fault_num > 0x00){
                        s_ycp_hfaut_info.fault_num--;
                    }
                    is_matched = NET_ENUM_TRUE;
                    break;
                }
            }
            if(is_matched == NET_ENUM_FALSE){
                return 0x00;
            }
        }
    }else{
        uint16_t temp = (uint8_t)((code &0xff00) >>0x08);
        uint8_t index = 0x00;

        temp |= (uint16_t)(code <<0x08);
        /** 这是枪口故障 */
        if(rank == NET_YCP_FAULT_CODE_TYPE_PORT){
            index = s_ycp_pfaut_info.fault_num;
            for(uint8_t pos = 0x00; pos < s_ycp_pfaut_info.fault_num; pos++){
                if((temp == s_ycp_pfaut_info.fault[pos].fault_code) &&
                        ((gunno + 0x01) == s_ycp_pfaut_info.fault[pos].gunno)){
                    return 0x00;
                }
            }
            if(index >= YCP_PORT_FAULT_NUM_MAX_DEFAULT){
                return -0x01;
            }

            s_ycp_pfaut_info.fault[index].fault_type = rank;
            s_ycp_pfaut_info.fault[index].gunno = gunno + 0x01;
            s_ycp_pfaut_info.fault[index].fault_code = temp;
            s_ycp_pfaut_info.fault_num++;
        }
        /** 这是主机故障 */
        else{
            index = s_ycp_hfaut_info.fault_num;
            for(uint8_t pos = 0x00; pos < s_ycp_hfaut_info.fault_num; pos++){
                if(temp == s_ycp_hfaut_info.fault[pos].fault_code){
                    return 0x00;
                }
            }
            if(index >= YCP_HOST_FAULT_NUM_MAX_DEFAULT){
                return -0x01;
            }

            s_ycp_hfaut_info.fault[index].fault_type = rank;
            s_ycp_hfaut_info.fault[index].fault_code = temp;
            s_ycp_hfaut_info.fault_num++;
        }
    }

    s_ycp_fault_report = rt_tick_get();

    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_PADDING_DEVICE_FAULT);

    return 0x00;
}

/*************************************************
 * 函数名      ycp_chargepile_create_local_transaction_number
 * 功能          创建本地交易号
 * **********************************************/
int8_t ycp_chargepile_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return -0x01;
    }
    if((vector == NULL) || (len == 0x00)){
        return -0x02;
    }
    if(len < NET_YCP_SERIAL_NUMBER_LENGTH_DEFAULT){
        return -0x03;
    }

    uint32_t option = (NET_SYSTEM_DATA_OPTION_PLAT_YCP |NET_SYSTEM_DATA_OPTION_DATA_CONTENT);
    uint8_t *pile_number = NULL;
    uint8_t sn_len = 0x00, *ptr = (uint8_t*)vector;
    struct tm _tm;
    uint16_t valid_len = 0x00;
    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    s_ycp_handle = net_get_net_handle();
    pile_number = (uint8_t*)(s_ycp_handle->get_system_data(NET_SYSTEM_DATA_NAME_PILE_NUMBER, NULL, 0x00, option));
    valid_len = strlen((char*)pile_number);
    valid_len = valid_len > (NET_YCP_CHARGEPILE_LENGTH_DEFAULT *0x02) ? (NET_YCP_CHARGEPILE_LENGTH_DEFAULT *0x02) : valid_len;
    ycp_ascii_to_bcd(pile_number, valid_len, ptr, NET_YCP_CHARGEPILE_LENGTH_DEFAULT, 0x01);

    s_ycp_local_start_sq++;
    sn_len = 0x07;

    ycp_enter_critical();
    _tm = *(localtime((const time_t*)&(base->current_time)));
    ycp_exit_critical();

    LOG_D("ycp_chargepile_create_local_transaction_number[%d](%d, %d, %d, %d, %d)", base->current_time, _tm.tm_year, _tm.tm_mon,
            _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);

    ptr[sn_len++] = gunno + 1;                                                     /* 枪号 */
    ptr[sn_len++] = (((_tm.tm_year - 100) /10) *16) + ((_tm.tm_year - 100) %10); /* 年 */
    ptr[sn_len++] = (((_tm.tm_mon + 0x01) /10) *16) + ((_tm.tm_mon + 0x01) %10); /* 月 */
    ptr[sn_len++] = ((_tm.tm_mday /10) *16) +  (_tm.tm_mday %10);                /* 日 */
    ptr[sn_len++] = ((_tm.tm_hour /10) *16) + (_tm.tm_hour %10);                 /* 时 */
    ptr[sn_len++] = ((_tm.tm_min /10) *16) + (_tm.tm_min %10);                   /* 分 */
    ptr[sn_len++] = ((_tm.tm_sec /10) *16) + (_tm.tm_sec %10);                   /* 秒 */
    memcpy((ptr + sn_len), &s_ycp_local_start_sq, sizeof(s_ycp_local_start_sq));   /* 自增序列号 */


    return 0x00;
}

/*************************************************
 * 函数名      ycp_chargepile_time_sync_revise
 * 功能          时间同步修正
 * **********************************************/
void ycp_chargepile_time_sync_revise(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    System_BaseData *base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));

    ycp_timestamp_to_timebcd(base->start_time, g_ycp_preq_transaction_records[gunno].body.start_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
    ycp_timestamp_to_timebcd(base->stop_time, g_ycp_preq_transaction_records[gunno].body.end_time, NET_YCP_TIME_BCD_LENGTH_DEFAULT);
}

/*************************************************
 * 函数名      ycp_chargepile_fault_converted
 * 功能          故障转换
 * **********************************************/
uint16_t ycp_chargepile_fault_converted(uint16_t bit)
{
    switch(bit){
    case APP_SYS_FAULT_SCRAM :
        return NET_GENERAL_FAULT_SCRAM;
    case APP_SYS_FAULT_CARD_READER :
        return NET_GENERAL_FAULT_CARD_READER;
    case APP_SYS_FAULT_DOOR :
        return NET_GENERAL_FAULT_DOOR;
    case APP_SYS_FAULT_AMMETER :
        return NET_GENERAL_FAULT_AMMETER;
    case APP_SYS_FAULT_CHARGE_MODULE :
        return NET_GENERAL_FAULT_CHARGE_MODULE;
    case APP_SYS_FAULT_OVER_TEMP :
        return NET_GENERAL_FAULT_OVER_TEMP;
    case APP_SYS_FAULT_OVER_VOLT :
        return NET_GENERAL_FAULT_OVER_VOLT;
    case APP_SYS_FAULT_UNDER_VOLT :
        return NET_GENERAL_FAULT_UNDER_VOLT;
    case APP_SYS_FAULT_OVER_CURR :
        return NET_GENERAL_FAULT_OVER_CURR;
    case APP_SYS_FAULT_RELAY :
        return NET_GENERAL_FAULT_MAIN_RELAY;
    case APP_SYS_FAULT_PARALLEL_RELAY :
        return NET_GENERAL_FAULT_PARALLEL_RELAY;
    case APP_SYS_FAULT_AC_RELAY :
        return NET_GENERAL_FAULT_AC_RELAY;
    case APP_SYS_FAULT_ELOCK :
        return NET_GENERAL_FAULT_ELOCK;
    case APP_SYS_FAULT_AUXPOWER :
        return NET_GENERAL_FAULT_AUXPOWER;
    case APP_SYS_FAULT_FLASH :
        return NET_GENERAL_FAULT_FLASH;
    case APP_SYS_FAULT_EEPROM :
        return NET_GENERAL_FAULT_EEPROM;
    case APP_SYS_FAULT_LIGHT_PRPTECT :
        return NET_GENERAL_FAULT_LIGHT_PRPTECT;
    case APP_SYS_FAULT_GUN_SITE :
        return NET_GENERAL_FAULT_GUN_SITE;
    case APP_SYS_FAULT_CIRCUIT_BREAKER :
        return NET_GENERAL_FAULT_CIRCUIT_BREAKER;
    case APP_SYS_FAULT_FLOODING :
        return NET_GENERAL_FAULT_FLOODING;
    case APP_SYS_FAULT_SMOKE :
        return NET_GENERAL_FAULT_SMOKE;
    case APP_SYS_FAULT_POUR :
        return NET_GENERAL_FAULT_POUR;
    case APP_SYS_FAULT_LIQUID_COOLING :
        return NET_GENERAL_FAULT_LIQUID_COOLING;
    case APP_SYS_FAULT_FUSE :
        return NET_GENERAL_FAULT_FUSE;
    case APP_SYS_FAULT_MAIN_CABINET_OFFLINE :
        return NET_GENERAL_FAULT_MAIN_CABINET_OFFLINE;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_1 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_1;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_2 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_2;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN1_3 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN1_3;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN2_1 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_1;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN2_2 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN2_2;
    case APP_SYSTEM_FAULT_MATRIX_RELAY_KPN3_1 :
        return NET_GENERAL_FAULT_MATRIX_RELAY_KPN3_1;
    case APP_SYSTEM_FAULT_SLAVE_DEVICE_OFFLINE :
        return NET_GENERAL_FAULT_SLAVE_DEVICE_OFFLINE;
    case APP_SYSTEM_FAULT_FAN :
        return NET_GENERAL_FAULT_FAN;
    case APP_SYSTEM_FAULT_MAINCABINET_SCRAM :
        return NET_GENERAL_FAULT_MAINCABINET_SCRAM;
    case APP_SYSTEM_FAULT_MAINCABINET_GATE :
        return NET_GENERAL_FAULT_MAINCABINET_GATE;
    case APP_SYSTEM_FAULT_MAINCABINET_PDUFAULT :
        return NET_GENERAL_FAULT_MAINCABINET_PDUFAULT;
    case APP_SYSTEM_FAULT_MAINCABINET_MODULEFAULT :
        return NET_GENERAL_FAULT_MAINCABINET_MODULEFAULT;
    case APP_SYSTEM_FAULT_MAINCABINET_CONFIG :
        return NET_GENERAL_FAULT_MAINCABINET_CONFIG;
    case APP_SYSTEM_FAULT_MAINCABINET_ACRELAY :
        return NET_GENERAL_FAULT_MAINCABINET_ACRELAY;
    case APP_SYSTEM_FAULT_MAINCABINET_SMOKE :
        return NET_GENERAL_FAULT_MAINCABINET_SMOKE;
    case APP_SYSTEM_FAULT_MAINCABINET_POUR :
        return NET_GENERAL_FAULT_MAINCABINET_POUR;
    case APP_SYSTEM_FAULT_MAINCABINET_FLOODING :
        return NET_GENERAL_FAULT_MAINCABINET_FLOODING;
    case APP_SYSTEM_FAULT_DEVICE_IS_LOCKED :
        return NET_GENERAL_FAULT_DEVICE_IS_LOCKED;
    default:
        return NET_GENERAL_FAULT_SIZE;
    }
    return NET_GENERAL_FAULT_SIZE;
}

/*************************************************
 * 函数名      ycp_chargepile_transaction_identity_converted
 * 功能          交易标识转换
 * **********************************************/
static uint8_t ycp_chargepile_transaction_identity_converted(uint8_t identity)
{
    switch(identity){
    case APP_CHARGE_START_WAY_APP:
        return NETYCP_START_WAY_APP;
        break;
    case APP_CHARGE_START_WAY_ONLINE_CARD:
    case APP_CHARGE_START_WAY_OFFLINE_CARD:
        return NETYCP_START_WAY_SWIP_CARD;
        break;
    case APP_CHARGE_START_WAY_VIN:
        return NETYCP_START_WAY_VIN;
        break;
    default:
        break;
    }
    return NETYCP_START_WAY_SWIP_CARD;
}

/*************************************************
 * 函数名      ycp_chargepile_stop_reason_converted
 * 功能          停充原因转换
 * **********************************************/
static uint16_t ycp_chargepile_stop_reason_converted(void *handle, uint16_t reason, uint8_t stop_in_starting)
{
    uint16_t _reason = NETYCP_AS_REASONFF_UNKNOW;
    thaisen_transaction_t *_transaction = (thaisen_transaction_t*)handle;

    switch(reason){
    /* 急停 */
    case APP_SYSTEM_STOP_WAY_SCRAM:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON16_EMERGENCY_STOP;
        }else{
            _reason = NETYCP_AS_REASON57_EMERGENCY_STOP;
        }
        break;
    /* 读卡器 */
    case APP_SYSTEM_STOP_WAY_CARDREADER:
        _reason = NETYCP_AS_REASON6F_CARDREADER;
        break;
    /* 门禁 */
    case APP_SYSTEM_STOP_WAY_DOOR:
        _reason = NETYCP_AS_REASON75_GATE;
        break;
    /* 电表 */
    case APP_SYSTEM_STOP_WAY_AMMETER:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON13_AMMETER_COMMUNICATION;
        }else{
            _reason = NETYCP_AS_REASON53_AMMETER_COMMUNICATE;
        }
        break;
    /* 充电模块 */
    case APP_SYSTEM_STOP_WAY_CHARGEMODULE:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON15_CHARGE_MODULE;
        }else{
            _reason = NETYCP_AS_REASON56_CHARGE_MODULE;
        }
        break;
    /* 过温 */
    case APP_SYSTEM_STOP_WAY_OVERTEMP:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON19_ABNORMAL_TEMP;
        }else{
            _reason = NETYCP_AS_REASON59_TEMPERATURE_ABNORMAL;
        }
        break;
    /* 过、欠压 */
    case APP_SYSTEM_STOP_WAY_OVERVOLT:
    case APP_SYSTEM_STOP_WAY_UNDERVOLT:
        _reason = NETYCP_AS_REASON5D_CHARGE_TVOLTAGE_ABNORMAL;
        break;
    /* 过流 */
    case APP_SYSTEM_STOP_WAY_OVERCURRENT:
        _reason = NETYCP_AS_REASON5E_CHARGE_TCURRENT_ABNORMAL;
        break;
    /* DC 继电器 */
    case APP_SYSTEM_STOP_WAY_RELAY:
        _reason = NETYCP_AS_REASON70_DC_RELAY;
        break;
    /* 并联 继电器 */
    case APP_SYSTEM_STOP_WAY_PARALLEL_RELAY:
        _reason = NETYCP_AS_REASON72_PARALLEL_RELAY;
        break;
    /* AC 继电器 */
    case APP_SYSTEM_STOP_WAY_AC_RELAY:
        _reason = NETYCP_AS_REASON71_AC_RELAY;
        break;
    /* 充满 */
    case APP_SYSTEM_STOP_WAY_CHARGE_FULL:
        _reason = NETYCP_CC_REASON02_CHARGE_FULL;
        break;
    /* 拔枪 */
    case APP_SYSTEM_STOP_WAY_PULL_GUN:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON11_GUIDE_DISCONNECT;
        }else{
            _reason = NETYCP_AS_REASON51_GUIDANCE_DISCONNECT;
        }
        break;
    /* 电子锁 */
    case APP_SYSTEM_STOP_WAY_ELECTRY_LOCK:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON1B_ELECT_LOCK;
        }else{
            _reason = NETYCP_AS_REASON5C_ELOCK_ABNORMAL;
        }
        break;
    /* 通讯 */
    case APP_SYSTEM_STOP_WAY_COMMINICATION:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON1F_RECV_BRM_TIMEOUT;
        }else{
            _reason = NETYCP_AS_REASON68_RECV_BCS_TIMEOUT;
        }
    break;
    /* 接收BRM超时 */
    case APP_SYSTEM_STOP_WAY_BRM_TIMEOUT:
        _reason = NETYCP_SF_REASON1F_RECV_BRM_TIMEOUT;
        break;
    /* 接收BCP超时 */
    case APP_SYSTEM_STOP_WAY_BCP_TIMEOUT:
        _reason = NETYCP_SF_REASON20_RECV_BCP_TIMEOUT;
        break;
    /* 接收BRO超时 */
    case APP_SYSTEM_STOP_WAY_BRO_TIMEOUT:
        _reason = NETYCP_SF_REASON21_RECV_BRO_AA_TIMEOUT;
        break;
    /* 接收BRO_AA超时 */
    case APP_SYSTEM_STOP_WAY_BRO_AA_TIMEOUT:
        _reason = NETYCP_SF_REASON21_RECV_BRO_AA_TIMEOUT;
        break;
    /* 启动中接收BCS超时 */
    case APP_SYSTEM_STOP_WAY_STARTING_BCS_TIMEOUT:
        _reason = NETYCP_SF_REASON22_RECV_BCS_TIMEOUT;
        break;
    /* 启动中接收BCL超时 */
    case APP_SYSTEM_STOP_WAY_STARTING_BCL_TIMEOUT:
        _reason = NETYCP_SF_REASON23_RECV_BCL_TIMEOUT;
        break;
    /* 充电中接收BCS超时 */
    case APP_SYSTEM_STOP_WAY_CHARGEING_BCS_TIMEOUT:
        _reason = NETYCP_AS_REASON68_RECV_BCS_TIMEOUT;
        break;
    /* 充电中接收BCL超时 */
    case APP_SYSTEM_STOP_WAY_CHARGING_BCL_TIMEOUT:
        _reason = NETYCP_AS_REASON69_RECV_BCL_TIMEOUT;
        break;
    /* 辅源 */
    case APP_SYSTEM_STOP_WAY_AUXPOWER:
        _reason = NETYCP_SF_REASON2B_AUXPOWER;
        break;
    /* 断电 */
    case APP_SYSTEM_STOP_WAY_POWER_OFF:
        _reason = NETYCP_AS_REASON67_POWER_OFF;
        break;
    /* 存储芯片 */
    case APP_SYSTEM_STOP_WAY_FLASH:
    case APP_SYSTEM_STOP_WAY_EEPROM:
        _reason = NETYCP_AS_REASON73_STORAGE_CHIP;
        break;
    /* 短路 */
    case APP_SYSTEM_STOP_WAY_SHORTS:
        _reason = NETYCP_AS_REASON52_CIRCUIT_BREAKER_ACTION;
        break;
    /* 枪电压 */
    case APP_SYSTEM_STOP_WAY_GUNVOLT:
        _reason = NETYCP_SF_REASON25_BHM_STAGE_VOLT_OVERRANGE;
        break;
    /* 绝缘 */
    case APP_SYSTEM_STOP_WAY_INSULT:
        _reason = NETYCP_SF_REASON1D_INSULATION_ABNORMAL;
        break;
    /* 电池电压 */
    case APP_SYSTEM_STOP_WAY_BATTERY_VOLT:
        _reason = NETYCP_AS_REASON76_BATTERY_VOLTAGE;
        break;
    /* 车机停止 */
    case APP_SYSTEM_STOP_WAY_BST:
        _reason = NETYCP_AS_REASON66_CAR_COMMAND_STOP;
        if(_transaction){
            if(_transaction->bms_fault_reason.battery_ot){
                _reason = NETYCP_AS_REASON60_BATTERY_GROUP_OVERTEMP;
            }else if(_transaction->bms_error_reason.over_current){
                _reason = NETYCP_AS_REASON5E_CHARGE_TCURRENT_ABNORMAL;
            }else if(_transaction->bms_error_reason.volt_abnormal){
                _reason = NETYCP_AS_REASON5D_CHARGE_TVOLTAGE_ABNORMAL;
            }
        }
        break;
    /* 准备电压 */
    case APP_SYSTEM_STOP_WAY_READY_VOLT:
        _reason = NETYCP_SF_REASON26_BRO_AA_STAGE_VOLT_OVERRANGE;
        break;
    /* 绝缘电压 */
    case APP_SYSTEM_STOP_WAY_INSULT_VOLT:
        _reason = NETYCP_SF_REASON2D_INSULT_VOLTAGE;
        break;
    /* BSM */
    case APP_SYSTEM_STOP_WAY_BSM:
        _reason = NETYCP_AS_REASON74_BSM_WARNNING;
        break;
    /* APP */
    case APP_SYSTEM_STOP_WAY_APP_STOP:
        _reason = NETYCP_CC_REASON01_APP;
        break;
    /* 刷卡 */
    case APP_SYSTEM_STOP_WAY_OFFLINECARD_STOP:
    case APP_SYSTEM_STOP_WAY_ONLINECARD_STOP:
        _reason = NETYCP_CC_REASON06_SWIP_CARD;
        break;
    /* 余额不足 */
    case APP_SYSTEM_STOP_WAY_NO_BALLANCE:
        if(stop_in_starting){
            _reason = NETYCP_SF_REASON14_NO_BALLANCE;
        }else{
            _reason = NETYCP_CC_REASON04_TARGET_MONEY;
        }
        break;
    /* 屏幕 */
    case APP_SYSTEM_STOP_WAY_SCREEN_STOP:
        _reason = NETYCP_CC_REASON06_SWIP_CARD;
        break;
    /* 到达设定电量 */
    case APP_SYSTEM_STOP_WAY_REACH_ELECT:
        _reason = NETYCP_CC_REASON03_TARGET_ELECT;
        break;
    /* 到达设定时间 */
    case APP_SYSTEM_STOP_WAY_REACH_TIME:
        _reason = NETYCP_CC_REASON05_TARGET_TIME;
        break;
    /* 到达设定余额 */
    case APP_SYSTEM_STOP_WAY_REACH_MONEY:
        _reason = NETYCP_CC_REASON04_TARGET_MONEY;
        break;
    /* 充电电流异常 */
    case APP_SYSTEM_STOP_WAY_CURRENT_ABNORMAL:
        _reason = NETYCP_AS_REASON5B_CURRENT_ABNORMAL;
        break;
    /* 达到SOC 限定值 */
    case APP_SYSTEM_STOP_WAY_SOC_LIMIT:
        _reason = NETYCP_CC_REASON07_RESERVE;
        break;
    /* VIN 码鉴权失败 */
    case APP_SYSTEM_STOP_WAY_AUTHEN_FAIL:
        _reason = NETYCP_AS_REASON77_VIN_AUTHEN_FAIL;
        break;
    /* 主机柜禁止充电 */
    case APP_SYSTEM_STOP_WAY_MAIN_CABINET_FORBID:
        _reason = NETYCP_AS_REASON9C_MAIN_CABINET_FORBID;
        break;
    default:
    {
        uint16_t _way = mw_system_stop_way_convert(reason);
        switch(_way){
        /* 防雷器 */
        case APP_SYSTEM_STOP_WAY_LIGHTPROTECT:
            _reason = NETYCP_AS_REASON78_LIGHTPROTECT;
            break;
        /* 枪座 */
        case APP_SYSTEM_STOP_WAY_GUNSITE:
            _reason = NETYCP_AS_REASON79_GUNSITE;
            break;
        /* 断路器 */
        case APP_SYSTEM_STOP_WAY_CIRCUIT_BREAKER:
            _reason = NETYCP_AS_REASON7A_CIRCUIT_BREAKER;
            break;
        /* 水浸 */
        case APP_SYSTEM_STOP_WAY_FLOODING:
            _reason = NETYCP_AS_REASON7B_FLOODING;
            break;
        /* 烟感 */
        case APP_SYSTEM_STOP_WAY_SMOKE:
            _reason = NETYCP_AS_REASON7C_SMOKE;
            break;
        /* 倾倒 */
        case APP_SYSTEM_STOP_WAY_POUR:
            _reason = NETYCP_AS_REASON7D_POUR;
            break;
        /* 液冷 */
        case APP_SYSTEM_STOP_WAY_LIQUIDCOOLING:
            _reason = NETYCP_AS_REASON7E_LIQUIDCOOLING;
            break;
        /* 熔断器 */
        case APP_SYSTEM_STOP_WAY_FUSE:
            _reason = NETYCP_AS_REASON7F_FUSE;
            break;
        /* 主机柜故障 */
        case APP_SYSTEM_STOP_WAY_MAIN_CABINET_OFFLINE:
            _reason = NETYCP_AS_REASON9D_MAIN_CABINET_FAULT;
            break;
        /* 宇通BFC */
        case APP_SYSTEM_STOP_WAY_YT_BFC:
            _reason = NETYCP_AS_REASON9D_YT_BFC;
            break;
        default:
            break;
        }
    }
        break;
    }
    return _reason;
}

/*************************************************
 * 函数名      ycp_query_transaction_verify_state
 * 功能          查询订单确认状态
 * **********************************************/
uint8_t ycp_query_transaction_verify_state(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return 0x00;
    }

    if(ycp_transaction_is_verify(gunno)){
        ycp_set_transaction_verify_state(gunno, 0x00);
        return 0x01;
    }
    return 0x00;
}

static void ycp_request_message_repeat(uint8_t gunno)
{
    uint8_t event = 0;
    if(ycp_exist_message_wait_response(gunno, NULL)){
        for(event = 0; event < NET_YCP_CHARGEPILE_PREQ_NUM; event++){
            if(event == NET_YCP_PREQ_EVENT_TRANSACTION_RECORD){
                if(ycp_get_message_wait_response_timeout_state(gunno, NET_YCP_WAIT_TRANSACTION_RESPONSE_TIMEOUT, event)){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, event);
                }
            }else{
                if(ycp_get_message_wait_response_timeout_state(gunno, NET_YCP_WAIT_RESPONSE_TIMEOUT, event)){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, event);
                }
            }
        }
    }
}

static void ycp_data_realtime_process(uint8_t gunno, System_BaseData *base)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }
    if(base == NULL){
        return;
    }

    if(ycp_get_socket_info()->socket_state == YCP_SOCKET_STATE_LOGIN_SUCCESS){
        ycp_request_message_repeat(gunno);

        if(base->state.current == APP_OFSM_STATE_CHARGING){
            if(s_ycp_state_data_count[gunno] > rt_tick_get()){
                s_ycp_state_data_count[gunno] = rt_tick_get();
            }
            if((rt_tick_get() - s_ycp_state_data_count[gunno]) > YCP_STATE_DATA_INTERVAL_CHARGING *1000){
                s_ycp_state_data_count[gunno] = rt_tick_get();
                ycp_chargepile_request_padding_state_data(gunno, 0x00);
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
                if(g_ycp_sreq_set_para.body.bmsdata_switch == 0x01){
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_CHARGER_OUTPUT_BMS_REQUIRE);
                    ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_BMS_INFO);
                }
            }
        }else{
            if(s_ycp_state_data_count[gunno] > rt_tick_get()){
                s_ycp_state_data_count[gunno] = rt_tick_get();
            }

            if((rt_tick_get() - s_ycp_state_data_count[gunno]) > s_ycp_state_data_interval[gunno] *1000){
                s_ycp_state_data_count[gunno] = rt_tick_get();
                if(s_ycp_state_data_interval[gunno] < YCP_STATE_DATA_INTERVAL_IDLE){
                    s_ycp_state_data_interval[gunno] = YCP_STATE_DATA_INTERVAL_IDLE;
                }
                ycp_chargepile_request_padding_state_data(gunno, 0x00);
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
            }
        }

        if(s_ycp_fault_report > rt_tick_get()){
            s_ycp_fault_report = rt_tick_get();
        }

        if((rt_tick_get() - s_ycp_fault_report) > YCP_FAULT_INFO_INTERVAL *1000){
            s_ycp_fault_report = rt_tick_get();
            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, 0x00, NET_YCP_PREQ_EVENT_PADDING_DEVICE_FAULT);
        }
    }else{
        s_ycp_fault_report = rt_tick_get();
        s_ycp_state_data_count[gunno] = rt_tick_get();
        s_ycp_state_data_interval[gunno] = YCP_STATE_DATA_INTERVAL_IDLE;
        ycp_chargepile_request_padding_state_data(gunno, 0x00);
        ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
    }

    switch(base->state.current){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_STOPING:
        ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
        break;
    case APP_OFSM_STATE_RESERVATION:
    case APP_OFSM_STATE_FINISHING:
    case APP_OFSM_STATE_FAULTING:
        break;
    case APP_OFSM_STATE_STARTING:
        if(base->start_type != APP_CHARGE_START_WAY_VIN){
            ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
        }
        ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
        break;
    case APP_OFSM_STATE_CHARGING:
        ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_APPLY_START_CHARGE);
        ycp_clear_message_wait_response_state(gunno, NET_YCP_PREQ_EVENT_TRANSACTION_RECORD);
        break;
    default:
        break;
    }
}

/*
 * 用于检测只上报一次的报文是否有漏报
 * */
static void ycp_disposable_message_check(uint8_t gunno)
{
    if(gunno >= NET_SYSTEM_GUN_NUMBER){
        return;
    }

    uint8_t _event = 0x00;
    ycp_get_disposable_event(gunno, 0x00, &_event);
    if(_event){
        if(_event &(1 <<YCP_DISPOSABLE_EVENT_STATE)){
            if(ycp_get_message_send_state(gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA) == NET_YCP_SEND_STATE_COMPLETE){
                g_ycp_preq_report_state_data[gunno].body.plug_gun = s_ycp_disposable_info[gunno].state.connect;
                g_ycp_preq_report_state_data[gunno].body.state = s_ycp_disposable_info[gunno].state.state;

                ycp_set_clear_disposable_event(gunno, YCP_DISPOSABLE_EVENT_STATE, 0x01);
                s_ycp_state_data_count[gunno] = rt_tick_get();
                ycp_chargepile_request_padding_state_data(gunno, 0x00);
                ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_REPORT_STATE_DATA);
            }
        }
    }
}

static void ycp_realtime_process_thread_entry(void *parameter)
{
    System_BaseData *base = NULL;
    uint32_t req_billing_rule_tick = 0x00;
    uint8_t gunno = 0x00, is_power_on = NET_ENUM_TRUE;

    while(1){
        net_thread_running(rt_thread_self(), NULL, 0x00, 0x00);
        if((net_get_ota_info()->state >= NET_OTA_STATE_LOGIN_WAIT) && (net_get_ota_info()->state <= NET_OTA_STATE_UPDATING)){
            rt_thread_mdelay(5000);
            continue;
        }
        if(s_ycp_handle == NULL){
            rt_thread_mdelay(100);
            continue;
        }

        if(ycp_get_socket_info()->socket_state == YCP_SOCKET_STATE_LOGIN_SUCCESS){
            if((is_power_on == NET_ENUM_TRUE) && (ycp_is_interact_normally())){
                uint8_t is_reported = NET_ENUM_FALSE;
                is_power_on = NET_ENUM_FALSE;
                for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                    base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
                    if(base->state.current != APP_OFSM_STATE_FAULTING){
                        if(is_reported == NET_ENUM_FALSE){
                            ycp_net_event_send(NET_YCP_EVENT_HANDLE_CHARGEPILE, NET_YCP_EVENT_TYPE_REQUEST, gunno, NET_YCP_PREQ_EVENT_PADDING_DEVICE_FAULT);
                            is_reported = NET_ENUM_TRUE;
                        }
                    }
                }
            }
            if(ycp_is_interact_normally()){
                extern uint8_t app_billingrule_is_valid(uint8_t gunno);
                for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
                    if(app_billingrule_is_valid(gunno) == 0x00){
                        if(req_billing_rule_tick > rt_tick_get()){
                            req_billing_rule_tick = rt_tick_get();
                        }
                        if((rt_tick_get() - req_billing_rule_tick) > YCP_REQ_BILLINGRULE_AGAIN_INTERVAL){
                            g_ycp_preq_billing_model_verify.body.model_sn = 0x00;
                            ycp_request_billingrule_again();
                            req_billing_rule_tick = rt_tick_get();
                            break;
                        }
                    }
                }
            }
        }else{
            req_billing_rule_tick = rt_tick_get();
        }

        for(gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
            base = (System_BaseData*)(s_ycp_handle->get_base_data(gunno));
            ycp_fault_detect_report(gunno);
            ycp_data_realtime_process(gunno, base);
            ycp_disposable_message_check(gunno);
        }

        rt_thread_mdelay(100);
    }
}

int32_t ycp_realtime_process_init(void)
{
    uint8_t entry = 0x03, name[NET_THREAD_MONITOR_NAME_MAX];

#ifdef NET_DESIGNATE_REGION
    for(uint8_t gunno = 0x00; gunno < NET_SYSTEM_GUN_NUMBER; gunno++){
        s_ycp_state_data_interval[gunno] = 0x00;
        s_ycp_state_data_count[gunno] = 0x00;

        memset(&s_ycp_flag_info[gunno], 0x00, sizeof(s_ycp_flag_info[gunno]));
        memset(&s_ycp_disposable_info[gunno], 0x00, sizeof(s_ycp_disposable_info[gunno]));
    }
    memset(&s_ycp_pfaut_info, 0x00, sizeof(s_ycp_pfaut_info));
    memset(&s_ycp_hfaut_info, 0x00, sizeof(s_ycp_hfaut_info));

    s_ycp_fault_report = 0x00;
    s_ycp_local_start_sq = 0x00;
    s_ycp_handle = NULL;
#endif /* NET_DESIGNATE_REGION */

    if(rt_thread_init(&s_ycp_realtime_process_thread, "ycp_rl_pro", ycp_realtime_process_thread_entry, NULL,
            s_ycp_realtime_process_thread_stack, YCP_REALTIME_PROCESS_THREAD_STACK_SIZE, 16, 10) != RT_EOK){
        LOG_E("ycp realtime process thread create fail, please check");
        return -0x01;
    }
    if(rt_thread_startup(&s_ycp_realtime_process_thread) != RT_EOK){
        LOG_E("ycp realtime process thread startup fail, please check");
        return -0x01;
    }

    net_thread_init_hook(&s_ycp_realtime_process_thread, &entry, sizeof(entry), NET_THREAD_RUNNING_OPTION_ENTRY_MAX);

    memset(name, 0x00, NET_THREAD_MONITOR_NAME_MAX);
    memcpy(name, "yc_rlp", strlen("yc_rlp"));
    net_thread_init_hook(&s_ycp_realtime_process_thread, name, strlen((char*)name), NET_THREAD_RUNNING_OPTION_NAME);

    return 0x00;
}

#endif /* NET_PACK_USING_YCP */
