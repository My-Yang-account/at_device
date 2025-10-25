/**
  ******************************************************************************
  * @file
  * @brief 平台状态机
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __APP_OSUPPPORT_H
#define __APP_OSUPPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#define APP_GENERAL_SYSTEM_FAULT_SET_NUM          4
#define APP_GENERAL_CHARGE_FAULT_SET_NUM          1

#define APP_GENERAL_SYSTEM_FAULT_SET_0            0
#define APP_GENERAL_SYSTEM_FAULT_SET_1            1
#define APP_GENERAL_SYSTEM_FAULT_SET_2            2
#define APP_GENERAL_SYSTEM_FAULT_SET_3            3

#define APP_GENERAL_CHARGE_FAULT_SET_LOW          0
#define APP_GENERAL_CHARGE_FAULT_SET_HIGH         1

#pragma pack(1)
struct error_info
{
    uint8_t error_flag;    /* 故障产生、恢复标志，0：故障产生， 1：故障恢复 */
    uint32_t error_index;
    uint32_t error_code;
    uint32_t occur_time;   /* 故障发生时间 */
    uint32_t resume_time;  /* 故障发生时间 */
};
#pragma pack()

uint8_t app_exist_forbid_charge_fault(uint8_t gunno);
void app_shield_allow_charge_fautl(uint8_t gunno);
void app_enable_detect_allow_charge_fautl(uint8_t gunno);
void app_charge_fault_occur(uint8_t gunno, uint32_t stopway, uint32_t charge_fault);
void app_charge_fault_resume(uint8_t gunno, uint32_t stopway, uint32_t charge_fault);
enum charge_fault_t app_get_highest_priority_charge_fault(uint8_t gunno);
enum system_fault_t app_get_highest_priority_system_fault(uint8_t gunno);

uint8_t *app_get_charge_fault_set(uint8_t gunno);
uint32_t *app_get_system_fault_set(uint8_t gunno);

void app_set_system_fault_enum(enum system_fault_t _fault, uint8_t gunno);
uint8_t app_get_system_fault_enum(enum system_fault_t _fault, uint8_t gunno);
void app_clear_system_fault_enum(enum system_fault_t _fault, uint8_t gunno);

void app_osupport_thread_entry(void *parameter);

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
