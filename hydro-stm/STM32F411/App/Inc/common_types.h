/*
 * common_types.h
 *
 *  Created on: Jan 2, 2026
 *      Author: nasah
 */

#ifndef INC_COMMON_TYPES_H_
#define INC_COMMON_TYPES_H_

#include <stdint.h>

typedef uint8_t flag_t;

typedef enum
{
    IDX_TEMP = 0,
    IDX_ALT,
	IDX_FF,
	IDX_TDS,
	IDX_HUM,

    SENSOR_COUNT
} SensorIndex_t;

#endif /* INC_COMMON_TYPES_H_ */
