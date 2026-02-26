/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-03-06     31638       the first version
 */
#include "app_can.h"
#include "app_hci.h"
#include "app_osupport.h"
#include "app_data_info_interface.h"
#include "rtthread.h"
#include "thaisen7102Public.h"
#include "thaisenBMS.h"
#include "app_ofsm.h"

#ifdef USING_TCU_CAN

#define APP_KS_CHARGER_CTRL_CMD_CAN_ID  0x2B02F                 /* 科式充电机控制帧CANID */

#define APP_KS_LOCAL_START_CMD_CAN_ID   0x18F1C1C0              /* 科式是否允许本地启动帧CANID */
#define APP_KS_SWIP_CARD_CMD_CAN_ID     0x18F4C1C0              /* 科式是否允许刷卡启动帧CANID */
#define APP_KS_FUNCTION_CMD_CAN_ID      0x18F5C1C0              /* 科式功能指令帧CANID */

#define APP_KS_CHARGE_DATA_CAN_ID       0x18F6C1C0              /* 科式充电数据CANID */
#define APP_KS_BMS_DATA_01_CAN_ID       0x18F7C1C0              /* 科式BMS数据01CANID */
#define APP_KS_BMS_DATA_02_CAN_ID       0x18F8C1C0              /* 科式BMS数据02CANID */

#define APP_TCU_CAN_MQ_SIZE             10                      /* CAN报文接收个数 */

#endif /* USING_TCU_CAN */

#ifdef CP_USING_LV_MODULE_BMS
#define APP_LV_MODULE_BMS_MSG_ID_0x309         0x309            /* 0x309报文ID */
#define APP_LV_MODULE_BMS_MSG_ID_0x307         0x307            /* 0x307报文ID */
#define APP_LV_MODULE_BMS_MSG_ID_0x206         0x206            /* 0x206报文ID */
#define APP_LV_MODULE_BMS_MSG_ID_0x306         0x306            /* 0x303报文ID */

#define APP_LV_MODULE_BMS_MSG_ID_0x3F1         0x3F1            /* 0x3F1报文ID */
#define APP_LV_MODULE_BMS_MSG_ID_0x3F3         0x3F3            /* 0x3F3报文ID */

#define APP_LV_MODULE_BMS_MSG_LEN_0x3F1        0x08             /* 0x3F1报文数据长度(B) */
#define APP_LV_MODULE_BMS_MSG_LEN_0x3F3        0x08             /* 0x3F3报文数据长度(B) */

#define APP_LV_MODULE_BMS_MSG_TYPE_0x3F1       CAN_ID_STD       /* 0x3F1帧类型：标准帧 */
#define APP_LV_MODULE_BMS_MSG_TYPE_0x3F3       CAN_ID_STD       /* 0x3F3帧类型：标准帧 */
#endif /* CP_USING_LV_MODULE_BMS */

#pragma pack(1)

#ifdef USING_TCU_CAN
struct can_data{
    uint32_t can_id;
    uint8_t data[8];
};

struct can_info{
    struct{
        uint8_t mq_is_init : 1;                                 /** 数据接收队列已初始化 */
        uint8_t maintenance_enable : 1;                         /** 使用保养模式 */
        uint8_t maintenance_enable_last : 1;                    /** 使用保养模式(前一次的状态) */
        uint8_t is_recvec_func_cmd : 1;                         /** 是否接收到了功能指令帧 */
        uint8_t is_guna_start : 1;                              /** A枪已启动 */
        uint8_t is_gunb_start : 1;                              /** B枪已启动 */
        uint8_t reserve : 2;                                    /** 预留 */
    }flag;
};
#endif /* USING_TCU_CAN */

#ifdef CP_USING_LV_MODULE_BMS
typedef struct{
    uint8_t cmd;                                                /** 电池充电指令 */
    uint8_t soc;                                                /** 电池SOC */
    uint16_t svolt_max;                                         /** 最高单体电压(0.001V) */
    int8_t temp_max;                                            /** 电池最高温度(1度) */
    uint8_t counter;                                            /** 计数值 */
    uint16_t target_curr;                                       /** 充电目标电流(0.01A) */
    uint16_t target_volt;                                       /** 充电目标电压(0.01V) */
}bms_app_info;

typedef struct{
    uint32_t work_mode : 2;                                     /** 工作模式  sbit:0 */
    uint32_t f_rank : 2;                                        /** 故障等级  sbit:2 */
    uint32_t f_hardware : 1;                                    /** 硬件故障  sbit:4 */
    uint32_t f_ot : 1;                                          /** 桩过温故障  sbit:5 */
    uint32_t f_reverse : 1;                                     /** 电池反接故障  sbit:6 */
    uint32_t f_communicate : 1;                                 /** 通讯故障  sbit:7 */
//    uint32_t reserve0 : 8;                                      /** 预留  sbit:8 */
    uint32_t out_current : 16;                                  /** 实际输出电流(0.01A)  sbit:16 */
    uint32_t out_voltage : 16;                                  /** 实际输出电压(0.01V)  sbit:32 */
    uint32_t elock_state : 1;                                   /** 电子锁状态  sbit:40 */
    uint32_t clinker_state : 2;                                 /** 充电连接器状态  sbit:41 */
    uint32_t f_in_uv : 1;                                       /** 输入欠压故障  sbit:43 */
    uint32_t f_in_ov : 1;                                       /** 输入过压故障  sbit:44 */
    uint32_t f_busbar_uv : 1;                                   /** 直流母线欠压故障  sbit:45 */
    uint32_t f_busbar_ov : 1;                                   /** 直流母线过压故障  sbit:46 */
    uint32_t f_busbar_oc : 1;                                   /** 直流母线过流故障  sbit:47 */
    uint32_t f_out_shorts : 1;                                  /** 输出短路故障  sbit:48 */
    uint32_t f_boot_timeout : 1;                                /** 启动超时  sbit:49 */
    uint32_t f_fan : 1;                                         /** 风扇故障  sbit:50 */
    uint32_t counter : 4;                                       /** 循环计数  sbit:51 */
    uint32_t reserve1 : 1;                                      /** 预留1  sbit:55 */
    uint32_t check : 8;                                         /** 校验和  sbit:56 */
}charger_app_0x3F1;

typedef struct{
    uint32_t request_cmd : 3;                                   /** 充电请求  sbit:0 */
    uint32_t f_emergency : 1;                                   /** 急停故障  sbit:3 */
    uint32_t f_protect_lighting : 1;                            /** 防雷器故障  sbit:4 */
    uint32_t f_gun_volt : 1;                                    /** 枪头电压故障(绝缘前外侧电压)  sbit:5 */
    uint32_t f_relief : 1;                                      /** 泄放故障  sbit:6 */
    uint32_t f_elock : 1;                                       /** 电子锁故障  sbit:7 */
    uint32_t f_ac_miss_phase : 1;                               /** 交流输入缺相  sbit:8 */
    uint32_t f_insulation : 1;                                  /** 绝缘故障  sbit:9 */
    uint32_t f_guidance : 1;                                    /** 控制导引故障  sbit:10 */
    uint32_t f_need_oc : 1;                                     /** 需求电流过高  sbit:11 */
    uint32_t f_module : 1;                                      /** 充电模块故障  sbit:12 */
    uint32_t f_gun_ot : 1;                                      /** 枪头过温故障  sbit:13 */
    uint32_t f_need_ov : 1;                                     /** BMS需求电压过高  sbit:14 */
    uint32_t f_dcrelay : 1;                                     /** 直流接触器故障  sbit:15 */
    uint32_t f_precharge_volt : 1;                              /** 软启前外侧电压异常  sbit:16 */
    uint32_t f_auxpower : 1;                                    /** 辅助电源故障  sbit:17 */
    uint32_t f_breaker : 1;                                     /** 交流断路器故障  sbit:18 */
    uint32_t f_poweroff : 1;                                    /** 掉电故障  sbit:19 */
    uint32_t f_gate : 1;                                        /** 门禁故障  sbit:20 */
    uint32_t f_acrelay : 1;                                     /** 交流接触器故障  sbit:21 */
    uint32_t reserve0 : 2;                                      /** 预留0  sbit:22 */
    uint32_t reserve1 : 24;                                     /** 预留1  sbit:24 */
    uint32_t counter : 4;                                       /** 循环计数器  sbit:48 */
    uint32_t reserve2 : 4;                                      /** 预留2  sbit:48 */
    uint32_t check : 8;                                         /** 校验和  sbit:56 */
}charger_app_0x3F3;

typedef struct{
    struct{
        uint8_t is_offline : 1;                                 /** 已离线 */
        uint8_t is_starting : 1;                                /** 已请求充电 */
        uint8_t reserve : 6;                                    /** 预留 */
    }flag[APP_SYSTEM_GUNNO_SIZE];
    uint16_t offline_count[APP_SYSTEM_GUNNO_SIZE];              /** BMS离线计数 */
}app_lv_bms_info;
#endif /* CP_USING_LV_MODULE_BMS */

#pragma pack()

#ifdef CP_USING_LV_MODULE_BMS
APP_DEF_SRAM1 static bms_app_info s_bms_app_info[APP_SYSTEM_GUNNO_SIZE];
APP_DEF_SRAM1 static app_lv_bms_info s_app_lv_bms_info;
#endif /* CP_USING_LV_MODULE_BMS */

#ifdef USING_TCU_CAN
APP_DEF_SRAM1 struct can_info s_can_info;
APP_DEF_SRAM1 static struct can_data s_can_data[APP_TCU_CAN_MQ_SIZE];
APP_DEF_SRAM1 static struct rt_messagequeue s_tcu_can_mq;
#endif /* USING_TCU_CAN */

#ifdef USING_TCU_CAN
/** CAN 数据接收回调 */
void thaisen_can_tcu_isrCallback(void)
{
    struct can_data data;
    can_msg_buf can_receive_message;
    can_receive_message = thaisen_get_tcu_dat();

    data.can_id = can_receive_message.CANID;
    memcpy(data.data, can_receive_message.data, sizeof(can_receive_message.data));
    if(s_can_info.flag.mq_is_init){
        rt_mq_send(&s_tcu_can_mq, &data, sizeof(data));
    }
}

/******************************************************************************
 * 函数名          ks_padding_charge_info
 * 功能             充电信息填充
 * 参数             data         填充缓存
 *       len          缓存长度
 *       gunno        枪号
 * 返回            >=0：成功       <0：失败
 *****************************************************************************/
static int8_t ks_padding_charge_info(uint8_t* data, uint8_t len, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return -0x01;
    }
    if(data && len >= 0x08){
        int32_t symbol_value = 0;
        uint32_t value = 0;
        struct ofsm_info *ofsm = get_ofsm_info(gunno);
        memset(data, 0x00, len);
        switch(ofsm->state){
        case APP_OFSM_STATE_WAIT_NET:
        case APP_OFSM_STATE_IDLEING:
            break;
        case APP_OFSM_STATE_READYING:
            data[0x00] = 0x01;
            break;
        case APP_OFSM_STATE_STARTING:
            data[0x00] = 0x02;
            break;
        case APP_OFSM_STATE_CHARGING:
            data[0x00] = 0x03;

            data[1] = ofsm->base.current_soc;  /* SOC */

            value = ofsm->base.voltage_a;
            value /= 10;
            memcpy(&data[0x02], &value, 0x02);

            symbol_value = ofsm->base.current_a;
#ifdef APP_INCLUDE_V2G
            if(ofsm->base.gun_running_mode == APP_GUN_RUNNING_MODE_V2G){
                if(symbol_value < 0x00)
                    symbol_value = 0x00 - symbol_value;
            }else{
                if(symbol_value < 0x00)
                    symbol_value = 0x00;
            }
#endif /* APP_INCLUDE_V2G */
            value = symbol_value;
            value /= 10;
            memcpy(&data[0x04], &value, 0x02);

            value = ofsm->base.elect_a;
            value /= 10;
            memcpy(&data[0x06], &value, 0x02);
            break;
        case APP_OFSM_STATE_STOPING:
        case APP_OFSM_STATE_FINISHING:
            data[0x00] = 0x04;
            break;
        case APP_OFSM_STATE_FAULTING:
            data[0x00] = 0x05;
            break;
        default:
            return -0x03;
            break;
        }
        data[0] |= (gunno <<0x03);

        return 0x00;
    }else{
        return -0x02;
    }
}

/******************************************************************************
 * 函数名          ks_padding_bms_info_01
 * 功能            BMS信息填充
 * 参数             data         填充缓存
 *       len          缓存长度
 *       gunno        枪号
 * 返回            >=0：成功       <0：失败
 *****************************************************************************/
static int8_t ks_padding_bms_info_01(uint8_t* data, uint8_t len, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return -0x01;
    }
    if(data && len >= 0x08){
        struct ofsm_info *ofsm = get_ofsm_info(gunno);
        struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(ofsm->base.bms_data);

        memset(data, 0x00, len);
        data[0x00] = gunno;

        switch(ofsm->state){
        case APP_OFSM_STATE_CHARGING:
        {
            uint16_t value = 0x00;

            value = bms->BCS.CellHigVolt;
            memcpy(&data[0x01], &value, sizeof(value));

            value = bms->BCL.BMSneedVolt;
            memcpy(&data[0x03], &value, sizeof(value));

            value = bms->BCL.BMSneedCurlt;
            memcpy(&data[0x05], &value, sizeof(value));
        }
            break;
        default:
            break;
        }
        return 0x00;
    }else{
        return -0x02;
    }
}

/******************************************************************************
 * 函数名          ks_padding_bms_info_02
 * 功能            BMS信息填充
 * 参数             data         填充缓存
 *       len          缓存长度
 *       gunno        枪号
 * 返回            >=0：成功       <0：失败
 *****************************************************************************/
static int8_t ks_padding_bms_info_02(uint8_t* data, uint8_t len, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return -0x01;
    }
    if(data && len >= 0x08){
        struct ofsm_info *ofsm = get_ofsm_info(gunno);
        struct thaisenBMS_Charger_struct *bms = (struct thaisenBMS_Charger_struct*)(ofsm->base.bms_data);

        memset(data, 0x00, len);

        data[0x00] = gunno;
        data[0x05] = app_get_highest_priority_system_fault(gunno);
        if(data[0x05] == APP_SYS_FAULT_NO_ERROR){
            data[0x05] = 0x00;
        }else{
            data[0x05] = data[0x05] + 0x01;
        }

        switch(ofsm->state){
        case APP_OFSM_STATE_CHARGING:
        {
            uint16_t value = 0x00;

            value = (ofsm->base.charge_time /60);
            memcpy(&data[0x01], &value, sizeof(value));
            data[0x03] = (bms->BSM.HigTemp + 50);
            data[0x04] = (bms->BSM.LowTemp + 50);
        }
            break;
        default:
            break;
        }
        return 0x00;
    }else{
        return -0x02;
    }
}

/******************************************************************************
 * 函数名          ks_padding_local_enable
 * 功能            是否允许本地启动信息填充
 * 参数             data         填充缓存
 *       len          缓存长度
 *       gunno        枪号
 * 返回            >=0：成功       <0：失败
 *****************************************************************************/
static int8_t ks_padding_local_enable(uint8_t* data, uint8_t len, uint8_t gunno)
{
    if(data && len >= 0x08){
        uint8_t function = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_LOCAL, 0));

        memset(data, 0x00, len);
        if(function == 0x01){
            data[0x01] = 0x03;
        }else{
            data[0x01] = 0x0C;
        }
        return 0x00;
    }else{
        return -0x02;
    }
}

/******************************************************************************
 * 函数名          ks_padding_swip_card_enable
 * 功能            是否允许刷卡启动信息填充
 * 参数             data         填充缓存
 *       len          缓存长度
 *       gunno        枪号
 * 返回            >=0：成功       <0：失败
 *****************************************************************************/
static int8_t ks_padding_swip_card_enable(uint8_t* data, uint8_t len, uint8_t gunno)
{
    if(data && len >= 0x08){
        uint8_t function = *(uint8_t*)(sys_read_config_item_content(CONFIG_ITEM_SUPORT_CARD, 0));

        memset(data, 0x00, len);
        if(function == 0x01){
            data[0x01] = 0x02;   /* 先做不刷卡版本 */
        }else{
            data[0x01] = 0x02;   /* 先做不刷卡版本 */
        }
        return 0x00;
    }else{
        return -0x02;
    }
}

void app_tcan_send_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

    uint8_t gunno = 0x00;
    can_msg_buf can_send_message;
    can_send_message.DLC = 0x08;
    can_send_message.length = 0x08;
    can_send_message.priority = 0x06;
    memset(can_send_message.data, 0x00, sizeof(can_send_message.data));

    while (1)
    {
        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

        switch (get_ofsm_info(0x00)->base.ota_state) {
        case APP_OTA_STATE_NULL:
            break;
        case APP_OTA_STATE_UP:
        case APP_OTA_STATE_LINK_UP:
        case APP_OTA_STATE_INTERNET_UP:
            break;
        case APP_OTA_STATE_AUTHING:
            break;
        case APP_OTA_STATE_AUTH_SUCCESS:
        case APP_OTA_STATE_UPDATEING:
            rt_thread_mdelay(5000);
            continue;
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        if(ks_padding_charge_info(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_CHARGE_DATA_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(50);
        }
        if(ks_padding_bms_info_01(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_BMS_DATA_01_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(50);
        }
        if(ks_padding_bms_info_02(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_BMS_DATA_02_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(50);
        }
        if(ks_padding_local_enable(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_LOCAL_START_CMD_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(50);
        }
        if(ks_padding_swip_card_enable(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_SWIP_CARD_CMD_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(50);
        }

        if(++gunno >= APP_SYSTEM_GUNNO_SIZE){
            gunno = 0x00;
        }
    }
}

/******************************************************************************
 * 函数名          ks_recv_process_charger_ctrl
 * 功能            充电机控制帧处理
 * 参数             data         数据
 *       len          数据长度
 * 返回
 *****************************************************************************/
static void ks_recv_process_charger_ctrl(uint8_t* data, uint8_t len)
{
    if(data && len >= 0x08){
        if((data[0x02] <= 0x01) && (data[0x02] != s_can_info.flag.is_guna_start)){
            s_can_info.flag.is_guna_start = data[0x02];
            if(s_can_info.flag.is_guna_start){
                app_set_hci_event(0x00, HCI_EVENT_SCREEN_START);
            }else{
                app_set_hci_event(0x00, HCI_EVENT_SCREEN_STOP);
            }
        }
        if((data[0x03] <= 0x01) && (data[0x03] != s_can_info.flag.is_gunb_start)){
            s_can_info.flag.is_gunb_start = data[0x03];
            if(s_can_info.flag.is_gunb_start){
                app_set_hci_event(0x01, HCI_EVENT_SCREEN_START);
            }else{
                app_set_hci_event(0x01, HCI_EVENT_SCREEN_STOP);
            }
        }
    }
}

void app_tcan_recv_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

    struct can_data data;
    uint16_t timeout_count = 0x00;

    if(rt_mq_init(&s_tcu_can_mq, "tcu_mq", s_can_data, sizeof(s_can_data[0]), sizeof(s_can_data), RT_IPC_FLAG_PRIO) != RT_EOK){
        printf("TCU CAN data mq init fail...\n");
        return;
    }
    s_can_info.flag.mq_is_init = 0x01;

    while (1)
    {
        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

        switch (get_ofsm_info(0x00)->base.ota_state) {
        case APP_OTA_STATE_NULL:
            break;
        case APP_OTA_STATE_UP:
        case APP_OTA_STATE_LINK_UP:
        case APP_OTA_STATE_INTERNET_UP:
            break;
        case APP_OTA_STATE_AUTHING:
            break;
        case APP_OTA_STATE_AUTH_SUCCESS:
        case APP_OTA_STATE_UPDATEING:
            rt_thread_mdelay(5000);
            continue;
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        s_can_info.flag.is_recvec_func_cmd = 0x00;

        if(rt_mq_recv(&s_tcu_can_mq, &data, sizeof(data), 0) == RT_EOK){
            if(data.can_id == APP_KS_FUNCTION_CMD_CAN_ID){
                s_can_info.flag.is_recvec_func_cmd = 0x01;
                timeout_count = 0x00;
                if(data.data[0x03]){
                    s_can_info.flag.maintenance_enable = 0x01;
                }else{
                    s_can_info.flag.maintenance_enable = 0x00;
                }
            }else if(data.can_id == APP_KS_CHARGER_CTRL_CMD_CAN_ID){
                ks_recv_process_charger_ctrl(data.data, sizeof(data.data));
            }
        }

        if(s_can_info.flag.is_recvec_func_cmd == 0x00){
            if(++timeout_count > 200){    /** 最长间隔500ms采集板应该发一帧功能指令报文 */
                s_can_info.flag.maintenance_enable = 0x00;
            }
        }
        rt_thread_mdelay(10);
    }
}

#endif /* USING_TCU_CAN */

/*******************************************
 * 函数名                app_is_using_maintenance_mode
 * 功能                    判断是否使用保养模式
 * 参数
 * 返回                    1：是         0：否
 ******************************************/
uint8_t app_is_using_maintenance_mode(void)
{
#ifdef USING_TCU_CAN
    return s_can_info.flag.maintenance_enable_last;
#else
    return 0x00;
#endif /* USING_TCU_CAN */
}

/*******************************************
 * 函数名                app_charge_mode_is_changed
 * 功能                    判断充电模式是否已改变
 * 参数
 * 返回                    1：是         0：否
 ******************************************/
uint8_t app_charge_mode_is_changed(void)
{
#ifdef USING_TCU_CAN
    if(s_can_info.flag.maintenance_enable != s_can_info.flag.maintenance_enable_last){
        s_can_info.flag.maintenance_enable_last = s_can_info.flag.maintenance_enable;
        return 0x01;
    }
    return 0x00;
#else
    return 0x00;
#endif /* USING_TCU_CAN */
}

/********************************************************** 带BMS 的低压模块 **********************************************************/
#ifdef CP_USING_LV_MODULE_BMS
static void app_bms_lv_msg_process(uint8_t gunno, can_msg_buf *msg);
#endif /* CP_USING_LV_MODULE_BMS */
/*************************************
 * 函数名       app_bsm_a_can_cb
 * 功能           BMS A can 接收回调
 * 参数           msg   报文数据
 * 返回
 ************************************/
static void app_bsm_a_can_cb(can_msg_buf *msg)
{
    /********************** 自动识别 CAN ID **********************/
    if(msg->CANID == APP_PARACHARGE_IDENTIFY_CAN_ID){
        struct ofsm_info *ofsm = get_ofsm_info(APP_SYSTEM_GUNNOA);
        ofsm->base.flag.recved_paracharge_identify_id = APP_THA_ENUM_TRUE;
    }
#ifdef CP_USING_LV_MODULE_BMS
    app_bms_lv_msg_process(APP_SYSTEM_GUNNOA, msg);
#endif /* CP_USING_LV_MODULE_BMS */
}

/*************************************
 * 函数名       app_bsm_b_can_cb
 * 功能           BMS B can 接收回调
 * 参数           msg   报文数据
 * 返回
 ************************************/
static void app_bsm_b_can_cb(can_msg_buf *msg)
{
#ifdef APP_USING_DOUBLEGUN
    /********************** 自动识别 CAN ID **********************/
    if(msg->CANID == APP_PARACHARGE_IDENTIFY_CAN_ID){
        struct ofsm_info *ofsm = get_ofsm_info(APP_SYSTEM_GUNNOB);
        ofsm->base.flag.recved_paracharge_identify_id = APP_THA_ENUM_TRUE;
    }
#ifdef CP_USING_LV_MODULE_BMS
    app_bms_lv_msg_process(APP_SYSTEM_GUNNOB, msg);
#endif /* CP_USING_LV_MODULE_BMS */
#endif /* APP_USING_DOUBLEGUN */
}

#ifdef CP_USING_LV_MODULE_BMS
/*************************************************
 * 函数名           app_bms_lv_is_fault_occured
 * 功能               判断故障是否发生
 * 参数              f        故障集体
 *         code     故障码
 * 返回              1：是      0：否
 ************************************************/
static uint8_t app_bms_lv_check_sum(const uint8_t *data, uint8_t dlen)
{
    uint8_t ret = 0x00;

    for(uint8_t i = 0x00; i < dlen; i++){
        ret += data[i];
    }
    return ret;
}

/*************************************************
 * 函数名           app_bms_lv_msg_process
 * 功能               接收报文处理
 * 参数              gunno        枪号
 *         msg          报文信息
 * 返回
 ************************************************/
static void app_bms_lv_msg_process(uint8_t gunno, can_msg_buf *msg)
{
    if((gunno >= APP_SYSTEM_GUNNO_SIZE) || (msg == NULL)){
        return;
    }
#if 0
    /** 数据校验 */
    if(msg->data[0x07] != app_bms_lv_check_sum((const uint8_t*)(msg->data), (sizeof(msg->data) - 0x01))){
        return;
    }
#endif
    /** BMS报文 */
    if(msg->CANID == APP_LV_MODULE_BMS_MSG_ID_0x309){
        /** 电池充电指令 */
        s_bms_app_info[gunno].cmd = (msg->data[0x00] &0x03);
        /** 目标充电电流 */
        s_bms_app_info[gunno].target_curr = (msg->data[0x02] |(msg->data[0x01] <<0x08));
        /** 目标充电电压 */
        s_bms_app_info[gunno].target_volt = (msg->data[0x04] |(msg->data[0x03] <<0x08));
        /** 循环计数 */
        s_bms_app_info[gunno].counter = (msg->data[0x06] &0x0F);

        s_app_lv_bms_info.flag[gunno].is_offline = APP_THA_ENUM_FALSE;
        s_app_lv_bms_info.offline_count[gunno] = 0x00;
    }else if(msg->CANID == APP_LV_MODULE_BMS_MSG_ID_0x307){
        /** 电池SOC */
        s_bms_app_info[gunno].soc = msg->data[0x00];

        s_app_lv_bms_info.flag[gunno].is_offline = APP_THA_ENUM_FALSE;
        s_app_lv_bms_info.offline_count[gunno] = 0x00;
    }else if(msg->CANID == APP_LV_MODULE_BMS_MSG_ID_0x206){
        struct thaisenBMS_Charger_struct* bms_data = mw_get_bms_data(gunno);
        /** 最高单体电压 */
        s_bms_app_info[gunno].svolt_max = msg->data[0x01];
        s_bms_app_info[gunno].svolt_max <<=0x08;
        s_bms_app_info[gunno].svolt_max |= msg->data[0x02];
        bms_data->BCS.CellHigVolt = (s_bms_app_info[gunno].svolt_max /10);

        s_app_lv_bms_info.flag[gunno].is_offline = APP_THA_ENUM_FALSE;
        s_app_lv_bms_info.offline_count[gunno] = 0x00;
    }else if(msg->CANID == APP_LV_MODULE_BMS_MSG_ID_0x306){
        struct thaisenBMS_Charger_struct* bms_data = mw_get_bms_data(gunno);
        /** 最高温度 */
        s_bms_app_info[gunno].temp_max = (msg->data[0x01] - 40);  /** -40偏移 */
        bms_data->BSM.HigTemp = s_bms_app_info[gunno].temp_max;

        s_app_lv_bms_info.flag[gunno].is_offline = APP_THA_ENUM_FALSE;
        s_app_lv_bms_info.offline_count[gunno] = 0x00;
    }
}

/*************************************************
 * 函数名           app_bms_lv_is_fault_occured
 * 功能               判断故障是否发生
 * 参数              f        故障集体
 *         code     故障码
 * 返回              1：是      0：否
 ************************************************/
static uint8_t app_bms_lv_is_fault_occured(uint32_t *f, uint8_t code)
{
    if(f == NULL){
        return APP_THA_ENUM_FALSE;
    }
    uint8_t set = (code /32), bit = (code %32);
    if(f[set] &(0x01 <<bit)){
        return APP_THA_ENUM_TRUE;
    }
    return APP_THA_ENUM_FALSE;
}
/*************************************************
 * 函数名           app_bms_lv_msg_0x0F1_padding
 * 功能               填充0x3F1报文
 * 参数              gunno    枪号
 *         msg      指向报文体
 * 返回
 ************************************************/
static void app_bms_lv_msg_0x0F1_padding(uint8_t gunno, can_msg_buf *msg)
{
    if((gunno >= APP_SYSTEM_GUNNO_SIZE) || (msg == NULL)){
        return;
    }
    struct ofsm_info *ofsm = get_ofsm_info(gunno);
    charger_app_0x3F1 *msg_0x3F1 = (charger_app_0x3F1*)(msg->data);
    uint32_t *f_pool = thaisenGetSysFault(gunno);
    int32_t _data = 0x00;

    msg->CANID = APP_LV_MODULE_BMS_MSG_ID_0x3F1;
    msg->length = APP_LV_MODULE_BMS_MSG_LEN_0x3F1;

    /** 同步电池SOC */
    ofsm->base.current_soc = s_bms_app_info[gunno].soc;
    /****************************** 工作模式 ******************************/
    switch(ofsm->state){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_RESERVATION:
        msg_0x3F1->work_mode = APP_CHARGER_LV_MODE_IDLE;
        break;
    case APP_OFSM_STATE_STARTING:
    case APP_OFSM_STATE_CHARGING:
        msg_0x3F1->work_mode = APP_CHARGER_LV_MODE_CHARGING;
        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        msg_0x3F1->work_mode = APP_CHARGER_LV_MODE_STOP;
        break;
    case APP_OFSM_STATE_FAULTING:
    case APP_OFSM_STATE_SIZE:
        msg_0x3F1->work_mode = APP_CHARGER_LV_MODE_FAULTING;
        break;
    default:
        msg_0x3F1->work_mode = APP_CHARGER_LV_MODE_FAULTING;
        break;
    }
    /****************************** 故障等级 ******************************/
    /** 从低到高 */
    msg_0x3F1->f_rank = APP_CHARGER_LV_F_RANK_NONE;
    msg_0x3F1->f_hardware = APP_THA_ENUM_FALSE;
    /** 桩过温 */
    msg_0x3F1->f_ot = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenFaultOverTemp)){
        msg_0x3F1->f_rank = APP_CHARGER_LV_F_RANK_LEVEL1;
        msg_0x3F1->f_hardware = APP_THA_ENUM_TRUE;
        msg_0x3F1->f_ot = APP_THA_ENUM_TRUE;
    }
    /** 电池反接故障 */
    msg_0x3F1->f_reverse = APP_THA_ENUM_FALSE;
    /** 通讯故障 */
    msg_0x3F1->f_communicate = APP_THA_ENUM_FALSE;
//    msg_0x3F1->reserve0 = APP_THA_ENUM_FALSE;
    /** 输入欠压故障 */
    msg_0x3F1->f_in_uv = APP_THA_ENUM_FALSE;
    /** 输入过压故障 */
    msg_0x3F1->f_in_ov = APP_THA_ENUM_FALSE;
    /** 直流母线欠压故障 */
    msg_0x3F1->f_busbar_uv = APP_THA_ENUM_FALSE;
    /** 直流母线过压故障 */
    msg_0x3F1->f_busbar_ov = APP_THA_ENUM_FALSE;
    /** 直流母线过流故障 */
    msg_0x3F1->f_busbar_oc = APP_THA_ENUM_FALSE;
    /** 输出短路故障 */
    msg_0x3F1->f_out_shorts = APP_THA_ENUM_FALSE;
    /** 启动超时 */
    msg_0x3F1->f_boot_timeout = APP_THA_ENUM_FALSE;
    /** 风扇故障 */
    msg_0x3F1->f_fan = APP_THA_ENUM_FALSE;
    /** 实际输出电流 */
    _data = ofsm->base.current_a;
    msg_0x3F1->out_current = (uint8_t)(_data &0xFF);
    msg_0x3F1->out_current <<=0x08;
    msg_0x3F1->out_current |= (uint8_t)((_data &0xFF00) >>0x08);
    /** 实际输出电压 */
    _data = ofsm->base.voltage_a;
    msg_0x3F1->out_voltage = (uint8_t)(_data &0xFF);
    msg_0x3F1->out_voltage <<=0x08;
    msg_0x3F1->out_voltage |= (uint8_t)((_data &0xFF00) >>0x08);
    /** 电子锁状态 */
    msg_0x3F1->elock_state = APP_CHARGER_LV_ELOCK_UNLOCK;
    if(thaisenElectLock_StateQuery(gunno) != thaisen_elock_break){
        msg_0x3F1->elock_state = APP_CHARGER_LV_ELOCK_LOCK;
    }
    /** 充电连接器状态 */
    msg_0x3F1->clinker_state = APP_CHARGER_LV_CLINKER_DISCONNECT;
    if(ofsm->base.flag.connect_state == APP_CONNECT_STATE_CONNECT){
        msg_0x3F1->clinker_state = APP_CHARGER_LV_CLINKER_CONNECTED;
    }
    /** 循环计数  */
    msg_0x3F1->counter++;
    msg_0x3F1->reserve1 = 0x00;
    /** 校验和 */
    msg_0x3F1->check = app_bms_lv_check_sum((const uint8_t*)msg_0x3F1, (sizeof(charger_app_0x3F1) - 0x01));
}

/*************************************************
 * 函数名           app_bms_lv_msg_0x0F3_padding
 * 功能               填充0x3F3报文
 * 参数              gunno    枪号
 *         msg      指向报文体
 * 返回
 ************************************************/
static void app_bms_lv_msg_0x0F3_padding(uint8_t gunno, can_msg_buf *msg)
{
    if((gunno >= APP_SYSTEM_GUNNO_SIZE) || (msg == NULL)){
        return;
    }
    struct ofsm_info *ofsm = get_ofsm_info(gunno);
    charger_app_0x3F3 *msg_0x3F3 = (charger_app_0x3F3*)(msg->data);
    uint32_t *f_pool = thaisenGetSysFault(gunno);

    msg->CANID = APP_LV_MODULE_BMS_MSG_ID_0x3F3;
    msg->length = APP_LV_MODULE_BMS_MSG_LEN_0x3F3;
    memset(msg->data, 0x00, sizeof(msg->data));
    /** 充电请求  */
    switch(ofsm->state){
    case APP_OFSM_STATE_WAIT_NET:
    case APP_OFSM_STATE_IDLEING:
    case APP_OFSM_STATE_READYING:
    case APP_OFSM_STATE_RESERVATION:
        s_app_lv_bms_info.flag[gunno].is_starting = APP_THA_ENUM_FALSE;
        msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_IDLE;
        break;
    case APP_OFSM_STATE_STARTING:
        /** 已发起充电 */
        if(s_app_lv_bms_info.flag[gunno].is_starting == APP_THA_ENUM_TRUE){
            msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_END_SELFCHECK;
        }else{
//            msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_SELFCHECK;
            msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_END_SELFCHECK;
        }
        break;
    case APP_OFSM_STATE_CHARGING:
        msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_END_SELFCHECK;
        break;
    case APP_OFSM_STATE_STOPING:
    case APP_OFSM_STATE_FINISHING:
        s_app_lv_bms_info.flag[gunno].is_starting = APP_THA_ENUM_FALSE;
        msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_IDLE;
        break;
    case APP_OFSM_STATE_FAULTING:
    case APP_OFSM_STATE_SIZE:
        s_app_lv_bms_info.flag[gunno].is_starting = APP_THA_ENUM_FALSE;
        msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_FAULTING;
        break;
    default:
        s_app_lv_bms_info.flag[gunno].is_starting = APP_THA_ENUM_FALSE;
        msg_0x3F3->request_cmd = APP_CHARGER_LV_CMD_FAULTING;
        break;
    }
    /** 急停故障 */
    msg_0x3F3->f_emergency = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenFaultScram)){
        msg_0x3F3->f_emergency = APP_THA_ENUM_TRUE;
    }
    /** 防雷器故障 */
    msg_0x3F3->f_protect_lighting = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenFaultMainCabinet_LightProtect)){
        msg_0x3F3->f_protect_lighting = APP_THA_ENUM_TRUE;
    }
    /** 绝缘外侧电压异常 */
    msg_0x3F3->f_gun_volt = APP_THA_ENUM_FALSE;
    /** 泄放故障 */
    msg_0x3F3->f_relief = APP_THA_ENUM_FALSE;
    /** 电子锁故障 */
    msg_0x3F3->f_elock = APP_THA_ENUM_FALSE;
    /** 交流输入缺相 */
    msg_0x3F3->f_ac_miss_phase = APP_THA_ENUM_FALSE;
    /** 绝缘故障 */
    msg_0x3F3->f_insulation = APP_THA_ENUM_FALSE;
    /** 控制导引故障 */
    msg_0x3F3->f_guidance = APP_THA_ENUM_FALSE;
    /** BMS需求电流过高 */
    msg_0x3F3->f_need_oc = APP_THA_ENUM_FALSE;
    /** 充电模块故障 */
    msg_0x3F3->f_module = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenChargModule)){
        msg_0x3F3->f_module = APP_THA_ENUM_TRUE;
    }
    /** 充电枪过温故障 */
    msg_0x3F3->f_gun_ot = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenFaultOverTemp)){
        msg_0x3F3->f_gun_ot = APP_THA_ENUM_TRUE;
    }
    /** BMS需求电压过高 */
    msg_0x3F3->f_need_ov = APP_THA_ENUM_FALSE;
    /** 直流接触器故障 */
    msg_0x3F3->f_dcrelay = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenRelay)){
        msg_0x3F3->f_dcrelay = APP_THA_ENUM_TRUE;
    }
    /** 软起外侧电压异常 */
    msg_0x3F3->f_precharge_volt = APP_THA_ENUM_FALSE;
    /** 辅组电源故障 */
    msg_0x3F3->f_auxpower = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenAuxPower)){
        msg_0x3F3->f_auxpower = APP_THA_ENUM_TRUE;
    }
    /** 交流断路器故障 */
    msg_0x3F3->f_breaker = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenFaultCircuitBreaker)){
        msg_0x3F3->f_breaker = APP_THA_ENUM_TRUE;
    }
    /** 门禁故障 */
    msg_0x3F3->f_gate = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenDoor)){
        msg_0x3F3->f_gate = APP_THA_ENUM_TRUE;
    }
    /** 掉电故障 */
    msg_0x3F3->f_poweroff = APP_THA_ENUM_FALSE;
    /** 交流接触器故障 */
    msg_0x3F3->f_acrelay = APP_THA_ENUM_FALSE;
    if(app_bms_lv_is_fault_occured(f_pool, thaisenRelayAc)){
        msg_0x3F3->f_acrelay = APP_THA_ENUM_TRUE;
    }
    msg_0x3F3->reserve0 = 0x00;
    msg_0x3F3->reserve1 = 0x00;
    msg_0x3F3->reserve2 = 0x00;
    /** 循环计数  */
    msg_0x3F3->counter++;
    /** 校验和 */
    msg_0x3F3->check = app_bms_lv_check_sum((const uint8_t*)msg_0x3F3, (sizeof(charger_app_0x3F3) - 0x01));
}

void app_bms_lv_can_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

#define BMS_APP_0X3F1_CYCLE                100              /** 报文发送周期 */
#define BMS_APP_0X3F3_CYCLE                100              /** 报文发送周期 */

#define BMS_APP_MSG_NUM                    2                /** 需要发送的报文数量 */
#define BMS_APP_MSG_INDEX_0X3F1            0                /** 需要发送的报文下标 */
#define BMS_APP_MSG_INDEX_0X3F3            1                /** 需要发送的报文下标 */

    uint8_t gunno = 0x00;
    struct ofsm_info *ofsm = NULL;
    can_msg_buf msg[APP_SYSTEM_GUNNO_SIZE][BMS_APP_MSG_NUM];
    uint32_t msg_send_tick[APP_SYSTEM_GUNNO_SIZE][BMS_APP_MSG_NUM], tick_temp;

    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
        for(uint8_t j = 0x00; j < BMS_APP_MSG_NUM; j++){
            msg_send_tick[i][j] = rt_tick_get();
            memset(&msg[i][j], 0x00, sizeof(can_msg_buf));
        }
    }

    while(1)
    {
        tick_temp = rt_tick_get();
        /** 线程监控 */
        app_thread_monitor_process(rt_thread_self(), NULL, 0x00, 0x00);

        switch (get_ofsm_info(0x00)->base.ota_state) {
        case APP_OTA_STATE_NULL:
            break;
        case APP_OTA_STATE_UP:
        case APP_OTA_STATE_LINK_UP:
        case APP_OTA_STATE_INTERNET_UP:
            break;
        case APP_OTA_STATE_AUTHING:
            break;
        case APP_OTA_STATE_AUTH_SUCCESS:
        case APP_OTA_STATE_UPDATEING:
            rt_thread_mdelay(5000);
            continue;                /** OTA时不执行 */
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            ofsm = get_ofsm_info(gunno);
            /** BMS离线检测 */
            if(s_app_lv_bms_info.flag[gunno].is_offline == APP_THA_ENUM_FALSE){
                if(s_app_lv_bms_info.offline_count[gunno] < (0xFFFF - 0x01)){
                    s_app_lv_bms_info.offline_count[gunno]++;
                }
                /** BMS 离线检测时间大概5s = (5000 /10) */
                if(s_app_lv_bms_info.offline_count[gunno] > 500){
                    s_app_lv_bms_info.flag[gunno].is_offline = APP_THA_ENUM_TRUE;
                }
            }
            /**************** 定期发送 0x3F1 报文 ****************/
            if(msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F1] > tick_temp){
                msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F1] = tick_temp;
            }
            if((tick_temp - msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F1]) >= BMS_APP_0X3F1_CYCLE){
                msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F1] = tick_temp;
                /** 报文填充 */
                app_bms_lv_msg_0x0F1_padding(gunno, &msg[gunno][BMS_APP_MSG_INDEX_0X3F1]);
                /** 启动、充电、完成时发送 */
                if((ofsm->state >= APP_OFSM_STATE_STARTING) && (ofsm->state <= APP_OFSM_STATE_STOPING)){
                    /** 发送报文 */
                    if(gunno == APP_SYSTEM_GUNNOA){
                        thaisen_bmsA_can_send(&msg[gunno][BMS_APP_MSG_INDEX_0X3F1]);
                    }else{
                        thaisen_bmsB_can_send(&msg[gunno][BMS_APP_MSG_INDEX_0X3F1]);
                    }
                }
            }
            /**************** 定期发送 0x3F3 报文 ****************/
            if(msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F3] > tick_temp){
                msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F3] = tick_temp;
            }
            if((tick_temp - msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F3]) >= BMS_APP_0X3F3_CYCLE){
                msg_send_tick[gunno][BMS_APP_MSG_INDEX_0X3F3] = tick_temp;
                /** 报文填充 */
                app_bms_lv_msg_0x0F3_padding(gunno, &msg[gunno][BMS_APP_MSG_INDEX_0X3F3]);
                /** 启动、充电、完成时发送 */
                if((ofsm->state >= APP_OFSM_STATE_STARTING) && (ofsm->state <= APP_OFSM_STATE_STOPING)){
                    /** 发送报文 */
                    if(gunno == APP_SYSTEM_GUNNOA){
                        thaisen_bmsA_can_send(&msg[gunno][BMS_APP_MSG_INDEX_0X3F3]);
                    }else{
                        thaisen_bmsB_can_send(&msg[gunno][BMS_APP_MSG_INDEX_0X3F3]);
                    }
                }
            }
        }
        rt_thread_mdelay(10);
    }

}

/*************************************************
 * 函数名           app_bms_lv_get_svolt_max
 * 功能               获取电池最高单体电压
 * 参数              gunno    枪号
 * 返回              电池最高单体电压(0.01V)
 ************************************************/
uint16_t app_bms_lv_get_svolt_max(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return s_bms_app_info[gunno].svolt_max;
}

/*************************************************
 * 函数名           app_bms_lv_get_temp_max
 * 功能               获取电池最高温度
 * 参数              gunno    枪号
 * 返回              电池最高温度(0.1度)
 ************************************************/
int8_t app_bms_lv_get_temp_max(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return s_bms_app_info[gunno].temp_max;
}

/*************************************************
 * 函数名           app_bms_lv_get_target_volt
 * 功能               获取充电目标电压
 * 参数              gunno    枪号
 * 返回              充电目标电压(0.01V)
 ************************************************/
uint16_t app_bms_lv_get_target_volt(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return s_bms_app_info[gunno].target_volt;
}

/*************************************************
 * 函数名           app_bms_lv_get_target_curr
 * 功能               获取充电目标电流
 * 参数              gunno    枪号
 * 返回              充电目标电压(0.01A)
 ************************************************/
uint16_t app_bms_lv_get_target_curr(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return s_bms_app_info[gunno].target_curr;
}

/*************************************************
 * 函数名           app_bms_lv_get_cmd
 * 功能               获取充电指令
 * 参数              gunno    枪号
 * 返回              充电指令@bms_lv_cmd
 ************************************************/
uint8_t app_bms_lv_get_cmd(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_BMSLV_CMD_SIZE;
    }
    return s_bms_app_info[gunno].cmd;
}

/*************************************************
 * 函数名           app_bms_lv_is_offline
 * 功能               判断BMS是否已离线
 * 参数              gunno    枪号
 * 返回              1：是    0：否
 ************************************************/
uint8_t app_bms_lv_is_offline(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_THA_ENUM_TRUE;
    }
    return s_app_lv_bms_info.flag[gunno].is_offline;
}

/*************************************************
 * 函数名           app_bms_lv_start_charge
 * 功能               开始充电
 * 参数              gunno    枪号
 * 返回
 ************************************************/
void app_bms_lv_start_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    s_app_lv_bms_info.flag[gunno].is_starting = APP_THA_ENUM_TRUE;
}

/*************************************************
 * 函数名           app_bms_lv_stop_charge
 * 功能               停止充电
 * 参数              gunno    枪号
 * 返回
 ************************************************/
void app_bms_lv_stop_charge(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }
    s_app_lv_bms_info.flag[gunno].is_starting = APP_THA_ENUM_FALSE;
}

/*************************************************
 * 函数名           app_bms_lv_get_start_state
 * 功能               获取启动状态
 * 参数              gunno    枪号
 * 返回              1：已启动      0：未启动
 ************************************************/
uint8_t app_bms_lv_get_start_state(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    return s_app_lv_bms_info.flag[gunno].is_starting;
}

#endif /* CP_USING_LV_MODULE_BMS */

/********************* 应用 CAN 部分信息初始化 *********************/
#ifdef APP_DESIGNATE_REGION
void app_app_can_info_init(void)
{
#ifdef USING_TCU_CAN
    memset(&s_can_info, 0x00, sizeof(s_can_info));
    memset(s_can_data, 0x00, sizeof(s_can_data));
#endif /* USING_TCU_CAN */

#ifdef CP_USING_LV_MODULE_BMS
    memset(&s_app_lv_bms_info, 0x00, sizeof(s_app_lv_bms_info));
    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
        memset(&s_bms_app_info[i], 0x00, sizeof(s_bms_app_info[i]));
        s_bms_app_info[i].cmd = APP_BMSLV_CMD_SIZE;
        s_app_lv_bms_info.flag[i].is_offline = APP_THA_ENUM_FALSE;
    }
#endif /* CP_USING_LV_MODULE_BMS */
    /** CAN 报文接收回调注册 */
    thaisen_user_can_cb_register(THAISEN_BMS_A_CAN_ENUM, app_bsm_a_can_cb);
    thaisen_user_can_cb_register(THAISEN_BMS_B_CAN_ENUM, app_bsm_b_can_cb);
}
#endif /* APP_DESIGNATE_REGION */

