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
#include "comm_manager.h"
#include "LCD.h"

static uint32_t lastSensorTick = 0;
static uint32_t lastESPTick = 0;
static uint32_t lastSaveTick = 0;		// Maybe this one is not needed
static uint32_t lastControlTick = 0;

static const uint32_t delaySensorTick = 5000;
static const uint32_t delayControlTick = delaySensorTick;	// Tied by design
//static const uint32_t delaySaveTick = 10000;

flag_t SPI_Done_Flag = 0;

void LCDUpdate(){

	char msg[32];
	float value = Sensor_GetValue(IDX_TEMP);
	snprintf(msg, sizeof(msg), "Temp: %.2f", value);

    LCD_Clear();

    LCD_SetCursor(0, 0);
	LCD_SendString(msg);

	value = Sensor_GetValue(IDX_ALT);
	snprintf(msg, sizeof(msg), "Alt: %.2f", value);

	LCD_SetCursor(1, 0);
	LCD_SendString(msg);
}

void SensorUpdateRoutine(void){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);		// BUSY PIN
	HAL_NVIC_DisableIRQ(SPI2_IRQn);
	Sensor_Update();
	HAL_NVIC_EnableIRQ(SPI2_IRQn);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	LCDUpdate();
}


void Scheduler_Init(void){
	uint32_t nowTick = HAL_GetTick();

	lastSensorTick = nowTick;
	lastESPTick = nowTick;
	lastSaveTick = nowTick;

	SensorUpdateRoutine();		// Might need to call all functions here for proper initialization
}

void Scheduler_Run(void){
	uint32_t nowTick = HAL_GetTick();
	if(nowTick - lastSensorTick >= delaySensorTick){
		// Check Sensors Values
		SensorUpdateRoutine();
		lastSensorTick = nowTick;
	}

	if(nowTick - lastControlTick >= delayControlTick){
		// Adjust according to sensor values
		ControlLoop_Run();
		lastControlTick = nowTick;
	}

	if(SPI_Done_Flag){
		Comm_UpdateSPISnapshot();
		SPI_Done_Flag = 0;
	}

	//if(nowTick - lastESPTick >= delayESPTick){
	//	// Communicate with ESP
	//	Comm_SendCurrentValues();
	//	lastESPTick = nowTick;
	//}

	//if(nowTick - lastSaveTick >= delaySaveTick){
		// Save to EEPROM
	//	lastSaveTick = nowTick;
	//}

	// RTC (Real-time Clock) Update Here
}

