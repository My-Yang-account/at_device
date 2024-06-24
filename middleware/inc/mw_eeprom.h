/**
 ******************************************************************************
 * @file mw_eeprom.h
 * @author leven
 * @brief 
 ******************************************************************************
 */

#ifndef MW_EEPROM_H_
#define MW_EEPROM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @return -1：失败；0：成功
 */
int32_t mw_eeprom_init(void);

/**
 * @param address 起始地址
 * @param buffer 读取到的数据
 * @param size 从起始地址开始读取数据的总大小
 * @return -1：失败；0：成功
 */
int32_t mw_eeprom_read(uint32_t address, uint8_t *buffer, uint16_t size);

/**
 * @param address 起始地址
 * @param buffer 待写入的数据
 * @param size 从起始地址开始写入数据的总大小
 * @return -1：失败；0：成功
 */
int32_t mw_eeprom_write(uint32_t address, uint8_t *buffer, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* MW_EEPROM_H_ */
