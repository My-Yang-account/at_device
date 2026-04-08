/**
 ******************************************************************************
 * @file mw_storage.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_STORAGE_H_
#define MW_STORAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "notfs_cfg.h"

#define MW_STORAGE_VERSION "1.0"
#define OS_MDELAY_PORT(ms)  rt_thread_mdelay(ms)

enum user_data_type{
    USER_DATA_TYPE_STORAGE,
    USER_DATA_TYPE_VERIFIED,
    USER_DATA_TYPE_REPORTED,
};

enum storage_index{
    STORAGE_INDEX_CURRENT = -1,      /* 当前下标 */
    STORAGE_INDEX_FIRST = -2,        /* 第一个下标 */
};

enum record_region{
    RECORD_REGION_CHARGE_RECORDA,                  /* 创建A枪充电记录 */
    RECORD_REGION_CHARGE_RECORDB,                  /* 创建B枪充电记录 */
    RECORD_REGION_FAULT_RECORDA,                   /* 创建A枪故障记录 */
    RECORD_REGION_FAULT_RECORDB,                   /* 创建B枪故障记录 */
};

enum storage_err{
    STORAGE_ERR_NONE = 0x00,                       /* 无错误 */
    STORAGE_ERR_INVALID_DATA = -0x01,              /* 无效数据 */
    STORAGE_ERR_OUT_OF_RANGE = -0x02,              /* 超过范围 */
    STORAGE_ERR_OPERATE_FAIL = -0x03,              /* 操作失败 */
    STORAGE_ERR_CHECK_ERROR = -0x04,               /* 校验失败 */
};

int32_t mw_storage_init(void);

/**
 * @brief 创建订单
 * @param data
 * @param data_len
 * @param user_data 用户数据（通常为数据的状态。如果状态为0xFF，则无效）
 * @param verify_mask 确认码(用于平台订单确认)
 * @param is_verify 订单已确认(用于单条订单确认)
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_create(const void *data, uint32_t data_len, uint32_t user_data, uint16_t verify_mask, uint8_t is_verify, uint8_t region);

/**
 * @brief 更新订单
 * @param data
 * @param data_len
 * @param user_data 用户数据（通常为数据的状态。如果状态为0xFF，则无效）
 * @param verify_mask 确认码(用于平台订单确认)
 * @param is_verify 订单已确认(用于单条订单确认)
 * @param index
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_designate_index_updated(const void *data, uint32_t data_len, uint32_t user_data, uint16_t verify_mask, uint8_t is_verify, uint8_t region, int32_t index);

/**
 * @brief 获取指定下标记录信息
 * @param buf
 * @param data_len
 * @param verify_mask
* @param region
 * @param index
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_designate_index_record(uint8_t *buf, uint32_t data_len, uint8_t region, int32_t index);

/**
 * @brief 获取指定下标记录信息(不校验数据是否正确)
 * @param buf
 * @param data_len
 * @param verify_mask
* @param region
 * @param index
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_designate_index_record_only(uint8_t *buf, uint32_t data_len, uint8_t region, int32_t index);

/**
 * @brief 获取指定区域总记录数
 * @param region 区域
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_record_total_num(uint8_t region);

/**
 * @brief 获取指定区域与user_data匹配的总记录数
 * @param region 区域
 * @param user_data
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_userdata_record_num(uint32_t user_data, uint8_t region);

/**
 * @brief 获取指定区域当前记录下标
 * @param region 区域
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_current_index(uint8_t region);

/**
 * @brief 清除指定区域记录信息
 * @param region 区域
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_clear_record_info(uint8_t region);

/**
 * @brief 获取指定区域与user_data匹配的记录信息的第一个下标
 * @param region 区域
 * @param user_data
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_first_index_userdata(uint32_t user_data, uint8_t region);

/**
 * @brief 获取指定区域与verify_mask匹配的总记录数
 * @param region 区域
 * @param verify_mask
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_recordverify_record_num(uint16_t verify_mask, uint8_t region);

/**
 * @brief 获取指定区域与verify_mask匹配的记录信息的第一个下标
 * @param region 区域
 * @param verify_mask
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_first_index_recordverify(uint16_t verify_mask, uint8_t region);

/**
 * @brief 查询指定存储区未确认的记录总数
 * @param region 区域
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_unverify_record_num(uint8_t region);

/**
 * @brief 查询指定存储区未确认的第一条记录下标
 * @param region 区域
 * @param sindex
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_get_first_index_unverify(uint8_t region, int32_t sindex);

/**
 * @brief 获取指定区域与verify_mask匹配的记录信息的第一个下标
 * @param region 区域
 * @param sindex 起始下标
 * @param stime 开始时间戳
 * @param etime 结束时间戳
 * @return < 0: 失败，== 0：成功
 */
int32_t mw_storage_record_query_findex_with_time_period(uint16_t sindex, uint32_t stime, uint32_t etime, uint8_t region);

#ifdef __cplusplus
}
#endif

#endif /* MW_STORAGE_H_ */
