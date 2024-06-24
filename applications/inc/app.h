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

#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

//#define USING_DOUBLE_PLATFORM    /* 使用双平台 */
#define USING_DOUBLE_GUN         /* 使用双枪 */
//#define USING_DAQI               /* 启用大气版本 */

void app_led_init(void);
void app_hci_init(void);
void app_operation_init(void);
void app_terminal_init(void);
void app_init(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
