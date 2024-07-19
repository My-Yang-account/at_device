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

#include "thaisenChargLib.h"
#include "thaisen7102Public.h"
#include "thaisenChargModuleLib.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

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

    thaisen_board_bsp_init();
    app_nfunc_config_init();
    prepose_init();

    SerialScreen_InputInfoGet();
    SerialScreen_SetInputInfo();
    rt_thread_mdelay(100);

    thaisenChargInit();
	thaisen_chargModule_Init(thaisen_get_charg_status, mw_get_bms_data(0), mw_get_bms_data(1),(struct thasienModuleSetStruct *)sys_get_module_config_info());
	MX_IWDG_Init();
    app_init();
	app_hci_init();

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

        mw_running_led_toggle(0, 0);

        extern uint8_t app_nsal_is_remote_reset(void);
        if(app_nsal_is_remote_reset()){
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
        }
    }

    return RT_EOK;
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
