/*
 * config_store.c
 *
 *  Created on: Dec 19, 2025
 *      Author: Murat Utku KETI
 */

#include "config_store.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#define CONFIG_FLASH_SECTOR FLASH_SECTOR_7
#define CONFIG_FLASH_ADDR   0x08060000U

/* =======================
 *  CRC32 (simple software)
 * ======================= */
static uint32_t Config_CalcCRC(const SystemConfig_t *cfg)
{
    const uint32_t *data = (const uint32_t *)cfg;
    uint32_t words = (sizeof(SystemConfig_t) - sizeof(uint32_t)) / 4;

    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < words; i++)
    {
        crc ^= data[i];
        for (uint32_t b = 0; b < 32; b++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return crc;
}

/* =======================
 *  Validation
 * ======================= */
int Config_IsValid(const SystemConfig_t *cfg)
{
    if (cfg->version != CONFIG_VERSION)
        return 0;

    uint32_t expected = Config_CalcCRC(cfg);
    return (expected == cfg->crc);
}

/* =======================
 *  Load from Flash
 * ======================= */
void Config_Load(SystemConfig_t *cfg)
{
    memcpy(cfg, (const void *)CONFIG_FLASH_ADDR, sizeof(SystemConfig_t));
}

/* =======================
 *  Save to Flash
 * ======================= */
void Config_Save(const SystemConfig_t *cfg)
{
    SystemConfig_t tmp = *cfg;
    tmp.crc = Config_CalcCRC(&tmp);

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase =
    {
        .TypeErase    = FLASH_TYPEERASE_SECTORS,
        .Sector       = CONFIG_FLASH_SECTOR,
        .NbSectors    = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };

    uint32_t sectorError;
    HAL_FLASHEx_Erase(&erase, &sectorError);

    uint32_t addr = CONFIG_FLASH_ADDR;
    const uint32_t *data = (const uint32_t *)&tmp;

    for (uint32_t i = 0; i < sizeof(SystemConfig_t) / 4; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data[i]);
        addr += 4;
    }

    HAL_FLASH_Lock();
}
