/**
  ******************************************************************************
  * @file
  * @author
  * @brief
  ******************************************************************************
  * @attention
  ******************************************************************************
  */

#ifndef _NOTFS_CFG_H_
#define _NOTFS_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "chargepile_config.h"

#define NOTFS_GUNNOA_CHARGE_ORDER_REGION_SIZE            1024* 1024       /* A枪充电订单区大小 */
#define NOTFS_GUNNOB_CHARGE_ORDER_REGION_SIZE            1024* 1024       /* B枪充电订单区大小 */
#define NOTFS_GUNNOA_FAULT_RECORD_REGION_SIZE            1024 *1024       /* A枪故障记录区大小 */
#define NOTFS_GUNNOB_FAULT_RECORD_REGION_SIZE            1024 *1024       /* B枪故障记录区大小 */

enum notfs_subregion {
    NOTFS_SUBREGION_GUNNOA_CHARGE_ORDER,     /* 订单 */
    NOTFS_SUBREGION_GUNNOB_CHARGE_ORDER,     /* 订单 */
    NOTFS_SUBREGION_GUNNOA_FAULT_RECORD,     /* 故障记录 */
    NOTFS_SUBREGION_GUNNOB_FAULT_RECORD,     /* 故障记录 */
    NOTFS_SUBREGION_MAX,
};

enum notfs_subregion_index{
    NOTFS_SUBREGION_INDEX_CURRENT = -1,      /* 当前下标 */
    NOTFS_SUBREGION_INDEX_FIRST = -2,        /* 第一个下标 */
};

/* 用户自定义的最大订单数量 */
#define NOTFS_ORDER_USER_FILE_MAX_COUNT      100
/* 用户自定义的最大快照数量 */
#define NOTFS_SNAPSHOT_USER_FILE_MAX_COUNT   5

/* 自定义数据分区的起始地址和大小 */
#define NOTFS_NORFLASH_SUBREGION_TABLE \
{                                                                                                             \
    [NOTFS_SUBREGION_GUNNOA_CHARGE_ORDER] = {.subregion_start_addr = GUNNOA_CHARGE_RECORD_ADDRESS,            \
                                             .file_max_count = NOTFS_ORDER_USER_FILE_MAX_COUNT,        \
                                             .subregion_total_size = NOTFS_GUNNOA_CHARGE_ORDER_REGION_SIZE},  \
                                                                                                        \
    [NOTFS_SUBREGION_GUNNOB_CHARGE_ORDER] = {.subregion_start_addr = GUNNOB_CHARGE_RECORD_ADDRESS,            \
                                             .file_max_count = NOTFS_ORDER_USER_FILE_MAX_COUNT,        \
                                             .subregion_total_size = NOTFS_GUNNOB_CHARGE_ORDER_REGION_SIZE},  \
                                                                                                         \
    [NOTFS_SUBREGION_GUNNOA_FAULT_RECORD] = {.subregion_start_addr = GUNNOA_FAULT_RECORD_ADDRESS,                \
                                             .file_max_count = NOTFS_ORDER_USER_FILE_MAX_COUNT,        \
                                             .subregion_total_size = NOTFS_GUNNOA_FAULT_RECORD_REGION_SIZE},     \
                                                                                                          \
    [NOTFS_SUBREGION_GUNNOB_FAULT_RECORD] = {.subregion_start_addr = GUNNOB_FAULT_RECORD_ADDRESS,                \
                                             .file_max_count = NOTFS_ORDER_USER_FILE_MAX_COUNT,        \
                                             .subregion_total_size = NOTFS_GUNNOB_FAULT_RECORD_REGION_SIZE},     \
}

#ifdef __cplusplus
}
#endif

#endif /* _NOTFS_CFG_H_ */

/*****************************(C)COPYRIGHT(c) 2022 Thaisen *****END OF FILE****/
