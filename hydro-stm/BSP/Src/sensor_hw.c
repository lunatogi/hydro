/*
 * sensor_hw.c
 *
 *  Created on: Dec 30, 2025
 *      Author: Murat Utku KETI
 */

#include "sensor_hw.h"
#include "bmp180.h"

static float Read_Temperature(void){
	return BMP180_GetTemperature();
}

static float Read_Pressure(void){
	return (float)BMP180_GetPressure();
}

float Read_Sensor(SensorIndex_t idx){
	switch(idx){
		case IDX_TEMP: return Read_Temperature(); break;
		case IDX_HUM: return Read_Pressure(); break;	// WARNING!! It's connected to pressure for debug
		default: return 0.0f;
	}
}

