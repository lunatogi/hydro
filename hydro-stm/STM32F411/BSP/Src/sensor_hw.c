/*
 * sensor_hw.c
 *
 *  Created on: Dec 30, 2025
 *      Author: Murat Utku KETI
 */

#include "sensor_hw.h"
#include "bmp180.h"
#include "mq135.h"
#include "tds.h"
#include "aht10.h"
#include <math.h>

static float Read_Temperature(void){
	return BMP180_GetTemperature();
}

static float Read_Altitude(void){
	float pressure = (float)BMP180_GetPressure()/100.0f;
	float altitude = -44330*(1.0f-powf(1013.25f/pressure, 0.1903f));
	return altitude;
}

static float Read_FF(void){
	return MQ135_GetPPM();
}

static float Read_TDS(void){
	return TDS_Read(Read_Temperature());
}

static float Read_Humidity(void){
	return AHT10_GetHumidity();
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
		case IDX_TEMP: 	return Read_Temperature();
		case IDX_ALT: 	return Read_Altitude();
		case IDX_FF: 	return Read_FF();
		case IDX_TDS: 	return Read_TDS();
		case IDX_HUM: 	return Read_Humidity();
		default: 		return 0.0f;
	}
}

