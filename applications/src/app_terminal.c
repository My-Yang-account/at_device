/**
  ******************************************************************************
  * @file
  * @brief 终端实现，用于与上位机的交互
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#include <rtthread.h>

#include <stdio.h>
#include <stdlib.h>

#include "chargepile_config.h"

#include "bsp_serial.h"

#include "app_terminal.h"
#include "app_ofsm.h"

#include "mw_fault_check.h"
#include "mw_cc1.h"
#include "mw_fault_check.h"
#include "app_data_info_interface.h"

#define DBG_TAG "app.terminal"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct terminal_data
{
    uint8_t buffer[256];
    uint32_t length;
};

static struct terminal_data s_terminal_data;

static uint8_t terminal_get_char(void)
{
    uint8_t ch = 0x00;

    while (rt_device_read(g_terminal_serial, -1, &ch, 1) != 1) {
        rt_sem_take(&g_terminal_rx_sem, RT_WAITING_FOREVER);
    }

    return ch;
}

void terminal_thread_entry(void *parameter)
{
    (void)parameter;

    uint8_t i, j, ch = 0, count = 0, isenable, state, data[256] = {0};
    memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));

    while (1)
    {
        ch = terminal_get_char();

        if (0x20 > ch) {
            LOG_D("receive terminal data: 0x%02X", ch);
        } else {
            LOG_D("receive terminal data: %c, 0x%02X", ch, ch);
        }

        if (s_terminal_data.length >= sizeof(s_terminal_data.buffer)) {
            s_terminal_data.length = 0;
            LOG_E("terminal receive data out of buff size|%d", sizeof(s_terminal_data.buffer));
        }
        s_terminal_data.buffer[s_terminal_data.length++] = ch;

        if (s_terminal_data.buffer[0] != 'U') {
            memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
            continue;
        }
        else
        {
            if (4 > s_terminal_data.length){
                continue;
            }
            else
            {
                if ((s_terminal_data.buffer[1] != 'C') || (s_terminal_data.buffer[2] != 'A') || (s_terminal_data.buffer[3] != 'C'))
                {
                    memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
                    continue;
                }
            }
        }

        if ((s_terminal_data.buffer[s_terminal_data.length - 1] == '=') && (0 == count)) {
            count = s_terminal_data.length - 1;
            LOG_D("CHAR '=' INDEX IS -> %d", count);
        }

        if (!((s_terminal_data.buffer[s_terminal_data.length - 1] == 0x0A) && (s_terminal_data.buffer[s_terminal_data.length - 2] == 0x0D))) {
            continue;
        }

        if (0 == count) {
            LOG_D("terminal read data");
            if (0 == memcmp(s_terminal_data.buffer, "UCACRDpileInfo", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDpileInfo OK\r\n");
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%s%s", "pileNum=", (char *)(sys_read_config_item_content(CONFIG_ITEM_PILE_NUMBER, 0)), "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d.%d.%d%s", "version=", SOFTWARE_VERSION, SOFTWARE_SUBVERSION, SOFTWARE_REVISION, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%s%s", "ipv4=", (char *)(sys_read_config_item_content(CONFIG_ITEM_IP_DOMAIN, 0)), "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%u%s", "port=", *((unsigned int*)(sys_read_config_item_content(CONFIG_ITEM_PORT, 0))), "\r\n");
                terminal_send_data((char *)&data);
                LOG_D("terminal read is UCACRDpileInfo");
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACRDpileStatus", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDpileStatus OK\r\n");
                memset(data, 0x00, sizeof(data));

                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    switch (get_ofsm_info(gunno)->state) {
                    case APP_OFSM_STATE_WAIT_NET:
                        state = 7;
                        break;
                    case APP_OFSM_STATE_IDLEING:
                        state = 0;
                        break;
                    case APP_OFSM_STATE_READYING:
                        state = 1;
                        break;
                    case APP_OFSM_STATE_STARTING:
                        state = 5;
                        break;
                    case APP_OFSM_STATE_CHARGING:
                        state = 3;
                        break;
                    case APP_OFSM_STATE_STOPING:
                        state = 6;
                        break;
                    case APP_OFSM_STATE_FINISHING:
                        state = 3;
                        break;
                    case APP_OFSM_STATE_FAULTING:
                        state = 4;
                        break;
                    default:
                        state = 0xFF;
                        break;
                    }
                    sprintf((char *)&data, "%s%c%c%d%s", "pileStatus Gun", ('A' + gunno), '=', state, "\r\n");
                    terminal_send_data((char *)&data);
                }
                LOG_D("terminal read is UCACRDpileStatus");
            }else if (0 == memcmp(s_terminal_data.buffer, "UCACRDchargeInfo", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDchargeInfo OK\r\n");
                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%d%s", "cc1Volt Gun", ('A' + gunno), '=', mw_get_cc1_value(mw_get_cc1(gunno)), "\r\n");
                    terminal_send_data((char *)&data);
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%.2f%s", "chargeVolt Gun", ('A' + gunno), '=', get_ofsm_info(gunno)->base.voltage_a / 100.0f, "\r\n");
                    terminal_send_data((char *)&data);
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%.2f%s", "chargeCurr Gun", ('A' + gunno), '=', get_ofsm_info(gunno)->base.current_a / 100.0f, "\r\n");
                    terminal_send_data((char *)&data);
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%.3f%s", "chargePower Gun", ('A' + gunno), '=', get_ofsm_info(gunno)->base.power_a / 10.0f, "\r\n");
                    terminal_send_data((char *)&data);
                }
                LOG_D("terminal read is UCACRDchargeInfo");
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACRDpileConfig", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDpileConfig OK\r\n");
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "Leakage=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "adhesionSingle=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "adhesionDouble=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "gndCheck=", 0, "\r\n");
                terminal_send_data((char *)&data);
                memset(data, 0x00, sizeof(data));
                sprintf((char *)&data, "%s%d%s", "shortCheck=", 0, "\r\n");
                terminal_send_data((char *)&data);
                LOG_D("terminal read is UCACRDpileConfig");
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACRDfaultInfo", s_terminal_data.length - 3)) {
                terminal_send_data("ACUCRDfaultInfo OK\r\n");
                for(uint8_t gunno = 0; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                    memset(data, 0x00, sizeof(data));
                    sprintf((char *)&data, "%s%c%c%d%s", "faultCode Gun", ('A' + gunno), '=', (uint8_t)app_get_highest_priority_system_fault(gunno), "\r\n");
                    terminal_send_data((char *)&data);
                }
                LOG_D("terminal read is UCACRDfaultInfo");
            }
        } else {
            LOG_D("terminal write data (%d)", count);
            if (0 == memcmp(s_terminal_data.buffer, "UCACWDpileNum", count)) {
                LOG_D("terminal write is UCACWDpileNum");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';

                if (0 > sys_sync_config_item_content(CONFIG_ITEM_PILE_NUMBER, data, j)) {
                    terminal_send_data("ACUCWDpileNum ERROR\r\n");
                    LOG_D("terminal configure charging-pile number failed");
                } else {
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDpileNum ERROR\r\n");
                        LOG_D("terminal configure charging-pile number failed");
                    }else{
                        terminal_send_data("ACUCWDpileNum OK\r\n");
                        LOG_D("terminal configure charging-pile number success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDqrCode", count)) {
                LOG_D("terminal write is UCACWDqrCode");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                if(sys_sync_config_item_content(CONFIG_ITEM_QRCODE_PRE, data, j) < 0){
                    terminal_send_data("ACUCWDqrCode ERROR\r\n");
                    LOG_D("terminal configure qrcode failed");
                }else{
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDqrCode ERROR\r\n");
                        LOG_D("terminal configure qrcode failed");
                    }else{
                        terminal_send_data("ACUCWDqrCode OK\r\n");
                        LOG_D("terminal configure qrcode success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDip", count)) {
                LOG_D("terminal write is UCACWDip");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i]; /* 获取数据 */
                }
                data[j] = '\0';
                if (0 > sys_sync_config_item_content(CONFIG_ITEM_IP_DOMAIN, data, j)) {
                    terminal_send_data("ACUCWDip ERROR\r\n");
                    LOG_D("terminal configure ip addr failed");
                } else {
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDip ERROR\r\n");
                        LOG_D("terminal configure ip addr failed");
                    }else{
                        terminal_send_data("ACUCWDip OK\r\n");
                        LOG_D("terminal configure ip addr success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDport", count)) {
                uint32_t port = 0;

                LOG_D("terminal write is UCACWDport");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                port = (uint32_t)(atoi((const char *)data));

                if (0 > sys_sync_config_item_content(CONFIG_ITEM_PORT, &port, j)) {
                    terminal_send_data("ACUCWDport ERROR\r\n");
                    LOG_D("terminal configure port addr failed");
                } else {
                    if(0 > sys_storage_config_item()){
                        terminal_send_data("ACUCWDport ERROR\r\n");
                        LOG_D("terminal configure port addr failed");
                    }else{
                        terminal_send_data("ACUCWDport OK\r\n");
                        LOG_D("terminal configure port addr success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDleakage", count)) {
                LOG_D("terminal write is UCACWDleakage");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDleakage ERROR\r\n");
                    LOG_D("terminal configure leakage failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDleakage ERROR\r\n");
                        LOG_D("terminal configure leakage failed");
                    } else {
                        terminal_send_data("ACUCWDleakage OK\r\n");
                        LOG_D("terminal configure leakage success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDadhesionSingle", count)) {
                LOG_D("terminal write is UCACWDadhesionSingle");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDadhesionSingle ERROR\r\n");
                    LOG_D("terminal configure single-adhesion failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDadhesionSingle ERROR\r\n");
                        LOG_D("terminal configure single-adhesion failed");
                    } else {
                        terminal_send_data("ACUCWDadhesionSingle OK\r\n");
                        LOG_D("terminal configure single-adhesion success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDadhesionDouble", count)) {
                LOG_D("terminal write is UCACWDadhesionDouble");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDadhesionDouble ERROR\r\n");
                    LOG_D("terminal configure double-adhesion failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDadhesionDouble ERROR\r\n");
                        LOG_D("terminal configure double-adhesion failed");
                    } else {
                        terminal_send_data("ACUCWDadhesionDouble OK\r\n");
                        LOG_D("terminal configure double-adhesion success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDgndCheck", count)) {
                LOG_D("terminal write is UCACWDgndCheck");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDgndCheck ERROR\r\n");
                    LOG_D("terminal configure ground failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDgndCheck ERROR\r\n");
                        LOG_D("terminal configure ground failed");
                    } else {
                        terminal_send_data("ACUCWDgndCheck OK\r\n");
                        LOG_D("terminal configure ground success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDshort", count)) {
                LOG_D("terminal write is UCACWDshort");
                isenable = s_terminal_data.buffer[count + 1];
                if ((isenable != '0') && (isenable != '1')) {
                    terminal_send_data("ACUCWDshort ERROR\r\n");
                    LOG_D("terminal configure short-circuit failed (0x%X)", isenable);
                } else {
                    if (1) {
                        terminal_send_data("ACUCWDshort ERROR\r\n");
                        LOG_D("terminal configure short-circuit failed");
                    } else {
                        terminal_send_data("ACUCWDshort OK\r\n");
                        LOG_D("terminal configure short-circuit success");
                    }
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDADDWhiteList", count)) {
                LOG_D("terminal write is UCACWDADDWhiteList");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                if (1) {
                    terminal_send_data("ACUCWDADDWhiteList ERROR\r\n");
                    LOG_D("terminal append white-list failed");
                } else {
                    terminal_send_data("ACUCWDADDWhiteList OK\r\n");
                    LOG_D("terminal append white-list success");
                }
            } else if (0 == memcmp(s_terminal_data.buffer, "UCACWDDELWhiteList", count)) {
                LOG_D("terminal write is UCACWDDELWhiteList");
                for (i = count + 1, j = 0; i < s_terminal_data.length - 2; i++) {
                    data[j++] = s_terminal_data.buffer[i];
                }
                data[j] = '\0';
                if (1) {
                    terminal_send_data("ACUCWDDELWhiteList ERROR\r\n");
                    LOG_D("terminal delete white-list failed");
                } else {
                    terminal_send_data("ACUCWDDELWhiteList OK\r\n");
                    LOG_D("terminal delete white-list success");
                }
            } else {
                LOG_D("terminal write data error");
            }
        }

        count = 0;
        memset(&s_terminal_data, 0x00, sizeof(s_terminal_data));
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
