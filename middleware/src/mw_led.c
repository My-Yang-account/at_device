/**
 ******************************************************************************
 * @file mw_led.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_led.h"
#include "thaisen7102Public.h"
#include "app_ofsm.h"

void mw_led_init(void)
{
   ;
}

void mw_adjust_led_bright_single(enum led no, int percent, uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
    switch(no)
    {
        case RED_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_C3_A(percent);
            }else{
#ifdef APP_USING_DOUBLEGUN
//                thaisen_led_red_B_on();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        case GREEN_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_C2_A(percent);
            }else{
#ifdef APP_USING_DOUBLEGUN
//                thaisen_led_red_B_on();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        case BLUE_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_C1_A(percent);
            }else{
#ifdef APP_USING_DOUBLEGUN
//                thaisen_led_red_B_on();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        default:
            break;
        }
    }
}

void mw_led_on_single(enum led no, uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        switch(no) {
        case STATUS_LED:
            break;
        case RED_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_red_A_on();
            }else{
#ifdef APP_USING_DOUBLEGUN
                thaisen_led_red_B_on();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        case GREEN_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_green_A_on();
            }else{
#ifdef APP_USING_DOUBLEGUN
                thaisen_led_green_B_on();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        case BLUE_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_blue_A_on();
            }else{
#ifdef APP_USING_DOUBLEGUN
                thaisen_led_blue_B_on();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        default:
            break;
        }
    }
}

void mw_led_off_single(enum led no, uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        switch(no) {
        case STATUS_LED:
            break;
        case RED_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_red_A_off();
            }else{
#ifdef APP_USING_DOUBLEGUN
                thaisen_led_red_B_off();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        case GREEN_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_green_A_off();
            }else{
#ifdef APP_USING_DOUBLEGUN
                thaisen_led_green_B_off();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        case BLUE_LED:
            if(gunno == APP_SYSTEM_GUNNOA){
                thaisen_led_blue_A_off();
            }else{
#ifdef APP_USING_DOUBLEGUN
                thaisen_led_blue_B_off();
#endif /* APP_USING_DOUBLEGUN */
            }
            break;
        default:
            break;
        }
    }
}

void mw_led_breathing_single(enum led no, uint8_t gunno)
{
    int brightness = 0;

    for(brightness = 0; brightness < 1000; brightness++){
        adjust_led_bright_single(no, brightness, gunno);
        /* 若是双枪则需增加枪判断 */
    }
}

void mw_running_led_toggle(enum led no, uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        if(gunno == APP_SYSTEM_GUNNOA){
            thaisen_led_board_toggle();
        }else{
#ifdef APP_USING_DOUBLEGUN
//                thaisen_led_red_B_on();
#endif /* APP_USING_DOUBLEGUN */
        }
    }
}

void mw_led_on_only(enum led no, uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        mw_led_off_all(gunno);
        mw_led_on_single(no, gunno);
    }
}

void mw_led_on_all(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        mw_led_on_single(RED_LED, gunno);
        mw_led_on_single(GREEN_LED, gunno);
        mw_led_on_single(BLUE_LED, gunno);
    }
}

void mw_led_off_all(uint8_t gunno)
{
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        mw_led_off_single(RED_LED, gunno);
        mw_led_off_single(GREEN_LED, gunno);
        mw_led_off_single(BLUE_LED, gunno);
    }
}
