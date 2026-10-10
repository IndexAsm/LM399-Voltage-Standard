#pragma once
#include <stdint.h>





typedef struct {
    uint16_t len; // Amount of calibration entries
    uint16_t entry_size; // Size of each entry
    uint16_t entry_addr; // Starting address of entries
    char LM399_serial_number[16];
    char initial_cal_date[10];
    char initial_cal_voltage[12];
    float initial_cal_temperature;

} CalibrationDataInfo;


typedef struct {
    uint8_t mask;       // Calibration bit mask
    char date[10];      // Date of calibration
    float temp;         // Temperature during calibration
    uint16_t uptime;    // Uptime at the time of calibration
} CalibrationData;
