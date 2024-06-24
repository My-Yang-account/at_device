/**
 ******************************************************************************
 * @file mw_temp.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_TEMP_H_
#define MW_TEMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief 获取环境温度
 * @return 环境温度（范围：-50 ~ 130 °C）
 */
int32_t mw_get_temp_dcp(uint8_t gunno);
int32_t mw_get_temp_dcn(uint8_t gunno);
enum temp_check mw_temperature_protect_check(uint8_t gunno, int32_t temperature);

enum temp_check{
    TCHECK_RESULT_NORMAL,            /* 温度正常 */
    TCHECK_RESULT_WARNNING,          /* 温度告警 */
    TCHECK_RESULT_OVERTEMP_1,        /* 一级过温 */
    TCHECK_RESULT_OVERTEMP_2,        /* 二级过温 */
    TCHECK_RESULT_OVERTEMP_3,        /* 三级过温 */
};

#ifdef __cplusplus
}
#endif

#endif /* MW_TEMP_H_ */
