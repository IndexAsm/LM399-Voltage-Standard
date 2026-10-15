#include "M24Cxx.h"

#include <math.h>


static uint8_t is_same_page(uint16_t a0, uint16_t a1, uint8_t page_address_bits) {
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

uint8_t M24Cxx_Write_Buffer(M24Cxx *device, uint16_t addr, const uint8_t *data, uint16_t len) {

    // Basic error checks
    if (!device || !data || addr + len > device->size)
        return 0;

    uint16_t sent = 0;


    while (sent < len) {
        uint32_t cur = addr + sent;
        uint16_t room = device->page_size - (cur % device->page_size);
        uint16_t chunk = (len - sent < room) ? (len - sent) : room;

        HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(device->i2c, device->address, cur, device->addressing_size, (uint8_t*)(data + sent), chunk, 100);

        if (ret != HAL_OK)
            return 0;

        uint32_t start = HAL_GetTick();

        while(HAL_I2C_IsDeviceReady(device->i2c, device->address, 1, 2) != HAL_OK) {
            if (HAL_GetTick() - start > 20)
                return 0;
        }
        sent += chunk;

    }
    return 1;
    
}

void M24Cxx_Write_Byte(M24Cxx* device, uint16_t address, uint8_t data) {
    HAL_I2C_IsDeviceReady(device->i2c, device->address, 2, 15);
    HAL_I2C_Mem_Write(device->i2c, device->address, address, device->addressing_size, &data, 1, 10);
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