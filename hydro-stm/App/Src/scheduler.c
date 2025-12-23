/*
 * scheduler.c
 *
 *  Created on: Dec 16, 2025
 *      Author: Murat Utku KETI
 */

#include "stm32f4xx_hal.h"
#include "scheduler.h"
#include "sensor_manager.h"
#include "control_loop.h"

static uint32_t lastSensorTick = 0;
static uint32_t lastESPTick = 0;
static uint32_t lastSaveTick = 0;		// Maybe this one is not needed
static uint32_t lastControlTick = 0;

static const uint32_t delaySensorTick = 500;
static const uint32_t delayControlTick = delaySensorTick;	// Tied by design
static const uint32_t delayESPTick = 1000;
static const uint32_t delaySaveTick = 10000;

void Scheduler_Init(void){
	uint32_t nowTick = HAL_GetTick();

	lastSensorTick = nowTick;
	lastESPTick = nowTick;
	lastSaveTick = nowTick;
}

void Scheduler_Run(void){
	uint32_t nowTick = HAL_GetTick();

	if(nowTick - lastSensorTick >= delaySensorTick){
		// Check Sensors Values
		Sensor_Update();
		lastSensorTick = nowTick;
	}

	if(nowTick - lastControlTick >= delayControlTick){
		// Adjust according to sensor values
		ControlLoop_Run();
		lastSensorTick = nowTick;
	}

	if(nowTick - lastESPTick >= delayESPTick){
		// Communicate with ESP
		lastESPTick = nowTick;
	}

	if(nowTick - lastSaveTick >= delaySaveTick){
		// Save to EEPROM
		lastSaveTick = nowTick;
	}

	// RTC (Real-time Clock) Update Here
}

