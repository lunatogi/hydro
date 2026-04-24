/*
 * mq135.h
 *
 *  Created on: Apr 23, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_MQ135_H_
#define INC_MQ135_H_

#include "main.h"

#define MQ135_VCC           5.0f
#define MQ135_RL            1000.0f
#define ADC_VREF            3.3f
#define ADC_MAX             4095.0f
#define DIVIDER_RATIO       1.5f
#define MQ135_RS_R0_CLEAN   3.6f

// Middle-of-the-road curve (avg of CO2, NH3, alcohol coefficients)
#define MQ135_A             96.6f
#define MQ135_B             -2.84f

extern float MQ135_R0;

void MQ135_Init(ADC_HandleTypeDef *in_hadc);
void  MQ135_Calibrate(void);
float MQ135_ReadRs   (void);
float MQ135_GetPPM   (void);   // single composite reading

#endif /* INC_MQ135_H_ */
