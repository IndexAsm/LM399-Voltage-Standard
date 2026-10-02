#pragma once
#include "stm32g4xx_hal.h"


void TMP102_Init(I2C_HandleTypeDef* i2c);

float TMP102_GetReading(I2C_HandleTypeDef* i2c);