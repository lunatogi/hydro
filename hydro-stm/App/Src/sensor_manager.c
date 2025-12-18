/*
 * sensor_manager.c
 *
 *  Created on: Dec 17, 2025
 *      Author: Murat Utku KETI
 */


#include "sensor_manager.h"

static const SensorConfig_t sensorConfigDefault[SENSOR_COUNT] =
{
	[IDX_TEMP] = {
		.name = "Temperature Sensor",
		.ref = 30.0f,
		.margin = 1.0f,
		.minValue = 25.0f,
		.maxValue = 45.0f,
		.increasePort = 2,
		.increasePin = 2,
		.decreasePort = 2,
		.decreasePin = 3
	},

	[IDX_HUM] = {
		.name = "Humidity Sensor",
		.ref = 70.0f,
		.margin = 3.0f,
		.minValue = 30.0f,
		.maxValue = 90.0f,
		.increasePort = 2,
		.increasePin = 4,
		.decreasePort = 2,
		.decreasePin = 5
	}
};

static SensorConfig_t sensorConfigRuntime[SENSOR_COUNT];

static SensorState_t sensorState[SENSOR_COUNT] = {0};

void Sensor_Init(void){
	for(uint8_t i = 0; i < SENSOR_COUNT; i++){				// Fill Runtime array with default values
		sensorConfigRuntime[i] = sensorConfigDefault[i];
	}


	// Override Runtime values according to EEPROM values
	/*
    if (EEPROM_LoadSensorConfig(sensorEeprom))
    {
        for (uint8_t i = 0; i < SENSOR_COUNT; i++)
        {
            if (IsValidRef(i, sensorEeprom[i].ref))
                sensorConfigRuntime[i].ref = sensorEeprom[i].ref;

            if (IsValidMargin(i, sensorEeprom[i].margin))
                sensorConfigRuntime[i].margin = sensorEeprom[i].margin;
        }
    }*/
}

void Sensor_Update(void){
	// Get new sensor values from Sensor Lib
}

// GET FUNCTIONS
float Sensor_GetValue(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorState[idx].value;
}

float Sensor_GetRef(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorConfigRuntime[idx].ref;
}

float Sensor_GetMargin(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorConfigRuntime[idx].margin;
}

uint8_t Sensor_GetPortIncrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorConfigRuntime[idx].increasePort;
}

uint8_t Sensor_GetPortDecrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorConfigRuntime[idx].decreasePort;
}

uint8_t Sensor_GetPinIncrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorConfigRuntime[idx].increasePin;
}

uint8_t Sensor_GetPinDecrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0.0f;
    }

	return sensorConfigRuntime[idx].decreasePin;
}

flag_t Sensor_GetPinActivity(SensorIndex_t idx)
{
    if (idx >= SENSOR_COUNT)
        return 0;

    flag_t state = 0;

    if (sensorState[idx].increaseEnabled)
        state |= SENSOR_PIN_INC;

    if (sensorState[idx].decreaseEnabled)
        state |= SENSOR_PIN_DEC;

    return state;
}


// SET FUNCTIONS
flag_t Sensor_SetRef(SensorIndex_t idx, float ref){
    if (idx >= SENSOR_COUNT){
        return 0;
    }

    if (ref < sensorConfigDefault[idx].minValue || ref > sensorConfigDefault[idx].maxValue){
    	return 0;
    }

    if(sensorConfigRuntime[idx].ref == ref){
    	return 0;
    }

	sensorConfigRuntime[idx].ref = ref;
	return 1;
}

static const float MARGIN_LIMIT = 10.0f;
flag_t Sensor_SetMargin(SensorIndex_t idx, float margin){

    if (idx >= SENSOR_COUNT){
        return 0;
    }

    if (margin < 0.0 || margin > MARGIN_LIMIT){
    	return 0;
    }

	if(sensorConfigRuntime[idx].margin == margin){
		return 0;		// No error but value is not changed
	}

	sensorConfigRuntime[idx].margin = margin;
	return 1;
}

flag_t Sensor_SetPinIncrease(SensorIndex_t idx, flag_t state){
	if(idx >= SENSOR_COUNT){
		return 0;
	}

	if(state != 0 && state != 1){
		return 0;
	}

	if(sensorState[idx].increaseEnabled == state){
		return 0;
	}

	sensorState[idx].increaseEnabled = state;
	return 1;
}

flag_t Sensor_SetPinDecrease(SensorIndex_t idx, flag_t state){
	if(idx >= SENSOR_COUNT){
		return 0;
	}

	if(state != 0 && state != 1){
		return 0;
	}

	if(sensorState[idx].decreaseEnabled == state){
		return 0;
	}

	sensorState[idx].decreaseEnabled = state;
	return 1;
}
