/*
 * sensor_hw.c
 *
 *  Created on: Dec 30, 2025
 *      Author: Murat Utku KETI
 */

#include "sensor_hw.h"
#include "bmp180.h"
#include <math.h>

static float Read_Temperature(void){
	return BMP180_GetTemperature();
}

static float Read_Altitude(void){
	float pressure = (float)BMP180_GetPressure()/100.0f;
	float altitude = -44330*(1.0f-powf(1013.25f/pressure, 0.1903f));
	return altitude;
}

float Read_Sensor(SensorIndex_t idx){

	/*
    if (idx >= SENSOR_COUNT)
    {
        // Invalid sensor index → programming error
        return 0.0f;
    }

	switch(idx){
		case IDX_TEMP: return Read_Temperature();
		case IDX_HUM: return Read_Pressure();	// WARNING!! It's connected to pressure for debug
	}

	return 0.0f;
	*/

	// This part would be better like the top but this way it will always be a warning, below is warning free

    if (idx >= SENSOR_COUNT)
    {
        // Invalid sensor index
        return 0.0f;
    }

	switch(idx){
		case IDX_TEMP: return Read_Temperature();
		case IDX_ALT: return Read_Altitude();
		default: return 0.0f;
	}
}

