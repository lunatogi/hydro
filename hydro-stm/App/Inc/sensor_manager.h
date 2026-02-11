/*
 * sensor_manager.h
 *
 *  Created on: Dec 17, 2025
 *      Author: Murat Utku KETI
 */

#ifndef INC_SENSOR_MANAGER_H_
#define INC_SENSOR_MANAGER_H_

#include <stdint.h>
#include "common_types.h"

#define SENSOR_PIN_INC   (1U << 1)
#define SENSOR_PIN_DEC   (1U << 0)

typedef uint8_t flag_t;

typedef struct
{
    const char *name;

    float ref;
    float margin;

    float minValue;
    float maxValue;

    uint8_t increasePort;
    uint16_t increasePin;

    uint8_t decreasePort;
    uint16_t decreasePin;

} SensorConfig_t;

typedef struct
{
    float value;

    flag_t increaseEnabled;
    flag_t decreaseEnabled;

} SensorState_t;

void Sensor_Init(void);
void Sensor_Update(void);

float Sensor_GetValue(SensorIndex_t idx);
float Sensor_GetRef(SensorIndex_t idx);
float Sensor_GetMargin(SensorIndex_t idx);
uint8_t Sensor_GetPortIncrease(SensorIndex_t idx);
uint8_t Sensor_GetPortDecrease(SensorIndex_t idx);
uint8_t Sensor_GetPinIncrease(SensorIndex_t idx);
uint8_t Sensor_GetPinDecrease(SensorIndex_t idx);
uint8_t Sensor_GetPinActivity(SensorIndex_t idx);

flag_t Sensor_SetRef(SensorIndex_t idx, float ref);
flag_t Sensor_SetMargin(SensorIndex_t idx, float margin);
flag_t Sensor_SetPinIncrease(SensorIndex_t idx, flag_t status);
flag_t Sensor_SetPinDecrease(SensorIndex_t idx, flag_t status);

#endif /* INC_SENSOR_MANAGER_H_ */
