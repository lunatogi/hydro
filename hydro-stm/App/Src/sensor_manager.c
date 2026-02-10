/*
 * sensor_manager.c
 *
 *  Created on: Dec 17, 2025
 *      Author: Murat Utku KETI
 */

#include "sensor_manager.h"
#include "config_store.h"
#include "sensor_hw.h"

static const SensorConfig_t sensorConfigDefault[SENSOR_COUNT] =
{
	[IDX_TEMP] = {
		.name = "Temperature Sensor",
		.ref = 27.0f,
		.margin = 1.0f,
		.minValue = 25.0f,
		.maxValue = 45.0f,
		.increasePort = 2,
		.increasePin = 2,
		.decreasePort = 2,
		.decreasePin = 3
	},

	[IDX_ALT] = {
		.name = "Altitude Sensor",
		.ref = 75.0f,
		.margin = 3.0f,
		.minValue = 0.0f,
		.maxValue = 2000.0f,
		.increasePort = 2,
		.increasePin = 4,
		.decreasePort = 2,
		.decreasePin = 5
	}
};

static SensorConfig_t sensorConfigRuntime[SENSOR_COUNT];

static SensorState_t sensorState[SENSOR_COUNT] = {0};

SystemConfig_t systemConfig;		//

static volatile uint8_t configDirty = 0;

static void BuildDefaultConfig(SystemConfig_t *cfg){
	cfg->version = CONFIG_VERSION;

	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		cfg->sensor[i].ref = sensorConfigDefault[i].ref;
		cfg->sensor[i].margin = sensorConfigDefault[i].margin;
	}
}

void Sensor_Init(void){
	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){				// Fill Runtime array with default values
		sensorConfigRuntime[i] = sensorConfigDefault[i];
	}

	Config_Load(&systemConfig);

	if(!Config_IsValid(&systemConfig)){
		BuildDefaultConfig(&systemConfig);
		Config_Save(&systemConfig);
	}

	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		sensorConfigRuntime[i].ref = systemConfig.sensor[i].ref;
		sensorConfigRuntime[i].margin = systemConfig.sensor[i].margin;
	}
}

void Sensor_Update(void){
	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		sensorState[i].value = Read_Sensor(i);
	}
}

void Sensor_CommitConfig(void){				// Unused, saves config updates to memory
	if(configDirty){
		Config_Save(&systemConfig);
		configDirty = 0;
	}
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
        return 0;
    }

	return sensorConfigRuntime[idx].increasePort;
}

uint8_t Sensor_GetPortDecrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0;
    }

	return sensorConfigRuntime[idx].decreasePort;
}

uint8_t Sensor_GetPinIncrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0;
    }

	return sensorConfigRuntime[idx].increasePin;
}

uint8_t Sensor_GetPinDecrease(SensorIndex_t idx){
    if (idx >= SENSOR_COUNT){
        return 0;
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
	systemConfig.sensor[idx].ref = ref;
	configDirty = 1;
	return 1;
}

static const float MARGIN_LIMIT = 10.0f;			// It would be better to put it in sensorConfigDefault
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
	systemConfig.sensor[idx].margin = margin;
	configDirty = 1;
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
