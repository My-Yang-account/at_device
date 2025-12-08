/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-05-23     我的杨yang       the first version
 */
#ifndef NET_INTERFACE_INC_NET_SAL_H_
#define NET_INTERFACE_INC_NET_SAL_H_

#include "net_operation.h"
#include "tha_message_padding.h"
#include "ykc_message_padding.h"
#include "ykc_monitor_message_padding.h"
#include "ycp_message_padding.h"
#include "sgcc_message_padding.h"

/**************************** 钛昕平台 ***************************/
#ifdef NET_PACK_USING_THA
#define APP_INCLUDE_NET

#ifdef NET_THA_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0001   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0001   /* 目标平台ID */
#endif /* NET_THA_AS_MONITOR */

#define APP_INCLUDE_THA_PROTOCOL
#endif /* NET_PACK_USING_THA */
#define APP_PLATFORM_ID_THA                           0x0001   /* 钛昕平台ID */
/**************************** 云快充平台 ***************************/
#ifdef NET_PACK_USING_YKC
#define APP_INCLUDE_NET

#ifdef NET_YKC_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0002   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0002   /* 目标平台ID */
#endif /* NET_YKC_AS_MONITOR */

#ifdef NET_YKC_MESSAGE_USING_YKC17
#define APP_INCLUDE_YKC17_PROTOCOL                             /* 云快充1.7 */
#elif (defined NET_YKC_MESSAGE_USING_QC)
#define APP_INCLUDE_YKC17_PROTOCOL                             /* 云快充1.7 */
#define APP_INCLUDE_YKC_QC_PROTOCOL                             /* 云快充衍生清晨 */
#elif (defined NET_YKC_MESSAGE_USING_TT)
#define APP_INCLUDE_YKC_FIVE_PROTOCOL                             /* 云快充五段计费*/
#define APP_INCLUDE_YKC_TT_PROTOCOL                             /* 云快充衍生铁塔 */
#elif (defined NET_YKC_MESSAGE_USING_HHD)
#define APP_INCLUDE_YKC_HHD_PROTOCOL                             /* 云快充衍生海汇德 */
#elif (defined NET_YKC_MESSAGE_USING_LL)
#define APP_INCLUDE_YKC_LL_PROTOCOL                             /* 云快充衍生龙力 */
#elif (defined NET_YKC_MESSAGE_USING_CA)
#define APP_INCLUDE_YKC_CA_PROTOCOL                             /* 云快充衍生常安 */
#elif (defined NET_YKC_MESSAGE_USING_SMY)
#define APP_INCLUDE_YKC_SMY_PROTOCOL                             /* 云快充衍生神马云 */
#elif (defined NET_YKC_MESSAGE_USING_XDT)
#define APP_INCLUDE_YKC_XDT_PROTOCOL                             /* 云快充衍生新电途*/
#elif (defined NET_YKC_MESSAGE_USING_XXCD_FIVE)
#define APP_INCLUDE_YKC_XXCD_FIVE_PROTOCOL                             /* 云快充衍生星星充电五段计费*/
#elif (defined NET_YKC_MESSAGE_USING_TLD)
#define APP_INCLUDE_YKC_TLD_PROTOCOL                             /* 云快充衍生星星充电五段计费*/
#define APP_INCLUDE_YKC_FIVE_PROTOCOL                             /* 云快充五段计费*/
#else
#define APP_INCLUDE_YKC_FIVE_PROTOCOL                             /* 云快充五段计费*/
#endif /* ((defined NET_YKC_MESSAGE_USING_YKC17)*/


#define APP_INCLUDE_YKC_PROTOCOL
#endif /* NET_PACK_USING_YKC */
#define APP_PLATFORM_ID_YKC                           0x0002   /* 云快充平台ID */
/**************************** 小桔平台 ***************************/
#ifdef NET_PACK_USING_XJ
#define APP_INCLUDE_NET

#ifdef NET_XJ_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0004   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0004   /* 目标平台ID */
#endif /* NET_XJ_AS_MONITOR */

#define APP_INCLUDE_XJ_PROTOCOL
#endif /* NET_PACK_USING_XJ */
#define APP_PLATFORM_ID_XJ                            0x0004   /* 小桔平台ID */
/**************************** 阳光乐通平台 ***************************/
#ifdef NET_PACK_USING_SL
#define APP_INCLUDE_NET

#ifdef NET_SL_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0008   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0008   /* 目标平台ID */
#endif /* NET_SL_AS_MONITOR */

#define APP_INCLUDE_SL_PROTOCOL
#endif /* NET_PACK_USING_SL */
#define APP_PLATFORM_ID_SL                            0x0008   /* 阳光乐通平台ID */
/**************************** 云快充监控平台 ***************************/
#ifdef NET_PACK_USING_YKC_MONITOR
#define APP_INCLUDE_NET

#ifdef NET_YKC_MONITOR_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0010   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0010   /* 目标平台ID */
#endif /* NET_YKC_MONITOR_AS_MONITOR */

#define APP_INCLUDE_YKC_PROTOCOL_MONITOR
#endif /* NET_PACK_USING_YKC_MONITOR */
#define APP_PLATFORM_ID_YKC_MONITOR                   0x0010   /* 云快充监控平台ID */
/**************************** 越城平台 ***************************/
#ifdef NET_PACK_USING_YCP
#define APP_INCLUDE_NET

#ifdef NET_YCP_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0020   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0020   /* 目标平台ID */
#endif /* NET_YCP_AS_MONITOR */

#define APP_INCLUDE_YCP_PROTOCOL
#endif /* NET_PACK_USING_YCP */
#define APP_PLATFORM_ID_YCP                           0x0020   /* 越城平台ID */
/**************************** 国网平台 ***************************/
#ifdef NET_PACK_USING_SGCC
#define APP_INCLUDE_NET

#ifdef NET_SGCC_AS_MONITOR
#define APP_INCLUDE_MONITOR_PLATFORM                           /* 包含监控平台 */
#define APP_MONITOR_PLATFORM_ID                       0x0040   /* 监控平台ID */
#else
#define APP_INCLUDE_TARGET_PLATFORM                            /* 包含目标平台 */
#define APP_TARGET_PLATFORM_ID                        0x0040   /* 目标平台ID */
#endif /* NET_SGCC_AS_MONITOR */

#define APP_INCLUDE_SGCC_PROTOCOL
#endif /* NET_PACK_USING_SGCC */
#define APP_PLATFORM_ID_SGCC                          0x0040   /* 国网平台ID */
/****************************************************************/

void app_nsal_realtime_process(uint8_t gunno);
void app_nsal_system_fault_report(uint8_t gunno, uint32_t code, uint32_t timestamp, uint8_t is_resume);

void app_nsal_message_init(uint8_t gunno);
void app_nsal_state_charged(uint8_t gunno);
void app_nsal_event_occurded(uint8_t gunno);
void app_nsal_clear_remote_start(uint8_t gunno);
void app_nsal_clear_remote_stop(uint8_t gunno);
void app_nsal_clear_remote_card_authorize(uint8_t gunno);
void app_nsal_clear_remote_vin_authorize(uint8_t gunno);
uint8_t app_nsal_is_remote_reset(void);
void app_nsal_clear_remote_reset(void);

void app_nsal_time_sync_revise(uint8_t gunno);
void app_nsal_init_charge_data(uint8_t gunno);
void app_nsal_padding_charge_data(uint8_t gunno);
void app_nsal_transaction_record_report_monitor(uint8_t gunno, void *transaction, uint8_t is_repeat);
void app_nsal_transaction_record_report_target(uint8_t gunno, void *transaction, uint8_t is_repeat);

/*********************************************** [计费相关] **************************************/
uint8_t app_nsal_need_update_billingrule(uint8_t gunno);
void app_nsal_clear_update_billingrule_event(uint8_t gunno);

int8_t app_nsal_card_authorize(uint8_t gunno);
int8_t app_nsal_vin_authorize(uint8_t gunno);
void app_nsal_report_bms_message_bmsinfo(uint8_t gunno);
void app_nsal_report_bms_message_bmsparameter(uint8_t gunno);
void app_nsal_report_bms_message_end(uint8_t gunno);
void app_nsal_report_bms_message_error(uint8_t gunno);
void app_nsal_report_bms_message_bmsend(uint8_t gunno);
void app_nsal_report_bms_message_chargerend(uint8_t gunno);
void app_nsal_report_bms_message_bmsrequire_pileoutput(uint8_t gunno, uint8_t is_init);
void app_nsal_report_bms_message_bmsstate(uint8_t gunno, uint8_t is_init);
void app_nsal_create_local_transaction_number(uint8_t gunno, void *vector, uint8_t len);
uint8_t app_nsal_query_transaction_verify_state(uint8_t gunno, uint16_t platform_id);

uint8_t app_nsal_get_link_state(void);
uint8_t app_nsal_get_ota_start_flag(void);
uint8_t app_nsal_get_ota_state(void);
uint8_t app_nsal_get_ota_result(void);
uint32_t app_nsal_get_ota_progress(void);
uint8_t app_nsal_is_remote_start(uint8_t gunno);
uint8_t app_nsal_is_remote_stop(uint8_t gunno);
void app_nsal_report_remote_start_result(uint8_t gunno, uint8_t result, uint16_t reason, uint16_t fault);
void app_nsal_report_remote_stop_result(uint8_t gunno, uint8_t result, uint16_t reason, uint16_t fault);

uint8_t app_nsal_is_card_authorize_success(uint8_t gunno);
uint8_t app_nsal_is_vin_authorize_success(uint8_t gunno);
uint8_t app_nsal_is_card_authorize_fail(uint8_t gunno);
uint8_t app_nsal_is_vin_authorize_fail(uint8_t gunno);

void app_nsal_is_permit_ota(uint8_t state);

void app_nsal_report_set_power_result(uint8_t gunno, uint8_t result, uint8_t reason);
uint8_t app_nsal_is_set_power(uint8_t gunno);
void app_nsal_clear_set_power(uint8_t gunno);
uint32_t app_nsal_get_setup_power(uint8_t gunno);
uint8_t app_nsal_is_time_sync(void);
void app_nsal_clear_time_sync(void);

uint8_t app_nsal_is_set_reservation(uint8_t gunno);
void app_nsal_clear_set_reservation(uint8_t gunno);
uint8_t app_nsal_is_cancel_reservation(uint8_t gunno);
void app_nsal_clear_cancel_reservation(uint8_t gunno);

uint8_t app_nsal_offlinecharge_is_limit(uint8_t gunno);
void app_nsal_clear_offlinecharge_limit(uint8_t gunno);
void app_nsal_storage_thread_monitor_info(char *name);

void app_nsal_info_modify_notice(uint8_t info);

#endif /* NET_INTERFACE_INC_NET_SAL_H_ */
