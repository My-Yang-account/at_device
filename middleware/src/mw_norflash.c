/**
 ******************************************************************************
 * @file mw_norflash.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_norflash.h"
#include "app_flash.h"
#include "thaisen7102Public.h"

int32_t mw_norflash_init(void)
{
    return 0;
}

int32_t mw_norflash_read(uint32_t address, uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    TAKE_FLASH_ACCESS_MUTEX

    thaisenW25qxxRead((unsigned char *)buffer, (unsigned int)address, (unsigned short)size);

    RELEASE_FLASH_ACCESS_MUTEX

    return 0;
}

int32_t mw_norflash_write(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    TAKE_FLASH_ACCESS_MUTEX

    thaisenW25qxxWrite((unsigned char *)buffer, (unsigned int)address, (unsigned short)size);

    RELEASE_FLASH_ACCESS_MUTEX

    return 0;
}

int32_t mw_norflash_write_directly(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    TAKE_FLASH_ACCESS_MUTEX

    thaisenW25qxxWriteNoCheck((unsigned char *)buffer, (unsigned int)address, (unsigned short)size);

    RELEASE_FLASH_ACCESS_MUTEX

    return 0;
}

int32_t mw_norflash_erase(uint32_t address, int32_t size)
{
    TAKE_FLASH_ACCESS_MUTEX

    thaisenW25qxxErase((unsigned int)address, (unsigned int)size);

    RELEASE_FLASH_ACCESS_MUTEX

    return 0;
}
