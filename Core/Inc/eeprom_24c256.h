/*
 * eeprom_24c256.h
 *
 *  Created on: 2026年11月17日
 *      Author: My_User
 */

#ifndef INC_EEPROM_24C256_H_
#define INC_EEPROM_24C256_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

/* EEPROM hardware configuration */
#define EEPROM24C256_CAPACITY   32768U
#define EEPROM24C256_PAGE_SIZE  64U
#define EEPROM24C256_I2C_ADDR   (0x50U << 1)

/* Initialize the driver with an I2C peripheral */
HAL_StatusTypeDef EEPROM24C256_Init(
    I2C_HandleTypeDef *hi2c
);

/* Write data with automatic page boundary handling */
HAL_StatusTypeDef EEPROM24C256_Write(
    uint16_t mem_addr,
    const uint8_t *data,
    uint16_t size
);

/* Read consecutive bytes */
HAL_StatusTypeDef EEPROM24C256_Read(
    uint16_t mem_addr,
    uint8_t *data,
    uint16_t size
);

#endif /* INC_EEPROM_24C256_H_ */
