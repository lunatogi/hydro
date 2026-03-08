/*
 * comm_manager.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Murat Utku KETI
 */

#include "comm_manager.h"
#include "sensor_manager.h"

const void Comm_ClearRxBuffer(uint8_t *rxBuffer){		// CURRENTLY UNUSED
	for(int i = 0; i < SPI_DATA_LENGTH; i++){
		rxBuffer[i] = 0;
	}
}

void Comm_HandleSPIData(uint8_t *rxBuffer){
	SingleSPIData_t spiData;
	memcpy(spiData.raw, rxBuffer, SPI_DATA_LENGTH);
	switch(_spiData.frame.type){
		case 1:			// Reference value change
			Sensor_SetRef(_spiData.frame.id, _spiData.frame.payload);
			break;
	}
	Comm_ClearBuffers(rxBuffer);
}

const void Comm_SendIncDecMatrix(void){
	spi2tx_buffer[0] = 0;
	spi2tx_buffer[1] = 2;
	uint8_t IncDecMatrix = 0;
	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		IncDecMatrix = IncDecMatrix << 2;
		IncDecMatrix |= Sensor_GetPinActivity(i);
	}
	spi2tx_buffer[2] = IncDecMatrix;
}

void Comm_FillTxBuffer(uint8_t *txBuffer, uint8_t id, uint8_t type){
	if(id < SENSOR_COUNT){					// Fill with sensor values
		txBuffer[0] = id;
		txBuffer[1] = type;
		float value = Sensor_GetValue(i);
		memcpy(&txBuffer[2], &value, sizeof(float));
	}else if(id == SENSOR_COUNT){			// Fill with IncDec Matrix
		txBuffer[0] = 0;
		txBuffer[1] = 2;
		uint8_t IncDecMatrix = 0;
		for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
			IncDecMatrix = IncDecMatrix << 2;
			IncDecMatrix |= Sensor_GetPinActivity(i);
		}
		txBuffer[2] = IncDecMatrix;				// txBuffer 3, 4 and 5 can be filled with jung data at this point
	}else{
		// INVALID ID
	}
}
