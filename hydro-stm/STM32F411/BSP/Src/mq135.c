/*
 * mq135.c
 *
 *  Created on: Apr 23, 2026
 *      Author: Murat Utku KETI
 */
#include "mq135.h"
#include <math.h>

float MQ135_R0 = 10000.0f;
ADC_HandleTypeDef *hadc;

void MQ135_Init(ADC_HandleTypeDef *in_hadc){
	hadc = in_hadc;
}

static uint32_t adc_read_avg(void) {
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        HAL_ADC_Start(hadc);
        HAL_ADC_PollForConversion(hadc, 10);
        sum += HAL_ADC_GetValue(hadc);
        HAL_ADC_Stop(hadc);
    }
    return sum >> 5;
}

float MQ135_ReadRs(void) {
    float v_adc = (adc_read_avg() * ADC_VREF) / ADC_MAX;
    float v_ao  = v_adc * DIVIDER_RATIO;
    if (v_ao < 0.01f) return 0.0f;
    return MQ135_RL * (MQ135_VCC - v_ao) / v_ao;
}

void MQ135_Calibrate(void) {
    MQ135_R0 = MQ135_ReadRs() / MQ135_RS_R0_CLEAN;
}

// Composite "air pollution" ppm — averaged over CO2/NH3/alcohol curves.
// Not gas-specific. Treat as a relative index, not a true concentration.
float MQ135_GetPPM(void) {
    float ratio = MQ135_ReadRs() / MQ135_R0;
    if (ratio <= 0.0f) return 0.0f;
    return MQ135_A * powf(ratio, MQ135_B);
}
