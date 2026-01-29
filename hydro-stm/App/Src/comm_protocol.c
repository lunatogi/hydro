/*
 * comm_manager.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Murat Utku KETI
 */

#include "comm_protocol.h"
#include "comm_if.h"
#include "stm32f4xx_hal.h"

static SPI_HandleTypeDef *hspi;

void CommProtocol_Init(void *comm_ctx){		// It's currently SPI only
	hspi = (SPI_HandleTypeDef*)comm_ctx;
}

static CommStatus_t Send_Receive(const uint8_t *tx_buffer, uint8_t *rx_buffer, size_t buffer_size){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(hspi, tx_buffer, rx_buffer, buffer_size, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);

	if(st == HAL_OK) return COMM_OK;

	if(st == HAL_TIMEOUT) return COMM_TIMEOUT;

	return COMM_ERROR;
}

const CommInterface_t *Comm_GetInterface(void){
	static const CommInterface_t iface = {
			.send_recv = Send_Receive
	};
	return &iface;
}
