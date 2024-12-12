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
    LOG_I("current program version: V%d.%d.%d\n", SOFTWARE_VERSION, SOFTWARE_SUBVERSION, SOFTWARE_REVISION);

    rt_base_t level;
    level = rt_hw_interrupt_disable();

    SerialScreen_SetInputInfo();  /* �ϵ���������ϼ��ʹ�� */

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
    thaisenCCVoltInit();



    thaisen_led_init();
    thaisen_ammeter_device_init();
    thaisen_SysFaultCheck_device_init();
    thaisenTempInit();
    get_eeprom_para();
    TH_HardwareData_init();
#else
    thaisen_board_bsp_init();
#endif /* APP_USING_DOUBLEGUN */
    prepose_init();
    app_nfunc_config_init();

    SerialScreen_InputInfoGet();
    SerialScreen_SetInputInfo();
    rt_thread_mdelay(100);

    thaisenChargInit();
	thaisen_chargModule_Init(thaisen_get_charg_status, mw_get_bms_data(0), mw_get_bms_data(1),(struct thasienModuleSetStruct *)sys_get_module_config_info());
	MX_IWDG_Init();
    app_init();
	app_hci_init();

#ifndef APP_USING_DOUBLEGUN
    TH_CAN1_FilterConf();
    TH_CAN2_FilterConf();
#endif /* APP_USING_DOUBLEGUN */
    extern void chargepile_power_adjust(void);

    rt_hw_interrupt_enable(level);

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
        }

        mw_running_led_toggle(0, 0);

        if((*(sys_read_config_item_content(CONFIG_ITEM_SUPORT_OFFLINE_BILLING, 0))) == 0x00){
            if(g_net_target_platform_tick > rt_tick_get()){
                if((rt_tick_get() + 0xFFFFFFFF - g_net_target_platform_tick) > 5 *60 *1000){
                    net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                }
            }else{
                if((rt_tick_get() - g_net_target_platform_tick) > 5 *60 *1000){
                    net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                }
            }

            if(s_net_alive_tick > rt_tick_get()){
                if((rt_tick_get() + 0xFFFFFFFF - s_net_alive_tick) > 90 *60000){
                    net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                }
            }else{
                if((rt_tick_get() - s_net_alive_tick) > 90 *60000){
                    net_operation_set_event(0x00, NET_OPERATION_EVENT_REBOOT);
                }
            }
        }else{
            g_net_target_platform_tick = rt_tick_get();
            s_net_alive_tick = rt_tick_get();
        }

        extern uint8_t app_nsal_is_remote_reset(void);
        extern uint8_t thaisen_query_screen_reboot(void);
        extern void thaisen_clear_screen_reboot(void);
        if(app_nsal_is_remote_reset() || thaisen_query_screen_reboot()){
            uint8_t gunno = 0x00;
            for(gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
                if(get_ofsm_info(gunno)->state != APP_OFSM_STATE_IDLEING){
                    break;
                }
            }
            if(gunno == APP_SYSTEM_GUNNO_SIZE){
                LOG_D("remote reset system");
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
