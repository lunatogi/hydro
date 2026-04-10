/*
 * config_store.h
 *
 *  Created on: Dec 19, 2025
 *      Author: Murat Utku KETI
 */

#ifndef INC_CONFIG_STORE_H_
#define INC_CONFIG_STORE_H_

#include <stdint.h>
#include "sensor_manager.h"

/* =======================
 *  Configuration version
 * ======================= */
#define CONFIG_VERSION  1

/* =======================
 *  Per-sensor config
 * ======================= */
typedef struct
{
    float ref;
    float margin;
} SensorStoreConfig_t;

/* =======================
 *  System configuration
 * ======================= */
typedef struct
{
    uint32_t version;
    SensorStoreConfig_t sensor[SENSOR_COUNT];
    uint32_t crc;     // CRC32 of everything above
} SystemConfig_t;

/* =======================
 *  Public API
 * ======================= */
void Config_Load(SystemConfig_t *cfg);
void Config_Save(const SystemConfig_t *cfg);
int  Config_IsValid(const SystemConfig_t *cfg);

/* =======================
 *  Defaults
 * ======================= */
extern const SystemConfig_t defaultConfig;


#endif /* INC_CONFIG_STORE_H_ */
