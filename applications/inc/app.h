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

#define USING_DOUBLE_GUN                     /* 使用双枪 */
#define USING_THREAD_MONITOR                 /* 使用线程监控 */
#define USING_TCU_CAN                        /* 使用TCU CAN */

#define APP_THREAD_MONITOR_OPT_URGENT        (1 <<0)     /* 线程监控选项：紧急(无需判断，直接处理) */
#define APP_THREAD_MONITOR_OPT_ENTRY         (1 <<1)     /* 线程监控选项：最大容忍次数 */
#define APP_THREAD_MONITOR_OPT_NAME          (1 <<2)     /* 线程监控选项：线程名字 */

#define APP_SYS_RESET_THREAD_MONITOR         0x00        /* 系统重启事件：线程监控 */
#define APP_SYS_RESET_NET_DISCONNECT         0x01        /* 系统重启事件：长时间断网 */
#define APP_SYS_RESET_NET_THREAD_STOP        0x02        /* 系统重启事件：网络线程不运行 */
#define APP_SYS_RESET_NDEV_ERROR             0x03        /* 系统重启事件：网络设备故障 */

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
