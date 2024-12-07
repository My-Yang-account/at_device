/**
  ******************************************************************************
  * @file
  * @brief 终端实现，用于与上位机的交互
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __APP_TERMINAL_H
#define __APP_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

void terminal_thread_entry(void *parameter);
void terminal_req_thread_entry(void *parameter);
uint8_t terminal_is_ems_adjust_power(void);
void terminal_clear_is_ems_adjust_power(void);
uint16_t terminal_get_ems_set_power(void);
uint16_t terminal_get_ems_soc(void);
uint16_t terminal_get_ems_voltage();
uint16_t terminal_get_ems_current();

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
