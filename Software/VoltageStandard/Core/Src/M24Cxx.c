#include "M24Cxx.h"

#include <math.h>

uint8_t is_same_page(uint16_t a0, uint16_t a1, uint8_t page_address_bits) {
    return (a0 >> page_address_bits) == (a1 >> page_address_bits);
}


M24Cxx M24Cxx_Init(I2C_HandleTypeDef *i2c, uint8_t i2c_address, uint16_t size, uint8_t page_size, uint8_t addressing_size)
{
    M24Cxx res;
    res.i2c = i2c;
    res.address = i2c_address << 1;
    res.size = size;
    res.page_size = page_size;
    res.addressing_size = addressing_size;
    res.page_address_bits = (uint8_t)log2f(res.page_size);
    return res;
}

void M24Cxx_Write_Buffer(M24Cxx *device, uint16_t address, uint8_t *data, uint16_t len) {

    uint16_t sent = 0;

    while (sent < len) {
        uint16_t current_address = address + sent;

        uint16_t bytes_left = device->page_size - (current_address % device->page_size);

        uint16_t chunk_size = (len - sent < bytes_left) ? (len - sent) : bytes_left;

        while (HAL_I2C_IsDeviceReady(device->i2c, device->address, 2, 10) != HAL_OK);
        HAL_I2C_Mem_Write(device->i2c, device->address, current_address, device->addressing_size, data, chunk_size, 10);

        sent += chunk_size;
    }

    
}

void M24Cxx_Write_Byte(M24Cxx* device, uint16_t address, uint8_t data) {
    HAL_I2C_IsDeviceReady(device->i2c, device->address, 2, 15);
    HAL_I2C_Mem_Write(device->i2c, device->address, address, device->addressing_size, &data, 1, 10);
}

void M24Cxx_Read_Page(M24Cxx *device, uint16_t address, uint8_t *data) {
    
}

void M24Cxx_Read_Bytes(M24Cxx* device, uint16_t address, uint8_t* data, uint16_t len) {
    HAL_I2C_IsDeviceReady(device->i2c, device->address, 1, 15);
    HAL_I2C_Mem_Read(device->i2c, device->address, address, device->addressing_size, data, len, 10);
}

uint8_t M24Cxx_Read_Byte(M24Cxx *device, uint16_t address) {
    uint8_t res = 0;

    HAL_I2C_IsDeviceReady(device->i2c, device->address, 1, 15);
    HAL_I2C_Mem_Read(device->i2c, device->address, address, device->addressing_size, &res, 1, 10);
    return res;
}