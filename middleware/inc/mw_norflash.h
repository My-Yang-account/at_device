/**
 ******************************************************************************
 * @file mw_norflash.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_NORFLASH_H_
#define MW_NORFLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NULL ((void *)0)

/**
 * @return -1：失败；0：成功
 */
int32_t mw_norflash_init(void);

/**
 * @param address 起始地址
 * @param buffer 读取到的数据
 * @param size 从起始地址开始读取数据的总大小
 * @return -1：失败；0：成功
 */
int32_t mw_norflash_read(uint32_t address, uint8_t *buffer, int32_t size);

/**
 * @param address 起始地址
 * @param buffer 待写入的数据
 * @param size 从起始地址开始写入数据的总大小
 * @return -1：失败；0：成功
 */
int32_t mw_norflash_write(uint32_t address, const uint8_t *buffer, int32_t size);

/**
 * @param address 起始地址
 * @param size 从起始地址开始擦除数据的总大小
 * @return -1：失败；0：成功
 * @note 擦除操作将会按照 Flash 芯片的擦除粒度对齐，请注意保证起始地址和擦除数据大小按照 Flash 芯片的擦除粒度对齐，否则执行擦除操作后，将会导致其他数据丢失。
 */
int32_t mw_norflash_write_directly(uint32_t address, const uint8_t *buffer, int32_t size);

int32_t mw_norflash_erase(uint32_t address, int32_t size);

int32_t mw_iflash_init(void);
int32_t mw_iflash_read(uint32_t address, uint8_t *buffer, int32_t size);
int32_t mw_iflash_write(uint32_t address, const uint8_t *buffer, int32_t size);
int32_t mw_iflash_write_directly(uint32_t address, const uint8_t *buffer, int32_t size, uint8_t nesting);
int32_t mw_iflash_erase_sector(uint32_t address, uint32_t size, uint8_t nesting);

#ifdef __cplusplus
}
#endif

#endif /* MW_NORFLASH_H_ */
