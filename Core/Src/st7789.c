/*
 * st7789.c
 *
 *  Created on: Oct 29, 2025
 *      Author: dmitry
 */


#include "st7789.h"

extern SPI_HandleTypeDef hspi1;

int16_t ST77xx_Width, ST77xx_Height;
uint8_t ST77xx_X_Start, ST77xx_Y_Start;

//==============================================================================
// Процедура отправки команды в дисплей
//==============================================================================
void ST77xx_SendCmd(uint8_t Cmd)
{
  ST77xx_DC_LOW();
  ST77xx_CS_LOW();

  //SPI_send8b(ST77xx_SPI_periph, &Cmd, 1);
  HAL_SPI_Transmit(&hspi1, &Cmd, 1, 1000);

  ST77xx_CS_HIGH();
  ST77xx_DC_HIGH();
}
//==============================================================================

//==============================================================================
// Процедура отправки данных (параметров) в дисплей
//==============================================================================
void ST77xx_SendData(uint8_t Data)
{
//  ST77xx_DC_HIGH();
  ST77xx_CS_LOW();

  //SPI_send8b(ST77xx_SPI_periph, &Data, 1);
  HAL_SPI_Transmit(&hspi1, &Data, 1, 1000);

  ST77xx_CS_HIGH();
}
//==============================================================================

void ST77xx_SendDataByteRaw(uint8_t data) {
	HAL_SPI_Transmit(&hspi1, &data, 1, 1000);
}

void ST77xx_SendDataBytesRaw(uint16_t* data, uint16_t len) {
	HAL_SPI_Transmit(&hspi1, (uint8_t*)data, len*2, 1000);
}

uint16_t repeatbuff[1024];
void ST77xx_SendDataByteRawRepeat(uint16_t data, int32_t repeat) {
	uint16_t swapped = (data >> 8) | (data << 8);
	for(int i=0; i<repeat && i<1024; i++) {
		repeatbuff[i] = swapped;
	}
	while(repeat > 0) {
		uint16_t len = repeat >= 1024 ? 1024 : repeat;
		HAL_SPI_Transmit(&hspi1, (uint8_t*)&repeatbuff, len, 1000);
		repeat -= 1024;
	}
}

//==============================================================================
// Процедура программного сброса дисплея
//==============================================================================
void ST77xx_SoftReset(void)
{
  ST77xx_SendCmd(ST77xx_Cmd_SWRESET);
  HAL_Delay(130);
}
//==============================================================================

//==============================================================================
// Процедура аппаратного сброса дисплея (ножкой RESET)
//==============================================================================
void ST77xx_HardReset(void)
{
  ST77xx_RESET_LOW();
  HAL_Delay(10);
  ST77xx_RESET_HIGH();
  HAL_Delay(20);
}
//==============================================================================

//==============================================================================
// Процедура включения/отключения режима сна
//==============================================================================
void ST77xx_SleepMode(uint8_t Mode)
{
  if (Mode)
    ST77xx_SendCmd(ST77xx_Cmd_SLPIN);
  else
    ST77xx_SendCmd(ST77xx_Cmd_SLPOUT);

  HAL_Delay(10);
}
//==============================================================================


//==============================================================================
// Процедура включения/отключения режима частичного заполнения экрана
//==============================================================================
void ST77xx_InversionMode(uint8_t Mode)
{
  if (Mode)
    ST77xx_SendCmd(ST77xx_Cmd_INVON);
  else
    ST77xx_SendCmd(ST77xx_Cmd_INVOFF);
}
//==============================================================================


//==============================================================================
// Процедура включения/отключения питания дисплея
//==============================================================================
void ST77xx_DisplayPower(uint8_t On)
{
  if (On)
    ST77xx_SendCmd(ST77xx_Cmd_DISPON);
  else
    ST77xx_SendCmd(ST77xx_Cmd_DISPOFF);
}
//==============================================================================

//==============================================================================
// Процедура установки начального и конечного адресов колонок
//==============================================================================
static void ST77xx_ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd)
{
  if (ColumnStart > ColumnEnd)
    return;
  if (ColumnEnd > ST77xx_Width)
    return;

  ColumnStart += ST77xx_X_Start;
  ColumnEnd += ST77xx_X_Start;

  ST77xx_SendCmd(ST77xx_Cmd_CASET);
  ST77xx_SendData(ColumnStart >> 8);
  ST77xx_SendData(ColumnStart & 0xFF);
  ST77xx_SendData(ColumnEnd >> 8);
  ST77xx_SendData(ColumnEnd & 0xFF);
}
//==============================================================================


//==============================================================================
// Процедура установки начального и конечного адресов строк
//==============================================================================
static void ST77xx_RowSet(uint16_t RowStart, uint16_t RowEnd)
{
  if (RowStart > RowEnd)
    return;
  if (RowEnd > ST77xx_Height)
    return;

  RowStart += ST77xx_Y_Start;
  RowEnd += ST77xx_Y_Start;

  ST77xx_SendCmd(ST77xx_Cmd_RASET);
  ST77xx_SendData(RowStart >> 8);
  ST77xx_SendData(RowStart & 0xFF);
  ST77xx_SendData(RowEnd >> 8);
  ST77xx_SendData(RowEnd & 0xFF);
}
//==============================================================================


//==============================================================================
// Процедура установка границ экрана для заполнения
//==============================================================================
void ST77xx_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  ST77xx_ColumnSet(x0, x1);
  ST77xx_RowSet(y0, y1);

  ST77xx_SendCmd(ST77xx_Cmd_RAMWR);
}
//==============================================================================


//==============================================================================
// Процедура записи данных в дисплей
//==============================================================================
void ST77xx_RamWrite(uint16_t *pBuff, uint16_t Len)
{
  while (Len--)
  {
    ST77xx_SendData(*pBuff >> 8);
    ST77xx_SendData(*pBuff & 0xFF);
  }
}
//==============================================================================


//==============================================================================
// Процедура выбора кривой гамма-коррекции
//==============================================================================
void ST77xx_GammaSet(uint8_t CurveNum)
{
  if (CurveNum > 4)
    return;

  ST77xx_SendCmd(ST77xx_Cmd_GAMSET);
  ST77xx_SendData(1 << (CurveNum - 1));
}
//==============================================================================


//==============================================================================
// Процедура настройки формата цвета
//==============================================================================
void ST77xx_ColorModeSet(uint8_t ColorMode)
{
  ST77xx_SendCmd(ST77xx_Cmd_COLMOD);
  ST77xx_SendData(ColorMode & 0x77);
}
//==============================================================================


//==============================================================================
// Процедура настройки отображения
//==============================================================================
void ST77xx_MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror, uint8_t HorizMirror, uint8_t IsBGR)
{
  uint8_t Value;
  Rotation &= 7;

  ST77xx_SendCmd(ST77xx_Cmd_MADCTL);

  // Настройка направления заполнения экрана
  switch (Rotation)
  {
  case 0:
    Value = 0;
    break;
  case 1:
    Value = ST77xx_MADCTL_MX;
    break;
  case 2:
    Value = ST77xx_MADCTL_MY;
    break;
  case 3:
    Value = ST77xx_MADCTL_MX | ST77xx_MADCTL_MY;
    break;
  case 4:
    Value = ST77xx_MADCTL_MV;
    break;
  case 5:
    Value = ST77xx_MADCTL_MV | ST77xx_MADCTL_MX;
    break;
  case 6:
    Value = ST77xx_MADCTL_MV | ST77xx_MADCTL_MY;
    break;
  case 7:
    Value = ST77xx_MADCTL_MV | ST77xx_MADCTL_MX | ST77xx_MADCTL_MY;
    break;
  }

  if (VertMirror) {
    Value |= ST77xx_MADCTL_ML;
  }
  if (HorizMirror) {
    Value |= ST77xx_MADCTL_MH;
  }

  // Использование порядка цветов BGR вместо RGB
  if (IsBGR) {
    Value |= ST77xx_MADCTL_BGR;
  }

  ST77xx_SendData(Value);
}
//==============================================================================


//==============================================================================
// Процедура окрашивает 1 пиксель дисплея
//==============================================================================
void ST77xx_DrawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) ||(x >= ST77xx_Width) || (y < 0) || (y >= ST77xx_Height))
    return;

  ST77xx_SetWindow(x, y, x, y);
  ST77xx_RamWrite(&color, 1);
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольника цветом color
//==============================================================================
void ST77xx_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  if ((x >= ST77xx_Width) || (y >= ST77xx_Height))
    return;

  if ((x + w) > ST77xx_Width)
    w = ST77xx_Width - x;

  if ((y + h) > ST77xx_Height)
    h = ST77xx_Height - y;

  ST77xx_SetWindow(x, y, x + w - 1, y + h - 1);

  //  for (uint32_t i = 0; i < (h * w); i++)
  //    ST77xx_RamWrite(&color, 1);

  ST77xx_CS_LOW();
  ST77xx_SendDataByteRawRepeat(color, w*h*2); // 2 bytes per pixel
  ST77xx_CS_HIGH();
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольной области из буфера. Порядок заполнения экрана Y - X
//==============================================================================
void ST77xx_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
  if ((x >= ST77xx_Width) || (y >= ST77xx_Height))
    return;

  if ((x + w - 1) >= ST77xx_Width)
    w = ST77xx_Width - x;

  if ((y + h - 1) >= ST77xx_Height)
    h = ST77xx_Height - y;

  ST77xx_SetWindow(x, y, x + w - 1, y + h - 1);

  for (uint32_t i = 0; i < (h * w); i++)
    ST77xx_RamWrite(pBuff++, 1);
}
//==============================================================================


//==============================================================================
// Процедура заполнения прямоугольной области из буфера. Порядок заполнения экрана X - Y
//==============================================================================
void ST77xx_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
  if ((x >= ST77xx_Width) || (y >= ST77xx_Height))
    return;

  if ((x + w - 1) >= ST77xx_Width)
    w = ST77xx_Width - x;

  if ((y + h - 1) >= ST77xx_Height)
    h = ST77xx_Height - y;

  for (uint16_t iy = y; iy < y + h; iy++)
  {
    ST77xx_SetWindow(x, iy, x + w - 1, iy + 1);
    for (x = w; x > 0; x--)
      ST77xx_RamWrite(pBuff++, 1);
  }
}
//==============================================================================


//==============================================================================
// Процедура закрашивает экран цветом color
//==============================================================================
void ST77xx_FillScreen(uint16_t color)
{
  ST77xx_FillRect(0, 0,  ST77xx_Width, ST77xx_Height, color);
}
//==============================================================================


//==============================================================================
// Процедура инициализации дисплея
//==============================================================================
void ST7789_Init(int16_t Width, int16_t Height)
{
  ST77xx_Width = Width;
  ST77xx_Height = Height;
  ST77xx_X_Start = ST7789_X_Start;
  ST77xx_Y_Start = ST7789_Y_Start;

  // Задержка после подачи питания
  HAL_Delay(40);

  // Сброс дисплея
  ST77xx_HardReset();
  // Отправляем последовательность инициализирующих команд
  ST77xx_SoftReset();

  ST77xx_SleepMode(0);

  ST77xx_ColorModeSet(ST7789_ColorMode_16bit);
  HAL_Delay(10);
  ST77xx_MemAccessModeSet(5, 0, 0, 0);
  HAL_Delay(10);
  ST77xx_FillScreen(0);

  // Включаем подсветку
  //st77xx_SetBL(100);

  ST77xx_InversionMode(1);
  ST77xx_DisplayPower(1);
  HAL_Delay(100);
}
//==============================================================================

void ST77xx_PrepareCharBuffer(char c, FontDef font, uint16_t color, uint16_t bgcolor, uint16_t char_buf[font.height][font.width])
{
    uint32_t b;
    for (uint16_t i = 0; i < font.height; i++) {
        b = font.data[(c - 32) * font.height + i];
        for (uint16_t j = 0; j < font.width; j++) {
            if ((b << j) & 0x8000) {
                char_buf[i][j] = color; // Foreground color
            } else {
                char_buf[i][j] = bgcolor; // Background color
            }
        }
    }
}


/**
 * @brief Write fast a string
 * @param  x&y -> cursor of the start point.
 * @param str -> string to write
 * @param font -> fontstyle of the string
 * @param color -> color of the string
 * @param bgcolor -> background color of the string
 * @return  none
 */
void ST77xx_WriteFastString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint16_t char_buf[font.height][font.width];

    while (*str) {
        if (x + font.width >= ST77xx_Width) {
            x = 0;
            y += font.height;
            if (y + font.height >= ST77xx_Height) {
                break;
            }

            if (*str == ' ') {
                // Skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        ST77xx_PrepareCharBuffer(*str, font, color, bgcolor, char_buf);
        ST77xx_SetWindow(x, y, x + font.width - 1, y + font.height - 1);
        ST77xx_CS_LOW();
        ST77xx_SendDataBytesRaw((uint16_t *)char_buf, font.width * font.height);
        ST77xx_CS_HIGH();

        x += font.width;
        str++;
    }
}

