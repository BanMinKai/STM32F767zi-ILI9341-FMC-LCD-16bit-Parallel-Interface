#ifndef ILI9341_KHAI_H
#define ILI9341_KHAI_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h> //stdio + stdarg needed for vsprintf to make lcd_printf

#include "stm32f7xx_hal.h"
#include "ili9341_registers.h"

#include "colors.h"
#include "fonts.h"
#include "image.h"

// LCD PROPERTIES ---------------------------------------------------------------------------------------------
// This struct is used to indicate the capabilities of different LCDs
// PIN PB1 --> LCD_BL
// PIN PB8 --> LCD_RST

#define LCD_ILI9341_WIDTH 320
#define LCD_ILI9341_HEIGHT 240

// FMC PROPERTIES -----------------------------------------------------------------------------------------------
// HCLK    --> 168 MHz
// ADDSET in HCLK --> 1
// DATASET in HCLK --> 5
// Bus Turnaround Time --> 0
#define FMC_NEx 1
#define FMC_Ax 18
// FMC cmd base address & data base address
#define FMC_BASE_ADDR        (0x60000000 + ((FMC_NEx - 1) << 26)) // favors NE1 so that the base is 0x60000000
#define FMC_CMD_BASE_ADDR    (FMC_BASE_ADDR)
#define FMC_DATA_BASE_ADDR   (FMC_CMD_BASE_ADDR | (1 << (FMC_Ax + 1))) // 1 << (FMC_Ax + 1) == 1*2^(FMC_Ax+1)



// DISPLAY ORIENTATION ------------------------------------------------------------------------------------------
typedef enum
{
	LCD_ORIENTATION_PORTRAIT 			= 0,
	LCD_ORIENTATION_LANDSCAPE 			= 1,
	LCD_ORIENTATION_PORTRAIT_MIRROR 	= 2,
	LCD_ORIENTATION_LANDSCAPE_MIRROR 	= 3
} LCD_OrientationTypeDef;

static uint16_t LCD_Build_MemoryAccessControl_Config(
                                bool rowAddressOrder,
                                bool columnAddressOrder,
                                bool rowColumnExchange,
                                bool verticalRefreshOrder,
                                bool colorOrder,
                                bool horizontalRefreshOrder);



// FONTS -------------------------------------------------------------------------------------------------------

typedef struct
{
	uint32_t 	TextColor;
	uint32_t 	BackColor;
	sFONT*    	pFont;
	uint8_t		TextWrap; // i dont understand the purpose of textwrap, just keep it at 1
}lcdFontPropTypeDef;

typedef struct
{
	unsigned short	x;
	unsigned short	y;
}lcdCursorPosTypeDef;

// ATOMIC FUNCTIONS VIA DEFINITIONS ---------------------------------------------------------------------------------
// write cmd to LCD via FMC <=> write to internal SRAM memory address FMC_CMD_BASE_ADDR
// write data to LCD via FMC <=> write to internal SRAM memory address FMC_DATA_BASE_ADDRESS
// cmd in this case means memory addresses of ILI9341 registers
// you cast a hex number into a uint16_t pointer, indicating it to be a memory location
// you access that memory location using dereference * of pointer
// data in this case means what to be written to ILI9341 GRAM
#define writeCmd16(cmd)  (*((volatile uint16_t *) FMC_CMD_BASE_ADDR) = cmd)
#define writeData16(data) (*((volatile uint16_t*) FMC_DATA_BASE_ADDR) = data)
#define readData16() (*((volatile uint16_t*) FMC_DATA_BASE_ADDR))
#define swap(a, b) { int16_t t = a; a = b; b = t; }



// API FUNCTIONS ----------------------------------------------------------------------------------------------------
void LCD_WriteData16(uint16_t data);
void LCD_WriteCmd16(uint16_t cmd);
uint16_t LCD_ReadData16(void);

void STM32F767ZI_MPU_Config(void);
void LCD_Init(void);

void LCD_Draw_SinglePixel(uint16_t xPosition, uint16_t yPosition, uint16_t color);
void LCD_Draw_HorLine(uint16_t xStart, uint16_t xEnd, uint16_t yPosition, uint16_t color);
void LCD_Draw_Rect(uint16_t xStart, uint16_t yStart, uint16_t rect_width, uint16_t rect_height, uint16_t color);
void LCD_Draw_MultiplePixels(uint16_t xPos, uint16_t yPos, uint16_t *data, uint32_t dataLength);
void LCD_Draw_Char(int16_t x, int16_t y, unsigned char c);
void LCD_Draw_SingleImage(uint16_t x, uint16_t y, GUI_CONST_STORAGE GUI_BITMAP* pBitmap);
void LCD_Printf(const char *format, ...);

void LCD_Set_WriteWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void LCD_Set_ReadWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void LCD_Set_Orientation(LCD_OrientationTypeDef LCD_ORIENTATION_TYPE);
void LCD_Set_Cursor(uint16_t xPos, uint16_t yPos);
void LCD_Set_TextColor(uint16_t text_color, uint16_t background_color);
void LCD_Set_TextWrap(uint8_t w);



#endif




