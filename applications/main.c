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

#include <rtthread.h>
#include <stdbool.h>
#include "prepose.h"
#include "app.h"
#include "app_ofsm.h"
#include "version.h"
#include <board.h>

#include "mw_led.h"
#include "mw_iwdg.h"

#include "chargepile_config.h"
#include "app_state_check.h"
#include "app_module.h"

#include "thaisenChargLib.h"
#include "thaisen7102Public.h"
#include "thaisenChargModuleLib.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//void rt_kprintf(const char *fmt, ...)
//{
//
//}

extern uint32_t g_net_target_platform_tick;
static uint32_t s_net_alive_tick;

int main(void)
{
    extern struct thaisenBMS_Charger_struct* mw_get_bms_data(uint8_t gunno);
    extern void *sys_get_module_config_info(void);
    extern void SerialScreen_InputInfoGet(void);
    extern void SerialScreen_SetInputInfo(void);
    extern int32_t app_nfunc_config_init(void);
    extern void app_state_guidance_changed(thaisenGuidanceInfo_t info, uint8_t flag, uint8_t port);
    extern void app_state_device_status_changed(uint8_t device, void *parameter, uint8_t plen, uint8_t port);
    extern int app_state_system_data(uint8_t port, uint8_t name, void *parameter, uint8_t pLen);
    extern void mw_charglib_register_get_sysdata_cb(void *cb);

    extern void app_system_delay(uint32_t ms);

    LOG_I("current program version: V%d.%d.%c\n", SOFTWARE_VERSION, SOFTWARE_SUBVERSION, (SOFTWARE_REVISION + 'A'));

    /** 等待所有器件初始化正常，以下部分的初始化是在关闭中断的条件下进行的，无法实现OS延时 */
    rt_thread_mdelay(2000);

#ifdef APP_DESIGNATE_REGION
    extern void app_ofsm_info_init(void);
    app_ofsm_info_init();

    extern void app_billingrule_info_init(void);
    app_billingrule_info_init();

    extern void app_data_info_interface_init(void);
    app_data_info_interface_init();

    extern void app_hci_info_init(void);
    app_hci_info_init();

    extern void app_osupport_info_init(void);
    app_osupport_info_init();

    extern void app_support_func_info_init(void);
    app_support_func_info_init();

    extern void app_terminal_info_init(void);
    app_terminal_info_init();

    extern void sys_chargeplie_config_info_init(void);
    sys_chargeplie_config_info_init();

    extern void notfs_info_init(void);
    notfs_info_init();

    extern void app_application_info_init(void);
    app_application_info_init();
#endif /* APP_DESIGNATE_REGION */

    rt_base_t level;
    level = rt_hw_interrupt_disable();

    SerialScreen_SetInputInfo();  /* �ϵ���������ϼ��ʹ�� */

#ifdef APP_DESIGNATE_REGION
    extern void app_app_can_info_init(void);
    app_app_can_info_init();
#endif /* APP_DESIGNATE_REGION */

#ifndef APP_USING_DOUBLEGUN
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();
    MX_SPI3_Init();
    MX_CAN2_Init();
    MX_RTC_Init();
    MX_ADC1_Init();
    MX_CAN1_Init();
    thaisen_dma_init();
    MX_I2C1_Init();
    MX_TIM1_Init();
    MX_IWDG_Init();
    thaisenW25qXX_init();
    get_eeprom_para();
    thaisenCCVoltInit();



    thaisen_led_init();
    thaisen_ammeter_device_init();
    thaisen_SysFaultCheck_device_init();
    thaisenTempInit();
    TH_HardwareData_init();
#else
    thaisen_board_bsp_init();
#endif /* APP_USING_DOUBLEGUN */

    thaisen_GuidanceChangedCallback_Register(app_state_guidance_changed);
    thaisenDeviceChangedCallbackRegister(app_state_device_status_changed);

    prepose_init();

    extern int ec20_device_register(void);
    ec20_device_register();

    app_nfunc_config_init();

    SerialScreen_InputInfoGet();
    SerialScreen_SetInputInfo();
    app_system_delay(100);

#ifndef APP_USING_LV_MODULE_BMS
    /** 带BMS的低压模块版本不使用充电库(要控制风扇) */
    thaisenChargInit();
    mw_charglib_register_get_sysdata_cb(app_state_system_data);
#endif /* APP_USING_LV_MODULE_BMS */

	thaisen_chargModule_Init(thaisen_get_charg_status, mw_get_bms_data(0), mw_get_bms_data(1),(struct thasienModuleSetStruct *)sys_get_module_config_info());

    SerialScreen_SetInputInfo();

	MX_IWDG_Init();
    app_init();
	app_module_ctrl_init();
    app_hci_init();
	app_state_check_init();

#ifndef APP_USING_DOUBLEGUN
    TH_CAN1_FilterConf();
    TH_CAN2_FilterConf();
#endif /* APP_USING_DOUBLEGUN */
    extern void chargepile_power_adjust(void);
    extern void app_set_system_reset_event(uint8_t event, uint8_t state);

    rt_hw_interrupt_enable(level);

    for(uint8_t i = 0x00; i < APP_SYSTEM_GUNNO_SIZE; i++){
        get_ofsm_info(i)->base.reset_reason = RCC->CSR;
    }
    RCC->CSR |= 0x1000002;

    while (1)
    {
        mw_iwdg_refresh();

        rt_thread_mdelay(200);
        mw_iwdg_refresh();
        chargepile_power_adjust();

        rt_thread_mdelay(200);
        mw_iwdg_refresh();
        chargepile_power_adjust();

        rt_thread_mdelay(200);
        mw_iwdg_refresh();
        chargepile_power_adjust();

        rt_thread_mdelay(200);
        mw_iwdg_refresh();
        chargepile_power_adjust();

        rt_thread_mdelay(200);
        mw_iwdg_refresh();
        chargepile_power_adjust();

        if(app_nsal_get_link_state() == APP_NET_STATE_AUTH_SECCESS){
            s_net_alive_tick = rt_tick_get();
            app_set_system_reset_event(APP_SYS_RESET_NET_DISCONNECT, 0x00);
            app_set_system_reset_event(APP_SYS_RESET_NET_THREAD_STOP, 0x00);
        }

        mw_running_led_toggle(0, 0);

        if(get_ofsm_info(0x00)->base.run_mode == APP_RUN_MODE_4G_ETH){
            if(g_net_target_platform_tick > rt_tick_get()){
                if((rt_tick_get() + 0xFFFFFFFF - g_net_target_platform_tick) > 5 *60 *1000){
                    app_set_system_reset_event(APP_SYS_RESET_NET_THREAD_STOP, 0x01);
                }
            }else{
                if((rt_tick_get() - g_net_target_platform_tick) > 5 *60 *1000){
                    app_set_system_reset_event(APP_SYS_RESET_NET_THREAD_STOP, 0x01);
                }
            }

            if(s_net_alive_tick > rt_tick_get()){
                if((rt_tick_get() + 0xFFFFFFFF - s_net_alive_tick) > 90 *60000){
                    app_set_system_reset_event(APP_SYS_RESET_NET_DISCONNECT, 0x01);
                }
            }else{
                if((rt_tick_get() - s_net_alive_tick) > 90 *60000){
                    app_set_system_reset_event(APP_SYS_RESET_NET_DISCONNECT, 0x01);
                }
            }
        }else{
            g_net_target_platform_tick = rt_tick_get();
            s_net_alive_tick = rt_tick_get();
            app_set_system_reset_event(APP_SYS_RESET_NET_DISCONNECT, 0x00);
            app_set_system_reset_event(APP_SYS_RESET_NET_THREAD_STOP, 0x00);
        }

        extern uint8_t app_nsal_is_remote_reset(void);
        extern uint8_t thaisen_query_screen_reboot(void);
        extern void thaisen_clear_screen_reboot(void);
        extern void thaisen_set_screen_reboot(void);
        extern uint8_t app_system_monitor_need_reset(void);
        if(app_nsal_is_remote_reset() || thaisen_query_screen_reboot() || app_system_monitor_need_reset()){
            uint8_t gunno = 0x00;
            for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                if(get_ofsm_info(gunno)->state != APP_OFSM_STATE_IDLEING){
                    break;
                }
            }
            /** 屏幕重启生效前已确认所有枪都空闲 */
            if((gunno == APP_SYSTEM_GUNNO_SIZE) || thaisen_query_screen_reboot()){
                LOG_D("remote reset system");
                /** 在此处需要保存重启信息 */
                extern uint8_t app_thread_monitor_occur_error(void);
                extern char *app_thread_monitor_get_err_thread_name(void);
                extern void app_nsal_storage_thread_monitor_info(char *name);
                /** 控制屏幕返回首页 */
                thaisen_set_screen_reboot();
                if(app_thread_monitor_occur_error()){
                    LOG_D("storage thread monitor error info:%s", app_thread_monitor_get_err_thread_name());
                    /** 保存错误线程名 */
                    app_nsal_storage_thread_monitor_info(app_thread_monitor_get_err_thread_name());
                }
                rt_thread_mdelay(5000);
                __set_FAULTMASK(1);
                NVIC_SystemReset();
            }

            thaisen_clear_screen_reboot();
        }
    }

    return RT_EOK;
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
