/**
 ******************************************************************************
 * @file mw_storage.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "notfs.h"
#include "mw_storage.h"

#define STORAGE_RECORD_RENTRY_MAX       0x03          /* 记录存储最大尝试次数 */
#define STORAGE_READ_RENTRY_MAX         0x05          /* 读取记录最大尝试次数 */
#define STORAGE_OPERATE_WAIT_TIME       50            /* 存储操作等待时长 */

int32_t mw_storage_init(void)
{
    return notfs_init();
}

int32_t mw_storage_record_create(const void *data, uint32_t data_len, uint32_t user_data, uint16_t verify_mask, uint8_t region)
{
    if ((NULL == data) || (0 == data_len)) {
        return -1;
    }

    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }

    uint8_t rbuf[data_len], save_rentry = 0, check_rentry = 0;
    if(notfs_subregion_append_record((enum notfs_subregion)region, data, data_len, user_data, verify_mask) < 0){
        return -1;
    }

    while(save_rentry < STORAGE_RECORD_RENTRY_MAX){
        check_rentry = 0;
        while(check_rentry < STORAGE_READ_RENTRY_MAX){
            notfs_get_subregion_designate_index_record_data((enum notfs_subregion)region, rbuf, data_len, NOTFS_SUBREGION_INDEX_CURRENT);
            if(memcmp(data, rbuf, data_len) == 0){
                return 0;
            }
            OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
            check_rentry++;
        }
        notfs_subregion_updated_designate_index_data((enum notfs_subregion)region, data, data_len, user_data, verify_mask, NOTFS_SUBREGION_INDEX_CURRENT);
        OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
        save_rentry++;
    }
    if(save_rentry >= STORAGE_RECORD_RENTRY_MAX){
        return -1;
    }
    return 0;
}

int32_t mw_storage_record_designate_index_updated(const void *data, uint32_t data_len, uint32_t user_data, uint16_t verify_mask, uint8_t region, int32_t index)
{
    if ((NULL == data) || (0 == data_len)) {
        return -1;
    }

    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }

    uint8_t rbuf[data_len], save_rentry = 0, check_rentry = 0;
    notfs_subregion_updated_designate_index_data((enum notfs_subregion)region, data, data_len, user_data, verify_mask, index);

    while(save_rentry < STORAGE_RECORD_RENTRY_MAX){
        check_rentry = 0;
        while(check_rentry < STORAGE_READ_RENTRY_MAX){
            notfs_get_subregion_designate_index_record_data((enum notfs_subregion)region, rbuf, data_len, index);
            if(memcmp(data, rbuf, data_len) == 0){
                return 0;
            }
            OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
            check_rentry++;
        }
        notfs_subregion_updated_designate_index_data((enum notfs_subregion)region, data, data_len, user_data, verify_mask, index);
        OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
        save_rentry++;
    }
    if(save_rentry >= STORAGE_RECORD_RENTRY_MAX){
        return -4;
    }
    return 0;
}

int32_t mw_storage_record_get_designate_index_record(uint8_t *buf, uint32_t data_len, uint8_t region, int32_t index)
{
    if ((NULL == buf) || (0 == data_len)) {
        return STORAGE_ERR_INVALID_DATA;
    }

    uint8_t check_rentry = 0;
    int32_t result = STORAGE_ERR_NONE;

    while(check_rentry < STORAGE_READ_RENTRY_MAX){
        rt_kprintf("vvvvvvvvvvvvvvvv(%d, %d)\n", check_rentry, index);
        result = notfs_get_subregion_designate_index_record_data((enum notfs_subregion)region, buf, data_len, index);
        if(result != STORAGE_ERR_NONE){
            rt_kprintf("fffffffsssssssss(%d, %d)\n", check_rentry, result);
            OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
            check_rentry++;
        }else{
            break;
        }
    }

    if(check_rentry >= STORAGE_READ_RENTRY_MAX){
        if(result == NOTFS_CHECK_ERR){
            return STORAGE_ERR_CHECK_ERROR;
        }else if(result == NOTFS_OUTRANGE_ERR){
            return STORAGE_ERR_OUT_OF_RANGE;
        }else if(result == NOTFS_ILLEGAL_ERR){
            return STORAGE_ERR_INVALID_DATA;
        }else{
            return STORAGE_ERR_OPERATE_FAIL;
        }
    }
    return STORAGE_ERR_NONE;
}

int32_t mw_storage_record_get_record_total_num(uint8_t region)
{
    return notfs_get_subregion_record_total_num((enum notfs_subregion)region);
}

int32_t mw_storage_record_get_userdata_record_num(uint32_t user_data, uint8_t region)
{
    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }
    return notfs_get_subregion_userdata_record_num((enum notfs_subregion)region, user_data);
}

int32_t mw_storage_record_get_current_index(uint8_t region)
{
    return notfs_get_subregion_current_index((enum notfs_subregion)region);
}

int32_t mw_storage_record_clear_record_info(uint8_t region)
{
    return notfs_clear_subregion_record_info((enum notfs_subregion)region);
}

int32_t mw_storage_record_get_first_index_userdata(uint32_t user_data, uint8_t region)
{
    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }
    return notfs_get_subregion_first_index_userdata((enum notfs_subregion)region, user_data);
}

int32_t mw_storage_record_get_recordverify_record_num(uint16_t verify_mask, uint8_t region)
{
    return notfs_get_subregion_record_verify_record_num((enum notfs_subregion)region, verify_mask);
}

int32_t mw_storage_record_get_first_index_recordverify(uint16_t verify_mask, uint8_t region)
{
    return notfs_get_subregion_first_index_record_verify((enum notfs_subregion)region, verify_mask);
}

int32_t mw_storage_record_query_findex_with_time_period(uint16_t sindex, uint32_t stime, uint32_t etime, uint8_t region)
{
    return notfs_query_subregion_findex_with_time_period((enum notfs_subregion)region, sindex, stime, etime);
}
