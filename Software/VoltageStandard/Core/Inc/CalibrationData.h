#pragma once
#include <stdint.h>





typedef struct {
    char LM399_serial_number[20];
    char initial_cal_date[12];
    char initial_cal_voltage[12];
    float initial_cal_temperature;

} CalibrationDataInfo;

typedef struct  {
    uint16_t len; // Amount of calibration entries
    uint16_t entry_size; // Size of each entry
    uint16_t entry_addr; // Starting address of entries

    uint16_t initial_addr; // Address of initial calibration
    uint16_t initial_done;
} CalibrationHeader;

typedef struct {
    uint8_t mask;       // Calibration bit mask
    char voltage[12];   // Voltage before calibration
    char date[12];      // Date of calibration 
    float temp;         // Temperature during calibration
    uint16_t uptime;    // Uptime at the time of calibration
} CalibrationData;
