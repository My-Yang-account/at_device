/**
 ******************************************************************************
 * @file notfs_port.c
 * @author leven
 * @brief
 ******************************************************************************
 */

#include "notfs.h"

#include "mw_norflash.h"

notfs_err_e notfs_port_erase(uint32_t addr, size_t size)
{
    notfs_err_e err = NOTFS_NO_ERR;

    if (0 > mw_norflash_erase(addr, size)) {
        err = NOTFS_ERASE_ERR;
    }

    return err;
}

notfs_err_e notfs_port_read(uint32_t addr, uint8_t *buf, size_t size)
{
    if (NULL == buf) {
        return NOTFS_ILLEGAL_ERR;
    }

    notfs_err_e err = NOTFS_NO_ERR;

    if (0 > mw_norflash_read(addr, buf, size)) {
        err = NOTFS_READ_ERR;
    }

    return err;
}

notfs_err_e notfs_port_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    if (NULL == buf) {
        return NOTFS_ILLEGAL_ERR;
    }

    notfs_err_e err = NOTFS_NO_ERR;

//    if (0 > mw_norflash_write(addr, buf, size)) {
//        err = NOTFS_WRITE_ERR;
//    }
    if (0 > mw_norflash_write_directly(addr, buf, size)) {
        err = NOTFS_WRITE_ERR;
    }

    return err;
}

notfs_err_e notfs_port_write_directly(uint32_t addr, const uint8_t *buf, size_t size)
{
    if (NULL == buf) {
        return NOTFS_ILLEGAL_ERR;
    }

    notfs_err_e err = NOTFS_NO_ERR;

    if (0 > mw_norflash_write_directly(addr, buf, size)) {
        err = NOTFS_WRITE_ERR;
    }

    return err;
}

/*****************************(C)COPYRIGHT(c) 2022 Thaisen *****END OF FILE****/
