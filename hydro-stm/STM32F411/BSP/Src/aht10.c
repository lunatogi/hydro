/*
 * aht10.c
 *
 *  Created on: Apr 28, 2026
 *      Author: Murat Utku KETI
 */

#include "aht10.h"
#include <math.h>      /* NAN */
#include <stddef.h>    /* NULL */
/* ============================================================
 *  Private state — invisible outside this translation unit
 * ============================================================ */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t            initialized;
} AHT10_HandleTypeDef;

static AHT10_HandleTypeDef s_aht10 = { NULL, 0 };

/* ============================================================
 *  Private helpers
 * ============================================================ */
static AHT10_Status aht10_read_status_byte(uint8_t *status)
{
    if (HAL_I2C_Master_Receive(s_aht10.hi2c, AHT10_I2C_ADDR,
                               status, 1, AHT10_I2C_TIMEOUT_MS) != HAL_OK) {
        return AHT10_ERROR;
    }
    return AHT10_OK;
}

static AHT10_Status aht10_trigger_measurement(void)
{
    uint8_t cmd[3] = { AHT10_CMD_MEASURE, AHT10_MEASURE_PARAM1, AHT10_MEASURE_PARAM2 };
    if (HAL_I2C_Master_Transmit(s_aht10.hi2c, AHT10_I2C_ADDR,
                                cmd, sizeof(cmd), AHT10_I2C_TIMEOUT_MS) != HAL_OK) {
        return AHT10_ERROR;
    }
    return AHT10_OK;
}

static AHT10_Status aht10_read_measurement(float *temperature, float *humidity)
{
    uint8_t buf[6];
    if (HAL_I2C_Master_Receive(s_aht10.hi2c, AHT10_I2C_ADDR,
                               buf, sizeof(buf), AHT10_I2C_TIMEOUT_MS) != HAL_OK) {
        return AHT10_ERROR;
    }

    if (buf[0] & AHT10_STATUS_BUSY) {
        return AHT10_BUSY;
    }

    /* Bit layout (datasheet §5.4.5):
     *   buf[0]      : status
     *   buf[1..2]   : humidity high 16 bits
     *   buf[3] high : humidity low 4 bits   (upper nibble)
     *   buf[3] low  : temperature high 4 bits (lower nibble)
     *   buf[4..5]   : temperature low 16 bits
     */
    uint32_t raw_h =
          ((uint32_t)buf[1] << 12)
        | ((uint32_t)buf[2] << 4)
        | ((uint32_t)buf[3] >> 4);

    uint32_t raw_t =
          (((uint32_t)buf[3] & 0x0F) << 16)
        |  ((uint32_t)buf[4]        << 8)
        |   (uint32_t)buf[5];

    if (humidity != NULL) {
        *humidity = ((float)raw_h / AHT10_RAW_DIVISOR_F) * 100.0f;
    }
    if (temperature != NULL) {
        *temperature = ((float)raw_t / AHT10_RAW_DIVISOR_F) * 200.0f - 50.0f;
    }
    return AHT10_OK;
}


/* ============================================================
 *  Public API
 * ============================================================ */
AHT10_Status AHT10_Init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) {
        return AHT10_ERROR;
    }

    s_aht10.hi2c        = hi2c;
    s_aht10.initialized = 0;

    HAL_Delay(AHT10_POWERON_DELAY_MS);

    if (HAL_I2C_IsDeviceReady(hi2c, AHT10_I2C_ADDR, 3, AHT10_I2C_TIMEOUT_MS) != HAL_OK) {
        return AHT10_ERROR;
    }

    uint8_t cmd[3] = { AHT10_CMD_INIT, AHT10_INIT_PARAM1, AHT10_INIT_PARAM2 };
    if (HAL_I2C_Master_Transmit(hi2c, AHT10_I2C_ADDR, cmd, sizeof(cmd),
                                AHT10_I2C_TIMEOUT_MS) != HAL_OK) {
        return AHT10_ERROR;
    }

    HAL_Delay(AHT10_RESET_DELAY_MS);

    uint8_t status = 0;
    if (aht10_read_status_byte(&status) != AHT10_OK) {
        return AHT10_ERROR;
    }
    if (!(status & AHT10_STATUS_CALIBRATED)) {
        return AHT10_NOT_CAL;
    }

    s_aht10.initialized = 1;
    return AHT10_OK;
}


AHT10_Status AHT10_Read(float *temperature, float *humidity)
{
    if (!s_aht10.initialized) {
        return AHT10_NOT_INIT;
    }

    AHT10_Status s = aht10_trigger_measurement();
    if (s != AHT10_OK) return s;

    HAL_Delay(AHT10_MEASUREMENT_DELAY_MS);

    return aht10_read_measurement(temperature, humidity);
}


float AHT10_GetTemperature(void)
{
    float t;
    if (AHT10_Read(&t, NULL) != AHT10_OK) {
        return NAN;
    }
    return t;
}


float AHT10_GetHumidity(void)
{
    float h;
    if (AHT10_Read(NULL, &h) != AHT10_OK) {
        return NAN;
    }
    return h;
}


AHT10_Status AHT10_SoftReset(void)
{
    if (s_aht10.hi2c == NULL) {
        return AHT10_NOT_INIT;
    }

    uint8_t cmd = AHT10_CMD_SOFT_RESET;
    if (HAL_I2C_Master_Transmit(s_aht10.hi2c, AHT10_I2C_ADDR, &cmd, 1,
                                AHT10_I2C_TIMEOUT_MS) != HAL_OK) {
        return AHT10_ERROR;
    }
    HAL_Delay(AHT10_RESET_DELAY_MS);
    s_aht10.initialized = 0;   /* require re-init after reset */
    return AHT10_OK;
}
