/*
 * control_loop.c
 *
 *  Created on: Dec 18, 2025
 *      Author: Murat Utku KETI
 */

#include "actuator_manager.h"
#include "control_loop.h"
#include "sensor_manager.h"

void ControlLoop_Run(void){
	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		float currentValue = Sensor_GetValue(i);
		float referenceValue = Sensor_GetRef(i);
		float marginValue = Sensor_GetMargin(i);
		uint8_t incPort = Sensor_GetPortIncrease(i);
		uint8_t incPin = Sensor_GetPinIncrease(i);
		uint8_t decPort = Sensor_GetPortDecrease(i);
		uint8_t decPin = Sensor_GetPinDecrease(i);
		flag_t pinActivity = Sensor_GetPinActivity(i);

		flag_t decreaseEn = (pinActivity & SENSOR_PIN_DEC) ? 1 : 0;		// If zero -> false, if non-zero -> true
		flag_t increaseEn = (pinActivity & SENSOR_PIN_INC) ? 1 : 0;

		float maxValue = referenceValue + marginValue;
		float minValue = referenceValue - marginValue;

		if((currentValue < minValue) && !increaseEn){
			if(Set_Pin(incPort, incPin, 1))
				Sensor_SetPinIncrease(i, 1);
		}else if((currentValue > maxValue) && !decreaseEn){
			if(Set_Pin(decPort, decPin, 1))
				Sensor_SetPinDecrease(i, 1);
		}else{
			if(increaseEn && (currentValue >= minValue)){
				if(Set_Pin(incPort, incPin, 0))
					Sensor_SetPinIncrease(i, 0);
			}else if(decreaseEn && (currentValue <= maxValue)){
				if(Set_Pin(decPort, decPin, 0))
					Sensor_SetPinDecrease(i, 0);
			}
		}
	}
}



