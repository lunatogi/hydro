/*
 * comm_manager.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Murat Utku KETI
 */

#include "comm_manager.h"
#include "sensor_manager.h"
#include "stm32f4xx_hal.h"

SystemSnapshot_t snapPassive;
SystemSnapshot_t snapActive;
uint8_t *rxBuffer = NULL;

void Comm_PassRxBufferPtr(uint8_t *rxBuff){
	rxBuffer = rxBuff;
}

static uint8_t Comm_BuildSwitchMatrix(void){
	uint8_t matrix = 0;
	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		matrix = matrix << 2;
		matrix |= Sensor_GetPinActivity(i);
	}
	return matrix;
}

static void Comm_CopySensorValues(SystemSnapshot_t *snapPtr){
	for(int i = 0; i < SENSOR_COUNT; i++){
		snapPtr->values[i] = Sensor_GetValue(i);
	}
}

static uint16_t CRC16_CCITT(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;   // common initial value

    for (size_t i = 0; i < length; i++)
    {
        crc ^= ((uint16_t)data[i] << 8);

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc = (crc << 1);
        }
    }

    return crc;
}

static uint16_t SystemSnapshot_CalculateCRC(const SystemSnapshot_t *packet)
{
    return CRC16_CCITT((const uint8_t *)packet,
                       sizeof(SystemSnapshot_t) - sizeof(packet->crc));
}

void Comm_UpdateSPISnapshot(void){
	snapPassive.switchMatrix = Comm_BuildSwitchMatrix();
	Comm_CopySensorValues(&snapPassive);
	uint16_t _crc = SystemSnapshot_CalculateCRC(&snapPassive);
	snapPassive.crc = _crc;
	snapActive = snapPassive;		// If struct becomes too big use pointer-swap
}

const void Comm_ClearRxBuffer(uint8_t *rxBuffer){		// CURRENTLY UNUSED
	for(int i = 0; i < SPI_DATA_LENGTH; i++){
		rxBuffer[i] = 0;
	}
}







void Comm_HandleSPIData(void){
	SingleSPIData_t _spiData;
	memcpy(_spiData.raw, rxBuffer, SPI_DATA_LENGTH);
	switch(_spiData.frame.type){
		case 1:			// Reference value change
			Sensor_SetRef(_spiData.frame.id, _spiData.frame.payload);
			break;
	}
	Comm_ClearRxBuffer(rxBuffer);
}

void Comm_FillTxBuffer(uint8_t *txBuffer, uint8_t id, uint8_t type){
	if(id < SENSOR_COUNT){					// Fill with sensor values
		txBuffer[0] = id;
		txBuffer[1] = type;
		float value = Sensor_GetValue(id);
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
