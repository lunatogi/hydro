/*
 * comm_manager.c
 *
 *  Created on: Jan 29, 2026
 *      Author: Murat Utku KETI
 */

#include "comm_manager.h"

static const CommInterface_t *comm_if;

uint8_t BUFFER_SIZE = 4;
uint8_t spi2tx_buffer[4] = {0xAA, 0xAA, 0xAA, 0xAA};
uint8_t spi2rx_buffer[4] = {0};

void CommManager_Init(const CommInterface_t *comm){
	comm_if = comm;
}

void CommManager_SendRecv(){
	comm_if->send_recv(spi2tx_buffer, spi2rx_buffer, BUFFER_SIZE);
}
