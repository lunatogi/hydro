/*
 * tds.h
 *
 *  Created on: Apr 28, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_TDS_H_
#define INC_TDS_H_

/**
 ******************************************************************************
 * @file    tds_sensor.h
 * @brief   Driver for DFRobot Gravity Analog TDS Sensor (SEN0244)
 * @target  STM32 Nucleo-F411RE (HAL)
 *
 * Wiring:
 *   VCC (red)    -> +5V
 *   GND (black)  -> GND
 *   AOUT (blue)  -> PA1  (ADC1_IN0, Arduino header A1)
 *
 * Usage:
 *   1. In CubeMX enable ADC1 channel IN0, 12-bit, sampling 480 cycles.
 *   2. In main.c:  #include "tds_sensor.h"
 *                  TDS_Init(&hadc1);
 *                  float ppm = TDS_Read(25.0f);
 ******************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ---------- Configuration (override in build flags if needed) ------------- */

#ifndef TDS_VREF
#define TDS_VREF        3.3f      /* ADC reference voltage (V)          */
#endif

#ifndef TDS_ADC_RES
#define TDS_ADC_RES     4095.0f   /* 12-bit ADC max count               */
#endif

#ifndef TDS_SAMPLES
#define TDS_SAMPLES     30        /* samples per reading (median filter)*/
#endif

#ifndef TDS_KCOEFF
#define TDS_KCOEFF      1.0f      /* probe calibration coefficient      */
#endif

#ifndef TDS_ADC_CHANNEL
#define TDS_ADC_CHANNEL ADC_CHANNEL_1          /* PA1 = ADC1_IN1          */
#endif

#ifndef TDS_SAMPLING_TIME
#define TDS_SAMPLING_TIME ADC_SAMPLETIME_480CYCLES
#endif
/* ---------- API ----------------------------------------------------------- */

/**
 * @brief  Bind the driver to a configured ADC handle.
 * @param  hadc  pointer to HAL ADC handle (e.g. &hadc1)
 */
void  TDS_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief  Take one filtered TDS reading.
 * @param  temperatureC  current water temperature in °C
 *                       (pass 25.0f if no temperature sensor is available)
 * @return TDS value in ppm (mg/L)
 */
float TDS_Read(float temperatureC); // Get the raw probe voltage from the last reading (V). Useful for calibration and debugging.

float TDS_GetLastVoltage(void); // Get the raw median ADC count from the last reading (0-4095).

uint16_t TDS_GetLastRaw(void);

#ifdef __cplusplus
}
#endif



#endif /* INC_TDS_H_ */
