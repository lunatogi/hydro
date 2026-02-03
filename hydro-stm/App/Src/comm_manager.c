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

void CommManager_Init(const CommInterface_t *comm){
	comm_if = comm;
}

void CommManager_SendRecv(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t BUFFER_SIZE){
	comm_if->send_recv(tx_buffer, rx_buffer, BUFFER_SIZE);
}

void Comm_SendCurrentValues(void){

	spi2tx_buffer[0] = SENSOR_COUNT;
	CommManager_SendRecv(spi2tx_buffer, spi2rx_buffer, 1);

	for(SensorIndex_t i = 0; i < SENSOR_COUNT; i++){
		spi2tx_buffer[0] = i;
		spi2tx_buffer[1] = 0;
		float value = Sensor_GetValue(i);
		memcpy(&spi2tx_buffer[2], &value, sizeof(float));
		CommManager_SendRecv(spi2tx_buffer, spi2rx_buffer, 6);
		HAL_Delay(10);
	}


}
