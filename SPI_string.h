/*
 * SPI_string.h
 *
 *  Created on: Sep 20, 2025
 *      Author: steff
 */

#ifndef SPI_STRING_H_
#define SPI_STRING_H_


void ST7789_DrawStringStatic(const char* string, const uint16_t text_color, const uint8_t x, const uint8_t y);
void ST7789_DrawCharStatic(const char c, const uint16_t text_color, const uint8_t x, const uint8_t y);



#endif /* SPI_STRING_H_ */
