/**
 ******************************************************************************
 * @file notfs_def.h
 * @author leven
 * @brief
 ******************************************************************************
 */

#ifndef _NOTFS_DEF_H_
#define _NOTFS_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "notfs_cfg.h"

typedef enum {
    NOTFS_NO_ERR       =  0, /* OK */
    NOTFS_ERASE_ERR    = -1, /* 擦除失败 */
    NOTFS_READ_ERR     = -2, /* 读失败 */
    NOTFS_WRITE_ERR    = -3, /* 写失败 */
    NOTFS_CHECK_ERR    = -4, /* 校验错误 */
    NOTFS_MALLOC_ERR   = -5, /* 内存申请失败 */
    NOTFS_NOTINIT_ERR  = -6, /* 分区未初始化 */
    NOTFS_ILLEGAL_ERR  = -7, /* 非法参数 */
    NOTFS_OUTRANGE_ERR = -8, /* 超出范围 */
} notfs_err_e;

#pragma pack(1)

typedef struct {
    uint32_t key;

    uint32_t subregion_start_addr; /* 分区起始地址 */
    uint32_t subregion_total_size; /* 分区总大小 */

    uint32_t file_max_size;     /* 分区中存储单个文件的最大大小 */
    uint32_t file_max_count;    /* 分区中存储文件的最大数量 */

    uint32_t inode_start_addr;  /* 索引节点区的起始地址 */
    uint32_t inode_addr_offset; /* 索引节点区中备份数据的偏移量（相对于主地址） */

    uint32_t file_start_addr;   /* 数据区的起始地址 */
    uint32_t file_addr_step;    /* 数据分区中主数据的地址步进 */
    uint32_t file_addr_offset;  /* 数据分区中备份数据的偏移量（相对于主地址） */

    uint32_t crc;
} notfs_super_block_t;

typedef struct {
    uint32_t timestamp;       /* 时间戳 */
    uint32_t index : 20;      /* 索引从 0 开始，最大到 0xFFFFF-1 */
    uint32_t reservation : 4; /* 预留 */
    uint32_t user_data : 8;   /* 支持 1 字节的用户数据定义 */
    uint16_t record_verify;   /* 记录确认 */
} notfs_inode_t;

struct nofs_header {
    uint32_t crc;         /* 校验值 */
    uint32_t reservation; /* 保留 */
};

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* _NOTFS_DEF_H_ */

/*****************************(C)COPYRIGHT(c) 2022 Thaisen *****END OF FILE****/
