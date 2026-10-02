#pragma once
#include <stdint.h>



typedef struct {
    uint16_t len;
    uint16_t entry_size;
    uint16_t entry_addr;
    char LM399_serial_number[16];
    char initial_cal_date[10];
    char initial_cal_voltage[12];
    float initial_cal_temperature;

} CalibrationDataInfo;
