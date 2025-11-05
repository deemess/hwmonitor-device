/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "st7789.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
char rxbuff[64];
char received[64];
uint8_t rxp = 0;

uint8_t delay = 100;

char cputxt[15];
uint16_t cpu_txt_color = WHITE;
char memtxt[15];
uint16_t mem_txt_color = WHITE;
char gpu1txt[15];
uint16_t gpu1_txt_color = WHITE;
char gpu2txt[15];
uint16_t gpu2_txt_color = WHITE;

uint8_t cpu_hist[150];
uint16_t cpu_hist_color = GREEN;
uint8_t mem_hist[150];
uint16_t mem_hist_color = BLUE;
uint8_t gpu1_hist[150];
uint16_t gpu1_hist_color = MAGENTA;
uint8_t gpu2_hist[150];
uint16_t gpu2_hist_color = MAGENTA;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

int startswith(char* cmd, uint8_t cmdlen, const char* start, uint8_t startlen) {
	int result = 1;
	for(int i=0; i<cmdlen && i<startlen; i++) {
		if(cmd[i] != start[i]) {
			result = 0;
			break;
		}
	}
	return result;
}

void copyfrom(uint8_t cmdpos, char *cmd, uint8_t cmdlen, char *target, uint8_t targetlen) {
	for(int i=0; i<targetlen && i + cmdpos < cmdlen; i++) {
		if(cmd[cmdpos+i] == 0x0A || cmd[cmdpos+i] == 0x0D) {//endline
			break;
		}
		target[i] = cmd[cmdpos+i];
	}
}

uint8_t readnumberfrom(uint8_t cmdpos, char *cmd, uint8_t cmdlen) {
	//Number should be 3 digit length like 001 or 255
	uint8_t res = 0;
	uint8_t p = 3;
	for(int i=0; i+cmdpos < cmdlen && p > 0; i++) {
		if(cmd[cmdpos+i] == 0x0A || cmd[cmdpos+i] == 0x0D) {//endline
			break;
		}
		if(cmd[cmdpos+i] >= '0' && cmd[cmdpos+i] <= '9') {
			uint8_t number = cmd[cmdpos+i] - '0';
			uint8_t mult = pow(10, p-1);
			res = res + number * mult;
			p--;
		}
	}
	return res;
}

void process_cmd(char* cmd, uint8_t cmdlen) {
	if(startswith(cmd, cmdlen, "CPU:", 4)) {
		cpu_hist[0] = readnumberfrom(4, cmd, cmdlen);
	} else if(startswith(cmd, cmdlen, "MEM:", 4)) {
		mem_hist[0] = readnumberfrom(4, cmd, cmdlen);
	} else if(startswith(cmd, cmdlen, "GPU1:", 5)) {
		gpu1_hist[0] = readnumberfrom(5, cmd, cmdlen);
	} else if(startswith(cmd, cmdlen, "GPU2:", 5)) {
		gpu2_hist[0] = readnumberfrom(5, cmd, cmdlen);
	} else if(startswith(cmd, cmdlen, "GPU1TXT:", 8)) {
		copyfrom(8, cmd, cmdlen, gpu1txt, 14);
	} else if(startswith(cmd, cmdlen, "CPUTXT:", 7)) {
		copyfrom(7, cmd, cmdlen, cputxt, 14);
	} else if(startswith(cmd, cmdlen, "MEMTXT:", 7)) {
		copyfrom(7, cmd, cmdlen, memtxt, 14);
	} else if(startswith(cmd, cmdlen, "GPU2TXT:", 8)) {
		copyfrom(8, cmd, cmdlen, gpu2txt, 14);
	} else if(startswith(cmd, cmdlen, "DELAY:", 6)) {
		delay = readnumberfrom(6, cmd, cmdlen);
	} else if(startswith(cmd, cmdlen, "CPUCOL:", 7)) {
		uint16_t r = readnumberfrom(7, cmd, cmdlen);
		uint16_t g = readnumberfrom(10, cmd, cmdlen);
		uint16_t b = readnumberfrom(13, cmd, cmdlen);
		cpu_hist_color =  RGB565(r,g,b);
	} else if(startswith(cmd, cmdlen, "MEMCOL:", 7)) {
		uint16_t r = readnumberfrom(7, cmd, cmdlen);
		uint16_t g = readnumberfrom(10, cmd, cmdlen);
		uint16_t b = readnumberfrom(13, cmd, cmdlen);
		mem_hist_color =  RGB565(r,g,b);
	} else if(startswith(cmd, cmdlen, "GPU1COL:", 8)) {
		uint16_t r = readnumberfrom(8, cmd, cmdlen);
		uint16_t g = readnumberfrom(11, cmd, cmdlen);
		uint16_t b = readnumberfrom(14, cmd, cmdlen);
		gpu1_hist_color =  RGB565(r,g,b);
	} else if(startswith(cmd, cmdlen, "GPU2COL:", 8)) {
		uint16_t r = readnumberfrom(8, cmd, cmdlen);
		uint16_t g = readnumberfrom(11, cmd, cmdlen);
		uint16_t b = readnumberfrom(14, cmd, cmdlen);
		gpu2_hist_color =  RGB565(r,g,b);
	}

}

void USB_Received(uint8_t* pbuf, uint32_t *len) {
	for(int i=0; i<*len; i++) {
		rxbuff[rxp++] = pbuf[i];
		if(pbuf[i] == '\n') {
			rxbuff[rxp++] = 0;
			memcpy(received, rxbuff, rxp);
			rxp = 0;
			process_cmd(received, strlen(received));
			CDC_Transmit_FS((uint8_t*)received, strlen(received));
		}
	}
}

void updategraph() {
	for(int i=149; i>=1; i--) {
		cpu_hist[i] = cpu_hist[i-1];
		mem_hist[i] = mem_hist[i-1];
		gpu1_hist[i] = gpu1_hist[i-1];
		gpu2_hist[i] = gpu2_hist[i-1];
	}
}

void drawall() {
	ST77xx_WriteFastString(0, 0, cputxt, Font_11x18, cpu_txt_color, BLACK);
	ST77xx_WriteFastString(160, 0, memtxt, Font_11x18, mem_txt_color, BLACK);
	ST77xx_WriteFastString(0, 122, gpu1txt, Font_11x18, gpu1_txt_color, BLACK);
	ST77xx_WriteFastString(160, 122, gpu2txt, Font_11x18, gpu2_txt_color, BLACK); //Font_16x26

	for(int i=0; i<150; i++) {
		ST77xx_FillRect(149-i, 19, 1, 100-cpu_hist[i], BLACK);
		ST77xx_FillRect(149-i, 119-cpu_hist[i], 1, cpu_hist[i] == 0 ? 1 : cpu_hist[i], cpu_hist_color);

		ST77xx_FillRect(319-i, 19, 1, 100-mem_hist[i], BLACK);
		ST77xx_FillRect(319-i, 119-mem_hist[i], 1, mem_hist[i] == 0 ? 1 : mem_hist[i], mem_hist_color);

		ST77xx_FillRect(149-i, 139, 1, 100-gpu1_hist[i], BLACK);
		ST77xx_FillRect(149-i, 239-gpu1_hist[i], 1, gpu1_hist[i] == 0 ? 1 : gpu1_hist[i], gpu1_hist_color);

		ST77xx_FillRect(319-i, 139, 1, 100-gpu2_hist[i], BLACK);
		ST77xx_FillRect(319-i, 239-gpu2_hist[i], 1, gpu2_hist[i] == 0 ? 1 : gpu2_hist[i], gpu2_hist_color);
	}
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  strcpy(cputxt,  "CPU           ");
  strcpy(memtxt,  "MEM           ");
  strcpy(gpu1txt, "GPU1          ");
  strcpy(gpu2txt, "GPU2          ");

  ST7789_Init(320, 240);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  drawall();
	  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  HAL_Delay(delay);
	  updategraph();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_CS_Pin|LCD_DC_Pin|LCD_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_DC_Pin LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_DC_Pin|LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
