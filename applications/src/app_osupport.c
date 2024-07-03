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

#include "net_interface.h"
#include "net_operation.h"

#include "thaisen7102Public.h"

#define DBG_TAG "app.fault"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define APP_ALLOW_CHARGE_FAULT_MASK    (0xFFFFFFF9)
#define APP_LIBRARY_DETECT_FAULT_MASK  (0xFFFFFFFD)

static uint32_t s_system_fault_last[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_SYSTEM_FAULT_SET_NUM];
static uint32_t s_system_fault_current[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_SYSTEM_FAULT_SET_NUM];
static uint8_t s_charge_fault_last[APP_SYSTEM_GUNNO_SIZE][APP_GENERAL_CHARGE_FAULT_SET_NUM];   /* 由于当前充电故障数量小于8个，所以使用uint8_t 型 */

static struct error_info s_system_error_info[APP_SYSTEM_GUNNO_SIZE];
static struct error_info s_charge_error_info[APP_SYSTEM_GUNNO_SIZE];

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
    if(_fault >= APP_USER_INFO_DEVICE_ERROR_SIZE){
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
    if(_fault >= APP_USER_INFO_DEVICE_ERROR_SIZE){
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
    if(_fault >= APP_USER_INFO_DEVICE_ERROR_SIZE){
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
        (void)mw_storage_record_create(error_info, sizeof(struct error_info), error_info->error_index, 0x00, region);    /* 故障为产生，新存储一条记录 */
        LOG_D("gunno(%d) save fault info|%x, %x  start|%x", gunno, error_info->error_code, error_info->error_index, error_info->occur_time);
    }else{
        if((index = mw_storage_record_get_first_index_userdata(error_info->error_index, region)) < 0){
            (void)mw_storage_record_create(error_info, sizeof(struct error_info), error_info->error_index, 0x00, region);
            return -0x02;
        }else{
            struct error_info rinfo;
            memset(&rinfo, 0x00, sizeof(struct error_info));
            if(mw_storage_record_get_designate_index_record((uint8_t*)(&rinfo), sizeof(struct error_info), region, index) < 0){  /* 故障为恢复，查找已存储的对应下标的故障 */
                (void)mw_storage_record_create(error_info, sizeof(struct error_info), error_info->error_index, 0x00, region);
                return -0x03;
            }
            rinfo.error_flag = 0x01;
            memcpy(&rinfo.resume_time, &(error_info->resume_time), sizeof(error_info->resume_time));

            (void)mw_storage_record_designate_index_updated(&rinfo, sizeof(struct error_info), 0x00, 0x00, region, index);
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
            current_fault_ptr = mw_get_system_fault_set(gunno);

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
                            switch((bit + start_bit))
                            {
                            case APP_SYS_FAULT_SCRAM:
                                s_system_error_info[gunno].error_index = 0x00;
                                s_system_error_info[gunno].error_code = 0x0000;
                                break;
                            case APP_SYS_FAULT_CARD_READER:
                                s_system_error_info[gunno].error_index = 0x01;
                                s_system_error_info[gunno].error_code = 0x0001;
                                break;
                            case APP_SYS_FAULT_DOOR:
                                s_system_error_info[gunno].error_index = 0x02;
                                s_system_error_info[gunno].error_code = 0x0002;
                                break;
                            case APP_SYS_FAULT_AMMETER:
                                s_system_error_info[gunno].error_index = 0x03;
                                s_system_error_info[gunno].error_code = 0x0003;
                                break;
                            case APP_SYS_FAULT_CHARGE_MODULE:
                                s_system_error_info[gunno].error_index = 0x04;
                                s_system_error_info[gunno].error_code = 0x0004;
                                break;
                            case APP_SYS_FAULT_OVER_TEMP:
                                s_system_error_info[gunno].error_index = 0x05;
                                s_system_error_info[gunno].error_code = 0x0005;
                                break;
                            case APP_SYS_FAULT_OVER_VOLT:
                                s_system_error_info[gunno].error_index = 0x06;
                                s_system_error_info[gunno].error_code = 0x0006;
                                break;
                            case APP_SYS_FAULT_UNDER_VOLT:
                                s_system_error_info[gunno].error_index = 0x07;
                                s_system_error_info[gunno].error_code = 0x0007;
                                break;
                            case APP_SYS_FAULT_OVER_CURR:
                                s_system_error_info[gunno].error_index = 0x08;
                                s_system_error_info[gunno].error_code = 0x0008;
                                break;
                            case APP_SYS_FAULT_RELAY:
                                s_system_error_info[gunno].error_index = 0x09;
                                s_system_error_info[gunno].error_code = 0x0009;
                                break;
                            case APP_SYS_FAULT_PARALLEL_RELAY:
                                s_system_error_info[gunno].error_index = 0x0A;
                                s_system_error_info[gunno].error_code = 0x000A;
                                break;
                            case APP_SYS_FAULT_AC_RELAY:
                                s_system_error_info[gunno].error_index = 0x0B;
                                s_system_error_info[gunno].error_code = 0x000B;
                                break;
                            case APP_SYS_FAULT_ELOCK:
                                s_system_error_info[gunno].error_index = 0x0C;
                                s_system_error_info[gunno].error_code = 0x000C;
                                break;
                            case APP_SYS_FAULT_AUXPOWER:
                                s_system_error_info[gunno].error_index = 0x0D;
                                s_system_error_info[gunno].error_code = 0x000D;
                                break;
                            case APP_SYS_FAULT_FLASH:
                                s_system_error_info[gunno].error_index = 0x0E;
                                s_system_error_info[gunno].error_code = 0x000E;
                                break;
                            case APP_SYS_FAULT_EEPROM:
                                s_system_error_info[gunno].error_index = 0x0F;
                                s_system_error_info[gunno].error_code = 0x000F;
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
            end_bit = APP_CHARGE_FAULT_NO_ERROR >= 32 ? 32 : APP_CHARGE_FAULT_NO_ERROR,
            remain_bit = APP_CHARGE_FAULT_NO_ERROR - end_bit,

            current_fault_ptr = mw_get_charge_fault_set(gunno);

            for(uint8_t set = 0; set < APP_GENERAL_CHARGE_FAULT_SET_NUM; set++){
                fault_xor = current_fault_ptr[set] ^s_charge_fault_last[gunno][set];
                if(fault_xor){
                    for(bit = 0; bit < (end_bit - start_bit); bit++){
                        if(fault_xor &(1 <<bit)){
                            switch((bit + start_bit))
                            {
                            case APP_CHARGE_FAULT_GUN_VOLT:
                                s_charge_error_info[gunno].error_index = 0x40;
                                s_charge_error_info[gunno].error_code = 0x0040;
                                break;
                            case APP_CHARGE_FAULT_INSULTA:
                                s_charge_error_info[gunno].error_index = 0x41;
                                s_charge_error_info[gunno].error_code = 0x0041;
                                break;
                            case APP_CHARGE_FAULT_COMMON:
                                s_charge_error_info[gunno].error_index = 0x42;
                                s_charge_error_info[gunno].error_code = 0x0042;
                                break;
                            case APP_CHARGE_FAULT_BATTERY_VOLT:
                                s_charge_error_info[gunno].error_index = 0x43;
                                s_charge_error_info[gunno].error_code = 0x0043;
                                break;
                            case APP_CHARGE_FAULT_READY_VOLT:
                                s_charge_error_info[gunno].error_index = 0x44;
                                s_charge_error_info[gunno].error_code = 0x0044;
                                break;
                            case APP_CHARGE_FAULT_INSULT_VOLT:
                                s_charge_error_info[gunno].error_index = 0x45;
                                s_charge_error_info[gunno].error_code = 0x0045;
                                break;
                            default:
                                break;
                            }

                            if(s_charge_fault_last[gunno][set] &(1 <<(bit + start_bit))){
                                s_charge_error_info[gunno].error_flag = 0x01;  /* 故障恢复 */
                                s_charge_error_info[gunno].resume_time = mw_get_current_timestamp();
                            }else{
                                s_charge_error_info[gunno].error_flag = 0x00;  /* 故障产生 */
                                memset(&s_charge_error_info[gunno].resume_time, 0x00, sizeof(s_charge_error_info[gunno].resume_time));
                                s_charge_error_info[gunno].occur_time = mw_get_current_timestamp();
                            }
                            app_fault_storage(&s_charge_error_info[gunno], s_charge_error_info[gunno].resume_time, gunno);
                        }
                    }
                    s_charge_fault_last[gunno][set] = current_fault_ptr[set];
                }
                start_bit = end_bit;
                end_bit = remain_bit > 32 ? 32 : remain_bit;
                end_bit += start_bit;
                remain_bit = APP_CHARGE_FAULT_NO_ERROR - end_bit;
            }
        }
        rt_thread_mdelay(500);
    }
}

/*****************************(C)COPYRIGHT(c) 2021 Thaisen *****END OF FILE****/
