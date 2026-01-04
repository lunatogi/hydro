/*
 * actuator.c
 *
 *  Created on: Dec 18, 2025
 *      Author: Murat Utku KETI
 */

#include "actuator_manager.h"
#include "stm32f4xx_hal.h"

static GPIO_TypeDef* Port_From_ID(uint8_t id)
{
    switch (id)
    {
        case 0: return GPIOA;
        case 1: return GPIOB;
        case 2: return GPIOC;
        default: return NULL;
    }
}

static inline uint16_t PinMask_From_Number(uint8_t pin)
{
    if (pin > 15)
        return 0;

    return (uint16_t)(1U << pin);
}

flag_t Set_Pin(uint8_t portId, uint8_t pinNum, flag_t state)
{
    GPIO_TypeDef *port = Port_From_ID(portId);
    uint16_t pinMask   = PinMask_From_Number(pinNum);

    if (port == NULL || pinMask == 0)
        return 0;

    HAL_GPIO_WritePin(
        port,
		pinMask,
        state ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    return 1;
}






