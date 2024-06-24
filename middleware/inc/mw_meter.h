/**
 ******************************************************************************
 * @file mw_meter.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_METER_H_
#define MW_METER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t mw_get_meter_ua(uint8_t gunno);
uint32_t mw_get_meter_ub(uint8_t gunno);
uint32_t mw_get_meter_uc(uint8_t gunno);

uint32_t mw_get_meter_uab(uint8_t gunno);
uint32_t mw_get_meter_ubc(uint8_t gunno);
uint32_t mw_get_meter_uca(uint8_t gunno);

uint32_t mw_get_meter_ia(uint8_t gunno);
uint32_t mw_get_meter_ib(uint8_t gunno);
uint32_t mw_get_meter_ic(uint8_t gunno);

uint32_t mw_get_meter_pa(uint8_t gunno);
uint32_t mw_get_meter_pb(uint8_t gunno);
uint32_t mw_get_meter_pc(uint8_t gunno);
uint32_t mw_get_meter_p(uint8_t gunno);

uint32_t mw_get_meter_qa(uint8_t gunno);
uint32_t mw_get_meter_qb(uint8_t gunno);
uint32_t mw_get_meter_qc(uint8_t gunno);
uint32_t mw_get_meter_q(uint8_t gunno);

uint32_t mw_get_meter_total_wh(uint8_t gunno);

#ifdef __cplusplus
}
#endif

#endif /* MW_METER_H_ */
