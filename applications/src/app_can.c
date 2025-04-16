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
#include "app.h"
#include "app_ofsm.h"
#include "app_osupport.h"
#include "rtthread.h"
#include "thaisen7102Public.h"
#include "thaisenBMS.h"

#ifdef USING_TCU_CAN

#define APP_KS_FUNCTION_CMD_CAN_ID      0x18F5C1C0              /* 科式功能指令帧CANID */

#define APP_KS_CHARGE_DATA_CAN_ID       0x18F6C1C0              /* 科式充电数据CANID */
#define APP_KS_BMS_DATA_01_CAN_ID       0x18F7C1C0              /* 科式BMS数据01CANID */
#define APP_KS_BMS_DATA_02_CAN_ID       0x18F8C1C0              /* 科式BMS数据02CANID */

#define APP_TCU_CAN_MQ_SIZE             10                      /* CAN报文接收个数 */

#pragma pack(1)
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
        uint8_t reserve : 4;                                    /** 预留 */
    }flag;
};
#pragma pack()

APP_DEF_SRAM1 struct can_info s_can_info;
APP_DEF_SRAM1 static struct can_data s_can_data[APP_TCU_CAN_MQ_SIZE];
APP_DEF_SRAM1 static struct rt_messagequeue s_tcu_can_mq;


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

static int8_t ks_padding_charge_info(uint8_t* data, uint8_t len, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return -0x01;
    }
    if(data && len >= 0x08){
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

            value = ofsm->base.current_a;
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
            rt_thread_mdelay(100);
        }
        if(ks_padding_bms_info_01(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_BMS_DATA_01_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(100);
        }
        if(ks_padding_bms_info_02(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
            can_send_message.CANID = APP_KS_BMS_DATA_02_CAN_ID;
            thaisen_tcu_can_send(&can_send_message);
            rt_thread_mdelay(100);
        }

        if(++gunno >= APP_SYSTEM_GUNNO_SIZE){
            gunno = 0x00;
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

#ifdef APP_DESIGNATE_REGION
void app_tcu_can_info_init(void)
{
#ifdef USING_TCU_CAN
    memset(&s_can_info, 0x00, sizeof(s_can_info));
    memset(s_can_data, 0x00, sizeof(s_can_data));
#endif /* USING_TCU_CAN */
}
#endif /* APP_DESIGNATE_REGION */
