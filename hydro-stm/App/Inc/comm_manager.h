/*
 * comm_manager.h
 *
 *  Created on: Jan 29, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_COMM_MANAGER_H_
#define INC_COMM_MANAGER_H_

#include <string.h>
#include "comm_if.h"

void CommManager_Init(const CommInterface_t *comm);
void CommManager_SendRecv(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t BUFFER_SIZE);
void Comm_SendCurrentValues(void);

#endif /* INC_COMM_MANAGER_H_ */
