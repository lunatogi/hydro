/*
 * scheduler.h
 *
 *  Created on: Dec 16, 2025
 *      Author: Murat Utku KETI
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdio.h>
#include "common_types.h"

extern flag_t SPI_Done_Flag;

void Scheduler_Init(void);
void Scheduler_Run(void);

#endif /* INC_SCHEDULER_H_ */
