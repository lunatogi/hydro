/*
 * comm_manager.h
 *
 *  Created on: Jan 29, 2026
 *      Author: Murat Utku KETI
 */

#ifndef INC_COMM_MANAGER_H_
#define INC_COMM_MANAGER_H_

#include <string.h>
#include <stdint.h>
#include "math.h"
#include "common_types.h"


typedef struct __attribute__((packed))
{
	uint8_t switchMatrix;
	float values[SENSOR_COUNT];
	uint16_t crc;
}SystemSnapshot_t;

extern SystemSnapshot_t snapActive;

#define SPI_DATA_LENGTH sizeof(SystemSnapshot_t)

typedef union
{
    struct __attribute__((packed))
    {
        uint8_t id;
        uint8_t type;		// 0 -> Sensor Reading, 1 -> Referemce Values, 2 -> IncDec Matrix
        float   payload;
    } frame;

    uint8_t raw[SPI_DATA_LENGTH];       // Be carefull about this size
} SingleSPIData_t;			// Can be unused from now on

void Comm_UpdateSPISnapshot(void);
void Comm_PassRxBufferPtr(uint8_t *rxBuff);



void Comm_HandleSPIData(void);
void Comm_FillTxBuffer(uint8_t *txBuffer, uint8_t id, uint8_t type);

#endif /* INC_COMM_MANAGER_H_ */
