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
#include "rtthread.h"
#include "thaisen7102Public.h"

#ifdef USING_TCU_CAN

#define APP_KS_CHARGE_DATA_PERIOD       250                     /* 科式充电数据发送间隔(ms) */

#define APP_KS_CAN_ID                   0x18F6C1C0              /* 科式充电数据CANID */

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

void app_tcan_thread_entry(void *parameter)
{
    extern int32_t app_thread_monitor_process(void *thread, void *para, uint32_t plen, uint32_t option);

    uint8_t gunno = 0x00;
    can_msg_buf can_send_message;
    can_send_message.DLC = 0x08;
    can_send_message.length = 0x08;
    can_send_message.priority = 0x06;
    memset(can_send_message.data, 0x00, sizeof(can_send_message.data));
    uint32_t charge_data_tick = rt_tick_get();

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
        if((rt_tick_get() - charge_data_tick) > APP_KS_CHARGE_DATA_PERIOD){
            charge_data_tick = rt_tick_get();
            if(ks_padding_charge_info(can_send_message.data, sizeof(can_send_message.data), gunno) >= 0){
                can_send_message.CANID = APP_KS_CAN_ID;
                thaisen_tcu_can_send(&can_send_message);
            }
        }
        if(++gunno >= APP_SYSTEM_GUNNO_SIZE){
            gunno = 0x00;
        }
        rt_thread_mdelay(50);
    }
}
#endif /* USING_TCU_CAN */
