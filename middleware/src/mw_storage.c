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

/**************************************************************************
 * 函数名           mw_storage_init
 * 功能              存储信息初始化
 * 参数
 * 返回              >=0：成功     <0：失败
 *************************************************************************/
int32_t mw_storage_init(void)
{
    return notfs_init();
}

/**************************************************************************
 * 函数名           mw_storage_record_create
 * 功能              在指定存储区内创建一个记录
 * 参数              data         记录数据
 *         data_len     数据长度
 *         user_data    用户数据
 *         verify_mask  平台确认码
 *         is_verify    订单确认码
 *         region       记录存储区
 * 返回              >=0：成功     <0：失败
 *************************************************************************/
int32_t mw_storage_record_create(const void *data, uint32_t data_len, uint32_t user_data, uint16_t verify_mask, uint8_t is_verify, uint8_t region)
{
    if ((NULL == data) || (0 == data_len)) {
        return -1;
    }

    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }

    uint8_t rbuf[data_len], save_rentry = 0, check_rentry = 0;
    if(notfs_subregion_append_record((enum notfs_subregion)region, data, data_len, user_data, verify_mask, is_verify) < 0){
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
        notfs_subregion_updated_designate_index_data((enum notfs_subregion)region, data, data_len, user_data, verify_mask, is_verify, NOTFS_SUBREGION_INDEX_CURRENT);
        OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
        save_rentry++;
    }
    if(save_rentry >= STORAGE_RECORD_RENTRY_MAX){
        return -1;
    }
    return 0;
}

/**************************************************************************
 * 函数名           mw_storage_record_designate_index_updated
 * 功能              更新指定存储区内指定下标的记录
 * 参数              data         记录数据
 *         data_len     数据长度
 *         user_data    用户数据
 *         verify_mask  平台确认码
 *         is_verify    订单确认码
 *         region       记录存储区
 *         index        下标
 * 返回              >=0：成功     <0：失败
 *************************************************************************/
int32_t mw_storage_record_designate_index_updated(const void *data, uint32_t data_len, uint32_t user_data, uint16_t verify_mask, uint8_t is_verify, uint8_t region, int32_t index)
{
    if ((NULL == data) || (0 == data_len)) {
        return -1;
    }

    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }

    uint8_t rbuf[data_len], save_rentry = 0, check_rentry = 0;
    notfs_subregion_updated_designate_index_data((enum notfs_subregion)region, data, data_len, user_data, verify_mask, is_verify, index);

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
        notfs_subregion_updated_designate_index_data((enum notfs_subregion)region, data, data_len, user_data, verify_mask, is_verify, index);
        OS_MDELAY_PORT(STORAGE_OPERATE_WAIT_TIME);
        save_rentry++;
    }
    if(save_rentry >= STORAGE_RECORD_RENTRY_MAX){
        return -4;
    }
    return 0;
}

/**************************************************************************
 * 函数名           mw_storage_record_get_designate_index_record
 * 功能              获取指定存储区内指定下标的记录数据
 * 参数             buf         存放数据的缓存
 *         data_len    缓存长度
 *         region      记录存储区
 *         index       下标
 * 返回              >=0：成功     <0：失败
 *************************************************************************/
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

/**************************************************************************
 * 函数名           mw_storage_record_get_record_total_num
 * 功能              查询指定存储区内记录的总数
 * 参数               region       记录存储区
 * 返回              >=0：记录的总数     <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_record_total_num(uint8_t region)
{
    return notfs_get_subregion_record_total_num((enum notfs_subregion)region);
}

/**************************************************************************
 * 函数名           mw_storage_record_get_userdata_record_num
 * 功能              查询指定存储区匹配指定用户数据的记录总数
 * 参数              user_data    用户数据
 *         region       记录存储区
 * 返回              >=0：记录总数     <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_userdata_record_num(uint32_t user_data, uint8_t region)
{
    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }
    return notfs_get_subregion_userdata_record_num((enum notfs_subregion)region, user_data);
}
/**************************************************************************
 * 函数名           mw_storage_record_get_current_index
 * 功能              获取指定存储区的当前下标
 * 参数              region    记录存储区
 * 返回              >=0：当前下标    <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_current_index(uint8_t region)
{
    return notfs_get_subregion_current_index((enum notfs_subregion)region);
}

/**************************************************************************
 * 函数名           mw_storage_record_clear_record_info
 * 功能              清除指定记录存储区
 * 参数              region    记录存储区
 * 返回              >=0：成功     <0：失败
 *************************************************************************/
int32_t mw_storage_record_clear_record_info(uint8_t region)
{
    return notfs_clear_subregion_record_info((enum notfs_subregion)region);
}

/**************************************************************************
 * 函数名           mw_storage_record_get_first_index_userdata
 * 功能              查询指定存储区匹配指定用户数据的第一条记录的下标
 * 参数              user_data    用户数据
 *         region       记录存储区
 * 返回              >=0：下标     <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_first_index_userdata(uint32_t user_data, uint8_t region)
{
    if (0xFF == user_data) {
        return -1; /* 不允许使用 0xFF 作为用户数据 */
    }
    return notfs_get_subregion_first_index_userdata((enum notfs_subregion)region, user_data);
}

/**************************************************************************
 * 函数名           mw_storage_record_get_recordverify_record_num
 * 功能              根据确认码查询指定存储区匹配确认码的记录总数
 * 参数               verify_mask      确认码
 *          region     记录存储区
 * 返回              >=0：记录总数    <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_recordverify_record_num(uint16_t verify_mask, uint8_t region)
{
    return notfs_get_subregion_record_verify_record_num((enum notfs_subregion)region, verify_mask);
}

/**************************************************************************
 * 函数名           mw_storage_record_get_first_index_recordverify
 * 功能              根据确认码查询指定存储区匹配确认码的第一条记录的下标
 * 参数               verify_mask      确认码
 *          region     记录存储区
 * 返回              >=0：下标    <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_first_index_recordverify(uint16_t verify_mask, uint8_t region)
{
    return notfs_get_subregion_first_index_record_verify((enum notfs_subregion)region, verify_mask);
}

/**************************************************************************
 * 函数名           mw_storage_record_get_unverify_record_num
 * 功能              查询指定存储区未确认的记录总数
 * 参数               region     记录存储区
 * 返回              >=0：未确认的记录总数     <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_unverify_record_num(uint8_t region)
{
    return notfs_get_subregion_unverify_record_num((enum notfs_subregion)region);
}

/**************************************************************************
 * 函数名           mw_storage_record_get_first_index_unverify
 * 功能              查询指定存储区未确认的第一条记录下标
 * 参数              sindex    开始下标(将从这个下标开始查询)
 *         region     记录存储区
 * 返回              >=0：下标     <0：失败
 *************************************************************************/
int32_t mw_storage_record_get_first_index_unverify(uint8_t region, int32_t sindex)
{
    return notfs_get_subregion_first_index_unverify((enum notfs_subregion)region, sindex);
}

/**************************************************************************
 * 函数名           mw_storage_record_query_findex_with_time_period
 * 功能              查询在指定时间段内的第一条订单的下标
 * 参数              sindex    开始下标(将从这个下标开始查询)
 *         stime     开始时间
 *         etime     结束时间
 *         region    记录存储区
 * 返回              >=0：下标     <0：失败
 *************************************************************************/
int32_t mw_storage_record_query_findex_with_time_period(uint16_t sindex, uint32_t stime, uint32_t etime, uint8_t region)
{
    return notfs_query_subregion_findex_with_time_period((enum notfs_subregion)region, sindex, stime, etime);
}
