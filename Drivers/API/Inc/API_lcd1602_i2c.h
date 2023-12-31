/*
 * API_lcd1602_i2c.h
 *
 *  Created on: Aug 9, 2023
 *      Author: nico
 */

#ifndef API_INC_API_LCD1602_I2C_H_
#define API_INC_API_LCD1602_I2C_H_

#include <stdint.h>

/**
 * Initialize the I2C interface using the HAL driver, then it initialize the
 * LCD1602_I2C.
 */
void LCD1602_Init(void);

/**
 * Add a string to be displayed on the screen. How is printed is managed by the
 * printer algorithm. Read more on LCD1602_PrintMode function.
 */
void LCD1602_AddToBuffer(const char *pString);

/**
 * Algorithm to print a string data on the display. currently exists two
 * algorithms:
 *
 * 1. NO_ANIMATION: Which only prints the latests inserted text on the display
 *    until new data arrive.
 *
 * 2. SLIDE: The text is placed on the right of the screen and is moved to the left
 *    on constant interval times. When more data is added to the buffer, is placed
 *    on the right and it will be eventually printed.
 */
void LCD1602_PrintMode(uint8_t mode);

/**
 * Updates the display if necessary. All the modes first check if the data
 * changed. If that is the case the display will be updated, if not this function
 * does nothing.
 */
void LCD1602_FSM_UpdateDisplay(void);

/**
 * Cicly between the print algorithms. This action removes all previous data and
 * print on the screen the current algorithm.
 */
void LCD1602_FSM_NextAlgorithm(void);

#endif /* API_INC_API_LCD1602_I2C_H_ */
