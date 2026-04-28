/*

	This library is written by Faruk Demirtaş 26.08.2024
	YT: https://www.youtube.com/@farukdemirta
*/
#ifndef LCD_H_
#define LCD_H_

#include <stdio.h>
#include "stm32f4xx_hal.h"

static const char *LABELS[] = {
    "Temp",    // 0
    "Alt",     // 1
	"FF",	   // 2
	"TDS"	   // 3
};

// Define the pin connections
#define RS_Pin GPIO_PIN_9
#define RS_GPIO_Port GPIOA

#define EN_Pin GPIO_PIN_7
#define EN_GPIO_Port GPIOC

#define D4_Pin GPIO_PIN_8
#define D4_GPIO_Port GPIOA

#define D5_Pin GPIO_PIN_10
#define D5_GPIO_Port GPIOA

#define D6_Pin GPIO_PIN_4
#define D6_GPIO_Port GPIOB

#define D7_Pin GPIO_PIN_5
#define D7_GPIO_Port GPIOB

// Function declarations
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_SendString(char* str);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Cycle(void);

#endif /* LCD_H_ */
