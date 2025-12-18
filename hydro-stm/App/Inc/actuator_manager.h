/*
 * actuator.h
 *
 *  Created on: Dec 18, 2025
 *      Author: Murat Utku KETI
 */

#ifndef INC_ACTUATOR_MANAGER_H_
#define INC_ACTUATOR_MANAGER_H_

#include <stdint.h>
#include <stm32f4xx_hal.h>

typedef uint8_t flag_t;

flag_t Actuator_Set(uint8_t inPort, uint8_t inPin, flag_t state);

#endif /* INC_ACTUATOR_MANAGER_H_ */
