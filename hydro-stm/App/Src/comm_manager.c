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
uint8_t spi2tx_buffer[6] = {0xAA, 0xAA, 0xAA, 0xAA};
uint8_t spi2rx_buffer[6] = {0};
float payloadd = 0;

void CommManager_Init(const CommInterface_t *comm){
	comm_if = comm;
}

void CommManager_SendRecv(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t BUFFER_SIZE){
	comm_if->send_recv(tx_buffer, rx_buffer, BUFFER_SIZE);
}

const void Comm_ClearTxBuffer(){		// UNSUED CURRENTLY
	for(int i = 0; i < sizeof(spi2tx_buffer); i++){
		spi2tx_buffer[i] = 0;
	}
}

void Comm_SendCurrentValues(void){

	SingleSPIData_t spiData;

	spi2tx_buffer[0] = SENSOR_COUNT;
	CommManager_SendRecv(spi2tx_buffer, spi2rx_buffer, 1);		// THIS FUNCTION HAS A RETURN VALUE, NOT HANDLING IT CAN CAUSE PROBLEM!! WARNING

	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		spi2tx_buffer[0] = i;
		spi2tx_buffer[1] = 0;
		float value = Sensor_GetValue(i);
		memcpy(&spi2tx_buffer[2], &value, sizeof(float));
		CommManager_SendRecv(spi2tx_buffer, spi2rx_buffer, 6);
		memcpy(spiData.raw, spi2rx_buffer, 6);
		//spiData.raw = spi2rx_buffer;
		if(spiData.frame.payload != 0 && spiData.frame.id == 0) payloadd = spiData.frame.payload;
		//HAL_Delay(10);
	}
}
