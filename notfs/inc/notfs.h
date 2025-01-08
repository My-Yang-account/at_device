/**
 ******************************************************************************
 * @file notfs.h
 * @author leven
 * @brief
 ******************************************************************************
 */

#ifndef _NOTFS_H_
#define _NOTFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "notfs_def.h"

#define NOTFS_VERSION "1.1"

//#define NOTFS_DESIGNATE_REGION                                     /* 变量定义到指定区 */

#ifdef NOTFS_DESIGNATE_REGION
#define NOTFS_DEF_TCMRAM __attribute__((section(".TCM_RAM")))      /* 将变量定义在TCMRAM区，注：对于GD32F470ZGT6 TCMRAM 不能存放代码，不能被任何 DMA 访问，可以将一些变量定义在该地址空间；定义的变量初始值是未知的 */
#define NOTFS_DEF_SRAM0  __attribute__((section(".SRAM0_RAM")))    /* 将变量定义在SRAM0区，注：对于GD32F470ZGT6 SRAM0 可以存放代码，也可以存放变量，也可以作为线程的栈地址空间，可以被 DMA 访问；定义的变量初始值是未知的 */
#define NOTFS_DEF_SRAM1  __attribute__((section(".SRAM1_RAM")))    /* 将变量定义在SRAM1区，注：对于GD32F470ZGT6 SRAM1 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#define NOTFS_DEF_SRAM2  __attribute__((section(".SRAM2_RAM")))    /* 将变量定义在SRAM2区，注：对于GD32F470ZGT6 SRAM2 不可以存放代码，也不可以将线程的栈地址空间定义在这里，可以被 DMA 访问；定义的变量初始值是未知的 */
#else
#define NOTFS_DEF_TCMRAM
#define NOTFS_DEF_SRAM0
#define NOTFS_DEF_SRAM1
#define NOTFS_DEF_SRAM2
#endif /* NOTFS_DESIGNATE_REGION */

/* 初始化 */
notfs_err_e notfs_init(void);
/* 追加 */
notfs_err_e notfs_subregion_append_record(enum notfs_subregion subregion, const void *buf, size_t size, uint32_t user_data, uint16_t verify_mask, uint8_t is_verify);
/* 根据指定下标更新记录数据 */
notfs_err_e notfs_subregion_updated_designate_index_data(enum notfs_subregion subregion, const void *buf, size_t size, uint32_t user_data, uint16_t verify_mask, uint8_t is_verify, int32_t index);
/* 获取指定下标故障或充电记录数据 */
notfs_err_e notfs_get_subregion_designate_index_record_data(enum notfs_subregion subregion, uint8_t* buf, size_t size, int32_t index);
/* 获取指定区域记录总数 */
int32_t notfs_get_subregion_record_total_num(enum notfs_subregion subregion);
/* 获取指定区域与 user_data 匹配的记录总数 */
int32_t notfs_get_subregion_userdata_record_num(enum notfs_subregion subregion, uint32_t user_data);
/* 获取指定区域当前记录下标 */
int32_t notfs_get_subregion_current_index(enum notfs_subregion subregion);
/* 擦除指定区域 */
int32_t notfs_clear_subregion_record_info(enum notfs_subregion subregion);
/* 获取第一条与  user_data 匹配的记录下标 */
int32_t notfs_get_subregion_first_index_userdata(enum notfs_subregion subregion, uint32_t user_data);
/* 获取第一条与 verify_mask 匹配的记录下标 */
int32_t notfs_get_subregion_first_index_record_verify(enum notfs_subregion subregion, uint16_t verify_mask);
/* 获取指定区域与 verify_mask 匹配的记录总数 */
int32_t notfs_get_subregion_record_verify_record_num(enum notfs_subregion subregion, uint16_t verify_mask);
/* 获取指定区域在指定时间段内的第一条记录的下标 */
int32_t notfs_query_subregion_findex_with_time_period(enum notfs_subregion subregion, uint16_t sindex, uint32_t stime, uint32_t etime);

int32_t notfs_get_subregion_unverify_record_num(enum notfs_subregion subregion);

int32_t notfs_get_subregion_first_index_unverify(enum notfs_subregion subregion, int32_t sindex);

/* notfs_utils.c */
uint32_t notfs_timestamp(void);
uint32_t notfs_crc32(uint32_t crc, const uint8_t *data, size_t len);

/* notfs_port.c */
notfs_err_e notfs_port_erase(uint32_t addr, size_t size);
notfs_err_e notfs_port_read (uint32_t addr, uint8_t *buf, size_t size);
notfs_err_e notfs_port_write(uint32_t addr, const uint8_t *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _NOTFS_H_ */

/*****************************(C)COPYRIGHT(c) 2022 Thaisen *****END OF FILE****/
