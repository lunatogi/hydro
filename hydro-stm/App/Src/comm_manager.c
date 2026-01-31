/*
 * comm_manager.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Murat Utku KETI
 */

#include "comm_manager.h"
#include "sensor_manager.h"

static const CommInterface_t *comm_if;

//uint8_t BUFFER_SIZE = 4;
//uint8_t spi2tx_buffer[4] = {0xAA, 0xAA, 0xAA, 0xAA};
//uint8_t spi2rx_buffer[4] = {0};

void CommManager_Init(const CommInterface_t *comm){
	comm_if = comm;
}

void CommManager_SendRecv(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t BUFFER_SIZE){
	comm_if->send_recv(tx_buffer, rx_buffer, BUFFER_SIZE);
}

void Comm_SendCurrentValues(void){
	//uint8_t buffSize = 9;
	uint8_t currentRx_buffer[6] = {0};
	uint8_t currentTx_buffer[6] = {0};
	currentTx_buffer[0] = SENSOR_COUNT;
	CommManager_SendRecv(currentTx_buffer, currentRx_buffer, 1);
	HAL_Delay(10);
	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		currentTx_buffer[0] = i;
		currentTx_buffer[1] = 0;
		float value = Sensor_GetValue(i);
		memcpy(&currentTx_buffer[2], &value, sizeof(float));
		CommManager_SendRecv(currentTx_buffer, currentRx_buffer, 6);
		HAL_Delay(10);
	}


}
