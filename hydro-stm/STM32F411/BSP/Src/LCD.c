/*

	This library is written by Faruk Demirtaş 26.08.2024
	YT: https://www.youtube.com/@farukdemirta
*/

#include "LCD.h"
#include "sensor_manager.h"
#include "common_types.h"

SensorIndex_t idx_counter = 0;

// Private function declarations
void LCD_SendNibble(uint8_t nibble);
void LCD_Enable(void);

// Initializes the LCD
void LCD_Init(void) {
    // Configure GPIO pins
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(40); // Wait for LCD to power up

    // Initialize in 4-bit mode
    LCD_SendCommand(0x33);
    LCD_SendCommand(0x32);

    // Function set: 4-bit, 2 line, 5x8 dots
    LCD_SendCommand(0x28);

    // Display on, cursor off, blink off
    LCD_SendCommand(0x0C);

    // Clear display
    LCD_Clear();

    // Entry mode set: increment cursor, no shift
    LCD_SendCommand(0x06);
}

// Clears the display
void LCD_Clear(void) {
    LCD_SendCommand(0x01); // Clear display command
    HAL_Delay(2);          // Delay to allow clear command to process
}

// Set cursor to the specified position
void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address;

    if (row == 0) {
        address = 0x80 + col;
    } else {
        address = 0xC0 + col;
    }

    LCD_SendCommand(address);
}

// Send a string to the LCD
void LCD_SendString(char* str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

// Send a command to the LCD
void LCD_SendCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET); // Command mode
    LCD_SendNibble(cmd >> 4);                                // Send high nibble
    LCD_SendNibble(cmd & 0x0F);                              // Send low nibble
}

// Send data to the LCD
void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET);   // Data mode
    LCD_SendNibble(data >> 4);                               // Send high nibble
    LCD_SendNibble(data & 0x0F);                             // Send low nibble
}

// Send a 4-bit nibble to the LCD
void LCD_SendNibble(uint8_t nibble) {
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (nibble >> 0) & 0x01);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (nibble >> 1) & 0x01);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (nibble >> 2) & 0x01);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (nibble >> 3) & 0x01);

    LCD_Enable();
}

// Enable pulse to latch the data/command
void LCD_Enable(void) {
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);  // Short delay for enable pulse
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);  // Short delay to complete
}

void LCD_WriteOneLine(uint8_t idx){
	char msg[32];
	float value = Sensor_GetValue(idx);
	snprintf(msg, sizeof(msg), "%s: %.2f", LABELS[idx], value);

	uint8_t cursor = idx % 2;
    LCD_SetCursor(cursor, 0);
	LCD_SendString(msg);
}

void LCD_Cycle(void){
	LCD_Clear();
	LCD_WriteOneLine(idx_counter);
	idx_counter++;
	if(idx_counter >= SENSOR_COUNT){
		idx_counter = 0;
		return;
	}
	LCD_WriteOneLine(idx_counter);
	idx_counter++;
	if(idx_counter >= SENSOR_COUNT){
		idx_counter = 0;
		return;
	}
}
