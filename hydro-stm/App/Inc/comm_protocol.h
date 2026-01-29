/*
 * comm_manager.h
 *
 *  Created on: Jan 28, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_COMM_MANAGER_H_
#define INC_COMM_MANAGER_H_

#include "comm_if.h"

void CommProtocol_Init(void *comm_ctx); // It's currently SPI only

const CommInterface_t *Comm_GetInterface(void);

void CommManager_SendRecv();

#endif /* INC_COMM_MANAGER_H_ */
