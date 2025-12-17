/*
 * sensor_manager.h
 *
 *  Created on: Dec 17, 2025
 *      Author: Administrator
 */

#ifndef INC_SENSOR_MANAGER_H_
#define INC_SENSOR_MANAGER_H_

#include <stdint.h>

typedef uint8_t flag_t;

typedef enum
{
    IDX_TEMP = 0,
    IDX_HUM,

    SENSOR_COUNT
} SensorIndex_t;


typedef struct
{
    const char *name;

    float ref;
    float margin;

    float minValue;
    float maxValue;

    uint8_t increasePin;
    uint8_t decreasePin;

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
flag_t Sensor_GetPinActivity(SensorIndex_t idx);

flag_t Sensor_SetRef(SensorIndex_t idx, float ref);
flag_t Sensor_SetMargin(SensorIndex_t idx, float margin);


#endif /* INC_SENSOR_MANAGER_H_ */
