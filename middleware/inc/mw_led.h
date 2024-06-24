/**
 ******************************************************************************
 * @file mw_led.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_LED_H_
#define MW_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

enum led {
    STATUS_LED = 0,
    RED_LED,
    GREEN_LED,
    BLUE_LED,
};

void mw_led_init(void);

/**
 * @brief 单个指示灯亮
 */
void mw_led_on_single(enum led no, uint8_t gunno);
/**
 * @brief 单个指示灯灭
 */
void mw_led_off_single(enum led no, uint8_t gunno);

/**
 * @brief 单个指示灯呼吸（不包含运行灯）
 */
void mw_led_breathing_single(enum led no, uint8_t gunno);
/**
 * @brief 运行指示灯翻转
 */
void mw_running_led_toggle(enum led no, uint8_t gunno);
/**
 * @brief 调整单个指示灯亮度（不包含运行灯）
 */
void mw_adjust_led_bright_single(enum led no, int percent, uint8_t gunno);
/**
 * @brief 仅单个指示灯亮（不包含运行灯）
 */
void mw_led_on_only(enum led no, uint8_t gunno);
/**
 * @brief 所有指示灯亮（不包含运行灯）
 */
void mw_led_on_all(uint8_t gunno);
/**
 * @brief 所有指示灯灭（不包含运行灯）
 */
void mw_led_off_all(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* MW_LED_H_ */
