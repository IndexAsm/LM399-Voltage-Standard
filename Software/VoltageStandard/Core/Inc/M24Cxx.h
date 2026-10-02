#pragma once
#include "stm32g4xx_hal.h"
#include <stdint.h>



typedef struct {
    I2C_HandleTypeDef* i2c;     // Handle to I2C
    uint8_t address;            // Device Address
    uint16_t size;              // Memory size in KB
    uint8_t page_size;          // Size of 1 page
    uint8_t addressing_size;
} M24Cxx;

M24Cxx M24Cxx_Init(I2C_HandleTypeDef* i2c, uint8_t i2c_address, uint16_t size, uint8_t page_size, uint8_t addressing_size);

void M24Cxx_Write_Buffer(M24Cxx* device, uint16_t address, uint8_t* data, uint16_t len);
void M24Cxx_Write_Byte(M24Cxx* device, uint16_t address, uint8_t data);

void M24Cxx_Read_Page(M24Cxx* device, uint16_t address, uint8_t* data);

void M24Cxx_Read_Bytes(M24Cxx* device, uint16_t address, uint8_t* data, uint16_t len);

uint8_t M24Cxx_Read_Byte(M24Cxx *device, uint16_t address);