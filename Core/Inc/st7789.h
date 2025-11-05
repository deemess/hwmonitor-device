/*
 * st7789.h
 *
 *  Created on: Oct 29, 2025
 *      Author: dmitry
 */

#ifndef INC_ST7789_H_
#define INC_ST7789_H_

#include "main.h"
#include "fonts.h"

#define ST77xx_CS_HIGH()	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define ST77xx_CS_LOW()		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define ST77xx_RESET_HIGH()	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
#define ST77xx_RESET_LOW()	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
#define ST77xx_DC_HIGH()	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
#define ST77xx_DC_LOW()		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)

#define RGB565(r, g, b)         (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

// Базовые цвета
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

// Битовые маски настройки цветности ST7789
#define ST7789_ColorMode_65K    0x50
#define ST7789_ColorMode_262K   0x60
#define ST7789_ColorMode_12bit  0x03
#define ST7789_ColorMode_16bit  0x05
#define ST7789_ColorMode_18bit  0x06
#define ST7789_ColorMode_16M    0x07

// Смещение матрицы относительно строк/столбцов контроллера
#define ST7789_X_Start          0
#define ST7789_Y_Start          0

// Набор команд
//#define ST7735_Cmd_NOP          0x00
#define ST77xx_Cmd_SWRESET      0x01
//#define ST7735_Cmd_RDDID        0x04
//#define ST7735_Cmd_RDDST        0x09
#define ST77xx_Cmd_SLPIN        0x10
#define ST77xx_Cmd_SLPOUT       0x11
#define ST77xx_Cmd_PTLON        0x12
#define ST77xx_Cmd_NORON        0x13
#define ST77xx_Cmd_INVOFF       0x20
#define ST77xx_Cmd_INVON        0x21
#define ST77xx_Cmd_GAMSET       0x26
#define ST77xx_Cmd_DISPOFF      0x28
#define ST77xx_Cmd_DISPON       0x29
#define ST77xx_Cmd_CASET        0x2A
#define ST77xx_Cmd_RASET        0x2B
#define ST77xx_Cmd_RAMWR        0x2C
//#define ST7735_Cmd_RAMRD        0x2E
#define ST77xx_Cmd_PTLAR        0x30
#define ST77xx_Cmd_COLMOD       0x3A
#define ST77xx_Cmd_MADCTL       0x36    // Memory data access control

#define ST7735_Cmd_FRMCTR1      0xB1    // Frame Rate Control in normal mode
#define ST7735_Cmd_FRMCTR2      0xB2    // Frame Rate Control in idle mode
#define ST7735_Cmd_FRMCTR3      0xB3    // Frame Rate Control in partial mode
#define ST7735_Cmd_INVCTR       0xB4
#define ST7735_Cmd_DISSET5      0xB6    // Display Function set 5
#define ST7735_Cmd_PWCTR1       0xC0    // Power control 1
#define ST7735_Cmd_PWCTR2       0xC1    // Power control 2
#define ST7735_Cmd_PWCTR3       0xC2    // Power control 3
#define ST7735_Cmd_PWCTR4       0xC3    // Power control 4
#define ST7735_Cmd_PWCTR5       0xC4    // Power control 5
#define ST7735_Cmd_VMCTR1       0xC5    // VCOM Control 1

#define ST7789_Cmd_MADCTL_MY    0x80
#define ST7789_Cmd_MADCTL_MX    0x40
#define ST7789_Cmd_MADCTL_MV    0x20
#define ST7789_Cmd_MADCTL_ML    0x10
#define ST7789_Cmd_MADCTL_RGB   0x00

#define ST7789_Cmd_RDID1        0xDA
#define ST7789_Cmd_RDID2        0xDB
#define ST7789_Cmd_RDID3        0xDC
#define ST7789_Cmd_RDID4        0xDD
//==============================================================================
#define ST7735_ColorMode_12bit  0x03
#define ST7735_ColorMode_16bit  0x05
#define ST7735_ColorMode_18bit  0x06

#define ST77xx_MADCTL_MY        0x80
#define ST77xx_MADCTL_MX        0x40
#define ST77xx_MADCTL_MV        0x20
#define ST77xx_MADCTL_ML        0x10
#define ST77xx_MADCTL_BGR       0x08
#define ST77xx_MADCTL_MH        0x04

// Процедура инициализации дисплея
void ST7789_Init(int16_t Width, int16_t Height);

// Процедура отправки команды в дисплей
void ST77xx_SendCmd(uint8_t Cmd);
// Процедура отправки данных (параметров) в дисплей
void ST77xx_SendData(uint8_t Data);
// Процедура аппаратного сброса дисплея (ножкой RESET)
void ST77xx_HardReset(void);
// Процедура программного сброса дисплея
void ST77xx_SoftReset(void);
// Процедура включения/отключения режима сна
void ST77xx_SleepMode(uint8_t Mode);
// Процедура включения/отключения режима частичного заполнения экрана
void ST77xx_InversionMode(uint8_t Mode);
// Процедура включения/отключения питания дисплея
void ST77xx_DisplayPower(uint8_t On);
// Процедура выбора кривой гамма-коррекции
void ST77xx_GammaSet(uint8_t CurveNum);
// Процедура настройки формата цвета
void ST77xx_ColorModeSet(uint8_t ColorMode);
// Процедура настройки отображения
void ST77xx_MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror, uint8_t HorizMirror, uint8_t IsBGR);
// Процедура установка границ экрана для заполнения
void ST77xx_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
// Процедура закрашивает экран цветом color
void ST77xx_FillScreen(uint16_t color);
// Процедура окрашивает 1 пиксель дисплея
void ST77xx_DrawPixel(int16_t x, int16_t y, uint16_t color);
// Процедура заполнения прямоугольника цветом color
void ST77xx_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
// Процедура заполнения прямоугольной области из буфера. Порядок заполнения экрана Y - X
void ST77xx_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff);
// Процедура заполнения прямоугольной области из буфера. Порядок заполнения экрана X - Y
void ST77xx_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff);

void ST77xx_WriteFastString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);

#endif /* INC_ST7789_H_ */
