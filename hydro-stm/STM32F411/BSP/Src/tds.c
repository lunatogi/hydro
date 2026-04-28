/*
 * tds.c
 *
 *  Created on: Apr 28, 2026
 *      Author: Murat Utku KETI
 */

/**
 ******************************************************************************
 * @file    tds_sensor.c
 * @brief   Driver for DFRobot Gravity Analog TDS Sensor (SEN0244)
 * @target  STM32 Nucleo-F411RE (HAL)
 ******************************************************************************
 */

#include "tds.h"
#include <stdlib.h>

/* ---------- Module state -------------------------------------------------- */

static ADC_HandleTypeDef *s_hadc      = NULL;
static uint16_t           s_last_raw  = 0;
static float              s_last_volt = 0.0f;

/* ---------- Helpers ------------------------------------------------------- */

static int compare_u16(const void *a, const void *b)
{
    uint16_t va = *(const uint16_t *)a;
    uint16_t vb = *(const uint16_t *)b;
    if (va < vb) return -1;
    if (va > vb) return  1;
    return 0;
}

/**
 * @brief  Point the ADC mux at the TDS channel.
 *         Called before every read so this driver can share ADC1
 *         with other analog sensors.
 */
static HAL_StatusTypeDef select_tds_channel(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = TDS_ADC_CHANNEL;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = TDS_SAMPLING_TIME;
    return HAL_ADC_ConfigChannel(s_hadc, &sConfig);
}

/* ---------- API ----------------------------------------------------------- */

void TDS_Init(ADC_HandleTypeDef *hadc)
{
    s_hadc      = hadc;
    s_last_raw  = 0;
    s_last_volt = 0.0f;
}

float TDS_Read(float temperatureC)
{
    if (s_hadc == NULL) {
        return 0.0f;
    }

    /* 1. Select our channel — required when sharing ADC1 */
    if (select_tds_channel() != HAL_OK) {
        return 0.0f;
    }

    uint16_t buf[TDS_SAMPLES];

    /* 1. Collect samples */
    for (int i = 0; i < TDS_SAMPLES; i++) {
        HAL_ADC_Start(s_hadc);
        if (HAL_ADC_PollForConversion(s_hadc, 10) == HAL_OK) {
            buf[i] = (uint16_t)HAL_ADC_GetValue(s_hadc);
        } else {
            buf[i] = 0;
        }
        HAL_ADC_Stop(s_hadc);
        HAL_Delay(10);
    }

    /* 2. Median filter — robust against the spiky probe signal */
    qsort(buf, TDS_SAMPLES, sizeof(uint16_t), compare_u16);
    uint16_t median = buf[TDS_SAMPLES / 2];

    /* 3. Convert ADC count to voltage */
    float voltage = ((float)median * TDS_VREF) / TDS_ADC_RES;

    /* 4. Temperature compensation (0.02 / °C, reference 25 °C) */
    float compCoeff   = 1.0f + 0.02f * (temperatureC - 25.0f);
    float compVoltage = voltage / compCoeff;

    /* 5. DFRobot polynomial: voltage -> TDS (ppm) */
    float tds = (133.42f * compVoltage * compVoltage * compVoltage
              -  255.86f * compVoltage * compVoltage
              +  857.39f * compVoltage) * 0.5f * TDS_KCOEFF;

    /* 6. Cache for diagnostics */
    s_last_raw  = median;
    s_last_volt = voltage;

    if (tds < 0.0f) tds = 0.0f;
    return tds;
}

float TDS_GetLastVoltage(void)
{
    return s_last_volt;
}

uint16_t TDS_GetLastRaw(void)
{
    return s_last_raw;
}
