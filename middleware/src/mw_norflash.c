/**
 ******************************************************************************
 * @file mw_norflash.c
 * @author leven
 * @brief 
 ******************************************************************************
 */

#include "mw_norflash.h"
#include "thaisen7102Public.h"

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

    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    thaisenW25qxxRead((unsigned char *)buffer, (unsigned int)address, (unsigned short)size);

    s_flash_lock = 1;

    return 0;
}

int32_t mw_norflash_write(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    thaisenW25qxxWrite((unsigned char *)buffer, (unsigned int)address, (unsigned short)size);

    s_flash_lock = 1;

    return 0;
}

int32_t mw_norflash_write_directly(uint32_t address, const uint8_t *buffer, int32_t size)
{
    if (NULL == buffer) {
        return -1;
    }

    while(s_flash_lock == 0){
        rt_thread_mdelay(200);
    }
    s_flash_lock = 0;

    thaisenW25qxxWriteNoCheck((unsigned char *)buffer, (unsigned int)address, (unsigned short)size);

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
