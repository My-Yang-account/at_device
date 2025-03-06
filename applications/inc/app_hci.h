/**
  ******************************************************************************
  * @file
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __APP_HCI_H
#define __APP_HCI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "string.h"
#include "app_ofsm.h"

enum hci_event{
    HCI_EVENT_SCREEN_START,    /* 屏幕启动 */
    HCI_EVENT_SCREEN_STOP,     /* 屏幕停止 */
    HCI_EVENT_VIN_START,       /* VIN启动 */
    HCI_EVENT_PASSWORD_START,  /* 密码启动 */
};

void app_hci_req_thread_entry(void *parameter);
void app_hci_res_thread_entry(void *parameter);

uint8_t app_get_hci_event(uint8_t gunno, uint16_t event, uint8_t is_clear);
void app_set_hci_event(uint8_t gunno, uint16_t event);

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
