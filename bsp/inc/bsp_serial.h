/**
  ******************************************************************************
  * @file
  * @author
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __BSP_SERIAL_H
#define __BSP_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

/*********************************供应用层使用*********************************/
extern rt_device_t g_terminal_serial;
extern struct rt_semaphore g_terminal_rx_sem;
extern rt_device_t g_hci_serial;
extern struct rt_semaphore g_hci_rx_sem;

void terminal_send_data(char *data);
void hci_send_data(uint8_t *data, uint8_t len);

int32_t bsp_terminal_serial_init(void);
int32_t bsp_hci_serial_init(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
