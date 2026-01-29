/*
 * comm_if.h
 *
 *  Created on: Jan 28, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_COMM_IF_H_
#define INC_COMM_IF_H_

#include <stdint.h>

typedef enum {
    COMM_OK,
    COMM_TIMEOUT,
    COMM_ERROR
} CommStatus_t;

typedef struct{
	CommStatus_t (*send_recv)(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint8_t buffer_size);
} CommInterface_t;

#endif /* INC_COMM_IF_H_ */
