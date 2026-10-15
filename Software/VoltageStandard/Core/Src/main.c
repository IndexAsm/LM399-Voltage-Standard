/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */



    /*
        TODO:
            WriteCalibrationData()
            ReadCalibrationData()


            Create a system to read from USB and calibrate

    */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stm32g431xx.h>
#include <stm32g4xx_hal_pwr_ex.h>
#include <stm32g4xx_hal_pcd.h>
#include <stm32g4xx_hal_rcc.h>
#include <stm32_hal_legacy.h>
#include "usbd_cdc_if.h"
#include <TMP102.h>
#include <M24Cxx.h>
#include <CalibrationData.h>
#include <Log.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USB_BUFLEN 128


#define EEPROM_SIZE_KB 8

#define EEPROM_ADDR_INIT_CAL_DONE 0x1
#define EEPROM_ADDR_OPERATING_TIME 0x2
#define EEPROM_ADDR_CAL_DATA_STRUC 0xf

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim16;

/* USER CODE BEGIN PV */
uint8_t usbRxBuffer[USB_BUFLEN];
uint16_t usbRxBufferLen;
uint8_t usbRxFlag = 0;
M24Cxx eeprom;
uint32_t uptime_s = 0;
uint16_t uptime_h;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/*
    EEPROM STRUCTURE
0x2     2 bytes         Uptime
0xf     10 bytes        CalibrationHeader
...     ...             InitialCal
...     ...             Calibrations

*/

uint16_t ReadHourStats(M24Cxx* eeprom) {
    union HourBuffer {
        uint8_t b8[2];
        uint16_t b16;
    }  buffer;

    M24Cxx_Read_Bytes(eeprom, EEPROM_ADDR_OPERATING_TIME, buffer.b8, 2);
    return buffer.b16;
}

void WriteHourStats(M24Cxx* eeprom, uint16_t val) {
    union HourBuffer {
        uint8_t b8[2];
        uint16_t b16;
    }  buffer;

    buffer.b16 = val;

    M24Cxx_Write_Buffer(eeprom, EEPROM_ADDR_OPERATING_TIME, buffer.b8, 2);
}



CalibrationHeader ReadCalHeader(M24Cxx* eeprom) {
    CalibrationHeader header;
    
    M24Cxx_Read_Bytes(eeprom, EEPROM_ADDR_CAL_DATA_STRUC, (uint8_t*)&header, sizeof(CalibrationHeader));

    return header;
}

    
void WriteCalHeader(M24Cxx* eeprom, CalibrationHeader* header) {
    
    M24Cxx_Write_Buffer(eeprom, EEPROM_ADDR_CAL_DATA_STRUC, (uint8_t*)header, sizeof(CalibrationHeader));

}

void InitCalHeader(M24Cxx* eeprom) {
    CalibrationHeader header;
    header.initial_addr = EEPROM_ADDR_CAL_DATA_STRUC + 0x8 + sizeof(CalibrationHeader);
    header.initial_done = 0;
    header.entry_addr = header.initial_addr + 0x8 + sizeof(CalibrationDataInfo);
    header.entry_size = sizeof(CalibrationData);
    header.len = 0;

    WriteCalHeader(eeprom, &header);
}


void WriteInitialCalibration(
    M24Cxx* eeprom, 
    CalibrationDataInfo* calInfo
) {

    // Get Calibration Header
    CalibrationHeader header = ReadCalHeader(eeprom);

    // Check if calibration has been done already
    if (header.initial_done)
        return; // Return if true



    

    // Write to memory

    M24Cxx_Write_Buffer(eeprom, header.initial_addr, (uint8_t*)calInfo, sizeof(CalibrationDataInfo));

    header.initial_done = 1;
    WriteCalHeader(eeprom, &header);
}

/// @brief Reads Initial Cal Data from eeprom
/// @param eeprom - pointer to M24Cxx struct to rea from
/// @param info - pointer to CalibrationDataInfo to write to
/// @return 0 - fail 
/// @return 1 - success
uint8_t ReadInitialCalibration(
    M24Cxx* eeprom,
    CalibrationDataInfo* info
) {

    // Get Calibration Header
    CalibrationHeader header = ReadCalHeader(eeprom);

    // Check if calibration has been done already
    if (!header.initial_done)
        return 0; // Return if false
    
    // Read calibration
    M24Cxx_Read_Bytes(eeprom, header.initial_addr, (uint8_t*)info, sizeof(CalibrationDataInfo));

    return 1;
}


uint8_t WriteCalibrationData(M24Cxx* eeprom, CalibrationData* data) {
    // Get Calibration Header
    CalibrationHeader header = ReadCalHeader(eeprom);


    // Get Addr
    uint16_t addr = header.entry_addr + header.entry_size * header.len;

    // Write data
    M24Cxx_Write_Buffer(eeprom, addr, (uint8_t*)data, sizeof(CalibrationData));

    // Increment lengh and write it back
    header.len++;
    WriteCalHeader(eeprom, &header);
    return 1;
}   

uint8_t ReadCalibrationData(M24Cxx* eeprom, CalibrationData* data, uint16_t index) {
    // Get Calibration Header
    CalibrationHeader header = ReadCalHeader(eeprom);


    // Get Addr
    uint16_t addr = header.entry_addr + header.entry_size * index;
    // Read
    M24Cxx_Read_Bytes(eeprom, addr, (uint8_t*)data, sizeof(CalibrationData));

    return 1;
}   




CalibrationData USB_CalData;
CalibrationDataInfo USB_InitialCal;
uint8_t USB_CalDataPos = 0;
uint16_t USB_Cal_Index = 0;
uint8_t USB_GET_Data_Flag = 0;

#define USB_CALDATA_POS_NONE 0
#define USB_CALDATA_POS_CAL_MASK 1
#define USB_CALDATA_POS_CAL_VOLTAGE 12
#define USB_CALDATA_POS_CAL_DATE 2
#define USB_CALDATA_POS_CAL_END 3
#define USB_CALDATA_POS_CAL_GET_INDEX 4
#define USB_CALDATA_POS_CAL_GET_END 5
#define USB_CALDATA_POS_CAL_GET_INFO 6
#define USB_CALDATA_POS_CAL_GET_INFO_END 7
#define USB_CALDATA_POS_CAL_INIT_SERIAL 8
#define USB_CALDATA_POS_CAL_INIT_DATE 9
#define USB_CALDATA_POS_CAL_INIT_VOLTAGE 10
#define USB_CALDATA_POS_CAL_INIT_END 11

void USB_RXCallback(uint8_t* buf, uint32_t* len) {
    memcpy(usbRxBuffer, buf, *len);
    usbRxBufferLen = *len;
    usbRxBuffer[usbRxBufferLen] = 0;
    usbRxFlag = 1;

    
    switch (USB_CalDataPos)
    {
    case 1: {
        USB_CalDataPos = USB_CALDATA_POS_CAL_VOLTAGE;
        USB_CalData.mask = (uint8_t)strtoul((char*)usbRxBuffer, NULL, 16);
        break;
    }
    case USB_CALDATA_POS_CAL_VOLTAGE: {
        //USB_printf("Date: %s\r\n", usbRxBuffer);
        uint16_t len = strlen((char*)usbRxBuffer);
        if (len >= 12)
            USB_printf("ERR: Invalid Voltage. Too long. Len = %u\r\n", len);
        strcpy(USB_CalData.voltage, (char*)usbRxBuffer);
        USB_CalDataPos = USB_CALDATA_POS_CAL_DATE;
        break;
    }
    case USB_CALDATA_POS_CAL_DATE: {
        //USB_printf("Date: %s\r\n", usbRxBuffer);
        uint16_t len = strlen((char*)usbRxBuffer);
        if (len >= 12)
            USB_printf("ERR: Invalid date. Too long. Len = %u\r\n", len);
        strcpy(USB_CalData.date, (char*)usbRxBuffer);
        USB_CalDataPos = USB_CALDATA_POS_CAL_END;
        break;
    }
    case USB_CALDATA_POS_CAL_END: {
        if(!strcmp((char*)usbRxBuffer, "END")) {
            USB_CalData.uptime = uptime_h;
            USB_CalData.temp = TMP102_GetReading(&hi2c1);
            WriteCalibrationData(&eeprom, &USB_CalData);
            USB_CalDataPos = 0;
            USB_printf("CAL DONE\r\n");
        } else
            USB_printf("ERROR\r\n");

        break;
    }
    case USB_CALDATA_POS_CAL_GET_INDEX: {
        USB_CalDataPos = USB_CALDATA_POS_CAL_GET_END;
        USB_Cal_Index = (uint8_t)strtoul((char*)usbRxBuffer, NULL, 16);
        break;
    }
    case USB_CALDATA_POS_CAL_GET_END: {
        CalibrationData data;
        ReadCalibrationData(&eeprom, &data, USB_Cal_Index);

        uint16_t integer_part = (int)data.temp;
        uint16_t fractional_part = (int)((data.temp - integer_part) * 100);

        USB_printf(
            "BEGIN\r\n"
            "0x%x\r\n"
            "%s\r\n"
            "%s\r\n"
            "%u.%u\r\n"
            "%u\r\n"
            "END\r\n",
            data.mask,
            data.voltage,
            data.date,
            integer_part, fractional_part,
            data.uptime
        );
        USB_CalDataPos = USB_CALDATA_POS_NONE;
        break;
    }
    case USB_CALDATA_POS_CAL_INIT_SERIAL: {
        strcpy(USB_InitialCal.LM399_serial_number, usbRxBuffer);
        USB_CalDataPos = USB_CALDATA_POS_CAL_INIT_DATE;
        break;
    }
    case USB_CALDATA_POS_CAL_INIT_DATE: {
        strcpy(USB_InitialCal.initial_cal_date, usbRxBuffer);
        USB_CalDataPos = USB_CALDATA_POS_CAL_INIT_VOLTAGE;
        break;
    }
    case USB_CALDATA_POS_CAL_INIT_VOLTAGE: {
        strcpy(USB_InitialCal.initial_cal_voltage, usbRxBuffer);
        USB_CalDataPos = USB_CALDATA_POS_CAL_INIT_END;
        break;
    }
    case USB_CALDATA_POS_CAL_INIT_END: {

        USB_InitialCal.initial_cal_temperature = TMP102_GetReading(&hi2c1);
        WriteInitialCalibration(&eeprom, &USB_InitialCal);
        USB_CalDataPos = USB_CALDATA_POS_NONE;
        USB_printf("DONE\r\n");
        break;
    }
    default:
        break;
    }

    if(!strcmp(usbRxBuffer, "BEGIN CAL")){
        if (!HAL_GPIO_ReadPin(CAL_EN_GPIO_Port, CAL_EN_Pin))
            USB_CalDataPos = USB_CALDATA_POS_CAL_MASK;
        else
            USB_printf("ERROR: Function Disabled\r\n");
    } else if(!strcmp(usbRxBuffer, "BEGIN GET_CAL")) {
        USB_CalDataPos = 4;
    } else if(!strcmp(usbRxBuffer, "GET_INFO")) {
    
        CalibrationHeader header = ReadCalHeader(&eeprom);
        USB_printf(
            "BEGIN\r\n"
            "%u\r\n"
            "%u\r\n"
            "%u\r\n"
            "END\r\n",
            header.entry_addr,
            header.entry_size,
            header.len
        );


    } else if(!strcmp(usbRxBuffer, "GET_INIT_CAL")) {
    
        CalibrationDataInfo initCal;
        if (ReadInitialCalibration(&eeprom, &initCal)) {
            uint16_t integer_part = (int)initCal.initial_cal_temperature;
            uint16_t fractional_part = (int)((initCal.initial_cal_temperature - integer_part) * 100);
            USB_printf(
                "BEGIN\r\n"
                "%s\r\n"
                "%s\r\n"
                "%s\r\n"
                "%u.%u\r\n"
                "END\r\n",
                initCal.LM399_serial_number,
                initCal.initial_cal_date,
                initCal.initial_cal_voltage,
                integer_part,
                fractional_part
            );
        } else {
            USB_printf("NO INIT CAL\r\n");
        }

        


    } else if(!strcmp(usbRxBuffer, "RESET")) {
        NVIC_SystemReset();
    } else if(!strcmp(usbRxBuffer, "INIT_FULL")) {
        if (!HAL_GPIO_ReadPin(CAL_EN_GPIO_Port, CAL_EN_Pin)) {
            InitCalHeader(&eeprom);
            USB_printf("DONE\r\n");
        } else {
            USB_printf("ERROR: Function Disabled\r\n");
        }
    } else if(!strcmp(usbRxBuffer, "CLEAR_STATS")) {
        if (!HAL_GPIO_ReadPin(CAL_EN_GPIO_Port, CAL_EN_Pin)) {
            WriteHourStats(&eeprom, 0);
            USB_printf("DONE\r\n");
        } else {
            USB_printf("ERROR: Function Disabled\r\n");
        }
        
    } else if(!strcmp(usbRxBuffer, "GET")) {
        USB_GET_Data_Flag = 1;
    } else if(!strcmp(usbRxBuffer, "BEGIN INIT_CAL")) {
        if (!HAL_GPIO_ReadPin(CAL_EN_GPIO_Port, CAL_EN_Pin)) {
            CalibrationHeader header = ReadCalHeader(&eeprom);
            if (header.initial_done)
                USB_printf("ERROR: Calibration has already been done\r\n");
            else
                USB_CalDataPos = USB_CALDATA_POS_CAL_INIT_SERIAL;
        } else {
            USB_printf("ERROR: Function Disabled\r\n");
        }
    }
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(500);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM16_Init();
  MX_USB_Device_Init();
  /* USER CODE BEGIN 2 */

    // Debug
    //#define HEATUP_TIME 2000
    // Final
    #define HEATUP_TIME 600000


    uint32_t time_setup_start = HAL_GetTick();
    uint8_t heater_state = 0;

    HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, 1);


    // Setup timer

    HAL_TIM_Base_Start(&htim16);
    uint32_t uptime_start = HAL_GetTick();

    eeprom = M24Cxx_Init(
        &hi2c1, 
        0b1010000, 
        8192, 
        32, 
        I2C_MEMADD_SIZE_16BIT
    );

    uptime_h = ReadHourStats(&eeprom);





    HAL_Delay(2000);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
        if (!heater_state) {
            if (HAL_GetTick() - time_setup_start > HEATUP_TIME) {
                heater_state = 1;
                HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, 0);
                HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, 1);
            }
        }



        // Get counter value
        uint32_t uptime = HAL_GetTick();
        if (uptime - uptime_start > 1000) {
            uptime_start = uptime;
            uptime_s++;
            if (uptime_s >= 3600) {
                uptime_s = 0;
                uptime_h++;
                WriteHourStats(&eeprom, uptime_h);
            }

        }
        if (USB_GET_Data_Flag) {

            USB_GET_Data_Flag = 0;
            
            USB_printf("BEGIN\r\n");
            // Uptime
            USB_printf("Uptime: %uh %umin %us\r\n",uptime_h, uptime_s / 60, uptime_s % 60);




            // Temperature external
            float temp_ex = TMP102_GetReading(&hi2c1);
            uint32_t integer_part = (int)temp_ex;
            uint32_t fractional_part = (int)((temp_ex - integer_part) * 100);
            USB_printf("Temperature External: %lu.%02lu°C\r\n", integer_part, fractional_part);

            // Reference Voltage
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            uint16_t reference_voltage_raw = HAL_ADC_GetValue(&hadc1);
            float reference_voltage = reference_voltage_raw * 18.5f;
            reference_voltage /= 4095.0f;
            const float offset_reference_voltage = 0.0f;
            reference_voltage += offset_reference_voltage;
            integer_part = (int)reference_voltage;
            fractional_part = (int)((reference_voltage - integer_part) * 100);
            USB_printf("Reference Voltage: %lu.%02luV\r\n", integer_part, fractional_part);
            
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            uint16_t heater_voltage_raw = HAL_ADC_GetValue(&hadc1);
            float heater_voltage = heater_voltage_raw * 18.5f;
            heater_voltage /= 4095.0f;
            const float offset_heater_voltage = 0.0f;
            heater_voltage += offset_heater_voltage;
            integer_part = (int)heater_voltage;
            fractional_part = (int)((heater_voltage - integer_part) * 100);
            USB_printf("Heater Voltage: %lu.%02luV\r\n", integer_part, fractional_part);

            // Heater current
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            uint16_t heater_current_raw = HAL_ADC_GetValue(&hadc1);
            double heater_current = heater_current_raw * 3.3f;
            heater_current /= 4095.0f;
            heater_current *= 100.0f;
            const double offset_heater_current = 0.0f;
            heater_current += offset_heater_current;
            integer_part = (int)heater_current;
            fractional_part = (int)((heater_current - integer_part) * 100);
            USB_printf("Heater Current: %lu.%02lumA\r\n", integer_part, fractional_part);

            HAL_ADC_Stop(&hadc1);

            /*
            uint32_t heater_voltage_raw;
            HAL_ADC_Start(&hadc1);

            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            heater_voltage_raw = HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);
            // 4095 - 18.5
            // raw - x
            float heater_voltage = heater_voltage_raw * 18.5f;
            heater_voltage /= 4095.0f;
            const float offset = 0.0f;
            heater_voltage += offset;
            integer_part = (int)heater_voltage;
            fractional_part = (int)((heater_voltage - integer_part) * 100);
            USB_printf("Heater Voltage: %lu.%luV\r\n", integer_part, fractional_part);
            */
            USB_printf("END\r\n");

            
        }


        /*
        if (usbRxFlag && usbRxBufferLen) {
            USB_printf(usbRxBuffer);

            




            usbRxBuffer[0] = 0;
            usbRxFlag = 0;
            usbRxBufferLen = 0;
        }*/


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 18;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV6;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_24CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x60715075;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 4000-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 65535;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_R_Pin|LED_G_Pin|LED_B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CAL_EN_Pin */
  GPIO_InitStruct.Pin = CAL_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(CAL_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_R_Pin LED_G_Pin LED_B_Pin */
  GPIO_InitStruct.Pin = LED_R_Pin|LED_G_Pin|LED_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_SENSE_Pin */
  GPIO_InitStruct.Pin = VBUS_SENSE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_SENSE_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
