
/*
 *
 *
 * KHAI PHAN, based on IWATAKE's & a UKRAINIAN guy libraries
 *
 *
 *
 * */

#include "ili9341_khai.h"

// DISPLAY ORIENTATION ---------------------------------------------------------------------------------------------
enum {
  MemoryAccessControlNormalOrder,
  MemoryAccessControlReverseOrder
} MemoryAccessControlRefreshOrder;

enum {
	MemoryAccessControlColorOrderRGB,
	MemoryAccessControlColorOrderBGR
} MemoryAccessControlColorOrder;

static uint16_t LCD_Build_MemoryAccessControl_Config(
                        bool rowAddressOrder,
                        bool columnAddressOrder,
                        bool rowColumnExchange,
                        bool verticalRefreshOrder,
                        bool colorOrder,
                        bool horizontalRefreshOrder)
{
  uint16_t value = 0;

  if(horizontalRefreshOrder)
	  value |= ILI9341_MADCTL_MH;
  if(colorOrder)
	  value |= ILI9341_MADCTL_BGR;
  if(verticalRefreshOrder)
	  value |= ILI9341_MADCTL_ML;
  if(rowColumnExchange)
	  value |= ILI9341_MADCTL_MV;
  if(columnAddressOrder)
	  value |= ILI9341_MADCTL_MX;
  if(rowAddressOrder)
	  value |= ILI9341_MADCTL_MY;

  return value;
};

static uint16_t LCD_PortraitConfig         = 0; // expected value = 0x48
static uint16_t LCD_LandscapeConfig        = 0; // expected value = 0x28
static uint16_t LCD_PortraitMirrorConfig   = 0; // 0x88
static uint16_t LCD_LandscapeMirrorConfig  = 0; // 0xE8



// FONTS ------------------------------------------
// by default YELLOW ON BLACK, font 12
// the purpose of two struct instances here
// is to provide default settings for txt color, txt background color, font, cursor x and cursor y
// in case user forget to set it
// OOP approach, the overall code is clean and easy to follow
static lcdFontPropTypeDef lcdFont = {
		.TextColor = COLOR_RED,    \
		.BackColor = COLOR_BLACK,  \
		.pFont     = &Font12,      \
		.TextWrap  = 1
};
static lcdCursorPosTypeDef cursorXY = {0, 0};



// API FUNCTION DEFINITIONS-------------------------------------------------------------------

/*
 * BASE ADDRESS    = 0x6000.0000
 * MPU REGION SIZE = 256MB
 * NOT CACHABLE
 * */
void STM32F767ZI_MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1; // may change to region0
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void LCD_WriteData16(uint16_t data)
{
    writeData16(data);
}

void LCD_WriteCmd16(uint16_t cmd)
{
    writeCmd16(cmd);
}

uint16_t LCD_ReadData16(void)
{
    return readData16();
}

void LCD_Init(void)
{
    // STM32F7 series have L1 cache, this impacts how FMC works compared to STM32F4
    // in order to work, MPU unit must be configged
    // it took me a few days to search for the solution on STM32 community
    // when portinng people's ILI9341 libraries for F4 using FMC
	// if you use STM32F4 then comment away this line because F4 does not have problem with MPU
    STM32F767ZI_MPU_Config();

    // Build Orientation Config
    // the reason why the following four lines of configs have to reside in a function
    // because the variables are static and you cannot assign them to returns of functions outside
    LCD_PortraitConfig = LCD_Build_MemoryAccessControl_Config(
                                                        MemoryAccessControlNormalOrder,		// rowAddressOrder
                                                        MemoryAccessControlReverseOrder,	// columnAddressOrder
                                                        MemoryAccessControlNormalOrder,		// rowColumnExchange
                                                        MemoryAccessControlNormalOrder,		// verticalRefreshOrder
                                                        MemoryAccessControlColorOrderBGR,	// colorOrder
                                                        MemoryAccessControlNormalOrder);	// horizontalRefreshOrder
    LCD_LandscapeConfig = LCD_Build_MemoryAccessControl_Config(
                                                        MemoryAccessControlNormalOrder,		// rowAddressOrder
                                                        MemoryAccessControlNormalOrder,		// columnAddressOrder
                                                        MemoryAccessControlReverseOrder,	// rowColumnExchange
                                                        MemoryAccessControlNormalOrder,		// verticalRefreshOrder
                                                        MemoryAccessControlColorOrderBGR,	// colorOrder
                                                        MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

    LCD_PortraitMirrorConfig = LCD_Build_MemoryAccessControl_Config(
    		  	  	  	  	  	  	  	  	  	  	  	MemoryAccessControlReverseOrder,	// rowAddressOrder
    		                                            MemoryAccessControlNormalOrder,		// columnAddressOrder
    		                                            MemoryAccessControlNormalOrder,		// rowColumnExchange
    		                                            MemoryAccessControlNormalOrder,		// verticalRefreshOrder
    		                                            MemoryAccessControlColorOrderBGR,	// colorOrder
    		                                            MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

    LCD_LandscapeMirrorConfig = LCD_Build_MemoryAccessControl_Config(
                                                        MemoryAccessControlReverseOrder,	// rowAddressOrder
                                                        MemoryAccessControlReverseOrder,	// columnAddressOrder
                                                        MemoryAccessControlReverseOrder,	// rowColumnExchange
                                                        MemoryAccessControlNormalOrder,		// verticalRefreshOrder
                                                        MemoryAccessControlColorOrderBGR,	// colorOrder
                                                        MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

    // SOFTWARE RESET
    // software reset, if not you have to pull the RST pin manually HIGH->LOW->HIGH (hardware reset), 
    // the RST must be continously held HIGH

    writeCmd16(0x01); 
    HAL_Delay(50);

    // exit sleep, after software reset, automatically enters sleep mode, thus have to wake ili9341 up
    writeCmd16(0x11);
    HAL_Delay(50);

    // DISPLAY FUNCTION CONTROl
    writeCmd16(ILI9341_DISPLAYFUNC); //0xB6
    writeData16(0x0A); // by default
    writeData16(0xC2); // following Iwatake 
    writeData16(0x27); // by default
    writeData16(0x00); // by default

    // MEMORY ACCESS CONTROL
    // this register dictates the ORIENTATION of the display screen 
    // Iwatake used 0x68, the default is landscape
    // if you change the screen orientation, you need to switch width and height blablabla
    // otherwise you would not be able to use full screen normally
    // in this library, that functionality is not done yet so jsut stick with Landscape
    writeCmd16(ILI9341_MEMCONTROL); // 0x36
    writeData16(0x68);

    // COLMO: PIXEL FORMAT
    writeCmd16(ILI9341_PIXELFORMAT); // 0x3A
    writeData16(0x55); // RGB565 (16bit)

    // POSITIVE GAMMA CORRECTION ~ positive_GAMMA
    // gamma correction ehances the dark-light contrast, the lower gamma value, the better, the more discernable
    // I followed Iwatake's configs
    writeCmd16(ILI9341_POSITIVEGAMMCORR); // 0xE0
    writeData16(0x10);
    writeData16(0x10);
    writeData16(0x10);
    writeData16(0x08);
    writeData16(0x0E);
    writeData16(0x06);
    writeData16(0x42);
    writeData16(0x28);
    writeData16(0x36);
    writeData16(0x03);
    writeData16(0x0E);
    writeData16(0x04);
    writeData16(0x13);
    writeData16(0x0E);
    writeData16(0x0C);

    // NEGATIVE GAMMA CORRECTION ~ negative_GAMMA // 0xE1
    writeCmd16(ILI9341_NEGATIVEGAMMCORR);
    writeData16(0x0C);
    writeData16(0x23);
    writeData16(0x26);
    writeData16(0x04);
    writeData16(0x0C);
    writeData16(0x04);
    writeData16(0x39);
    writeData16(0x24);
    writeData16(0x4B);
    writeData16(0x03);
    writeData16(0x0B);
    writeData16(0x0B);
    writeData16(0x33);
    writeData16(0x37);
    writeData16(0x0F);  

    // COLUMN ADDRESS SET
    writeCmd16(ILI9341_COLADDRSET); // 0x2a
    writeData16(0x00);
    writeData16(0x00); // 0x0000 by default
    writeData16(0x00);
    writeData16(0xef); // 0x00EF by default

    // PAGE ADDRESS SET
    writeCmd16(ILI9341_PAGEADDRSET); // 0x2b
    writeData16(0x00);
    writeData16(0x00); // 0x0000 by default
    writeData16(0x01);
    writeData16(0x3f); // 0x00EF by default

    // DISPLAY ON
    // the Frame Memory is output to the display screen
    writeCmd16(ILI9341_DISPLAYON); // 0x29
    HAL_Delay(10); // the datasheet does not tell so but Iwatake puts a 10 ms delay here
    
    // MEMORY WRITE 
    // the last line for any ILI9341 init function
    writeCmd16(ILI9341_MEMORYWRITE);

    // TEST
    LCD_Draw_Rect(0, 0, 175,175, COLOR_BLUE);

    for (uint8_t i = 0; i<10; i++)
    {
  	  LCD_Draw_HorLine(50,300,i*20,COLOR_YELLOW);
    }

    LCD_Set_WriteWindow(0, 0, LCD_ILI9341_WIDTH - 1, LCD_ILI9341_HEIGHT - 1);

    HAL_Delay(100);
    LCD_Draw_Char(0, 0, 'K');
    LCD_Draw_Char(50, 0, 'K');
    LCD_Draw_Char(100, 0, 'K');
    LCD_Draw_Char(150, 0, 'K');

    LCD_Set_Cursor(10, 100);
    LCD_Printf("HELLO WORLD!\n");
    LCD_Printf("ENTER");

//    LCD_Draw_SingleImage(0, 0, &bmSTLogo);
}

/*
 *
 * my x-y assumes Iwatake's landscape orientation (data 0x68 written to MEMCONTROL register 0x36)
 *
 * */
void LCD_Draw_SinglePixel(uint16_t xPosition, uint16_t yPosition, uint16_t color)
{
    if ((xPosition < 0) || (yPosition < 0) || (xPosition >= LCD_ILI9341_WIDTH) || (yPosition >= LCD_ILI9341_HEIGHT))
        return;

    // according to the ILI9341 datasheet, if you want to display anything
    // first config a memory access region (a drawing window, a drawing area whatsoever)
    // second, write data to that region
    // the lcd IC (ILI9341) takes care of fitting data into that region!
    // all LCD_Draw_??? functions follow this procedure
    LCD_Set_WriteWindow(xPosition, yPosition, xPosition, yPosition);
    writeData16(color);
}

void LCD_Draw_HorLine(uint16_t xStart, uint16_t xEnd, uint16_t yPosition, uint16_t color)
{
    if (xStart > xEnd)
        swap(xStart, xEnd);

	if (xEnd >= LCD_ILI9341_WIDTH)
	{
		xEnd = LCD_ILI9341_WIDTH - 1;
	}

    LCD_Set_WriteWindow(xStart,yPosition,xEnd,yPosition);
    for (uint16_t x = xStart; x < xEnd; x++)
    		writeData16(color);

}

void LCD_Draw_Rect(uint16_t xStart, uint16_t yStart, uint16_t rect_width, uint16_t rect_height, uint16_t color)
{
  LCD_Set_WriteWindow(xStart, yStart, xStart + rect_width - 1, yStart + rect_height - 1);
  for( uint16_t y = 0; y < rect_height; y++ ){
    for( uint16_t x = 0; x < rect_width; x++ ){
      writeData16(color);
    }
    HAL_Delay(5); // I put a delay for fun here, to see a gradual shaping of a rectangle
  }
}

/**
 * \brief Draws a ASCII character at the specified coordinates
 *
 * \param x			The x-coordinate
 * \param y			The y-coordinate
 * \param c			Character
 *
 * \return void
 */
void LCD_Draw_Char(int16_t x, int16_t y, unsigned char c)
{
	if ((x >= LCD_ILI9341_WIDTH) || 			// Clip right
			(y >= LCD_ILI9341_HEIGHT) || 		// Clip bottom
			((x + lcdFont.pFont->Width) < 0) || // Clip left
			((y + lcdFont.pFont->Height) < 0))  // Clip top
		return;

	uint8_t fontCoeff = lcdFont.pFont->Height / 8;
	uint8_t xP = 0;

	for(uint8_t i = 0; i < lcdFont.pFont->Height; i++)
	{
		uint8_t line;

		for(uint8_t k = 0; k < fontCoeff; k++)
		{
			line = lcdFont.pFont->table[((c - 0x20) * lcdFont.pFont->Height * fontCoeff) + (i * fontCoeff) + k];

			for(uint8_t j = 0; j < 8; j++)
			{
				if((line & 0x80) == 0x80)
				{
					LCD_Draw_SinglePixel(x + j + xP, y + i, lcdFont.TextColor);
				}
				else if (lcdFont.BackColor != lcdFont.TextColor)
				{
					LCD_Draw_SinglePixel(x + j + xP, y + i, lcdFont.BackColor);
				}
				line <<= 1;
			}

			xP += 8;
		}

		xP = 0;
	}
}

void LCD_Draw_MultiplePixels(uint16_t xPos, uint16_t yPos, uint16_t *data, uint32_t dataLength)
{
  uint32_t i = 0;

  LCD_Set_WriteWindow(xPos, yPos, LCD_ILI9341_WIDTH - 1, LCD_ILI9341_HEIGHT - 1);

  do
  {
    LCD_WriteData16(data[i++]);
  }
  while (i < dataLength);
}

// NOTE: if the size of your image is 240x240, then x can be in [0,80] but y has to be 0,
// otherwise no image displayed
// in other words, the size (in pixels) of your image must be =< 240 for height and =< 320 for width
// because the default orientation is landscape (value of 0x68 to MEMCONTROL register of ILI9341)
void LCD_Draw_SingleImage(uint16_t x, uint16_t y, GUI_CONST_STORAGE GUI_BITMAP* pBitmap)
{
	if((x >= LCD_ILI9341_WIDTH) || (y >= LCD_ILI9341_HEIGHT)) return;
	if((x + pBitmap->xSize - 1) >= LCD_ILI9341_WIDTH) return;
	if((y + pBitmap->ySize - 1) >= LCD_ILI9341_HEIGHT) return;

	for (int i = 0; i < pBitmap->ySize; ++i)
	{
		LCD_Draw_MultiplePixels(x, y + i, (uint16_t*)(pBitmap->pData + i * pBitmap->bytesPerLine), pBitmap->bytesPerLine / (pBitmap->bitsPerPixel / 8));
	}
}

// inlucde <stdio.h> and <stdarg.h>
void LCD_Printf(const char *format, ...)
{
	/// this vsprintf is like the kernel of printf, to be able to generate [ "%d %l", a,b ] ability of printf
	/// we copy the following 6 lines (to va_end(lst)) as a magic spell:>
	static char buf[256];
	char *p;
	va_list lst;

	va_start(lst, format);
	vsprintf(buf, format, lst);
	va_end(lst); // after this, the formatted argument is stored in the buffer
	// now we can display the buffer whatsoever using LCD API functions


	// if we did LCD_Printf("Hello World %d", 16)
	// buf would be now storing 'Hello World 16\0'
	p = buf;
	while (*p) // loops until \0
	{
		if (*p == '\n')
		{
			cursorXY.y += lcdFont.pFont->Height;
			cursorXY.x = 0;
		}
		else if (*p == '\r')
		{
			// skip em
		}
		else if (*p == '\t')
		{
			cursorXY.x += lcdFont.pFont->Width * 4;
		}
		else
		{
			LCD_Draw_Char(cursorXY.x, cursorXY.y, *p);
			cursorXY.x += lcdFont.pFont->Width; // after drawing 1 char, do this to move the cursor to the next char

			// check if the cursor.x is > the LCD width, if so, enters a newline
			if (lcdFont.TextWrap && (cursorXY.x > (LCD_ILI9341_WIDTH - lcdFont.pFont->Width)))
			{
				cursorXY.y += lcdFont.pFont->Height;
				cursorXY.x = 0;
			}
		}
		p++;

		if (cursorXY.y >= LCD_ILI9341_HEIGHT)
		{
			cursorXY.y = 0;
		}
	}
}

void LCD_Set_WriteWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    // COLUMN ADDRESS SET
    writeCmd16(ILI9341_COLADDRSET); // 0x2a
    writeData16(xStart >> 8);
    writeData16(xStart & 0xff); // 0x0000 by default
    writeData16(xEnd >> 8);
    writeData16(xEnd & 0xff); // 0x00EF by default

    // PAGE ADDRESS SET
    writeCmd16(ILI9341_PAGEADDRSET); // 0x2b
    writeData16(yStart >> 8);
    writeData16(yStart & 0xff); // 0x0000 by default
    writeData16(yEnd >> 8);
    writeData16(yEnd & 0xff); // 0x00EF by default

    // MEMORY WRITE
    writeCmd16(ILI9341_MEMORYWRITE);
}

void LCD_Set_ReadWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    // COLUMN ADDRESS SET
    writeCmd16(ILI9341_COLADDRSET); // 0x2a
    writeData16(xStart >> 8);
    writeData16(xStart & 0xff); // 0x0000 by default
    writeData16(xEnd >> 8);
    writeData16(xEnd & 0xff); // 0x00EF by default

    // PAGE ADDRESS SET
    writeCmd16(ILI9341_PAGEADDRSET); // 0x2b
    writeData16(yStart >> 8);
    writeData16(yStart & 0xff); // 0x0000 by default
    writeData16(yEnd >> 8);
    writeData16(yEnd & 0xff); // 0x00EF by default

    // MEMORY READ
    writeCmd16(ILI9341_MEMORYREAD);
}

void LCD_Set_Orientation(LCD_OrientationTypeDef LCD_ORIENTATION_TYPE)
{
    // MEMORY ACCESS CONTROL
    // this register dictates the ORIENTATION of the display screen 
    // NOTE: the 0x86 orienation is not used by the Ukranian guy's library so it is not named as an orientation_config

    writeCmd16(ILI9341_MEMCONTROL); // register 0x36
    writeData16(LCD_ORIENTATION_TYPE); // Iwatake used 0x68, and I use 0x68 for orientation type as default
}


void LCD_Set_Cursor(uint16_t xPos, uint16_t yPos)
{
	LCD_Set_WriteWindow(xPos,yPos,xPos,yPos);

	cursorXY.x = xPos;
	cursorXY.y = yPos;
}

void LCD_Set_TextColor(uint16_t text_color, uint16_t background_color)
{
	lcdFont.TextColor = text_color;
	lcdFont.BackColor = background_color;
}

void LCD_Set_TextWrap(uint8_t w)
{
	lcdFont.TextWrap = w;
}






