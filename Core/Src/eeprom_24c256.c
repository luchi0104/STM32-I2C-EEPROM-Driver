/*
 * eeprom_24c256.c
 *
 *  Created on: 2026年11月17日
 *      Author: My_User
 */


#include "eeprom_24c256.h"
#include <stddef.h>

#define EEPROM_WRITE_TIMEOUT_MS 100U
#define EEPROM_READY_TIMEOUT_MS 20U

static I2C_HandleTypeDef *eeprom_i2c = NULL;

/* Private helper: only used inside this source file */
static HAL_StatusTypeDef EEPROM_WaitReady(uint32_t timeout_ms)
{
    if (eeprom_i2c == NULL)
    {
        return HAL_ERROR;
    }

    uint32_t start_tick = HAL_GetTick();

    while (1)
    {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(
            eeprom_i2c,
            EEPROM24C256_I2C_ADDR,
            1,
            10
        );

        if (status == HAL_OK)
        {
            return HAL_OK;
        }

        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }

        uint32_t error = HAL_I2C_GetError(eeprom_i2c);

        /*
         * This STM32L4 HAL implementation may report
         * TIMEOUT when address trials are exhausted.
         */
        if (status != HAL_ERROR ||
            (error != HAL_I2C_ERROR_TIMEOUT &&
             error != HAL_I2C_ERROR_AF))
        {
            return status;
        }

        if ((HAL_GetTick() - start_tick) >= timeout_ms)
        {
            return HAL_TIMEOUT;
        }

        HAL_Delay(1);
    }
}

HAL_StatusTypeDef EEPROM24C256_Init(
    I2C_HandleTypeDef *hi2c
)
{
    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    eeprom_i2c = hi2c;

    return HAL_I2C_IsDeviceReady(
        eeprom_i2c,
        EEPROM24C256_I2C_ADDR,
        3,
        100
    );
}

HAL_StatusTypeDef EEPROM24C256_Write(
    uint16_t mem_addr,
    const uint8_t *data,
    uint16_t size
)
{
    if (eeprom_i2c == NULL)
    {
        return HAL_ERROR;
    }

    if (size == 0U)
    {
        return HAL_OK;
    }

    if (data == NULL ||
        ((uint32_t)mem_addr + size) > EEPROM24C256_CAPACITY)
    {
        return HAL_ERROR;
    }

    uint32_t current_addr = mem_addr;
    uint16_t remaining = size;
    const uint8_t *current_data = data;

    while (remaining > 0U)
    {
        uint16_t page_offset =
            (uint16_t)(current_addr % EEPROM24C256_PAGE_SIZE);

        uint16_t page_remaining =
            EEPROM24C256_PAGE_SIZE - page_offset;

        uint16_t chunk_size =
            (remaining < page_remaining)
            ? remaining
            : page_remaining;

        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
            eeprom_i2c,
            EEPROM24C256_I2C_ADDR,
            (uint16_t)current_addr,
            I2C_MEMADD_SIZE_16BIT,
            (uint8_t *)current_data,
            chunk_size,
            EEPROM_WRITE_TIMEOUT_MS
        );

        if (status != HAL_OK)
        {
            return status;
        }

        status = EEPROM_WaitReady(EEPROM_READY_TIMEOUT_MS);

        if (status != HAL_OK)
        {
            return status;
        }

        current_addr += chunk_size;
        current_data += chunk_size;
        remaining -= chunk_size;
    }

    return HAL_OK;
}

HAL_StatusTypeDef EEPROM24C256_Read(
    uint16_t mem_addr,
    uint8_t *data,
    uint16_t size
)
{
    if (eeprom_i2c == NULL)
    {
        return HAL_ERROR;
    }

    if (size == 0U)
    {
        return HAL_OK;
    }

    if (data == NULL ||
        ((uint32_t)mem_addr + size) > EEPROM24C256_CAPACITY)
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(
        eeprom_i2c,
        EEPROM24C256_I2C_ADDR,
        mem_addr,
        I2C_MEMADD_SIZE_16BIT,
        data,
        size,
        100
    );
}

