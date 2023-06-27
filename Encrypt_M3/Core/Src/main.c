/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
I2C_HandleTypeDef hi2c2;

USART_HandleTypeDef husart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

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
  MX_USART1_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  // read configuration zone: {COMMAND, COUNT, OPCODE, ZONE, ADDRESS_1, ADDRESS_2, CRC_LSB, CRC_MSB}
  uint8_t readCommand0[8] = {0x03, 0x07, 0x02, 0x80, 0x00, 0x00, 0x09, 0xAD}; // read -> param1 = zone = 1000 0000
  uint8_t readCommand1[8] = {0x03, 0x07, 0x02, 0x80, 0x08, 0x00, 0x0a, 0x4d}; // read -> param1 = zone = 1000 0000
  uint8_t readCommand2[8] = {0x03, 0x07, 0x02, 0x00, 0x10, 0x00, 0x1d, 0x9d}; // read -> param1 = zone = 1000 0000
  uint8_t readCommand3[8] = {0x03, 0x07, 0x02, 0x00, 0x11, 0x00, 0x14, 0x1d};
  uint8_t readCommand4[8] = {0x03, 0x07, 0x02, 0x00, 0x12, 0x00, 0x1b, 0x1d};
  uint8_t readCommand5[8] = {0x03, 0x07, 0x02, 0x00, 0x13, 0x00, 0x12, 0x9d};
  uint8_t readCommand6[8] = {0x03, 0x07, 0x02, 0x00, 0x14, 0x00, 0x1e, 0xdd};
  uint8_t readCommand7[8] = {0x03, 0x07, 0x02, 0x00, 0x15, 0x00, 0x17, 0x5d};


  uint8_t read_byte[4];
  uint8_t receiv_ack[1] = {0};

  //uint8_t read_config[32] = {0};
  uint8_t read_config0[35] = {0};
  uint8_t read_config1[35] = {0};
  uint8_t read_config2[7] = {0};
  uint8_t read_config3[7] = {0};
  uint8_t read_config4[7] = {0};
  uint8_t read_config5[7] = {0};
  uint8_t read_config6[7] = {0};
  uint8_t read_config7[7] = {0};
  uint8_t read_config8[7] = {0};

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  WakeUp(read_byte);
	 // ReadConfig(readCommand0, read_byte1, read_config);
	 // WriteConfigZone();
	  ReadConfig(readCommand0, 35, read_config0);
	  ReadConfig(readCommand1, 35, read_config1);
	  ReadConfig(readCommand2, 4, read_config2);
	  ReadConfig(readCommand3, 4, read_config3);
	  ReadConfig(readCommand4, 4, read_config4);
	  ReadConfig(readCommand5, 4, read_config5);
	  ReadConfig(readCommand6, 4, read_config6);
	  ReadConfig(readCommand7, 4, read_config7);

	 // BlockConfigZone(receiv_ack);

	 // ReadConfig(readCommand7, 4, read_config8);
	  HAL_Delay(10);
	  // fazer a configuração de todos os slots, travar a configuração

	  // WRITE MASTERKEY
/*
	  // comando para bloqueio da zona de configuração do slot
	  uint8_t configBlock[8] = {};
	  HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configBlock, 8, 1000);
	  HAL_Delay(5);

	  // MASTERKEY (32bytes)
	  uint8_t masterKEY[32] = {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9};

	  // comando para envio da MASTERKEY no SLOT ID 0x800E
	  uint8_t writeKEY[8] = {0x03, 0x07, 0x12, 0x80, 0x0E, 0x00, 0x0C, 0xAD};
	  HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeKEY, 8, 1000);
	  HAL_Delay(5);
	  HAL_I2C_Master_Transmit(&hi2c2, 0xC8, masterKEY, 32, 1000);
	  HAL_Delay(5);

	  // comando para bloqueio da zona de dados do slot 0x800E
	  uint8_t configData[8] = {};
	  HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configData, 8, 1000);
	  HAL_Delay(5);

	  // leitura para ver se funcionou
	  uint8_t readKEY[8] = {0x03, 0x07, 0x02, 0x80, 0x0E, 0x00, 0x0F, 0x8D};
	  HAL_I2C_Master_Transmit(&hi2c2, 0xC8, readKEY, 8, 1000);
	  HAL_Delay(5);
	  HAL_I2C_Master_Receive(&hi2c2, 0xC8, rec_MasterKey, 32, 1000);
	  HAL_Delay(5);
*/
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  husart1.Instance = USART1;
  husart1.Init.BaudRate = 115200;
  husart1.Init.WordLength = USART_WORDLENGTH_8B;
  husart1.Init.StopBits = USART_STOPBITS_1;
  husart1.Init.Parity = USART_PARITY_NONE;
  husart1.Init.Mode = USART_MODE_TX_RX;
  husart1.Init.CLKPolarity = USART_POLARITY_LOW;
  husart1.Init.CLKPhase = USART_PHASE_1EDGE;
  husart1.Init.CLKLastBit = USART_LASTBIT_DISABLE;
  if (HAL_USART_Init(&husart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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

#ifdef  USE_FULL_ASSERT
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
