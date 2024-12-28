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

#include <string.h>

#include "app_osupport.h"

#include "mw_fault_check.h"
#include "mw_time.h"
#include "mw_storage.h"

#include "net_operation.h"

#include "thaisen7102Public.h"

#define DBG_TAG "app.fault"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define APP_ALLOW_CHARGE_FAULT_MASK    (0xFFFFFFF9)
#define APP_LIBRARY_DETECT_FAULT_MASK  (0xFFFFFFFD)

#ifdef APP_INCLUDE_SGCC_PROTOCOL
#define APP_STOPWAY_FAULT_INFO_OCCUR           0x00          /** 停充原因型故障：故障产生 */
#define APP_STOPWAY_FAULT_INFO_RESUME          0x01          /** 停充原因型故障：故障恢复 */
#define APP_STOPWAY_FAULT_INFO_NULL            0x02          /** 停充原因型故障：无 */
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

#ifdef APP_INCLUDE_SGCC_PROTOCOL
static uint8_t s_stopway_fault_flag[APP_SYSTEM_GUNNO_SIZE];  /** 停充原因类型故障(0：故障产生，1：故障恢复，2：无) */
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

static uint32_t s_system_fault_last[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_SYSTEM_FAULT_SET_NUM];
static uint32_t s_system_fault_current[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_SYSTEM_FAULT_SET_NUM];
static uint8_t s_charge_fault_last[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_CHARGE_FAULT_SET_NUM];   /* 由于当前充电故障数量小于8个，所以使用uint8_t 型 */
static uint8_t s_charge_fault_current[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_CHARGE_FAULT_SET_NUM];   /* 由于当前充电故障数量小于8个，所以使用uint8_t 型 */

static struct error_info s_system_error_info[APP_SYSTEM_GUNNO_SIZE];
static struct error_info s_charge_error_info[APP_SYSTEM_GUNNO_SIZE];
#ifdef APP_INCLUDE_SGCC_PROTOCOL
static uint32_t s_stopway_fault[APP_SYSTEM_GUNNO_SIZE];
static struct error_info s_stopway_error_info[APP_SYSTEM_GUNNO_SIZE];
#endif /* APP_INCLUDE_SGCC_PROTOCOL */

/************************************************
 * 函数名         app_query_system_fault_set
 * 功能             查询系统故障集
 * 参数             gunno    枪号
 * 返回
 ***********************************************/
static uint32_t *app_query_system_fault_set(uint8_t gunno)
{
    return mw_get_system_fault_set(gunno);
}


/************************************************
 * 函数名         app_charge_fault_occur
 * 功能             充电故障产生
 * 参数             gunno    枪号
 *       stopway  停充原因
 *       charge_fault 充电故障码(集)
 * 返回
 ***********************************************/
void app_charge_fault_occur(uint8_t gunno, uint32_t stopway, uint32_t charge_fault)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }

    rt_enter_critical();
#ifdef APP_INCLUDE_SGCC_PROTOCOL
    s_stopway_fault[gunno] = stopway;
    s_stopway_fault_flag[gunno] = APP_STOPWAY_FAULT_INFO_OCCUR;
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    for(uint8_t set = 0x00; set <APP_GENERAL_CHARGE_FAULT_SET_NUM; set++){
        s_charge_fault_current[gunno][set] = charge_fault;
    }
    rt_exit_critical();
}
/************************************************
 * 函数名         app_charge_fault_resume
 * 功能             充电故障恢复
 * 参数             gunno    枪号
 *       stopway  停充原因
 *       charge_fault 充电故障码(集)
 * 返回
 ***********************************************/
void app_charge_fault_resume(uint8_t gunno, uint32_t stopway, uint32_t charge_fault)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }

    rt_enter_critical();
#ifdef APP_INCLUDE_SGCC_PROTOCOL
    s_stopway_fault[gunno] = stopway;
    s_stopway_fault_flag[gunno] = APP_STOPWAY_FAULT_INFO_RESUME;
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    for(uint8_t set = 0x00; set <APP_GENERAL_CHARGE_FAULT_SET_NUM; set++){
        s_charge_fault_current[gunno][set] = charge_fault;
    }
    rt_exit_critical();
}

uint8_t app_exist_forbid_charge_fault(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }

    if(s_system_fault_current[gunno][APP_GENERAL_SYSTEM_FAULT_SET_LOW] &APP_ALLOW_CHARGE_FAULT_MASK){
        return 0x01;
    }
    return 0x00;
}

void app_shield_allow_charge_fautl(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }

    thaisenClearSysFaultCheckBit(APP_SYS_FAULT_CARD_READER);
    thaisenClearSysFaultCheckBit(APP_SYS_FAULT_DOOR);

    thaisenClearSysFaultLib(APP_SYS_FAULT_CARD_READER, gunno);
    thaisenClearSysFaultLib(APP_SYS_FAULT_DOOR, gunno);
}

void app_enable_detect_allow_charge_fautl(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return;
    }

    thaisenSetSysFaultCheckBit(APP_SYS_FAULT_CARD_READER);
    thaisenSetSysFaultCheckBit(APP_SYS_FAULT_DOOR);
}

enum charge_fault_t app_get_highest_priority_charge_fault(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_CHARGE_FAULT_NO_ERROR;
    }

    uint8_t bit = 0x00, start_bit = 0x00, end_bit = 0x00, remain_bit = 0x00, set = 0x00;

    end_bit = APP_CHARGE_FAULT_NO_ERROR >= 32 ? 32 : APP_CHARGE_FAULT_NO_ERROR;
    remain_bit = APP_CHARGE_FAULT_NO_ERROR - end_bit;

    for(set = 0x00; set < APP_GENERAL_SYSTEM_FAULT_SET_NUM; set++){
        for(bit = 0; bit < (end_bit - start_bit); bit++){
            if(s_charge_fault_last[gunno][set] &(1 <<(bit + start_bit))){
                return (enum charge_fault_t)(bit + start_bit);
            }
        }
        start_bit = end_bit;
        end_bit = remain_bit > 32 ? 32 : remain_bit;
        end_bit += start_bit;
        remain_bit = APP_CHARGE_FAULT_NO_ERROR - end_bit;
    }

    return APP_CHARGE_FAULT_NO_ERROR;
}

enum system_fault_t app_get_highest_priority_system_fault(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return APP_SYS_FAULT_NO_ERROR;
    }

    uint8_t bit = 0x00, start_bit = 0x00, end_bit = 0x00, remain_bit = 0x00, set = 0x00;

    end_bit = APP_SYS_FAULT_NO_ERROR >= 32 ? 32 : APP_SYS_FAULT_NO_ERROR;
    remain_bit = APP_SYS_FAULT_NO_ERROR - end_bit;

    for(set = 0x00; set < APP_GENERAL_SYSTEM_FAULT_SET_NUM; set++){
        for(bit = 0; bit < (end_bit - start_bit); bit++){
            if(s_system_fault_last[gunno][set] &(1 <<(bit + start_bit))){
                return (enum system_fault_t)(bit + start_bit);
            }
        }
        start_bit = end_bit;
        end_bit = remain_bit > 32 ? 32 : remain_bit;
        end_bit += start_bit;
        remain_bit = APP_SYS_FAULT_NO_ERROR - end_bit;
    }

    return APP_SYS_FAULT_NO_ERROR;
}

uint8_t *app_get_charge_fault_set(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return NULL;
    }
    return s_charge_fault_last[gunno];
}

uint32_t *app_get_system_fault_set(uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return NULL;
    }
    return s_system_fault_current[gunno];
}

void app_set_system_fault_enum(enum system_fault_t _fault, uint8_t set, uint8_t gunno)
{
    if(set >= APP_GENERAL_SYSTEM_FAULT_SET_NUM){
        return;
    }
    if(_fault >= APP_SYS_FAULT_NO_ERROR){
        return;
    }
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        s_system_fault_current[set][gunno] |= (1 <<_fault);
    }
}

uint8_t app_get_system_fault_enum(enum system_fault_t _fault, uint8_t set, uint8_t gunno)
{
    if(set >= APP_GENERAL_SYSTEM_FAULT_SET_NUM){
        return 0x00;
    }
    if(_fault >= APP_SYS_FAULT_NO_ERROR){
        return 0x00;
    }
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return 0x00;
    }
    if(s_system_fault_current[set][gunno] & (1 <<_fault)){
        return 0x01;
    }
    return 0x00;
}

void app_clear_system_fault_enum(enum system_fault_t _fault, uint8_t set, uint8_t gunno)
{
    if(set >= APP_GENERAL_CHARGE_FAULT_SET_NUM){
        return;
    }
    if(_fault >= APP_SYS_FAULT_NO_ERROR){
        return;
    }
    if(gunno < APP_SYSTEM_GUNNO_SIZE){
        s_system_fault_current[set][gunno] &= (~(1 <<_fault));
    }
}

static int32_t app_fault_storage(struct error_info *error_info, uint32_t resume_time, uint8_t gunno)
{
    if(gunno >= APP_SYSTEM_GUNNO_SIZE){
        return -0x01;
    }
    int32_t index = 0x00;
    uint8_t region = NOTFS_SUBREGION_GUNNOA_FAULT_RECORD;
    if(gunno != APP_SYSTEM_GUNNOA){
        region = NOTFS_SUBREGION_GUNNOB_FAULT_RECORD;
    }
    if(error_info->error_flag == 0x00){
        (void)mw_storage_record_create(error_info, sizeof(struct error_info), error_info->error_index, 0x00, 0x00, region);    /* 故障为产生，新存储一条记录 */
        LOG_D("gunno(%d) save fault info|%x, %x  start|%x", gunno, error_info->error_code, error_info->error_index, error_info->occur_time);
    }else{
        if((index = mw_storage_record_get_first_index_userdata(error_info->error_index, region)) < 0){
            (void)mw_storage_record_create(error_info, sizeof(struct error_info), error_info->error_index, 0x00, 0x00, region);
            return -0x02;
        }else{
            struct error_info rinfo;
            memset(&rinfo, 0x00, sizeof(struct error_info));
            if(mw_storage_record_get_designate_index_record((uint8_t*)(&rinfo), sizeof(struct error_info), region, index) < 0){  /* 故障为恢复，查找已存储的对应下标的故障 */
                (void)mw_storage_record_create(error_info, sizeof(struct error_info), error_info->error_index, 0x00, 0x00, region);
                return -0x03;
            }
            rinfo.error_flag = 0x01;
            memcpy(&rinfo.resume_time, &(error_info->resume_time), sizeof(error_info->resume_time));

            (void)mw_storage_record_designate_index_updated(&rinfo, sizeof(struct error_info), 0x00, 0x00, 0x01, region, index);
            LOG_D("gunno(%d) update fault info|%x, %x %d  resume|%x", gunno, rinfo.error_code, rinfo.error_index, resume_time);
        }
    }
    return 0x00;
}

void app_osupport_thread_entry(void *parameter)
{
    for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
        s_system_fault_last[gunno][APP_GENERAL_SYSTEM_FAULT_SET_LOW] = 0x00;
        s_system_fault_current[gunno][APP_GENERAL_SYSTEM_FAULT_SET_LOW] = 0x00;
#ifdef APP_INCLUDE_SGCC_PROTOCOL
        s_stopway_fault_flag[gunno] = APP_STOPWAY_FAULT_INFO_NULL;
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
    }
    uint8_t bit = 0x00, start_bit = 0x00, end_bit = 0x00, remain_bit = 0x00;
    uint32_t fault_xor = 0x00, fault_temp = 0x00, *current_fault_ptr = NULL;

    rt_thread_mdelay(8000);     /* 忽略上电前8s故障 */

    while (1)
    {
        switch (get_ofsm_info(0x00)->base.ota_state) {
        case APP_OTA_STATE_NULL:
            break;
        case APP_OTA_STATE_UP:
        case APP_OTA_STATE_LINK_UP:
        case APP_OTA_STATE_INTERNET_UP:
            break;
        case APP_OTA_STATE_AUTHING:
            break;
        case APP_OTA_STATE_AUTH_SUCCESS:
        case APP_OTA_STATE_UPDATEING:
            rt_thread_mdelay(5000);
            continue;
            break;
        case APP_OTA_STATE_UPDATE_SECCESS:
        case APP_OTA_STATE_UPDATE_FAILED:
            break;
        default:
            break;
        }

        for(uint8_t gunno = 0x00; gunno < APP_SYSTEM_GUNNO_SIZE; gunno++){
            bit = 0x00;
            start_bit = 0x00;
            end_bit = APP_SYS_FAULT_NO_ERROR >= 32 ? 32 : APP_SYS_FAULT_NO_ERROR;
            remain_bit = APP_SYS_FAULT_NO_ERROR - end_bit;
            current_fault_ptr = app_query_system_fault_set(gunno);

            for(uint8_t set = 0x00; set < APP_GENERAL_SYSTEM_FAULT_SET_NUM; set++){
                fault_temp = current_fault_ptr[set];

                fault_temp &= APP_LIBRARY_DETECT_FAULT_MASK;
                s_system_fault_current[gunno][set] &= ~(APP_LIBRARY_DETECT_FAULT_MASK);
                s_system_fault_current[gunno][set] |= fault_temp;

                fault_xor = s_system_fault_current[gunno][set] ^s_system_fault_last[gunno][set];
                /*************************** 系统故障检测 **********************************/
                if(fault_xor){
                    for(bit = 0; bit < (end_bit - start_bit); bit++){
                        if(fault_xor &(1 <<bit)){
                            switch(mw_system_fault_convert(bit + start_bit))
                            {
                            case APP_SYS_FAULT_SCRAM:
                                s_system_error_info[gunno].error_index = 0x0000;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_SCRAM;
                                break;
                            case APP_SYS_FAULT_CARD_READER:
                                s_system_error_info[gunno].error_index = 0x0001;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_CARDREADER;
                                break;
                            case APP_SYS_FAULT_DOOR:
                                s_system_error_info[gunno].error_index = 0x0002;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_DOOR;
                                break;
                            case APP_SYS_FAULT_AMMETER:
                                s_system_error_info[gunno].error_index = 0x0003;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_AMMETER;
                                break;
                            case APP_SYS_FAULT_CHARGE_MODULE:
                                s_system_error_info[gunno].error_index = 0x0004;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_CHARGEMODULE;
                                break;
                            case APP_SYS_FAULT_OVER_TEMP:
                                s_system_error_info[gunno].error_index = 0x0005;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_OVERTEMP;
                                break;
                            case APP_SYS_FAULT_OVER_VOLT:
                                s_system_error_info[gunno].error_index = 0x0006;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_OVERVOLT;
                                break;
                            case APP_SYS_FAULT_UNDER_VOLT:
                                s_system_error_info[gunno].error_index = 0x0007;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_UNDERVOLT;
                                break;
                            case APP_SYS_FAULT_OVER_CURR:
                                s_system_error_info[gunno].error_index = 0x0008;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_OVERCURRENT;
                                break;
                            case APP_SYS_FAULT_RELAY:
                                s_system_error_info[gunno].error_index = 0x0009;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_RELAY;
                                break;
                            case APP_SYS_FAULT_PARALLEL_RELAY:
                                s_system_error_info[gunno].error_index = 0x000A;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_PARALLEL_RELAY;
                                break;
                            case APP_SYS_FAULT_AC_RELAY:
                                s_system_error_info[gunno].error_index = 0x000B;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_AC_RELAY;
                                break;
                            case APP_SYS_FAULT_ELOCK:
                                s_system_error_info[gunno].error_index = 0x000C;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_ELECTRY_LOCK;
                                break;
                            case APP_SYS_FAULT_AUXPOWER:
                                s_system_error_info[gunno].error_index = 0x000D;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_AUXPOWER;
                                break;
                            case APP_SYS_FAULT_FLASH:
                                s_system_error_info[gunno].error_index = 0x000E;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_FLASH;
                                break;
                            case APP_SYS_FAULT_EEPROM:
                                s_system_error_info[gunno].error_index = 0x000F;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_EEPROM;
                                break;
                            case APP_SYS_FAULT_LIGHT_PRPTECT:
                                s_system_error_info[gunno].error_index = 0x0010;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_LIGHTPROTECT;
                                break;
                            case APP_SYS_FAULT_GUN_SITE:
                                s_system_error_info[gunno].error_index = 0x0011;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_GUNSITE;
                                break;
                            case APP_SYS_FAULT_CIRCUIT_BREAKER:
                                s_system_error_info[gunno].error_index = 0x0012;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_CIRCUIT_BREAKER;
                                break;
                            case APP_SYS_FAULT_FLOODING:
                                s_system_error_info[gunno].error_index = 0x0013;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_FLOODING;
                                break;
                            case APP_SYS_FAULT_SMOKE:
                                s_system_error_info[gunno].error_index = 0x0014;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_SMOKE;
                                break;
                            case APP_SYS_FAULT_POUR:
                                s_system_error_info[gunno].error_index = 0x0015;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_POUR;
                                break;
                            case APP_SYS_FAULT_LIQUID_COOLING:
                                s_system_error_info[gunno].error_index = 0x0016;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_LIQUIDCOOLING;
                                break;
                            case APP_SYS_FAULT_FUSE:
                                s_system_error_info[gunno].error_index = 0x0017;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_FUSE;
                                break;
                            case APP_SYS_FAULT_MAIN_CABINET:
                                s_system_error_info[gunno].error_index = 0x0018;
                                s_system_error_info[gunno].error_code = APP_SYSTEM_STOP_WAY_MAIN_CABINET;
                                break;
                            default:
                                break;
                            }

                            if(s_system_fault_last[gunno][set] &(1 <<(bit + start_bit))){
                                s_system_error_info[gunno].error_flag = 0x01;  /* 故障恢复 */
                                s_system_error_info[gunno].resume_time = mw_get_current_timestamp();
                                app_nsal_system_fault_report(gunno, s_system_error_info[gunno].error_code, s_system_error_info[gunno].resume_time, 0x01);
                            }else{
                                s_system_error_info[gunno].error_flag = 0x00;  /* 故障产生 */
                                memset(&s_system_error_info[gunno].resume_time, 0x00, sizeof(s_system_error_info[gunno].resume_time));
                                s_system_error_info[gunno].occur_time = mw_get_current_timestamp();

                                app_nsal_system_fault_report(gunno, s_system_error_info[gunno].error_code, s_system_error_info[gunno].occur_time, 0x00);
                            }
                            app_fault_storage(&s_system_error_info[gunno], s_system_error_info[gunno].resume_time, gunno);
                        }
                    }
                    s_system_fault_last[gunno][set] = s_system_fault_current[gunno][set];
                }
                start_bit = end_bit;
                end_bit = remain_bit > 32 ? 32 : remain_bit;
                end_bit += start_bit;
                remain_bit = APP_SYS_FAULT_NO_ERROR - end_bit;
            }

            /*************************** 充电故障检测 **********************************/
            bit = 0,
            start_bit = 0,
            end_bit = APP_CHARGE_FAULT_NO_ERROR >= 32 ? 32 : APP_CHARGE_FAULT_NO_ERROR;
            remain_bit = APP_CHARGE_FAULT_NO_ERROR - end_bit;

            for(uint8_t set = 0; set < APP_GENERAL_CHARGE_FAULT_SET_NUM; set++){
                uint32_t _fault_set = s_charge_fault_current[gunno][set];
                fault_xor = _fault_set ^s_charge_fault_last[gunno][set];
                if(fault_xor){
                    for(bit = 0; bit < (end_bit - start_bit); bit++){
                        if(fault_xor &(1 <<bit)){
                            uint8_t _cfault = 0x00;
                            switch((bit + start_bit))
                            {
                            case APP_CHARGE_FAULT_GUN_VOLT:
                                s_charge_error_info[gunno].error_index = 0x00FFFFFF;
                                s_charge_error_info[gunno].error_code = mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_GUNVOLT);
                                _cfault = APP_SYSTEM_STOP_WAY_GUNVOLT;
                                break;
                            case APP_CHARGE_FAULT_INSULTA:
                                s_charge_error_info[gunno].error_index = 0x00FFFFFE;
                                s_charge_error_info[gunno].error_code = mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_INSULT);
                                _cfault = APP_SYSTEM_STOP_WAY_INSULT;
                                break;
                            case APP_CHARGE_FAULT_COMMON:
                                s_charge_error_info[gunno].error_index = 0x00FFFFFD;
                                s_charge_error_info[gunno].error_code = mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_COMMINICATION);
                                _cfault = APP_SYSTEM_STOP_WAY_COMMINICATION;
                                break;
                            case APP_CHARGE_FAULT_BATTERY_VOLT:
                                s_charge_error_info[gunno].error_index = 0x00FFFFFC;
                                s_charge_error_info[gunno].error_code = mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_BATTERY_VOLT);
                                _cfault = APP_SYSTEM_STOP_WAY_BATTERY_VOLT;
                                break;
                            case APP_CHARGE_FAULT_READY_VOLT:
                                s_charge_error_info[gunno].error_index = 0x00FFFFFB;
                                s_charge_error_info[gunno].error_code = mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_READY_VOLT);
                                _cfault = APP_SYSTEM_STOP_WAY_READY_VOLT;
                                break;
                            case APP_CHARGE_FAULT_INSULT_VOLT:
                                s_charge_error_info[gunno].error_index = 0x00FFFFFA;
                                s_charge_error_info[gunno].error_code = mw_system_stop_way_convert(APP_SYSTEM_STOP_WAY_INSULT_VOLT);
                                _cfault = APP_SYSTEM_STOP_WAY_INSULT_VOLT;
                                break;
                            default:
                                break;
                            }

                            if(s_charge_fault_last[gunno][set] &(1 <<(bit + start_bit))){
                                s_charge_error_info[gunno].error_flag = 0x01;  /* 故障恢复 */
                                s_charge_error_info[gunno].resume_time = mw_get_current_timestamp();
                                app_nsal_system_fault_report(gunno, _cfault, s_charge_error_info[gunno].resume_time, 0x01);
                            }else{
                                s_charge_error_info[gunno].error_flag = 0x00;  /* 故障产生 */
                                memset(&s_charge_error_info[gunno].resume_time, 0x00, sizeof(s_charge_error_info[gunno].resume_time));
                                s_charge_error_info[gunno].occur_time = mw_get_current_timestamp();

                                app_nsal_system_fault_report(gunno, _cfault, s_charge_error_info[gunno].occur_time, 0x00);
                            }
                            app_fault_storage(&s_charge_error_info[gunno], s_charge_error_info[gunno].resume_time, gunno);
                        }
                    }
                    s_charge_fault_last[gunno][set] = _fault_set;
                }
                start_bit = end_bit;
                end_bit = remain_bit > 32 ? 32 : remain_bit;
                end_bit += start_bit;
                remain_bit = APP_CHARGE_FAULT_NO_ERROR - end_bit;
            }

#ifdef APP_INCLUDE_SGCC_PROTOCOL
            /*************************** 停充原因类故障检测 **********************************/
            if(s_stopway_fault_flag[gunno] == APP_STOPWAY_FAULT_INFO_OCCUR){
                uint32_t _sfault = s_stopway_fault[gunno];
                s_stopway_fault_flag[gunno] = APP_STOPWAY_FAULT_INFO_NULL;
                if(_sfault == APP_SYSTEM_STOP_WAY_PULL_GUN){
                    s_stopway_error_info[gunno].error_flag = 0x00;  /* 故障产生 */
                    memset(&s_stopway_error_info[gunno].resume_time, 0x00, sizeof(s_stopway_error_info[gunno].resume_time));
                    s_stopway_error_info[gunno].occur_time = mw_get_current_timestamp();
                    s_stopway_error_info[gunno].error_index = 0xFFFF;
                    s_stopway_error_info[gunno].error_code = _sfault;

                    LOG_D("gunno(%d) occured stopway fault(%d)\n", gunno, APP_SYSTEM_STOP_WAY_PULL_GUN);
                    app_nsal_system_fault_report(gunno, APP_SYSTEM_STOP_WAY_PULL_GUN, s_stopway_error_info[gunno].occur_time, 0x00);

//                    app_fault_storage(&s_charge_error_info[gunno], s_charge_error_info[gunno].resume_time, gunno);
                }
            }else if(s_stopway_fault_flag[gunno] == APP_STOPWAY_FAULT_INFO_RESUME){
                uint32_t _sfault = s_stopway_fault[gunno];
                s_stopway_fault_flag[gunno] = APP_STOPWAY_FAULT_INFO_NULL;
                if(_sfault == APP_SYSTEM_STOP_WAY_PULL_GUN){
                    s_stopway_error_info[gunno].error_flag = 0x01;  /* 故障恢复 */
                    s_stopway_error_info[gunno].resume_time = mw_get_current_timestamp();
                    s_stopway_error_info[gunno].error_index = 0xFFFF;
                    s_stopway_error_info[gunno].error_code = _sfault;

                    LOG_D("gunno(%d) resume stopway fault(%d)\n", gunno, APP_SYSTEM_STOP_WAY_PULL_GUN);
                    app_nsal_system_fault_report(gunno, APP_SYSTEM_STOP_WAY_PULL_GUN, s_stopway_error_info[gunno].resume_time, 0x01);

//                    app_fault_storage(&s_charge_error_info[gunno], s_charge_error_info[gunno].resume_time, gunno);
                }
            }
#endif /* APP_INCLUDE_SGCC_PROTOCOL */
        }
        rt_thread_mdelay(500);
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
