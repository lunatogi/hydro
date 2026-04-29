/*
 * aht10.h
 *
 *  Created on: Apr 28, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_AHT10_H_
#define INC_AHT10_H_

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- I2C address ---- */
#define AHT10_I2C_ADDR              (0x38 << 1)   /* HAL expects 8-bit form */

/* ---- Commands (datasheet §5.3) ---- */
#define AHT10_CMD_INIT              0xBE		//0xE1
#define AHT10_CMD_MEASURE           0xAC
#define AHT10_CMD_SOFT_RESET        0xBA

#define AHT10_INIT_PARAM1           0x08    /* enable calibration */
#define AHT10_INIT_PARAM2           0x00
#define AHT10_MEASURE_PARAM1        0x33
#define AHT10_MEASURE_PARAM2        0x00

/* ---- Status bits ---- */
#define AHT10_STATUS_BUSY           0x80
#define AHT10_STATUS_CALIBRATED     0x08

/* ---- Timing (ms) ---- */
#define AHT10_POWERON_DELAY_MS      40
#define AHT10_MEASUREMENT_DELAY_MS  80    /* datasheet typ 75 ms */
#define AHT10_RESET_DELAY_MS        20
#define AHT10_I2C_TIMEOUT_MS        100

/* 2^20, used for raw -> physical conversion */
#define AHT10_RAW_DIVISOR_F         1048576.0f

typedef enum {
    AHT10_OK       = 0,
    AHT10_ERROR    = 1,   /* I2C error or no ACK */
    AHT10_BUSY     = 2,   /* sensor still converting */
    AHT10_NOT_CAL  = 3,   /* calibration bit not set after init */
    AHT10_NOT_INIT = 4    /* AHT10_Init() never called (or it failed) */
} AHT10_Status;

AHT10_Status AHT10_Init(I2C_HandleTypeDef *hi2c);
float AHT10_GetTemperature(void);
float AHT10_GetHumidity(void);
AHT10_Status AHT10_Read(float *temperature, float *humidity);
AHT10_Status AHT10_SoftReset(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_AHT10_H_ */
