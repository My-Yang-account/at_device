/**
 ******************************************************************************
 * @file mw_norflash.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_norflash.h"
#include "thaisen7102Public.h"

#define MW_IFLASH_CONFIG_ADDR_MIN              0x08010000               /* 内部flash配置部分最小地址 */
#define MW_IFLASH_CONFIG_ADDR_MAX              0x08200000               /* 内部flash配置部分最大地址 */
#define MW_IFLASH_SINGLE_SECTOR_SIZE           4096      /* 内部flash单个扇区大小 */

#define MW_SYSTEM_CHIP_RW_SINGLE_SIZE          4096      /* 单次读写大小 */

static uint8_t s_flash_lock = 1;

int32_t mw_norflash_init(void)
{
    return 0;
}

int32_t mw_norflash_read(uint32_t address, uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    uint32_t remain_len = size, used_len = 0x00;

    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    while((remain_len > 0x00) && (remain_len /MW_SYSTEM_CHIP_RW_SINGLE_SIZE)){
        thaisenW25qxxRead((uint8_t *)(buffer + used_len), (address + used_len), MW_SYSTEM_CHIP_RW_SINGLE_SIZE);
        remain_len -= MW_SYSTEM_CHIP_RW_SINGLE_SIZE;
        used_len += MW_SYSTEM_CHIP_RW_SINGLE_SIZE;
    }
    if(remain_len > 0x00){
        thaisenW25qxxRead((uint8_t *)(buffer + used_len), (address + used_len), remain_len);
    }

    s_flash_lock = 1;

    return 0;
}

int32_t mw_norflash_write(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    uint32_t remain_len = size, used_len = 0x00;

    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    while((remain_len > 0x00) && (remain_len /MW_SYSTEM_CHIP_RW_SINGLE_SIZE)){
        thaisenW25qxxWrite((uint8_t *)(buffer + used_len), (address + used_len), MW_SYSTEM_CHIP_RW_SINGLE_SIZE);
        remain_len -= MW_SYSTEM_CHIP_RW_SINGLE_SIZE;
        used_len += MW_SYSTEM_CHIP_RW_SINGLE_SIZE;
    }
    if(remain_len > 0x00){
        thaisenW25qxxWrite((uint8_t *)(buffer + used_len), (address + used_len), remain_len);
    }

    s_flash_lock = 1;

    return 0;
}

int32_t mw_norflash_write_directly(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    uint32_t remain_len = size, used_len = 0x00;

    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    while((remain_len > 0x00) && (remain_len /MW_SYSTEM_CHIP_RW_SINGLE_SIZE)){
        thaisenW25qxxWriteNoCheck((uint8_t *)(buffer + used_len), (address + used_len), MW_SYSTEM_CHIP_RW_SINGLE_SIZE);
        remain_len -= MW_SYSTEM_CHIP_RW_SINGLE_SIZE;
        used_len += MW_SYSTEM_CHIP_RW_SINGLE_SIZE;
    }
    if(remain_len > 0x00){
        thaisenW25qxxWriteNoCheck((uint8_t *)(buffer + used_len), (address + used_len), remain_len);
    }

    s_flash_lock = 1;

    return 0;
}

int32_t mw_norflash_erase(uint32_t address, int32_t size)
{
    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    thaisenW25qxxErase((unsigned int)address, (unsigned int)size);

    s_flash_lock = 1;

    return 0;
}

/**
 * 以下是对内部 flash 的操作
 */

int32_t mw_iflash_init(void)
{
    return 0;
}

int32_t mw_iflash_write_directly(uint32_t address, const uint8_t *buffer, int32_t size, uint8_t nesting)
{
    if((address < MW_IFLASH_CONFIG_ADDR_MIN) || ((address + size)) > MW_IFLASH_CONFIG_ADDR_MAX){
        return -0x01;
    }
    if((buffer == NULL) || (size == 0x00)){
        return -0x01;
    }

    return thaisenIflashWriteDirectly(address, buffer, size, nesting);
}

int32_t mw_iflash_erase_sector(uint32_t address, uint32_t size, uint8_t nesting)
{
    if((address < MW_IFLASH_CONFIG_ADDR_MIN) || ((address + size) > MW_IFLASH_CONFIG_ADDR_MAX)){
        return -0x01;
    }

    return thaisenIflashEraseSector(address, size, nesting);
}

int32_t mw_iflash_read(uint32_t address, uint8_t *buffer, int32_t size)
{
    if((address < MW_IFLASH_CONFIG_ADDR_MIN) || ((address + size)) > MW_IFLASH_CONFIG_ADDR_MAX){
        return -0x01;
    }
    if((buffer == NULL) || (size == 0x00)){
        return -0x01;
    }

    return thaisenIflashRead(address, buffer, size);
}

int32_t mw_iflash_write(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if((address < MW_IFLASH_CONFIG_ADDR_MIN) || ((address + size)) > MW_IFLASH_CONFIG_ADDR_MAX){
        return -0x01;
    }
    if((buffer == NULL) || (size == 0x00)){
        return -0x01;
    }

    return thaienIflashWrite(address, buffer, size);
}

