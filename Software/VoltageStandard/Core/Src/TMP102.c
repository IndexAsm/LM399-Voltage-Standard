#include "TMP102.h"
#include <stdint.h>

#define TMP102_ADDRESS 0x48 << 1
#define TMP102_TEMP_REG 0x00



float TMP102_GetReading(I2C_HandleTypeDef* i2c) {
    uint8_t buf[8];
    buf[0] = TMP102_TEMP_REG;
    HAL_StatusTypeDef ret;
    uint16_t reading = 0x0;
    float temp = 0.0f;

    ret = HAL_I2C_Master_Transmit(i2c, TMP102_ADDRESS, buf, 1, HAL_MAX_DELAY);

    if (ret != HAL_OK)
        return -1.0f;

    ret = HAL_I2C_Master_Receive(i2c, TMP102_ADDRESS, buf, 2, HAL_MAX_DELAY);

    if (ret != HAL_OK)
        return -1.0f;
    reading = ((uint16_t)buf[0] << 4 | (buf[1] >> 4));

    if (reading > 0x7ff) {
        reading |= 0xf000;
    }

    temp = reading * 0.0625f;


    return temp;
}